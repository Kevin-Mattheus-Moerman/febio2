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
#include "vec3d.h"
#include "mat3d.h"
#include "tens4d.h"

//-----------------------------------------------------------------------------
// This class can be used to serialize data.
// This is part of a new experimental feature that allows domain classes to define
// data exports. This in turn will eliminate the need for many of the plot classes. 
class FEDataStream
{
public:
	FEDataStream(){}

	void clear() { m_a.clear(); }

	FEDataStream& operator << (const double& f) { m_a.push_back((float) f); return *this; }
	FEDataStream& operator << (const vec3d& v) 
	{
		m_a.push_back((float) v.x);
		m_a.push_back((float) v.y);
		m_a.push_back((float) v.z);
		return *this;
	}
	FEDataStream& operator << (const mat3ds& m) 
	{
		m_a.push_back((float) m.xx());
		m_a.push_back((float) m.yy());
		m_a.push_back((float) m.zz());
		m_a.push_back((float) m.xy());
		m_a.push_back((float) m.yz());
		m_a.push_back((float) m.xz());
		return *this;
	}
	FEDataStream& operator << (const mat3d& m)
	{
		m_a.push_back((float)m(0, 0));
		m_a.push_back((float)m(0, 1));
		m_a.push_back((float)m(0, 2));
		m_a.push_back((float)m(1, 0));
		m_a.push_back((float)m(1, 1));
		m_a.push_back((float)m(1, 2));
		m_a.push_back((float)m(2, 0));
		m_a.push_back((float)m(2, 1));
		m_a.push_back((float)m(2, 2));
		return *this;
	}
	FEDataStream& operator << (const tens4ds& a)
	{
        for (int k=0; k<21; ++k) m_a.push_back((float) a.d[k]);
		return *this;
	}

	void assign(size_t count, float f) { m_a.assign(count, f); }
	void reserve(size_t count) { m_a.reserve(count); }
	void push_back(const float& f) { m_a.push_back(f); }
	size_t size() const { return m_a.size(); }

	float& operator [] (int i) { return m_a[i]; }

	vector<float>& data() { return m_a; }

private:
	vector<float>	m_a;
};
