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
#include "FERigidSphericalJoint.h"
#include "FECore/FERigidBody.h"
#include "FECore/log.h"
#include "FECore/FEModel.h"
#include "FECore/FEMaterial.h"

//-----------------------------------------------------------------------------
BEGIN_PARAMETER_LIST(FERigidSphericalJoint, FERigidConnector);
	ADD_PARAMETER(m_atol, FE_PARAM_DOUBLE, "tolerance"     );
	ADD_PARAMETER(m_gtol, FE_PARAM_DOUBLE, "gaptol"        );
	ADD_PARAMETER(m_qtol, FE_PARAM_DOUBLE, "angtol"        );
	ADD_PARAMETER(m_eps , FE_PARAM_DOUBLE, "force_penalty" );
	ADD_PARAMETER(m_ups , FE_PARAM_DOUBLE, "moment_penalty");
	ADD_PARAMETER(m_q0  , FE_PARAM_VEC3D , "joint_origin"  );
	ADD_PARAMETER(m_naugmin,FE_PARAM_INT , "minaug"        );
	ADD_PARAMETER(m_naugmax,FE_PARAM_INT , "maxaug"        );
	ADD_PARAMETER(m_bq     , FE_PARAM_BOOL  , "prescribed_rotation");
	ADD_PARAMETER(m_qpx    , FE_PARAM_DOUBLE, "rotation_x" );
	ADD_PARAMETER(m_qpy    , FE_PARAM_DOUBLE, "rotation_y" );
	ADD_PARAMETER(m_qpz    , FE_PARAM_DOUBLE, "rotation_z" );
	ADD_PARAMETER(m_Mpx    , FE_PARAM_DOUBLE, "moment_x"   );
	ADD_PARAMETER(m_Mpy    , FE_PARAM_DOUBLE, "moment_y"   );
	ADD_PARAMETER(m_Mpz    , FE_PARAM_DOUBLE, "moment_z"   );
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FERigidSphericalJoint::FERigidSphericalJoint(FEModel* pfem) : FERigidConnector(pfem)
{
    m_nID = m_ncount++;
    m_atol = 0;
    m_gtol = 0;
    m_qtol = 0;
    m_naugmin = 0;
    m_naugmax = 10;
    m_qpx = m_qpy = m_qpz = 0;
    m_Mpx = m_Mpy = m_Mpz = 0;
    m_bq = false;
}

//-----------------------------------------------------------------------------
//! TODO: This function is called twice: once in the Init and once in the Solve
//!       phase. Is that necessary?
bool FERigidSphericalJoint::Init()
{
    if (m_bq && ((m_Mpx != 0) || (m_Mpy != 0) || (m_Mpz != 0))) {
        felog.printbox("FATAL ERROR", "Rotation and moment cannot be prescribed simultaneously in rigid connector %d (spherical joint)\n", m_nID+1);
        return false;
    }
    
    FEModel& fem = *GetFEModel();
    
    // reset force
    m_F = vec3d(0,0,0); m_L = vec3d(0,0,0);
    m_M = vec3d(0,0,0); m_U = vec3d(0,0,0);
    
	// base class first
	if (FERigidConnector::Init() == false) return false;

    m_qa0 = m_q0 - m_rbA->m_r0;
    m_qb0 = m_q0 - m_rbB->m_r0;
    
    m_ea0[0] = m_e0[0]; m_ea0[1] = m_e0[1]; m_ea0[2] = m_e0[2];
    m_eb0[0] = m_e0[0]; m_eb0[1] = m_e0[1]; m_eb0[2] = m_e0[2];
    
    return true;
}

//-----------------------------------------------------------------------------
void FERigidSphericalJoint::Serialize(DumpStream& ar)
{
	FERigidConnector::Serialize(ar);
    if (ar.IsSaving())
    {
        ar << m_qa0 << m_qb0;
        ar << m_L << m_U;
		ar << m_e0[0] << m_e0[1] << m_e0[2];
		ar << m_ea0[0] << m_ea0[1] << m_ea0[2];
		ar << m_eb0[0] << m_eb0[1] << m_eb0[2];
    }
    else
    {
        ar >> m_qa0 >> m_qb0;
        ar >> m_L >> m_U;
		ar >> m_e0[0] >> m_e0[1] >> m_e0[2];
		ar >> m_ea0[0] >> m_ea0[1] >> m_ea0[2];
		ar >> m_eb0[0] >> m_eb0[1] >> m_eb0[2];
    }
}

//-----------------------------------------------------------------------------
//! \todo Why is this class not using the FESolver for assembly?
void FERigidSphericalJoint::Residual(FEGlobalVector& R, const FETimeInfo& tp)
{
    vector<double> fa(6);
    vector<double> fb(6);
    
	FERigidBody& RBa = *m_rbA;
	FERigidBody& RBb = *m_rbB;

	double alpha = tp.alpha;
    
    // body A
    vec3d ra = RBa.m_rt*alpha + RBa.m_rp*(1-alpha);
	vec3d zat = m_qa0; RBa.GetRotation().RotateVector(zat);
    vec3d zap = m_qa0; RBa.m_qp.RotateVector(zap);
    vec3d za = zat*alpha + zap*(1-alpha);
    
    // body b
    vec3d rb = RBb.m_rt*alpha + RBb.m_rp*(1-alpha);
	vec3d zbt = m_qb0; RBb.GetRotation().RotateVector(zbt);
    vec3d zbp = m_qb0; RBb.m_qp.RotateVector(zbp);
    vec3d zb = zbt*alpha + zbp*(1-alpha);
    
    vec3d c = rb + zb - ra - za;
    m_F = m_L + c*m_eps;
    
    if (m_bq) {
		quatd q = (alpha*RBb.GetRotation() + (1 - alpha)*RBb.m_qp)*(alpha*RBa.GetRotation() + (1 - alpha)*RBa.m_qp).Inverse();
        quatd a(vec3d(m_qpx,m_qpy,m_qpz));
        quatd r = a*q.Inverse();
        r.MakeUnit();
        vec3d ksi = r.GetVector()*r.GetAngle();
        m_M = m_U + ksi*m_ups;
    }
    else m_M = vec3d(m_Mpx,m_Mpy,m_Mpz);
    
    fa[0] = m_F.x;
    fa[1] = m_F.y;
    fa[2] = m_F.z;
    
    fa[3] = za.y*m_F.z-za.z*m_F.y + m_M.x;
    fa[4] = za.z*m_F.x-za.x*m_F.z + m_M.y;
    fa[5] = za.x*m_F.y-za.y*m_F.x + m_M.z;
    
    fb[0] = -m_F.x;
    fb[1] = -m_F.y;
    fb[2] = -m_F.z;
    
    fb[3] = -zb.y*m_F.z+zb.z*m_F.y - m_M.x;
    fb[4] = -zb.z*m_F.x+zb.x*m_F.z - m_M.y;
    fb[5] = -zb.x*m_F.y+zb.y*m_F.x - m_M.z;
    
    for (int i=0; i<6; ++i) if (RBa.m_LM[i] >= 0) R[RBa.m_LM[i]] += fa[i];
    for (int i=0; i<6; ++i) if (RBb.m_LM[i] >= 0) R[RBb.m_LM[i]] += fb[i];
    
    RBa.m_Fr -= vec3d(fa[0],fa[1],fa[2]);
    RBa.m_Mr -= vec3d(fa[3],fa[4],fa[5]);
    RBb.m_Fr -= vec3d(fb[0],fb[1],fb[2]);
    RBb.m_Mr -= vec3d(fb[3],fb[4],fb[5]);
}

//-----------------------------------------------------------------------------
//! \todo Why is this class not using the FESolver for assembly?
void FERigidSphericalJoint::StiffnessMatrix(FESolver* psolver, const FETimeInfo& tp)
{
    int j;

	double alpha = tp.alpha;
    
    vector<int> LM(12);
    matrix ke(12,12);
    ke.zero();
    
	FERigidBody& RBa = *m_rbA;
	FERigidBody& RBb = *m_rbB;

    // body A
    vec3d ra = RBa.m_rt*alpha + RBa.m_rp*(1-alpha);
	vec3d zat = m_qa0; RBa.GetRotation().RotateVector(zat);
    vec3d zap = m_qa0; RBa.m_qp.RotateVector(zap);
    vec3d za = zat*alpha + zap*(1-alpha);
    mat3d zahat; zahat.skew(za);
    mat3d zathat; zathat.skew(zat);
    
    // body b
    vec3d rb = RBb.m_rt*alpha + RBb.m_rp*(1-alpha);
	vec3d zbt = m_qb0; RBb.GetRotation().RotateVector(zbt);
    vec3d zbp = m_qb0; RBb.m_qp.RotateVector(zbp);
    vec3d zb = zbt*alpha + zbp*(1-alpha);
    mat3d zbhat; zbhat.skew(zb);
    mat3d zbthat; zbthat.skew(zbt);
    
    vec3d c = rb + zb - ra - za;
    m_F = m_L + c*m_eps;
    mat3dd I(1);
    
    quatd q, a, r;
    if (m_bq) {
		q = (alpha*RBb.GetRotation() + (1 - alpha)*RBb.m_qp)*(alpha*RBa.GetRotation() + (1 - alpha)*RBa.m_qp).Inverse();
        a = quatd(vec3d(m_qpx,m_qpy,m_qpz));
        r = a*q.Inverse();
        r.MakeUnit();
        vec3d ksi = r.GetVector()*r.GetAngle();
        m_M = m_U + ksi*m_ups;
    }
    else m_M = vec3d(m_Mpx,m_Mpy,m_Mpz);
    
    mat3d K, Wba(0,0,0,0,0,0,0,0,0), Wab(0,0,0,0,0,0,0,0,0);
    if (m_bq) {
		quatd qa = RBa.GetRotation()*(alpha*RBa.GetRotation() + (1 - alpha)*RBa.m_qp).Inverse();
		quatd qb = RBb.GetRotation()*(alpha*RBb.GetRotation() + (1 - alpha)*RBb.m_qp).Inverse();
        qa.MakeUnit();
        qb.MakeUnit();
        mat3d Qa = qa.RotationMatrix();
        mat3d Qb = qb.RotationMatrix();
        mat3d A = a.RotationMatrix();
        mat3d R = r.RotationMatrix();
        mat3dd I(1);
        Wba = A*(I*Qa.trace()-Qa)/2;
        Wab = R*(I*Qb.trace()-Qb)/2;
    }

    // (1,1)
    K = I*(alpha*m_eps);
    ke[0][0] = K[0][0]; ke[0][1] = K[0][1]; ke[0][2] = K[0][2];
    ke[1][0] = K[1][0]; ke[1][1] = K[1][1]; ke[1][2] = K[1][2];
    ke[2][0] = K[2][0]; ke[2][1] = K[2][1]; ke[2][2] = K[2][2];
    
    // (1,2)
    K = zathat*(-m_eps*alpha);
    ke[0][3] = K[0][0]; ke[0][4] = K[0][1]; ke[0][5] = K[0][2];
    ke[1][3] = K[1][0]; ke[1][4] = K[1][1]; ke[1][5] = K[1][2];
    ke[2][3] = K[2][0]; ke[2][4] = K[2][1]; ke[2][5] = K[2][2];
    
    // (1,3)
    K = I*(-alpha*m_eps);
    ke[0][6] = K[0][0]; ke[0][7] = K[0][1]; ke[0][8] = K[0][2];
    ke[1][6] = K[1][0]; ke[1][7] = K[1][1]; ke[1][8] = K[1][2];
    ke[2][6] = K[2][0]; ke[2][7] = K[2][1]; ke[2][8] = K[2][2];
    
    // (1,4)
    K = zbthat*(alpha*m_eps);
    ke[0][9] = K[0][0]; ke[0][10] = K[0][1]; ke[0][11] = K[0][2];
    ke[1][9] = K[1][0]; ke[1][10] = K[1][1]; ke[1][11] = K[1][2];
    ke[2][9] = K[2][0]; ke[2][10] = K[2][1]; ke[2][11] = K[2][2];
    
    // (2,1)
    K = zahat*(alpha*m_eps);
    ke[3][0] = K[0][0]; ke[3][1] = K[0][1]; ke[3][2] = K[0][2];
    ke[4][0] = K[1][0]; ke[4][1] = K[1][1]; ke[4][2] = K[1][2];
    ke[5][0] = K[2][0]; ke[5][1] = K[2][1]; ke[5][2] = K[2][2];
    
    // (2,2)
    K = (zahat*zathat*m_eps + Wba*m_ups)*(-alpha);
    ke[3][3] = K[0][0]; ke[3][4] = K[0][1]; ke[3][5] = K[0][2];
    ke[4][3] = K[1][0]; ke[4][4] = K[1][1]; ke[4][5] = K[1][2];
    ke[5][3] = K[2][0]; ke[5][4] = K[2][1]; ke[5][5] = K[2][2];
    
    // (2,3)
    K = zahat*(-alpha*m_eps);
    ke[3][6] = K[0][0]; ke[3][7] = K[0][1]; ke[3][8] = K[0][2];
    ke[4][6] = K[1][0]; ke[4][7] = K[1][1]; ke[4][8] = K[1][2];
    ke[5][6] = K[2][0]; ke[5][7] = K[2][1]; ke[5][8] = K[2][2];
    
    // (2,4)
    K = (zahat*zbthat*m_eps + Wab*m_ups)*alpha;
    ke[3][9] = K[0][0]; ke[3][10] = K[0][1]; ke[3][11] = K[0][2];
    ke[4][9] = K[1][0]; ke[4][10] = K[1][1]; ke[4][11] = K[1][2];
    ke[5][9] = K[2][0]; ke[5][10] = K[2][1]; ke[5][11] = K[2][2];
    
    
    // (3,1)
    K = I*(-alpha*m_eps);
    ke[6][0] = K[0][0]; ke[6][1] = K[0][1]; ke[6][2] = K[0][2];
    ke[7][0] = K[1][0]; ke[7][1] = K[1][1]; ke[7][2] = K[1][2];
    ke[8][0] = K[2][0]; ke[8][1] = K[2][1]; ke[8][2] = K[2][2];
    
    // (3,2)
    K = zathat*(m_eps*alpha);
    ke[6][3] = K[0][0]; ke[6][4] = K[0][1]; ke[6][5] = K[0][2];
    ke[7][3] = K[1][0]; ke[7][4] = K[1][1]; ke[7][5] = K[1][2];
    ke[8][3] = K[2][0]; ke[8][4] = K[2][1]; ke[8][5] = K[2][2];
    
    // (3,3)
    K = I*(alpha*m_eps);
    ke[6][6] = K[0][0]; ke[6][7] = K[0][1]; ke[6][8] = K[0][2];
    ke[7][6] = K[1][0]; ke[7][7] = K[1][1]; ke[7][8] = K[1][2];
    ke[8][6] = K[2][0]; ke[8][7] = K[2][1]; ke[8][8] = K[2][2];
    
    // (3,4)
    K = zbthat*(-alpha*m_eps);
    ke[6][9] = K[0][0]; ke[6][10] = K[0][1]; ke[6][11] = K[0][2];
    ke[7][9] = K[1][0]; ke[7][10] = K[1][1]; ke[7][11] = K[1][2];
    ke[8][9] = K[2][0]; ke[8][10] = K[2][1]; ke[8][11] = K[2][2];
    
    
    // (4,1)
    K = zbhat*(-alpha*m_eps);
    ke[9 ][0] = K[0][0]; ke[ 9][1] = K[0][1]; ke[ 9][2] = K[0][2];
    ke[10][0] = K[1][0]; ke[10][1] = K[1][1]; ke[10][2] = K[1][2];
    ke[11][0] = K[2][0]; ke[11][1] = K[2][1]; ke[11][2] = K[2][2];
    
    // (4,2)
    K = (zbhat*zathat*m_eps + Wba*m_ups)*alpha;
    ke[9 ][3] = K[0][0]; ke[ 9][4] = K[0][1]; ke[ 9][5] = K[0][2];
    ke[10][3] = K[1][0]; ke[10][4] = K[1][1]; ke[10][5] = K[1][2];
    ke[11][3] = K[2][0]; ke[11][4] = K[2][1]; ke[11][5] = K[2][2];
    
    // (4,3)
    K = zbhat*(alpha*m_eps);
    ke[9 ][6] = K[0][0]; ke[ 9][7] = K[0][1]; ke[ 9][8] = K[0][2];
    ke[10][6] = K[1][0]; ke[10][7] = K[1][1]; ke[10][8] = K[1][2];
    ke[11][6] = K[2][0]; ke[11][7] = K[2][1]; ke[11][8] = K[2][2];
    
    // (4,4)
    K = (zbhat*zbthat*m_eps + Wab*m_ups)*(-alpha);
    ke[9 ][9] = K[0][0]; ke[ 9][10] = K[0][1]; ke[ 9][11] = K[0][2];
    ke[10][9] = K[1][0]; ke[10][10] = K[1][1]; ke[10][11] = K[1][2];
    ke[11][9] = K[2][0]; ke[11][10] = K[2][1]; ke[11][11] = K[2][2];
    
    for (j=0; j<6; ++j)
    {
        LM[j  ] = RBa.m_LM[j];
        LM[j+6] = RBb.m_LM[j];
    }
    
    psolver->AssembleStiffness(LM, ke);
}

//-----------------------------------------------------------------------------
bool FERigidSphericalJoint::Augment(int naug, const FETimeInfo& tp)
{
    vec3d ra, rb, qa, qb, c, ksi, Lm;
    vec3d za, zb;
    double normF0, normF1;
    vec3d Um;
    double normM0, normM1;
    bool bconv = true;
    
	FERigidBody& RBa = *m_rbA;
	FERigidBody& RBb = *m_rbB;

	double alpha = tp.alpha;
    
    ra = RBa.m_rt*alpha + RBa.m_rp*(1-alpha);
    rb = RBb.m_rt*alpha + RBb.m_rp*(1-alpha);
    
	vec3d zat = m_qa0; RBa.GetRotation().RotateVector(zat);
    vec3d zap = m_qa0; RBa.m_qp.RotateVector(zap);
    za = zat*alpha + zap*(1-alpha);
    
	vec3d zbt = m_qb0; RBb.GetRotation().RotateVector(zbt);
    vec3d zbp = m_qb0; RBb.m_qp.RotateVector(zbp);
    zb = zbt*alpha + zbp*(1-alpha);
    
    c = rb + zb - ra - za;
    
    normF0 = sqrt(m_L*m_L);
    
    // calculate trial multiplier
    Lm = m_L + c*m_eps;
    
    normF1 = sqrt(Lm*Lm);
    
    // check convergence of constraints
    felog.printf(" rigid connector # %d (spherical joint)\n", m_nID+1);
    felog.printf("                  CURRENT        REQUIRED\n");
    double pctn = 0;
    double gap = c.norm();
    double qap = ksi.norm();
    if (fabs(normF1) > 1e-10) pctn = fabs((normF1 - normF0)/normF1);
    if (m_atol) felog.printf("    force : %15le %15le\n", pctn, m_atol);
    else        felog.printf("    force : %15le        ***\n", pctn);
    if (m_gtol) felog.printf("    gap   : %15le %15le\n", gap, m_gtol);
    else        felog.printf("    gap   : %15le        ***\n", gap);
    if (m_atol && (pctn >= m_atol)) bconv = false;
    if (m_gtol && (gap >= m_gtol)) bconv = false;

    if (m_bq) {
		quatd q = (alpha*RBb.GetRotation() + (1 - alpha)*RBb.m_qp)*(alpha*RBa.GetRotation() + (1 - alpha)*RBa.m_qp).Inverse();
        quatd a(vec3d(m_qpx,m_qpy,m_qpz));
        quatd r = a*q.Inverse();
        r.MakeUnit();
        vec3d ksi = r.GetVector()*r.GetAngle();
        normM0 = sqrt(m_U*m_U);
        
        // calculate trial multiplier
        Um = m_U + ksi*m_ups;
        
        normM1 = sqrt(Um*Um);
        
        double qctn = 0;
        if (fabs(normM1) > 1e-10) qctn = fabs((normM1 - normM0)/normM1);
        if (m_atol) felog.printf("    moment: %15le %15le\n", qctn, m_atol);
        else        felog.printf("    moment: %15le        ***\n", qctn);
        if (m_qtol) felog.printf("    angle : %15le %15le\n", qap, m_qtol);
        else        felog.printf("    angle : %15le        ***\n", qap);
        if (m_atol && (qctn >= m_atol)) bconv = false;
        if (m_qtol && (qap >= m_qtol)) bconv = false;
    }
    
    if (naug < m_naugmin ) bconv = false;
    if (naug >= m_naugmax) bconv = true;
    
    if (!bconv)
    {
        // update multipliers
        m_L = Lm;
        m_U = Um;
    }
    
    return bconv;
}

//-----------------------------------------------------------------------------
void FERigidSphericalJoint::Update(int niter, const FETimeInfo& tp)
{
    vec3d ra, rb;
    vec3d za, zb;

	double alpha = tp.alpha;
    
	FERigidBody& RBa = *m_rbA;
	FERigidBody& RBb = *m_rbB;

    ra = RBa.m_rt*alpha + RBa.m_rp*(1-alpha);
    rb = RBb.m_rt*alpha + RBb.m_rp*(1-alpha);
    
	vec3d zat = m_qa0; RBa.GetRotation().RotateVector(zat);
    vec3d zap = m_qa0; RBa.m_qp.RotateVector(zap);
    za = zat*alpha + zap*(1-alpha);
    
	vec3d zbt = m_qb0; RBb.GetRotation().RotateVector(zbt);
    vec3d zbp = m_qb0; RBb.m_qp.RotateVector(zbp);
    zb = zbt*alpha + zbp*(1-alpha);
    
    vec3d c = rb + zb - ra - za;
    m_F = m_L + c*m_eps;
    
    if (m_bq) {
		quatd q = (alpha*RBb.GetRotation() + (1 - alpha)*RBb.m_qp)*(alpha*RBa.GetRotation() + (1 - alpha)*RBa.m_qp).Inverse();
        quatd a(vec3d(m_qpx,m_qpy,m_qpz));
        quatd r = a*q.Inverse();
        r.MakeUnit();
        vec3d ksi = r.GetVector()*r.GetAngle();
        m_M = m_U + ksi*m_ups;
    }
    else m_M = vec3d(m_Mpx,m_Mpy,m_Mpz);
    
}

//-----------------------------------------------------------------------------
void FERigidSphericalJoint::Reset()
{
    m_F = vec3d(0,0,0);
    m_L = vec3d(0,0,0);
    m_M = vec3d(0,0,0);
    m_U = vec3d(0,0,0);
    
	FERigidBody& RBa = *m_rbA;
	FERigidBody& RBb = *m_rbB;

    m_qa0 = m_q0 - RBa.m_r0;
    m_qb0 = m_q0 - RBb.m_r0;
}
