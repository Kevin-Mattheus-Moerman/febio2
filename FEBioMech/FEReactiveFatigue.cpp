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
#include "FEReactiveFatigue.h"
#include "FEDamageCriterion.h"
#include "FEDamageCDF.h"
#include "FEUncoupledMaterial.h"
#include "FECore/FECoreKernel.h"
#include <FECore/DumpStream.h>

////////////////////// FATIGUE MATERIAL POINT /////////////////////////////////
//-----------------------------------------------------------------------------
FEMaterialPoint* FEReactiveFatigueMaterialPoint::Copy()
{
    FEReactiveFatigueMaterialPoint* pt = new FEReactiveFatigueMaterialPoint(*this);
    if (m_pNext) pt->m_pNext = m_pNext->Copy();
    return pt;
}

//-----------------------------------------------------------------------------
void FEReactiveFatigueMaterialPoint::Init()
{
    FEMaterialPoint::Init();
    
    // intialize data
    m_D = 0;
    m_wip = m_wit = 1.0;
    m_wfp = m_wft = 0.0;
    
    // initialize damage criterion
    m_Ximax = m_Xitrl = 0;
    m_Xfmax = m_Xftrl = 0;
    
    // initialize fatigue criterion
    m_Xfp = m_Xft = 0;
    m_dXfp = m_dXft = 0;
}

//-----------------------------------------------------------------------------
void FEReactiveFatigueMaterialPoint::Update(const FETimeInfo& timeInfo)
{
    FEMaterialPoint::Update(timeInfo);
    
    // update damage response for intact bonds
    if (m_Xitrl > m_Ximax) m_Ximax = m_Xitrl;
    
    // update damage response for fatigues bonds
    if (m_Xftrl > m_Xfmax) m_Xfmax = m_Xftrl;
    
    // update fatigue criterion and its increment
    m_Xfp = m_Xft;
    m_dXfp = m_dXft;
    
    // intact bonds
    m_vip = m_vit;
    m_wip = m_wit;
    
    // update fatigue response
    m_vfp = m_vft;
    m_wfp = m_wft;
}

//-----------------------------------------------------------------------------
void FEReactiveFatigueMaterialPoint::Serialize(DumpStream& ar)
{
    if (ar.IsSaving())
    {
        ar << m_D;
        ar << m_wip << m_wit;
        ar << m_wfp << m_wft;
        ar << m_Ximax << m_Xitrl;
        ar << m_Xfmax << m_Xftrl;
        ar << m_Xfp << m_Xft;
        ar << m_dXfp << m_dXft;
//        ar << m_vip.size();
//        for (int i=0; i<m_vip.size(); ++i) {
//            ar << m_vip[i] << m_vit[i];
//            ar << m_vfp[i] << m_vft[i];
//        }
    }
    else
    {
        int n;
        ar >> m_D;
        ar >> m_wip >> m_wit;
        ar >> m_wfp >> m_wft;
        ar >> m_Ximax >> m_Xitrl;
        ar >> m_Xfmax >> m_Xftrl;
        ar >> m_Xfp >> m_Xft;
        ar >> m_dXfp >> m_dXft;
//        ar >> n;
//        m_vip.resize(n);
//        m_vit.resize(n);
//        m_vfp.resize(n);
//        m_vft.resize(n);
//        for (int i=0; i<n; ++i) {
//            ar >> m_vip[i] >> m_vit[i];
//            ar >> m_vfp[i] >> m_vft[i];
//        }
    }
    FEMaterialPoint::Serialize(ar);
}

//////////////////////////// FATIGUE MATERIAL /////////////////////////////////
//-----------------------------------------------------------------------------
// define the material parameters
BEGIN_PARAMETER_LIST(FEReactiveFatigue, FEMaterial)
ADD_PARAMETER2(m_k0   , FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "k0"  );
ADD_PARAMETER2(m_beta , FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "beta");
ADD_PARAMETER2(m_gamma, FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "gamma");
ADD_PARAMETER2(m_ndiv , FE_PARAM_INT   , FE_RANGE_GREATER(0), "ndiv");
ADD_PARAMETER(m_Imin, FE_PARAM_DOUBLE, "intact_min");
ADD_PARAMETER(m_Imax, FE_PARAM_DOUBLE, "intact_max");
ADD_PARAMETER(m_Fmin, FE_PARAM_DOUBLE, "fatigue_min");
ADD_PARAMETER(m_Fmax, FE_PARAM_DOUBLE, "fatigue_max");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
//! Constructor.
FEReactiveFatigue::FEReactiveFatigue(FEModel* pfem) : FEElasticMaterial(pfem)
{
    // set material properties
    AddProperty(&m_pBase, "elastic"          );
    AddProperty(&m_pIdmg, "intact_damage"    );
    AddProperty(&m_pFdmg, "fatigue_damage"   );
    AddProperty(&m_pCrit, "criterion"        );
}

//-----------------------------------------------------------------------------
//! Initialization.
bool FEReactiveFatigue::Init()
{
    FEUncoupledMaterial* m_pMat = dynamic_cast<FEUncoupledMaterial*>((FEElasticMaterial*)m_pBase);
    if (m_pMat != nullptr)
        return MaterialError("Elastic material should not be of type uncoupled");
    
    // allocate space and populate m_Xi and m_Xf
    m_Xi.resize(m_ndiv+1);
    m_Xf.resize(m_ndiv+1);
    double dXi = (m_Imax - m_Imin)/m_ndiv;
    double dXf = (m_Fmax - m_Fmin)/m_ndiv;
    for (int i=0; i<=m_ndiv; ++i) {
        m_Xi[i] = m_Imin + i*dXi;
        m_Xf[i] = m_Fmin + i*dXf;
        if (m_Xf[i] > m_Xi[i]) {
            return MaterialError("Damage of fatigued bonds should occur at lower threshold than that of intact bonds");
        }
    }
    
    return FEElasticMaterial::Init();
}

//-----------------------------------------------------------------------------
//! calculate stress at material point
mat3ds FEReactiveFatigue::Stress(FEMaterialPoint& pt)
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
tens4ds FEReactiveFatigue::Tangent(FEMaterialPoint& pt)
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
double FEReactiveFatigue::StrainEnergyDensity(FEMaterialPoint& pt)
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
double FEReactiveFatigue::Damage(FEMaterialPoint& pt)
{
    // get the damage material point data
    FEReactiveFatigueMaterialPoint& pd = *pt.ExtractData<FEReactiveFatigueMaterialPoint>();
    
    return pd.m_D;
}

//-----------------------------------------------------------------------------
// update fatigue material point at each iteration
void FEReactiveFatigue::UpdateSpecializedMaterialPoints(FEMaterialPoint& pt, const FETimeInfo& tp)
{
    // get the fatigue material point data
    FEReactiveFatigueMaterialPoint& pd = *pt.ExtractData<FEReactiveFatigueMaterialPoint>();
    
    // initialization
    if (tp.currentTime == tp.timeIncrement) {
        pd.m_vit.resize(m_ndiv+1);
        pd.m_vip.resize(m_ndiv+1);
        pd.m_vft.resize(m_ndiv+1);
        pd.m_vfp.resize(m_ndiv+1);
        
        double dXi = (m_Imax - m_Imin)/m_ndiv;
        pd.m_wit = pd.m_wft = 0;
        
        // evaluate initial concentration of intact bonds based on pdf
        for (int i=0; i<=m_ndiv; ++i) {
            pd.m_vit[i] = pd.m_vip[i] = m_pIdmg->pdf(m_Xi[i])*dXi;
            pd.m_wit += pd.m_vit[i];
            pd.m_vft[i] = pd.m_vfp[i] = 0;
            pd.m_wft += pd.m_vft[i];
        }
        pd.m_wip = pd.m_wit;
        pd.m_wfp = pd.m_wft;
    }
    
    double dt = tp.timeIncrement;
    
    // assume that time derivative of damage variable is constant over time interval
    double alpha = tp.alpha;
    
    // get damage and fatigue criteria at intermediate time point
    pd.m_Xitrl = pd.m_Xftrl = pd.m_Xft = (m_pCrit->DamageCriterion(pt) + (1 - alpha)*pd.m_Xfp)/alpha;
    
    // evaluate time derivative of fatigue criterion
    pd.m_dXft = pd.m_Xft - pd.m_Xfp;
    double aXft = pd.m_dXft/dt;
    
    pd.m_wit = pd.m_wft = 0;
    for (int i=0; i<=m_ndiv; ++i) {
        // check for damage of intact bonds
        if (pd.m_Xitrl >= m_Xi[i]) {
            pd.m_vit[i] = 0;
            pd.m_vft[i] = 0;
        }
        else {
            // evaluate mass supply from fatigue
//            double k = -m_k0*pow(fabs(pd.m_Xft/m_Xi[i]),m_gamma)*pow(fabs(aXft), m_beta);
            double k = -m_k0*pow(fabs(pd.m_Xft/m_Xi[i]),m_gamma)*pow(fabs(aXft)*pd.m_D, m_beta);
            // evaluate bond mass fraction after fatigue
            pd.m_vit[i] = pd.m_vip[i]*exp(k*dt);
            pd.m_vft[i] = pd.m_vfp[i] - pd.m_vit[i] + pd.m_vip[i];
            // update bond mass fractions based on mass supplies
            if (pd.m_vit[i] < 0) { pd.m_vit[i] = 0; }
            if (pd.m_vft[i] < 0) { pd.m_vft[i] = 0; }
            if (pd.m_vft[i] > 1) { pd.m_vft[i] = 1; }
            // check damage of fatigued bonds
            if (pd.m_Xftrl >= m_Xf[i]) pd.m_vft[i] = 0;
        }
        pd.m_wit += pd.m_vit[i];
        pd.m_wft += pd.m_vft[i];
    }
    // evaluate damage
    pd.m_D = 1 - alpha*(pd.m_wit + pd.m_wft) - (1-alpha)*(pd.m_wip + pd.m_wfp);
}


//-----------------------------------------------------------------------------
void FEReactiveFatigue::SetLocalCoordinateSystem(FEElement& el, int n, FEMaterialPoint& mp)
{
    FEElasticMaterial::SetLocalCoordinateSystem(el, n, mp);
    m_pBase->SetLocalCoordinateSystem(el, n, mp);
}
