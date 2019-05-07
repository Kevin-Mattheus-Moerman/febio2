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
#include "FEElasticMaterial2O.h"
#include <FECore/DumpStream.h>

//-----------------------------------------------------------------------------
FEElasticMaterialPoint2O::FEElasticMaterialPoint2O(FEMaterialPoint* pt) : FEMaterialPoint(pt)
{
	m_PK1.zero();
	m_G.zero();
	m_Q.zero();
}

//-----------------------------------------------------------------------------
FEMaterialPoint* FEElasticMaterialPoint2O::Copy()
{
	FEElasticMaterialPoint2O* pt = new FEElasticMaterialPoint2O(0);
	pt->m_PK1 = m_PK1;
	pt->m_G   = m_G;
	pt->m_Q   = m_Q;
	if (m_pNext) pt->SetNext(m_pNext->Copy());

	return pt;
}

//-----------------------------------------------------------------------------
void FEElasticMaterialPoint2O::Serialize(DumpStream& ar)
{
	FEMaterialPoint::Serialize(ar);
	if (ar.IsSaving())
	{
		ar << m_PK1;
		ar << m_G;
		ar << m_Q; 
	}
	else
	{
		ar >> m_PK1;
		ar >> m_G;
		ar >> m_Q; 
	}
}

//-----------------------------------------------------------------------------
// define the material parameters
BEGIN_PARAMETER_LIST(FEElasticMaterial2O, FEElasticMaterial)
	ADD_PARAMETER(m_beta     , FE_PARAM_DOUBLE, "beta"    );
	ADD_PARAMETER(m_bKDG1    , FE_PARAM_BOOL  , "KDG1"    );
	ADD_PARAMETER(m_bKDG2    , FE_PARAM_BOOL  , "KDG2"    );
	ADD_PARAMETER(m_bKDG3    , FE_PARAM_BOOL  , "KDG3"    );
	ADD_PARAMETER(m_buseJ0   , FE_PARAM_BOOL  , "useJ0"   );
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FEElasticMaterial2O::FEElasticMaterial2O(FEModel* pfem) : FEElasticMaterial(pfem)
{
	m_beta = 10.0;

	m_bKDG1 = true;
	m_bKDG2 = true;
	m_bKDG3 = true;
	m_buseJ0 = true;
}

//-----------------------------------------------------------------------------
FEMaterialPoint* FEElasticMaterial2O::CreateMaterialPointData()
{
	return new FEElasticMaterialPoint2O(new FEElasticMaterialPoint);
}

//-----------------------------------------------------------------------------
// The stiffness is evaluated at the same time the stress is evaluated so we 
// can just return it here. Note that this assumes that the stress function 
// is always called prior to the tangent function.
// Note that this function is not used in the second-order implemenetation
tens4ds FEElasticMaterial2O::Tangent(FEMaterialPoint &mp)
{
	assert(false);
	tens4ds c; c.zero();
	return c;
}

//-----------------------------------------------------------------------------
// Note that this function is not used in the second-order implemenetation
mat3ds FEElasticMaterial2O::Stress(FEMaterialPoint &mp)
{
	assert(false);
	mat3ds sa; sa.zero();
	return sa;
}
