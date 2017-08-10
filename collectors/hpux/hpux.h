#ifndef RUNQ_HPUX_H
#define RUNQ_HPUX_H

/******************************************************************************
 * FILE : hpux.h
 * SYSTEM : RunQ (Runq/collectors/hpux/hpux.h)
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

#include <unistd.h>

#include "perfdata.h"
#include "runq_error.h"


class HPUXPerfData : public PerfData {
public:
    HPUXPerfData();
    ~HPUXPerfData();
    virtual void
    sample (const bool fullSample);
private:
    //  kstat_ctl_t * kc;
    //  DIR *procDirectory;
private:
    void
    getStaticCPUData ();
    void
    getStaticMemoryData ();
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
    getProcesssData (const long Pid);
    void
    getProcesssesData ();
};

#endif // RUNQ_HPUX_H
