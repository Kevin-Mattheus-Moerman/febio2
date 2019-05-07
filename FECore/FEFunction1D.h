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
#include "fecore_api.h"

//-----------------------------------------------------------------------------
class FEModel;
class DumpStream;

//-----------------------------------------------------------------------------
// Class that represents a 1D function
// Currently, only function defined via load curves are defined. That's why
// we need to FEModel class. 
class FECORE_API FEFunction1D
{
public:
	FEFunction1D(FEModel* pfem);

	// Set the load curve index and scale factor
	void SetLoadCurveIndex(int lc, double scale = 1.0);

	// serialization
	void Serialize(DumpStream& ar);

public: 
	// evaluate the function at x
	double value(double x) const;

	// value of first derivative of function at x
	double derive(double x) const;

private:
	int		m_nlc;		//!< load curve index
	double	m_scale;	//!< load curve scale factor
	FEModel&	m_fem;
};
