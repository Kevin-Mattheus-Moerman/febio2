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
#include "FE_enum.h"
#include "FEDataStream.h"

//-----------------------------------------------------------------------------
// This class is used by domain classes to define their data exports.
// This is part of an experimental feature that allows domain classes to handle
// the data that they want to export. 
class FECORE_API FEDataExport
{
public:
	FEDataExport(Var_Type itype, Storage_Fmt ifmt, void* pd, const char* szname)
	{
		m_pd = pd;
		m_type = itype;
		m_fmt = ifmt;
		m_szname = szname;
	}

	virtual ~FEDataExport(){}

	virtual void Serialize(FEDataStream& d);

public:
	Var_Type	m_type;
	Storage_Fmt	m_fmt;
	void*		m_pd;		//!< pointer to data field
	const char*	m_szname;
};

#define EXPORT_DATA(iType, iFmt, pVar, Name) AddDataExport(new FEDataExport(iType, iFmt, pVar, Name));