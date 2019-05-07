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
#include <stdlib.h>
#include "PardisoSolver.h"

//! This implementation of the Pardiso solver is for the version
//! available in the Intel MKL.


#ifdef PARDISO
/* Pardiso prototypes for MKL version */
extern "C"
{
	int pardisoinit_(void *, int *, int *);

	int pardiso_(void *, int *, int *, int *, int *, int *,
		double *, int *, int *, int *, int *, int *,
		int *, double *, double *, int *);
}

#else
/* Pardiso prototypes for shared object library version */

#ifdef WIN32

#define pardisoinit_ PARDISOINIT
#define pardiso_ PARDISO

#endif

extern "C"
{
	int pardisoinit_(void *, int *, int *, int *, double*, int*);

	int pardiso_(void *, int *, int *, int *, int *, int *,
		double *, int *, int *, int *, int *, int *,
		int *, double *, double *, int *, double*);
}
#endif

#ifdef PARDISO

//-----------------------------------------------------------------------------
// print pardiso error message
void print_err(int nerror)
{
	switch (-nerror)
	{
	case 1: fprintf(stderr, "Inconsistent input\n"); break;
	case 2: fprintf(stderr, "Not enough memory\n"); break;
	case 3: fprintf(stderr, "Reordering problem\n"); break;
	case 4: fprintf(stderr, "Zero pivot, numerical fact. or iterative refinement problem\n"); break;
	case 5: fprintf(stderr, "Unclassified (internal) error\n"); break;
	case 6: fprintf(stderr, "Preordering failed\n"); break;
	case 7: fprintf(stderr, "Diagonal matrix problem\n"); break;
	case 8: fprintf(stderr, "32-bit integer overflow problem\n"); break;
	default:
		fprintf(stderr, " Unknown\n");
	}
}

//////////////////////////////////////////////////////////////
// PardisoSolver
//////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
PardisoSolver::PardisoSolver() : m_pA(0)
{
	/* If both PARDISO AND PARDISODL are defined, print a warning */
#ifdef PARDISODL
	fprintf(stderr, "WARNING: The MKL version of the Pardiso solver is being used\n\n");
	exit(1);
#endif
}

//-----------------------------------------------------------------------------
SparseMatrix* PardisoSolver::CreateSparseMatrix(Matrix_Type ntype)
{
	m_bsymm = (ntype == REAL_SYMMETRIC);
	if (m_bsymm) m_pA = new CompactSymmMatrix(1);
	else m_pA = new CRSSparseMatrix(1);

	return m_pA;
}

//-----------------------------------------------------------------------------
void PardisoSolver::SetSparseMatrix(CompactMatrix* pA)
{
	m_pA = pA;
}

//-----------------------------------------------------------------------------
bool PardisoSolver::PreProcess()
{
	m_mtype = (m_bsymm ? -2 : 11); /* Real symmetric matrix */
	m_iparm[0] = 0; /* Use default values for parameters */

	//fprintf(stderr, "In PreProcess\n");

	pardisoinit_(m_pt, &m_mtype, m_iparm);

	m_n = m_pA->Rows();
	m_nnz = m_pA->NonZeroes();
	m_nrhs = 1;

	// number of processors: This parameter is no longer used.
	// Use OMP_NUM_THREADS
	// m_iparm[2] = m_numthreads;

	m_maxfct = 1;	/* Maximum number of numerical factorizations */
	m_mnum = 1;	/* Which factorization to use */

	m_msglvl = 0;	/* 0 Suppress printing, 1 Print statistical information */

	return LinearSolver::PreProcess();
}

//-----------------------------------------------------------------------------
bool PardisoSolver::Factor()
{
	// make sure we have work to do
	if (m_pA->Rows() == 0) return true;

// ------------------------------------------------------------------------------
// Reordering and Symbolic Factorization.  This step also allocates all memory
// that is necessary for the factorization.
// ------------------------------------------------------------------------------

	int phase = 11;

	int error = 0;
	pardiso_(m_pt, &m_maxfct, &m_mnum, &m_mtype, &phase, &m_n, m_pA->Values(), m_pA->Pointers(), m_pA->Indices(),
		 NULL, &m_nrhs, m_iparm, &m_msglvl, NULL, NULL, &error);

	if (error)
	{
		fprintf(stderr, "\nERROR during symbolic factorization: ");
		print_err(error);
		exit(2);
	}

// ------------------------------------------------------------------------------
// This step does the factorization
// ------------------------------------------------------------------------------

	phase = 22;

#ifdef PRINTHB
	A->print_hb();
#endif

	error = 0;
	pardiso_(m_pt, &m_maxfct, &m_mnum, &m_mtype, &phase, &m_n, m_pA->Values(), m_pA->Pointers(), m_pA->Indices(),
		 NULL, &m_nrhs, m_iparm, &m_msglvl, NULL, NULL, &error);

	if (error)
	{
		fprintf(stderr, "\nERROR during factorization: ");
		print_err(error);
		exit(2);
	}

	return true;
}

//-----------------------------------------------------------------------------
bool PardisoSolver::BackSolve(vector<double>& x, vector<double>& b)
{
	// make sure we have work to do
	if (m_pA->Rows() == 0) return true;

	int phase = 33;

	m_iparm[7] = 1;	/* Maximum number of iterative refinement steps */

	int error = 0;
	pardiso_(m_pt, &m_maxfct, &m_mnum, &m_mtype, &phase, &m_n, m_pA->Values(), m_pA->Pointers(), m_pA->Indices(),
		 NULL, &m_nrhs, m_iparm, &m_msglvl, &b[0], &x[0], &error);

	if (error)
	{
		fprintf(stderr, "\nERROR during solution: ");
		print_err(error);
		exit(3);
	}

	return true;
}

//-----------------------------------------------------------------------------
void PardisoSolver::Destroy()
{
	int phase = -1;

	int error = 0;

	if (m_pA->Pointers())
	{
		pardiso_(m_pt, &m_maxfct, &m_mnum, &m_mtype, &phase, &m_n, NULL, m_pA->Pointers(), m_pA->Indices(),
			NULL, &m_nrhs, m_iparm, &m_msglvl, NULL, NULL, &error);
	}

	LinearSolver::Destroy();

}

#endif
