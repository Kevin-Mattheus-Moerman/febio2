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

#include "FESurface.h"
#include "FECoreBase.h"
#include "Archive.h"
#include "FE_enum.h"
#include "FEDataStream.h"

//-----------------------------------------------------------------------------
// Region types
enum Region_Type {
	FE_REGION_NODE,
	FE_REGION_DOMAIN,
	FE_REGION_SURFACE
};

//-----------------------------------------------------------------------------
// forward declarations
class FEModel;
class FEMesh;

//-----------------------------------------------------------------------------
//! This is the base class for all classes that wish to store data to the 
//! plot file. However, classes will not use this class directly as their
//! base class. Instead they'll use one of the more specialized classes
//! defined below.
//!
class FECORE_API FEPlotData : public FECoreBase
{
public:
	FEPlotData(Region_Type R, Var_Type t, Storage_Fmt s);

	// Derived classes must implement this function
	virtual void Save(FEModel& fem, Archive& ar) = 0;

	// The filter can be used to pass additional information to the plot field.
	// The interpretation of this filter is up to the derived class, but could
	// be used e.g. for disambiguation. There are currently two flavors of this
	// function: one that takes a const char* and one that takes an int. Derived
	// classes can overload both or just the one that makes sense for that plot field.
	// Note that these functions return false by default. This implies that trying
	// to specify a filter on a variable that doesn't support it will automatically cause 
	// an error.
	virtual bool SetFilter(const char* sz) { return false; }
	virtual bool SetFilter(int n) { return false;}

	Region_Type RegionType() { return m_nregion; }
	Var_Type DataType() { return m_ntype; }
	Storage_Fmt StorageFormat() { return m_sfmt; }

	int VarSize(Var_Type t);

	void SetItemList(vector<int>& item) { m_item = item; }
    
    void SetDomainName(const char* szdom);

	FEModel* GetFEModel() { return m_pfem; }

public: // used by array variables

	void SetArraySize(int n);
	int GetArraysize() const;

	void SetArrayNames(vector<string>& s) { m_arrayNames = s; }
	vector<string>& GetArrayNames() { return m_arrayNames; }

protected:
	Region_Type		m_nregion;		//!< region type
	Var_Type		m_ntype;		//!< data type
	Storage_Fmt		m_sfmt;			//!< data storage format
	vector<int>		m_item;			//!< Data will only be stored for the item's in this list
    char			m_szdom[64];	//!< Data will only be stored for the domain with this name
	FEModel*		m_pfem;

	int				m_arraySize;	//!< size of arrays (used by arrays)
	vector<string>	m_arrayNames;	//!< optional names of array components (used by arrays)
};

//-----------------------------------------------------------------------------
//! This is the base class for node data. Classes that wish to store data
//! associated with each node of the mesh, will use this base class.
class FECORE_API FENodeData : public FEPlotData
{
public:
	FENodeData(Var_Type t, Storage_Fmt s) : FEPlotData(FE_REGION_NODE, t, s) {}
	void Save(FEModel& fem, Archive& ar);
	virtual bool Save(FEMesh& m, FEDataStream& a) = 0;
};

//-----------------------------------------------------------------------------
//! This is the base class for domain data. Classes that wish to store data
//! associated with each element or node of a domain, will use this base class.
class FECORE_API FEDomainData : public FEPlotData
{
public:
	FEDomainData(Var_Type t, Storage_Fmt s) : FEPlotData(FE_REGION_DOMAIN, t, s) {}
	void Save(FEModel& fem, Archive& ar);
	virtual bool Save(FEDomain& D, FEDataStream& a) = 0;
};

//-----------------------------------------------------------------------------
//! This is the base class for surface data. Classes that wish to store data
//! associated with each node or facet of a surface, will use this base class.
class FECORE_API FESurfaceData : public FEPlotData
{
public:
	FESurfaceData(Var_Type t, Storage_Fmt s) : FEPlotData(FE_REGION_SURFACE, t, s) {}
	void Save(FEModel& fem, Archive& ar);
	virtual bool Save(FESurface& S, FEDataStream& a) = 0;
};
