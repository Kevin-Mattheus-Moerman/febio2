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


#pragma once
#include "FEUncoupledMaterial.h"

//-----------------------------------------------------------------------------
//! Base class for single fiber response

class FEElasticFiberMaterialUC : public FEUncoupledMaterial
{
public:
    FEElasticFiberMaterialUC(FEModel* pfem);

	// returns a pointer to a new material point object
	FEMaterialPoint* CreateMaterialPointData() override;

	vec3d GetFiberVector(FEMaterialPoint& mp);

protected:
	// NOTE: Some fiber materials define a theta, phi parameter to define the fiber vector.
	//       Although this is deprecated, for backward compatibility this was feature was moved here
	double	m_thd;
	double	m_phd;

	DECLARE_PARAMETER_LIST();
};

//-----------------------------------------------------------------------------
//! Exponential-power law

class FEFiberExponentialPowerUC : public FEElasticFiberMaterialUC
{
public:
	FEFiberExponentialPowerUC(FEModel* pfem);
	
	//! Validation
	bool Validate() override;
	
	//! Cauchy stress
	mat3ds DevStress(FEMaterialPoint& mp) override;
	
	// Spatial tangent
	tens4ds DevTangent(FEMaterialPoint& mp) override;
	
	//! Strain energy density
	double DevStrainEnergyDensity(FEMaterialPoint& mp) override;
    
public:
	double	m_alpha;	// coefficient of (In-1) in exponential
	double	m_beta;		// power of (In-1) in exponential
	double	m_ksi;		// fiber modulus
    double  m_mu;       // shear modulus
    
	// declare the parameter list
	DECLARE_PARAMETER_LIST();
};

//-----------------------------------------------------------------------------
//! Neo-Hookean law

class FEFiberNHUC : public FEElasticFiberMaterialUC
{
public:
	FEFiberNHUC(FEModel* pfem) : FEElasticFiberMaterialUC(pfem) { m_mu = 0; }
	
	//! Cauchy stress
	mat3ds DevStress(FEMaterialPoint& mp) override;
	
	// Spatial tangent
	tens4ds DevTangent(FEMaterialPoint& mp) override;
	
	//! Strain energy density
	double DevStrainEnergyDensity(FEMaterialPoint& mp) override;
    
public:
	double	m_mu;       // shear modulus
    
	// declare the parameter list
	DECLARE_PARAMETER_LIST();
};
