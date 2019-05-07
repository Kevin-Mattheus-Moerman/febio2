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


// Interrupt.cpp: implementation of the Interrupt class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Interrupt.h"
#include "CommandManager.h"
#include "console.h"
#include <signal.h>

bool Interruption::m_bsig = false;

Interruption::Interruption()
{
	static bool binit = false;

	if (!binit) 
	{
		signal(SIGINT, Interruption::handler);
		binit = true;
	}
}

//-----------------------------------------------------------------------------
//! Destructor
//! \todo Restore original intteruption handler
Interruption::~Interruption()
{
	
}

void Interruption::handler(int sig)
{
	m_bsig = true;
	signal(SIGINT, Interruption::handler);
}

void Interruption::interrupt()
{
	// get a pointer to the console window
	Console* pShell = Console::GetHandle();

	// get a pointer to the command manager
	CommandManager* pCM = CommandManager::GetInstance();

	int nargs;
	char* argv[32];

	// enter command loop
	while (1)
	{
		// get a command from the shell
		pShell->GetCommand(nargs, argv);
		if (nargs > 0)
		{
			// find the command that has this name
			Command* pcmd = pCM->Find(argv[0]);
			if (pcmd)
			{
				int nret = pcmd->run(nargs, argv);
				if (nret == 1) break;
			}
			else
			{
				printf("Unknown command: %s\n", argv[0]);
			}
		}
	}
}
