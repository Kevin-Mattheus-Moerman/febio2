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
//! Tension-compression nonlinear orthrotropic

//! This is Gerard's material model for articular cartilage.
//! \todo Make an orthotropic material base class where we 
//!       can derive this material from.

class FETCNonlinearOrthotropic : public FEUncoupledMaterial
{
public:
	FETCNonlinearOrthotropic(FEModel* pfem) : FEUncoupledMaterial(pfem) {}

public:
	//! calculate deviatoric stress at material point
	virtual mat3ds DevStress(FEMaterialPoint& pt) override;

	//! calculate deviatoric tangent stiffness at material point
	virtual tens4ds DevTangent(FEMaterialPoint& pt) override;

	//! Strain energy density
	double DevStrainEnergyDensity(FEMaterialPoint& mp) override;
    
public:
	double	m_beta[3];
	double	m_ksi[3];

	double m_c1;	//!< Mooney-Rivlin coefficient c1
	double m_c2;	//!< Mooney-Rivlin coefficient c2	

	// declare the parameter list
	DECLARE_PARAMETER_LIST();
};
