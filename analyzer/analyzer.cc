/*
 * FILE : analyzer.cc
 * SYSTEM : RunQ (Runq/analyzer/analyzer.cc)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 *
 * CREATED : 05 jan 2000
 * VERSION : 1.01 (18-mar-2000)
 *
 * DESCRIPTION:
 *   This file provides the analyze functionality.
 *   It will build a model from the rawdata.
 *   It will do so by using a workload definition file.
 *
 * CHANGELOG:
 *	 22-may-2000 (SR) : Various changes
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


#include "analyzer.h"
#include "util.h"
#include <ctype.h>
#include <algorithm>

using std::endl;
using std::cerr;


static bool
excludeProcess (const ProcessGroup & aProcessGroup,
                const char name[], const char args[],
                const char user[], const char group[])
{
    for (auto const & regExp : aProcessGroup.excludeProcs)
        if (regexec (&regExp.name.preg, name, 0, 0, 0) == 0
            && (regExp.args.expression != "" || regexec (&regExp.args.preg, args, 0, 0, 0) == 0)
            && (regExp.user.expression != "" || regexec (&regExp.user.preg, user, 0, 0, 0) == 0)
            && (regExp.group.expression != "" || regexec (&regExp.group.preg, group, 0, 0, 0) == 0)
            )
            return true;
    return false;
}


static bool
includeProcess (const ProcessGroup & aProcessGroup,
                const char name[], const char args[],
                const char user[], const char group[])
{
    for (auto const & regExp : aProcessGroup.includeProcs)
        if (regexec (&regExp.name.preg, name, 0, 0, 0) == 0
            && (regExp.args.expression != "" || regexec (&regExp.args.preg, args, 0, 0, 0) == 0)
            && (regExp.user.expression != "" || regexec (&regExp.user.preg, user, 0, 0, 0) == 0)
            && (regExp.group.expression != "" || regexec (&regExp.group.preg, group, 0, 0, 0) == 0)
            )
            if (!excludeProcess (aProcessGroup, name, args, user, group))
                return true;
    return false;
}


ProcessList
Analyzer::processRawData (PerfData & rawData, const bool fixTimes,
                          const int startHour, const int startMinute,
                          const int endHour, const int endMinute)
throw (RunQError)
{
    auto idlecpu = 0.0;
    auto waitcpu = 0.0;
    auto usercpu = 0.0;
    auto nicecpu = 0.0;
    auto systemcpu = 0.0;
    auto numOfSamples = 0;
    auto processList = ProcessList {};
    auto previousProcesses = ProcessList {};

    numberOfCPUs = rawData.staticData.CPU.availableCPUs;

    rawData.get ();

    auto i_idlecpu = rawData.dynamicData.CPU.idle;
    auto i_waitcpu = rawData.dynamicData.CPU.wait;
    auto i_usercpu = rawData.dynamicData.CPU.user;
    auto i_nicecpu = rawData.dynamicData.CPU.nice;
    auto i_systemcpu = rawData.dynamicData.CPU.system;
    auto initialProcesses = rawData.dynamicData.processList;

    try {
        for (;; ) {
            rawData.get ();

            if (beforeTime (rawData.dynamicData.timeStamp, startHour, startMinute)) {
                i_idlecpu = rawData.dynamicData.CPU.idle;
                i_waitcpu = rawData.dynamicData.CPU.wait;
                i_usercpu = rawData.dynamicData.CPU.user;
                i_nicecpu = rawData.dynamicData.CPU.nice;
                i_systemcpu = rawData.dynamicData.CPU.system;
                initialProcesses = rawData.dynamicData.processList;
                continue;
            } else if (!beforeTime (rawData.dynamicData.timeStamp, endHour, endMinute)) {
                break;
            }
            numOfSamples++;

            for (auto const & processListIterator : rawData.dynamicData.processList) {
                // Forget about badly formed names.
                // seems to happen sometimes on Solaris
                if (isprint (processListIterator.second.name[0])) {
                    processList[processListIterator.first] = processListIterator.second;
                }
            }

            idlecpu = rawData.dynamicData.CPU.idle;
            waitcpu = rawData.dynamicData.CPU.wait;
            usercpu = rawData.dynamicData.CPU.user;
            nicecpu = rawData.dynamicData.CPU.nice;
            systemcpu = rawData.dynamicData.CPU.system;
        }
    } catch (RunQError & error) {
        switch (error.error) {
        case RunQError::FileError:
            break;
        default:
            throw;
        }
    }

    if (0 < numOfSamples) {
        idleCPU = idlecpu - i_idlecpu;
        waitCPU = waitcpu - i_waitcpu;
        userCPU = usercpu - i_usercpu;
        niceCPU = nicecpu - i_nicecpu;
        systemCPU = systemcpu - i_systemcpu;

        for (auto const & processListIterator : initialProcesses) {
            auto lastImage = processList.findProcess (processListIterator.first.PID);
            if (lastImage != nullptr) {
                // cout << "fixing " << lastImage->PID << " by " << iter.second.PID;
                // cout << " : " << lastImage->systemTime
                //	       << " - " <<iter.second.systemTime << endl;
                lastImage->userTime -= processListIterator.second.userTime;
                lastImage->systemTime -= processListIterator.second.systemTime;
                lastImage->waitTime -= processListIterator.second.waitTime;
                lastImage->sleepTime -= processListIterator.second.sleepTime;
                lastImage->readBytes -= processListIterator.second.readBytes;
                lastImage->writtenBytes -= processListIterator.second.writtenBytes;
            }
            //      else
            //	cout << "not fixing " << iter->first.PID << endl;
        }
    }

    return processList;
}


void
Analyzer::addProcessToGroup (PerfData & rawData,
                             DataStore* const logFile,
                             const WorkLoad & theWorkLoad,
                             ProcessGroup & theProcessGroup,
                             const ProcessData & theProcess)
{
    if (logFile != nullptr) {
        *logFile << theProcess.name
                 << " (" << theProcess.PID << "-"
                 << theProcess.PPID << ") ("
                 << rawData.staticData.users[theProcess.uid].name
                 << ":"
                 << rawData.staticData.groups[theProcess.gid].name
                 << ") \""
                 << theProcess.args << "\" -> "
                 << theWorkLoad.name
                 << "."
                 << theProcessGroup.name
                 << endl;
    }
    theProcessGroup.processCount++;
    if ((theProcess.userTime + theProcess.systemTime) >= 0.000001)
        theProcessGroup.activeProcessCount++;
    theProcessGroup.userCPU += theProcess.userTime;
    theProcessGroup.systemCPU += theProcess.systemTime;
    theProcessGroup.waitCPU += theProcess.waitTime;
    theProcessGroup.sleepCPU += theProcess.sleepTime;
    theProcessGroup.RSS += theProcess.RSS;
    theProcessGroup.readBytes += theProcess.readBytes;
    theProcessGroup.writtenBytes += theProcess.writtenBytes;
}


inline std::tuple <bool, WorkLoad*, ProcessGroup*>
Analyzer::findProcessGroup (const char name[], const char args[],
                            const char user[], const char group[])
{
    for (auto & workLoad : workLoads)
        for (auto & processGroup : workLoad.processGroups)
            if (includeProcess (processGroup, name, args, user, group))
                return {true, &workLoad, &processGroup};
    return {false, nullptr, nullptr};
}


void
Analyzer::analyze (PerfData & rawData, const bool fixTimes,
                   DataStore* const logFile,
                   const int startHour, const int startMinute,
                   const int endHour, const int endMinute)
{
    auto sucpu = 0.0;
    auto sscpu = 0.0;
    auto processList = processRawData (rawData, fixTimes, startHour, startMinute, endHour, endMinute);
    for (auto & iter : processList) {
        sucpu += iter.second.userTime;
        sscpu += iter.second.systemTime;
    }
    unaccountedUserCPU = userCPU + niceCPU - sucpu;
    unaccountedSystemCPU = systemCPU - sscpu;

    // Start assigning Process to WorkLoads

    // First childeren of existing allocations
    auto theProcessFamily = std::vector<ProcessFamily> {};
    for (auto & iter : processList) {
        ProcessFamily aProcessFamilyEntry (iter.second.PPID, 0, 0);
        auto aProcessFamily = find (theProcessFamily.begin (), theProcessFamily.end (), aProcessFamilyEntry);
        if (aProcessFamily != theProcessFamily.end ()
            && !excludeProcess (*(aProcessFamily->processGroup),
                                iter.second.name, iter.second.args,
                                rawData.staticData.users[iter.second.uid].name,
                                rawData.staticData.groups[iter.second.gid].name)) {
            if (logFile != nullptr) {
                *logFile << iter.second.name
                         << " (" << iter.second.PID << ") is a child of "
                         << iter.second.PPID
                         << endl;
            }
            addProcessToGroup (rawData, logFile, *(aProcessFamily->workLoad),
                               *(aProcessFamily->processGroup), iter.second);
            ProcessFamily newProcessFamilyEntry (iter.second.PID, aProcessFamily->workLoad, aProcessFamily->processGroup);
            theProcessFamily.push_back (newProcessFamilyEntry);
            continue;
        }

        // Second, new allocations
        bool found;
        WorkLoad* aWorkLoad;
        ProcessGroup* aProcessGroup;
        std::tie (found, aWorkLoad, aProcessGroup) = findProcessGroup (iter.second.name, iter.second.args,
                                                                       rawData.staticData.users[iter.second.uid].name,
                                                                       rawData.staticData.groups[iter.second.gid].name);
        if (found) {
            addProcessToGroup (rawData, logFile, *aWorkLoad, *aProcessGroup, iter.second);
            if (aProcessGroup->withChildren) {
                ProcessFamily newProcessFamilyEntry (iter.second.PID, aWorkLoad, aProcessGroup);
                theProcessFamily.push_back (newProcessFamilyEntry);
            }
            continue;
        }

        // Cannot allocatte
        cerr << "ERROR: UNABLE to catagorize "
             << iter.second.name << endl;
    }
}


void
Analyzer::report (std::ostream & output)
{
    output << "IdleCPU " << idleCPU << " seconds" << endl;
    output << "WaitCPU " << waitCPU << " seconds" << endl;
    output << "UserCPU " << userCPU << " seconds" << endl;
    output << "NiceCPU " << niceCPU << " seconds" << endl;
    output << "SystemCPU " << systemCPU << " seconds" << endl;
    output << "Unaccounted UserCPU " << unaccountedUserCPU << " seconds" << endl;
    output << "Unaccounted SystemCPU " << unaccountedSystemCPU << " seconds" << endl;

    output << "CPU Sum: " << idleCPU + waitCPU + userCPU + niceCPU + systemCPU << endl;

    for (auto const & workLoads : workLoads) {
        auto wklProcessCount = 0u;
        auto wklRSS = 0.0;
        auto wklUTime = 0.0;
        auto wklSTime = 0.0;
        auto wklWTime = 0.0;
        auto wklRBytes = 0ul;
        auto wklWBytes = 0ul;

        output << "Workload " << workLoads.name << endl;
        for (auto const & processGroups : workLoads.processGroups) {
            wklProcessCount += processGroups.activeProcessCount;
            wklRSS += processGroups.RSS;
            wklUTime += processGroups.userCPU;
            wklSTime += processGroups.systemCPU;
            wklWTime += processGroups.waitCPU;
            wklRBytes += processGroups.readBytes;
            wklWBytes += processGroups.writtenBytes;
            output << "  ProcesssGroup " << processGroups.name << " : "
                   << processGroups.processCount << " processes "
                   << processGroups.activeProcessCount << " active processes "
                   << processGroups.RSS << " pages "
                   << processGroups.userCPU << " Utime "
                   << processGroups.systemCPU << " Stime "
                   << processGroups.waitCPU << " Wtime "
                   << processGroups.sleepCPU << " Sleep Time "
                   << processGroups.readBytes << " bytes read "
                   << processGroups.writtenBytes << " bytes written"
                   << endl;
        }
        output << "Total for workload " << workLoads.name << " : "
               << wklProcessCount << " active processes "
               << wklRSS << " pages "
               << wklUTime << " Utime "
               << wklSTime << " Stime "
               << wklWTime << " Wtime "
               << wklRBytes << " bytes read "
               << wklWBytes << " bytes written"
               << endl;
    }
    output << endl;
}

void
Analyzer::analyze (const char name[], const bool fixTimes,
                   DataStore* logFile,
                   const int startHour, const int startMinute,
                   const int endHour, const int endMinute)
{
    PerfData rawData;

    rawData.open (name);
    analyze (rawData, fixTimes, logFile, startHour, startMinute, endHour, endMinute);
    rawData.close ();
}
