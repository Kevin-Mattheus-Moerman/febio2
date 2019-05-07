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
#include "FESolidMaterial.h"

//-----------------------------------------------------------------------------
//! This class defines material point data for elastic materials.
class FEElasticMaterialPoint : public FEMaterialPoint
{
public:
	//! constructor
	FEElasticMaterialPoint();

	//! Initialize material point data
	void Init();

	//! create a shallow copy
	FEMaterialPoint* Copy();

	//! serialize material point data
	void Serialize(DumpStream& ar);

public:
	mat3ds Strain();
	mat3ds SmallStrain();

	mat3ds RightCauchyGreen();
	mat3ds LeftCauchyGreen ();

	mat3ds DevRightCauchyGreen();
	mat3ds DevLeftCauchyGreen ();
    
    mat3ds RateOfDeformation() { return m_L.sym(); }

	mat3ds pull_back(const mat3ds& A);
	mat3ds push_forward(const mat3ds& A);

	tens4ds pull_back(const tens4ds& C);
	tens4ds push_forward(const tens4ds& C);

public:
    bool    m_buncoupled;   //!< set to true if this material point was created by an uncoupled material
	mat3d	m_Q0;			//!< initial material orientation
    mat3d   m_Q;            //!< global material orientation
    
	// position
	vec3d	m_r0;	//!< material position

	// deformation data at intermediate time
    vec3d   m_rt;   //!< spatial position
	mat3d	m_F;	//!< deformation gradient
	double	m_J;	//!< determinant of F
    vec3d   m_v;    //!< velocity
    vec3d   m_a;    //!< acceleration
    mat3d   m_L;    //!< spatial velocity gradient

	// solid material data
	mat3ds		m_s;		//!< Cauchy stress
	mat3ds		m_s0;		//!< Initial stress (only used by linear solid solver)
    
    // current time data
    double      m_Wt;       //!< strain energy density at current time
    
    // previous time data
    double      m_Wp;       //!< strain energy density
};

//-----------------------------------------------------------------------------
//! Base class for (hyper-)elastic materials

class FEElasticMaterial : public FESolidMaterial
{
public:
	//! constructor 
	FEElasticMaterial(FEModel* pfem);

	//! destructor
	~FEElasticMaterial();

	//! Initialization
	bool Validate();

	//! create material point data for this material
	virtual FEMaterialPoint* CreateMaterialPointData() { return new FEElasticMaterialPoint; }

	//! calculate strain energy density at material point
	virtual double StrainEnergyDensity(FEMaterialPoint& pt);
    
	//! Get the elastic component
	FEElasticMaterial* GetElasticMaterial() { return this; }

	//! Set the local coordinate system for a material point (overridden from FEMaterial)
	void SetLocalCoordinateSystem(FEElement& el, int n, FEMaterialPoint& mp);

public:
	bool SetAttribute(const char* szatt, const char* szval);
};
