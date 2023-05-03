/****************************************************************************
FILE          : nbnotice.cpp
LAST REVISION : January 2000
SUBJECT       : Implementation of the NB_Notice class.
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
#include <fstream>
#include <windows.h>

using namespace std;

#include "history.hpp"
#include "nbobject.hpp"
#include "str.hpp"
#include "windebug.hpp"

//
// The definition of the static objects in class NB_Notice.
//
const int NB_Notice::Num_Objects = 3;

//
// Process_Summary
//
void NB_Notice::Process_Summary()
  {
    Tracer(4, "Processing a notice to extract its summary.");

    Subject = "UNKNOWN Subject";
    From    = "UNKNOWN Poster";
    Date    = "UNKNOWN Date";

    ifstream Posting(Notice_Path);

    // If we can't open the posting file for some strange reason, let the
    //   user know by sending back a message as the notice summary.
    //
    if (!Posting) {
      Subject = "Unable to open notice: ";
      Subject.append(Notice_Path);
      return;
    }

    int         Objects_Filled = 0;
    spica::String Line;

    while ((Objects_Filled < Num_Objects) && Posting) {
      Posting >> Line;
      spica::String First_Word = Line.word(1);

      if (First_Word == static_cast<spica::String>("Subject:")) {
        Subject = Line.subword(2);
        Objects_Filled++;
      }
      else if (First_Word == static_cast<spica::String>("From:")) {
        From = Line.subword(2);
        Objects_Filled++;
      }
      else if (First_Word == static_cast<spica::String>("Date:")) {
        Date = Line.subword(2);
        Objects_Filled++;
      }
    }

    // Clean up the "From" string a bit.
    int Angle_Bracket = From.pos('<');
    spica::String Clean_Name = From.substr(1, Angle_Bracket - 1);
    Clean_Name = Clean_Name.strip();
    Clean_Name = Clean_Name.strip('B', '"');
    From       = Clean_Name;

    // Clean up the "Date" string a bit. The format right now is:
    // Fri, 1 May 1998 16:29:35 EST5EDT
    //
    Raw_Date = Date;
    spica::String Clean_Date = Date.word(3);
    Clean_Date.append(" ");
    Clean_Date.append(Date.word(2));
    Clean_Date.append(", ");
    spica::String Raw_Time = Date.word(5);
    Clean_Date.append(Raw_Time.substr(1, 5));
    Date = Clean_Date;

    Processed = true;
  }


//
// NB_Notice::Description
//
spica::String &NB_Notice::Description()
  {
    if (!Processed) Process_Summary();
    return Subject;
  }


//
// NB_Notice::Poster_Name
//
spica::String &NB_Notice::Poster_Name()
  {
    if (!Processed) Process_Summary();
    return From;
  }


//
// NB_Notice::Date_String
//
spica::String &NB_Notice::Date_String()
  {
    if (!Processed) Process_Summary();
    return Date;
  }


//
// NB_Notice::RawDate_String
//
// This function is a copy of the entire date string that was stored in
//   the notice file. It is used during the date comparisons that are done
//   when notices are sorted by date. The Date_String() function returns
//   an abbreviated date (more suitable for display).
//
spica::String &NB_Notice::RawDate_String()
  {
    if (!Processed) Process_Summary();
    return Raw_Date;
  }


//
// NB_Notice::Mark_AsRead
//
// This function causes the notice to mark itself as read in the history
//   database.
//
void NB_Notice::Mark_AsRead(History *History_Database)
  {
    History_Database->Mark_Read(Notice_Path);
  }


//
// NB_Notice::Is_Read
//
// This function looks up this notice in the give history database and returns
//   true if it has been read there.
//
bool NB_Notice::Is_Read(History *History_Database)
  {
    return History_Database->Has_Read(Notice_Path);
  }


//
// NB_Notice::Redraw
//
void NB_Notice::Redraw(
  const HWND &Window_Handle, const HDC &Context_Handle)
  {
    RECT       The_Rectangle;
    TEXTMETRIC Text_Metrics;
    int        Char_Height;
    int        Char_Width;

    // Be sure the title bar of the window is correct.
    spica::String Title("Notice: ");
    Title.append(Subject);
    SetWindowText(Window_Handle, Title);

    // Learn how big the window is.
    SelectObject(Context_Handle, GetStockObject(OEM_FIXED_FONT));
    GetTextMetrics(Context_Handle, &Text_Metrics);
    Char_Height = Text_Metrics.tmHeight + Text_Metrics.tmExternalLeading;
    Char_Width  = Text_Metrics.tmAveCharWidth;
    GetClientRect(Window_Handle, &The_Rectangle);
    unsigned Page_Height = The_Rectangle.bottom/Char_Height;
    unsigned Page_Width  = The_Rectangle.right/Char_Width;

    // If we dont' have the text of the message yet, we need to get it.
    if (!Have_Text) {
      ifstream Posting(Notice_Path);
      Longest_Line = 0;

      // If we can't open the notice file, I guess there is no text!
      if (!Posting) {
        Notice_Text.erase(Notice_Text.begin(), Notice_Text.end());
        Have_Text = true;
        return;
      }

      spica::String Line;
      while (Posting >> Line) {
        Notice_Text.push_back(Line);
        if (Line.length() > Longest_Line) Longest_Line = Line.length();
      }
      Have_Text = true;
    }
	
    // Adjust position appropriately.
    if (Position < 0) Position = 0;
    if (Notice_Text.size() <= Page_Height) Position = 0;
    else {
      if (Position > static_cast<int>(Notice_Text.size() - Page_Height)) Position = Notice_Text.size() - Page_Height;
    }

    // Adjust hoffset appropriately.
    if (HOffset < 0) HOffset = 0;
    if (Longest_Line <= Page_Width)  HOffset = 0;
    else {
      if (HOffset > static_cast<int>(Longest_Line - Page_Width)) HOffset = Longest_Line - Page_Width;
    }

    // Now update the entire window.
    for (unsigned Row = 0; Row < Notice_Text.size() && Row < Page_Height; Row++) {
      const char *Start_Position;
      spica::String &This_Line = Notice_Text[Row+Position];

      if (This_Line.length() < HOffset) continue;
      Start_Position = static_cast<const char *>(This_Line) + HOffset;
      TextOut(Context_Handle, 0, Row * Char_Height, Start_Position, strlen(Start_Position));
    }

    // Redraw the scroll bars.
    SetScrollRange(Window_Handle, SB_VERT, 0, Notice_Text.size(), FALSE);
    SetScrollRange(Window_Handle, SB_HORZ, 0, Longest_Line, FALSE);
    SetScrollPos(Window_Handle, SB_VERT, Position, TRUE);
    SetScrollPos(Window_Handle, SB_HORZ, HOffset, TRUE);
  }


//
// NB_Notice::HScroll
//
void NB_Notice::HScroll(const HWND &Notice_Window, const WPARAM &wParam, const int &Char_Width)
  {
    RECT     The_Rectangle;
    unsigned Page;

    GetClientRect(Notice_Window, &The_Rectangle);
    Page = The_Rectangle.right/Char_Width;

    // The value of hoffset is "corrected" in the Redraw() function.
    switch (LOWORD(wParam)) {

      case SB_LINELEFT     : HOffset -= 1;    break;
      case SB_LINERIGHT    : HOffset += 1;    break;
      case SB_PAGELEFT     : HOffset -= Page; break;
      case SB_PAGERIGHT    : HOffset += Page; break;
      case SB_THUMBPOSITION: HOffset = HIWORD(wParam); break;
    }
    InvalidateRect(Notice_Window, 0, TRUE);
  }


//
// NB_Notice::VScroll
//
void NB_Notice::VScroll(const HWND &Notice_Window, const WPARAM &wParam, const int &Char_Height)
  {
    RECT     The_Rectangle;
    unsigned Page;

    GetClientRect(Notice_Window, &The_Rectangle);
    Page = The_Rectangle.bottom/Char_Height;

    // The value of position is "corrected" in the Redraw() function.
    switch (LOWORD(wParam)) {

      case SB_LINEUP       : Position -= 1;    break;
      case SB_LINEDOWN     : Position += 1;    break;
      case SB_PAGEUP       : Position -= Page; break;
      case SB_PAGEDOWN     : Position += Page; break;
      case SB_THUMBPOSITION: Position = HIWORD(wParam); break;
    }
    InvalidateRect(Notice_Window, 0, TRUE);
  }

