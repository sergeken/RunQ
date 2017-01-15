/*
 * FILE : linux.cc
 * SYSTEM : RunQ (Runq/collectors/linux/linux.cc)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@advalvas.be
 * COPYRIGHT : (C) 2000, by Robyns Consulting & Services (RC&S)
 *
 * CREATED : 04 jan 2000
 * VERSION : 1.00 (18-mar-2000)
 *
 * DESCRIPTION:
 *   This file implementes the performance collector for OpenBSD.
 *
 * CHANGELOG:
 *
 */


#include <unistd.h>

#include <pwd.h>
#include <grp.h>
#include <sys/utsname.h>

#include <iostream>
using std::cerr;
using std::endl;

#include "openbsd.h"

#define OBSD_STATFILE  "/proc/stat"
#define OBSD_MEMFILE   "/proc/meminfo"
#define OBSD_LOADFILE  "/proc/loadavg"
#define OBSD_PROCDIR   "/proc"
#define OBSD_CPUINFO   "/proc/cpuinfo"
#define OBSD_DISKFILE  "/proc/diskstats"
#define OBSD_NETWORK    "/proc/net/dev"

#define PROCESSORNUM    "processor"
#define VENDORID        "vendor_id"
#define MODELNAME       "model name"
#define FREQUENCY       "cpu MHz"
#define CACHESIZE       "cache size"
#define SPECINT         "bogomips"

#define MEMTOTID        "MemTotal:"
#define MEMFREEID       "MemFree:"
#define MEMBUFFERID     "Buffers:"
#define MEMCACHEDID     "Cached:"

#define PAGINGID        "page"
#define CPUDYNID        "cpu "
#define IDEHDID         "hd"
#define SCSIHDID        "sd"
#define LASTPIDID       "processses"


#if 0
#define GOOD_OBSD_VERSION "3.7"
#endif

OpenBSDPerfData::OpenBSDPerfData() throw (RunQError)
{
    struct utsname unameData;

    uname (&unameData);
#if 0
    if (strcmp (unameData.sysname, "OpenBSD")
        || strncmp (unameData.release, GOOD_OBSD_VERSION,
                    sizeof(GOOD_OBSD_VERSION) - 1)
        ) {
        throw RunQError (RunQError::InvalidVersion);
    }
#endif

    procDirectory = opendir (OBSD_PROCDIR);
    if (procDirectory == NULL) {
        fclose (memoryFile);
        fclose (statFile);
        fclose (networkFile);
        throw (RunQError::NoProcDirectory, OBSD_PROCDIR);
    }

    getStaticCPUData ();
    getStaticMemoryData ();
    getStaticIOData ();
    getStaticNetworkData ();
    getUsersData ();
    getGroupsData ();
}

OpenBSDPerfData::~OpenBSDPerfData()
{
    closedir (procDirectory);
    fclose (memoryFile);
    fclose (statFile);
}

void
OpenBSDPerfData::sample (bool lastSample) throw (RunQError)
{
    getProcessList ();
    if (lastSample) {
        getDynamicCPUData ();
        getDynamicMemoryData ();
        getDynamicIOData ();
    }
}

void
OpenBSDPerfData::getDynamicCPUData () throw (RunQError)
{}


void
OpenBSDPerfData::getDynamicIOData () throw (RunQError)
{}

void
OpenBSDPerfData::getDynamicMemoryData () throw (RunQError)
{}

void
OpenBSDPerfData::getProcessData (long PID) throw (RunQError)
{
    ProcessData aProcess;
    FILE* processFile;
    int bytes;
    char buffer[256];

    snprintf (buffer, sizeof(buffer), OBSD_PROCDIR "/%d/stat", (int)PID);
    processFile = fopen (buffer, "r");
    if (processFile == NULL) {
        // We won't throw an execption here, as probably the process died
        // while we where watching.
        return;
    }
    if (fscanf (processFile,
                "%d (%s %*c %d %d %*d %*d %*d %*d %lu %lu %lu %lu %lf %lf %lf %lf %*d %*d %*d %*d %lu %lu %lu %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*d",
                &aProcess.PID,
                aProcess.name,
                // char state,
                &aProcess.PPID,
                &aProcess.PGRP,
                // int tsk->session,
                // int tsk->tty ? kdev_t_to_nr(tsk->tty->device) : 0,
                // int tty_pgrp,
                // unsigned long tsk->flags,
                &aProcess.minorFaults,
                &aProcess.cMinorFaults,
                &aProcess.majorFaults,
                &aProcess.cMajorFaults,
                &aProcess.userTime,
                &aProcess.systemTime,
                &aProcess.cUserTime,
                &aProcess.cSystemTime,
                // priority,
                // nice,
                // 0UL ,
                // tsk->it_real_value,
                &aProcess.startTime,
                &aProcess.VSize,
                &aProcess.RSS
                // tsk->rlim ? tsk->rlim[RLIMIT_RSS].rlim_cur : 0,
                // tsk->mm ? tsk->mm->start_code : 0,
                // tsk->mm ? tsk->mm->end_code : 0,
                // tsk->mm ? tsk->mm->start_stack : 0,
                // esp,
                // eip,
                // tsk->signal .sig[0] & 0x7fffffffUL,
                // tsk->blocked.sig[0] & 0x7fffffffUL,
                // sigign      .sig[0] & 0x7fffffffUL,
                // sigcatch    .sig[0] & 0x7fffffffUL,
                // wchan,
                // tsk->nswap,
                // tsk->cnswap,
                // int tsk->exit_signal
                ) > 0) {
        aProcess.userTime /= 100.0;
        aProcess.systemTime /= 100.0;
        aProcess.waitTime = 0.0;
        aProcess.sleepTime = 0.0;
        aProcess.cUserTime /= 100.0;
        aProcess.cSystemTime /= 100.0;
        aProcess.cWaitTime = 0.0;
        aProcess.cSleepTime = 0.0;
        aProcess.name[strlen (aProcess.name) - 1] = '\0'; // Trim trailing ) Yuck!
        aProcess.readBytes = 0;
        aProcess.writtenBytes = 0;
    }
    fclose (processFile);

    snprintf (buffer, sizeof(buffer), OBSD_PROCDIR "/%d/status", (int)PID);
    processFile = fopen (buffer, "r");
    if (processFile != NULL) {
        while (fgets (buffer, sizeof(buffer) - 1, processFile)) {
            if (memcmp (buffer, "Uid:", 4) == 0) {
                sscanf (buffer, "%*s %d",
                        &aProcess.uid);
            } else if (memcmp (buffer, "Gid:", 4) == 0)     {
                sscanf (buffer, "%*s %d",
                        &aProcess.gid);
                break;
            }
        }
        fclose (processFile);
    }

    snprintf (buffer, sizeof(buffer), OBSD_PROCDIR "/%d/cmdline", (int)PID);
    processFile = fopen (buffer, "r");
    if (processFile != NULL) {
        bytes = fread (aProcess.args, 1, sizeof(aProcess.args), processFile);
        fclose (processFile);
        aProcess.args[bytes] = '\0';
        // Patch the commandline, don't remember anymore why.
        for (bytes--; bytes >= 0; bytes--) {
            switch (aProcess.args[bytes]) {
            case '\0': aProcess.args[bytes] = ' ';
                break;
                // case '\n': aProcess.args[bytes] = '\0'; break;
            }
        }
    }

    dynamicData.processList[PID] = aProcess;
}

void
OpenBSDPerfData::getProcessList () throw (RunQError)
{
    struct dirent* directoryEntry;
    long PID;
    char buffer[256];

    rewinddir (procDirectory);
    while ((directoryEntry = readdir (procDirectory))) {
        try {
            PID = atol (directoryEntry->d_name);
            if (PID > 0)
                getProcessData (PID);
        } catch (RunQError & error)        {
            switch (error.error) {
            case RunQError::FileNotFound:
                break;
            default:
                throw;
            }
        }
    }
}

void
OpenBSDPerfData::getStaticCPUData () throw (RunQError)
{}


void
OpenBSDPerfData::getStaticMemoryData () throw (RunQError)
{}


void
OpenBSDPerfData::getStaticNetworkData () throw (RunQError)
{}


void
OpenBSDPerfData::getStaticIOData () throw (RunQError)
{}

void
OpenBSDPerfData::getUsersData () throw (RunQError)
{
    StaticUserData aUser;
    struct passwd* passwdEntry;

    while (passwdEntry = getpwent ()) {
        aUser.uid = passwdEntry->pw_uid;
        strcpy (aUser.name, passwdEntry->pw_name);
        staticData.users[aUser.uid] = aUser;
    }
    endpwent ();
}

void
OpenBSDPerfData::getGroupsData () throw (RunQError)
{
    StaticGroupData aGroup;
    struct group* groupEntry;

    while (groupEntry = getgrent ()) {
        aGroup.gid = groupEntry->gr_gid;
        strcpy (aGroup.name, groupEntry->gr_name);
        staticData.groups[aGroup.gid] = aGroup;
    }
    endgrent ();
}
