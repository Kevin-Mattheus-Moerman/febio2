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
#include <FEBioMix/FEMultiphasicStandard.h>

//  This material implements a FEMultiphasicStandard material where
//  inhomogeneous fixed charge density may be specified for each
//  element in the mesh data description.  The FCD at the element
//  level is multiplied by the FCD at the material level, to account
//  for a loadcurve associated with the material level FCD.

class FEFCDMaterialPoint : public FESolutesMaterialPoint
{
public:
    FEFCDMaterialPoint(FEMaterialPoint* ppt);
    
    void Init(bool bflag);
    
public:
    double    m_cFr;
    
    DECLARE_PARAMETER_LIST();
};

class FEMultiphasicFCD : public FEMultiphasicStandard
{
public:
    FEMultiphasicFCD(FEModel* pfem) : FEMultiphasicStandard(pfem){}
    
    FEMaterialPoint* CreateMaterialPointData() override;
    
    //! fixed charge density
    double FixedChargeDensity(FEMaterialPoint& pt) override;
    
};
