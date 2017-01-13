/*
 * FILE : compiler.y
 * SYSTEM : RunQ (RunQ/compiler/compiler.y)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 * 
 * CREATED : 03 jan 2000
 * VERSION : 1.5 (03-aug-2000)
 *
 * DESCRIPTION:
 *    Yacc rules for the workload definition file compiler.
 *
 * CHANGELOG:
 *   1.5 Changed to compile on the fly for analyzer.
 *   1.4 Too much to fell, sorry.
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


%{
#include <stdlib.h>
#include <string.h>

#include "mytype.h"
#define YYSTYPE MyType
#define YYDEBUG 0
#define YYERROR_VERBOSE 1

#ifdef __hpux
#include <yacc.h>
#else
extern int yylex();
extern int yylineno;
#define yyerror(text) fprintf(stderr, "ERROR at line %d: %s\n", yylineno, text)
#endif // __hpux

 static Analyzer * anAnalyzer = 0;

%}

%token WORKLOAD
%token PROCESSGROUP
%token INCLUDE
%token EXCLUDE
%token WITH
%token CHILDREN
%token OPENBRA
%token CLOSEBRA
%token PLUS
%token COLON
%token IDENTIFIER
%token STRING

%%

wkl_definitions : wkl_definitions wkl_definition 
                    {
		      anAnalyzer->workLoads.push_back(*($2.workLoad));
		      delete $2.workLoad;
		    }
                |
                ;

wkl_definition : WORKLOAD IDENTIFIER OPENBRA processgroups CLOSEBRA
                   { $$.workLoad = new WorkLoad;
		     strcpy($$.workLoad->name, $2.id);
		     $$.workLoad->processGroups = *($4.processGroupList);
		     delete $4.processGroupList;
		   }
               ;

processgroups : processgroups processgroup
                 { $1.processGroupList->push_back(*($2.processGroup));
		   delete $2.processGroup;
		   $$ = $1;
		 }
	     | { $$.processGroupList = new std::vector<ProcessGroup>; }
             ;

processgroup : PROCESSGROUP IDENTIFIER WITH CHILDREN include exclude
                { $$.processGroup = new ProcessGroup;
		  strcpy($$.processGroup->name, $2.id);
		  $$.processGroup->withChildren = true;
		  $$.processGroup->includeProcs = *($5.processList);
		  delete $5.processList;
		  $$.processGroup->excludeProcs = *($6.processList);
		  delete $6.processList;
		}
            | PROCESSGROUP IDENTIFIER include exclude
                { $$.processGroup = new ProcessGroup;
		  strcpy($$.processGroup->name, $2.id);
		  $$.processGroup->withChildren = false;
		  $$.processGroup->includeProcs = *($3.processList);
		  delete $3.processList;
		  $$.processGroup->excludeProcs = *($4.processList);
		  delete $4.processList;
		}
            ;

include : INCLUDE OPENBRA proclist CLOSEBRA { $$ = $3; }
        | { $$.processList = new std::vector<RegExpRule>; }
        ;

exclude : EXCLUDE OPENBRA proclist CLOSEBRA { $$ = $3; }
        | { $$.processList = new std::vector<RegExpRule>; }
        ;

proclist : proclist STRING PLUS STRING userlist
             { RegExpRule temp($2.str, $4.str,
			       $5.userList.str1, $5.userList.str2);
	     $1.processList->push_back(temp);
	     $$ = $1; }
         | proclist STRING userlist
             { RegExpRule temp($2.str, "",
			       $3.userList.str1, $3.userList.str2 );
	     $1.processList->push_back(temp);
	     $$ = $1; }
         | { $$.processList = new std::vector<RegExpRule>;}
         ;

userlist : COLON STRING PLUS STRING
             { strcpy($$.userList.str1, $2.str); strcpy($$.userList.str2, $4.str); }
         | COLON STRING
             { strcpy($$.userList.str1, $2.str);
	       memset($$.userList.str2, sizeof($$.userList.str2), '\0'); }
         |
             { memset($$.userList.str1, '\0', sizeof($$.userList.str1));
	       memset($$.userList.str2, '\0', sizeof($$.userList.str2)); }
         ;

%%

int yymain(const char inFileName[], Analyzer * theAnalyzer)
{
  extern int yyparse();
  extern FILE * yyin;
  extern FILE * yyout;

  yyin = fopen(inFileName, "r");
  if ( yyin == 0 )
    {
      fprintf(stderr, "Failed to open '%s', compile aborted.\n", inFileName);
      return -1;
    }
  yyout = fopen("/dev/null", "w");

  anAnalyzer = theAnalyzer; // Setup the alias, burk!
  if (yyparse() != 0)
    return -1;

  fclose(yyout);
  fclose(yyin);
  return 0;
}
