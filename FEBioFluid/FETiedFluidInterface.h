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
#include "FEBioMech/FEContactInterface.h"
#include "FEBioMech/FEContactSurface.h"
#include "FEFluid.h"

//-----------------------------------------------------------------------------
class FETiedFluidSurface : public FEContactSurface
{
public:
    //! Integration point data
    class Data
    {
    public:
        Data();
        
    public:
        vec3d   m_Gap;      //!< initial gap in reference configuration
        vec3d   m_vg;       //!< tangential velocity gap function at integration points
        vec3d   m_nu;       //!< normal at integration points
        vec2d   m_rs;       //!< natural coordinates of projection of integration point
        vec3d   m_Lmd;      //!< lagrange multipliers for tangential velocity
        vec3d   m_tv;       //!< viscous tangential traction
        double  m_Lmp;      //!< lagrange multipliers for fluid pressures
        double  m_epst;     //!< viscous traction penalty factor
        double  m_epsn;     //!< normal velocity penalty factor
        double  m_pg;       //!< pressure "gap"
        double  m_vn;       //!< normal fluid velocity gap
        FESurfaceElement*    m_pme;    //!< master element of projected integration point
    };
    
    //! constructor
    FETiedFluidSurface(FEModel* pfem);
    
    //! initialization
    bool Init();
    
    void Serialize(DumpStream& ar);
    
    //! Unpack surface element data
    void UnpackLM(FEElement& el, vector<int>& lm);
    
public:
    void GetVelocityGap     (int nface, vec3d& vg);
    void GetPressureGap     (int nface, double& pg);
    void GetViscousTraction (int nface, vec3d& tv);
    void GetNormalVelocity  (int nface, double& vn);

protected:
    FEModel*    m_pfem;
    
public:
    vector< vector<Data> >  m_Data;    //!< integration point data
    
    int             m_dofWX, m_dofWY, m_dofWZ;
    int             m_dofEF;
};

//-----------------------------------------------------------------------------
class FETiedFluidInterface :    public FEContactInterface
{
public:
    //! constructor
    FETiedFluidInterface(FEModel* pfem);
    
    //! destructor
    ~FETiedFluidInterface();
    
    //! initialization
    bool Init() override;
    
    //! interface activation
    void Activate() override;
    
    //! serialize data to archive
    void Serialize(DumpStream& ar) override;
    
    //! return the master and slave surface
    FESurface* GetMasterSurface() override { return &m_ms; }
    FESurface* GetSlaveSurface () override { return &m_ss; }
    
    //! return integration rule class
    bool UseNodalIntegration() override { return false; }
    
    //! build the matrix profile for use in the stiffness matrix
    void BuildMatrixProfile(FEGlobalMatrix& K) override;
    
public:
    //! calculate contact forces
    void Residual(FEGlobalVector& R, const FETimeInfo& tp) override;
    
    //! calculate contact stiffness
    void StiffnessMatrix(FESolver* psolver, const FETimeInfo& tp) override;
    
    //! calculate Lagrangian augmentations
    bool Augment(int naug, const FETimeInfo& tp) override;
    
    //! update
    void Update(int niter, const FETimeInfo& tp) override;
    
protected:
    void InitialProjection(FETiedFluidSurface& ss, FETiedFluidSurface& ms);
    void ProjectSurface(FETiedFluidSurface& ss, FETiedFluidSurface& ms);
    
    //! calculate penalty factor
    void CalcAutoPressurePenalty(FETiedFluidSurface& s);
    double AutoPressurePenalty(FESurfaceElement& el, FETiedFluidSurface& s);
    
public:
    FETiedFluidSurface    m_ms;    //!< master surface
    FETiedFluidSurface    m_ss;    //!< slave surface
    
    bool            m_btwo_pass;    //!< two-pass flag
    double          m_atol;         //!< augmentation tolerance
    double          m_gtol;         //!< gap tolerance
    double          m_ptol;         //!< pressure gap tolerance
    double          m_stol;         //!< search tolerance
    double          m_srad;         //!< contact search radius
    int             m_naugmax;      //!< maximum nr of augmentations
    int             m_naugmin;      //!< minimum nr of augmentations
    
    double          m_epst;         //!< tangential viscous traction penalty factor
    double          m_epsn;         //!< normal fluid velocity penalty factor
    bool            m_bautopen;     //!< use autopenalty factor
    
    FEFluid*        m_pfluid;       //!< fluid pointer
    
    int             m_dofWX, m_dofWY, m_dofWZ;
    int             m_dofEF;
    
    DECLARE_PARAMETER_LIST();
};
