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
#include "FEFluidFSI.h"
#include <FECore/FECoreKernel.h>

//-----------------------------------------------------------------------------
// Material parameters for the FEFluidFSI material
BEGIN_PARAMETER_LIST(FEFluidFSI, FEMaterial)
END_PARAMETER_LIST();

//============================================================================
// FEFSIMaterialPoint
//============================================================================
FEFSIMaterialPoint::FEFSIMaterialPoint(FEMaterialPoint* pt) : FEMaterialPoint(pt) {}

//-----------------------------------------------------------------------------
FEMaterialPoint* FEFSIMaterialPoint::Copy()
{
    FEFSIMaterialPoint* pt = new FEFSIMaterialPoint(*this);
    if (m_pNext) pt->m_pNext = m_pNext->Copy();
    return pt;
}

//-----------------------------------------------------------------------------
void FEFSIMaterialPoint::Serialize(DumpStream& ar)
{
    if (ar.IsSaving())
    {
        ar << m_w << m_aw << m_Jdot;
    }
    else
    {
        ar >> m_w >> m_aw >> m_Jdot;
    }
    
    FEMaterialPoint::Serialize(ar);
}

//-----------------------------------------------------------------------------
void FEFSIMaterialPoint::Init()
{
    m_w = m_aw = vec3d(0,0,0);
    m_Jdot = 0;
    
    FEMaterialPoint::Init();
}

//============================================================================
// FEFluidFSI
//============================================================================

//-----------------------------------------------------------------------------
//! FEFluidFSI constructor

FEFluidFSI::FEFluidFSI(FEModel* pfem) : FEMaterial(pfem)
{
    // set material properties
    AddProperty(&m_pSolid, "solid");
    AddProperty(&m_pFluid, "fluid");
}

//-----------------------------------------------------------------------------
// returns a pointer to a new material point object
FEMaterialPoint* FEFluidFSI::CreateMaterialPointData()
{
    FEFluidMaterialPoint* fpt = new FEFluidMaterialPoint(m_pSolid->CreateMaterialPointData());
    return new FEFSIMaterialPoint(fpt);
}

//-----------------------------------------------------------------------------
// Set the local coordinate system
void FEFluidFSI::SetLocalCoordinateSystem(FEElement& el, int n, FEMaterialPoint& mp)
{
	FEElasticMaterial* pme = GetElasticMaterial();
	pme->SetLocalCoordinateSystem(el, n, mp);
}

//-----------------------------------------------------------------------------
// initialize
bool FEFluidFSI::Init()
{
    // set the solid density to zero (required for the solid of a FSI domain)
    m_pSolid->SetDensity(0.0);
    
    return FEMaterial::Init();
}
