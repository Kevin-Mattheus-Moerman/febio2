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
#include "DumpStream.h"
#include <vector>

//-----------------------------------------------------------------------------
// forward declaration of the model class.
class FEModel;

//-----------------------------------------------------------------------------
//! The FEObject class defines a physical object. An object can be for instance
//! a rigid body or a deformable object. Objects can be connected. For example,
//! a rigid body can be tied to a deformable object.

// NOTE: This is currently only used as a method to abstract the rigid body concept.

class FECORE_API FEObject : public FEParamContainer
{
public:
	//! constructor
	FEObject(FEModel* pfem) : m_fem(*pfem) {}

	//! destructor
	virtual ~FEObject(){}

	// object serialization
	virtual void Serialize(DumpStream& ar) = 0;

	//! initialize object
	virtual void Init() = 0;

	//! reset object data
	virtual void Reset() = 0;

	//! get the material ID
	virtual int GetMaterialID() { assert(false); return -1; }

protected:
	FEModel&	m_fem;	//!< Pointer to FE model
};
