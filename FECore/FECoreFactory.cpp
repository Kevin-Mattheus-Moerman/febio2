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
#include "FECoreFactory.h"
#include <assert.h>

//-----------------------------------------------------------------------------
//! constructor
FECoreFactory::FECoreFactory(SUPER_CLASS_ID scid, const char* sztype) : m_scid(scid) { m_sztype = sztype; m_module = 0; }

//-----------------------------------------------------------------------------
//! virtual constructor
FECoreFactory::~FECoreFactory(){}

//-----------------------------------------------------------------------------
//! set the module ID
void FECoreFactory::SetModuleID(unsigned int mid)
{
	m_module = mid;
}

//-----------------------------------------------------------------------------
void* FECoreFactory::CreateInstance(FEModel* pfem)
{
	// create a new instance of this class
	FECoreBase* pclass = static_cast<FECoreBase*>(Create(pfem)); assert(pclass);
	if (pclass == 0) return 0;

	// make sure the super-class ID matches
	// TODO: I need to delete pclass
	if (pclass->GetSuperClassID() != GetSuperClassID()) return 0;

	// set the type string of this class
	pclass->SetTypeStr(GetTypeStr());

	// return the pointer
	return pclass;
}
