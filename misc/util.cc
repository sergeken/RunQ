#include <string.h>
#include <stdlib.h>
#include <sys/utsname.h>

#include "util.h"

using std::endl;

void usage(std::ostream & output)
{
#ifndef COLLECTOR_ONLY
  output << "usage: runq collect -N -d datafile -m number_of_minutes -S spantime_in_seconds" << endl;
  output << "usage: runq info -d datafile" << endl;
  output << "usage: runq report -C -d datafile -s startTime (HH:MM) -e endTime (HH:MM)" << endl;
  output << "usage: runq procs -D -T -d datafile -s startTime (HH:MM) -e endTime (HH:MM)" << endl;
  output << "usage: runq analyze -C -S -F -d datafile -w workloads -s startTime (HH:MM) -e endTime (HH:MM) -l logfile" << endl;
#else
  output << "usage: rqcollect -N -d datafile -m number_of_minutes -S spantime_in_seconds" << endl;
#endif
}

void runQLogo(std::ostream & output) throw(RunQError)
{
  struct utsname unameData;

  uname(&unameData);

  output << endl;
  output << RUNQ_PRODUCT_NAME
	 << " v" << RUNQ_VERSION
	 << " (c) 2000-2017 RC&S (Serge Robyns - Robyns Consulting & Services)."
	 << endl << endl;
  output << RUNQ_PRODUCT_NAME
	 << " comes with ABSOLUTELY NO WARRANTY." << endl
	 << " This software is licensed under the GPL." << endl
	 << " This is free software, and you are welcome" << endl
	 << " to redistribute it under certain conditions." << endl
	 << endl;
  output << "Running on "
	 << unameData.nodename
	 << " with "
	 << unameData.sysname
	 << " "
	 << unameData.release
	 << "." << endl;
  output << endl;

#ifdef linux
#else
#ifdef __OpenBSD__
#else
#ifdef sun
#else
#ifdef __hpux
#else
#error "INVALID PLATFORM"
#endif // HPUX
#endif // SOLARIS
#endif // OBSD
#endif // LINUX

}


bool beforeTime(time_t * inTimeStamp, int hour, int minute)
{
  struct tm * timeStamp;

  timeStamp = localtime(inTimeStamp);
  return (timeStamp->tm_hour < hour
	    || (timeStamp->tm_hour == hour && timeStamp->tm_min < minute)
	  );
}

void parseTime(char timeString[], int &hour, int &minute)
{
  char *p1, *p2;
  
  p1=strtok(timeString, ":");
  if (p1)
    {
      hour = atol(p1);
      p2 = strtok(0, ":");
      if (p2)
	{
	  minute=atol(p2);
	}
    }
}