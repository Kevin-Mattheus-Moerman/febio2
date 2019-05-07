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
#include "JFNKMatrix.h"
#include "FENewtonSolver.h"

JFNKMatrix::JFNKMatrix(FENewtonSolver* pns, SparseMatrix* K) : m_pns(pns), m_K(K)
{
	m_nrow = m_ncol = pns->m_neq;
	m_nsize = 0;

	// TODO: For contact problems we'll need some mechanism to change the array size
	m_v.resize(m_nrow);
	m_R.resize(m_nrow);
}

//! Create a sparse matrix from a sparse-matrix profile
void JFNKMatrix::Create(SparseMatrixProfile& MP) 
{ 
	m_K->Create(MP); 
	m_nrow = m_K->Rows();
	m_ncol = m_K->Columns();
	m_nsize = m_K->NonZeroes(); 
}

bool JFNKMatrix::mult_vector(double* x, double* r)
{
	double eps = 0.001;
	int neq = m_pns->m_neq;

	for (int i = 0; i<neq; ++i) m_v[i] = eps*x[i];

	m_pns->Update2(m_v);
	if (m_pns->Residual(m_R) == false) return false;

	for (int i = 0; i<neq; ++i)
	{
		r[i] = (m_pns->m_R0[i] - m_R[i]) / eps;
	}

	return  true;
}
