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
#include "FELinearOrthotropic.h"

//-----------------------------------------------------------------------------
// define the material parameters
BEGIN_PARAMETER_LIST(FELinearOrthotropic, FEElasticMaterial)
	ADD_PARAMETER2(E1, FE_PARAM_DOUBLE, FE_RANGE_GREATER(0.0), "E1");
	ADD_PARAMETER2(E2, FE_PARAM_DOUBLE, FE_RANGE_GREATER(0.0), "E2");
	ADD_PARAMETER2(E3, FE_PARAM_DOUBLE, FE_RANGE_GREATER(0.0), "E3");
	ADD_PARAMETER2(G12, FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "G12");
	ADD_PARAMETER2(G23, FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "G23");
	ADD_PARAMETER2(G31, FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "G31");
	ADD_PARAMETER(v12, FE_PARAM_DOUBLE, "v12");
	ADD_PARAMETER(v23, FE_PARAM_DOUBLE, "v23");
	ADD_PARAMETER(v31, FE_PARAM_DOUBLE, "v31");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
//! Check material parameters.
bool FELinearOrthotropic::Validate()
{
	if (FEElasticMaterial::Validate() == false) return false;

	if (v12 > sqrt(E1/E2)) return MaterialError("Invalid value for v12. Let v12 <= sqrt(E1/E2)");
	if (v23 > sqrt(E2/E3)) return MaterialError("Invalid value for v23. Let v23 <= sqrt(E2/E3)");
	if (v31 > sqrt(E3/E1)) return MaterialError("Invalid value for v31. Let v31 <= sqrt(E3/E1)");
    
	// Evaluate Lame coefficients
	mu[0] = G12 + G31 - G23;
	mu[1] = G12 - G31 + G23;
	mu[2] =-G12 + G31 + G23;
	lam[0][0] = 1.0/E1; lam[0][1] = -v12/E1; lam[0][2] = -v31/E3;
	lam[1][0] = -v12/E1; lam[1][1] = 1.0/E2; lam[1][2] = -v23/E2;
	lam[2][0] = -v31/E3; lam[2][1] = -v23/E2; lam[2][2] = 1.0/E3;
	
	// check that compliance matrix is positive definite
	mat3ds c(lam[0][0],lam[1][1],lam[2][2],lam[0][1],lam[1][2],lam[0][2]);
	double l[3];
	c.exact_eigen(l);
    
	if ((l[0]<0) || (l[1]<0) || (l[2]<0))
		return MaterialError("Stiffness matrix is not positive definite.");
    
	// evaluate stiffness matrix and extract Lame constants
	c = c.inverse();
	lam[0][0] = c(0,0) - 2*mu[0];
	lam[1][1] = c(1,1) - 2*mu[1];
	lam[2][2] = c(2,2) - 2*mu[2];
	lam[1][2] = c(1,2); lam[2][1] = c(2,1);
	lam[2][0] = c(2,0); lam[0][2] = c(0,2);
	lam[0][1] = c(0,1); lam[1][0] = c(1,0);

	return true;
}

//-----------------------------------------------------------------------------
//! Calculates the stress for a linear orthotropic material. It calls the 
//! FElinearOrthotropic::Tangent function and contracts it with the small
//! strain tensor.
mat3ds FELinearOrthotropic::Stress(FEMaterialPoint& mp)
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
tens4ds FELinearOrthotropic::Tangent(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
	
	int i,j;
	vec3d a0[3];		// texture direction in reference configuration
	mat3ds A0[3];		// texture tensor in reference configuration

	mat3dd I(1.);
	
	for (i=0; i<3; i++) {	// Perform sum over all three texture directions
		// Copy the texture direction in the reference configuration to a0
		a0[i].x = pt.m_Q[0][i]; a0[i].y = pt.m_Q[1][i]; a0[i].z = pt.m_Q[2][i];
		A0[i] = dyad(a0[i]);			// Evaluate the texture tensor in the reference configuration
	}
	
	tens4ds C(0.0);
	for (i=0; i<3; i++) {
		C += mu[i]*dyad4s(A0[i],I);
		for (j=0; j<3; j++)
			C += lam[i][j]*dyad1s(A0[i],A0[j])/2.;
	}
	
	return C;
}

//-----------------------------------------------------------------------------
//! Calculates the strain energy density for a linear orthotropic material. It calls the
//! FElinearOrthotropic::Tangent function and contracts it with the small
//! strain tensor.
double FELinearOrthotropic::StrainEnergyDensity(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
    
	// Evaluate the small-strain tensor
	mat3ds e = pt.SmallStrain();
    
	// get the tangent
	tens4ds C = Tangent(mp);
	
	// sed = e:C:e/2
	return (C.dot(e)).dotdot(e)/2.0;
}
