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


#pragma once
#include "FEBoundaryCondition.h"
#include "FEGlobalVector.h"
#include "FENodeDataMap.h"

class FESolver;
class FENodeSet;
class FESurface;

//-----------------------------------------------------------------------------
//! Nodal load boundary condition
class FECORE_API FENodalLoad : public FEBoundaryCondition
{
public:
	//! constructor
	FENodalLoad(FEModel* pfem);

	//! initialization
	bool Init() override;

	//! serialiation
	void Serialize(DumpStream& ar) override;

	//! Add a node to the node set
	void AddNode(int nid, double scale = 1.0);

	//! add a node set
	void AddNodes(const FENodeSet& ns, double scale = 1.0);

	//! number of nodes
	int Nodes() const { return (int) m_item.size(); }

	//! Node ID
	int NodeID(int n) const { return m_item[n]; }

	//! get nodal value
	double NodeValue(int n) const;

	//! get/set load 
	void SetLoad(double s, int lc = -1);
	double GetLoad() const { return m_scale; }

	//! get/set degree of freedom
	void SetDOF(int ndof) { m_dof = ndof; }
	int GetDOF() const { return m_dof; }

private:
	int		m_dof;		// degree of freedom index

	double			m_scale;	// applied load scale factor
	vector<int>		m_item;		// item list
	FENodeDataMap	m_data;		// nodal data

	DECLARE_PARAMETER_LIST();
};

//-----------------------------------------------------------------------------
//! This class represents a fixed degree of freedom
//! This boundary conditions sets the BC attribute of the nodes in the nodeset
//! to DOF_FIXED when activated.
class FECORE_API FEFixedBC : public FEBoundaryCondition
{
public:
	//! constructors
	FEFixedBC(FEModel* pfem);
	FEFixedBC(FEModel* pfem, int node, int dof);

	//! add a node to the node set
	void AddNode(int node);

	//! add a node set
	void AddNodes(const FENodeSet& ns);

	//! set the degree of freedom that will be fixed
	void SetDOF(int dof);

public:
	//! serialization
	void Serialize(DumpStream& ar);

	//! activation
	void Activate();

	//! deactivations
	void Deactivate();

public:
	vector<int>		m_node;		//!< node set
	int				m_dof;		//!< fixed degree of freedom
};

//-----------------------------------------------------------------------------
// base class for prescribed boundary conditions
class FECORE_API FEPrescribedBC : public FEBoundaryCondition
{
public:
	FEPrescribedBC(FEModel* pfem);

public:
	// implement these functions

	// assign a node set to the prescribed BC
	virtual void AddNodes(const FENodeSet& set) {};

	// assign a surface to the BC
	virtual void AddNodes(const FESurface& surf) {}

	// This function is called when the solver needs to know the 
	// prescribed dof values. The brel flag indicates wheter the total 
	// value is needed or the value with respect to the current nodal dof value
	virtual void PrepStep(std::vector<double>& ui, bool brel = true) = 0;

	// This is called during nodal update and should be used to enforce the 
	// nodal degrees of freedoms
	virtual void Update() = 0;

	// copy data from another class
	virtual void CopyFrom(FEPrescribedBC* pbc) = 0;

	// Also implement the following functions.
	// These are already declared in base classes.
//  bool Init();
//  void Activate();
//  void Deactivate();
//  void Serialize(DumpStream& ar);
};

//-----------------------------------------------------------------------------
//! prescribed boundary condition data
//! \todo Should I make a derived class for the relative prescribed BC's?
class FECORE_API FEPrescribedDOF : public FEPrescribedBC
{
	struct ITEM
	{
		int		nid;	// nodal ID
		double	ref;	// reference value (for relative BC's)
	};

public:
	FEPrescribedDOF(FEModel* pfem);
	FEPrescribedDOF(FEModel* pfem, const FEPrescribedDOF& bc);

	void AddNode(int node, double scale = 1.0);
	void AddNodes(const FENodeSet& s, double scale);
	void AddNodes(const FENodeSet& s) override { AddNodes(s, 1.0); }

	int NodeID(int i) { return m_item[i].nid; }

	size_t Items() const { return m_item.size(); }

public:
	void Serialize(DumpStream& ar) override;

	void Activate() override;

	void Deactivate() override;

	bool Init() override;

	void Update() override;

	void PrepStep(std::vector<double>& ui, bool brel = true) override;

	void CopyFrom(FEPrescribedBC* pbc) override;

public:
	FEPrescribedDOF& SetScale(double s, int lc = -1);
	FEPrescribedDOF& SetDOF(int dof) { m_dof = dof; return *this; }
	FEPrescribedDOF& SetRelativeFlag(bool br) { m_br = br; return *this; }

	void SetNodeScale(int n, double s) { m_data.setValue(n, s); }

	double GetScaleFactor() const { return m_scale; }
	int GetDOF() const { return m_dof; }

	double NodeValue(int n) const;

private:
	int			m_dof;		//!< dof
	double		m_scale;	//!< overall scale factor
	bool		m_br;		//!< flag for relative bc
	FENodeDataMap	m_data;		//!< nodal displacement values

	vector<ITEM>	m_item;		//!< item list

	DECLARE_PARAMETER_LIST();
};
