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

//-----------------------------------------------------------------------------
//! The pressure surface is a surface domain that sustains pressure boundary
//! conditions
//!
class FEPressureLoad : public FESurfaceLoad
{
public:
	//! constructor
	FEPressureLoad(FEModel* pfem);

	//! Set the surface to apply the load to
	void SetSurface(FESurface* ps) override;

	//! calculate pressure stiffness
	void StiffnessMatrix(const FETimeInfo& tp, FESolver* psolver) override;

	//! calculate residual
	void Residual(const FETimeInfo& tp, FEGlobalVector& R) override;

	//! serialize data
	void Serialize(DumpStream& ar) override;

	//! set the linear flag
	void SetLinear(bool blinear) { m_blinear = blinear; }

	//! Check if this is a linear force or not
	bool IsLinear() { return m_blinear; }

	//! Unpack surface element data
	void UnpackLM(FEElement& el, vector<int>& lm);

protected:
	//! calculate stiffness for an element
	void PressureStiffness(FESurfaceElement& el, matrix& ke, vector<double>& tn);

	void SymmetricPressureStiffness(FESurfaceElement& el, matrix& ke, vector<double>& tn);
	void UnsymmetricPressureStiffness(FESurfaceElement& el, matrix& ke, vector<double>& tn);

	//! Calculates external pressure forces
	void PressureForce(FESurfaceElement& el, vector<double>& fe, vector<double>& tn);

	//! Calculates the linear external pressure forces (ie. non-follower forces)
	void LinearPressureForce(FESurfaceElement& el, vector<double>& fe, vector<double>& tn);

protected:
	bool			m_blinear;	//!< pressure load type (linear or nonlinear)
    bool            m_bshellb; //!< flag for prescribing pressure on shell bottom
	double			m_pressure;	//!< pressure value
	bool			m_bsymm;	//!< use symmetric formulation
	bool			m_bstiff;	//!< use stiffness or not
	FESurfaceMap	m_PC;		//!< pressure scale factors

	// degrees of freedom
	// (TODO: find a better way of defining this. 
	//        I don't want to have to do this in each class)
	int	m_dofX;
	int	m_dofY;
	int	m_dofZ;
    int	m_dofSX;
    int	m_dofSY;
    int	m_dofSZ;

	DECLARE_PARAMETER_LIST();
};
