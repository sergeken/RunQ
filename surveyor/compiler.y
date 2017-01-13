/*
 * FILE : compiler.y
 * SYSTEM : RunQ (RunQ/surveyor/compiler.y
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 * 
 * CREATED : 15 aug 2000
 * VERSION : 1.0
 *
 * DESCRIPTION:
 *    Yacc rules for the list of to be watched processes
 *
 * CHANGELOG:
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


%{
#include <stdlib.h>
#include <string.h>

#include "mytype.h"
#define YYSTYPE MyType
#define YYDEBUG 0
#define YYERROR_VERBOSE 1

extern int yylex();
extern int yylineno;
#define yyerror(text) fprintf(stderr, "ERROR at line %d: %s\n", yylineno, text)

static Surveyor * aSurveyor = 0;

%}

%token PLUS
%token MINUS
%token OPEN_BRACE
%token CLOSE_BRACE
%token STRING
%token VALUE
%token KEYWORD_DATE
%token KEYWORD_TIME

%%

proclist : proclist STRING PLUS STRING watchvalue message
             { 
	       char temp1[256];
	       char temp2[256];
	       strcpy(temp1+1, $2.str);
	       strcpy(temp2+1, $4.str);
	       temp1[0] = temp2[0] = '^';
	       strcat(temp1, "$");
	       strcat(temp2, "$");
	       WatchValues temp(temp1, temp2, $5.value, *($6.messages));
	       $1.watchValue->push_back(temp);
	       $$ = $1;
	     }
         | proclist STRING watchvalue message
             {
	       char temp1[256];
	       strcpy(temp1+1, $2.str);
	       temp1[0] = '^';
	       strcat(temp1, "$");
	       WatchValues temp(temp1, $3.value, *($4.messages));
	       $1.watchValue->push_back(temp);
	       $$ = $1;
	     }
         |
           { 
	     $$.watchValue = &(aSurveyor->watchValues);
	   }
         ;

watchvalue : MINUS VALUE
             {
	       $$.value = - $2.value;
	     }
           | VALUE
             {
	       $$ = $1;
	     }
           ;

message : OPEN_BRACE message_parts CLOSE_BRACE { $$ = $2; }
        ;

message_parts : message_parts message_part
                {
		  $1.messages->push_back($2.message);
		  $$ = $1;
		}
              | { $$.messages = new vector<Message>; }
              ;

message_part : STRING
               {
		 $$.message.type = Message::TEXT;
		 $$.message.text = strdup($1.str);
               }
             | KEYWORD_DATE { $$.message.type = Message::DATE; }
             | KEYWORD_TIME { $$.message.type = Message::TIME; }
             ;

%%

int yymain(const char inFileName[], Surveyor * theSurveyor)
{
  extern int yyparse();
  extern FILE * yyin;
  extern FILE * yyout;

  yyin = fopen(inFileName, "r");
  if ( yyin == 0 )
    {
      cerr << "Failed to open '" << inFileName << "' compile aborted."
	   << endl;
      return -1;
    }
  yyout = fopen("/dev/null", "w");

  aSurveyor = theSurveyor; // Setup the alias, burk!
  if (yyparse() != 0)
      return -1;

  fclose(yyout);
  fclose(yyin);
  return 0;
}
