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
#include <cmath>

using std::endl;


inline static double
fac (const double n)
{
    auto result = 1.0;
    for (auto i = 1.0; i <= n; i++)
        result *= i;
    return result;
}


inline static double
ErlangC (const int numOfCPUs, const double utilisation)
{
    auto adjustedUtilization = utilisation * numOfCPUs;
    auto ratio = pow (adjustedUtilization, numOfCPUs) / fac (numOfCPUs);
    auto sums = 1.0;
    for (auto k = 1; k < numOfCPUs; k++)
        sums += pow (adjustedUtilization, k) / fac (k);
    return ratio / (ratio + (1.0 - utilisation) * sums);
}


double
Model::responseTime (const int numOfCPUs, const double utilisation, const double serviceTime)
{
    if (numOfCPUs == 1)
        return serviceTime / (1 - utilisation);
    else
        return serviceTime + (ErlangC (numOfCPUs, utilisation) * serviceTime) / (numOfCPUs * (1 - utilisation));
}


void
Model::simulate (std::ostream & output, const bool csvOutput)
{
    auto totalCPU = 0.0;
    auto elapsedCPU = 0.0;
    auto elapsedSeconds = 0.0;
    auto CPULoad = 0.0;
    auto CPUTime = 0.0;
    auto serviceTime = 0.0;

    for (auto const & workLoad : workLoads) {
        for (auto const & processGroups : workLoad.processGroups) {
            totalCPU += processGroups.userCPU + processGroups.systemCPU;
        }
    }

    totalCPU = userCPU + niceCPU + systemCPU;
    elapsedCPU = totalCPU + idleCPU + waitCPU;
    if (elapsedCPU == 0) {
        elapsedCPU = 1;
        elapsedSeconds = 1;
    }
    elapsedSeconds = elapsedCPU / numberOfCPUs;
    CPULoad = totalCPU / elapsedCPU;

    if (!csvOutput) {
        output << "Elapsed CPU: " << elapsedCPU << " seconds" << endl;
        output << "Elapsed Time: " << elapsedSeconds << " seconds" << endl;
        output << "Total CPU: " << totalCPU << " seconds" << endl;
        output << "CPU Load: " << 100.0 * CPULoad << "%" << endl;
        output << "Capture Ratio: "
               << 100.0 * (1.0 -
                    ((unaccountedUserCPU + unaccountedSystemCPU) / totalCPU)
                    )
               << "%" << endl;
        output << endl;
    } else {
        output << "WorkLoad;ProcessGroup;CPUSeconds;UserSeconds;SystemSeconds;BytesRead;BytesWritten;CPULoad;ServiceTime;ResponseTime" << endl;
    }

    for (auto const & workLoad : workLoads) {
        double wklCPU = 0;
        auto wklProcessCount = 0;
        if (!csvOutput)
            output << "Workload : " << workLoad.name << endl;
        for (auto const & processGroup : workLoad.processGroups) {
            CPUTime = processGroup.userCPU + processGroup.systemCPU;
            wklCPU += CPUTime;
            wklProcessCount += processGroup.activeProcessCount;
            if (processGroup.activeProcessCount > 1)
                serviceTime = CPUTime
                              / ((double) processGroup.activeProcessCount * elapsedSeconds);
            else
                serviceTime = CPUTime / (double)elapsedSeconds;
            if (!csvOutput) {
                output << "  Process Group: " << processGroup.name << endl;
                output << "    CPU usage : "
                       << 100.0 * CPUTime / elapsedCPU
                       << "% ("
                       << CPUTime
                       << " seconds ("
                       << processGroup.userCPU << " user + "
                       << processGroup.systemCPU << " system))" << endl;
                output << "    Bytes read : " << processGroup.readBytes
                       << " bytes written: " << processGroup.writtenBytes << endl;
                output << "    CPU service time : " << serviceTime
                       << "    Response time : " << responseTime (numberOfCPUs, CPULoad, serviceTime)
                       << endl;
            } else {
                output << workLoad.name << ";"
                       << processGroup.name << ";"
                       << CPUTime << ";"
                       << processGroup.userCPU << ";"
                       << processGroup.systemCPU << ";"
                       << processGroup.readBytes << ";"
                       << processGroup.writtenBytes << ";"
                       << 100.0 * CPUTime / elapsedCPU << ";"
                       << serviceTime << ";"
                       << responseTime (numberOfCPUs, CPULoad, serviceTime)
                       << endl;
            }
        }
        if (!csvOutput) {
            if (wklProcessCount > 1)
                serviceTime = wklCPU / ((double) wklProcessCount * elapsedSeconds);
            else
                serviceTime = wklCPU / elapsedSeconds;
            output << "  CPU usage : "
                   << 100.0 * wklCPU / elapsedCPU << "% ("
                   << wklCPU << " seconds)" << endl;
            output << "  CPU service time : " << serviceTime << endl;
            output << "  CPU response time : " << responseTime (numberOfCPUs,
                                                                CPULoad, serviceTime) << endl;
            output << endl;
        }
    }
    CPUTime = unaccountedUserCPU + unaccountedSystemCPU;
    if (!csvOutput) {
        output << "Unaccounted CPU: " << endl;
        output << "   CPU usage : "
               << 100.0 * CPUTime / elapsedCPU
               << "% ("
               << CPUTime
               << " seconds)" << endl;
    } else {
        output << "Unaccounted; Unaccounted;"
               << CPUTime << ";"
               << 100.0 * CPUTime / elapsedCPU << ";"
               << CPUTime << ";0"
               << endl;

    }
}
