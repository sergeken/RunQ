/******************************************************************************
 * FILE : perfdata.cc
 * SYSTEM : RunQ (RunQ/datafile/perfdata.cc)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 * 
 * CREATED : 03 jan 2000
 * VERSION : 1.00 (18-mar-2000)
 *
 * DESCRIPTION:
 *   This file provides the total framework for the RunQ performance metrics.
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

#include "perfdata.h"

void PerfData::create(const char fileName[]) throw(RunQError)
{
  dataStore.open((char*)fileName, std::ios_base::binary | std::ios_base::out, RunQFileType::RawData);
  staticData.put(dataStore);
}

void PerfData::open(const char fileName[]) throw(RunQError)
{
  dataStore.open((char*)fileName, std::ios_base::binary | std::ios_base::in,  		 RunQFileType::RawData);
  staticData.get(dataStore);
}

void PerfData::close()
{
  dataStore.close();
}

void PerfData::get() throw(RunQError)
{
  dynamicData.get(dataStore);
}

void PerfData::put() throw(RunQError)
{
  dynamicData.put(dataStore);
}


void PerfData::sample(bool fullSample) throw(RunQError)
{
    throw(RunQError::NotSupported, "PerfData::sample");
}
