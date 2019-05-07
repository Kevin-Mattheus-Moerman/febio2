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
#include "Logfile.h"
#include <stdarg.h>
#include <string.h>

//-----------------------------------------------------------------------------
LogFileStream::LogFileStream()
{
	m_fp = 0;
}

//-----------------------------------------------------------------------------
LogFileStream::~LogFileStream()
{
	close();
}

//-----------------------------------------------------------------------------
void LogFileStream::close()
{
	if (m_fp) fclose(m_fp);
	m_fp = 0;
}

//-----------------------------------------------------------------------------
void LogFileStream::flush()
{
	if (m_fp) fflush(m_fp);
}

//-----------------------------------------------------------------------------
bool LogFileStream::open(const char* szfile)
{
	if (m_fp) close();
	m_fp = fopen(szfile, "wt");
	return (m_fp != NULL);
}

//-----------------------------------------------------------------------------
bool LogFileStream::append(const char* szfile)
{
	// make sure we don't have a log file already open
	if (m_fp)
	{
		fseek(m_fp, SEEK_END, 0);
		return true;
	}

	// create the log file
	m_fp = fopen(szfile, "a+t");

	return (m_fp != NULL);
}

//-----------------------------------------------------------------------------
void LogFileStream::print(const char* sztxt)
{
	if (m_fp) fprintf(m_fp, "%s", sztxt);
}

//=============================================================================
// The one-and-only logfile
//Logfile& felog = *Logfile::GetInstance();

//-----------------------------------------------------------------------------
Logfile* Logfile::m_plog = 0;

//-----------------------------------------------------------------------------
Logfile* Logfile::GetInstance()
{
	if (m_plog == 0) m_plog = new Logfile();
	return m_plog;
}

//-----------------------------------------------------------------------------
// constructor for the Logfile class
Logfile::Logfile()
{
	m_fp = 0;
	m_szfile[0] = 0;

	m_ps = 0;

	m_mode = LOG_FILE_AND_SCREEN;
}

//-----------------------------------------------------------------------------
// destructor for the Logfile class
//
Logfile::~Logfile()
{
	close();
	m_plog = 0;
}

//-----------------------------------------------------------------------------
// open a file
//
bool Logfile::open(const char* szfile)
{
	strcpy(m_szfile, szfile);
	if (m_fp == 0) m_fp = new LogFileStream;
	return m_fp->open(szfile);
}

//-----------------------------------------------------------------------------
//  opens a file and prepares for appending
//
bool Logfile::append(const char* szfile)
{
	// store a copy of the filename
	if (m_fp == 0) m_fp = new LogFileStream;
	strcpy(m_szfile, szfile);
	return m_fp->append(szfile);
}

//-----------------------------------------------------------------------------
//! flush the logfile
void Logfile::flush()
{
	if (m_fp) m_fp->flush(); 
	if (m_ps) m_ps->flush();
}

//-----------------------------------------------------------------------------
//! close the logfile
void Logfile::close()
{ 
	if (m_fp)
	{
		m_fp->close(); 
		delete m_fp;
		m_fp = 0;
	}
}

//-----------------------------------------------------------------------------
// This function works like all other printf functions
// with the exception that everything that is output to the file
// is (optionally) also output to the screen.
//
void Logfile::printf(const char* sz, ...)
{
	static char szmsg[1024] = {0};

	// get a pointer to the argument list
	va_list	args;

	// make the message
	static char sztxt[1024] = {0};
	va_start(args, sz);
	vsprintf(sztxt, sz, args);
	va_end(args);
	
	// print to file
	if (m_fp && (m_mode & LOG_FILE)) m_fp->print(sztxt);

	// print to screen
	if (m_ps && (m_mode & LOG_SCREEN)) m_ps->print(sztxt);
}

//-----------------------------------------------------------------------------
// FUNCTION: Logfile::printbox
// This function prints a message insided a box
//
void Logfile::printbox(const char* sztitle, const char* sz, ...)
{
	// get a pointer to the argument list
	va_list	args;

	// make the message
	static char sztxt[1024] = {0};
	va_start(args, sz);
	vsprintf(sztxt, sz, args);
	va_end(args);

	// print the box
	char szmsg[1024] = {0};
	char* ch = szmsg;
	sprintf(szmsg," *************************************************************************\n"); ch += strlen(ch);
	// print the title
	if (sztitle)
	{
		int l = (int)strlen(sztitle);
		char left[60] = {0};
		char right[60] = {0};
		strncpy(left, sztitle, l/2);
		strncpy(right, sztitle+l/2, l - l/2);
		sprintf(ch," * %33s", left); ch += strlen(ch);
		sprintf(ch,"%-36s *\n", right); ch += strlen(ch);
		sprintf(ch," *%71s*\n", ""); ch += strlen(ch);
	}

	// print the message
	char* ct = sztxt, *cn;
	do
	{
		cn = strchr(ct,'\n');
		if (cn) *cn = 0;
		sprintf(ch," * %-69s *\n", ct); ch += strlen(ch);
		if (cn) ct = cn+1;
	}
	while (cn);
	sprintf(ch," *************************************************************************\n");

	// print the message
	printf(szmsg);
}


//-----------------------------------------------------------------------------
// Sets the Logfile mode. 
//
Logfile::MODE Logfile::SetMode(Logfile::MODE mode)
{
	MODE old = m_mode;
	m_mode = mode;
	return old;
}
