/*
 * FILE : surveyor.cc
 * SYSTEM : RunQ (Runq/surveyor/surveyor.cc)
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

#include <ctype.h>
#include <syslog.h>

#include "surveyor.h"
#include "util.h"

static bool verbose = false;

WatchValues*
Surveyor::watchProcess (const char name[], const char args[])
{
    for (auto & regExps : watchValues)
        if (regexec (&regExps.name.preg, name, 0, 0, 0) == 0
            && (strlen (regExps.args.expression) == 0 || regexec (&regExps.args.preg, args, 0, 0, 0) == 0)
            )
            return regExps;

    return 0;
}

static char*
formatUserMessage (vector<Message> message)
{
    static char theMessage[4096];

    time_t now = time (0);
    char tempString[256];

    theMessage[0] = '\0';

    for (auto const & messageIter : message) {
        switch (messageIter.type) {
        case Message::TEXT:
            strcat (theMessage, messageIter.text);
            break;
        case Message::DATE:
            strftime (tempString, sizeof(tempString), " %Y%m%d ",
                      localtime (&now));
            strcat (theMessage, tempString);
            break;
        case Message::TIME:
            strftime (tempString, sizeof(tempString), " %Y%m%d ",
                      localtime (&now));
            strcat (theMessage, tempString);
            break;
        }
    }

    return theMessage;
}


void
Surveyor::watchProcesses (void) throw (RunQError)
{
    ProcessList previousProcesses;

    syslog (LOG_INFO, "I started");

    sample (false);

    for (;;) { // Ever
        sleep (60);
        if (verbose)
            syslog (LOG_DEBUG, "Mark");
        previousProcesses = dynamicData.processList;
        dynamicData.processList.clear ();
        sample (false);

        for (auto const & iter : previousProcesses) {
            ProcessData* lastImage;
            WatchValues* watch;

            watch = watchProcess (iter.second.name, iter.second.args);
            if (watch != 0) {
                lastImage = dynamicData.processList.findProcess (iter.first.PID);
                if (lastImage != 0) {
                    double previousCPU, currentCPU;

                    previousCPU = iter->second.userTime + iter->second.systemTime;
                    currentCPU = lastImage->userTime + lastImage->systemTime;
                    if (watch->CPUUsage > 0 &&
                        currentCPU - previousCPU < watch->CPUUsage) {
                        syslog (LOG_WARNING,
                                "%s %s [%d] has used only %f CPU seconds < %f",
                                formatUserMessage (watch->message),
                                iter.second.name,
                                iter.first.PID,
                                currentCPU - previousCPU,
                                watch->CPUUsage);
                    } else if (watch->CPUUsage < 0
                               && currentCPU - previousCPU > watch->CPUUsage) {
                        syslog (LOG_WARNING,
                                "%s %s [%d] has used %f > %f",
                                formatUserMessage (watch->message),
                                iter.second.name,
                                iter.first.PID,
                                currentCPU - previousCPU,
                                -watch->CPUUsage);
                    }
                } else {
                    syslog (LOG_ALERT,
                            "%s %s [%d] has died",
                            formatUserMessage (watch->message),
                            iter.second.name,
                            iter.first.PID);
                }
            }
        }
    }
}


int
main (int argc, char** argv)
{
    try {
        Surveyor theSurveyor;
        int option;

        openlog ("SURVEYOR", 0, LOG_USER);

        cout.flags (ios::fixed);
        runQLogo (cerr);

        while ((option = getopt (argc, argv, "v")) != EOF) {
            switch (option) {
            case 'v':
                verbose = true;
                break;
            default:
                cerr << "getopt err" << endl;
                break;
            }
        }
        if (argc - optind != 1) {
            cerr << "usage: surveyor [-v] watchlist" << endl;
            return -1;
        }

        extern int yymain (const char[], Surveyor*);
        if (yymain (argv[optind], &theSurveyor)) {
            cerr << "*** ERROR: Failed to parse " << argv[optind] << endl;
            return -1;
        }

        for (auto & iter = theSurveyor.watchValues) {
            regcomp (&iter.name.preg, iter.name.expression, REG_NOSUB);
            if (strlen (iter.args.expression) > 0) {
                regcomp (&iter.args.preg, iter.args.expression, REG_NOSUB);
                if (verbose)
                    fprintf (stderr, "scanning %s + %s %s %f",
                             iter.name.expression,
                             iter.args.expression,
                             (iter.CPUUsage < 0) ? "<" : ">",
                             fabs (iter.CPUUsage));
            } else if (verbose)
                fprintf (stderr, "scanning %s %s %f",
                         iter.name.expression,
                         (iter.CPUUsage < 0) ? "<" : ">",
                         fabs (iter.CPUUsage));
            fprintf (stderr, " {");
            for (auto & messageIter : iter.message) {
                switch (messageIter.type) {
                case Message::TEXT:
                    fprintf (stderr, messageIter.text);
                    break;
                case Message::DATE:
                    fprintf (stderr, " <DATE> ");
                    break;
                case Message::TIME:
                    fprintf (stderr, " <TIME> ");
                    break;
                }
            }
            fprintf (stderr, "}\n");
        }

        theSurveyor.watchProcesses ();

        closelog ();
        return EXIT_SUCCESS;

    } catch (RunQError & error)      {
        switch (error.error) {
        case RunQError::LicenseError:
            cerr << endl
                 << "There is a license error, please check your installation."
                 << endl
                 << "If your installation is ok, contact RC&S for assitance."
                 << endl;
            break;
        default:
            cerr << endl
                 << "*** ERROR: Unhandled RunQ exception ("
                 << error.error
                 << "), aborting ..."
                 << endl;
            if (error.info != 0)
                cerr << "*** Additional info: "
                     << error.info
                     << endl;
        }
        exit (-1);
    }
}
