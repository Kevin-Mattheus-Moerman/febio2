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
#include "FECore/FENewtonSolver.h"
#include <FECore/FETimeInfo.h>
#include "FECore/FEGlobalVector.h"
#include "FEBioMech/FERigidSolver.h"

//-----------------------------------------------------------------------------
//! The FEFluidFSISolver class solves fluid-FSI problems
//! It can deal with quasi-static and dynamic problems
//!
class FEFluidFSISolver : public FENewtonSolver
{
public:
    //! constructor
    FEFluidFSISolver(FEModel* pfem);
    
    //! destructor
    ~FEFluidFSISolver();
    
    //! serialize data to/from dump file
    void Serialize(DumpStream& ar) override;
    
    //! Initializes data structures
    bool Init() override;
    
    //! initialize the step
    bool InitStep(double time) override;
    
    //! Initialize linear equation system
    bool InitEquations() override;
    
    //! Generate warnings if needed
    void SolverWarnings() override;
    
public:
    //! assemble the element residual into the global residual
    //! \todo This was implemented for nodal forces
    void AssembleResidual(int node, int dof, double f, vector<double>& R);
    
    //! adjust the residual matrix for prescribed velocities
    void AssembleStiffness(vector<int>& en, vector<int>& elm, matrix& ke) override;
    
    //! assemble global stiffness matrix \todo this is only used by rigid joints
    void AssembleStiffness(vector<int>& elm, matrix& ke) override;
    
    //! adjust the residual matrix for prescribed displacements
    void AssembleStiffness2(vector<int>& lmi, vector<int>& lmj, matrix& ke) override;
    
public:
    //{ --- evaluation and update ---
    //! Perform an update
    void Update(vector<double>& ui) override;

	//! update nodal positions, velocities, accelerations, etc.
	void UpdateKinematics(vector<double>& ui);

	//! Update EAS
	void UpdateEAS(vector<double>& ui);
	void UpdateIncrementsEAS(vector<double>& ui, const bool binc);

	//! update DOF increments
	virtual void UpdateIncrements(vector<double>& Ui, vector<double>& ui, bool emap);

	//! Update Stresses
	void UpdateModel() override;

	//! update contact data
	void UpdateContact();

	//! update constraint data
	void UpdateConstraints();
    //}
    
    //{ --- Solution functions ---
    
    //! prepares the data for the first QN iteration
    void PrepStep() override;
    
    //! Performs a Newton-Raphson iteration
    bool Quasin() override;
    
    //! Lagrangian augmentation
    bool Augment() override;
    
    //{ --- Stiffness matrix routines ---
    
    //! calculates the global stiffness matrix
    bool StiffnessMatrix() override;
    
    //! contact stiffness
    void ContactStiffness();
    
    //! calculates stiffness contributon of nonlinear constraints
    void NonLinearConstraintStiffness(const FETimeInfo& tp);
    
    //{ --- Residual routines ---
    
    //! Calculates concentrated nodal forces
    void NodalForces(vector<double>& F, const FETimeInfo& tp);
    
    //! Calculate the contact forces
    void ContactForces(FEGlobalVector& R);
    
    //! Calculates residual
    bool Residual(vector<double>& R) override;
    
    //! Calculate nonlinear constraint forces
    void NonLinearConstraintForces(FEGlobalVector& R, const FETimeInfo& tp);
    
protected:
    void GetDisplacementData(vector<double>& xi, vector<double>& ui);
    void GetVelocityData(vector<double>& vi, vector<double>& ui);
    void GetDilatationData(vector<double>& ei, vector<double>& ui);
    
public:
    // convergence tolerances
    double	m_Rtol;			//!< residual tolerance
    double	m_Dtol;			//!< displacement tolerance
    double	m_Vtol;			//!< velocity tolerance
    double	m_Ftol;			//!< dilatation tolerance
    double	m_Etol;			//!< energy tolerance
    double	m_Rmin;			//!< min residual value
	double	m_Rmax;			//!< max residual value

public:
    // equation numbers
    int     m_nreq;         //!< start of rigid body equations
    int		m_ndeq;			//!< number of equations related to displacement dofs
    int		m_nveq;			//!< number of equations related to velocity dofs
    int		m_nfeq;			//!< number of equations related to dilatation dofs
    
public:
    vector<double> m_Fn;	//!< concentrated nodal force vector
    vector<double> m_Ui;	//!< Total DOF vector for iteration
    vector<double> m_Ut;	//!< Total DOF vector at time t (incl all previous timesteps)
    vector<double> m_Fr;	//!< nodal reaction forces
    vector<double> m_di;	//!< displacement increment vector
    vector<double> m_Di;	//!< Total displacement vector for iteration
    vector<double> m_vi;	//!< velocity increment vector
    vector<double> m_Vi;	//!< Total velocity vector for iteration
    vector<double> m_fi;	//!< dilatation increment vector
    vector<double> m_Fi;	//!< Total dilatation vector for iteration
    
    // generalized alpha method
    double  m_rhoi;         //!< spectral radius (rho infinity)
    double  m_alphaf;       //!< alpha step for Y={v,e}
    double  m_alpham;       //!< alpha step for Ydot={∂v/∂t,∂e/∂t}
    double  m_alpha;        //!< alpha
    double  m_beta;         //!< beta
    double  m_gamma;        //!< gamma
    int     m_pred;         //!< predictor method
    
protected:
    // solid displacement
    int		m_dofX;
    int		m_dofY;
    int		m_dofZ;
    // solid velocity
    int		m_dofVX;
    int		m_dofVY;
    int		m_dofVZ;

    // shell displacement
    int        m_dofSX;
    int        m_dofSY;
    int        m_dofSZ;
    // shell velocity
    int        m_dofSVX;
    int        m_dofSVY;
    int        m_dofSVZ;
    // shell acceleration
    int        m_dofSAX;
    int        m_dofSAY;
    int        m_dofSAZ;
    // shell displacement at previous time
    int        m_dofSXP;
    int        m_dofSYP;
    int        m_dofSZP;
    // shell velocity at previous time
    int        m_dofSVXP;
    int        m_dofSVYP;
    int        m_dofSVZP;
    // shell acceleration at previous time
    int        m_dofSAXP;
    int        m_dofSAYP;
    int        m_dofSAZP;
    
    // rigid body rotations
    int        m_dofRU;
    int        m_dofRV;
    int        m_dofRW;
    
    // fluid velocity
    int		m_dofVFX;
    int		m_dofVFY;
    int		m_dofVFZ;
    // material time derivative of fluid velocity
    int		m_dofAFX;
    int		m_dofAFY;
    int		m_dofAFZ;

    // fluid velocity relative to solid
    int		m_dofWX;
    int		m_dofWY;
    int		m_dofWZ;
    // material time derivative of fluid velocity relative to solid
    int		m_dofAWX;
    int		m_dofAWY;
    int		m_dofAWZ;
    // fluid dilatation
    int		m_dofEF;
    // material time derivative of fluid dilatation
    int     m_dofAEF;

    // fluid velocity relative to solid at previous time
    int		m_dofWXP;
    int		m_dofWYP;
    int		m_dofWZP;
    // material time derivative of fluid velocity relative to solid at previous time
    int		m_dofAWXP;
    int		m_dofAWYP;
    int		m_dofAWZP;
    // fluid dilatation at previous time
    int		m_dofEFP;
    // material time derivative of fluid dilatation at previous time
    int     m_dofAEFP;
    
protected:
    FERigidSolverNew    m_rigidSolver;

    // declare the parameter list
    DECLARE_PARAMETER_LIST();
};
