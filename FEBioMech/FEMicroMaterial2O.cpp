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
#include "FEMicroMaterial2O.h"
#include "FECore/log.h"
#include "FESolidSolver2.h"
#include "FEElasticSolidDomain.h"
#include "FECore/FEAnalysis.h"
#include "FEBioXML/FEBioImport.h"
#include "FEBioPlot/FEBioPlotFile.h"
#include "FECore/BC.h"
#include "FECore/tens3d.h"
#include "FEPeriodicBoundary2O.h"

//-----------------------------------------------------------------------------
FEMicroMaterialPoint2O::FEMicroMaterialPoint2O(FEMaterialPoint* mp) : FEMaterialPoint(mp)
{
	m_elem_id = -1;
	m_gpt_id = -1;
}

//-----------------------------------------------------------------------------
//! create a shallow copy
FEMaterialPoint* FEMicroMaterialPoint2O::Copy()
{
	FEMicroMaterialPoint2O* pt = new FEMicroMaterialPoint2O(m_pNext?m_pNext->Copy():0);
	return pt;
}

//-----------------------------------------------------------------------------
//! serialize material point data
void FEMicroMaterialPoint2O::Serialize(DumpStream& ar)
{
	FEMaterialPoint::Serialize(ar);
}

//=============================================================================

//-----------------------------------------------------------------------------
// define the material parameters
BEGIN_PARAMETER_LIST(FEMicroMaterial2O, FEElasticMaterial2O)
	ADD_PARAMETER(m_szrve    , FE_PARAM_STRING, "RVE"     );
	ADD_PARAMETER(m_szbc     , FE_PARAM_STRING, "bc_set"  );
	ADD_PARAMETER(m_rveType  , FE_PARAM_INT   , "rve_type" );
	ADD_PARAMETER(m_scale    , FE_PARAM_DOUBLE, "scale");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FEMicroMaterial2O::FEMicroMaterial2O(FEModel* pfem) : FEElasticMaterial2O(pfem)
{
	// initialize parameters
	m_szrve[0] = 0;
	m_szbc[0] = 0;
	m_rveType = FERVEModel2O::DISPLACEMENT;
	m_scale = 1.0;

	AddProperty(&m_probe, "probe", false);
}

//-----------------------------------------------------------------------------
FEMicroMaterial2O::~FEMicroMaterial2O(void)
{
}

//-----------------------------------------------------------------------------
FEMaterialPoint* FEMicroMaterial2O::CreateMaterialPointData()
{
	return new FEMicroMaterialPoint2O(new FEElasticMaterialPoint2O(new FEElasticMaterialPoint()));
}

//-----------------------------------------------------------------------------
bool FEMicroMaterial2O::Init()
{
	// initialize base class first
	if (FEElasticMaterial::Init() == false) return false;

	// load the master RVE model
	FEBioImport fim;
	if (fim.Load(m_mrve, m_szrve) == false)
	{
		return MaterialError("An error occured trying to read the RVE model from file %s.", m_szrve);
	}

	// the logfile is a shared resource between the master FEM and the RVE
	// in order not to corrupt the logfile we don't print anything for
	// the RVE problem.
	Logfile::MODE nmode = felog.GetMode();
	felog.SetMode(Logfile::LOG_NEVER);

	// scale geometry
	m_mrve.ScaleGeometry(m_scale);

	// initialize master RVE
	if (m_mrve.InitRVE(m_rveType, m_szbc) == false) return MaterialError("An error occurred preparing RVE model");

	// reset the logfile mode
	felog.SetMode(nmode);

	return true;
}

//-----------------------------------------------------------------------------
void FEMicroMaterial2O::Stress(FEMaterialPoint &mp, mat3d& P, tens3drs& Q)
{
	// get the deformation gradient and its gradient
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
	FEElasticMaterialPoint2O& pt2 = *mp.ExtractData<FEElasticMaterialPoint2O>();
	FEMicroMaterialPoint2O& mmpt2O = *mp.ExtractData<FEMicroMaterialPoint2O>();

	// get the deformation gradient and its gradient
	const mat3d& F = pt.m_F;
	const tens3drs& G = pt2.m_G;

	// solve the RVE
	bool bret = mmpt2O.m_rve.Solve(F, G);

	// make sure it converged
	if (bret == false) throw FEMultiScaleException(mmpt2O.m_elem_id, mmpt2O.m_gpt_id);

	// calculate the averaged Cauchy stress
	mmpt2O.m_rve.AveragedStress2O(P, Q);
}

//-----------------------------------------------------------------------------
void FEMicroMaterial2O::Tangent(FEMaterialPoint& mp, tens4d& C, tens5d& L, tens5d& H, tens6d& J)
{
	FEMicroMaterialPoint2O& mmpt2O = *mp.ExtractData<FEMicroMaterialPoint2O>();
	
	// calculate the averaged stiffness here
	mmpt2O.m_rve.AveragedStiffness(mp, C, L, H, J);
}
