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
#include "mat3d.h"
#include "FECoreBase.h"

//-----------------------------------------------------------------------------
// Forward declarations
class FEModel;
class FEMesh;
class FEElement;

//-----------------------------------------------------------------------------
//! The FECoordSysMap class is used to create local coordinate systems.

class FECORE_API FECoordSysMap : public FECoreBase
{
public:
	FECoordSysMap(FEModel* pfem);
	virtual ~FECoordSysMap();

	//! initialization
	bool Init();

	//! return the local coordinate system at an element's gauss point
	virtual mat3d LocalElementCoord(FEElement& el, int n) = 0;

	//! serialization
	virtual void Serialize(DumpStream& ar) = 0;

public:
	FEModel* GetFEModel() { return m_pfem; }

private:
	FEModel*	m_pfem;
};

//-----------------------------------------------------------------------------
//! This class generates a material axes based on the local element node numbering.
class FECORE_API FELocalMap : public FECoordSysMap
{
public:
	FELocalMap(FEModel* pfem);

	bool Init() override;

	void SetLocalNodes(int n1, int n2, int n3);

	mat3d LocalElementCoord(FEElement& el, int n) override;

	virtual void Serialize(DumpStream& ar) override;

public:
	int			m_n[3];	// local element nodes

protected:
	DECLARE_PARAMETER_LIST();
};

//-----------------------------------------------------------------------------
//! This class generates material axes based on a spherical map. 
class FECORE_API FESphericalMap : public FECoordSysMap
{
public:
	FESphericalMap(FEModel* pfem);

	bool Init() override;

	void SetSphereCenter(const vec3d& c) { m_c = c; }

	void SetSphereVector(const vec3d& r) { m_r = r;}

	mat3d LocalElementCoord(FEElement& el, int n) override;

	virtual void Serialize(DumpStream& ar) override;

public:
	vec3d		m_c;	// center of map
	vec3d		m_r;	// vector for parallel transport

protected:
	DECLARE_PARAMETER_LIST();
};


//-----------------------------------------------------------------------------

class FECORE_API FECylindricalMap : public FECoordSysMap
{
public:
	FECylindricalMap(FEModel* pfem);

	bool Init() override;

	void SetCylinderCenter(vec3d c) { m_c = c; }

	void SetCylinderAxis(vec3d a) { m_a = a; m_a.unit(); }

	void SetCylinderRef(vec3d r) { m_r = r; m_r.unit(); }

	mat3d LocalElementCoord(FEElement& el, int n) override;

	virtual void Serialize(DumpStream& ar) override;

public:
	vec3d		m_c;	// center of map
	vec3d		m_a;	// axis
	vec3d		m_r;	// reference direction

protected:
	DECLARE_PARAMETER_LIST();
};

//-----------------------------------------------------------------------------

class FECORE_API FEPolarMap : public FECoordSysMap
{
public:
	FEPolarMap(FEModel* pfem);

	bool Init() override;

	void SetCenter(vec3d c) { m_c = c; }

	void SetAxis(vec3d a) { m_a = a; m_a.unit(); }

	void SetVector0(vec3d r) { m_d0 = r; m_d0.unit(); }
	void SetVector1(vec3d r) { m_d1 = r; m_d1.unit(); }

	void SetRadius0(double r) { m_R0 = r; }
	void SetRadius1(double r) { m_R1 = r; }

	mat3d LocalElementCoord(FEElement& el, int n) override;

	virtual void Serialize(DumpStream& ar) override;

public:
	vec3d		m_c;		// center of map
	vec3d		m_a;		// axis
	vec3d		m_d0, m_d1;	// reference direction
	double		m_R0, m_R1;

protected:
	DECLARE_PARAMETER_LIST();
};

//-----------------------------------------------------------------------------

class FECORE_API FEVectorMap : public FECoordSysMap
{
public:
	FEVectorMap(FEModel* pfem);

	bool Init() override;

	void SetVectors(vec3d a, vec3d d);

	mat3d LocalElementCoord(FEElement& el, int n) override;

	virtual void Serialize(DumpStream& ar) override;

public:
	vec3d	m_a, m_d;

	DECLARE_PARAMETER_LIST();
};

//-----------------------------------------------------------------------------
class FECORE_API FESphericalAngleMap : public FECoordSysMap
{
public:
	FESphericalAngleMap(FEModel* pfem);

	bool Init() override;

	void SetAngles(double theta, double phi);

	mat3d LocalElementCoord(FEElement& el, int n) override;

	virtual void Serialize(DumpStream& ar) override;

public:
	double	m_theta;
	double	m_phi;

	DECLARE_PARAMETER_LIST();
};
