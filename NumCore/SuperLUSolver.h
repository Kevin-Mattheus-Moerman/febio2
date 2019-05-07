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
#include "FECore/LinearSolver.h"
#include "CompactUnSymmMatrix.h"

//-----------------------------------------------------------------------------
//! Implements a wrapper class for the SuperLU library

//! This solver can only be used on systems where it is available.
//! This solver also uses some of the BLAS routines so this package also needs
//! to be available on the system. Although SuperLU comes with a stripped down
//! version of BLAS.

#ifdef SUPERLU
		#include "slu_ddefs.h"
#endif

class SuperLUSolver : public LinearSolver
{
public:
	SuperLUSolver();
	bool PreProcess();
	bool Factor();
	bool BackSolve(vector<double>& x, vector<double>& b);
	void Destroy();
	SparseMatrix* CreateSparseMatrix(Matrix_Type ntype);

	void print_cnorm(bool b) { m_bcond = b; }

#ifdef SUPERLU
protected:
	double norm(SparseMatrix& K); // calculates the 1-norm of the matrix A
#endif

private:

	bool m_bsymm;	// use symmetric mode or not
	bool m_balloc;
	bool m_bfact;
	bool m_bcond;	// calculate condition numbers

	CCSSparseMatrix*	m_pA;

#ifdef SUPERLU

	SuperMatrix A, L, U, B, X;
	vector<int>	perm_c;
	vector<int>	perm_r;
	vector<int>	etree;

	superlu_options_t	options;
	SuperLUStat_t	stat;
	mem_usage_t	mem_usage;

	double	rpg, rcond;
	double	ferr, berr;
	int		info;
	char	equed[1];

#endif // SUPERLU
};
