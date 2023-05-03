/****************************************************************************
FILE          : history.cpp
LAST REVISION : 2005-12-30
SUBJECT       : Class that manages a list of read noticeboard entries.
PROGRAMMER    : (C) Copyright 2005 by VTC Computer Club


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

#include <cctype>
#include <cstring>
#include <iostream>
#include <fstream>
#include <map>
#include <set>

using namespace std;

#include "history.hpp"
#include "str.hpp"

// This should really come from the environment or configuration file.
#ifdef ON_NETWORK
const char * const History_FileName = "f:\\nbread.hst";
#else
const char * const History_FileName = "c:\\home\\svn\\VTC\\nbread\\nbread.hst";
#endif

#if eCOMPILER == eMETROWERKS

// I need stricmp() below and this compiler does not supply it.

  // The prototype below is to prevent CodeWarrior from generating a warning. DUH!
int stricmp(const char * , const char * );
int stricmp(const char *L, const char *R)
  {
    while (*L) {
    
      // If both characters are letters, compare them without regard to case.
      if (isalpha(*L) && isalpha(*R)) {
        if (toupper(*L) < toupper(*R)) return -1;
        if (toupper(*L) > toupper(*R)) return +1;
      }
      
      // Otherwise at least one is not a letter. Treat them both as is.
      else {
        if (*L < *R) return -1;
        if (*L > *R) return +1;
      }
      
      // Advance to the next pair of characters to consider.
      L++;
      R++;
    }

    // If we got to the end of L, let's see what R is doing.
    if (*R != '\0') return -1;
    return 0;
  }

#endif

//
// The following predicate is used to make case insensitive comparisons of two
//   String objects. It uses the non-standard function stricmp().
//
class Insensitive_Compare {
  public:
    bool operator()(const spica::String &Left, const spica::String &Right) const;
};

bool Insensitive_Compare::operator()(const spica::String &Left, const spica::String &Right) const
  {
    const char *L = static_cast<const char *>(Left);
    const char *R = static_cast<const char *>(Right);

    if (_stricmp(L, R) < 0) return true;
    return false;
  }


//
// Handy typedefs for the high level STL types.
//
typedef set<spica::String, Insensitive_Compare> str_set;


//
// The implementation of class History.
//
struct History::Implementation {

  // The read notice database itself.
  str_set Database;
};


//
// History::History
//
// The constructor creates an instance of the implementation.
//
History::History() : Do_Write(true)
  {
    Imp = new Implementation;

    // Now read the history file and insert every filename we find in it
    //   into the database.
    //
    ifstream History_File(History_FileName);
    if (!History_File) return;

    // If the file opened okay, read every line.
    spica::String Line;
    while (History_File >> Line) {
      Imp->Database.insert(Line);
    }
  }


//
// History::~History
//
// The destructor cleans up the implementation.
//
History::~History()
  {
    if (Do_Write) {

      // Write out the new history file.
      ofstream History_File(History_FileName);
      if (!History_File) return;

      str_set::iterator Stepper;
      for (Stepper = Imp->Database.begin(); Stepper != Imp->Database.end(); Stepper++) {
        History_File << *Stepper << endl;
      }
    }
    delete Imp;
  }


//
// History::Inhibit_Write
//
// This function just turns off the Do_Write flag so that the history won't
//   be written to disk on destruction. I suggest doing this for all constant
//   History objects.
//
void History::Inhibit_Write() const
  {
    Do_Write = false;
  }


//
// History::Has_Read
//
// This function returns true if the given notice has been read. It is
//   basically a wrapper around the STL functions.
//
bool History::Has_Read(const spica::String &Notice_Path) const
  {
    str_set::iterator Result = Imp->Database.find(Notice_Path);

    if (Result == Imp->Database.end()) return false;
    return true;
  }


//
// History::Mark_Read
//
// This function marks a notice as read. It is basically a wrapper around
//   the STL functions.
//
void History::Mark_Read(const spica::String &Notice_Path)
  {
    Imp->Database.insert(Notice_Path);
  }

