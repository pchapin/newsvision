/****************************************************************************
FILE          : history.h
LAST REVISION : January 2000
SUBJECT       : 
PROGRAMMER    : (C) Copyright 2000 by VTC Computer Club


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

#ifndef HISTORY_H
#define HISTORY_H

#include "str.hpp"

class History {
  private:

    // I'm using the technique of putting the implementation totally in the
    //   .cpp file so as to avoid bring in lots of otherwise needless
    //   declarations into this file. The implementation uses a lot of STL
    //   stuff.
    //
    struct Implementation;

    Implementation *Imp;
      // Points at the meat of the implementation.

    mutable bool Do_Write;
      // =true if the history should be written back to disk on destruction.

  public:
    History();
      // Read the history file.

   ~History();
      // Write the updated history file.

    void Inhibit_Write() const;
      // Invoke this function to prevent the history from being written out
      //   during destruction. This feature is used by programs that want
      //   to consult the history but that don't want to modify it.

    bool Has_Read(const spica::String &) const;
      // Returns "true" if the user has read the notice with the give path
      //   (full path required -- including drive specifier).

    void Mark_Read(const spica::String &);
      // Mark the given notice path (full path required) as a read notice. If
      //   the notice has already been read, there is no effect.
};

#endif

