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
#include "FEDiffRefIso.h"

// define the material parameters
BEGIN_PARAMETER_LIST(FEDiffRefIso, FESoluteDiffusivity)
	ADD_PARAMETER2(m_free_diff, FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "free_diff");
	ADD_PARAMETER2(m_diff0    , FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "diff0"    );
	ADD_PARAMETER2(m_diff1    , FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "diff1"    );
	ADD_PARAMETER2(m_diff2    , FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "diff2"    );
	ADD_PARAMETER2(m_M        , FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "M"        );
	ADD_PARAMETER2(m_alpha    , FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "alpha"    );
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
//! Constructor. 
FEDiffRefIso::FEDiffRefIso(FEModel* pfem) : FESoluteDiffusivity(pfem)
{
	m_free_diff = 1;
	m_diff0 = 1;
	m_diff1 = 0;
	m_diff2 = 0;
	m_M = m_alpha = 0;
}

//-----------------------------------------------------------------------------
//! Free diffusivity
double FEDiffRefIso::Free_Diffusivity(FEMaterialPoint& mp)
{
	return m_diff0;
}

//-----------------------------------------------------------------------------
//! Tangent of free diffusivity with respect to concentration
double FEDiffRefIso::Tangent_Free_Diffusivity_Concentration(FEMaterialPoint& mp, const int isol)
{
	return 0;
}

//-----------------------------------------------------------------------------
//! Diffusivity tensor.
mat3ds FEDiffRefIso::Diffusivity(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& et = *mp.ExtractData<FEElasticMaterialPoint>();
	FEBiphasicMaterialPoint& ppt = *mp.ExtractData<FEBiphasicMaterialPoint>();
	
	// Identity
	mat3dd I(1);
	
	// left cauchy-green matrix
	mat3ds b = et.LeftCauchyGreen();
	
	// relative volume
	double J = et.m_J;
	
	// solid volume fraction in reference configuration
	double phi0 = ppt.m_phi0;
	
	// --- strain-dependent permeability ---
	
	double f = pow((J-phi0)/(1-phi0),m_alpha)*exp(m_M*(J*J-1.0)/2.0);
	double d0 = m_diff0*f;
	double d1 = m_diff1/(J*J)*f;
	double d2 = 0.5*m_diff2/pow(J,4)*f;
	mat3ds dt = d0*I+d1*b+2*d2*b*b;
	
	return dt;
}

//-----------------------------------------------------------------------------
//! Tangent of diffusivity with respect to strain
tens4ds FEDiffRefIso::Tangent_Diffusivity_Strain(FEMaterialPoint &mp)
{
	FEElasticMaterialPoint& et = *mp.ExtractData<FEElasticMaterialPoint>();
	FEBiphasicMaterialPoint& ppt = *mp.ExtractData<FEBiphasicMaterialPoint>();
	
	// Identity
	mat3dd I(1);
	
	// left cauchy-green matrix
	mat3ds b = et.LeftCauchyGreen();
	
	// relative volume
	double J = et.m_J;
	
	// solid volume fraction in reference configuration
	double phi0 = ppt.m_phi0;
	
	double f = pow((J-phi0)/(1-phi0),m_alpha)*exp(m_M*(J*J-1.0)/2.0);
	double d0 = m_diff0*f;
	double d1 = m_diff1/(J*J)*f;
	double d2 = 0.5*m_diff2/pow(J,4)*f;
	double D0prime = (J*J*m_M+(J*(m_alpha+1)-phi0)/(J-phi0))*d0;
	double D1prime = (J*J*m_M+(J*(m_alpha-1)+phi0)/(J-phi0))*d1;
	double D2prime = (J*J*m_M+(J*(m_alpha-3)+3*phi0)/(J-phi0))*d2;
	mat3ds d0hat = I*D0prime;
	mat3ds d1hat = I*D1prime;
	mat3ds d2hat = I*D2prime;
	
	tens4ds D4 = dyad1s(I,d0hat)/2.0-dyad4s(I)*2*d0
	+ dyad1s(b,d1hat)/2.0
	+ dyad1s(b*b,d2hat)+dyad4s(b)*4*d2;
	
	return D4;
}

//-----------------------------------------------------------------------------
//! Tangent of diffusivity with respect to concentration
mat3ds FEDiffRefIso::Tangent_Diffusivity_Concentration(FEMaterialPoint &mp, const int isol)
{
	mat3ds d;
	d.zero();
	return d;
}
