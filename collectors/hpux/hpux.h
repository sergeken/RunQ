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
    HPUXPerfData() throw (RunQError);
    ~HPUXPerfData();
    virtual void
    sample (const bool lastSample) throw (RunQError);
private:
    //  kstat_ctl_t * kc;
    //  DIR *procDirectory;
private:
    void
    getStaticCPUData () throw (RunQError);
    void
    getStaticMemoryData () throw (RunQError);
    void
    getStaticIOData () throw (RunQError);
    void
    getUsersData () throw (RunQError);
    void
    getGroupsData () throw (RunQError);
    void
    getDynamicCPUData () throw (RunQError);
    void
    getDynamicIOData () throw (RunQError);
    void
    getDynamicMemoryData () throw (RunQError);
    void
    getProcesssData (const long Pid) throw (RunQError);
    void
    getProcesssesData () throw (RunQError);
};

#endif // RUNQ_HPUX_H
