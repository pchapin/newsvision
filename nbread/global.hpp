/****************************************************************************
FILE          : global.h
LAST REVISION : 2005-12-30
SUBJECT       : Declarations of global variables.
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

#ifndef GLOBAL_H
#define GLOBAL_H

// #define ON_NETWORK
//
// Define the symbol above if the source is being compiled on the VTC network.

#include "nbobject.hpp"
#include "str.hpp"

// The NBread vesion number.
extern spica::String *Version_Number;
extern int          Major_Version;
extern int          Minor_Version;

// The configuration version is <= the program version. It represents the last
// time the format of the configuration information changed.
//
extern int Configuration_Version;

// This points at the object currently displayed in the topic window.
extern NB_Topic *Current_Topic;

// This points at the object current displayed in the notice window.
extern NB_Notice *Current_Notice;

// The user's full name as entered into the configuration dialog.
extern spica::String *Full_Name;

// The user's email address as entered into the configuration dialog.
extern spica::String *Email_Address;

#if eOPSYS != eWIN32
#error Class Global requires the Win32 operating system!
#endif

// This class handles Win32 specific stuff. It contains a bunch of globals
// as static members.
//
class Global {

  private:
    // The instance handle of the current process.
    static HINSTANCE Instance;
    static bool      Instance_Defined;

    // The command line given to the current process.
    static PSTR      CommandLine;
    static bool      CommandLine_Defined;

    // The command "show" parameter given to the current process.
    static int       CommandShow;
    static bool      CommandShow_Defined;

  public:

    // The functions below allow the various global values to be set and
    // retreived. Applications should try to set up all this information
    // as early as possible, typically just inside the WinMain()
    // function. These three functions save the parameters to WinMain()
    // for use by other parts of the program.

    static void Set_Instance(HINSTANCE);
    static HINSTANCE Get_Instance();

    static void Set_CommandLine(PSTR);
    static PSTR Get_CommandLine();

    static void Set_CommandShow(int);
    static int  Get_CommandShow();
};

#endif

