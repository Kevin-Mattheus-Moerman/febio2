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
#include "FERigidCable.h"
#include <FECore/FERigidSystem.h>
#include <FECore/FERigidBody.h>
#include <FECore/FEModel.h>

//=============================================================================
BEGIN_PARAMETER_LIST(FERigidCable::FECablePoint, FECoreBase)
	ADD_PARAMETER(m_pos, FE_PARAM_VEC3D, "point");
	ADD_PARAMETER(m_pos, FE_PARAM_VEC3D, "position");
END_PARAMETER_LIST();

bool FERigidCable::FECablePoint::SetAttribute(const char* szatt, const char* szval)
{
	if (strcmp(szatt, "rb") == 0)
	{
		m_rb = atoi(szval);
		return true;
	}
	else return false;
}

//=============================================================================
BEGIN_PARAMETER_LIST(FERigidCable, FEModelLoad)
	ADD_PARAMETER(m_force   , FE_PARAM_DOUBLE, "force");
	ADD_PARAMETER(m_forceDir, FE_PARAM_VEC3D , "force_direction");
	ADD_PARAMETER(m_brelative, FE_PARAM_BOOL , "relative");
END_PARAMETER_LIST();

FERigidCable::FERigidCable(FEModel* fem) : FEModelLoad(FEBC_ID, fem)
{
	m_force = 0;
	m_forceDir = vec3d(0,0,-1);
	m_brelative = true;

	// add the points property
	AddProperty(&m_points, "point", FEProperty::ValueProperty);
}

//! override for building points list
FECoreBase* FERigidCable::GetProperty(int n)
{
	// make sure this references the point property
	if (n != 0) return 0;

	// create a new point
	FECablePoint* p = new FECablePoint(0);
	m_points.AddProperty(p);

	return p;
}

//! initialization
bool FERigidCable::Init()
{
	if (FEModelLoad::Init() == false) return false;

	// make sure the force direction is a unit vector
	m_forceDir.unit();

	// get the rigid system
	FERigidSystem& rigid = *GetFEModel()->GetRigidSystem();

	// correct the rigid body indices
	for (int i=0; i<m_points.size(); ++i)
	{
		m_points[i]->m_rb = rigid.FindRigidbodyFromMaterialID(m_points[i]->m_rb - 1);
		if (m_points[i]->m_rb == -1) return false;
	}

	return true;
}

// helper function for applying forces to rigid bodies
void FERigidCable::applyRigidForce(FERigidBody& rb, const vec3d& F, const vec3d& p, FEGlobalVector& R)
{
	vec3d rd = (m_brelative ? p : p - rb.m_r0);
	vec3d M = rd ^ F;
	int n;
	n = rb.m_LM[0]; if (n >= 0) R[n] += F.x;
	n = rb.m_LM[1]; if (n >= 0) R[n] += F.y;
	n = rb.m_LM[2]; if (n >= 0) R[n] += F.z;
	n = rb.m_LM[3]; if (n >= 0) R[n] += M.x;
	n = rb.m_LM[4]; if (n >= 0) R[n] += M.y;
	n = rb.m_LM[5]; if (n >= 0) R[n] += M.z;

	rb.m_Fr += F;
	rb.m_Mr += M;
}

//! Residual
void FERigidCable::Residual(FEGlobalVector& R, const FETimeInfo& tp)
{
	int npoints = m_points.size();
	if (npoints == 0) return;

	// get the rigid system
	FERigidSystem& rigid = *GetFEModel()->GetRigidSystem();

	// get the last rigid body
	int rb0 = m_points[npoints - 1]->m_rb;
	FERigidBody& bodyZ = *rigid.Object(rb0);
	vec3d p0 = m_points[npoints - 1]->m_pos;

	// apply the force to the last attachment point
	vec3d F = m_forceDir*m_force;
	applyRigidForce(bodyZ, F, p0, R);

	// apply paired forces to all the other pairs
	for (int i=0; i<npoints-1; ++i)
	{
		int rbA = m_points[i  ]->m_rb;
		int rbB = m_points[i+1]->m_rb;

		FERigidBody& bodyA = *rigid.Object(rbA);
		FERigidBody& bodyB = *rigid.Object(rbB);

		vec3d r0A = m_points[i    ]->m_pos;
		vec3d r0B = m_points[i + 1]->m_pos;

		// get the attachment position in global coordinates for body A
		vec3d da0 = (m_brelative ? r0A : r0A - bodyA.m_r0);
		vec3d da = bodyA.GetRotation()*da0;
		vec3d a = bodyA.m_rt + da;

		// get the attachment position in global coordinates for body B
		vec3d db0 = (m_brelative ? r0B : r0B - bodyB.m_r0);
		vec3d db = bodyB.GetRotation()*db0;
		vec3d b = bodyB.m_rt + db;

		// get the unit axial vector
		vec3d N = b - a; N.unit();

		// calculate the force value
		double f = m_force;

		// get the axial force
		vec3d F = N*f;

		// apply the forces
		applyRigidForce(bodyA,  F, r0A, R);
		applyRigidForce(bodyB, -F, r0B, R);
	}
}

//! Stiffness matrix
void FERigidCable::StiffnessMatrix(FESolver* psolver, const FETimeInfo& tp)
{
	int npoints = m_points.size();
	if (npoints < 2) return;

	FERigidSystem& rigid = *GetFEModel()->GetRigidSystem();
	for (int i=0; i<npoints-1; ++i)
	{
		int idA = m_points[i  ]->m_rb;
		int idB = m_points[i+1]->m_rb;

		// Get the rigid bodies
		FERigidBody& bodyA = *rigid.Object(idA);
		FERigidBody& bodyB = *rigid.Object(idB);

		// get the force positions
		vec3d ra0 = m_points[i]->m_pos;
		vec3d rb0 = m_points[i+1]->m_pos;

		// get the attachment position in global coordinates for body A
		vec3d da0 = (m_brelative ? ra0 : ra0 - bodyA.m_r0);
		vec3d da = bodyA.GetRotation()*da0;
		vec3d pa = da + bodyA.m_rt;

		// get the attachment position in global coordinates for body B
		vec3d db0 = (m_brelative ? rb0 : rb0 - bodyB.m_r0);
		vec3d db = bodyB.GetRotation()*db0;
		vec3d pb = db + bodyB.m_rt;

		// setup the axial unit vector
		vec3d N = pb - pa;
		double L = N.unit();

		// build the stiffness matrix components
		mat3ds NxN = dyad(N);
		mat3ds M = mat3dd(1.0) - NxN;
		mat3ds S = M*(m_force / L);
		mat3da A(-da);
		mat3da B(-db);
		mat3da F(m_forceDir*m_force);

		mat3d SA = S*A;
		mat3d SB = S*B;
		mat3d AS = A*S;
		mat3d BS = B*S;
		mat3d FA = F*A;
		mat3d FB = F*B;

		mat3d ASA = A*SA;
		mat3d BSB = B*SB;
		mat3d ASB = A*SB;

		// put it all together
		matrix K(12, 12); K.zero();
		K.sub(0, 0, S);
		K.sub(0, 3, SA);
		K.add(0, 6, S);
		K.add(0, 9, SB);
		K.add(3, 3, ASA - FA);
		K.sub(3, 6, AS);
		K.sub(3, 9, ASB);
		K.sub(6, 6, S);
		K.sub(6, 9, SB);
		K.add(9, 9, BSB + FB);
	
		// since this is a symmetric matrix, fill the bottom triangular part
		K.copy_ut();

		// TODO: Not sure why, but it looks like I need a negative sign
		K *= -1.0;

		// get the equation numbers
		vector<int> lm(12);
		lm[0] = bodyA.m_LM[0];
		lm[1] = bodyA.m_LM[1];
		lm[2] = bodyA.m_LM[2];
		lm[3] = bodyA.m_LM[3];
		lm[4] = bodyA.m_LM[4];
		lm[5] = bodyA.m_LM[5];

		lm[6] = bodyB.m_LM[0];
		lm[7] = bodyB.m_LM[1];
		lm[8] = bodyB.m_LM[2];
		lm[9] = bodyB.m_LM[3];
		lm[10] = bodyB.m_LM[4];
		lm[11] = bodyB.m_LM[5];

		// assemble into global matrix
		psolver->AssembleStiffness(lm, K);
	}
}
