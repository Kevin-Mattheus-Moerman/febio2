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
#include "FECoreBase.h"
#include "DumpStream.h"
#include "FECoreKernel.h"

//-----------------------------------------------------------------------------
//! The constructor takes one argument, namely the SUPER_CLASS_ID which
//! defines the type of class this is. (The SUPER_CLASS_ID was introduced to
//! eliminate a lot of akward dynamic_casts.)
FECoreBase::FECoreBase(SUPER_CLASS_ID sid) : m_sid(sid) 
{ 
	m_nID = -1;
	m_sztype = 0;
	m_pParent = 0;
}

//-----------------------------------------------------------------------------
//! destructor does nothing for now.
FECoreBase::~FECoreBase(){}

//-----------------------------------------------------------------------------
//! return the super class id
SUPER_CLASS_ID FECoreBase::GetSuperClassID() { return m_sid; }

//-----------------------------------------------------------------------------
//! return a (unique) string describing the type of this class
//! This string is used in object creation
const char* FECoreBase::GetTypeStr() { return m_sztype; }

//-----------------------------------------------------------------------------
//! Set the type string (This is used by the factory methods to make sure 
//! the class has the same type string as corresponding factory class
void FECoreBase::SetTypeStr(const char* sz) { m_sztype = sz; }

//-----------------------------------------------------------------------------
//! Sets the user defined name of the component
void FECoreBase::SetName(const std::string& name)
{ 
	m_name = name;
}

//-----------------------------------------------------------------------------
//! Return the name
const std::string& FECoreBase::GetName() const
{ 
	return m_name; 
}

//-----------------------------------------------------------------------------
void FECoreBase::Serialize(DumpStream& ar)
{
	// do base class first
	FEParamContainer::Serialize(ar);

	// serialize name
	if (ar.IsShallow() == false)
	{
		if (ar.IsSaving())
		{
			ar << m_name;
			ar << m_nID;
		}
		else
		{
			ar >> m_name;
			ar >> m_nID;
		}
	}

	// serialize all the properties
	int NP = (int)m_Prop.size();
	for (int i = 0; i<NP; ++i)
	{
		FEProperty* pmat = m_Prop[i];
		pmat->SetParent(this);
		pmat->Serialize(ar);
	}
}

//-----------------------------------------------------------------------------
bool FECoreBase::Validate()
{
	// call base class first
	if (FEParamContainer::Validate() == false) return false;

	// check properties
	const int nprop = (int)m_Prop.size();
	for (int i = 0; i<nprop; ++i)
	{
		FEProperty* pi = m_Prop[i];
		if (pi)
		{
			if (pi->Validate() == false) return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool FECoreBase::Init()
{
	// check the parameter ranges
	if (Validate() == false) return false;

	// initialize properties
	const int nprop = (int)m_Prop.size();
	for (int i = 0; i<nprop; ++i)
	{
		FEProperty* pi = m_Prop[i];
		if (pi)
		{
			if (pi->Init() == false) return false;
		}
		else return fecore_error("A nullptr was set for property i");
	}
	return true;
}

//-----------------------------------------------------------------------------
void FECoreBase::AddProperty(FEProperty* pp, const char* sz, unsigned int flags)
{
	pp->SetName(sz);
	pp->m_brequired = ((flags & FEProperty::Required) != 0);
	pp->m_bvalue    = ((flags & FEProperty::ValueProperty) != 0);
	m_Prop.push_back(pp);
}

//-----------------------------------------------------------------------------
int FECoreBase::Properties()
{
	int N = (int)m_Prop.size();
	int n = 0;
	for (int i = 0; i<N; ++i) n += m_Prop[i]->size();
	return n;
}

//-----------------------------------------------------------------------------
int FECoreBase::FindPropertyIndex(const char* sz)
{
	int NP = (int)m_Prop.size();
	for (int i = 0; i<NP; ++i)
	{
		const FEProperty* pm = m_Prop[i];
		if (pm && (strcmp(pm->GetName(), sz) == 0)) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
FEProperty* FECoreBase::FindProperty(const char* sz)
{
	int NP = (int)m_Prop.size();
	for (int i = 0; i<NP; ++i)
	{
		FEProperty* pm = m_Prop[i];
		if (pm && (strcmp(pm->GetName(), sz) == 0)) return pm;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
FECoreBase* FECoreBase::GetProperty(int n)
{
	int N = (int)m_Prop.size();
	int m = 0;
	for (int i = 0; i<N; ++i)
	{
		FEProperty* pm = m_Prop[i];
		int l = pm->size();
		if (m + l > n) return pm->get(n - m);
		m += l;
	}
	return 0;
}

//-----------------------------------------------------------------------------
bool FECoreBase::SetProperty(int i, FECoreBase* pb)
{
	FEProperty* pm = m_Prop[i];
	if (pm->IsType(pb))
	{
		pm->SetProperty(pb);
		if (pb) pb->SetParent(this);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
FEParam* FECoreBase::FindParameter(const ParamString& s)
{
	// first search the parameter list
	FEParam* p = FEParamContainer::FindParameter(s);
	if (p) return p;

	// next, let's try the property list
	int NP = (int)m_Prop.size();
	for (int i = 0; i<NP; ++i)
	{
		// get the property
		FEProperty* mp = m_Prop[i];

		// see if matches
		if (s == mp->GetName())
		{
			if (mp->IsArray())
			{
				// get the number of items in this property
				int nsize = mp->size();
				int index = s.Index();
				if ((index >= 0) && (index < nsize))
				{
					return mp->get(index)->FindParameter(s.next());
				}
				else
				{
					int nid = s.ID();
					if (nid != -1)
					{
						FECoreBase* pc = mp->getFromID(nid);
						if (pc) return pc->FindParameter(s.next());
					}
					else if (s.IDString())
					{
						FECoreBase* c = mp->get(s.IDString());
						if (c) return c->FindParameter(s.next());
					}
				}
			}
			else
			{
				return mp->get(0)->FindParameter(s.next());
			}
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
FECoreBase* FECoreBase::GetProperty(const ParamString& prop)
{
	int NP = (int) m_Prop.size();
	for (int i=0; i<NP; ++i)
	{	
		FEProperty* mp = m_Prop[i];

		if (prop == mp->GetName())
		{
			if (mp->IsArray())
			{
				// get the number of items in this property
				int nsize = mp->size();
				int index = prop.Index();
				if ((index >= 0) && (index < nsize))
				{
					FECoreBase* pc = mp->get(index);
					if (pc)
					{
						ParamString next = prop.next();
						if (next.count() == 0) return pc;
						else return pc->GetProperty(next);
					}
				}
				else
				{
					int nid = prop.ID();
					if (nid != -1)
					{
						FECoreBase* pc = mp->getFromID(nid);
					}
					else if (prop.IDString())
					{
						FECoreBase* pc = mp->get(prop.IDString());
						if (pc)
						{
							ParamString next = prop.next();
							if (next.count() == 0) return pc;
							else return pc->GetProperty(next);
						}
					}
				}
			}
			else
			{
				FECoreBase* pc = mp->get(0);
				ParamString next = prop.next();
				if (next.count() == 0) return pc;
				else return pc->GetProperty(next);
			}
		}
	}

	return 0;
}
