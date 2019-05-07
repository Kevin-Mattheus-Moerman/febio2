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


#pragma once
#include "FESolver.h"
#include "FENewtonStrategy.h"
#include "FETimeInfo.h"
#include "FELineSearch.h"

//-----------------------------------------------------------------------------
// forward declarations
class FEModel;
class FEGlobalMatrix;

//-----------------------------------------------------------------------------
// Scheme for assigning equation numbers
// STAGGERED: | a0, b0, a1, b1, ..., an, bn |
// BLOCK    : | a0, a1, ..., an, b0, b1, ..., bn |
enum EQUATION_SCHEME
{
	STAGGERED,
	BLOCK
};

//-----------------------------------------------------------------------------
// NOTE: Currently, the value 2 is used as an alternative for Broyden
enum QN_STRATEGY
{
	QN_BFGS,
	QN_BROYDEN,
	QN_JFNK = 3
};

//-----------------------------------------------------------------------------
//! This class defines the base class for Newton-type solvers. 
//! The class implements the basic logic behind a newton-solver but defers some
//! of the logic, especially relating to the update of the stiffness matrix to
//! the FENewtonStrategy class. 
//! \todo there is some common functionality with the FELinearSolver. Perhaps
//! I can make the FELinearSolver a base class?
//! \todo Perhaps I can introduce a base class for linear search algorithms
//! so that the line search strategy can be customized as well.
class FECORE_API FENewtonSolver : public FESolver
{
public:
	//! constructor
	FENewtonSolver(FEModel* pfem);

	//! destrcutor
	~FENewtonSolver();

	//! Set the default solution strategy
	void SetDefaultStrategy(QN_STRATEGY qn);

	//! Check the zero diagonal
	void CheckZeroDiagonal(bool bcheck, double ztol = 0.0);

public: // overloaded from FESolver

	//! Initialization
	bool Init() override;

	//! Initialize linear equation system
	bool InitEquations() override;

	//! return number of equations
	int NumberOfEquations() const { return m_neq; }

	//! Clean up
	void Clean() override;

	//! serialization
	void Serialize(DumpStream& ar) override;

	//! Solve an analysis step
	bool SolveStep() override;

	//! rewind solver
	void Rewind() override;

	//! prep the solver for the QN updates
	virtual void PrepStep() {}

	//! adjust the residual matrix for prescribed displacements
	void AssembleStiffness(vector<int>& en, vector<int>& elm, matrix& ke) override;

public:	// Quasi-Newton methods

	//! call this at the start of the quasi-newton loop
	bool QNInit();

	//! Do a qn update
	bool QNUpdate();

	//! solve the equations using QN method (returns line search size)
	double QNSolve();

	//! Force a stiffness reformation during next update
	void QNForceReform(bool b);

	//! TODO: This is a helper function to get around an issue with the current implementation
	//        regarding prescribed displacements. The purpose of this Update2 function is to update
	//        all degrees of freedom, including prescribed ones. This is currently only used by the JFNKMatrix class.
	//        and overridden in FESolidSolver2. 
	virtual void Update2(const vector<double>& ui) {}

public:
	//! return the stiffness matrix
	FEGlobalMatrix& GetStiffnessMatrix();

	//! reform the stiffness matrix
    bool ReformStiffness();

    //! recalculates the shape of the stiffness matrix
    bool CreateStiffness(bool breset);

public:
	//! do augmentations
	bool DoAugmentations();

public:
	//! Set the solution strategy
	void SetSolutionStrategy(FENewtonStrategy* pstrategy);

	//! solve the linear system of equations
	void SolveLinearSystem(vector<double>& x, vector<double>& R);

	//! Do a Quasi-Newton step
	//! This is called from SolveStep.
	virtual bool Quasin();

    //! calculates the global stiffness matrix (needs to be overwritten by derived classes)
    virtual bool StiffnessMatrix() = 0;

	//! calculates the global residual vector (needs to be overwritten by derived classes)
	virtual bool Residual(vector<double>& R) = 0;

	//! Check convergence. Derived classes that don't override Quasin, should implement this
	//! niter = iteration number
	//! ui    = search direction
	//! ls    = line search factor
	virtual bool CheckConvergence(int niter, const vector<double>& ui, double ls) { return true; };

	//! Set the partitions of the global stiffness matrix
	void SetPartitions(vector<int>& part);

	//! initialize linear system
	bool InitLinearSystem();

public:
	// line search options
	FELineSearch*	m_lineSearch;

	// solver parameters
	int					m_nqnmethod;	//!< quasi-Newton strategy that will be selected
	int					m_maxups;		//!< max number of quasi-newton updates
	int					m_max_buf_size;	//!< max buffer size for update vector storage
	bool				m_cycle_buffer;	//!< cycle the qn buffer when updates larger than buffer size
	double				m_cmax;			//!< max condition numbers
	int					m_maxref;		//!< max nr of reformations per time step
	int					m_eq_scheme;	//!< equation number scheme (used in InitEquations)
	int					m_force_partition;	//!< Force a partition of the global matrix (e.g. for testing with BIPN solver)

	// solution strategy
	FENewtonStrategy*	m_strategy;			//!< class handling the specific stiffness update logic
	bool				m_breformtimestep;	//!< reform at start of time step
	bool				m_bforceReform;		//!< forces a reform in QNInit
	bool				m_bdivreform;		//!< reform when diverging
	bool				m_bdoreforms;		//!< do reformations

	// counters
	int		m_nref;			//!< nr of stiffness retormations

	// Error handling
	bool	m_bzero_diagonal;	//!< check for zero diagonals
	double	m_zero_tol;			//!< tolerance for zero diagonal

	// linear solver data
	LinearSolver*		m_plinsolve;	//!< the linear solver
	FEGlobalMatrix*		m_pK;			//!< global stiffness matrix
	int					m_neq;			//!< number of equations
    bool				m_breshape;		//!< Matrix reshape flag
	vector<int>			m_part;

	// data used by Quasin
	vector<double> m_R0;	//!< residual at iteration i-1
	vector<double> m_R1;	//!< residual at iteration i
	vector<double> m_ui;	//!< displacement increment vector
	vector<double> m_Fd;	//!< residual correction due to prescribed degrees of freedom

private:
	double	m_ls;	//!< line search factor calculated in last call to QNSolve

	DECLARE_PARAMETER_LIST();
};
