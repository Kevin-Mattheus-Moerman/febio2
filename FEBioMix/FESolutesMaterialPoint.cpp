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
#include "FESolutesMaterialPoint.h"
#include "FECore/DumpStream.h"

//=============================================================================
//   FESolutesMaterialPoint
//=============================================================================


//-----------------------------------------------------------------------------
//! Create a shallow copy of the material point data
FEMaterialPoint* FESolutesMaterialPoint::Copy()
{
	FESolutesMaterialPoint* pt = new FESolutesMaterialPoint(*this);
	if (m_pNext) pt->m_pNext = m_pNext->Copy();
	return pt;
}

//-----------------------------------------------------------------------------
//! Initialize material point data
void FESolutesMaterialPoint::Init()
{
	m_nsol = m_nsbm = 0;
	m_psi = m_cF = 0;
	m_Ie = vec3d(0,0,0);
	m_rhor = 0;
    m_c.clear();
    m_gradc.clear();
    m_j.clear();
    m_ca.clear();
    m_crp.clear();
    m_sbmr.clear();
    m_sbmrp.clear();
    m_sbmrhat.clear();
    m_sbmrhatp.clear();
    m_sbmrmin.clear();
    m_sbmrmax.clear();
    m_k.clear();
    m_dkdJ.clear();
    m_dkdJJ.clear();
    m_dkdc.clear();
    m_dkdJc.clear();
    m_dkdcc.clear();
    m_dkdr.clear();
    m_dkdJr.clear();
    m_dkdrc.clear();
    m_cri.clear();
    m_crd.clear();
    m_strain = 0;
    m_pe = m_pi = 0;
    m_ce.clear();
    m_ci.clear();
    m_ide.clear();
    m_idi.clear();
    
	// don't forget to initialize the base class
    FEMaterialPoint::Init();
}

//-----------------------------------------------------------------------------
//! Serialize material point data to the archive
void FESolutesMaterialPoint::Serialize(DumpStream& ar)
{
    if (ar.IsShallow())
    {
        if (ar.IsSaving())
        {
            ar << m_nsol << m_psi << m_cF << m_Ie << m_nsbm;
            for (int i=0; i<m_nsol; ++i) {
                ar << m_c[i] << m_gradc[i] << m_j[i] << m_ca[i]
                << m_crp[i] << m_k[i] << m_dkdJ[i];
                for (int j=0; j<m_nsol; ++j)
                    ar << m_dkdc[i][j];
            }
            for (int i=0; i<m_nsbm; ++i)
            ar << m_sbmr[i] << m_sbmrp[i] << m_sbmrhat[i] << m_sbmrhatp[i];
            for (int i=0; i<m_cri.size(); ++i)
                ar << m_cri[i];
            for (int i=0; i<m_crd.size(); ++i)
                ar << m_crd[i];
            ar << m_strain << m_pe << m_pi;
            for (int i=0; i<m_ce.size(); ++i)
                ar << m_ce[i];
            for (int i=0; i<m_ci.size(); ++i)
                ar << m_ci[i];
        }
        else
        {
            ar >> m_nsol >> m_psi >> m_cF >> m_Ie >> m_nsbm;
            for (int i=0; i<m_nsol; ++i) {
                ar >> m_c[i] >> m_gradc[i] >> m_j[i] >> m_ca[i]
                >> m_crp[i] >> m_k[i] >> m_dkdJ[i];
                for (int j=0; j<m_nsol; ++j)
                    ar >> m_dkdc[i][j];
            }
            for (int i=0; i<m_nsbm; ++i)
                ar >> m_sbmr[i] >> m_sbmrp[i] >> m_sbmrhat[i] >> m_sbmrhatp[i];
            for (int i=0; i<m_cri.size(); ++i)
                ar >> m_cri[i];
            for (int i=0; i<m_crd.size(); ++i)
                ar >> m_crd[i];
            ar >> m_strain >> m_pe >> m_pi;
            for (int i=0; i<m_ce.size(); ++i)
                ar >> m_ce[i];
            for (int i=0; i<m_ci.size(); ++i)
                ar >> m_ci[i];
        }
    }
    else
    {
        if (ar.IsSaving())
        {
            ar << m_nsol << m_psi << m_cF << m_Ie << m_nsbm;
            for (int i=0; i<m_nsol; ++i) {
                ar << m_c[i] << m_gradc[i] << m_j[i] << m_ca[i]
                << m_crp[i] << m_k[i] << m_dkdJ[i];
                for (int j=0; j<m_nsol; ++j)
                    ar << m_dkdc[i][j];
            }
            for (int i=0; i<m_nsbm; ++i)
                ar << m_sbmr[i] << m_sbmrp[i] << m_sbmrhat[i] << m_sbmrhatp[i];
            int ncri = (int)m_cri.size();
            ar << ncri;
            for (int i=0; i<ncri; ++i)
                ar << m_cri[i];
            int ncrd = (int)m_crd.size();
            ar << ncrd;
            for (int i=0; i<ncrd; ++i)
                ar << m_crd[i];
            int nse = (int)m_ce.size();
            int nsi = (int)m_ci.size();
            ar << nse << nsi << m_strain << m_pe << m_pi;
            for (int i=0; i<m_ce.size(); ++i)
                ar << m_ce[i] << m_ide[i];
            for (int i=0; i<m_ci.size(); ++i)
                ar << m_ci[i] << m_idi[i];
        }
        else
        {
            ar >> m_nsol >> m_psi >> m_cF >> m_Ie >> m_nsbm;
            
            m_c.resize(m_nsol);
            m_gradc.resize(m_nsol);
            m_j.resize(m_nsol);
            m_ca.resize(m_nsol);
            m_crp.resize(m_nsol);
            m_k.resize(m_nsol);
            m_dkdJ.resize(m_nsol);
            m_dkdc.resize(m_nsol);
            
            for (int i=0; i<m_nsol; ++i) {
                ar >> m_c[i] >> m_gradc[i] >> m_j[i] >> m_ca[i]
                >> m_crp[i] >> m_k[i] >> m_dkdJ[i];
                
                m_dkdc[i].resize(m_nsol);
                for (int j=0; j<m_nsol; ++j)
                {
                    ar >> m_dkdc[i][j];
                }
            }
            
            if (m_nsbm)
            {
                m_sbmr.resize(m_nsbm);
                m_sbmrp.resize(m_nsbm);
                m_sbmrhat.resize(m_nsbm);
                m_sbmrhatp.resize(m_nsbm);
                
                for (int i=0; i<m_nsbm; ++i)
                    ar >> m_sbmr[i] >> m_sbmrp[i] >> m_sbmrhat[i] >> m_sbmrhatp[i];
            }
            
            int ncri;
            ar >> ncri;
            m_cri.resize(ncri);
            for (int i=0; i<ncri; ++i)
                ar >> m_cri[i];
            
            int ncrd;
            ar >> ncrd;
            m_crd.resize(ncrd);
            for (int i=0; i<ncrd; ++i)
                ar >> m_crd[i];
            
            int nse, nsi;
            ar >> nse >> nsi >> m_strain >> m_pe >> m_pi;
            m_ce.resize(nse);
            m_ci.resize(nsi);
            for (int i=0; i<m_ce.size(); ++i)
                ar >> m_ce[i] >> m_ide[i];
            for (int i=0; i<m_ci.size(); ++i)
                ar >> m_ci[i] >> m_idi[i];
        }
    }
    
    FEMaterialPoint::Serialize(ar);
}
