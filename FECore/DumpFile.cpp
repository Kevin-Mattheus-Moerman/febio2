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
#include "DumpFile.h"

DumpFile::DumpFile(FEModel& fem) : DumpStream(fem)
{
	m_fp = 0;
	m_nindex = 0;
}

DumpFile::~DumpFile()
{
	Close();
}

bool DumpFile::Open(const char* szfile)
{
	m_fp = fopen(szfile, "rb");
	if (m_fp == 0) return false;

	DumpStream::Open(false, false);

	return true;
}

bool DumpFile::Create(const char* szfile)
{
	m_fp = fopen(szfile, "wb");
	if (m_fp == 0) return false;

	DumpStream::Open(true, false);

	return true;
}

bool DumpFile::Append(const char* szfile)
{
	m_fp = fopen(szfile, "a+b");
	if (m_fp == 0) return false;

	DumpStream::Open(true, false);

	return true;
}

void DumpFile::Close()
{
	if (m_fp) fclose(m_fp); 
	m_fp = 0;
	m_nindex = 0;
}

//! write buffer to archive
size_t DumpFile::write(const void* pd, size_t size, size_t count)
{
	assert(IsSaving());
	m_nindex += (int)(size*count);
	return fwrite(pd, size, count, m_fp);
}

//! read buffer from archive
size_t DumpFile::read(void* pd, size_t size, size_t count)
{
	assert(IsSaving()==false);
	m_nindex += (int)(size*count);
	return fread(pd, size, count, m_fp);
}
