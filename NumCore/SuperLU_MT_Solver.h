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
//! This class implements a wrapper class for the SuperLU_MT solver

#ifdef SUPERLU_MT
	#include "pdsp_defs.h"
#endif


class SuperLU_MT_Solver : public LinearSolver
{
public:
	SuperLU_MT_Solver();
	bool PreProcess();
	bool Factor();
	bool BackSolve(vector<double>& x, vector<double>& b);
	void Destroy();
	SparseMatrix* CreateSparseMatrix(Matrix_Type ntype);

private:
	CCSSparseMatrix*	m_pA;

#ifdef SUPERLU_MT

protected:

	bool m_balloc;
	bool m_bfact;

	SuperMatrix m_A, m_L, m_U, m_B, m_X;
	vector<int>	m_perm_c;
	vector<int>	m_perm_r;
	vector<int>	etree;

    superlumt_options_t		m_ops;
	superlu_memusage_t		m_mem;

	double	rpg, rcond;
	double	ferr, berr;
	int		info;
	equed_t	equed;

#endif // SUPERLU_MT
};
