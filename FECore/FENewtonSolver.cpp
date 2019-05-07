/*This file is part of the FEBio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio.txt for details.

Copyright (c) 2019 University of Utah, The Trustees of Columbia University in 
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/


#include "stdafx.h"
#include "FENewtonSolver.h"
#include "NumCore/NumCore.h"
#include "FENodeReorder.h"
#include "FEModel.h"
#include "FEGlobalMatrix.h"
#include "BFGSSolver.h"
#include "JFNKStrategy.h"
#include "FEBroydenStrategy.h"
#include "FELinearConstraintManager.h"
#include "FEAnalysis.h"
#include "BC.h"
#include "log.h"
#include "sys.h"

//-----------------------------------------------------------------------------
// define the parameter list
BEGIN_PARAMETER_LIST(FENewtonSolver, FESolver)
	ADD_PARAMETER2(m_lineSearch->m_LStol , FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "lstol"   );
	ADD_PARAMETER2(m_lineSearch->m_LSmin , FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "lsmin"   );
	ADD_PARAMETER2(m_lineSearch->m_LSiter, FE_PARAM_INT   , FE_RANGE_GREATER_OR_EQUAL(0), "lsiter"  );
	ADD_PARAMETER2(m_maxref             , FE_PARAM_INT   , FE_RANGE_GREATER_OR_EQUAL(0.0), "max_refs");
	ADD_PARAMETER2(m_maxups             , FE_PARAM_INT   , FE_RANGE_GREATER_OR_EQUAL(0.0), "max_ups" );
	ADD_PARAMETER2(m_max_buf_size       , FE_PARAM_INT   , FE_RANGE_GREATER_OR_EQUAL(0), "qn_max_buffer_size");
	ADD_PARAMETER(m_cycle_buffer        , FE_PARAM_BOOL  , "qn_cycle_buffer");
	ADD_PARAMETER2(m_cmax               , FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "cmax"    );
	ADD_PARAMETER(m_nqnmethod           , FE_PARAM_INT   , "qnmethod");
	ADD_PARAMETER(m_bzero_diagonal      , FE_PARAM_BOOL  , "check_zero_diagonal");
	ADD_PARAMETER(m_zero_tol            , FE_PARAM_DOUBLE, "zero_diagonal_tol"  );
	ADD_PARAMETER(m_eq_scheme           , FE_PARAM_INT   , "equation_scheme");
	ADD_PARAMETER(m_force_partition     , FE_PARAM_INT   , "force_partition");
	ADD_PARAMETER(m_breformtimestep     , FE_PARAM_BOOL  , "reform_each_time_step");
	ADD_PARAMETER(m_bdivreform          , FE_PARAM_BOOL  , "diverge_reform");
	ADD_PARAMETER(m_bdoreforms          , FE_PARAM_BOOL  , "do_reforms"  );
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FENewtonSolver::FENewtonSolver(FEModel* pfem) : FESolver(pfem)
{
	m_lineSearch = new FELineSearch(this);
	m_ls = 0.0;

	// default parameters
	m_maxref = 15;

	m_nref = 0;

    m_neq = 0;
    m_plinsolve = 0;
	m_pK = 0;

	m_cmax   = 1e5;
	m_maxups = 10;
	m_max_buf_size = 0;
	m_cycle_buffer = true;
	m_nqnmethod = QN_BFGS;
	m_strategy = 0;

	m_bforceReform = true;
	m_bdivreform = true;
	m_bdoreforms = true;

	m_bzero_diagonal = true;
	m_zero_tol = 0.0;

	m_force_partition = 0;
	m_breformtimestep = true;

	m_eq_scheme = EQUATION_SCHEME::STAGGERED;
}

//-----------------------------------------------------------------------------
//! Set the default solution strategy
void FENewtonSolver::SetDefaultStrategy(QN_STRATEGY qn)
{
	m_nqnmethod = qn;
}

//-----------------------------------------------------------------------------
void FENewtonSolver::SetSolutionStrategy(FENewtonStrategy* pstrategy)
{
	if (m_strategy) delete m_strategy;
	m_strategy = pstrategy;
}

//-----------------------------------------------------------------------------
FENewtonSolver::~FENewtonSolver()
{
	delete m_plinsolve;	// clean up linear solver data
	delete m_pK;		// clean up stiffnes matrix data
	if (m_strategy) delete m_strategy;
}

//-----------------------------------------------------------------------------
FEGlobalMatrix& FENewtonSolver::GetStiffnessMatrix()
{
	return *m_pK;
}

//-----------------------------------------------------------------------------
//! Check the zero diagonal
void FENewtonSolver::CheckZeroDiagonal(bool bcheck, double ztol)
{
	m_bzero_diagonal = bcheck;
	m_zero_tol = fabs(ztol);
}

//-----------------------------------------------------------------------------
void FENewtonSolver::AssembleStiffness(vector<int>& en, vector<int>& lm, matrix& ke)
{
	if (lm.size() == 0) return;

	// assemble into the global stiffness
	m_pK->Assemble(ke, lm);

	// adjust for linear constraints
	FELinearConstraintManager& LCM = m_fem.GetLinearConstraintManager();
	if (LCM.LinearConstraints() > 0)
	{
		LCM.AssembleStiffness(*m_pK, m_Fd, m_ui, en, lm, ke);
	}

	// if there are prescribed bc's we need to adjust the residual
	SparseMatrix& K = *m_pK;
	int N = ke.rows();
	for (int j = 0; j<N; ++j)
	{
		int J = -lm[j] - 2;
		if ((J >= 0) && (J<m_neq))
		{
			// dof j is a prescribed degree of freedom

			// loop over rows
			for (int i = 0; i<N; ++i)
			{
				int I = lm[i];
				if (I >= 0)
				{
					// dof i is not a prescribed degree of freedom
					m_Fd[I] -= ke[i][j] * m_ui[J];
				}
			}
			// set the diagonal element of K to 1
			K.set(J, J, 1);
		}
	}
}

//-----------------------------------------------------------------------------
//! Reforms a stiffness matrix and factorizes it
bool FENewtonSolver::ReformStiffness()
{
	felog.printf("Reforming stiffness matrix: reformation #%d\n\n", m_nref + 1);

    // first, let's make sure we have not reached the max nr of reformations allowed
    if (m_nref >= m_maxref) throw MaxStiffnessReformations();
    
    // recalculate the shape of the stiffness matrix if necessary
    if (m_breshape)
    {
        // TODO: I don't think I need to update here
        //		if (m_fem.m_bcontact) UpdateContact();
        
        // reshape the stiffness matrix
        if (!CreateStiffness(m_niter == 0)) return false;
        
        // reset reshape flag, except for contact
		m_breshape = (((m_fem.SurfacePairConstraints() > 0) || (m_fem.NonlinearConstraints() > 0)) ? true : false);
    }
    
    // calculate the global stiffness matrix
	bool bret = false;
	{
		TRACK_TIME("stiffness");

		// zero the stiffness matrix
		m_pK->Zero();

		// Zero the rhs adjustment vector
		zero(m_Fd);

		// calculate the global stiffness matrix
	    bret = StiffnessMatrix();

		// check for zero diagonals
		if (m_bzero_diagonal)
		{
			// get the stiffness matrix
			SparseMatrix& K = *m_pK;
			vector<int> zd;
			int neq = K.Rows();
			for (int i=0; i<neq; ++i)
			{
				double di = fabs(K.diag(i));
				if (di <= m_zero_tol) zd.push_back(i);
			}

			if (zd.empty() == false) throw ZeroDiagonal(-1, -1);
		}
	}

	// if the stiffness matrix was evaluated successfully,
	// we factor it.
    if (bret)
    {
        {
			TRACK_TIME("solve");
			// factorize the stiffness matrix
            m_plinsolve->Factor();
        }
        
        // increase total nr of reformations
        m_nref++;
        m_ntotref++;
        
        // reset bfgs update counter
		m_strategy->m_nups = 0;
    }
    
    return bret;
}

//-----------------------------------------------------------------------------
//!  Creates the global stiffness matrix
//! \todo Can we move this to the FEGlobalMatrix::Create function?
bool FENewtonSolver::CreateStiffness(bool breset)
{
	{
		TRACK_TIME("reform");
		// clean up the solver
		if (m_pK->NonZeroes()) m_plinsolve->Destroy();

		// clean up the stiffness matrix
		m_pK->Clear();

		// create the stiffness matrix
		felog.printf("===== reforming stiffness matrix:\n");
		if (m_pK->Create(&GetFEModel(), m_neq, breset) == false) 
		{
			felog.printf("FATAL ERROR: An error occured while building the stiffness matrix\n\n");
			return false;
		}
		else
		{
			// output some information about the direct linear solver
			int neq = m_pK->Rows();
			int nnz = m_pK->NonZeroes();
			felog.printf("\tNr of equations ........................... : %d\n", neq);
			felog.printf("\tNr of nonzeroes in stiffness matrix ....... : %d\n", nnz);
			felog.printf("\n");
		}
		// let's flush the logfile to make sure the last output will not get lost
		felog.flush();
	}

	// Do the preprocessing of the solver
	{
		TRACK_TIME("solve");
		if (!m_plinsolve->PreProcess())
		{
			// TODO: get rid of throwing this exception. We should just return false.
			throw FatalError();
		}
	}

	// done!
	return true;
}

//-----------------------------------------------------------------------------
void FENewtonSolver::SetPartitions(vector<int>& part)
{
	m_part = part;
}

//-----------------------------------------------------------------------------
bool FENewtonSolver::Init()
{
	// Base class initialization and validation
	if (FESolver::Init() == false) return false;

	// choose a solution strategy
	switch (m_nqnmethod)
	{
	case QN_BFGS   : SetSolutionStrategy(new BFGSSolver       (this) ); break;
	case QN_BROYDEN: SetSolutionStrategy(new FEBroydenStrategy(this)); break;
	case QN_JFNK   : SetSolutionStrategy(new JFNKStrategy     (this)); break;
	// NOTE: Temporary hack for backward compatibility since the BFGSSolver2 was deprecated
	// This solver used to have the value 1 and Broyden 2, but 1 is now used for Broyden.
	case 2: SetSolutionStrategy(new FEBroydenStrategy(this)); break;
	default:
		return false;
	}

	// copy some solution parameters
	m_strategy->m_maxups = m_maxups;
	m_strategy->m_max_buf_size = m_max_buf_size;
	m_strategy->m_cycle_buffer = m_cycle_buffer;
	m_strategy->m_cmax = m_cmax;

	// initialize the linear system
	if (InitLinearSystem() == false)
	{
		return false;
	}

	// allocate data vectors
	m_R0.assign(m_neq, 0);
	m_R1.assign(m_neq, 0);
	m_ui.assign(m_neq, 0);
	m_Fd.assign(m_neq, 0);

	return true;
}

//-----------------------------------------------------------------------------
//! initialize linear system
bool FENewtonSolver::InitLinearSystem()
{
	// Now that we have determined the equation numbers we can continue
	// with creating the stiffness matrix. First we select the linear solver
	// The stiffness matrix is created in CreateStiffness
	// Note that if a particular solver was requested in the input file
	// then the solver might already be allocated. That's way we need to check it.
	if (m_plinsolve == 0)
	{
		FECoreKernel& fecore = FECoreKernel::GetInstance();
		m_plinsolve = fecore.CreateLinearSolver(m_fem.GetLinearSolverType());
		if (m_plinsolve == 0)
		{
			felog.printbox("FATAL ERROR", "Unknown solver type selected\n");
			return false;
		}

		if (m_part.empty() == false)
		{
			m_plinsolve->SetPartitions(m_part);
		}
	}

	// initialize strategy data
	// Must be done after initialization of linear solver
	m_strategy->Init(m_neq, m_plinsolve);

	// set the create stiffness matrix flag
	m_breshape = true;

	// allocate storage for the sparse matrix that will hold the stiffness matrix data
	// we let the linear solver allocate the correct type of matrix format
	SparseMatrix* pS = m_strategy->CreateSparseMatrix(m_bsymm ? REAL_SYMMETRIC : REAL_UNSYMMETRIC);
	if ((pS == 0) && m_bsymm)
	{
		// oh, oh, something went wrong. It's probably because the user requested a symmetric matrix for a 
		// solver that wants a non-symmetric. If so, let's force a non-symmetric format.
		pS = m_strategy->CreateSparseMatrix(REAL_UNSYMMETRIC);

		if (pS)
		{
			// Problem solved! Let's inform the user.
			m_bsymm = false;
			felog.printbox("WARNING", "The matrix format was changed to non-symmetric since the selected\nlinear solver does not support a symmetric format. \n");
		}
	}

	// if the sparse matrix is still zero, we have a problem
	if (pS == 0)
	{
		felog.printbox("FATAL ERROR", "The selected linear solver does not support the requested\n matrix format.\nPlease select a different linear solver.\n");
		return false;
	}

	// clean up the stiffness matrix if we have one
	if (m_pK) delete m_pK; m_pK = 0;

	// Create the stiffness matrix.
	// Note that this does not construct the stiffness matrix. This
	// is done later in the CreateStiffness routine.
	m_pK = new FEGlobalMatrix(pS);
	if (m_pK == 0)
	{
		felog.printbox("FATAL ERROR", "Failed allocating stiffness matrix\n\n");
		return false;
	}

	// Set the partitioning of the global matrix
	// This is only used for debugging block solvers for problems that
	// usually don't generate a block structure
	if (m_force_partition > 0) m_plinsolve->SetPartition(m_force_partition);

	return true;
}

//-----------------------------------------------------------------------------
//!	This function initializes the equation system.
//! It is assumed that all free dofs up until now have been given an ID >= 0
//! and the fixed or rigid dofs an ID < 0.
//! After this operation the nodal ID array will contain the equation
//! number assigned to the corresponding degree of freedom. To distinguish
//! between free or unconstrained dofs and constrained ones the following rules
//! apply to the ID array:
//!
//!           /
//!          |  >=  0 --> dof j of node i is a free dof
//! ID[i][j] <  == -1 --> dof j of node i is a fixed (no equation assigned too)
//!          |  <  -1 --> dof j of node i is constrained and has equation nr = -ID[i][j]-2
//!           \
//!
bool FENewtonSolver::InitEquations()
{
    // get the mesh
    FEMesh& mesh = m_fem.GetMesh();
    
    // initialize nr of equations
    int neq = 0;
    
    // see if we need to optimize the bandwidth
    if (m_fem.OptimizeBandwidth())
    {
		assert(m_eq_scheme == EQUATION_SCHEME::STAGGERED);
        // reorder the node numbers
        vector<int> P(mesh.Nodes());
        FENodeReorder mod;
        mod.Apply(mesh, P);
        
        // set the equation numbers
        for (int i=0; i<mesh.Nodes(); ++i)
        {
            FENode& node = mesh.Node(P[i]);
            for (int j=0; j<(int)node.m_ID.size(); ++j)
            {
                if      (node.m_ID[j] == DOF_FIXED     ) { node.m_ID[j] = -1; }
                else if (node.m_ID[j] == DOF_OPEN      ) { node.m_ID[j] =  neq++; }
                else if (node.m_ID[j] == DOF_PRESCRIBED) { node.m_ID[j] = -neq-2; neq++; }
                else { assert(false); return false; }
            }
        }
    }
    else
    {
		if (m_eq_scheme == EQUATION_SCHEME::STAGGERED)
		{
			// give all free dofs an equation number
			for (int i=0; i<mesh.Nodes(); ++i)
			{
				FENode& node = mesh.Node(i);
				for (int j=0; j<(int)node.m_ID.size(); ++j)
				{
					if      (node.m_ID[j] == DOF_FIXED     ) { node.m_ID[j] = -1; }
					else if (node.m_ID[j] == DOF_OPEN      ) { node.m_ID[j] =  neq++; }
					else if (node.m_ID[j] == DOF_PRESCRIBED) { node.m_ID[j] = -neq-2; neq++; }
					else { assert(false); return false; }
				}
			}
		}
		else
		{
			assert(m_eq_scheme == EQUATION_SCHEME::BLOCK);

			// Assign equations numbers in blocks
			DOFS& dofs = m_fem.GetDOFS();
			for (int nv=0; nv<dofs.Variables(); ++nv)
			{
				int n = dofs.GetVariableSize(nv);
				for (int l=0; l<n; ++l)
				{
					int nl = dofs.GetDOF(nv, l);

					for (int i = 0; i<mesh.Nodes(); ++i)
					{
						FENode& node = mesh.Node(i);
						if      (node.m_ID[nl] == DOF_FIXED     ) { node.m_ID[nl] = -1; }
						else if (node.m_ID[nl] == DOF_OPEN      ) { node.m_ID[nl] = neq++; }
						else if (node.m_ID[nl] == DOF_PRESCRIBED) { node.m_ID[nl] = -neq - 2; neq++; }
						else { assert(false); return false; }
					}
				}
			}
		}
    }
    
    // store the number of equations
    m_neq = neq;
    
    // All initialization is done
    return true;
}

//-----------------------------------------------------------------------------
//! Clean
//! \todo Why can this not be done in destructor?
void FENewtonSolver::Clean()
{
	if (m_plinsolve) m_plinsolve->Destroy();
}

//-----------------------------------------------------------------------------
void FENewtonSolver::Serialize(DumpStream& ar)
{
	FESolver::Serialize(ar);

	if (m_lineSearch) m_lineSearch->Serialize(ar);

	if (ar.IsShallow()) return;

	if (ar.IsSaving())
	{
		ar << m_neq;
		ar << m_maxref;
		ar << m_nref;

		if (m_strategy == 0) ar << 0; else ar << 1;
		if (m_strategy)
		{
			ar << m_nqnmethod;
			ar << m_strategy->m_maxups;
			ar << m_strategy->m_max_buf_size;
			ar << m_strategy->m_cycle_buffer;
			ar << m_strategy->m_cmax;
			ar << m_strategy->m_nups;
		}

		ar << m_R0 << m_R1 << m_ui << m_Fd;
	}
	else
	{
		ar >> m_neq;
		ar >> m_maxref;
		ar >> m_nref;

		int n = -1;
		ar >> n;
		if (n)
		{
			ar >> m_nqnmethod;
			switch (m_nqnmethod)
			{
			case QN_BFGS: SetSolutionStrategy(new BFGSSolver(this)); break;
			case QN_BROYDEN: SetSolutionStrategy(new FEBroydenStrategy(this)); break;
			case QN_JFNK: SetSolutionStrategy(new JFNKStrategy(this)); break;
			// NOTE: Temporary hack for backward compatibility since the BFGSSolver2 was deprecated
			// This solver used to have the value 1 and Broyden 2, but 1 is now used for Broyden.
			case 2: SetSolutionStrategy(new FEBroydenStrategy(this)); break;
			default:
				return;
			}

			ar >> m_strategy->m_maxups;
			ar >> m_strategy->m_max_buf_size;
			ar >> m_strategy->m_cycle_buffer;
			ar >> m_strategy->m_cmax;
			ar >> m_strategy->m_nups;
		}

		// realloc data
		ar >> m_R0 >> m_R1 >> m_ui >> m_Fd;

		// re-initialize the linear system
		if (m_neq) InitLinearSystem();
	}
}

//-----------------------------------------------------------------------------
//!  This function mainly calls the Quasin routine 
//!  and deals with exceptions that require the immediate termination of
//!	quasi-Newton iterations.
bool FENewtonSolver::SolveStep()
{
	bool bret;

	// initialize counters
	m_niter = 0;	// nr of iterations
	m_nrhs = 0;		// nr of RHS evaluations
	m_nref = 0;		// nr of stiffness reformations
	m_ntotref = 0;
	m_naug = 0;		// nr of augmentations

	try
	{
		// let's try to call Quasin
		bret = Quasin();
	}
	catch (NegativeJacobian e)
	{
		// A negative jacobian was detected
		felog.printbox("ERROR","Negative jacobian was detected at element %d at gauss point %d\njacobian = %lg\n", e.m_iel, e.m_ng+1, e.m_vol);
		return false;
	}
	catch (MaxStiffnessReformations)
	{
		// max nr of reformations is reached
		felog.printbox("ERROR", "Max nr of reformations reached.");
		return false;
	}
	catch (ForceConversion)
	{
		// user forced conversion of problem
		felog.printbox("WARNING", "User forced conversion.\nSolution might not be stable.");
		return true;
	}
	catch (IterationFailure)
	{
		// user caused a forced iteration failure
		felog.printbox("WARNING", "User forced iteration failure.");
		return false;
	}
	catch (MaxResidualError)
	{
		// user caused a forced iteration failure
		felog.printbox("WARNING", "Maximum residual exceeded.");
		return false;
	}
	catch (ZeroLinestepSize)
	{
		// a zero line step size was detected
		felog.printbox("ERROR", "Zero line step size.");
		return false;
	}
	catch (EnergyDiverging)
	{
		// problem was diverging after stiffness reformation
		felog.printbox("ERROR", "Problem diverging uncontrollably.");
		return false;
	}
	catch (FEMultiScaleException e)
	{
		// the RVE problem didn't solve
		// logging was turned off during multi-scale runs
		// so we need to turn it back on
		felog.SetMode(Logfile::LOG_SCREEN);
		felog.printbox("ERROR", "The RVE problem has failed at element %d, gauss point %d.\nAborting macro run.", e.elemId, e.gptIndex+1);

		return false;
	}
	catch (DoRunningRestart)
	{
		// a request to fail the iteration and restart the time step
		return false;
	}

	if (bret)
	{
		// print a convergence summary to the felog file
		Logfile::MODE mode = felog.GetMode();
		if (mode != Logfile::LOG_NEVER)
		{
			felog.SetMode(Logfile::LOG_FILE);
			felog.printf("\nconvergence summary\n");
			felog.printf("    number of iterations   : %d\n", m_niter);
			felog.printf("    number of reformations : %d\n", m_nref);
			felog.SetMode(mode);
		}
	}

	return bret;
}

//-----------------------------------------------------------------------------
bool FENewtonSolver::Quasin()
{
	// initialize counters
	m_niter = 0;		// nr of iterations
	m_nrhs = 0;			// nr of RHS evaluations
	m_nref = 0;			// nr of stiffness reformations
	m_ntotref = 0;
	m_strategy->m_nups = 0;	// nr of stiffness updates between reformations

	FEModel& fem = GetFEModel();
	FETimeInfo& tp = fem.GetTime();

	// Do the pre-solve domain update
	FEMesh& mesh = fem.GetMesh();
	for (int i = 0; i<mesh.Domains(); ++i) mesh.Domain(i).PreSolveUpdate(tp);

	// set-up the prescribed displacements
	zero(m_ui);
	int nbc = fem.PrescribedBCs();
	for (int i = 0; i<nbc; ++i)
	{
		FEPrescribedDOF& dc = dynamic_cast<FEPrescribedDOF&>(*m_fem.PrescribedBC(i));
		if (dc.IsActive()) dc.PrepStep(m_ui);
	}

	// Initialize QN method
	QNInit();

	// Start the quasi-Newton loop
	bool bconv = false;
	do
	{
		felog.printf(" %d\n", m_niter + 1);

		// solve the equations (returns line search; solution stored in m_ui)
		double ls = QNSolve();

		felog.printf(" Nonlinear solution status: time= %lg\n", tp.currentTime);
		felog.printf("\tstiffness updates             = %d\n", m_strategy->m_nups);
		felog.printf("\tright hand side evaluations   = %d\n", m_nrhs);
		felog.printf("\tstiffness matrix reformations = %d\n", m_nref);

		// check convergence
		bconv = CheckConvergence(m_niter, m_ui, ls);

		// if we did not converge, do QN update
		if (bconv == false)
		{
			// do the QN update (this may also do a stiffness reformation if necessary)
			bool bret = QNUpdate();

			// Oh, oh, something went wrong
			if (bret == false) break;
		}

		// increase iteration number
		m_niter++;

		// let's flush the logfile to make sure the last output will not get lost
		felog.flush();

		// do minor iterations callbacks
		m_fem.DoCallback(CB_MINOR_ITERS);
	}
	while (!bconv);

	return bconv;
}

//-----------------------------------------------------------------------------
//! Solve the linear system of equations.
//! x is the solution vector
//! R is the right-hand-side vector
void FENewtonSolver::SolveLinearSystem(vector<double>& x, vector<double>& R)
{
	// solve the equations
	if (m_plinsolve->BackSolve(x, R) == false)
		throw LinearSolverFailed();
}

//-----------------------------------------------------------------------------
//! rewind solver
//! This is called when the time step failed.
void FENewtonSolver::Rewind()
{
	// reset the forceReform flag so that we reform the stiffness matrix
	m_bforceReform = true;
}

//-----------------------------------------------------------------------------
//! call this at the start of the quasi-newton loop (after PrepStep)
bool FENewtonSolver::QNInit()
{
	// see if we reform at the start of every time step
	bool breform = m_breformtimestep;

	// if the force reform flag was set, we force a reform
	// (This will be the case for the first time this is called, or when the previous time step failed)
	if (m_bforceReform)
	{
		breform = true;

		m_bforceReform = false;
	}

	m_strategy->PreSolveUpdate();

	// do the reform
	if (breform)
	{
		// do the first stiffness formation
		if (m_strategy->ReformStiffness() == false) return false;
	}

	// calculate initial residual
	if (Residual(m_R0) == false) return false;

	// add the contribution from prescribed dofs
	m_R0 += m_Fd;

	// TODO: I can check here if the residual is zero.
	// If it is than there is probably no force acting on the system
	// if (m_R0*m_R0 < eps) bconv = true;

	//	double r0 = m_R0*m_R0;

	return true;
}

//-----------------------------------------------------------------------------
double FENewtonSolver::QNSolve()
{
	{ // call the strategy to solve the linear equations
		TRACK_TIME("solve");
		m_strategy->SolveEquations(m_ui, m_R0);

		// check for nans
		double du = m_ui*m_ui;
		if (ISNAN(du)) throw NANDetected();
	}

	// perform a linesearch
	// the geometry is also updated in the line search
	m_ls = 1.0;
	if (m_lineSearch && (m_lineSearch->m_LStol > 0.0)) m_ls = m_lineSearch->DoLineSearch();
	else
	{
		// Update geometry
		Update(m_ui);

		// calculate residual at this point
		Residual(m_R1);
	}

	// return line search
	return m_ls;
}

//-----------------------------------------------------------------------------
void FENewtonSolver::QNForceReform(bool b)
{
	m_bforceReform = b;
}

//-----------------------------------------------------------------------------
//! Do a QN update
bool FENewtonSolver::QNUpdate()
{
	// see if the force reform flag was set
	bool breform = m_bforceReform; m_bforceReform = false;

	// for full-Newton, we skip QN update
	if (m_maxups == 0) breform = true;

	// if not, do a QN update
	if (breform == false)
	{
		TRACK_TIME("qn_update");

		// make sure we didn't reach max updates
		if (m_strategy->m_nups >= m_strategy->m_maxups - 1)
		{
			// print a warning only if the user did not intent full-Newton
			if (m_strategy->m_maxups > 0)
				felog.printbox("WARNING", "Max nr of iterations reached.\nStiffness matrix will now be reformed.");
			breform = true;
		}

		// try to do an update
		bool bret = m_strategy->Update(m_ls, m_ui, m_R0, m_R1);
		if (bret == false)
		{
			// Stiffness update has failed.
			// this might be due a too large condition number
			// or the update was no longer positive definite.
			felog.printbox("WARNING", "The QN update has failed.\nStiffness matrix will now be reformed.");
			breform = true;
		}
	}

	// zero displacement increments
	// we must set this to zero before the reformation
	// because we assume that the prescribed displacements are stored 
	// in the m_ui vector.
	zero(m_ui);

	// reform stiffness matrices if necessary
	if (breform && m_bdoreforms)
	{
		// reform the matrix
		if (m_strategy->ReformStiffness() == false) return false;
	}

	// copy last calculated residual
	m_R0 = m_R1;

	return true;
}

//-----------------------------------------------------------------------------
bool FENewtonSolver::DoAugmentations()
{
	FEAnalysis* pstep = m_fem.GetCurrentStep();

	// we have converged, so let's see if the augmentations have converged as well
	felog.printf("\n........................ augmentation # %d\n", m_naug + 1);

	// do callback
	pstep->GetFEModel().DoCallback(CB_AUGMENT);

	// do the augmentations
	bool bconv = Augment();

	// update counter
	++m_naug;

	// we reset the reformations counter
	m_nref = 0;

	// If we havn't converged we prepare for the next iteration
	if (!bconv)
	{
		// Since the Lagrange multipliers have changed, we can't just copy
		// the last residual but have to recalculate the residual
		// we also recalculate the stresses in case we are doing augmentations
		// for incompressible materials
		UpdateModel();
		Residual(m_R0);

		m_strategy->PreSolveUpdate();

		// reform the matrix if we are using full-Newton
		if (m_strategy->m_maxups == 0)
		{
			// TODO: Note sure how to handle a false return from ReformStiffness. 
			//       I think this is pretty rare so I'm ignoring it for now.
//			if (ReformStiffness() == false) break;
			m_strategy->ReformStiffness();
		}
	}

	return bconv;
}
