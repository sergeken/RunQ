#ifndef RUNQ_STATICCPUDATA_H
#define RUNQ_STATICCPUDATA_H

/******************************************************************************
 * FILE : staticcpudata.h
 * SYSTEM : RunQ (RunQ/datafile/staticcpudata.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 * 
 * CREATED : 04 jan 2000
 * VERSION : 18-mar-2000
 *
 * DESCRIPTION:
 *   This header file provides the static cpu data.
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

class StaticCPUData
{
 public:
  void get(DataStore &dataStore) throw(RunQError);
  void put(DataStore &dataStore) throw(RunQError);

public:
  char node[64];
  char vendor[64];
  char model[64];
  char OSName[64];
  char OSRelease[64];
  char OSVersion[64];
  double frequency;
  long cacheSize;
  long numberOfCPUs;
  long availableCPUs;
  double specint;
};

inline void StaticCPUData::get(DataStore& dataStore) throw(RunQError)
{
  dataStore.get(this, sizeof(*this));
}

inline void StaticCPUData::put(DataStore& dataStore) throw(RunQError)
{
  dataStore.put(this, sizeof(*this));
}

#endif // RUNQ_STATICCPUDATA_H
