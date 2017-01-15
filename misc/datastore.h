#ifndef RUNQ_DATASTORE_H
#define RUNQ_DATASTORE_H

/*
 * FILE : datastore.h
 * SYSTEM : RunQ (RunQ/datafile/datastore.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 *
 * CREATED : 06 jan 2000
 * VERSION : 1.00 (18-mar-2000)
 *
 * DESCRIPTION:
 *   This header file provides the datastore.
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

#include <string.h>
#include <fstream>

#include "runq_error.h"
#include "version.h"

#define RUNQ_FILETYPE_MAX 16
#define RUNQ_PRODUCTNAME_MAX 16
#define RUNQ_VERSION_MAX 16

#define RUNQ_RAWDATA_FILE "RAWPERFDATA"

enum class RunQFileType {
    UnTyped = 0,
    RawData
};

class MagicID {
public:
    MagicID() = default;
    MagicID(const RunQFileType aFileType)
    {
        strcpy (productName, RUNQ_PRODUCT_NAME);
        strcpy (version, RUNQ_VERSION);
        switch (aFileType) {
        case RunQFileType::RawData:
            strcpy (fileType, RUNQ_RAWDATA_FILE);
            break;
        default:
            break;
        }
    };
public:
    char productName[RUNQ_PRODUCTNAME_MAX];
    char version[RUNQ_VERSION_MAX];
    char fileType[RUNQ_FILETYPE_MAX];
};

class DataStore : public std::fstream {
public:
    DataStore() : std::fstream () {};
    DataStore(const char name[], const std::_Ios_Openmode mode, RunQFileType fileType)
    throw (RunQError);
    void
    open (const char name[], const std::_Ios_Openmode mode, const RunQFileType fileType)
    throw (RunQError);
    void
    put (void*data, const size_t size) throw (RunQError);
    void
    get (void*data, const size_t size) throw (RunQError);
private:
    void
    checkFileType (const std::_Ios_Openmode mode, const RunQFileType fileType)
    throw (RunQError);
private:
    MagicID magicID;
};

#endif // RUNQ_DATASTORE_H
