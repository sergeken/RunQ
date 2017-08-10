#ifndef RUNQ_LINUX_H
#define RUNQ_LINUX_H

/*
 * FILE : linux.h
 * SYSTEM : RunQ (Runq/collectors/linux/linux.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@advalvas.be
 * COPYRIGHT : (C) 2000, by Robyns Consulting & Services (RC&S)
 *
 * CREATED : 04 jan 2000
 * VERSION : 1.00 (18-mar-2000)
 *
 * DESCRIPTION:
 *   This header file defines the performance collector for Linux.
 *
 * CHANGELOG:
 *
 */

// using namespace std;

#include <cstdio>
#include <dirent.h>

#include "runq_error.h"
#include "perfdata.h"

#define MAX_DISKS       256

class LinuxPerfData : public PerfData {
public:
    LinuxPerfData();
    ~LinuxPerfData();
    void
    sample (bool lastSample) override;
private:
    void
    getStaticCPUData ();
    void
    getStaticMemoryData ();
    void
    getStaticNetworkData ();
    void
    getStaticIOData ();
    void
    getUsersData ();
    void
    getGroupsData ();
    void
    getDynamicCPUData ();
    void
    getDynamicIOData ();
    void
    getDynamicMemoryData ();
    void
    getProcessData (long);
    void
    getProcessList ();
    void
    scanIOControllerDirectory (char controllerDir[], char diskPrefix[]);

private:
    FILE* statFile;
    FILE* memoryFile;
    FILE* diskFile;
    FILE* networkFile;
    FILE* loadFile;
    DIR* procDirectory;
};

#endif // RUNQ_LINUX_H

