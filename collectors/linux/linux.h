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

#include <stdio.h>
#include <dirent.h>

#include "runq_error.h"
#include "perfdata.h"

#define MAX_DISKS	256

class LinuxPerfData : public PerfData
{
 public:
  LinuxPerfData() throw(RunQError);
  ~LinuxPerfData();
  virtual void sample(bool fullSample) throw(RunQError);
 private:
  void getStaticCPUData() throw(RunQError);
  void getStaticMemoryData() throw(RunQError);
  void getStaticNetworkData() throw (RunQError);
  void getStaticIOData() throw(RunQError);
  void getUsersData() throw(RunQError);
  void getGroupsData() throw(RunQError);
  void getDynamicCPUData() throw(RunQError);
  void getDynamicIOData() throw(RunQError);
  void getDynamicMemoryData() throw(RunQError);
  void getProcessData(long) throw(RunQError);
  void getProcessList() throw(RunQError);
  void scanIOControllerDirectory(char controllerDir[], char diskPrefix[])
    throw(RunQError);

 private:
  FILE * statFile;
  FILE * memoryFile;
  FILE * diskFile;
  FILE * networkFile;
  FILE * loadFile;
  DIR * procDirectory;
};

#endif // RUNQ_LINUX_H

