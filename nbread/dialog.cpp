/****************************************************************************
FILE          : dialog.cpp
LAST REVISION : 2005-12-29
SUBJECT       : Dialog box procedures.
PROGRAMMER    : (C) Copyright 2000 by VTC Computer Club


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

#include <ctime>
#include <iomanip>
#include <fstream>
#include <strstream>

using namespace std;

#include <windows.h>

#include "config.hpp"
#include "dialog.hpp"
#include "global.hpp"
#include "nbread.rh"
#include "str.hpp"


//
// Compute_DateHeader
//
// This function returns the current date/time in the right format for a
//   notice header.
static spica::String Compute_DateHeader()
  {
    static const char *Day_Names[] = {
      "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    static const char *Month_Names[] = {
      "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    time_t Raw_Time = time(0);
    struct tm *Cooked_Time = localtime(&Raw_Time);

    // Compute the date/time string. This uses the non-standard "EST5EDT"
    //   string for denoting the time zone. Pegasus uses the same string.
    //
    ostrstream Result;
    Result << Day_Names[Cooked_Time->tm_wday]  << ", ";
    Result << Cooked_Time->tm_mday             <<  " ";
    Result << Month_Names[Cooked_Time->tm_mon] <<  " ";
    Result << (Cooked_Time->tm_year + 1900)    <<  " ";
    Result << setw(2) << setfill('0') << Cooked_Time->tm_hour << ":";
    Result << setw(2) << setfill('0') << Cooked_Time->tm_min  << ":";
    Result << setw(2) << setfill('0') << Cooked_Time->tm_sec  << " ";
    Result << "EST5EDT";
    Result << ends;

    // This annoying stuff is because I don't really have standard strings
    //   and ostringstream.
    //
    char *Result_Buffer = Result.str();
    spica::String Return_Value(Result_Buffer);
    delete [] Result_Buffer;
    return Return_Value;
  }


//
// Config_Dialog
//
// This dialog procedure handles the configuration dialog box. It also performs the
//   operations necessary to update the configuration both in the config files and in the
//   active program. (That is it "applies" the changes and "saves" the changes).
//
BOOL CALLBACK Config_Dialog(HWND Config_Handle, UINT Message, WPARAM wParam, LPARAM)
  {
    char Buffer[128+1];

    switch (Message) {
      case WM_INITDIALOG: {
          SetDlgItemText(Config_Handle, CONFIG_NAME, *Full_Name);
          SetDlgItemText(Config_Handle, CONFIG_ADDRESS, *Email_Address);
        }
        return TRUE;

      case WM_COMMAND:
        switch (LOWORD(wParam)) {

          // The use wants to reconfigure.
          case CONFIG_OK: {
              GetDlgItemText(Config_Handle, CONFIG_NAME, Buffer, 128+1);
              *Full_Name = Buffer;
              GetDlgItemText(Config_Handle, CONFIG_ADDRESS, Buffer, 128+1);
              *Email_Address = Buffer;

              // Verify that they entered good stuff. Eventually we might want this to
              //   make more extensive sanity checks.
              if (Full_Name->length() == 0 || Email_Address->length() == 0) {
                MessageBox(Config_Handle, "You must enter a valid name and email address!", "Error", MB_ICONEXCLAMATION);
              }
              else {
                // Update the config files.
                spica::register_parameter("Full_Name", *Full_Name, true);
                spica::register_parameter("Email_Address", *Email_Address, true);
                spica::write_config_file();

                // We're done.
                EndDialog(Config_Handle, 0);
              }
            }
            return TRUE;

          // The user gives up. Oh well.
          case CONFIG_CANCEL:

            // Don't let the user cancel without entering interesting stuff!
            if (Full_Name->length() == 0 || Email_Address->length() == 0) {
              MessageBox(Config_Handle, "You must enter a valid name and email address!", "Error", MB_ICONEXCLAMATION);
            }
            else {
              EndDialog(Config_Handle, 0);
            }
            return TRUE;
        }
        break;

    }
    return FALSE;
  }


//
// Post_Dialog
//
// This dialog procedure handles the "post notice" dialog. This procedure
//   should have exception handling in it -- especially when (if) we write our
//   own editing control.
//
BOOL CALLBACK Post_Dialog(HWND Dialog_Handle, UINT Message, WPARAM wParam, LPARAM)
  {
    switch (Message) {
      case WM_INITDIALOG: {
          spica::String Topic_Description = "Post To: ";
          Topic_Description.append(Current_Topic->Description());
          SetWindowText(Dialog_Handle, static_cast<const char *>(Topic_Description));
        }
        return TRUE;

      case WM_COMMAND:
        switch (LOWORD(wParam)) {

          // The use wants to post a notice. Cool!
          case POST_OK: {
              spica::String File_Name = Current_Topic->New_NoticePath();
              ofstream    Notice_File(File_Name);

              // If we can't open the output file, that is bad.
              if (!Notice_File) {
                MessageBox(Dialog_Handle, "Can't open output file!\rAborting post", "Error", MB_ICONEXCLAMATION);
              }
              else {

                // Several of these items need to come from the configuration database.
                Notice_File << "From: " << Full_Name << " <" << Email_Address << ">" << endl;
                Notice_File << "Date: " << Compute_DateHeader()             << endl;
                Notice_File << "Organization: Vermont Technical College"    << endl;
                Notice_File << "MIME-Version: 1.0"                          << endl;
                Notice_File << "Content-type: text/plain; charset=US-ASCII" << endl;
                Notice_File << "Priority: normal"                           << endl;
                Notice_File << "X-mailer: nbread (v" << Version_Number << ")" << endl;

                HWND  Subject_Handle = GetDlgItem(Dialog_Handle, POST_SUBJECT);
                int   Subject_Size   = GetWindowTextLength(Subject_Handle);
                char *Subject_Text   = new char[Subject_Size + 1];
                GetWindowText(Subject_Handle, Subject_Text, Subject_Size + 1);
                Notice_File << "Subject: " << Subject_Text << endl;
                delete [] Subject_Text;

                HWND  Body_Handle = GetDlgItem(Dialog_Handle, POST_BODY);
                int   Body_Size   = GetWindowTextLength(Body_Handle);
                char *Body_Text   = new char[Body_Size + 1];
                GetWindowText(Body_Handle, Body_Text, Body_Size + 1);
                Notice_File << "\n" << Body_Text << endl;
                delete [] Body_Text;
              }
              EndDialog(Dialog_Handle, 0);
            }
            return TRUE;

          // The user gives up. Oh well.
          case POST_CANCEL:
            EndDialog(Dialog_Handle, 0);
            return TRUE;
        }
        break;

    }
    return FALSE;
  }
