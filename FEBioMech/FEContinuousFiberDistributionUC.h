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
#include "FEElasticFiberMaterialUC.h"
#include "FEFiberDensityDistribution.h"
#include "FEFiberIntegrationScheme.h"
#include "FEFiberMaterialPoint.h"

//  This material is a container for a fiber material, a fiber density
//  distribution, and an integration scheme.
//
class FEContinuousFiberDistributionUC : public FEUncoupledMaterial
{
public:
    FEContinuousFiberDistributionUC(FEModel* pfem);
    ~FEContinuousFiberDistributionUC();
    
    // Initialization
    bool Init();
    
public:
	//! calculate stress at material point
	mat3ds DevStress(FEMaterialPoint& pt);
    
	//! calculate tangent stiffness at material point
	tens4ds DevTangent(FEMaterialPoint& pt);
    
	//! calculate deviatoric strain energy density
	double DevStrainEnergyDensity(FEMaterialPoint& pt);
    
	// returns a pointer to a new material point object
	FEMaterialPoint* CreateMaterialPointData();

protected:
	// integrated Fiber density
	void IntegrateFiberDensity();
    
public:
    FEPropertyT<FEElasticFiberMaterialUC>   m_pFmat;    // pointer to fiber material
	FEPropertyT<FEFiberDensityDistribution> m_pFDD;     // pointer to fiber density distribution
	FEPropertyT<FEFiberIntegrationScheme>	m_pFint;    // pointer to fiber integration scheme
	double	m_IFD;	// integrated fiber distribution
};
