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

//-----------------------------------------------------------------------------
class FEEdge;
class FEModel;
class FESolver;
class FEGlobalVector;

//-----------------------------------------------------------------------------
class FECORE_API FEEdgeLoad : public FEBoundaryCondition
{
public:
	FEEdgeLoad(FEModel* pfem);
	virtual ~FEEdgeLoad(void);

	virtual void Create(int nsegs) = 0;

	//! Set the edge to apply the load to
	void SetEdge(FEEdge* pe) { m_pedge = pe; }

	//! Get the edge
	FEEdge& Edge() { return *m_pedge; }

public:
	//! set an attribute of the surface load
	virtual bool SetAttribute(const char* szatt, const char* szval) { return false; }

public:
	//! calculate stiffness matrix
	virtual void StiffnessMatrix(FESolver* psolver) = 0;

	//! calculate residual
	virtual void Residual(FEGlobalVector& R) = 0;

protected:
	FEEdge*	m_pedge;
};
