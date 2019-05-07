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
#include "FE_enum.h"
#include "vec3d.h"
#include "quatd.h"
#include "FEObject.h"

//-----------------------------------------------------------------------------
class FEModel;
class FERigidBodyDisplacement;

//-----------------------------------------------------------------------------
//! rigid body class
//! \todo perhaps the rigid body should store a list of domains it uses.
//!       That way, we can have multiple domains per RB using multiple 
//!       materials.
class FECORE_API FERigidBody : public FEObject
{
public:
	// Constructor
	FERigidBody(FEModel* pfem);

	//! desctructor
	virtual ~FERigidBody();

	//! Set the center of mass directly
	void SetCOM(vec3d rc);

	//! Update the mass of the rigid body
	void UpdateMass();

	//! Update total mass and center of mass
	void UpdateCOM();

	//! Update the moment of intertia
	void UpdateMOI();

	//! reset rigid body data
	void Reset() override;

	//! initialize data
	void Init() override;

	//! serialize data to archive
	void Serialize(DumpStream& ar) override;

	//! get the material ID
	int GetMaterialID() override { return m_mat; }
    
    //! incremental compound rotation from Cayley transform
    vec3d CayleyIncrementalCompoundRotation();

public:
	const quatd& GetRotation() const { return m_qt; }
	quatd GetPreviousRotation() const { return m_qp; }
	void SetRotation(const quatd& q)
	{
		m_qt = q;
		m_qt.GetEuler(m_euler.x, m_euler.y, m_euler.z);
	}

public:
	int		m_nID;		//!< ID of rigid body
	int		m_mat;		//!< material ID (TODO: Since rigid bodies can have multiple materials, I want to remove this)
	double	m_mass;		//!< total mass of rigid body
    mat3ds  m_moi;      //!< mass moment of inertia about center of mass
	vec3d	m_Fr, m_Mr;	//!< reaction force and torque
	vec3d	m_Fp, m_Mp;	//!< reaction force and torque at the end of the previous step

	vec3d	m_r0;	//!< initial position of rigid body
	vec3d	m_rp;	//!< previous position of rigid body
	vec3d	m_rt;	//!< current position of rigid body
    
	vec3d	m_vp;	//!< previous velocity of rigid body
	vec3d	m_vt;	//!< current velocity of rigid body
    
	vec3d	m_ap;	//!< previous acceleration of rigid body
	vec3d	m_at;	//!< current acceleration of rigid body

	quatd	m_qp;	//!< previous orientation of rigid body

private:
	// TODO: This is a hack!I only need this so I can access the euler angles directly from
	//       the optimization module. Need to figure out a better way.
	quatd	m_qt;		//!< current orientation of rigid body
	vec3d	m_euler;	//!< Euler angles of rotation 

public:
    vec3d   m_wp;   //!< previous angular velocity of rigid body
    vec3d   m_wt;   //!< current angular velocity of rigid body
    
    vec3d   m_alp;  //!< previous angular acceleration of rigid body
    vec3d   m_alt;  //!< current angular acceleration of rigid body
    
	int		m_BC[6];	//!< DOF types
	int		m_LM[6];	//!< dof equation numbers
	double	m_Up[6];	//!< previous displacement/rotation vector
	double	m_Ut[6];	//!< total displacement/rotation vector
	double	m_du[6];	//!< incremental displacement vector
	double	m_dul[6];	//!< displacement in local coordinates system

    bool    m_bpofr;    //!< flag for all or none of rotation dofs prescribed/fixed
    
public:
	FERigidBodyDisplacement*	m_pDC[6];	//!< active displacement constraints
	FERigidBody*	m_prb;	//!< parent rigid body

public:
	DECLARE_PARAMETER_LIST();
};
