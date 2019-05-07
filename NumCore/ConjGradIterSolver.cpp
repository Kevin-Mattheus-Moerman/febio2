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
#include "ConjGradIterSolver.h"
#include "FECore/vector.h"
#include <algorithm>
#include <assert.h>

//-----------------------------------------------------------------------------
ConjGradIterSolver::ConjGradIterSolver() : m_pA(0)
{
	m_tol = 0.01;
	m_kmax = 200;
	m_nprint = 0;
}

//-----------------------------------------------------------------------------
//! Create a sparse matrix for this linear solver
SparseMatrix* ConjGradIterSolver::CreateSparseMatrix(Matrix_Type ntype)
{ 
	return (m_pA = (ntype == REAL_SYMMETRIC? new CompactSymmMatrix() : 0)); 
}

//-----------------------------------------------------------------------------
bool ConjGradIterSolver::PreProcess()
{
	assert(m_pA);
	return true;
}

//-----------------------------------------------------------------------------
bool ConjGradIterSolver::Factor()
{
/*	
	int neq = m_pA->.Size();

	int* pi = m_pA->indices();
	int* pr = m_pA->pointers();
	double* pv = m_pA->values();

	// copy the diagonals to P
	m_P.create(neq);
	int i, j, n, l;

	// store the diagonal elements in P
	for (j=0; j<neq; ++j) m_P[j] = 1.0 / pv[pr[j]];

	for (j=0; j<neq; ++j)
	{
		// get the number of entries on the column
		l = pr[j+1]-pr[j];

		// loop over all rows
		for (i=0; i<l; ++i)
		{
			n = pi[pr[j] + i];
			pv[ pr[j] + i] *= m_P[n];
		}
	}
*/
	return true;
}

//-----------------------------------------------------------------------------
bool ConjGradIterSolver::BackSolve(vector<double>& x, vector<double>& b)
{
	int i;

	int N = (int)x.size();

	// intialize x to zero
	std::fill(x.begin(), x.end(), 0);

	// iteration counter
	int k = 0;

	// initial residual vector
	vector<double> r(N, 0.0);
	m_pA->mult_vector(&x[0], &r[0]);
	for (i=0; i<N; ++i) r[i] = b[i] - r[i];

	// initial norms
	double rho1 = r*r;
	double rho2 = rho1;
	double normb = 0;
	for (i=0; i<N; ++i) normb += (b[i])*(b[i]);
	normb = sqrt(normb);

	// search direction
	std::vector<double> p; p.assign(N, 0.0);

	// work vector
	std::vector<double> w(N);

	double alpha, beta;

	if (m_nprint > 0) fprintf(stderr, "Solving linear system ...\n");

	// loop until converged or until max iterations reached
	while ((sqrt(rho1) > m_tol*normb) && (k < m_kmax))
	{
		// increase iteration counter
		++k;

		// calculate search direction
		beta = rho1/rho2;
		for (i=0; i<N; ++i) p[i] = r[i] + p[i]*beta;

		for (i=0; i<N; ++i) w[i] = 0.0;
		m_pA->mult_vector(&p[0], &w[0]);
		alpha = rho1 / (p*w);

		for (i=0; i<N; ++i)
		{
			x[i] += p[i]*alpha;
			r[i] -= w[i]*alpha;
		}
		rho2 = rho1;
		rho1 = r*r;

		if (m_nprint > 0)
			fprintf(stderr, "%d: %lg\n", k, sqrt(rho1)/normb);
	}

	if (m_nprint > 0)
	{
		if ((k >= m_kmax) && (sqrt(rho1) > m_tol*normb))
		{
			fprintf(stderr, "Max iterations reached. Solution has not converged.\n");
		}
		else fprintf(stderr, "Solution converged\n");
	}

	return true;
}

//-----------------------------------------------------------------------------
void ConjGradIterSolver::Destroy()
{
	LinearSolver::Destroy();
}
