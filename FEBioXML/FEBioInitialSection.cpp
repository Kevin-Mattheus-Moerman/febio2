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
#include "FEBioInitialSection.h"
#include "FECore/FEModel.h"
#include "FECore/DOFS.h"
#include <FECore/FEInitialCondition.h>
#include <FECore/FECoreKernel.h>
#include <FECore/FEMaterial.h>

//-----------------------------------------------------------------------------
void FEBioInitialSection::Parse(XMLTag& tag)
{
	if (tag.isleaf()) return;

	FEModel& fem = *GetFEModel();
	FEMesh& mesh = fem.GetMesh();
	DOFS& dofs = fem.GetDOFS();

	// make sure we've read the nodes section
	if (mesh.Nodes() == 0) throw XMLReader::InvalidTag(tag);

	// read nodal data
	++tag;
	do
	{
		if (tag == "velocity")
		{
			const int dof_VX = dofs.GetDOF("vx");
			const int dof_VY = dofs.GetDOF("vy");
			const int dof_VZ = dofs.GetDOF("vz");
			FEInitialBCVec3D* pic = dynamic_cast<FEInitialBCVec3D*>(fecore_new<FEInitialCondition>(FEIC_ID, "init_bc_vec3d", &fem));
			pic->SetDOF(dof_VX, dof_VY, dof_VZ);

			// add it to the model
			GetBuilder()->AddInitialCondition(pic);

			++tag;
			do
			{
				if (tag == "node")
				{
					int nid = ReadNodeID(tag);
					vec3d v;
					value(tag, v);

					pic->Add(nid, v);
				}
				else throw XMLReader::InvalidTag(tag);
				++tag;
			}
			while (!tag.isend());
		}
		else if (tag == "ic")
		{
			const char* sztype = tag.AttributeValue("type");
			FEInitialCondition* pic = dynamic_cast<FEInitialCondition*>(fecore_new<FEInitialCondition>(FEIC_ID, sztype, &fem));

			if (tag.isleaf() == false)
			{
				FEParameterList& pl = pic->GetParameterList();
				++tag;
				do
				{
					if (ReadParameter(tag, pl) == false) throw XMLReader::InvalidTag(tag);
					++tag;
				}
				while (!tag.isend());
			}
			
			// add it to the model
			GetBuilder()->AddInitialCondition(pic);
		}
		else
		{
			// Get the degree of freedom
			int ndof = -1;
			if      (tag == "temperature"   ) ndof = dofs.GetDOF("T");
			else if (tag == "fluid_pressure") ndof = dofs.GetDOF("p");
            else if (tag == "shell_fluid_pressure") ndof = dofs.GetDOF("q");
            else if (tag == "dilatation"    ) ndof = dofs.GetDOF("ef");
			else if (tag == "concentration" )
			{
				// TODO: Add a check to make sure that a solute with this ID exists
				int isol = 0;
				const char* sz = tag.AttributeValue("sol", true);
				if (sz) isol = atoi(sz) - 1;

				ndof = dofs.GetDOF("concentration", isol);
			}
            else if (tag == "shell_concentration" )
            {
                // TODO: Add a check to make sure that a solute with this ID exists
                int isol = 0;
                const char* sz = tag.AttributeValue("sol", true);
                if (sz) isol = atoi(sz) - 1;
                
                ndof = dofs.GetDOF("shell concentration", isol);
            }
			else throw XMLReader::InvalidTag(tag);
			if (ndof == -1) throw XMLReader::InvalidTag(tag);

			// allocate initial condition
			FEInitialBC* pic = dynamic_cast<FEInitialBC*>(fecore_new<FEInitialCondition>(FEIC_ID, "init_bc", &fem));
			pic->SetDOF(ndof);

			// add it to the model
			GetBuilder()->AddInitialCondition(pic);

			// read the node list and values
			++tag;
			do
			{
				if (tag == "node")
				{
					int nid = ReadNodeID(tag);
					double p;
					value(tag, p);
					pic->Add(nid, p);
				}
				else throw XMLReader::InvalidTag(tag);
				++tag;
			}
			while (!tag.isend());
		}
		++tag;
	}
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioInitialSection25::Parse(XMLTag& tag)
{
	if (tag.isleaf()) return;

	FEModel& fem = *GetFEModel();
	FEMesh& mesh = fem.GetMesh();
	DOFS& dofs = fem.GetDOFS();

	// make sure we've read the nodes section
	if (mesh.Nodes() == 0) throw XMLReader::InvalidTag(tag);

	// read nodal data
	++tag;
	do
	{
		if (tag == "init")
		{
			// get the BC
			const char* sz = tag.AttributeValue("bc");
			int ndof = dofs.GetDOF(sz);
			if (ndof == -1) throw XMLReader::InvalidAttributeValue(tag, "bc", sz);

			// get the node set
			const char* szset = tag.AttributeValue("node_set");
			FENodeSet* pns = mesh.FindNodeSet(szset);
			if (pns == 0) throw XMLReader::InvalidTag(tag);

			// allocate initial condition
			FEInitialBC* pic = dynamic_cast<FEInitialBC*>(fecore_new<FEInitialCondition>(FEIC_ID, "init_bc", &fem));
			pic->SetDOF(ndof);
			pic->SetNodes(*pns);

			// add it to the model
			GetBuilder()->AddInitialCondition(pic);

			// read parameters
			ReadParameterList(tag, pic);
		}
		else if (tag == "ic")
		{
			const char* sztype = tag.AttributeValue("type");
			FEInitialCondition* pic = dynamic_cast<FEInitialCondition*>(fecore_new<FEInitialCondition>(FEIC_ID, sztype, &fem));

			ReadParameterList(tag, pic);
			
			// add it to the model
			GetBuilder()->AddInitialCondition(pic);
		}
		else if (tag == "rigid_body")
		{
			// get the material ID
			const char* szm = tag.AttributeValue("mat");
			int nmat = atoi(szm);
			if ((nmat <= 0) || (nmat > fem.Materials())) throw XMLReader::InvalidAttributeValue(tag, "mat", szm);

			// make sure this is a valid rigid material
			FEMaterial* pm = fem.GetMaterial(nmat - 1);
			if (pm->IsRigid() == false) throw XMLReader::InvalidAttributeValue(tag, "mat", szm);

			++tag;
			do
			{
				if (tag == "initial_velocity")
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

				++tag;
			}
			while (!tag.isend());
		}
		else
			throw XMLReader::InvalidTag(tag);
		++tag;
	}
	while (!tag.isend());
}
