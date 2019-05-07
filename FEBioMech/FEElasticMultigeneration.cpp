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
#include "FECore/tens4d.h"
#include "FECore/log.h"
#include "FECore/FECoreKernel.h"
#include "FEElasticMultigeneration.h"

//=============================================================================
// define the material parameters
BEGIN_PARAMETER_LIST(FEGenerationMaterial, FEElasticMaterial)
	ADD_PARAMETER(btime, FE_PARAM_DOUBLE, "start_time");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FEGenerationMaterial::FEGenerationMaterial(FEModel* pfem) : FEElasticMaterial(pfem)
{
	AddProperty(&m_pMat, "solid");
}

//-----------------------------------------------------------------------------
//! calculate stress at material point
mat3ds FEGenerationMaterial::Stress(FEMaterialPoint& pt)
{
	return m_pMat->Stress(pt);
}
		
//-----------------------------------------------------------------------------
//! calculate tangent stiffness at material point
tens4ds FEGenerationMaterial::Tangent(FEMaterialPoint& pt)
{
	return m_pMat->Tangent(pt);
}

//-----------------------------------------------------------------------------
//! calculate strain energy density at material point
double FEGenerationMaterial::StrainEnergyDensity(FEMaterialPoint& pt)
{
	return m_pMat->StrainEnergyDensity(pt);
}

//=============================================================================
FEMultigenerationMaterialPoint::FEMultigenerationMaterialPoint() : FEMaterialPoint(new FEElasticMaterialPoint)
{
    m_tgen = 0.0;
    m_ngen = 1;     // the first generation is always active
}

//-----------------------------------------------------------------------------
void FEMultigenerationMaterialPoint::AddMaterialPoint(FEMaterialPoint* pt)
{
	m_mp.push_back(pt);
	pt->SetPrev(this);
}

//-----------------------------------------------------------------------------
FEMaterialPoint* FEMultigenerationMaterialPoint::Copy()
{
	FEMultigenerationMaterialPoint* pt = new FEMultigenerationMaterialPoint(*this);
    pt->m_mp = m_mp;
	pt->m_pmat = m_pmat;
    pt->m_tgen = m_tgen;
	if (m_pNext) pt->m_pNext = m_pNext->Copy();
	return pt;
}

//-----------------------------------------------------------------------------
void FEMultigenerationMaterialPoint::Serialize(DumpStream& ar)
{
	if (ar.IsShallow())
	{
		if (ar.IsSaving())
		{
			ar << m_tgen << m_ngen;
		}
		else
		{
			ar >> m_tgen >> m_ngen;
		}
		for (int i=0; i < (int)m_mp.size(); i++) m_mp[i]->Serialize(ar);
    
		// TODO: shallow copy m_pmat
	}
	else
	{
		if (ar.IsSaving())
		{
			ar << m_tgen << m_ngen;
			ar << (int)m_mp.size();
			for (int i=0; i < (int)m_mp.size(); i++) m_mp[i]->Serialize(ar);
		}
		else
		{
			ar >> m_tgen >> m_ngen;
			int mp_size;
			ar >> mp_size;
			m_mp.resize(mp_size);
			for (int i=0; i < mp_size; i++)
			{
				m_mp[i] = new FEElasticMaterialPoint;
				m_mp[i]->Serialize(ar);
			}
		}
	}
	FEMaterialPoint::Serialize(ar);
}

//-----------------------------------------------------------------------------
void FEMultigenerationMaterialPoint::Init()
{
    FEMaterialPoint::Init();
    for (int i=0; i<(int)m_mp.size(); ++i) m_mp[i]->Init();

	m_tgen = 0.0;
	m_ngen = 1;
}

//-----------------------------------------------------------------------------
void FEMultigenerationMaterialPoint::Update(const FETimeInfo& timeInfo)
{
    FEMaterialPoint::Update(timeInfo);
    for (int i=0; i<(int)m_mp.size(); ++i) m_mp[i]->Update(timeInfo);

	// get the time
	double t = timeInfo.currentTime;

	// Check if this constitutes a new generation
	int igen = m_pmat->CheckGeneration(t);
	t = m_pmat->m_MG[igen]->btime;
	if (t>m_tgen)
	{
		FEElasticMaterialPoint& pt = *((*this).ExtractData<FEElasticMaterialPoint>());
					
		// push back F and J to define relative deformation gradient of this generation
		mat3d F = pt.m_F;
		double J = pt.m_J;
        FEElasticMaterialPoint& pe = *(m_mp[m_ngen]->ExtractData<FEElasticMaterialPoint>());
        pe.m_F = F.inverse();
        pe.m_J = 1.0/J;
		m_tgen = t;
        ++m_ngen;
	}
}

//=============================================================================

//-----------------------------------------------------------------------------
FEElasticMultigeneration::FEElasticMultigeneration(FEModel* pfem) : FEElasticMaterial(pfem)
{
	AddProperty(&m_MG, "generation");
}

//-----------------------------------------------------------------------------
// returns a pointer to a new material point object
FEMaterialPoint* FEElasticMultigeneration::CreateMaterialPointData()
{
    // use the zero-th generation material point as the base elastic material point
    FEMultigenerationMaterialPoint* pt = new FEMultigenerationMaterialPoint();
    pt->m_pmat = this;
    int NMAT = Materials();
    for (int i=0; i<NMAT; ++i) pt->AddMaterialPoint(m_MG[i]->CreateMaterialPointData());
    return pt;
}

//-----------------------------------------------------------------------------
void FEElasticMultigeneration::SetLocalCoordinateSystem(FEElement& el, int n, FEMaterialPoint& mp)
{
    FEElasticMaterial::SetLocalCoordinateSystem(el, n, mp);
    FEElasticMaterialPoint& pt = *(mp.ExtractData<FEElasticMaterialPoint>());
    
    // check the local coordinate systems for each component
    for (int j=0; j<Materials(); ++j)
    {
        FEElasticMaterial* pmj = GetMaterial(j)->GetElasticMaterial();
        FEMaterialPoint& mpj = *mp.GetPointData(j);
        FEElasticMaterialPoint& pj = *(mpj.ExtractData<FEElasticMaterialPoint>());
        pj.m_Q = pt.m_Q;    // copy mixture material's coordinate system into component
        pmj->SetLocalCoordinateSystem(el, n, mpj);
    }
}

//--------------------------------------------------------------------------------
// Check if time t constitutes a new generation and return that generation
int FEElasticMultigeneration::CheckGeneration(const double t)
{
	int ngen = (int)m_MG.size();
	for (int igen=1; igen<ngen; ++igen)
	{
		if (t < m_MG[igen]->btime) return igen - 1;
	}
	return ngen - 1;
}

//-----------------------------------------------------------------------------
mat3ds FEElasticMultigeneration::Stress(FEMaterialPoint& mp)
{
	FEMultigenerationMaterialPoint& pt = *mp.ExtractData<FEMultigenerationMaterialPoint>();
	FEElasticMaterialPoint& ep = *mp.ExtractData<FEElasticMaterialPoint>();

	mat3ds s;
	
	// calculate stress
	s.zero();
	
	// extract deformation gradient
	mat3d Fs = ep.m_F;
	double Js = ep.m_J;
	
	for (int i=0; i < pt.m_ngen; ++i)
	{
        // evaluate deformation gradient for this generation
        FEElasticMaterialPoint& epi = *pt.m_mp[i]->ExtractData<FEElasticMaterialPoint>();
        
        // store safe copies of Fi and Ji for this generation
        mat3d Fi = epi.m_F;
        double Ji = epi.m_J;
        
        // copy the elastic material point data to the components
        // but don't copy m_Q since correct value was set in SetLocalCoordinateSystem
        epi.m_rt = ep.m_rt;
        epi.m_r0 = ep.m_r0;
        
        // evaluate relative deformation gradient
       	epi.m_F = Fs*Fi;
        epi.m_J = Js*Ji;
        
        // evaluate stress for this generation
        s += epi.m_s = Ji*m_MG[i]->Stress(*pt.m_mp[i]);
        
        // restore the material point deformation gradient
        epi.m_F = Fi;
        epi.m_J = Ji;
	}
    
	return s;
}

//-----------------------------------------------------------------------------
tens4ds FEElasticMultigeneration::Tangent(FEMaterialPoint& mp)
{
	FEMultigenerationMaterialPoint& pt = *mp.ExtractData<FEMultigenerationMaterialPoint>();
	FEElasticMaterialPoint& ep = *mp.ExtractData<FEElasticMaterialPoint>();
	
	tens4ds c(0.);
	
	// extract deformation gradient
	mat3d Fs = ep.m_F;
	double Js = ep.m_J;
	
	for (int i=0; i < pt.m_ngen; ++i)
	{
		// evaluate deformation gradient for this generation
        FEElasticMaterialPoint& epi = *pt.m_mp[i]->ExtractData<FEElasticMaterialPoint>();
        
        // store safe copies of Fi and Ji for this generation
        mat3d Fi = epi.m_F;
        double Ji = epi.m_J;
        
        // copy the elastic material point data to the components
        // but don't copy m_Q since correct value was set in SetLocalCoordinateSystem
        epi.m_rt = ep.m_rt;
        epi.m_r0 = ep.m_r0;
        
        // evaluate relative deformation gradient
       	epi.m_F = Fs*Fi;
        epi.m_J = Js*Ji;
        
		// evaluate tangent for this generation
		c += Ji*m_MG[i]->Tangent(*pt.m_mp[i]);

        // restore the material point deformation gradient
        epi.m_F = Fi;
        epi.m_J = Ji;
	}
	
	return c;
}

//-----------------------------------------------------------------------------
double FEElasticMultigeneration::StrainEnergyDensity(FEMaterialPoint& mp)
{
	FEMultigenerationMaterialPoint& pt = *mp.ExtractData<FEMultigenerationMaterialPoint>();
	FEElasticMaterialPoint& ep = *mp.ExtractData<FEElasticMaterialPoint>();
    
	double sed = 0.0;
	
	// extract deformation gradient
	mat3d Fs = ep.m_F;
	double Js = ep.m_J;
	
	for (int i=0; i < pt.m_ngen; ++i)
	{
        // evaluate deformation gradient for this generation
        FEElasticMaterialPoint& epi = *pt.m_mp[i]->ExtractData<FEElasticMaterialPoint>();
        
        // store safe copies of Fi and Ji for this generation
        mat3d Fi = epi.m_F;
        double Ji = epi.m_J;
        
        // copy the elastic material point data to the components
        // but don't copy m_Q since correct value was set in SetLocalCoordinateSystem
        epi.m_rt = ep.m_rt;
        epi.m_r0 = ep.m_r0;
        
        // evaluate relative deformation gradient
       	epi.m_F = Fs*Fi;
        epi.m_J = Js*Ji;
        
        // evaluate strain energy density for this generation
//        sed += Ji*m_MG[i]->StrainEnergyDensity(*pt.m_mp[i]);
        double dsed = Ji*m_MG[i]->StrainEnergyDensity(*pt.m_mp[i]);
        sed += dsed;
        
        // restore the material point deformation gradient
        epi.m_F = Fi;
        epi.m_J = Ji;
	}
     
	return sed;
}
