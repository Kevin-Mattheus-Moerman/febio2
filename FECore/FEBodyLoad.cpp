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
#include "FEBodyLoad.h"
#include "FEModel.h"

//-----------------------------------------------------------------------------
FEBodyLoad::FEBodyLoad(FEModel* pfem) : FEModelComponent(FEBODYLOAD_ID, pfem)
{
}

//-----------------------------------------------------------------------------
FEBodyLoad::~FEBodyLoad()
{
}

//-----------------------------------------------------------------------------
//! initialization
bool FEBodyLoad::Init()
{
	// If the domain list is empty, add all the domains
	if (m_dom.empty())
	{
		FEMesh& mesh = GetFEModel()->GetMesh();
		for (int i=0; i<mesh.Domains(); ++i)
		{
			FEDomain* dom = &mesh.Domain(i);
			m_dom.push_back(dom);
		}
	}
	return FEModelComponent::Init(); 
}

//-----------------------------------------------------------------------------
//! update
void FEBodyLoad::Update()
{
}

//-----------------------------------------------------------------------------
int FEBodyLoad::Domains() const
{
	return (int) m_dom.size();
}

//-----------------------------------------------------------------------------
FEDomain* FEBodyLoad::Domain(int i)
{
	return m_dom[i];
}

//-----------------------------------------------------------------------------
void FEBodyLoad::AddDomain(FEDomain* dom)
{
	m_dom.push_back(dom);
}

//-----------------------------------------------------------------------------
void FEBodyLoad::Serialize(DumpStream& ar)
{
	FEModelComponent::Serialize(ar);

	if ((ar.IsShallow() == false) && (ar.IsSaving() == false))
	{
		if (m_dom.empty())
		{
			FEMesh& mesh = GetFEModel()->GetMesh();
			for (int i = 0; i<mesh.Domains(); ++i)
			{
				FEDomain* dom = &mesh.Domain(i);
				m_dom.push_back(dom);
			}
		}
	}
}
