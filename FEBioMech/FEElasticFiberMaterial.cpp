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
#include "FEElasticFiberMaterial.h"
#include "FEFiberMaterialPoint.h"

BEGIN_PARAMETER_LIST(FEElasticFiberMaterial, FEElasticMaterial)
	ADD_PARAMETER(m_thd, FE_PARAM_DOUBLE, "theta");
	ADD_PARAMETER(m_phd, FE_PARAM_DOUBLE, "phi");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FEElasticFiberMaterial::FEElasticFiberMaterial(FEModel* pfem) : FEElasticMaterial(pfem) 
{
	m_thd = 0.0;
	m_phd = 90.0;
}

//-----------------------------------------------------------------------------
FEMaterialPoint* FEElasticFiberMaterial::CreateMaterialPointData()
{
	FEFiberMaterialPoint* fp = new FEFiberMaterialPoint(FEElasticMaterial::CreateMaterialPointData());

	// Some fiber materials defined the theta,phi parameters for setting the fiber vector
	// Although this is deprecated, we still support it here for backward compatibility
	if ((m_thd != 0.0) || (m_phd != 90.0))
	{
		// convert angles from degrees to radians
		double pi = 4 * atan(1.0);
		double the = m_thd*pi / 180.;
		double phi = m_phd*pi / 180.;

		// fiber direction in local coordinate system (reference configuration)
		vec3d n0;
		n0.x = cos(the)*sin(phi);
		n0.y = sin(the)*sin(phi);
		n0.z = cos(phi);
		n0.unit();
		fp->m_n0 = n0;
	}

	return fp;
}

//-----------------------------------------------------------------------------
vec3d FEElasticFiberMaterial::GetFiberVector(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
	FEFiberMaterialPoint& fp = *mp.ExtractData<FEFiberMaterialPoint>();

	return pt.m_Q*fp.m_n0;
}
