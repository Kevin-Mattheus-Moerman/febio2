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
#include "FEMortarInterface.h"
#include "FEMortarContactSurface.h"

//-----------------------------------------------------------------------------
//! This class represents a surface used by the mortar contact interface.
class FEMortarSlidingSurface : public FEMortarContactSurface
{
public:
	FEMortarSlidingSurface(FEModel* pfem);

	//! Initializes data structures
	bool Init();

	//! update the normals
	void UpdateNormals(bool binit);

public:
	vector<double>	m_p;		//!< nodal contact pressures
	vector<double>	m_L;		//!< Lagrange multipliers
	vector<vec3d>	m_nu;		//!< nodal normals
	vector<double>	m_norm0;	//!< initial (inverse) normal lenghts
};

//-----------------------------------------------------------------------------
//! This class implements a mortar contact formulation for frictionless, sliding contact
class FEMortarSlidingContact : public FEMortarInterface
{
public:
	//! constructor
	FEMortarSlidingContact(FEModel* pfem);

	//! destructor
	~FEMortarSlidingContact();

	//! return the master and slave surface
	FESurface* GetMasterSurface() override { return &m_ms; }
	FESurface* GetSlaveSurface () override { return &m_ss; }

public:
	//! temporary construct to determine if contact interface uses nodal integration rule (or facet)
	bool UseNodalIntegration() override { return false; }

	//! interface activation
	void Activate() override;

	//! one-time initialization
	bool Init() override;

	//! calculate contact forces
	void Residual(FEGlobalVector& R, const FETimeInfo& tp) override;

	//! calculate contact stiffness
	void StiffnessMatrix(FESolver* psolver, const FETimeInfo& tp) override;

	//! calculate Lagrangian augmentations
	bool Augment(int naug, const FETimeInfo& tp) override;

	//! serialize data to archive
	void Serialize(DumpStream& ar) override;

	//! build the matrix profile for use in the stiffness matrix
	void BuildMatrixProfile(FEGlobalMatrix& K) override;

	//! update interface data
	void Update(int niter, const FETimeInfo& tp) override;

protected:
	// contact stiffness contributions
	void ContactGapStiffness(FESolver* psolver);
	void ContactNormalStiffness(FESolver* psolver);

private:
	double	m_atol;		//!< augmented Lagrangian tolerance
	double	m_eps;		//!< penalty factor
	int		m_naugmin;	//!< minimum number of augmentations
	int		m_naugmax;	//!< maximum number of augmentations

private:
	FEMortarSlidingSurface	m_ms;	//!< mortar surface
	FEMortarSlidingSurface	m_ss;	//!< non-mortar surface

	int		m_dofX;
	int		m_dofY;
	int		m_dofZ;

	DECLARE_PARAMETER_LIST();
};
