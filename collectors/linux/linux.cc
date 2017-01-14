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
 *   This file implementes the performance collector for Linux.
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

#include "linux.h"

#define LINUX_STATFILE  "/proc/stat"
#define LINUX_MEMFILE   "/proc/meminfo"
#define LINUX_LOADFILE  "/proc/loadavg"
#define LINUX_PROCDIR   "/proc"
#define LINUX_CPUINFO   "/proc/cpuinfo"
#define LINUX_DISKFILE  "/proc/diskstats"
#define LINUX_NETWORK	"/proc/net/dev"

#define PROCESSORNUM	"processor"
#define VENDORID	"vendor_id"
#define MODELNAME	"model name"
#define FREQUENCY	"cpu MHz"
#define CACHESIZE	"cache size"
#define SPECINT		"bogomips"

#define MEMTOTID	"MemTotal:"
#define MEMFREEID	"MemFree:"
#define MEMBUFFERID	"Buffers:"
#define MEMCACHEDID	"Cached:"

#define PAGINGID	"page"
#define CPUDYNID	"cpu "
#define IDEHDID         "hd"
#define IDEBASE		"/proc/ide/"
#define IDEMODEL	"/model"
#define SCSIHDID        "sd"
#define SCSIBASE	"/proc/scsi/scsi"
#define LASTPIDID	"processses"


//#define GOOD_LINUX_VERSION "2.6"
#define GOOD_LINUX_VERSION "4.8"

LinuxPerfData::LinuxPerfData() throw(RunQError)
{
    struct utsname unameData;

    uname(&unameData);
    if ( strcmp(unameData.sysname, "Linux") 
	    || strncmp(unameData.release, GOOD_LINUX_VERSION,
		sizeof(GOOD_LINUX_VERSION) - 1 ) 
       )
    {
	throw RunQError(RunQError::InvalidVersion);
    }

    statFile = fopen(LINUX_STATFILE, "r");
    if (statFile == NULL)
	throw(RunQError::FileNotFound, LINUX_STATFILE);

    diskFile = fopen(LINUX_DISKFILE, "r");
    if (diskFile == NULL)
	throw(RunQError::FileNotFound, LINUX_DISKFILE);

    memoryFile = fopen(LINUX_MEMFILE, "r");
    if (memoryFile == NULL)
    {
	fclose(statFile);
	throw(RunQError::FileNotFound, LINUX_MEMFILE);
    }

    loadFile = fopen(LINUX_LOADFILE, "r");
    if (loadFile == NULL)
    {
	fclose(memoryFile);
	fclose(statFile);
	throw(RunQError::FileNotFound, LINUX_LOADFILE);
    }

    networkFile = fopen(LINUX_NETWORK, "r");
    if (networkFile == NULL)
    {
	fclose(memoryFile);
	fclose(statFile);
	throw(RunQError::FileNotFound, LINUX_NETWORK);
    }

    procDirectory = opendir(LINUX_PROCDIR);
    if (procDirectory == NULL)
    {
	fclose(memoryFile);
	fclose(statFile);
	fclose(networkFile);
	throw(RunQError::NoProcDirectory, LINUX_PROCDIR);
    }

    getStaticCPUData();
    getStaticMemoryData();
    getStaticIOData();
    getStaticNetworkData();
    getUsersData();
    getGroupsData();
}

LinuxPerfData::~LinuxPerfData()
{
    closedir(procDirectory);
    fclose(memoryFile);
    fclose(statFile);
}

void LinuxPerfData::sample(bool lastSample) throw(RunQError)
{
    getProcessList();
    if (lastSample)
    {
	getDynamicCPUData();
	getDynamicMemoryData();
	getDynamicIOData();
    }
}

void LinuxPerfData::getDynamicCPUData() throw(RunQError)
{
    char	buffer[256];

    rewind(statFile);	
    while ( fgets(buffer, sizeof(buffer)-1, statFile) )
    {
	if (memcmp(buffer, CPUDYNID, sizeof(CPUDYNID) - 1) == 0 )
	{
	    sscanf(buffer, "%*s %lf %lf %lf %lf %lf",
		    &dynamicData.CPU.user,
		    &dynamicData.CPU.nice,
		    &dynamicData.CPU.system,
		    &dynamicData.CPU.idle,
		    &dynamicData.CPU.wait);
	}
    }

    rewind(loadFile);	
    fscanf(loadFile, "%lf", &dynamicData.CPU.runQueue);

    dynamicData.CPU.user /= 100.0;
    dynamicData.CPU.nice /= 100.0;
    dynamicData.CPU.system /= 100.0;
    dynamicData.CPU.idle /= 100.0;
}


void LinuxPerfData::getDynamicIOData() throw(RunQError)
{
    double d1, d2, d3, d4;
    int	minor;
    char	buffer[256];
    char	name[256];

    dynamicData.IO.bytesRead = 0;
    dynamicData.IO.bytesWritten = 0;
    rewind(diskFile);
    while ( fgets(buffer, sizeof(buffer)-1, diskFile) )
    {
	minor = -1;
	sscanf(buffer, "%*d %d %s %*lf %*lf %lf %*lf %*lf %*lf %lf %*lf %*lf %*lf %*lf", &minor, name, &d1, &d2);
	if (minor >= 0 && (minor & 0x0F) == 0x0)
	{
	    if (memcmp(name, IDEHDID, sizeof(IDEHDID) - 1) == 0 ||
		    memcmp(name, SCSIHDID, sizeof(SCSIHDID) - 1) == 0 )
	    {
		dynamicData.IO.bytesWritten += d2;
	    }
	}
    }
}

void LinuxPerfData::getDynamicMemoryData() throw(RunQError)
{
    char buffer[256];

    rewind(memoryFile);	
    while ( fgets(buffer, sizeof(buffer)-1, memoryFile) )
    {
	if (memcmp(buffer, MEMFREEID, sizeof(MEMFREEID) - 1) == 0 )
	{
	    sscanf(buffer, "%*s %lf", &dynamicData.memory.free);
	}
	else if (memcmp(buffer, MEMBUFFERID, sizeof(MEMBUFFERID) - 1) == 0 )
	{
	    sscanf(buffer, "%*s %lf", &dynamicData.memory.buffers);
	}
	else if (memcmp(buffer, MEMCACHEDID, sizeof(MEMCACHEDID) - 1) == 0 )
	{
	    sscanf(buffer, "%*s %lf", &dynamicData.memory.cached);
	}
    }
    dynamicData.memory.used = staticData.memory.physicalMemory
	- dynamicData.memory.free;

    rewind(statFile);	
    while ( fgets(buffer, sizeof(buffer)-1, statFile) )
    {
	if (memcmp(buffer, PAGINGID, sizeof(PAGINGID) - 1) == 0 )
	{
	    sscanf(buffer, "%*s %ld %ld",
		    &dynamicData.memory.pagedIn,
		    &dynamicData.memory.pagedOut );
	}
    }
}

void LinuxPerfData::getProcessData(long PID) throw(RunQError)
{
    ProcessData aProcess;
    FILE * processFile;
    int bytes;
    char buffer[256];

    sprintf(buffer, LINUX_PROCDIR "/%d/stat", (int)PID);
    processFile = fopen(buffer, "r");
    if (processFile == NULL)
    {
	// We won't throw an execption here, as probably the process died
	// while we where watching.
	return;
    }
    if ( fscanf(processFile,
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
		    ) > 0)
		    {
			aProcess.userTime /= 100.0;
			aProcess.systemTime /= 100.0;
			aProcess.waitTime = 0.0;
			aProcess.sleepTime = 0.0;
			aProcess.cUserTime /= 100.0;
			aProcess.cSystemTime /= 100.0;
			aProcess.cWaitTime = 0.0;
			aProcess.cSleepTime = 0.0;
			aProcess.name[strlen(aProcess.name) - 1] = '\0'; // Trim trailing ) Yuck!
			aProcess.readBytes = 0;
			aProcess.writtenBytes = 0;
		    }
    fclose(processFile);

    sprintf(buffer, LINUX_PROCDIR "/%d/status", (int)PID);
    processFile = fopen(buffer, "r");
    if (processFile != NULL)
    {
	while ( fgets(buffer, sizeof(buffer)-1, processFile) )
	{
	    if (memcmp(buffer, "Uid:", 4) == 0 )
	    {
		sscanf(buffer, "%*s %d",
			&aProcess.uid );
	    }
	    else if (memcmp(buffer, "Gid:", 4) == 0 )
	    {
		sscanf(buffer, "%*s %d",
			&aProcess.gid );
		break;
	    }
	}
	fclose(processFile);
    }

    sprintf(buffer, LINUX_PROCDIR "/%d/cmdline", (int)PID);
    processFile = fopen(buffer, "r");
    if (processFile != NULL)
    {
	bytes = fread(aProcess.args, 1, sizeof(aProcess.args), processFile);
	fclose(processFile);
	aProcess.args[bytes] = '\0';
	// Patch the commandline, don't remember anymore why.
	for (bytes--; bytes>=0; bytes--)
	{
	    switch(aProcess.args[bytes])
	    {
		case '\0' : aProcess.args[bytes] = ' '; break;
			    // case '\n': aProcess.args[bytes] = '\0'; break;
	    }
	}
    }

    dynamicData.processList[PID] = aProcess;
}

void LinuxPerfData::getProcessList() throw(RunQError)
{
    struct dirent * directoryEntry;
    long PID;
    char buffer[256];

    rewinddir(procDirectory);
    while ( (directoryEntry = readdir(procDirectory)) )
    {
	try
	{
	    PID = atol(directoryEntry->d_name);
	    if (PID > 0)
		getProcessData(PID);
	}
	catch (RunQError &error)
	{
	    switch(error.error)
	    {
		case RunQError::FileNotFound:
		    break;
		default:
		    throw;
	    }
	}
    }
}

void LinuxPerfData::getStaticCPUData() throw(RunQError)
{
    char	buffer[256];
    struct utsname unameData;

    uname(&unameData);
    strncpy(staticData.CPU.node, unameData.nodename,
	    sizeof(staticData.CPU.node)-1);
    staticData.CPU.node[sizeof(staticData.CPU.node)-1] = '\0';
    strcpy(staticData.CPU.model, unameData.machine);
    strcpy(staticData.CPU.OSName, unameData.sysname);
    strcpy(staticData.CPU.OSVersion, unameData.version);
    strcpy(staticData.CPU.OSRelease, unameData.release);

    FILE * CPUFile = fopen (LINUX_CPUINFO, "r");
    if (CPUFile == NULL)
	throw(RunQError::FileNotFound, LINUX_CPUINFO);

    staticData.CPU.numberOfCPUs = 0;

    while ( fgets(buffer, sizeof(buffer)-1, CPUFile) )
    {
	if (memcmp(buffer, PROCESSORNUM, sizeof(PROCESSORNUM) - 1) == 0 )
	{
	    staticData.CPU.numberOfCPUs++;
	}
	else if (memcmp(buffer, VENDORID, sizeof(VENDORID) - 1) == 0 )
	{
	    sscanf(buffer, "%*s : %s", staticData.CPU.vendor);
	}
	else if (memcmp(buffer, MODELNAME, sizeof(MODELNAME) - 1) == 0 )
	{
	    sscanf(buffer, "%*s %*s : %[^\n]", staticData.CPU.model);
	}
	else if (memcmp(buffer, FREQUENCY, sizeof(FREQUENCY) - 1) == 0 )
	{
	    sscanf(buffer, "%*s %*s : %lf", &staticData.CPU.frequency);
	}
	else if (memcmp(buffer, CACHESIZE, sizeof(CACHESIZE) - 1) == 0 )
	{
	    sscanf(buffer, "%*s %*s : %d", &staticData.CPU.cacheSize);
	}
	else if (memcmp(buffer, SPECINT, sizeof(SPECINT) - 1) == 0 )
	{
	    sscanf(buffer, "%*s : %lf", &staticData.CPU.specint);
	}
    }

    staticData.CPU.availableCPUs = staticData.CPU.numberOfCPUs;

    fclose(CPUFile);
}


void LinuxPerfData::getStaticMemoryData() throw(RunQError)
{
    char	buffer[256];

    staticData.memory.pageSize = getpagesize();

    rewind(memoryFile);	
    while ( fgets(buffer, sizeof(buffer)-1, memoryFile) )
    {
	if (memcmp(buffer, MEMTOTID, sizeof(MEMTOTID) - 1) == 0 )
	{
	    sscanf(buffer, "%*s %lf", &staticData.memory.physicalMemory);
	}
    }
}


void LinuxPerfData::getStaticNetworkData() throw(RunQError)
{
    StaticNetworkInterfaceData anInterface;
    char	buffer[256];
    int i;

    for ( i=2; i>0 && fgets(buffer, sizeof(buffer)-1, networkFile); i--)
	;
    while ( fgets(buffer, sizeof(buffer)-1, networkFile) )
    {
	char	buffer2[256];
	if (sscanf(buffer, "%*[ ]%[a-zA-Z0-9]:", buffer2) > 0 )
	{
	    strncpy(anInterface.name, buffer2, sizeof(anInterface.name)-1);	    
	    anInterface.ip4[0] = '\0';
	    staticData.network.interfaces.push_back(anInterface);
	}
    }
}


void LinuxPerfData::getStaticIOData() throw(RunQError)
{
    StaticDiskData aDisk;
    char	buffer[256];
    int	minor;

    aDisk.type[0] = '\0';
    while ( fgets(buffer, sizeof(buffer)-1, diskFile) )
    {
	minor = -1;
	sscanf(buffer, "%*d %d %s ", &minor, aDisk.name);
	if (minor >= 0 && (minor & 0x0F) == 0x0)
	{
	    aDisk.type[0] = '\0';
	    if (memcmp(aDisk.name, IDEHDID, sizeof(IDEHDID) - 1) == 0)
	    {
		FILE * ideFile;
		char buffer2[256];

		strncpy(buffer2, IDEBASE, sizeof(buffer2)-1);
		strncat(buffer2, aDisk.name, sizeof(buffer2)-1);
		strncat(buffer2, IDEMODEL, sizeof(buffer2)-1);
		ideFile = fopen(buffer2, "r");
		if (ideFile != NULL)
		{
		    if (fgets(buffer, sizeof(buffer)-1, ideFile))
		    {
			int i = strlen(buffer);
			if (i>0 && buffer[i-1] == '\n')
			{
			    buffer[i-1] = '\0';
			}
		    	strncpy(aDisk.type, buffer, sizeof(aDisk.type)-1);
		    }
		    fclose(ideFile);
		}
	    }
	    else if (memcmp(aDisk.name, SCSIHDID, sizeof(SCSIHDID) - 1) == 0)
	    {
#if 0
		FILE * scsiFile;
		char buffer2[256];

		strncpy(buffer2, SCSIBASE, sizeof(buffer2)-1);
		scsiFile = fopen(buffer2, "r");
		if (scsiFile != NULL)
		{
		    if (fgets(buffer, sizeof(buffer)-1, scsiFile))
		    {
		    }
		    fclose(scsiFile);
		}
#endif
	    }
	    else
		continue;
	    staticData.IO.diskData.push_back(aDisk);
	}
    }
}

void LinuxPerfData::getUsersData() throw(RunQError)
{
    StaticUserData aUser;
    struct passwd * passwdEntry;

    while ( passwdEntry = getpwent() )
    {
	aUser.uid = passwdEntry->pw_uid;
	strcpy(aUser.name, passwdEntry->pw_name);
	staticData.users[aUser.uid] = aUser;
    }
    endpwent();
}

void LinuxPerfData::getGroupsData() throw(RunQError)
{
    StaticGroupData aGroup;
    struct group * groupEntry;

    while ( groupEntry = getgrent() )
    {
	aGroup.gid = groupEntry->gr_gid;
	strcpy(aGroup.name, groupEntry->gr_name);
	staticData.groups[aGroup.gid] = aGroup;
    }
    endgrent();
}
