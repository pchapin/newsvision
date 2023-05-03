/****************************************************************************
FILE          : nbnotify.cpp
LAST REVISION : 2005-12-30
SUBJECT       : Noticeboard notification program.
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

#include <iomanip>
#include <cstdlib>
#include <strstream>

using namespace std;

#include <windows.h>

#include "history.hpp"
#include "str.hpp"

LRESULT CALLBACK Notify_Procedure(HWND, UINT, WPARAM, LPARAM);

const char * const MAILBOX_DIRECTORY = "F:\\PMAIL";
  // The location for unread mail (assumed to be the same for all users).

const char * const Notify_ClassName = "NBNotification_Class";
  // Used in two places, defined only here.

char *Results;
  // This holds the full text to be displayed in the window.


//
// Scan_Mail_Directory
//
// The following function looks for unread mail in the given directory. It returns
//   a count of such notices.
//
static int Scan_Mail_Directory(const spica::String &Mail_Directory)
  {
    spica::String   WildCard_Name;
    WIN32_FIND_DATA Scan_Information;
    HANDLE          Search_Handle;
    int             Counter = 0;

    // Scan for the mail files. For now, assume a file is a mail file iff it
    //   matches *.CNM.
    //
    WildCard_Name = Mail_Directory;
    WildCard_Name.append("\\*.CNM");

    Search_Handle = FindFirstFile(static_cast<const char *>(WildCard_Name), &Scan_Information);
    if (Search_Handle == INVALID_HANDLE_VALUE) return 0;

    // Process the first file and then the rest.
    do {
      spica::String Entity_Name(Mail_Directory);
      Entity_Name.append("\\");
      Entity_Name.append(Scan_Information.cFileName);

      // Let's verify that this is a regular file first...
      if ((Scan_Information.dwFileAttributes &  FILE_ATTRIBUTE_ARCHIVE) ||
          (Scan_Information.dwFileAttributes == 0 )) {
        Counter++;
      }
    } while (FindNextFile(Search_Handle, &Scan_Information));

    // Close down the search handle.
    FindClose(Search_Handle);

    return Counter;
  }


//
// Scan_Directory
//
// The following function looks for unread notices in the given directory. It returns
//   a count of such notices.
//
static int Scan_Directory(const spica::String &Notice_Directory, const History &History_Database)
  {
    spica::String   WildCard_Name;
    WIN32_FIND_DATA Scan_Information;
    HANDLE          Search_Handle;
    int             Counter = 0;

    // Scan for the notice files. For now, assume a file is a notice file iff it
    //   matches *.CNB.
    //
    WildCard_Name = Notice_Directory;
    WildCard_Name.append("\\*.CNB");

    Search_Handle = FindFirstFile(static_cast<const char *>(WildCard_Name), &Scan_Information);
    if (Search_Handle == INVALID_HANDLE_VALUE) return 0;

    // Process the first file and then the rest.
    do {
      spica::String Entity_Name(Notice_Directory);
      Entity_Name.append("\\");
      Entity_Name.append(Scan_Information.cFileName);

      // Let's verify that this is a regular file first...
      if ((Scan_Information.dwFileAttributes &  FILE_ATTRIBUTE_ARCHIVE) ||
          (Scan_Information.dwFileAttributes == 0 )) {

        // Have we read this one?
        if (!History_Database.Has_Read(Entity_Name)) Counter++;
      }
    } while (FindNextFile(Search_Handle, &Scan_Information));

    // Close down the search handle.
    FindClose(Search_Handle);

    return Counter;
  }


//
// Display_Results
//
// This function displays the results of the check to the user. It uses the GUI
//   to do this.
//
void Display_Results(HINSTANCE Instance, int Command_Show)
  {
    WNDCLASSEX Notify_Class;
    MSG        Message;

    // Register the necessary window class.
    Notify_Class.cbSize        = sizeof(Notify_Class);
    Notify_Class.style         = CS_HREDRAW | CS_VREDRAW;
    Notify_Class.lpfnWndProc   = Notify_Procedure;
    Notify_Class.cbClsExtra    = 0;
    Notify_Class.cbWndExtra    = 0;
    Notify_Class.hInstance     = Instance;
    Notify_Class.hIcon         = LoadIcon(0, IDI_APPLICATION);
    Notify_Class.hCursor       = LoadCursor(0, IDC_ARROW);
    Notify_Class.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    Notify_Class.lpszMenuName  = 0;
    Notify_Class.lpszClassName = Notify_ClassName;
    Notify_Class.hIconSm       = LoadIcon(0, IDI_APPLICATION);

    if (RegisterClassEx(&Notify_Class) == 0)
      throw "Can't register the window class";

    // Find out how large the display is.
    RECT Desktop;

    if (!SystemParametersInfo(SPI_GETWORKAREA, 0, &Desktop, 0))
      throw "Can't get the desktop size";

    // Compute window coordinates.
    int Top    =   Desktop.bottom/4;
    int Left   =   Desktop.right/4;
    int Bottom = 3*Desktop.bottom/4;
    int Right  = 3*Desktop.right/4;

    // Create the window.
    HWND Notify_Window = CreateWindow(
      Notify_ClassName,
      "You Have Unread Notices!",
      WS_OVERLAPPED | WS_CLIPCHILDREN | WS_SYSMENU,
      Left, Top, Right - Left, Bottom - Top,
      0, // Parent window handle.
      0, // Menu handle.
      Instance,
      0  // Creation parameters
    );
    if (Notify_Window == 0) {
      DWORD      Error_Code = GetLastError();
      ostrstream Formatter;

      Formatter << "Can't create the notification window. Error code = " << Error_Code << ends;
      char *Raw_Text = Formatter.str();
      throw Raw_Text;
        // This is a memory leak. Raw_Text is never deleted.
    }

    // Get the window on screen!
    ShowWindow(Notify_Window, Command_Show);
    UpdateWindow(Notify_Window);

    // The message loop.
    while (GetMessage(&Message, 0, 0, 0)) {
      TranslateMessage(&Message);
      DispatchMessage(&Message);
    }

  }


//
// The Main Program
//
int WINAPI WinMain(
  HINSTANCE Instance,
  HINSTANCE /* Previous_Instance */,
  LPSTR     /* Command_Line */,
  int       Command_Show
  )
  {
    try {
      // Change class History so that its constructor takes the name of the history file.
      const History History_Database;

      // Don't bother wasting time writing out the (unchanged) history database to
      //   disk during destruction.
      //
      History_Database.Inhibit_Write();

      // Now scan the desired noticeboards looking for unread messages.
      char *Raw_Environment = getenv("NB");
      if (Raw_Environment == 0) return 0;

      spica::String Notice_Root(Raw_Environment);
      spica::String Notice_Directory;
      spica::String Mail_Directory;
      ostrstream Formatter;

      Mail_Directory = MAILBOX_DIRECTORY;
      int Unread_Email = Scan_Mail_Directory(Mail_Directory);

      Notice_Directory = Notice_Root;
      Notice_Directory.append("\\everyone");
      int Unread_Count1 = Scan_Directory(Notice_Directory, History_Database);

      Notice_Directory = Notice_Root;
      Notice_Directory.append("\\lstfnd");
      int Unread_Count2 = Scan_Directory(Notice_Directory, History_Database);

      Notice_Directory = Notice_Root;
      Notice_Directory.append("\\meetings");
      int Unread_Count3 = Scan_Directory(Notice_Directory, History_Database);

      Notice_Directory = Notice_Root;
      Notice_Directory.append("\\rides");
      int Unread_Count4 = Scan_Directory(Notice_Directory, History_Database);

      Notice_Directory = Notice_Root;
      Notice_Directory.append("\\classifi");
      int Unread_Count5 = Scan_Directory(Notice_Directory, History_Database);

      if (Unread_Email  == 0 && Unread_Count1 == 0 &&
          Unread_Count2 == 0 && Unread_Count3 == 0 &&
          Unread_Count4 == 0 && Unread_Count5 == 0)
        return 0;

      if (Unread_Email != 0)
        Formatter << setw(3) << Unread_Email  << " UNREAD: " << "E-Mail Messages" << "\n" << "\n";
      if (Unread_Count1 != 0)
        Formatter << setw(3) << Unread_Count1 << " UNREAD: " << "Announcements for Everyone" << "\n";
      if (Unread_Count2 != 0)
        Formatter << setw(3) << Unread_Count2 << " UNREAD: " << "Lost and Found" << "\n";
      if (Unread_Count3 != 0)
        Formatter << setw(3) << Unread_Count3 << " UNREAD: " << "Meetings" << "\n";
      if (Unread_Count4 != 0)
        Formatter << setw(3) << Unread_Count4 << " UNREAD: " << "Rides" << "\n";
      if (Unread_Count5 != 0)
        Formatter << setw(3) << Unread_Count5 << " UNREAD: " << "Classifieds" << "\n";

      Formatter << ends;

      Results = Formatter.str();
        // This is a memory leak. Results is never deleted.

      Display_Results(Instance, Command_Show);
    }
    catch (char *Message) {
      MessageBox(0, Message, "Exception!", MB_ICONEXCLAMATION);
    }
    catch (...) {
      MessageBox(0, "Unknown exception occured", "Exception!", MB_ICONEXCLAMATION);
    }

    return 0;
  }

//
// Notify_Procedure
//
// This window procedure handles the display of the juicy information.
//
LRESULT CALLBACK Notify_Procedure(HWND Notify_Handle, UINT Message, WPARAM wParam, LPARAM lParam)
  {
    HDC         Context_Handle;
    PAINTSTRUCT Painter;

    static HWND Skip_Button;

    switch (Message) {

      case WM_CREATE: {
          TEXTMETRIC Text_Metrics;
          HDC        Device_Context = GetDC(Notify_Handle);
          RECT       Client_Rect;

          // How big is the system fixed font?
          SelectObject(Device_Context, GetStockObject(SYSTEM_FIXED_FONT));
          GetTextMetrics(Device_Context, &Text_Metrics);
          int Char_X = Text_Metrics.tmAveCharWidth;
          int Char_Y = Text_Metrics.tmHeight + Text_Metrics.tmExternalLeading;
          ReleaseDC(Notify_Handle, Device_Context);

          // How big is the client area?
          GetClientRect(Notify_Handle, &Client_Rect);

          // Now create the button.
          int Width  = 10*Char_X;
          int Height = 7*Char_Y/4;

          Skip_Button = CreateWindow(
            "button",
            "OK",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            (Client_Rect.right - Width)/2, Client_Rect.bottom - 3*Height/2, Width, Height,
            Notify_Handle,
            reinterpret_cast<HMENU>(2),
            reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance,
            0
          );
        }
        return 0;

      // If the user clicks the button, terminate.
      case WM_COMMAND:
        DestroyWindow(Notify_Handle);
        return 0;

      case WM_PAINT: {
          RECT Client_Rect;

          Context_Handle = BeginPaint(Notify_Handle, &Painter);
          GetClientRect(Notify_Handle, &Client_Rect);
          DrawText(Context_Handle, Results, strlen(Results), &Client_Rect, 0);
          EndPaint(Notify_Handle, &Painter);
        }
        return 0;

      case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(Notify_Handle, Message, wParam, lParam);
  }

