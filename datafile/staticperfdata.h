#ifndef RUNQ_STATICPERFDATA_H
#define RUNQ_STATICPERFDATA_H

/******************************************************************************
 * FILE : staticperfdata.hpp
 * SYSTEM : RunQ (RunQ/datafile/staticperfdata.hpp)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robysn
 * 
 * CREATED : 04 jan 2000
 * VERSION : 0.01 (04/jan/2000)
 *
 * DESCRIPTION:
 *   This header file defines the superclass for RunQ's datafile
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

#include "datastore.h"
#include "runq_error.h"
#include "staticcpudata.h"
#include "staticmemorydata.h"
#include "staticiodata.h"
#include "staticnetworkdata.h"
#include "staticusersdata.h"
#include "staticgroupsdata.h"

class StaticPerfData
{
 public:
  void get(DataStore &dataStore) throw(RunQError);
  void put(DataStore &dataStore) throw(RunQError);
 public:
  StaticCPUData CPU;
  StaticMemoryData memory;
  StaticIOData IO;
  StaticNetworkData network;
  StaticUserList users;
  StaticGroupList groups;
};

inline void StaticPerfData::put(DataStore &dataStore) throw(RunQError)
{
    CPU.put(dataStore);
    memory.put(dataStore);
    IO.put(dataStore);
    network.put(dataStore);
    users.put(dataStore);
    groups.put(dataStore);
}

inline void StaticPerfData::get(DataStore &dataStore) throw(RunQError)
{
    CPU.get(dataStore);
    memory.get(dataStore);
    IO.get(dataStore);
    network.get(dataStore);
    users.get(dataStore);
    groups.get(dataStore);
}

#endif // RUNQ_STATICPERFDATA_H
