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
#include "FEModelComponent.h"
#include "FESurface.h"

//-----------------------------------------------------------------------------
class FEModel;
class FEGlobalMatrix;

//-----------------------------------------------------------------------------
//! This class describes a general purpose interaction between two surfaces.
// TODO: I like to inherit this from FENLConstraint and potentially eliminate
//       The distinction between a nonlinear constraint and a contact interface.
//       Since a contact interface essentially is a nonlinear constraint, I think
//       this may make things a lot easier. I already made the function definitions consistent
//       but am hesitant to push this through at this point. 
class FECORE_API FESurfacePairConstraint : public FEModelComponent
{
public:
	//! constructor
	FESurfacePairConstraint(FEModel* pfem);

public:
	//! return the master surface
	virtual FESurface* GetMasterSurface() = 0;

	//! return the slave surface
	virtual FESurface* GetSlaveSurface () = 0;

	//! temporary construct to determine if contact interface uses nodal integration rule (or facet)
	virtual bool UseNodalIntegration() = 0;

	//! create a copy of this interface
	virtual void CopyFrom(FESurfacePairConstraint* pci) {}

public:
	// Build the matrix profile
	virtual void BuildMatrixProfile(FEGlobalMatrix& M) = 0;

	// Update state
	virtual void Update(int niter, const FETimeInfo& tp) {}

	// reset the state data
	virtual void Reset() {}
};
