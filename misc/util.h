#ifndef RUNQ_MISC_H
#define RUNQ_MISC_H

/*
 * FILE : misc.h
 * SYSTEM : RunQ (RunQ/datafile/misc.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 *
 * CREATED : 06 jan 2000
 * VERSION : 1.00 (18-mar-2000)
 *
 * DESCRIPTION:
 *   This header file provides prototypes for miscelanous functions
 *   In the future this will will provide a compressed datastore.
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

#include <time.h>
#include <unistd.h>
#include <iostream>

#include "runq_error.h"
#include "version.h"

void
usage (std::ostream & output);
bool
beforeTime (const time_t & inTimeStamp, const int hour, const int minute);
void
parseTime (char timeString[], int & hour, int & minute);
void
runQLogo (std::ostream & output) throw (RunQError);

#endif // RUNQ_MISC_H
