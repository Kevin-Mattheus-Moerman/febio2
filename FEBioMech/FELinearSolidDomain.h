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
#include "FECore/FESolidDomain.h"

//-----------------------------------------------------------------------------
class FESolidMaterial;

//-----------------------------------------------------------------------------
class FELinearElasticDomain
{
public:
	FELinearElasticDomain(FEModel* pfem);
	virtual ~FELinearElasticDomain(){}
	virtual void StiffnessMatrix(FESolver* psolver) = 0;
	virtual void RHS(FEGlobalVector& R) = 0;
};

//-----------------------------------------------------------------------------
//! Class describing a linear elastic solid domain
class FELinearSolidDomain : public FESolidDomain, public FELinearElasticDomain
{
public:
	//! constructor
	FELinearSolidDomain(FEModel* pfem, FEMaterial* pmat);

	//! get the material (overridden from FEDomain)
	FEMaterial* GetMaterial() override;

	//! initialize elements
	void PreSolveUpdate(const FETimeInfo& timeInfo) override;

public: // overrides from FELinearElasticDomain

	//! Build the stiffness matrix
	void StiffnessMatrix(FESolver* psolver) override;

	// Calculate the RHS vector
	void RHS(FEGlobalVector& R) override;

	//! Update domain data
	void Update(const FETimeInfo& tp) override;

protected:
	void InitialStress(FESolidElement& el, vector<double>& fe);
	void InternalForce(FESolidElement& el, vector<double>& fe);

	void ElementStiffness(FESolidElement& el, matrix& ke);

protected:
	FESolidMaterial*	m_pMat;
};
