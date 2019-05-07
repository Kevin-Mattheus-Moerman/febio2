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
#include "FECore/FEMesh.h"
#include "FECore/FEPlotData.h"

//-----------------------------------------------------------------------------
class FEModel;

//-----------------------------------------------------------------------------
//! This class implements the facilities to write to a plot database. 
//!
class PlotFile
{
public:
	//! constructor
	PlotFile();

	//! descructor
	virtual ~PlotFile();

	//! close the plot database
	virtual void Close();

	//! Open the plot database
	virtual bool Open(FEModel& fem, const char* szfile) = 0;

	//! Open for appending
	virtual bool Append(FEModel& fem, const char* szfile) = 0;

	//! Write current FE state to plot database
	virtual bool Write(FEModel& fem, float ftime) = 0;

	//! see if the plot file is valid
	virtual bool IsValid() const = 0;

protected:
	FEModel*	m_pfem;		//!< pointer to FE model
};
