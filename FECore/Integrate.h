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
#include "matrix.h"

//-----------------------------------------------------------------------------
// The purpose of this file is to explore mechanisms for evaluating the integrals 
// that commonly appear in FE formulations. The goal is that this would simplify 
// the implementation of new FE features.

//-----------------------------------------------------------------------------
class FESolidDomain;
class FESolidElement;
class FEMaterialPoint;

//-----------------------------------------------------------------------------
// This class can be used to evaluate quantities that depend on the material point.
// TODO: I wonder if it might be possible for a material to store these data classes
// directly. Maybe I can integrate this with the FEProperty class and create a
// FEMaterialProperty class that offers the functionality presented here. 
template <typename T>
class FEMaterialPointValue
{
public:
	FEMaterialPointValue(){}
	virtual ~FEMaterialPointValue(){}

	// overload this function and implement
	virtual T operator () (FEMaterialPoint& mp) = 0;
};

//-----------------------------------------------------------------------------
// Integrator function for BDB forms
// where B is the shape function gradients
FECORE_API void IntegrateBDB(FESolidDomain& dom, FESolidElement& el, double D, matrix& ke);
FECORE_API void IntegrateBDB(FESolidDomain& dom, FESolidElement& el, const mat3ds& D, matrix& ke);
FECORE_API void IntegrateBDB(FESolidDomain& dom, FESolidElement& el, FEMaterialPointValue<mat3ds>& D, matrix& ke);

//-----------------------------------------------------------------------------
// Integrator function for NCN forms
// where N are the shape functions
FECORE_API void IntegrateNCN(FESolidDomain& dom, FESolidElement& el, double C, matrix& ke);
