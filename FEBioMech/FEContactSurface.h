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

#include "FECore/FESurface.h"
#include "FECore/vec2d.h"
#include "FEBioMech/FEContactInterface.h"

//-----------------------------------------------------------------------------
//! This class describes a contact slave or master surface

//!	this class is used in contact analyses to describe a contacting
//! surface in a contact interface.

class FEContactSurface : public FESurface
{
public:
	//! constructor
	FEContactSurface(FEModel* pfem);

	//! destructor
	~FEContactSurface();

	// initialization
	bool Init() override;

	//! Set the sibling of this contact surface
	void SetSibling(FEContactSurface* ps);

    //! Set the parent of this contact surface
    void SetContactInterface(FEContactInterface* ps);
    
    //! Get the parent of this contact surface
    FEContactInterface* GetContactInterface() { return m_pContactInterface; }
    
	//! Unpack surface element data
	void UnpackLM(FEElement& el, vector<int>& lm) override;

public:
    virtual void GetContactGap     (int nface, double& pg);
    virtual void GetVectorGap      (int nface, vec3d& pg);
    virtual void GetContactPressure(int nface, double& pg);
    virtual void GetContactTraction(int nface, vec3d& pt);
    
	virtual void GetNodalContactGap     (int nface, double* pg);
    virtual void GetNodalVectorGap      (int nface, vec3d* pg);
	virtual void GetNodalContactPressure(int nface, double* pg);
	virtual void GetNodalContactTraction(int nface, vec3d* pt);

    virtual void GetStickStatus(int nface, double& pt);
    
    void GetSurfaceTraction(int nface, vec3d& pt);
    void GetNodalSurfaceTraction(int nface, vec3d* pt);
    void GetGPSurfaceTraction(int nface, vec3d* pt);

	virtual vec3d GetContactForce();
    virtual double GetContactArea();

	FEModel* GetFEModel() { return m_pfem; }

protected:
	FEContactSurface* m_pSibling;
    FEContactInterface* m_pContactInterface;
	FEModel*	m_pfem;

	int	m_dofX;
	int	m_dofY;
	int	m_dofZ;
};
