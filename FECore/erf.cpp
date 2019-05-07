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
#include "erf.h"
#include <math.h>

#ifdef WIN32
//-----------------------------------------------------------------------------
// approximation to error function using rational expansion.
double erf(double x)
{
	const double p = 0.3275911;
	const double a1 =  0.254829592;
	const double a2 = -0.284496736;
	const double a3 =  1.421413741;
	const double a4 = -1.453152027;
	const double a5 =  1.061405429;
	double t = 1.0/(1.0 + p*fabs(x));
	double e = t*(a1 + t*(a2 + t*(a3 + t*(a4 + a5*t))))*exp(-x*x);
	return (x > 0 ? 1.0 - e : e - 1.0);
}

//-----------------------------------------------------------------------------
// complementary error function (erfc(x) = 1 - erf(x))
double erfc(double x)
{
	return 1.0 - erf(x);
}
#endif
