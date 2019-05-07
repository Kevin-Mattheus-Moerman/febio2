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
#include "FECore/FESurfaceLoad.h"
#include <FECore/FESurfaceMap.h>
#include "FEFluid.h"

//-----------------------------------------------------------------------------
//! FEFluidResistanceBC is a fluid surface that has a normal
//! pressure proportional to the flow rate (resistance).
//!
class FEFluidResistanceBC : public FESurfaceLoad
{
public:
    //! constructor
    FEFluidResistanceBC(FEModel* pfem);
    
    //! Set the surface to apply the load to
    void SetSurface(FESurface* ps) override;
    
    //! calculate traction stiffness (there is none)
    void StiffnessMatrix(const FETimeInfo& tp, FESolver* psolver) override {}
    
    //! calculate residual
    void Residual(const FETimeInfo& tp, FEGlobalVector& R) override { m_alpha = tp.alpha; m_alphaf = tp.alphaf; }
    
    //! set the dilatation
    void Update() override;
    
    //! evaluate flow rate
    double FlowRate();
    
    //! initialize
    bool Init() override;
    
    //! activate
    void Activate() override;

	//! serialize
	void Serialize(DumpStream& ar);

private:
    double			m_R;        //!< flow resistance
    double          m_alpha;
    double          m_alphaf;
    double          m_p0;       //!< fluid pressure offset
    FEFluid*        m_pfluid;   //!< pointer to fluid
    
    int		m_dofWX, m_dofWY, m_dofWZ;
    int		m_dofWXP, m_dofWYP, m_dofWZP;
    int		m_dofEF;
    
    DECLARE_PARAMETER_LIST();
};
