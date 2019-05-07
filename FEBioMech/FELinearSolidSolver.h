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
#include "FECore/LinearSolver.h"

class FEGlobalMatrix;

//-----------------------------------------------------------------------------
//! The FELinearSolidSolver class solves linear (quasi-static) elasticity 
//! problems.
//!
class FELinearSolidSolver : public FENewtonSolver
{
public:
	//! constructor
	FELinearSolidSolver(FEModel* pfem);

	//! destructor
	~FELinearSolidSolver();

	//! Solve the analysis step
	bool Quasin() override;

	//! Data serialization
	void Serialize(DumpStream& ar) override;

protected:
	//! calculate the residual
	bool Residual(vector<double>& R) override;

	//! calculate the stiffness matrix
	bool StiffnessMatrix() override;

	//! update solution
	void Update(vector<double>& u) override;

public:
	//! assemble element stiffness matrix
	void AssembleStiffness(vector<int>& en, vector<int>& elm, matrix& ke) override;

public:
	double	m_Dtol;			//!< displacement tolerance

protected:
	vector<double>	m_u;	//!< nodal displacements
	vector<double>	m_R;	//!< right hand side
	vector<double>	m_d;	//!< prescribed displacements

	int	m_dofX;
	int	m_dofY;
	int	m_dofZ;

public:
	// declare the parameter list
	DECLARE_PARAMETER_LIST();
};
