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
#include <stdio.h>
#include <FECore/SparseMatrix.h>
#include <FECore/LinearSolver.h>

// This class implements a sparse matrix operator that represents the Schur complement of a matrix M
//
//       | A | B |
//  M =  | --+-- |
//       | C | D |
//
// The Schur complement of A, is given by 
//       
//  S\A = C*A^-1*B - D
//
// The only operator this class implements is the matrix-vector multiplication operator (in mult_vector function). 
// This function evaluates a product of the form S*x, where v is a vector, as follows:
// 1. evaluate u = B*x
// 2. evaluate v = A^-1*u, by solving A*v = u
// 3. evaluate r = C*v
// 4. if D is given, then subtract r <-- r - D*x
//
// If D = 0, it does not need to be specified, in which case step 4 is not done.

class SchurComplement : public SparseMatrix
{
public:
	SchurComplement(LinearSolver* A, SparseMatrix* B, SparseMatrix* C, SparseMatrix* D = 0);

	// set the print level
	void SetPrintLevel(int printLevel);

	//! multiply with vector
	bool mult_vector(double* x, double* r) override;

private: // we need to override these functions although we don't want to use them
	void Zero() { assert(false); }
	void Create(SparseMatrixProfile& MP) { assert(false); }
	void Assemble(matrix& ke, std::vector<int>& lm) { assert(false); }
	void Assemble(matrix& ke, std::vector<int>& lmi, std::vector<int>& lmj) { assert(false); }
	bool check(int i, int j) { assert(false); return false; }
	void set(int i, int j, double v)  { assert(false); }
	void add(int i, int j, double v)  { assert(false); }
	double diag(int i)  { assert(false); return 0.0; }

private:
	int	m_print_level;
	
	LinearSolver*	m_A;
	SparseMatrix*	m_B;
	SparseMatrix*	m_C;
	SparseMatrix*	m_D;

	vector<double>	m_tmp1, m_tmp2;
};
