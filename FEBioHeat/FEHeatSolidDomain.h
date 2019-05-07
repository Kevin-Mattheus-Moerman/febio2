#pragma once
#include <FECore/FESolidDomain.h>
#include <FECore/FESolver.h>
#include <FECore/FELinearSystem.h>
#include "FEHeatTransferMaterial.h"

class FEModel;

//-----------------------------------------------------------------------------
class FEHeatDomain
{
public:
	FEHeatDomain(FEModel* pfem) : m_pfem(pfem) {}
	virtual ~FEHeatDomain(){}
	virtual void ConductionMatrix (FELinearSystem& ls) = 0;
	virtual void CapacitanceMatrix(FELinearSystem& ls, double dt) = 0;

	FEModel* GetFEModel() { return m_pfem; }

protected:
	FEModel* m_pfem;
};

//-----------------------------------------------------------------------------
//! domain class for 3D heat elements
class FEHeatSolidDomain : public FESolidDomain, public FEHeatDomain
{
	enum { DOF_T = 0 };

public:
	//! constructor
	FEHeatSolidDomain(FEModel* pfem);

	//! get the material (overridden from FEDomain)
	FEMaterial* GetMaterial() { return m_pMat; }

	//! set the material
	void SetMaterial(FEMaterial* pmat);

	//! Update state data
	void Update(const FETimeInfo& tp);

public: // overloaded from FEHeatDomain

	//! Calculate the conduction stiffness 
	void ConductionMatrix(FELinearSystem& ls);

	//! Calculate capacitance stiffness matrix
	void CapacitanceMatrix(FELinearSystem& ls, double dt);

protected:
	//! calculate the conductive element stiffness matrix
	void ElementConduction(FESolidElement& el, matrix& ke);

	//! calculate the capacitance element stiffness matrix
	void ElementCapacitance(FESolidElement& el, matrix& ke, double dt);

protected:
	FEHeatTransferMaterial*	m_pMat;
};
