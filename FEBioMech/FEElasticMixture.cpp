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
#include "FEElasticMixture.h"
#include "FECore/FECoreKernel.h"

//-----------------------------------------------------------------------------
FEElasticMixtureMaterialPoint::FEElasticMixtureMaterialPoint() : FEMaterialPointArray(new FEElasticMaterialPoint)
{ 
}

//-----------------------------------------------------------------------------
FEMaterialPoint* FEElasticMixtureMaterialPoint::Copy()
{
	FEElasticMixtureMaterialPoint* pt = new FEElasticMixtureMaterialPoint;
	pt->m_w = m_w;
	pt->m_mp = m_mp;
	if (m_pNext) pt->m_pNext = m_pNext->Copy();
	return pt;
}

//-----------------------------------------------------------------------------
void FEElasticMixtureMaterialPoint::Init()
{
	FEElasticMaterialPoint& pt = *(ExtractData<FEElasticMaterialPoint>());
	pt.Init();

	// allocate weight array
	m_w.resize(Components(), 1.0);

	// don't forget to initialize the base class
    FEMaterialPointArray::Init();
}

//-----------------------------------------------------------------------------
void FEElasticMixtureMaterialPoint::Serialize(DumpStream& ar)
{
	if (ar.IsSaving())
	{
		ar << m_w;
	}
	else
	{
		ar >> m_w;
	}
   
	FEMaterialPointArray::Serialize(ar);
}

//=============================================================================
//								FEElasticMixture
//=============================================================================

//-----------------------------------------------------------------------------
FEElasticMixture::FEElasticMixture(FEModel* pfem) : FEElasticMaterial(pfem)
{
	AddProperty(&m_pMat, "solid");
}

//-----------------------------------------------------------------------------
FEMaterialPoint* FEElasticMixture::CreateMaterialPointData() 
{ 
	FEElasticMixtureMaterialPoint* pt = new FEElasticMixtureMaterialPoint();
	pt->SetName(m_pMat.GetName());
	int NMAT = Materials();
	for (int i=0; i<NMAT; ++i) 
	{
		FEMaterialPoint* pi = m_pMat[i]->CreateMaterialPointData();
		pt->AddMaterialPoint(pi);
	}
	return pt;
}

//-----------------------------------------------------------------------------
void FEElasticMixture::SetLocalCoordinateSystem(FEElement& el, int n, FEMaterialPoint& mp)
{
	FEElasticMaterial::SetLocalCoordinateSystem(el, n, mp);
    FEElasticMaterialPoint& pt = *(mp.ExtractData<FEElasticMaterialPoint>());
	FEElasticMixtureMaterialPoint& mmp = *(mp.ExtractData<FEElasticMixtureMaterialPoint>());
    
	// check the local coordinate systems for each component
	for (int j=0; j<Materials(); ++j)
	{
		FEElasticMaterial* pmj = GetMaterial(j)->GetElasticMaterial();
		FEMaterialPoint& mpj = *mmp.GetPointData(j);
        FEElasticMaterialPoint& pj = *(mpj.ExtractData<FEElasticMaterialPoint>());
		pj.m_Q = pt.m_Q;
		pmj->SetLocalCoordinateSystem(el, n, mpj);
	}
}

//-----------------------------------------------------------------------------
void FEElasticMixture::AddMaterial(FEElasticMaterial* pm) 
{ 
	m_pMat.SetProperty(pm); 
}

//-----------------------------------------------------------------------------
//! This function evaluates the stress at the material point by evaluating the
//! individual stress components. 
mat3ds FEElasticMixture::Stress(FEMaterialPoint& mp)
{
	FEElasticMixtureMaterialPoint& pt = *mp.ExtractData<FEElasticMixtureMaterialPoint>();
	vector<double>& w = pt.m_w;
	assert(w.size() == m_pMat.size());

	// get the elastic material point
	FEElasticMaterialPoint& ep = *mp.ExtractData<FEElasticMaterialPoint>();

	// calculate stress
	mat3ds s; s.zero();
	for (int i=0; i < (int) m_pMat.size(); ++i)
	{
		// copy the elastic material point data to the components
        // but don't copy m_Q since correct value was set in SetLocalCoordinateSystem
		FEElasticMaterialPoint& epi = *pt.GetPointData(i)->ExtractData<FEElasticMaterialPoint>();
		epi.m_rt = ep.m_rt;
		epi.m_r0 = ep.m_r0;
		epi.m_F = ep.m_F;
		epi.m_J = ep.m_J;

		s += epi.m_s = m_pMat[i]->Stress(*pt.GetPointData(i))*w[i];
	}

	return s;
}

//-----------------------------------------------------------------------------
tens4ds FEElasticMixture::Tangent(FEMaterialPoint& mp)
{
	FEElasticMixtureMaterialPoint& pt = *mp.ExtractData<FEElasticMixtureMaterialPoint>();
	vector<double>& w = pt.m_w;
	assert(w.size() == m_pMat.size());

	// get the elastic material point
	FEElasticMaterialPoint& ep = *mp.ExtractData<FEElasticMaterialPoint>();

	// calculate elasticity tensor
	tens4ds c(0.);
	for (int i=0; i < (int) m_pMat.size(); ++i)
	{
		// copy the elastic material point data to the components
        // but don't copy m_Q since correct value was set in SetLocalCoordinateSystem
		FEElasticMaterialPoint& epi = *pt.GetPointData(i)->ExtractData<FEElasticMaterialPoint>();
		epi.m_rt = ep.m_rt;
		epi.m_r0 = ep.m_r0;
		epi.m_F = ep.m_F;
		epi.m_J = ep.m_J;
        
		c += m_pMat[i]->Tangent(*pt.GetPointData(i))*w[i];
	}

	return c;
}

//-----------------------------------------------------------------------------
//! This function evaluates the stress at the material point by evaluating the
//! individual stress components.
double FEElasticMixture::StrainEnergyDensity(FEMaterialPoint& mp)
{
	FEElasticMixtureMaterialPoint& pt = *mp.ExtractData<FEElasticMixtureMaterialPoint>();
	vector<double>& w = pt.m_w;
	assert(w.size() == m_pMat.size());
    
	// get the elastic material point
	FEElasticMaterialPoint& ep = *mp.ExtractData<FEElasticMaterialPoint>();
    
	// calculate strain energy density
	double sed = 0.0;
	for (int i=0; i < (int) m_pMat.size(); ++i)
	{
		// copy the elastic material point data to the components
        // but don't copy m_Q since correct value was set in SetLocalCoordinateSystem
		FEElasticMaterialPoint& epi = *pt.GetPointData(i)->ExtractData<FEElasticMaterialPoint>();
		epi.m_rt = ep.m_rt;
		epi.m_r0 = ep.m_r0;
		epi.m_F = ep.m_F;
		epi.m_J = ep.m_J;
        
		sed += m_pMat[i]->StrainEnergyDensity(*pt.GetPointData(i))*w[i];
	}
    
	return sed;
}

//-----------------------------------------------------------------------------
//! specialized material points
void FEElasticMixture::UpdateSpecializedMaterialPoints(FEMaterialPoint& mp, const FETimeInfo& tp)
{
    FEElasticMixtureMaterialPoint& pt = *mp.ExtractData<FEElasticMixtureMaterialPoint>();
    
    for (int i=0; i < (int) m_pMat.size(); ++i)
    {
        FEElasticMaterialPoint& epi = *pt.GetPointData(i)->ExtractData<FEElasticMaterialPoint>();
        FEMaterial* pmj = GetMaterial(i);
        pmj->UpdateSpecializedMaterialPoints(epi, tp);
    }
    
}
