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
#include "FE_enum.h"
#include "FECoreBase.h"

//-----------------------------------------------------------------------------
//! Forward declaration of the FEModel class. All classes that register
//! with the framework take a pointer to FEModel as their constructor parameter.
class FEModel;

//-----------------------------------------------------------------------------
//! The factory class contains the mechanism for instantiating a class.
class FECORE_API FECoreFactory
{
public:
	//! constructor
	FECoreFactory(SUPER_CLASS_ID scid, const char* sztype);

	//! virtual constructor
	virtual ~FECoreFactory();

	//! This is the function that the kernel will use to intantiate an object
	void* CreateInstance(FEModel* pfem);

public:
	// return the type string identifier
	const char* GetTypeStr() const { return m_sztype; }

	//! return the super-class ID
	SUPER_CLASS_ID GetSuperClassID() const { return m_scid; }

	//! return the module name
	unsigned int GetModuleID() const { return m_module; }

	//! set the module name
	void SetModuleID(unsigned int nid);
	
public:
	//! derived classes implement this to create an instance of a class
	virtual void* Create(FEModel*) = 0;

private:
	const char*		m_sztype;	//!< class type string
	unsigned int	m_module;	//!< ID of module this class belongs to
	SUPER_CLASS_ID	m_scid;		//!< the super-class ID
};

//-----------------------------------------------------------------------------
//! Forward declarations of classes used by the domain factory
class FEDomain;
class FEMesh;
class FEMaterial;

//-----------------------------------------------------------------------------
//! Creation of domains are a little more elaborate and deviate from the usual
//! factory methods.
class FECORE_API FEDomainFactory
{
public:
	FEDomainFactory(){}
	virtual ~FEDomainFactory(){}

	virtual FEDomain* CreateDomain(const FE_Element_Spec& spec, FEMesh* pm, FEMaterial* pmat) = 0;
};

//-----------------------------------------------------------------------------
// factory class for linear solvers.
class LinearSolver;

class FECORE_API FELinearSolverFactory : public FEParamContainer
{
public:
	FELinearSolverFactory(int nid) : m_nsolver_id(nid) {}
	virtual ~FELinearSolverFactory(){}

	virtual LinearSolver* Create() = 0;

	int GetID() { return m_nsolver_id; }

private:
	int	m_nsolver_id;
};
