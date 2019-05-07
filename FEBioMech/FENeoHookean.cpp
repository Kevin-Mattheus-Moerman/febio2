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
#include "FENeoHookean.h"

//-----------------------------------------------------------------------------
// define the material parameters
BEGIN_PARAMETER_LIST(FENeoHookean, FEElasticMaterial)
	ADD_PARAMETER2(m_E, FE_PARAM_DOUBLE, FE_RANGE_GREATER   (      0.0), "E");
	ADD_PARAMETER2(m_v, FE_PARAM_DOUBLE, FE_RANGE_RIGHT_OPEN(-1.0, 0.5), "v");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
mat3ds FENeoHookean::Stress(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();

	double detF = pt.m_J;
	double detFi = 1.0/detF;
	double lndetF = log(detF);

	// calculate left Cauchy-Green tensor
	mat3ds b = pt.LeftCauchyGreen();

	// lame parameters
	double lam = m_v*m_E/((1+m_v)*(1-2*m_v));
	double mu  = 0.5*m_E/(1+m_v);

	// Identity
	mat3dd I(1);

	// calculate stress
	mat3ds s = (b - I)*(mu*detFi) + I*(lam*lndetF*detFi);

	return s;
}

//-----------------------------------------------------------------------------
tens4ds FENeoHookean::Tangent(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();

	// deformation gradient
	double detF = pt.m_J;

	// lame parameters
	double lam = m_v*m_E/((1+m_v)*(1-2*m_v));
	double mu  = 0.5*m_E/(1+m_v);

	double lam1 = lam / detF;
	double mu1  = (mu - lam*log(detF)) / detF;
	
	double D[6][6] = {0};
	D[0][0] = lam1+2.*mu1; D[0][1] = lam1       ; D[0][2] = lam1       ;
	D[1][0] = lam1       ; D[1][1] = lam1+2.*mu1; D[1][2] = lam1       ;
	D[2][0] = lam1       ; D[2][1] = lam1       ; D[2][2] = lam1+2.*mu1;
	D[3][3] = mu1;
	D[4][4] = mu1;
	D[5][5] = mu1;

	return tens4ds(D);
}

//-----------------------------------------------------------------------------
double FENeoHookean::StrainEnergyDensity(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
	
	double J = pt.m_J;
	double lnJ = log(J);
	
	// calculate left Cauchy-Green tensor
	mat3ds b = pt.LeftCauchyGreen();
	double I1 = b.tr();
	
	// lame parameters
	double lam = m_v*m_E/((1+m_v)*(1-2*m_v));
	double mu  = 0.5*m_E/(1+m_v);
	
	double sed = mu*((I1-3)/2.0 - lnJ)+lam*lnJ*lnJ/2.0;
	
	return sed;
}

//-----------------------------------------------------------------------------
mat3ds FENeoHookean::PK2Stress(FEMaterialPoint& pt, const mat3ds E)
{
    // Identity
    mat3dd I(1);
    
    // calculate right Cauchy-Green tensor
    mat3ds C = I + E*2;
    mat3ds Ci = C.inverse();
    
    double detF = sqrt(C.det());
    double lndetF = log(detF);
    
    
    // lame parameters
    double lam = m_v*m_E/((1+m_v)*(1-2*m_v));
    double mu  = 0.5*m_E/(1+m_v);
    
    // calculate stress
    mat3ds S = (I - Ci)*mu + Ci*(lam*lndetF);
    
    return S;
}

//-----------------------------------------------------------------------------
tens4ds FENeoHookean::MaterialTangent(FEMaterialPoint& pt, const mat3ds E)
{
    // calculate right Cauchy-Green tensor
    mat3ds C = mat3dd(1) + E*2;
    mat3ds Ci = C.inverse();
    double J = sqrt(C.det());
    
    // lame parameters
    double lam = m_v*m_E/((1+m_v)*(1-2*m_v));
    double mu  = 0.5*m_E/(1+m_v);
    
    tens4ds c = dyad1s(Ci)*lam + dyad4s(Ci)*(2*(mu-lam*log(J)));
    
    return c;
}
