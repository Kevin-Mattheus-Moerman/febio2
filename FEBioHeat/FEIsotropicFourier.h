#pragma once
#include "FEHeatTransferMaterial.h"

//-----------------------------------------------------------------------------
// Isotropic Fourer heat-transfer material
class FEIsotropicFourier : public FEHeatTransferMaterial
{
public:
	FEIsotropicFourier(FEModel* pfem) : FEHeatTransferMaterial(pfem) {}

public:
	double	m_k;	//!< heat conductivity
	double	m_c;	//!< heat capacitance
	double	m_rho;	//!< density

public:
	//! get the material's conductivity tensor
	mat3ds Conductivity(FEMaterialPoint& mp) override;

	//! get the material's capacitance
	double Capacitance() override { return m_c; }

	//! get the material's density
	double Density() override { return m_rho; }

	// declare parameter list
	DECLARE_PARAMETER_LIST();
};
