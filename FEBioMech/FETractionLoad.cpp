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
#include "FETractionLoad.h"
#include "FECore/FEModel.h"

//=============================================================================
BEGIN_PARAMETER_LIST(FETractionLoad, FESurfaceLoad)
	ADD_PARAMETER(m_scale, FE_PARAM_DOUBLE    , "scale"   );
	ADD_PARAMETER(m_TC   , FE_PARAM_DATA_ARRAY, "traction");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
//! constructor
FETractionLoad::FETractionLoad(FEModel* pfem) : FESurfaceLoad(pfem), m_TC(FE_VEC3D)
{
	m_scale = 1.0;

	// get the degrees of freedom
	m_dofX = pfem->GetDOFIndex("x");
	m_dofY = pfem->GetDOFIndex("y");
	m_dofZ = pfem->GetDOFIndex("z");
}

//-----------------------------------------------------------------------------
//! allocate storage
void FETractionLoad::SetSurface(FESurface* ps)
{
	FESurfaceLoad::SetSurface(ps);
	m_TC.Create(ps); 
}

//-----------------------------------------------------------------------------
//! Calculate the residual for the traction load
void FETractionLoad::Residual(const FETimeInfo& tp, FEGlobalVector& R)
{
	vector<double> fe;
	vector<int> lm;

	vec3d r0[FEElement::MAX_NODES];
	vec3d tn[FEElement::MAX_NODES];

	FESurface& surf = *m_psurf;
	FEMesh& mesh = *surf.GetMesh();
	int NF = surf.Elements();
	for (int iel=0; iel<NF; ++iel)
	{
		FESurfaceElement& el = surf.Element(iel);

		int ndof = 3*el.Nodes();
		fe.resize(ndof);

		// nr integration points
		int nint = el.GaussPoints();

		// nr of element nodes
		int neln = el.Nodes();

		// nodal coordinates
		for (int i=0; i<neln; ++i)
		{
			r0[i] = mesh.Node(el.m_node[i]).m_r0;
			tn[i] = m_TC.value<vec3d>(iel, i)*m_scale;
		}

		double* Gr, *Gs;
		double* N;
		double* w  = el.GaussWeights();

		// repeat over integration points
		zero(fe);
		for (int n=0; n<nint; ++n)
		{
			N  = el.H(n);
			Gr = el.Gr(n);
			Gs = el.Gs(n);


			// calculate the tangent vectors
			vec3d dxr(0,0,0), dxs(0,0,0);
			for (int i=0; i<neln; ++i) 
			{
				dxr.x += Gr[i]*r0[i].x;
				dxr.y += Gr[i]*r0[i].y;
				dxr.z += Gr[i]*r0[i].z;

				dxs.x += Gs[i]*r0[i].x;
				dxs.y += Gs[i]*r0[i].y;
				dxs.z += Gs[i]*r0[i].z;
			}
			double dv = ((dxr ^ dxs).norm()*w[n]);

			for (int i=0; i<neln; ++i)
			{
				fe[3*i  ] += N[i]*tn[i].x*dv;
				fe[3*i+1] += N[i]*tn[i].y*dv;
				fe[3*i+2] += N[i]*tn[i].z*dv;
			}
		}

		// get the element's LM vector
		UnpackLM(el, lm);

		// add element force vector to global force vector
		R.Assemble(el.m_node, lm, fe);
	}
}

//-----------------------------------------------------------------------------
void FETractionLoad::UnpackLM(FEElement& el, vector<int>& lm)
{
	FEMesh& mesh = GetFEModel()->GetMesh();
	int N = el.Nodes();
	lm.resize(N*3);
	for (int i=0; i<N; ++i)
	{
		int n = el.m_node[i];
		FENode& node = mesh.Node(n);
		vector<int>& id = node.m_ID;

		lm[3*i  ] = id[m_dofX];
		lm[3*i+1] = id[m_dofY];
		lm[3*i+2] = id[m_dofZ];
	}
}
