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
#include "FEFluidVelocity.h"
#include <FECore/FEModel.h>
#include <FECore/FEElemElemList.h>
#include <FECore/FEGlobalMatrix.h>
#include <FECore/FEGlobalVector.h>
#include <FECore/log.h>
#include <FECore/LinearSolver.h>

//=============================================================================
BEGIN_PARAMETER_LIST(FEFluidVelocity, FESurfaceLoad)
ADD_PARAMETER(m_scale   , FE_PARAM_DOUBLE    , "scale");
ADD_PARAMETER(m_VC      , FE_PARAM_DATA_ARRAY, "velocity"   );
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
//! constructor
FEFluidVelocity::FEFluidVelocity(FEModel* pfem) : FESurfaceLoad(pfem), m_VC(FE_VEC3D)
{
    m_scale = 1.0;
    
    m_dofWX = pfem->GetDOFIndex("wx");
    m_dofWY = pfem->GetDOFIndex("wy");
    m_dofWZ = pfem->GetDOFIndex("wz");
    m_dofEF = pfem->GetDOFIndex("ef");
}

//-----------------------------------------------------------------------------
//! allocate storage
void FEFluidVelocity::SetSurface(FESurface* ps)
{
    FESurfaceLoad::SetSurface(ps);
    m_VC.Create(ps);
}

//-----------------------------------------------------------------------------
void FEFluidVelocity::UnpackLM(FEElement& el, vector<int>& lm)
{
    FEMesh& mesh = GetFEModel()->GetMesh();
    int N = el.Nodes();
    lm.resize(N);
    for (int i=0; i<N; ++i)
    {
        int n = el.m_node[i];
        FENode& node = mesh.Node(n);
        vector<int>& id = node.m_ID;
        
        lm[i] = id[m_dofEF];
    }
}

//-----------------------------------------------------------------------------
//! Calculate the residual for the prescribed normal component of velocity
void FEFluidVelocity::Residual(const FETimeInfo& tp, FEGlobalVector& R)
{
    vector<double> fe;
    vector<int> elm;
    
    vec3d r0[FEElement::MAX_NODES];
    
    int i, n;
    int N = m_psurf->Elements();
    for (int iel=0; iel<N; ++iel)
    {
        FESurfaceElement& el = m_psurf->Element(iel);
        
        int ndof = el.Nodes();
        fe.resize(ndof);
        
        // nr integration points
        int nint = el.GaussPoints();
        
        // nr of element nodes
        int neln = el.Nodes();
        
        // nodal coordinates
        for (i=0; i<neln; ++i) 
		{
			r0[i] = m_psurf->GetMesh()->Node(el.m_node[i]).m_r0;
		}
        
        double* Gr, *Gs;
        double* N;
        double* w  = el.GaussWeights();
        
        vec3d dxr, dxs, v;
        double vn;
        
        // repeat over integration points
        zero(fe);
        for (n=0; n<nint; ++n)
        {
            N  = el.H(n);
            Gr = el.Gr(n);
            Gs = el.Gs(n);
            
            // calculate the tangent vectors
            v = dxr = dxs = vec3d(0,0,0);
            for (i=0; i<neln; ++i)
            {
                v += m_VN[el.m_lnode[i]]*N[i];
                dxr += r0[i]*Gr[i];
                dxs += r0[i]*Gs[i];
            }
            
            vec3d nu = dxr ^ dxs;
            double da = nu.unit();
            vn = (v*nu)*m_scale;
            
            for (i=0; i<neln; ++i)
                fe[i] += N[i]*vn*w[n]*da;
        }
        
        // get the element's LM vector and adjust it
        UnpackLM(el, elm);
        
        // add element force vector to global force vector
        R.Assemble(el.m_node, elm, fe);
    }
}

//-----------------------------------------------------------------------------
//! initialize
bool FEFluidVelocity::Init()
{
    FEModelComponent::Init();
    
    FESurface* ps = &GetSurface();
    ps->Init();
    
    // evaluate nodal velocities from boundary cards
    m_VN.resize(ps->Nodes(),vec3d(0,0,0));
    vector<int> nf(ps->Nodes(),0);

    for (int iel=0; iel<ps->Elements(); ++iel)
    {
        FESurfaceElement& el = m_psurf->Element(iel);
        
        // nr of element nodes
        int neln = el.Nodes();
        
        for (int i=0; i<neln; ++i) 
		{
            m_VN[el.m_lnode[i]] += m_VC.value<vec3d>(iel, i);
            ++nf[el.m_lnode[i]];
        }
    }
    
    for (int i=0; i<ps->Nodes(); ++i) m_VN[i] /= nf[i];
    
    return true;
}

//-----------------------------------------------------------------------------
//! Activate the degrees of freedom for this BC
void FEFluidVelocity::Activate()
{
    FESurface* ps = &GetSurface();
    
    for (int i=0; i<ps->Nodes(); ++i)
    {
        FENode& node = ps->Node(i);
        // mark node as having prescribed DOF
        node.m_BC[m_dofWX] = DOF_PRESCRIBED;
        node.m_BC[m_dofWY] = DOF_PRESCRIBED;
        node.m_BC[m_dofWZ] = DOF_PRESCRIBED;
    }
}

//-----------------------------------------------------------------------------
//! Evaluate and prescribe the nodal velocities
void FEFluidVelocity::Update()
{
    // prescribe this velocity at the nodes
    FESurface* ps = &GetSurface();
    
    for (int i=0; i<ps->Nodes(); ++i)
    {
        // evaluate the nodal velocity
        vec3d v = m_VN[i]*m_scale;
        FENode& node = ps->Node(i);
        if (node.m_ID[m_dofWX] < -1) node.set(m_dofWX, v.x);
        if (node.m_ID[m_dofWY] < -1) node.set(m_dofWY, v.y);
        if (node.m_ID[m_dofWZ] < -1) node.set(m_dofWZ, v.z);
    }
}
