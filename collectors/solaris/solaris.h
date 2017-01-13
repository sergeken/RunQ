#ifndef RUNQ_SOLARIS_H
#define RUNQ_SOLARIS_H

/******************************************************************************
 * FILE : linux.h
 * SYSTEM : RunQ (Runq/collectors/solaris/solaris.h)
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

#include <stdio.h>
#include <unistd.h>
#include <dirent.h>

#include <kstat.h>
#include <sys/cpuvar.h>
/* #include <sys/proc.h> */
#include <procfs.h>

#undef CPU

#include "perfdata.h"
#include "runq_error.h"


class SolarisPerfData : public PerfData
{
 public: 
  SolarisPerfData() throw(RunQError);
  ~SolarisPerfData();
  virtual void sample(const bool lastSample) throw(RunQError);
 private:
  kstat_ctl_t * kc;
  DIR *procDirectory;
 private:
  void getStaticCPUData() throw(RunQError);
  void getStaticMemoryData() throw(RunQError);
  void getStaticIOData() throw(RunQError);
  void getUsersData() throw(RunQError);
  void getGroupsData() throw(RunQError);
  void getDynamicCPUData() throw(RunQError);
  void getDynamicIOData() throw(RunQError);
  void getDynamicMemoryData() throw(RunQError);
  void getProcesssData(const long Pid) throw(RunQError);
  void getProcesssesData() throw(RunQError);
};

#endif // RUNQ_SOLARIS_H
