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
#include "FERestartImport.h"
#include "FECore/FESolver.h"
#include "FECore/FEAnalysis.h"
#include "FECore/FEModel.h"
#include "FECore/DumpFile.h"
#include "FECore/FEDataLoadCurve.h"
#include "FEBioLoadDataSection.h"
#include "FEBioStepSection.h"

void FERestartControlSection::Parse(XMLTag& tag)
{
	FEModel& fem = *GetFEModel();
	FEAnalysis* pstep = fem.GetCurrentStep();

	++tag;
	do
	{
		if      (tag == "time_steps"        ) tag.value(pstep->m_ntime);
		else if (tag == "final_time"        ) tag.value(pstep->m_final_time);
		else if (tag == "step_size"         ) tag.value(pstep->m_dt0);
		else if (tag == "restart" ) 
		{
//			const char* szf = tag.AttributeValue("file", true);
//			if (szf) strcpy(m_szdmp, szf);
			char szval[256];
			tag.value(szval);
			if		(strcmp(szval, "DUMP_DEFAULT"    ) == 0) {} // don't change the restart level
			else if (strcmp(szval, "DUMP_NEVER"      ) == 0) pstep->SetDumpLevel(FE_DUMP_NEVER);
			else if (strcmp(szval, "DUMP_MAJOR_ITRS" ) == 0) pstep->SetDumpLevel(FE_DUMP_MAJOR_ITRS);
			else if (strcmp(szval, "DUMP_STEP"       ) == 0) pstep->SetDumpLevel(FE_DUMP_STEP);
			else if (strcmp(szval, "0" ) == 0) pstep->SetDumpLevel(FE_DUMP_NEVER);		// for backward compatibility only
			else if (strcmp(szval, "1" ) == 0) pstep->SetDumpLevel(FE_DUMP_MAJOR_ITRS); // for backward compatibility only
			else throw XMLReader::InvalidValue(tag);
		}
		else if (tag == "time_stepper")
		{
			pstep->m_bautostep = true;
			FETimeStepController& tc = pstep->m_timeController;
			++tag;
			do
			{
				if      (tag == "max_retries") tag.value(tc.m_maxretries);
				else if (tag == "opt_iter"   ) tag.value(tc.m_iteopt);
				else if (tag == "dtmin"      ) tag.value(tc.m_dtmin);
				else throw XMLReader::InvalidTag(tag);

				++tag;
			}
			while (!tag.isend());
		}
		else if (tag == "plot_level")
		{
			char szval[256];
			tag.value(szval);
			if      (strcmp(szval, "PLOT_NEVER"        ) == 0) pstep->SetPlotLevel(FE_PLOT_NEVER);
			else if (strcmp(szval, "PLOT_MAJOR_ITRS"   ) == 0) pstep->SetPlotLevel(FE_PLOT_MAJOR_ITRS);
			else if (strcmp(szval, "PLOT_MINOR_ITRS"   ) == 0) pstep->SetPlotLevel(FE_PLOT_MINOR_ITRS);
			else if (strcmp(szval, "PLOT_MUST_POINTS"  ) == 0) pstep->SetPlotLevel(FE_PLOT_MUST_POINTS);
			else if (strcmp(szval, "PLOT_FINAL"        ) == 0) pstep->SetPlotLevel(FE_PLOT_FINAL);
			else if (strcmp(szval, "PLOT_STEP_FINAL"   ) == 0) pstep->SetPlotLevel(FE_PLOT_STEP_FINAL);
			else if (strcmp(szval, "PLOT_AUGMENTATIONS") == 0) pstep->SetPlotLevel(FE_PLOT_AUGMENTATIONS);
			else throw XMLReader::InvalidValue(tag);
		}
		else throw XMLReader::InvalidTag(tag);

		++tag;
	}
	while (!tag.isend());

	// we need to reevaluate the time step size and end time
	fem.GetTime().timeIncrement = pstep->m_dt0;
	pstep->m_tend = pstep->m_tstart = pstep->m_ntime*pstep->m_dt0;

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FERestartImport::FERestartImport()
{
	
}

FERestartImport::~FERestartImport()
{
	
}

//-----------------------------------------------------------------------------

bool FERestartImport::Parse(const char* szfile)
{
	// open the XML file
	if (m_xml.Open(szfile) == false) return errf("FATAL ERROR: Failed opening restart file %s\n", szfile);

	FEModel& fem = *GetFEModel();

	m_szdmp[0] = 0;

	m_map["Control" ] = new FERestartControlSection(this);

	// make sure we can redefine curves in the LoadData section
	FEBioLoadDataSection* lcSection = new FEBioLoadDataSection(this);
	lcSection->SetRedefineCurvesFlag(true);

	m_map["LoadData"] = lcSection;

	// set the file version to make sure we are using the correct format
	SetFileVerion(0x0205);

	// loop over child tags
	try
	{
		// find the root element
		XMLTag tag;
		if (m_xml.FindTag("febio_restart", tag) == false) return errf("FATAL ERROR: File does not contain restart data.\n");

		// check the version number
		const char* szversion = tag.m_att[0].m_szatv;
		int nversion = -1;
		if      (strcmp(szversion, "1.0") == 0) nversion = 1;
		else if (strcmp(szversion, "2.0") == 0) nversion = 2;

		if (nversion == -1) return errf("FATAL ERROR: Incorrect restart file version\n");

		// Add the Step section for version 2
		if (nversion == 2)
		{
			m_map["Step"] = new FEBioStepSection25(this);
		}

		// the first section has to be the archive
		++tag;
		if (tag != "Archive") return errf("FATAL ERROR: The first element must be the archive name\n");
		char szar[256];
		tag.value(szar);

		// open the archive
		DumpFile ar(fem);
		if (ar.Open(szar) == false) return errf("FATAL ERROR: failed opening restart archive\n");

		// read the archive
		fem.Serialize(ar);

		// set the module name
		GetBuilder()->SetModuleName(fem.GetModuleName());

		// read the rest of the restart input file
		m_map.Parse(tag);
	}
	catch (XMLReader::Error& e)
	{
		fprintf(stderr, "FATAL ERROR: %s (line %d)\n", e.GetErrorString(), m_xml.GetCurrentLine());
		return false;
	}
	catch (...)
	{
		fprintf(stderr, "FATAL ERROR: unrecoverable error (line %d)\n", m_xml.GetCurrentLine());
		return false;
	}

	// close the XML file
	m_xml.Close();

	// we're done!
	return true;
}
