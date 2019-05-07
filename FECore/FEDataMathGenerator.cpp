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
#include "FEDataMathGenerator.h"
#include "MathParser.h"
#include "FEMesh.h"

//=======================================================================================
FEDataMathGenerator::FEDataMathGenerator()
{
}

// set the math expression
void FEDataMathGenerator::setExpression(const std::string& math)
{
	m_math = math;
}

// generate the data array for the given node set
bool FEDataMathGenerator::Generate(FENodeDataMap& ar, const FENodeSet& set)
{
	MathParser parser;

	int N = set.size();
	ar.Create(N);
	int ierr;
	for (int i = 0; i<N; ++i)
	{
		const FENode* ni = set.Node(i);

		vec3d ri = ni->m_r0;
		parser.SetVariable("X", ri.x);
		parser.SetVariable("Y", ri.y);
		parser.SetVariable("Z", ri.z);

		double vi = parser.eval(m_math.c_str(), ierr);
		if (ierr != 0) return false;

		ar.setValue(i, vi);
	}

	return true;
}

// generate the data array for the given facet set
bool FEDataMathGenerator::Generate(FESurfaceMap& data, const FEFacetSet& surf)
{
	MathParser parser;

	const FEMesh& mesh = *surf.GetMesh();

	int N = surf.Faces();
	data.Create(&surf);
	int ierr;
	for (int i = 0; i<N; ++i)
	{
		const FEFacetSet::FACET& face = surf.Face(i);

		int nf = face.ntype;
		for (int j=0; j<nf; ++j)
		{
			vec3d ri = mesh.Node(face.node[j]).m_r0;
			parser.SetVariable("X", ri.x);
			parser.SetVariable("Y", ri.y);
			parser.SetVariable("Z", ri.z);

			double vi = parser.eval(m_math.c_str(), ierr);
			if (ierr != 0) return false;

			data.setValue(i, j, vi);
		}
	}

	return true;
}
