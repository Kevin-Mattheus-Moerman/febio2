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
#include "FESurfaceLoad.h"
#include "FEMesh.h"

FESurfaceLoad::FESurfaceLoad(FEModel* pfem) : FEBoundaryCondition(FESURFACELOAD_ID, pfem)
{
	m_psurf = 0;
}

FESurfaceLoad::~FESurfaceLoad(void)
{

}

//! calculate residual
// NOTE: Experimental implementation! Goal is to do loops over elements in base class
//       and only implement integrand in derived classes.
void FESurfaceLoad::Residual(const FETimeInfo& tp, FEGlobalVector& R)
{
	vector<double> fe;
	vector<int> lm;

	vec3d rt[FEElement::MAX_NODES];
	vector<double> flux; flux.reserve(FEElement::MAX_NODES);

	FESurface& surf = GetSurface();
	FEMesh& mesh = *surf.GetMesh();
	int npr = surf.Elements();
	for (int i = 0; i<npr; ++i)
	{
		FESurfaceElement& el = m_psurf->Element(i);

		// calculate nodal fluxes
		int neln = el.Nodes();

		// equivalent nodal fluxes
		fe.resize(neln);

		// get the element's nodal coordinates
		for (int j = 0; j<neln; ++j) rt[j] = mesh.Node(el.m_node[j]).m_rt;

		// get the nodal values
		flux.resize(neln, 0.0);
		NodalValues(el, flux);

		// repeat over integration points
		zero(fe);
		double* w = el.GaussWeights();
		int nint = el.GaussPoints();
		for (int n = 0; n<nint; ++n)
		{
			double* N = el.H(n);
			double* Gr = el.Gr(n);
			double* Gs = el.Gs(n);

			vec3d dxr(0, 0, 0), dxs(0, 0, 0);
			for (int j = 0; j<neln; ++j)
			{
				dxr += rt[j] * Gr[j];
				dxs += rt[j] * Gs[j];
			}
			vec3d dxt = dxr ^ dxs;
			double J = dxt.norm();

			for (int j = 0; j<neln; ++j)
			{
				fe[j] -= N[j] * flux[j] * w[n] * J;
			}
		}

		// get the element's LM vector
		UnpackLM(el, lm);

		// add element force vector to global force vector
		R.Assemble(el.m_node, lm, fe);
	}
}
