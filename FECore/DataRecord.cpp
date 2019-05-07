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
#include "DataRecord.h"
#include "DumpStream.h"
#include "FEModel.h"
#include "FEAnalysis.h"
#include "log.h"

//-----------------------------------------------------------------------------
UnknownDataField::UnknownDataField(const char* sz)
{
	m_szdata[0] = 0;
	int l = (int)strlen(sz);
	if (l > 63) l = 63;
	if (l>0) { strncpy(m_szdata, sz, l); m_szdata[l] = 0; }
}

//-----------------------------------------------------------------------------
DataRecord::DataRecord(FEModel* pfem, const char* szfile, int ntype) : m_type(ntype)
{
	m_pfem = pfem;
	m_nid = 0;
	m_szname[0] = 0;
	m_szdata[0] = 0;
	m_szfmt[0] = 0;

	strcpy(m_szdelim, " ");
	
	m_bcomm = true;

	m_fp = 0;
	m_szfile[0] = 0;

	if (szfile)
	{
		strcpy(m_szfile, szfile);
		m_fp = fopen(szfile, "wt");
		if (m_fp == 0) felog.printf("FAILED CREATING DATA FILE %s\n\n", szfile);
	}
}

//-----------------------------------------------------------------------------
DataRecord::~DataRecord()
{
	if (m_fp)
	{
		fclose(m_fp);
		m_fp = 0;
	}
}

//-----------------------------------------------------------------------------
void DataRecord::SetName(const char* sz)
{
	strcpy(m_szname, sz);
}

//-----------------------------------------------------------------------------
void DataRecord::SetDelim(const char* sz)
{
	strcpy(m_szdelim, sz);
}

//-----------------------------------------------------------------------------
void DataRecord::SetFormat(const char* sz)
{
	strcpy(m_szfmt, sz);
}

//-----------------------------------------------------------------------------
bool DataRecord::Initialize()
{
	if (m_item.empty()) SelectAllItems();
	return true;
}

//-----------------------------------------------------------------------------
bool DataRecord::Write()
{
	int nstep = m_pfem->GetCurrentStep()->m_ntimesteps;
	double ftime = m_pfem->GetCurrentTime();
	double val;

	FILE* fplog = (FILE*) felog;
	if (fplog)
	{
		// make a note in the log file
		fprintf(fplog, "\nData Record #%d\n", m_nid);
		fprintf(fplog, "===========================================================================\n");
		fprintf(fplog, "Step = %d\n", nstep);
		fprintf(fplog, "Time = %.9lg\n", ftime);
		fprintf(fplog, "Data = %s\n", m_szname);
	}

	// see if we are saving the data to the logfile or to a 
	// seperate data file
	FILE* fp = m_fp;
	if (fp == 0)
	{
		// we store the data in the logfile
		fp = fplog;
		if (fp==0) return true;
	}
	else if (m_bcomm)
	{
		// we save the data in a seperate file
		fprintf(fplog, "File = %s\n", m_szfile);

		// make a note in the data file
		fprintf(fp,"*Step  = %d\n", nstep);
		fprintf(fp,"*Time  = %.9lg\n", ftime);
		fprintf(fp,"*Data  = %s\n", m_szname);
	}

	// save the data
	if (m_szfmt[0]==0)
	{
		for (size_t i=0; i<m_item.size(); ++i)
		{
			fprintf(fp, "%d%s", m_item[i], m_szdelim);
			int nd = Size();
			for (int j=0; j<nd; ++j)
			{
				val = Evaluate(m_item[i], j);
				fprintf(fp, "%lg", val);
				if (j!=nd-1) fprintf(fp, "%s", m_szdelim);
				else fprintf(fp, "\n");
			}
		}
	}
	else
	{
		// print using the format string
		int ndata = Size();
		char szfmt[MAX_STRING];
		strcpy(szfmt, m_szfmt);

		for (size_t i=0; i<m_item.size(); ++i)
		{
			int nitem = m_item[i];
			char* sz = szfmt, *ch = 0;
			int j = 0;
			do
			{
				ch = strchr(sz, '%');
				if (ch)
				{
					if (ch[1]=='i')
					{
						*ch = 0;
						fprintf(fp, "%s", sz);
						*ch = '%'; sz = ch+2;
						fprintf(fp, "%d", nitem);
					}
					else if (ch[1]=='l')
					{
						*ch = 0;
						fprintf(fp, "%s", sz);
						*ch = '%'; sz = ch + 2;
						fprintf(fp, "%lu", i+1);
					}
					else if (ch[1]=='g')
					{
						*ch = 0;
						fprintf(fp, "%s", sz);
						*ch = '%'; sz = ch+2;
						if (j<ndata)
						{
							val = Evaluate(nitem, j++);
							fprintf(fp, "%lg", val);
						}
					}
					else if (ch[1]=='t')
					{
						*ch = 0;
						fprintf(fp, "%s", sz);
						*ch = '%'; sz = ch+2;
						fprintf(fp, "\t");
					}
					else if (ch[1]=='n')
					{
						*ch = 0;
						fprintf(fp, "%s", sz);
						*ch = '%'; sz = ch+2;
						fprintf(fp, "\n");
					}
					else
					{
						*ch = 0;
						fprintf(fp, "%s", sz);
						*ch = '%'; sz = ch+1;
					}
				}
				else { fprintf(fp, "%s", sz); break; }
			}
			while (*sz);
			fprintf(fp, "\n");
		}
	}

	if (fp) fflush(fp);

	return true;
}

//-----------------------------------------------------------------------------

void DataRecord::SetItemList(const char* szlist)
{
	int i, n = 0, n0, n1, nn;
	char* ch;
	char* sz = (char*) szlist;
	int nread;
	do
	{
		ch = strchr(sz, ',');
		if (ch) *ch = 0;
		nread = sscanf(sz, "%d:%d:%d", &n0, &n1, &nn);
		switch (nread)
		{
		case 1:
			n1 = n0;
			nn = 1;
			break;
		case 2:
			nn = 1;
			break;
		case 3:
			break;
		default:
			n0 = 0;
			n1 = -1;
			nn = 1;
		}

		for (i=n0; i<=n1; i += nn) ++n;

		if (ch) *ch = ',';
		sz = ch+1;
	}
	while (ch != 0);

	if (n != 0)
	{
		m_item.resize(n);

		sz = (char*) szlist;
		n = 0;
		do
		{
			ch = strchr(sz, ',');
			if (ch) *ch = 0;
			nread = sscanf(sz, "%d:%d:%d", &n0, &n1, &nn);
			switch (nread)
			{
			case 1:
				n1 = n0;
				nn = 1;
				break;
			case 2:
				nn = 1;
			}

			for (i=n0; i<=n1; i += nn) m_item[n++] = i;
			assert(n <= (int) m_item.size());

			if (ch) *ch = ',';
			sz = ch+1;
		}
		while (ch != 0);
	}
}

//-----------------------------------------------------------------------------

void DataRecord::Serialize(DumpStream &ar)
{
	if (ar.IsShallow()) return;

	if (ar.IsSaving())
	{
		ar << m_nid;
		ar << m_szname;
		ar << m_szdelim;
		ar << m_szfile;
		ar << m_bcomm;
		ar << m_item;
		ar << m_szdata;
	}
	else
	{
		ar >> m_nid;
		ar >> m_szname;
		ar >> m_szdelim;
		ar >> m_szfile;
		ar >> m_bcomm;
		ar >> m_item;
		ar >> m_szdata;

		Parse(m_szdata);

		if (m_fp) fclose(m_fp);
		m_fp = 0;
		if (m_szfile[0] != 0)
		{
			// reopen data file for appending
			m_fp = fopen(m_szfile, "a+");
		}
	}
}
