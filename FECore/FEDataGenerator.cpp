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
#include "FEDataGenerator.h"
#include "FEMesh.h"
#include "log.h"

FEDataGenerator::FEDataGenerator() : FECoreBase(FEDATAGENERATOR_ID)
{
}

FEDataGenerator::~FEDataGenerator()
{
}

bool FEDataGenerator::Init()
{
	return true;
}

bool FEDataGenerator::Apply(FEDomain* part, const char* szvar)
{
	felog.SetMode(Logfile::LOG_FILE_AND_SCREEN);

	// check input
	if (part == 0) return false;
	if (szvar == 0) return false;

	// get the index
	int index = 0;
	char szbuf[256] = { 0 };
	strcpy(szbuf, szvar);

	// get the last dot if present
	char* cd = strrchr(szbuf, '.');
	if (cd == 0) cd = szbuf;

	char* chl = strchr(cd, '[');
	if (chl)
	{
		*chl++ = 0;
		char* chr = strrchr(chl, ']');
		if (chr == 0) return false;
		*chr = 0;
		index = atoi(chl);
		if (index < 0) return false;
	}

	FEMesh& mesh = *part->GetMesh();

	vec3d r[FEElement::MAX_NODES];
	size_t nsize = part->Elements();
	for (size_t i = 0; i<nsize; ++i)
	{
		FEElement& el = part->ElementRef(i);
		int neln = el.Nodes();
		int nint = el.GaussPoints();

		// get the element's coordinates
		for (int j=0; j<neln; ++j) r[j] = mesh.Node(el.m_node[j]).m_r0;

		// evaluate the Gauss points
		for (int j = 0; j<nint; ++j)
		{
			// evaluate the spatial position of this gauss point
			vec3d x = el.Evaluate(r, j);

			// find the parameter
			FEMaterialPoint* pt = el.GetMaterialPoint(j);
			FEParam* p = pt->FindParameter(szbuf);
			if (p && (index < p->dim()) && (p->type() == FE_PARAM_DOUBLE))
			{
				*p->pvalue<double>(index) = value(x);
			}
			else return false;
		}
	}

	return true;
}
