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
#include "FEDomain.h"
#include "FEMaterial.h"
#include "FEDataExport.h"
#include "FEMesh.h"
#include "DOFS.h"
#include "FEGlobalMatrix.h"
#include <string.h>

//-----------------------------------------------------------------------------
FEDomain::FEDomain(int nclass, FEMesh* pm) : FECoreBase(FEDOMAIN_ID), m_pMesh(pm), m_nclass(nclass)
{
	m_bactive = true;
}

//-----------------------------------------------------------------------------
FEDomain::~FEDomain()
{
	// delete all data export classes
	if (m_Data.empty() == false)
	{
		size_t ND = m_Data.size();
		for (size_t i=0; i<ND; ++i) delete m_Data[i];
		m_Data.clear();
	}
}

//-----------------------------------------------------------------------------
void FEDomain::AddDataExport(FEDataExport* pd)
{
	if (pd) m_Data.push_back(pd);
}

//-----------------------------------------------------------------------------
FEElement* FEDomain::FindElementFromID(int nid)
{
	for (int i=0; i<Elements(); ++i)
	{
		FEElement& el = ElementRef(i);
		if (el.GetID() == nid) return &el;
	}

	return 0;
}

//-----------------------------------------------------------------------------
void FEDomain::Serialize(DumpStream& ar)
{
	if (ar.IsShallow() == false)
	{
		if (ar.IsSaving())
		{
			ar << m_Node;
		}
		else
		{
			ar >> m_Node;
		}
	}

	if (ar.IsShallow())
	{
		int NEL = Elements();
		for (int i = 0; i<NEL; ++i)
		{
			FEElement& el = ElementRef(i);
			el.Serialize(ar);
			int nint = el.GaussPoints();
			for (int j = 0; j<nint; ++j) el.GetMaterialPoint(j)->Serialize(ar);
		}
	}
	else
	{
		int NEL = Elements();
		if (ar.IsSaving())
		{
			for (size_t i = 0; i<NEL; ++i)
			{
				FEElement& el = ElementRef(i);
				el.Serialize(ar);
				for (int j = 0; j<el.GaussPoints(); ++j) el.GetMaterialPoint(j)->Serialize(ar);
			}
		}
		else
		{
			FEMaterial* pmat = GetMaterial();
			assert(pmat);

			for (size_t i = 0; i<NEL; ++i)
			{
				FEElement& el = ElementRef(i);
				el.Serialize(ar);
				for (int j = 0; j<el.GaussPoints(); ++j)
				{
					el.SetMaterialPointData(pmat->CreateMaterialPointData(), j);
					el.GetMaterialPoint(j)->Serialize(ar);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// This routine allocates the material point data for the element's integration points.
// Currently, this has to be called after the elements have been assigned a type (since this
// determines how many integration point an element gets). 
void FEDomain::CreateMaterialPointData()
{
	FEMaterial* pmat = GetMaterial();
	if (pmat != 0)
	{
		for (int i=0; i<Elements(); ++i)
		{
			FEElement& el = ElementRef(i);
			for (int k=0; k<el.GaussPoints(); ++k) el.SetMaterialPointData(pmat->CreateMaterialPointData(), k);
		}
	}
}

//-----------------------------------------------------------------------------
void FEDomain::SetMatID(int mid)
{
	for (int i=0; i<Elements(); ++i) ElementRef(i).SetMatID(mid);
}

//-----------------------------------------------------------------------------
void FEDomain::SetDOFList(vector<int>& dof)
{
	m_dof = dof;
}

//-----------------------------------------------------------------------------
// This is the default packing method. 
// It stores all the degrees of freedom for the first node in the order defined
// by the DOF array, then for the second node, and so on. 
void FEDomain::UnpackLM(FEElement& el, vector<int>& lm)
{
	int N = el.Nodes();
	int ndofs = (int)m_dof.size();
	lm.resize(N*ndofs);
	for (int i=0; i<N; ++i)
	{
		int n = el.m_node[i];
		FENode& node = m_pMesh->Node(n);
		vector<int>& id = node.m_ID;
		for (int j=0; j<ndofs; ++j) lm[i*ndofs + j] = id[m_dof[j]];
	}
}

//-----------------------------------------------------------------------------
void FEDomain::BuildMatrixProfile(FEGlobalMatrix& M)
{
	vector<int> elm;
	const int NE = Elements();
	for (int j=0; j<NE; ++j)
	{
		FEElement& el = ElementRef(j);
		UnpackLM(el, elm);
		M.build_add(elm);
	}
}

//-----------------------------------------------------------------------------
void FEDomain::Activate()
{
	// get the number of degrees of freedom for this domain.
	const int ndofs = (int)m_dof.size();

	// activate all the degrees of freedom of this domain
	for (int i=0; i<Nodes(); ++i)
	{
		FENode& node = Node(i);
		if (node.HasFlags(FENode::EXCLUDE) == false)
		{
			for (int j=0; j<ndofs; ++j) node.m_ID[m_dof[j]] = DOF_ACTIVE;
		}
	}
}

//-----------------------------------------------------------------------------
//! return a specific node
FENode& FEDomain::Node(int i)
{ 
	return m_pMesh->Node(m_Node[i]); 
}

//-----------------------------------------------------------------------------
//! return a specific node
const FENode& FEDomain::Node(int i) const
{ 
	return m_pMesh->Node(m_Node[i]); 
}

//-----------------------------------------------------------------------------
void FEDomain::CopyFrom(FEDomain* pd)
{
	m_Node = pd->m_Node;
	m_dof = pd->m_dof;
	SetName(pd->GetName());
}

//-----------------------------------------------------------------------------
bool FEDomain::Init()
{
	// base class first
	if (FECoreBase::Init() == false) return false;

	// make sure that there are elements in this domain
	if (Elements() == 0) return false;

	// get the mesh to which this domain belongs
	FEMesh& mesh = *GetMesh();

	// This array is used to keep tags on each node
	int NN = mesh.Nodes();
	vector<int> tag; tag.assign(NN, -1);

	// let's find all nodes the domain needs
	int nn = 0;
	int NE = Elements();
	for (int i=0; i<NE; ++i)
	{
		FEElement& el = ElementRef(i);
		int ne = el.Nodes();
		for (int j=0; j<ne; ++j)
		{
			// get the global node number
			int m = el.m_node[j];
		
			// create a local node number
			if (tag[m] == -1) tag[m] = nn++;

			// set the local node number
			el.m_lnode[j] = tag[m];
		}
	}

	// allocate node index table
	m_Node.assign(nn, -1);

	// fill the node index table
	for (int i=0; i<NN; ++i)
	{
		if (tag[i] >= 0)
		{
			m_Node[tag[i]] = i;
		}
	}

#ifdef _DEBUG
	// make sure all nodes are assigned a local index
	for (int i=0; i<nn; ++i)
	{
		assert(m_Node[i] >= 0);
	}
#endif

	return true;
}

void FEDomain::Reset()
{
	// Init material point data
	InitMaterialPointData();
}

void FEDomain::InitMaterialPointData()
{
	// reset integration point data
	for (int i = 0; i<Elements(); ++i)
	{
		FEElement& el = ElementRef(i);
		int nint = el.GaussPoints();
		for (int j = 0; j<nint; ++j)
		{
			FEMaterialPoint* pt = el.GetMaterialPoint(j);
			if (pt) pt->Init();
		}
	}

	// Re-initialize the local coordinate systems
	FEMaterial* mat = GetMaterial();
	if (mat)
	{
		// assign local coordinate system to each integration point
		for (size_t i = 0; i<Elements(); ++i)
		{
			FEElement& el = ElementRef(i);
			for (int n = 0; n<el.GaussPoints(); ++n) mat->SetLocalCoordinateSystem(el, n, *(el.GetMaterialPoint(n)));
		}
	}
}
