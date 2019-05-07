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
#include "FEViscoElasticMaterial.h"
#include "FEUncoupledMaterial.h"
#include "FECore/FECoreKernel.h"
#include <FECore/FEModel.h>

//-----------------------------------------------------------------------------
// define the material parameters
BEGIN_PARAMETER_LIST(FEViscoElasticMaterial, FEElasticMaterial)
	ADD_PARAMETER(m_t[0], FE_PARAM_DOUBLE, "t1");
	ADD_PARAMETER(m_t[1], FE_PARAM_DOUBLE, "t2");
	ADD_PARAMETER(m_t[2], FE_PARAM_DOUBLE, "t3");
	ADD_PARAMETER(m_t[3], FE_PARAM_DOUBLE, "t4");
	ADD_PARAMETER(m_t[4], FE_PARAM_DOUBLE, "t5");
	ADD_PARAMETER(m_t[5], FE_PARAM_DOUBLE, "t6");
	ADD_PARAMETER(m_g0  , FE_PARAM_DOUBLE, "g0");
	ADD_PARAMETER(m_g[0], FE_PARAM_DOUBLE, "g1");
	ADD_PARAMETER(m_g[1], FE_PARAM_DOUBLE, "g2");
	ADD_PARAMETER(m_g[2], FE_PARAM_DOUBLE, "g3");
	ADD_PARAMETER(m_g[3], FE_PARAM_DOUBLE, "g4");
	ADD_PARAMETER(m_g[4], FE_PARAM_DOUBLE, "g5");
	ADD_PARAMETER(m_g[5], FE_PARAM_DOUBLE, "g6");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
//! Create a shallow copy of the material point data
FEMaterialPoint* FEViscoElasticMaterialPoint::Copy()
{
	FEViscoElasticMaterialPoint* pt = new FEViscoElasticMaterialPoint(*this);
	if (m_pNext) pt->m_pNext = m_pNext->Copy();
	return pt;
}

//-----------------------------------------------------------------------------
//! Initializes material point data.
void FEViscoElasticMaterialPoint::Init()
{
	FEElasticMaterialPoint& pt = *m_pNext->ExtractData<FEElasticMaterialPoint>();

	// intialize data to zero
	m_se.zero();
	m_Sep.zero();
//	m_sed = 0.0;
//  m_sedp = 0.0;
	for (int i=0; i<MAX_TERMS; ++i) {
		m_H[i].zero();
		m_Hp[i].zero();
//      m_Hsed[i] = 0;
//      m_Hsedp[i] = 0;
	}

    // don't forget to initialize the base class
    FEMaterialPoint::Init();
}

//-----------------------------------------------------------------------------
//! Update material point data.
void FEViscoElasticMaterialPoint::Update(const FETimeInfo& timeInfo)
{
	FEElasticMaterialPoint& pt = *m_pNext->ExtractData<FEElasticMaterialPoint>();

	// the elastic stress stored in pt is the Cauchy stress.
	// however, we need to store the 2nd PK stress
	m_Sep = pt.pull_back(m_se);
//  m_sedp = m_sed;

	// copy previous data
	for (int i=0; i<MAX_TERMS; ++i) {
		m_Hp[i] = m_H[i];
//      m_Hsedp[i] = m_Hsed[i];
    }

    // don't forget to call the base class
    FEMaterialPoint::Update(timeInfo);
}

//-----------------------------------------------------------------------------
//! Serialize data to the archive
void FEViscoElasticMaterialPoint::Serialize(DumpStream& ar)
{
	FEMaterialPoint::Serialize(ar);

	if (ar.IsSaving())
	{
		ar << m_se;
		ar << m_Sep;
		for (int i=0; i<MAX_TERMS; ++i) ar << m_H[i] << m_Hp[i];
	}
	else
	{
		ar >> m_se;
		ar >> m_Sep;
		for (int i=0; i<MAX_TERMS; ++i) ar >> m_H[i] >> m_Hp[i];
	}
}

//-----------------------------------------------------------------------------
//! constructor
FEViscoElasticMaterial::FEViscoElasticMaterial(FEModel* pfem) : FEElasticMaterial(pfem)
{
	m_g0 = 1;
	for (int i=0; i<MAX_TERMS; ++i)
	{
		m_t[i] = 1;
		m_g[i] = 0;
	}

	// define the material properties
	AddProperty(&m_Base, "elastic");
}

//-----------------------------------------------------------------------------
//! get the elastic base material \todo I want to call this GetElasticMaterial, but this name is being used
FEElasticMaterial* FEViscoElasticMaterial::GetBaseMaterial()
{ 
	return m_Base; 
}

//-----------------------------------------------------------------------------
//! Set the base material
void FEViscoElasticMaterial::SetBaseMaterial(FEElasticMaterial* pbase)
{ 
	m_Base = pbase; 
}

//-----------------------------------------------------------------------------
void FEViscoElasticMaterial::SetLocalCoordinateSystem(FEElement& el, int n, FEMaterialPoint& mp)
{
	FEElasticMaterial::SetLocalCoordinateSystem(el, n, mp);
	FEElasticMaterial* pme2 = GetBaseMaterial();
	pme2->SetLocalCoordinateSystem(el, n, mp);
}

//-----------------------------------------------------------------------------
//! Create material point data for this material
FEMaterialPoint* FEViscoElasticMaterial::CreateMaterialPointData()
{ 
	return new FEViscoElasticMaterialPoint(m_Base->CreateMaterialPointData());
}

//-----------------------------------------------------------------------------
//! Stress function
mat3ds FEViscoElasticMaterial::Stress(FEMaterialPoint& mp)
{
	double dt = GetFEModel()->GetTime().timeIncrement;
	if (dt == 0) return mat3ds(0, 0, 0, 0, 0, 0);
    
	// get the elastic part
	FEElasticMaterialPoint& ep = *mp.ExtractData<FEElasticMaterialPoint>();

	// get the viscoelastic point data
	FEViscoElasticMaterialPoint& pt = *mp.ExtractData<FEViscoElasticMaterialPoint>();

	// Calculate the new elastic Cauchy stress
	pt.m_se = m_Base->Stress(mp);

	// pull-back to get PK2 stress
	mat3ds Se = ep.pull_back(pt.m_se);

	// get elastic PK2 stress of previous timestep
	mat3ds Sep = pt.m_Sep;

	// calculate new history variables
	// terms are accumulated in S, the total PK2-stress
	mat3ds S = Se*m_g0;
	double g, h;
	for (int i=0; i<MAX_TERMS; ++i)
	{
		g = exp(-dt/m_t[i]);
		h = (1 - g)/(dt/m_t[i]);

		pt.m_H[i] = pt.m_Hp[i]*g + (Se - Sep)*h;
		S += pt.m_H[i]*m_g[i];
	}

	// return the total Cauchy stress,
	// which is the push-forward of S
	return ep.push_forward(S);
}

//-----------------------------------------------------------------------------
//! Material tangent
tens4ds FEViscoElasticMaterial::Tangent(FEMaterialPoint& pt)
{
	double dt = GetFEModel()->GetTime().timeIncrement;

	// calculate the spatial elastic tangent
	tens4ds C = m_Base->Tangent(pt);
	if (dt == 0.0) return C;

	// calculate the visco scale factor
	double f = m_g0, g, h;
	for (int i=0; i<MAX_TERMS; ++i)
	{
		g = exp(-dt/m_t[i]);
		h = ( 1 - exp(-dt/m_t[i]) )/( dt/m_t[i] );
		f += m_g[i]*h; 
	}

	// multiply tangent with visco-factor
	return C*f;
}

//-----------------------------------------------------------------------------
//! Strain energy density function
double FEViscoElasticMaterial::StrainEnergyDensity(FEMaterialPoint& mp)
{
/*    if (mp.dt == 0) return 0;
    
	// get the viscoelastic point data
	FEViscoElasticMaterialPoint& pt = *mp.ExtractData<FEViscoElasticMaterialPoint>();
    
	// Calculate the new elastic strain energy density
	pt.m_sed = m_pBase->StrainEnergyDensity(mp);
    double sed = pt.m_sed;
    
	// get elastic strain energy density of previous timestep
	double sedp = pt.m_sedp;
    
	// calculate new history variables
	// terms are accumulated in sedt, the total strain energy density
	double sedt = sed*m_g0;
	double dt = mp.dt, g, h;
	for (int i=0; i<MAX_TERMS; ++i)
	{
		g = exp(-dt/m_t[i]);
		h = (1 - g)/(dt/m_t[i]);
        
		pt.m_Hsed[i] = pt.m_Hsedp[i]*g + (sed - sedp)*h;
		sedt += pt.m_Hsed[i]*m_g[i];
	}
    
	// return the total strain energy density
	return sedt; */
    return 0;
}
