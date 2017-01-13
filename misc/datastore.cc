/*
 * FILE : datastore.cc
 * SYSTEM : RunQ (RunQ/datafile/datastore.cc)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 * 
 * CREATED : 06 jan 2000
 * VERSION : 1.00 (18-mar-2000)
 *
 * DESCRIPTION:
 *   This file provides the datastore.
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
#include "datastore.h"

DataStore::DataStore(const char name[], const std::_Ios_Openmode mode,
		     const RunQFileType::RunQFileTypes aFileType)
   throw(RunQError) : std::fstream(name, mode)
{
  checkFileType(mode, aFileType);
}

void DataStore::open(const char name[], const std::_Ios_Openmode mode,
		     const RunQFileType::RunQFileTypes aFileType)
  throw(RunQError)
{
  std::fstream::open(name, mode);
  if ( ! is_open() )
    throw RunQError(RunQError::FileNotFound, strdup(name));
  checkFileType(mode, aFileType);
}

void DataStore::get(void *data, const size_t size) throw(RunQError)
{
  this->read((char*)data, size);
  if ( ! *this )
    throw RunQError(RunQError::FileError, "DataStore::get");
}

void DataStore::put(void *data, const size_t size) throw(RunQError)
{
  this->write((char*)data, size);
  if ( ! *this )
    throw RunQError(RunQError::FileError, "DataStore::put");
}

void DataStore::checkFileType(const std::_Ios_Openmode mode,
			      const RunQFileType::RunQFileTypes aFileType)
  throw(RunQError)
{
  MagicID aMagicID(aFileType);

  if (mode & ios_base::out)
    {
      magicID = aMagicID;
      switch (aFileType)
	{
	case RunQFileType::RawData:
	  {
	    this->write((char*)&magicID, sizeof(MagicID));
	  }
	  break;	  
	default:
	  break;
	}
    }
  else if (mode & ios_base::in)
    {
      switch (aFileType)
	{
	case RunQFileType::RawData:
	  {
	    this->read((char*)&magicID, sizeof(MagicID));
	    if (strcmp(magicID.productName, aMagicID.productName) != 0)
	      throw RunQError(RunQError::InvalidFile);
	    if (strcmp(magicID.fileType, aMagicID.fileType) != 0)
	      throw RunQError(RunQError::InvalidFile);
	    if (strcmp(magicID.version, aMagicID.version) != 0)
	      throw RunQError(RunQError::InvalidVersion, aMagicID.version);
	  }
	  break;	  
	default:
	  break;
	}
    }
  else
    {
      throw RunQError(RunQError::InternalError);
    }
}
