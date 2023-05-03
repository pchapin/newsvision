/****************************************************************************
FILE          : nbobject.cpp
LAST REVISION : January 2000
SUBJECT       : Supporting functions for all NB_Object classes.
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

#include "environ.hpp"
#include "nbobject.hpp"

//
// TObject_List::~TObject_List
//
TObject_List::~TObject_List()
{
  iterator Killer;
  for (Killer = begin(); Killer != end(); Killer++) {
    delete *Killer;
  }
}


//
// NObject_List::~NObject_List
//
NObject_List::~NObject_List()
{
  iterator Killer;
  for (Killer = begin(); Killer != end(); Killer++) {
    delete *Killer;
  }
}


//
// NB_Object::~NB_Object
//
NB_Object::~NB_Object()
  { return; }

