/****************************************************************************
FILE          : idinfo.h
LAST REVISION : January 2000
SUBJECT       : Interface to the ID_Info class.
PROGRAMMER    : (C) Copyright 2000 by VTC Computer Club

This class contains (and manages) the information normally stored in
NB.ID. If NB.ID for a topic does not exist, or if it is incomplete, this
class will "fake" the information by perhaps checking other sources or
by using suitable defaults.

Note that the constructor for this class does *not* attempt to read
NB.ID. In keeping with the lazy evaluation philosophy of NBread, no disk
activity should be done unless necessary. However, any attempt to access
the information in an ID_Info will trigger the reading of the ID file.
Information in that file will then be cached internally to speed up all
future access.


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

#ifndef IDINFO_H
#define IDINFO_H

#include "str.hpp"

class ID_Info {
  public:
    ID_Info(const spica::String &Path) : Cache_Valid(false), ID_Path(Path) { }

    const spica::String &Short_Name();
    const spica::String &Long_Name();

  private:
    bool        Cache_Valid;   // =true when I've already read the ID file.
    spica::String ID_Path;       // Path to the directory where ID file exists.
    spica::String S_Name;        // The short name defined in the ID file.
    spica::String L_Name;        // The long name defined in the ID file.

    void Read_IDFile();
      // Reads the ID file and loads the cache with goodies.
};

#endif

