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
#include "FECore/FESolver.h"
#include "FECore/FEGlobalVector.h"
#include <FECore/FETimeInfo.h>

//-----------------------------------------------------------------------------
//! This class implements a nonlinear explicit solver for solid mechanics
//! problems.
class FEExplicitSolidSolver : public FESolver
{
public:
	//! constructor
	FEExplicitSolidSolver(FEModel* pfem);

	//! destructor
	virtual ~FEExplicitSolidSolver() {}

public:
	//! Data initialization
	bool Init() override;

	//! clean up
	void Clean() override;

	//! Solve an analysis step
	bool SolveStep() override;

	//! Update data
	void Update(vector<double>& ui) override;

	//! Serialize data
	void Serialize(DumpStream& ar) override;

public:
	//! assemble the element residual into the global residual
//	void AssembleResidual(vector<int>& en, vector<int>& elm, vector<double>& fe, vector<double>& R);

public:

	// initialize equations
	bool InitEquations() override;

	//! update kinematics
	void UpdateKinematics(vector<double>& ui);

	//! Update rigid bodies 
	void UpdateRigidBodies(vector<double>& ui);

	//! Update stresses
	void UpdateModel() override;

	//! solve the step
	bool DoSolve();

	void PrepStep();

	void NodalForces(vector<double>& F, const FETimeInfo& tp);

	bool Residual(vector<double>& R);

	void NonLinearConstraintForces(FEGlobalVector& R, const FETimeInfo& tp);

	void InertialForces(FEGlobalVector& R);
	
	void ContactForces(FEGlobalVector& R);

private:
	//! \todo I have to overload this but I need to remove this.
	virtual void AssembleStiffness(vector<int>& en, vector<int>& elm, matrix& ke) override { assert(false); }

public:
	double		m_dyn_damping;	//!< velocity damping for the explicit solver

public:
	// equation numbers
	int		m_neq;			//!< number of equations
	int		m_nreq;			//!< start of rigid body equations

	vector<double> m_inv_mass;	//!< inverse mass vector for explicit analysis
	vector<double> m_Fn;	//!< concentrated nodal force vector
	vector<double> m_Fr;	//!< nodal reaction forces
	vector<double> m_Ui;	//!< Total displacement vector for iteration
	vector<double> m_Ut;	//!< Total dispalcement vector at time t (incl all previous timesteps)
	vector<double> m_Fd;	//!< residual correction due to prescribed displacements

	vector<double> m_ui;	//!< displacement increment vector

	vector<double> m_R0;	//!< residual at iteration i-1
	vector<double> m_R1;	//!< residual at iteration i
	double *** domain_mass;	//! Pointer to data structure for nodal masses, dynamically allocated during initiation

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

	// declare the parameter list
	DECLARE_PARAMETER_LIST();
};
