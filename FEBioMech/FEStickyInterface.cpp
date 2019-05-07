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
#include "FEStickyInterface.h"
#include "FECore/FEModel.h"
#include "FECore/FEClosestPointProjection.h"
#include "FECore/FEGlobalMatrix.h"
#include "FECore/log.h"

//-----------------------------------------------------------------------------
// Define sliding interface parameters
BEGIN_PARAMETER_LIST(FEStickyInterface, FEContactInterface)
	ADD_PARAMETER(m_blaugon, FE_PARAM_BOOL  , "laugon"          ); 
	ADD_PARAMETER(m_atol   , FE_PARAM_DOUBLE, "tolerance"       );
	ADD_PARAMETER(m_eps    , FE_PARAM_DOUBLE, "penalty"         );
	ADD_PARAMETER(m_naugmin, FE_PARAM_INT   , "minaug"          );
	ADD_PARAMETER(m_naugmax, FE_PARAM_INT   , "maxaug"          );
	ADD_PARAMETER(m_stol   , FE_PARAM_DOUBLE, "search_tolerance");
	ADD_PARAMETER(m_tmax   , FE_PARAM_DOUBLE, "max_traction"    );
	ADD_PARAMETER(m_snap   , FE_PARAM_DOUBLE, "snap_tol"        );
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
//! Creates a surface for use with a sliding interface. All surface data
//! structures are allocated.
//! Note that it is assumed that the element array is already created
//! and initialized.

bool FEStickySurface::Init()
{
	// always intialize base class first!
	if (FEContactSurface::Init() == false) return false;

	// get the number of nodes
	int nn = Nodes();

	// allocate other surface data
	m_Node.resize(nn);

	return true;
}

//-----------------------------------------------------------------------------
void FEStickySurface::Serialize(DumpStream &ar)
{
	FEContactSurface::Serialize(ar);
	if (ar.IsShallow())
	{
		if (ar.IsSaving())
		{
			for (int i=0; i<(int)m_Node.size(); ++i)
			{
				NODE& n = m_Node[i];
				ar << n.gap << n.rs << n.Lm;
			}
		}
		else
		{
			for (int i=0; i<(int)m_Node.size(); ++i)
			{
				NODE& n = m_Node[i];
				ar >> n.gap >> n.rs >> n.Lm;
			}
		}
	}
	else
	{
		if (ar.IsSaving())
		{
			for (int i=0; i<(int)m_Node.size(); ++i)
			{
				NODE& n = m_Node[i];
				ar << n.gap << n.rs << n.Lm;
			}
		}
		else
		{
			for (int i=0; i<(int)m_Node.size(); ++i)
			{
				NODE& n = m_Node[i];
				ar >> n.gap >> n.rs >> n.Lm;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEStickySurface::GetContactGap(int nface, double& pg)
{
    FESurfaceElement& el = Element(nface);
    int ne = el.Nodes();
    pg = 0;
    for (int k=0; k<ne; ++k) pg += m_Node[el.m_lnode[k]].gap.norm();
    pg /= ne;
}

//-----------------------------------------------------------------------------
void FEStickySurface::GetContactPressure(int nface, double& pg)
{
    FESurfaceElement& el = Element(nface);
    int ne = el.Nodes();
    pg = 0;
    for (int k=0; k<ne; ++k) pg += m_Node[el.m_lnode[k]].tn.norm();
    pg /= ne;
}

//-----------------------------------------------------------------------------
void FEStickySurface::GetContactTraction(int nface, vec3d& pt)
{
    FESurfaceElement& el = Element(nface);
    int ne = el.Nodes();
    pt = vec3d(0,0,0);
    for (int k=0; k<ne; ++k) pt += m_Node[el.m_lnode[k]].tn;
    pt /= ne;
}

//-----------------------------------------------------------------------------
void FEStickySurface::GetNodalContactGap(int nface, double* gn)
{
	FESurfaceElement& f = Element(nface);
	int ne = f.Nodes();
	for (int j= 0; j< ne; ++j) gn[j] = m_Node[f.m_lnode[j]].gap.norm();
}

//-----------------------------------------------------------------------------
void FEStickySurface::GetNodalContactPressure(int nface, double* pn)
{
	FESurfaceElement& f = Element(nface);
	int ne = f.Nodes();
	for (int j= 0; j< ne; ++j) pn[j] = m_Node[f.m_lnode[j]].tn.norm();
}

//-----------------------------------------------------------------------------
void FEStickySurface::GetNodalContactTraction(int nface, vec3d* tn)
{
	FESurfaceElement& f = Element(nface);
	int ne = f.Nodes();
	for (int j= 0; j< ne; ++j) tn[j] = m_Node[f.m_lnode[j]].tn;
}

//=============================================================================
//
//		F E S T I C K Y I N T E R F A C E
//
//=============================================================================

//-----------------------------------------------------------------------------
//! Constructor. Initialize default values.
FEStickyInterface::FEStickyInterface(FEModel* pfem) : FEContactInterface(pfem), ss(pfem), ms(pfem)
{
	static int count = 1;
	SetID(count++);

	// define sibling relationships
	ss.SetSibling(&ms);
	ms.SetSibling(&ss);

	// initial parameter values
	m_blaugon = false;
	m_atol = 0.01;
	m_eps = 1.0;
	m_stol = 0.0001;
	m_naugmin = 0;
	m_naugmax = 10;
	m_tmax = 0.0;
	m_snap = 0.0;
}

//-----------------------------------------------------------------------------
//! Initialization. This function intializes the surfaces data and projects the
//! slave surface onto the master surface.
//! 
bool FEStickyInterface::Init()
{
	// create the surfaces
	if (ss.Init() == false) return false;
	if (ms.Init() == false) return false;

	return true;
}

//-----------------------------------------------------------------------------
//! build the matrix profile for use in the stiffness matrix
void FEStickyInterface::BuildMatrixProfile(FEGlobalMatrix& K)
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

	const int LMSIZE = 6*(FEElement::MAX_NODES+1);
	vector<int> lm(LMSIZE);

	for (int j=0; j<ss.Nodes(); ++j)
	{
		FEStickySurface::NODE& snj = ss.m_Node[j];
		FESurfaceElement* pe = snj.pme;
		if (pe != 0)
		{
			FESurfaceElement& me = *pe;
			int* en = &me.m_node[0];

			int n = me.Nodes();
			lm.assign(LMSIZE, -1);

			lm[0] = ss.Node(j).m_ID[dof_X];
			lm[1] = ss.Node(j).m_ID[dof_Y];
			lm[2] = ss.Node(j).m_ID[dof_Z];
			lm[3] = ss.Node(j).m_ID[dof_RU];
			lm[4] = ss.Node(j).m_ID[dof_RV];
			lm[5] = ss.Node(j).m_ID[dof_RW];

			for (int k=0; k<n; ++k)
			{
				vector<int>& id = mesh.Node(en[k]).m_ID;
				lm[6*(k+1)  ] = id[dof_X];
				lm[6*(k+1)+1] = id[dof_Y];
				lm[6*(k+1)+2] = id[dof_Z];
				lm[6*(k+1)+3] = id[dof_RU];
				lm[6*(k+1)+4] = id[dof_RV];
				lm[6*(k+1)+5] = id[dof_RW];
			}

			K.build_add(lm);
		}
	}
}

//-----------------------------------------------------------------------------
//! Interface activation
void FEStickyInterface::Activate()
{
	// Don't forget to call base member!
	FEContactInterface::Activate();

	// project slave surface onto master surface
	ProjectSurface(ss, ms, false);
}

//-----------------------------------------------------------------------------
//! Update sticky interface data. This function re-evaluates the gaps between
//! the slave node and their projections onto the master surface.
//!
void FEStickyInterface::Update(int niter, const FETimeInfo& tp)
{
	// closest point projection method
	FEClosestPointProjection cpp(ms);
	cpp.HandleSpecialCases(true);
	cpp.SetTolerance(m_stol);
	cpp.Init();

	// get the mesh
	FEMesh& mesh = *ss.GetMesh();

	// flag used for contact searching algorithm
	bool binit = true;

	// loop over all slave nodes
	for (int i=0; i<ss.Nodes(); ++i)
	{
		FEStickySurface::NODE& sni = ss.m_Node[i];
		FESurfaceElement* pme = sni.pme;
		if (pme)
		{
			// get the current slave nodal position
			vec3d rt = ss.Node(i).m_rt;

			// get the natural coordinates of the slave projection
			// onto the master element
			double r = sni.rs[0];
			double s = sni.rs[1];

			// get the nodal coordinates
			int ne = pme->Nodes();
			vec3d y[FEElement::MAX_NODES];
			for (int l=0; l<ne; ++l) y[l] = mesh.Node( pme->m_node[l] ).m_rt;

			// calculate the slave node projection
			vec3d q = pme->eval(y, r, s);

			// calculate the gap function
			sni.gap = rt - q;

			// see if the max traction was exceded
			if (m_tmax > 0.0)
			{
				// get slave node contact force
				vec3d tc = sni.Lm + sni.gap*m_eps;

				// calculate the master normal
				vec3d nu = ms.SurfaceNormal(*sni.pme, sni.rs[0], sni.rs[1]);
				double t = nu*tc;
				if (t > m_tmax)
				{
					// detach this node
					sni.gap = vec3d(0,0,0);
					sni.pme = 0;
					sni.Lm = vec3d(0,0,0);
					sni.tn = vec3d(0,0,0);
				}
			}
		}
		else
		{
			// get the nodal position of this slave node
			FENode& node = ss.Node(i);
			vec3d x = node.m_rt;

			// find the master element
			vec3d q; vec2d rs;
			FESurfaceElement* pme = cpp.Project(x, q, rs);
			if (pme)
			{
				// calculate the master normal
				vec3d nu = ms.SurfaceNormal(*pme, rs[0], rs[1]);

				// calculate gap
				double d = nu*(q - x);

				// only allow contact after penetration
				if (d > -m_snap)
				{
					// calculate signed distance
					sni.gap = x - q;

					// store the master element
					sni.pme = pme;
					sni.rs[0] = rs[0];
					sni.rs[1] = rs[1];
				}
			}			
		}
	}
}

//-----------------------------------------------------------------------------
//! project surface

void FEStickyInterface::ProjectSurface(FEStickySurface& ss, FEStickySurface& ms, bool bmove)
{
	// closest point projection method
	FEClosestPointProjection cpp(ms);
	cpp.HandleSpecialCases(true);
	cpp.SetTolerance(m_stol);
	cpp.Init();

	// loop over all slave nodes
	for (int i=0; i<ss.Nodes(); ++i)
	{
		// get the next node
		FENode& node = ss.Node(i);
		FEStickySurface::NODE& sni = ss.m_Node[i];

		// assume we won't find a projection
		sni.pme = 0;

		// get the nodal position of this slave node
		vec3d x = node.m_rt;

		// find the master element
		vec3d q; vec2d rs;
		FESurfaceElement* pme = cpp.Project(x, q, rs);
		if (pme)
		{
			// calculate the master normal
			vec3d nu = ms.SurfaceNormal(*pme, rs[0], rs[1]);

			// calculate gap
			double d = nu*(q - x);

			// only allow contact after penetration
			if (d > 0)
			{
				// calculate signed distance
				sni.gap = x - q;

				// store the master element
				sni.pme = pme;
				sni.rs[0] = rs[0];
				sni.rs[1] = rs[1];


				// move the node if necessary
				if (bmove && (sni.gap.norm()>0))
				{
					node.m_r0 = node.m_rt = q;
					sni.gap = vec3d(0,0,0);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
//! This function calculates the contact forces for a tied interface.

void FEStickyInterface::Residual(FEGlobalVector& R, const FETimeInfo& tp)
{
	// shape function values
	double N[FEElement::MAX_NODES];

	// element contact force vector
	vector<double> fe;

	// the lm array for this force vector
	vector<int> lm;

	// the en array
	vector<int> en;

	vector<int> sLM;
	vector<int> mLM;

	// loop over all slave facets
	const int NE = ss.Elements();
	for (int j=0; j<NE; ++j)
	{
		// get the slave element
		FESurfaceElement& sel = ss.Element(j);

		// get the element's LM vector
		ss.UnpackLM(sel, sLM);

		int nseln = sel.Nodes();

		double* w = sel.GaussWeights();

		// loop over slave element nodes (which are the integration points as well)
		for (int n=0; n<nseln; ++n)
		{
			int m = sel.m_lnode[n];

			FEStickySurface::NODE& sm = ss.m_Node[m];

			// see if this node's constraint is active
			// that is, if it has a master element associated with it
			// TODO: is this a good way to test for an active constraint
			// The rigid wall criteria seems to work much better.
			if (sm.pme != 0)
			{
				// calculate jacobian
				double detJ = ss.jac0(sel, n);

				// get slave node contact force
				vec3d tc = sm.Lm + sm.gap*m_eps;

				// cap it
				if (m_tmax > 0.0)
				{
					// calculate the master normal
					vec3d nu = ms.SurfaceNormal(*sm.pme, sm.rs[0], sm.rs[1]);
					double t = nu*tc;
					if (t > m_tmax) tc = vec3d(0,0,0);
				}

				// store traction
				sm.tn = tc;

				// get the master element
				FESurfaceElement& mel = *sm.pme;
				ms.UnpackLM(mel, mLM);

				int nmeln = mel.Nodes();

				// isoparametric coordinates of the projected slave node
				// onto the master element
				double r = sm.rs[0];
				double s = sm.rs[1];

				// get the master shape function values at this slave node
				mel.shape_fnc(N, r, s);

				// calculate force vector
				fe.resize(3*(nmeln+1));
				fe[0] = -detJ*w[n]*tc.x;
				fe[1] = -detJ*w[n]*tc.y;
				fe[2] = -detJ*w[n]*tc.z;
				for (int l=0; l<nmeln; ++l)
				{	
					fe[3*(l+1)  ] = detJ*w[n]*tc.x*N[l];
					fe[3*(l+1)+1] = detJ*w[n]*tc.y*N[l];
					fe[3*(l+1)+2] = detJ*w[n]*tc.z*N[l];
				}
	
				// fill the lm array
				lm.resize(3*(nmeln+1));
				lm[0] = sLM[n*3  ];
				lm[1] = sLM[n*3+1];
				lm[2] = sLM[n*3+2];

				for (int l=0; l<nmeln; ++l)
				{
					lm[3*(l+1)  ] = mLM[l*3  ];
					lm[3*(l+1)+1] = mLM[l*3+1];
					lm[3*(l+1)+2] = mLM[l*3+2];
				}

				// fill the en array
				en.resize(nmeln+1);
				en[0] = sel.m_node[n];
				for (int l=0; l<nmeln; ++l) en[l+1] = mel.m_node[l];

				// assemble into global force vector
				R.Assemble(en, lm, fe);
			}
		}
	}
}

//-----------------------------------------------------------------------------
//! Calculate the stiffness matrix contribution.
void FEStickyInterface::StiffnessMatrix(FESolver* psolver, const FETimeInfo& tp)
{
	vector<int> sLM, mLM, lm, en;
	const int MN = FEElement::MAX_NODES;
	matrix ke;

	// shape functions
	double H[MN];

	// loop over all slave elements
	const int NE = ss.Elements();
	for (int i=0; i<NE; ++i)
	{
		// get the next element
		FESurfaceElement& se = ss.Element(i);
		int nseln = se.Nodes();

		// get the element's LM vector
		ss.UnpackLM(se, sLM);

		double* w = se.GaussWeights();

		// loop over all integration points (that is nodes)
		for (int n=0; n<nseln; ++n)
		{
			int m = se.m_lnode[n];

			FEStickySurface::NODE& sm = ss.m_Node[m];

			// get the master element
			FESurfaceElement* pme = sm.pme;
			if (pme)
			{
				// get the master element
				FESurfaceElement& me = *pme;
				int nmeln = me.Nodes();
				ms.UnpackLM(me, mLM);

				// calculate jacobian
				double detJ = ss.jac0(se, n);

				// slave node natural coordinates in master element
				double r = sm.rs[0];
				double s = sm.rs[1];

				// get the master shape function values at this slave node
				me.shape_fnc(H, r, s);

				// number of degrees of freedom
				int ndof = 3*(1 + nmeln);

				// fill stiffness matrix
				ke.resize(ndof, ndof); ke.zero();
				ke[0][0] = w[n]*detJ*m_eps;
				ke[1][1] = w[n]*detJ*m_eps;
				ke[2][2] = w[n]*detJ*m_eps;
				for (int k=0; k<nmeln; ++k)
				{
					ke[0][3+3*k  ] = -w[n]*detJ*m_eps*H[k];
					ke[1][3+3*k+1] = -w[n]*detJ*m_eps*H[k];
					ke[2][3+3*k+2] = -w[n]*detJ*m_eps*H[k];

					ke[3+3*k  ][0] = -w[n]*detJ*m_eps*H[k];
					ke[3+3*k+1][1] = -w[n]*detJ*m_eps*H[k];
					ke[3+3*k+2][2] = -w[n]*detJ*m_eps*H[k];
				}
				for (int k=0; k<nmeln; ++k)
					for (int l=0; l<nmeln; ++l)
					{
						ke[3+3*k  ][3+3*l  ] = w[n]*detJ*m_eps*H[k]*H[l];
						ke[3+3*k+1][3+3*l+1] = w[n]*detJ*m_eps*H[k]*H[l];
						ke[3+3*k+2][3+3*l+2] = w[n]*detJ*m_eps*H[k]*H[l];
					}

				// create lm array
				lm.resize(3*(1+nmeln));
				lm[0] = sLM[n*3  ];
				lm[1] = sLM[n*3+1];
				lm[2] = sLM[n*3+2];

				for (int k=0; k<nmeln; ++k)
				{
					lm[3*(k+1)  ] = mLM[k*3  ];
					lm[3*(k+1)+1] = mLM[k*3+1];
					lm[3*(k+1)+2] = mLM[k*3+2];
				}

				// create the en array
				en.resize(nmeln+1);
				en[0] = se.m_node[n];
				for (int k=0; k<nmeln; ++k) en[k+1] = me.m_node[k];
						
				// assemble stiffness matrix
				psolver->AssembleStiffness(en, lm, ke);
			}
		}
	}
}

//-----------------------------------------------------------------------------
//! Do an augmentation.
bool FEStickyInterface::Augment(int naug, const FETimeInfo& tp)
{
	// make sure we need to augment
	if (!m_blaugon) return true;

	int i;

	// calculate initial norms
	double normL0 = 0;
	for (i=0; i<ss.Nodes(); ++i)
	{
		FEStickySurface::NODE& si = ss.m_Node[i];
		vec3d lm = si.Lm;
		normL0 += lm*lm;
	}
	normL0 = sqrt(normL0);

	// update Lagrange multipliers and calculate current norms
	double normL1 = 0;
	double normgc = 0;
	int N = 0;
	for (i=0; i<ss.Nodes(); ++i)
	{
		FEStickySurface::NODE& si = ss.m_Node[i];

		vec3d lm = si.Lm + si.gap*m_eps;

		normL1 += lm*lm;
		if (si.pme != 0)
		{
			double g = si.gap.norm();
			normgc += g*g;
			++N;
		}
	}	
	if (N == 0) N=1;

	normL1 = sqrt(normL1);
	normgc = sqrt(normgc / N);

	// check convergence of constraints
	felog.printf(" tied interface # %d\n", GetID());
	felog.printf("                        CURRENT        REQUIRED\n");
	double pctn = 0;
	if (fabs(normL1) > 1e-10) pctn = fabs((normL1 - normL0)/normL1);
	felog.printf("    normal force : %15le %15le\n", pctn, m_atol);
	felog.printf("    gap function : %15le       ***\n", normgc);
		
	// check convergence
	bool bconv = true;
	if (pctn >= m_atol) bconv = false;
	if (naug < m_naugmin ) bconv = false;
	if (naug >= m_naugmax) bconv = true;

	if (bconv == false) 
	{
		for (i=0; i<ss.Nodes(); ++i)
		{
			FEStickySurface::NODE& si = ss.m_Node[i];
			// update Lagrange multipliers
			si.Lm = si.Lm + si.gap*m_eps;
		}	
	}

	return bconv;
}

//-----------------------------------------------------------------------------
//! Serialize the data to the archive.
void FEStickyInterface::Serialize(DumpStream &ar)
{
	// store contact data
	FEContactInterface::Serialize(ar);

	// store contact surface data
	ms.Serialize(ar);
	ss.Serialize(ar);
}
