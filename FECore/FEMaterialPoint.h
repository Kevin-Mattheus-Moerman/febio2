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

#include "mat3d.h"
#include "FEParameterList.h"
#include "FETimeInfo.h"
#include <vector>
using namespace std;

//-----------------------------------------------------------------------------
//! Material point class

//! This class implements the concept of a material point. This point carries
//! with it not only information about its location, both in the reference and  
//! current configuration but also about the local deformation. In addition
//! it contains the state information that is associated with the current
//! point.

class FECORE_API FEMaterialPoint : public FEParamContainer
{
public:
	FEMaterialPoint(FEMaterialPoint* ppt = 0);
	virtual ~FEMaterialPoint();

public:
	// sets the name of material point
	// (this string is not copied so must point to static string)
	void SetName(const char* sz) { m_szname = sz; }

	// get the name of this material point
	// (can be null if name was not set)
	const char* GetName() { return m_szname; }

public:
	//! The init function is used to intialize data
	virtual void Init();

	//! The Update function is used to update material point data
	//! Note that this gets called at the start of the time step during PreSolveUpdate
	virtual void Update(const FETimeInfo& timeInfo);

	//! copy material point data (for running restarts) \todo Is this still used?
	virtual FEMaterialPoint* Copy() = 0;

	//! get the number of material point components
	virtual int Components() { return 1; }

	//! Get the material point data
	virtual FEMaterialPoint* GetPointData(int i) { return this; }

	//! Get the next material point data
	FEMaterialPoint* Next() { return m_pNext; }

	//! Get the previous (parent) material point data
	FEMaterialPoint* Prev() { return m_pPrev; }
    
	//! Extract data (\todo Is it safe for a plugin to use this function?)
	template <class T> T* ExtractData();

	// assign the previous pointer
	void SetPrev(FEMaterialPoint* pt);

	//! assign the next pointer
	//! this also sets the prev pointer of the passed pointer
	//! in other words, it makes this the parent of the passed pointer
	void SetNext(FEMaterialPoint* pt);

	// serialization
	void Serialize(DumpStream& ar);

	// find a parameter with a given name
	virtual FEParam* FindParameter(const std::string& paramName);

protected:
	FEMaterialPoint*	m_pNext;	//<! next data in the list
	FEMaterialPoint*	m_pPrev;	//<! previous data in the list
	const char*			m_szname;	//<! optional name of material point
};

//-----------------------------------------------------------------------------
template <class T> inline T* FEMaterialPoint::ExtractData()
{
	// first see if this is the correct type
	T* p = dynamic_cast<T*>(this);
	if (p) return p;

	// check all the child classes 
	FEMaterialPoint* pt = this;
	while (pt->m_pNext)
	{
		pt = pt->m_pNext;
		p = dynamic_cast<T*>(pt);
		if (p) return p;
	}

	// search up
	pt = this;
	while (pt->m_pPrev)
	{
		pt = pt->m_pPrev;
		p = dynamic_cast<T*>(pt);
		if (p) return p;
	}

	// Everything has failed. Material point data can not be found
	return 0;
}

//-----------------------------------------------------------------------------
// Material point base class for materials that define vector properties
class FECORE_API FEMaterialPointArray : public FEMaterialPoint
{
public:
	FEMaterialPointArray(FEMaterialPoint* ppt = 0);

	//! Add a child material point
	void AddMaterialPoint(FEMaterialPoint* pt);

	//! initialization
	void Init();

	//! serialization
	void Serialize(DumpStream& ar);

	//! material point update
	void Update(const FETimeInfo& timeInfo);

	//! get the number of material point components
	int Components() { return (int)m_mp.size(); }

	//! retrieve point data
	FEMaterialPoint* GetPointData(int i) { return m_mp[i]; }

	// find a parameter with a given name
	virtual FEParam* FindParameter(const char* szname);

	// this is used to build the parameters of all the components
	virtual void BuildParamList();

protected:
	vector<FEMaterialPoint*>	m_mp;	//!< material point data for indidivual properties
};
