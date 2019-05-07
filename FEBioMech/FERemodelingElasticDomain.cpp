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
#include "FERemodelingElasticDomain.h"
#include "FERemodelingElasticMaterial.h"
#include <FECore/FEModel.h>
#include "FECore/FEAnalysis.h"

//-----------------------------------------------------------------------------
//! constructor
FERemodelingElasticDomain::FERemodelingElasticDomain(FEModel* pfem) : FEElasticSolidDomain(pfem)
{
}

//-----------------------------------------------------------------------------
// Reset data
void FERemodelingElasticDomain::Reset()
{
	FEElasticSolidDomain::Reset();

	FEElasticMaterial* pme = m_pMat->GetElasticMaterial();
	for (size_t i=0; i<m_Elem.size(); ++i)
	{
		FESolidElement& el = m_Elem[i];
		int n = el.GaussPoints();
		for (int j=0; j<n; ++j) {
			FERemodelingMaterialPoint& pt = *el.GetMaterialPoint(j)->ExtractData<FERemodelingMaterialPoint>();
			pt.m_rhor = pme->Density();
		}
	}
}

//-----------------------------------------------------------------------------
//! \todo The material point initialization needs to move to the base class.
bool FERemodelingElasticDomain::Init()
{
	// initialize base class
	if (FEElasticSolidDomain::Init() == false) return false;

	// get the elements material
	FEElasticMaterial* pme = m_pMat->GetElasticMaterial();
	for (size_t i=0; i<m_Elem.size(); ++i)
	{
		FESolidElement& el = m_Elem[i];

		// initialize referential solid density
		for (int n=0; n<el.GaussPoints(); ++n)
		{
			FERemodelingMaterialPoint& pt = *el.GetMaterialPoint(n)->ExtractData<FERemodelingMaterialPoint>();
			pt.m_rhor = pme->Density();
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
//! calculates the global stiffness matrix for this domain
void FERemodelingElasticDomain::StiffnessMatrix(FESolver* psolver)
{
	// repeat over all solid elements
	int NE = (int)m_Elem.size();

	// I only need this for the element density stiffness
	FEModel& fem = psolver->GetFEModel();
	double dt = fem.GetTime().timeIncrement;

	#pragma omp parallel for
	for (int iel=0; iel<NE; ++iel)
	{
		// element stiffness matrix
		matrix ke;
		vector<int> lm;
		
		FESolidElement& el = m_Elem[iel];

		// create the element's stiffness matrix
		int ndof = 3*el.Nodes();
		ke.resize(ndof, ndof);
		ke.zero();

		// calculate geometrical stiffness
		ElementGeometricalStiffness(el, ke);

		// calculate material stiffness
		ElementMaterialStiffness(el, ke);

		// calculate density stiffness
		ElementDensityStiffness(dt, el, ke);

		// assign symmetic parts
		// TODO: Can this be omitted by changing the Assemble routine so that it only
		// grabs elements from the upper diagonal matrix?
		for (int i=0; i<ndof; ++i)
			for (int j=i+1; j<ndof; ++j)
				ke[j][i] = ke[i][j];

		// get the element's LM vector
		UnpackLM(el, lm);

		// assemble element matrix in global stiffness matrix
		#pragma omp critical
		psolver->AssembleStiffness(el.m_node, lm, ke);
	}
}

//-----------------------------------------------------------------------------
//! calculates the solid element stiffness matrix
void FERemodelingElasticDomain::ElementStiffness(const FETimeInfo& tp, int iel, matrix& ke)
{
	FESolidElement& el = Element(iel);

	// calculate material stiffness (i.e. constitutive component)
	ElementMaterialStiffness(el, ke);

	// calculate geometrical stiffness
	ElementGeometricalStiffness(el, ke);

	// calculate density stiffness
	ElementDensityStiffness(tp.timeIncrement, el, ke);

	// assign symmetic parts
	// TODO: Can this be omitted by changing the Assemble routine so that it only
	// grabs elements from the upper diagonal matrix?
	int ndof = 3*el.Nodes();
	for (int i=0; i<ndof; ++i)
		for (int j=i+1; j<ndof; ++j)
			ke[j][i] = ke[i][j];
}


//-----------------------------------------------------------------------------
//! calculates element's density stiffness component for integration point n
//! \todo Remove the FEModel parameter. We only need to dt parameter, not the entire model.
//! \todo Problems seem to run better without this stiffness matrix
void FERemodelingElasticDomain::ElementDensityStiffness(double dt, FESolidElement &el, matrix &ke)
{
	int n, i, j;

	// make sure this is a remodeling material
	FERemodelingElasticMaterial* pmat = dynamic_cast<FERemodelingElasticMaterial*>(m_pMat); assert(pmat);
    
    const int NE = FEElement::MAX_NODES;
    vec3d gradN[NE];
    double *Grn, *Gsn, *Gtn;
    double Gr, Gs, Gt;
    vec3d kru, kur;
        
    // nr of nodes
    int neln = el.Nodes();
        
    // nr of integration points
    int nint = el.GaussPoints();
        
    // jacobian
    double Ji[3][3], detJt;
        
    // weights at gauss points
    const double *gw = el.GaussWeights();
        
    // density stiffness component for the stiffness matrix
    mat3d kab;
        
    // calculate geometrical element stiffness matrix
    for (n=0; n<nint; ++n)
    {
        // calculate jacobian
        double J = invjact(el, Ji, n);
        detJt = J*gw[n];
            
        Grn = el.Gr(n);
        Gsn = el.Gs(n);
        Gtn = el.Gt(n);
            
        for (i=0; i<neln; ++i)
        {
            Gr = Grn[i];
            Gs = Gsn[i];
            Gt = Gtn[i];
                
            // calculate global gradient of shape functions
            // note that we need the transposed of Ji, not Ji itself !
            gradN[i].x = Ji[0][0]*Gr+Ji[1][0]*Gs+Ji[2][0]*Gt;
            gradN[i].y = Ji[0][1]*Gr+Ji[1][1]*Gs+Ji[2][1]*Gt;
            gradN[i].z = Ji[0][2]*Gr+Ji[1][2]*Gs+Ji[2][2]*Gt;
        }
            
        // get the material point data
        FEMaterialPoint& mp = *el.GetMaterialPoint(n);
        double drhohat = pmat->m_pSupp->Tangent_Supply_Density(mp);
        mat3ds ruhat = pmat->m_pSupp->Tangent_Supply_Strain(mp);
        mat3ds crho = pmat->Tangent_Stress_Density(mp);
        double krr = (drhohat - 1./dt)/J;
            
        for (i=0; i<neln; ++i) {
            kur = (crho*gradN[i])/krr;
            for (j=0; j<neln; ++j)
            {
                kru = ruhat*gradN[j];
                kab = kur & kru;
                ke[3*i  ][3*j  ] -= kab(0,0)*detJt;
                ke[3*i  ][3*j+1] -= kab(0,1)*detJt;
                ke[3*i  ][3*j+2] -= kab(0,2)*detJt;
                    
                ke[3*i+1][3*j  ] -= kab(1,0)*detJt;
                ke[3*i+1][3*j+1] -= kab(1,1)*detJt;
                ke[3*i+1][3*j+2] -= kab(1,2)*detJt;
                    
                ke[3*i+2][3*j  ] -= kab(2,0)*detJt;
                ke[3*i+2][3*j+1] -= kab(2,1)*detJt;
                ke[3*i+2][3*j+2] -= kab(2,2)*detJt;
            }
        }
    }
}
