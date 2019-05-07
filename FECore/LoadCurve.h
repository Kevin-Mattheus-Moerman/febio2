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
#include "FECoreBase.h"

//-----------------------------------------------------------------------------
// Base class for load curves.
class FECORE_API FELoadCurve : public FECoreBase
{
public:
	// constructor
	FELoadCurve();
	FELoadCurve(const FELoadCurve& lc);

	void operator = (const FELoadCurve& lc);

	// destructor
	virtual ~FELoadCurve();

	// return the last evaluated function value
	double Value() const { return m_value; }

	//! evaluates the loadcurve at time
	void Evaluate(double time)
	{
		m_value = Value(time);
	}

	void Serialize(DumpStream& ar);

	virtual bool CopyFrom(FELoadCurve* lc) = 0;

public:
	// evaluate the function at time t
	virtual double Value(double t) const = 0;

	// evaluate the derivative at time t
	virtual double Deriv(double t) const = 0;

private:
	double	m_value;	//!< value of last call to Value
};

//-----------------------------------------------------------------------------
// A loadcurve that generates a linear ramp
class FECORE_API FELinearRamp : public FELoadCurve
{
public:
	FELinearRamp(FEModel* fem) : m_slope(0.0), m_intercept(0.0) {}
	FELinearRamp(double m, double y0) : m_slope(m), m_intercept(y0){}

	double Value(double t) const
	{
		return m_slope*t  + m_intercept;
	}

	double Deriv(double t) const
	{
		return m_slope;
	}

	bool CopyFrom(FELoadCurve* lc);

private:
	double	m_slope;
	double	m_intercept;
};
