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
#include "FEMathValue.h"
#include "DumpStream.h"

FEMathDouble::FEMathDouble()
{
	m_scale = 1.0;
	setExpression("0.0");
}

void FEMathDouble::setExpression(const std::string& expr)
{
	m_expr = expr;
}

std::string FEMathDouble::getExpression() const
{
	return m_expr;
}

void FEMathDouble::setScale(double s)
{
	m_scale = s;
}

double FEMathDouble::value()
{
	int ierr = 0;
	double v = m_math.eval(m_expr.c_str(), ierr);
	return m_scale*v;
}

void FEMathDouble::setVariable(const char* sz, double v)
{
	m_math.SetVariable(sz, v);
}

void FEMathDouble::Serialize(DumpStream& ar)
{
	if (ar.IsSaving())
	{
		ar << m_expr;
		ar << m_scale;
	}
	else
	{
		ar >> m_expr;
		ar >> m_scale;
	}
}
