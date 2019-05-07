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

#include "tens4d.h"
#include "FECoreBase.h"
#include "FEMaterialPoint.h"
#include "FECoordSysMap.h"
#include "DumpStream.h"
#include "FECoreKernel.h"
#include "FEProperty.h"
#include <string.h>
#include <stddef.h>

#define INRANGE(x, a, b) ((x)>=(a) && (x)<=(b))
#define IN_RIGHT_OPEN_RANGE(x, a, b) ((x)>=(a) && (x)<(b))

//-----------------------------------------------------------------------------
// forward declaration of some classes
class FEModel;
class FEElement;

//-----------------------------------------------------------------------------
//! helper functions for reporting material errors

bool FECORE_API MaterialError(const char* sz, ...);

//-----------------------------------------------------------------------------
// Forward declaration of the FEElasticMaterial class. 
// TODO: The only reason I had to do this is to define the FEMaterial::GetElasticMaterial.
// However, this is only a temporary construct so make sure to delete this forward declaration
// when no longer needed.
class FEElasticMaterial;

//-----------------------------------------------------------------------------
//! Abstract base class for material types

//! From this class all other material classes are derived.

class FECORE_API FEMaterial : public FECoreBase
{
public:
	FEMaterial(FEModel* pfem);
	virtual ~FEMaterial();

	//! returns a pointer to a new material point object
	virtual FEMaterialPoint* CreateMaterialPointData() { return 0; };

	//! performs initialization
	bool Init();

	//! Serialize material data to archive
	void Serialize(DumpStream& ar);

	//! Return elastic material \todo I need to move this function up the hierarchy once I redesign the material library
	virtual FEElasticMaterial* GetElasticMaterial() { return 0; }

    //! Update specialized material points at each iteration
    virtual void UpdateSpecializedMaterialPoints(FEMaterialPoint& mp, const FETimeInfo& tp) {}
    
public:
	// TODO: Some rigid body stuff is moved to here to avoid RTTI and the definition of rigid materials in FECore, 
	//       as well as simplify some initialization. I hope someday to refactor this a bit.
	//! is this a rigid material
	virtual bool IsRigid() { return false; }

	//! get the ID of the rigid body this material is assigned to (-1 if not)
	int GetRigidBodyID() { return m_nRB; }

	//! Set the rigid body ID this material is assigned to
	void SetRigidBodyID(int rid) { m_nRB = rid; }

	//! return the density
	//! TODO: This was added here because the rigid bodies need it to determine the COM
	virtual double Density() { return 0.0; }

public:
	//! Set the local coordinate system map
	void SetCoordinateSystemMap(FECoordSysMap* pmap);

	//! Get the local coordinate system
	FECoordSysMap* GetCoordinateSystemMap();

	//! Set the local coordinate for a material point
	virtual void SetLocalCoordinateSystem(FEElement& el, int n, FEMaterialPoint& mp);

	//! return the model this material belongs to
	FEModel* GetFEModel();

private:
	int		m_nRB;			//!< rigid body ID (TODO: I hope to remove this sometime)

private:
	FEPropertyT<FECoordSysMap>	m_map;			//!< local material coordinate system
	FEModel*		m_pfem;			//!< pointer to model this material belongs to
};
