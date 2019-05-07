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
#include "ObjectDataRecord.h"
#include "FECoreKernel.h"
#include "FEModel.h"
#include "FERigidSystem.h"
#include "FERigidBody.h"
#include "FEMaterial.h"

//-----------------------------------------------------------------------------
void ObjectDataRecord::Parse(const char* szexpr)
{
	char szcopy[MAX_STRING] = {0};
	strcpy(szcopy, szexpr);
	char* sz = szcopy, *ch;
	m_Data.clear();
	strcpy(m_szdata, szexpr);
	do
	{
		ch = strchr(sz, ';');
		if (ch) *ch++ = 0;
		FELogObjectData* pdata = fecore_new<FELogObjectData>(FEOBJLOGDATA_ID, sz, m_pfem);
		if (pdata) m_Data.push_back(pdata);
		else throw UnknownDataField(sz);
		sz = ch;
	}
	while (ch);
}

//-----------------------------------------------------------------------------
double ObjectDataRecord::Evaluate(int item, int ndata)
{
	FEMesh& mesh = m_pfem->GetMesh();
	int nrb = item - 1;
	if ((nrb < 0) || (nrb >= m_pfem->Materials())) return 0;

	double val = 0;
	FEMaterial* pm = m_pfem->GetMaterial(nrb);
	assert(pm);
	if (pm == 0) return 0;

	// find the rigid body that has this material
	FERigidSystem& rs = *m_pfem->GetRigidSystem();
	int NRB = rs.Objects();
	for (int i=0; i<NRB; ++i)
	{
		FERigidBody& obj = *rs.Object(i);
		if (obj.GetMaterialID() == nrb) return m_Data[ndata]->value(obj);
	}

	return val;
}

//-----------------------------------------------------------------------------
void ObjectDataRecord::SelectAllItems()
{
	int n = 0, i;
	for (i=0; i<m_pfem->Materials(); ++i)
	{
		FEMaterial* pm = m_pfem->GetMaterial(i);
		if (pm->IsRigid()) ++n;
	}

	if (n > 0)
	{
		m_item.resize(n);
		n = 0;
		for (i=0; i<m_pfem->Materials(); ++i)
		{
			FEMaterial* pm  = m_pfem->GetMaterial(i);
			if (pm->IsRigid())
			{
				m_item[n++] = i+1;
			}
		}
	}
}
