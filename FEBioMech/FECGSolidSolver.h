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
#include <FECore/FESolver.h>
#include <FECore/FEGlobalVector.h>
#include <FECore/FETimeInfo.h>

//-----------------------------------------------------------------------------
//! This class implements a solver for solid mechanics problems that uses
//! the conjugate gradient method to solve the nonlinear finite element equations
class FECGSolidSolver : public FESolver
{
public:
	//! constructor
	FECGSolidSolver(FEModel* pfem);

	//! initialization
	bool Init() override;

	//! clean up
	void Clean() override;

	//! Performs a CG step
	bool SolveStep() override;

	//! update nodal positions, velocities, accelerations, etc.
	void UpdateKinematics(vector<double>& ui);

	//! assemble global stiffness matrix
	//! \todo Get rid of this function
	virtual void AssembleStiffness(vector<int>& en, vector<int>& elm, matrix& ke) override {}

	// Initialize linear equation system (TODO: Is this the right place to do this?)
	// \todo Can I make this part of the Init function?
	virtual bool InitEquations() override;

protected:
	//! Update the stresses
	void UpdateModel() override;

	//! Update contact
	void UpdateContact();

	//! update constraints
	void UpdateConstraints();

	//! update rigid bodies
	void UpdateRigidBodies(vector<double>& ui);

	//! Evaluate the residual
	bool Residual(vector<double>& R);

	//! assemble into the residual
	void AssembleResidual(int node_id, int dof, double f, vector<double>& R);

	//! contact forces
	void ContactForces(FEGlobalVector& R);

	//! the non-linear constraint forces
	void NonLinearConstraintForces(FEGlobalVector& R, const FETimeInfo& tp);

	//! nodal forces
	void NodalForces(vector<double>& F, const FETimeInfo& tp);

	//! Inertial forces
	void InertialForces(FEGlobalVector& R);

	//! helper function for setting up the solution phase
	void PrepStep();

	//! modified linesearch for Hager-Zhang solver
	double LineSearchCG(double s);

	//! do an augmentation
	bool Augment() override;

public:
	double	m_Dtol;
	double	m_Etol;
	double	m_Rtol;
	double	m_Rmin;
	double	m_LStol;
	double	m_LSmin;
	int		m_LSiter;

	// Newmark parameters (for dynamic analyses)
	double	m_beta;			//!< Newmark parameter beta (displacement integration)
	double	m_gamma;		//!< Newmark parameter gamme (velocity integration)

private:
	vector<double>	m_R0;
	vector<double>	m_R1;
	vector<double>	m_Ui;
	vector<double>	m_ui;
	vector<double>	m_Ut;
	vector<double>	m_Fn;
	vector<double>	m_Fd;
	vector<double>	m_Fr;

	int				m_neq;
	int				m_nreq;

protected:
	int		m_dofX;
	int		m_dofY;
	int		m_dofZ;
	int		m_dofVX;
	int		m_dofVY;
	int		m_dofVZ;
	int		m_dofU;
	int		m_dofV;
	int		m_dofW;
	int		m_dofRU;
	int		m_dofRV;
	int		m_dofRW;

	DECLARE_PARAMETER_LIST();
};
