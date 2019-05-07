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
#include "FEBioDiagnostic.h"
#include <FECore/log.h>

//-----------------------------------------------------------------------------
bool FEBioDiagnostic::Init(const char *szfile)
{
	FEModel& fem = *GetFEModel();

	// read the diagnostic file
	// this will also create a specific diagnostic test
	FEDiagnosticImport im;
	m_pdia = im.LoadFile(fem, szfile);
	if (m_pdia == 0)
	{
		fprintf(stderr, "Failed reading diagnostic file\n");
		return false;
	}

	// intialize diagnostic
	if (m_pdia->Init() == false)
	{
		fprintf(stderr, "Diagnostic initialization failed\n\n");
		return false;
	}

	// --- initialize FE Model data ---
	if (fem.Init() == false)
	{
		fprintf(stderr, "FE-model data initialized has failed\n\n");
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioDiagnostic::Run()
{
	if (m_pdia == 0) return false;

	// --- run the diagnostic ---

	// the return value will designate the pass/fail result
	bool bret = false;
	try
	{
		bret = m_pdia->Run();
	}
	catch (...)
	{
		felog.SetMode(Logfile::LOG_FILE_AND_SCREEN);
		felog.printf("Exception thrown. Aborting diagnostic.\n");
		bret = false;
	}

	if (bret) felog.printf("Diagnostic passed\n");
	else felog.printf("Diagnostic failed\n");

	return bret;
}
