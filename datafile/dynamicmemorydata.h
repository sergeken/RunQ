#ifndef RUNQ_DYNAMICMEMORYDATA_H
#define RUNQ_DYNAMICMEMORYDATA_H

/******************************************************************************
 * FILE : dynamicmemorydata.h
 * SYSTEM : RunQ (RunQ/datafile/dynamicmemorydata.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 *
 * CREATED : 04 jan 2000
 * VERSION : 1.00 (18-mar-2000)
 *
 * DESCRIPTION:
 *   This header file provides the dynamic memory data.
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

class DynamicMemoryData {
public:
    void
    get (DataStore & dataStore);
    void
    put (DataStore & dataStore);

public:
    double free;
    double used;
    double buffers;
    double shared;
    double cached;
    double pagedIn;
    double pagedOut;
};

inline void
DynamicMemoryData::get (DataStore & dataStore)
{
    dataStore.get (this, sizeof(*this));
}

inline void
DynamicMemoryData::put (DataStore & dataStore)
{
    dataStore.put (this, sizeof(*this));
}

#endif // RUNQ_DYNAMICMEMORYDATA_H
