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
#include "FEDamageMaterialPoint.h"
#include <FECore/DumpStream.h>

#ifdef WIN32
#define max(a,b) ((a)>(b)?(a):(b))
#endif

FEMaterialPoint* FEDamageMaterialPoint::Copy()
{
    FEDamageMaterialPoint* pt = new FEDamageMaterialPoint(*this);
    if (m_pNext) pt->m_pNext = m_pNext->Copy();
    return pt;
}

void FEDamageMaterialPoint::Init()
{
	FEMaterialPoint::Init();

	// intialize data to zero
	m_Emax = 0;
	m_Etrial = 0;
	m_D = 0;
}

void FEDamageMaterialPoint::Update(const FETimeInfo& timeInfo)
{
	FEMaterialPoint::Update(timeInfo);

	m_Emax = max(m_Emax, m_Etrial);
}

void FEDamageMaterialPoint::Serialize(DumpStream& ar)
{
    if (ar.IsSaving())
    {
        ar << m_Etrial << m_Emax << m_D;
    }
    else
    {
        ar >> m_Etrial >> m_Emax >> m_D;
    }
	FEMaterialPoint::Serialize(ar);
}
