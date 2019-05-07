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
#include "FEDataArray.h"
#include "DumpStream.h"

//-----------------------------------------------------------------------------
FEDataArray::FEDataArray(int dataType) : m_dataSize(dataType), m_dataCount(0)
{
}

//-----------------------------------------------------------------------------
FEDataArray::~FEDataArray()
{
}

//-----------------------------------------------------------------------------
FEDataArray::FEDataArray(const FEDataArray& map)
{
	m_dataSize = map.m_dataSize;
	m_dataCount = map.m_dataCount;
	m_val = map.m_val;
}

//-----------------------------------------------------------------------------
FEDataArray& FEDataArray::operator = (const FEDataArray& map)
{
	m_dataSize = map.m_dataSize;
	m_dataCount = map.m_dataCount;
	m_val = map.m_val;
	return *this;
}

//-----------------------------------------------------------------------------
bool FEDataArray::resize(int n, double val)
{
	if (n < 0) return false;
	m_val.resize(n*DataSize(), val);
	return true;
}

//-----------------------------------------------------------------------------
//! set the data sized
void FEDataArray::SetDataSize(int dataSize)
{
	m_dataSize = dataSize;
	if (m_val.empty() == false)
	{
		m_val.resize(m_dataSize*m_dataCount);
	}
}

//-----------------------------------------------------------------------------
void FEDataArray::Serialize(DumpStream& ar)
{
	if (ar.IsSaving())
	{
		ar << m_dataSize;
		ar << m_dataCount;
		ar << m_val;
	}
	else
	{
		ar >> m_dataSize;
		ar >> m_dataCount;
		ar >> m_val;
//		assert(m_val.size() == m_dataSize*m_dataCount);
	}
}
