#ifndef RUNQ_DYNAMICIODATA_H
#define RUNQ_DYNAMICIODATA_H

/******************************************************************************
 * FILE : dynamiciodata.h
 * SYSTEM : RunQ (RunQ/datafile/dynamiciodata.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 * 
 * CREATED : 05 feb 2000
 * VERSION : 1.00 (18-mar-2000)
 *
 * DESCRIPTION:
 *   This header file provides the dynamic IO data.
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

#include "runq_error.h"
#include "datastore.h"

class DynamicIOData
{
 public:
  void put(DataStore &dataStore) throw(RunQError);
  void get(DataStore &dataStore) throw(RunQError);

 public:
  double bytesRead;
  double bytesWritten;
};

inline void DynamicIOData::get(DataStore& dataStore) throw(RunQError)
{
  dataStore.get(this, sizeof(*this));
}

inline void DynamicIOData::put(DataStore& dataStore) throw(RunQError)
{
  dataStore.put(this, sizeof(*this));
}

#endif // RUNQ_DYNAMICIODATA_H
