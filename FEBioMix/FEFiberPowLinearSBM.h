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
#include "FEBioMech/FEElasticMaterial.h"

//-----------------------------------------------------------------------------
//! Material class for single fiber, tension only
//! Power law - linear
//! Fiber modulus depends on SBM content

class FEFiberPowLinearSBM : public FEElasticMaterial
{
public:
    FEFiberPowLinearSBM(FEModel* pfem) : FEElasticMaterial(pfem) { m_thd = 0; m_phd = 90; m_sbm = 0; }
    
    //! Initialization
    bool Init() override;
    
    //! Cauchy stress
    mat3ds Stress(FEMaterialPoint& mp) override;
    
    // Spatial tangent
    tens4ds Tangent(FEMaterialPoint& mp) override;
    
    //! Strain energy density
    double StrainEnergyDensity(FEMaterialPoint& mp) override;
    
    //! return fiber modulus
    double FiberModulus(double rhor) { return m_E0*pow(rhor/m_rho0, m_g);}
    
    // declare the parameter list
    DECLARE_PARAMETER_LIST();
    
public:
    double	m_E0;		// fiber modulus E = E0*(rhor/rho0)^gamma
    double  m_lam0;     // stretch ratio at end of toe region
    double  m_beta;     // power law exponent in toe region
    double  m_rho0;     // rho0
    double  m_g;        // gamma
    int		m_sbm;      //!< global id of solid-bound molecule
    int		m_lsbm;     //!< local id of solid-bound molecule
    double	m_thd;		// theta angle for fiber orientation (local coordinates system)
    double	m_phd;		// phi angle for fiber orientation (local coordinates system)
    vec3d	m_n0;		// unit vector along fiber direction (local coordinate system)
};
