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
#include "febio.h"
#include "validate.h"
#include "febio.h"
#include <string.h>

#ifndef FEBIOLM
int GetLicenseKeyStatus(const char* licenseKey)
{
	return 0;
}
#else
//#include <febiolm.h>

#endif

std::string LoadLicenseKey()
{
	// Get the location of the executable
	char szpath[1024] = { 0 };
	febio::get_app_path(szpath, 1023);

	// append the file name
	strcat(szpath, "license.txt");

	// try to open the file
	std::string licenseKey;
	FILE* fp = fopen(szpath, "rt");
	if (fp != NULL)
	{
		char szbuf[128] = {0};
		fgets(szbuf, 127, fp);
		licenseKey = szbuf;
		fclose(fp);
	}
	return licenseKey;
}
