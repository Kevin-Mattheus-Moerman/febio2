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
#include "BlockSolver.h"

//-----------------------------------------------------------------------------
//! constructor
BlockSolver::BlockSolver()
{
	m_pA = 0;
	m_tol = 1e-12;
	m_maxiter = 150;
	m_iter = 0;
	m_printLevel = 0;
}

//-----------------------------------------------------------------------------
//! constructor
BlockSolver::~BlockSolver()
{
}

//-----------------------------------------------------------------------------
void BlockSolver::SetRelativeTolerance(double tol)
{
	m_tol = tol;
}

//-----------------------------------------------------------------------------
// get the iteration count
int BlockSolver::GetIterations() const
{
	return m_iter;
}

//-----------------------------------------------------------------------------
// set the print level
void BlockSolver::SetPrintLevel(int n)
{
	m_printLevel = n;
}

//-----------------------------------------------------------------------------
//! Preprocess 
bool BlockSolver::PreProcess()
{
	// make sure we have a matrix
	if (m_pA == 0) return false;

	// get the number of partitions
	int NP = m_pA->Partitions();

	// allocate solvers for diagonal blocks
	m_solver.resize(NP);
	for (int i=0; i<NP; ++i)
	{
		m_solver[i] = new PardisoSolver();
		BlockMatrix::BLOCK& Bi = m_pA->Block(i,i);
		m_solver[i]->SetSparseMatrix(Bi.pA);
		if (m_solver[i]->PreProcess() == false) return false;
	}

	m_iter = 0;

	return true;
}

//-----------------------------------------------------------------------------
//! Factor matrix
bool BlockSolver::Factor()
{
	// factor the diagonal matrices
	int N = (int) m_solver.size();
	for (int i=0; i<N; ++i) m_solver[i]->Factor();

	return true;
}

//-----------------------------------------------------------------------------
//! Backsolve the linear system
bool BlockSolver::BackSolve(vector<double>& x, vector<double>& b)
{
	// get partitions
	int NP = m_pA->Partitions();

	// split right-hand-side and solution vector in partitions
	vector< vector<double> > R(NP);
	vector< vector<double> > X(NP);
	int neq0 = 0;
	for (int i=0; i<NP; ++i)
	{
		int neq = m_pA->PartitionEquations(i);
		vector<double>& Ri = R[i];
		Ri.resize(neq);
		for (int j=0; j<neq; ++j) Ri[j] = b[j + neq0];
		neq0 += neq;

		// also allocate and initialize solution vectors
		X[i].resize(neq);
		zero(X[i]);
	}

	// temp storage for RHS
	vector< vector<double> > T = R;

	// calculate initial norm
	double norm0 = l2_norm(b);
	if (m_printLevel != 0) fprintf(stderr, "%d: %lg\n", 0, norm0);

	// residual vector
	vector<double> res(m_pA->Rows());

	// solve the linear system iteratively
	bool bconv = false;
	m_iter = 0;
	for (int n=0; n<m_maxiter; ++n)
	{
		// loop over rows
		for (int i=0; i<NP; ++i)
		{
			// loop over columns
			for (int j=0; j<NP; ++j)
			{
				if (i != j)
				{
					// get the off-diagonal matrix
					CompactMatrix& Cij = *(m_pA->Block(i,j).pA);

					// multiply with X[j] and add to T[i]
					vector<double>& Xj = X[j];
					vector<double>& Ti = T[i];
					Cij.mult_vector(&Xj[0], &Ti[0]);
				}
			}

			// subtract temp from RHS
			int neq = m_pA->PartitionEquations(i);
			for (int j=0; j<neq; ++j) T[i][j] = R[i][j] - T[i][j];
		}

		// backsolve the equations
		for (int i=0; i<NP; ++i)
		{
			if (m_solver[i]->BackSolve(X[i], T[i]) == false)
				return false;
		}

		// combine solution into single solution vector
		neq0 = 0;
		for (int i = 0; i<NP; ++i)
		{
			int neq = m_pA->PartitionEquations(i);
			vector<double>& Xi = X[i];
			for (int j = 0; j<neq; ++j) x[neq0 + j] = Xi[j];
			neq0 += neq;
		}

		// increment iteration count
		m_iter++;

		// calculate residual
		m_pA->mult_vector(&x[0], &res[0]);
		res -= b;
		double norm = l2_norm(res);
		if (m_printLevel != 0) fprintf(stderr, "%d: %lg\n", m_iter, norm);
		if (norm <= norm0*m_tol)
		{
			bconv = true;
			break;	
		}
	}

	return bconv;
}

//-----------------------------------------------------------------------------
//! Clean up
void BlockSolver::Destroy()
{
	int N = (int) m_solver.size();
	for (int i=0; i<N; ++i) m_solver[i]->Destroy();
}

//-----------------------------------------------------------------------------
//! Create a sparse matrix
SparseMatrix* BlockSolver::CreateSparseMatrix(Matrix_Type ntype)
{
	return (m_pA = new BlockMatrix());
}
