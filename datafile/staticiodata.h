#ifndef RUNQ_STATICIODATA_H
#define RUNQ_STATICIODATA_H

/******************************************************************************
 * FILE : staticiodata.h
 * SYSTEM : RunQ (RunQ/datafile/staticiodata.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 * 
 * CREATED : 24 mey 2000
 * VERSION : 1.00 (24-may-2000)
 *
 * DESCRIPTION:
 *   This header file provides the static IO data.
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

#include "runq_error.h"
#include "datastore.h"

#define RUNQ_DISKNAME_MAX 64
#define RUNQ_DISKTYPE_MAX 256

class StaticDiskData
{
 public:
  void put(DataStore &dataStore) throw(RunQError);
  void get(DataStore &dataStore) throw(RunQError);
 public:
  char name[RUNQ_DISKNAME_MAX];
  char type[RUNQ_DISKTYPE_MAX];
};

class StaticIOData
{
 public:
  void put(DataStore &dataStore) throw(RunQError);
  void get(DataStore &dataStore) throw(RunQError);

 public:
  std::vector<StaticDiskData> diskData;
};

inline void StaticDiskData::put(DataStore& dataStore) throw(RunQError)
{
  dataStore.put(this, sizeof(*this));
}

inline void StaticDiskData::get(DataStore& dataStore) throw(RunQError)
{
  dataStore.get(this, sizeof(*this));
}

inline void StaticIOData::put(DataStore & dataStore) throw(RunQError)
{
  std::vector<StaticDiskData>::iterator diskDataIterator;

  int size = diskData.size();
  dataStore.put(&size, sizeof(size));
  for (diskDataIterator = diskData.begin();
       diskDataIterator != diskData.end();
       diskDataIterator++)
    {
      diskDataIterator->put(dataStore);
    }
}

inline void StaticIOData::get(DataStore & dataStore) throw(RunQError)
{
  int size;
  StaticDiskData disk;

  dataStore.get(&size, sizeof(size));
  for (; size>0; size--)
    {
      disk.get(dataStore);
      diskData.push_back(disk);
    }
}

#endif // RUNQ_STATICIODATA_H
