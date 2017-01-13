/*
 * FILE : compiler.l
 * SYSTEM : RunQ (RunQ/compiler/compiler.l)
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
 */

/* %option yylineno */

%{
#include "mytype.h"
#include "y.tab.h"
/* #include "compiler.tab.h" */
#define yywrap(x) (1)
#define YY_SKIP_YYWRAP

extern YYSTYPE yylval;
%}

ID	[A-Za-z_][A-Za-z0-9_]*

%%

workload	     { return WORKLOAD; }
processgroup         { return PROCESSGROUP; }
include	             { return INCLUDE; }
exclude	             { return EXCLUDE; }
with		     { return WITH; }
children	     { return CHILDREN; }
"{"	             { return OPENBRA; }
"}"                  { return CLOSEBRA; }
"+"                  { return PLUS; }
":"                  { return COLON; }
{ID}		     { strcpy(yylval.id, yytext); return IDENTIFIER; }
"\""[^"]*"\""        { memcpy(yylval.str+1, yytext+1, yyleng-2); 
                       yylval.str[0] = '^';
                       yylval.str[yyleng-1] = '$';
                       yylval.str[yyleng] = '\0';
                       return STRING; }
"#"[^\n]*            { /* eat up */ }
%%
