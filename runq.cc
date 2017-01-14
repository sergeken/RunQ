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
    char const* dataFile = "perf.dat";
    char* startTime = 0;
    char* endTime = 0;
    int startHour = 0;
    int startMinute = 0;
    int endHour = 24;
    int endMinute = 00;
    bool csv = false;
    bool summary = false;

    PerfData perfData;
    double total_cpu;
    int count = 0;

    double nicecpu;
    double usercpu;
    double systemcpu;
    double idlecpu;
    double waitcpu;
    double runqueue;
    double bytesread;
    double byteswritten;

    double p_nicecpu;
    double p_usercpu;
    double p_systemcpu;
    double p_idlecpu;
    double p_waitcpu;
    double p_runqueue;
    double p_bytesread;
    double p_byteswritten;

    double c_nicecpu = 0;
    double c_usercpu = 0;
    double c_systemcpu = 0;
    double c_idlecpu = 0;
    double c_waitcpu = 0;
    double c_runqueue = 0;
    double c_bytesread = 0;
    double c_byteswritten = 0;

    char timeBuffer[64];
    std::vector<StaticDiskData>::iterator iterIO;
    std::vector<StaticNetworkInterfaceData>::iterator iterNet;

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
        for (iterIO = perfData.staticData.IO.diskData.begin ();
             iterIO != perfData.staticData.IO.diskData.end ();
             iterIO++) {
            cout << "Disk "
                 << iterIO->name
                 << " ("
                 << iterIO->type
                 << ")"
                 << endl;
        }
        for (iterNet = perfData.staticData.network.interfaces.begin ();
             iterNet != perfData.staticData.network.interfaces.end ();
             iterNet++) {
            cout << "interface "
                 << iterNet->name
                 << " ("
                 << iterNet->ip4
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
            p_nicecpu = perfData.dynamicData.CPU.nice;
            p_usercpu = perfData.dynamicData.CPU.user;
            p_systemcpu = perfData.dynamicData.CPU.system;
            p_idlecpu = perfData.dynamicData.CPU.idle;
            p_waitcpu = perfData.dynamicData.CPU.wait;
            p_runqueue = perfData.dynamicData.CPU.runQueue;
            p_bytesread = perfData.dynamicData.IO.bytesRead;
            p_byteswritten = perfData.dynamicData.IO.bytesWritten;
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

            nicecpu = perfData.dynamicData.CPU.nice - p_nicecpu;
            usercpu = perfData.dynamicData.CPU.user - p_usercpu;
            systemcpu = perfData.dynamicData.CPU.system - p_systemcpu;
            idlecpu = perfData.dynamicData.CPU.idle - p_idlecpu;
            waitcpu = perfData.dynamicData.CPU.wait - p_waitcpu;
            runqueue = perfData.dynamicData.CPU.runQueue;
            bytesread = perfData.dynamicData.IO.bytesRead - p_bytesread;
            byteswritten = perfData.dynamicData.IO.bytesWritten - p_byteswritten;

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
        c_runqueue /= runqueue;
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
                 << runqueue << ";"
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
                 << runqueue
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
    char const* dataFile = "perf.dat";
    char* startTime = 0;
    char* endTime = 0;
    int startHour = 0;
    int startMinute = 0;
    int endHour = 24;
    int endMinute = 00;
    bool dateFormat = false;
    bool deltaFormat = false;

    int count = 0;
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
    std::vector<StaticDiskData>::iterator diskIter;
    std::vector<StaticNetworkInterfaceData>::iterator interfaceIter;
    DataStore* logFile = nullptr;

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

    for (diskIter = perfData.staticData.IO.diskData.begin ();
         diskIter != perfData.staticData.IO.diskData.end ();
         diskIter++) {
        cout << "Disk "
             << diskIter->name
             << " ("
             << diskIter->type
             << ")"
             << endl;
    }

    for (interfaceIter = perfData.staticData.network.interfaces.begin ();
         interfaceIter != perfData.staticData.network.interfaces.end ();
         interfaceIter++) {
        cout << "Interface "
             << interfaceIter->name
             << " ("
             << interfaceIter->ip4
             << ")"
             << endl;
    }
}

void
analyze (int argc, char*argv[])
{
    int option;
    bool csv = false;
    bool fixTimes = false;
    char const* dataFile = "perf.dat";
    char const* workLoadFile = "workloads.wkl";
    char* logFileName = 0;
    char* startTime = 0;
    char* endTime = 0;

    int startHour = 0;
    int startMinute = 0;
    int endHour = 24;
    int endMinute = 00;

    DataStore* logFile = 0;

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
    if (dataFile == 0) {
        cerr << "*** ERROR: invalid datafile (-d)." << endl;
        return;
    }
    if (workLoadFile == 0) {
        cerr << "*** ERROR: invalid workloadfile (-w)." << endl;
        return;
    }
    if (startTime != 0) {
        parseTime (startTime, startHour, startMinute);
    }
    if (endTime != 0) {
        parseTime (endTime, endHour, endMinute);
    }
    if (logFileName != 0) {
        logFile = new DataStore (logFileName, std::ios_base::out,
                                 RunQFileType::UnTyped);
        if (logFile == 0) {
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
    if (logFile != 0)
        analyzeData.report (*logFile);
    ((Model*)&analyzeData)->simulate (cout, csv);

    if (logFile != 0) {
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

