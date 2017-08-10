#ifndef RUNQ_STATICUSERSDATA_H
#define RUNQ_STATICUSERSDATA_H

/******************************************************************************
 * FILE : staticuserdata.h
 * SYSTEM : RunQ (RunQ/datafile/staticuserdata.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 *
 * CREATED : 12 march 2001
 * VERSION : 1.00 (12-mar-2001)
 *
 * DESCRIPTION:
 *   This header file provides the static user data.
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

#define RUNQ_USERNAME_MAX 32

struct userLessThan {
    inline bool
    operator() (const int first, const int second) const
    {
        return first < second;
    }
};

class StaticUserData {
public:
    void
    put (DataStore & dataStore);
    void
    get (DataStore & dataStore);
public:
    int uid;
    char name[RUNQ_USERNAME_MAX];
};

class StaticUserList : public std::map<int, StaticUserData, userLessThan>{
public:
    void
    put (DataStore & dataStore);
    void
    get (DataStore & dataStore);
};

inline void
StaticUserData::put (DataStore & dataStore)
{
    dataStore.put (this, sizeof(*this));
}

inline void
StaticUserData::get (DataStore & dataStore)
{
    dataStore.get (this, sizeof(*this));
}

inline void
StaticUserList::put (DataStore & dataStore)
{
    size_t size = this->size ();
    dataStore.put (&size, sizeof(size));
    for (auto & userListIterator : *this) {
        userListIterator.second.put (dataStore);
    }
}

inline void
StaticUserList::get (DataStore & dataStore)
{
    size_t size;
    StaticUserData aUser;

    dataStore.get (&size, sizeof(size));
    for (; size > 0; size--) {
        aUser.get (dataStore);
        (*this)[aUser.uid] = aUser;
    }
}

#endif // RUNQ_STATICUSERSDATA_H
