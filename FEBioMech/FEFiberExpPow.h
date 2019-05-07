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
#include "FEElasticFiberMaterial.h"

//-----------------------------------------------------------------------------
//! Material class for single fiber, tension only
//! Exponential-power law

class FEFiberExpPow : public FEElasticFiberMaterial
{
public:
	FEFiberExpPow(FEModel* pfem);
	
	//! Cauchy stress
	mat3ds Stress(FEMaterialPoint& mp) override;
	
	// Spatial tangent
	tens4ds Tangent(FEMaterialPoint& mp) override;
	
	//! Strain energy density
	double StrainEnergyDensity(FEMaterialPoint& mp) override;
    
protected:
	double	m_alpha;	// coefficient of (In-1) in exponential
	double	m_beta;		// power of (In-1) in exponential
	double	m_ksi;		// fiber modulus

	// declare the parameter list
	DECLARE_PARAMETER_LIST();
};

//-----------------------------------------------------------------------------
//! Exponential-power law
//! (Variation that includes a shear term)
//! TODO: I want to delete one of these two formulations.
class FEFiberExponentialPower : public FEElasticFiberMaterial
{
public:
	FEFiberExponentialPower(FEModel* pfem);

	//! Initialization
	bool Validate() override;

	//! Cauchy stress
	mat3ds Stress(FEMaterialPoint& mp) override;

	// Spatial tangent
	tens4ds Tangent(FEMaterialPoint& mp) override;

	//! Strain energy density
	double StrainEnergyDensity(FEMaterialPoint& mp) override;

public:
	double	m_alpha;	// coefficient of (In-1) in exponential
	double	m_beta;		// power of (In-1) in exponential
	double	m_ksi;		// measure of fiber modulus
	double  m_mu;       // shear modulus

	// declare the parameter list
	DECLARE_PARAMETER_LIST();
};

