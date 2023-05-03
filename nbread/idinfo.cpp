/****************************************************************************
FILE          : idinfo.cpp
LAST REVISION : January 2000
SUBJECT       : Implementation of the ID_Info class.
PROGRAMMER    : (C) Copyright 2000 by VTC Computer Club

This file contains the code that reads and manages NB.ID.


REVISION HISTORY

+ January 22, 2000: Formally put under the GPL.


LICENSE

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANT-
ABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

Please send comments or bug reports to

     VTC^3
     c/o Peter Chapin
     Vermont Technical College
     Randolph Center, VT 05061
     VTC3-L@vtc.vsc.edu
****************************************************************************/

#include "environ.hpp"
#include <fstream>

using namespace std;

#include "idinfo.hpp"
#include "str.hpp"

//
// ID_Info::Short_Name
//
const spica::String &ID_Info::Short_Name()
{
  if (!Cache_Valid) Read_IDFile();

  return S_Name;
}


//
// ID_Info::Long_Name
//
const spica::String &ID_Info::Long_Name()
{
  if (!Cache_Valid) Read_IDFile();

  return L_Name;
}


//
// ID_Info::Read_IDFile
//
void ID_Info::Read_IDFile()
{
  // Compute the name of the ID file.
  spica::String File_Name(ID_Path);
  File_Name.append("\\NB.ID");

  // Open the file.
  ifstream In_File(static_cast<const char *>(File_Name));

  // If we can't open the file, use defaults, etc.
  if (!In_File) {
    const char *Start = static_cast<const char *>(ID_Path);
    const char *Name  = strchr(Start, '\0');

    // Locate the last component to the directory path. Use that for both long and
    //   short names.
    //
    while (Name != Start && *Name != '\\') Name--;
    if (*Name == '\\') Name++;
    S_Name = Name;
    L_Name = S_Name;
  }

  // Otherwise, process the file.
  else {

    // Read the entire file a line at a time.
    spica::String Line;
    while (In_File >> Line) {
      int Rest = Line.pos(':');
      if (Rest == 0) continue;

      spica::String Header = Line.word(1, ":");
      spica::String Value  = Line.substr(Rest + 1);

      if (Header == static_cast<spica::String>("Short name")) {
	S_Name = Value;
	if (L_Name.length() == 0) L_Name = S_Name;
      }
      if (Header == static_cast<spica::String>("Long name")) L_Name = Value;
    }
  }

  Cache_Valid = true;
}

