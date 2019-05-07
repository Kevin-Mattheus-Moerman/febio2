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
#include <FECore/FEPlotData.h>

//=============================================================================
//                            N O D E   D A T A
//=============================================================================

//-----------------------------------------------------------------------------
//! Nodal velocities
//!
class FEPlotNodeVelocity : public FENodeData
{
public:
	FEPlotNodeVelocity(FEModel* pfem) : FENodeData(PLT_VEC3F, FMT_NODE){}
	bool Save(FEMesh& m, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Nodal accelerations
//!
class FEPlotNodeAcceleration : public FENodeData
{
public:
	FEPlotNodeAcceleration(FEModel* pfem) : FENodeData(PLT_VEC3F, FMT_NODE){}
	bool Save(FEMesh& m, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Nodal reaction forces
class FEPlotNodeReactionForces : public FENodeData
{
public:
	FEPlotNodeReactionForces(FEModel* pfem) : FENodeData(PLT_VEC3F, FMT_NODE){}
	bool Save(FEMesh& m, FEDataStream& a);
};

//=============================================================================
//                         S U R F A C E   D A T A
//=============================================================================

//-----------------------------------------------------------------------------
//! Contact gap
//!
class FEPlotContactGap : public FESurfaceData
{
public:
    FEPlotContactGap(FEModel* pfem) : FESurfaceData(PLT_FLOAT, FMT_ITEM){}
    bool Save(FESurface& surf, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Vector gap
//!
class FEPlotVectorGap : public FESurfaceData
{
public:
    FEPlotVectorGap(FEModel* pfem) : FESurfaceData(PLT_VEC3F, FMT_ITEM){}
    bool Save(FESurface& surf, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Contact pressure
//!
class FEPlotContactPressure : public FESurfaceData
{
public:
    FEPlotContactPressure(FEModel* pfem) : FESurfaceData(PLT_FLOAT, FMT_ITEM){}
    bool Save(FESurface& surf, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Contact traction
//!
class FEPlotContactTraction : public FESurfaceData
{
public:
    FEPlotContactTraction(FEModel* pfem) : FESurfaceData(PLT_VEC3F, FMT_ITEM){}
    bool Save(FESurface& surf, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Nodal contact gap
//!
class FEPlotNodalContactGap : public FESurfaceData
{
public:
	FEPlotNodalContactGap(FEModel* pfem) : FESurfaceData(PLT_FLOAT, FMT_MULT){}
	bool Save(FESurface& surf, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Nodal vector gap
//!
class FEPlotNodalVectorGap : public FESurfaceData
{
public:
    FEPlotNodalVectorGap(FEModel* pfem) : FESurfaceData(PLT_VEC3F, FMT_MULT){}
    bool Save(FESurface& surf, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Nodal contact pressure
//!
class FEPlotNodalContactPressure : public FESurfaceData
{
public:
    FEPlotNodalContactPressure(FEModel* pfem) : FESurfaceData(PLT_FLOAT, FMT_MULT){}
    bool Save(FESurface& surf, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Nodal contact traction
//!
class FEPlotNodalContactTraction : public FESurfaceData
{
public:
	FEPlotNodalContactTraction(FEModel* pfem) : FESurfaceData(PLT_VEC3F, FMT_MULT){}
	bool Save(FESurface& surf, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Surface traction
//!
class FEPlotSurfaceTraction : public FESurfaceData
{
public:
    FEPlotSurfaceTraction(FEModel* pfem) : FESurfaceData(PLT_VEC3F, FMT_ITEM){}
    bool Save(FESurface& surf, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Nodal surface traction
//!
class FEPlotNodalSurfaceTraction : public FESurfaceData
{
public:
    FEPlotNodalSurfaceTraction(FEModel* pfem) : FESurfaceData(PLT_VEC3F, FMT_MULT){}
    bool Save(FESurface& surf, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Stick status
//!
class FEPlotStickStatus : public FESurfaceData
{
public:
    FEPlotStickStatus(FEModel* pfem) : FESurfaceData(PLT_FLOAT, FMT_ITEM){}
    bool Save(FESurface& surf, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Contact force
//!
class FEPlotContactForce : public FESurfaceData
{
public:
	FEPlotContactForce(FEModel* pfem) : FESurfaceData(PLT_VEC3F, FMT_REGION){}
	bool Save(FESurface& surf, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Contact area
//!
class FEPlotContactArea : public FESurfaceData
{
public:
	FEPlotContactArea(FEModel* pfem) : FESurfaceData(PLT_FLOAT, FMT_MULT){}
	bool Save(FESurface& surf, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Contact penalty parameter
class FEPlotContactPenalty : public FESurfaceData
{
public:
	FEPlotContactPenalty(FEModel* pfem) : FESurfaceData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FESurface& surf, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Mortar gap
class FEPlotMortarContactGap : public FESurfaceData
{
public:
	FEPlotMortarContactGap(FEModel* pfem) : FESurfaceData(PLT_FLOAT, FMT_NODE){}
	bool Save(FESurface& S, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Enclosed volume
//!
class FEPlotEnclosedVolume : public FESurfaceData
{
private:
    FEModel*            m_pfem;
    bool                m_binit;
    vector<FEElement*>  m_elem;
    vector<vec3d>       m_area;
    
public:
    FEPlotEnclosedVolume(FEModel* pfem) : FESurfaceData(PLT_FLOAT, FMT_REGION){ m_pfem = pfem; m_binit = true; }
    bool Save(FESurface& surf, FEDataStream& a);
};

//=============================================================================
//							D O M A I N   D A T A
//=============================================================================
//-----------------------------------------------------------------------------
//! Velocity
class FEPlotElementVelocity : public FEDomainData
{
public:
    FEPlotElementVelocity(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_ITEM){}
    bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Acceleration
class FEPlotElementAcceleration : public FEDomainData
{
public:
    FEPlotElementAcceleration(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_ITEM){}
    bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Element norm for G
class FEPlotElementGnorm : public FEDomainData
{
public:
	FEPlotElementGnorm(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Element stresses
class FEPlotElementStress : public FEDomainData
{
public:
	FEPlotElementStress(FEModel* pfem) : FEDomainData(PLT_MAT3FS, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Element uncoupled pressure
class FEPlotElementUncoupledPressure : public FEDomainData
{
public:
    FEPlotElementUncoupledPressure(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
    bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Element norm for Cauchy stress
class FEPlotElementsnorm : public FEDomainData
{
public:
	FEPlotElementsnorm(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Element norm for Cauchy stress moment
class FEPlotElementtaunorm : public FEDomainData
{
public:
	FEPlotElementtaunorm(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Element norm for PK1 stress
class FEPlotElementPK1norm : public FEDomainData
{
public:
	FEPlotElementPK1norm(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Element norm for PK1 stress moment
class FEPlotElementQK1norm : public FEDomainData
{
public:
	FEPlotElementQK1norm(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Element norm for PK2 stress
class FEPlotElementSnorm : public FEDomainData
{
public:
	FEPlotElementSnorm(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Element norm for PK2 stress moment
class FEPlotElementTnorm : public FEDomainData
{
public:
	FEPlotElementTnorm(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Element infinitesimal strain gradiet norm
class FEPlotElementinfstrnorm : public FEDomainData
{
public:
	FEPlotElementinfstrnorm(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Element Green-Lagrange strain gradient norm
class FEPlotElementGLstrnorm : public FEDomainData
{
public:
	FEPlotElementGLstrnorm(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Element Euler-Almansi strain gradient norm
class FEPlotElementEAstrnorm : public FEDomainData
{
public:
	FEPlotElementEAstrnorm(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Element macro energy
class FEPlotElementMacroEnergy : public FEDomainData
{
public:
	FEPlotElementMacroEnergy(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Element micro energy
class FEPlotElementMicroEnergy : public FEDomainData
{
public:
	FEPlotElementMicroEnergy(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};


//-----------------------------------------------------------------------------
//! Element difference between macro and micro energy
class FEPlotElementenergydiff : public FEDomainData
{
public:
	FEPlotElementenergydiff(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Strain energy density
class FEPlotStrainEnergyDensity : public FEDomainData
{
public:
	FEPlotStrainEnergyDensity(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Deviatoric strain energy density
class FEPlotDevStrainEnergyDensity : public FEDomainData
{
public:
	FEPlotDevStrainEnergyDensity(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Specific strain energy
class FEPlotSpecificStrainEnergy : public FEDomainData
{
public:
	FEPlotSpecificStrainEnergy(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Kinetic energy density
class FEPlotKineticEnergyDensity : public FEDomainData
{
public:
    FEPlotKineticEnergyDensity(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
    bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Mass density
class FEPlotDensity : public FEDomainData
{
public:
	FEPlotDensity(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Strain energy
class FEPlotElementStrainEnergy : public FEDomainData
{
public:
    FEPlotElementStrainEnergy(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
    bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Kinetic energy
class FEPlotElementKineticEnergy : public FEDomainData
{
public:
    FEPlotElementKineticEnergy(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
    bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Center of mass
class FEPlotElementCenterOfMass : public FEDomainData
{
public:
    FEPlotElementCenterOfMass(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_ITEM){}
    bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Linear momentum
class FEPlotElementLinearMomentum : public FEDomainData
{
public:
    FEPlotElementLinearMomentum(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_ITEM){}
    bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Angular momentum
class FEPlotElementAngularMomentum : public FEDomainData
{
public:
    FEPlotElementAngularMomentum(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_ITEM){}
    bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Stress power
class FEPlotElementStressPower : public FEDomainData
{
public:
    FEPlotElementStressPower(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
    bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Strain energy at current time
class FEPlotCurrentElementStrainEnergy : public FEDomainData
{
public:
    FEPlotCurrentElementStrainEnergy(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
    bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Kinetic energy at current time
class FEPlotCurrentElementKineticEnergy : public FEDomainData
{
public:
    FEPlotCurrentElementKineticEnergy(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
    bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Center of mass at current time
class FEPlotCurrentElementCenterOfMass : public FEDomainData
{
public:
    FEPlotCurrentElementCenterOfMass(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_ITEM){}
    bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Linear momentum at current time
class FEPlotCurrentElementLinearMomentum : public FEDomainData
{
public:
    FEPlotCurrentElementLinearMomentum(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_ITEM){}
    bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Angular momentum at current time
class FEPlotCurrentElementAngularMomentum : public FEDomainData
{
public:
    FEPlotCurrentElementAngularMomentum(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_ITEM){}
    bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Relative volume
class FEPlotRelativeVolume : public FEDomainData
{
public:
	FEPlotRelativeVolume(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Material fibers
class FEPlotFiberVector : public FEDomainData
{
public:
	FEPlotFiberVector(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Material axes
class FEPlotMaterialAxes : public FEDomainData
{
public:
	FEPlotMaterialAxes(FEModel* pfem) : FEDomainData(PLT_MAT3F, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! fiber stretch
class FEPlotFiberStretch : public FEDomainData
{
public:
	FEPlotFiberStretch(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! deviatoric fiber stretch
class FEPlotDevFiberStretch : public FEDomainData
{
public:
	FEPlotDevFiberStretch(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Shell thicknesses
class FEPlotShellThickness : public FEDomainData
{
public:
	FEPlotShellThickness(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_MULT){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Shell directors
class FEPlotShellDirector : public FEDomainData
{
public:
	FEPlotShellDirector(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_MULT){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Element elasticity tensor
class FEPlotElementElasticity : public FEDomainData
{
public:
	FEPlotElementElasticity(FEModel* pfem) : FEDomainData(PLT_TENS4FS, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Damage reduction factor
class FEPlotDamage : public FEDomainData
{
public:
	FEPlotDamage(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& m, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Damage reduction factor
class FEPlotNestedDamage : public FEDomainData
{
public:
    FEPlotNestedDamage(FEModel* pfem);
    bool Save(FEDomain& m, FEDataStream& a);
    bool SetFilter(int nsol);
protected:
    int			m_nmat;
    FEModel*	m_pfem;
};

//-----------------------------------------------------------------------------
//! Intact bond fraction (fatigue)
class FEPlotIntactBondFraction : public FEDomainData
{
public:
    FEPlotIntactBondFraction(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
    bool Save(FEDomain& m, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Fatigued bond fraction (fatigue)
class FEPlotFatigueBondFraction : public FEDomainData
{
public:
    FEPlotFatigueBondFraction(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
    bool Save(FEDomain& m, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Octahedral Plastic Strain
class FEPlotOctahedralPlasticStrain : public FEDomainData
{
public:
    FEPlotOctahedralPlasticStrain(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
    bool Save(FEDomain& m, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Mixture volume fraction
class FEPlotMixtureVolumeFraction : public FEDomainData
{
public:
	FEPlotMixtureVolumeFraction(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
	bool Save(FEDomain& m, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Class that outputs the element nodal stresses for UT4 domains
class FEPlotUT4NodalStresses : public FEDomainData
{
public:
	FEPlotUT4NodalStresses(FEModel* pfem) : FEDomainData(PLT_MAT3FS, FMT_NODE) {}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Store shell strains
class FEPlotShellStrain : public FEDomainData
{
public:
	FEPlotShellStrain(FEModel* pfem) : FEDomainData(PLT_MAT3FS, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Shell relative volume
class FEPlotShellRelativeVolume : public FEDomainData
{
public:
    FEPlotShellRelativeVolume(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_ITEM){}
    bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! class the projects stresses from integration points to nodes using
//! SPR (superconvergergent patch recovery)
class FEPlotSPRStresses : public FEDomainData
{
public:
	FEPlotSPRStresses(FEModel* pfem) : FEDomainData(PLT_MAT3FS, FMT_NODE){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! class the projects stresses from integration points to nodes using
//! SPR (superconvergergent patch recovery)
class FEPlotSPRLinearStresses : public FEDomainData
{
public:
	FEPlotSPRLinearStresses(FEModel* pfem) : FEDomainData(PLT_MAT3FS, FMT_NODE){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! class that projects the principal stresses from integration points to nodes
//! using the SPR projection method
class FEPlotSPRPrincStresses : public FEDomainData
{
public:
	FEPlotSPRPrincStresses(FEModel* pfem) : FEDomainData(PLT_MAT3FD, FMT_NODE){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! class the projects stresses from integration points to nodes using
//! SPR (superconvergergent patch recovery)
class FEPlotSPRTestLinear: public FEDomainData
{
public:
	FEPlotSPRTestLinear(FEModel* pfem) : FEDomainData(PLT_MAT3FD, FMT_NODE){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! class the projects stresses from integration points to nodes using
//! SPR (superconvergergent patch recovery)
class FEPlotSPRTestQuadratic: public FEDomainData
{
public:
	FEPlotSPRTestQuadratic(FEModel* pfem) : FEDomainData(PLT_MAT3FS, FMT_NODE){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Rigid body displacement
class FEPlotRigidDisplacement : public FEDomainData
{
public:
	FEPlotRigidDisplacement(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_REGION),  m_pfem(pfem) {}
	bool Save(FEDomain& dom, FEDataStream& a);
private:
	FEModel* m_pfem;
};

//-----------------------------------------------------------------------------
//! Rigid body velocity
class FEPlotRigidVelocity : public FEDomainData
{
public:
	FEPlotRigidVelocity(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_REGION),  m_pfem(pfem) {}
	bool Save(FEDomain& dom, FEDataStream& a);
private:
	FEModel* m_pfem;
};

//-----------------------------------------------------------------------------
//! Rigid body acceleration
class FEPlotRigidAcceleration : public FEDomainData
{
public:
	FEPlotRigidAcceleration(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_REGION),  m_pfem(pfem) {}
	bool Save(FEDomain& dom, FEDataStream& a);
private:
	FEModel* m_pfem;
};

//-----------------------------------------------------------------------------
//! Rigid body rotation
class FEPlotRigidRotation : public FEDomainData
{
public:
	FEPlotRigidRotation(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_REGION),  m_pfem(pfem) {}
	bool Save(FEDomain& dom, FEDataStream& a);
private:
	FEModel* m_pfem;
};

//-----------------------------------------------------------------------------
//! Rigid body angular velocity
class FEPlotRigidAngularVelocity : public FEDomainData
{
public:
	FEPlotRigidAngularVelocity(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_REGION),  m_pfem(pfem) {}
	bool Save(FEDomain& dom, FEDataStream& a);
private:
	FEModel* m_pfem;
};

//-----------------------------------------------------------------------------
//! Rigid body angular acceleration
class FEPlotRigidAngularAcceleration : public FEDomainData
{
public:
	FEPlotRigidAngularAcceleration(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_REGION),  m_pfem(pfem) {}
	bool Save(FEDomain& dom, FEDataStream& a);
private:
	FEModel* m_pfem;
};

//-----------------------------------------------------------------------------
//! Rigid body kinetic energy
class FEPlotRigidKineticEnergy : public FEDomainData
{
public:
	FEPlotRigidKineticEnergy(FEModel* pfem) : FEDomainData(PLT_FLOAT, FMT_REGION),  m_pfem(pfem) {}
	bool Save(FEDomain& dom, FEDataStream& a);
private:
	FEModel* m_pfem;
};

//-----------------------------------------------------------------------------
//! Rigid body linear momentum
class FEPlotRigidLinearMomentum : public FEDomainData
{
public:
    FEPlotRigidLinearMomentum(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_REGION),  m_pfem(pfem) {}
    bool Save(FEDomain& dom, FEDataStream& a);
private:
    FEModel* m_pfem;
};

//-----------------------------------------------------------------------------
//! Rigid body angular momentum
class FEPlotRigidAngularMomentum : public FEDomainData
{
public:
    FEPlotRigidAngularMomentum(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_REGION),  m_pfem(pfem) {}
    bool Save(FEDomain& dom, FEDataStream& a);
private:
    FEModel* m_pfem;
};

//-----------------------------------------------------------------------------
//! Rigid Euler angles
class FEPlotRigidEuler : public FEDomainData
{
public:
	FEPlotRigidEuler(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_REGION), m_pfem(pfem) {}
	bool Save(FEDomain& dom, FEDataStream& a);
private:
	FEModel* m_pfem;
};

//-----------------------------------------------------------------------------
//! Rotation vector
class FEPlotRigidRotationVector : public FEDomainData
{
public:
	FEPlotRigidRotationVector(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_REGION), m_pfem(pfem) {}
	bool Save(FEDomain& dom, FEDataStream& a);
private:
	FEModel* m_pfem;
};

//-----------------------------------------------------------------------------
//! Class that projects stresses from integration points to the nodes
//! TODO: This only works with tet10 and hex8 -domains
class FEPlotNodalStresses : public FEDomainData
{
public:
	FEPlotNodalStresses(FEModel* pfem) : FEDomainData(PLT_MAT3FS, FMT_MULT){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Lagrange strains
class FEPlotLagrangeStrain : public FEDomainData
{
public:
	FEPlotLagrangeStrain(FEModel* pfem) : FEDomainData(PLT_MAT3FS, FMT_ITEM){}
	bool Save(FEDomain& dom, FEDataStream& a);
};

//-----------------------------------------------------------------------------
//! Lagrange strains
class FEPlotSPRLagrangeStrain : public FEDomainData
{
public:
	FEPlotSPRLagrangeStrain(FEModel* pfem) : FEDomainData(PLT_MAT3FS, FMT_NODE){}
	bool Save(FEDomain& dom, FEDataStream& a);
};


//-----------------------------------------------------------------------------
//! Rigid body reaction force
class FEPlotRigidReactionForce : public FEDomainData
{
public:
	FEPlotRigidReactionForce(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_REGION),  m_pfem(pfem) {}
	bool Save(FEDomain& dom, FEDataStream& a);
private:
	FEModel* m_pfem;
};

//-----------------------------------------------------------------------------
//! Rigid body reaction torque
class FEPlotRigidReactionTorque : public FEDomainData
{
public:
    FEPlotRigidReactionTorque(FEModel* pfem) : FEDomainData(PLT_VEC3F, FMT_REGION), m_pfem(pfem) {}
    bool Save(FEDomain& dom, FEDataStream& a);
private:
	FEModel* m_pfem;
};
