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
#include "mat2d.h"
#include "vec2d.h"
#include "FEDomain.h"

//-----------------------------------------------------------------------------
class FEMesh;
class FENodeSet;
class FEFacetSet;

//-----------------------------------------------------------------------------
//! Surface mesh

//! This class implements the basic functionality for an FE surface.
//! More specialized surfaces are derived from this class

class FECORE_API FESurface : public FEDomain
{
public:
	//! constructor
	FESurface(FEMesh* pm);

	//! destructor
	virtual ~FESurface(){}

	//! initialize surface data structure
	virtual bool Init() override;
    
	//! creates surface
	void Create(int nsize, int elemType = -1) override;

	//! Build a surface from a facet set
	void BuildFromSet(FEFacetSet& set);

	//! serialization
	void Serialize(DumpStream& ar) override;

	//! Unpack surface element data
	//! TODO: This is obsolete. Remove this.
	void UnpackLM(FEElement& el, vector<int>& lm) override;
	
	//! Extract a node set from this surface
	FENodeSet GetNodeSet();
    
    //! Set alpha parameter for intermediate time
    void SetAlpha(const double alpha) { m_alpha = alpha; }

public:

	//! return number of surface elements
	int Elements() const override { return (int)m_el.size(); }

	//! return an element of the surface
	FESurfaceElement& Element(int i) { return m_el[i]; }

	//! return an element of the surface
	const FESurfaceElement& Element(int i) const { return m_el[i]; }

	//! returns reference to element
	FEElement& ElementRef(int n) override { return m_el[n]; }

	//! find the index of a surface element
	int FindElement(FESurfaceElement& el);

    //! for interface surfaces, find the index of both solid elements
    //! on either side of the interface
    void FindElements(FESurfaceElement& el);

public:

	//! Project a node onto a surface element
	vec3d ProjectToSurface(FESurfaceElement& el, vec3d x, double& r, double& s);

	//! check to see if a point is on element
	bool IsInsideElement(FESurfaceElement& el, double r, double s, double tol = 0);

	//! See if a ray intersects an element
	bool Intersect(FESurfaceElement& el, vec3d r, vec3d n, double rs[2], double& g, double eps);

	//! Invert the surface
	void Invert();

	//! Get the spatial position given natural coordinates
	vec3d Position(FESurfaceElement& el, double r, double s);

public:
	//! calculate the surface area of a surface element
	double FaceArea(FESurfaceElement& el);

	//! return the max element size
	double MaxElementSize();

	//! calculate the metric tensor in the current configuration
	mat2d Metric(FESurfaceElement& el, double r, double s);

    //! calculate the metric tensor at an integration point
    mat2d Metric(FESurfaceElement& el, int n);
    
    //! calculate the metric tensor at an integration point at previous time
    mat2d MetricP(FESurfaceElement& el, int n);
    
	//! calculate the metric tensor in the reference configuration
	mat2d Metric0(FESurfaceElement& el, double r, double s);

	//! calculate the surface normal
	vec3d SurfaceNormal(FESurfaceElement& el, double r, double s);

	//! calculate the surface normal at an integration point
	vec3d SurfaceNormal(FESurfaceElement& el, int n);

	//! calculate the global position of a point on the surface
	vec3d Local2Global(FESurfaceElement& el, double r, double s);

	//! calculate the global position of an integration point
	vec3d Local2Global(FESurfaceElement& el, int n);

    //! calculate the global position of a point on the surface at previous time
    vec3d Local2GlobalP(FESurfaceElement& el, double r, double s);
    
    //! calculate the global position of an integration point at previous time
    vec3d Local2GlobalP(FESurfaceElement& el, int n);
    
	//! calculates the covariant base vectors of a surface at an integration point
	void CoBaseVectors(FESurfaceElement& el, int j, vec3d t[2]);

	//! calculates the covariant base vectors of a surface
	void CoBaseVectors(FESurfaceElement& el, double r, double s, vec3d t[2]);

	//! calculates covariant base vectors of a surface
	void CoBaseVectors0(FESurfaceElement& el, double r, double s, vec3d t[2]);

    //! calculates the covariant base vectors of a surface at an integration point at previoust time step
    void CoBaseVectorsP(FESurfaceElement& el, int j, vec3d t[2]);
    
    //! calculates contravariant base vectors of a surface  at an integration point
    void ContraBaseVectors(FESurfaceElement& el, int j, vec3d t[2]);
    
    //! calculates the contravariant base vectors of a surface at an integration point at previoust time step
    void ContraBaseVectorsP(FESurfaceElement& el, int j, vec3d t[2]);
    
	//! calculates contravariant base vectors of a surface
	void ContraBaseVectors(FESurfaceElement& el, double r, double s, vec3d t[2]);

	//! calculates contravariant base vectors of a surface
	void ContraBaseVectors0(FESurfaceElement& el, double r, double s, vec3d t[2]);

	//! Jacobian in reference configuration for integration point n
	double jac0(FESurfaceElement& el, int n);

	//! Jacobian in reference configuration for integration point n (and returns normal)
	double jac0(const FESurfaceElement& el, int n, vec3d& nu);

    //! Interface status
    void SetInterfaceStatus(const bool bitfc) { m_bitfc = bitfc; }
    bool GetInterfaceStatus() { return m_bitfc; }
    
protected:
	vector<FESurfaceElement>	m_el;	//!< surface elements
    bool                        m_bitfc;    //!< interface status
    double                      m_alpha;    //!< intermediate time fraction
};
