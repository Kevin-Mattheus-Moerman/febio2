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
#include "FEEFDMooneyRivlin.h"

// define the material parameters
BEGIN_PARAMETER_LIST(FEEFDMooneyRivlin, FEUncoupledMaterial)
	ADD_PARAMETER(m_MR.c1, FE_PARAM_DOUBLE, "c1");
	ADD_PARAMETER(m_MR.c2, FE_PARAM_DOUBLE, "c2");
	ADD_PARAMETERV(m_EFD.m_beta, FE_PARAM_DOUBLE, 3, "beta");
	ADD_PARAMETERV(m_EFD.m_ksi , FE_PARAM_DOUBLE, 3, "ksi" );
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FEEFDMooneyRivlin::FEEFDMooneyRivlin(FEModel* pfem) : FEUncoupledMaterial(pfem), m_EFD(pfem), m_MR(pfem)
{
	// although we don't use K of the child materials, we need to set it to a non-zero value
	// otherwise FEBio will complain
	m_MR.m_K = 1.0;
	m_EFD.m_K = 1.0;
}

//-----------------------------------------------------------------------------
bool FEEFDMooneyRivlin::Init()
{
	if (FEUncoupledMaterial::Init() == false) return false;
	if (m_MR.Init() == false) return false;
	if (m_EFD.Init() == false) return false;
	return true;
}

//-----------------------------------------------------------------------------
void FEEFDMooneyRivlin::Serialize(DumpStream& ar)
{
	FEUncoupledMaterial::Serialize(ar);
	m_MR.Serialize(ar);
	m_EFD.Serialize(ar);
}

//-----------------------------------------------------------------------------
mat3ds FEEFDMooneyRivlin::DevStress(FEMaterialPoint& pt)
{
	return m_MR.DevStress(pt) + m_EFD.DevStress(pt);
}

//-----------------------------------------------------------------------------
tens4ds FEEFDMooneyRivlin::DevTangent(FEMaterialPoint& pt)
{
	return m_MR.DevTangent(pt) + m_EFD.DevTangent(pt);
}

//-----------------------------------------------------------------------------
//! calculate deviatoric strain energy density
double FEEFDMooneyRivlin::DevStrainEnergyDensity(FEMaterialPoint& pt)
{
    return m_MR.DevStrainEnergyDensity(pt) + m_EFD.DevStrainEnergyDensity(pt);
}
