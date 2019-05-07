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
#include "WSMPSolver.h"

//-----------------------------------------------------------------------------
WSMPSolver::WSMPSolver() : m_pA(0)
{
}

//-----------------------------------------------------------------------------
SparseMatrix* WSMPSolver::CreateSparseMatrix(Matrix_Type ntype)
{ 
	return (m_pA = (ntype == REAL_SYMMETRIC? new CompactSymmMatrix(1) : 0)); 
}

//-----------------------------------------------------------------------------
bool WSMPSolver::PreProcess()
{
	// Make sure the solver is available
#ifndef WSMP
	fprintf(stderr, "FATAL ERROR: The WSMP solver is not available on this platform\n\n");
	return false;
#else
	// Auxiliary variables
	int idum, nrhs=1, naux=0;
	double ddum;

	m_n = m_pA->Size();
	m_nnz = m_pA->NonZeroes();

	// Initialize m_perm and m_invp
	m_perm.create(m_n); m_perm.zero();
	m_invp.create(m_n); m_invp.zero();
	m_b.create(m_n);    m_b.zero();


	// Number of processors OMP_NUM_THREADS
	char* var = getenv("OMP_NUM_THREADS");
	int num_procs;
	if(var) num_procs = -atoi(var); // edited 6/1/09 (added negative) per Anshul Gupta
	else {
		fprintf(stderr, "Set environment OMP_NUM_THREADS to 1");
		exit(1);
	}
	wsetmaxthrds_(&num_procs);

// ------------------------------------------------------------------------------
// This step initializes 'm_iparm'
// ------------------------------------------------------------------------------

//	wsmp_initialize_();
	m_iparm[0] = 0;
	m_iparm[1] = 0;
	m_iparm[2] = 0;

	wssmp_(&m_n, m_pA->pointers(), m_pA->indices(), m_pA->values(), &ddum, m_perm, m_invp,
		 m_b, &m_n, &nrhs, &ddum, &naux, &idum, m_iparm, m_dparm);

	if (m_iparm[63])
	{
		fprintf(stderr, "\nERROR during initialization: %i", m_iparm[63]);
		exit(2);
	}

	return LinearSolver::PreProcess();
#endif
}

bool WSMPSolver::Factor()
{
	// Make sure the solver is available
#ifndef WSMP
	fprintf(stderr, "FATAL ERROR: The WSMP solver is not available on this platform\n\n");
	return false;
#else
	// Auxiliary variables
	int idum, nrhs=1, naux=0;
	double ddum;

#ifdef PRINTHB
	m_pA->print_hb(); // Write Harwell-Boeing matrix to file
#endif

// ------------------------------------------------------------------------------
// This step performs matrix ordering
// ------------------------------------------------------------------------------

	m_iparm[1] = 1;
	m_iparm[2] = 1;
	m_dparm[9] = 1.0e-18; // matrix singularity threshold

	wssmp_(&m_n, m_pA->pointers(), m_pA->indices(), m_pA->values(), &ddum, m_perm, m_invp,
		 m_b, &m_n, &nrhs, &ddum, &naux, &idum, m_iparm, m_dparm);

	if (m_iparm[63])
	{
		fprintf(stderr, "\nERROR during ordering: %i", m_iparm[63]);
		exit(2);
	}

// ------------------------------------------------------------------------------
// This step performs symbolic factorization
// ------------------------------------------------------------------------------

	m_iparm[1] = 2;
	m_iparm[2] = 2;

	wssmp_(&m_n, m_pA->pointers(), m_pA->indices(), m_pA->values(), &ddum, m_perm, m_invp,
		 m_b, &m_n, &nrhs, &ddum, &naux, &idum, m_iparm, m_dparm);

	if (m_iparm[63])
	{
		fprintf(stderr, "\nERROR during ordering: %i", m_iparm[63]);
		exit(2);
	}
// ------------------------------------------------------------------------------
// This step performs Cholesky or LDLT factorization
// ------------------------------------------------------------------------------

	m_iparm[1] = 3;
	m_iparm[2] = 3;
	m_iparm[30] = 1; // 0: Cholesky factorization

	wssmp_(&m_n, m_pA->pointers(), m_pA->indices(), m_pA->values(), &ddum, m_perm, m_invp,
		 m_b, &m_n, &nrhs, &ddum, &naux, &idum, m_iparm, m_dparm);

	if (m_iparm[63])
	{
		fprintf(stderr, "\nERROR during Cholesky factorization: %i", m_iparm[63]);

		if (m_iparm[63] > 0) // Try LDL factorization
		{
			m_iparm[1] = 3;
			m_iparm[2] = 3;
			m_iparm[30] = 1;

			wssmp_(&m_n, m_pA->pointers(), m_pA->indices(), m_pA->values(), &ddum, m_perm, m_invp,
				 &ddum, &m_n, &nrhs, &ddum, &naux, &idum, m_iparm, m_dparm);

			if (m_iparm[63])
			{
				fprintf(stderr, "\nERROR during LDL factorization: %i", m_iparm[63]);
				exit(2);
			}
		}
	}


	return true;
#endif
}

bool WSMPSolver::BackSolve(vector<double>& x, vector<double>& b)
{
	/* Make sure the solver is available */
#ifndef WSMP
	fprintf(stderr, "FATAL ERROR: The WSMP solver is not available on this platform\n\n");
	return false;
#else

	/* Auxiliary variables */
	int i, idum, nrhs=1, naux=0;
	double ddum;

// ------------------------------------------------------------------------------
// This step performs back substitution
// ------------------------------------------------------------------------------

	m_iparm[1] = 4;
	m_iparm[2] = 4;

	wssmp_(&m_n, m_pA->pointers(), m_pA->indices(), m_pA->values(), &ddum, m_perm, m_invp,
		 b, &m_n, &nrhs, &ddum, &naux, &idum, m_iparm, m_dparm);

	if (m_iparm[63])
	{
		fprintf(stderr, "\nERROR during ordering: %i", m_iparm[63]);
		exit(2);
	}

	for (i=0; i<m_n; i++) x[i] = b[i];

	return true;
#endif
}

void WSMPSolver::Destroy()
{
	/* Make sure the solver is available */
#ifndef WSMP
	fprintf(stderr, "FATAL ERROR: The WSMP solver is not available on this platform\n\n");
	exit(1);
#else

	wsmp_clear_();
	LinearSolver::Destroy();

#endif
}
