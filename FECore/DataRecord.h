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
#include <stdio.h>
#include <vector>
#include "fecore_api.h"

//-----------------------------------------------------------------------------
// forward declaration
class FEModel;
class DumpStream;

//-----------------------------------------------------------------------------
#define FE_DATA_NODE	1
#define FE_DATA_ELEM	2
#define FE_DATA_RB		3
#define FE_DATA_NLC		4

//-----------------------------------------------------------------------------
// Exception thrown when parsing fails
class FECORE_API UnknownDataField
{
public:
	UnknownDataField(const char* sz);
	char	m_szdata[64];
};

//-----------------------------------------------------------------------------

class FECORE_API DataRecord
{
public:
	enum {MAX_DELIM=16, MAX_STRING=1024};
public:
	DataRecord(FEModel* pfem, const char* szfile, int ntype);
	virtual ~DataRecord();

	bool Write();

	void SetItemList(const char* szlist);

	void SetName(const char* sz);
	void SetDelim(const char* sz);
	void SetFormat(const char* sz);
	void SetComments(bool b) { m_bcomm = b; }

public:
	virtual bool Initialize();
	virtual double Evaluate(int item, int ndata) = 0;
	virtual void SelectAllItems() = 0;
	virtual void Serialize(DumpStream& ar);
	virtual void Parse(const char* sz) = 0;
	virtual int Size() = 0;

public:
	int					m_nid;		//!< ID of data record
	std::vector<int>	m_item;		//!< item list
	int					m_type;		//!< type of data record

protected:
	bool	m_bcomm;				//!< export comments or not
	char	m_szname[MAX_STRING];	//!< name of expression
	char	m_szdelim[MAX_DELIM];	//!< data delimitor
	char	m_szdata[MAX_STRING];	//!< data expression
	char	m_szfmt[MAX_STRING];	//!< max format string

protected:
	char	m_szfile[MAX_STRING];	//!< file name of data record

	FEModel*	m_pfem;
	FILE*		m_fp;
};
