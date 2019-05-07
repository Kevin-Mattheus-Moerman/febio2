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
#include "FEBioMech/FEElasticMaterial.h"
#include "FEFluid.h"

//-----------------------------------------------------------------------------
//! Biphasic material point class.
//
class FEFSIMaterialPoint : public FEMaterialPoint
{
public:
    //! constructor
    FEFSIMaterialPoint(FEMaterialPoint* pt);
    
    //! create a shallow copy
    FEMaterialPoint* Copy();
    
    //! data serialization
    void Serialize(DumpStream& ar);
    
    //! Data initialization
    void Init();
    
public:
    // FSI material data
    vec3d       m_w;      //!< fluid flux relative to solid
    vec3d       m_aw;     //!< material time derivative of m_wt
    double      m_Jdot;   //!< time derivative of solid volume ratio
};

//-----------------------------------------------------------------------------
//! Base class for FluidFSI materials.

class FEFluidFSI : public FEMaterial
{
public:
    FEFluidFSI(FEModel* pfem);
    
    // returns a pointer to a new material point object
    FEMaterialPoint* CreateMaterialPointData() override;
    
    // Get the elastic component (overridden from FEMaterial)
    FEElasticMaterial* GetElasticMaterial() override { return m_pSolid->GetElasticMaterial(); }
    
    //! performs initialization
    bool Init() override;
    
	// Set the local coordinate system
	void SetLocalCoordinateSystem(FEElement& el, int n, FEMaterialPoint& mp) override;

public:
    FEFluid* Fluid() { return m_pFluid; }
    FEElasticMaterial* Solid() { return m_pSolid; }
    
private: // material properties
    FEPropertyT<FEElasticMaterial>			m_pSolid;	//!< pointer to elastic solid material
    FEPropertyT<FEFluid>                    m_pFluid;	//!< pointer to fluid material
    
    DECLARE_PARAMETER_LIST();
};
