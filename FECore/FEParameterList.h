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
#include "vec2d.h"
#include "vec3d.h"
#include "mat3d.h"
#include <assert.h>
#include <list>
#include <memory>
#include "FEParam.h"
#include "FEParamValidator.h"
#include "ParamString.h"
#include "fecore_api.h"
using namespace std;

//-----------------------------------------------------------------------------
class DumpStream;
class FEParamContainer;

//-----------------------------------------------------------------------------
typedef list<FEParam>::iterator FEParamIterator;
typedef list<FEParam>::const_iterator FEParamIteratorConst;

//-----------------------------------------------------------------------------
//! A list of material parameters
class FECORE_API FEParameterList
{
public:
	FEParameterList(FEParamContainer* pc) : m_pc(pc) {}
	virtual ~FEParameterList(){}

	//! assignment operator
	void operator = (FEParameterList& l);

	//! Add a parameter to the list
	void AddParameter(void* pv, FEParamType itype, int ndim, const char* sz);

	//! Add a parameter to the list
	void AddParameter(void* pv, FEParamType type, int ndim, FEParamRange rng, double fmin, double fmax, const char* sz);

	//! find a parameter using the data pointer
	FEParam* FindFromData(void* pv);

	//! find a parameter using it's name (the safe way)
	FEParam* FindFromName(const char* sz);

	//! get a parameter (the dangerous way)
	FEParam& operator [] (const char* sz) { return *FindFromName(sz); }

	//! returs the first parameter
	FEParamIterator first() { return m_pl.begin(); }

	//! returs the first parameter
	FEParamIteratorConst first() const { return m_pl.begin(); }

	//! number of parameters
	int Parameters() const { return (int) m_pl.size(); }

	//! return the parameter container
	FEParamContainer* GetContainer() { return m_pc; }

protected:
	FEParamContainer*	m_pc;	//!< parent container
	list<FEParam>		m_pl;	//!< the actual parameter list
};

//-----------------------------------------------------------------------------
//! Base class for classes that wish to support parameter lists
class FECORE_API FEParamContainer
{
public:
	//! constructor
	FEParamContainer();

	//! destructor
	virtual ~FEParamContainer();

	//! return the material's parameter list
	FEParameterList& GetParameterList();
	const FEParameterList& GetParameterList() const;

	//! find a parameter using it's name
	virtual FEParam* FindParameter(const ParamString& s);

	//! find a parameter using a pointer to the variable
	virtual FEParam* FindParameterFromData(void* pv);

	//! serialize parameter data
	virtual void Serialize(DumpStream& ar);

	//! validate material parameters.
	//! This function returns false on the first parameter encountered
	//! that is not valid (i.e. is outside its defined range). 
	//! Overload this function to do additional validation. Make sure to always call the base class.
	//! Use fecore_get_error_string() to find out which parameter failed validation.
	virtual bool Validate();

public:
	//! This function is called after the parameter was read in from the input file.
	//! It can be used to do additional processing when a parameter is read in.
	virtual void SetParameter(FEParam& p) {}

	//! If a parameter has attributes, this function will be called
	virtual bool SetParameterAttribute(FEParam& p, const char* szatt, const char* szval) { return false; }

	//! This copies the state of a parameter list (i.e. assigned load curve IDs)
	//! This function assumes that there is a one-to-one correspondence between
	//! source and target parameter lists.
	void CopyParameterListState(const FEParameterList& pl);

public:
	//! This function will be overridden by each class that defines a parameter list
	virtual void BuildParamList() {}

	//! Add a parameter to the list
	void AddParameter(void* pv, FEParamType itype, int ndim, const char* sz);

	//! Add a parameter to the list
	void AddParameter(void* pv, FEParamType type, int ndim, RANGE rng, const char* sz);

private:
	FEParameterList*	m_pParam;	//!< parameter list
};

//-----------------------------------------------------------------------------
// To add parameter list to a material, simply do the following two steps
// 1) add the DECLARE_PARAMETER_LIST macro in the material class declaration
// 2) use the BEGIN_PARAMETER_LIST, ADD_PARAM and END_PARAMETER_LIST to
//    define a parameter list

// the following macro declares the parameter list for a material
#define DECLARE_PARAMETER_LIST() \
protected: \
	void BuildParamList() override;

// the BEGIN_PARAMETER_LIST defines the beginning of a parameter list
#define BEGIN_PARAMETER_LIST(theClass, baseClass) \
	void theClass::BuildParamList() { \
			baseClass::BuildParamList(); \

// the ADD_PARAMETER macro adds a parameter to the parameter list
#define ADD_PARAMETER(theParam, theType, theName) \
	AddParameter(&theParam, theType, 1, theName);

// the ADD_PARAMETERV macro adds a parameter to the paramter list
// that is an array
#define ADD_PARAMETERV(theParam, theType, theDim, theName) \
	AddParameter(theParam, theType, theDim, theName);

// the ADD_PARAMETER2 macro adds a parameter with range checking to the parameter list
#define ADD_PARAMETER2(theParam, theType, theRange, theName) \
	AddParameter(&theParam, theType, 1, theRange, theName);

// the ADD_PARAMETERV macro adds a parameter to the paramter list
// that is an array
#define ADD_PARAMETERV2(theParam, theType, theDim, theRange, theName) \
	AddParameter(theParam, theType, theDim, theRange, theName);


// the END_PARAMETER_LIST defines the end of a parameter list
#define END_PARAMETER_LIST() \
	}
