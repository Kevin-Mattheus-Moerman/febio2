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
#include "FECore.h"
#include "FECoordSysMap.h"
#include "FECoreKernel.h"
#include "BC.h"
#include "FEInitialCondition.h"
#include "FECorePlot.h"
#include "FEDataLoadCurve.h"

#define FECORE_VERSION		0
#define FECORE_SUBVERSION	1

//-----------------------------------------------------------------------------
// Get the version info
void FECore::get_version(int& version, int& subversion)
{
	version = FECORE_VERSION;
	subversion = FECORE_SUBVERSION;
}

//-----------------------------------------------------------------------------
// get the version string
const char* FECore::get_version_string()
{
	static const char fecore_str[4] = {'0'+FECORE_VERSION, '.', '0'+FECORE_SUBVERSION };
	return fecore_str;
}

//-----------------------------------------------------------------------------
void FECore::InitModule()
{
// coordinate system map
REGISTER_FECORE_CLASS(FELocalMap         , FECOORDSYSMAP_ID, "local"      );
REGISTER_FECORE_CLASS(FESphericalMap     , FECOORDSYSMAP_ID, "spherical"  );
REGISTER_FECORE_CLASS(FECylindricalMap   , FECOORDSYSMAP_ID, "cylindrical");
REGISTER_FECORE_CLASS(FEVectorMap        , FECOORDSYSMAP_ID, "vector"     );
REGISTER_FECORE_CLASS(FESphericalAngleMap, FECOORDSYSMAP_ID, "angles"     );
REGISTER_FECORE_CLASS(FEPolarMap         , FECOORDSYSMAP_ID, "polar"      );

// boundary conditions
REGISTER_FECORE_CLASS(FEFixedBC      , FEBC_ID, "fix"      );
REGISTER_FECORE_CLASS(FEPrescribedDOF, FEBC_ID, "prescribe");
REGISTER_FECORE_CLASS(FENodalLoad    , FEBC_ID, "nodal load");

// initial conditions
REGISTER_FECORE_CLASS(FEInitialBC     , FEIC_ID, "init_bc"       );
REGISTER_FECORE_CLASS(FEInitialBCVec3D, FEIC_ID, "init_bc_vec3d" );

// plot field
REGISTER_FECORE_CLASS(FEPlotMaterialParameter, FEPLOTDATA_ID, "parameter");

// load curves
REGISTER_FECORE_CLASS(FEDataLoadCurve, FELOADCURVE_ID, "loadcurve");
REGISTER_FECORE_CLASS(FELinearRamp   , FELOADCURVE_ID, "linear ramp");
}
