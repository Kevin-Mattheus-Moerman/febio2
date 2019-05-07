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
#include "FEConstrainedLMOptimizeMethod.h"
#include "FEOptimizeData.h"
#include "FEOptimizeInput.h"
#include "FECore/FEAnalysis.h"
#include "FECore/log.h"

#ifdef HAVE_LEVMAR
#include "levmar.h"

//-----------------------------------------------------------------------------
BEGIN_PARAMETER_LIST(FEConstrainedLMOptimizeMethod, FEOptimizeMethod)
	ADD_PARAMETER(m_objtol, FE_PARAM_DOUBLE, "obj_tol"     );
	ADD_PARAMETER(m_tau   , FE_PARAM_DOUBLE, "tau"         );
	ADD_PARAMETER(m_fdiff , FE_PARAM_DOUBLE, "f_diff_scale");
	ADD_PARAMETER(m_nmax  , FE_PARAM_INT   , "max_iter"    );
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FEConstrainedLMOptimizeMethod* FEConstrainedLMOptimizeMethod::m_pThis = 0;

//-----------------------------------------------------------------------------
void clevmar_cb(double *p, double *hx, int m, int n, void *adata)
{
	FEConstrainedLMOptimizeMethod* pLM = (FEConstrainedLMOptimizeMethod*) adata;

	// get the optimization data
	FEOptimizeData& opt = *pLM->GetOptimizeData();
	FEObjectiveFunction& obj = opt.GetObjective();

	// evaluate at a
	vector<double> a(m);
	for (int i = 0; i<m; ++i) a[i] = p[i];
	if (opt.FESolve(a) == false) throw FEErrorTermination();

	// store the measurement vector
	vector<double> y(n, 0.0);
	opt.GetObjective().Evaluate(y);
	for (int i=0; i<n; ++i) hx[i] = y[i];

	// store the last calculated values
	pLM->m_yopt = y;
}

//-----------------------------------------------------------------------------
FEConstrainedLMOptimizeMethod::FEConstrainedLMOptimizeMethod()
{
	m_tau = 1e-3;
	m_objtol = 0.001;
	m_fdiff  = 0.001;
	m_nmax   = 100;
    m_loglevel = Logfile::LOG_NEVER;
}

//-----------------------------------------------------------------------------
bool FEConstrainedLMOptimizeMethod::Solve(FEOptimizeData *pOpt, vector<double>& amin, vector<double>& ymin, double* minObj)
{
	m_pOpt = pOpt;
	FEOptimizeData& opt = *pOpt;

	// set the variables
	int ma = opt.InputParameters();
	vector<double> a(ma);
	for (int i=0; i<ma; ++i)
	{
		FEInputParameter& var = *opt.GetInputParameter(i);
		a[i] = var.GetValue();
	}

	// get the data
	FEObjectiveFunction& obj = opt.GetObjective();
	int ndata = obj.Measurements();
	vector<double> y(ndata, 0);
	obj.GetMeasurements(y);

	// set the sigma's
	// for now we set them all to 1
	vector<double> sig(ndata);
	for (int i=0; i<ndata; ++i) sig[i] = 1;

	// allocate matrices
	matrix covar(ma, ma), alpha(ma, ma);

	// set the this pointer
	m_pThis = this;

	opt.m_niter = 0;

	// return value
	double fret = 0.0;

	felog.SetMode(Logfile::LOG_FILE_AND_SCREEN);

	int niter = 1;

	try
	{
		double* p = new double[ma];
		for (int i=0; i<ma; ++i) p[i] = a[i];

		double* lb = new double[ma];
		for (int i=0; i<ma; ++i) lb[i] = opt.GetInputParameter(i)->MinValue();

		double* ub = new double[ma];
		for (int i=0; i<ma; ++i) ub[i] = opt.GetInputParameter(i)->MaxValue();

		double* q = new double[ndata];
		for (int i=0; i<ndata; ++i) q[i] = y[i];

		const double tol = m_objtol;
		double opts[5] = {m_tau, tol, tol, tol, m_fdiff};

		int itmax = m_nmax;
		if (opt.Constraints() > 0)
		{
			int NC = opt.Constraints();
			double* A = new double[NC*ma];
			double* b = new double[NC];
			for (int i=0; i<NC; ++i)
			{
				OPT_LIN_CONSTRAINT& con = opt.Constraint(i);
				for (int j=0; j<ma; ++j) A[i*ma + j] = con.a[j];
				b[i] = con.b;
			}

			int ret = dlevmar_blec_dif(clevmar_cb, p, q, ma, ndata, lb, ub, A, b, NC, 0, itmax, opts, 0, 0, 0, (void*) this);

			delete [] b;
			delete [] A;
		}
		else
		{
			int ret = dlevmar_bc_dif(clevmar_cb, p, q, ma, ndata, lb, ub, 0, itmax, opts, 0, 0, 0, (void*) this);
		}

		for (int i=0; i<ma; ++i) a[i] = p[i];

		// store the optimal values
		fret = obj.Evaluate(m_yopt);

		delete [] q;
		delete [] ub;
		delete [] lb;
		delete [] p;
	}
	catch (FEErrorTermination)
	{
		felog.printbox("F A T A L   E R R O R", "FEBio error terminated. Parameter optimization cannot continue.");
		return false;
	}

	// return optimal values
	amin = a;
	ymin = m_yopt;
	if (minObj) *minObj = fret;
    
	return true;
}

//-----------------------------------------------------------------------------
void FEConstrainedLMOptimizeMethod::ObjFun(vector<double>& x, vector<double>& a, vector<double>& y, matrix& dyda)
{
	// get the optimization data
	FEOptimizeData& opt = *m_pOpt;

	// poor man's box constraints
	int ma = (int)a.size();
	vector<int> dir(ma,1);	// forward difference by default
	for (int i=0; i<opt.InputParameters(); ++i)
	{
		FEInputParameter& var = *opt.GetInputParameter(i);
		if (a[i] < var.MinValue()) {
			a[i] = var.MinValue();
		} else if (a[i] >= var.MaxValue()) {
			a[i] = var.MaxValue();
			dir[i] = -1;	// use backward difference
		}
	}
	
	// evaluate at a
	if (opt.FESolve(a) == false) throw FEErrorTermination();
	opt.GetObjective().Evaluate(y);
	m_yopt = y;

	// now calculate the derivatives using forward differences
	int ndata = (int)x.size();
	vector<double> a1(a);
	vector<double> y1(ndata);
	for (int i=0; i<ma; ++i)
	{
		double b = opt.GetInputParameter(i)->ScaleFactor();

		a1[i] = a1[i] + dir[i]*m_fdiff*(b + fabs(a[i]));

		if (opt.FESolve(a1) == false) throw FEErrorTermination();
		opt.GetObjective().Evaluate(y1);
		for (int j=0; j<ndata; ++j) dyda[j][i] = (y1[j] - y[j])/(a1[i] - a[i]);
		a1[i] = a[i];
	}
}

#endif
