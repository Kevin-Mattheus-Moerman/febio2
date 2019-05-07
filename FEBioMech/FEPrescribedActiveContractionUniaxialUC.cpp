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
#include "FEPrescribedActiveContractionUniaxialUC.h"

//-----------------------------------------------------------------------------
// define the material parameters
BEGIN_PARAMETER_LIST(FEPrescribedActiveContractionUniaxialUC, FEUncoupledMaterial)
ADD_PARAMETER(m_T0 , FE_PARAM_DOUBLE, "T0"   );
ADD_PARAMETER(m_thd, FE_PARAM_DOUBLE, "theta");
ADD_PARAMETER(m_phd, FE_PARAM_DOUBLE, "phi"  );
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FEPrescribedActiveContractionUniaxialUC::FEPrescribedActiveContractionUniaxialUC(FEModel* pfem) : FEUncoupledMaterial(pfem)
{
    m_thd = 0;
    m_phd = 90;
}

//-----------------------------------------------------------------------------
bool FEPrescribedActiveContractionUniaxialUC::Validate()
{
	if (FEUncoupledMaterial::Validate() == false) return false;

    // convert angles from degrees to radians
    double pi = 4*atan(1.0);
    double the = m_thd*pi/180.;
    double phi = m_phd*pi/180.;
    // fiber direction in local coordinate system (reference configuration)
    m_n0.x = cos(the)*sin(phi);
    m_n0.y = sin(the)*sin(phi);
    m_n0.z = cos(phi);

	return true;
}

//-----------------------------------------------------------------------------
void FEPrescribedActiveContractionUniaxialUC::Serialize(DumpStream& ar)
{
	FEUncoupledMaterial::Serialize(ar);
	if (ar.IsShallow() == false)
	{
		if (ar.IsSaving())
		{
			ar << m_n0;
		}
		else
		{
			ar >> m_n0;
		}
	}
}

//-----------------------------------------------------------------------------
mat3ds FEPrescribedActiveContractionUniaxialUC::DevStress(FEMaterialPoint &mp)
{
    FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
    
    // deformation gradient
    double J = pt.m_J;
    mat3d F = pt.m_F;
    
    // get the initial fiber direction
    vec3d n0, nt;
    
    // evaluate fiber direction in global coordinate system
    n0 = pt.m_Q*m_n0;
    
    // evaluate the deformed fiber direction
    nt = F*n0;
    mat3ds N = dyad(nt);
    
    // evaluate the active stress
    mat3ds s = N*(m_T0/J);
    
    return s;
}

//-----------------------------------------------------------------------------
tens4ds FEPrescribedActiveContractionUniaxialUC::DevTangent(FEMaterialPoint &mp)
{
    return tens4ds(0.0);
}
