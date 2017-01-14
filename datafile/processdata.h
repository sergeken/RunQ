#ifndef RUNQ_PROCESSDATA_H
#define RUNQ_PROCESSDATA_H

/******************************************************************************
 * FILE : processdata.h
 * SYSTEM : RunQ (RunQ/datafile/processdata.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 * 
 * CREATED : 04 jan 2000
 * VERSION : 0.01 (04/jan/2000)
 *
 * DESCRIPTION:
 *   This header file provides the process data.
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

#include "datastore.h"
#include "runq_error.h"

#define RUNQ_PROCNAME_MAX 64
#define RUNQ_ARGS_MAX 128

class ProcessID
{
 public:
  ProcessID(const long aPID = 0, const long aPPID = 0) : PID {aPID}, PPID {aPPID} {}
  bool operator== (const ProcessID &rigth) const;
  bool operator!= (const ProcessID &rigth) const;
  bool operator< (const ProcessID &rigth) const;

 public:
  long PID;
  long PPID;
};

inline bool ProcessID::operator== (const ProcessID &rigth) const 
{
  return PID == rigth.PID && PPID == rigth.PPID;
}

inline bool ProcessID::operator!= (const ProcessID &rigth)  const 
{
  return ! operator==(rigth);
}

inline bool ProcessID::operator< (const ProcessID &rigth)  const 
{
  return PID < rigth.PID;
}

struct ProcessID_lt
{
  bool operator () (const ProcessID &first, const ProcessID &second) const
  {
    return first < second;
  }
};


class ProcessData : public ProcessID
{
 public:
  void get(DataStore &dataStore) throw(RunQError);
  void put(DataStore &dataStore) throw(RunQError);

 public:
  long PGRP;
  char name[RUNQ_PROCNAME_MAX];
  char args[RUNQ_ARGS_MAX];
  int uid;
  int gid;
  double userTime;
  double systemTime;
  double waitTime;
  double sleepTime;
  double cUserTime;
  double cSystemTime;
  double cWaitTime;
  double cSleepTime;
  unsigned long minorFaults;
  unsigned long cMinorFaults;
  unsigned long majorFaults;
  unsigned long cMajorFaults;
  unsigned long startTime;
  unsigned long VSize;
  unsigned long RSS;
  unsigned long readBytes;
  unsigned long writtenBytes;
};


class ProcessList : public std::map<ProcessID, ProcessData, ProcessID_lt>
{
 public:
  ProcessList findChildren(const long PPID);
  ProcessData * findProcess(const ProcessID PID);
};

inline void ProcessData::get(DataStore& dataStore) throw(RunQError)
{
  dataStore.get(this, sizeof(*this));
}

inline void ProcessData::put(DataStore& dataStore) throw(RunQError)
{
  dataStore.put(this, sizeof(*this));
}

inline ProcessData * ProcessList::findProcess(const ProcessID PID)
{
  auto processListIterator = find(PID);
  if (processListIterator == end())
    return 0;
  else
    return &(processListIterator->second);
}

inline ProcessList ProcessList::findChildren(const long PPID)
{
   ProcessList myList;

  for (auto & processListIterator : *this)
    {
      if (processListIterator.second.PPID == PPID)
	myList[processListIterator.second.PID] = processListIterator.second;
    }
  return myList;
}

#endif // RUNQ_PROCESSDATA_H
