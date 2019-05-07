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
#include "FEElasticMaterial.h"
#include "FEDamageCriterion.h"
#include "FEDamageCDF.h"
#include <vector>

//-----------------------------------------------------------------------------
// Define a material point that stores the fatigue and damage variables.
class FEReactiveFatigueMaterialPoint : public FEMaterialPoint
{
public:
    FEReactiveFatigueMaterialPoint(FEMaterialPoint *pt) : FEMaterialPoint(pt) {}
    
    FEMaterialPoint* Copy();
    
    void Init();
    void Update(const FETimeInfo& timeInfo);
    
    void Serialize(DumpStream& ar);
    
public:
    double      m_D;            //!< damage (0 = no damage, 1 = complete damage)
    double      m_wit;          //!< intact bond fraction at current time
    double      m_wft;          //!< fatigued bond fraction at current time
    double      m_wip;          //!< intact bond fraction at previous time
    double      m_wfp;          //!< fatigued bond fraction at previous time

    vector<double>      m_vip;          //!< intact bond fraction at previous time
    vector<double>      m_vit;          //!< intact bond fraction at current time
    
    vector<double>      m_vfp;          //!< fatigued bond fraction at previous time
    vector<double>      m_vft;          //!< fatigued bond fraction at current time
    
    double      m_Ximax;        //!< max damage criterion for intact bonds
    double      m_Xitrl;        //!< trial value of Ximax
    
    double      m_Xfmax;        //!< max damage criterion for fatigued bonds
    double      m_Xftrl;        //!< trial value of Xfmax
    
    double      m_Xfp;          //!< fatigue criterion at previous time
    double      m_Xft;          //!< fatigue criterion at current time
    
    double      m_dXfp;          //!< incremental change in fatigue criterion at previous time
    double      m_dXft;          //!< incremental change in fatigue criterion at current time
};

//-----------------------------------------------------------------------------
// This material models fatigue and damage in any hyper-elastic materials.

class FEReactiveFatigue : public FEElasticMaterial
{
public:
    FEReactiveFatigue(FEModel* pfem);
    
public:
    //! calculate stress at material point
    mat3ds Stress(FEMaterialPoint& pt) override;
    
    //! calculate tangent stiffness at material point
    tens4ds Tangent(FEMaterialPoint& pt) override;
    
    //! calculate strain energy density at material point
    double StrainEnergyDensity(FEMaterialPoint& pt) override;
    
    //! damage
    double Damage(FEMaterialPoint& pt);
    
    //! data initialization and checking
    bool Init() override;
    
    // returns a pointer to a new material point object
    FEMaterialPoint* CreateMaterialPointData() override
    {
        return new FEReactiveFatigueMaterialPoint(m_pBase->CreateMaterialPointData());
    }
    
    // get the elastic material
    FEElasticMaterial* GetElasticMaterial() override { return m_pBase; }
    
    // update fatigue material point at each iteration
    void UpdateSpecializedMaterialPoints(FEMaterialPoint& mp, const FETimeInfo& tp) override;
    
public:
    
    //! Set the local coordinate system for a material point (overridden from FEMaterial)
    void SetLocalCoordinateSystem(FEElement& el, int n, FEMaterialPoint& mp) override;
    
public:
    FEPropertyT<FEElasticMaterial>  m_pBase;    // base elastic material
    FEPropertyT<FEDamageCDF>        m_pIdmg;    // damage model for intact bonds
    FEPropertyT<FEDamageCDF>        m_pFdmg;    // damage model for fatigued bonds
    FEPropertyT<FEDamageCriterion>  m_pCrit;    // damage and fatigue criterion
    
public:
    double      m_k0;       // reaction rate for fatigue reaction
    double      m_beta;     // power exponent of fatigue measure time derivative for fatigue reaction
    double      m_gamma;    // power exponent of fatigue measure for fatigue reaction
    int         m_ndiv;     // number of divisions of PDF/CDF domain
    double      m_Imin;     // range of fatigue measure for intact bonds
    double      m_Imax;     // range of fatigue measure for intact bonds
    double      m_Fmin;     // range of fatigure measure for fatigued bonds
    double      m_Fmax;     // range of fatigure measure for fatigued bonds

    vector<double>  m_Xi;   // values of failure measure of intact bonds at all intervals
    vector<double>  m_Xf;   // values of failure measure of fatigued bonds at all intervals

    DECLARE_PARAMETER_LIST();
};
