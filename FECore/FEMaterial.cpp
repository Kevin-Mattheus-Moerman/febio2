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
#include "FEMaterial.h"
#include <math.h>
#include <stdarg.h>
#include "FECoreKernel.h"

//-----------------------------------------------------------------------------
bool MaterialError(const char* szfmt, ...)
{
	// get a pointer to the argument list
	va_list	args;

	// create the message
	char szerr[512] = {0};
	va_start(args, szfmt);
	vsprintf(szerr, szfmt, args);
	va_end(args);

	return fecore_error(szerr);
}

//-----------------------------------------------------------------------------
FEMaterial::FEMaterial(FEModel* pfem) : FECoreBase(FEMATERIAL_ID), m_pfem(pfem)
{
	static int n = 1;
	m_nRB = -1;

	AddProperty(&m_map, "mat_axis", 0);
}

//-----------------------------------------------------------------------------
FEMaterial::~FEMaterial()
{
	if (m_map) delete m_map;
}

//-----------------------------------------------------------------------------
//! Get the model this material belongs to
FEModel* FEMaterial::GetFEModel()
{
	return m_pfem;
}

//-----------------------------------------------------------------------------
void FEMaterial::SetCoordinateSystemMap(FECoordSysMap* pmap)
{
	m_map = pmap;
}

//-----------------------------------------------------------------------------
FECoordSysMap* FEMaterial::GetCoordinateSystemMap()
{
	return m_map;
}

//-----------------------------------------------------------------------------
//! This function does nothing here. Derived classes will use this to set the 
//! local coordinate systems for material points.
void FEMaterial::SetLocalCoordinateSystem(FEElement& el, int n, FEMaterialPoint& mp)
{
	
}

//-----------------------------------------------------------------------------
//! Initial material.
bool FEMaterial::Init()
{
	// initialize material axes
	if (m_map) m_map->Init();

	// initialize base class
	return FECoreBase::Init();
}

//-----------------------------------------------------------------------------
//! Store the material data to the archive
void FEMaterial::Serialize(DumpStream &ar)
{
	// We don't need to serialize material data for shallow copies.
	if (ar.IsShallow()) return;

	if (ar.IsSaving())
	{
		ar << m_nRB;

		// save the local coodinate system generator
		int nmap = (m_map ? 1 : 0);
		ar << nmap;
		if (m_map)
		{
			ar << m_map->GetTypeStr();
			m_map->Serialize(ar);
		}
	}
	else
	{
		ar >> m_nRB;

		// read the local cordinate system
		int nmap;
		ar >> nmap;
		if (m_map) delete m_map;
		m_map = 0;

		if (nmap)
		{
			FEModel& pfem = ar.GetFEModel();

			char sztype[64]={0};
			ar >> sztype;
			m_map = fecore_new<FECoordSysMap>(FECOORDSYSMAP_ID, sztype, &pfem);
			m_map->Serialize(ar);
		}
	}

	// Save the material's parameters
	FECoreBase::Serialize(ar);
}
