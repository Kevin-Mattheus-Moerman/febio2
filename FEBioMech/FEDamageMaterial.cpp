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
#include "FEDamageMaterial.h"
#include "FEDamageCriterion.h"
#include "FEDamageCDF.h"
#include "FEUncoupledMaterial.h"
#include "FECore/FECoreKernel.h"

//-----------------------------------------------------------------------------
//! Constructor.
FEDamageMaterial::FEDamageMaterial(FEModel* pfem) : FEElasticMaterial(pfem)
{
	// set material properties
	AddProperty(&m_pBase, "elastic"  );
	AddProperty(&m_pDamg, "damage"   );
	AddProperty(&m_pCrit, "criterion");
}

//-----------------------------------------------------------------------------
//! Initialization.
bool FEDamageMaterial::Init()
{
    FEUncoupledMaterial* m_pMat = dynamic_cast<FEUncoupledMaterial*>((FEElasticMaterial*)m_pBase);
    if (m_pMat != nullptr)
        return MaterialError("Elastic material should not be of type uncoupled");
    
	return FEElasticMaterial::Init();
}

//-----------------------------------------------------------------------------
//! calculate stress at material point
mat3ds FEDamageMaterial::Stress(FEMaterialPoint& pt)
{
    // evaluate the damage
    double d = Damage(pt);
    
    // evaluate the stress
    mat3ds s = m_pBase->Stress(pt);
    
    // return damaged stress
    return s*(1-d);
}

//-----------------------------------------------------------------------------
//! calculate tangent stiffness at material point
tens4ds FEDamageMaterial::Tangent(FEMaterialPoint& pt)
{
    // evaluate the damage
    double d = Damage(pt);
    
    // evaluate the tangent
    tens4ds c = m_pBase->Tangent(pt);
    
    // return damaged tangent
    return c*(1-d);
}

//-----------------------------------------------------------------------------
//! calculate strain energy density at material point
double FEDamageMaterial::StrainEnergyDensity(FEMaterialPoint& pt)
{
    // evaluate the damage
    double d = Damage(pt);
    
    // evaluate the strain energy density
    double sed = m_pBase->StrainEnergyDensity(pt);

    // return damaged sed
    return sed*(1-d);
}

//-----------------------------------------------------------------------------
//! calculate damage at material point
double FEDamageMaterial::Damage(FEMaterialPoint& pt)
{
    // get the damage material point data
    FEDamageMaterialPoint& pd = *pt.ExtractData<FEDamageMaterialPoint>();

    // evaluate the trial value of the damage criterion
    // this must be done before evaluating the damage
    pd.m_Etrial = m_pCrit->DamageCriterion(pt);
    
    // evaluate and set the damage
    double d = m_pDamg->Damage(pt);
    pd.m_D = d;
    
    return d;
}

//-----------------------------------------------------------------------------
void FEDamageMaterial::SetLocalCoordinateSystem(FEElement& el, int n, FEMaterialPoint& mp)
{
	FEElasticMaterial::SetLocalCoordinateSystem(el, n, mp);
	m_pBase->SetLocalCoordinateSystem(el, n, mp);
}
