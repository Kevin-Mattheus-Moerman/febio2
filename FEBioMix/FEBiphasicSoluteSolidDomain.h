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
#include "FEBiphasicSolute.h"
#include "FEBiphasicSoluteDomain.h"

//-----------------------------------------------------------------------------
//! Domain class for biphasic-solute 3D solid elements
//! Note that this class inherits from FEElasticSolidDomain since this domain
//! also needs to calculate elastic stiffness contributions.
//!
class FEBiphasicSoluteSolidDomain : public FESolidDomain, public FEBiphasicSoluteDomain
{
public:
    //! constructor
    FEBiphasicSoluteSolidDomain(FEModel* pfem);
    
    //! reset domain data
    void Reset() override;
    
    //! get the material (overridden from FEDomain)
    FEMaterial* GetMaterial() override { return m_pMat; }
    
    //! set the material
    void SetMaterial(FEMaterial* pmat) override;
    
    //! Unpack solid element data (overridden from FEDomain)
    void UnpackLM(FEElement& el, vector<int>& lm) override;
    
    //! Activate
    void Activate() override;
    
    //! initialize material points in the domain
    void InitMaterialPoints() override;
    
    //! initialize elements for this domain
    void PreSolveUpdate(const FETimeInfo& timeInfo) override;
    
    // update domain data
    void Update(const FETimeInfo& tp) override;
    
    // update element stress
    void UpdateElementStress(int iel);
    
public:
    // internal work (overridden from FEElasticDomain)
    void InternalForces(FEGlobalVector& R) override;
    
    // internal work (steady-state analyses)
    void InternalForcesSS(FEGlobalVector& R) override;
    
public:
    //! calculates the global stiffness matrix for this domain
    void StiffnessMatrix(FESolver* psolver, bool bsymm) override;
    
    //! calculates the global stiffness matrix for this domain (steady-state case)
    void StiffnessMatrixSS(FESolver* psolver, bool bsymm) override;
    
protected:
    //! element internal force vector
    void ElementInternalForce(FESolidElement& el, vector<double>& fe);
    
    //! element internal force vector (steady-state analyses)
    void ElementInternalForceSS(FESolidElement& el, vector<double>& fe);
    
    //! calculates the element solute-poroelastic stiffness matrix
    bool ElementBiphasicSoluteStiffness(FESolidElement& el, matrix& ke, bool bsymm);
    
    //! calculates the element solute-poroelastic stiffness matrix
    bool ElementBiphasicSoluteStiffnessSS(FESolidElement& el, matrix& ke, bool bsymm);
    
protected: // overridden from FEElasticDomain, but not implemented in this domain
    void BodyForce(FEGlobalVector& R, FEBodyForce& bf) override {}
    void InertialForces(FEGlobalVector& R, vector<double>& F) override {}
    void StiffnessMatrix(FESolver* psolver) override {}
    void BodyForceStiffness(FESolver* psolver, FEBodyForce& bf) override {}
    void MassMatrix(FESolver* psolver, double scale) override {}
};
