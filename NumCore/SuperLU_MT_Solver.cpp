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
#include <stdio.h>
#include "SuperLU_MT_Solver.h"

//-----------------------------------------------------------------------------
//! constructor

SuperLU_MT_Solver::SuperLU_MT_Solver() : m_pA(0)
{
#ifdef SUPERLU_MT
	m_bfact = false;
	m_balloc = false;
#endif
}

//-----------------------------------------------------------------------------
SparseMatrix* SuperLU_MT_Solver::CreateSparseMatrix(Matrix_Type ntype)
{ 
	return (m_pA = new CCSSparseMatrix());
}

//-----------------------------------------------------------------------------
//! Solver preprocessing

bool SuperLU_MT_Solver::PreProcess()
{
#ifndef SUPERLU_MT
	fprintf(stderr, "FATAL ERROR: The SuperLU_MT solver is not supported on this platform.\n\n");
	return false;
#else

	// get the number of columns/rows
	int N = m_pA->Size();

	// get the number of non-zero entries
	int nnz = m_pA->NonZeroes();

	// allocate storage for the permutation matrices
	m_perm_c.resize(N);
	m_perm_r.resize(N);

	// set solver options
    m_ops.nprocs			= 1;			// nr. of threads that will be created
    m_ops.fact				= DOFACT;		// factorization flag
    m_ops.trans				= NOTRANS;		// type of system to solve: A*x = B
    m_ops.refact			= NO;			// refactorization flag
    m_ops.panel_size		= sp_ienv(1);	// a panel consist of at most panel_size columns
    m_ops.relax				= sp_ienv(2);	// relaxation parameter
    m_ops.diag_pivot_thresh = 1.0;			// diagonal pivoting threshold (0 <= d <= 1)
    m_ops.usepr				= NO;			// user-specified perm_r
	m_ops.SymmetricMode		= YES;			// is the matrix symmetric or not
    m_ops.drop_tol			= 0;			// drop tolerance (apparently not yet implemented)
	m_ops.PrintStat			= NO;			// print solver statistics or not
    m_ops.perm_c			= &m_perm_c[0];
    m_ops.perm_r			= &m_perm_r[0];
    m_ops.work				= 0;
    m_ops.lwork				= 0;

	// create the SuperMatrix m_A
    dCreate_CompCol_Matrix(&m_A, N, N, nnz, m_pA->Values(), m_pA->Indices(), m_pA->Pointers(), SLU_NC, SLU_D, SLU_GE);

	// create the dense matrices B and X
	// note that we don't provide any data yet
    dCreate_Dense_Matrix(&m_B, N, 0, NULL, N, SLU_DN, SLU_D, SLU_GE);
    dCreate_Dense_Matrix(&m_X, N, 0, NULL, N, SLU_DN, SLU_D, SLU_GE);

	// set the allocation flag
	m_balloc = true;


	return true;
#endif
}

//-----------------------------------------------------------------------------
//! Factor the sparse matrix

bool SuperLU_MT_Solver::Factor()
{
#ifndef SUPERLU_MT
	fprintf(stderr, "FATAL ERROR: The SuperLU_MT solver is not supported on this platform.\n\n");
	return false;
#else
	if (m_bfact)
	{
		Destroy_SuperNode_SCP(&m_L);
		Destroy_CompCol_NCP(&m_U);
	}

	// set nr of columns of B to zero to make sure we don't solve anything here
	m_B.ncol = 0;
	m_X.ncol = 0;

	// set the options to do a factorization
	m_ops.fact = DOFACT;

	// perform factorization
    pdgssvx(
		m_ops.nprocs,			// (in) nr of threads
		&m_ops,					// (in) solver options
		&m_A,					// (in/out) sparse matrix to factor
		&m_perm_c[0],				// (in/out) column permutation vector
		&m_perm_r[0],				// (in/out) row permutation vector
		&equed,					// (in/out) speicify the form of equilibrium that was done
		NULL,					// (in/out) row scale factors
		NULL,					// (in/out) column scale factors
		&m_L,					// (out) the factor L from the factorization
		&m_U,					// (out) the factor U from the factorization
		&m_B,					// (in/out) right hand side matrix
		&m_X,					// (in/out) contains solution matrix
		&rpg,					// (out) reciprocal growth factor
		&rcond,					// (out) estimate of the reciprocal condition number
	    &ferr,					// (out) estimated forward error bound
		&berr,					// (out) relative backward error
		&m_mem,					// (out) memory usage statistics
		&info					// (out) exit number
		);

	m_bfact = true;

	return true;
#endif
}

//-----------------------------------------------------------------------------
//! Solve the linear system

bool SuperLU_MT_Solver::BackSolve(vector<double> &x, vector<double> &b)
{
#ifndef SUPERLU_MT
	fprintf(stderr, "FATAL ERROR: The SuperLU_MT solver is not supported on this platform.\n\n");
	return false;
#else

	// set the data in the B matrix
	DNformat *Bstore = (DNformat*) m_B.Store;
	Bstore->nzval = &b[0];
	m_B.ncol = 1;

	// set the data in the X matrix
	DNformat *Xstore = (DNformat*) m_X.Store;
	Xstore->nzval = &x[0];
	m_X.ncol = 1;

	// solve the system
	m_ops.fact = FACTORED;
    pdgssvx(
		m_ops.nprocs,			// (in) nr of threads
		&m_ops,					// (in) solver options
		&m_A,					// (in/out) sparse matrix to factor
		&m_perm_c[0],				// (in/out) column permutation vector
		&m_perm_r[0],				// (in/out) row permutation vector
		&equed,					// (in/out) speicify the form of equilibrium that was done
		NULL,					// (in/out) row scale factors
		NULL,					// (in/out) column scale factors
		&m_L,					// (out) the factor L from the factorization
		&m_U,					// (out) the factor U from the factorization
		&m_B,					// (in/out) right hand side matrix
		&m_X,					// (in/out) contains solution matrix
		&rpg,					// (out) reciprocal growth factor
		&rcond,					// (out) estimate of the reciprocal condition number
	    &ferr,					// (out) estimated forward error bound
		&berr,					// (out) relative backward error
		&m_mem,					// (out) memory usage statistics
		&info					// (out) exit number
		);

	return true;
#endif
}

//-----------------------------------------------------------------------------
//! Clean up

void SuperLU_MT_Solver::Destroy()
{
#ifndef SUPERLU_MT
	fprintf(stderr, "FATAL ERROR: The SuperLU_MT solver is not supported on this platform.\n\n");
#else
	// since superlu by default deallocates the memory for the matrix data
	// we can't use the destroy routines for A and B. In stead we deallocate the memory for A
	// ourselve
	if (m_balloc)
	{
		Destroy_SuperMatrix_Store(&m_A);
		Destroy_SuperMatrix_Store(&m_B);
		Destroy_SuperMatrix_Store(&m_X);

		m_balloc = false;
	}

	if (m_bfact)
	{
		// we can however for L and U
		Destroy_SuperNode_SCP(&m_L);
		Destroy_CompCol_NCP(&m_U);

		m_bfact = false;
	}

	LinearSolver::Destroy();

#endif
}
