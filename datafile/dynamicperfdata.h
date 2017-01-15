#ifndef RUNQ_DYNAMICPERFDATA_H
#define RUNQ_DYNAMICPERFDATA_H

/******************************************************************************
 * FILE : dynamicperfdata.h
 * SYSTEM : RunQ (RunQ/datafile/dynamicperfdata.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 *
 * CREATED : 04 jan 2000
 * VERSION : 1.00 (18-mar-2000)
 *
 * DESCRIPTION:
 *   This file provides the Dynamic Performance metrics.
 *   This is the record which is written away every spantime interval.
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

#include <sys/time.h>

#include "runq_error.h"
#include "datastore.h"
#include "dynamiccpudata.h"
#include "dynamiciodata.h"
#include "dynamicmemorydata.h"
#include "processdata.h"

class DynamicPerfData {
public:
    void
    put (DataStore & dataStore) throw (RunQError);
    void
    get (DataStore & dataStore) throw (RunQError);

public:
    time_t timeStamp;
    DynamicCPUData CPU;
    DynamicIOData IO;
    DynamicMemoryData memory;
    ProcessList processList;
};


inline void
DynamicPerfData::get (DataStore & dataStore) throw (RunQError)
{
    dataStore.get (&timeStamp, sizeof(timeStamp));
    CPU.get (dataStore);
    IO.get (dataStore);
    memory.get (dataStore);

    ProcessData process;
    size_t size;

    processList.clear ();
    dataStore.get (&size, sizeof(size));
    for (; size > 0; size--) {
        process.get (dataStore);
        processList[process.PID] = process;
    }
}

inline void
DynamicPerfData::put (DataStore & dataStore) throw (RunQError)
{
    dataStore.put (&timeStamp, sizeof(timeStamp));
    CPU.put (dataStore);
    IO.put (dataStore);
    memory.put (dataStore);

    size_t size = processList.size ();
    dataStore.put (&size, sizeof(size));

    for (auto & processIterator : processList)
        processIterator.second.put (dataStore);
    processList.clear ();
}

#endif // RUNQ_DYNAMICPERFDATA_H

