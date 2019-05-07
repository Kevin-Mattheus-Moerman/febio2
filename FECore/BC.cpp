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
#include "BC.h"
#include "FEModel.h"
#include "FESolver.h"
#include "DOFS.h"
#include "FECoreKernel.h"

//-----------------------------------------------------------------------------
BEGIN_PARAMETER_LIST(FENodalLoad, FEBoundaryCondition)
	ADD_PARAMETER(m_scale, FE_PARAM_DOUBLE    , "scale");
	ADD_PARAMETER(m_data , FE_PARAM_DATA_ARRAY, "value");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FENodalLoad::FENodalLoad(FEModel* pfem) : FEBoundaryCondition(FEBC_ID, pfem), m_data(FE_DOUBLE)
{
	m_scale = 1.0;
	m_dof = -1;
}

//-----------------------------------------------------------------------------
void FENodalLoad::Serialize(DumpStream& ar)
{
	if (ar.IsShallow()) return;

	FEBoundaryCondition::Serialize(ar);
	if (ar.IsSaving())
	{
		ar << m_dof << m_item;
	}
	else
	{
		ar >> m_dof >> m_item;
	}
}

//-----------------------------------------------------------------------------
bool FENodalLoad::Init()
{
	return true;
}

//-----------------------------------------------------------------------------
void FENodalLoad::AddNode(int nid, double scale)
{
	m_item.push_back(nid);
	m_data.Add(scale);
}

//-----------------------------------------------------------------------------
void FENodalLoad::AddNodes(const FENodeSet& ns, double scale)
{
	int N = ns.size();
	for (int i=0; i<N; ++i) AddNode(ns[i], scale);
}

//-----------------------------------------------------------------------------
void FENodalLoad::SetLoad(double s, int lc)
{
	m_scale = s;
	if (lc >= 0)
	{
		FEParam& p = *FEParamContainer::FindParameterFromData((void*)(&m_scale));
		p.SetLoadCurve(lc, m_scale);
	}
}

//-----------------------------------------------------------------------------
//! Return the current value of the nodal load
double FENodalLoad::NodeValue(int n) const
{
	return m_scale*m_data.getValue(n);
}

//-----------------------------------------------------------------------------
FEFixedBC::FEFixedBC(FEModel* pfem) : FEBoundaryCondition(FEBC_ID, pfem)
{ 
	m_dof = -1; 
}

//-----------------------------------------------------------------------------
FEFixedBC::FEFixedBC(FEModel* pfem, int node, int dof) : FEBoundaryCondition(FEBC_ID, pfem)
{ 
	m_node.push_back(node); 
	m_dof = dof; 
}

//-----------------------------------------------------------------------------
void FEFixedBC::AddNode(int node)
{
	m_node.push_back(node);
}

//-----------------------------------------------------------------------------
void FEFixedBC::AddNodes(const FENodeSet& ns)
{
	int N = ns.size();
	for (int i=0; i<N; ++i) AddNode(ns[i]);
}

//-----------------------------------------------------------------------------
void FEFixedBC::SetDOF(int dof)
{
	m_dof = dof;
}

//-----------------------------------------------------------------------------
void FEFixedBC::Serialize(DumpStream& ar)
{
	if (ar.IsShallow()) return;

	FEBoundaryCondition::Serialize(ar);
	if (ar.IsSaving())
	{
		ar << m_node << m_dof;
	}
	else
	{
		ar >> m_node >> m_dof;
	}
}

//-----------------------------------------------------------------------------
void FEFixedBC::Activate()
{
	FEBoundaryCondition::Activate();
	if (m_dof >= 0)
	{
		FEMesh& mesh = GetFEModel()->GetMesh();
		int n = (int) m_node.size();
		for (int i=0; i<n; ++i)
		{
			// make sure we only activate open dof's
			vector<int>& BC = mesh.Node(m_node[i]).m_BC;
			if (BC[m_dof] == DOF_OPEN) BC[m_dof] = DOF_FIXED;
		}
	}
}

//-----------------------------------------------------------------------------
void FEFixedBC::Deactivate()
{
	FEBoundaryCondition::Deactivate();
	if (m_dof >= 0)
	{
		FEMesh& mesh = GetFEModel()->GetMesh();
		int n = (int) m_node.size();
		for (int i=0; i<n; ++i)
		{
			vector<int>& BC = mesh.Node(m_node[i]).m_BC;
			BC[m_dof] = DOF_OPEN;
		}
	}
}

//=============================================================================
FEPrescribedBC::FEPrescribedBC(FEModel* pfem) : FEBoundaryCondition(FEBC_ID, pfem)
{
}


//=============================================================================
BEGIN_PARAMETER_LIST(FEPrescribedDOF, FEPrescribedBC)
	ADD_PARAMETER(m_scale, FE_PARAM_DOUBLE, "scale");
	ADD_PARAMETER(m_br   , FE_PARAM_BOOL  , "relative");
	ADD_PARAMETER(m_data , FE_PARAM_DATA_ARRAY, "value");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FEPrescribedDOF::FEPrescribedDOF(FEModel* pfem) : FEPrescribedBC(pfem), m_data(FE_DOUBLE)
{
	m_scale = 0.0;
	m_dof = -1;
	m_br = false;
}

//-----------------------------------------------------------------------------
FEPrescribedDOF::FEPrescribedDOF(FEModel* pfem, const FEPrescribedDOF& bc) : FEPrescribedBC(pfem), m_data(FE_DOUBLE)
{
	m_scale = bc.m_scale;
	m_dof   = bc.m_dof;
	m_br    = bc.m_br;
	m_item  = bc.m_item;
	m_data  = bc.m_data;
	CopyParameterListState(bc.GetParameterList());
}

//-----------------------------------------------------------------------------
// Sets the displacement scale factor. An optional load curve index can be given
// of the load curve that will control the scale factor.
FEPrescribedDOF& FEPrescribedDOF::SetScale(double s, int lc)
{
	m_scale = s;
	if (lc >= 0)
	{
		FEParam& p = *FEParamContainer::FindParameterFromData((void*)(&m_scale));
		p.SetLoadCurve(lc, m_scale);
	}
	return *this; 
}

//-----------------------------------------------------------------------------
void FEPrescribedDOF::AddNode(int nid, double s)
{
	ITEM item = {nid, s};
	m_item.push_back(item);
	m_data.Add(s);
}

//-----------------------------------------------------------------------------
void FEPrescribedDOF::AddNodes(const FENodeSet& nset, double s)
{
	int N = nset.size();
	for (int i=0; i<N; ++i) AddNode(nset[i], s);
}

//-----------------------------------------------------------------------------
bool FEPrescribedDOF::Init()
{
	// don't forget to call the base class
	if (FEPrescribedBC::Init() == false) return false;

	// make sure this is not a rigid node
	FEModel& fem = *GetFEModel();
	FEMesh& mesh = fem.GetMesh();
	int NN = mesh.Nodes();
	for (size_t i=0; i<m_item.size(); ++i)
	{
		int nid = m_item[i].nid;
		if ((nid < 0) || (nid >= NN)) return false;
		if (mesh.Node(nid).m_rid != -1)
		{
			return fecore_error("Rigid nodes cannot be prescribed.");
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
void FEPrescribedDOF::Activate()
{
	FEPrescribedBC::Activate();

	FEModel& fem = *GetFEModel();
	FEMesh& mesh = fem.GetMesh();

	for (size_t j = 0; j<m_item.size(); ++j)
	{
		// get the node
		FENode& node = mesh.Node(m_item[j].nid);

		// set the dof to prescribed
		node.m_BC[m_dof] = DOF_PRESCRIBED;

		// evaluate the relative offset
		if (m_br)
		{
			assert(m_dof < (int)node.m_val.size());
			double r = node.get(m_dof);
			m_item[j].ref = r;
		}
	}
}

//-----------------------------------------------------------------------------
void FEPrescribedDOF::Deactivate()
{
	FEPrescribedBC::Deactivate();

	FEModel& fem = *GetFEModel();
	FEMesh& mesh = fem.GetMesh();

	for (size_t j = 0; j<m_item.size(); ++j)
	{
		FENode& node = mesh.Node(m_item[j].nid);
		node.m_BC[m_dof] = DOF_OPEN;
	}
}

//-----------------------------------------------------------------------------
double FEPrescribedDOF::NodeValue(int n) const
{
	const ITEM& it = m_item[n];
	double val = m_scale*m_data.getValue(n);
	if (m_br) val += it.ref;
	return val;
}

//-----------------------------------------------------------------------------
void FEPrescribedDOF::Serialize(DumpStream& ar)
{
	if (ar.IsShallow()) return;

	FEPrescribedBC::Serialize(ar);
	if (ar.IsSaving())
	{
		ar << m_dof << m_item;
	}
	else
	{
		ar >> m_dof >> m_item;
	}
}

//-----------------------------------------------------------------------------
void FEPrescribedDOF::CopyFrom(FEPrescribedBC* pbc)
{
	FEPrescribedDOF* ps = dynamic_cast<FEPrescribedDOF*>(pbc); assert(ps);
	m_dof   = ps->m_dof;
	m_scale = ps->m_scale;
	m_br    = ps->m_br;
	m_data  = ps->m_data;
	m_item  = ps->m_item;
	CopyParameterListState(ps->GetParameterList());
}

//-----------------------------------------------------------------------------
//! Update the values of the prescribed degrees of freedom.
//! This is called during model update (FESolver::Update)
void FEPrescribedDOF::Update()
{
	// get the mesh
	FEMesh& mesh = GetFEModel()->GetMesh();

	// update the current nodal values
	for (size_t i = 0; i<m_item.size(); ++i)
	{
		FENode& node = mesh.Node(m_item[i].nid);
		double g = NodeValue((int)i);
		node.set(m_dof, g);
	}
}

//-----------------------------------------------------------------------------
void FEPrescribedDOF::PrepStep(std::vector<double>& ui, bool brel)
{
	// get the mesh
	FEMesh& mesh = GetFEModel()->GetMesh();

	for (size_t i = 0; i<m_item.size(); ++i)
	{
		FENode& node = mesh.Node(m_item[i].nid);
		double dq = NodeValue((int)i);
		int I = -node.m_ID[m_dof] - 2;
		if (I >= 0) ui[I] = (brel ? dq - node.get(m_dof) : dq);
	}
}
