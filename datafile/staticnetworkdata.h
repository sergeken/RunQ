#ifndef RUNQ_STATICNETWORKDATA_H
#define RUNQ_STATICNETWORKDATA_H

/******************************************************************************
 * FILE : staticnetworkdata.h
 * SYSTEM : RunQ (RunQ/datafile/staticnetworkdata.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2005 Serge Robyns
 * 
 * CREATED : 09 februari 2005
 * VERSION : 1.00 (09-feb-2005)
 *
 * DESCRIPTION:
 *   This header file provides the static network data.
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

#define RUNQ_NETNAME_MAX 8
#define RUNQ_IP4_MAX 16

class StaticNetworkInterfaceData
{
 public:
  void put(DataStore &dataStore) throw(RunQError);
  void get(DataStore &dataStore) throw(RunQError);
 public:
  char name[RUNQ_NETNAME_MAX];
  char ip4[RUNQ_IP4_MAX];
};

class StaticNetworkData
{
 public:
  void put(DataStore &dataStore) throw(RunQError);
  void get(DataStore &dataStore) throw(RunQError);

 public:
  std::vector<StaticNetworkInterfaceData> interfaces;
};

inline void StaticNetworkInterfaceData::put(DataStore& dataStore) throw(RunQError)
{
  dataStore.put(this, sizeof(*this));
}

inline void StaticNetworkInterfaceData::get(DataStore& dataStore) throw(RunQError)
{
  dataStore.get(this, sizeof(*this));
}

inline void StaticNetworkData::put(DataStore & dataStore) throw(RunQError)
{
  std::vector<StaticNetworkInterfaceData>::iterator interfaceIterator;

  int size = interfaces.size();
  dataStore.put(&size, sizeof(size));
  for (interfaceIterator = interfaces.begin();
       interfaceIterator != interfaces.end();
       interfaceIterator++)
    {
      interfaceIterator->put(dataStore);
    }
}

inline void StaticNetworkData::get(DataStore & dataStore) throw(RunQError)
{
  int size;
  StaticNetworkInterfaceData interface;

  dataStore.get(&size, sizeof(size));
  for (; size>0; size--)
    {
      interface.get(dataStore);
      interfaces.push_back(interface);
    }
}

#endif // RUNQ_STATICNETWORKDATA_H
