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
#include "FGMRES_ILU0_Solver.h"

//=============================================================================
FGMRES_ILU0_Solver::FGMRES_ILU0_Solver()
{
	SetPreconditioner(m_PC = new ILU0_Preconditioner);
}

//-----------------------------------------------------------------------------
// do the zero diagonal check during preconditioner
void FGMRES_ILU0_Solver::DoZeroDiagonalCheck(bool b)
{
	m_PC->m_checkZeroDiagonal = b;
}

//-----------------------------------------------------------------------------
// Set the zero diagonal tolerance value
void FGMRES_ILU0_Solver::SetZeroDiagonalTolerance(double tol)
{
	m_PC->m_zeroThreshold = tol;
}

//-----------------------------------------------------------------------------
// set the zero diagonal replacement value
void FGMRES_ILU0_Solver::SetZeroDiagonalReplacement(double val)
{
	m_PC->m_zeroReplace = val;
}

//-----------------------------------------------------------------------------
SparseMatrix* FGMRES_ILU0_Solver::CreateSparseMatrix(Matrix_Type ntype)
{
	// We can only support non-symmetric matrices on account of the preconditioner
	if (ntype != REAL_UNSYMMETRIC) return 0;
	return FGMRESSolver::CreateSparseMatrix(ntype);
}
