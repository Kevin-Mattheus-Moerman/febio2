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

//-----------------------------------------------------------------------------
//! Rigd body material data

//! Since rigid elements are skipped during the stiffness and residual calculations
//! we don't implement the Stress and Tangent functions
//! \todo make the m_rc a parameter
//! \todo Can I remove the m_bc variable?

class FERigidMaterial : public FESolidMaterial
{
public:
	FERigidMaterial(FEModel* pfem);

public:
	double	m_E;		//!< Young's modulus
	double	m_v;		//!< Poisson's ratio
	int		m_pmid;		//!< parent material ID

public:
	int		m_com;	//!< center of mass input flag
	vec3d	m_rc;	//!< center of mass

public:
	// inherited from FEMaterial
	virtual bool IsRigid() override { return true; }

	// override this function to set the COM logic
	void SetParameter(FEParam& p) override;

public:
	//! Create a rigid material point
	FEMaterialPoint* CreateMaterialPointData() override { return new FEElasticMaterialPoint(); }

	//! calculate stress at material point
	virtual mat3ds Stress(FEMaterialPoint& pt) override { return mat3ds(); }

	//! calculate tangent stiffness at material point
	virtual tens4ds Tangent(FEMaterialPoint& pt) override { return tens4ds(); }

	//! data initialization
	bool Init() override;

	//! serialization
	void Serialize(DumpStream& ar) override;

	// declare a parameter list
	DECLARE_PARAMETER_LIST();

private:
	bool	m_binit;	//!< flag for first initialization
};
