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
#include "FEFacet2FacetSliding.h"
#include "FECore/FEModel.h"
#include "FECore/FEClosestPointProjection.h"
#include "FECore/log.h"
#include "FECore/FEGlobalMatrix.h"
#include "FECore/FEDataExport.h"

//-----------------------------------------------------------------------------
// Define sliding interface parameters
BEGIN_PARAMETER_LIST(FEFacet2FacetSliding, FEContactInterface)
	ADD_PARAMETER(m_epsn     , FE_PARAM_DOUBLE, "penalty"      );
	ADD_PARAMETER(m_bautopen , FE_PARAM_BOOL  , "auto_penalty" );
	ADD_PARAMETER(m_blaugon  , FE_PARAM_BOOL  , "laugon"       );
	ADD_PARAMETER(m_atol     , FE_PARAM_DOUBLE, "tolerance"    );
	ADD_PARAMETER(m_btwo_pass, FE_PARAM_BOOL  , "two_pass"     );
	ADD_PARAMETER(m_gtol     , FE_PARAM_DOUBLE, "gaptol"       );
	ADD_PARAMETER(m_naugmin  , FE_PARAM_INT   , "minaug"       );
	ADD_PARAMETER(m_naugmax  , FE_PARAM_INT   , "maxaug"       );
	ADD_PARAMETER(m_knmult   , FE_PARAM_DOUBLE, "knmult"       );
	ADD_PARAMETER(m_stol     , FE_PARAM_DOUBLE, "search_tol"   );
	ADD_PARAMETER(m_srad     , FE_PARAM_DOUBLE, "search_radius");
	ADD_PARAMETER(m_dxtol    , FE_PARAM_DOUBLE, "dxtol"        );
	ADD_PARAMETER(m_mu       , FE_PARAM_DOUBLE, "fric_coeff"   );
	ADD_PARAMETER(m_epsf     , FE_PARAM_DOUBLE, "fric_penalty" );
	ADD_PARAMETER(m_nsegup   , FE_PARAM_INT   , "seg_up"       );
	ADD_PARAMETER(m_breloc   , FE_PARAM_BOOL  , "node_reloc"   );
    ADD_PARAMETER(m_bsmaug   , FE_PARAM_BOOL  , "smooth_aug"   );
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FEFacetSlidingSurface::Data::Data()
{
	m_gap = 0.0;
	m_Lm  = 0.0;
	m_Ln  = 0.0;
	m_eps = 1.0;
	m_nu = vec3d(0,0,0);
	m_rs = vec2d(0,0);
	m_pme = (FESurfaceElement*)0;
}

//-----------------------------------------------------------------------------
// FEFacetSlidingSurface
//-----------------------------------------------------------------------------

FEFacetSlidingSurface::FEFacetSlidingSurface(FEModel* pfem) : FEContactSurface(pfem)
{
	// define class exports
	EXPORT_DATA(PLT_VEC3F, FMT_NODE, &m_Fn, "contact nodal forces");
}

//-----------------------------------------------------------------------------
bool FEFacetSlidingSurface::Init()
{
	// initialize surface data first
	if (FEContactSurface::Init() == false) return false;

	int nn = Nodes();
	m_Fn.assign(nn, vec3d(0,0,0));

	// allocate data structures
	const int NE = Elements();
	m_Data.resize(NE);
	for (int i=0; i<NE; ++i)
	{
		FESurfaceElement& el = Element(i);
		int nint = el.GaussPoints();
		m_Data[i].resize(nint);
	}

	return true;
}

//-----------------------------------------------------------------------------
vec3d FEFacetSlidingSurface::GetContactForce()
{
	// initialize contact force
	vec3d f(0,0,0);
	
	// loop over all elements of the primary surface
	for (int i=0; i<m_Fn.size(); ++i) f += m_Fn[i];

/*
	for (int n=0; n<Elements(); ++n)
	{
		FESurfaceElement& el = Element(n);
		int nint = el.GaussPoints();
		
		// evaluate the contact force for that element
		for (int i=0; i<nint; ++i) 
		{
			Data& pt = m_Data[n][i];
			// unit vector
			vec3d n = SurfaceNormal(el, i);
			// gauss weight
			double w = el.GaussWeights()[i];
			// area in reference configuration
			vec3d g0[2];
			double r = el.gr(i);
			double s = el.gs(i);
			CoBaseVectors0(el, r, s, g0);
			double A = (g0[0] ^ g0[1]).unit();
			// contact force
			f += n*(w*pt.m_Ln*A);
		}
	}
*/	
	return f;
}

//-----------------------------------------------------------------------------
double FEFacetSlidingSurface::GetContactArea()
{
	// initialize contact area
	double a = 0;
	
	// loop over all elements of the primary surface
	for (int n=0; n<Elements(); ++n)
	{
		FESurfaceElement& el = Element(n);
		int nint = el.GaussPoints();
		
		// evaluate the contact force for that element
		for (int i=0; i<nint; ++i)
		{
			// get data for this integration point
			Data& data = m_Data[n][i];
            double s = (data.m_Ln > 0) ? 1 : 0;
            
			// get the base vectors
			vec3d g[2];
			CoBaseVectors(el, i, g);
            
			// normal (magnitude = area)
			vec3d n = g[0] ^ g[1];
            
			// gauss weight
			double w = el.GaussWeights()[i];
            
			// contact force
			a += n.norm()*(w*s);
		}
	}
	
	return a;
}

//-----------------------------------------------------------------------------
void FEFacetSlidingSurface::Serialize(DumpStream& ar)
{
	FEContactSurface::Serialize(ar);

	if (ar.IsShallow())
	{
		if (ar.IsSaving())
		{
			for (int i=0; i<(int) m_Data.size(); ++i)
			{
				vector<Data>& di = m_Data[i];
				int nint = (int) di.size();
				for (int j=0; j<nint; ++j)
				{
					Data& d = di[j];
					ar << d.m_gap;
					ar << d.m_nu;
					ar << d.m_rs;
					ar << d.m_Lm;
					ar << d.m_eps;
					ar << d.m_Ln;
				}
			}
		}
		else
		{
			for (int i=0; i<(int) m_Data.size(); ++i)
			{
				vector<Data>& di = m_Data[i];
				int nint = (int) di.size();
				for (int j=0; j<nint; ++j)
				{
					Data& d = di[j];
					ar >> d.m_gap;
					ar >> d.m_nu;
					ar >> d.m_rs;
					ar >> d.m_Lm;
					ar >> d.m_eps;
					ar >> d.m_Ln;
				}
			}
		}	
	}
	else
	{
		if (ar.IsSaving())
		{
			for (int i=0; i<(int) m_Data.size(); ++i)
			{
				vector<Data>& di = m_Data[i];
				int nint = (int) di.size();
				for (int j=0; j<nint; ++j)
				{
					Data& d = di[j];
					ar << d.m_gap;
					ar << d.m_nu;
					ar << d.m_rs;
					ar << d.m_Lm;
					ar << d.m_eps;
					ar << d.m_Ln;
				}
			}
		}
		else
		{
			for (int i=0; i<(int) m_Data.size(); ++i)
			{
				vector<Data>& di = m_Data[i];
				int nint = (int) di.size();
				for (int j=0; j<nint; ++j)
				{
					Data& d = di[j];
					ar >> d.m_gap;
					ar >> d.m_nu;
					ar >> d.m_rs;
					ar >> d.m_Lm;
					ar >> d.m_eps;
					ar >> d.m_Ln;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEFacetSlidingSurface::GetContactGap(int nface, double& pg)
{
    FESurfaceElement& el = Element(nface);
    int ni = el.GaussPoints();
    pg = 0;
    for (int k=0; k<ni; ++k) pg += m_Data[nface][k].m_gap;
    pg /= ni;
}

//-----------------------------------------------------------------------------
void FEFacetSlidingSurface::GetContactPressure(int nface, double& pg)
{
    FESurfaceElement& el = Element(nface);
    int ni = el.GaussPoints();
    pg = 0;
    for (int k=0; k<ni; ++k) pg += m_Data[nface][k].m_Ln;
    pg /= ni;
}

//-----------------------------------------------------------------------------
void FEFacetSlidingSurface::GetContactTraction(int nface, vec3d& pt)
{
    FESurfaceElement& el = Element(nface);
    int ni = el.GaussPoints();
    pt = vec3d(0,0,0);
    for (int k=0; k<ni; ++k) pt -= m_Data[nface][k].m_nu*m_Data[nface][k].m_Ln;
    pt /= ni;
}

//-----------------------------------------------------------------------------
void FEFacetSlidingSurface::GetNodalContactGap(int nface, double* gn)
{
	FESurfaceElement& el = Element(nface);
	int ne = el.Nodes();
	int ni = el.GaussPoints();
	double gi[FEElement::MAX_INTPOINTS];
	for (int k=0; k<ni; ++k) gi[k] = m_Data[nface][k].m_gap;

	for (int k=0; k<ni; ++k) if (gi[k] < 0) gi[k] = 0;
	el.project_to_nodes(gi, gn);

	for (int k=0; k<ne; ++k) if (gn[k] < 0) gn[k] = 0;
}

//-----------------------------------------------------------------------------
void FEFacetSlidingSurface::GetNodalContactPressure(int nface, double* pn)
{
	FESurfaceElement& el = Element(nface);
	int ne = el.Nodes();
	int ni = el.GaussPoints();
	double ti[FEElement::MAX_INTPOINTS];
	for (int k=0; k<ni; ++k)
	{
		double L = m_Data[nface][k].m_Ln;
		ti[k] = L;// + pf->m_epsn*gi[k];
		ti[k] = (ti[k]>=0?ti[k] : 0);		
	}

	el.project_to_nodes(ti, pn);
	for (int k=0; k<ni; ++k)
		pn[k] = (pn[k]>=0?pn[k] : 0);		
}

//-----------------------------------------------------------------------------
void FEFacetSlidingSurface::GetNodalContactTraction(int nface, vec3d* tn)
{
	FESurfaceElement& el = Element(nface);
	int ne = el.Nodes();
	int ni = el.GaussPoints();

	vec3d t;
	const int MFI = FEElement::MAX_INTPOINTS;
	double tix[MFI], tiy[MFI], tiz[MFI];
	for (int k=0; k<ni; ++k)
	{
		FEFacetSlidingSurface::Data& pt = m_Data[nface][k];
		double gi = pt.m_gap;
		double Li = pt.m_Ln;
		vec3d  ti = pt.m_nu;
		if (gi > 0) t = ti*(Li); else t = vec3d(0,0,0);
		tix[k] = t.x; tiy[k] = t.y; tiz[k] = t.z;
	}

	// project traction to nodes
	const int MFN = FEElement::MAX_NODES;
	double tnx[MFN], tny[MFN], tnz[MFN];
	el.project_to_nodes(tix, tnx);
	el.project_to_nodes(tiy, tny);
	el.project_to_nodes(tiz, tnz);

	// store data
	for (int k=0; k<ne; ++k)
	{
		tn[k].x = tnx[k];
		tn[k].y = tny[k];
		tn[k].z = tnz[k];
	}
}

//-----------------------------------------------------------------------------
// FEFacet2FacetSliding
//-----------------------------------------------------------------------------

FEFacet2FacetSliding::FEFacet2FacetSliding(FEModel* pfem) : FEContactInterface(pfem), m_ss(pfem), m_ms(pfem)
{
	static int ncount = 1;
	SetID(ncount++);

	// default parameters
	m_epsn = 1.0;
	m_knmult = 1.0;
	m_stol = 0.01;
	m_btwo_pass = false;
	m_bautopen = false;
	m_nsegup = 0;	// always do segment updates
	m_breloc = false;
    m_bsmaug = false;

	m_atol = 0.01;
	m_gtol = 0;
	m_naugmin = 0;
	m_naugmax = 10;
	m_srad = 1.0;

	m_dxtol = 0;

	// Note that friction has not been implemented yet
	m_mu = 0;
	m_epsf = 0;

	m_ss.SetSibling(&m_ms);
	m_ms.SetSibling(&m_ss);
}

//-----------------------------------------------------------------------------
//! build the matrix profile for use in the stiffness matrix
void FEFacet2FacetSliding::BuildMatrixProfile(FEGlobalMatrix& K)
{
	FEModel& fem = *GetFEModel();
	FEMesh& mesh = fem.GetMesh();

	// get the DOFS
	const int dof_X = fem.GetDOFIndex("x");
	const int dof_Y = fem.GetDOFIndex("y");
	const int dof_Z = fem.GetDOFIndex("z");
	const int dof_RU = fem.GetDOFIndex("Ru");
	const int dof_RV = fem.GetDOFIndex("Rv");
	const int dof_RW = fem.GetDOFIndex("Rw");

	vector<int> lm(6*FEElement::MAX_NODES*2);

	int npass = (m_btwo_pass?2:1);
	for (int np=0; np<npass; ++np)
	{
		FEFacetSlidingSurface& ss = (np == 0? m_ss : m_ms);
		FEFacetSlidingSurface& ms = (np == 0? m_ms : m_ss);

		for (int j=0; j<ss.Elements(); ++j)
		{
			FESurfaceElement& se = ss.Element(j);
			int nint = se.GaussPoints();
			int* sn = &se.m_node[0];
			for (int k=0; k<nint; ++k)
			{
				FEFacetSlidingSurface::Data& pt = ss.m_Data[j][k];
				FESurfaceElement* pe = pt.m_pme;
				if (pe != 0)
				{
					FESurfaceElement& me = *pe;
					int* mn = &me.m_node[0];

					assign(lm, -1);

					int nseln = se.Nodes();
					int nmeln = me.Nodes();

					for (int l=0; l<nseln; ++l)
					{
						vector<int>& id = mesh.Node(sn[l]).m_ID;
						lm[6*l  ] = id[dof_X];
						lm[6*l+1] = id[dof_Y];
						lm[6*l+2] = id[dof_Z];
						lm[6*l+3] = id[dof_RU];
						lm[6*l+4] = id[dof_RV];
						lm[6*l+5] = id[dof_RW];
					}

					for (int l=0; l<nmeln; ++l)
					{
						vector<int>& id = mesh.Node(mn[l]).m_ID;
						lm[6*(l+nseln)  ] = id[dof_X];
						lm[6*(l+nseln)+1] = id[dof_Y];
						lm[6*(l+nseln)+2] = id[dof_Z];
						lm[6*(l+nseln)+3] = id[dof_RU];
						lm[6*(l+nseln)+4] = id[dof_RV];
						lm[6*(l+nseln)+5] = id[dof_RW];
					}

					K.build_add(lm);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
//! Initialization routine
bool FEFacet2FacetSliding::Init()
{
	m_bfirst = true;
	m_normg0 = 0.0;

	// initialize surface data
	if (m_ss.Init() == false) return false;
	if (m_ms.Init() == false) return false;

	return true;
}

//-----------------------------------------------------------------------------
void FEFacet2FacetSliding::Activate()
{
	// don't forget the base class
	FEContactInterface::Activate();

	// calculate penalty factors
	if (m_bautopen) CalcAutoPenalty(m_ss);

	// project slave surface onto master surface
	ProjectSurface(m_ss, m_ms, true, m_breloc);

	if (m_btwo_pass) 
	{
		ProjectSurface(m_ms, m_ss, true);
		if (m_bautopen) CalcAutoPenalty(m_ms);
	}

	// check friction parameters
	// since friction has not been implemented yet
	if ((m_mu != 0) || (m_epsf != 0))
	{
		felog.printbox("WARNING", "Friction has NOT been implemented yet for facet-to-facet contact\ninterfaces. Friction parameters are ignored.");
		m_mu = 0;
		m_epsf = 0;
	}
}

//-----------------------------------------------------------------------------
void FEFacet2FacetSliding::CalcAutoPenalty(FEFacetSlidingSurface& s)
{
	// get the mesh
	FEMesh& m = GetFEModel()->GetMesh();

	// loop over all surface elements
	for (int i=0; i<s.Elements(); ++i)
	{
		// get the surface element
		FESurfaceElement& el = s.Element(i);

		// calculate a penalty
		double eps = AutoPenalty(el, s);

		// assign to integation points of surface element
		int nint = el.GaussPoints();
		for (int j=0; j<nint; ++j) 
		{
			FEFacetSlidingSurface::Data& pt = s.m_Data[i][j];
			pt.m_eps = eps;
		}
	}
}

//-----------------------------------------------------------------------------
//! In this function we project the integration points to the master surface,
//! calculate the projection's natural coordinates and normal vector
//
void FEFacet2FacetSliding::ProjectSurface(FEFacetSlidingSurface &ss, FEFacetSlidingSurface &ms, bool bsegup, bool bmove)
{
	FEClosestPointProjection cpp(ms);
	cpp.HandleSpecialCases(true);
	cpp.SetTolerance(m_stol);
	cpp.Init();

	// if we need to project the nodes onto the master surface,
	// let's do this first
	if (bmove)
	{
		int NN = ss.Nodes();
		int NE = ss.Elements();
		// first we need to calculate the node normals
		vector<vec3d> normal; normal.assign(NN, vec3d(0,0,0));
		for (int i=0; i<NE; ++i)
		{
			FESurfaceElement& el = ss.Element(i);
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j)
			{
				vec3d r0 = ss.Node(el.m_lnode[ j         ]).m_rt;
				vec3d rp = ss.Node(el.m_lnode[(j+   1)%ne]).m_rt;
				vec3d rm = ss.Node(el.m_lnode[(j+ne-1)%ne]).m_rt;
				vec3d n = (rp - r0)^(rm - r0);
				normal[el.m_lnode[j]] += n;
			}
		}
		for (int i=0; i<NN; ++i) normal[i].unit();

		// loop over all nodes
		for (int i=0; i<NN; ++i)
		{
			FENode& node = ss.Node(i);

			// get the spatial nodal coordinates
			vec3d rt = node.m_rt;
			vec3d nu = normal[i];

			// project onto the master surface
			vec3d q;
			vec2d rs(0,0);
			FESurfaceElement* pme = cpp.Project(rt, q, rs);
			if (pme) 
			{
				double gap = (nu*(rt - q));
				if (gap>0) node.m_r0 = node.m_rt = q;
			}
		}
	}

	// loop over all slave elements
	int NE = ss.Elements();

#pragma omp parallel for shared(cpp) schedule(dynamic)
	for (int i=0; i<NE; ++i)
	{
		// get the slave element
		FESurfaceElement& se = ss.Element(i);
		int nn = se.Nodes();

		// get nodal coordinates
		vec3d re[FEElement::MAX_NODES];
		for (int l=0; l<nn; ++l) re[l] = ss.GetMesh()->Node(se.m_node[l]).m_rt;

		// loop over all its integration points
		int nint = se.GaussPoints();
		for (int j=0; j<nint; ++j)
		{
			// get the integration point data
			FEFacetSlidingSurface::Data& pt = ss.m_Data[i][j];

			// calculate the global coordinates of this integration point
			double* H = se.H(j);

			vec3d x(0,0,0), q;
			for (int k=0; k<nn; ++k) x += re[k]*H[k];

			// see if the point still projects to the same element
			if (pt.m_pme)
			{
				// update projection to master element
				FESurfaceElement& mel = *pt.m_pme;
				q = ms.ProjectToSurface(mel, x, pt.m_rs[0], pt.m_rs[1]);

				// see if the projection is still in the element
				if (bsegup && (!ms.IsInsideElement(mel, pt.m_rs[0], pt.m_rs[1], m_stol)))
				{
					// if not, do a new search
					pt.m_rs = vec2d(0,0);
					FESurfaceElement* pme = 0;
					pme = cpp.Project(x, q, pt.m_rs);
					pt.m_pme = pme;
				}
			}
			if (bsegup)
			{
				// find the master segment this element belongs to
				pt.m_rs = vec2d(0,0);
				FESurfaceElement* pme = 0;
				pme = cpp.Project(x, q, pt.m_rs);
				pt.m_pme = pme;
			}

			// update normal and gap at integration point
			if (pt.m_pme)
			{
				double r = pt.m_rs[0];
				double s = pt.m_rs[1];

				// the slave normal is set to the master element normal
				pt.m_nu = ms.SurfaceNormal(*pt.m_pme, r, s);

				// calculate gap
				pt.m_gap = -pt.m_nu*(x - q);
			}
			else
			{
				// since the node is not in contact, we set the gap and Lagrange multiplier to zero
				pt.m_gap = 0;
				pt.m_Lm = 0;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEFacet2FacetSliding::Update(int niter, const FETimeInfo& tp)
{
	FEModel& fem = *GetFEModel();

	// should we do a segment update or not?
	// TODO: check what happens when m_nsegup == -1 and m_npass = 2;
	// We have to make sure that in this case, both surfaces get at least
	// one pass!
	bool bupdate = (m_bfirst || (m_nsegup == 0)? true : (niter <= m_nsegup));

	// project slave surface to master surface
	ProjectSurface(m_ss, m_ms, bupdate);
	if (m_btwo_pass) ProjectSurface(m_ms, m_ss, bupdate);

	// Update the net contact pressures
	UpdateContactPressures();

	m_bfirst = false;
}

//-----------------------------------------------------------------------------
void FEFacet2FacetSliding::Residual(FEGlobalVector& R, const FETimeInfo& tp)
{
	vector<int> sLM, mLM, LM, en;
	vector<double> fe;

	const int MELN = FEElement::MAX_NODES;
	double detJ[MELN], w[MELN], *Hs, Hm[MELN];
	vec3d r0[MELN];

	m_ss.m_Fn.assign(m_ss.Nodes(), vec3d(0,0,0));
	m_ms.m_Fn.assign(m_ms.Nodes(), vec3d(0,0,0));

	int npass = (m_btwo_pass?2:1);
	for (int np=0; np<npass; ++np)
	{
		FEFacetSlidingSurface& ss = (np == 0? m_ss : m_ms);
		FEFacetSlidingSurface& ms = (np == 0? m_ms : m_ss);

		// loop over all slave elements
		for (int i=0; i<ss.Elements(); ++i)
		{
			FESurfaceElement& se = ss.Element(i);
			int nseln = se.Nodes();
			int nint = se.GaussPoints();

			// get the element's LM vector
			ss.UnpackLM(se, sLM);

			// nodal coordinates
			for (int j=0; j<nseln; ++j) r0[j] = ss.GetMesh()->Node(se.m_node[j]).m_r0;

			// we calculate all the metrics we need before we
			// calculate the nodal forces
			for (int j=0; j<nint; ++j)
			{
				double* Gr = se.Gr(j);
				double* Gs = se.Gs(j);

				// calculate jacobian
				// note that we are integrating over the reference surface
				vec3d dxr, dxs;
				for (int k=0; k<nseln; ++k)
				{
					dxr.x += Gr[k]*r0[k].x;
					dxr.y += Gr[k]*r0[k].y;
					dxr.z += Gr[k]*r0[k].z;

					dxs.x += Gs[k]*r0[k].x;
					dxs.y += Gs[k]*r0[k].y;
					dxs.z += Gs[k]*r0[k].z;
				}

				// jacobians
				detJ[j] = (dxr ^ dxs).norm();

				// integration weights
				w[j] = se.GaussWeights()[j];
			}

			// loop over all integration points
			for (int j=0; j<nint; ++j)
			{
				// get integration point data
				FEFacetSlidingSurface::Data& pt = ss.m_Data[i][j];

				// get the master element
				FESurfaceElement* pme = pt.m_pme;
				if (pme)
				{
					FESurfaceElement& me = *pme;

					int nmeln = me.Nodes();
					ms.UnpackLM(me, mLM);

					// calculate degrees of freedom
					int ndof = 3*(nseln + nmeln);

					// build the LM vector
					LM.resize(ndof);
					for (int k=0; k<nseln; ++k)
					{
						LM[3*k  ] = sLM[3*k  ];
						LM[3*k+1] = sLM[3*k+1];
						LM[3*k+2] = sLM[3*k+2];
					}

					for (int k=0; k<nmeln; ++k)
					{
						LM[3*(k+nseln)  ] = mLM[3*k  ];
						LM[3*(k+nseln)+1] = mLM[3*k+1];
						LM[3*(k+nseln)+2] = mLM[3*k+2];
					}

					// build the en vector
					en.resize(nseln+nmeln);
					for (int k=0; k<nseln; ++k) en[k] = se.m_node[k];
					for (int k=0; k<nmeln; ++k) en[k+nseln] = me.m_node[k];

					// calculate shape functions
					Hs = se.H(j);

					double r = pt.m_rs[0];
					double s = pt.m_rs[1];
					me.shape_fnc(Hm, r, s);

					// get normal vector
					vec3d nu = pt.m_nu;

					// gap function
					double g = pt.m_gap;
					
					// lagrange multiplier
					double Lm = pt.m_Lm;

					// penalty value
					double eps = m_epsn*pt.m_eps;

					// contact traction
					double tn = Lm + eps*g;
					tn = MBRACKET(tn);

					// calculate the force vector
					fe.resize(ndof);

					for (int k=0; k<nseln; ++k)
					{
						fe[3*k  ] = Hs[k]*nu.x;
						fe[3*k+1] = Hs[k]*nu.y;
						fe[3*k+2] = Hs[k]*nu.z;
					}

					for (int k=0; k<nmeln; ++k)
					{
						fe[3*(k+nseln)  ] = -Hm[k]*nu.x;
						fe[3*(k+nseln)+1] = -Hm[k]*nu.y;
						fe[3*(k+nseln)+2] = -Hm[k]*nu.z;
					}

					for (int k=0; k<ndof; ++k) fe[k] *= tn*detJ[j]*w[j];

					for (int k=0; k<nseln; ++k) ss.m_Fn[se.m_lnode[k]] += vec3d(fe[3*k], fe[3*k+1], fe[3*k+2]);
					for (int k=0; k<nmeln; ++k) ms.m_Fn[me.m_lnode[k]] += vec3d(fe[3*nseln+3*k], fe[3*nseln+3*k+1], fe[3*nseln+3*k+2]);

					// assemble the global residual
					R.Assemble(en, LM, fe);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------

void FEFacet2FacetSliding::StiffnessMatrix(FESolver* psolver, const FETimeInfo& tp)
{
	vector<int> sLM, mLM, LM, en;
	const int MN = FEElement::MAX_NODES;
	const int ME = 3*MN*2;
	double N[ME], T1[ME], T2[ME], N1[ME] = {0}, N2[ME] = {0}, D1[ME], D2[ME], Nb1[ME], Nb2[ME];
	matrix ke;

	// get the mesh
	FEMesh* pm = m_ss.GetMesh();

	// see how many reformations we've had to do so far
	int nref = psolver->m_nref;

	// get the "size" of the model
	// We need this to scale the insertion distance
	double R = GetFEModel()->GetMesh().GetBoundingBox().radius();
	double dxtol = R*m_dxtol;

	// set higher order stiffness mutliplier
	double knmult = m_knmult;
	if (m_knmult < 0)
	{
		int ni = int(-m_knmult);
		if (nref >= ni)
		{
			knmult = 1; 
			felog.printf("Higher order stiffness terms included.\n");
		}
		else knmult = 0;
	}

	double detJ[MN], w[MN], *Hs, Hm[MN], Hmr[MN], Hms[MN];
	vec3d r0[MN];

	int npass = (m_btwo_pass?2:1);
	for (int np=0; np < npass; ++np)
	{
		FEFacetSlidingSurface& ss = (np == 0? m_ss : m_ms);
		FEFacetSlidingSurface& ms = (np == 0? m_ms : m_ss);

		// loop over all slave elements
		for (int i=0; i<ss.Elements(); ++i)
		{
			FESurfaceElement& se = ss.Element(i);
			int nseln = se.Nodes();
			int nint = se.GaussPoints();

			// get the element's LM vector
			ss.UnpackLM(se, sLM);

			// nodal coordinates
			for (int j=0; j<nseln; ++j) r0[j] = ss.GetMesh()->Node(se.m_node[j]).m_r0;

			// we calculate all the metrics we need before we
			// calculate the nodal forces
			for (int j=0; j<nint; ++j)
			{
				double* Gr = se.Gr(j);
				double* Gs = se.Gs(j);

				// calculate jacobian
				// note that we are integrating over the reference surface
				vec3d dxr, dxs;
				for (int k=0; k<nseln; ++k)
				{
					dxr.x += Gr[k]*r0[k].x;
					dxr.y += Gr[k]*r0[k].y;
					dxr.z += Gr[k]*r0[k].z;

					dxs.x += Gs[k]*r0[k].x;
					dxs.y += Gs[k]*r0[k].y;
					dxs.z += Gs[k]*r0[k].z;
				}

				// jacobians
				detJ[j] = (dxr ^ dxs).norm();

				// integration weights
				w[j] = se.GaussWeights()[j];
			}

			// loop over all integration points
			for (int j=0; j<nint; ++j)
			{
				// get integration point data
				FEFacetSlidingSurface::Data& pt = ss.m_Data[i][j];

				// get the master element
				FESurfaceElement* pme = pt.m_pme;
				if (pme)
				{
					FESurfaceElement& me = *pme;

					int nmeln = me.Nodes();
					ms.UnpackLM(me, mLM);

					// calculate degrees of freedom
					int ndof = 3*(nseln + nmeln);

					// build the LM vector
					LM.resize(ndof);
					for (int k=0; k<nseln; ++k)
					{
						LM[3*k  ] = sLM[3*k  ];
						LM[3*k+1] = sLM[3*k+1];
						LM[3*k+2] = sLM[3*k+2];
					}

					for (int k=0; k<nmeln; ++k)
					{
						LM[3*(k+nseln)  ] = mLM[3*k  ];
						LM[3*(k+nseln)+1] = mLM[3*k+1];
						LM[3*(k+nseln)+2] = mLM[3*k+2];
					}

					// build the en vector
					en.resize(nseln+nmeln);
					for (int k=0; k<nseln; ++k) en[k] = se.m_node[k];
					for (int k=0; k<nmeln; ++k) en[k+nseln] = me.m_node[k];

					// calculate shape functions
					Hs = se.H(j);
					double r = pt.m_rs[0];
					double s = pt.m_rs[1];
					me.shape_fnc(Hm, r, s);

					// get normal vector
					vec3d nu = pt.m_nu;

					// gap function
					double g = pt.m_gap;

					// when the node is on the surface, the gap value
					// can flip-flop between positive and negative.
					if (fabs(g)<1e-20) g = 0;
					
					// lagrange multiplier
					double Lm = pt.m_Lm;

					// penalty value
					double eps = m_epsn*pt.m_eps;

					// contact traction
					double tn = Lm + eps*g;
					tn = MBRACKET(tn);

					double dtn = eps*HEAVYSIDE(Lm + eps*g);

					// define buffer layer for penalty insertion
					// TODO: I don't think this does anything since dtn cannot < 0
					if ((dtn < 1e-7) && (g < 0) && (dxtol != 0))
					{
						if (dxtol < 0) dtn = eps*exp(-g/dxtol);
						else if (-g<=dxtol) dtn = eps*(1 + g/dxtol);
					}

					// calculate the N-vector
					for (int k=0; k<nseln; ++k)
					{
						N[3*k  ] = Hs[k]*nu.x;
						N[3*k+1] = Hs[k]*nu.y;
						N[3*k+2] = Hs[k]*nu.z;
					}

					for (int k=0; k<nmeln; ++k)
					{
						N[3*(k+nseln)  ] = -Hm[k]*nu.x;
						N[3*(k+nseln)+1] = -Hm[k]*nu.y;
						N[3*(k+nseln)+2] = -Hm[k]*nu.z;
					}

					// --- N O R M A L   S T I F F N E S S ---

					// create the stiffness matrix
					ke.resize(ndof, ndof);

					// add the first order term (= D(tn)*dg )
					for (int k=0; k<ndof; ++k)
						for (int l=0; l<ndof; ++l) ke[k][l] = dtn*N[k]*N[l]*detJ[j]*w[j];

					// add the higher order terms (= tn*D(dg) )
					if (knmult > 0)
					{
						// calculate the master shape fncs derivatives
						me.shape_deriv(Hmr, Hms, r, s);

						// get the master nodes
						vec3d rt[MN];
						for (int k=0; k<nmeln; ++k) rt[k] = ms.GetMesh()->Node(me.m_node[k]).m_rt;

						// get the tangent vectors
						vec3d tau1(0,0,0), tau2(0,0,0);
						for (int k=0; k<nmeln; ++k)
						{
							tau1.x += Hmr[k]*rt[k].x;
							tau1.y += Hmr[k]*rt[k].y;
							tau1.z += Hmr[k]*rt[k].z;
		
							tau2.x += Hms[k]*rt[k].x;
							tau2.y += Hms[k]*rt[k].y;
							tau2.z += Hms[k]*rt[k].z;
						}

						// set up the Ti vectors
						for (int k=0; k<nseln; ++k)
						{
							T1[k*3  ] = Hs[k]*tau1.x; T2[k*3  ] = Hs[k]*tau2.x;
							T1[k*3+1] = Hs[k]*tau1.y; T2[k*3+1] = Hs[k]*tau2.y;
							T1[k*3+2] = Hs[k]*tau1.z; T2[k*3+2] = Hs[k]*tau2.z;
						}

						for (int k=0; k<nmeln; ++k) 
						{
							T1[(k+nseln)*3  ] = -Hm[k]*tau1.x;
							T1[(k+nseln)*3+1] = -Hm[k]*tau1.y;
							T1[(k+nseln)*3+2] = -Hm[k]*tau1.z;

							T2[(k+nseln)*3  ] = -Hm[k]*tau2.x;
							T2[(k+nseln)*3+1] = -Hm[k]*tau2.y;
							T2[(k+nseln)*3+2] = -Hm[k]*tau2.z;
						}

						// set up the Ni vectors
						for (int k=0; k<nmeln; ++k) 
						{
							N1[(k+nseln)*3  ] = -Hmr[k]*nu.x;
							N1[(k+nseln)*3+1] = -Hmr[k]*nu.y;
							N1[(k+nseln)*3+2] = -Hmr[k]*nu.z;

							N2[(k+nseln)*3  ] = -Hms[k]*nu.x;
							N2[(k+nseln)*3+1] = -Hms[k]*nu.y;
							N2[(k+nseln)*3+2] = -Hms[k]*nu.z;
						}

						// calculate metric tensor
						mat2d M;
						M[0][0] = tau1*tau1; M[0][1] = tau1*tau2; 
						M[1][0] = tau2*tau1; M[1][1] = tau2*tau2; 

						// calculate reciprocal metric tensor
						mat2d Mi = M.inverse();

						// calculate curvature tensor
						double K[2][2] = {0};
						double Grr[MN];
						double Gss[MN];
						double Grs[MN];
						me.shape_deriv2(Grr, Grs, Gss, r, s);
						for (int k=0; k<nmeln; ++k)
						{
							K[0][0] += (nu*rt[k])*Grr[k];
							K[0][1] += (nu*rt[k])*Grs[k];
							K[1][0] += (nu*rt[k])*Grs[k];
							K[1][1] += (nu*rt[k])*Gss[k];
						}

						// setup A matrix A = M + gK
						double A[2][2];
						A[0][0] = M[0][0] + g*K[0][0];
						A[0][1] = M[0][1] + g*K[0][1];
						A[1][0] = M[1][0] + g*K[1][0];
						A[1][1] = M[1][1] + g*K[1][1];

						// calculate determinant of A
						double detA = A[0][0]*A[1][1] - A[0][1]*A[1][0];

						// setup Di vectors
						for (int k=0; k<ndof; ++k)
						{
							D1[k] = (1/detA)*(A[1][1]*(T1[k]+g*N1[k]) - A[0][1]*(T2[k] + g*N2[k]));
							D2[k] = (1/detA)*(A[0][0]*(T2[k]+g*N2[k]) - A[0][1]*(T1[k] + g*N1[k]));
						}

						// setup Nbi vectors
						for (int k=0; k<ndof; ++k)
						{
							Nb1[k] = N1[k] - K[0][1]*D2[k];
							Nb2[k] = N2[k] - K[0][1]*D1[k];
						}

						// add it to the stiffness
						double sum;
						for (int k=0; k<ndof; ++k)
							for (int l=0; l<ndof; ++l)
							{
								sum = Mi[0][0]*Nb1[k]*Nb1[l]+Mi[0][1]*(Nb1[k]*Nb2[l]+Nb2[k]*Nb1[l])+Mi[1][1]*Nb2[k]*Nb2[l];
								sum *= g;
								sum -= D1[k]*N1[l]+D2[k]*N2[l]+N1[k]*D1[l]+N2[k]*D2[l];
								sum += K[0][1]*(D1[k]*D2[l]+D2[k]*D1[l]);
								sum *= tn*knmult;

								ke[k][l] += sum*detJ[j]*w[j];
							}
					}

					// assemble the global residual
					psolver->AssembleStiffness(en, LM, ke);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEFacet2FacetSliding::UpdateContactPressures()
{
	int npass = (m_btwo_pass?2:1);
	for (int np=0; np<npass; ++np)
	{
		FEFacetSlidingSurface& ss = (np == 0? m_ss : m_ms);
		FEFacetSlidingSurface& ms = (np == 0? m_ms : m_ss);
		
		// loop over all elements of the primary surface
		for (int n=0; n<ss.Elements(); ++n)
		{
			FESurfaceElement& el = ss.Element(n);
			int nint = el.GaussPoints();
			
			// get the normal tractions at the integration points
			double gap, eps;
			for (int i=0; i<nint; ++i) 
			{
				FEFacetSlidingSurface::Data& pt = ss.m_Data[n][i];

				gap = pt.m_gap;
				eps = m_epsn*pt.m_eps;
				pt.m_Ln = MBRACKET(pt.m_Lm + eps*gap);
				FESurfaceElement* pme = pt.m_pme;
				if (m_btwo_pass && pme)
				{
					// get master element data
					vector<FEFacetSlidingSurface::Data>& mdv = ms.m_Data[pme->m_lid];
					int mint = pme->GaussPoints();
					double ti[FEElement::MAX_NODES];
					for (int j=0; j<mint; ++j) 
					{
						FEFacetSlidingSurface::Data& md = mdv[j];
						gap = md.m_gap;
						eps = m_epsn*md.m_eps;
						ti[j] = MBRACKET(md.m_Lm + m_epsn*md.m_eps*md.m_gap);
					}
					// project the data to the nodes
					double tn[FEElement::MAX_NODES];
					pme->project_to_nodes(ti, tn);
					// now evaluate the traction at the intersection point
					double Ln = pme->eval(tn, pt.m_rs[0], pt.m_rs[1]);
					pt.m_Ln += MBRACKET(Ln);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
bool FEFacet2FacetSliding::Augment(int naug, const FETimeInfo& tp)
{
	// make sure we need to augment
	if (!m_blaugon) return true;

	bool bconv = true;

	// --- c a l c u l a t e   i n i t i a l   n o r m s ---
	// a. normal component
	int NS = (int) m_ss.m_Data.size();
	int NM = (int) m_ms.m_Data.size();

	double normL0 = 0;
	for (int i=0; i<NS; ++i)
	{
		vector<FEFacetSlidingSurface::Data>& sd = m_ss.m_Data[i];
		for (int j=0; j<(int)sd.size(); ++j)
		{
			FEFacetSlidingSurface::Data& ds = sd[j];
			normL0 += ds.m_Lm*ds.m_Lm;
		}
	}
	for (int i=0; i<NM; ++i)
	{
		vector<FEFacetSlidingSurface::Data>& md = m_ms.m_Data[i];
		for (int j=0; j<(int)md.size(); ++j)
		{
			FEFacetSlidingSurface::Data& dm = md[j];
			normL0 += dm.m_Lm*dm.m_Lm;
		}
	}
	normL0 = sqrt(normL0);

	// --- c a l c u l a t e   c u r r e n t   n o r m s ---
	// b. normal component
	double normL1 = 0;	// force norm
	double normg1 = 0;	// gap norm
	int N = 0;
    double Ln;
    for (int i=0; i<m_ss.Elements(); ++i) {
        FESurfaceElement& el = m_ss.Element(i);
        vec3d tn[FEElement::MAX_INTPOINTS];
        if (m_bsmaug) m_ss.GetGPSurfaceTraction(i, tn);
        for (int j=0; j<el.GaussPoints(); ++j) {
            FEFacetSlidingSurface::Data& data = m_ss.m_Data[i][j];
            // update Lagrange multipliers on slave surface
            if (m_bsmaug) {
                // replace this multiplier with a smoother version
                Ln = -(tn[j]*data.m_nu);
                data.m_Lm = MBRACKET(Ln);
                if (m_btwo_pass) data.m_Lm /= 2;
            }
            else {
                double eps = m_epsn*data.m_eps;
                Ln = data.m_Lm + eps*data.m_gap;
                data.m_Lm = MBRACKET(Ln);
            }
            
            normL1 += data.m_Lm*data.m_Lm;
            
            if (data.m_gap > 0)
            {
                normg1 += data.m_gap*data.m_gap;
                ++N;
            }
        }
    }
    
    for (int i=0; i<m_ms.Elements(); ++i) {
        FESurfaceElement& el = m_ms.Element(i);
        vec3d tn[FEElement::MAX_INTPOINTS];
        if (m_bsmaug) m_ms.GetGPSurfaceTraction(i, tn);
        for (int j=0; j<el.GaussPoints(); ++j) {
            FEFacetSlidingSurface::Data& data = m_ms.m_Data[i][j];
            // update Lagrange multipliers on master surface
            if (m_bsmaug) {
                // replace this multiplier with a smoother version
                Ln = -(tn[j]*data.m_nu);
                data.m_Lm = MBRACKET(Ln);
                if (m_btwo_pass) data.m_Lm /= 2;
            }
            else {
                double eps = m_epsn*data.m_eps;
                Ln = data.m_Lm + eps*data.m_gap;
                data.m_Lm = MBRACKET(Ln);
            }
            
            normL1 += data.m_Lm*data.m_Lm;
            
            if (data.m_gap > 0)
            {
                normg1 += data.m_gap*data.m_gap;
                ++N;
            }
        }
    }
    
	if (N == 0) N=1;

	normL1 = sqrt(normL1);
	normg1 = sqrt(normg1 / N);

	if (naug == 0) m_normg0 = 0;

	// calculate and print convergence norms
	double lnorm = 0, gnorm = 0;
	if (normL1 != 0) lnorm = fabs(normL1 - normL0)/normL1; else lnorm = fabs(normL1 - normL0);
//	if (normg1 != 0) gnorm = fabs(normg1 - m_normg0)/normg1; else gnorm = fabs(normg1 - m_normg0);
	gnorm = normg1;

	felog.printf(" sliding interface # %d\n", GetID());
	felog.printf("                        CURRENT        REQUIRED\n");
	felog.printf("    normal force : %15le", lnorm);
	if (m_atol > 0) felog.printf("%15le\n", m_atol); else felog.printf("       ***\n");
	felog.printf("    gap function : %15le", gnorm);
	if (m_gtol > 0) felog.printf("%15le\n", m_gtol); else felog.printf("       ***\n");

	// check convergence
	bconv = true;
	if ((m_atol > 0) && (lnorm > m_atol)) bconv = false;
	if ((m_gtol > 0) && (gnorm > m_gtol)) bconv = false;
	if (m_naugmin > naug) bconv = false;
	if (m_naugmax <= naug) bconv = true;
		
	if (bconv == false)
	{
		// we did not converge so update multipliers
		for (int i=0; i<NS; ++i)
		{
			vector<FEFacetSlidingSurface::Data>& sd = m_ss.m_Data[i];
			for (int j=0; j<(int)sd.size(); ++j)
			{
				FEFacetSlidingSurface::Data& ds = sd[j];

				// penalty value
				double eps = m_epsn*ds.m_eps;

				// update Lagrange multipliers
				double Ln = ds.m_Lm + eps*ds.m_gap;
				ds.m_Lm = MBRACKET(Ln);
			}
		}	

		for (int i=0; i<NM; ++i)
		{
			vector<FEFacetSlidingSurface::Data>& md = m_ms.m_Data[i];
			for (int j=0; j<(int)md.size(); ++j)
			{
				FEFacetSlidingSurface::Data& dm = md[j];

				// penalty value
				double eps = m_epsn*dm.m_eps;

				// update Lagrange multipliers
				double Ln = dm.m_Lm + eps*dm.m_gap;
				dm.m_Lm = MBRACKET(Ln);
			}
		}
	}

	// store the last gap norm
	m_normg0 = normg1;

	return bconv;
}

//-----------------------------------------------------------------------------
void FEFacet2FacetSliding::Serialize(DumpStream &ar)
{
	// store contact data
	FEContactInterface::Serialize(ar);

	// store contact surface data
	m_ms.Serialize(ar);
	m_ss.Serialize(ar);
}
