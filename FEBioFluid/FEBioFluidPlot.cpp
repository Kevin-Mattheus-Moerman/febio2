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
#include "FEBioFluidPlot.h"
#include "FEFluidDomain3D.h"
#include "FEFluidDomain2D.h"
#include "FEFluid.h"
#include "FEFluidDomain.h"
#include "FEFluidFSIDomain.h"
#include "FEFluidFSI.h"
#include "FEBioPlot/FEBioPlotFile.h"

//=============================================================================
//                            N O D E   D A T A
//=============================================================================

//-----------------------------------------------------------------------------
//! Store the nodal displacements
bool FEPlotDisplacement::Save(FEMesh& m, FEDataStream& a)
{
    // loop over all nodes
    for (int i=0; i<m.Nodes(); ++i)
    {
        FENode& node = m.Node(i);
        a << node.m_rt - node.m_r0;
    }
    return true;
}

//-----------------------------------------------------------------------------
//! Store the nodal fluid velocity
bool FEPlotNodalFluidVelocity::Save(FEMesh& m, FEDataStream& a)
{
    int dofWX = m_pfem->GetDOFIndex("wx");
    int dofWY = m_pfem->GetDOFIndex("wy");
    int dofWZ = m_pfem->GetDOFIndex("wz");
    int dofVX = m_pfem->GetDOFIndex("vx");
    int dofVY = m_pfem->GetDOFIndex("vy");
    int dofVZ = m_pfem->GetDOFIndex("vz");

	bool bvel = true;
	if ((dofVX == -1) || (dofVY == -1) || (dofVZ == -1))
	{
		bvel = false;
	}
    
    // loop over all nodes
    for (int i=0; i<m.Nodes(); ++i)
    {
        FENode& node = m.Node(i);
        vec3d vs = (bvel ? node.get_vec3d(dofVX, dofVY, dofVZ) : vec3d(0,0,0));
        vec3d w = node.get_vec3d(dofWX, dofWY, dofWZ);
        a << vs+w;
    }
    return true;
}

//-----------------------------------------------------------------------------
//! Store the nodal relative fluid velocity
bool FEPlotNodalRelativeFluidVelocity::Save(FEMesh& m, FEDataStream& a)
{
    int dofWX = m_pfem->GetDOFIndex("wx");
    int dofWY = m_pfem->GetDOFIndex("wy");
    int dofWZ = m_pfem->GetDOFIndex("wz");
    
    // loop over all nodes
    for (int i=0; i<m.Nodes(); ++i)
    {
        FENode& node = m.Node(i);
        vec3d vf = node.get_vec3d(dofWX, dofWY, dofWZ);
        a << vf;
    }
    return true;
}

//-----------------------------------------------------------------------------
//! Store the nodal dilatations
bool FEPlotFluidDilatation::Save(FEMesh& m, FEDataStream& a)
{
    // get the dilatation dof index
    int dof_e = GetFEModel()->GetDOFIndex("ef");

    // loop over all nodes
    for (int i=0; i<m.Nodes(); ++i)
    {
        FENode& node = m.Node(i);
        a << node.get(dof_e);
    }
    return true;
}

//=============================================================================
//                       S U R F A C E    D A T A
//=============================================================================

//-----------------------------------------------------------------------------
bool FEPlotFluidSurfaceForce::Save(FESurface &surf, FEDataStream &a)
{
    FESurface* pcs = &surf;
    if (pcs == 0) return false;
    
    // Evaluate this field only for a specific domain, by checking domain name
    if (pcs->GetName() != m_szdom) return false;
    
    int NF = pcs->Elements();
    vec3d fn(0,0,0);    // initialize
    
    FEMesh* m_pMesh = pcs->GetMesh();
    
    // initialize on the first pass to calculate the vectorial area of each surface element and to identify solid element associated with this surface element
    if (m_binit) {
        m_area.resize(NF);
        m_elem.resize(NF);
        for (int j=0; j<NF; ++j)
        {
            FESurfaceElement& el = pcs->Element(j);
            m_area[j] = pcs->SurfaceNormal(el,0,0)*pcs->FaceArea(el);
            m_elem[j] = m_pMesh->FindElementFromID(pcs->FindElement(el));
        }
        m_binit = false;
    }
    
    // calculate net fluid force
    for (int j=0; j<NF; ++j)
    {
        // get the element this surface element belongs to
        FEElement* pe = m_elem[j];
        if (pe)
        {
            // get the material
            FEMaterial* pm = m_pfem->GetMaterial(pe->GetMatID());
            
            // see if this is a fluid element
            FEFluid* fluid = dynamic_cast<FEFluid*> (pm);
            FEFluidFSI* fsi = dynamic_cast<FEFluidFSI*> (pm);
            if (fluid || fsi) {
                // evaluate the average stress in this element
                int nint = pe->GaussPoints();
                mat3ds s(mat3dd(0));
                for (int n=0; n<nint; ++n)
                {
                    FEMaterialPoint& mp = *pe->GetMaterialPoint(n);
                    FEFluidMaterialPoint& pt = *(mp.ExtractData<FEFluidMaterialPoint>());
                    s += pt.m_sf;
                }
                s /= nint;
                
                // Evaluate contribution to net force on surface.
                // Negate the fluid traction since we want the traction on the surface,
                // which is the opposite of the traction on the fluid.
                fn -= s*m_area[j];
            }
        }
    }
    
    // save results
	a << fn;
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidSurfaceTractionPower::Save(FESurface &surf, FEDataStream &a)
{
    FESurface* pcs = &surf;
    if (pcs == 0) return false;
    
    // Evaluate this field only for a specific domain, by checking domain name
	if (pcs->GetName() != m_szdom) return false;

    int NF = pcs->Elements();
    double fn = 0;    // initialize
    
    FEMesh* m_pMesh = pcs->GetMesh();
    
    // initialize on the first pass to calculate the vectorial area of each surface element and to identify solid element associated with this surface element
    if (m_binit) {
        m_area.resize(NF);
        m_elem.resize(NF);
        for (int j=0; j<NF; ++j)
        {
            FESurfaceElement& el = pcs->Element(j);
            m_area[j] = pcs->SurfaceNormal(el,0,0)*pcs->FaceArea(el);
            m_elem[j] = m_pMesh->FindElementFromID(pcs->FindElement(el));
        }
        m_binit = false;
    }
    
    // calculate net fluid force
    for (int j=0; j<NF; ++j)
    {
        // get the element this surface element belongs to
        FEElement* pe = m_elem[j];
        if (pe)
        {
            // get the material
            FEMaterial* pm = m_pfem->GetMaterial(pe->GetMatID());
            
            // see if this is a fluid element
            FEFluid* fluid = dynamic_cast<FEFluid*> (pm);
            if (fluid) {
                // evaluate the average stress in this element
                int nint = pe->GaussPoints();
                double s = 0;
                for (int n=0; n<nint; ++n)
                {
                    FEMaterialPoint& mp = *pe->GetMaterialPoint(n);
                    FEFluidMaterialPoint& pt = *(mp.ExtractData<FEFluidMaterialPoint>());
                    s += pt.m_vft*(pt.m_sf*m_area[j]);
                }
                s /= nint;
                
                // Evaluate contribution to net traction power on surface.
                fn += s;
            }
        }
    }
    
    // save results
    a << fn;
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidSurfaceEnergyFlux::Save(FESurface &surf, FEDataStream &a)
{
    FESurface* pcs = &surf;
    if (pcs == 0) return false;
    
    // Evaluate this field only for a specific domain, by checking domain name
	if (pcs->GetName() != m_szdom) return false;

    int NF = pcs->Elements();
    double fn = 0;    // initialize
    
    FEMesh* m_pMesh = pcs->GetMesh();
    
    // initialize on the first pass to calculate the vectorial area of each surface element and to identify solid element associated with this surface element
    if (m_binit) {
        m_area.resize(NF);
        m_elem.resize(NF);
        for (int j=0; j<NF; ++j)
        {
            FESurfaceElement& el = pcs->Element(j);
            m_area[j] = pcs->SurfaceNormal(el,0,0)*pcs->FaceArea(el);
            m_elem[j] = m_pMesh->FindElementFromID(pcs->FindElement(el));
        }
        m_binit = false;
    }
    
    // calculate net fluid force
    for (int j=0; j<NF; ++j)
    {
        // get the element this surface element belongs to
        FEElement* pe = m_elem[j];
        if (pe)
        {
            // get the material
            FEMaterial* pm = m_pfem->GetMaterial(pe->GetMatID());
            
            // see if this is a fluid element
            FEFluid* fluid = dynamic_cast<FEFluid*> (pm);
            if (fluid) {
                // evaluate the average stress in this element
                int nint = pe->GaussPoints();
                double s = 0;
                for (int n=0; n<nint; ++n)
                {
                    FEMaterialPoint& mp = *pe->GetMaterialPoint(n);
                    FEFluidMaterialPoint& pt = *(mp.ExtractData<FEFluidMaterialPoint>());
                    s += fluid->EnergyDensity(mp)*(pt.m_vft*m_area[j]);
                }
                s /= nint;
                
                // Evaluate contribution to net energy flux on surface.
                fn += s;
            }
        }
    }
    
    // save results
    a << fn;
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidMassFlowRate::Save(FESurface &surf, FEDataStream &a)
{
    FESurface* pcs = &surf;
    if (pcs == 0) return false;
    
    // Evaluate this field only for a specific domain, by checking domain name
	if (pcs->GetName() != m_szdom) return false;

    int dofWX = m_pfem->GetDOFIndex("wx");
    int dofWY = m_pfem->GetDOFIndex("wy");
    int dofWZ = m_pfem->GetDOFIndex("wz");
    int dofEF  = m_pfem->GetDOFIndex("ef");
    
    int NF = pcs->Elements();
    double fn = 0;    // initialize
    
    FEMesh* m_pMesh = pcs->GetMesh();
    
    // initialize on the first pass to identify solid element associated with this surface element
    if (m_binit) {
        m_elem.resize(NF);
        for (int j=0; j<NF; ++j)
        {
            FESurfaceElement& el = pcs->Element(j);
            m_elem[j] = m_pMesh->FindElementFromID(pcs->FindElement(el));
        }
        m_binit = false;
    }
    
    // calculate net fluid mass flow rate
    for (int j=0; j<NF; ++j)
    {
        // get the element this surface element belongs to
        FEElement* pe = m_elem[j];
        if (pe)
        {
            // get the material
            FEMaterial* pm = m_pfem->GetMaterial(pe->GetMatID());
            
            // see if this is a fluid element
            FEFluid* fluid = dynamic_cast<FEFluid*> (pm);
            FEFluidFSI* fsi = dynamic_cast<FEFluidFSI*> (pm);
            if (fluid || fsi) {
                double rhor;
                if (fluid) rhor = fluid->m_rhor;
                else rhor = fsi->Fluid()->m_rhor;
                // get the surface element
                FESurfaceElement& el = pcs->Element(j);
                // extract nodal velocities and dilatation
                int neln = el.Nodes();
                vec3d vt[FEElement::MAX_NODES];
                double et[FEElement::MAX_NODES];
                for (int j=0; j<neln; ++j) {
                    vt[j] = m_pMesh->Node(el.m_node[j]).get_vec3d(dofWX, dofWY, dofWZ);
                    et[j] = m_pMesh->Node(el.m_node[j]).get(dofEF);
                }
                
                // evaluate mass flux across this surface element
                int nint = el.GaussPoints();
                double*	gw = el.GaussWeights();
                vec3d gcov[2];
                for (int n=0; n<nint; ++n) {
                    vec3d v = el.eval(vt, n);
                    double J = 1 + el.eval(et, n);
                    pcs->CoBaseVectors(el, n, gcov);
                    fn += (v*(gcov[0] ^ gcov[1]))*rhor/J*gw[n];
                }
            }
        }
    }
    
    // save results
    a << fn;
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidFlowRate::Save(FESurface &surf, FEDataStream &a)
{
	FESurface* pcs = &surf;
	if (pcs == 0) return false;

	// Evaluate this field only for a specific domain, by checking domain name
	if (pcs->GetName() != m_szdom) return false;

	int NF = pcs->Elements();
	double fn = 0;    // initialize

	FEMesh* m_pMesh = pcs->GetMesh();

	// initialize on the first pass to calculate the vectorial area of each surface element and to identify solid element associated with this surface element
	if (m_binit) {
		m_area.resize(NF);
		m_elem.resize(NF);
		for (int j = 0; j<NF; ++j)
		{
			FESurfaceElement& el = pcs->Element(j);
			m_area[j] = pcs->SurfaceNormal(el, 0, 0)*pcs->FaceArea(el);
			m_elem[j] = m_pMesh->FindElementFromID(pcs->FindElement(el));
		}
		m_binit = false;
	}

	// calculate net flow rate normal to this surface
	for (int j = 0; j<NF; ++j)
	{
		// get the element this surface element belongs to
		FEElement* pe = m_elem[j];
		if (pe)
		{
			// evaluate the average fluid flux in this element
			int nint = pe->GaussPoints();
			vec3d w(0, 0, 0);
			for (int n = 0; n<nint; ++n)
			{
				FEMaterialPoint& mp = *pe->GetMaterialPoint(n);
				FEFluidMaterialPoint* ptf = mp.ExtractData<FEFluidMaterialPoint>();
				if (ptf) w += ptf->m_vft / ptf->m_Jf;
			}
			w /= nint;

			// Evaluate contribution to net flow rate across surface.
			fn += w*m_area[j];
		}
	}

	// save results
	a << fn;

	return true;
}

//=============================================================================
//							D O M A I N   D A T A
//=============================================================================

//-----------------------------------------------------------------------------
bool FEPlotFluidPressure::Save(FEDomain &dom, FEDataStream& a)
{
	FESolidDomain& bd = static_cast<FESolidDomain&>(dom);

	if (dynamic_cast<FEFluidDomain* >(&bd) ||
		dynamic_cast<FEFluidFSIDomain* >(&bd))
	{
		for (int i = 0; i<bd.Elements(); ++i)
		{
			FESolidElement& el = bd.Element(i);

			// calculate average pressure
			double ew = 0;
			for (int j = 0; j<el.GaussPoints(); ++j)
			{
				FEMaterialPoint& mp = *el.GetMaterialPoint(j);
				FEFluidMaterialPoint* pt = (mp.ExtractData<FEFluidMaterialPoint>());

				if (pt) ew += pt->m_pf;
			}
			ew /= el.GaussPoints();

			a << ew;
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool FEPlotElasticFluidPressure::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    // write solid element data
    int N = dom.Elements();
    for (int i=0; i<N; ++i)
    {
        FEElement& el = dom.ElementRef(i);
        
        int nint = el.GaussPoints();
        double f = 1.0 / (double) nint;
        
        // since the PLOT file requires floats we need to convert
        // the doubles to single precision
        // we output the average elastic pressure values of the gauss points
        double r = 0;
        for (int j=0; j<nint; ++j)
        {
            FEFluidMaterialPoint* ppt = (el.GetMaterialPoint(j)->ExtractData<FEFluidMaterialPoint>());
            if (ppt) r += ppt->m_pf;
        }
        r *= f;
        
        a << r;
    }
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidTemperature::Save(FEDomain &dom, FEDataStream& a)
{
    FESolidDomain& bd = static_cast<FESolidDomain&>(dom);

    if (dynamic_cast<FEFluidDomain* >(&bd) ||
        dynamic_cast<FEFluidFSIDomain* >(&bd))
    {
        for (int i = 0; i<bd.Elements(); ++i)
        {
            FESolidElement& el = bd.Element(i);
            // get the material
            FEMaterial* pm = m_pfem->GetMaterial(el.GetMatID());
            FEFluid* pfluid = dynamic_cast<FEFluid*>(pm);
            if (pfluid == nullptr) pfluid = (dynamic_cast<FEFluidFSI*>(pm))->Fluid();

            if (pfluid) {
                // calculate average temperature
                double ew = 0;
                for (int j = 0; j<el.GaussPoints(); ++j)
                {
                    FEMaterialPoint& mp = *el.GetMaterialPoint(j);
                    ew += pfluid->Temperature(mp);
                }
                ew /= el.GaussPoints();
                
                a << ew;
            }
        }
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidVolumeRatio::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    FEMesh& m_pMesh = m_pfem->GetMesh();
    
    if (dom.Class() == FE_DOMAIN_SOLID)
    {
        FESolidDomain& sd = static_cast<FESolidDomain&>(dom);
        int dofEF  = m_pfem->GetDOFIndex("ef");
        double et[FEElement::MAX_NODES];
        
        // write solid element data
        int N = dom.Elements();
        for (int i=0; i<N; ++i)
        {
            FESolidElement& el = sd.Element(i);
            int neln = el.Nodes();
            for (int j=0; j<neln; ++j)
                et[j] = m_pMesh.Node(el.m_node[j]).get(dofEF);
            
            int nint = el.GaussPoints();
            double f = 1.0 / (double) nint;
            
            // since the PLOT file requires floats we need to convert
            // the doubles to single precision
            // we output the average density values of the gauss points
            double r = 0;
            for (int j=0; j<nint; ++j)
            {
                double Jf = 1 + el.Evaluate(et, j);
                r += Jf;
            }
            r *= f;
            
            a << r;
        }
    }
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidDensity::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    FEMesh& m_pMesh = m_pfem->GetMesh();
    double rhor = pme ? pme->m_rhor : sme->Fluid()->m_rhor;

    if (dom.Class() == FE_DOMAIN_SOLID)
    {
        FESolidDomain& sd = static_cast<FESolidDomain&>(dom);
        int dofEF  = m_pfem->GetDOFIndex("ef");
        double et[FEElement::MAX_NODES];

        // write solid element data
        int N = dom.Elements();
        for (int i=0; i<N; ++i)
        {
            FESolidElement& el = sd.Element(i);
            int neln = el.Nodes();
            for (int j=0; j<neln; ++j)
                et[j] = m_pMesh.Node(el.m_node[j]).get(dofEF);

            int nint = el.GaussPoints();
            double f = 1.0 / (double) nint;
            
            // since the PLOT file requires floats we need to convert
            // the doubles to single precision
            // we output the average density values of the gauss points
            double r = 0;
            for (int j=0; j<nint; ++j)
            {
                double Jf = 1 + el.Evaluate(et, j);
                r += rhor/Jf;
            }
            r *= f;
            
            a << r;
        }
    }
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidDensityRate::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    FEMesh& m_pMesh = m_pfem->GetMesh();
    double rhor = pme ? pme->m_rhor : sme->Fluid()->m_rhor;
    
    if (dom.Class() == FE_DOMAIN_SOLID)
    {
        FESolidDomain& sd = static_cast<FESolidDomain&>(dom);
        int dofVX = m_pfem->GetDOFIndex("vx");
        int dofVY = m_pfem->GetDOFIndex("vy");
        int dofVZ = m_pfem->GetDOFIndex("vz");
        int dofEF  = m_pfem->GetDOFIndex("ef");
        int dofAEF  = m_pfem->GetDOFIndex("aef");
        vec3d vt[FEElement::MAX_NODES];
        double et[FEElement::MAX_NODES];
        double aet[FEElement::MAX_NODES];

        // write solid element data
        int N = dom.Elements();
        for (int i=0; i<N; ++i)
        {
            FESolidElement& el = sd.Element(i);
            int neln = el.Nodes();
            for (int j=0; j<neln; ++j) {
                vt[j] = m_pMesh.Node(el.m_node[j]).get_vec3d(dofVX, dofVY, dofVZ);
                et[j] = m_pMesh.Node(el.m_node[j]).get(dofEF);
                aet[j] = m_pMesh.Node(el.m_node[j]).get(dofAEF);
            }
            
            int nint = el.GaussPoints();
            double f = 1.0 / (double) nint;
            
            // since the PLOT file requires floats we need to convert
            // the doubles to single precision
            // we output the average density values of the gauss points
            double r = 0;
            for (int j=0; j<nint; ++j)
            {
                double Jf = 1 + el.Evaluate(et, j);
                double Jfdot = el.Evaluate(aet, j);
                double divvs = sd.gradient(el, vt, j).trace();
                r += rhor/Jf*(divvs - Jfdot/Jf);
            }
            r *= f;
            
            a << r;
        }
    }
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidVelocity::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    // write solid element data
    int N = dom.Elements();
    for (int i=0; i<N; ++i)
    {
        FEElement& el = dom.ElementRef(i);
        
        int nint = el.GaussPoints();
        double f = 1.0 / (double) nint;
        
        // since the PLOT file requires floats we need to convert
        // the doubles to single precision
        // we output the average velocity values of the gauss points
        vec3d r = vec3d(0,0,0);
        for (int j=0; j<nint; ++j)
        {
            FEFluidMaterialPoint* ppt = (el.GetMaterialPoint(j)->ExtractData<FEFluidMaterialPoint>());
            if (ppt) r += ppt->m_vft;
        }
        r *= f;
        
        a << r;
    }
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotRelativeFluidVelocity::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    // write solid element data
    int N = dom.Elements();
    for (int i=0; i<N; ++i)
    {
        FEElement& el = dom.ElementRef(i);
        
        int nint = el.GaussPoints();
        double f = 1.0 / (double) nint;
        
        // since the PLOT file requires floats we need to convert
        // the doubles to single precision
        // we output the average velocity values of the gauss points
        vec3d r = vec3d(0,0,0);
        for (int j=0; j<nint; ++j)
        {
            FEFSIMaterialPoint* ppt = (el.GetMaterialPoint(j)->ExtractData<FEFSIMaterialPoint>());
            if (ppt) r += ppt->m_w;
        }
        r *= f;
        
        a << r;
    }
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidAcceleration::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    // write solid element data
    int N = dom.Elements();
    for (int i=0; i<N; ++i)
    {
        FEElement& el = dom.ElementRef(i);
        
        int nint = el.GaussPoints();
        double f = 1.0 / (double) nint;
        
        // since the PLOT file requires floats we need to convert
        // the doubles to single precision
        // we output the average acceleration values of the gauss points
        vec3d r = vec3d(0,0,0);
        for (int j=0; j<nint; ++j)
        {
            FEFluidMaterialPoint* ppt = (el.GetMaterialPoint(j)->ExtractData<FEFluidMaterialPoint>());
            if (ppt) r += ppt->m_aft;
        }
        r *= f;
        
        a << r;
    }
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidVorticity::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    // write solid element data
    int N = dom.Elements();
    for (int i=0; i<N; ++i)
    {
        FEElement& el = dom.ElementRef(i);
        
        int nint = el.GaussPoints();
        double f = 1.0 / (double) nint;
        
        // since the PLOT file requires floats we need to convert
        // the doubles to single precision
        // we output the average vorticity values of the gauss points
        vec3d r = vec3d(0,0,0);
        for (int j=0; j<nint; ++j)
        {
            FEFluidMaterialPoint* ppt = (el.GetMaterialPoint(j)->ExtractData<FEFluidMaterialPoint>());
            if (ppt) r += ppt->Vorticity();
        }
        r *= f;
        
        a << r;
    }
    
    return true;
}

//-----------------------------------------------------------------------------
//! Store the average stresses for each element.
bool FEPlotElementFluidStress::Save(FEDomain& dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    // write solid element data
    int N = dom.Elements();
    for (int i=0; i<N; ++i)
    {
        FEElement& el = dom.ElementRef(i);
        
        int nint = el.GaussPoints();
        double f = 1.0 / (double) nint;
        
        // since the PLOT file requires floats we need to convert
        // the doubles to single precision
        // we output the average stress values of the gauss points
		mat3ds s; s.zero();
        for (int j=0; j<nint; ++j)
        {
            FEFluidMaterialPoint* ppt = (el.GetMaterialPoint(j)->ExtractData<FEFluidMaterialPoint>());
            if (ppt) s += ppt->m_sf;
        }
		s *= f;
        
		a << s;
    }
    
    return true;
}

//-----------------------------------------------------------------------------
//! Store the average stresses for each element.
bool FEPlotElementFluidRateOfDef::Save(FEDomain& dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    // write solid element data
    int N = dom.Elements();
    for (int i=0; i<N; ++i)
    {
        FEElement& el = dom.ElementRef(i);
        
        int nint = el.GaussPoints();
        double f = 1.0 / (double) nint;
        
        // since the PLOT file requires floats we need to convert
        // the doubles to single precision
        // we output the average stress values of the gauss points
		mat3ds s; s.zero();
        for (int j=0; j<nint; ++j)
        {
            FEFluidMaterialPoint* ppt = (el.GetMaterialPoint(j)->ExtractData<FEFluidMaterialPoint>());
            if (ppt) s += ppt->RateOfDeformation();
        }
		s *= f;
        
		a << s;
    }
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidStressPowerDensity::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    // write solid element data
    int N = dom.Elements();
    for (int i=0; i<N; ++i)
    {
        FEElement& el = dom.ElementRef(i);
        
        int nint = el.GaussPoints();
        double f = 1.0 / (double) nint;
        
        // since the PLOT file requires floats we need to convert
        // the doubles to single precision
        // we output the average stress power values of the gauss points
        double r = 0;
        for (int j=0; j<nint; ++j)
        {
            FEMaterialPoint& mp = *el.GetMaterialPoint(j);
            FEFluidMaterialPoint* ppt = (mp.ExtractData<FEFluidMaterialPoint>());
            if (ppt) r += (ppt->m_sf*ppt->m_Lf).trace();
        }
        r *= f;
        
        a << r;
    }
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidHeatSupplyDensity::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    // write solid element data
    int N = dom.Elements();
    for (int i=0; i<N; ++i)
    {
        FEElement& el = dom.ElementRef(i);
        
        int nint = el.GaussPoints();
        double f = 1.0 / (double) nint;
        
        // since the PLOT file requires floats we need to convert
        // the doubles to single precision
        // we output the average stress power values of the gauss points
        double r = 0;
        if (pme) {
            for (int j=0; j<nint; ++j)
            {
                FEMaterialPoint& mp = *el.GetMaterialPoint(j);
                FEFluidMaterialPoint* ppt = (mp.ExtractData<FEFluidMaterialPoint>());
                if (ppt) r -= (pme->GetViscous()->Stress(mp)*ppt->m_Lf).trace();
            }
        }
        else {
            for (int j=0; j<nint; ++j)
            {
                FEMaterialPoint& mp = *el.GetMaterialPoint(j);
                FEFluidMaterialPoint* ppt = (mp.ExtractData<FEFluidMaterialPoint>());
                if (ppt) r -= (sme->Fluid()->GetViscous()->Stress(mp)*ppt->m_Lf).trace();
            }
        }
        r *= f;
        
        a << r;
    }
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidShearViscosity::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    // write solid element data
    int N = dom.Elements();
    for (int i=0; i<N; ++i)
    {
        FEElement& el = dom.ElementRef(i);
        
        int nint = el.GaussPoints();
        double f = 1.0 / (double) nint;
        
        // since the PLOT file requires floats we need to convert
        // the doubles to single precision
        // we output the average density values of the gauss points
        double r = 0;
        if (pme) {
            for (int j=0; j<nint; ++j)
            {
                FEMaterialPoint& mp = *el.GetMaterialPoint(j);
                FEFluidMaterialPoint* ppt = (mp.ExtractData<FEFluidMaterialPoint>());
                if (ppt) r += pme->GetViscous()->ShearViscosity(mp);
            }
        }
        else {
            for (int j=0; j<nint; ++j)
            {
                FEMaterialPoint& mp = *el.GetMaterialPoint(j);
                FEFluidMaterialPoint* ppt = (mp.ExtractData<FEFluidMaterialPoint>());
                if (ppt) r += sme->Fluid()->GetViscous()->ShearViscosity(mp);
            }
        }
        r *= f;
        
        a << r;
    }
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidStrainEnergyDensity::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    // write solid element data
    int N = dom.Elements();
    for (int i=0; i<N; ++i)
    {
        FEElement& el = dom.ElementRef(i);
        
        int nint = el.GaussPoints();
        double f = 1.0 / (double) nint;
        
        // since the PLOT file requires floats we need to convert
        // the doubles to single precision
        // we output the average density values of the gauss points
        double r = 0;
        if (pme) {
            for (int j=0; j<nint; ++j)
            {
                FEMaterialPoint& mp = *el.GetMaterialPoint(j);
                FEFluidMaterialPoint* ppt = (mp.ExtractData<FEFluidMaterialPoint>());
                if (ppt) r += pme->StrainEnergyDensity(mp);
            }
        }
        else {
            for (int j=0; j<nint; ++j)
            {
                FEMaterialPoint& mp = *el.GetMaterialPoint(j);
                FEFluidMaterialPoint* ppt = (mp.ExtractData<FEFluidMaterialPoint>());
                if (ppt) r += sme->Fluid()->StrainEnergyDensity(mp);
            }
        }
        r *= f;
        
        a << r;
    }
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidKineticEnergyDensity::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    // write solid element data
    int N = dom.Elements();
    for (int i=0; i<N; ++i)
    {
        FEElement& el = dom.ElementRef(i);
        
        int nint = el.GaussPoints();
        double f = 1.0 / (double) nint;
        
        // since the PLOT file requires floats we need to convert
        // the doubles to single precision
        // we output the average density values of the gauss points
        double r = 0;
        if (pme) {
            for (int j=0; j<nint; ++j)
            {
                FEMaterialPoint& mp = *el.GetMaterialPoint(j);
                FEFluidMaterialPoint* ppt = (mp.ExtractData<FEFluidMaterialPoint>());
                if (ppt) r += pme->KineticEnergyDensity(mp);
            }
        }
        else {
            for (int j=0; j<nint; ++j)
            {
                FEMaterialPoint& mp = *el.GetMaterialPoint(j);
                FEFluidMaterialPoint* ppt = (mp.ExtractData<FEFluidMaterialPoint>());
                if (ppt) r += sme->Fluid()->KineticEnergyDensity(mp);
            }
        }
        r *= f;
        
        a << r;
    }
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidEnergyDensity::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    // write solid element data
    int N = dom.Elements();
    for (int i=0; i<N; ++i)
    {
        FEElement& el = dom.ElementRef(i);
        
        int nint = el.GaussPoints();
        double f = 1.0 / (double) nint;
        
        // since the PLOT file requires floats we need to convert
        // the doubles to single precision
        // we output the average density values of the gauss points
        double r = 0;
        if (pme) {
            for (int j=0; j<nint; ++j)
            {
                FEMaterialPoint& mp = *el.GetMaterialPoint(j);
                FEFluidMaterialPoint* ppt = (mp.ExtractData<FEFluidMaterialPoint>());
                if (ppt) r += pme->EnergyDensity(mp);
            }
        }
        else {
            for (int j=0; j<nint; ++j)
            {
                FEMaterialPoint& mp = *el.GetMaterialPoint(j);
                FEFluidMaterialPoint* ppt = (mp.ExtractData<FEFluidMaterialPoint>());
                if (ppt) r += sme->Fluid()->EnergyDensity(mp);
            }
        }
        r *= f;
        
        a << r;
    }
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidElementStrainEnergy::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    if (dom.Class() == FE_DOMAIN_SOLID)
    {
        FESolidDomain& bd = static_cast<FESolidDomain&>(dom);
        for (int i=0; i<bd.Elements(); ++i)
        {
            FESolidElement& el = bd.Element(i);
            double* gw = el.GaussWeights();
            int nint = el.GaussPoints();
            
            // since the PLOT file requires floats we need to convert
            // the doubles to single precision
            // we output the average density values of the gauss points
            double r = 0;
            if (pme) {
                for (int j=0; j<nint; ++j)
                {
                    FEMaterialPoint& mp = *el.GetMaterialPoint(j);
                    FEFluidMaterialPoint* ppt = (mp.ExtractData<FEFluidMaterialPoint>());
                    if (ppt) {
                        double detJ = bd.detJ0(el, j)*gw[j];
                        r += pme->StrainEnergyDensity(mp)*detJ;
                    }
                }
            }
            else {
                for (int j=0; j<nint; ++j)
                {
                    FEMaterialPoint& mp = *el.GetMaterialPoint(j);
                    FEFluidMaterialPoint* ppt = (mp.ExtractData<FEFluidMaterialPoint>());
                    if (ppt) {
                        double detJ = bd.detJ0(el, j)*gw[j];
                        r += sme->Fluid()->StrainEnergyDensity(mp)*detJ;
                    }
                }
            }
            
            a << r;
        }
    }
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidElementKineticEnergy::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    if (dom.Class() == FE_DOMAIN_SOLID)
    {
        FESolidDomain& bd = static_cast<FESolidDomain&>(dom);
        for (int i=0; i<bd.Elements(); ++i)
        {
            FESolidElement& el = bd.Element(i);
            double* gw = el.GaussWeights();
            int nint = el.GaussPoints();
            
            // since the PLOT file requires floats we need to convert
            // the doubles to single precision
            // we output the average density values of the gauss points
            double r = 0;
            if (pme) {
                for (int j=0; j<nint; ++j)
                {
                    FEMaterialPoint& mp = *el.GetMaterialPoint(j);
                    FEFluidMaterialPoint* ppt = (mp.ExtractData<FEFluidMaterialPoint>());
                    if (ppt) {
                        double detJ = bd.detJ0(el, j)*gw[j];
                        r += pme->KineticEnergyDensity(mp)*detJ;
                    }
                }
            }
            else {
                for (int j=0; j<nint; ++j)
                {
                    FEMaterialPoint& mp = *el.GetMaterialPoint(j);
                    FEFluidMaterialPoint* ppt = (mp.ExtractData<FEFluidMaterialPoint>());
                    if (ppt) {
                        double detJ = bd.detJ0(el, j)*gw[j];
                        r += sme->Fluid()->KineticEnergyDensity(mp)*detJ;
                    }
                }
            }
            
            a << r;
        }
    }
    
    return true;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidElementCenterOfMass::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;

    if (pme) {
        double dens = pme->m_rhor;
        
        if (dom.Class() == FE_DOMAIN_SOLID)
        {
            FESolidDomain& bd = static_cast<FESolidDomain&>(dom);
            for (int i=0; i<bd.Elements(); ++i)
            {
                FESolidElement& el = bd.Element(i);
                double* gw = el.GaussWeights();
                
                // integrate zeroth and first mass moments
                vec3d ew = vec3d(0,0,0);
                double m = 0;
                for (int j=0; j<el.GaussPoints(); ++j)
                {
                    FEFluidMaterialPoint& pt = *(el.GetMaterialPoint(j)->ExtractData<FEFluidMaterialPoint>());
                    double detJ = bd.detJ0(el, j)*gw[j];
                    ew += pt.m_r0*(dens*detJ);
                    m += dens*detJ;
                }
                
                a << ew/m;
            }
            return true;
        }
    }
    else {
        double dens = sme->Fluid()->m_rhor;
        
        if (dom.Class() == FE_DOMAIN_SOLID)
        {
            FESolidDomain& bd = static_cast<FESolidDomain&>(dom);
            for (int i=0; i<bd.Elements(); ++i)
            {
                FESolidElement& el = bd.Element(i);
                double* gw = el.GaussWeights();
                
                // integrate zeroth and first mass moments
                vec3d ew = vec3d(0,0,0);
                double m = 0;
                for (int j=0; j<el.GaussPoints(); ++j)
                {
                    FEElasticMaterialPoint& pt = *(el.GetMaterialPoint(j)->ExtractData<FEElasticMaterialPoint>());
                    double detJ = bd.detJ0(el, j)*gw[j];
                    ew += pt.m_rt*(dens*detJ);
                    m += dens*detJ;
                }
                
                a << ew/m;
            }
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidElementLinearMomentum::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    double dens = (sme == 0) ? pme->m_rhor : sme->Fluid()->m_rhor;
    
    if (dom.Class() == FE_DOMAIN_SOLID)
    {
        FESolidDomain& bd = static_cast<FESolidDomain&>(dom);
        for (int i=0; i<bd.Elements(); ++i)
        {
            FESolidElement& el = bd.Element(i);
            double* gw = el.GaussWeights();
            
            // integrate zeroth and first mass moments
            vec3d ew = vec3d(0,0,0);
            for (int j=0; j<el.GaussPoints(); ++j)
            {
                FEFluidMaterialPoint& pt = *(el.GetMaterialPoint(j)->ExtractData<FEFluidMaterialPoint>());
                double detJ = bd.detJ0(el, j)*gw[j];
                ew += pt.m_vft*(dens*detJ);
            }
            
            a << ew;
        }
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
bool FEPlotFluidElementAngularMomentum::Save(FEDomain &dom, FEDataStream& a)
{
    FEFluid* pme = dynamic_cast<FEFluid*>(dom.GetMaterial());
    FEFluidFSI* sme = dynamic_cast<FEFluidFSI*>(dom.GetMaterial());
    if ((pme == 0) && (sme == 0)) return false;
    
    if (pme) {
        double dens = pme->m_rhor;
        
        if (dom.Class() == FE_DOMAIN_SOLID)
        {
            FESolidDomain& bd = static_cast<FESolidDomain&>(dom);
            for (int i=0; i<bd.Elements(); ++i)
            {
                FESolidElement& el = bd.Element(i);
                double* gw = el.GaussWeights();
                
                // integrate zeroth and first mass moments
                vec3d ew = vec3d(0,0,0);
                for (int j=0; j<el.GaussPoints(); ++j)
                {
                    FEFluidMaterialPoint& pt = *(el.GetMaterialPoint(j)->ExtractData<FEFluidMaterialPoint>());
                    double detJ = bd.detJ0(el, j)*gw[j];
                    ew += (pt.m_r0 ^ pt.m_vft)*(dens*detJ);
                }
                
                a << ew;
            }
            return true;
        }
    }
    else {
        double dens = sme->Fluid()->m_rhor;
        
        if (dom.Class() == FE_DOMAIN_SOLID)
        {
            FESolidDomain& bd = static_cast<FESolidDomain&>(dom);
            for (int i=0; i<bd.Elements(); ++i)
            {
                FESolidElement& el = bd.Element(i);
                double* gw = el.GaussWeights();
                
                // integrate zeroth and first mass moments
                vec3d ew = vec3d(0,0,0);
                for (int j=0; j<el.GaussPoints(); ++j)
                {
                    FEElasticMaterialPoint& et = *(el.GetMaterialPoint(j)->ExtractData<FEElasticMaterialPoint>());
                    FEFluidMaterialPoint& pt = *(el.GetMaterialPoint(j)->ExtractData<FEFluidMaterialPoint>());
                    double detJ = bd.detJ0(el, j)*gw[j];
                    ew += (et.m_rt ^ pt.m_vft)*(dens*detJ);
                }
                
                a << ew;
            }
            return true;
        }
    }
    return false;
}
