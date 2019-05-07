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
#include <map>
#include <string>
using namespace std;
#include "fecore_api.h"

class FECORE_API MathParser
{
protected:
	enum Token_value {
		NAME,	NUMBER, END,
		PLUS='+', MINUS='-', MUL='*', DIV='/', POW='^',
		LP='(',	RP=')', PRINT
	};

public:
	MathParser();
	virtual ~MathParser();

	double eval(const char* szexpr, int& ierr);

	const char* error_str() { return m_szerr; }

	void SetVariable(const char* szvar, double g = 0);

protected:
	double expr();	// add and subtract
	double term();	// multiply and divide
	double prim();	// handle primaries
	double power();	// power
	Token_value get_token();
	double error(const char* str);

	double get_number();
	void get_name(char* str);

	Token_value	curr_tok;

	const char* m_szexpr;

	map<std::string, double> m_table;	// table that stores variables and constants

	double	number_value;
	char	string_value[256];

	char	m_szerr[256];

	int		m_nerrs;
};
