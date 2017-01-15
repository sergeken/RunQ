#ifndef RUNQ_SURVEYOR_H
#define RUNQ_SURVEYOR_H

/*
 * FILE : surveyor.h
 * SYSTEM : RunQ (Runq/surveyor/surveyor.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 *
 * CREATED : 19 aug 2000
 * VERSION : 1.00 ()
 *
 * DESCRIPTION:
 *
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

using namespace std;

#include <iostream.h>
#include <string.h>
#include <stl.h>

#include "perfdata.h"
#include "rq_regexp.h"

#ifdef linux
#include "../collectors/linux/linux.h"
#define PERFDATA LinuxPerfData
#else
#ifdef sun
#include "../collectors/solaris/solaris.h"
#define PERFDATA SolarisPerfData
#else
#error "INVALID PLATFORM"
#endif // SOLARIS
#endif // LINUX

class Message {
public:
    enum MessageType { UKNOWN=0, TEXT, DATE, TIME };
    MessageType type;
    char*      text;
};

class WatchValues {
public:
    WatchValues()
    {
        name = RegExp ("");
        args = RegExp ("");
        CPUUsage = 0.0;
    };
    WatchValues(char aName[])
    {
        name = RegExp (aName);
        args = RegExp ("");
        CPUUsage = 0.0;
    };
    WatchValues(char aName[],
                double aValue, vector<Message> aMessage)
    {
        name = RegExp (aName);
        args = RegExp ("");
        CPUUsage = aValue;
        message = aMessage;
    };
    WatchValues(char aName[], char arg[])
    {
        name = RegExp (aName);
        args = RegExp (arg);
        CPUUsage = 0.0;
    };
    WatchValues(char aName[], char arg[], double aValue,
                vector<Message> aMessage)
    {
        name = RegExp (aName);
        args = RegExp (arg);
        CPUUsage = aValue;
        message = aMessage;
    };

public:
    RegExp name;
    RegExp args;
    double CPUUsage;
    vector<Message> message;
};


class Surveyor : public PERFDATA {
public:
    void
    watchProcesses (void) throw (RunQError);
private:
    WatchValues*
    watchProcess (const char name[], const char args[]);
public:
    vector<WatchValues> watchValues;
private:
};

#endif // RUNQ_SUVEYOR_H
