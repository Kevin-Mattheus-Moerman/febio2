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

#include "FECore/vector.h"
#include "FECore/matrix.h"
#include "FECore/FESurfaceConstraint.h"
#include <list>
using namespace std;

//-----------------------------------------------------------------------------
//! linear constraint enforced using augmented lagrangian

class FEAugLagLinearConstraint
{
public:
	// this class describes a degree of freedom (dof) that
	// participates in the linear constraint
	class DOF
	{
	public:
		DOF() { node = bc = 0; val = 0.0; }
	public:
		int	node;		// the node to which this dof belongs to
		int	bc;			// the degree of freedom
		double	val;	// coefficient value
	};

	typedef list<FEAugLagLinearConstraint::DOF>::iterator Iterator;

public:
	//! constructor
	FEAugLagLinearConstraint() { m_lam = 0; }

	//! serialize data to archive
	void Serialize(DumpStream& ar);

public:
	list<DOF>	m_dof;	//!< list of participating dofs
	double		m_lam;	//!< lagrange multiplier
};

//-----------------------------------------------------------------------------
//! This class manages a group of linear constraints

class FELinearConstraintSet : public FESurfaceConstraint
{
public:
	//! constructor
	FELinearConstraintSet(FEModel* pfem);

	//! add a linear constraint to the list
	void add(FEAugLagLinearConstraint* plc) { m_LC.push_back(plc); }

public:
	//! serialize data to archive
	void Serialize(DumpStream& ar) override;

	//! add the linear constraint contributions to the residual
	void Residual(FEGlobalVector& R, const FETimeInfo& tp) override;

	//! add the linear constraint contributions to the stiffness matrix
	void StiffnessMatrix(FESolver* psolver, const FETimeInfo& tp) override;

	//! do the augmentation
	bool Augment(int naug, const FETimeInfo& tp) override;

	//! build connectivity for matrix profile
	void BuildMatrixProfile(FEGlobalMatrix& M) override;

protected:
	//! calculate the constraint value
	double constraint(FEAugLagLinearConstraint& LC);

public:
	list<FEAugLagLinearConstraint*>	m_LC;	//!< list of linear constraints

public:
	bool	m_laugon;	//!< augmentation flag
	double	m_tol;	//!< augmentation tolerance
	double	m_eps;	//!< penalty factor
    double  m_rhs;  //!< right-hand-side of linear constraint equation
	int		m_naugmax;	//!< max nr of augmentations
	int		m_naugmin;	//!< min nf of augmentations

	int	m_nID;		//!< ID of manager

	DECLARE_PARAMETER_LIST();
};
