#ifndef RUNQ_COLLECT_H
#define RUNQ_COLLECT_H

/*
 * FILE : collect.h
 * SYSTEM : RunQ (RunQ/collector/collect.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 * 
 * CREATED : 04 jan 2000
 * VERSION : 1.00 (18-mar-2000)
 *
 * DESCRIPTION:
 *   This header file defines the superclass for RunQ's performance collectors.
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

#include "runq_error.h"

void collect(const char dataFile[], const int duration, const int spanTime,
	     const bool noSleep)
     throw(RunQError);
void collect_main(const int argc, char *argv[]) throw(RunQError);

#endif // RUNQ_COLLECT_H
