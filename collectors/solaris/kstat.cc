/******************************************************************************
 * FILE : linux.h
 * SYSTEM : RunQ (Runq/collectors/solaris/solaris.cpp)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@advalvas.be
 * COPYRIGHT : (C) 2000, by Robyns Consulting & Services (RC&S)
 *
 * CREATED : 14 jan 2000
 * VERSION : 0.01 (14/jan/2000)
 *
 * DESCRIPTION:
 *   This c++ file defines the superclass for RunQ's datafile
 *
 * CHANGELOG:
 *
 */

#include <cstdlib>
#include <iostream.h>
#include <dirent.h>
#include <unistd.h>
#include <kstat.h>

#define PAGE_SIZE 1
#define SOLARIS_PROCDIR "/proc"
#define CPUDYNID "cpu_stat"
#define LOADAVGID "avenrun_1min"

int
main (int argc, char**argv)
{
    kstat_ctl_t* kc;
    kstat_t*ks;
    kid_t kcid, nkcid;
    kstat_named_t* kn;
    kstat_io io_stat;
    kstat_named named_stat[255];
    int type = -1;
    int i;

    if (argc == 2)
        type = atol (argv[1]);
    kc = kstat_open ();
    do {
        nkcid = kstat_chain_update (kc);
        if (nkcid == -1) {
            cerr << "update failed" << endl;
            return -1;
        }
    } while (nkcid != 0);

    for (ks = kc->kc_chain; ks; ks = ks->ks_next) {
        if (type > 0 && ks->ks_type != type)
            continue;
        cout << ks->ks_module << "." << ks->ks_name << "@" << ks->ks_instance
             << ":" << ks->ks_class
             << endl;
        switch (ks->ks_type) {
        case KSTAT_TYPE_RAW:
            break;
        case KSTAT_TYPE_NAMED:
            kstat_read (kc, ks, named_stat);
            for (i = 0; i < ks->ks_ndata; i++) {
                cout << "\t" << named_stat[i].name << " = ";
                switch (named_stat[i].data_type) {
                case KSTAT_DATA_CHAR:
                    cout << named_stat[i].value.c << endl;
                    break;
                case KSTAT_DATA_INT32:
                    cout << named_stat[i].value.i32 << endl;
                    break;
                case KSTAT_DATA_UINT32:
                    cout << named_stat[i].value.ui32 << endl;
                    break;
                case KSTAT_DATA_INT64:
                    cout << named_stat[i].value.i64 << endl;
                    break;
                case KSTAT_DATA_UINT64:
                    cout << named_stat[i].value.ui64 << endl;
                    break;
                }
            }
            break;
        case KSTAT_TYPE_TIMER:
            break;
        case KSTAT_TYPE_INTR:
            break;
        case KSTAT_TYPE_IO:
            kstat_read (kc, ks, &io_stat);
            cout << "\t"
                 << io_stat.nread << " read "
                 << io_stat.nwritten << " written"
                 << endl;
            break;
        }
    }
    return 0;
    kstat_close (kc);
}
