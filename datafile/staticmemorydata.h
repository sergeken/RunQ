#ifndef RUNQ_STATICMEMORYDATA_H
#define RUNQ_STATICMEMORYDATA_H

/******************************************************************************
 * FILE : staticcpudata.cpp
 * SYSTEM : RunQ (RunQ/datafile/staticcpudata.cpp)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
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

class StaticMemoryData {
public:
    void
    get (DataStore & dataStore) throw (RunQError);
    void
    put (DataStore & dataStore) throw (RunQError);

public:
    double physicalMemory;
    double virtualMemory;
    long pageSize;
};

inline void
StaticMemoryData::get (DataStore & dataStore) throw (RunQError)
{
    dataStore.get (this, sizeof(*this));
}

inline void
StaticMemoryData::put (DataStore & dataStore) throw (RunQError)
{
    dataStore.put (this, sizeof(*this));
}

#endif // RUNQ_STATICMEMORYDATA_H
