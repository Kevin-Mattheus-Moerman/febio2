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

#include "FEBioMech.h"
#include "FE2DFiberNeoHookean.h"
#include "FE2DTransIsoMooneyRivlin.h"
#include "FE2DTransIsoVerondaWestmann.h"
#include "FEArrudaBoyce.h"
#include "FECarterHayesOld.h"
#include "FECellGrowth.h"
#include "FECubicCLE.h"
#include "FEDamageMooneyRivlin.h"
#include "FEDamageNeoHookean.h"
#include "FEDamageTransIsoMooneyRivlin.h"
#include "FESpringMaterial.h"
#include "FEDonnanEquilibrium.h"
#include "FEEFDDonnanEquilibrium.h"
#include "FEEFDMooneyRivlin.h"
#include "FEEFDNeoHookean.h"
#include "FEEFDUncoupled.h"
#include "FEEFDVerondaWestmann.h"
#include "FEElasticMixture.h"
#include "FEElasticMultigeneration.h"
#include "FEEllipsoidalFiberDistribution.h"
#include "FEFatigueMaterial.h"
#include "FEReactiveFatigue.h"
#include "FEReactivePlasticity.h"
#include "FEFiberExpPow.h"
#include "FEFiberExpPowUncoupled.h"
#include "FEFiberNeoHookean.h"
#include "FEFiberPowLinear.h"
#include "FEFiberPowLinearUncoupled.h"
#include "FEFiberEFDNeoHookean.h"
#include "FEFungOrthoCompressible.h"
#include "FEFungOrthotropic.h"
#include "FEGasserOgdenHolzapfel.h"
#include "FEGasserOgdenHolzapfelUC.h"
#include "FEHolmesMow.h"
#include "FEHuiskesSupply.h"
#include "FEIncompNeoHookean.h"
#include "FEIsotropicElastic.h"
#include "FELinearElastic.h"
#include "FELinearOrthotropic.h"
#include "FELinearTransIso.h"
#include "FEMooneyRivlin.h"
#include "FEMRVonMisesFibers.h"
#include "FEMuscleMaterial.h"
#include "FENeoHookean.h"
#include "FENeoHookeanTransIso.h"
#include "FENewtonianViscousSolid.h"
#include "FENewtonianViscousSolidUC.h"
#include "FEOgdenMaterial.h"
#include "FEOgdenUnconstrained.h"
#include "FEOrthoElastic.h"
#include "FEOrthotropicCLE.h"
#include "FEOsmoticVirialExpansion.h"
#include "FEPerfectOsmometer.h"
#include "FEPorousNeoHookean.h"
#include "FERemodelingElasticMaterial.h"
#include "FERigidMaterial.h"
#include "FESphericalFiberDistribution.h"
#include "FEStVenantKirchhoff.h"
#include "FETCNonlinearOrthotropic.h"
#include "FETendonMaterial.h"
#include "FETransIsoMooneyRivlin.h"
#include "FETransIsoVerondaWestmann.h"
#include "FETrussMaterial.h"
#include "FEUncoupledActiveContraction.h"
#include "FEUncoupledElasticMixture.h"
#include "FEUncoupledViscoElasticMaterial.h"
#include "FEVerondaWestmann.h"
#include "FEViscoElasticMaterial.h"
#include "FEVonMisesPlasticity.h"
#include "FEElasticFiberMaterial.h"
#include "FEElasticFiberMaterialUC.h"
#include "FEFiberDensityDistribution.h"
#include "FEContinuousFiberDistribution.h"
#include "FEContinuousFiberDistributionUC.h"
#include "FEFiberIntegrationGauss.h"
#include "FEFiberIntegrationTrapezoidal.h"
#include "FEFiberIntegrationGeodesic.h"
#include "FEFiberIntegrationGaussKronrod.h"
#include "FEFiberIntegrationTriangle.h"
#include "FECoupledTransIsoMooneyRivlin.h"
#include "FECoupledTransIsoVerondaWestmann.h"
#include "FEMicroMaterial.h"
#include "FEMicroMaterial2O.h"
#include "FESpringMaterial.h"
#include "FEElasticMultigeneration.h"
#include "FEPRLig.h"
#include "FECoupledMooneyRivlin.h"
#include "FECoupledVerondaWestmann.h"
#include "FEReactiveViscoelastic.h"
#include "FEUncoupledReactiveViscoelastic.h"
#include "FEBondRelaxation.h"
#include "FEDamageMaterial.h"
#include "FEDamageMaterialUC.h"
#include "FEDamageCDF.h"
#include "FEDamageCriterion.h"
#include "FEFiberExpLinear.h"
#include "FEUncoupledFiberExpLinear.h"
#include "FEPrescribedActiveContractionUniaxial.h"
#include "FEPrescribedActiveContractionUniaxialUC.h"
#include "FEPrescribedActiveContractionTransIso.h"
#include "FEPrescribedActiveContractionTransIsoUC.h"
#include "FEPrescribedActiveContractionIsotropic.h"
#include "FEPrescribedActiveContractionIsotropicUC.h"
#include "FEGentMaterial.h"
#include "FEWrinkleOgdenMaterial.h"
#include "FEMindlinElastic2O.h"

#include "FEPressureLoad.h"
#include "FETractionLoad.h"
#include "FEConstBodyForce.h"
#include "FENonConstBodyForce.h"
#include "FECentrifugalBodyForce.h"
#include "FEPointBodyForce.h"

#include "FEFacet2FacetSliding.h"
#include "FEPeriodicBoundary.h"
#include "FEPeriodicBoundary2O.h"
#include "FERigidWallInterface.h"
#include "FERigidSlidingContact.h"
#include "FESlidingInterface.h"
#include "FESlidingInterfaceBW.h"
#include "FEPeriodicSurfaceConstraint.h"
#include "FETiedInterface.h"
#include "FETiedElasticInterface.h"
#include "FEStickyInterface.h"
#include "FEPointConstraint.h"
#include "FEFacet2FacetTied.h"
#include "FEVolumeConstraint.h"
#include "FEDistanceConstraint.h"
#include "FE2OMicroConstraint.h"
#include "FEMortarSlidingContact.h"
#include "FEMortarTiedContact.h"

#include "FEAugLagLinearConstraint.h"
#include "FESymmetryPlane.h"
#include "FERigidJoint.h"
#include "FERigidSphericalJoint.h"
#include "FERigidRevoluteJoint.h"
#include "FERigidPrismaticJoint.h"
#include "FERigidCylindricalJoint.h"
#include "FERigidPlanarJoint.h"
#include "FERigidLock.h"
#include "FERigidSpring.h"
#include "FERigidDamper.h"
#include "FERigidAngularDamper.h"
#include "FERigidContractileForce.h"
#include "FERigidForce.h"
#include "FERigidCable.h"
#include "FEDiscreteContact.h"

#include "FESolidSolver.h"
#include "FESolidSolver2.h"
#include "FELinearSolidSolver.h"
#include "FEExplicitSolidSolver.h"
#include "FECGSolidSolver.h"

#include "FEBioMechPlot.h"
#include "FEBioMechData.h"

#include "FESolidDomainFactory.h"
#include "FEElasticSolidDomain.h"
#include "FEElasticShellDomain.h"
#include "FEElasticShellDomainOld.h"
#include "FEElasticEASShellDomain.h"
#include "FEElasticANSShellDomain.h"
#include "FEElasticTrussDomain.h"
#include "FERigidSolidDomain.h"
#include "FERigidShellDomain.h"
#include "FERemodelingElasticDomain.h"
#include "FEUDGHexDomain.h"
#include "FEUT4Domain.h"
#include "FEElasticMultiscaleDomain1O.h"
#include "FEElasticMultiscaleDomain2O.h"
#include "FESRIElasticSolidDomain.h"
#include "FE3FieldElasticSolidDomain.h"
#include "FE3FieldElasticShellDomain.h"
#include "FEDiscreteSpringDomain.h"
#include "FEDeformableSpringDomain.h"
#include <FECore/RigidBC.h>
#include "FEBCPrescribedDeformation.h"
#include "FEPrescribedNormalDisplacement.h"

//-----------------------------------------------------------------------------
//! Register all the classes of the FEBioMech module with the FEBio framework.
void FEBioMech::InitModule()
{
//-----------------------------------------------------------------------------
// Domain factory
	FECoreKernel& febio = FECoreKernel::GetInstance();
	febio.RegisterDomain(new FESolidDomainFactory);

//-----------------------------------------------------------------------------
// Solver classes
REGISTER_FECORE_CLASS(FESolidSolver        , FESOLVER_ID, "solid_old"     );
REGISTER_FECORE_CLASS(FESolidSolver2       , FESOLVER_ID, "solid"         );
REGISTER_FECORE_CLASS(FEExplicitSolidSolver, FESOLVER_ID, "explicit-solid");
REGISTER_FECORE_CLASS(FELinearSolidSolver  , FESOLVER_ID, "linear-solid"  );
REGISTER_FECORE_CLASS(FECGSolidSolver      , FESOLVER_ID, "CG-solid"      );

//-----------------------------------------------------------------------------
// material classes
REGISTER_FECORE_CLASS(FE2DFiberNeoHookean            ,FEMATERIAL_ID, "2D fiber neo-Hookean"          );
REGISTER_FECORE_CLASS(FE2DTransIsoMooneyRivlin       ,FEMATERIAL_ID, "2D trans iso Mooney-Rivlin"    );
REGISTER_FECORE_CLASS(FE2DTransIsoVerondaWestmann    ,FEMATERIAL_ID, "2D trans iso Veronda-Westmann" );
REGISTER_FECORE_CLASS(FEArrudaBoyce                  ,FEMATERIAL_ID, "Arruda-Boyce"                  );
REGISTER_FECORE_CLASS(FECellGrowth                   ,FEMATERIAL_ID, "cell growth"                   );
REGISTER_FECORE_CLASS(FECubicCLE                     ,FEMATERIAL_ID, "cubic CLE"                     );
REGISTER_FECORE_CLASS(FEDamageMooneyRivlin           ,FEMATERIAL_ID, "damage Mooney-Rivlin"          );
REGISTER_FECORE_CLASS(FEDamageNeoHookean             ,FEMATERIAL_ID, "damage neo-Hookean"            );
REGISTER_FECORE_CLASS(FEDamageTransIsoMooneyRivlin   ,FEMATERIAL_ID, "damage trans iso Mooney-Rivlin");
REGISTER_FECORE_CLASS(FEDonnanEquilibrium            ,FEMATERIAL_ID, "Donnan equilibrium"            );
REGISTER_FECORE_CLASS(FEEFDDonnanEquilibrium         ,FEMATERIAL_ID, "EFD Donnan equilibrium"        );
REGISTER_FECORE_CLASS(FEEFDMooneyRivlin              ,FEMATERIAL_ID, "EFD Mooney-Rivlin"             );
REGISTER_FECORE_CLASS(FEEFDNeoHookean                ,FEMATERIAL_ID, "EFD neo-Hookean (new)"         );
REGISTER_FECORE_CLASS(FEEFDNeoHookeanOld             ,FEMATERIAL_ID, "EFD neo-Hookean"               );
REGISTER_FECORE_CLASS(FEEFDUncoupled                 ,FEMATERIAL_ID, "EFD uncoupled"                 );
REGISTER_FECORE_CLASS(FEEFDVerondaWestmann           ,FEMATERIAL_ID, "EFD Veronda-Westmann"          );
REGISTER_FECORE_CLASS(FEElasticMixture               ,FEMATERIAL_ID, "solid mixture"                 );
REGISTER_FECORE_CLASS(FEEllipsoidalFiberDistribution ,FEMATERIAL_ID, "ellipsoidal fiber distribution");
REGISTER_FECORE_CLASS(FEEllipsoidalFiberDistributionOld,FEMATERIAL_ID, "ellipsoidal fiber distribution (old)");
REGISTER_FECORE_CLASS(FEFiberExpPow                  ,FEMATERIAL_ID, "fiber-exp-pow"                 );
REGISTER_FECORE_CLASS(FEFiberExpPowUncoupled         ,FEMATERIAL_ID, "fiber-exp-pow-uncoupled"       );
REGISTER_FECORE_CLASS(FEFiberEFDNeoHookean           ,FEMATERIAL_ID, "fiber neo-Hookean"             );
REGISTER_FECORE_CLASS(FEFiberPowLinear               ,FEMATERIAL_ID, "fiber-pow-linear"              );
REGISTER_FECORE_CLASS(FEFiberPowLinearUncoupled      ,FEMATERIAL_ID, "fiber-pow-linear-uncoupled"    );
REGISTER_FECORE_CLASS(FEFungOrthoCompressible        ,FEMATERIAL_ID, "Fung-ortho-compressible"       );
REGISTER_FECORE_CLASS(FEFungOrthotropic              ,FEMATERIAL_ID, "Fung orthotropic"              );
REGISTER_FECORE_CLASS(FEGasserOgdenHolzapfel         ,FEMATERIAL_ID, "Gasser-Ogden-Holzapfel"        );
REGISTER_FECORE_CLASS(FEGasserOgdenHolzapfelUC       ,FEMATERIAL_ID, "Gasser-Ogden-Holzapfel-uncoupled");
REGISTER_FECORE_CLASS(FEGentMaterial                 ,FEMATERIAL_ID, "Gent"                          );
REGISTER_FECORE_CLASS(FECompressibleGentMaterial     ,FEMATERIAL_ID, "compressible Gent"             );
REGISTER_FECORE_CLASS(FEHolmesMow                    ,FEMATERIAL_ID, "Holmes-Mow"                    );
REGISTER_FECORE_CLASS(FEIncompNeoHookean             ,FEMATERIAL_ID, "incomp neo-Hookean"            );
REGISTER_FECORE_CLASS(FEIsotropicElastic             ,FEMATERIAL_ID, "isotropic elastic"             );
REGISTER_FECORE_CLASS(FELinearElastic                ,FEMATERIAL_ID, "linear elastic"                );
REGISTER_FECORE_CLASS(FELinearOrthotropic            ,FEMATERIAL_ID, "linear orthotropic"            );
REGISTER_FECORE_CLASS(FELinearTransIso               ,FEMATERIAL_ID, "linear trans iso"              );
REGISTER_FECORE_CLASS(FEMooneyRivlin                 ,FEMATERIAL_ID, "Mooney-Rivlin"                 );
REGISTER_FECORE_CLASS(FECoupledMooneyRivlin          ,FEMATERIAL_ID, "coupled Mooney-Rivlin"         );
REGISTER_FECORE_CLASS(FECoupledVerondaWestmann       ,FEMATERIAL_ID, "coupled Veronda-Westmann"      );
REGISTER_FECORE_CLASS(FEMuscleMaterial               ,FEMATERIAL_ID, "muscle material"               );
REGISTER_FECORE_CLASS(FENeoHookean                   ,FEMATERIAL_ID, "neo-Hookean"                   );
REGISTER_FECORE_CLASS(FENeoHookeanTransIso           ,FEMATERIAL_ID, "neo-Hookean transiso"          );
REGISTER_FECORE_CLASS(FENewtonianViscousSolid        ,FEMATERIAL_ID, "Newtonian viscous solid"       );
REGISTER_FECORE_CLASS(FENewtonianViscousSolidUC      ,FEMATERIAL_ID, "Newtonian viscous solid uncoupled");
REGISTER_FECORE_CLASS(FEOgdenMaterial                ,FEMATERIAL_ID, "Ogden"                         );
REGISTER_FECORE_CLASS(FEOgdenUnconstrained           ,FEMATERIAL_ID, "Ogden unconstrained"           );
REGISTER_FECORE_CLASS(FEOrthoElastic                 ,FEMATERIAL_ID, "orthotropic elastic"           );
REGISTER_FECORE_CLASS(FEOrthotropicCLE               ,FEMATERIAL_ID, "orthotropic CLE"               );
REGISTER_FECORE_CLASS(FEPerfectOsmometer             ,FEMATERIAL_ID, "perfect osmometer"             );
REGISTER_FECORE_CLASS(FEPorousNeoHookean             ,FEMATERIAL_ID, "porous neo-Hookean"            );
REGISTER_FECORE_CLASS(FEOsmoticVirialExpansion       ,FEMATERIAL_ID, "osmotic virial expansion"      );
REGISTER_FECORE_CLASS(FERigidMaterial                ,FEMATERIAL_ID, "rigid body"                    );
REGISTER_FECORE_CLASS(FESphericalFiberDistribution   ,FEMATERIAL_ID, "spherical fiber distribution"  );
REGISTER_FECORE_CLASS(FEStVenantKirchhoff            ,FEMATERIAL_ID, "St.Venant-Kirchhoff"           );
REGISTER_FECORE_CLASS(FETCNonlinearOrthotropic       ,FEMATERIAL_ID, "TC nonlinear orthotropic"      );
REGISTER_FECORE_CLASS(FETendonMaterial               ,FEMATERIAL_ID, "tendon material"               );
REGISTER_FECORE_CLASS(FETransIsoMooneyRivlin         ,FEMATERIAL_ID, "trans iso Mooney-Rivlin"       );
REGISTER_FECORE_CLASS(FETransIsoVerondaWestmann      ,FEMATERIAL_ID, "trans iso Veronda-Westmann"    );
REGISTER_FECORE_CLASS(FETrussMaterial                ,FEMATERIAL_ID, "linear truss"                  );
REGISTER_FECORE_CLASS(FEUncoupledElasticMixture      ,FEMATERIAL_ID, "uncoupled solid mixture"       );
REGISTER_FECORE_CLASS(FEVerondaWestmann              ,FEMATERIAL_ID, "Veronda-Westmann"              );
REGISTER_FECORE_CLASS(FEViscoElasticMaterial         ,FEMATERIAL_ID, "viscoelastic"                  );
REGISTER_FECORE_CLASS(FEUncoupledViscoElasticMaterial,FEMATERIAL_ID, "uncoupled viscoelastic"        );
REGISTER_FECORE_CLASS(FEVonMisesPlasticity           ,FEMATERIAL_ID, "von-Mises plasticity"          );
REGISTER_FECORE_CLASS(FEElasticMultigeneration       ,FEMATERIAL_ID, "multigeneration"               );
REGISTER_FECORE_CLASS(FEMRVonMisesFibers             ,FEMATERIAL_ID, "Mooney-Rivlin von Mises Fibers");
REGISTER_FECORE_CLASS(FEUncoupledActiveContraction   ,FEMATERIAL_ID, "uncoupled active contraction"  );
REGISTER_FECORE_CLASS(FEHuiskesSupply                ,FEMATERIAL_ID, "Huiskes-supply"                );
REGISTER_FECORE_CLASS(FERemodelingElasticMaterial    ,FEMATERIAL_ID, "remodeling solid"              );
REGISTER_FECORE_CLASS(FECarterHayesOld               ,FEMATERIAL_ID, "Carter-Hayes (old)"            );
REGISTER_FECORE_CLASS(FEActiveFiberContraction       ,FEMATERIAL_ID, "active_contraction"            );
REGISTER_FECORE_CLASS(FEFiberExponentialPower        ,FEMATERIAL_ID, "fiber-exponential-power-law"   );
REGISTER_FECORE_CLASS(FEFiberExponentialPowerUC      ,FEMATERIAL_ID, "fiber-exponential-power-law-uncoupled");
REGISTER_FECORE_CLASS(FEFiberNH                      ,FEMATERIAL_ID, "fiber-NH"                      );
REGISTER_FECORE_CLASS(FEFiberNHUC                    ,FEMATERIAL_ID, "fiber-NH-uncoupled"            );
REGISTER_FECORE_CLASS(FEFiberPowerLinear             ,FEMATERIAL_ID, "fiber-power-linear"            );
REGISTER_FECORE_CLASS(FESphericalFiberDensityDistribution  , FEMATERIAL_ID, "spherical"   );
REGISTER_FECORE_CLASS(FEEllipsodialFiberDensityDistribution, FEMATERIAL_ID, "ellipsoidal" );
REGISTER_FECORE_CLASS(FEVonMises3DFiberDensityDistribution , FEMATERIAL_ID, "von-Mises-3d");
REGISTER_FECORE_CLASS(FEVonMises3DTwoFDDAxisymmetric       , FEMATERIAL_ID, "von-Mises-3d-two-axisym");
REGISTER_FECORE_CLASS(FECircularFiberDensityDistribution   , FEMATERIAL_ID, "circular"    );
REGISTER_FECORE_CLASS(FEEllipticalFiberDensityDistribution , FEMATERIAL_ID, "elliptical"  );
REGISTER_FECORE_CLASS(FEVonMises2DFiberDensityDistribution , FEMATERIAL_ID, "von-Mises-2d");
REGISTER_FECORE_CLASS(FEContinuousFiberDistribution        , FEMATERIAL_ID, "continuous fiber distribution");
REGISTER_FECORE_CLASS(FEContinuousFiberDistributionUC      , FEMATERIAL_ID, "continuous fiber distribution uncoupled");
REGISTER_FECORE_CLASS(FEFiberIntegrationGauss              , FEMATERIAL_ID, "fibers-3d-gauss");
//REGISTER_FECORE_CLASS(FEFiberIntegrationGaussUC            , FEMATERIAL_ID, "fibers-3d-gauss-uncoupled");
REGISTER_FECORE_CLASS(FEFiberIntegrationGeodesic           , FEMATERIAL_ID, "fibers-3d-geodesic");
//REGISTER_FECORE_CLASS(FEFiberIntegrationGeodesicUC         , FEMATERIAL_ID, "fibers-3d-geodesic-uncoupled");
REGISTER_FECORE_CLASS(FEFiberIntegrationGaussKronrod       , FEMATERIAL_ID, "fibers-3d-gkt");
//REGISTER_FECORE_CLASS(FEFiberIntegrationGaussKronrodUC     , FEMATERIAL_ID, "fibers-3d-gkt-uncoupled");
REGISTER_FECORE_CLASS(FEFiberIntegrationTriangle           , FEMATERIAL_ID, "fibers-3d-fei");
//REGISTER_FECORE_CLASS(FEFiberIntegrationTriangleUC         , FEMATERIAL_ID, "fibers-3d-fei-uncoupled");
REGISTER_FECORE_CLASS(FEFiberIntegrationTrapezoidal        , FEMATERIAL_ID, "fibers-2d-trapezoidal");
//REGISTER_FECORE_CLASS(FEFiberIntegrationTrapezoidalUC      , FEMATERIAL_ID, "fibers-2d-trapezoidal-uncoupled");
REGISTER_FECORE_CLASS(FECoupledTransIsoVerondaWestmann     , FEMATERIAL_ID, "coupled trans-iso Veronda-Westmann");
REGISTER_FECORE_CLASS(FECoupledTransIsoMooneyRivlin        , FEMATERIAL_ID, "coupled trans-iso Mooney-Rivlin");
REGISTER_FECORE_CLASS(FEMicroMaterial                      , FEMATERIAL_ID, "micro-material");
REGISTER_FECORE_CLASS(FEMicroMaterial2O                    , FEMATERIAL_ID, "micro-material2O");
REGISTER_FECORE_CLASS(FEMicroProbe                         , FEMATERIAL_ID, "probe");
REGISTER_FECORE_CLASS(FEMindlinElastic2O                   , FEMATERIAL_ID, "mindlin elastic");
REGISTER_FECORE_CLASS(FEGenerationMaterial                 , FEMATERIAL_ID, "generation");
REGISTER_FECORE_CLASS(FEPRLig					           , FEMATERIAL_ID, "PRLig");
REGISTER_FECORE_CLASS(FEFiberExpLinear					   , FEMATERIAL_ID, "fiber-exp-linear");
REGISTER_FECORE_CLASS(FEUncoupledFiberExpLinear            , FEMATERIAL_ID, "uncoupled fiber-exp-linear");
REGISTER_FECORE_CLASS(FEPrescribedActiveContractionUniaxial   , FEMATERIAL_ID, "prescribed uniaxial active contraction");
REGISTER_FECORE_CLASS(FEPrescribedActiveContractionUniaxialUC , FEMATERIAL_ID, "uncoupled prescribed uniaxial active contraction");
REGISTER_FECORE_CLASS(FEPrescribedActiveContractionTransIso   , FEMATERIAL_ID, "prescribed trans iso active contraction");
REGISTER_FECORE_CLASS(FEPrescribedActiveContractionTransIsoUC , FEMATERIAL_ID, "uncoupled prescribed trans iso active contraction");
REGISTER_FECORE_CLASS(FEPrescribedActiveContractionIsotropic  , FEMATERIAL_ID, "prescribed isotropic active contraction");
REGISTER_FECORE_CLASS(FEPrescribedActiveContractionIsotropicUC, FEMATERIAL_ID, "uncoupled prescribed isotropic active contraction");
REGISTER_FECORE_CLASS(FEWrinkleOgdenMaterial                  , FEMATERIAL_ID, "wrinkle Ogden");
REGISTER_FECORE_CLASS(FEElasticMembrane                       , FEMATERIAL_ID, "elastic membrane");

REGISTER_FECORE_CLASS(FELinearSpring           , FEMATERIAL_ID, "linear spring");
REGISTER_FECORE_CLASS(FETensionOnlyLinearSpring, FEMATERIAL_ID, "tension-only linear spring");
REGISTER_FECORE_CLASS(FENonLinearSpring        , FEMATERIAL_ID, "nonlinear spring");
REGISTER_FECORE_CLASS(FEExperimentalSpring     , FEMATERIAL_ID, "experimental spring");

REGISTER_FECORE_CLASS(FEReactiveViscoelasticMaterial       , FEMATERIAL_ID, "reactive viscoelastic"     );
REGISTER_FECORE_CLASS(FEUncoupledReactiveViscoelasticMaterial, FEMATERIAL_ID, "uncoupled reactive viscoelastic");
REGISTER_FECORE_CLASS(FEBondRelaxationExponential          , FEMATERIAL_ID, "relaxation-exponential"    );
REGISTER_FECORE_CLASS(FEBondRelaxationExpDistortion        , FEMATERIAL_ID, "relaxation-exp-distortion" );
REGISTER_FECORE_CLASS(FEBondRelaxationFung                 , FEMATERIAL_ID, "relaxation-Fung"           );
REGISTER_FECORE_CLASS(FEBondRelaxationPark                 , FEMATERIAL_ID, "relaxation-Park"           );
REGISTER_FECORE_CLASS(FEBondRelaxationParkDistortion       , FEMATERIAL_ID, "relaxation-Park-distortion");
REGISTER_FECORE_CLASS(FEBondRelaxationPower                , FEMATERIAL_ID, "relaxation-power"           );
REGISTER_FECORE_CLASS(FEBondRelaxationPowerDistortion      , FEMATERIAL_ID, "relaxation-power-distortion");
REGISTER_FECORE_CLASS(FEBondRelaxationCarreau              , FEMATERIAL_ID, "relaxation-Carreau"         );
    
REGISTER_FECORE_CLASS(FEDamageMaterial                     , FEMATERIAL_ID, "elastic damage"       );
REGISTER_FECORE_CLASS(FEDamageMaterialUC                   , FEMATERIAL_ID, "uncoupled elastic damage");
REGISTER_FECORE_CLASS(FEDamageCDFSimo                      , FEMATERIAL_ID, "CDF Simo"             );
REGISTER_FECORE_CLASS(FEDamageCDFLogNormal                 , FEMATERIAL_ID, "CDF log-normal"       );
REGISTER_FECORE_CLASS(FEDamageCDFWeibull                   , FEMATERIAL_ID, "CDF Weibull"          );
REGISTER_FECORE_CLASS(FEDamageCDFStep                      , FEMATERIAL_ID, "CDF step"             );
REGISTER_FECORE_CLASS(FEDamageCDFPQP                       , FEMATERIAL_ID, "CDF quintic"          );
REGISTER_FECORE_CLASS(FEDamageCDFGamma                     , FEMATERIAL_ID, "CDF gamma"            );
REGISTER_FECORE_CLASS(FEDamageCriterionSimo                , FEMATERIAL_ID, "DC Simo"              );
REGISTER_FECORE_CLASS(FEDamageCriterionSED                 , FEMATERIAL_ID, "DC strain energy density"     );
REGISTER_FECORE_CLASS(FEDamageCriterionSSE                 , FEMATERIAL_ID, "DC specific strain energy"    );
REGISTER_FECORE_CLASS(FEDamageCriterionVMS                 , FEMATERIAL_ID, "DC von Mises stress"          );
REGISTER_FECORE_CLASS(FEDamageCriterionMSS                 , FEMATERIAL_ID, "DC max shear stress"          );
REGISTER_FECORE_CLASS(FEDamageCriterionMNS                 , FEMATERIAL_ID, "DC max normal stress"         );
REGISTER_FECORE_CLASS(FEDamageCriterionMNLS                , FEMATERIAL_ID, "DC max normal Lagrange strain");
REGISTER_FECORE_CLASS(FEFatigueMaterial                    , FEMATERIAL_ID, "reactive fatigue"             );
REGISTER_FECORE_CLASS(FEReactiveFatigue                    , FEMATERIAL_ID, "reactive fatigue 2"           );
REGISTER_FECORE_CLASS(FEReactivePlasticity                 , FEMATERIAL_ID, "reactive plasticity"          );

//-----------------------------------------------------------------------------
// domain classes
REGISTER_FECORE_CLASS(FERigidSolidDomain         , FEDOMAIN_ID, "rigid-solid"       );
REGISTER_FECORE_CLASS(FERigidShellDomain         , FEDOMAIN_ID, "rigid-shell"       );
REGISTER_FECORE_CLASS(FERigidShellDomainOld      , FEDOMAIN_ID, "rigid-shell-old"   );
REGISTER_FECORE_CLASS(FERemodelingElasticDomain  , FEDOMAIN_ID, "remodeling-solid"  );
REGISTER_FECORE_CLASS(FEElasticMultiscaleDomain1O, FEDOMAIN_ID, "elastic-mm-solid"  );
REGISTER_FECORE_CLASS(FEElasticMultiscaleDomain2O, FEDOMAIN_ID, "elastic-mm-solid2O");
REGISTER_FECORE_CLASS(FEElasticSolidDomain2O     , FEDOMAIN_ID, "elastic-solid2O"   );
REGISTER_FECORE_CLASS(FE3FieldElasticSolidDomain , FEDOMAIN_ID, "three-field-solid" );
REGISTER_FECORE_CLASS(FE3FieldElasticShellDomain , FEDOMAIN_ID, "three-field-shell" );
REGISTER_FECORE_CLASS(FEUDGHexDomain             , FEDOMAIN_ID, "udg-hex"           );
REGISTER_FECORE_CLASS(FESRIElasticSolidDomain    , FEDOMAIN_ID, "sri-solid"         );
REGISTER_FECORE_CLASS(FEUT4Domain                , FEDOMAIN_ID, "ut4-solid"         );
REGISTER_FECORE_CLASS(FEElasticSolidDomain       , FEDOMAIN_ID, "elastic-solid"     );
REGISTER_FECORE_CLASS(FEElasticShellDomain       , FEDOMAIN_ID, "elastic-shell"     );
REGISTER_FECORE_CLASS(FEElasticShellDomainOld    , FEDOMAIN_ID, "elastic-shell-old" );
REGISTER_FECORE_CLASS(FEElasticEASShellDomain    , FEDOMAIN_ID, "elastic-shell-eas" );
REGISTER_FECORE_CLASS(FEElasticANSShellDomain    , FEDOMAIN_ID, "elastic-shell-ans" );
REGISTER_FECORE_CLASS(FEElasticTrussDomain       , FEDOMAIN_ID, "elastic-truss"     );
REGISTER_FECORE_CLASS(FEDiscreteSpringDomain     , FEDOMAIN_ID, "discrete-spring"   );
REGISTER_FECORE_CLASS(FEDeformableSpringDomain   , FEDOMAIN_ID, "deformable-spring" );
REGISTER_FECORE_CLASS(FEDeformableSpringDomain2  , FEDOMAIN_ID, "deformable-spring2");

//-----------------------------------------------------------------------------
// classes derived from FEBoundaryCondition
REGISTER_FECORE_CLASS(FEBCPrescribedDeformation  , FEBC_ID, "prescribed deformation"   );
REGISTER_FECORE_CLASS(FEBCPrescribedDeformation2O, FEBC_ID, "prescribed deformation 2O");
REGISTER_FECORE_CLASS(FEPrescribedNormalDisplacement, FEBC_ID, "normal displacement");

//-----------------------------------------------------------------------------
// classes derived from FESurfaceLoad
REGISTER_FECORE_CLASS(FEPressureLoad, FESURFACELOAD_ID, "pressure");
REGISTER_FECORE_CLASS(FETractionLoad, FESURFACELOAD_ID, "traction");

//-----------------------------------------------------------------------------
// classes derived from FEBodyForce
REGISTER_FECORE_CLASS(FEConstBodyForce      , FEBODYLOAD_ID, "const"      );
REGISTER_FECORE_CLASS(FENonConstBodyForce   , FEBODYLOAD_ID, "non-const"  );
REGISTER_FECORE_CLASS(FECentrifugalBodyForce, FEBODYLOAD_ID, "centrifugal");
REGISTER_FECORE_CLASS(FEPointBodyForce      , FEBODYLOAD_ID, "point"      );

//-----------------------------------------------------------------------------
// constraint classes
REGISTER_FECORE_CLASS(FEPointConstraint      , FENLCONSTRAINT_ID, "point"                  );
REGISTER_FECORE_CLASS(FELinearConstraintSet  , FENLCONSTRAINT_ID, "linear constraint"      );
REGISTER_FECORE_CLASS(FESymmetryPlane        , FENLCONSTRAINT_ID, "symmetry plane"         );
REGISTER_FECORE_CLASS(FERigidJoint           , FENLCONSTRAINT_ID, "rigid joint"            );
REGISTER_FECORE_CLASS(FERigidSphericalJoint  , FENLCONSTRAINT_ID, "rigid spherical joint"  );
REGISTER_FECORE_CLASS(FERigidRevoluteJoint   , FENLCONSTRAINT_ID, "rigid revolute joint"   );
REGISTER_FECORE_CLASS(FERigidPrismaticJoint  , FENLCONSTRAINT_ID, "rigid prismatic joint"  );
REGISTER_FECORE_CLASS(FERigidCylindricalJoint, FENLCONSTRAINT_ID, "rigid cylindrical joint");
REGISTER_FECORE_CLASS(FERigidPlanarJoint     , FENLCONSTRAINT_ID, "rigid planar joint"     );
REGISTER_FECORE_CLASS(FERigidLock            , FENLCONSTRAINT_ID, "rigid lock"             );
REGISTER_FECORE_CLASS(FERigidSpring          , FENLCONSTRAINT_ID, "rigid spring"           );
REGISTER_FECORE_CLASS(FERigidDamper          , FENLCONSTRAINT_ID, "rigid damper"           );
REGISTER_FECORE_CLASS(FERigidAngularDamper   , FENLCONSTRAINT_ID, "rigid angular damper"   );
REGISTER_FECORE_CLASS(FERigidContractileForce, FENLCONSTRAINT_ID, "rigid contractile force");
REGISTER_FECORE_CLASS(FEVolumeConstraint     , FENLCONSTRAINT_ID, "volume"                 );
REGISTER_FECORE_CLASS(FEDiscreteContact      , FENLCONSTRAINT_ID, "discrete contact"       );
REGISTER_FECORE_CLASS(FEDiscreteContact2     , FENLCONSTRAINT_ID, "discrete contact2"      );
REGISTER_FECORE_CLASS(FEDistanceConstraint   , FENLCONSTRAINT_ID, "node distance"          );
REGISTER_FECORE_CLASS(FE2OMicroConstraint    , FENLCONSTRAINT_ID, "2O microfluc"           );

//-----------------------------------------------------------------------------
// classes derived from FEContactInterface
REGISTER_FECORE_CLASS(FESlidingInterface         , FESURFACEPAIRINTERACTION_ID, "sliding-node-on-facet"      );
REGISTER_FECORE_CLASS(FEFacet2FacetSliding       , FESURFACEPAIRINTERACTION_ID, "sliding-facet-on-facet"     );
REGISTER_FECORE_CLASS(FESlidingInterfaceBW       , FESURFACEPAIRINTERACTION_ID, "sliding-elastic"            );
REGISTER_FECORE_CLASS(FETiedInterface            , FESURFACEPAIRINTERACTION_ID, "tied-node-on-facet"         );
REGISTER_FECORE_CLASS(FEFacet2FacetTied          , FESURFACEPAIRINTERACTION_ID, "tied-facet-on-facet"        );
REGISTER_FECORE_CLASS(FETiedElasticInterface     , FESURFACEPAIRINTERACTION_ID, "tied-elastic"               );
REGISTER_FECORE_CLASS(FEPeriodicBoundary         , FESURFACEPAIRINTERACTION_ID, "periodic boundary"          );
REGISTER_FECORE_CLASS(FEPeriodicBoundary1O       , FESURFACEPAIRINTERACTION_ID, "periodic boundary1O"        );
REGISTER_FECORE_CLASS(FEPeriodicBoundary2O       , FESURFACEPAIRINTERACTION_ID, "periodic boundary2O"        );
REGISTER_FECORE_CLASS(FERigidWallInterface       , FESURFACEPAIRINTERACTION_ID, "rigid_wall"                 );
REGISTER_FECORE_CLASS(FERigidSlidingContact      , FESURFACEPAIRINTERACTION_ID, "rigid sliding"              );
REGISTER_FECORE_CLASS(FEPeriodicSurfaceConstraint, FESURFACEPAIRINTERACTION_ID, "surface constraint");
REGISTER_FECORE_CLASS(FEStickyInterface          , FESURFACEPAIRINTERACTION_ID, "sticky"                     );
REGISTER_FECORE_CLASS(FEMortarSlidingContact     , FESURFACEPAIRINTERACTION_ID, "mortar-sliding"             );
REGISTER_FECORE_CLASS(FEMortarTiedContact        , FESURFACEPAIRINTERACTION_ID, "mortar-tied"                );

// The type strings have changed for the following contact interface since FEBio 2.7, 
// but continue to be supported for now. 
REGISTER_FECORE_CLASS_OBSOLETE(FESlidingInterface         , FESURFACEPAIRINTERACTION_ID, "sliding_with_gaps"          );
REGISTER_FECORE_CLASS_OBSOLETE(FEFacet2FacetSliding       , FESURFACEPAIRINTERACTION_ID, "facet-to-facet sliding"     );
REGISTER_FECORE_CLASS_OBSOLETE(FESlidingInterfaceBW       , FESURFACEPAIRINTERACTION_ID, "sliding-tension-compression");
REGISTER_FECORE_CLASS_OBSOLETE(FETiedInterface            , FESURFACEPAIRINTERACTION_ID, "tied"                       );
REGISTER_FECORE_CLASS_OBSOLETE(FEFacet2FacetTied          , FESURFACEPAIRINTERACTION_ID, "facet-to-facet tied"        );

//-----------------------------------------------------------------------------
// classes derived from FERigidSurface
REGISTER_FECORE_CLASS(FERigidPlane    , FERIGIDOBJECT_ID, "plane"   );
REGISTER_FECORE_CLASS(FERigidSphere   , FERIGIDOBJECT_ID, "sphere"  );
REGISTER_FECORE_CLASS(FERigidCylinder , FERIGIDOBJECT_ID, "cylinder");
REGISTER_FECORE_CLASS(FERigidEllipsoid, FERIGIDOBJECT_ID, "ellipsoid");

//-----------------------------------------------------------------------------
// classes derived directly from FEModelLoad
// TODO: define another SUPER_CLASS_ID for this
REGISTER_FECORE_CLASS(FERigidAxialForce      , FEBC_ID, "rigid_axial_force");
REGISTER_FECORE_CLASS(FERigidBodyForce       , FEBC_ID, "rigid_force"      );
REGISTER_FECORE_CLASS(FERigidBodyFixedBC     , FEBC_ID, "rigid_fixed"      );
REGISTER_FECORE_CLASS(FERigidBodyDisplacement, FEBC_ID, "rigid_prescribed" );
REGISTER_FECORE_CLASS(FERigidCable           , FEBC_ID, "rigid_cable"      );

//-----------------------------------------------------------------------------
// classes derived from FEPlotData
REGISTER_FECORE_CLASS(FEPlotElementVelocity      , FEPLOTDATA_ID, "velocity"              );
REGISTER_FECORE_CLASS(FEPlotElementAcceleration  , FEPLOTDATA_ID, "acceleration"          );
REGISTER_FECORE_CLASS(FEPlotDensity              , FEPLOTDATA_ID, "density"               );
REGISTER_FECORE_CLASS(FEPlotElementStress        , FEPLOTDATA_ID, "stress"                );
REGISTER_FECORE_CLASS(FEPlotElementUncoupledPressure, FEPLOTDATA_ID, "uncoupled pressure" );
REGISTER_FECORE_CLASS(FEPlotElementElasticity    , FEPLOTDATA_ID, "elasticity"            );
REGISTER_FECORE_CLASS(FEPlotRelativeVolume       , FEPLOTDATA_ID, "relative volume"       );
REGISTER_FECORE_CLASS(FEPlotFiberVector          , FEPLOTDATA_ID, "fiber vector"          );
REGISTER_FECORE_CLASS(FEPlotFiberStretch         , FEPLOTDATA_ID, "fiber stretch"         );
REGISTER_FECORE_CLASS(FEPlotDevFiberStretch      , FEPLOTDATA_ID, "deviatoric fiber stretch");
REGISTER_FECORE_CLASS(FEPlotMaterialAxes		 , FEPLOTDATA_ID, "material axes"         );
REGISTER_FECORE_CLASS(FEPlotShellThickness       , FEPLOTDATA_ID, "shell thickness"       );
REGISTER_FECORE_CLASS(FEPlotShellDirector        , FEPLOTDATA_ID, "shell director"        );
REGISTER_FECORE_CLASS(FEPlotDamage               , FEPLOTDATA_ID, "damage"                );
REGISTER_FECORE_CLASS(FEPlotNestedDamage         , FEPLOTDATA_ID, "nested damage"         );
REGISTER_FECORE_CLASS(FEPlotIntactBondFraction   , FEPLOTDATA_ID, "intact bond fraction"  );
REGISTER_FECORE_CLASS(FEPlotFatigueBondFraction  , FEPLOTDATA_ID, "fatigue bond fraction" );
REGISTER_FECORE_CLASS(FEPlotOctahedralPlasticStrain, FEPLOTDATA_ID, "octahedral plastic strain");
REGISTER_FECORE_CLASS(FEPlotMixtureVolumeFraction, FEPLOTDATA_ID, "volume fraction"       );
REGISTER_FECORE_CLASS(FEPlotUT4NodalStresses     , FEPLOTDATA_ID, "ut4 nodal stress"      );
REGISTER_FECORE_CLASS(FEPlotShellStrain          , FEPLOTDATA_ID, "shell strain"          );
REGISTER_FECORE_CLASS(FEPlotShellRelativeVolume  , FEPLOTDATA_ID, "shell relative volume" );
REGISTER_FECORE_CLASS(FEPlotContactGap			 , FEPLOTDATA_ID, "contact gap"           );
REGISTER_FECORE_CLASS(FEPlotNodalContactGap      , FEPLOTDATA_ID, "nodal contact gap"     );
REGISTER_FECORE_CLASS(FEPlotVectorGap			 , FEPLOTDATA_ID, "vector gap"            );
REGISTER_FECORE_CLASS(FEPlotNodalVectorGap       , FEPLOTDATA_ID, "nodal vector gap"      );
REGISTER_FECORE_CLASS(FEPlotContactPressure		 , FEPLOTDATA_ID, "contact pressure"      );
REGISTER_FECORE_CLASS(FEPlotNodalContactPressure , FEPLOTDATA_ID, "nodal contact pressure");
REGISTER_FECORE_CLASS(FEPlotContactTraction		 , FEPLOTDATA_ID, "contact traction"      );
REGISTER_FECORE_CLASS(FEPlotNodalContactTraction , FEPLOTDATA_ID, "nodal contact traction");
REGISTER_FECORE_CLASS(FEPlotStickStatus			 , FEPLOTDATA_ID, "contact stick"         );
REGISTER_FECORE_CLASS(FEPlotContactForce 		 , FEPLOTDATA_ID, "contact force"         );
REGISTER_FECORE_CLASS(FEPlotContactArea 		 , FEPLOTDATA_ID, "contact area"          );
REGISTER_FECORE_CLASS(FEPlotContactPenalty 		 , FEPLOTDATA_ID, "contact penalty"       );
REGISTER_FECORE_CLASS(FEPlotSPRStresses          , FEPLOTDATA_ID, "SPR stress"            );
REGISTER_FECORE_CLASS(FEPlotSPRLinearStresses    , FEPLOTDATA_ID, "SPR-P1 stress"         );
REGISTER_FECORE_CLASS(FEPlotSPRPrincStresses     , FEPLOTDATA_ID, "SPR principal stress"  );
REGISTER_FECORE_CLASS(FEPlotSPRTestLinear		 , FEPLOTDATA_ID, "SPR test linear"       );
REGISTER_FECORE_CLASS(FEPlotSPRTestQuadratic	 , FEPLOTDATA_ID, "SPR test quadratic"    );
REGISTER_FECORE_CLASS(FEPlotNodalStresses		 , FEPLOTDATA_ID, "nodal stress"		  );
REGISTER_FECORE_CLASS(FEPlotLagrangeStrain       , FEPLOTDATA_ID, "Lagrange strain"       );
REGISTER_FECORE_CLASS(FEPlotSPRLagrangeStrain    , FEPLOTDATA_ID, "SPR Lagrange strain"   );
REGISTER_FECORE_CLASS(FEPlotMortarContactGap     , FEPLOTDATA_ID, "mortar-gap"            );
REGISTER_FECORE_CLASS(FEPlotSurfaceTraction		 , FEPLOTDATA_ID, "surface traction"      );
REGISTER_FECORE_CLASS(FEPlotNodalSurfaceTraction , FEPLOTDATA_ID, "nodal surface traction");
REGISTER_FECORE_CLASS(FEPlotEnclosedVolume       , FEPLOTDATA_ID, "enclosed volume"       );
REGISTER_FECORE_CLASS(FEPlotStrainEnergyDensity  , FEPLOTDATA_ID, "strain energy density" );
REGISTER_FECORE_CLASS(FEPlotDevStrainEnergyDensity  , FEPLOTDATA_ID, "deviatoric strain energy density" );
REGISTER_FECORE_CLASS(FEPlotSpecificStrainEnergy , FEPLOTDATA_ID, "specific strain energy");
REGISTER_FECORE_CLASS(FEPlotKineticEnergyDensity , FEPLOTDATA_ID, "kinetic energy density");
REGISTER_FECORE_CLASS(FEPlotElementStrainEnergy  , FEPLOTDATA_ID, "element strain energy" );
REGISTER_FECORE_CLASS(FEPlotElementKineticEnergy , FEPLOTDATA_ID, "element kinetic energy");
REGISTER_FECORE_CLASS(FEPlotElementCenterOfMass  , FEPLOTDATA_ID, "element center of mass");
REGISTER_FECORE_CLASS(FEPlotElementLinearMomentum, FEPLOTDATA_ID, "element linear momentum");
REGISTER_FECORE_CLASS(FEPlotElementAngularMomentum, FEPLOTDATA_ID, "element angular momentum");
REGISTER_FECORE_CLASS(FEPlotElementStressPower   , FEPLOTDATA_ID, "element stress power"  );
REGISTER_FECORE_CLASS(FEPlotCurrentElementStrainEnergy   , FEPLOTDATA_ID, "current element strain energy"   );
REGISTER_FECORE_CLASS(FEPlotCurrentElementKineticEnergy  , FEPLOTDATA_ID, "current element kinetic energy"  );
REGISTER_FECORE_CLASS(FEPlotCurrentElementCenterOfMass   , FEPLOTDATA_ID, "current element center of mass"  );
REGISTER_FECORE_CLASS(FEPlotCurrentElementLinearMomentum , FEPLOTDATA_ID, "current element linear momentum" );
REGISTER_FECORE_CLASS(FEPlotCurrentElementAngularMomentum, FEPLOTDATA_ID, "current element angular momentum");

// 2O continuum fields
REGISTER_FECORE_CLASS(FEPlotElementGnorm         , FEPLOTDATA_ID, "G norm"                );              
REGISTER_FECORE_CLASS(FEPlotElementsnorm         , FEPLOTDATA_ID, "s norm"   			  );
REGISTER_FECORE_CLASS(FEPlotElementtaunorm       , FEPLOTDATA_ID, "tau norm"			  );
REGISTER_FECORE_CLASS(FEPlotElementPK1norm       , FEPLOTDATA_ID, "PK1 norm"   			  );
REGISTER_FECORE_CLASS(FEPlotElementQK1norm       , FEPLOTDATA_ID, "QK1 norm"			  );
REGISTER_FECORE_CLASS(FEPlotElementSnorm         , FEPLOTDATA_ID, "S norm"   			  );
REGISTER_FECORE_CLASS(FEPlotElementTnorm         , FEPLOTDATA_ID, "T norm"  			  );
REGISTER_FECORE_CLASS(FEPlotElementinfstrnorm    , FEPLOTDATA_ID, "inf strain grad norm"  );
REGISTER_FECORE_CLASS(FEPlotElementGLstrnorm     , FEPLOTDATA_ID, "GL strain grad norm"   );
REGISTER_FECORE_CLASS(FEPlotElementEAstrnorm     , FEPLOTDATA_ID, "EA strain grad norm"   );

REGISTER_FECORE_CLASS(FEPlotElementMacroEnergy   , FEPLOTDATA_ID, "M energy"		);
REGISTER_FECORE_CLASS(FEPlotElementMicroEnergy   , FEPLOTDATA_ID, "micro energy"	);
REGISTER_FECORE_CLASS(FEPlotElementenergydiff    , FEPLOTDATA_ID, "M-m energy diff" );

//-----------------------------------------------------------------------------
REGISTER_FECORE_CLASS(FEPlotNodeVelocity       , FEPLOTDATA_ID, "nodal velocity"                 );
REGISTER_FECORE_CLASS(FEPlotNodeAcceleration   , FEPLOTDATA_ID, "nodal acceleration"             );
REGISTER_FECORE_CLASS(FEPlotNodeReactionForces , FEPLOTDATA_ID, "reaction forces");
REGISTER_FECORE_CLASS(FEPlotRigidReactionForce , FEPLOTDATA_ID, "rigid force"    );
REGISTER_FECORE_CLASS(FEPlotRigidReactionTorque, FEPLOTDATA_ID, "rigid torque"   );
REGISTER_FECORE_CLASS(FEPlotRigidDisplacement       , FEPLOTDATA_ID, "rigid position"            );
REGISTER_FECORE_CLASS(FEPlotRigidVelocity           , FEPLOTDATA_ID, "rigid velocity"            );
REGISTER_FECORE_CLASS(FEPlotRigidAcceleration       , FEPLOTDATA_ID, "rigid acceleration"        );
REGISTER_FECORE_CLASS(FEPlotRigidRotation           , FEPLOTDATA_ID, "rigid angular position"    );
REGISTER_FECORE_CLASS(FEPlotRigidAngularVelocity    , FEPLOTDATA_ID, "rigid angular velocity"    );
REGISTER_FECORE_CLASS(FEPlotRigidAngularAcceleration, FEPLOTDATA_ID, "rigid angular acceleration");
REGISTER_FECORE_CLASS(FEPlotRigidLinearMomentum     , FEPLOTDATA_ID, "rigid linear momentum"     );
REGISTER_FECORE_CLASS(FEPlotRigidAngularMomentum    , FEPLOTDATA_ID, "rigid angular momentum"    );
REGISTER_FECORE_CLASS(FEPlotRigidKineticEnergy      , FEPLOTDATA_ID, "rigid kinetic energy"      );
REGISTER_FECORE_CLASS(FEPlotRigidEuler              , FEPLOTDATA_ID, "Euler angle"               );
REGISTER_FECORE_CLASS(FEPlotRigidRotationVector     , FEPLOTDATA_ID, "rigid rotation vector"     );

//-----------------------------------------------------------------------------
// Derived from FENodeLogData
REGISTER_FECORE_CLASS(FENodeXPos  , FENODELOGDATA_ID, "x");
REGISTER_FECORE_CLASS(FENodeYPos  , FENODELOGDATA_ID, "y");
REGISTER_FECORE_CLASS(FENodeZPos  , FENODELOGDATA_ID, "z");
REGISTER_FECORE_CLASS(FENodeXDisp , FENODELOGDATA_ID, "ux");
REGISTER_FECORE_CLASS(FENodeYDisp , FENODELOGDATA_ID, "uy");
REGISTER_FECORE_CLASS(FENodeZDisp , FENODELOGDATA_ID, "uz");
REGISTER_FECORE_CLASS(FENodeXVel  , FENODELOGDATA_ID, "vx");
REGISTER_FECORE_CLASS(FENodeYVel  , FENODELOGDATA_ID, "vy");
REGISTER_FECORE_CLASS(FENodeZVel  , FENODELOGDATA_ID, "vz");
REGISTER_FECORE_CLASS(FENodeXAcc  , FENODELOGDATA_ID, "ax");
REGISTER_FECORE_CLASS(FENodeYAcc  , FENODELOGDATA_ID, "ay");
REGISTER_FECORE_CLASS(FENodeZAcc  , FENODELOGDATA_ID, "az");
REGISTER_FECORE_CLASS(FENodeForceX, FENODELOGDATA_ID, "Rx");
REGISTER_FECORE_CLASS(FENodeForceY, FENODELOGDATA_ID, "Ry");
REGISTER_FECORE_CLASS(FENodeForceZ, FENODELOGDATA_ID, "Rz");

//-----------------------------------------------------------------------------
// Derived from FELogElemData
REGISTER_FECORE_CLASS(FELogElemPosX    , FEELEMLOGDATA_ID, "x");
REGISTER_FECORE_CLASS(FELogElemPosY    , FEELEMLOGDATA_ID, "y");
REGISTER_FECORE_CLASS(FELogElemPosZ    , FEELEMLOGDATA_ID, "z");
REGISTER_FECORE_CLASS(FELogElemJacobian, FEELEMLOGDATA_ID, "J");
REGISTER_FECORE_CLASS(FELogElemStrainX , FEELEMLOGDATA_ID, "Ex");
REGISTER_FECORE_CLASS(FELogElemStrainY , FEELEMLOGDATA_ID, "Ey");
REGISTER_FECORE_CLASS(FELogElemStrainZ , FEELEMLOGDATA_ID, "Ez");
REGISTER_FECORE_CLASS(FELogElemStrainXY, FEELEMLOGDATA_ID, "Exy");
REGISTER_FECORE_CLASS(FELogElemStrainYZ, FEELEMLOGDATA_ID, "Eyz");
REGISTER_FECORE_CLASS(FELogElemStrainXZ, FEELEMLOGDATA_ID, "Exz");
REGISTER_FECORE_CLASS(FELogElemStrain1 , FEELEMLOGDATA_ID, "E1");
REGISTER_FECORE_CLASS(FELogElemStrain2 , FEELEMLOGDATA_ID, "E2");
REGISTER_FECORE_CLASS(FELogElemStrain3 , FEELEMLOGDATA_ID, "E3");
REGISTER_FECORE_CLASS(FELogElemInfStrainX , FEELEMLOGDATA_ID, "ex");
REGISTER_FECORE_CLASS(FELogElemInfStrainY , FEELEMLOGDATA_ID, "ey");
REGISTER_FECORE_CLASS(FELogElemInfStrainZ , FEELEMLOGDATA_ID, "ez");
REGISTER_FECORE_CLASS(FELogElemInfStrainXY, FEELEMLOGDATA_ID, "exy");
REGISTER_FECORE_CLASS(FELogElemInfStrainYZ, FEELEMLOGDATA_ID, "eyz");
REGISTER_FECORE_CLASS(FELogElemInfStrainXZ, FEELEMLOGDATA_ID, "exz");
REGISTER_FECORE_CLASS(FELogElemStressX, FEELEMLOGDATA_ID, "sx");
REGISTER_FECORE_CLASS(FELogElemStressY , FEELEMLOGDATA_ID, "sy");
REGISTER_FECORE_CLASS(FELogElemStressZ , FEELEMLOGDATA_ID, "sz");
REGISTER_FECORE_CLASS(FELogElemStressXY, FEELEMLOGDATA_ID, "sxy");
REGISTER_FECORE_CLASS(FELogElemStressYZ, FEELEMLOGDATA_ID, "syz");
REGISTER_FECORE_CLASS(FELogElemStressXZ, FEELEMLOGDATA_ID, "sxz");
REGISTER_FECORE_CLASS(FELogElemStress1 , FEELEMLOGDATA_ID, "s1");
REGISTER_FECORE_CLASS(FELogElemStress2 , FEELEMLOGDATA_ID, "s2");
REGISTER_FECORE_CLASS(FELogElemStress3 , FEELEMLOGDATA_ID, "s3");
REGISTER_FECORE_CLASS(FELogElemDeformationGradientXX, FEELEMLOGDATA_ID, "Fxx");
REGISTER_FECORE_CLASS(FELogElemDeformationGradientXY, FEELEMLOGDATA_ID, "Fxy");
REGISTER_FECORE_CLASS(FELogElemDeformationGradientXZ, FEELEMLOGDATA_ID, "Fxz");
REGISTER_FECORE_CLASS(FELogElemDeformationGradientYX, FEELEMLOGDATA_ID, "Fyx");
REGISTER_FECORE_CLASS(FELogElemDeformationGradientYY, FEELEMLOGDATA_ID, "Fyy");
REGISTER_FECORE_CLASS(FELogElemDeformationGradientYZ, FEELEMLOGDATA_ID, "Fyz");
REGISTER_FECORE_CLASS(FELogElemDeformationGradientZX, FEELEMLOGDATA_ID, "Fzx");
REGISTER_FECORE_CLASS(FELogElemDeformationGradientZY, FEELEMLOGDATA_ID, "Fzy");
REGISTER_FECORE_CLASS(FELogElemDeformationGradientZZ, FEELEMLOGDATA_ID, "Fzz");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID,  0, "cxxxx");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID,  1, "cxxyy");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID,  2, "cyyyy");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID,  3, "cxxzz");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID,  4, "cyyzz");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID,  5, "czzzz");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID,  6, "cxxxy");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID,  7, "cyyxy");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID,  8, "czzxy");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID,  9, "cxyxy");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID, 10, "cxxyz");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID, 11, "cyyyz");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID, 12, "czzyz");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID, 13, "cxyyz");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID, 14, "cyzyz");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID, 15, "cxxxz");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID, 16, "cyyxz");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID, 17, "czzxz");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID, 18, "cxyxz");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID, 19, "cyzxz");
REGISTER_FECORE_CLASS_T(FELogElemElasticity, FEELEMLOGDATA_ID, 20, "cxzxz");
REGISTER_FECORE_CLASS(FELogElemStrainEnergyDensity    , FEELEMLOGDATA_ID, "sed");
REGISTER_FECORE_CLASS(FELogElemDevStrainEnergyDensity , FEELEMLOGDATA_ID, "devsed");
REGISTER_FECORE_CLASS(FELogElemFiberStretch           , FEELEMLOGDATA_ID, "fiber_stretch");
REGISTER_FECORE_CLASS(FELogElemFiberVectorX, FEELEMLOGDATA_ID, "fiber_x");
REGISTER_FECORE_CLASS(FELogElemFiberVectorY, FEELEMLOGDATA_ID, "fiber_y");
REGISTER_FECORE_CLASS(FELogElemFiberVectorZ, FEELEMLOGDATA_ID, "fiber_z");
REGISTER_FECORE_CLASS(FELogDamage          , FEELEMLOGDATA_ID, "D");

//-----------------------------------------------------------------------------
// Derived from FELogObjectData
REGISTER_FECORE_CLASS(FELogRigidBodyPosX   , FEOBJLOGDATA_ID, "x");
REGISTER_FECORE_CLASS(FELogRigidBodyPosY   , FEOBJLOGDATA_ID, "y");
REGISTER_FECORE_CLASS(FELogRigidBodyPosZ   , FEOBJLOGDATA_ID, "z");
REGISTER_FECORE_CLASS(FELogRigidBodyVelX   , FEOBJLOGDATA_ID, "vx");
REGISTER_FECORE_CLASS(FELogRigidBodyVelY   , FEOBJLOGDATA_ID, "vy");
REGISTER_FECORE_CLASS(FELogRigidBodyVelZ   , FEOBJLOGDATA_ID, "vz");
REGISTER_FECORE_CLASS(FELogRigidBodyAccX   , FEOBJLOGDATA_ID, "ax");
REGISTER_FECORE_CLASS(FELogRigidBodyAccY   , FEOBJLOGDATA_ID, "ay");
REGISTER_FECORE_CLASS(FELogRigidBodyAccZ   , FEOBJLOGDATA_ID, "az");
REGISTER_FECORE_CLASS(FELogRigidBodyAngPosX, FEOBJLOGDATA_ID, "thx");
REGISTER_FECORE_CLASS(FELogRigidBodyAngPosY, FEOBJLOGDATA_ID, "thy");
REGISTER_FECORE_CLASS(FELogRigidBodyAngPosZ, FEOBJLOGDATA_ID, "thz");
REGISTER_FECORE_CLASS(FELogRigidBodyAngVelX, FEOBJLOGDATA_ID, "omx");
REGISTER_FECORE_CLASS(FELogRigidBodyAngVelY, FEOBJLOGDATA_ID, "omy");
REGISTER_FECORE_CLASS(FELogRigidBodyAngVelZ, FEOBJLOGDATA_ID, "omz");
REGISTER_FECORE_CLASS(FELogRigidBodyAngAccX, FEOBJLOGDATA_ID, "alx");
REGISTER_FECORE_CLASS(FELogRigidBodyAngAccY, FEOBJLOGDATA_ID, "aly");
REGISTER_FECORE_CLASS(FELogRigidBodyAngAccZ, FEOBJLOGDATA_ID, "alz");
REGISTER_FECORE_CLASS(FELogRigidBodyQuatX  , FEOBJLOGDATA_ID, "qx");
REGISTER_FECORE_CLASS(FELogRigidBodyQuatY  , FEOBJLOGDATA_ID, "qy");
REGISTER_FECORE_CLASS(FELogRigidBodyQuatZ  , FEOBJLOGDATA_ID, "qz");
REGISTER_FECORE_CLASS(FELogRigidBodyQuatW  , FEOBJLOGDATA_ID, "qw");
REGISTER_FECORE_CLASS(FELogRigidBodyR11    , FEOBJLOGDATA_ID, "R11");
REGISTER_FECORE_CLASS(FELogRigidBodyR12    , FEOBJLOGDATA_ID, "R12");
REGISTER_FECORE_CLASS(FELogRigidBodyR13    , FEOBJLOGDATA_ID, "R13");
REGISTER_FECORE_CLASS(FELogRigidBodyR21    , FEOBJLOGDATA_ID, "R21");
REGISTER_FECORE_CLASS(FELogRigidBodyR22    , FEOBJLOGDATA_ID, "R22");
REGISTER_FECORE_CLASS(FELogRigidBodyR23    , FEOBJLOGDATA_ID, "R23");
REGISTER_FECORE_CLASS(FELogRigidBodyR31    , FEOBJLOGDATA_ID, "R31");
REGISTER_FECORE_CLASS(FELogRigidBodyR32    , FEOBJLOGDATA_ID, "R32");
REGISTER_FECORE_CLASS(FELogRigidBodyR33    , FEOBJLOGDATA_ID, "R33");
REGISTER_FECORE_CLASS(FELogRigidBodyForceX , FEOBJLOGDATA_ID, "Fx");
REGISTER_FECORE_CLASS(FELogRigidBodyForceY , FEOBJLOGDATA_ID, "Fy");
REGISTER_FECORE_CLASS(FELogRigidBodyForceZ , FEOBJLOGDATA_ID, "Fz");
REGISTER_FECORE_CLASS(FELogRigidBodyTorqueX, FEOBJLOGDATA_ID, "Mx");
REGISTER_FECORE_CLASS(FELogRigidBodyTorqueY, FEOBJLOGDATA_ID, "My");
REGISTER_FECORE_CLASS(FELogRigidBodyTorqueZ, FEOBJLOGDATA_ID, "Mz");
REGISTER_FECORE_CLASS(FELogRigidBodyKineticEnergy, FEOBJLOGDATA_ID, "KE");

//-----------------------------------------------------------------------------
    // Derived from FELogConnectorData
REGISTER_FECORE_CLASS(FELogRigidConnectorForceX , FENLCLOGDATA_ID, "RCFx");
REGISTER_FECORE_CLASS(FELogRigidConnectorForceY , FENLCLOGDATA_ID, "RCFy");
REGISTER_FECORE_CLASS(FELogRigidConnectorForceZ , FENLCLOGDATA_ID, "RCFz");
REGISTER_FECORE_CLASS(FELogRigidConnectorMomentX, FENLCLOGDATA_ID, "RCMx");
REGISTER_FECORE_CLASS(FELogRigidConnectorMomentY, FENLCLOGDATA_ID, "RCMy");
REGISTER_FECORE_CLASS(FELogRigidConnectorMomentZ, FENLCLOGDATA_ID, "RCMz");
}

//-----------------------------------------------------------------------------
// Derived from FELogNLConstraintData
REGISTER_FECORE_CLASS(FELogVolumeConstraint , FENLCLOGDATA_ID, "constrained volume");
REGISTER_FECORE_CLASS(FELogVolumePressure   , FENLCLOGDATA_ID, "volume pressure"   );

