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
#include "FETiedElasticInterface.h"
#include "FECore/FEModel.h"
#include "FECore/FEAnalysis.h"
#include "FECore/FENormalProjection.h"
#include "FECore/log.h"

//-----------------------------------------------------------------------------
// Define sliding interface parameters
BEGIN_PARAMETER_LIST(FETiedElasticInterface, FEContactInterface)
ADD_PARAMETER(m_blaugon  , FE_PARAM_BOOL  , "laugon"             );
ADD_PARAMETER(m_atol     , FE_PARAM_DOUBLE, "tolerance"          );
ADD_PARAMETER(m_gtol     , FE_PARAM_DOUBLE, "gaptol"             );
ADD_PARAMETER(m_epsn     , FE_PARAM_DOUBLE, "penalty"            );
ADD_PARAMETER(m_bautopen , FE_PARAM_BOOL  , "auto_penalty"       );
ADD_PARAMETER(m_btwo_pass, FE_PARAM_BOOL  , "two_pass"           );
ADD_PARAMETER(m_knmult   , FE_PARAM_INT   , "knmult"             );
ADD_PARAMETER(m_stol     , FE_PARAM_DOUBLE, "search_tol"         );
ADD_PARAMETER(m_bsymm    , FE_PARAM_BOOL  , "symmetric_stiffness");
ADD_PARAMETER(m_srad     , FE_PARAM_DOUBLE, "search_radius"      );
ADD_PARAMETER(m_naugmin  , FE_PARAM_INT   , "minaug"             );
ADD_PARAMETER(m_naugmax  , FE_PARAM_INT   , "maxaug"             );
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FETiedElasticSurface::Data::Data()
{
    m_Gap = vec3d(0,0,0);
    m_dg = vec3d(0,0,0);
    m_nu = vec3d(0,0,0);
    m_rs = vec2d(0,0);
    m_Lmd = vec3d(0,0,0);
    m_tr = vec3d(0,0,0);
    m_epsn = 1.0;
    m_pme  = (FESurfaceElement*)0;
}

//-----------------------------------------------------------------------------
// FETiedElasticSurface
//-----------------------------------------------------------------------------

FETiedElasticSurface::FETiedElasticSurface(FEModel* pfem) : FEContactSurface(pfem)
{
    m_pfem = pfem;
}

//-----------------------------------------------------------------------------
bool FETiedElasticSurface::Init()
{
    // initialize surface data first
    if (FEContactSurface::Init() == false) return false;
    
    // allocate data structures
    int NE = Elements();
    m_Data.resize(NE);
    for (int i=0; i<NE; ++i)
    {
        FESurfaceElement& el = Element(i);
        int nint = el.GaussPoints();
        m_Data[i].resize(nint);
    }
    
    // allocate node normals
    m_nn.assign(Nodes(), vec3d(0,0,0));
    
    return true;
}

//-----------------------------------------------------------------------------
//! This function calculates the node normal. Due to the piecewise continuity
//! of the surface elements this normal is not uniquely defined so in order to
//! obtain a unique normal the normal is averaged for each node over all the
//! element normals at the node

void FETiedElasticSurface::UpdateNodeNormals()
{
    int N = Nodes(), i, j, ne, jp1, jm1;
    vec3d y[FEElement::MAX_NODES], n;
    
    // zero nodal normals
    zero(m_nn);
    
    // loop over all elements
    for (i=0; i<Elements(); ++i)
    {
        FESurfaceElement& el = Element(i);
        ne = el.Nodes();
        
        // get the nodal coordinates
        for (j=0; j<ne; ++j) y[j] = Node(el.m_lnode[j]).m_rt;
        
        // calculate the normals
        for (j=0; j<ne; ++j)
        {
            jp1 = (j+1)%ne;
            jm1 = (j+ne-1)%ne;
            n = (y[jp1] - y[j]) ^ (y[jm1] - y[j]);
            m_nn[el.m_lnode[j]] += n;
        }
    }
    
    // normalize all vectors
    for (i=0; i<N; ++i) m_nn[i].unit();
}

//-----------------------------------------------------------------------------
void FETiedElasticSurface::Serialize(DumpStream& ar)
{
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
                    ar << d.m_Lmd;
                    ar << d.m_Gap;
                    ar << d.m_dg;
                    ar << d.m_tr;
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
                    ar >> d.m_Lmd;
                    ar >> d.m_Gap;
                    ar >> d.m_dg;
                    ar >> d.m_tr;
                }
            }
        }
    }
    else
    {
        // Next, we can serialize the base-class data
        FEContactSurface::Serialize(ar);
        
        // And finally, we serialize the surface data
        if (ar.IsSaving())
        {
            for (int i=0; i<(int) m_Data.size(); ++i)
            {
                vector<Data>& di = m_Data[i];
                int nint = (int) di.size();
                for (int j=0; j<nint; ++j)
                {
                    Data& d = di[j];
                    ar << d.m_Gap;
                    ar << d.m_dg;
                    ar << d.m_nu;
                    ar << d.m_rs;
                    ar << d.m_Lmd;
                    ar << d.m_epsn;
                    ar << d.m_tr;
                }
            }
            ar << m_nn;
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
                    ar >> d.m_Gap;
                    ar >> d.m_dg;
                    ar >> d.m_nu;
                    ar >> d.m_rs;
                    ar >> d.m_Lmd;
                    ar >> d.m_epsn;
                    ar >> d.m_tr;
                }
            }
            ar >> m_nn;
        }
    }
}

//-----------------------------------------------------------------------------
void FETiedElasticSurface::GetVectorGap(int nface, vec3d& pg)
{
    FESurfaceElement& el = Element(nface);
    int ni = el.GaussPoints();
    pg = vec3d(0,0,0);
    for (int k=0; k<ni; ++k) pg += m_Data[nface][k].m_dg;
    pg /= ni;
}

//-----------------------------------------------------------------------------
void FETiedElasticSurface::GetContactTraction(int nface, vec3d& pt)
{
    FESurfaceElement& el = Element(nface);
    int ni = el.GaussPoints();
    pt = vec3d(0,0,0);
    for (int k=0; k<ni; ++k) pt += m_Data[nface][k].m_tr;
    pt /= ni;
}

//-----------------------------------------------------------------------------
// FETiedElasticInterface
//-----------------------------------------------------------------------------

FETiedElasticInterface::FETiedElasticInterface(FEModel* pfem) : FEContactInterface(pfem), m_ss(pfem), m_ms(pfem)
{
    static int count = 1;
    SetID(count++);
    
    // initial values
    m_knmult = 1;
    m_atol = 0.1;
    m_epsn = 1;
    m_btwo_pass = false;
    m_stol = 0.01;
    m_bsymm = true;
    m_srad = 1.0;
    m_gtol = -1;    // we use augmentation tolerance by default
    m_bautopen = false;
    
    m_naugmin = 0;
    m_naugmax = 10;
    
    m_ss.SetSibling(&m_ms);
    m_ms.SetSibling(&m_ss);
}

//-----------------------------------------------------------------------------

FETiedElasticInterface::~FETiedElasticInterface()
{
}

//-----------------------------------------------------------------------------
bool FETiedElasticInterface::Init()
{
    // initialize surface data
    if (m_ss.Init() == false) return false;
    if (m_ms.Init() == false) return false;
    
    return true;
}

//-----------------------------------------------------------------------------
//! build the matrix profile for use in the stiffness matrix
void FETiedElasticInterface::BuildMatrixProfile(FEGlobalMatrix& K)
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
    
    const int ndpn = 6;
    vector<int> lm(ndpn*FEElement::MAX_NODES*2);
    
    int npass = (m_btwo_pass?2:1);
    for (int np=0; np<npass; ++np)
    {
        FETiedElasticSurface& ss = (np == 0? m_ss : m_ms);
        
        int ni = 0, k, l;
        for (int j=0; j<ss.Elements(); ++j)
        {
            FESurfaceElement& se = ss.Element(j);
            int nint = se.GaussPoints();
            int* sn = &se.m_node[0];
            for (k=0; k<nint; ++k, ++ni)
            {
                FETiedElasticSurface::Data& pt = ss.m_Data[j][k];
                FESurfaceElement* pe = pt.m_pme;
                if (pe != 0)
                {
                    FESurfaceElement& me = *pe;
                    int* mn = &me.m_node[0];
                    
                    assign(lm, -1);
                    
                    int nseln = se.Nodes();
                    int nmeln = me.Nodes();
                    
                    for (l=0; l<nseln; ++l)
                    {
                        vector<int>& id = mesh.Node(sn[l]).m_ID;
                        lm[ndpn*l  ] = id[dof_X];
                        lm[ndpn*l+1] = id[dof_Y];
                        lm[ndpn*l+2] = id[dof_Z];
                        lm[ndpn*l+4] = id[dof_RU];
                        lm[ndpn*l+5] = id[dof_RV];
                        lm[ndpn*l+6] = id[dof_RW];
                    }
                    
                    for (l=0; l<nmeln; ++l)
                    {
                        vector<int>& id = mesh.Node(mn[l]).m_ID;
                        lm[ndpn*(l+nseln)  ] = id[dof_X];
                        lm[ndpn*(l+nseln)+1] = id[dof_Y];
                        lm[ndpn*(l+nseln)+2] = id[dof_Z];
                        lm[ndpn*(l+nseln)+4] = id[dof_RU];
                        lm[ndpn*(l+nseln)+5] = id[dof_RV];
                        lm[ndpn*(l+nseln)+6] = id[dof_RW];
                    }
                    
                    K.build_add(lm);
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
void FETiedElasticInterface::Activate()
{
    // don't forget to call the base class
    FEContactInterface::Activate();
    
    // calculate the penalty
    if (m_bautopen)
    {
        CalcAutoPenalty(m_ss);
        if (m_btwo_pass) CalcAutoPenalty(m_ms);
    }
    
    // project the surfaces onto each other
    // this will evaluate the gap functions in the reference configuration
    InitialProjection(m_ss, m_ms);
    if (m_btwo_pass) InitialProjection(m_ms, m_ss);
}

//-----------------------------------------------------------------------------
void FETiedElasticInterface::CalcAutoPenalty(FETiedElasticSurface& s)
{
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
            FETiedElasticSurface::Data& pt = s.m_Data[i][j];
            pt.m_epsn = eps;
        }
    }
}

//-----------------------------------------------------------------------------
// Perform initial projection between tied surfaces in reference configuration
void FETiedElasticInterface::InitialProjection(FETiedElasticSurface& ss, FETiedElasticSurface& ms)
{
    FESurfaceElement* pme;
    vec3d r, nu;
    double rs[2];
    
    // initialize projection data
    FENormalProjection np(ms);
    np.SetTolerance(m_stol);
    np.SetSearchRadius(m_srad);
    np.Init();
    
    // loop over all integration points
    int n = 0;
    for (int i=0; i<ss.Elements(); ++i)
    {
        FESurfaceElement& el = ss.Element(i);
        
        int nint = el.GaussPoints();
        
        for (int j=0; j<nint; ++j, ++n)
        {
            // calculate the global position of the integration point
            r = ss.Local2Global(el, j);
            
            // calculate the normal at this integration point
            nu = ss.SurfaceNormal(el, j);
            
            // find the intersection point with the master surface
            pme = np.Project2(r, nu, rs);
            
            FETiedElasticSurface::Data& pt = ss.m_Data[i][j];
            pt.m_pme = pme;
            pt.m_rs[0] = rs[0];
            pt.m_rs[1] = rs[1];
            if (pme)
            {
                // the node could potentially be in contact
                // find the global location of the intersection point
                vec3d q = ms.Local2Global(*pme, rs[0], rs[1]);
                
                // calculate the gap function
                pt.m_Gap = q - r;
            }
            else
            {
                // the node is not in contact
                pt.m_Gap = vec3d(0,0,0);
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Evaluate gap functions for position and fluid pressure
void FETiedElasticInterface::ProjectSurface(FETiedElasticSurface& ss, FETiedElasticSurface& ms)
{
    FESurfaceElement* pme;
    vec3d r;
    
    // loop over all integration points
    for (int i=0; i<ss.Elements(); ++i)
    {
        FESurfaceElement& el = ss.Element(i);
        
        int nint = el.GaussPoints();
        
        for (int j=0; j<nint; ++j)
        {
            FETiedElasticSurface::Data& pt = ss.m_Data[i][j];
            
            // calculate the global position of the integration point
            r = ss.Local2Global(el, j);
            
            // calculate the normal at this integration point
            pt.m_nu = ss.SurfaceNormal(el, j);
            
            // if this node is tied, evaluate gap functions
            pme = pt.m_pme;
            if (pme)
            {
                // find the global location of the intersection point
                vec3d q = ms.Local2Global(*pme, pt.m_rs[0], pt.m_rs[1]);
                
                // calculate the gap function
                vec3d g = q - r;
                pt.m_dg = g - pt.m_Gap;
                
            }
            else
            {
                // the node is not tied
                pt.m_dg = vec3d(0,0,0);
            }
        }
    }
}

//-----------------------------------------------------------------------------

void FETiedElasticInterface::Update(int niter, const FETimeInfo& tp)
{
    // project the surfaces onto each other
    // this will update the gap functions as well
    ProjectSurface(m_ss, m_ms);
    if (m_btwo_pass) ProjectSurface(m_ms, m_ss);
    
}

//-----------------------------------------------------------------------------
void FETiedElasticInterface::Residual(FEGlobalVector& R, const FETimeInfo& tp)
{
    int i, j, k;
    vector<int> sLM, mLM, LM, en;
    vector<double> fe;
    const int MN = FEElement::MAX_NODES;
    double detJ[MN], w[MN], *Hs, Hm[MN];
    double N[8*MN];
    
    // loop over the nr of passes
    int npass = (m_btwo_pass?2:1);
    for (int np=0; np<npass; ++np)
    {
        // get slave and master surface
        FETiedElasticSurface& ss = (np == 0? m_ss : m_ms);
        FETiedElasticSurface& ms = (np == 0? m_ms : m_ss);
        
        // loop over all slave elements
        for (i=0; i<ss.Elements(); ++i)
        {
            // get the surface element
            FESurfaceElement& se = ss.Element(i);
            
            // get the nr of nodes and integration points
            int nseln = se.Nodes();
            int nint = se.GaussPoints();
            
            // copy the LM vector; we'll need it later
            ss.UnpackLM(se, sLM);
            
            // we calculate all the metrics we need before we
            // calculate the nodal forces
            for (j=0; j<nint; ++j)
            {
                // get the base vectors
                vec3d g[2];
                ss.CoBaseVectors(se, j, g);
                
                // jacobians: J = |g0xg1|
                detJ[j] = (g[0] ^ g[1]).norm();
                
                // integration weights
                w[j] = se.GaussWeights()[j];
            }
            
            // loop over all integration points
            // note that we are integrating over the current surface
            for (j=0; j<nint; ++j)
            {
                FETiedElasticSurface::Data& pt = ss.m_Data[i][j];
                
                // get the master element
                FESurfaceElement* pme = pt.m_pme;
                if (pme)
                {
                    // get the master element
                    FESurfaceElement& me = *pme;
                    
                    // get the nr of master element nodes
                    int nmeln = me.Nodes();
                    
                    // copy LM vector
                    ms.UnpackLM(me, mLM);
                    
                    // calculate degrees of freedom
                    int ndof = 3*(nseln + nmeln);
                    
                    // build the LM vector
                    LM.resize(ndof);
                    for (k=0; k<nseln; ++k)
                    {
                        LM[3*k  ] = sLM[3*k  ];
                        LM[3*k+1] = sLM[3*k+1];
                        LM[3*k+2] = sLM[3*k+2];
                    }
                    
                    for (k=0; k<nmeln; ++k)
                    {
                        LM[3*(k+nseln)  ] = mLM[3*k  ];
                        LM[3*(k+nseln)+1] = mLM[3*k+1];
                        LM[3*(k+nseln)+2] = mLM[3*k+2];
                    }
                    
                    // build the en vector
                    en.resize(nseln+nmeln);
                    for (k=0; k<nseln; ++k) en[k      ] = se.m_node[k];
                    for (k=0; k<nmeln; ++k) en[k+nseln] = me.m_node[k];
                    
                    // get slave element shape functions
                    Hs = se.H(j);
                    
                    // get master element shape functions
                    double r = pt.m_rs[0];
                    double s = pt.m_rs[1];
                    me.shape_fnc(Hm, r, s);
                    
                    // gap function
                    vec3d dg = pt.m_dg;
                    
                    // lagrange multiplier
                    vec3d Lm = pt.m_Lmd;
                    
                    // penalty
                    double eps = m_epsn*pt.m_epsn;
                    
                    // contact traction
                    vec3d t = Lm + dg*eps;
                    pt.m_tr = t;
                    
                    // calculate the force vector
                    fe.resize(ndof);
                    zero(fe);
                    
                    for (k=0; k<nseln; ++k)
                    {
                        N[3*k  ] = Hs[k]*t.x;
                        N[3*k+1] = Hs[k]*t.y;
                        N[3*k+2] = Hs[k]*t.z;
                    }
                    
                    for (k=0; k<nmeln; ++k)
                    {
                        N[3*(k+nseln)  ] = -Hm[k]*t.x;
                        N[3*(k+nseln)+1] = -Hm[k]*t.y;
                        N[3*(k+nseln)+2] = -Hm[k]*t.z;
                    }
                    
                    for (k=0; k<ndof; ++k) fe[k] += N[k]*detJ[j]*w[j];
                    
                    // assemble the global residual
                    R.Assemble(en, LM, fe);
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
void FETiedElasticInterface::StiffnessMatrix(FESolver* psolver, const FETimeInfo& tp)
{
    int i, j, k, l;
    vector<int> sLM, mLM, LM, en;
    const int MN = FEElement::MAX_NODES;
    double detJ[MN], w[MN], *Hs, Hm[MN];
    matrix ke;
    
    // see how many reformations we've had to do so far
    int nref = psolver->m_nref;
    
    // set higher order stiffness mutliplier
    // NOTE: this algrotihm doesn't really need this
    // but I've added this functionality to compare with the other contact
    // algorithms and to see the effect of the different stiffness contributions
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
    
    // do single- or two-pass
    int npass = (m_btwo_pass?2:1);
    for (int np=0; np < npass; ++np)
    {
        // get the slave and master surface
        FETiedElasticSurface& ss = (np == 0? m_ss : m_ms);
        FETiedElasticSurface& ms = (np == 0? m_ms : m_ss);
        
        // loop over all slave elements
        for (i=0; i<ss.Elements(); ++i)
        {
            // get ths slave element
            FESurfaceElement& se = ss.Element(i);
            
            // get nr of nodes and integration points
            int nseln = se.Nodes();
            int nint = se.GaussPoints();
            
            // copy the LM vector
            ss.UnpackLM(se, sLM);
            
            // we calculate all the metrics we need before we
            // calculate the nodal forces
            for (j=0; j<nint; ++j)
            {
                // get the base vectors
                vec3d g[2];
                ss.CoBaseVectors(se, j, g);
                
                // jacobians: J = |g0xg1|
                detJ[j] = (g[0] ^ g[1]).norm();
                
                // integration weights
                w[j] = se.GaussWeights()[j];
                
            }
            
            // loop over all integration points
            for (j=0; j<nint; ++j)
            {
                FETiedElasticSurface::Data& pt = ss.m_Data[i][j];
                
                // get the master element
                FESurfaceElement* pme = pt.m_pme;
                if (pme)
                {
                    FESurfaceElement& me = *pme;
                    
                    // get the nr of master nodes
                    int nmeln = me.Nodes();
                    
                    // copy the LM vector
                    ms.UnpackLM(me, mLM);
                    
                    int ndpn;    // number of dofs per node
                    int ndof;    // number of dofs in stiffness matrix
                    
                    // calculate degrees of freedom for elastic-on-elastic contact
                    ndpn = 3;
                    ndof = ndpn*(nseln + nmeln);
                    
                    // build the LM vector
                    LM.resize(ndof);
                    
                    for (k=0; k<nseln; ++k)
                    {
                        LM[3*k  ] = sLM[3*k  ];
                        LM[3*k+1] = sLM[3*k+1];
                        LM[3*k+2] = sLM[3*k+2];
                    }
                    
                    for (k=0; k<nmeln; ++k)
                    {
                        LM[3*(k+nseln)  ] = mLM[3*k  ];
                        LM[3*(k+nseln)+1] = mLM[3*k+1];
                        LM[3*(k+nseln)+2] = mLM[3*k+2];
                    }

                    // build the en vector
                    en.resize(nseln+nmeln);
                    for (k=0; k<nseln; ++k) en[k      ] = se.m_node[k];
                    for (k=0; k<nmeln; ++k) en[k+nseln] = me.m_node[k];
                    
                    // slave shape functions
                    Hs = se.H(j);
                    
                    // master shape functions
                    double r = pt.m_rs[0];
                    double s = pt.m_rs[1];
                    me.shape_fnc(Hm, r, s);
                    
                    // get slave normal vector
                    vec3d nu = pt.m_nu;
                    
                    // gap function
                    vec3d dg = pt.m_dg;
                    
                    // lagrange multiplier
                    vec3d Lm = pt.m_Lmd;
                    
                    // penalty
                    double eps = m_epsn*pt.m_epsn;
                    
                    // contact traction
                    vec3d t = Lm + dg*eps;
                    
                    // create the stiffness matrix
                    ke.resize(ndof, ndof); ke.zero();
                    
                    // --- S O L I D - S O L I D   C O N T A C T ---
                    
                    // a. I-term
                    //------------------------------------
                    
                    for (k=0; k<nseln; ++k) {
                        for (l=0; l<nseln; ++l)
                        {
                            ke[ndpn*k    ][ndpn*l    ] += eps*Hs[k]*Hs[l]*detJ[j]*w[j];
                            ke[ndpn*k + 1][ndpn*l + 1] += eps*Hs[k]*Hs[l]*detJ[j]*w[j];
                            ke[ndpn*k + 2][ndpn*l + 2] += eps*Hs[k]*Hs[l]*detJ[j]*w[j];
                        }
                        for (l=0; l<nmeln; ++l)
                        {
                            ke[ndpn*k    ][ndpn*(nseln+l)    ] += -eps*Hs[k]*Hm[l]*detJ[j]*w[j];
                            ke[ndpn*k + 1][ndpn*(nseln+l) + 1] += -eps*Hs[k]*Hm[l]*detJ[j]*w[j];
                            ke[ndpn*k + 2][ndpn*(nseln+l) + 2] += -eps*Hs[k]*Hm[l]*detJ[j]*w[j];
                        }
                    }
                    
                    for (k=0; k<nmeln; ++k) {
                        for (l=0; l<nseln; ++l)
                        {
                            ke[ndpn*(nseln+k)    ][ndpn*l    ] += -eps*Hm[k]*Hs[l]*detJ[j]*w[j];
                            ke[ndpn*(nseln+k) + 1][ndpn*l + 1] += -eps*Hm[k]*Hs[l]*detJ[j]*w[j];
                            ke[ndpn*(nseln+k) + 2][ndpn*l + 2] += -eps*Hm[k]*Hs[l]*detJ[j]*w[j];
                        }
                        for (l=0; l<nmeln; ++l)
                        {
                            ke[ndpn*(nseln+k)    ][ndpn*(nseln+l)    ] += eps*Hm[k]*Hm[l]*detJ[j]*w[j];
                            ke[ndpn*(nseln+k) + 1][ndpn*(nseln+l) + 1] += eps*Hm[k]*Hm[l]*detJ[j]*w[j];
                            ke[ndpn*(nseln+k) + 2][ndpn*(nseln+l) + 2] += eps*Hm[k]*Hm[l]*detJ[j]*w[j];
                        }
                    }
                    
                    // b. A-term
                    //-------------------------------------
                    
                    double* Gr = se.Gr(j);
                    double* Gs = se.Gs(j);
                    vec3d gs[2];
                    ss.CoBaseVectors(se, j, gs);
                    
                    vec3d as[FEElement::MAX_NODES];
                    mat3d As[FEElement::MAX_NODES];
                    for (l=0; l<nseln; ++l) {
                        as[l] = nu ^ (gs[1]*Gr[l] - gs[0]*Gs[l]);
                        As[l] = t & as[l];
                    }
                    
                    if (!m_bsymm)
                    {
                        // non-symmetric
                        for (k=0; k<nseln; ++k) {
                            for (l=0; l<nseln; ++l)
                            {
                                ke[ndpn*k    ][ndpn*l    ] += Hs[k]*As[l](0,0)*w[j];
                                ke[ndpn*k    ][ndpn*l + 1] += Hs[k]*As[l](0,1)*w[j];
                                ke[ndpn*k    ][ndpn*l + 2] += Hs[k]*As[l](0,2)*w[j];
                                
                                ke[ndpn*k + 1][ndpn*l    ] += Hs[k]*As[l](1,0)*w[j];
                                ke[ndpn*k + 1][ndpn*l + 1] += Hs[k]*As[l](1,1)*w[j];
                                ke[ndpn*k + 1][ndpn*l + 2] += Hs[k]*As[l](1,2)*w[j];
                                
                                ke[ndpn*k + 2][ndpn*l    ] += Hs[k]*As[l](2,0)*w[j];
                                ke[ndpn*k + 2][ndpn*l + 1] += Hs[k]*As[l](2,1)*w[j];
                                ke[ndpn*k + 2][ndpn*l + 2] += Hs[k]*As[l](2,2)*w[j];
                            }
                        }
                        
                        for (k=0; k<nmeln; ++k) {
                            for (l=0; l<nseln; ++l)
                            {
                                ke[ndpn*(nseln+k)    ][ndpn*l    ] += -Hm[k]*As[l](0,0)*w[j];
                                ke[ndpn*(nseln+k)    ][ndpn*l + 1] += -Hm[k]*As[l](0,1)*w[j];
                                ke[ndpn*(nseln+k)    ][ndpn*l + 2] += -Hm[k]*As[l](0,2)*w[j];
                                
                                ke[ndpn*(nseln+k) + 1][ndpn*l    ] += -Hm[k]*As[l](1,0)*w[j];
                                ke[ndpn*(nseln+k) + 1][ndpn*l + 1] += -Hm[k]*As[l](1,1)*w[j];
                                ke[ndpn*(nseln+k) + 1][ndpn*l + 2] += -Hm[k]*As[l](1,2)*w[j];
                                
                                ke[ndpn*(nseln+k) + 2][ndpn*l    ] += -Hm[k]*As[l](2,0)*w[j];
                                ke[ndpn*(nseln+k) + 2][ndpn*l + 1] += -Hm[k]*As[l](2,1)*w[j];
                                ke[ndpn*(nseln+k) + 2][ndpn*l + 2] += -Hm[k]*As[l](2,2)*w[j];
                            }
                        }
                        
                    }
                    else
                    {
                        // symmetric
                        for (k=0; k<nseln; ++k) {
                            for (l=0; l<nseln; ++l)
                            {
                                ke[ndpn*k    ][ndpn*l    ] += 0.5*(Hs[k]*As[l](0,0)+Hs[l]*As[k](0,0))*w[j];
                                ke[ndpn*k    ][ndpn*l + 1] += 0.5*(Hs[k]*As[l](0,1)+Hs[l]*As[k](1,0))*w[j];
                                ke[ndpn*k    ][ndpn*l + 2] += 0.5*(Hs[k]*As[l](0,2)+Hs[l]*As[k](2,0))*w[j];
                                
                                ke[ndpn*k + 1][ndpn*l    ] += 0.5*(Hs[k]*As[l](1,0)+Hs[l]*As[k](0,1))*w[j];
                                ke[ndpn*k + 1][ndpn*l + 1] += 0.5*(Hs[k]*As[l](1,1)+Hs[l]*As[k](1,1))*w[j];
                                ke[ndpn*k + 1][ndpn*l + 2] += 0.5*(Hs[k]*As[l](1,2)+Hs[l]*As[k](2,1))*w[j];
                                
                                ke[ndpn*k + 2][ndpn*l    ] += 0.5*(Hs[k]*As[l](2,0)+Hs[l]*As[k](0,2))*w[j];
                                ke[ndpn*k + 2][ndpn*l + 1] += 0.5*(Hs[k]*As[l](2,1)+Hs[l]*As[k](1,2))*w[j];
                                ke[ndpn*k + 2][ndpn*l + 2] += 0.5*(Hs[k]*As[l](2,2)+Hs[l]*As[k](2,2))*w[j];
                            }
                        }
                        
                        for (k=0; k<nmeln; ++k) {
                            for (l=0; l<nseln; ++l)
                            {
                                ke[ndpn*(nseln+k)    ][ndpn*l    ] += -0.5*Hm[k]*As[l](0,0)*w[j];
                                ke[ndpn*(nseln+k)    ][ndpn*l + 1] += -0.5*Hm[k]*As[l](0,1)*w[j];
                                ke[ndpn*(nseln+k)    ][ndpn*l + 2] += -0.5*Hm[k]*As[l](0,2)*w[j];
                                
                                ke[ndpn*(nseln+k) + 1][ndpn*l    ] += -0.5*Hm[k]*As[l](1,0)*w[j];
                                ke[ndpn*(nseln+k) + 1][ndpn*l + 1] += -0.5*Hm[k]*As[l](1,1)*w[j];
                                ke[ndpn*(nseln+k) + 1][ndpn*l + 2] += -0.5*Hm[k]*As[l](1,2)*w[j];
                                
                                ke[ndpn*(nseln+k) + 2][ndpn*l    ] += -0.5*Hm[k]*As[l](2,0)*w[j];
                                ke[ndpn*(nseln+k) + 2][ndpn*l + 1] += -0.5*Hm[k]*As[l](2,1)*w[j];
                                ke[ndpn*(nseln+k) + 2][ndpn*l + 2] += -0.5*Hm[k]*As[l](2,2)*w[j];
                            }
                        }
                        
                        for (k=0; k<nseln; ++k) {
                            for (l=0; l<nmeln; ++l)
                            {
                                ke[ndpn*k    ][ndpn*(nseln+l)    ] += -0.5*Hm[l]*As[k](0,0)*w[j];
                                ke[ndpn*k    ][ndpn*(nseln+l) + 1] += -0.5*Hm[l]*As[k](1,0)*w[j];
                                ke[ndpn*k    ][ndpn*(nseln+l) + 2] += -0.5*Hm[l]*As[k](2,0)*w[j];
                                
                                ke[ndpn*k + 1][ndpn*(nseln+l)    ] += -0.5*Hm[l]*As[k](0,1)*w[j];
                                ke[ndpn*k + 1][ndpn*(nseln+l) + 1] += -0.5*Hm[l]*As[k](1,1)*w[j];
                                ke[ndpn*k + 1][ndpn*(nseln+l) + 2] += -0.5*Hm[l]*As[k](2,1)*w[j];
                                
                                ke[ndpn*k + 2][ndpn*(nseln+l)    ] += -0.5*Hm[l]*As[k](0,2)*w[j];
                                ke[ndpn*k + 2][ndpn*(nseln+l) + 1] += -0.5*Hm[l]*As[k](1,2)*w[j];
                                ke[ndpn*k + 2][ndpn*(nseln+l) + 2] += -0.5*Hm[l]*As[k](2,2)*w[j];
                            }
                        }
                    }
                    // assemble the global stiffness
                    psolver->AssembleStiffness(en, LM, ke);
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
bool FETiedElasticInterface::Augment(int naug, const FETimeInfo& tp)
{
    // make sure we need to augment
    if (!m_blaugon) return true;
    
    int i;
    vec3d Ln;
    bool bconv = true;
    
    int NS = (int)m_ss.m_Data.size();
    int NM = (int)m_ms.m_Data.size();
    
    // --- c a l c u l a t e   i n i t i a l   n o r m s ---
    // a. normal component
    double normL0 = 0, normP = 0, normDP = 0;
    for (int i=0; i<NS; ++i)
    {
        vector<FETiedElasticSurface::Data>& sd = m_ss.m_Data[i];
        for (int j=0; j<(int)sd.size(); ++j)
        {
            FETiedElasticSurface::Data& ds = sd[j];
            normL0 += ds.m_Lmd*ds.m_Lmd;
        }
    }
    for (int i=0; i<NM; ++i)
    {
        vector<FETiedElasticSurface::Data>& md = m_ms.m_Data[i];
        for (int j=0; j<(int)md.size(); ++j)
        {
            FETiedElasticSurface::Data& dm = md[j];
            normL0 += dm.m_Lmd*dm.m_Lmd;
        }
    }
    
    // b. gap component
    // (is calculated during update)
    double maxgap = 0;
    
    // update Lagrange multipliers
    double normL1 = 0, eps;
    for (i=0; i<NS; ++i)
    {
        vector<FETiedElasticSurface::Data>& sd = m_ss.m_Data[i];
        for (int j=0; j<(int)sd.size(); ++j)
        {
            FETiedElasticSurface::Data& ds = sd[j];
            
            // update Lagrange multipliers on slave surface
            eps = m_epsn*ds.m_epsn;
            ds.m_Lmd = ds.m_Lmd + ds.m_dg*eps;
            
            normL1 += ds.m_Lmd*ds.m_Lmd;
            
            maxgap = max(maxgap,sqrt(ds.m_dg*ds.m_dg));
        }
    }
    
    for (i=0; i<NM; ++i)
    {
        vector<FETiedElasticSurface::Data>& md = m_ms.m_Data[i];
        for (int j=0; j<(int)md.size(); ++j)
        {
            FETiedElasticSurface::Data& dm = md[j];
            
            // update Lagrange multipliers on master surface
            eps = m_epsn*dm.m_epsn;
            dm.m_Lmd = dm.m_Lmd + dm.m_dg*eps;
            
            normL1 += dm.m_Lmd*dm.m_Lmd;
            
            maxgap = max(maxgap,sqrt(dm.m_dg*dm.m_dg));
        }
    }
    
    // Ideally normP should be evaluated from the fluid pressure at the
    // contact interface (not easily accessible).  The next best thing
    // is to use the contact traction.
    normP = normL1;
    
    // calculate relative norms
    double lnorm = (normL1 != 0 ? fabs((normL1 - normL0) / normL1) : fabs(normL1 - normL0));
    double pnorm = (normP != 0 ? (normDP/normP) : normDP);
    
    // check convergence
    if ((m_gtol > 0) && (maxgap > m_gtol)) bconv = false;
    
    if ((m_atol > 0) && (lnorm > m_atol)) bconv = false;
    if ((m_atol > 0) && (pnorm > m_atol)) bconv = false;
    
    if (naug < m_naugmin ) bconv = false;
    if (naug >= m_naugmax) bconv = true;
    
    felog.printf(" sliding interface # %d\n", GetID());
    felog.printf("                        CURRENT        REQUIRED\n");
    felog.printf("    D multiplier : %15le", lnorm); if (m_atol > 0) felog.printf("%15le\n", m_atol); else felog.printf("       ***\n");
    
    felog.printf("    maximum gap  : %15le", maxgap);
    if (m_gtol > 0) felog.printf("%15le\n", m_gtol); else felog.printf("       ***\n");
    
    return bconv;
}

//-----------------------------------------------------------------------------
void FETiedElasticInterface::Serialize(DumpStream &ar)
{
    // store contact data
    FEContactInterface::Serialize(ar);
    
    // store contact surface data
    m_ms.Serialize(ar);
    m_ss.Serialize(ar);
    
    // serialize pointers
    if (ar.IsShallow() == false)
    {
        if (ar.IsSaving())
        {
            int NE = (int)m_ss.m_Data.size();
            for (int i=0; i<NE; ++i)
            {
                int NI = (int)m_ss.m_Data[i].size();
                for (int j=0; j<NI; ++j)
                {
                    FESurfaceElement* pe = m_ss.m_Data[i][j].m_pme;
                    if (pe) ar << pe->m_lid; else ar << -1;
                }
            }
        }
        else
        {
            int NE = (int)m_ss.m_Data.size(), lid;
            for (int i=0; i<NE; ++i)
            {
                int NI = (int)m_ss.m_Data[i].size();
                for (int j = 0; j<NI; ++j)
                {
                    ar >> lid;
                    m_ss.m_Data[i][j].m_pme = (lid < 0 ? 0 : &m_ms.Element(lid));
                }
            }
        }
    }
}
