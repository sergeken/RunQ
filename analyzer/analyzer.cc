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


Analyzer::Analyzer()
{
    userCPU = 0.0;
    systemCPU = 0.0;
    niceCPU = 0.0;
    waitCPU = 0.0;
    idleCPU = 0.0;
    numberOfCPUs = 0;
    unaccountedUserCPU = 0.0;
    unaccountedSystemCPU = 0.0;
}

static inline bool
excludeProcess (ProcessGroup aProcessGroup,
                const char name[], const char args[],
                const char user[], const char group[])
{
    for (auto const & regExps : aProcessGroup.excludeProcs)
        if (regexec (&regExps.name.preg, name, 0, 0, 0) == 0
            && (strlen (regExps.args.expression) == 0 || regexec (&regExps.args.preg, args, 0, 0, 0) == 0)
            && (strlen (regExps.user.expression) == 0 || regexec (&regExps.user.preg, user, 0, 0, 0) == 0)
            && (strlen (regExps.group.expression) == 0 || regexec (&regExps.group.preg, group, 0, 0, 0) == 0)
            )
            return true;

    return false;
}

static inline bool
includeProcess (ProcessGroup aProcessGroup,
                const char name[], const char args[],
                const char user[], const char group[])
{
    for (auto const & regExps : aProcessGroup.includeProcs)
        if (regexec (&regExps.name.preg, name, 0, 0, 0) == 0
            && (strlen (regExps.args.expression) == 0 || regexec (&regExps.args.preg, args, 0, 0, 0) == 0)
            && (strlen (regExps.user.expression) == 0 || regexec (&regExps.user.preg, user, 0, 0, 0) == 0)
            && (strlen (regExps.group.expression) == 0 || regexec (&regExps.group.preg, group, 0, 0, 0) == 0)
            )
            if (!excludeProcess (aProcessGroup, name, args, user, group))
                return true;

    return false;
}

void
Analyzer::processRawData (ProcessList & processList, PerfData & rawData,
                          const bool fixTimes,
                          const int startHour, const int startMinute,
                          const int endHour, const int endMinute)
throw (RunQError)
{
    double idlecpu, waitcpu, usercpu, nicecpu, systemcpu;
    int numOfSamples = 0;
    ProcessList previousProcesses;

    numberOfCPUs = rawData.staticData.CPU.availableCPUs;

    rawData.get ();

    double i_idlecpu = rawData.dynamicData.CPU.idle;
    double i_waitcpu = rawData.dynamicData.CPU.wait;
    double i_usercpu = rawData.dynamicData.CPU.user;
    double i_nicecpu = rawData.dynamicData.CPU.nice;
    double i_systemcpu = rawData.dynamicData.CPU.system;
    ProcessList initialProcesses = rawData.dynamicData.processList;

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

    if (numOfSamples <= 0) {
        idleCPU = 0;
        waitCPU = 0;
        userCPU = 0;
        niceCPU = 0;
        systemCPU = 0;
        return;
    }

    idleCPU = idlecpu - i_idlecpu;
    waitCPU = waitcpu - i_waitcpu;
    userCPU = usercpu - i_usercpu;
    niceCPU = nicecpu - i_nicecpu;
    systemCPU = systemcpu - i_systemcpu;

    for (auto const & processListIterator : initialProcesses) {
        // Forget about badly formed names.
        // seems to happen sometimes on Solaris
        if (!isprint (processListIterator.second.name[0]))
            continue;

        ProcessData* lastImage;
        lastImage = processList.findProcess (processListIterator.first.PID);
        if (lastImage != 0) {
            // cout << "fixing " << lastImage->PID << " by " << iter->second.PID;
            // cout << " : " << lastImage->systemTime
            //	       << " - " <<iter->second.systemTime << endl;
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


void
Analyzer::compileRegExps (DataStore* const logFile)
{
    for (auto & aWorkLoad : workLoads) {
        if (logFile != 0)
            *logFile << "Workload " << aWorkLoad.name << endl;
        for (auto & aProcessGroup : aWorkLoad.processGroups) {
            if (logFile != 0)
                *logFile << "  Group " << aProcessGroup.name << endl;
            for (auto & iRegExps : aProcessGroup.includeProcs) {
                if (logFile != 0)
                    *logFile << "    iexpression " << iRegExps.name.expression;
                regcomp (&iRegExps.name.preg, iRegExps.name.expression, REG_NOSUB);
                if (strlen (iRegExps.args.expression) > 0) {
                    if (logFile != 0)
                        *logFile << " + " << iRegExps.args.expression;
                    regcomp (&iRegExps.args.preg, iRegExps.args.expression, REG_NOSUB);
                }
                if (strlen (iRegExps.user.expression) > 0) {
                    if (logFile != 0)
                        *logFile << " + " << iRegExps.user.expression;
                    regcomp (&iRegExps.user.preg, iRegExps.user.expression, REG_NOSUB);
                }
                if (strlen (iRegExps.group.expression) > 0) {
                    if (logFile != 0)
                        *logFile << " + " << iRegExps.group.expression;
                    regcomp (&iRegExps.group.preg, iRegExps.group.expression, REG_NOSUB);
                }
                if (logFile != 0)
                    *logFile << endl;
            }
            for (auto & eRegExps : aProcessGroup.excludeProcs) {
                if (logFile != 0)
                    *logFile << "    eexpression " << eRegExps.name.expression;
                regcomp (&eRegExps.name.preg, eRegExps.name.expression, REG_NOSUB);
                if (strlen (eRegExps.args.expression) > 0) {
                    if (logFile != 0)
                        *logFile << " + " << eRegExps.args.expression;
                    regcomp (&eRegExps.args.preg, eRegExps.args.expression, REG_NOSUB);
                }
                if (strlen (eRegExps.user.expression) > 0) {
                    if (logFile != 0)
                        *logFile << " + " << eRegExps.user.expression;
                    regcomp (&eRegExps.user.preg, eRegExps.user.expression, REG_NOSUB);
                }
                if (strlen (eRegExps.group.expression) > 0) {
                    if (logFile != 0)
                        *logFile << " + " << eRegExps.group.expression;
                    regcomp (&eRegExps.group.preg, eRegExps.group.expression, REG_NOSUB);
                }
                if (logFile != 0)
                    *logFile << endl;
            }
        }
    }
}


void
Analyzer::addProcessToGroup (PerfData & rawData,
                             DataStore* const logFile,
                             const WorkLoad theWorkLoad,
                             ProcessGroup & theProcessGroup,
                             const ProcessData theProcess)
{
    if (logFile != 0) {
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


bool
Analyzer::findProcessGroup (WorkLoad* & theWorkLoad,
                            ProcessGroup* & theProcessGroup,
                            const char name[], const char args[],
                            const char user[], const char group[])
{
    for (auto & workLoads : workLoads)
        for (auto & processGroups : workLoads.processGroups)
            for (auto & iRegExps : processGroups.includeProcs)
                if (includeProcess (processGroups, name, args, user, group)) {
                    theWorkLoad = &workLoads;
                    theProcessGroup = &processGroups;
                    return true;
                }
    return false;
}



void
Analyzer::analyze (PerfData & rawData, const bool fixTimes,
                   DataStore* const logFile,
                   const int startHour, const int startMinute,
                   const int endHour, const int endMinute)
{
    double sucpu = 0.0, sscpu = 0.0;

    ProcessList processList;
    ProcessData* process;

    WorkLoad*            aWorkLoad;
    ProcessGroup*        aProcessGroup;

    std::vector<ProcessFamily> theProcessFamily;

    processRawData (processList, rawData, fixTimes,
                    startHour, startMinute, endHour, endMinute);

    for (auto & iter : processList) {
        sucpu += iter.second.userTime;
        sscpu += iter.second.systemTime;
    }

    unaccountedUserCPU = userCPU + niceCPU - sucpu;
    unaccountedSystemCPU = systemCPU - sscpu;

    compileRegExps (logFile);

    // Start assigning Process to WorkLoads
    for (auto & iter : processList) {
        ProcessFamily aProcessFamilyEntry (iter.second.PPID, 0, 0);
        std::vector<ProcessFamily>::iterator aProcessFamily;
        aProcessFamily = find (theProcessFamily.begin (), theProcessFamily.end (),
                               aProcessFamilyEntry);
        if (aProcessFamily != theProcessFamily.end ()
            && !excludeProcess (*(aProcessFamily->processGroup),
                                iter.second.name, iter.second.args,
                                rawData.staticData.users[iter.second.uid].name,
                                rawData.staticData.groups[iter.second.gid].name)) {
            if (logFile != 0) {
                *logFile << iter.second.name
                         << " (" << iter.second.PID << ") is a child of "
                         << iter.second.PPID
                         << endl;
            }
            addProcessToGroup (rawData, logFile, *(aProcessFamily->workLoad),
                               *(aProcessFamily->processGroup), iter.second);
            ProcessFamily newProcessFamilyEntry (iter.second.PID,
                                                 aProcessFamily->workLoad,
                                                 aProcessFamily->processGroup);
            theProcessFamily.push_back (newProcessFamilyEntry);
            continue;
        }
        if (findProcessGroup (aWorkLoad, aProcessGroup,
                              iter.second.name, iter.second.args,
                              rawData.staticData.users[iter.second.uid].name,
                              rawData.staticData.groups[iter.second.gid].name)) {
            addProcessToGroup (rawData, logFile, *aWorkLoad, *aProcessGroup,
                               iter.second);
            if (aProcessGroup->withChildren) {
                ProcessFamily newProcessFamilyEntry (iter.second.PID, aWorkLoad,
                                                     aProcessGroup);
                theProcessFamily.push_back (newProcessFamilyEntry);
            }
            continue;
        }
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
        unsigned int wklProcessCount = 0;
        double wklRSS = 0;
        double wklUTime = 0;
        double wklSTime = 0;
        double wklWTime = 0;
        unsigned long wklRBytes = 0;
        unsigned long wklWBytes = 0;

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
    analyze (rawData, fixTimes, logFile,
             startHour, startMinute, endHour, endMinute);
    rawData.close ();
}
