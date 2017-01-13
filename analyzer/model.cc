/*
 * FILE : model.cc
 * SYSTEM : RunQ (Runq/analyzer/model.cc)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 * 
 * CREATED : 05 jan 2000
 * VERSION : 1.00 (18-mar-2000)
 *
 * DESCRIPTION:
 *   This file provides model analysis functionality.
 *   It computes various things like response time.
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


#include "model.h"
#include <math.h>

using std::endl;


inline static double fac(const double n)
{
  double i;
  double result = 1;
  
  for (i=1; i<=n; i++)
    result *= i;
  
  return result;
}

inline static double ErlangC(const int numOfCPUs, const double utilisation)
{
  int k;
  
  double adjustedUtilization = utilisation * numOfCPUs;
  double ratio = pow(adjustedUtilization, numOfCPUs) / fac(numOfCPUs);
  
  double sums = 1.0;
  for (k=1; k<numOfCPUs; k++)
    sums += pow(adjustedUtilization, k) / fac(k);
  
  return ratio / ( ratio + (1.0-utilisation) * sums);
}

double Model::responseTime(const int numOfCPUs, const double utilisation,
				const double serviceTime)
{
  if (numOfCPUs == 1)
  {
    return serviceTime / (1 - utilisation);
  }
  else
  {
    return serviceTime + (ErlangC(numOfCPUs, utilisation) * serviceTime) /
    	(numOfCPUs * (1 - utilisation) );
  }
}


void Model::simulate(std::ostream & output, const bool csvOutput)
{
  double	totalCPU = 0;
  double 	elapsedCPU = 0;
  double        elapsedSeconds = 0;
  double	CPULoad = 0;
  double	CPUTime = 0;
  double        serviceTime = 0 ;
  std::vector<WorkLoad>::iterator	workLoads;
  std::vector<ProcessGroup>::iterator	processGroups;
  
  for (workLoads = this->workLoads.begin();
       workLoads != this->workLoads.end();
       workLoads++)
    {
      for (processGroups = workLoads->processGroups.begin();
	   processGroups != workLoads->processGroups.end();
	   processGroups++)
	{
	  totalCPU += processGroups->userCPU + processGroups->systemCPU;
	}
    }
  
  totalCPU = userCPU + niceCPU + systemCPU;
  elapsedCPU = totalCPU + idleCPU + waitCPU;
  if (elapsedCPU == 0)
    {
      elapsedCPU = 1;
      elapsedSeconds = 1;
    }
  elapsedSeconds = elapsedCPU / numberOfCPUs;
  CPULoad = totalCPU / elapsedCPU;
  
  if (!csvOutput)
    {
      output << "Elapsed CPU: " << elapsedCPU << " seconds" << endl;
      output << "Elapsed Time: " << elapsedSeconds << " seconds" << endl;
      output << "Total CPU: " << totalCPU << " seconds" << endl;
      output << "CPU Load: " << 100.0 * CPULoad << "%" << endl;
      output << "Capture Ratio: " 
	     << 100.0 * ( 1.0 - 
			  ((unaccountedUserCPU + unaccountedSystemCPU) / totalCPU)
			  )
	     << "%" << endl;
      output << endl;
    }
  else
    {
      output << "WorkLoad;ProcessGroup;CPUSeconds;UserSeconds;SystemSeconds;BytesRead;BytesWritten;CPULoad;ServiceTime;ResponseTime" << endl;
    }
  
  for (workLoads = this->workLoads.begin();
       workLoads != this->workLoads.end();
       workLoads++)
    {
      double wklCPU = 0;
      int wklProcessCount = 0;
      if (!csvOutput)
	output << "Workload : " << workLoads->name << endl;
      for (processGroups = workLoads->processGroups.begin();
	   processGroups != workLoads->processGroups.end();
	   processGroups++)
	{
	  CPUTime = processGroups->userCPU + processGroups->systemCPU;
	  wklCPU += CPUTime;
	  wklProcessCount += processGroups->activeProcessCount;
	  if (processGroups->activeProcessCount > 1)
	    serviceTime = CPUTime
	      / ((double) processGroups->activeProcessCount * elapsedSeconds);
	  else
	    serviceTime = CPUTime / (double)elapsedSeconds;
	  if (!csvOutput)
	    {
	      output << "  Process Group: " << processGroups->name << endl;
	      output << "    CPU usage : "
		     << 100.0 * CPUTime / elapsedCPU
		     << "% (" 
		     << CPUTime
		     << " seconds ("
		     << processGroups->userCPU << " user + "
		     << processGroups->systemCPU << " system))" << endl;
	      output << "    Bytes read : " << processGroups->readBytes
		     << " bytes written: " << processGroups->writtenBytes << endl;
	      output << "    CPU service time : " << serviceTime
		     << "    Response time : " << responseTime(numberOfCPUs, CPULoad, serviceTime)
		     << endl;
	    }
	  else
	    {
	      output << workLoads->name << ";"
		     << processGroups->name << ";"
		     << CPUTime << ";"
		     << processGroups->userCPU << ";"
		     << processGroups->systemCPU << ";"
		     << processGroups->readBytes << ";"
		     << processGroups->writtenBytes << ";"
		     << 100.0 * CPUTime / elapsedCPU << ";"
		     << serviceTime << ";"
		     << responseTime(numberOfCPUs, CPULoad, serviceTime)
		     << endl;
	    }
	}
      if (!csvOutput)
	{
	  if (wklProcessCount > 1 )
	    serviceTime = wklCPU / ((double) wklProcessCount * elapsedSeconds);
	  else
	    serviceTime = wklCPU / elapsedSeconds;
	  output << "  CPU usage : " 
		 << 100.0 * wklCPU / elapsedCPU << "% (" 
		 << wklCPU << " seconds)" << endl;
	  output << "  CPU service time : " << serviceTime << endl;
	  output << "  CPU response time : " << responseTime(numberOfCPUs,
	  		CPULoad, serviceTime) << endl;
	  output << endl;
	}
    }
  CPUTime = unaccountedUserCPU + unaccountedSystemCPU ;
  if (!csvOutput)
    {
      output << "Unaccounted CPU: " << endl;
      output << "   CPU usage : "
	     << 100.0 * CPUTime / elapsedCPU
	     << "% ("
	     << CPUTime
	     << " seconds)" << endl;
    }
  else
    {
      output << "Unaccounted; Unaccounted;"
	     << CPUTime << ";"
	     << 100.0 * CPUTime / elapsedCPU << ";"
	     << CPUTime << ";0"
	     << endl;
      
    }
}
