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

//-----------------------------------------------------------------------------
// forward declarations
class FEGlobalVector;
class FEGlobalMatrix;
class FELinearSystem;
class LinearSolver;

//-----------------------------------------------------------------------------
//! Abstract Base class for finite element solution algorithms (i.e. "FE solvers") that require the solution
//! of a linear system of equations.
class FECORE_API FELinearSolver : public FESolver
{
public:
	//! constructor
	FELinearSolver(FEModel* pfem);

	//! Set the degrees of freedom
	void SetDOF(vector<int>& dof);

	//! Get the number of equations
	int NumberOfEquations() const;

	//! add equations
	void AddEquations(int neq);

	//! Get the linear solver
	LinearSolver* GetLinearSolver();

	//! set the linear system partitions
	void SetPartitions(const vector<int>& part);

public: // from FESolver

	//! solve the step
	bool SolveStep() override;

	//! Initialize and allocate data
	bool Init() override;

	//! Initialize equation numbers
	bool InitEquations() override;

	//! Clean up data
	void Clean() override;

	//! Serialization
	void Serialize(DumpStream& ar) override;

public: // these functions need to be implemented by the derived class

	//! Evaluate the right-hand side "force" vector
	virtual void ForceVector(FEGlobalVector& R) = 0;

	//! Evaluate the stiffness matrix
	virtual bool StiffnessMatrix(FELinearSystem& K) { return false; }

	//! Update the model state
	virtual void Update(vector<double>& u) override;

protected: // some helper functions

	//! Reform the stiffness matrix
	bool ReformStiffness();

	//! Create and evaluate the stiffness matrix
	bool CreateStiffness();

private:
	//! assemble global stiffness matrix (TODO: remove this)
	void AssembleStiffness(vector<int>& en, vector<int>& elm, matrix& ke) override { assert(false); }
	
protected:
	vector<double>		m_R;	//!< RHS vector
	vector<double>		m_u;	//!< vector containing prescribed values

private:
	LinearSolver*		m_pls;		//!< The linear equation solver
	FEGlobalMatrix*		m_pK;		//!< The global stiffness matrix
	int					m_neq;		//!< The number of equations (TODO: Get this from linear solver)
	vector<int>			m_part;		//!< set partitions of linear system

	vector<int>		m_dof;	//!< list of active degrees of freedom
	bool			m_breform;	//!< matrix reformation flag
};
