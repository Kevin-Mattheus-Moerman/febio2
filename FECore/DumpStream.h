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
#include <vector>
#include <string>
#include <string.h>
#include "vec3d.h"
#include "mat3d.h"
#include "quatd.h"
#include "fecore_api.h"

//-----------------------------------------------------------------------------
class FEModel;

//-----------------------------------------------------------------------------
//! A dump stream is used to serialize data to and from a data stream.
//! This is used in FEBio for running and cold restarts. 
//! This is just an abstract base class. Classes must be derived from this
//! to implement the actual storage mechanism.
class FECORE_API DumpStream
{
public:
	// This class is thrown when an error occurs reading the dumpfile
	class ReadError{};

public:
	//! constructor
	DumpStream(FEModel& fem);

	//! destructor
	virtual ~DumpStream();

	//! See if the stream is used for input or output
	bool IsSaving() const { return m_bsave; }

	//! See if shallow flag is set
	bool IsShallow() const { return m_bshallow; }

	// open the stream
	virtual void Open(bool bsave, bool bshallow);

	// get the FE model
	FEModel& GetFEModel() { return m_fem; }

public:
	// These functions must be overloaded by derived classes
	virtual size_t write(const void* pd, size_t size, size_t count) = 0;
	virtual size_t read(void* pd, size_t size, size_t count) = 0;
	virtual void clear() = 0;
	virtual void check(){}

public: // output operators
	DumpStream& operator << (const char* sz);
	DumpStream& operator << (char* sz);
	DumpStream& operator << (const double a[3][3]);
	DumpStream& operator << (const std::string& s);
	template <typename T> DumpStream& operator << (const T& o);
	template <typename T> DumpStream& operator << (std::vector<T>& o);

public: // input operators
	DumpStream& operator >> (char* sz);
	DumpStream& operator >> (double a[3][3]);
	DumpStream& operator >> (std::string& s);
	template <typename T> DumpStream& operator >> (T& o);
	template <typename T> DumpStream& operator >> (std::vector<T>& o);

private:
	bool		m_bsave;	//!< true if output stream, false for input stream
	bool		m_bshallow;	//!< if true only shallow data needs to be serialized
	FEModel&	m_fem;		//!< the FE Model that is being serialized
};

template <typename T> inline DumpStream& DumpStream::operator << (const T& o)
{
	write(&o, sizeof(T), 1);
	return *this;
}

template <typename T> inline DumpStream& DumpStream::operator >> (T& o)
{
	read(&o, sizeof(T), 1);
	return *this;
}

template <> inline DumpStream& DumpStream::operator << (const bool& o)
{
	int b = (o==true?1:0);
	write(&b, sizeof(int), 1);
	return *this;
}

template <> inline DumpStream& DumpStream::operator >> (bool& o)
{
	int b;
	read(&b, sizeof(int), 1);
	o = (b==1);
	return *this;
}

template <typename T> inline DumpStream& DumpStream::operator << (std::vector<T>& o)
{
	int N = (int) o.size();
	write(&N, sizeof(int), 1);
	if (N > 0) write((T*)(&o[0]), sizeof(T), N);
	return *this;
}

template <typename T> inline DumpStream& DumpStream::operator >> (std::vector<T>& o)
{
	DumpStream& This = *this;
	int N;
	read(&N, sizeof(int), 1);
	if (N > 0)
	{
		o.resize(N);
		read((T*)(&o[0]), sizeof(T), N);
	}
	return This;
}

template <> inline DumpStream& DumpStream::operator << (std::vector<bool>& o)
{
	DumpStream& This = *this;
	int N = (int) o.size();
	write(&N, sizeof(int), 1);
	for (int i=0; i<N; ++i) 
	{
		bool b = o[i];
		This << b;
	}
	return This;
}

template <> inline DumpStream& DumpStream::operator >> (std::vector<bool>& o)
{
	DumpStream& This = *this;
	int N;
	read(&N, sizeof(int), 1);
	if (N > 0)
	{
		o.resize(N);
		for (int i=0; i<N; ++i) 
		{
			bool b;
			This >> b;
			o[i] = b;
		}
	}
	return This;
}
