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
#include "FEFluidResistanceBC.h"
#include "FEFluid.h"
#include "FEFluidFSI.h"
#include <FECore/FEModel.h>
#include <FECore/DOFS.h>

//=============================================================================
BEGIN_PARAMETER_LIST(FEFluidResistanceBC, FESurfaceLoad)
ADD_PARAMETER(m_R, FE_PARAM_DOUBLE    , "R");
ADD_PARAMETER(m_p0, FE_PARAM_DOUBLE   , "pressure_offset");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
//! constructor
FEFluidResistanceBC::FEFluidResistanceBC(FEModel* pfem) : FESurfaceLoad(pfem)
{
    m_R = 0.0;
    m_pfluid = nullptr;
    m_alpha = 1.0;
    m_p0 = 0;
    
    m_dofWX = pfem->GetDOFIndex("wx");
    m_dofWY = pfem->GetDOFIndex("wy");
    m_dofWZ = pfem->GetDOFIndex("wz");
    m_dofEF  = pfem->GetDOFIndex("ef" );

    m_dofWXP = pfem->GetDOFIndex("wxp");
    m_dofWYP = pfem->GetDOFIndex("wyp");
    m_dofWZP = pfem->GetDOFIndex("wzp");
}

//-----------------------------------------------------------------------------
//! allocate storage
void FEFluidResistanceBC::SetSurface(FESurface* ps)
{
    FESurfaceLoad::SetSurface(ps);
}

//-----------------------------------------------------------------------------
FEFluid* GetFluidMaterial(FEModel* fem, int mid)
{
	FEMaterial* pm = fem->GetMaterial(mid);
	FEFluid* fluid = dynamic_cast<FEFluid*> (pm);
	FEFluidFSI* fsi = dynamic_cast<FEFluidFSI*>(pm);
	if (fluid) return fluid;
	else if (fsi) return fsi->Fluid();
	else return nullptr;
}

//-----------------------------------------------------------------------------
//! initialize
bool FEFluidResistanceBC::Init()
{
    FEModelComponent::Init();
    
    FESurface* ps = &GetSurface();
    ps->Init();
    // get fluid bulk modulus from first surface element
    // assuming the entire surface bounds the same fluid
    FESurfaceElement& el = ps->Element(0);
    FEMesh* mesh = ps->GetMesh();
    FEElement* pe = mesh->FindElementFromID(el.m_elem[0]);
    if (pe == nullptr) return false;

    // get the material
	m_pfluid = GetFluidMaterial(GetFEModel(), pe->GetMatID());
	if (m_pfluid == nullptr) return false;
    
    return true;
}

//-----------------------------------------------------------------------------
//! Activate the degrees of freedom for this BC
void FEFluidResistanceBC::Activate()
{
    FESurface* ps = &GetSurface();
    
    for (int i=0; i<ps->Nodes(); ++i)
    {
        FENode& node = ps->Node(i);
        // mark node as having prescribed DOF
        node.m_BC[m_dofEF] = DOF_PRESCRIBED;
    }
}

//-----------------------------------------------------------------------------
//! Evaluate and prescribe the resistance pressure
void FEFluidResistanceBC::Update()
{
    // evaluate the flow rate
    double Q = FlowRate();
    
    // calculate the resistance pressure
    double p = m_R*Q;
    
    // calculate the dilatation
    double e = m_pfluid->Dilatation(p+m_p0);
    
    // prescribe this dilatation at the nodes
    FESurface* ps = &GetSurface();

    for (int i=0; i<ps->Nodes(); ++i)
    {
        if (ps->Node(i).m_ID[m_dofEF] < -1)
        {
            FENode& node = ps->Node(i);
            // set node as having prescribed DOF
            node.set(m_dofEF, e);
        }
    }
}

//-----------------------------------------------------------------------------
//! evaluate the flow rate across this surface
double FEFluidResistanceBC::FlowRate()
{
    double Q = 0;
    
    vec3d rt[FEElement::MAX_NODES];
    vec3d vt[FEElement::MAX_NODES];
    
    for (int iel=0; iel<m_psurf->Elements(); ++iel)
    {
        FESurfaceElement& el = m_psurf->Element(iel);
        
        // nr integration points
        int nint = el.GaussPoints();
        
        // nr of element nodes
        int neln = el.Nodes();
        
        // nodal coordinates
        for (int i=0; i<neln; ++i) {
            FENode& node = m_psurf->GetMesh()->Node(el.m_node[i]);
            rt[i] = node.m_rt*m_alpha + node.m_rp*(1-m_alpha);
            vt[i] = node.get_vec3d(m_dofWX, m_dofWY, m_dofWZ)*m_alphaf + node.get_vec3d(m_dofWXP, m_dofWYP, m_dofWZP)*(1-m_alphaf);
        }
        
        double* Nr, *Ns;
        double* N;
        double* w  = el.GaussWeights();
        
        vec3d dxr, dxs, v;
        
        // repeat over integration points
        for (int n=0; n<nint; ++n)
        {
            N  = el.H(n);
            Nr = el.Gr(n);
            Ns = el.Gs(n);
            
            // calculate the velocity and tangent vectors at integration point
            dxr = dxs = v = vec3d(0,0,0);
            for (int i=0; i<neln; ++i)
            {
                v += vt[i]*N[i];
                dxr += rt[i]*Nr[i];
                dxs += rt[i]*Ns[i];
            }
            
            vec3d normal = dxr ^ dxs;
            double q = normal*v;
            Q += q*w[n];
        }
    }

    return Q;
}

//! serialize
void FEFluidResistanceBC::Serialize(DumpStream& ar)
{
	FESurfaceLoad::Serialize(ar);
	if (ar.IsShallow()) return;

	if (ar.IsSaving())
	{
		ar << m_alphaf << m_alpha;
		int mid = m_pfluid->GetID();
		if (mid == -1)
		{
			mid = dynamic_cast<FEMaterial*>(m_pfluid->GetParent())->GetID();
		}
		ar << mid;
	}
	else
	{
		ar >> m_alphaf >> m_alpha;
		int mid = -1;
		ar >> mid;

		m_pfluid = GetFluidMaterial(GetFEModel(), mid - 1);
		if (m_pfluid == nullptr) throw DumpStream::ReadError();
	}
}
