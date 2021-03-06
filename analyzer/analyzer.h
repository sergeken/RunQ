#ifndef RUNQ_ANALYZER_H
#define RUNQ_ANALYZER_H

/*
 * FILE : analyze.h
 * SYSTEM : RunQ (Runq/analyzer/analyze.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 *
 * CREATED : 05 jan 2000
 * VERSION : 1.00 (18-mar-2000)
 *
 * DESCRIPTION:
 *   This file provides the analyze functionality.
 *   It will build a model from the rawdata.
 *   It will do so by using a workload definition file.
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

#include <iostream>
#include <string>
#include <vector>

#include "datastore.h"
#include "perfdata.h"
#include "workload.h"


class ProcessFamily {
public:
    ProcessFamily(const long aPPID, WorkLoad* aWorkLoad,
                  ProcessGroup* aProcessGroup)
    {PPID = aPPID; workLoad = aWorkLoad; processGroup = aProcessGroup;};
    bool
    operator== (const ProcessFamily & right) const;
    bool
    operator!= (const ProcessFamily & right) const;
public:
    long PPID;
    WorkLoad* workLoad;
    ProcessGroup* processGroup;
};

inline bool
ProcessFamily::operator== (const ProcessFamily & right) const
{
    return PPID == right.PPID;
}

inline bool
ProcessFamily::operator!= (const ProcessFamily & right) const
{
    return PPID != right.PPID;
}

class Analyzer {
public:
    void
    report (std::ostream & output = std::cout);
    void
    analyze (PerfData & rawData, const bool fixTimes,
             DataStore* logFile = nullptr,
             const int startHour = 0, const int startMinute = 0,
             const int endHour = 24, const int endMinute = 0);
    void
    analyze (const char name[], const bool fixTimes,
             DataStore* logFile = nullptr,
             const int startHour = 0, const int startMinute = 0,
             const int endHour = 24, const int endMinute = 0);

private:
    ProcessList
    processRawData (PerfData & rawData, const bool fixTimes,
                    const int startHour, const int startMinute,
                    const int endHour, const int endMinute);
    std::pair <WorkLoad*, ProcessGroup*>
    findProcessGroup (const char name[], const char args[],
                      const char user[], const char group[]);


public:
    double userCPU = 0.0;
    double systemCPU = 0.0;
    double niceCPU = 0.0;
    double waitCPU = 0.0;
    double idleCPU = 0.0;
    double runQueue = 0.0;
    int numberOfCPUs = 0;
    double unaccountedUserCPU = 0.0;
    double unaccountedSystemCPU = 0.0;
    std::vector<WorkLoad> workLoads;
private:
    void
    addProcessToGroup (PerfData & rawData,
                       DataStore* const logFile,
                       const WorkLoad & theWorkLoad,
                       ProcessGroup & theProcessGroup,
                       const ProcessData & theProcess);
};

#endif // RUNQ_ANALYZER_H
