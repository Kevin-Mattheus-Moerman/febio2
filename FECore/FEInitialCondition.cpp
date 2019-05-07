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
#include "FEInitialCondition.h"
#include "FEModel.h"
#include "FEMesh.h"

BEGIN_PARAMETER_LIST(FEInitialBC, FEInitialCondition)
	ADD_PARAMETER(m_data, FE_PARAM_DATA_ARRAY, "value");
END_PARAMETER_LIST();

FEInitialCondition::FEInitialCondition(FEModel* pfem) : FEModelComponent(FEIC_ID, pfem)
{
}

//-----------------------------------------------------------------------------
FEInitialBC::FEInitialBC(FEModel* pfem) : FEInitialCondition(pfem), m_data(FE_DOUBLE)
{
	m_dof = -1;
}

//-----------------------------------------------------------------------------
void FEInitialBC::Serialize(DumpStream& ar)
{
	FEInitialCondition::Serialize(ar);
	if (ar.IsSaving())
	{
		ar << m_dof;
		int nsize = m_item.size();
		ar << nsize;
		for (size_t i=0; i<nsize; ++i) ar << m_item[i];
	}
	else
	{
		ar >> m_dof;
		int nsize = 0;
		ar >> nsize;
		m_item.resize(nsize);
		for (size_t i=0; i<nsize; ++i) ar >> m_item[i];
	}
}

//-----------------------------------------------------------------------------
void FEInitialBC::SetNodes(const FENodeSet& set)
{
	int N = set.size();
	m_item.resize(N);
	for (int i=0; i<N; ++i) m_item[i] = set[i];
	m_data.Create(N, 0.0);
}

//-----------------------------------------------------------------------------
void FEInitialBC::Add(int node, double value)
{
	m_item.push_back(node);
	m_data.Add(value);
}

//-----------------------------------------------------------------------------
void FEInitialBC::Activate()
{
	FEInitialCondition::Activate();
	assert(m_dof >= 0);
	if (m_dof == -1) return;
	FEModel& fem = *GetFEModel();
	FEMesh& mesh = fem.GetMesh();
	int N = m_item.size();
	for (size_t i=0; i<N; ++i)
	{
		FENode& node = mesh.Node(m_item[i]);
		node.set(m_dof, m_data.getValue(i));
	}
}

//-----------------------------------------------------------------------------
void FEInitialBCVec3D::Serialize(DumpStream& ar)
{
	FEInitialCondition::Serialize(ar);
	if (ar.IsSaving())
	{
		ar << m_dof[0] << m_dof[1] << m_dof[2];
		int nsize = m_item.size();
		ar << nsize;
		for (size_t i=0; i<nsize; ++i)
		{
			ar << m_item[i].nid << m_item[i].v0;
		}
	}
	else
	{
		ar >> m_dof[0] >> m_dof[1] >> m_dof[2];
		int nsize = 0;
		ar >> nsize;
		m_item.resize(nsize);
		for (size_t i=0; i<nsize; ++i)
		{
			ar >> m_item[i].nid >> m_item[i].v0;
		}
	}
}

//-----------------------------------------------------------------------------
void FEInitialBCVec3D::Activate()
{
	assert((m_dof[0]>=0)&&(m_dof[1]>=0)&&(m_dof[2]>=0));
	FEInitialCondition::Activate();
	FEModel& fem = *GetFEModel();
	FEMesh& mesh = fem.GetMesh();
	for (size_t i=0; i<m_item.size(); ++i)
	{
		FENode& node = mesh.Node(m_item[i].nid);
		node.set_vec3d(m_dof[0], m_dof[1], m_dof[2], m_item[i].v0);
	}
}
