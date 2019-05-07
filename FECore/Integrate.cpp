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
#include "Integrate.h"
#include "FESolidDomain.h"

//-----------------------------------------------------------------------------
void FECORE_API IntegrateBDB(FESolidDomain& dom, FESolidElement& el, double D, matrix& ke)
{
	// vector to store global shape functions
	const int EN = FEElement::MAX_NODES;
	vec3d G[EN];

	// loop over all integration points
	const double *gw = el.GaussWeights();
	int ne = el.Nodes();
	int ni = el.GaussPoints();
	for (int n = 0; n<ni; ++n)
	{
		// calculate jacobian
		double detJt = dom.ShapeGradient(el, n, G);

		// form the matrix
		for (int i = 0; i<ne; ++i)
		{
			for (int j = 0; j<ne; ++j)
			{
				ke[i][j] += (G[i]*G[j])*(D*detJt*gw[n]);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FECORE_API IntegrateBDB(FESolidDomain& dom, FESolidElement& el, const mat3ds& D, matrix& ke)
{
	// vector to store global shape functions
	const int EN = FEElement::MAX_NODES;
	vec3d G[EN];

	// loop over all integration points
	const double *gw = el.GaussWeights();
	int ne = el.Nodes();
	int ni = el.GaussPoints();
	for (int n = 0; n<ni; ++n)
	{
		// calculate jacobian
		double detJt = dom.ShapeGradient(el, n, G);

		// form the matrix
		for (int i = 0; i<ne; ++i)
		{
			for (int j = 0; j<ne; ++j)
			{
				ke[i][j] += (G[i] * (D * G[j]))*(detJt*gw[n]);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FECORE_API IntegrateBDB(FESolidDomain& dom, FESolidElement& el, FEMaterialPointValue<mat3ds>& D, matrix& ke)
{
	// vector to store global shape functions
	const int EN = FEElement::MAX_NODES;
	vec3d G[EN];

	// loop over all integration points
	const double *gw = el.GaussWeights();
	int ne = el.Nodes();
	int ni = el.GaussPoints();
	for (int n = 0; n<ni; ++n)
	{
		// get the material point
		FEMaterialPoint& mp = *el.GetMaterialPoint(n);

		// calculate jacobian
		double detJt = dom.ShapeGradient(el, n, G);

		// calculate D at this point
		mat3ds Dn = D(mp);

		// form the matrix
		for (int i = 0; i<ne; ++i)
		{
			for (int j = 0; j<ne; ++j)
			{
				ke[i][j] += (G[i] * (Dn * G[j]))*(detJt*gw[n]);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FECORE_API IntegrateNCN(FESolidDomain& dom, FESolidElement& el, double C, matrix& ke)
{
	// number of nodes
	int ne = el.Nodes();

	// jacobian
	double Ji[3][3];

	// loop over all integration points
	const double *gw = el.GaussWeights();
	int ni = el.GaussPoints();
	for (int n = 0; n<ni; ++n)
	{
		// calculate jacobian
		double detJt = dom.invjact(el, Ji, n);

		// shape function values at integration point n
		double* H = el.H(n);

		for (int i = 0; i<ne; ++i)
		{
			for (int j = 0; j<ne; ++j)
			{
				ke[i][j] += H[i] * H[j]*C*detJt*gw[n];
			}
		}
	}
}
