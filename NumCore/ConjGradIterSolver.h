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
#include "FECore/LinearSolver.h"
#include "CompactSymmMatrix.h"
#include <vector>

//-----------------------------------------------------------------------------
//! this class implements an iterative conjugate gradient solver 
class ConjGradIterSolver : public LinearSolver
{
public:
	//! constructor
	ConjGradIterSolver();

	//! Pre-process data
	bool PreProcess();

	//! Factor matrix
	bool Factor();

	//! solve system given a rhs vector
	bool BackSolve(std::vector<double>& x, std::vector<double>& b);

	//! Clean up
	void Destroy();

	//! Create a sparse matrix for this linear solver
	SparseMatrix* CreateSparseMatrix(Matrix_Type ntype);

public:
	CompactSymmMatrix*	m_pA;

	double	m_tol;		//!< convergence tolerance
	int		m_kmax;		//!< max iterations
	int		m_nprint;	//!< printing level

	std::vector<double>	m_P;	//!< preconditioning vector
};
