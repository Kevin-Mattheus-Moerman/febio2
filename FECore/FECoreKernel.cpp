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
#include "FECoreKernel.h"
#include "Logfile.h"
#include "Timer.h"
#include <stdarg.h>
using namespace std;

// set the default linear solver (0 is equivalent to skyline solver)
int FECoreKernel::m_ndefault_solver = 0;

//-----------------------------------------------------------------------------
//! Helper function for reporting errors
bool fecore_error(const char* sz, ...)
{
	// get a pointer to the argument list
	va_list	args;

	// make the message
	char szerr[512] = {0};
	va_start(args, sz);
	vsprintf(szerr, sz, args);
	va_end(args);

	// TODO: Perhaps I should report it to the logfile?
	FECoreKernel& fecore = FECoreKernel::GetInstance();
	fecore.SetErrorString(szerr);

	return false;
}

//-----------------------------------------------------------------------------
const char* fecore_get_error_string()
{
	FECoreKernel& fecore = FECoreKernel::GetInstance();
	return fecore.GetErrorString();
}

//-----------------------------------------------------------------------------
FECoreKernel* FECoreKernel::m_pKernel = 0;

//-----------------------------------------------------------------------------
FECoreKernel& FECoreKernel::GetInstance()
{
	if (m_pKernel == 0) m_pKernel = new FECoreKernel;
	return *m_pKernel;
}

//-----------------------------------------------------------------------------
// This function is used by plugins to make sure that the plugin and the executable
// are using the same kernel
void FECoreKernel::SetInstance(FECoreKernel* pkernel)
{
	m_pKernel = pkernel;
}

//-----------------------------------------------------------------------------
Logfile& FECoreKernel::GetLogfile()
{
	return *m_pKernel->m_plog;
}

//-----------------------------------------------------------------------------
FECoreKernel::FECoreKernel()
{
	m_plog = Logfile::GetInstance();
	m_szerr = 0;
	m_activeModule = -1;
}

//-----------------------------------------------------------------------------
// Sets the error string.
// Calling SetErrorString(null) can be used to clear the error string.
void FECoreKernel::SetErrorString(const char* sz)
{
	// always clear the current error first
	if (m_szerr) delete [] m_szerr; m_szerr = 0;

	// make sure there is a new error string
	if (sz == 0) return;
	int l = strlen(sz);
	m_szerr = new char[l+1];
	strncpy(m_szerr, sz, l);
	m_szerr[l] = 0;
}

//-----------------------------------------------------------------------------
const char* FECoreKernel::GetErrorString()
{
	return m_szerr;
}

//-----------------------------------------------------------------------------
void FECoreKernel::RegisterFactory(FECoreFactory* ptf)
{
	unsigned int activeID = 0;
	if (m_activeModule != -1)
	{
		Module& activeModule = m_modules[m_activeModule];
		activeID = activeModule.id;
	}

	// see if the name already exists
	for (int i=0; i<m_Fac.size(); ++i)
	{
		FECoreFactory* pfi = m_Fac[i];

		if (pfi->GetSuperClassID() == ptf->GetSuperClassID())
		{
			unsigned int id = pfi->GetModuleID();

			if ((id == activeID) && (strcmp(pfi->GetTypeStr(), ptf->GetTypeStr()) == 0))
			{
#ifdef _DEBUG
				fprintf(stderr, "WARNING: %s feature is redefined\n", ptf->GetTypeStr());
#endif
				m_Fac[i] = ptf;
				return;
			}
		}
	}

	// it doesn't so add it
	ptf->SetModuleID(activeID);
	m_Fac.push_back(ptf);
}

//-----------------------------------------------------------------------------
bool FECoreKernel::UnregisterFactory(FECoreFactory* ptf)
{
	for (vector<FECoreFactory*>::iterator it = m_Fac.begin(); it != m_Fac.end(); ++it)
	{
		FECoreFactory* pfi = *it;
		if (pfi == ptf)
		{
			m_Fac.erase(it);
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
//! Create an object. An object is created by specifying the super-class id
//! and the type-string. 
void* FECoreKernel::Create(SUPER_CLASS_ID id, const char* sztype, FEModel* pfem)
{
	if (sztype == 0) return 0;

	unsigned int activeID = 0;
	unsigned int flags = 0;
	if (m_activeModule != -1)
	{
		Module& activeModule = m_modules[m_activeModule];
		activeID = activeModule.id;
		flags = activeModule.flags;
	}

	// first find by module
	if (activeID != 0)
	{
		std::vector<FECoreFactory*>::iterator pf;
		for (pf=m_Fac.begin(); pf!= m_Fac.end(); ++pf)
		{
			FECoreFactory* pfac = *pf;
			if (pfac->GetSuperClassID() == id) {

				unsigned int mid = pfac->GetModuleID();
				if ((mid == activeID) && (strcmp(pfac->GetTypeStr(), sztype) == 0))
				{
					return pfac->CreateInstance(pfem);
				}
			}
		}
	}

	// check dependencies
	if (flags != 0)
	{
		std::vector<FECoreFactory*>::iterator pf;
		for (pf = m_Fac.begin(); pf != m_Fac.end(); ++pf)
		{
			FECoreFactory* pfac = *pf;
			if (pfac->GetSuperClassID() == id) {

				unsigned int mid = pfac->GetModuleID();
				if ((mid & flags) && (strcmp(pfac->GetTypeStr(), sztype) == 0))
				{
					return pfac->CreateInstance(pfem);
				}
			}
		}
	}

	// we didn't find it.
	// Let's ignore module
	// TODO: This is mostly for backward compatibility, but eventually should be removed
	std::vector<FECoreFactory*>::iterator pf;
	for (pf = m_Fac.begin(); pf != m_Fac.end(); ++pf)
	{
		FECoreFactory* pfac = *pf;
		if (pfac->GetSuperClassID() == id) {
			if (strcmp(pfac->GetTypeStr(), sztype) == 0)
			{
				return pfac->CreateInstance(pfem);
			}
		}
	}

/*
#ifdef _DEBUG
	fprintf(stderr, "Unable to create class\n. These are the possible values:\n");
	for (pf=m_Fac.begin(); pf!=m_Fac.end(); ++pf)
	  {
	    FECoreFactory* pfac = *pf;
	    if (pfac->GetSuperClassID() == id) fprintf(stderr, "%s\n", pfac->GetTypeStr());
	  }
#endif
*/
	return 0;
}

//-----------------------------------------------------------------------------
int FECoreKernel::Count(SUPER_CLASS_ID sid)
{
	int N = 0;
	std::vector<FECoreFactory*>::iterator pf;
	for (pf=m_Fac.begin(); pf!= m_Fac.end(); ++pf)
	{
		FECoreFactory* pfac = *pf;
		if (pfac->GetSuperClassID() == sid) N++;
	}
	return N;
}

//-----------------------------------------------------------------------------
void FECoreKernel::List(SUPER_CLASS_ID sid)
{
  std::vector<FECoreFactory*>::iterator pf;
  for (pf = m_Fac.begin(); pf != m_Fac.end(); ++pf)
    {
      FECoreFactory* pfac = *pf;
      if (pfac->GetSuperClassID() == sid) fprintf(stdout, "%s\n", pfac->GetTypeStr());
    }
}

//-----------------------------------------------------------------------------
int FECoreKernel::FactoryClasses()
{
	return (int) m_Fac.size();
}

//-----------------------------------------------------------------------------
const FECoreFactory* FECoreKernel::GetFactoryClass(int i)
{
	return m_Fac[i];
}

//-----------------------------------------------------------------------------
FECoreFactory* FECoreKernel::FindFactoryClass(int classID, const char* sztype)
{
	for (size_t i=0; i<m_Fac.size(); ++i)
	{
		FECoreFactory* fac = m_Fac[i];
		if ((fac->GetSuperClassID() == classID) &&
			(strcmp(fac->GetTypeStr(), sztype) == 0)) return fac;
	}
	return 0;
}

//-----------------------------------------------------------------------------
//! set the active module
bool FECoreKernel::SetActiveModule(const char* szmod)
{
	// See if user want to deactivate modules
	if (szmod == 0)
	{
		m_activeModule = -1;
		return true;
	}

	// see if the module exists or not
	for (size_t i=0; i<m_modules.size(); ++i) 
	{
		Module& mi = m_modules[i];
		if (strcmp(mi.szname, szmod) == 0)
		{
			m_activeModule = (int) i;
			return true;
		}
	}

	// couldn't find it
	m_activeModule = -1;
	return false;
}

//-----------------------------------------------------------------------------
//! create a module
bool FECoreKernel::CreateModule(const char* szmod)
{
	m_activeModule = -1;
	if (szmod == 0) return false;

	// see if this module already exist
	if (SetActiveModule(szmod) == false)
	{
		// The module does not exist, so let's add it.
		int newID = (1 << m_modules.size());
		Module newModule;
		newModule.szname = szmod;
		newModule.id = newID;
		newModule.flags = newID;
		m_modules.push_back(newModule);

		// make this the active module
		m_activeModule = (int)m_modules.size() - 1;
	}

	return true;
}

//-----------------------------------------------------------------------------
//! remove a module
bool FECoreKernel::RemoveModule(const char* szmodule)
{
	for (std::vector<Module>::iterator it = m_modules.begin(); it != m_modules.end(); ++it)
	{
		if (strcmp((*it).szname, szmodule) == 0)
		{
			m_modules.erase(it);
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
//! set a dependency on a module
bool FECoreKernel::SetModuleDependency(const char* szmodule)
{
	if (m_activeModule == -1) return false;
	Module& activeModule = m_modules[m_activeModule];

	if (szmodule == 0)
	{
		// clear dependencies
		activeModule.flags = activeModule.id;
		return true;
	}

	// find the module
	for (size_t i = 0; i<m_modules.size(); ++i)
	{
		Module& mi = m_modules[i];
		if (strcmp(mi.szname, szmodule) == 0)
		{
			activeModule.flags |= mi.id;
			return true;
		}
	}

	// oh, oh, couldn't find it
	return false;
}

//-----------------------------------------------------------------------------
//! Register a new domain class
void FECoreKernel::RegisterDomain(FEDomainFactory* pf)
{
	m_Dom.push_back(pf); 
}

//-----------------------------------------------------------------------------
FEDomain* FECoreKernel::CreateDomain(const FE_Element_Spec& spec, FEMesh* pm, FEMaterial* pmat)
{
	for (int i=0; i<(int)m_Dom.size(); ++i)
	{
		FEDomain* pdom = m_Dom[i]->CreateDomain(spec, pm, pmat);
		if (pdom != 0) return pdom;
	}
	return 0;
}

//-----------------------------------------------------------------------------
void FECoreKernel::RegisterLinearSolver(FELinearSolverFactory* pf)
{
	m_LS.push_back(pf); 
}

//-----------------------------------------------------------------------------
LinearSolver* FECoreKernel::CreateLinearSolver(int nsolver)
{
	for (int i=0; i<(int)m_LS.size(); ++i)
	{
		FELinearSolverFactory* pls = m_LS[i];
		if (pls->GetID() == nsolver) return pls->Create();
	}
	return 0;
}

//-----------------------------------------------------------------------------
FELinearSolverFactory* FECoreKernel::FindLinearSolverFactory(int nsolver)
{
	for (int i = 0; i<(int)m_LS.size(); ++i)
	{
		FELinearSolverFactory* pls = m_LS[i];
		if (pls->GetID() == nsolver) return pls;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// reset all the timers
void FECoreKernel::ResetAllTimers()
{
	for (size_t i = 0; i<m_timers.size(); ++i)
	{
		Timer* ti = m_timers[i];
		ti->reset();
	}
}

//-----------------------------------------------------------------------------
Timer* FECoreKernel::FindTimer(const std::string& name)
{
	// see if the timer already exists
	for (size_t i = 0; i<m_timers.size(); ++i)
	{
		if (m_timers[i]->name() == name) return m_timers[i];
	}

	// create new timer
	Timer* newTimer = new Timer;
	newTimer->setName(name);

	// add it to the list
	m_timers.push_back(newTimer);

	// return it
	return newTimer;
}

//-----------------------------------------------------------------------------
int FECoreKernel::Timers()
{
	return (int)m_timers.size();
}

//-----------------------------------------------------------------------------
Timer* FECoreKernel::GetTimer(int i)
{
	return m_timers[i];
}
