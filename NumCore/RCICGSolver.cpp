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
#include "RCICGSolver.h"

//-----------------------------------------------------------------------------
// We must undef PARDISO since it is defined as a function in mkl_solver.h
#ifdef MKL_ISS
#ifdef PARDISO
#undef PARDISO
#endif
#include "mkl_rci.h"
#include "mkl_blas.h"
#include "mkl_spblas.h"
#endif // MKL_ISS

//-----------------------------------------------------------------------------
RCICGSolver::RCICGSolver() : m_pA(0), m_P(0)
{
	m_maxiter = 0;
	m_tol = 1e-5;
	m_print_level = 0;
}

//-----------------------------------------------------------------------------
SparseMatrix* RCICGSolver::CreateSparseMatrix(Matrix_Type ntype)
{
#ifdef MKL_ISS
	if (ntype != REAL_SYMMETRIC) return 0;
	m_pA = new CompactSymmMatrix(1);
	return m_pA;
#else
	return 0;
#endif
}

//-----------------------------------------------------------------------------
bool RCICGSolver::SetSparseMatrix(SparseMatrix* A)
{
	m_pA = A;
	return (m_pA != 0);
}

//-----------------------------------------------------------------------------
void RCICGSolver::SetPreconditioner(Preconditioner* P)
{
	m_P = P;
}

//-----------------------------------------------------------------------------
bool RCICGSolver::PreProcess()
{
	return true;
}

//-----------------------------------------------------------------------------
bool RCICGSolver::Factor()
{
	if (m_pA == 0) return false;
	if (m_P) return m_P->Create(m_pA);
	return true;
}

//-----------------------------------------------------------------------------
bool RCICGSolver::BackSolve(vector<double>& x, vector<double>& b)
{
#ifdef MKL_ISS
	// make sure we have a matrix
	if (m_pA == 0) return false;

	// get number of equations
	MKL_INT n = m_pA->Rows();

	// zero solution vector
	zero(x);

	// get pointers to solution and RHS vector
	double* px = &x[0];
	double* pb = &b[0];

	// output parameters
	MKL_INT rci_request;
	MKL_INT ipar[128];
	double dpar[128];
	vector<double> tmp(n*4);
	double* ptmp = &tmp[0];

	// initialize parameters
	dcg_init(&n, px, pb, &rci_request, ipar, dpar, ptmp);
	if (rci_request != 0) return false;

	// set the desired parameters:
	if (m_maxiter > 0) ipar[4] = m_maxiter;	// max nr of iterations
	ipar[8] = 1;			// do residual stopping test
	ipar[9] = 0;			// do not request for the user defined stopping test
	ipar[10] = (m_P ? 1 : 0);		// preconditioning
	dpar[0] = m_tol;		// set the relative tolerance

	// check the consistency of the newly set parameters
	dcg_check(&n, px, pb, &rci_request, ipar, dpar, ptmp);
	if (rci_request != 0) return false;

	// loop until converged
	bool bsuccess = false;
	bool bdone = false;
	do
	{
		// compute the solution by RCI
		dcg(&n, px, pb, &rci_request, ipar, dpar, ptmp);

		switch (rci_request)
		{
		case 0: // solution converged! 
			bsuccess = true;
			bdone = true;
			break;
		case 1: // compute vector A*tmp[0] and store in tmp[n]
			{
				bool b = m_pA->mult_vector(ptmp, ptmp+n);
				if (b == false)
				{
					bsuccess = false;
					bdone = true;
					break;
				}

				if (m_print_level == 1)
				{
					fprintf(stderr, "%3d = %lg (%lg), %lg (%lg)\n", ipar[3], dpar[4], dpar[3], dpar[6], dpar[7]);
				}
			}
			break;
		case 3:
			{
				assert(m_P);
				m_P->mult_vector(ptmp + n*2, ptmp + n*3);
			}
			break;
		default:
			bsuccess = false;
			bdone = true;
			break;
		}
	}
	while (!bdone);

	// get convergence information
	int niter;
	dcg_get(&n, px, pb, &rci_request, ipar, dpar, ptmp, &niter);

	if (m_print_level > 0)
	{
		fprintf(stderr, "%3d = %lg (%lg), %lg (%lg)\n", ipar[3], dpar[4], dpar[3], dpar[6], dpar[7]);
	}

	// release internal MKL buffers
	MKL_Free_Buffers();

	return bsuccess;
#else
	return false;
#endif // MKL_ISS
}

//-----------------------------------------------------------------------------
void RCICGSolver::Destroy()
{
}

//! convenience function for solving linear system Ax = b
bool RCICGSolver::Solve(SparseMatrix* A, vector<double>& x, vector<double>& b, Preconditioner* P)
{
	SetSparseMatrix(A);
	SetPreconditioner(P);
	if (PreProcess() == false) return false;
	if (Factor() == false) return false;
	return BackSolve(x, b);
}
