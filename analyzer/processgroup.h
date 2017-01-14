#ifndef RUNQ_PROCESSSGROUP_H
#define RUNQ_PROCESSSGROUP_H

/*
 * FILE : processgroup.h
 * SYSTEM : RunQ (Runq/analyzer/processgroup.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 * 
 * CREATED : 05 jan 2000
 * VERSION : 1.00 (18-mar-2000)
 *
 * DESCRIPTION:
 *   This header file provides the processgroup functionamity.
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

#include <vector>

#include "datastore.h"
#include "rq_regexp.h"

class RegExpRule
{
 public:
  RegExpRule() = default;
  RegExpRule(const char aName[], const char anArg[]="",
	     const char aUser[]="", const char aGroup[]="")
    {
      name = RegExp(aName);
      args = RegExp(anArg);
      user = RegExp(aUser);
      group = RegExp(aGroup);
    };

 public:
  RegExp	name;
  RegExp	args;
  RegExp        user;
  RegExp        group;
};


class ProcessGroup {
 public: 
  ProcessGroup()
    {
      name[0] = '\0';
      processCount = 0;
      activeProcessCount = 0;
      userCPU = 0;
      systemCPU = 0;
      waitCPU = 0;
      sleepCPU = 0;
      RSS = 0;
      readBytes = 0;
      writtenBytes = 0;
    };
  
 public:
  char   name[32];
  int    processCount;
  int    activeProcessCount;
  double RSS;
  double userCPU;
  double systemCPU;
  double waitCPU;
  double sleepCPU;
  unsigned long readBytes;
  unsigned long writtenBytes;
  std::vector<RegExpRule>	includeProcs;
  std::vector<RegExpRule>	excludeProcs;
  bool withChildren;
};

#endif // RUNQ_PROCESSGROUP_H
