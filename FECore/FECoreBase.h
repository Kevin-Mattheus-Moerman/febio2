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
#include "FEParameterList.h"
#include "FE_enum.h"
#include "FEProperty.h"
#include <string>

//-----------------------------------------------------------------------------
class FECoreFactory;
class FEModel;

//-----------------------------------------------------------------------------
//! Base class for most classes in FECore library and the base class for all 
//! classes that can be registered with the framework.
class FECORE_API FECoreBase : public FEParamContainer
{
public:
	//! constructor
	FECoreBase(SUPER_CLASS_ID sid);

	//! destructor
	virtual ~FECoreBase();

	//! set class name
	void SetName(const std::string& name);

	//! get the class' name
	const std::string& GetName() const;

	//! data serialization
	void Serialize(DumpStream& ar);

	//! Initialization
	virtual bool Init();

	//! validates all properties and parameters
	bool Validate();

public:
	//! return the super class id
	SUPER_CLASS_ID GetSuperClassID();

	//! return a (unique) string describing the type of this class
	//! This string is used in object creation
	const char* GetTypeStr();

public: // interface for managing attributes

	//! Set the attribute
	virtual bool SetAttribute(const char* szname, const char* szval) { return true; }

public:
	//! return a parameter
	FEParam* FindParameter(const ParamString& s);

public: // interface for getting/setting properties

	//! get the number of properties
	int Properties();

	//! Set a property
	bool SetProperty(int nid, FECoreBase* pm);

	//! return a property
	virtual FECoreBase* GetProperty(int i);

	//! find a property index ( returns <0 for error)
	int FindPropertyIndex(const char* szname);

	//! return a property (class)
	FEProperty* FindProperty(const char* sz);

	//! return a property from a paramstring
	FECoreBase* GetProperty(const ParamString& prop);

	//! return the number of properties defined
	int PropertyClasses() { return (int)m_Prop.size(); }

	//! return a property
	FEProperty* PropertyClass(int i) { return m_Prop[i]; }

public:
	//! Get the parent of this object (zero if none)
	FECoreBase* GetParent() { return m_pParent; }

	//! Get the ancestor of this class (this if none)
	FECoreBase* GetAncestor() { FECoreBase* mp = GetParent(); return (mp ? mp->GetAncestor() : this); }

	//! Set the parent of this class
	void SetParent(FECoreBase* parent) { m_pParent = parent; }

	//! return the component ID
	int GetID() const { return m_nID; }

	//! set the component ID
	void SetID(int nid) { m_nID = nid; }

protected:
	//! Add a property
	//! Call this in the constructor of derived classes to 
	//! build the property list
	void AddProperty(FEProperty* pp, const char* sz, unsigned int flags = FEProperty::Required);

private:
	//! Set the type string (This is used by the factory methods to make sure 
	//! the class has the same type string as corresponding factory class
	void SetTypeStr(const char* sz);

private:
	std::string		m_name;			//!< user defined name of component
	FECoreBase*		m_pParent;		//!< pointer to "parent" object (if any) (NOTE: only used by materials)
	SUPER_CLASS_ID	m_sid;			//!< The super-class ID
	const char*		m_sztype;		//!< the type string

	vector<FEProperty*>	m_Prop;		//!< list of properties

private:
	int		m_nID;			//!< component ID

	friend class FECoreFactory;
};
