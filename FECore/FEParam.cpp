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
#include "FEParam.h"
#include "FEParamValidator.h"
#include "DumpStream.h"
#include "FEFunction1D.h"
#include "FEDataArray.h"
#include "tens3d.h"
#include "FEMathValue.h"

void FEParamValue::Serialize(DumpStream& ar)
{
	if (ar.IsSaving())
	{
		ar << (int) m_itype;
		ar << m_ndim;
		if (m_ndim == 1)
		{
			switch (m_itype)
			{
			case FE_PARAM_INT       : ar << value<int>(); break;
			case FE_PARAM_BOOL      : ar << value<bool>(); break;
			case FE_PARAM_DOUBLE    : ar << value<double>(); break;
			case FE_PARAM_VEC3D     : ar << value<vec3d>(); break;
			case FE_PARAM_MAT3D     : ar << value<mat3d>(); break;
			case FE_PARAM_MAT3DS    : ar << value<mat3ds>(); break;
			case FE_PARAM_TENS3DRS  : ar << value<tens3ds>(); break;
			case FE_PARAM_DATA_ARRAY:
			{
				FEDataArray& m = value<FEDataArray>();
				m.Serialize(ar);
			}
			break;
			case FE_PARAM_STRING: ar << (const char*)data_ptr(); break;
			case FE_PARAM_FUNC1D:
			{
				FEFunction1D& f = value<FEFunction1D>();
				f.Serialize(ar);
			}
			break;
			case FE_PARAM_MATH_DOUBLE:
			{
				FEMathDouble& p = value<FEMathDouble>();
				p.Serialize(ar);
			}
			break;
			default:
				assert(false);
			}
		}
		else
		{
			switch (m_itype)
			{
			case FE_PARAM_INT:
			{
				int* pi = (int*) m_pv;
				for (int i = 0; i<m_ndim; ++i) ar << pi[i];
			}
			break;
			case FE_PARAM_DOUBLE:
			{
				double* pv = (double*) m_pv;
				for (int i = 0; i<m_ndim; ++i) ar << pv[i];
			}
			break;
			default:
				assert(false);
			}
		}
	}
	else
	{
		int ntype, ndim;
		ar >> ntype;
		ar >> ndim;
		if (ndim != m_ndim) throw DumpStream::ReadError();
		if (ntype != (int)m_itype) throw DumpStream::ReadError();
		if (m_ndim == 1)
		{
			switch (m_itype)
			{
			case FE_PARAM_INT       : ar >> value<int         >(); break;
			case FE_PARAM_BOOL      : ar >> value<bool        >(); break;
			case FE_PARAM_DOUBLE    : ar >> value<double      >(); break;
			case FE_PARAM_VEC3D     : ar >> value<vec3d       >(); break;
			case FE_PARAM_MAT3D     : ar >> value<mat3d       >(); break;
			case FE_PARAM_MAT3DS    : ar >> value<mat3ds      >(); break;
			case FE_PARAM_TENS3DRS  : ar >> value<tens3drs>(); break;
			case FE_PARAM_DATA_ARRAY:
			{
				FEDataArray& m = value<FEDataArray>();
				m.Serialize(ar);
			}
			break;
			case FE_PARAM_STRING: ar >> (char*)data_ptr(); break;
			case FE_PARAM_FUNC1D:
			{
				FEFunction1D& f = value<FEFunction1D>();
				f.Serialize(ar);
			}
			break;
			case FE_PARAM_MATH_DOUBLE:
			{
				FEMathDouble& p = value<FEMathDouble>();
				p.Serialize(ar);
			}
			break;
			default:
				assert(false);
			}
		}
		else
		{
			switch (m_itype)
			{
			case FE_PARAM_INT:
			{
				int* pi = (int*)data_ptr();
				for (int i = 0; i<m_ndim; ++i) ar >> pi[i];
			}
			break;
			case FE_PARAM_DOUBLE:
			{
				double* pv = (double*)data_ptr();
				for (int i = 0; i<m_ndim; ++i) ar >> pv[i];
			}
			break;
			default:
				assert(false);
			}
		}
	}
}


//-----------------------------------------------------------------------------
FEParam::FEParam(void* pdata, FEParamType itype, int ndim, const char* szname) : m_val(pdata, itype, ndim)
{
	m_nlc = -1;
	m_scl = 1.0;
	m_vscl = vec3d(0, 0, 0);

	// set the name
	// note that we just copy the pointer, not the actual string
	// this is okay as long as the name strings are defined
	// as literal strings
	m_szname = szname;

	m_szenum = 0;

	m_pvalid = 0;	// no default validator
}

//-----------------------------------------------------------------------------
FEParam::FEParam(const FEParam& p) : m_val(p.m_val) 
{
	m_nlc = p.m_nlc;
	m_scl = p.m_scl;
	m_vscl = p.m_vscl;
	m_szname = p.m_szname;
	m_szenum = 0;

	m_pvalid = (p.m_pvalid ? p.m_pvalid->copy() : 0);
}

//-----------------------------------------------------------------------------
FEParam& FEParam::operator=(const FEParam& p)
{
	m_val = p.m_val;
	m_nlc = p.m_nlc;
	m_scl = p.m_scl;
	m_vscl = p.m_vscl;
	m_szname = p.m_szname;
	m_szenum = 0;

	if (m_pvalid) delete m_pvalid;
	m_pvalid = (p.m_pvalid ? p.m_pvalid->copy() : 0);

	return *this;
}

//-----------------------------------------------------------------------------
bool FEParam::is_valid() const
{
	if (m_pvalid) return m_pvalid->is_valid(*this);
	return true;
}

//-----------------------------------------------------------------------------
//! This function deletes the existing validator and replaces it with the parameter
//! passed in the function. 
//! The pvalid can be null in which case the parameter will no longer be validated.
//! (i.e. is_valid() will always return true.
//! TODO: Should I delete the validator here? What if it was allocated in a plugin?
//!       Perhaps I should just return the old validator?
void FEParam::SetValidator(FEParamValidator* pvalid)
{
	if (m_pvalid) delete m_pvalid;
	m_pvalid = pvalid;
}

//-----------------------------------------------------------------------------
//! Sets the load curve ID and scale factor
void FEParam::SetLoadCurve(int lc)
{
	m_nlc = lc;
}

//-----------------------------------------------------------------------------
//! Sets the load curve ID and scale factor
void FEParam::SetLoadCurve(int lc, double s)
{
	assert(m_val.type() == FE_PARAM_DOUBLE);
	m_nlc = lc;
	m_scl = s;
}

//-----------------------------------------------------------------------------
//! Sets the load curve ID and scale factor
void FEParam::SetLoadCurve(int lc, const vec3d& v)
{
	assert(m_val.type() == FE_PARAM_VEC3D);
	m_nlc = lc;
	m_vscl = v;
}

//-----------------------------------------------------------------------------
void FEParam::Serialize(DumpStream& ar)
{
	// serialize the value
	m_val.Serialize(ar);

	// serialize the parameter 
	if (ar.IsSaving())
	{
		ar << m_nlc;
		ar << m_scl;
		ar << m_vscl;
	}
	else
	{
		ar >> m_nlc;
		ar >> m_scl;
		ar >> m_vscl;
	}

	// serialize the validator
	if (m_pvalid) m_pvalid->Serialize(ar);
}

//-----------------------------------------------------------------------------
//! This function copies the state of a parameter to this parameter.
//! This assumes that the parameters are compatible (i.e. have the same type)
//! This is used in FEParamContainer::CopyParameterListState()
bool FEParam::CopyState(const FEParam& p)
{
	if (p.type() != type()) return false;

	m_nlc = p.m_nlc;
	m_scl = p.m_scl;
	m_vscl = p.m_vscl;

	return true;
}
