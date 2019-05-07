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
#include "FEContinuousFiberDistribution.h"

//-----------------------------------------------------------------------------
FEContinuousFiberDistribution::FEContinuousFiberDistribution(FEModel* pfem) : FEElasticMaterial(pfem)
{
	m_IFD = 0.0;

	// set material properties
	AddProperty(&m_pFmat, "fibers"      );
	AddProperty(&m_pFDD , "distribution");
	AddProperty(&m_pFint, "scheme"      );
}

//-----------------------------------------------------------------------------
FEContinuousFiberDistribution::~FEContinuousFiberDistribution() {}

//-----------------------------------------------------------------------------
bool FEContinuousFiberDistribution::Init()
{
    // initialize base class
	if (FEElasticMaterial::Init() == false) return false;

	// calculate the integrated fiber density
	IntegrateFiberDensity();

	return true;
}

//-----------------------------------------------------------------------------
// returns a pointer to a new material point object
FEMaterialPoint* FEContinuousFiberDistribution::CreateMaterialPointData()
{
	FEMaterialPoint* mp = m_pFmat->CreateMaterialPointData();
	mp->SetName(m_pFmat.GetName());
	return mp;
}

//-----------------------------------------------------------------------------
//! Serialization
void FEContinuousFiberDistribution::Serialize(DumpStream& ar)
{	
	FEElasticMaterial::Serialize(ar);

	if (ar.IsShallow()) return;

	if (ar.IsSaving())
	{
		ar << m_IFD;
	}
	else
	{
		ar >> m_IFD;
	}
}

//-----------------------------------------------------------------------------
//! calculate stress at material point
//mat3ds FEContinuousFiberDistribution::Stress(FEMaterialPoint& pt) { return m_pFint->Stress(pt); }

mat3ds FEContinuousFiberDistribution::Stress(FEMaterialPoint& mp)
{ 
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
	FEFiberMaterialPoint& fp = *mp.ExtractData<FEFiberMaterialPoint>();

	// calculate stress
	mat3ds s; s.zero();

	// get the element's local coordinate system
	mat3d QT = (pt.m_Q).transpose();

	// obtain an integration point iterator
	FEFiberIntegrationSchemeIterator* it = m_pFint->GetIterator(&pt);
	if (it->IsValid())
	{
		do
		{
			// get the global fiber direction
			vec3d& n0 = it->m_fiber;

			// convert to local coordinates
			fp.m_n0 = QT*n0;

			// rotate to local configuration to evaluate ellipsoidally distributed material coefficients
			double R = m_pFDD->FiberDensity(fp.m_n0) / m_IFD;

			// calculate the stress
			double wn = it->m_weight;
			s += m_pFmat->Stress(pt)*(R*wn);
		}
		while (it->Next());
	}

	// don't forget to delete the iterator
	delete it;

	return s;
}

//-----------------------------------------------------------------------------
//! calculate tangent stiffness at material point
//tens4ds FEContinuousFiberDistribution::Tangent(FEMaterialPoint& pt) { return m_pFint->Tangent(pt); }

tens4ds FEContinuousFiberDistribution::Tangent(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
	FEFiberMaterialPoint& fp = *mp.ExtractData<FEFiberMaterialPoint>();

	// get the element's local coordinate system
	mat3d QT = (pt.m_Q).transpose();

	// initialize stress tensor
	tens4ds c;
	c.zero();

	FEFiberIntegrationSchemeIterator* it = m_pFint->GetIterator(&pt);
	if (it->IsValid())
	{
		do
		{
			// get the global fiber direction
			vec3d& n0e = it->m_fiber;

			// convert to local
			fp.m_n0 = QT*n0e;

			// rotate to local configuration to evaluate ellipsoidally distributed material coefficients
			double R = m_pFDD->FiberDensity(fp.m_n0) / m_IFD;

			// calculate the tangent
			c += m_pFmat->Tangent(mp)*(R*it->m_weight);
		}
		while (it->Next());
	}

	// don't forget to delete the iterator
	delete it;

	// we multiply by two to add contribution from other half-sphere
	return c;
}

//-----------------------------------------------------------------------------
//! calculate strain energy density at material point
//double FEContinuousFiberDistribution::StrainEnergyDensity(FEMaterialPoint& pt) { return m_pFint->StrainEnergyDensity(pt); }
double FEContinuousFiberDistribution::StrainEnergyDensity(FEMaterialPoint& mp)
{ 
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
	FEFiberMaterialPoint& fp = *mp.ExtractData<FEFiberMaterialPoint>();

	// get the element's local coordinate system
	mat3d QT = (pt.m_Q).transpose();

	double sed = 0.0;
	FEFiberIntegrationSchemeIterator* it = m_pFint->GetIterator(&pt);
	if (it->IsValid())
	{
		do
		{
			// get fiber direction in global coordinate system
			vec3d& n0e = it->m_fiber;

			// convert to local coordinates
			fp.m_n0 = QT*n0e;

			// rotate to local configuration to evaluate ellipsoidally distributed material coefficients
			double R = m_pFDD->FiberDensity(fp.m_n0) / m_IFD;

			// calculate the stress
			sed += m_pFmat->StrainEnergyDensity(mp)*(R*it->m_weight);
		}
		while (it->Next());
	}

	// don't forget to delete the iterator
	delete it;

	// we multiply by two to add contribution from other half-sphere
	return sed;
}

//-----------------------------------------------------------------------------
void FEContinuousFiberDistribution::IntegrateFiberDensity()
{
	m_IFD = 0;
	FEFiberIntegrationSchemeIterator* it = m_pFint->GetIterator(0);
	if (it->IsValid())
	{
		do
		{
			// get fiber direction
			// BUG: why is this fiber not rotated to the local coordinates like in the other functions
			vec3d& n0a = it->m_fiber;

			// evaluate local fiber distribution
			double R = m_pFDD->FiberDensity(n0a);

			// integrate the fiber distribution
			m_IFD += R*it->m_weight;
		}
		while (it->Next());
	}

	// don't forget to delete the iterator
	delete it;
}
