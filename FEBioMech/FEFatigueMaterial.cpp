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
#include "FEFatigueMaterial.h"
#include "FEDamageCriterion.h"
#include "FEDamageCDF.h"
#include "FEUncoupledMaterial.h"
#include "FECore/FECoreKernel.h"
#include <FECore/DumpStream.h>

////////////////////// FATIGUE MATERIAL POINT /////////////////////////////////
//-----------------------------------------------------------------------------
FEMaterialPoint* FEFatigueMaterialPoint::Copy()
{
    FEFatigueMaterialPoint* pt = new FEFatigueMaterialPoint(*this);
    if (m_pNext) pt->m_pNext = m_pNext->Copy();
    return pt;
}

//-----------------------------------------------------------------------------
void FEFatigueMaterialPoint::Init()
{
    FEMaterialPoint::Init();
    
    // intialize data
    m_D = 0;
    
    // initialize intact bond fraction to 1
    m_wip = m_wit = 1.0;
    m_wifp = m_wift = 1.0;

    // initialize fatigued bond fraction to 0
    m_wfp = m_wft = 0;

    // initialize damage criterion
    m_Ximax = m_Xitrl = 0;
    m_Xfmax = m_Xftrl = 0;

    // initialize fatigue criterion
    m_Xfp = m_Xft = 0;
    m_dXfp = m_dXft = 0;
}

//-----------------------------------------------------------------------------
void FEFatigueMaterialPoint::Update(const FETimeInfo& timeInfo)
{
    FEMaterialPoint::Update(timeInfo);
    
    // update damage response for intact bonds
    if (m_Xitrl > m_Ximax) m_Ximax = m_Xitrl;
    
    // update damage response for fatigues bonds
    if (m_Xftrl > m_Xfmax) m_Xfmax = m_Xftrl;
    
    // account for possibility that loading history has changed
    // which affects fatigue bonds since they are produced continually
    // check if we just passed a local maximum in m_Xft
//    if ((m_dXfp > 0) && (m_dXft <= 0)) m_Xfmax = m_Xfp;
    
    // update fatigue criterion and its increment
    m_Xfp = m_Xft;
    m_dXfp = m_dXft;
    
    // intact bonds
    m_wip = m_wit;
    m_wifp = m_wift;
    
    // update fatigue response
    m_wfp = m_wft;
}

//-----------------------------------------------------------------------------
void FEFatigueMaterialPoint::Serialize(DumpStream& ar)
{
    if (ar.IsSaving())
    {
        ar << m_D;
        ar << m_wip << m_wit;
        ar << m_wfp << m_wft;
        ar << m_wifp << m_wift;
        ar << m_Ximax << m_Xitrl;
        ar << m_Xfmax << m_Xftrl;
        ar << m_Xfp << m_Xft;
        ar << m_dXfp << m_dXft;
    }
    else
    {
        ar >> m_D;
        ar >> m_wip >> m_wit;
        ar >> m_wfp >> m_wft;
        ar >> m_wifp >> m_wift;
        ar >> m_Ximax >> m_Xitrl;
        ar >> m_Xfmax >> m_Xftrl;
        ar >> m_Xfp >> m_Xft;
        ar >> m_dXfp >> m_dXft;
    }
    FEMaterialPoint::Serialize(ar);
}

//////////////////////////// FATIGUE MATERIAL /////////////////////////////////
//-----------------------------------------------------------------------------
// define the material parameters
BEGIN_PARAMETER_LIST(FEFatigueMaterial, FEMaterial)
ADD_PARAMETER2(m_k0   , FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "k0"  );
ADD_PARAMETER2(m_beta , FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "beta");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
//! Constructor.
FEFatigueMaterial::FEFatigueMaterial(FEModel* pfem) : FEElasticMaterial(pfem)
{
    // set material properties
    AddProperty(&m_pBase, "elastic"          );
    AddProperty(&m_pIdmg, "intact_damage"    );
    AddProperty(&m_pFdmg, "fatigue_damage"   );
    AddProperty(&m_pCrit, "criterion"        );
}

//-----------------------------------------------------------------------------
//! Initialization.
bool FEFatigueMaterial::Init()
{
    FEUncoupledMaterial* m_pMat = dynamic_cast<FEUncoupledMaterial*>((FEElasticMaterial*)m_pBase);
    if (m_pMat != nullptr)
        return MaterialError("Elastic material should not be of type uncoupled");
    
    return FEElasticMaterial::Init();
}

//-----------------------------------------------------------------------------
//! calculate stress at material point
mat3ds FEFatigueMaterial::Stress(FEMaterialPoint& pt)
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
tens4ds FEFatigueMaterial::Tangent(FEMaterialPoint& pt)
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
double FEFatigueMaterial::StrainEnergyDensity(FEMaterialPoint& pt)
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
double FEFatigueMaterial::Damage(FEMaterialPoint& pt)
{
    // get the damage material point data
    FEFatigueMaterialPoint& pd = *pt.ExtractData<FEFatigueMaterialPoint>();
    
    return pd.m_D;
}

//-----------------------------------------------------------------------------
// update fatigue material point at each iteration
void FEFatigueMaterial::UpdateSpecializedMaterialPoints(FEMaterialPoint& pt, const FETimeInfo& tp)
{
    double dt = tp.timeIncrement;
    
    // assume that time derivative of damage variable is constant over time interval
    double alpha = tp.alpha;

    // get the fatigue material point data
    FEFatigueMaterialPoint& pd = *pt.ExtractData<FEFatigueMaterialPoint>();
    
    // get damage and fatigue criteria at intermediate time point
    pd.m_Xitrl = pd.m_Xftrl = pd.m_Xft = (m_pCrit->DamageCriterion(pt) + (1 - alpha)*pd.m_Xfp)/alpha;

    // evaluate time derivative of fatigue criterion
    pd.m_dXft = pd.m_Xft - pd.m_Xfp;
    double aXft = pd.m_dXft/dt;
    
    // evaluate mass supply from fatigue
    double k = -m_k0*pow(fabs(aXft)*pd.m_D, m_beta);
    
    // evaluate intact bond mass fractions due to fatigue
    pd.m_wift = pd.m_wifp*exp(k*dt);
    
    // evaluate bond mass fractions due to damage
    // intact bonds
    double Fi = m_pIdmg->cdf(fmax(pd.m_Xitrl,pd.m_Ximax));
    pd.m_wit = pd.m_wift*(1 - Fi);
    
    // fatigued bonds
    double Ff = m_pFdmg->cdf(fmax(pd.m_Xftrl,pd.m_Xfmax));
    pd.m_wft = (1 - pd.m_wift)*(1 - Ff);
    
    // update bond mass fractions based on mass supplies
    if (pd.m_wit < 0) { pd.m_wit = 0; }
    if (pd.m_wft < 0) { pd.m_wft = 0; }
    if (pd.m_wft > 1) { pd.m_wft = 1; }

    // evaluate intermediate values of bond mass fractions
    double wi = (1-alpha)*pd.m_wip + alpha*pd.m_wit;
    double wf = (1-alpha)*pd.m_wfp + alpha*pd.m_wft;

    pd.m_D = 1 - wi - wf;
}


//-----------------------------------------------------------------------------
void FEFatigueMaterial::SetLocalCoordinateSystem(FEElement& el, int n, FEMaterialPoint& mp)
{
    FEElasticMaterial::SetLocalCoordinateSystem(el, n, mp);
    m_pBase->SetLocalCoordinateSystem(el, n, mp);
}
