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
#include "FENonConstBodyForce.h"
#include "FEElasticMaterial.h"

BEGIN_PARAMETER_LIST(FENonConstBodyForce, FEBodyForce);
	ADD_PARAMETER(m_val[0], FE_PARAM_MATH_DOUBLE, "x");
	ADD_PARAMETER(m_val[1], FE_PARAM_MATH_DOUBLE, "y");
	ADD_PARAMETER(m_val[2], FE_PARAM_MATH_DOUBLE, "z");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FENonConstBodyForce::FENonConstBodyForce(FEModel* pfem) : FEBodyForce(pfem)
{
}

//-----------------------------------------------------------------------------
vec3d FENonConstBodyForce::force(FEMaterialPoint &mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();

	// get the material point's spatial position
	vec3d r0 = pt.m_r0;

	// calculate the force
	double f[3] = { 0 };
	for (int i = 0; i<3; ++i)
	{
		m_val[i].setVariable("X", r0.x);
		m_val[i].setVariable("Y", r0.y);
		m_val[i].setVariable("Z", r0.z);

		f[i] = m_val[i].value();
	}

	return vec3d(f[0], f[1], f[2]);
}

//-----------------------------------------------------------------------------
mat3ds FENonConstBodyForce::stiffness(FEMaterialPoint& pt)
{
	return mat3ds(0, 0, 0, 0, 0, 0);
}
