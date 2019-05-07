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
#include "FEBioPlotFile2.h"
#include "FECore/FECoreKernel.h"
#include "FECore/FEDataExport.h"
#include "FECore/FEModel.h"
#include "FECore/FEMaterial.h"
#include <FEBioLib/version.h>

class FEPlotDataExport : public FEPlotData
{
public:
	FEPlotDataExport(const char* szname, Var_Type itype, Storage_Fmt fmt) : FEPlotData(FE_REGION_SURFACE, itype, fmt) { m_szname = szname; }
	void Save(FEModel& fem, Archive& ar)
	{
		FEMesh& mesh = fem.GetMesh();
		int NS = mesh.Surfaces();
		for (int i=0; i<NS; ++i)
		{
			FESurface& s = mesh.Surface(i);
			int ND = s.DataExports();
			if (ND > 0)
			{
				for (int j=0; j<ND; ++j)
				{
					FEDataExport* pd = s.GetDataExport(j);
					if (strcmp(pd->m_szname, m_szname) == 0)
					{
						FEDataStream d;
						pd->Serialize(d);
						ar.WriteData(i+1, d.data());
						break;
					}
				}
			}
		}
	}

private:
	const char*		m_szname;
};

class FEPlotVariable : public FENodeData
{
public:
	FEPlotVariable(const char* szname, Var_Type itype, Storage_Fmt fmt) : FENodeData(itype, fmt) { strcpy(m_szname, szname); }
	bool Save(FEMesh& mesh, FEDataStream& str)
	{
		// get the DOFS
		FEModel& fem = *GetFEModel();
		DOFS& dofs = fem.GetDOFS();

		// see if this variable exists
		int nvar = dofs.GetVariableIndex(m_szname);
		if (nvar < 0) return false;

		// get the size of the variable
		int n = dofs.GetVariableSize(nvar);
		if (n == 0) return false;

		// get the start index of the DOFS
		int ndof = dofs.GetDOF(nvar, 0);
		if (ndof < 0) return false;

		// store the nodal data
		int NN = mesh.Nodes();
		for (int i=0; i<NN; ++i)
		{
			FENode& node = mesh.Node(i);
			for (int j=0; j<n; ++j) str << node.get(ndof + j);
		}

		return true;
	}

private:
	char	m_szname[256];
};

//-----------------------------------------------------------------------------
//! Adds a variable to the plot file. 
//! 
//! The name of the filter can be composed of three parts and in general takes on
//! the following format.
//!
//! szname = "field_name[filter]=alias". 
//!
//! field_name = This is the actual filter naeme as it is registered with the framework.
//! filter     = This is a filter that is used to resolve ambiguities.
//! alias      = This is an alternative name for the field variable.
//!
//! The alias is optional but can be used by post-processing software to present an alternative
//! (often simpler) name for the field variable than the default field_name + filter combo.
//! 
//! Whether a filter is required depends entirely on the field variable. Most field variables don't
//! require it, but some do in order to resolve an ambiguity. For instance, the "parameter" field
//! allows users to plot the spatially varying value of a material parameter. The filter is used to
//! specify the material and parameter name. 
//!
//! The filter can be a numerical value or a string. If it's a string then it must be enclosed in
//! single quotes. 
//!
//! szname = "field_name[12]"  \\ example of a numerical filter
//! szname = "field_name['val'] \\ example of a string filter
//!
//! The interpretation of these filters is entirely left up to the field variable. 
//!
bool FEBioPlotFile2::Dictionary::AddVariable(FEModel* pfem, const char* szname, vector<int>& item, const char* szdom)
{
	FECoreKernel& febio = FECoreKernel::GetInstance();

	// create a copy so we can strip the alias and the filter from the name
	char sz[1024] = {0};
	strcpy(sz, szname);

	// see if there is an alias defined
	char* ch = strchr(sz, '=');
	if (ch)
	{
		// replace the equal sign with a null character.
		*ch++ = 0;

		// make sure there is an alias
		if (ch==0) return false;
	}

	// extract the filter
	char* szflt = strchr(sz, '[');
	int index = 0;
	int ntype = 0;
	if (szflt)
	{
		*szflt++ = 0;
		char* ch = strrchr(szflt, ']');
		if (ch == 0) return false;
		*ch = 0;

		// see if the filter is a number or a string
		ch = strchr(szflt, '\'');
		if (ch)
		{
			*szflt++ = 0;
			// find the end character
			char* ch2 = strrchr(szflt, '\'');
			if (ch2 == 0) return false;
			*ch2 = 0;
		}
		else
		{
			ntype = 1;
			index = atoi(szflt);
		}
	}

	// create the plot variable
	FEPlotData* ps = fecore_new<FEPlotData>(FEPLOTDATA_ID, sz, pfem);
	if (ps)
	{
		// set the optional item list and filter
		ps->SetItemList(item);
		if (szflt) 
		{
			if (ntype == 0) 
			{
				if (ps->SetFilter(szflt) == false) return false;
			}
			else if (ntype == 1)
			{
				if (ps->SetFilter(index) == false) return false;
			}
		}

		// add the field to the plot file
        ps->SetDomainName(szdom);
		switch (ps->RegionType())
		{
		case FE_REGION_NODE   : return AddNodalVariable  (ps, szname, item);
		case FE_REGION_DOMAIN : return AddDomainVariable (ps, szname, item);
		case FE_REGION_SURFACE: return AddSurfaceVariable(ps, szname, item);
		default:
			assert(false);
			return false;
		}
	}
	else
	{
		// If we get here then this variable is not a plot field.
		// But let's see if it is an export variable from a domain
		FEMesh& mesh = pfem->GetMesh();
		for (int i=0; i<mesh.Surfaces(); ++i)
		{
			FESurface& s = mesh.Surface(i);
			int ND = s.DataExports();
			for (int j=0; j<ND; ++j)
			{
				FEDataExport* pd = s.GetDataExport(j);
				if (strcmp(pd->m_szname, szname) == 0)
				{
					// We have a match. Create a plot field for this export
					ps = new FEPlotDataExport(pd->m_szname, pd->m_type, pd->m_fmt);
					return AddSurfaceVariable(ps, szname, item);
				}
			}
		}

		// If we still didn't find it, maybe it's a model variable.
		DOFS& dofs = pfem->GetDOFS();
		int nvar = dofs.GetVariableIndex(szname);
		if (nvar >= 0)
		{
			int ntype = dofs.GetVariableType(nvar);
			if (ntype == VAR_SCALAR)
			{
				ps = new FEPlotVariable(szname, PLT_FLOAT, FMT_NODE);
				return AddNodalVariable(ps, szname, item);
			}
			else if (ntype == VAR_VEC3)
			{
				ps = new FEPlotVariable(szname, PLT_VEC3F, FMT_NODE);
				return AddNodalVariable(ps, szname, item);
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile2::Dictionary::AddGlobalVariable(FEPlotData* ps, const char* szname)
{
	return false;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile2::Dictionary::AddMaterialVariable(FEPlotData* ps, const char* szname)
{
	return false;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile2::Dictionary::AddNodalVariable(FEPlotData* ps, const char* szname, vector<int>& item)
{
	assert(ps->RegionType()==FE_REGION_NODE);
	if (ps->RegionType()==FE_REGION_NODE)
	{
		DICTIONARY_ITEM it;
		it.m_ntype = ps->DataType();
		it.m_nfmt  = ps->StorageFormat();
		it.m_psave = ps;
		strcpy(it.m_szname, szname);
		m_Node.push_back(it);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile2::Dictionary::AddDomainVariable(FEPlotData* ps, const char* szname, vector<int>& item)
{
	assert(ps->RegionType()==FE_REGION_DOMAIN);
	if (ps->RegionType()==FE_REGION_DOMAIN)
	{
		DICTIONARY_ITEM it;
		it.m_ntype = ps->DataType();
		it.m_nfmt  = ps->StorageFormat();
		it.m_psave = ps;
		strcpy(it.m_szname, szname);
		m_Elem.push_back(it);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile2::Dictionary::AddSurfaceVariable(FEPlotData* ps, const char* szname, vector<int>& item)
{
	assert(ps->RegionType()==FE_REGION_SURFACE);
	if (ps->RegionType()==FE_REGION_SURFACE)
	{
		DICTIONARY_ITEM it;
		it.m_ntype = ps->DataType();
		it.m_nfmt  = ps->StorageFormat();
		it.m_psave = ps;
		strcpy(it.m_szname, szname);
		m_Face.push_back(it);
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------
void FEBioPlotFile2::Dictionary::Defaults(FEModel& fem)
{
	// First we build the dictionary
	// get the mesh
	FEMesh& m = fem.GetMesh();

	// Define default variables
	if (m_Node.empty() && m_Elem.empty() && m_Face.empty())
	{
		vector<int> l; // empty list
		AddVariable(&fem, "displacement", l);
		AddVariable(&fem, "stress", l);
	}
}

//-----------------------------------------------------------------------------
void FEBioPlotFile2::Dictionary::Clear()
{
	m_Glob.clear();
	m_Mat.clear();
	m_Node.clear();
	m_Elem.clear();
	m_Face.clear();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

FEBioPlotFile2::FEBioPlotFile2(FEModel& fem) : m_fem(fem)
{
	m_ncompress = 0;
}

//-----------------------------------------------------------------------------
FEBioPlotFile2::~FEBioPlotFile2(void)
{
	// close the archive
	Close();

	// clear all arrays
	list<DICTIONARY_ITEM>::iterator it = m_dic.m_Glob.begin();
	for (int i=0; i<(int) m_dic.m_Glob.size(); ++i, ++it) delete it->m_psave;

	it = m_dic.m_Mat.begin();
	for (int i=0; i<(int) m_dic.m_Mat.size(); ++i, ++it) delete it->m_psave;

	it = m_dic.m_Node.begin();
	for (int i=0; i<(int) m_dic.m_Node.size(); ++i, ++it) delete it->m_psave;

	it = m_dic.m_Elem.begin();
	for (int i=0; i<(int) m_dic.m_Elem.size(); ++i, ++it) delete it->m_psave;

	it = m_dic.m_Face.begin();
	for (int i=0; i<(int) m_dic.m_Face.size(); ++i, ++it) delete it->m_psave;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile2::AddVariable(FEPlotData* ps, const char* szname)
{
	vector<int> dummy;
	switch (ps->RegionType())
	{
	case FE_REGION_NODE: return m_dic.AddNodalVariable(ps, szname, dummy);
	case FE_REGION_DOMAIN: return m_dic.AddDomainVariable(ps, szname, dummy);
	case FE_REGION_SURFACE: return m_dic.AddSurfaceVariable(ps, szname, dummy);
	default:
		assert(false);
		return false;
	}
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile2::AddVariable(const char* sz)
{
	vector<int> dummy;
	return AddVariable(sz, dummy);
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile2::AddVariable(const char* sz, vector<int>& item, const char* szdom)
{ 
	return m_dic.AddVariable(&m_fem, sz, item, szdom); 
}

//-----------------------------------------------------------------------------
void FEBioPlotFile2::SetCompression(int n)
{
	m_ncompress = n;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile2::IsValid() const
{
	return m_ar.IsValid();
}

//-----------------------------------------------------------------------------
void FEBioPlotFile2::Close()
{
	m_ar.Close();
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile2::Open(FEModel &fem, const char *szfile)
{
	// open the archive
	m_ar.Create(szfile);

	try
	{
		// write the root element
		if (WriteRoot(fem) == false) return false;

		// write the mesh section
		if (WriteMeshSection(fem) == false) return false;
	}
	catch (...)
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile2::WriteRoot(FEModel& fem)
{
	// write the root element
	// (don't compress this section)
	m_ar.SetCompression(0);
	m_ar.BeginChunk(PLT_ROOT);
	{
		// --- save the header file ---
		m_ar.BeginChunk(PLT_HEADER);
		{
			if (WriteHeader(fem) == false) return false;
		}
		m_ar.EndChunk();

		// --- save the dictionary ---
		m_ar.BeginChunk(PLT_DICTIONARY);
		{
			if (WriteDictionary(fem) == false) return false;
		}
		m_ar.EndChunk();
	}
	m_ar.EndChunk();

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile2::WriteHeader(FEModel& fem)
{
	// setup the header
	unsigned int nversion = PLT_VERSION;

	// output header
	m_ar.WriteChunk(PLT_HDR_VERSION, nversion);

	// compression flag
	m_ar.WriteChunk(PLT_HDR_COMPRESSION, m_ncompress);

	// software flag
	char sz[256] = {0};
	sprintf(sz, "FEBio %d.%d.%d", VERSION, SUBVERSION, SUBSUBVERSION);
	m_ar.WriteChunk(PLT_HDR_SOFTWARE, (const char*)sz);

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile2::WriteDictionary(FEModel& fem)
{
	// setup defaults for the dictionary
	m_dic.Defaults(fem);

	// Next, we save the dictionary
	// Global variables
	if (!m_dic.m_Glob.empty())
	{
		m_ar.BeginChunk(PLT_DIC_GLOBAL);
		{
			WriteDicList(m_dic.m_Glob);
		}
		m_ar.EndChunk();
	}

	// store nodal variables
	if (!m_dic.m_Node.empty())
	{
		m_ar.BeginChunk(PLT_DIC_NODAL);
		{
			WriteDicList(m_dic.m_Node);
		}
		m_ar.EndChunk();
	}

	// store element variables
	if (!m_dic.m_Elem.empty())
	{
		m_ar.BeginChunk(PLT_DIC_DOMAIN);
		{
			WriteDicList(m_dic.m_Elem);
		}
		m_ar.EndChunk();
	}

	// store surface data
	if (!m_dic.m_Face.empty())
	{
		m_ar.BeginChunk(PLT_DIC_SURFACE);
		{
			WriteDicList(m_dic.m_Face);
		}
		m_ar.EndChunk();
	}

	return true;
}

//-----------------------------------------------------------------------------
void FEBioPlotFile2::WriteDicList(list<FEBioPlotFile2::DICTIONARY_ITEM>& dic)
{
	int N = (int) dic.size();
	list<DICTIONARY_ITEM>::iterator pi = dic.begin();
	for (int i=0; i<N; ++i, ++pi)
	{
		m_ar.BeginChunk(PLT_DIC_ITEM);
		{
			m_ar.WriteChunk(PLT_DIC_ITEM_TYPE, pi->m_ntype);
			m_ar.WriteChunk(PLT_DIC_ITEM_FMT , pi->m_nfmt);
			m_ar.WriteChunk(PLT_DIC_ITEM_NAME, pi->m_szname, STR_SIZE);
		}
		m_ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile2::WriteMeshSection(FEModel& fem)
{
	// get the mesh
	FEMesh& m = fem.GetMesh();

	m_ar.BeginChunk(PLT_MESH);
	{
		// node section
		m_ar.BeginChunk(PLT_NODE_SECTION);
		{
			WriteNodeSection(m);
		}
		m_ar.EndChunk();

		// domain section
		m_ar.BeginChunk(PLT_DOMAIN_SECTION);
		{
			WriteDomainSection(m);
		}
		m_ar.EndChunk();

		// surface section
		if (m.Surfaces() > 0)
		{
			m_ar.BeginChunk(PLT_SURFACE_SECTION);
			{
				WriteSurfaceSection(m);
			}
			m_ar.EndChunk();
		}

		// node sets
		if (m.NodeSets() > 0)
		{
			m_ar.BeginChunk(PLT_NODESET_SECTION);
			{
				WriteNodeSetSection(m);
			}
			m_ar.EndChunk();
		}

		// parts
		// (we write the materials as parts)
		if (fem.Materials() > 0)
		{
			m_ar.BeginChunk(PLT_PARTS_SECTION);
			{
				WritePartsSection(fem);
			}
			m_ar.EndChunk();
		}
	}
	m_ar.EndChunk();

	return true;
}

//-----------------------------------------------------------------------------
void FEBioPlotFile2::WriteNodeSection(FEMesh& m)
{
	// write the node header
	m_ar.BeginChunk(PLT_NODE_HEADER);
	{
		int NN = m.Nodes();
		int dim = 3;
		m_ar.WriteChunk(PLT_NODE_SIZE, NN);
		m_ar.WriteChunk(PLT_NODE_DIM , dim);
//		m_ar.WriteChunk(PLT_NODE_NAME, "AllNodes");
	}
	m_ar.EndChunk();

	// write the reference coordinates
	int NN = m.Nodes();
	vector<float> X(4*NN);
	for (int i=0; i<m.Nodes(); ++i)
	{
		FENode& node = m.Node(i);
		*((int*) (&X[0] + 4*i)) = i;
		X[4*i+1] = (float) node.m_r0.x;
		X[4*i+2] = (float) node.m_r0.y;
		X[4*i+3] = (float) node.m_r0.z;
	}
	m_ar.WriteChunk(PLT_NODE_COORDS, X);
}

//-----------------------------------------------------------------------------
void FEBioPlotFile2::WriteDomainSection(FEMesh& m)
{
	// write all domains
	for (int nd = 0; nd<m.Domains(); ++nd)
	{
		FEDomain& dom = m.Domain(nd);
		m_ar.BeginChunk(PLT_DOMAIN);
		{
			switch (dom.Class())
			{
			case FE_DOMAIN_SOLID   : WriteSolidDomain   (static_cast<FESolidDomain&   >(dom)); break;
			case FE_DOMAIN_SHELL   : WriteShellDomain   (static_cast<FEShellDomain&   >(dom)); break;
			case FE_DOMAIN_TRUSS   : WriteTrussDomain   (static_cast<FETrussDomain&   >(dom)); break;
			case FE_DOMAIN_DISCRETE: WriteDiscreteDomain(static_cast<FEDiscreteDomain&>(dom)); break;
            case FE_DOMAIN_2D      : WriteDomain2D      (static_cast<FEDomain2D&      >(dom)); break;
			}
		}
		m_ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
void FEBioPlotFile2::WriteSolidDomain(FESolidDomain& dom)
{
	int mid = dom.GetMaterial()->GetID();
	assert(mid > 0);
	int eshape = dom.GetElementShape();

	int i, j;
	int NE = dom.Elements();

	// figure out element type
	int ne = 0;
	int dtype = 0;
	switch (eshape)
	{
		case ET_HEX8   : ne =  8; dtype = PLT_ELEM_HEX; break;
		case ET_PENTA6 : ne =  6; dtype = PLT_ELEM_PENTA; break;
		case ET_TET4   : ne =  4; dtype = PLT_ELEM_TET; break;
		case ET_TET10  : ne = 10; dtype = PLT_ELEM_TET10; break;
		case ET_TET15  : ne = 15; dtype = PLT_ELEM_TET15; break;
		case ET_HEX20  : ne = 20; dtype = PLT_ELEM_HEX20; break;
		case ET_HEX27  : ne = 27; dtype = PLT_ELEM_HEX27; break;
		case ET_TET20  : ne = 20; dtype = PLT_ELEM_TET20; break;
        case ET_PENTA15: ne = 15; dtype = PLT_ELEM_PENTA15; break;
		case ET_PYRA5  : ne =  5; dtype = PLT_ELEM_PYRA5; break;
        default:
			assert(false);
	}

	// write the header
	m_ar.BeginChunk(PLT_DOMAIN_HDR);
	{
		m_ar.WriteChunk(PLT_DOM_ELEM_TYPE, dtype);
		m_ar.WriteChunk(PLT_DOM_PART_ID  ,   mid);
		m_ar.WriteChunk(PLT_DOM_ELEMS    ,    NE);
		m_ar.WriteChunk(PLT_DOM_NAME     , dom.GetName());
	}
	m_ar.EndChunk();

	// write the element list
	int n[FEElement::MAX_NODES + 1];
	m_ar.BeginChunk(PLT_DOM_ELEM_LIST);
	{
		for (i=0; i<NE; ++i)
		{
			FESolidElement& el = dom.Element(i);
			n[0] = el.GetID();
			for (j=0; j<ne; ++j) n[j+1] = el.m_node[j];
			m_ar.WriteChunk(PLT_ELEMENT, n, ne+1);
		}
	}
	m_ar.EndChunk();
}

//-----------------------------------------------------------------------------
void FEBioPlotFile2::WriteShellDomain(FEShellDomain& dom)
{
	int mid = dom.GetMaterial()->GetID();
	assert(mid > 0);
	int etype = dom.GetElementType();

	int i, j;
	int NE = dom.Elements();

	// figure out element type
	int ne = 0;
	int dtype = 0;
	switch (etype)
	{
        case FE_SHELL_QUAD4G8 :
        case FE_SHELL_QUAD4G12 :
            ne = 4; dtype = PLT_ELEM_QUAD; break;
        case FE_SHELL_TRI3G6  :
        case FE_SHELL_TRI3G9  :
            ne = 3; dtype = PLT_ELEM_TRI; break;
        case FE_SHELL_QUAD8G18:
        case FE_SHELL_QUAD8G27:
            ne = 8; dtype = PLT_ELEM_QUAD8; break;
        case FE_SHELL_TRI6G14 :
        case FE_SHELL_TRI6G21 :
            ne = 6; dtype = PLT_ELEM_TRI6; break;
        default:
            assert(false);
	}

	// write the header
	m_ar.BeginChunk(PLT_DOMAIN_HDR);
	{
		m_ar.WriteChunk(PLT_DOM_ELEM_TYPE, dtype);
		m_ar.WriteChunk(PLT_DOM_PART_ID  ,   mid);
		m_ar.WriteChunk(PLT_DOM_ELEMS    ,    NE);
	}
	m_ar.EndChunk();

	// write the element list
	int n[9];
	m_ar.BeginChunk(PLT_DOM_ELEM_LIST);
	{
		for (i=0; i<NE; ++i)
		{
			FEShellElement& el = dom.Element(i);
			n[0] = el.GetID();
			for (j=0; j<ne; ++j) n[j+1] = el.m_node[j];
			m_ar.WriteChunk(PLT_ELEMENT, n, ne+1);
		}
	}
	m_ar.EndChunk();
}

//-----------------------------------------------------------------------------
void FEBioPlotFile2::WriteTrussDomain(FETrussDomain& dom)
{
	int mid = dom.GetMaterial()->GetID();
	assert(mid > 0);

	int i, j;
	int NE = dom.Elements();

	// figure out element type
	int ne = 2;
	int dtype = PLT_ELEM_TRUSS;

	// write the header
	m_ar.BeginChunk(PLT_DOMAIN_HDR);
	{
		m_ar.WriteChunk(PLT_DOM_ELEM_TYPE, dtype);
		m_ar.WriteChunk(PLT_DOM_PART_ID  ,   mid);
		m_ar.WriteChunk(PLT_DOM_ELEMS    ,    NE);
	}
	m_ar.EndChunk();

	// write the element list
	int n[5];
	m_ar.BeginChunk(PLT_DOM_ELEM_LIST);
	{
		for (i=0; i<NE; ++i)
		{
			FEElement& el = dom.ElementRef(i);
			n[0] = el.GetID();
			for (j=0; j<ne; ++j) n[j+1] = el.m_node[j];
			m_ar.WriteChunk(PLT_ELEMENT, n, ne+1);
		}
	}
	m_ar.EndChunk();
}

//-----------------------------------------------------------------------------
void FEBioPlotFile2::WriteDiscreteDomain(FEDiscreteDomain& dom)
{
	int mid = dom.GetMaterial()->GetID();
	assert(mid > 0);

	int i, j;
	int NE = dom.Elements();

	// figure out element type
	int ne = 2;
	int dtype = PLT_ELEM_TRUSS;

	// write the header
	m_ar.BeginChunk(PLT_DOMAIN_HDR);
	{
		m_ar.WriteChunk(PLT_DOM_ELEM_TYPE, dtype);
		m_ar.WriteChunk(PLT_DOM_PART_ID  ,   mid);
		m_ar.WriteChunk(PLT_DOM_ELEMS    ,    NE);
	}
	m_ar.EndChunk();

	// write the element list
	int n[5];
	m_ar.BeginChunk(PLT_DOM_ELEM_LIST);
	{
		for (i=0; i<NE; ++i)
		{
			FEElement& el = dom.ElementRef(i);
			n[0] = el.GetID();
			for (j=0; j<ne; ++j) n[j+1] = el.m_node[j];
			m_ar.WriteChunk(PLT_ELEMENT, n, ne+1);
		}
	}
	m_ar.EndChunk();
}

//-----------------------------------------------------------------------------
void FEBioPlotFile2::WriteDomain2D(FEDomain2D& dom)
{
    int mid = dom.GetMaterial()->GetID();
    assert(mid > 0);
    int etype = dom.GetElementType();
    
    int i, j;
    int NE = dom.Elements();
    
    // figure out element type
    int ne = 0;
    int dtype = 0;
    switch (etype)
    {
        case FE2D_TRI3G1 : ne = 3; dtype = PLT_ELEM_TRI; break;
        case FE2D_TRI6G3 : ne = 6; dtype = PLT_ELEM_TRI6; break;
        case FE2D_QUAD4G4: ne = 4; dtype = PLT_ELEM_QUAD; break;
        case FE2D_QUAD8G9: ne = 8; dtype = PLT_ELEM_QUAD8; break;
        case FE2D_QUAD9G9: ne = 9; dtype = PLT_ELEM_QUAD9; break;
		default:
			assert(false);
    }
    
    // write the header
    m_ar.BeginChunk(PLT_DOMAIN_HDR);
    {
        m_ar.WriteChunk(PLT_DOM_ELEM_TYPE, dtype);
        m_ar.WriteChunk(PLT_DOM_PART_ID  ,   mid);
        m_ar.WriteChunk(PLT_DOM_ELEMS    ,    NE);
    }
    m_ar.EndChunk();
    
    // write the element list
    int n[10];
    m_ar.BeginChunk(PLT_DOM_ELEM_LIST);
    {
        for (i=0; i<NE; ++i)
        {
            FEElement2D& el = dom.Element(i);
            n[0] = el.GetID();
            for (j=0; j<ne; ++j) n[j+1] = el.m_node[j];
            m_ar.WriteChunk(PLT_ELEMENT, n, ne+1);
        }
    }
    m_ar.EndChunk();
}

//-----------------------------------------------------------------------------
void FEBioPlotFile2::WriteSurfaceSection(FEMesh& m)
{
	for (int ns = 0; ns<m.Surfaces(); ++ns)
	{
		FESurface& s = m.Surface(ns);
		int NF = s.Elements();
		m_ar.BeginChunk(PLT_SURFACE);
		{
			m_ar.BeginChunk(PLT_SURFACE_HDR);
			{
				int sid = ns+1;
				m_ar.WriteChunk(PLT_SURFACE_ID, sid);
				m_ar.WriteChunk(PLT_SURFACE_FACES, NF);
				m_ar.WriteChunk(PLT_SURFACE_NAME, s.GetName());
			}
			m_ar.EndChunk();

			m_ar.BeginChunk(PLT_FACE_LIST);
			{
				int n[FEBioPlotFile2::PLT_MAX_FACET_NODES + 2];
				for (int i=0; i<NF; ++i)
				{
					FESurfaceElement& f = s.Element(i);
					int nf = f.Nodes();
					n[0] = i+1;
					n[1] = nf;
					for (int i=0; i<nf; ++i) n[i+2] = f.m_node[i];
					m_ar.WriteChunk(PLT_FACE, n, FEBioPlotFile2::PLT_MAX_FACET_NODES+2);
				}
			}
			m_ar.EndChunk();
		}
		m_ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
void FEBioPlotFile2::WriteNodeSetSection(FEMesh& m)
{
	for (int ns = 0; ns < m.NodeSets(); ++ns)
	{
		FENodeSet& l = *m.NodeSet(ns);
		int nodes = l.size();
		m_ar.BeginChunk(PLT_NODESET);
		{
			m_ar.BeginChunk(PLT_NODESET_HDR);
			{
				int nid = ns+1;
				m_ar.WriteChunk(PLT_NODESET_ID, nid);
				m_ar.WriteChunk(PLT_NODESET_SIZE, nodes);
				m_ar.WriteChunk(PLT_NODESET_NAME, l.GetName());
			}
			m_ar.EndChunk();

			m_ar.WriteChunk(PLT_NODESET_LIST, l.GetNodeList());
		}
		m_ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
void FEBioPlotFile2::WritePartsSection(FEModel& fem)
{
	int NMAT = fem.Materials();
	for (int i=0; i<NMAT; ++i)
	{
		FEMaterial* pm = fem.GetMaterial(i);
		m_ar.BeginChunk(PLT_PART);
		{
			unsigned int nid = (unsigned int) pm->GetID();
			char szname[STR_SIZE] = {0};

			// Make sure that the material name fits in the buffer
			std::string name = pm->GetName();
			const char* sz = name.c_str();
			int l = (int)strlen(sz);
			if (l >= STR_SIZE) l = STR_SIZE - 1;
			strncpy(szname, sz, l);

			// write the material data
			m_ar.WriteChunk(PLT_PART_ID, nid);
			m_ar.WriteChunk(PLT_PART_NAME, szname, STR_SIZE);
		}
		m_ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile2::Write(FEModel &fem, float ftime)
{
	// store the fem pointer
	m_pfem = &fem;

	// compress these sections if requested
	m_ar.SetCompression(m_ncompress);
	m_ar.BeginChunk(PLT_STATE);
	{
		// state header
		m_ar.BeginChunk(PLT_STATE_HEADER);
		{
			m_ar.WriteChunk(PLT_STATE_HDR_TIME, ftime);
		}
		m_ar.EndChunk();

		m_ar.BeginChunk(PLT_STATE_DATA);
		{
			// Global Data
			if (!m_dic.m_Glob.empty())
			{
				m_ar.BeginChunk(PLT_GLOBAL_DATA);
				{
					WriteGlobalData(fem);
				}
				m_ar.EndChunk();
			}

			// Node Data
			if (!m_dic.m_Node.empty())
			{
				m_ar.BeginChunk(PLT_NODE_DATA);
				{
					WriteNodeData(fem);
				}
				m_ar.EndChunk();
			}

			// Element Data
			if (!m_dic.m_Elem.empty())
			{
				m_ar.BeginChunk(PLT_ELEMENT_DATA);
				{
					WriteDomainData(fem);
				}
				m_ar.EndChunk();
			}

			// surface data
			if (!m_dic.m_Face.empty())
			{
				m_ar.BeginChunk(PLT_FACE_DATA);
				{
					WriteSurfaceData(fem);
				}
				m_ar.EndChunk();
			}
		}
		m_ar.EndChunk();
	}
	m_ar.EndChunk();

	return true;
}

//-----------------------------------------------------------------------------
void FEBioPlotFile2::WriteGlobalData(FEModel& fem)
{

}

//-----------------------------------------------------------------------------
void FEBioPlotFile2::WriteNodeData(FEModel& fem)
{
	list<DICTIONARY_ITEM>::iterator it = m_dic.m_Node.begin();
	for (int i=0; i<(int) m_dic.m_Node.size(); ++i, ++it)
	{
		m_ar.BeginChunk(PLT_STATE_VARIABLE);
		{
			unsigned int nid = i+1;
			m_ar.WriteChunk(PLT_STATE_VAR_ID, nid);
			m_ar.BeginChunk(PLT_STATE_VAR_DATA);
			{
				if (it->m_psave) (it->m_psave)->Save(fem, m_ar);
			}
			m_ar.EndChunk();
		}
		m_ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
void FEBioPlotFile2::WriteDomainData(FEModel& fem)
{
	list<DICTIONARY_ITEM>::iterator it = m_dic.m_Elem.begin();
	for (int i=0; i<(int) m_dic.m_Elem.size(); ++i, ++it)
	{
		m_ar.BeginChunk(PLT_STATE_VARIABLE);
		{
			unsigned int nid = i+1;
			m_ar.WriteChunk(PLT_STATE_VAR_ID, nid);
			m_ar.BeginChunk(PLT_STATE_VAR_DATA);
			{
				if (it->m_psave) (it->m_psave)->Save(fem, m_ar);
			}
			m_ar.EndChunk();
		}
		m_ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
void FEBioPlotFile2::WriteSurfaceData(FEModel& fem)
{
	list<DICTIONARY_ITEM>::iterator it = m_dic.m_Face.begin();
	for (int i=0; i<(int) m_dic.m_Face.size(); ++i, ++it)
	{
		m_ar.BeginChunk(PLT_STATE_VARIABLE);
		{
			unsigned int nid = i+1;
			m_ar.WriteChunk(PLT_STATE_VAR_ID, nid);
			m_ar.BeginChunk(PLT_STATE_VAR_DATA);
			{
				if (it->m_psave) (it->m_psave)->Save(fem, m_ar);
			}
			m_ar.EndChunk();
		}
		m_ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile2::Append(FEModel& fem, const char *szfile)
{
	// try to open the file
	if (m_ar.Open(szfile) == false) return false;

	// open the root element
	m_ar.OpenChunk();
	unsigned int nid = m_ar.GetChunkID();
	if (nid != PLT_ROOT) return false;

	bool bok = false;
	while (m_ar.OpenChunk() == IO_OK)
	{
		nid = m_ar.GetChunkID();
		if (nid == PLT_DICTIONARY)
		{
			// read the dictionary
			bok = ReadDictionary();
			break;
		}
		m_ar.CloseChunk();
	}

	// close it again ...
	m_ar.Close();

	// ... and open for appending
	if (bok) return m_ar.Append(szfile);

	return false;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile2::ReadDictionary()
{
	m_dic.Clear();

	while (m_ar.OpenChunk() == IO_OK)
	{
		unsigned int nid = m_ar.GetChunkID();
		switch (nid)
		{
		case PLT_DIC_GLOBAL: assert(false); return false;
		case PLT_DIC_NODAL  : ReadDicList(); break;
		case PLT_DIC_DOMAIN : ReadDicList(); break;
		case PLT_DIC_SURFACE: ReadDicList(); break;
		default:
			assert(false);
			return false;
		}
		m_ar.CloseChunk();
	}
	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile2::ReadDicList()
{
	vector<int> l; // empty item list
	while (m_ar.OpenChunk() == IO_OK)
	{
		unsigned int nid = m_ar.GetChunkID();
		if (nid == PLT_DIC_ITEM)
		{
			while (m_ar.OpenChunk() == IO_OK)
			{
				unsigned int nid = m_ar.GetChunkID();
				if (nid == PLT_DIC_ITEM_NAME)
				{
					char sz[STR_SIZE];
					m_ar.read(sz, STR_SIZE);
					AddVariable(sz, l);
				}
				m_ar.CloseChunk();
			}
		}
		else return false;
		m_ar.CloseChunk();
	}
	return true;
}
