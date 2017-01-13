/*
 * FILE : compiler.l
 * SYSTEM : RunQ (RunQ/compiler/compiler.l
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 * 
 * CREATED : 03 jan 2000
 * VERSION : 1.00 (18-mar-2000)
 *
 * DESCRIPTION:
 *    Lexer for the workload definition file 
 *
 * CHANGE-LOG:
 *
 */

/*  GNU General Public License
 *
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

%option yylineno

%{
#include "mytype.h"
#include "y.tab.h" 
#define yywrap() (1)

extern YYSTYPE yylval;
%}

ID	[A-Za-z_][A-Za-z0-9_]*
INTEGER [0-9]+
FLOAT   [0-9]+.[0-9]+

%%

{FLOAT}              { yylval.value = atof(yytext);
                       return VALUE; }
"-"                  { return MINUS; }
"{"                  { return OPEN_BRACE; }
"}"                  { return CLOSE_BRACE; }
"\""[^"]*"\""        { memcpy(yylval.str, yytext+1, yyleng-2); 
                       yylval.str[yyleng-2] = '\0';
                       return STRING; }
"DATE"               { return KEYWORD_DATE; }
"TIME"               { return KEYWORD_TIME; }
"#"[^\n]*            { /* eat up */ }
%%
