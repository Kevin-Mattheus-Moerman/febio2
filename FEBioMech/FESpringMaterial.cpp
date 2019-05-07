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
#include "FESpringMaterial.h"

//-----------------------------------------------------------------------------
// FELinearSpring
//-----------------------------------------------------------------------------

// define the material parameters
BEGIN_PARAMETER_LIST(FELinearSpring, FEDiscreteMaterial)
	ADD_PARAMETER2(m_E, FE_PARAM_DOUBLE, FE_RANGE_GREATER(0.0), "E");
END_PARAMETER_LIST();

double FELinearSpring::force(double dl)
{
	return m_E*dl;
}

double FELinearSpring::stiffness(double dl)
{
	return m_E;
}

//-----------------------------------------------------------------------------
// FETensionOnlyLinearSpring
//-----------------------------------------------------------------------------

// define the material parameters
BEGIN_PARAMETER_LIST(FETensionOnlyLinearSpring, FEDiscreteMaterial)
	ADD_PARAMETER2(m_E, FE_PARAM_DOUBLE, FE_RANGE_GREATER(0.0), "E");
END_PARAMETER_LIST();

double FETensionOnlyLinearSpring::force(double dl)
{
	if (dl >= 0) return m_E*dl; else return 0;
}

double FETensionOnlyLinearSpring::stiffness(double dl)
{
	return (dl >= 0 ? m_E : 0);
}

//-----------------------------------------------------------------------------
// FENonLinearSpring
//-----------------------------------------------------------------------------

// define the material parameters
BEGIN_PARAMETER_LIST(FENonLinearSpring, FEDiscreteMaterial)
	ADD_PARAMETER(m_F, FE_PARAM_FUNC1D, "force");
END_PARAMETER_LIST();

FENonLinearSpring::FENonLinearSpring(FEModel* pfem) : FESpringMaterial(pfem), m_F(pfem)
{
}

double FENonLinearSpring::force(double dl)
{
	return m_F.value(dl);
}

double FENonLinearSpring::stiffness(double dl)
{
	return m_F.derive(dl);
}

//-----------------------------------------------------------------------------
// FEExperimentalSpring
//-----------------------------------------------------------------------------

// define the material parameters
BEGIN_PARAMETER_LIST(FEExperimentalSpring, FESpringMaterial)
	ADD_PARAMETER(m_E, FE_PARAM_DOUBLE, "E");
	ADD_PARAMETER(m_sM, FE_PARAM_DOUBLE, "sM");
	ADD_PARAMETER(m_sm, FE_PARAM_DOUBLE, "sm");
	END_PARAMETER_LIST();

FEExperimentalSpring::FEExperimentalSpring(FEModel* pfem) : FESpringMaterial(pfem)
{
	m_E = 0.0;
	m_sM = 0.0;
	m_sm = 0.0;
}

double FEExperimentalSpring::force(double dl)
{
	if (dl >= 0.0)
		return m_sM*(1.0 - exp(-m_E*dl / m_sM));
	else
		return -m_sm*(1.0 - exp(m_E*dl / m_sm));
}

double FEExperimentalSpring::stiffness(double dl)
{
	if (dl >= 0.0)
		return m_E*exp(-m_E*dl / m_sM);
	else
		return m_E*exp(m_E*dl / m_sm);
}
