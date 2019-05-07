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
#include <FECore/FESurfaceLoad.h>
#include <FECore/FESurfaceMap.h>

//-----------------------------------------------------------------------------
//! FETractionLoad is a surface that has a constant (deformation independant)
//! traction force on it.
//!
class FETractionLoad : public FESurfaceLoad
{
public:
	//! constructor
	FETractionLoad(FEModel* pfem);

	//! Set the surface to apply the load to
	void SetSurface(FESurface* ps) override;

	//! calculate traction stiffness (there is none)
	void StiffnessMatrix(const FETimeInfo& tp, FESolver* psolver) override {}

	//! calculate residual
	void Residual(const FETimeInfo& tp, FEGlobalVector& R) override;

	//! Unpack surface element data
	void UnpackLM(FEElement& el, vector<int>& lm);

private:
	double			m_scale;	//!< scale factor for traction load
	FESurfaceMap	m_TC;		//!< traction loads

protected:
	// degrees of freedom
	// (TODO: find a better way of defining this. 
	//        I don't want to have to do this in each class)
	int	m_dofX;
	int	m_dofY;
	int	m_dofZ;

	DECLARE_PARAMETER_LIST();
};
