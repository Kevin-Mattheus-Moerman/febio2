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
#include "FEMaterialPoint.h"
#include <string.h>

FEMaterialPoint::FEMaterialPoint(FEMaterialPoint* ppt)
{
	m_pPrev = 0;
	m_pNext = ppt;
	m_szname = 0;
	if (ppt) ppt->m_pPrev = this;
}

FEMaterialPoint::~FEMaterialPoint()
{ 
	if (m_pNext) delete m_pNext;
	m_pNext = m_pPrev = 0;
}

void FEMaterialPoint::SetPrev(FEMaterialPoint* pt)
{
	m_pPrev = pt;
}

// TODO: What if the next pointer is already assigned?
void FEMaterialPoint::SetNext(FEMaterialPoint* pt)
{
	m_pNext = pt;
	pt->m_pPrev = this;
}

void FEMaterialPoint::Init()
{
	if (m_pNext) m_pNext->Init();
}

void FEMaterialPoint::Update(const FETimeInfo& timeInfo)
{
	if (m_pNext) m_pNext->Update(timeInfo);
}

void FEMaterialPoint::Serialize(DumpStream& ar)
{
	FEParamContainer::Serialize(ar);
	if (m_pNext) m_pNext->Serialize(ar);
}

// find a parameter with a given name
FEParam* FEMaterialPoint::FindParameter(const std::string& paramName)
{
	if (paramName.empty()) return 0;

	const char* szname = paramName.c_str();

	// see if there is a dot
	const char* ch = strchr(szname, '.');

	if (ch == 0)
	{
		FEMaterialPoint* pt = this;
		while (pt)
		{
			FEParameterList& pl = pt->GetParameterList();
			FEParam* p = pl.FindFromName(szname);
			if (p) return p;
			else pt = pt->Next();
		}
	}
	else
	{
		int l = ch - szname;
		const char* mpName = GetName();
		if (mpName && (strncmp(mpName, szname, l) == 0)) return FindParameter(szname + l + 1);
	}
	return 0;
}

//-----------------------------------------------------------------------------
FEMaterialPointArray::FEMaterialPointArray(FEMaterialPoint* ppt) : FEMaterialPoint(ppt)
{
	
}

//-----------------------------------------------------------------------------
void FEMaterialPointArray::AddMaterialPoint(FEMaterialPoint* pt)
{
	m_mp.push_back(pt);
	pt->SetPrev(this);
}


//-----------------------------------------------------------------------------
void FEMaterialPointArray::Init()
{
	for (int i = 0; i<(int)m_mp.size(); ++i) m_mp[i]->Init();
}

//-----------------------------------------------------------------------------
void FEMaterialPointArray::Serialize(DumpStream& ar)
{
	for (int i = 0; i<(int)m_mp.size(); ++i) m_mp[i]->Serialize(ar);
}

//-----------------------------------------------------------------------------
void FEMaterialPointArray::Update(const FETimeInfo& timeInfo)
{
	FEMaterialPoint::Update(timeInfo);
	for (int i = 0; i<(int)m_mp.size(); ++i) m_mp[i]->Update(timeInfo);
}

//-----------------------------------------------------------------------------
// find a parameter with a given name
FEParam* FEMaterialPointArray::FindParameter(const char* szname)
{
	if (szname == 0) return 0;

	// see if there is a dot
	const char* ch = strchr(szname, '.');

	// if not, proceed as usual
	if (ch == 0) return FEMaterialPoint::FindParameter(szname);

	// if there is, get the index
	const char* lb = strchr(szname, '[');
	const char* rb = strchr(szname, ']');
	if ((lb == 0) || (rb == 0) || (lb > ch) || (lb > rb)) return 0;

	int index = atoi(lb+1);
	if ((index < 0) || (index >= Components())) return 0;

	const char* thisName = GetName();
	if (thisName)
	{
		int l = ch - szname;
		int n = lb - szname;
		if (strncmp(thisName, szname, n) == 0)
		{
			FEMaterialPoint& pi = *m_mp[index];
			return pi.FindParameter(szname + l + 1);
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
void FEMaterialPointArray::BuildParamList()
{
	FEMaterialPoint::BuildParamList();
	for (int i=0; i<Components(); ++i)
	{
		m_mp[i]->BuildParamList();
	}
}
