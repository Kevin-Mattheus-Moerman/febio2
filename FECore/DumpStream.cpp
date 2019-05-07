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
#include "DumpStream.h"

//-----------------------------------------------------------------------------
DumpStream::DumpStream(FEModel& fem) : m_fem(fem)
{
	m_bsave = false;
	m_bshallow = false;
}

//-----------------------------------------------------------------------------
DumpStream::~DumpStream()
{
}

//-----------------------------------------------------------------------------
void DumpStream::Open(bool bsave, bool bshallow)
{
	m_bsave = bsave;
	m_bshallow = bshallow;
}

//-----------------------------------------------------------------------------
DumpStream& DumpStream::operator << (const char* sz) 
{ 
	int n = (sz ? (int)strlen(sz) : 0);
	write(&n, sizeof(int), 1);
	if (sz) write(sz, sizeof(char), n);
	return (*this);
}

//-----------------------------------------------------------------------------
DumpStream& DumpStream::operator << (char* sz) 
{ 
	int n = (sz ? (int)strlen(sz) : 0); 
	write(&n, sizeof(int), 1);
	if (sz) write(sz, sizeof(char), n);
	return (*this);
}

//-----------------------------------------------------------------------------
DumpStream& DumpStream::operator<<(const std::string& s)
{
	const char* sz = s.c_str();
	this->operator<<(sz);
	return *this;
}

//-----------------------------------------------------------------------------
DumpStream& DumpStream::operator << (const double a[3][3])
{
	write(a, sizeof(double), 9);
	return (*this);
}

//-----------------------------------------------------------------------------
DumpStream& DumpStream::operator >> (char* sz) 
{ 
	int n;
	read(&n, sizeof(int), 1);
	if (n>0) read(sz, sizeof(char), n);
	sz[n] = 0;
	return (*this);
}

//-----------------------------------------------------------------------------
DumpStream& DumpStream::operator >> (std::string& s)
{
	char buf[64]={0};
	this->operator>>(buf);
	s = std::string(buf);
	return *this;
}

//-----------------------------------------------------------------------------
DumpStream& DumpStream::operator >> (double a[3][3])
{
	read(a, sizeof(double), 9);
	return (*this);
}
