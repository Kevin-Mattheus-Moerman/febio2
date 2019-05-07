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
#include "FEFunction1D.h"
#include "FEModel.h"
#include "LoadCurve.h"
#include "DumpStream.h"

FEFunction1D::FEFunction1D(FEModel* pfem) : m_fem(*pfem)
{
	m_nlc = -1;
	m_scale = 0.0;
}

void FEFunction1D::SetLoadCurveIndex(int lc, double scale)
{
	m_nlc = lc;
	m_scale = scale;
}

double FEFunction1D::value(double x) const
{
	if (m_nlc < 0) return m_scale;

	FELoadCurve* plc = m_fem.GetLoadCurve(m_nlc);
	if (plc == 0) return m_scale;

	return m_scale*plc->Value(x);
}

double FEFunction1D::derive(double x) const
{
	if (m_nlc < 0) return 0.0;

	FELoadCurve* plc = m_fem.GetLoadCurve(m_nlc);
	if (plc == 0) return 0.0;

	return m_scale*plc->Deriv(x);
}

void FEFunction1D::Serialize(DumpStream& ar)
{
	if (ar.IsSaving())
	{
		ar << m_nlc << m_scale;
	}
	else
	{
		ar >> m_nlc >> m_scale;
	}
}
