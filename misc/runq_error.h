#ifndef RUNQ_ERROR_H
#define RUNQ_ERROR_H

/******************************************************************************
 * FILE : runq_error.h
 * SYSTEM : RunQ (RunQ/misc/runq_error.h)
 *
 * AUTHOR : Serge Robyns mailto:serge.robyns@rc-s.be
 * COPYRIGHT : (C) 2000 Serge Robyns
 * 
 * CREATED : 03 jan 2000
 * VERSION : 1.00 (18-mar-2000)
 *
 * DESCRIPTION:
 *   This header contains the exceptions that can be thrown in RunQ
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

class RunQError
{
public:
  enum ErrorCode
  {
    NoError = 0,
    GenericError,
    InternalError,
    NotSupported,
    InvalidFile,
    InvalidVersion,
    FileNotFound,
    NoProcDirectory,
    FileError,
    OptionError
  };
  enum ErrorCode error;
  char const * info;

  RunQError(enum ErrorCode errorCode )
    { error = errorCode;
      info = 0;
    };
  RunQError(enum ErrorCode errorCode, const char text[] ) 
    { error = errorCode;
      info = text;
    };
};

#endif // RUNQ_ERROR_H
