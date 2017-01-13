#ifndef RUNQ_REGEXP_H
#define RUNQ_REGEXP_H

/*
 * FILE : rq_regexp.h
 * SYSTEM : RunQ (Runq/misc/rq_regexp.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 * 
 * CREATED : 05 jan 2000
 * VERSION : 1.00 (18-mar-2000)
 *
 * DESCRIPTION:
 *   This header file provides regular expressions.
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

#include <regex.h>

#include <stdio.h>

#include "datastore.h"

class RegExp {
 public:
  RegExp() {expression[0] = '\0';};
  RegExp(const char Input[]);

 public:
  char	expression[32];
  regex_t preg;
};

inline RegExp::RegExp(const char input[])
{
  strcpy(expression, input);
}

#endif // RUNQ_REGEXP_H