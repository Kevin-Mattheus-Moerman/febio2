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
// NOTE: This file is automatically included from tens3drs.h
// Users should not include this file manually!

// access operator
inline double tens3d::operator()(int i, int j, int k) const
{
	return d[i*9 + j*3 + k];
}

// access operator
inline double& tens3d::operator()(int i, int j, int k)
{
	return d[i*9 + j*3 + k];
}

// symmetrize a general 3o tensor
inline tens3ds tens3d::symm()
{
	tens3ds t;

	t.d[0] =  d[0]; 
	t.d[1] = (d[1] + d[3]  + d[9])/3.; 
	t.d[2] = (d[2] + d[6]  + d[18])/3.;
	t.d[3] = (d[4] + d[10] + d[12])/3.; 
	t.d[4] = (d[5] + d[11] + d[21] + d[7] + d[19] + d[15])/6.; 
	t.d[5] = (d[8] + d[20] + d[24])/3.;
	t.d[6] =  d[13]; 
	t.d[7] = (d[14] + d[16] + d[22])/3.;
	t.d[8] = (d[17] + d[23] + d[25])/3.;
	t.d[9] =  d[26]; 

	return t;
}

inline tens3d operator + (const tens3dls& l, const tens3drs& r)
{
	tens3d s;
	s.d[ 0] = l.d[ 0] + r.d[ 0];	// S111 = L111 + R111
	s.d[ 1] = l.d[ 1] + r.d[ 1];	// S112 = L112 + R112
	s.d[ 2] = l.d[ 2] + r.d[ 2];	// S113 = L113 + R113
	s.d[ 3] = l.d[ 3] + r.d[ 1];	// S121 = L121 + R112
	s.d[ 4] = l.d[ 4] + r.d[ 3];	// S122 = L122 + R122
	s.d[ 5] = l.d[ 5] + r.d[ 4];	// S123 = L123 + R123
	s.d[ 6] = l.d[ 6] + r.d[ 2];	// S131 = L131 + R113
	s.d[ 7] = l.d[ 7] + r.d[ 4];	// S132 = L132 + R123
	s.d[ 8] = l.d[ 8] + r.d[ 5];	// S133 = L133 + R133
	s.d[ 9] = l.d[ 3] + r.d[ 6];	// S211 = L121 + R211
	s.d[10] = l.d[ 4] + r.d[ 7];	// S212 = L122 + R212
	s.d[11] = l.d[ 5] + r.d[ 8];	// S213 = L123 + R213
	s.d[12] = l.d[ 9] + r.d[ 7];	// S221 = L221 + R212
	s.d[13] = l.d[10] + r.d[ 9];	// S222 = L222 + R222
	s.d[14] = l.d[11] + r.d[10];	// S223 = L223 + R223
	s.d[15] = l.d[12] + r.d[ 8];	// S231 = L231 + R213
	s.d[16] = l.d[13] + r.d[10];	// S232 = L232 + R223
	s.d[17] = l.d[14] + r.d[11];	// S233 = L233 + R233
	s.d[18] = l.d[ 6] + r.d[12];	// S311 = L131 + R311
	s.d[19] = l.d[ 7] + r.d[13];	// S312 = L132 + R312
	s.d[20] = l.d[ 8] + r.d[14];	// S313 = L133 + R313
	s.d[21] = l.d[12] + r.d[13];	// S321 = L231 + R312
	s.d[22] = l.d[13] + r.d[15];	// S322 = L232 + R322
	s.d[23] = l.d[14] + r.d[16];	// S323 = L233 + R323
	s.d[24] = l.d[15] + r.d[14];	// S331 = L331 + R313
	s.d[25] = l.d[16] + r.d[16];	// S332 = L332 + R323
	s.d[26] = l.d[17] + r.d[17];	// S333 = L333 + R333
	return s;
}
