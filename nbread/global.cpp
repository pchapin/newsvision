/****************************************************************************
FILE          : global.cpp
LAST REVISION : January 2000
SUBJECT       : Definitions of global variables.
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

#include <windows.h>
#include <assert.h>
#include "global.hpp"
#include "str.hpp"

// This file contains the defintions of the global data objects. See
//   global.h for documentation.
//

spica::String *Version_Number;
int          Major_Version  = 1;
int          Minor_Version  = 0;
int          Configuration_Version = 100;
NB_Topic    *Current_Topic  = 0;
NB_Notice   *Current_Notice = 0;
spica::String *Full_Name;
spica::String *Email_Address;

//
// Here are the definitions of the various Win32 global parameters.
//

HINSTANCE Global::Instance;
bool      Global::Instance_Defined = false;

PSTR      Global::CommandLine;
bool      Global::CommandLine_Defined = false;

int       Global::CommandShow;
bool      Global::CommandShow_Defined = false;

//
// Public Member Functions
//

void Global::Set_Instance(HINSTANCE I)
{
  assert(Instance_Defined == false);
  Instance = I;
  Instance_Defined = true;
}

HINSTANCE Global::Get_Instance()
{
  assert(Instance_Defined == true);
  return Instance;
}


void Global::Set_CommandLine(PSTR C)
{
  assert(CommandLine_Defined == false);
  CommandLine = C;
  CommandLine_Defined = true;
}

PSTR Global::Get_CommandLine()
{
  assert(CommandLine_Defined == true);
  return CommandLine;
}


void Global::Set_CommandShow(int C)
{
  assert(CommandShow_Defined == false);
  CommandShow = C;
  CommandShow_Defined = true;
}

int Global::Get_CommandShow()
{
  assert(CommandShow_Defined == true);
  return CommandShow;
}
