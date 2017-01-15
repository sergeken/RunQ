/*
 * FILE : runq.cc
 * SYSTEM : RunQ (RunQ/runq.cc)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 *
 * CREATED : 06 jan 2000
 * VERSION : 1.7 (05/mar/2002)
 *
 * DESCRIPTION:
 *   This file implements the RunQ performance suite.
 *   It provides the main function and all sub functions.
 *
 * CHANGELOG:
 *   1.7 Fixed datafile exceptions
 *   1.5 Removed the compile option and compile on the fly in analyze
 *   1.4 Too much to tell, sorry....
 *
 */


/*  GNU General Public License
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

#include "runq_error.h"
#include "util.h"
#include "perfdata.h"
#include "analyzer.h"
#include "model.h"
#include "collectors/collect.h"

using std::endl;
using std::cerr;
using std::cout;


#ifndef COLLECTOR_ONLY
void
report (int argc, char*argv[])
{
    int option;
    auto dataFile = "perf.dat";
    char* startTime = nullptr;
    char* endTime = nullptr;
    auto startHour = 0;
    auto startMinute = 0;
    auto endHour = 24;
    auto endMinute = 0;
    auto csv = false;
    auto summary = false;

    PerfData perfData;
    double total_cpu;
    auto count = 0;

    auto c_nicecpu = 0.0;
    auto c_usercpu = 0.0;
    auto c_systemcpu = 0.0;
    auto c_idlecpu = 0.0;
    auto c_waitcpu = 0.0;
    auto c_runqueue = 0.0;
    auto c_bytesread = 0.0;
    auto c_byteswritten = 0.0;

    char timeBuffer[64];

    argc--;
    argv++;
    while ((option = getopt (argc, argv, "d:s:e:CS")) != EOF) {
        switch (option) {
        case 'd':
            dataFile = optarg;
            break;
        case 's':
            startTime = optarg;
            break;
        case 'e':
            endTime = optarg;
            break;
        case 'C':
            csv = true;
            break;
        case 'S':
            summary = true;
            break;
        default:
            return;
            break;
        }
    }
    if (dataFile == 0) {
        cerr << "*** ERROR: invalid datafile (-d)." << endl;
        return;
    }
    if (startTime != 0) {
        parseTime (startTime, startHour, startMinute);
    }
    if (endTime != 0) {
        parseTime (endTime, endHour, endMinute);
    }

    perfData.open (dataFile);
    if (!csv) {
        cout << "Host "
             << perfData.staticData.CPU.node << " running "
             << perfData.staticData.CPU.OSName << " "
             << perfData.staticData.CPU.OSRelease << " "
             << perfData.staticData.CPU.OSVersion
             << endl;
        cout << perfData.staticData.CPU.model << " by "
             << perfData.staticData.CPU.vendor << " with "
             << perfData.staticData.CPU.availableCPUs << " out "
             << perfData.staticData.CPU.numberOfCPUs << " CPU(S)"
             << endl;
        cout << "Memory: "
             << perfData.staticData.memory.physicalMemory / (1024 * 1024)
             << " Mbytes" // " for "
            // << perfData.staticData.memory.virtualMemory << " bytes"
             << " (" << perfData.staticData.memory.pageSize << " bytes/page)"
             << endl;
        for (auto const & iterIO : perfData.staticData.IO.diskData) {
            cout << "Disk "
                 << iterIO.name
                 << " ("
                 << iterIO.type
                 << ")"
                 << endl;
        }
        for (auto const & iterNet : perfData.staticData.network.interfaces) {
            cout << "interface "
                 << iterNet.name
                 << " ("
                 << iterNet.ip4
                 << ")"
                 << endl;
        }
        cout << endl;
    } else {
        cout << "Date;Time;"
             << "Nice;"
             << "User;"
             << "System;"
             << "Idle;"
             << "Wait;"
             << "Qlen;"
             << "RBytes;"
             << "WBytes;"
             << "FreeMemory"
             << endl;
    }

    perfData.get ();

    try {
        for (;; ) {
            auto p_nicecpu = perfData.dynamicData.CPU.nice;
            auto p_usercpu = perfData.dynamicData.CPU.user;
            auto p_systemcpu = perfData.dynamicData.CPU.system;
            auto p_idlecpu = perfData.dynamicData.CPU.idle;
            auto p_waitcpu = perfData.dynamicData.CPU.wait;
            auto p_runqueue = perfData.dynamicData.CPU.runQueue;
            auto p_bytesread = perfData.dynamicData.IO.bytesRead;
            auto p_byteswritten = perfData.dynamicData.IO.bytesWritten;
            perfData.get ();

            if (beforeTime (perfData.dynamicData.timeStamp, startHour, startMinute)) {
                p_nicecpu = perfData.dynamicData.CPU.nice;
                p_usercpu = perfData.dynamicData.CPU.user;
                p_systemcpu = perfData.dynamicData.CPU.system;
                p_idlecpu = perfData.dynamicData.CPU.idle;
                p_waitcpu = perfData.dynamicData.CPU.wait;
                p_runqueue = perfData.dynamicData.CPU.runQueue;
                p_bytesread = perfData.dynamicData.IO.bytesRead;
                p_byteswritten = perfData.dynamicData.IO.bytesWritten;
                continue;
            } else if (!beforeTime (perfData.dynamicData.timeStamp, endHour, endMinute)) {
                break;
            }

            auto nicecpu = perfData.dynamicData.CPU.nice - p_nicecpu;
            auto usercpu = perfData.dynamicData.CPU.user - p_usercpu;
            auto systemcpu = perfData.dynamicData.CPU.system - p_systemcpu;
            auto idlecpu = perfData.dynamicData.CPU.idle - p_idlecpu;
            auto waitcpu = perfData.dynamicData.CPU.wait - p_waitcpu;
            auto runqueue = perfData.dynamicData.CPU.runQueue - p_runqueue;
            auto bytesread = perfData.dynamicData.IO.bytesRead - p_bytesread;
            auto byteswritten = perfData.dynamicData.IO.bytesWritten - p_byteswritten;

            total_cpu = nicecpu + usercpu + systemcpu + idlecpu + waitcpu;
            total_cpu /= 100.0;

            if (!summary) {
                if (!csv) {
                    strftime (timeBuffer, sizeof(timeBuffer), "%d %b %Y %T :: ",
                              localtime (&perfData.dynamicData.timeStamp));
                    cout << timeBuffer;
                    cout << "NICE: "
                         << nicecpu / total_cpu
                         << "% USER: "
                         << usercpu / total_cpu
                         << "% SYSTEM: "
                         << systemcpu / total_cpu
                         << "% IDLE: "
                         << idlecpu / total_cpu
                         << "% WAIT: "
                         << waitcpu / total_cpu
                         << "% QLEN: "
                         << runqueue
                         << " RBytes: "
                         << bytesread
                         << " WBytes: "
                         << byteswritten
                         << " FreeMemory: "
                         << perfData.dynamicData.memory.free
                         << endl;
                } else {
                    strftime (timeBuffer, sizeof(timeBuffer), "%d-%b-%Y;%T;",
                              localtime (&perfData.dynamicData.timeStamp));
                    cout << timeBuffer;
                    cout << nicecpu / total_cpu << ";"
                         << usercpu / total_cpu << ";"
                         << systemcpu / total_cpu << ";"
                         << idlecpu / total_cpu << ";"
                         << waitcpu / total_cpu << ";"
                         << runqueue << ";"
                         << bytesread << ";"
                         << byteswritten << ";"
                         << perfData.dynamicData.memory.free
                         << endl;
                }
            }

            count++;
            c_nicecpu += nicecpu;
            c_usercpu += usercpu;
            c_systemcpu += systemcpu;
            c_idlecpu += idlecpu;
            c_waitcpu += waitcpu;
            c_runqueue += runqueue;
            c_bytesread += bytesread;
            c_byteswritten += byteswritten;
        }
    } catch (RunQError & error) {
        switch (error.error) {
        case RunQError::FileError:
            break;
        default:
            throw;
        }
    }
    if (count > 0 && (!csv || summary)) {
        c_nicecpu /= count;
        c_usercpu /= count;
        c_systemcpu /= count;
        c_idlecpu /= count;
        c_waitcpu /= count;
        c_runqueue /= count;
        total_cpu = c_nicecpu + c_usercpu + c_systemcpu + c_idlecpu + c_waitcpu;
        total_cpu /= 100.0;

        if (csv) {
            strftime (timeBuffer, sizeof(timeBuffer), "%d-%b-%Y;%H:00:00;",
                      localtime (&perfData.dynamicData.timeStamp));
            cout << timeBuffer;
            cout << c_nicecpu / total_cpu << ";"
                 << c_usercpu / total_cpu << ";"
                 << c_systemcpu / total_cpu << ";"
                 << c_idlecpu / total_cpu << ";"
                 << c_waitcpu / total_cpu << ";"
                 << c_runqueue << ";"
                 << c_bytesread << ";"
                 << c_byteswritten << endl;
        } else {
            cout << endl << "TOTALS / AVERAGES" << endl;
            cout << "NICE: "
                 << c_nicecpu / total_cpu
                 << "% USER: "
                 << c_usercpu / total_cpu
                 << "% SYSTEM: "
                 << c_systemcpu / total_cpu
                 << "% IDLE: "
                 << c_idlecpu / total_cpu
                 << "% WAIT: "
                 << c_waitcpu / total_cpu
                 << "% QLEN: "
                 << c_runqueue
                 << " RBytes: "
                 << c_bytesread
                 << " WBytes: "
                 << c_byteswritten
                 << endl;
        }
    }
}

void
report_procs (int argc, char*argv[])
{
    int option;
    auto dataFile = "perf.dat";
    char* startTime = nullptr;
    char* endTime = nullptr;
    auto startHour = 0;
    auto startMinute = 0;
    auto endHour = 24;
    auto endMinute = 0;
    auto dateFormat = false;
    auto deltaFormat = false;

    auto count = 0;
    PerfData perfData;
    ProcessList::iterator iter;
    time_t startTimeStamp = 0;
    char timeBuffer[64];

    argc--;
    argv++;
    while ((option = getopt (argc, argv, "d:s:e:DT")) != EOF) {
        switch (option) {
        case 'd':
            dataFile = optarg;
            break;
        case 's':
            startTime = optarg;
            break;
        case 'e':
            endTime = optarg;
            break;
        case 'T':
            dateFormat = true;
            break;
        case 'D':
            deltaFormat = true;
            break;
        default:
            return;
            break;
        }
    }
    if (dataFile == 0) {
        cerr << "*** ERROR: invalid datafile (-d)." << endl;
        return;
    }
    if (startTime != 0) {
        parseTime (startTime, startHour, startMinute);
    }
    if (endTime != 0) {
        parseTime (endTime, endHour, endMinute);
    }

    perfData.open (dataFile);

    if (dateFormat)
        cout <<
            "Date;Time;PID;PPID;Name;Args;User;Group;utime;stime;wtime;sleeptime;cutime;cstime;cwtime;csleeptime;MinFLT;MajFlt;cMinFlt;CMajFlt;inbytes;oubytes;RSS"
             << endl;
    else
        cout <<
            "RECORD;TimeStamp;PID;PPID;Name;Args;User;Group;utime;stime;wtime;sleeptime;cutime;cstime;cwtime;csleeptime;MinFLT;MajFlt;cMinFlt;CMajFlt;inbytes;oubytes;RSS"
             << endl;

    try {
        for (;; ) {
            perfData.get ();
            count++;

            if (beforeTime (perfData.dynamicData.timeStamp, startHour, startMinute)) {
                continue;
            } else if (!beforeTime (perfData.dynamicData.timeStamp, endHour, endMinute)) {
                break;
            }

            if (startTimeStamp == 0) {
                startTimeStamp = perfData.dynamicData.timeStamp;
            }

            for (iter = perfData.dynamicData.processList.begin ();
                 iter != perfData.dynamicData.processList.end ();
                 iter++) {
                if (dateFormat) {
                    strftime (timeBuffer, sizeof(timeBuffer), "%d-%b-%Y;%T.",
                              localtime ((time_t*)&perfData.dynamicData.timeStamp));
                    cout << timeBuffer;
                    // cout << perfData.dynamicData.timeStamp.tv_usec << ";";
                    cout << ";";
                } else if (deltaFormat) {
                    cout << count << ";";
                    cout << (perfData.dynamicData.timeStamp)
                        // + perfData.dynamicData.timeStamp.tv_usec / 1000000.0)
                        - (startTimeStamp);
                    // + startTimeStamp.tv_usec / 1000000.0);
                    cout << ";";
                } else {
                    cout << count << ";";
                    cout << perfData.dynamicData.timeStamp;
                    // + perfData.dynamicData.timeStamp.tv_usec / 1000000.0;
                    cout << ";";
                }
                cout << iter->second.PID << ";"
                     << iter->second.PPID << ";"
                     << iter->second.name << ";"
                     << iter->second.args << ";"
                     << perfData.staticData.users[iter->second.uid].name << ";"
                     << perfData.staticData.groups[iter->second.gid].name << ";"
                     << iter->second.userTime << ";"
                     << iter->second.systemTime << ";"
                     << iter->second.waitTime << ";"
                     << iter->second.sleepTime << ";"
                     << iter->second.cUserTime << ";"
                     << iter->second.cSystemTime << ";"
                     << iter->second.cWaitTime << ";"
                     << iter->second.cSleepTime << ";"
                     << iter->second.minorFaults << ";"
                     << iter->second.majorFaults << ";"
                     << iter->second.cMinorFaults << ";"
                     << iter->second.cMajorFaults << ";"
                     << iter->second.readBytes << ";"
                     << iter->second.writtenBytes << ";"
                     << iter->second.RSS
                     << endl;
            }
        }
    } catch (RunQError & error) {
        switch (error.error) {
        case RunQError::FileError:
            break;
        default:
            throw;
        }
    }
}

void
info (int argc, char*argv[])
{
    char const* dataFile = "perf.dat";
    PerfData perfData;
    int option;

    argc--;
    argv++;
    while ((option = getopt (argc, argv, "d:w:s:e:l:CF")) != EOF) {
        switch (option) {
        case 'd':
            dataFile = optarg;
            break;
        default:
            return;
            break;
        }
    }
    perfData.open (dataFile);

    cout << "Host "
         << perfData.staticData.CPU.node << " running "
         << perfData.staticData.CPU.OSName << " "
         << perfData.staticData.CPU.OSRelease << " "
         << perfData.staticData.CPU.OSVersion
         << endl;
    cout << perfData.staticData.CPU.model << " by "
         << perfData.staticData.CPU.vendor << " with "
         << perfData.staticData.CPU.availableCPUs << " out "
         << perfData.staticData.CPU.numberOfCPUs << " CPU(S)"
         << endl;
    cout << "Memory: "
         << perfData.staticData.memory.physicalMemory << " bytes" // " for "
        // << perfData.staticData.memory.virtualMemory << " bytes"
         << " (" << perfData.staticData.memory.pageSize << " bytes/page)"
         << endl;

    for (auto const & diskIter : perfData.staticData.IO.diskData) {
        cout << "Disk "
             << diskIter.name
             << " ("
             << diskIter.type
             << ")"
             << endl;
    }

    for (auto const & interfaceIter : perfData.staticData.network.interfaces) {
        cout << "Interface "
             << interfaceIter.name
             << " ("
             << interfaceIter.ip4
             << ")"
             << endl;
    }
}

void
analyze (int argc, char*argv[])
{
    int option;
    auto csv = false;
    auto fixTimes = false;
    auto dataFile = "perf.dat";
    auto workLoadFile = "workloads.wkl";
    char* logFileName = nullptr;
    char* startTime = nullptr;
    char* endTime = nullptr;

    auto startHour = 0;
    auto startMinute = 0;
    auto endHour = 24;
    auto endMinute = 0;

    DataStore* logFile = nullptr;

    Analyzer analyzeData;

    argc--;
    argv++;
    while ((option = getopt (argc, argv, "d:w:s:e:l:CF")) != EOF) {
        switch (option) {
        case 'd':
            dataFile = optarg;
            break;
        case 'w':
            workLoadFile = optarg;
            break;
        case 's':
            startTime = optarg;
            break;
        case 'e':
            endTime = optarg;
            break;
        case 'l':
            logFileName = optarg;
            break;
        case 'C':
            csv = true;
            break;
        case 'F':
            fixTimes = true;
            break;
        default:
            return;
            break;
        }
    }
    if (dataFile == nullptr) {
        cerr << "*** ERROR: invalid datafile (-d)." << endl;
        return;
    }
    if (workLoadFile == nullptr) {
        cerr << "*** ERROR: invalid workloadfile (-w)." << endl;
        return;
    }
    if (startTime != 0) {
        parseTime (startTime, startHour, startMinute);
    }
    if (endTime != 0) {
        parseTime (endTime, endHour, endMinute);
    }
    if (logFileName != nullptr) {
        logFile = new DataStore (logFileName, std::ios_base::out,
                                 RunQFileType::UnTyped);
        if (logFile == nullptr) {
            cerr << "*** ERROR: failed to open logfile '" << logFileName << "'"
                 << endl;
            return;
        }
    }

    // Parse the workload file
    extern int yymain (const char[], Analyzer*);
    if (yymain (workLoadFile, &analyzeData)) {
        cerr << "*** ERROR: Failed to parse " << workLoadFile << endl;
        return;
    }

    analyzeData.analyze (dataFile, fixTimes, logFile,
                         startHour, startMinute, endHour, endMinute);
    if (logFile != nullptr)
        analyzeData.report (*logFile);
    ((Model*)&analyzeData)->simulate (cout, csv);

    if (logFile != nullptr) {
        logFile->close ();
        delete logFile;
    }
}

#endif


int
main (int argc, char** argv)
{
    try {
        cout.flags (std::ios_base::fixed);
        runQLogo (cerr);
        if (argc < 2) {
            usage (cerr);
            return -1;
        }

#ifndef COLLECTOR_ONLY
        if (strcmp (argv[1], "collect") == 0)
#endif
        try {
            collect_main (argc - 1, argv + 1);
        } catch (RunQError & error) {
            switch (error.error) {
            case RunQError::InvalidVersion:
                cerr << endl
                     << "Version of Operating System not Supported, quiting ..."
                     << endl;
                break;
            default:
                throw;
                break;
            }
            exit (-1);
        } catch (...) {
            cerr << "Unhandled exception" << endl;
        }
#ifndef COLLECTOR_ONLY
        else if (strcmp (argv[1], "info") == 0)
            info (argc, argv);
        else if (strcmp (argv[1], "report") == 0)
            report (argc, argv);
        else if (strcmp (argv[1], "procs") == 0)
            report_procs (argc, argv);
        else if (strcmp (argv[1], "analyze") == 0)
            analyze (argc, argv);
        else {
            cerr << "wrong or incorrect command" << endl;
            usage (cerr);
            return -2;
        }
#endif

        return EXIT_SUCCESS;
    } catch (RunQError & error) {
        switch (error.error) {
        case RunQError::InvalidFile:
            cerr << endl
                 << "Datafile is not a valid RunQ file."
                 << endl;
            break;
        case RunQError::InvalidVersion:
            cerr << endl
                 << "Datafile is from a different version of RunQ."
                 << endl;
            break;
        default:
            cerr << endl
                 << "*** ERROR: Unhandled RunQ exception ("
                 << static_cast <int> (error.error)
                 << "), aborting ..."
                 << endl;
            if (error.info != 0)
                cerr << "*** Additional info: "
                     << error.info
                     << endl;
        }
        exit (-1);
    } catch (...) {
        cerr << "Unhandled exception" << endl;
        throw;
    }
}

