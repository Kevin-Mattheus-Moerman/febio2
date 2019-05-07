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
#include "FECore/FETrussDomain.h"
#include "FEElasticDomain.h"
#include "FETrussMaterial.h"

//-----------------------------------------------------------------------------
//! Domain described by 3D truss elements
class FEElasticTrussDomain : public FETrussDomain, public FEElasticDomain
{
public:
	//! Constructor
	FEElasticTrussDomain(FEModel* pfem);

	//! copy operator
	FEElasticTrussDomain& operator = (FEElasticTrussDomain& d) { m_Elem = d.m_Elem; m_pMesh = d.m_pMesh; return (*this); }

	//! Initialize elements
	void PreSolveUpdate(const FETimeInfo& timeInfo);

	//! Unpack truss element data
	void UnpackLM(FEElement& el, vector<int>& lm);

	//! get the material (overridden from FEDomain)
	FEMaterial* GetMaterial() { return m_pMat; }

	//! set the material
	void SetMaterial(FEMaterial* pmat);

	//! Activate domain
	void Activate();

public: // overloads from FEElasticDomain

	//! update the truss stresses
	void Update(const FETimeInfo& tp);

	//! internal stress forces
	void InternalForces(FEGlobalVector& R);

	//! calculate body force \todo implement this
	void BodyForce(FEGlobalVector& R, FEBodyForce& bf) { assert(false); }

	//! Calculates inertial forces for dynamic problems
	void InertialForces(FEGlobalVector& R, vector<double>& F) { assert(false); }

	//! calculates the global stiffness matrix for this domain
	void StiffnessMatrix(FESolver* psolver);

	//! intertial stiffness matrix \todo implement this
	void MassMatrix(FESolver* psolver, double scale) { assert(false); }

	//! body force stiffness matrix \todo implement this
	void BodyForceStiffness(FESolver* psolver, FEBodyForce& bf) { assert(false); }

protected:
	//! calculates the truss element stiffness matrix
	void ElementStiffness(int iel, matrix& ke);

	//! Calculates the internal stress vector for solid elements
	void ElementInternalForces(FETrussElement& el, vector<double>& fe);

protected:
	FETrussMaterial*	m_pMat;
};
