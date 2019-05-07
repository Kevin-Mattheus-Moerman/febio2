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

//-----------------------------------------------------------------------------
class FESlidingSurfaceBiphasic : public FEBiphasicContactSurface
{
public:
    //! Integration point data
    class Data
    {
    public:
        Data();
        
    public:
        double	m_gap;      //!< normal gap function
        vec3d   m_dg;       //!< vector gap
        double	m_Lmd;      //!< Lagrange multipliers for normal traction
        vec3d   m_Lmt;      //!< Lagrange multipliers for vector traction
        double	m_Lmp;      //!< lagrange multipliers for fluid pressures
        double	m_Ln;       //!< net contact pressure
        double	m_epsn;     //!< penalty factor
        double	m_epsp;     //!< pressure penalty factor
        double	m_pg;       //!< pressure "gap" for biphasic contact
        double  m_p1;       //!< fluid pressure
        double  m_mueff;    //!< effective friction coefficient
        vec3d	m_nu;       //!< normal at integration points
        vec3d   m_s1;       //!< tangent along slip direction
        vec3d   m_tr;       //!< contact traction
        vec2d	m_rs;       //!< natural coordinates of projection
        vec2d   m_rsp;      //!< m_rs at the previous time step
        bool    m_bstick;   //!< stick flag
        FESurfaceElement*	m_pme;	//!< master element
        FESurfaceElement*   m_pmep; //!< m_pme at the previous time step
    };
    
public:
    //! constructor
    FESlidingSurfaceBiphasic(FEModel* pfem);
    
    //! initialization
    bool Init();
    
    // data serialization
    void Serialize(DumpStream& ar);
    
    //! initialize sliding surface and store previous values
    void InitSlidingSurface();
    
    //! evaluate net contact force
    vec3d GetContactForce();
    
    //! evaluate net contact area
    double GetContactArea();
    
    //! evaluate net fluid force
    vec3d GetFluidForce();
    
    //! calculate the nodal normals
    void UpdateNodeNormals();
    
    void SetPoroMode(bool bporo) { m_bporo = bporo; }
    
public:
    void GetContactGap     (int nface, double& pg);
    void GetVectorGap      (int nface, vec3d& pg);
    void GetContactPressure(int nface, double& pg);
    void GetContactTraction(int nface, vec3d& pt);
    void GetSlipTangent    (int nface, vec3d& pt);
    void GetMuEffective    (int nface, double& pg);
    //    void GetSolidSolidLoad (int nface, double& pg);
    void GetNodalContactGap     (int nface, double* pg);
    void GetNodalVectorGap      (int nface, vec3d* pg);
    void GetNodalContactPressure(int nface, double* pg);
    void GetNodalContactTraction(int nface, vec3d* pt);
    void GetNodalPressureGap    (int nface, double* pg);
    void GetStickStatus(int nface, double& pg);
    void EvaluateNodalContactPressures();
    void EvaluateNodalContactTractions();
    
protected:
    FEModel*	m_pfem;
    
public:
    bool	m_bporo;	//!< set poro-mode
    
    vector< vector<Data> >	m_Data;	//!< integration point data
    vector<bool>		m_poro;	//!< surface element poro status
    vector<vec3d>		m_nn;	//!< node normals
    vector<vec3d>       m_tn;   //!< nodal contact tractions
    vector<double>      m_pn;   //!< nodal contact pressures
    
    vec3d    m_Ft;     //!< total contact force (from equivalent nodal forces)
};

//-----------------------------------------------------------------------------
class FESlidingInterfaceBiphasic :	public FEContactInterface
{
public:
    //! constructor
    FESlidingInterfaceBiphasic(FEModel* pfem);
    
    //! destructor
    ~FESlidingInterfaceBiphasic();
    
    //! initialization
    bool Init() override;
    
    //! interface activation
    void Activate() override;
    
    //! calculate the slip direction on the primary surface
    vec3d SlipTangent(FESlidingSurfaceBiphasic& ss, const int nel, const int nint, FESlidingSurfaceBiphasic& ms, double& dh, vec3d& r);
    
    //! calculate contact traction
    vec3d ContactTraction(FESlidingSurfaceBiphasic& ss, const int nel, const int n, FESlidingSurfaceBiphasic& ms, double& pn);
    
    //! calculate contact pressures for file output
    void UpdateContactPressures();
    
    //! serialize data to archive
    void Serialize(DumpStream& ar) override;
    
    //! mark free-draining condition
    void MarkFreeDraining();
    
    //! set free-draining condition
    void SetFreeDraining();
    
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
    void ProjectSurface(FESlidingSurfaceBiphasic& ss, FESlidingSurfaceBiphasic& ms, bool bupseg, bool bmove = false);
    
    //! calculate penalty factor
    void CalcAutoPenalty(FESlidingSurfaceBiphasic& s);
    
    void CalcAutoPressurePenalty(FESlidingSurfaceBiphasic& s);
    double AutoPressurePenalty(FESurfaceElement& el, FESlidingSurfaceBiphasic& s);
    
public:
    FESlidingSurfaceBiphasic	m_ms;	//!< master surface
    FESlidingSurfaceBiphasic	m_ss;	//!< slave surface
    
    int				m_knmult;		//!< higher order stiffness multiplier
    bool			m_btwo_pass;	//!< two-pass flag
    double			m_atol;			//!< augmentation tolerance
    double			m_gtol;			//!< gap tolerance
    double			m_ptol;			//!< pressure gap tolerance
    double			m_stol;			//!< search tolerance
    bool			m_bsymm;		//!< use symmetric stiffness components only
    double			m_srad;			//!< contact search radius
    int				m_naugmax;		//!< maximum nr of augmentations
    int				m_naugmin;		//!< minimum nr of augmentations
    int				m_nsegup;		//!< segment update parameter
    bool			m_breloc;		//!< node relocation on startup
    bool            m_bsmaug;       //!< smooth augmentation
    
    double			m_epsn;		    //!< normal penalty factor
    bool			m_bautopen;	    //!< use autopenalty factor
    bool            m_bupdtpen;     //!< update penalty at each time step
    
    double          m_mu;           //!< friction coefficient
    bool            m_bfreeze;      //!< freeze stick/slip status
    
    // biphasic contact parameters
    double	        m_epsp;		    //!< flow rate penalty
    double          m_phi;          //!< solid-solid contact fraction
    
protected:
    int	m_dofP;
    
    DECLARE_PARAMETER_LIST();
};
