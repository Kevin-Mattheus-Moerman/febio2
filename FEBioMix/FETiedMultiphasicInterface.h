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
#include "FEBiphasicContactSurface.h"
#include "FESolute.h"

//-----------------------------------------------------------------------------
class FETiedMultiphasicSurface : public FEBiphasicContactSurface
{
public:
    //! Integration point data
    class Data
    {
    public:
        Data();
        
    public:
        vec3d	m_Gap;	//!< initial gap in reference configuration
        vec3d	m_dg;	//!< gap function at integration points
        vec3d	m_nu;	//!< normal at integration points
        vec2d	m_rs;	//!< natural coordinates of projection of integration point
        vec3d	m_Lmd;	//!< lagrange multipliers for displacements
        double	m_Lmp;	//!< lagrange multipliers for fluid pressures
        double	m_epsn;	//!< penalty factors
        double	m_epsp;	//!< pressure penalty factors
        double	m_pg;	//!< pressure "gap"
        vector<double>	m_Lmc;	//!< Lagrange multipliers for solute concentrations
        vector<double>	m_epsc;	//!< concentration penatly factors
        vector<double>	m_cg;	//!< concentration "gap"
        FESurfaceElement*	m_pme;	//!< master element of projected integration point
    };
    
    //! constructor
    FETiedMultiphasicSurface(FEModel* pfem);
    
    //! initialization
    bool Init();
    
    //! calculate the nodal normals
    void UpdateNodeNormals();
    
    void Serialize(DumpStream& ar);
    
    void SetPoroMode(bool bporo) { m_bporo = bporo; }
    
    void UnpackLM(FEElement& el, vector<int>& lm);
    
protected:
    FEModel*	m_pfem;
    
public:
    bool                    m_bporo;	//!< set poro-mode
    bool					m_bsolu;	//!< set solute-mode
    
    vector< vector<Data> >  m_Data;	//!< integration point data
    vector<bool>			m_poro;	//!< surface element poro status
    vector<vec3d>			m_nn;	//!< node normals
    vector<int>				m_sid;	//!< list of solute id's for this surface
    
protected:
    int	m_dofC;
};

//-----------------------------------------------------------------------------
class FETiedMultiphasicInterface :	public FEContactInterface
{
public:
    //! constructor
    FETiedMultiphasicInterface(FEModel* pfem);
    
    //! destructor
    ~FETiedMultiphasicInterface();
    
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
    void InitialProjection(FETiedMultiphasicSurface& ss, FETiedMultiphasicSurface& ms);
    void ProjectSurface(FETiedMultiphasicSurface& ss, FETiedMultiphasicSurface& ms);
    
    //! calculate penalty factor
    void CalcAutoPenalty(FETiedMultiphasicSurface& s);
    
    void CalcAutoPressurePenalty(FETiedMultiphasicSurface& s);
    double AutoPressurePenalty(FESurfaceElement& el, FETiedMultiphasicSurface& s);
    
    void CalcAutoConcentrationPenalty(FETiedMultiphasicSurface& s, const int isol);
    double AutoConcentrationPenalty(FESurfaceElement& el, FETiedMultiphasicSurface& s, const int isol);
    
    double AutoPenalty(FESurfaceElement& el, FESurface &s);
    
    //! get solute data
    FESoluteData* FindSoluteData(int nid);
    
public:
    FETiedMultiphasicSurface	m_ms;	//!< master surface
    FETiedMultiphasicSurface	m_ss;	//!< slave surface
    
    int				m_knmult;		//!< higher order stiffness multiplier
    bool			m_btwo_pass;	//!< two-pass flag
    double			m_atol;			//!< augmentation tolerance
    double			m_gtol;			//!< gap tolerance
    double			m_ptol;			//!< pressure gap tolerance
    double			m_ctol;			//!< concentration gap tolerance
    double			m_stol;			//!< search tolerance
    bool			m_bsymm;		//!< use symmetric stiffness components only
    double			m_srad;			//!< contact search radius
    int				m_naugmax;		//!< maximum nr of augmentations
    int				m_naugmin;		//!< minimum nr of augmentations
    
    double			m_epsn;			//!< normal penalty factor
    bool			m_bautopen;		//!< use autopenalty factor
    
    // multiphasic contact parameters
    double			m_epsp;         //!< fluid flow rate penalty
    double          m_epsc;			//!< solute molar flow rate penalty
    double          m_Rgas;			//!< universal gas constant
    double          m_Tabs;			//!< absolute temperature
    vector<int> m_sid;				//!< list of solute ids common to both contact surfaces
    vector<int> m_ssl;				//!< list of slave surface solutes common to both contact surfaces
    vector<int> m_msl;				//!< list of master surface solutes common to both contact surfaces
    vector<int> m_sz;               //!< charge number of solutes common to both contact surfaces
    
protected:
    int	m_dofP;
    int	m_dofC;
    
    DECLARE_PARAMETER_LIST();
};
