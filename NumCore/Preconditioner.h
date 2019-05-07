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
#include <FECore/SparseMatrix.h>

class CRSSparseMatrix;

// Base class for preconditioners for iterative linear solvers
class Preconditioner
{
public:
	Preconditioner();
	virtual ~Preconditioner();

	// create a preconditioner for a sparse matrix
	virtual bool Create(SparseMatrix* A) = 0;
	
	// apply to vector P x = y
	virtual void mult_vector(double* x, double* y) = 0;
};

class ILU0_Preconditioner : public Preconditioner
{
public:
	ILU0_Preconditioner();

	// create a preconditioner for a sparse matrix
	bool Create(SparseMatrix* A) override;

	// apply to vector P x = y
	void mult_vector(double* x, double* y) override;

public:
	bool	m_checkZeroDiagonal;	// check for zero diagonals
	double	m_zeroThreshold;		// threshold for zero diagonal check
	double	m_zeroReplace;			// replacement value for zero diagonal

private:
	vector<double>		m_bilu0;
	vector<double>		m_tmp;
	CRSSparseMatrix*	m_K;
};

class ILUT_Preconditioner : public Preconditioner
{
public:
	ILUT_Preconditioner();

	// create a preconditioner for a sparse matrix
	bool Create(SparseMatrix* A) override;

	// apply to vector P x = y
	void mult_vector(double* x, double* y) override;

public:
	int		m_maxfill;
	double	m_fillTol;
	bool	m_checkZeroDiagonal;	// check for zero diagonals
	double	m_zeroThreshold;		// threshold for zero diagonal check
	double	m_zeroReplace;			// replacement value for zero diagonal

private:
	CRSSparseMatrix*	m_K;
	vector<double>	m_bilut;
	vector<int>		m_jbilut;
	vector<int>		m_ibilut;
	vector<double>	m_tmp;
};

class DiagonalPreconditioner : public Preconditioner
{
public:
	DiagonalPreconditioner();

	// create a preconditioner for a sparse matrix
	bool Create(SparseMatrix* A) override;

	// apply to vector P x = y
	void mult_vector(double* x, double* y) override;

private:
	SparseMatrix*	m_P;
	vector<double>	m_D;
};
