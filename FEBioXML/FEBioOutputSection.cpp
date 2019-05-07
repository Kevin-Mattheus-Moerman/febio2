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
#include "FEBioOutputSection.h"
#include "FECore/NodeDataRecord.h"
#include "FECore/ElementDataRecord.h"
#include "FECore/ObjectDataRecord.h"
#include "FECore/NLConstraintDataRecord.h"
#include "FECore/FEModel.h"
#include <FECore/FEModelData.h>

//-----------------------------------------------------------------------------
void FEBioOutputSection::Parse(XMLTag& tag)
{
	++tag;
	do
	{
		if      (tag == "logfile" ) ParseLogfile(tag);
		else if (tag == "plotfile") ParsePlotfile(tag);
		else if (tag == "data"    ) ParseDataSection(tag);
		else throw XMLReader::InvalidTag(tag);
		++tag;
	}
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioOutputSection::ParseDataSection(XMLTag &tag)
{
	FEModel& fem = *GetFEModel();

	++tag;
	do
	{
		if (tag == "element_data")
		{
			const char* szdata = tag.AttributeValue("data");

			FELogElemData* pd = fecore_new<FELogElemData>(FEELEMLOGDATA_ID, szdata, &fem);
			if (pd == 0) throw XMLReader::InvalidAttributeValue(tag, "data", szdata);

			vector<int> items;
			tag.value(items);

			FEModelData* data = new FEModelData(&fem, pd, items);
			data->SetName(szdata);
			fem.AddModelData(data);
		}
		else throw XMLReader::InvalidTag(tag);
		++tag;
	}
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioOutputSection::ParseLogfile(XMLTag &tag)
{
	FEModel& fem = *GetFEModel();
	FEMesh& mesh = fem.GetMesh();

	// see if the log file has any attributes
	const char* szlog = tag.AttributeValue("file", true);
	if (szlog) GetFEBioImport()->SetLogfileName(szlog);

	const char* szpath = GetFileReader()->GetFilePath();

	if (tag.isleaf()) return;
	++tag;
	do
	{
		if (tag == "node_data")
		{
			const char* sz = tag.AttributeValue("file", true);

			NodeDataRecord* prec = 0;
			if (sz)
			{
				// if we have a path, prepend the path's name
				char szfile[1024] = {0};
				if (szpath && szpath[0])
				{
					sprintf(szfile, "%s%s", szpath, sz);
				}
				else strcpy(szfile, sz);

				prec = new NodeDataRecord(&fem, szfile);
			}
			else prec = new NodeDataRecord(&fem, 0);

			const char* szdata = tag.AttributeValue("data");
			prec->Parse(szdata);

			const char* szname = tag.AttributeValue("name", true);
			if (szname != 0) prec->SetName(szname); else prec->SetName(szdata);

			sz = tag.AttributeValue("delim", true);
			if (sz != 0) prec->SetDelim(sz);

			sz = tag.AttributeValue("format", true);
			if (sz!=0) prec->SetFormat(sz);

			sz = tag.AttributeValue("comments", true);
			if (sz != 0)
			{
				if      (strcmp(sz, "on") == 0) prec->SetComments(true);
				else if (strcmp(sz, "off") == 0) prec->SetComments(false); 
			}

			const char* sztmp = "set";
			if (GetFileReader()->GetFileVersion() >= 0x0205) sztmp = "node_set";
			sz = tag.AttributeValue(sztmp, true);
			if (sz)
			{
				FENodeSet* pns = mesh.FindNodeSet(sz);
				if (pns == 0) throw XMLReader::InvalidAttributeValue(tag, sztmp, sz);
				prec->SetItemList(pns);
			}
			else prec->DataRecord::SetItemList(tag.szvalue());

			GetFEBioImport()->AddDataRecord(prec);
		}
		else if (tag == "element_data")
		{
			const char* sz = tag.AttributeValue("file", true);

			ElementDataRecord* prec = 0;
			if (sz)
			{
				// if we have a path, prepend the path's name
				char szfile[1024] = {0};
				if (szpath && szpath[0])
				{
					sprintf(szfile, "%s%s", szpath, sz);
				}
				else strcpy(szfile, sz);
				prec = new ElementDataRecord(&fem, szfile);
			}
			else prec = new ElementDataRecord(&fem, 0);

			const char* szdata = tag.AttributeValue("data");
			prec->Parse(szdata);

			const char* szname = tag.AttributeValue("name", true);
			if (szname != 0) prec->SetName(szname); else prec->SetName(szdata);

			sz = tag.AttributeValue("delim", true);
			if (sz != 0) prec->SetDelim(sz);

			sz = tag.AttributeValue("format", true);
			if (sz!=0) prec->SetFormat(sz);

			sz = tag.AttributeValue("comments", true);
			if (sz != 0)
			{
				if      (strcmp(sz, "on") == 0) prec->SetComments(true);
				else if (strcmp(sz, "off") == 0) prec->SetComments(false); 
			}

			const char* sztmp = "elset";
			if (GetFileReader()->GetFileVersion() >= 0x0205) sztmp = "elem_set";

			sz = tag.AttributeValue(sztmp, true);
			if (sz)
			{
				FEElementSet* pes = mesh.FindElementSet(sz);
				if (pes == 0) throw XMLReader::InvalidAttributeValue(tag, sztmp, sz);
				prec->SetItemList(pes);
			}
			else prec->DataRecord::SetItemList(tag.szvalue());

			GetFEBioImport()->AddDataRecord(prec);
		}
		else if (tag == "rigid_body_data")
		{
			const char* sz = tag.AttributeValue("file", true);

			ObjectDataRecord* prec = 0;
			if (sz)
			{
				// if we have a path, prepend the path's name
				char szfile[1024] = {0};
				if (szpath && szpath[0])
				{
					sprintf(szfile, "%s%s", szpath, sz);
				}
				else strcpy(szfile, sz);
				prec = new ObjectDataRecord(&fem, szfile);
			}
			else prec = new ObjectDataRecord(&fem, 0);

			const char* szdata = tag.AttributeValue("data");
			prec->Parse(szdata);

			const char* szname = tag.AttributeValue("name", true);
			if (szname != 0) prec->SetName(szname); else prec->SetName(szdata);

			sz = tag.AttributeValue("delim", true);
			if (sz != 0) prec->SetDelim(sz);

			sz = tag.AttributeValue("format", true);
			if (sz!=0) prec->SetFormat(sz);

			sz = tag.AttributeValue("comments", true);
			if (sz != 0)
			{
				if      (strcmp(sz, "on") == 0) prec->SetComments(true);
				else if (strcmp(sz, "off") == 0) prec->SetComments(false); 
			}

			prec->SetItemList(tag.szvalue());

			GetFEBioImport()->AddDataRecord(prec);
		}
        else if (tag == "rigid_connector_data")
        {
            const char* sz = tag.AttributeValue("file", true);
            
            NLConstraintDataRecord* prec = 0;
            if (sz)
            {
                // if we have a path, prepend the path's name
                char szfile[1024] = {0};
                if (szpath && szpath[0])
                {
                    sprintf(szfile, "%s%s", szpath, sz);
                }
                else strcpy(szfile, sz);
                prec = new NLConstraintDataRecord(&fem, szfile);
            }
            else prec = new NLConstraintDataRecord(&fem, 0);
            
            const char* szdata = tag.AttributeValue("data");
            prec->Parse(szdata);
            
            const char* szname = tag.AttributeValue("name", true);
            if (szname != 0) prec->SetName(szname); else prec->SetName(szdata);
            
            sz = tag.AttributeValue("delim", true);
            if (sz != 0) prec->SetDelim(sz);
            
            sz = tag.AttributeValue("format", true);
            if (sz!=0) prec->SetFormat(sz);
            
            sz = tag.AttributeValue("comments", true);
            if (sz != 0)
            {
                if      (strcmp(sz, "on") == 0) prec->SetComments(true);
                else if (strcmp(sz, "off") == 0) prec->SetComments(false); 
            }
            
            prec->SetItemList(tag.szvalue());
            
			GetFEBioImport()->AddDataRecord(prec);
        }
		else throw XMLReader::InvalidTag(tag);

		++tag;
	}
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioOutputSection::ParsePlotfile(XMLTag &tag)
{
	FEModel& fem = *GetFEModel();

	// get the plot file type. Must be "febio"!
	const char* sz = tag.AttributeValue("type", true);
	if (sz)
	{
		if ((strcmp(sz, "febio") != 0) && (strcmp(sz, "febio2") != 0)) throw XMLReader::InvalidAttributeValue(tag, "type", sz);
	}
	else sz = "febio";
	strcpy(GetFEBioImport()->m_szplot_type, sz);

	// get the optional plot file name
	const char* szplt = tag.AttributeValue("file", true);
	if (szplt) GetFEBioImport()->SetPlotfileName(szplt);

	// read and store the plot variables
	if (!tag.isleaf())
	{
		++tag;
		do
		{
			if (tag == "var")
			{
				// get the variable name
				const char* szt = tag.AttributeValue("type");

				// get the item list
				vector<int> item;
				if (tag.isempty() == false) tag.value(item);

                // see if a surface is referenced
                const char* szset = tag.AttributeValue("surface", true);
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
                        // create a new surface
                        FESurface* psurf = new FESurface(&fem.GetMesh());
                        fem.GetMesh().AddSurface(psurf);
                        if (GetBuilder()->BuildSurface(*psurf, *ps) == false) throw XMLReader::InvalidTag(tag);

                        // Add the plot variable
                        const std::string& surfName = psurf->GetName();
						GetFEBioImport()->AddPlotVariable(szt, item, surfName.c_str());
                    }
                    else throw XMLReader::InvalidAttributeValue(tag, "set", szset);
                }
                else
                {
                    // Add the plot variable
					GetFEBioImport()->AddPlotVariable(szt, item);
                }
			}
			else if (tag=="compression")
			{
				int ncomp;
				tag.value(ncomp);
				GetFEBioImport()->SetPlotCompression(ncomp);
			}
			++tag;
		}
		while (!tag.isend());
	}
}
