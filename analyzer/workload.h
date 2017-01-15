#ifndef RUNQ_WORKLOAD_H
#define RUNQ_WORKLOAD_H

/*
 * FILE : workload.h
 * SYSTEM : RunQ (Runq/analyzer/workload.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 *
 * CREATED : 05 jan 2000
 * VERSION : 1.00 (18-mar-2000)
 *
 * DESCRIPTION:
 *   This header file provides the processsgroup functionality.
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
#include "processgroup.h"

class WorkLoad {
public:
    WorkLoad()
    {
        name[0] = '\0';
    }
public:  // Public attributes
    char name[32];
    std::vector<ProcessGroup> processGroups;
};

#endif // RUNQ_WORKLOAD_H
