/******************************************************************************
 * FILE : hpux.h
 * SYSTEM : RunQ (Runq/collectors/hpux/hpux.cc)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@advalvas.be
 * COPYRIGHT : (C) 2000-2002, by Robyns Consulting & Services (RC&S)
 *
 * CREATED : 10 may 2002
 * VERSION : 0.01 (10/may/2002)
 *
 * DESCRIPTION:
 *   This c++ file defines the superclass for RunQ's datafile
 *
 * CHANGELOG:
 *
 */

#include "hpux.h"

#include <grp.h>
#include <sys/utsname.h>
#include <sys/pstat.h>

#define CHUNK_SIZE      256

// #define CPU_IDLE	0
#define CPU_WAIT        0
#define CPU_USER        1
#define CPU_SYS         2
// #define CPU_WAIT	3
#define CPU_IDLE        3
// #define CPU_SXBRK	4
#define CPU_NICE        4

HPUXPerfData::HPUXPerfData() throw (RunQError)
{
    // seteuid(getuid());
    getStaticCPUData ();
    getStaticMemoryData ();
    getStaticIOData ();
    getUsersData ();
    getGroupsData ();
}

HPUXPerfData::~HPUXPerfData()
{}


void
HPUXPerfData::getStaticCPUData () throw (RunQError)
{
    struct utsname unameData;
    struct pst_dynamic psd;

    uname (&unameData);
    if (pstat_getdynamic (&psd, sizeof(psd), (size_t)1, 0) == -1)
        throw (RunQError::GenericError, "pstat_getdynamic");

    strncpy (staticData.CPU.node, unameData.nodename,
             sizeof(staticData.CPU.node) - 1);
    staticData.CPU.node[sizeof(staticData.CPU.node) - 1] = '\0';
    strcpy (staticData.CPU.vendor, "HP");
    strcpy (staticData.CPU.model, unameData.machine);
    strcpy (staticData.CPU.OSName, "HP-UX");
    strncpy (staticData.CPU.OSVersion, unameData.version,
             sizeof(staticData.CPU.OSVersion) - 1);
    staticData.CPU.OSVersion[sizeof(staticData.CPU.OSVersion) - 1] = '\0';
    strncpy (staticData.CPU.OSRelease, unameData.release,
             sizeof(staticData.CPU.OSRelease) - 1);
    staticData.CPU.OSRelease[sizeof(staticData.CPU.OSRelease) - 1] = '\0';

    staticData.CPU.numberOfCPUs = psd.psd_max_proc_cnt;
    staticData.CPU.availableCPUs = psd.psd_proc_cnt;
}


void
HPUXPerfData::getStaticMemoryData () throw (RunQError)
{
    struct pst_static pst;
    struct pst_dynamic psd;

    if (pstat_getstatic (&pst, sizeof(pst), (size_t)1, 0) == -1)
        throw (RunQError::GenericError, "pstat_getstatic");
    if (pstat_getdynamic (&psd, sizeof(psd), (size_t)1, 0) == -1)
        throw (RunQError::GenericError, "pstat_getdynamic");

    staticData.memory.pageSize = pst.page_size;
    staticData.memory.physicalMemory = (double)psd.psd_rm
                                       * (double)staticData.memory.pageSize;
    staticData.memory.virtualMemory = (double)psd.psd_vm
                                      * (double)staticData.memory.pageSize;
}



void
HPUXPerfData::getDynamicMemoryData () throw (RunQError)
{
    struct pst_dynamic psd;

    if (pstat_getdynamic (&psd, sizeof(psd), (size_t)1, 0) == -1)
        throw (RunQError::GenericError, "pstat_getdynamic");

    dynamicData.memory.free = (double)psd.psd_free
                              * (double)staticData.memory.pageSize;
}


void
HPUXPerfData::getStaticIOData () throw (RunQError)
{
    StaticDiskData aDisk;

    dynamicData.IO.bytesRead = 0;
    dynamicData.IO.bytesWritten = 0;

    // staticData.IO.diskData.push_back(aDisk);
}


void
HPUXPerfData::getDynamicCPUData () throw (RunQError)
{
    int Counter;
    struct pst_dynamic psd;

    dynamicData.CPU.idle = 0.0;
    dynamicData.CPU.wait = 0.0;
    dynamicData.CPU.user = 0.0;
    dynamicData.CPU.system = 0.0;
    dynamicData.CPU.nice = 0.0;

    if (pstat_getdynamic (&psd, sizeof(psd), (size_t)1, 0) == -1)
        throw (RunQError::GenericError, "pstat_getdynamic");

    dynamicData.CPU.idle = psd.psd_cpu_time[CPU_IDLE] / 100.0;
    dynamicData.CPU.wait = psd.psd_cpu_time[CPU_WAIT] / 100.0;
    dynamicData.CPU.user = psd.psd_cpu_time[CPU_USER] / 100.0;
    dynamicData.CPU.system = psd.psd_cpu_time[CPU_SYS] / 100.0;
    dynamicData.CPU.nice = 0; // psd.psd_cpu_time[CPU_NICE] / 100.0;
    // dynamicData.CPU.runQueue = psd.psd_rq;
    dynamicData.CPU.runQueue = psd.psd_avg_1_min;
}


void
HPUXPerfData::getDynamicIOData () throw (RunQError)
{
    dynamicData.IO.bytesRead = 0;
    dynamicData.IO.bytesWritten = 0;
}


void
HPUXPerfData::sample (const bool fullSample) throw (RunQError)
{
    getProcesssesData ();
    if (fullSample) {
        getDynamicCPUData ();
        getDynamicIOData ();
        getDynamicMemoryData ();
    }
}


void
HPUXPerfData::getProcesssesData () throw (RunQError)
{
    int NrOfProcesses;
    int Index;
    int i;
    ProcessData aProcess;
    struct pst_status pst[CHUNK_SIZE];
    const double cycles2secondsDivider = pow (2.0, 32 - 5);
    double cycleSeconds;

    // seteuid(0);
    for (Index = 0;
         (NrOfProcesses = pstat_getproc (pst, sizeof(pst[0]), CHUNK_SIZE, Index))
         > 0;
         Index += NrOfProcesses) {
        for (i = 0; i < NrOfProcesses; i++) {
            aProcess.PID = pst[i].pst_pid;
            ;
            strncpy (aProcess.name, pst[i].pst_ucomm, sizeof(aProcess.name) - 1);
            aProcess.name[sizeof(aProcess.name) - 1] = '\0';
            strncpy (aProcess.args, pst[i].pst_cmd, sizeof(aProcess.args) - 1);
            aProcess.args[sizeof(aProcess.args) - 1] = '\0';
            aProcess.PPID = pst[i].pst_ppid;
            aProcess.PGRP = pst[i].pst_pgrp;
            aProcess.uid = pst[i].pst_uid;
            aProcess.gid = pst[i].pst_gid;
            aProcess.minorFaults = pst[i].pst_minorfaults;
            aProcess.cMinorFaults = 0;
            aProcess.majorFaults = pst[i].pst_majorfaults;
            aProcess.cMajorFaults = 0;
//	  cycleSeconds = pst[i].pst_usercycles.psc_hi * 32.0
//	        + pst[i].pst_usercycles.psc_lo / cycles2secondsDivider;
//	  if (pst[i].pst_utime < cycleSeconds
//	      && pst[i].pst_utime+1 > cycleSeconds)
//	    aProcess.userTime = cycleSeconds;
//	  else
            aProcess.userTime = pst[i].pst_utime;
//	  cycleSeconds = pst[i].pst_systemcycles.psc_hi * 32.0
//	        + pst[i].pst_systemcycles.psc_lo / cycles2secondsDivider;
//	  if (pst[i].pst_stime < cycleSeconds
//	      && pst[i].pst_stime+1 > cycleSeconds)
//	    aProcess.systemTime = cycleSeconds;
//	  else
            aProcess.systemTime = pst[i].pst_stime;
            aProcess.waitTime = 0;
            aProcess.cUserTime = 0;
            aProcess.cSystemTime = 0;
            aProcess.cWaitTime = 0;
            aProcess.startTime = pst[i].pst_start;
            aProcess.VSize = 0;
            aProcess.RSS = pst[i].pst_rssize;
            aProcess.readBytes = 0;
            aProcess.writtenBytes = 0;

            dynamicData.processList[aProcess.PID] = aProcess;
        }
    }
    // seteuid(getuid());
}


void
HPUXPerfData::getUsersData () throw (RunQError)
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
HPUXPerfData::getGroupsData () throw (RunQError)
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
