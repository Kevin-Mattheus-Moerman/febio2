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
#include <cstddef>
#include <FECore/FEBox.h>
#include <FECore/quatd.h>
#include "febiolib_api.h"

//-----------------------------------------------------------------------------
// Defines the FEBio namespace
namespace febio 
{
	// Initialize all the FEBio modules
	FEBIOLIB_API void InitLibrary();

	// read the configuration file
	FEBIOLIB_API bool Configure(const char* szfile);

	// load a plugin
	FEBIOLIB_API bool ImportPlugin(const char* szfile);

	// call this to clean up all FEBio data
	FEBIOLIB_API void FinishLibrary();

	// helper function for retrieving the executable's path
	FEBIOLIB_API int get_app_path(char *pname, size_t pathsize);

	// print hello message
	FEBIOLIB_API int Hello();

	// set the number of OMP threads
	FEBIOLIB_API void SetOMPThreads(int n);
}

//-----------------------------------------------------------------------------
// These are some dummy classes to force export of the base class
// TODO: Probably should find a better solution. Maybe create a DLL for the FECore library
class _dummy_FEBox : public FEBox {};
class _dummy_quatd : public quatd {};
