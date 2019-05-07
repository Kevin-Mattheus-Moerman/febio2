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
#include "FEFluid.h"
#include "FECore/FECoreKernel.h"

// define the material parameters
BEGIN_PARAMETER_LIST(FEFluid, FEMaterial)
    ADD_PARAMETER2(m_rhor, FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "density");
    ADD_PARAMETER2(m_k, FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "k");
END_PARAMETER_LIST();

//============================================================================
// FEFluidMaterialPoint
//============================================================================
//-----------------------------------------------------------------------------
FEFluidMaterialPoint::FEFluidMaterialPoint(FEMaterialPoint* pt) : FEMaterialPoint(pt)
{
    m_pf = 0;
    m_Lf.zero();
    m_Jf = 1;
    m_Jfdot = 0;
    m_vft = m_aft = m_gradJf = vec3d(0,0,0);
    m_sf.zero();
}

//-----------------------------------------------------------------------------
FEMaterialPoint* FEFluidMaterialPoint::Copy()
{
	FEFluidMaterialPoint* pt = new FEFluidMaterialPoint(*this);
	if (m_pNext) pt->m_pNext = m_pNext->Copy();
	return pt;
}

//-----------------------------------------------------------------------------
void FEFluidMaterialPoint::Serialize(DumpStream& ar)
{
	if (ar.IsSaving())
	{
		ar << m_pf << m_Lf << m_Jf << m_Jfdot << m_gradJf << m_vft << m_aft << m_sf;
	}
	else
	{
		ar >> m_pf >> m_Lf >> m_Jf >> m_Jfdot >> m_gradJf >> m_vft >> m_aft >> m_sf;
	}

	if (m_pNext) m_pNext->Serialize(ar);
}

//-----------------------------------------------------------------------------
void FEFluidMaterialPoint::Init()
{
	m_pf = 0;
	m_Lf.zero();
	m_Jf = 1;
	m_Jfdot = 0;
	m_vft = m_aft = m_gradJf = vec3d(0,0,0);
	m_sf.zero();
    
    // don't forget to initialize the base class
    FEMaterialPoint::Init();
}

//============================================================================
// FEFluid
//============================================================================

//-----------------------------------------------------------------------------
//! FEFluid constructor

FEFluid::FEFluid(FEModel* pfem) : FEMaterial(pfem)
{ 
	m_rhor = 0;
    m_k = 0;

	// set material properties
	AddProperty(&m_pViscous ,"viscous"  );
}

//-----------------------------------------------------------------------------
//! initialization
bool FEFluid::Init()
{
	m_Tr = GetFEModel()->GetGlobalConstant("T");
	return FEMaterial::Init();
}

//-----------------------------------------------------------------------------
//! returns a pointer to a new material point object
FEMaterialPoint* FEFluid::CreateMaterialPointData()
{
	return new FEFluidMaterialPoint();
}

//-----------------------------------------------------------------------------
//! calculate current fluid density
double FEFluid::Density(FEMaterialPoint& pt)
{
    FEFluidMaterialPoint& vt = *pt.ExtractData<FEFluidMaterialPoint>();
    return m_rhor/vt.m_Jf;
}

//-----------------------------------------------------------------------------
//! bulk modulus
double FEFluid::BulkModulus(FEMaterialPoint& mp)
{
    FEFluidMaterialPoint& vt = *mp.ExtractData<FEFluidMaterialPoint>();
    return -vt.m_Jf*Tangent_Pressure_Strain(mp);
}

//-----------------------------------------------------------------------------
//! elastic pressure
double FEFluid::Pressure(FEMaterialPoint& mp)
{
    FEFluidMaterialPoint& fp = *mp.ExtractData<FEFluidMaterialPoint>();
    double e = fp.m_Jf - 1;

    return Pressure(e);
}

//-----------------------------------------------------------------------------
//! elastic pressure from dilatation
double FEFluid::Pressure(const double e)
{
    return -m_k*e;
}

//-----------------------------------------------------------------------------
//! The stress of a fluid material is the sum of the fluid pressure
//! and the viscous stress.

mat3ds FEFluid::Stress(FEMaterialPoint& mp)
{
	// calculate solid material stress
	mat3ds s = m_pViscous->Stress(mp);
    
    double p = Pressure(mp);
	
	// add fluid pressure
	s.xx() -= p;
	s.yy() -= p;
	s.zz() -= p;
	
	return s;
}

//-----------------------------------------------------------------------------
//! The tangent of stress with respect to strain J of a fluid material is the
//! sum of the tangent of the fluid pressure and that of the viscous stress.

mat3ds FEFluid::Tangent_Strain(FEMaterialPoint& mp)
{
    // get tangent of viscous stress
    mat3ds sJ = m_pViscous->Tangent_Strain(mp);
    
    // add tangent of fluid pressure
    double dp = Tangent_Pressure_Strain(mp);
    sJ.xx() -= dp;
    sJ.yy() -= dp;
    sJ.zz() -= dp;
    
    return sJ;
}

//-----------------------------------------------------------------------------
//! calculate current fluid kinematic viscosity
double FEFluid::KinematicViscosity(FEMaterialPoint& mp)
{
    return m_pViscous->ShearViscosity(mp)/Density(mp);
}

//-----------------------------------------------------------------------------
//! calculate current acoustic speed
double FEFluid::AcousticSpeed(FEMaterialPoint& mp)
{
    double c = sqrt(BulkModulus(mp)/Density(mp));
    
    return c;
}

//-----------------------------------------------------------------------------
//! calculate strain energy density (per reference volume)
double FEFluid::StrainEnergyDensity(FEMaterialPoint& mp)
{
    FEFluidMaterialPoint& fp = *mp.ExtractData<FEFluidMaterialPoint>();
    double sed = m_k*pow(fp.m_Jf-1,2)/2;
    return sed;
}

//-----------------------------------------------------------------------------
//! calculate kinetic energy density (per reference volume)
double FEFluid::KineticEnergyDensity(FEMaterialPoint& mp)
{
    FEFluidMaterialPoint& fp = *mp.ExtractData<FEFluidMaterialPoint>();
    double ked = m_rhor*(fp.m_vft*fp.m_vft)/2;
    return ked;
}

//-----------------------------------------------------------------------------
//! calculate strain + kinetic energy density (per reference volume)
double FEFluid::EnergyDensity(FEMaterialPoint& mp)
{
    return StrainEnergyDensity(mp) + KineticEnergyDensity(mp);
}

//-----------------------------------------------------------------------------
//! invert pressure-dilatation relation
double FEFluid::Dilatation(const double p)
{
    return -p/m_k;
}

