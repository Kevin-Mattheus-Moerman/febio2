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
#include "FEFiberNeoHookean.h"

//-----------------------------------------------------------------------------
// FEFiberNH
//-----------------------------------------------------------------------------

// define the material parameters
BEGIN_PARAMETER_LIST(FEFiberNH, FEElasticFiberMaterial)
	ADD_PARAMETER2(m_mu, FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "mu");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FEFiberNH::FEFiberNH(FEModel* pfem) : FEElasticFiberMaterial(pfem) 
{ 
	m_mu = 0; 
}

//-----------------------------------------------------------------------------
mat3ds FEFiberNH::Stress(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
	
	// deformation gradient
	mat3d &F = pt.m_F;
	double J = pt.m_J;
	
	// loop over all integration points
	mat3ds C = pt.RightCauchyGreen();
	
	// fiber direction in global coordinate system
	vec3d n0 = GetFiberVector(mp);
	
	// Calculate In = n0*C*n0
	double In_1 = n0*(C*n0) - 1.0;
	
	// only take fibers in tension into consideration
	mat3ds s;
	const double eps = 0;
	if (In_1 > eps)
	{
		// get the global spatial fiber direction in current configuration
		vec3d nt = F*n0;
		
		// calculate the outer product of nt
		mat3ds N = dyad(nt);
		
		// calculate the fiber stress
		s = N*(m_mu*In_1/J);
	}
	else
	{
		s.zero();
	}
	
	return s;
}

//-----------------------------------------------------------------------------
tens4ds FEFiberNH::Tangent(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
	
	// deformation gradient
	mat3d &F = pt.m_F;
	double J = pt.m_J;
	
	// loop over all integration points
	mat3ds C = pt.RightCauchyGreen();
	
	// fiber direction in global coordinate system
	vec3d n0 = GetFiberVector(mp);
	
	// Calculate In = n0*C*n0
	double In_1 = n0*(C*n0) - 1.0;
	
	// only take fibers in tension into consideration
	tens4ds c;
	const double eps = 0;
	if (In_1 > eps)
	{
		// get the global spatial fiber direction in current configuration
		vec3d nt = F*n0;
		
		// calculate the outer product of nt
		mat3ds N = dyad(nt);
		tens4ds NxN = dyad1s(N);
		
		// calculate the fiber tangent
		c = NxN*(2*m_mu/J);
	}
	else
	{
		c.zero();
	}
	
	return c;
}

//-----------------------------------------------------------------------------
//! Strain energy density
double FEFiberNH::StrainEnergyDensity(FEMaterialPoint& mp)
{
    
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
	
	// loop over all integration points
	mat3ds C = pt.RightCauchyGreen();
	
	// fiber direction in global coordinate system
	vec3d n0 = GetFiberVector(mp);
	
	// Calculate In = n0*C*n0
	double In_1 = n0*(C*n0) - 1.0;
	
	// only take fibers in tension into consideration
	double sed = 0.0;
	const double eps = 0;
	if (In_1 > eps)
        sed = 0.25*m_mu*In_1*In_1;
    
    return sed;
}
