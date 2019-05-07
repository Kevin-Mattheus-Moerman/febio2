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
#include "FEBioImport.h"
#include "FEBioParametersSection.h"
#include "FEBioIncludeSection.h"
#include "FEBioModuleSection.h"
#include "FEBioControlSection.h"
#include "FEBioGlobalsSection.h"
#include "FEBioMaterialSection.h"
#include "FEBioGeometrySection.h"
#include "FEBioBoundarySection.h"
#include "FEBioLoadsSection.h"
#include "FEBioContactSection.h"
#include "FEBioConstraintsSection.h"
#include "FEBioInitialSection.h"
#include "FEBioLoadDataSection.h"
#include "FEBioOutputSection.h"
#include "FEBioStepSection.h"
#include "FEBioDiscreteSection.h"
#include "FEBioMeshDataSection.h"
#include "FEBioCodeSection.h"
#include "FEBioRigidSection.h"
#include "FECore/DataStore.h"
#include "FECore/FEModel.h"
#include "FECore/FECoreKernel.h"
#include <FECore/FESurfaceMap.h>
#include <FECore/FEFunction1D.h>
#include <FECore/tens3d.h>
#include "FECore/DOFS.h"
#include <string.h>
#include <stdarg.h>
#include "xmltool.h"

FEBioFileSection::FEBioFileSection(FEBioImport* feb) : FEFileSection(feb) {}

FEBioImport* FEBioFileSection::GetFEBioImport() { return static_cast<FEBioImport*>(GetFileReader()); }

//-----------------------------------------------------------------------------
FEBioImport::InvalidVersion::InvalidVersion()
{
	SetErrorString("Invalid version");
}

//-----------------------------------------------------------------------------
FEBioImport::InvalidMaterial::InvalidMaterial(int nel)
{
	SetErrorString("Element %d has an invalid material type", nel);
}

//-----------------------------------------------------------------------------
FEBioImport::InvalidDomainType::InvalidDomainType()	
{
	SetErrorString("Invalid domain type");
}

//-----------------------------------------------------------------------------
FEBioImport::InvalidDomainMaterial::InvalidDomainMaterial()
{
	SetErrorString("Invalid domain material");
}

//-----------------------------------------------------------------------------
FEBioImport::FailedCreatingDomain::FailedCreatingDomain()
{
	SetErrorString("Failed creating domain");
}

//-----------------------------------------------------------------------------
FEBioImport::InvalidElementType::InvalidElementType()
{
	SetErrorString("Invalid element type\n");
}

//-----------------------------------------------------------------------------
FEBioImport::FailedLoadingPlugin::FailedLoadingPlugin(const char* szfile)
{
	SetErrorString("failed loading plugin %s\n", szfile);
}

//-----------------------------------------------------------------------------
FEBioImport::DuplicateMaterialSection::DuplicateMaterialSection()
{
	SetErrorString("Material section has already been defined");
}

//-----------------------------------------------------------------------------
FEBioImport::MissingMaterialProperty::MissingMaterialProperty(const std::string& matName, const char* szprop)
{
	SetErrorString("Material \"%s\" needs to have property \"%s\" defined", matName.c_str(), szprop);
}

//-----------------------------------------------------------------------------
FEBioImport::FailedAllocatingSolver::FailedAllocatingSolver(const char* sztype)
{
	SetErrorString("Failed allocating solver \"%s\"", sztype);
}

//-----------------------------------------------------------------------------
FEBioImport::DataGeneratorError::DataGeneratorError()
{
	SetErrorString("Error in data generation");
}

//-----------------------------------------------------------------------------
FEBioImport::FailedBuildingPart::FailedBuildingPart(const std::string& partName)
{
	SetErrorString("Failed building part %s", partName.c_str());
}

//-----------------------------------------------------------------------------
FEBioImport::PlotVariable::PlotVariable(const FEBioImport::PlotVariable& pv)
{
	strcpy(m_szvar, pv.m_szvar);
    strcpy(m_szdom, pv.m_szdom);
	m_item = pv.m_item;
}

FEBioImport::PlotVariable::PlotVariable(const std::string& var, vector<int>& item, const char* szdom)
{
    strcpy(m_szvar, var.c_str());
    m_item = item;
    strcpy(m_szdom, szdom);
}

//-----------------------------------------------------------------------------
FEBioImport::FEBioImport()
{
}

//-----------------------------------------------------------------------------
FEBioImport::~FEBioImport()
{
}

//-----------------------------------------------------------------------------
// Build the file section map based on the version number
void FEBioImport::BuildFileSectionMap(int nversion)
{
	// define the file structure
	m_map["Module"     ] = new FEBioModuleSection     (this);
	m_map["Control"    ] = new FEBioControlSection    (this);
	m_map["Material"   ] = new FEBioMaterialSection   (this);
	m_map["LoadData"   ] = new FEBioLoadDataSection   (this);
	m_map["Globals"    ] = new FEBioGlobalsSection    (this);
	m_map["Output"     ] = new FEBioOutputSection     (this);

	// older formats
	if (nversion < 0x0200)
	{
	    m_map["Geometry"   ] = new FEBioGeometrySection1x   (this);
		m_map["Boundary"   ] = new FEBioBoundarySection1x   (this);
		m_map["Loads"      ] = new FEBioLoadsSection1x      (this);
		m_map["Constraints"] = new FEBioConstraintsSection1x(this);
		m_map["Step"       ] = new FEBioStepSection         (this);
		m_map["Initial"    ] = new FEBioInitialSection      (this);
	}

	// version 2.0
	if (nversion == 0x0200)
	{
		m_map["Parameters" ] = new FEBioParametersSection  (this);
	    m_map["Geometry"   ] = new FEBioGeometrySection2   (this);
		m_map["Initial"    ] = new FEBioInitialSection     (this);
		m_map["Boundary"   ] = new FEBioBoundarySection2   (this);
		m_map["Loads"      ] = new FEBioLoadsSection2      (this);
		m_map["Include"    ] = new FEBioIncludeSection     (this);
		m_map["Contact"    ] = new FEBioContactSection2    (this);
		m_map["Discrete"   ] = new FEBioDiscreteSection    (this);
		m_map["Code"       ] = new FEBioCodeSection        (this); // added in FEBio 2.4 (experimental feature!)
		m_map["Constraints"] = new FEBioConstraintsSection2(this);
		m_map["Step"       ] = new FEBioStepSection2       (this);
	}

	// version 2.5
	if (nversion == 0x0205)
	{
		m_map["Parameters" ] = new FEBioParametersSection   (this);
	    m_map["Geometry"   ] = new FEBioGeometrySection25   (this);
		m_map["Include"    ] = new FEBioIncludeSection      (this);
		m_map["Initial"    ] = new FEBioInitialSection25    (this);
		m_map["Boundary"   ] = new FEBioBoundarySection25   (this);
		m_map["Loads"      ] = new FEBioLoadsSection25      (this);
		m_map["Contact"    ] = new FEBioContactSection25    (this);
		m_map["Discrete"   ] = new FEBioDiscreteSection25   (this);
		m_map["Constraints"] = new FEBioConstraintsSection25(this);
		m_map["Code"       ] = new FEBioCodeSection         (this); // added in FEBio 2.4 (experimental feature!)
		m_map["MeshData"   ] = new FEBioMeshDataSection     (this);
		m_map["Rigid"      ] = new FEBioRigidSection        (this); // added in FEBio 2.6 (experimental feature!)
		m_map["Step"       ] = new FEBioStepSection25       (this);
	}
}

//-----------------------------------------------------------------------------
bool FEBioImport::Parse(const char* szfile)
{
	FEModel& fem = *GetFEModel();

	// keep a pointer to the mesh
	m_pMesh = &fem.GetMesh();

	// intialize some variables
	m_szdmp[0] = 0;
	m_szlog[0] = 0;
	m_szplt[0] = 0;

	// plot output
	m_szplot_type[0] = 0;
	m_plot.clear();
	m_nplot_compression = 0;

	m_data.clear();

	// extract the path
	strcpy(m_szpath, szfile);
	char* ch = strrchr(m_szpath, '\\');
	if (ch==0) ch = strrchr(m_szpath, '/');
	if (ch==0) m_szpath[0] = 0; else *(ch+1)=0;

	// clean up
	ClearFileParams();
	fem.ClearDataArrays();

	// read the file
	return ReadFile(szfile);
}

//-----------------------------------------------------------------------------
// This function parses the XML input file. The broot parameter is used to indicate
// if this is the master file or an included file. 
bool FEBioImport::ReadFile(const char* szfile, bool broot)
{
	// Open the XML file
	XMLReader xml;
	if (xml.Open(szfile) == false) return errf("FATAL ERROR: Failed opening input file %s\n\n", szfile);

	// Find the root element
	XMLTag tag;
	try
	{
		if (xml.FindTag("febio_spec", tag) == false) return errf("FATAL ERROR: febio_spec tag was not found. This is not a valid input file.\n\n");
	}
	catch (...)
	{
		return errf("An error occured while finding the febio_spec tag.\nIs this a valid FEBio input file?\n\n");
	}

	// parse the file
	try
	{
		// get the version number
		ParseVersion(tag);

		// FEBio2 only supports file version 1.2, 2.0, and 2.5
		int nversion = GetFileVersion();
		if ((nversion != 0x0102) && 
			(nversion != 0x0200) && 
			(nversion != 0x0205)) throw InvalidVersion();

		// build the file section map based on the version number
		BuildFileSectionMap(nversion);

		// For versions before 2.5 we need to allocate all the degrees of freedom beforehand. 
		// This is necessary because the Module section doesn't have to defined until a Control section appears.
		// That means that model components that depend on DOFs can be defined before the Module tag (e.g. in multi-step analyses) and this leads to problems.
		// In 2.5 this is solved by requiring that the Module tag is defined at the top of the file. 
		if (broot && (nversion < 0x0205))
		{
			// We need to define a default Module type since before 2.5 this tag is optional for structural mechanics model definitions.
			GetBuilder()->SetModuleName("solid");

			// Reset degrees of
			FEModel& fem = *GetFEModel();
			DOFS& dofs = fem.GetDOFS();
			dofs.Reset();

			// Add the default variables and degrees of freedom
			int varD = dofs.AddVariable("displacement", VAR_VEC3);
			dofs.SetDOFName(varD, 0, "x");
			dofs.SetDOFName(varD, 1, "y");
			dofs.SetDOFName(varD, 2, "z");
			int varQ = dofs.AddVariable("rotation", VAR_VEC3);
			dofs.SetDOFName(varQ, 0, "u");
			dofs.SetDOFName(varQ, 1, "v");
			dofs.SetDOFName(varQ, 2, "w");
            int varSD = dofs.AddVariable("shell displacement", VAR_VEC3);
            dofs.SetDOFName(varSD, 0, "sx");
            dofs.SetDOFName(varSD, 1, "sy");
            dofs.SetDOFName(varSD, 2, "sz");
			int varP = dofs.AddVariable("fluid pressure");
			dofs.SetDOFName(varP, 0, "p");
            int varSP = dofs.AddVariable("shell fluid pressure");
            dofs.SetDOFName(varSP, 0, "q");
			int varQR = dofs.AddVariable("rigid rotation", VAR_VEC3);
			dofs.SetDOFName(varQR, 0, "Ru");
			dofs.SetDOFName(varQR, 1, "Rv");
			dofs.SetDOFName(varQR, 2, "Rw");
			int varT = dofs.AddVariable("temperature");
			dofs.SetDOFName(varT, 0, "T");
			int varV = dofs.AddVariable("velocity", VAR_VEC3);
			dofs.SetDOFName(varV, 0, "vx");
			dofs.SetDOFName(varV, 1, "vy");
			dofs.SetDOFName(varV, 2, "vz");
            int varW = dofs.AddVariable("relative fluid velocity", VAR_VEC3);
            dofs.SetDOFName(varW, 0, "wx");
            dofs.SetDOFName(varW, 1, "wy");
            dofs.SetDOFName(varW, 2, "wz");
            int varWP = dofs.AddVariable("previous relative fluid velocity", VAR_VEC3);
            dofs.SetDOFName(varWP, 0, "wxp");
            dofs.SetDOFName(varWP, 1, "wyp");
            dofs.SetDOFName(varWP, 2, "wzp");
            int varAW = dofs.AddVariable("relative fluid acceleration", VAR_VEC3);
            dofs.SetDOFName(varAW, 0, "awx");
            dofs.SetDOFName(varAW, 1, "awy");
            dofs.SetDOFName(varAW, 2, "awz");
            int varAWP = dofs.AddVariable("previous relative fluid acceleration", VAR_VEC3);
            dofs.SetDOFName(varAWP, 0, "awxp");
            dofs.SetDOFName(varAWP, 1, "awyp");
            dofs.SetDOFName(varAWP, 2, "awzp");
            int varVF = dofs.AddVariable("fluid velocity", VAR_VEC3);
            dofs.SetDOFName(varVF, 0, "vfx");
            dofs.SetDOFName(varVF, 1, "vfy");
            dofs.SetDOFName(varVF, 2, "vfz");
            int varAF = dofs.AddVariable("fluid acceleration", VAR_VEC3);
            dofs.SetDOFName(varAF, 0, "afx");
            dofs.SetDOFName(varAF, 1, "afy");
            dofs.SetDOFName(varAF, 2, "afz");
			int varEF = dofs.AddVariable("fluid dilation");
			dofs.SetDOFName(varEF, 0, "ef");
            int varEFP = dofs.AddVariable("previous fluid dilation");
            dofs.SetDOFName(varEFP, 0, "efp");
            int varAEF = dofs.AddVariable("fluid dilation tderiv");
            dofs.SetDOFName(varAEF, 0, "aef");
            int varAEP = dofs.AddVariable("previous fluid dilation tderiv");
            dofs.SetDOFName(varAEP, 0, "aefp");
			int varQP = dofs.AddVariable("previous rotation", VAR_VEC3);
			dofs.SetDOFName(varQP, 0, "up");
			dofs.SetDOFName(varQP, 1, "vp");
			dofs.SetDOFName(varQP, 2, "wp");
            int varSDP = dofs.AddVariable("previous shell displacement", VAR_VEC3);
            dofs.SetDOFName(varSDP, 0, "sxp");
            dofs.SetDOFName(varSDP, 1, "syp");
            dofs.SetDOFName(varSDP, 2, "szp");
			int varQV = dofs.AddVariable("shell velocity", VAR_VEC3);
			dofs.SetDOFName(varQV, 0, "svx");
			dofs.SetDOFName(varQV, 1, "svy");
			dofs.SetDOFName(varQV, 2, "svz");
			int varQA = dofs.AddVariable("shell acceleration", VAR_VEC3);
			dofs.SetDOFName(varQA, 0, "sax");
			dofs.SetDOFName(varQA, 1, "say");
			dofs.SetDOFName(varQA, 2, "saz");
			int varQVP = dofs.AddVariable("previous shell velocity", VAR_VEC3);
			dofs.SetDOFName(varQVP, 0, "svxp");
			dofs.SetDOFName(varQVP, 1, "svyp");
			dofs.SetDOFName(varQVP, 2, "svzp");
			int varQAP = dofs.AddVariable("previous shell acceleration", VAR_VEC3);
			dofs.SetDOFName(varQAP, 0, "saxp");
			dofs.SetDOFName(varQAP, 1, "sayp");
			dofs.SetDOFName(varQAP, 2, "sazp");
			// must be last variable definition!!
			int varC = dofs.AddVariable("concentration", VAR_ARRAY); // we start with zero concentrations
            // must be last variable definition!!
            int varSC = dofs.AddVariable("shell concentration", VAR_ARRAY); // we start with zero concentrations
		}

		// parse the file
		++tag;

		// From version 2.5 and up the first tag of the master file has to be the Module tag.
		if (broot && (nversion >= 0x0205))
		{
			if (tag != "Module")
			{
				return errf("First tag must be the Module section.\n\n");
			}

			// try to find a section parser
			FEFileSectionMap::iterator is = m_map.find(tag.Name());

			// make sure we found a section reader
			if (is == m_map.end()) throw XMLReader::InvalidTag(tag);

			// parse the module tag
			is->second->Parse(tag);

			// Now that the Module tag is read in, we'll want to create an analysis step.
			// Creating an analysis step will allocate a solver class (based on the module) 
			// and this in turn will allocate the degrees of freedom.
			// TODO: This is kind of a round-about way and I really want to find a better solution.
			GetBuilder()->GetStep();

			// let's get the next tag
			++tag;
		}

		do
		{
			// try to find a section parser
			FEFileSectionMap::iterator is = m_map.find(tag.Name());

			// make sure we found a section reader
			if (is == m_map.end()) throw XMLReader::InvalidTag(tag);

			// see if the file has the "from" attribute (for version 2.0 and up)
			if (nversion >= 0x0200)
			{
				const char* szinc = tag.AttributeValue("from", true);
				if (szinc)
				{
					// make sure this is a leaf
					if (tag.isleaf() == false) return errf("FATAL ERROR: included sections may not have child sections.\n\n");

					// read this section from an included file.
					XMLReader xml2;
					if (xml2.Open(szinc) == false) return errf("FATAL ERROR: failed opening input file %s\n\n", szinc);

					// find the febio_spec tag
					XMLTag tag2;
					if (xml2.FindTag("febio_spec", tag2) == false) return errf("FATAL ERROR: febio_spec tag was not found. This is not a valid input file.\n\n");

					// find the section we are looking for
					char sz[512] = {0};
					sprintf(sz, "febio_spec/%s", tag.Name());
					if (xml2.FindTag(sz, tag2) == false) return errf("FATAL ERROR: Couldn't find %s section in file %s.\n\n", tag.Name(), szinc);

					// parse the section
					is->second->Parse(tag2);
				}
				else is->second->Parse(tag);
			}
			else
			{
				// parse the section
				is->second->Parse(tag);
			}

			// go to the next tag
			++tag;
		}
		while (!tag.isend());
	}
	// --- XML Reader Exceptions ---
	catch (XMLReader::Error& e)
	{
		return errf("FATAL ERROR: %s (line %d)\n", e.GetErrorString(), xml.GetCurrentLine());
	}
	// --- FEBioImport Exceptions ---
	catch (FEFileException& e)
	{
		return errf("FATAL ERROR: %s (line %d)\n", e.GetErrorString(), xml.GetCurrentLine());
	}
	// --- Exception from DataStore ---
	catch (UnknownDataField& e)
	{
		return errf("Fatal Error: \"%s\" is not a valid field variable name (line %d)\n", e.m_szdata, xml.GetCurrentLine()-1);
	}
	// --- Unknown exceptions ---
	catch (...)
	{
		return errf("FATAL ERROR: unrecoverable error (line %d)\n", xml.GetCurrentLine());
		return false;
	}

	// close the XML file
	xml.Close();

	// we're done!
	return true;
}

//-----------------------------------------------------------------------------
//! This function parses the febio_spec tag for the version number
void FEBioImport::ParseVersion(XMLTag &tag)
{
	const char* szv = tag.AttributeValue("version");
	assert(szv);
	int n1, n2;
	int nr = sscanf(szv, "%d.%d", &n1, &n2);
	if (nr != 2) throw InvalidVersion();
	if ((n1 < 1) || (n1 > 0xFF)) throw InvalidVersion();
	if ((n2 < 0) || (n2 > 0xFF)) throw InvalidVersion();
	int nversion = (n1 << 8) + n2;
	SetFileVerion(nversion);
}

//-----------------------------------------------------------------------------
void FEBioImport::SetDumpfileName(const char* sz) { sprintf(m_szdmp, "%s", sz); }
void FEBioImport::SetLogfileName (const char* sz) { sprintf(m_szlog, "%s", sz); }
void FEBioImport::SetPlotfileName(const char* sz) { sprintf(m_szplt, "%s", sz); }

//-----------------------------------------------------------------------------
void FEBioImport::AddDataRecord(DataRecord* pd)
{
	m_data.push_back(pd);
}

//-----------------------------------------------------------------------------
void FEBioImport::AddPlotVariable(const char* szvar, vector<int>& item, const char* szdom)
{
    PlotVariable var(szvar, item, szdom);
    m_plot.push_back(var);
}

//-----------------------------------------------------------------------------
void FEBioImport::SetPlotCompression(int n)
{
	m_nplot_compression = n;
}

//-----------------------------------------------------------------------------
// This tag parses a node set.
FENodeSet* FEBioImport::ParseNodeSet(XMLTag& tag, const char* szatt)
{
	FEMesh& mesh = GetFEModel()->GetMesh();
	FENodeSet* pns = 0;

	// see if the set attribute is defined
	const char* szset = tag.AttributeValue(szatt, true);
	if (szset)
	{
		// Make sure this is an empty tag
		if (tag.isempty() == false) throw XMLReader::InvalidValue(tag);

		// find the node set
		pns = mesh.FindNodeSet(szset);
		if (pns == 0) throw XMLReader::InvalidAttributeValue(tag, szatt, szset);
	}
	else
	{
		// This defines a node set, so we need a name tag
		// (For now this name is optional)
		const char* szname = tag.AttributeValue("name", true);
		if (szname == 0) szname = "_unnamed";

		// create a new node set
		pns = new FENodeSet(&mesh);
		pns->SetName(szname);

		// add the nodeset to the mesh
		mesh.AddNodeSet(pns);

		// read the nodes
		if (tag.isleaf())
		{
			// This format is deprecated
			vector<int> l;
			fexml::readList(tag, l);
			for (int i=0; i<l.size(); ++i) pns->add(GetBuilder()->FindNodeFromID(l[i]));
		}
		else
		{
			// read the nodes
			++tag;
			do
			{
				if (tag == "node")
				{
					int nid = -1;
					tag.AttributeValue("id", nid);

					nid = GetBuilder()->FindNodeFromID(nid);
					pns->add(nid);
				}
				else if (tag == "NodeSet")
				{
					const char* szset = tag.AttributeValue(szatt);
					
					// Make sure this is an empty tag
					if (tag.isempty() == false) throw XMLReader::InvalidValue(tag);

					// find the node set
					FENodeSet* ps = mesh.FindNodeSet(szset);
					if (ps == 0) throw XMLReader::InvalidAttributeValue(tag, szatt, szset);

					// add the node set
					pns->add(*ps);
				}
				else if (tag == "node_list")
				{
					vector<int> nl;
					fexml::readList(tag, nl);
					for (int i = 0; i<nl.size(); ++i) pns->add(GetBuilder()->FindNodeFromID(nl[i]));
				}
				else throw XMLReader::InvalidTag(tag);
				++tag;
			}
			while (!tag.isend());
		}
	}

	return pns;
}

//-----------------------------------------------------------------------------
FESurface* FEBioImport::ParseSurface(XMLTag& tag, const char* szatt)
{
	FEMesh& m = GetFEModel()->GetMesh();

	// create new surface
	FESurface* psurf = new FESurface(&m);

	// see if the surface is referenced by a set of defined explicitly
	const char* szset = tag.AttributeValue(szatt, true);
	if (szset)
	{
		// make sure this tag does not have any children
		if (!tag.isleaf()) throw XMLReader::InvalidTag(tag);

		// see if we can find the facet set
		FEMesh& m = GetFEModel()->GetMesh();
		FEFacetSet* ps = 0;
		for (int i=0; i<m.FacetSets(); ++i)
		{
			FEFacetSet& fi = m.FacetSet(i);
			if (strcmp(fi.GetName(), szset) == 0)
			{
				ps = &fi;
				break;
			}
		}

		// create a surface from the facet set
		if (ps)
		{
			if (GetBuilder()->BuildSurface(*psurf, *ps) == false) throw XMLReader::InvalidTag(tag);
		}
		else throw XMLReader::InvalidAttributeValue(tag, "set", szset);
	}
	else
	{
		// count how many pressure cards there are
		int npr = tag.children();
		psurf->Create(npr);

		FEModelBuilder* feb = GetBuilder();

		++tag;
		int nf[FEElement::MAX_NODES ], N;
		for (int i=0; i<npr; ++i)
		{
			FESurfaceElement& el = psurf->Element(i);

			if      (tag == "quad4") el.SetType(FE_QUAD4G4);
			else if (tag == "tri3" ) el.SetType(feb->m_ntri3);
			else if (tag == "tri6" ) el.SetType(feb->m_ntri6);
			else if (tag == "tri7" ) el.SetType(feb->m_ntri7);
			else if (tag == "tri10") el.SetType(feb->m_ntri10);
			else if (tag == "quad8") el.SetType(FE_QUAD8G9);
			else if (tag == "quad9") el.SetType(FE_QUAD9G9);
			else throw XMLReader::InvalidTag(tag);

			N = el.Nodes();
			tag.value(nf, N);
			for (int j=0; j<N; ++j) el.m_node[j] = nf[j]-1;

			++tag;
		}
	}

	return psurf;
}

//-----------------------------------------------------------------------------
void FEBioImport::ParseDataArray(XMLTag& tag, FEDataArray& map, const char* sztag)
{
	int dataType = map.DataSize();

	if (dataType == FE_DOUBLE)
	{
		++tag;
		do
		{
			if (tag == sztag)
			{
				int nid;
				tag.AttributeValue("lid", nid);

				double v;
				tag.value(v);

				map.setValue(nid - 1, v);
			}
			else throw XMLReader::InvalidTag(tag);
			++tag;
		}
		while (!tag.isend());
	}
	else if (dataType == FE_VEC3D)
	{
		++tag;
		do
		{
			if (tag == sztag)
			{
				int nid;
				tag.AttributeValue("lid", nid);

				double v[3];
				tag.value(v, 3);

				map.setValue(nid - 1, vec3d(v[0], v[1], v[2]));
			}
			else throw XMLReader::InvalidTag(tag);
			++tag;
		}
		while (!tag.isend());
	}
}
