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
#include "FEDamageCDF.h"
#include "FEDamageCriterion.h"
#include "FEDamageMaterialPoint.h"

#ifndef M_PI
#define M_PI	3.14159265359
#endif

#ifdef HAVE_GSL
#include "gsl/gsl_sf_gamma.h"
#endif

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// define the material parameters
BEGIN_PARAMETER_LIST(FEDamageCDF, FEMaterial)
ADD_PARAMETER2(m_Dmax, FE_PARAM_DOUBLE, FE_RANGE_CLOSED(0.0, 1.0), "Dmax");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
//! Constructor.
double FEDamageCDF::Damage(FEMaterialPoint& mp) {
    
    // get the damage material point data
    FEDamageMaterialPoint& dp = *mp.ExtractData<FEDamageMaterialPoint>();
    
    // get the damage criterion (assuming dp.m_Etrial is up-to-date)
    double Es = max(dp.m_Etrial, dp.m_Emax);

    dp.m_D = cdf(Es)*m_Dmax;
    
    return dp.m_D;
}

//-----------------------------------------------------------------------------
//! Constructor.
bool FEDamageCDF::SolveCDF(const double f0, double& x)
{
    const double errrel = 1e-6;
    const double errabs = 1e-6;
    const int nmax = 20;
    int niter = 0;
    
    bool bdone = false;
    double dx = 0, f = 0;
    
    f = f0 - cdf(x);
    while (!bdone) {
        double df = pdf(x);
        if (fabs(df) > 0) {
            dx = f/pdf(x);
            x += dx;
            f = f0 - cdf(x);
            if ((fabs(f) <= errabs) || (fabs(dx) <= errrel*fabs(x))) return true;
        }
        if (++niter == nmax) bdone = true;
    }
    
    return false;
}

///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// define the material parameters
BEGIN_PARAMETER_LIST(FEDamageCDFSimo, FEDamageCDF)
    ADD_PARAMETER2(m_alpha, FE_PARAM_DOUBLE, FE_RANGE_GREATER(0.0), "a");
    ADD_PARAMETER2(m_beta, FE_PARAM_DOUBLE , FE_RANGE_CLOSED(0.0, 1.0), "b");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
//! Constructor.
FEDamageCDFSimo::FEDamageCDFSimo(FEModel* pfem) : FEDamageCDF(pfem)
{
}

//-----------------------------------------------------------------------------
// Simo damage cumulative distribution function
// Simo, CMAME 60 (1987), 153-173
// Simo damage cumulative distribution function
double FEDamageCDFSimo::cdf(const double X)
{
    if (m_alpha == 0) {
        return 0;
    }
    
    double cdf = 0;
    
    // this CDF only admits positive values
    if (X >= 0) {
        if (X > 1e-12) cdf = 1 - m_beta - (1.0 - m_beta)*(1.0 - exp(-X/m_alpha))*m_alpha/X;
        else cdf = 0.5*(1.0 - m_beta)/m_alpha*X;
    }
    
    return cdf;
}

// Simo damage probability density function
double FEDamageCDFSimo::pdf(const double X)
{
    if (m_alpha == 0) {
        return 0;
    }
    
    double pdf = 0;
    
    // this CDF only admits positive values
    if (X >= 0) {
        if (X > 1e-12) pdf = (1.0 - m_beta)*(m_alpha - (m_alpha + X)*exp(-X/m_alpha))/(X*X);
        else pdf = (1.0 - m_beta)/m_alpha*(0.5 - X/3/m_alpha);
    }
    
    return pdf;
}

///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// define the material parameters
BEGIN_PARAMETER_LIST(FEDamageCDFLogNormal, FEDamageCDF)
    ADD_PARAMETER2(m_mu , FE_PARAM_DOUBLE  , FE_RANGE_GREATER(0.0), "mu");
    ADD_PARAMETER2(m_sigma, FE_PARAM_DOUBLE, FE_RANGE_GREATER(0.0), "sigma");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
//! Constructor.
FEDamageCDFLogNormal::FEDamageCDFLogNormal(FEModel* pfem) : FEDamageCDF(pfem)
{
    m_mu = 1;
    m_sigma = 1;
    m_Dmax = 1;
}

//-----------------------------------------------------------------------------
// Lognormal damage cumulative distribution function
double FEDamageCDFLogNormal::cdf(const double X)
{
    double cdf = 0;
    
    // this CDF only admits positive values
    if (X >= 0)
        cdf = 0.5*erfc(-log(X/m_mu)/m_sigma/sqrt(2.));
    
    return cdf;
}

// Lognormal damage probability density function
double FEDamageCDFLogNormal::pdf(const double X)
{
    double pdf = 0;
    
    // this CDF only admits positive values
    if (X > 1e-12) pdf = exp(-pow(log(X/m_mu)/m_sigma,2)/2)/(sqrt(2*M_PI)*X*m_sigma);
    
    return pdf;
}

///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// define the material parameters
BEGIN_PARAMETER_LIST(FEDamageCDFWeibull, FEDamageCDF)
    ADD_PARAMETER2(m_alpha, FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(1.0), "alpha");
    ADD_PARAMETER2(m_mu, FE_PARAM_DOUBLE   , FE_RANGE_GREATER_OR_EQUAL(0.0), "mu");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
//! Constructor.
FEDamageCDFWeibull::FEDamageCDFWeibull(FEModel* pfem) : FEDamageCDF(pfem)
{
    m_alpha = m_mu;
}

//-----------------------------------------------------------------------------
// Weibull damage cumulative distribution function
double FEDamageCDFWeibull::cdf(const double X)
{
    double cdf = 0;
    
    // this CDF only admits positive values
    if (X > 0)
        cdf = 1 - exp(-pow(X/m_mu,m_alpha));
    
    return cdf;
}

// Weibull damage probability density function
double FEDamageCDFWeibull::pdf(const double X)
{
    double pdf = 0;
    
    // this CDF only admits positive values
    if ((m_alpha > 1) && (X > 0))
        pdf = exp(-pow(X/m_mu,m_alpha))*m_alpha*pow(X, m_alpha-1)/pow(m_mu, m_alpha);
    else if (m_alpha == 1)
        pdf = exp(-X/m_mu)/m_mu;
    
    return pdf;
}

///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// define the material parameters
BEGIN_PARAMETER_LIST(FEDamageCDFStep, FEDamageCDF)
    ADD_PARAMETER2(m_mu, FE_PARAM_DOUBLE  , FE_RANGE_GREATER_OR_EQUAL(0.0), "mu");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
//! Constructor.
FEDamageCDFStep::FEDamageCDFStep(FEModel* pfem) : FEDamageCDF(pfem)
{
    m_mu = 1;
}

//-----------------------------------------------------------------------------
// Step cumulative distribution function (sudden fracture)
// Step damage cumulative distribution function
double FEDamageCDFStep::cdf(const double X)
{
    double cdf = 0;
    
    // this CDF only admits positive values
    if (X > m_mu)
        cdf = 1.0;
    
    return cdf;
}

// Step damage probability density function
double FEDamageCDFStep::pdf(const double X)
{
    double pdf = 0;
    
    // this PDF only admits positive values
    if (X == m_mu) pdf = 1.0;
    
    return pdf;
}

///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// define the material parameters
BEGIN_PARAMETER_LIST(FEDamageCDFPQP, FEDamageCDF)
    ADD_PARAMETER2(m_mumin, FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "mumin");
    ADD_PARAMETER2(m_mumax, FE_PARAM_DOUBLE, FE_RANGE_GREATER_OR_EQUAL(0.0), "mumax");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
//! Constructor.
FEDamageCDFPQP::FEDamageCDFPQP(FEModel* pfem) : FEDamageCDF(pfem)
{
    m_mumin = 0;
    m_mumax = 1;
}

//-----------------------------------------------------------------------------
//! Initialization.
bool FEDamageCDFPQP::Validate()
{
	if (m_mumax <= m_mumin) return MaterialError("mumax must be > mumin");
	return FEDamageCDF::Validate();
}

//-----------------------------------------------------------------------------
// Piecewise S-shaped quintic polynomial damage cumulative distribution function
double FEDamageCDFPQP::cdf(const double X)
{
    double cdf = 0;
    
    if (X <= m_mumin) cdf = 0;
    else if (X >= m_mumax) cdf = 1;
    else
    {
        double x = (X - m_mumin)/(m_mumax - m_mumin);
        cdf = pow(x,3)*(10 - 15*x + 6*x*x);
    }

    return cdf;
}

// Piecewise S-shaped quintic polynomial damage probability density function
double FEDamageCDFPQP::pdf(const double X)
{
    double pdf = 0;
    
    if (X <= m_mumin) pdf = 0;
    else if (X >= m_mumax) pdf = 0;
    else
    {
        double x = (X - m_mumin)/(m_mumax - m_mumin);
        pdf = pow(x*(x-1),2)*30;
    }

    return pdf;
}

///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// define the material parameters
BEGIN_PARAMETER_LIST(FEDamageCDFGamma, FEDamageCDF)
ADD_PARAMETER2(m_alpha, FE_PARAM_DOUBLE, FE_RANGE_GREATER(0)           , "alpha");
ADD_PARAMETER2(m_mu, FE_PARAM_DOUBLE   , FE_RANGE_GREATER_OR_EQUAL(0.0), "mu");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
//! Constructor.
FEDamageCDFGamma::FEDamageCDFGamma(FEModel* pfem) : FEDamageCDF(pfem)
{
    m_alpha = 2;
    m_mu = 4;
}

//-----------------------------------------------------------------------------
// Weibull damage cumulative distribution function
double FEDamageCDFGamma::cdf(const double X)
{
    double cdf = 0;
    
    // this CDF only admits positive values
#ifdef HAVE_GSL
    if (X > 0)
        cdf = gsl_sf_gamma_inc_P(m_alpha,X/m_mu);
#endif
    
    return cdf;
}

// Weibull damage probability density function
double FEDamageCDFGamma::pdf(const double X)
{
    double pdf = 0;
    
    // this CDF only admits positive values
#ifdef HAVE_GSL
    if (X > 0)
        pdf = pow(X/m_mu, m_alpha-1)*exp(-X/m_mu)/m_mu*gsl_sf_gammainv(m_alpha);
#endif
    
    return pdf;
}
