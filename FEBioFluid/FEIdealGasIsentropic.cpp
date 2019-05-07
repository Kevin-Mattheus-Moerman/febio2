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
#include "FEIdealGasIsentropic.h"
#include <FECore/FEModel.h>
#include <FECore/FECoreKernel.h>

// define the material parameters
BEGIN_PARAMETER_LIST(FEIdealGasIsentropic, FEMaterial)
ADD_PARAMETER2(m_gamma, FE_PARAM_DOUBLE, FE_RANGE_GREATER(0.0), "gamma");
ADD_PARAMETER2(m_M    , FE_PARAM_DOUBLE, FE_RANGE_GREATER(0.0), "M"    );
END_PARAMETER_LIST();

//============================================================================
// FEIdealGasIsentropic
//============================================================================

//-----------------------------------------------------------------------------
//! FEIdealGasIsentropic constructor

FEIdealGasIsentropic::FEIdealGasIsentropic(FEModel* pfem) : FEFluid(pfem)
{
    m_rhor = 0;
    m_k = 0;
    m_gamma = 0;
    m_M = 0;
}

//-----------------------------------------------------------------------------
//! initialization
bool FEIdealGasIsentropic::Init() {

    m_R  = GetFEModel()->GetGlobalConstant("R");
    m_Tr = GetFEModel()->GetGlobalConstant("T");
    m_pr = GetFEModel()->GetGlobalConstant("p");
    
    if (m_R <= 0) return MaterialError("A positive universal gas constant R must be defined in Globals section");
    if (m_Tr <= 0) return MaterialError("A positive ambient absolute temperature T must be defined in Globals section");
    if (m_pr <= 0) return MaterialError("A positive ambient absolute pressure p must be defined in Globals section");

    m_rhor = m_M*m_pr/(m_R*m_Tr);
    
    return true;
}

//-----------------------------------------------------------------------------
//! elastic pressure from dilatation
double FEIdealGasIsentropic::Pressure(const double e)
{
    double J = 1 + e;
    return m_pr*(pow(J, -m_gamma) - 1);
}

//-----------------------------------------------------------------------------
//! tangent of elastic pressure with respect to strain J
double FEIdealGasIsentropic::Tangent_Pressure_Strain(FEMaterialPoint& mp)
{
    FEFluidMaterialPoint& fp = *mp.ExtractData<FEFluidMaterialPoint>();
    double J = fp.m_Jf;
    double dp = -m_gamma*m_pr*pow(J, -m_gamma-1);
    return dp;
}

//-----------------------------------------------------------------------------
//! 2nd tangent of elastic pressure with respect to strain J
double FEIdealGasIsentropic::Tangent_Pressure_Strain_Strain(FEMaterialPoint& mp)
{
    FEFluidMaterialPoint& fp = *mp.ExtractData<FEFluidMaterialPoint>();
    double J = fp.m_Jf;
    double d2p = m_gamma*(m_gamma+1)*m_pr*pow(J, -m_gamma-2);
    return d2p;
}

//-----------------------------------------------------------------------------
//! evaluate temperature
double FEIdealGasIsentropic::Temperature(FEMaterialPoint& mp)
{
    FEFluidMaterialPoint& fp = *mp.ExtractData<FEFluidMaterialPoint>();
    double J = fp.m_Jf;
    double T = m_Tr*pow(J, 1-m_gamma);
    return T;
}

//-----------------------------------------------------------------------------
//! calculate free energy density (per reference volume)
double FEIdealGasIsentropic::StrainEnergyDensity(FEMaterialPoint& mp)
{
    FEFluidMaterialPoint& fp = *mp.ExtractData<FEFluidMaterialPoint>();
    double J = fp.m_Jf;
    double sed = m_pr*(J-1+(pow(J, 1-m_gamma)-1)/(m_gamma-1));
    return sed;
}

//-----------------------------------------------------------------------------
//! invert pressure-dilatation relation
double FEIdealGasIsentropic::Dilatation(const double p)
{
    double J = pow(p/m_pr+1, -1./m_gamma);
    return J - 1;
}

