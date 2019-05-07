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
#include "FEFiberPowLinear.h"

// define the material parameters
BEGIN_PARAMETER_LIST(FEFiberPowLinear, FEElasticFiberMaterial)
    ADD_PARAMETER2(m_E    , FE_PARAM_DOUBLE, FE_RANGE_GREATER(0.0), "E"    );
    ADD_PARAMETER2(m_lam0 , FE_PARAM_DOUBLE, FE_RANGE_GREATER(1.0), "lam0" );
    ADD_PARAMETER2(m_beta , FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(2.0), "beta" );
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
// FEFiberPowLinear
//-----------------------------------------------------------------------------

FEFiberPowLinear::FEFiberPowLinear(FEModel* pfem) : FEElasticFiberMaterial(pfem)
{

}

//-----------------------------------------------------------------------------
mat3ds FEFiberPowLinear::Stress(FEMaterialPoint& mp)
{
    FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
    
    // initialize material constants
    double I0 = m_lam0*m_lam0;
    double ksi = m_E/4/(m_beta-1)*pow(I0, -3./2.)*pow(I0-1, 2-m_beta);
    double b = ksi*pow(I0-1, m_beta-1) + m_E/2/sqrt(I0);
    
    // deformation gradient
    mat3d &F = pt.m_F;
    double J = pt.m_J;
    
    // loop over all integration points
    const double eps = 0;
    mat3ds C = pt.RightCauchyGreen();
    mat3ds s;
    
    // evaluate fiber direction in global coordinate system
    vec3d n0 = GetFiberVector(mp);
    
    // Calculate In
    double In = n0*(C*n0);
    
    // only take fibers in tension into consideration
    if (In - 1 > eps)
    {
        // get the global spatial fiber direction in current configuration
        vec3d nt = F*n0/sqrt(In);
        
        // calculate the outer product of nt
        mat3ds N = dyad(nt);
        
        // calculate the fiber stress magnitude
        double sn = (In < I0) ?
        2*In*ksi*pow(In-1, m_beta-1) :
        2*b*In - m_E*sqrt(In);
        
        // calculate the fiber stress
        s = N*(sn/J);
    }
    else
    {
        s.zero();
    }
    
    return s;
}

//-----------------------------------------------------------------------------
tens4ds FEFiberPowLinear::Tangent(FEMaterialPoint& mp)
{
    FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
    
    // initialize material constants
    double I0 = m_lam0*m_lam0;
    double ksi = m_E/4/(m_beta-1)*pow(I0, -3./2.)*pow(I0-1, 2-m_beta);
    
    // deformation gradient
    mat3d &F = pt.m_F;
    double J = pt.m_J;
    
    // loop over all integration points
    const double eps = 0;
    mat3ds C = pt.RightCauchyGreen();
    tens4ds c;
    
    // evaluate fiber direction in global coordinate system
    vec3d n0 = GetFiberVector(mp);
    
    // Calculate In
    double In = n0*(C*n0);
    
    // only take fibers in tension into consideration
    if (In - 1 > eps)
    {
        // get the global spatial fiber direction in current configuration
        vec3d nt = F*n0/sqrt(In);
        
        // calculate the outer product of nt
        mat3ds N = dyad(nt);
        tens4ds NxN = dyad1s(N);
        
        // calculate modulus
        double cn = (In < I0) ?
        4*In*In*ksi*(m_beta-1)*pow(In-1, m_beta-2) :
        m_E*sqrt(In);
        
        // calculate the fiber tangent
        c = NxN*(cn/J);
    }
    else
    {
        c.zero();
    }
    
    return c;
}

//-----------------------------------------------------------------------------
double FEFiberPowLinear::StrainEnergyDensity(FEMaterialPoint& mp)
{
    double sed = 0.0;
    
    FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
    
    // initialize material constants
    double I0 = m_lam0*m_lam0;
    double ksi = m_E/4/(m_beta-1)*pow(I0, -3./2.)*pow(I0-1, 2-m_beta);
    double b = ksi*pow(I0-1, m_beta-1) + m_E/2/sqrt(I0);
    
    // loop over all integration points
    const double eps = 0;
    mat3ds C = pt.RightCauchyGreen();
    
    // evaluate fiber direction in global coordinate system
    vec3d n0 = GetFiberVector(mp);
    
    // Calculate In = n0*C*n0
    double In = n0*(C*n0);
    
    // only take fibers in tension into consideration
    if (In - 1 > eps)
    {
        // calculate strain energy density
        sed = (In < I0) ?
        ksi/m_beta*pow(In-1, m_beta) :
        b*(In-I0) - m_E*(sqrt(In)-sqrt(I0)) + ksi/m_beta*pow(I0-1, m_beta);
    }
    
    return sed;
}

//-----------------------------------------------------------------------------
// FEFiberPowerLinear
//-----------------------------------------------------------------------------

// define the material parameters
BEGIN_PARAMETER_LIST(FEFiberPowerLinear, FEElasticFiberMaterial)
ADD_PARAMETER2(m_E, FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "E");
ADD_PARAMETER2(m_beta, FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(2.0), "beta");
ADD_PARAMETER2(m_lam0, FE_PARAM_DOUBLE, FE_RANGE_GREATER(1.0), "lam0");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FEFiberPowerLinear::FEFiberPowerLinear(FEModel* pfem) : FEElasticFiberMaterial(pfem)
{
	m_E = 0;
	m_lam0 = 1;
	m_beta = 3;
}

//-----------------------------------------------------------------------------
bool FEFiberPowerLinear::Validate()
{
	if (FEElasticFiberMaterial::Validate() == false) return false;

	// initialize material constants
	m_I0 = m_lam0*m_lam0;
	m_ksi = 0.25*m_E*pow(m_I0 - 1, 2 - m_beta) / (m_beta - 1)*pow(m_I0, -3. / 2.);
	m_b = m_ksi*pow(m_I0 - 1, m_beta - 1) + m_E / 2 / sqrt(m_I0);

	return true;
}

//-----------------------------------------------------------------------------
mat3ds FEFiberPowerLinear::Stress(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();

	// deformation gradient
	mat3d &F = pt.m_F;
	double J = pt.m_J;

	// loop over all integration points
	mat3ds C = pt.RightCauchyGreen();
	mat3ds s;

	// fiber direction in global coordinate system
	vec3d n0 = GetFiberVector(mp);

	// Calculate In
	double In = n0*(C*n0);

	// only take fibers in tension into consideration
	const double eps = 0;
	if (In - 1 > eps)
	{
		// get the global spatial fiber direction in current configuration
		vec3d nt = F*n0 / sqrt(In);

		// calculate the outer product of nt
		mat3ds N = dyad(nt);

		// calculate the fiber stress magnitude
		double sn = (In < m_I0) ?
			2 * In*m_ksi*pow(In - 1, m_beta - 1) :
			2 * m_b*In - m_E*sqrt(In);

		// calculate the fiber stress
		s = N*(sn / J);
	}
	else
	{
		s.zero();
	}

	return s;
}

//-----------------------------------------------------------------------------
tens4ds FEFiberPowerLinear::Tangent(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();

	// deformation gradient
	mat3d &F = pt.m_F;
	double J = pt.m_J;

	mat3ds C = pt.RightCauchyGreen();
	tens4ds c;

	// fiber direction in global coordinate system
	vec3d n0 = GetFiberVector(mp);

	// Calculate In
	double In = n0*(C*n0);

	// only take fibers in tension into consideration
	const double eps = 0;
	if (In - 1 > eps)
	{
		// get the global spatial fiber direction in current configuration
		vec3d nt = F*n0 / sqrt(In);

		// calculate the outer product of nt
		mat3ds N = dyad(nt);
		tens4ds NxN = dyad1s(N);

		// calculate modulus
		double cn = (In < m_I0) ?
			4 * In*In*m_ksi*(m_beta - 1)*pow(In - 1, m_beta - 2) :
			m_E*sqrt(In);

		// calculate the fiber tangent
		c = NxN*(cn / J);
	}
	else
	{
		c.zero();
	}

	return c;
}

//-----------------------------------------------------------------------------
//! Strain energy density
double FEFiberPowerLinear::StrainEnergyDensity(FEMaterialPoint& mp)
{
	double sed = 0.0;

	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();

	// loop over all integration points
	mat3ds C = pt.RightCauchyGreen();

	// fiber direction in global coordinate system
	vec3d n0 = GetFiberVector(mp);

	// Calculate In = n0*C*n0
	double In = n0*(C*n0);

	// only take fibers in tension into consideration
	const double eps = 0;
	if (In - 1 > eps)
	{
		// calculate strain energy density
		sed = (In < m_I0) ?
			m_ksi / m_beta*pow(In - 1, m_beta) :
			m_b*(In - m_I0) - m_E*(sqrt(In) - sqrt(m_I0)) + m_ksi / m_beta*pow(m_I0 - 1, m_beta);
	}

	return sed;
}
