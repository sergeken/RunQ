/*
 * FILE : collect.cc
 * SYSTEM : RunQ (RunQ/collecor/collect.cc)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 *
 * CREATED : 06 jan 2000
 * VERSION : 0.01 (06/jan/2000)
 *
 * DESCRIPTION:
 *   This file defines the superclass for RunQ's peformance collectors.
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
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

#include "collect.h"
#include "perfdata.h"

#ifdef linux
#include "linux/linux.h"
#define PERFDATA LinuxPerfData
#else
#ifdef __OpenBSD__
#include "openbsd/openbsd.h"
#define PERFDATA OpenBSDPerfData
#else
#ifdef sun
#include "solaris/solaris.h"
#define PERFDATA SolarisPerfData
#else
#ifdef __hpux
#include "hpux/hpux.h"
#define PERFDATA HPUXPerfData
#else
#error "INVALID PLATFORM"
#endif // HPUX
#endif // SOLARIS
#endif // OBSD
#endif // LINUX


using std::endl;
using std::cerr;


void
collect (const char dataFile[], const int duration, const int spanTime,
         const bool noSleep)
throw (RunQError)
{
    time_t startTime, currentTime;
    PerfData* perfData =
        new PERFDATA ();

    perfData->create (dataFile);
    perfData->sample (true);
    perfData->dynamicData.timeStamp = time (NULL);
    perfData->put ();

    int i, j;
    startTime = time (0);
    // convert duration to seconds and divide by the spanTime (in secs)
    for (i = 0; i < (int)((60.0 * duration) / (double)spanTime); i++) {
        currentTime = time (0);
        while (difftime (currentTime, startTime + i * spanTime) < spanTime - 1) {
            perfData->sample (false);
            if (!noSleep)
                sleep (1);
            currentTime = time (0);
        }
        perfData->sample (true);
        perfData->dynamicData.timeStamp = time (NULL);
        perfData->put ();
    }

    perfData->close ();
    delete perfData;
}

void
collect_main (const int argc, char*argv[]) throw (RunQError)
{
    int option;
    int minutes = 0;
    int spanTime = 60;
    bool noSleep = false;
    char const* dataFile = "perf.dat";

    while ((option = getopt (argc, argv, "d:m:S:N")) != EOF) {
        switch (option) {
        case 'd':
            dataFile = optarg;
            break;
        case 'm':
            minutes = atol (optarg);
            break;
        case 'S':
            spanTime = atol (optarg);
            break;
        case 'N':
            noSleep = true;
            break;
        default:
            return;
            break;
        }
    }
    if (minutes <= 0) {
        cerr << "*** ERROR: invalid duration (-m) : " << minutes << endl;
        return;
    }
    if (dataFile == 0) {
        cerr << "*** ERROR: invalid datafile." << endl;
        return;
    }
    collect (dataFile, minutes, spanTime, noSleep);
}
