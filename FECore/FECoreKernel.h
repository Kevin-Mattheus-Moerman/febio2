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
#include "FECoreFactory.h"
#include <vector>
#include <string.h>
#include <stdio.h>
#include "version.h"

//-----------------------------------------------------------------------------
// Forward declarations
class FEModel;
class Logfile;
class Timer;

//-----------------------------------------------------------------------------
//! This is the FECore kernel class that manages the interactions between the 
//! different modules. In particular, it manages the factory classes
//! which are responsible for the creation of different classes that are registered
//! with the kernel.
class FECORE_API FECoreKernel
{
	enum { ALL_MODULES = 0xFFFF };

	struct Module
	{
		const char*		szname;	// name of module
		unsigned int	id;		// unqiue ID (this is a bit value)
		unsigned int	flags;	// ID + IDs of dependent modules
	};

public:
	// Do not call this function from a plugin as it will not return the correct
	// instance. Instead, use the FECoreKernel object that is passed in the PluginInitialize method
	static FECoreKernel& GetInstance();

	// set the instance of the kernel
	static void SetInstance(FECoreKernel* pkernel);

	// Get the logfile
	static Logfile& GetLogfile();

public:
	//! Register a class with the framework
	void RegisterFactory(FECoreFactory* ptf);

	//! Create a specific class
	void* Create(SUPER_CLASS_ID, const char* sztag, FEModel* pfem);

	//! count the number of registered classes with a given super-class id
	int Count(SUPER_CLASS_ID sid);

	//! List the registered classes with a given super-class id
	void List(SUPER_CLASS_ID sid);

	//! Get the number of registered factory classes
	int FactoryClasses();

	//! return a factory class
	const FECoreFactory* GetFactoryClass(int i);

	//! find a factory class
	FECoreFactory* FindFactoryClass(int classID, const char* sztype);

	//! remove a factory class
	bool UnregisterFactory(FECoreFactory* ptf);

public: // Modules

	//! Create a module (also makes it the active module)
	bool CreateModule(const char* szmodule);

	//! set the active module
	bool SetActiveModule(const char* szmodule);

	//! set a dependency on a module
	bool SetModuleDependency(const char* szmodule);

	//! remove a module
	bool RemoveModule(const char* szmodule);

public:
	//! Register a new domain class
	void RegisterDomain(FEDomainFactory* pf);

	//! Create a domain of a certain type
	FEDomain* CreateDomain(const FE_Element_Spec& spec, FEMesh* pm, FEMaterial* pmat);

public: // error reporting mechanism

	// Set the error string
	void SetErrorString(const char* sz);

	// returns error string (can be null if no error reported)
	const char* GetErrorString();

public:
	//! Register a linear solver factory
	void RegisterLinearSolver(FELinearSolverFactory* pf);

	//! create a linear solver factory
	LinearSolver* CreateLinearSolver(int nsolver);

	//! Find linear solver factory
	FELinearSolverFactory* FindLinearSolverFactory(int nsolver);

public:
	//! set the default linear solver
	static void SetDefaultSolver(int nsolver) { m_ndefault_solver = nsolver; }
	static int m_ndefault_solver;

public:
	// reset all the timers
	void ResetAllTimers();

	// Find a timer by name. Returns an existing timer or otherwise creates a new timer with that name
	Timer* FindTimer(const std::string& name);

	// return total number of timers
	int Timers();

	// return a timer by index
	Timer* GetTimer(int i);

private:
	std::vector<FECoreFactory*>			m_Fac;	// list of registered factory classes
	std::vector<FEDomainFactory*>		m_Dom;	// list of domain factory classes
	std::vector<FELinearSolverFactory*> m_LS;	// list of linear solver factories
	std::vector<Timer*>					m_timers;	// list of timers

	// module list
	vector<Module>	m_modules;
	int				m_activeModule;

	Logfile*	m_plog;	// keep a pointer to the logfile (used by plugins)

	char*	m_szerr;	//!< error string

private: // make singleton
	FECoreKernel();
	FECoreKernel(const FECoreKernel&){}
	void operator = (const FECoreKernel&){}

private:
	static FECoreKernel* m_pKernel;	// the one-and-only kernel object
};

//-----------------------------------------------------------------------------
// helper function for reporting errors
FECORE_API bool fecore_error(const char* szerr, ...);
FECORE_API const char* fecore_get_error_string();

//-----------------------------------------------------------------------------
//! This class helps with the registration of a class with the framework
template <typename T> class FERegisterClass_T : public FECoreFactory
{
public:
	FERegisterClass_T(SUPER_CLASS_ID sid, const char* sz) : FECoreFactory(sid, sz)
	{
		FECoreKernel& fecore = FECoreKernel::GetInstance();
		fecore.RegisterFactory(this);
	}
	void* Create(FEModel* pfem) { return new T(pfem); }
};

//-----------------------------------------------------------------------------
// Register a class using default creation parameters
#define REGISTER_FECORE_CLASS(theClass, theSID, theName) \
	static FERegisterClass_T<theClass> _##theClass##_rc(theSID, theName);

//-----------------------------------------------------------------------------
// Register a deprecated class using default creation parameters
#define REGISTER_FECORE_CLASS_OBSOLETE(theClass, theSID, theName) \
	static FERegisterClass_T<theClass> _##theClass##_old_rc(theSID, theName);

//-----------------------------------------------------------------------------
// version for classes that require template arguments
#define REGISTER_FECORE_CLASS_T(theClass, theSID, theArg, theName) \
	static FERegisterClass_T<theClass<theArg> > _##theClass##theArg##_rc(theSID, theName);

//-----------------------------------------------------------------------------
template <typename TBase> inline TBase* fecore_new(SUPER_CLASS_ID sid, const char* sztype, FEModel* pfem)
{
	FECoreKernel& fecore = FECoreKernel::GetInstance();
	return static_cast<TBase*>(fecore.Create(sid, sztype, pfem));
}

//=============================================================================
// TODO: Move all this stuff to sdk.h

//-----------------------------------------------------------------------------
// Template class for factory classes for plugins
template <typename T, SUPER_CLASS_ID sid> class FEPluginFactory_T : public FECoreFactory
{
public:
	FEPluginFactory_T(const char* sz) : FECoreFactory(sid, sz){}
	void* Create(FEModel* pfem) { return new T(pfem); }
};

//------------------------------------------------------------------------------
// This is for functions exported from a plugin
#ifdef WIN32
#define FECORE_EXPORT extern "C" __declspec(dllexport)
#else
#define FECORE_EXPORT extern "C"
#endif
