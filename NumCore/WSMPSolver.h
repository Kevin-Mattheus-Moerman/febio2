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

#include "FECore/SparseMatrix.h"
#include "FECore/LinearSolver.h"
#include "FECore/vector.h"
#include "FECore/matrix.h"
#include "CompactSymmMatrix.h"

//! This class implements the Watson Sparse Matrix Package.

//! The WSMP solver requires a license file.
//! Documentation can be found at:
//!	http://www-users.cs.umn.edu/~agupta/wsmp

	/* WSMP Fortran prototypes */
#ifdef WSMP
extern "C"
{
	void wsetmaxthrds_(int *);

	void wsmp_initialize_();

	void wssmp_(int *, int *, int *, double *, double *, int *, int *, double *,
		int *, int *, double *, int *, int *, int *, double *);

	void wsmp_clear_();
}
#endif //WSMP

class WSMPSolver : public LinearSolver
{
public:
	WSMPSolver();
	bool PreProcess();
	bool Factor();
	bool BackSolve(vector<double>& x, vector<double>& b);
	void Destroy();

	SparseMatrix* CreateSparseMatrix(Matrix_Type ntype);

private:
	// WSMP control parameters
	int m_iparm[64];
	double m_dparm[64];

	// Matrix data
	int m_n, m_nnz;
	vector<int> m_perm, m_invp;
	vector<double> m_b;

	CompactSymmMatrix*	m_pA;
};
