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
#include "FELinearTransIso.h"

//-----------------------------------------------------------------------------
// define the material parameters
BEGIN_PARAMETER_LIST(FELinearTransIso, FEElasticMaterial)
	ADD_PARAMETER2(E1 , FE_PARAM_DOUBLE, FE_RANGE_GREATER(0.0), "E1");
	ADD_PARAMETER2(E3 , FE_PARAM_DOUBLE, FE_RANGE_GREATER(0.0), "E3");
	ADD_PARAMETER2(G12, FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "G12");
	ADD_PARAMETER (v12, FE_PARAM_DOUBLE, "v12");
	ADD_PARAMETER2(v23, FE_PARAM_DOUBLE, FE_RANGE_GREATER(1.0), "v23");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
//! Check material parameters.
bool FELinearTransIso::Validate()
{
	if (FEElasticMaterial::Validate() == false) return false;
    
	if (v12 > sqrt(E1/E3)) return MaterialError("Invalid value for v12. Let v12 <= sqrt(E1/E3)");
    
	// Evaluate shear moduli
	muT = E3/2/(1+v23);
    mu = 2*G12 - muT;
	
	// check that compliance matrix is positive definite
	mat3ds c(1.0/E1,1.0/E3,1.0/E3,-v12/E1,-v23/E3,-v12/E1);
	double l[3];
	c.exact_eigen(l);
    
	if ((l[0]<0) || (l[1]<0) || (l[2]<0))
		return MaterialError("Stiffness matrix is not positive definite.");
    
	// evaluate stiffness matrix and extract Lame constants
	c = c.inverse();
    lam = c(0,0) - 2*mu;
    lamL = 0.5*(c(0,1)+c(0,2)); // c(0,1) = c(0,2)
    lamT = c(1,2);              // c(1,2) = c(1,1) - 2*muT = c(2,2) - 2*muT

	return true;
}

//-----------------------------------------------------------------------------
//! Calculates the stress for a linear orthotropic material. It calls the 
//! FElinearOrthotropic::Tangent function and contracts it with the small
//! strain tensor.
mat3ds FELinearTransIso::Stress(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();

	// Evaluate the small-strain tensor
	mat3ds e = pt.SmallStrain();

	// get the tangent
	tens4ds C = Tangent(mp);
	
	// stress = C:e
	return C.dot(e);
}

//-----------------------------------------------------------------------------
//! Calculates the elasticity tensor for an orthotropic material.
//! \todo come up with some verification problems for this material model
tens4ds FELinearTransIso::Tangent(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
	
	vec3d a0;		// texture direction in reference configuration
	mat3ds A0;		// texture tensor in reference configuration
    
	mat3dd I(1.);
	
    a0.x = pt.m_Q[0][0]; a0.y = pt.m_Q[1][0]; a0.z = pt.m_Q[2][0];
    A0 = dyad(a0);			// Evaluate the texture tensor in the reference configuration
	
	tens4ds C = lamT*dyad1s(I) + (lamT + lam - 2*lamL)*dyad1s(A0)
    + (lamL - lamT)*dyad1s(A0,I) + 2*muT*dyad4s(I) + (mu - muT)*dyad4s(A0,I);
    
	return C;
}

//-----------------------------------------------------------------------------
//! Calculates the strain energy density for a linear transversely isotropic material. It calls the
//! FELinearTransIso::Tangent function and contracts it with the small
//! strain tensor.
double FELinearTransIso::StrainEnergyDensity(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
    
	// Evaluate the small-strain tensor
	mat3ds e = pt.SmallStrain();
    
	// get the tangent
	tens4ds C = Tangent(mp);
	
	// sed = e:C:e/2
	return (C.dot(e)).dotdot(e)/2.0;
}
