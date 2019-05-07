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
#include "FESolver.h"
#include "FEModel.h"

BEGIN_PARAMETER_LIST(FESolver, FECoreBase)
	ADD_PARAMETER(m_bsymm, FE_PARAM_BOOL, "symmetric_stiffness");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FESolver::FESolver(FEModel* pfem) : FECoreBase(FESOLVER_ID), m_fem(*pfem)
{ 
	m_bsymm = true; // assume symmetric stiffness matrix
	m_niter = 0;

	m_nref = 0;
	
	m_baugment = false;
	m_naug = 0;
}

//-----------------------------------------------------------------------------
FESolver::~FESolver()
{
}

//-----------------------------------------------------------------------------
//! Get the FE model
FEModel& FESolver::GetFEModel()
{ 
	return m_fem; 
}

//-----------------------------------------------------------------------------
void FESolver::Clean()
{
}

//-----------------------------------------------------------------------------
//! This function is called right before SolveStep and should be used to initialize
//! time dependent information and other settings.
bool FESolver::InitStep(double time)
{
	FEModel& fem = GetFEModel();

	// evaluate load curve values at current time
	fem.EvaluateLoadCurves(time);

	// evaluate the parameter lists
	fem.EvaluateAllParameterLists();

	// re-validate materials
	// This is necessary since the material parameters can have changed (e.g. via load curves) and thus 
	// a new validation needs to be done to see if the material parameters are still valid. 
	if (fem.ValidateMaterials() == false) return false;

	return true;
}

//-----------------------------------------------------------------------------
bool FESolver::Init()
{
	// parameter checking
	return Validate();
}

//-----------------------------------------------------------------------------
void FESolver::Serialize(DumpStream& ar)
{
	FECoreBase::Serialize(ar);
	if (ar.IsShallow())
	{
		if (ar.IsSaving())
		{
			ar << m_bsymm;
			ar << m_nrhs << m_niter << m_nref << m_ntotref << m_naug;
		}
		else
		{
			ar >> m_bsymm;
			ar >> m_nrhs >> m_niter >> m_nref >> m_ntotref >> m_naug;
		}
	}
}
