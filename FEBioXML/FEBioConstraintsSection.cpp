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
#include "FEBioConstraintsSection.h"
#include "FEBioMech/FERigidMaterial.h"
#include "FEBioMech/FEPointConstraint.h"
#include "FEBioMech/FERigidForce.h"
#include "FECore/FEModel.h"
#include "FECore/FECoreKernel.h"
#include <FECore/FERigidSystem.h>
#include <FECore/RigidBC.h>
#include <FEBioMech/FEDiscreteContact.h>
#include <FECore/FESurfaceConstraint.h>

void FEBioConstraintsSection1x::Parse(XMLTag &tag)
{
	// make sure there is something to read
	if (tag.isleaf()) return;

	FEModel& fem = *GetFEModel();
	FEMesh& m = fem.GetMesh();

	++tag;
	do
	{
		if (tag == "rigid_body")
		{
			ParseRigidConstraint(tag);
		}
		else if (tag == "constraint")
		{
			const char* sztype = tag.AttributeValue("type", true);
			if (sztype == 0)
			{
				// check the name attribute
				const char* szname = tag.AttributeValue("name");
				if (szname == 0) throw XMLReader::InvalidAttributeValue(tag, "name", "(unknown)");

				// make sure this is a leaf
				if (tag.isempty() == false) throw XMLReader::InvalidValue(tag);

				// see if we can find this constraint
				FEModel& fem = *GetFEModel();
				int NLC = fem.NonlinearConstraints();
				FENLConstraint* plc = 0;
				for (int i = 0; i<NLC; ++i)
				{
					FENLConstraint* pci = fem.NonlinearConstraint(i);
					if (pci->GetName() == szname) { plc = pci; }
				}
				if (plc == 0) throw XMLReader::InvalidAttributeValue(tag, "name", szname);

				// add this boundary condition to the current step
				GetBuilder()->AddComponent(plc);
			}
			else
			{
				FENLConstraint* plc = fecore_new<FENLConstraint>(FENLCONSTRAINT_ID, sztype, GetFEModel());
				if (plc == 0) throw XMLReader::InvalidAttributeValue(tag, "type", sztype);

				const char* szname = tag.AttributeValue("name", true);
				if (szname) plc->SetName(szname);

				FEParameterList& pl = plc->GetParameterList();

				++tag;
				do
				{
					if (ReadParameter(tag, pl) == false)
					{
						if (tag == "surface")
						{
							FESurfaceConstraint* psc = dynamic_cast<FESurfaceConstraint*>(plc);
							if (psc == 0) throw XMLReader::InvalidTag(tag);

							const char* sztype = tag.AttributeValue("type", true);
							FESurface* psurf = psc->GetSurface();
							if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, "type", sztype);

							m.AddSurface(psurf);

							// see if the set attribute is defined
							const char* szset = tag.AttributeValue("set", true);
							if (szset)
							{
								// make sure this tag does not have any children
								if (!tag.isleaf()) throw XMLReader::InvalidTag(tag);

								// see if we can find the facet set
								FEFacetSet* pset = 0;
								for (int i = 0; i<m.FacetSets(); ++i)
								{
									FEFacetSet& fi = m.FacetSet(i);
									if (strcmp(fi.GetName(), szset) == 0)
									{
										pset = &fi;
										break;
									}
								}

								// create a surface from the facet set
								if (pset)
								{
									if (GetBuilder()->BuildSurface(*psurf, *pset, true) == false) throw XMLReader::InvalidTag(tag);
								}
								else throw XMLReader::InvalidAttributeValue(tag, "set", szset);
							}
							else ParseSurfaceSection(tag, *psurf, 0, true);
						}
						else throw XMLReader::InvalidTag(tag);
					}
					++tag;
				} while (!tag.isend());

				// add this boundary condition to the current step
				GetBuilder()->AddNonlinearConstraint(plc);
			}
		}
		else throw XMLReader::InvalidTag(tag);
		++tag;
	} while (!tag.isend());
}

void FEBioConstraintsSection2::Parse(XMLTag &tag)
{
	// make sure there is something to read
	if (tag.isleaf()) return;

	FEModel& fem = *GetFEModel();
	FEMesh& m = fem.GetMesh();

	++tag;
	do
	{
		if (tag == "rigid_body") 
		{
			ParseRigidConstraint20(tag);
		}
		else if (tag == "constraint")
		{
			const char* sztype = tag.AttributeValue("type", true);
			if (sztype == 0)
			{
				// check the name attribute
				const char* szname = tag.AttributeValue("name");
				if (szname == 0) throw XMLReader::InvalidAttributeValue(tag, "name", "(unknown)");

				// make sure this is a leaf
				if (tag.isempty() == false) throw XMLReader::InvalidValue(tag);

				// see if we can find this constraint
				FEModel& fem = *GetFEModel();
				int NLC = fem.NonlinearConstraints();
				FENLConstraint* plc = 0;
				for (int i=0; i<NLC; ++i)
				{
					FENLConstraint* pci = fem.NonlinearConstraint(i);
					if (pci->GetName() == szname) { plc = pci; }
				}
				if (plc == 0) throw XMLReader::InvalidAttributeValue(tag, "name", szname);

				// add this boundary condition to the current step
				GetBuilder()->AddComponent(plc);
			}
			else
			{
				FENLConstraint* plc = fecore_new<FENLConstraint>(FENLCONSTRAINT_ID, sztype, GetFEModel());
				if (plc == 0) throw XMLReader::InvalidAttributeValue(tag, "type", sztype);

				const char* szname = tag.AttributeValue("name", true);
				if (szname) plc->SetName(szname);

				FEParameterList& pl = plc->GetParameterList();

				++tag;
				do
				{
					if (ReadParameter(tag, pl) == false)
					{
						if (tag == "surface")
						{
							FESurfaceConstraint* psc = dynamic_cast<FESurfaceConstraint*>(plc);
							if (psc == 0) throw XMLReader::InvalidTag(tag);

							FESurface* psurf = psc->GetSurface();
							if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, "type", sztype);

							m.AddSurface(psurf);

							// see if the set attribute is defined
							const char* szset = tag.AttributeValue("set", true);
							if (szset)
							{
								// make sure this tag does not have any children
								if (!tag.isleaf()) throw XMLReader::InvalidTag(tag);

								// see if we can find the facet set
								FEFacetSet* pset = 0;
								for (int i=0; i<m.FacetSets(); ++i)
								{
									FEFacetSet& fi = m.FacetSet(i);
									if (strcmp(fi.GetName(), szset) == 0)
									{
										pset = &fi;
										break;
									}
								}

								// create a surface from the facet set
								if (pset)
								{
									if (GetBuilder()->BuildSurface(*psurf, *pset, true) == false) throw XMLReader::InvalidTag(tag);
								}
								else throw XMLReader::InvalidAttributeValue(tag, "set", szset);
							}
							else ParseSurfaceSection(tag, *psurf, 0, true);
						}
						else throw XMLReader::InvalidTag(tag);
					}
					++tag;
				}
				while (!tag.isend());

				// add this boundary condition to the current step
				GetBuilder()->AddNonlinearConstraint(plc);
			}
		}
		else throw XMLReader::InvalidTag(tag);
		++tag;
	}
	while (!tag.isend());
}

void FEBioConstraintsSection25::Parse(XMLTag &tag)
{
	// make sure there is something to read
	if (tag.isleaf()) return;

	FEModel& fem = *GetFEModel();
	FEMesh& mesh = fem.GetMesh();

	++tag;
	do
	{
		if (tag == "constraint")
		{
			const char* sztype = tag.AttributeValue("type", true);
			if (sztype == 0)
			{
				// check the name attribute
				const char* szname = tag.AttributeValue("name");
				if (szname == 0) throw XMLReader::InvalidAttributeValue(tag, "name", "(unknown)");

				// make sure this is a leaf
				if (tag.isempty() == false) throw XMLReader::InvalidValue(tag);

				// see if we can find this constraint
				FEModel& fem = *GetFEModel();
				int NLC = fem.NonlinearConstraints();
				FENLConstraint* plc = 0;
				for (int i=0; i<NLC; ++i)
				{
					FENLConstraint* pci = fem.NonlinearConstraint(i);
					if (pci->GetName() == szname) { plc = pci; }
				}
				if (plc == 0) throw XMLReader::InvalidAttributeValue(tag, "name", szname);

				// add this boundary condition to the current step
				GetBuilder()->AddComponent(plc);
			}
			else
			{
				FENLConstraint* plc = fecore_new<FENLConstraint>(FENLCONSTRAINT_ID, sztype, GetFEModel());
				if (plc == 0) throw XMLReader::InvalidAttributeValue(tag, "type", sztype);

				const char* szname = tag.AttributeValue("name", true);
				if (szname) plc->SetName(szname);

				// get the surface
				// Note that not all constraints define a surface
				FESurfaceConstraint* psc = dynamic_cast<FESurfaceConstraint*>(plc);
				if (psc && psc->GetSurface())
				{
					FESurface* psurf = psc->GetSurface();
					mesh.AddSurface(psurf);
					const char* szsurf = tag.AttributeValue("surface");
					FEFacetSet* pface = mesh.FindFacetSet(szsurf);
					if (pface == 0) throw XMLReader::InvalidAttributeValue(tag, "surface", szsurf);
					if (GetBuilder()->BuildSurface(*psurf, *pface, true) == false) throw XMLReader::InvalidAttributeValue(tag, "surface", szsurf);
				}

				// get the nodeset (this is needed by FEDiscreteContact)
				if (dynamic_cast<FEDiscreteContact*>(plc))
				{
					FEDiscreteContact* pdc = dynamic_cast<FEDiscreteContact*>(plc);
					const char* szdset = tag.AttributeValue("discrete_set");
					FEDiscreteSet* pset = mesh.FindDiscreteSet(szdset);
					if (pset == 0) throw XMLReader::InvalidAttributeValue(tag, "discrete_set", szdset);
					pdc->SetDiscreteSet(pset);
				}

				// get the nodeset (this is needed by FEDiscreteContact2)
				if (dynamic_cast<FEDiscreteContact2*>(plc))
				{
					FEDiscreteContact2* pdc = dynamic_cast<FEDiscreteContact2*>(plc);
					const char* szdset = tag.AttributeValue("discrete_set");
					FEDeformableSpringDomain2* pdom = dynamic_cast<FEDeformableSpringDomain2*>(mesh.FindDomain(szdset));
					if (pdom == 0) throw XMLReader::InvalidAttributeValue(tag, "discrete_set", szdset);
					pdc->SetDiscreteDomain(pdom);
				}

				// read the parameter list
				ReadParameterList(tag, plc);

				// add this constraint to the current step
				GetBuilder()->AddNonlinearConstraint(plc);
			}
		}
		else throw XMLReader::InvalidTag(tag);
		++tag;
	}
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioConstraintsSection1x::ParseRigidConstraint(XMLTag& tag)
{
	FEModel& fem = *GetFEModel();

	const char* szm = tag.AttributeValue("mat");
	assert(szm);

	// get the material ID
	int nmat = atoi(szm);
	if ((nmat <= 0) || (nmat > fem.Materials())) throw XMLReader::InvalidAttributeValue(tag, "mat", szm);

	// make sure this is a valid rigid material
	FEMaterial* pm = fem.GetMaterial(nmat-1);
	if (pm->IsRigid() == false) throw XMLReader::InvalidAttributeValue(tag, "mat", szm);

	++tag;
	do
	{
		if (strncmp(tag.Name(), "trans_", 6) == 0)
		{
			const char* szt = tag.AttributeValue("type");

			int bc = -1;
			if      (tag.Name()[6] == 'x') bc = 0;
			else if (tag.Name()[6] == 'y') bc = 1;
			else if (tag.Name()[6] == 'z') bc = 2;
			assert(bc >= 0);
			
			if (strcmp(szt, "prescribed") == 0)
			{
				const char* szlc = tag.AttributeValue("lc");
				int lc = atoi(szlc) - 1;

				bool brel = false;
				const char* szrel = tag.AttributeValue("relative", true);
				if (szrel)
				{
					if      (strcmp(szrel, "true" ) == 0) brel = true;
					else if (strcmp(szrel, "false") == 0) brel = false;
					else throw XMLReader::InvalidAttributeValue(tag, "relative", szrel);
				}

				FERigidBodyDisplacement* pDC = static_cast<FERigidBodyDisplacement*>(fecore_new<FEBoundaryCondition>(FEBC_ID, "rigid_prescribed", &fem));
				pDC->id = nmat;
				pDC->bc = bc;
				pDC->lc = lc;
				pDC->brel = brel;
				tag.value(pDC->sf);

				// add this boundary condition to the current step
				GetBuilder()->AddRigidPrescribedBC(pDC);
			}
			else if (strcmp(szt, "force") == 0)
			{
				const char* szlc = tag.AttributeValue("lc");
				int lc = atoi(szlc) - 1;

				FERigidBodyForce* pFC = static_cast<FERigidBodyForce*>(fecore_new<FEModelLoad>(FEBC_ID, "rigid_force",  &fem));
				pFC->id = nmat;
				pFC->bc = bc;
				pFC->lc = lc;
				tag.value(pFC->sf);

				// add this boundary condition to the current step
				GetBuilder()->AddModelLoad(pFC);
			}
			else if (strcmp(szt, "fixed") == 0)
			{
				FERigidBodyFixedBC* pBC = static_cast<FERigidBodyFixedBC*>(fecore_new<FEBoundaryCondition>(FEBC_ID, "rigid_fixed",  &fem));
				pBC->id = nmat;
				pBC->bc = bc;

				// add this boundary condition to the current step
				GetBuilder()->AddRigidFixedBC(pBC);
			}
			else throw XMLReader::InvalidAttributeValue(tag, "type", szt);
		}
		else if (strncmp(tag.Name(), "rot_", 4) == 0)
		{
			const char* szt = tag.AttributeValue("type");

			int bc = -1;
			if      (tag.Name()[4] == 'x') bc = 3;
			else if (tag.Name()[4] == 'y') bc = 4;
			else if (tag.Name()[4] == 'z') bc = 5;
			assert(bc >= 0);

			if (strcmp(szt, "prescribed") == 0)
			{
				const char* szlc = tag.AttributeValue("lc");
				int lc = atoi(szlc) - 1;

				FERigidBodyDisplacement* pDC = static_cast<FERigidBodyDisplacement*>(fecore_new<FEBoundaryCondition>(FEBC_ID, "rigid_prescribed", &fem));
				pDC->id = nmat;
				pDC->bc = bc;
				pDC->lc = lc;
				tag.value(pDC->sf);

				// add this boundary condition to the current step
				GetBuilder()->AddRigidPrescribedBC(pDC);
			}
			else if (strcmp(szt, "force") == 0)
			{
				const char* szlc = tag.AttributeValue("lc");
				int lc = atoi(szlc) - 1;

				FERigidBodyForce* pFC = static_cast<FERigidBodyForce*>(fecore_new<FEModelLoad>(FEBC_ID, "rigid_force",  &fem));
				pFC->id = nmat;
				pFC->bc = bc;
				pFC->lc = lc;
				tag.value(pFC->sf);

				// add this boundary condition to the current step
				GetBuilder()->AddModelLoad(pFC);
			}
			else if (strcmp(szt, "fixed") == 0)
			{
				FERigidBodyFixedBC* pBC = static_cast<FERigidBodyFixedBC*>(fecore_new<FEBoundaryCondition>(FEBC_ID, "rigid_fixed",  &fem));
				pBC->id = nmat;
				pBC->bc = bc;

				// add this boundary condition to the current step
				GetBuilder()->AddRigidFixedBC(pBC);
			}
			else throw XMLReader::InvalidAttributeValue(tag, "type", szt);
		}
		else throw XMLReader::InvalidTag(tag);
		++tag;
	}
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioConstraintsSection2::ParseRigidConstraint20(XMLTag& tag)
{
	FEModel& fem = *GetFEModel();

	const char* szm = tag.AttributeValue("mat");
	assert(szm);

	// get the material ID
	int nmat = atoi(szm);
	if ((nmat <= 0) || (nmat > fem.Materials())) throw XMLReader::InvalidAttributeValue(tag, "mat", szm);

	// make sure this is a valid rigid material
	FEMaterial* pm = fem.GetMaterial(nmat-1);
	if (pm->IsRigid() == false) throw XMLReader::InvalidAttributeValue(tag, "mat", szm);

	++tag;
	do
	{
		if (tag == "prescribed")
		{
			// get the dof
			int bc = -1;
			const char* szbc = tag.AttributeValue("bc");
			if      (strcmp(szbc, "x") == 0) bc = 0;
			else if (strcmp(szbc, "y") == 0) bc = 1;
			else if (strcmp(szbc, "z") == 0) bc = 2;
			else if (strcmp(szbc, "Rx") == 0) bc = 3;
			else if (strcmp(szbc, "Ry") == 0) bc = 4;
			else if (strcmp(szbc, "Rz") == 0) bc = 5;
			else throw XMLReader::InvalidAttributeValue(tag, "bc", szbc);

			// get the loadcurve
			const char* szlc = tag.AttributeValue("lc");
			int lc = atoi(szlc) - 1;

			// get the (optional) type attribute
			bool brel = false;
			const char* szrel = tag.AttributeValue("type", true);
			if (szrel)
			{
				if      (strcmp(szrel, "relative" ) == 0) brel = true;
				else if (strcmp(szrel, "absolute" ) == 0) brel = false;
				else throw XMLReader::InvalidAttributeValue(tag, "type", szrel);
			}

			// create the rigid displacement constraint
			FERigidBodyDisplacement* pDC = static_cast<FERigidBodyDisplacement*>(fecore_new<FEBoundaryCondition>(FEBC_ID, "rigid_prescribed", &fem));
			pDC->id = nmat;
			pDC->bc = bc;
			pDC->lc = lc;
			pDC->brel = brel;
			value(tag, pDC->sf);

			// add this boundary condition to the current step
			GetBuilder()->AddRigidPrescribedBC(pDC);
		}
		else if (tag == "force")
		{
			// get the dof
			int bc = -1;
			const char* szbc = tag.AttributeValue("bc");
			if      (strcmp(szbc, "x") == 0) bc = 0;
			else if (strcmp(szbc, "y") == 0) bc = 1;
			else if (strcmp(szbc, "z") == 0) bc = 2;
			else if (strcmp(szbc, "Rx") == 0) bc = 3;
			else if (strcmp(szbc, "Ry") == 0) bc = 4;
			else if (strcmp(szbc, "Rz") == 0) bc = 5;
			else throw XMLReader::InvalidAttributeValue(tag, "bc", szbc);

			// get the type
			int ntype = 0;
			bool bfollow = false;
			const char* sztype = tag.AttributeValue("type", true);
			if (sztype)
			{
				if (strcmp(sztype, "ramp") == 0) ntype = 1;
				else if (strcmp(sztype, "follow") == 0) bfollow = true;
				else throw XMLReader::InvalidAttributeValue(tag, "type", sztype);
			}

			// get the loadcurve
			const char* szlc = tag.AttributeValue("lc", true);
			int lc = -1;
			if (szlc) lc = atoi(szlc) - 1;

			// make sure there is a loadcurve for type=0 forces
			if ((ntype == 0)&&(lc==-1)) throw XMLReader::MissingAttribute(tag, "lc");

			// create the rigid body force
			FERigidBodyForce* pFC = static_cast<FERigidBodyForce*>(fecore_new<FEModelLoad>(FEBC_ID, "rigid_force",  &fem));
			pFC->m_ntype = ntype;
			pFC->id = nmat;
			pFC->bc = bc;
			pFC->lc = lc;
			pFC->m_bfollow = bfollow;
			value(tag, pFC->sf);

			// add this boundary condition to the current step
			GetBuilder()->AddModelLoad(pFC);
		}
		else if (tag == "fixed")
		{
			// get the dof
			int bc = -1;
			const char* szbc = tag.AttributeValue("bc");
			if      (strcmp(szbc, "x") == 0) bc = 0;
			else if (strcmp(szbc, "y") == 0) bc = 1;
			else if (strcmp(szbc, "z") == 0) bc = 2;
			else if (strcmp(szbc, "Rx") == 0) bc = 3;
			else if (strcmp(szbc, "Ry") == 0) bc = 4;
			else if (strcmp(szbc, "Rz") == 0) bc = 5;
			else throw XMLReader::InvalidAttributeValue(tag, "bc", szbc);

			// create the fixed dof
			FERigidBodyFixedBC* pBC = static_cast<FERigidBodyFixedBC*>(fecore_new<FEBoundaryCondition>(FEBC_ID, "rigid_fixed",  &fem));
			pBC->id = nmat;
			pBC->bc = bc;

			// add this boundary condition to the current step
			GetBuilder()->AddRigidFixedBC(pBC);
		}
		else if (tag == "initial_velocity")
		{
			// get the initial velocity
			vec3d v;
			value(tag, v);

			// create the initial condition
			FERigidBodyVelocity* pic = new FERigidBodyVelocity(&fem);
			pic->m_rid = nmat;
			pic->m_vel = v;

			// add this initial condition to the current step
			GetBuilder()->AddRigidBodyVelocity(pic);
		}
		else if (tag == "initial_angular_velocity")
		{
			// get the initial angular velocity
			vec3d w;
			value(tag, w);

			// create the initial condition
			FERigidBodyAngularVelocity* pic = new FERigidBodyAngularVelocity(&fem);
			pic->m_rid = nmat;
			pic->m_w = w;

			// add this initial condition to the current step
			GetBuilder()->AddRigidBodyAngularVelocity(pic);
		}
		else throw XMLReader::InvalidTag(tag);
		++tag;
	}
	while (!tag.isend());
}

//---------------------------------------------------------------------------------
// parse a surface section for contact definitions
//
bool FEBioConstraintsSection::ParseSurfaceSection(XMLTag &tag, FESurface& s, int nfmt, bool bnodal)
{
	FEModel& fem = *GetFEModel();
	FEMesh& m = fem.GetMesh();
	int NN = m.Nodes();

	// count nr of faces
	int faces = 0, N, nf[9];
	XMLTag t(tag); ++t;
	while (!t.isend()) { faces++; ++t; }

	// allocate storage for faces
	s.Create(faces);

	FEModelBuilder* feb = GetBuilder();

	// read faces
	++tag;
	for (int i=0; i<faces; ++i)
	{
		FESurfaceElement& el = s.Element(i);

		// set the element type/integration rule
		if (bnodal)
		{
			if      (tag == "quad4") el.SetType(FE_QUAD4NI);
			else if (tag == "tri3" ) el.SetType(FE_TRI3NI );
			else if (tag == "tri6" ) el.SetType(FE_TRI6NI);
            else if (tag == "quad8" ) el.SetType(FE_QUAD8NI);
            else if (tag == "quad9" ) el.SetType(FE_QUAD9NI);
			else throw XMLReader::InvalidTag(tag);
		}
		else
		{
			if      (tag == "quad4") el.SetType(FE_QUAD4G4);
			else if (tag == "tri3" ) el.SetType(feb->m_ntri3);
			else if (tag == "tri6" ) el.SetType(feb->m_ntri6);
			else if (tag == "tri7" ) el.SetType(feb->m_ntri7);
			else if (tag == "tri10") el.SetType(feb->m_ntri10);
			else if (tag == "quad8") el.SetType(FE_QUAD8G9);
			else if (tag == "quad9") el.SetType(FE_QUAD9G9);
			else throw XMLReader::InvalidTag(tag);
		}

		N = el.Nodes();

		if (nfmt == 0)
		{
			tag.value(nf, N);
			for (int j=0; j<N; ++j) 
			{
				int nid = nf[j]-1;
				if ((nid<0)||(nid>= NN)) throw XMLReader::InvalidValue(tag);
				el.m_node[j] = nid;
			}
		}
		else if (nfmt == 1)
		{
			tag.value(nf, 2);
			FEElement* pe = m.FindElementFromID(nf[0]);
			if (pe)
			{
				int ne[9];
				int nn = m.GetFace(*pe, nf[1]-1, ne);
				if (nn != N) throw XMLReader::InvalidValue(tag);
				for (int j=0; j<N; ++j) el.m_node[j] = ne[j];
				el.m_elem[0] = nf[0];
			}
			else throw XMLReader::InvalidValue(tag);
		}

		++tag;
	}
	return true;
}
