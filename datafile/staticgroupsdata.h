#ifndef RUNQ_STATICGROUPSDATA_H
#define RUNQ_STATICGROUPSDATA_H

/******************************************************************************
 * FILE : staticgroupsdata.h
 * SYSTEM : RunQ (RunQ/datafile/staticgroupsdata.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 * 
 * CREATED : 17 march 2001
 * VERSION : 1.00 (17-mar-2001)
 *
 * DESCRIPTION:
 *   This header file provides the static group data.
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

#include <map>

#include "runq_error.h"
#include "datastore.h"

#define RUNQ_GROUPNAME_MAX 32

struct groupLessThan
{
  inline bool operator()(const int first, const int second) const
  {
    return first < second;
  }
};

class StaticGroupData
{
 public:
  void put(DataStore &dataStore) throw(RunQError);
  void get(DataStore &dataStore) throw(RunQError);
 public:
  int  gid;
  char name[RUNQ_GROUPNAME_MAX];
};

class StaticGroupList : public std::map<int, StaticGroupData, groupLessThan>
{
 public:
  void put(DataStore &dataStore) throw(RunQError);
  void get(DataStore &dataStore) throw(RunQError);
};

inline void StaticGroupData::put(DataStore& dataStore) throw(RunQError)
{
  dataStore.put(this, sizeof(*this));
}

inline void StaticGroupData::get(DataStore& dataStore) throw(RunQError)
{
  dataStore.get(this, sizeof(*this));
}

inline void StaticGroupList::put(DataStore & dataStore) throw(RunQError)
{
  StaticGroupList::iterator groupListIterator;

  int size = this->size();
  dataStore.put(&size, sizeof(size));
  for (groupListIterator = begin();
       groupListIterator != end();
       groupListIterator++)
    {
      groupListIterator->second.put(dataStore);
    }
}

inline void StaticGroupList::get(DataStore & dataStore) throw(RunQError)
{
  int size;
  StaticGroupData aGroup;

  dataStore.get(&size, sizeof(size));
  for (; size>0; size--)
    {
      aGroup.get(dataStore);
      (*this)[aGroup.gid] = aGroup;
    }
}

#endif // RUNQ_STATICGROUPSDATA_H
