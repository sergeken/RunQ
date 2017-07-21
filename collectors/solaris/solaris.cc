/******************************************************************************
 * FILE : linux.h
 * SYSTEM : RunQ (Runq/collectors/solaris/solaris.cpp)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@advalvas.be
 * COPYRIGHT : (C) 2000, by Robyns Consulting & Services (RC&S)
 *
 * CREATED : 14 jan 2000
 * VERSION : 0.01 (14/jan/2000)
 *
 * DESCRIPTION:
 *   This c++ file defines the superclass for RunQ's datafile
 *
 * CHANGELOG:
 *
 */

#include "solaris.h"

#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <sys/utsname.h>
#include <sys/systeminfo.h>

#define PAGE_SIZE 1
#define SOLARIS_PROCDIR "/proc"
#define CPUDYNID "cpu_stat"
#define LOADAVGID "avenrun_1min"
#define DISK "disk"
#define SDERR "sderr"
#define VENDOR "Vendor"
#define PRODUCT "Product"

SolarisPerfData::SolarisPerfData() throw (RunQError)
{
    // seteuid(getuid());
    kc = kstat_open ();
    if (!kc)
        throw (RunQError::GenericError, "kstat_open");
    procDirectory = opendir (SOLARIS_PROCDIR);
    if (procDirectory == nullptr)
        throw (RunQError::NoProcDirectory, SOLARIS_PROCDIR);

    getStaticCPUData ();
    getStaticMemoryData ();
    getStaticIOData ();
    getUsersData ();
    getGroupsData ();
}

SolarisPerfData::~SolarisPerfData()
{
    kstat_close (kc);
    closedir (procDirectory);
}


void
SolarisPerfData::getStaticCPUData () throw (RunQError)
{
    kstat_t*ks;
    kid_t kcid;
    cpu_stat_t cpu_stat;
    kstat_named_t* kn;

    sysinfo (SI_HOSTNAME, staticData.CPU.node, sizeof(staticData.CPU.node));
    sysinfo (SI_HW_PROVIDER, staticData.CPU.vendor, sizeof(staticData.CPU.vendor));
    sysinfo (SI_PLATFORM, staticData.CPU.model, sizeof(staticData.CPU.model));
    sysinfo (SI_SYSNAME, staticData.CPU.OSName, sizeof(staticData.CPU.OSName));
    sysinfo (SI_VERSION, staticData.CPU.OSVersion, sizeof(staticData.CPU.OSVersion));
    sysinfo (SI_RELEASE, staticData.CPU.OSRelease, sizeof(staticData.CPU.OSRelease));

    ks = kstat_lookup (kc, "unix", 0, "system_misc");
    if ((int)ks == -1)
        throw (RunQError::GenericError, "kstat_lookup");
    if (kstat_read (kc, ks, 0) == -1)
        throw (RunQError::GenericError, "kstat_read");
    kn = (kstat_named_t*) kstat_data_lookup (ks, "ncpus");
    if (kn)
        staticData.CPU.numberOfCPUs = kn->value.ui32;
    staticData.CPU.availableCPUs = 0;

    for (ks = kc->kc_chain; ks; ks = ks->ks_next) {
        if (strncmp (ks->ks_name, CPUDYNID, sizeof(CPUDYNID) - 1) == 0) {
            kcid = kstat_read (kc, ks, &cpu_stat);
            if (kcid == -1)
                throw (RunQError::GenericError, "kstat_read");
            staticData.CPU.availableCPUs++;
        }
    }
}


void
SolarisPerfData::getStaticMemoryData () throw (RunQError)
{
    kstat_t*ks;
    kid_t kcid;
    kstat_named_t* kn;

    staticData.memory.pageSize = getpagesize ();

    ks = kstat_lookup (kc, "unix", 0, "system_pages");
    if ((int)ks == -1)
        throw (RunQError::GenericError, "kstat_lookup");
    if (kstat_read (kc, ks, 0) == -1)
        throw (RunQError::GenericError, "kstat_read");
    kn = (kstat_named_t*) kstat_data_lookup (ks, "physmem");
    if (kn)
        staticData.memory.physicalMemory = (double)kn->value.ul * (double)staticData.memory.pageSize;
    staticData.memory.virtualMemory = 0;
}



void
SolarisPerfData::getDynamicMemoryData () throw (RunQError)
{
    kstat_t*ks;
    kid_t kcid;
    kstat_named_t* kn;

    ks = kstat_lookup (kc, "unix", 0, "system_pages");
    if ((int)ks == -1)
        throw (RunQError::GenericError, "kstat_lookup");
    if (kstat_read (kc, ks, 0) == -1)
        throw (RunQError::GenericError, "kstat_read");
    kn = (kstat_named_t*) kstat_data_lookup (ks, "freemem");
    if (kn)
        dynamicData.memory.free = (double)kn->value.ul * (double)staticData.memory.pageSize;
}


void
SolarisPerfData::getStaticIOData () throw (RunQError)
{
    StaticDiskData aDisk;
    kstat_t*ks;
    kid_t kcid;
    int i;
    kstat_named named_stat[255];

    dynamicData.IO.bytesRead = 0;
    dynamicData.IO.bytesWritten = 0;
    for (ks = kc->kc_chain; ks; ks = ks->ks_next) {
        // SCSI device names are only stored on the sderr property
        // IDE disks seems to be lost here ...
        if (ks->ks_type == KSTAT_TYPE_NAMED && strcmp (ks->ks_module, SDERR) == 0) {
            kcid = kstat_read (kc, ks, &named_stat);
            if (kcid == -1)
                throw (RunQError::GenericError, "kstat_read");
            sprintf (aDisk.name, "sd%d", ks->ks_instance);
            aDisk.type[0] = '\0';
            for (i = 0; i < ks->ks_ndata; i++) {
                if (strcmp (named_stat[i].name, VENDOR) == 0) {
                    if (strlen (named_stat[i].value.c) <= sizeof(named_stat[i].value.c))
                        strcat (aDisk.type, named_stat[i].value.c);
                    else {
                        int len = strlen (aDisk.type);
                        memcpy (aDisk.type + len, named_stat[i].value.c,
                                sizeof (named_stat[i].value.c));
                        aDisk.type[len + sizeof(named_stat[i].value.c)] = '\0';
                    }
                } else if (strcmp (named_stat[i].name, PRODUCT) == 0) {
                    if (strlen (named_stat[i].value.c) <= sizeof(named_stat[i].value.c))
                        strcat (aDisk.type, named_stat[i].value.c);
                    else {
                        int len = strlen (aDisk.type);
                        memcpy (aDisk.type + len, named_stat[i].value.c,
                                sizeof (named_stat[i].value.c));
                        aDisk.type[len + sizeof(named_stat[i].value.c)] = '\0';
                    }
                }
            }
            staticData.IO.diskData.push_back (aDisk);
        }
    }
}

void
SolarisPerfData::getDynamicCPUData () throw (RunQError)
{
    kstat_t*ks;
    kid_t kcid;
    cpu_stat_t cpu_stat;
    kstat_named_t* kn;

    dynamicData.CPU.idle = 0.0;
    dynamicData.CPU.wait = 0.0;
    dynamicData.CPU.user = 0.0;
    dynamicData.CPU.system = 0.0;
    dynamicData.CPU.nice = 0.0;

    for (ks = kc->kc_chain; ks; ks = ks->ks_next) {
        if (strncmp (ks->ks_name, CPUDYNID, sizeof(CPUDYNID) - 1) == 0) {
            kcid = kstat_read (kc, ks, &cpu_stat);
            if (kcid == -1)
                throw (RunQError::GenericError, "kstat_read");
            dynamicData.CPU.idle += cpu_stat.cpu_sysinfo.cpu[CPU_IDLE] / 100.0;
            dynamicData.CPU.wait += cpu_stat.cpu_sysinfo.cpu[CPU_WAIT] / 100.0;
            dynamicData.CPU.user += cpu_stat.cpu_sysinfo.cpu[CPU_USER] / 100.0;
            dynamicData.CPU.system += cpu_stat.cpu_sysinfo.cpu[CPU_KERNEL] / 100.0;
        }
    }
    ks = kstat_lookup (kc, "unix", 0, "system_misc");
    if ((int)ks == -1)
        throw (RunQError::GenericError, "kstat_lookup");
    if (kstat_read (kc, ks, 0) == -1)
        throw (RunQError::GenericError, "kstat_read");
    kn = (kstat_named_t*) kstat_data_lookup (ks, LOADAVGID);
    if (kn)
        dynamicData.CPU.runQueue = (double)(kn->value.ui32) / (double) (1 << 8);
}


void
SolarisPerfData::getDynamicIOData () throw (RunQError)
{
    kstat_t*ks;
    kid_t kcid;
    kstat_io io_stat;

    dynamicData.IO.bytesRead = 0;
    dynamicData.IO.bytesWritten = 0;
    for (ks = kc->kc_chain; ks; ks = ks->ks_next) {
        if (ks->ks_type == KSTAT_TYPE_IO && strcmp (ks->ks_class, DISK) == 0) {
            kcid = kstat_read (kc, ks, &io_stat);
            if (kcid == -1)
                throw (RunQError::GenericError, "kstat_read");
            dynamicData.IO.bytesRead += io_stat.nread;
            dynamicData.IO.bytesWritten += io_stat.nwritten;
        }
    }
}


void
SolarisPerfData::sample (const bool fullSample) throw (RunQError)
{
    if (fullSample) {
        kid_t kcid;
        do {
            kcid = kstat_chain_update (kc);
            if (kcid == -1)
                throw (RunQError::GenericError, "kstat_update_chain");
        } while (kcid != 0);

        getDynamicCPUData ();
        getDynamicIOData ();
        getDynamicMemoryData ();
    }
    getProcesssesData ();
}


void
SolarisPerfData::getProcesssData (const long PID) throw (RunQError)
{
    ProcessData aProcess;
    FILE* processFile;
    psinfo_t currproc;
    prusage_t procusage;
    pstatus_t procstatus;
    char buffer[256];

    sprintf (buffer, "/proc/%d/psinfo", (int)PID);
    processFile = fopen (buffer, "r");
    if (processFile == 0) {
        return;
    }
    fread (&currproc, sizeof(currproc), 1, processFile);
    fclose (processFile);

    // Check for bad samples, defunct procs, etc .....
    // if (strcmp(aProcess.name, "") == 0
    if (!isprint (currproc.pr_fname[0])) {
        return;
    }

    sprintf (buffer, "/proc/%d/usage", (int)PID);
    processFile = fopen (buffer, "r");
    if (processFile == 0) {
        return;
    }
    fread (&procusage, sizeof(procusage), 1, processFile);
    fclose (processFile);

    aProcess.PID = currproc.pr_pid;
    ;
    strncpy (aProcess.name, currproc.pr_fname, sizeof(aProcess.name) - 1);
    aProcess.name[sizeof(aProcess.name) - 1] = '\0';
    strncpy (aProcess.args, currproc.pr_psargs, sizeof(aProcess.args) - 1);
    aProcess.args[sizeof(aProcess.args) - 1] = '\0';
    aProcess.PPID = currproc.pr_ppid;
    aProcess.PGRP = currproc.pr_pgid;
    aProcess.uid = currproc.pr_uid;
    aProcess.gid = currproc.pr_gid;
    aProcess.minorFaults = procusage.pr_minf;
    aProcess.cMinorFaults = 0;
    aProcess.majorFaults = procusage.pr_majf;
    aProcess.cMajorFaults = 0;
    aProcess.userTime =
        procusage.pr_utime.tv_sec + procusage.pr_utime.tv_nsec / 1000000000.0;
    aProcess.systemTime =
        procusage.pr_stime.tv_sec + procusage.pr_stime.tv_nsec / 1000000000.0
        + procusage.pr_ttime.tv_sec + procusage.pr_ttime.tv_nsec / 1000000000.0;
    aProcess.waitTime =
        procusage.pr_tftime.tv_sec + procusage.pr_tftime.tv_nsec / 1000000000.0
        + procusage.pr_dftime.tv_sec + procusage.pr_dftime.tv_nsec / 1000000000.0
        + procusage.pr_kftime.tv_sec + procusage.pr_kftime.tv_nsec / 1000000000.0
        + procusage.pr_ltime.tv_sec + procusage.pr_ltime.tv_nsec / 1000000000.0
        + procusage.pr_wtime.tv_sec + procusage.pr_wtime.tv_nsec / 1000000000.0
        + procusage.pr_stoptime.tv_sec + procusage.pr_stoptime.tv_nsec / 1000000000.0;
    aProcess.sleepTime =
        procusage.pr_slptime.tv_sec + procusage.pr_slptime.tv_nsec / 1000000000.0;
    aProcess.cUserTime =
        currproc.pr_ctime.tv_sec + currproc.pr_ctime.tv_nsec / 1000000000.0;
    aProcess.cSystemTime = 0;
    aProcess.cWaitTime = 0;
    aProcess.cSleepTime = 0;
    aProcess.startTime = currproc.pr_start.tv_sec;
    aProcess.VSize = currproc.pr_size;
    aProcess.RSS = currproc.pr_rssize;
    aProcess.readBytes = procusage.pr_inblk * staticData.memory.pageSize;
    aProcess.writtenBytes = procusage.pr_oublk * staticData.memory.pageSize;


    dynamicData.processList[PID] = aProcess;
}


void
SolarisPerfData::getProcesssesData () throw (RunQError)
{
    struct dirent* directoryEntry;
    long PID;
    char buffer[256];

    // seteuid(0);
    rewinddir (procDirectory);
    while ((directoryEntry = readdir (procDirectory))) {
        PID = atol (directoryEntry->d_name);
        if (PID > 0)
            getProcesssData (PID);
    }
    // seteuid(getuid());
}


void
SolarisPerfData::getUsersData () throw (RunQError)
{
    StaticUserData aUser;
    struct passwd* passwdEntry;

    while (passwdEntry = getpwent ()) {
        aUser.uid = passwdEntry->pw_uid;
        strncpy (aUser.name, passwdEntry->pw_name, sizeof(aUser.name) - 1);
        aUser.name[sizeof(aUser.name) - 1] = '\0';
        staticData.users[aUser.uid] = aUser;
    }
    endpwent ();
}


void
SolarisPerfData::getGroupsData () throw (RunQError)
{
    StaticGroupData aGroup;
    struct group* groupEntry;

    while (groupEntry = getgrent ()) {
        aGroup.gid = groupEntry->gr_gid;
        strncpy (aGroup.name, groupEntry->gr_name, sizeof(aGroup.name));
        aGroup.name[sizeof(aGroup.name)] = '\0';
        staticData.groups[aGroup.gid] = aGroup;
    }
    endgrent ();
}
