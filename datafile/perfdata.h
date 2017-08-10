#ifndef RUNQ_PERFDATA_H
#define RUNQ_PERFDATA_H

/*
 * FILE : perfdata.h
 * SYSTEM : RunQ (RunQ/datafile/perfdata.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 *
 * CREATED : 03 jan 2000
 * VERSION : 1.00 (18-mar-2000)
 *
 * DESCRIPTION:
 *   This header file provides the total framework for the RunQ
 *   performance metrics.
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

#include "datastore.h"
#include "runq_error.h"
#include "staticperfdata.h"
#include "dynamicperfdata.h"

class PerfData {
public:
    void
    create (const char fileName[]);
    void
    open (const char fileName[]);
    void
    close ();
    void
    get ();
    void
    put ();
    virtual void
    sample (const bool fullSample);

public:
    DataStore dataStore;
    StaticPerfData staticData;
    DynamicPerfData dynamicData;
};

#endif // RUNQ_PERFDATA_H

