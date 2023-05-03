/****************************************************************************
FILE          : nbread.cpp
LAST REVISION : 2005-12-30
SUBJECT       : Simple noticeboard reader.
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

#include <stdlib.h>
#include <windows.h>
#include <commctrl.h>

#include "config.hpp"
#include "dialog.hpp"
#include "global.hpp"
#include "history.hpp"
#include "nbread.rh"
#include "nbobject.hpp"
#include "str.hpp"
#include "windebug.hpp"
#include "winexcept.hpp"

#ifdef ON_NETWORK
// Needed for NetWare API stuff.
#include <nwcalls.h>
#include <nwnet.h>
#endif

// It would probably be better to pick this up from the environment.
#ifdef ON_NETWORK
#define MASTER_CONFIGPATH "s:\\nb\\nbread.cfg"
#else
#define MASTER_CONFIGPATH "c:\\home\\prog\\nbread\\nbread.cfg"
#endif

LRESULT CALLBACK Frame_Procedure(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK Topic_Procedure(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK Notice_Procedure(HWND, UINT, WPARAM, LPARAM);
BOOL    CALLBACK Close_Childs(HWND, LPARAM);

//---------------------------------
//           Global Data
//---------------------------------

// These are the names of the window classes we'll need.
const char * const Frame_ClassName  = "NBread_Frame";
const char * const Topic_ClassName  = "NBread_Topic";
const char * const Notice_ClassName = "NBread_Notice";

// This points at the History object being used to manage read notices.
static History *History_Database = 0;

// This holds the handle to the image list. I apparently can't pass this
// from the frame procedure to the WM_CREATE case of the topic procedure
// via CreateMDIWindow(). Casts of HIMAGELIST to LPARAM and back
// generate an error message about an "undefined structure."
//
static HIMAGELIST Image_Handle;

//------------------------------------------------
//           Internally Linked Functions
//------------------------------------------------

//
// Get_Username
//
// The following function figures out what a user's name is by querying
// the NDS. The given buffer is assumed to be large enough to hold the
// answer. You must call NWCallsInit() before calling this function.
//
static bool Get_Username(char *Buffer)
  {
    strcpy(Buffer, "Unknown");

    #ifdef ON_NETWORK
    NWDSContextHandle   Context_Handle;   
    NWDSCCODE           Return_Code;
    char                Object_Name[MAX_DN_CHARS+1];

    Context_Handle = NWDSCreateContext();
    if (Context_Handle != ERR_CONTEXT_CREATION) {
      Return_Code = NWDSWhoAmI(Context_Handle, Object_Name);
      if (Return_Code == SUCCESSFUL)
        strcpy(Buffer, &Object_Name[3]);
        return true;
    }
    #endif

    return false;
  }


//
// Check_Configuration
//
// This function reads the configuration files and/or interacts with the
// user to insure that required configuration items are set to
// something. This function depends on bindery emulation. It should be
// updated sometime to use NDS facilities instead.
//
static void Check_Configuration()
  {
    // Read the configuration files.
    spica::read_config_files(MASTER_CONFIGPATH);

    // Do we have the required configuration items?
    string *Name    = spica::lookup_parameter("Full_Name");
    string *Address = spica::lookup_parameter("Email_Address");

    // If so, load the current options.
    if (Name != 0 && Address != 0) {
      *Full_Name     = Name->c_str();
      *Email_Address = Address->c_str();
    } 

    // Otherwise try to compute this information or, as a last ditch,
    // ask the user.
    // 
    else {
      #ifndef ON_NETWORK
      bool          Ask_User = true;
      #endif

      #ifdef ON_NETWORK
      NWCONN_HANDLE Connection;
      BYTE         *Buffer = new BYTE[128];
      NWFLAGS       More_Data;
      char          Username[MAX_DN_CHARS+1];

      bool          Ask_User = false;

      // Can we initialize the NetWare client?
      if (NWCallsInit(0, 0) != SUCCESSFUL) {
        Ask_User = true;
      }

      // Can we figure out our username?
      else if (!Get_Username(Username)) {
        Ask_User = true;
      }

      // Can we get a valid connection handle?
      else if (NWGetConnectionHandle("NIGHT", 0, &Connection, 0)) {
        Ask_User = true;
      }

      // Can we figure out our full name?
      else if (NWReadPropertyValue(Connection, Username, OT_USER, "IDENTIFICATION", 1, Buffer, &More_Data, 0)) {
        Ask_User = true;
      }

      // All of that worked. Update our internal information *and* the
      // config file.
      // 
      else {
        Full_Name     = reinterpret_cast<char *>(Buffer);
        Email_Address = Username;
        Email_Address.Append("@vtc.vsc.edu");

        spica::Register_Parameter("Full_Name", Full_Name, false);
        spica::Register_Parameter("Email_Address", Email_Address, false);
        spica::Write_ConfigFile();
      }

      delete [] Buffer;
      #endif

      // Something didn't work. Ask the user.
      if (Ask_User) {
        DialogBox(Global::Get_Instance(), MAKEINTRESOURCE(CONFIG_DIALOG), 0, Config_Dialog);
      }
    }
  }


//
// Set_Classes
//
// This function defines the various window classes the program needs.
//
static void Set_Classes()
{
  WNDCLASS    The_Class;
  
  // Define an appropriate window class for the main window.
  The_Class.style         = CS_HREDRAW | CS_VREDRAW;
  The_Class.lpfnWndProc   = Frame_Procedure;
  The_Class.cbClsExtra    = 0;
  The_Class.cbWndExtra    = 0;
  The_Class.hInstance     = Global::Get_Instance();
  The_Class.hIcon         = LoadIcon(Global::Get_Instance(), MAKEINTRESOURCE(MAIN_ICON));
  The_Class.hCursor       = LoadCursor(0, IDC_ARROW);
  The_Class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_APPWORKSPACE + 1);
  The_Class.lpszMenuName  = MAKEINTRESOURCE(MAIN_MENU);
  The_Class.lpszClassName = Frame_ClassName;
  if (RegisterClass(&The_Class) == 0)
    throw spica::Win32::API_Error("Failed to register the frame window class");

  // Now define appropriate classes for the MDI client windows.
  The_Class.style         = CS_HREDRAW | CS_VREDRAW;
  The_Class.lpfnWndProc   = Topic_Procedure;
  The_Class.cbClsExtra    = 0;
  The_Class.cbWndExtra    = 0;
  The_Class.hInstance     = Global::Get_Instance();
  The_Class.hIcon         = LoadIcon(Global::Get_Instance(), MAKEINTRESOURCE(TOPIC_ICON));
  The_Class.hCursor       = LoadCursor(0, IDC_ARROW);
  The_Class.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
  The_Class.lpszMenuName  = 0;
  The_Class.lpszClassName = Topic_ClassName;
  if (RegisterClass(&The_Class) == 0)
    throw spica::Win32::API_Error("Failed to register the topic window class");

  The_Class.style         = CS_HREDRAW | CS_VREDRAW;
  The_Class.lpfnWndProc   = Notice_Procedure;
  The_Class.cbClsExtra    = 0;
  The_Class.cbWndExtra    = 0;
  The_Class.hInstance     = Global::Get_Instance();
  The_Class.hIcon         = LoadIcon(Global::Get_Instance(), MAKEINTRESOURCE(NOTICE_ICON));
  The_Class.hCursor       = LoadCursor(0, IDC_ARROW);
  The_Class.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
  The_Class.lpszMenuName  = 0;
  The_Class.lpszClassName = Notice_ClassName;
  if (RegisterClass(&The_Class) == 0)
    throw spica::Win32::API_Error("Failed to register the notice window class");

  Tracer(2, "Window classes registered");
}


//
// Create_SubtopicLV
//
// This function creates a listview control within the topic window. The
// listview will contain all the subtopics. This function returns the
// handle of the list view control.
//
static HWND Create_SubtopicLV(HINSTANCE Instance, HWND Topic_Window)
  {
    RECT      Topic_Rect;
    HWND      List_Window;
    LV_COLUMN Col;

    GetClientRect(Topic_Window, &Topic_Rect);
    List_Window = CreateWindowEx(
      0,
      WC_LISTVIEW,
      "", 
      WS_CHILD | WS_VISIBLE | WS_VSCROLL |  LVS_REPORT,
      0, Topic_Rect.bottom/2, Topic_Rect.right, Topic_Rect.bottom/2,
      Topic_Window,
      reinterpret_cast<HMENU>(1),
      Instance,
      0
    );
    if (List_Window == 0)
      throw spica::Win32::API_Error("Can't create the subtopic listview");

    Col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    Col.fmt      = LVCFMT_LEFT;
    Col.cx       = Topic_Rect.right;
    Col.pszText  = "Subtopics";
    Col.iSubItem = 0;

    int Err;
    Err = ListView_InsertColumn(List_Window, 0, &Col);
    if (Err == -1)
      throw spica::Win32::API_Error("Can't insert 'Topic' column into the subtopic listview");

    Tracer(3, "Finished creating the subtopic listview.");
    return List_Window;
  }


//
// Create_NoticeLV
//
// This function creates a listview control within the topic window. The
// listview will contain all the notices. This function returns the
// handle of the list view control.
//
static HWND Create_NoticeLV(HINSTANCE Instance, HWND Topic_Window)
  {
    RECT      Topic_Rect;
    LV_COLUMN Col;
    HWND      List_Window;

    GetClientRect(Topic_Window, &Topic_Rect);
    List_Window = CreateWindowEx(
      0,
      WC_LISTVIEW,
      "", 
      WS_CHILD | WS_VISIBLE | WS_VSCROLL | LVS_REPORT | LVS_SHAREIMAGELISTS,
      0, 0, Topic_Rect.right, Topic_Rect.bottom,
      Topic_Window,
      reinterpret_cast<HMENU>(2),
      Instance,
      0
    );
    if (List_Window == 0)
      throw spica::Win32::API_Error("Can't create the notice listview");

#ifdef NEVER
    // Associate the image list with the list view.
    if (ListView_SetImageList(List_Window, Image_Handle, LVSIL_SMALL) == 0)
      // throw spica::Win32::Windows_Error("Can't associate image list with the notice list view");
      /* Do nothing. ListView_SetImageList() appears to always return NULL */ ;
#endif

    Col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    Col.fmt      = LVCFMT_LEFT;
    Col.cx       = Topic_Rect.right/3;
    Col.pszText  = "Subject";
    Col.iSubItem = 0;

    int Err;
    Err = ListView_InsertColumn(List_Window, 0, &Col);
    if (Err == -1)
      throw spica::Win32::API_Error("Can't insert 'From' column into the notice listview");

    Col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    Col.fmt      = LVCFMT_LEFT;
    Col.cx       = Topic_Rect.right/3;
    Col.pszText  = "From";
    Col.iSubItem = 1;

    Err = ListView_InsertColumn(List_Window, 1, &Col);
    if (Err == -1)
      throw spica::Win32::API_Error("Can't insert 'Subject' column into the notice listview");

    Col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    Col.fmt      = LVCFMT_LEFT;
    Col.cx       = Topic_Rect.right/3;
    Col.pszText  = "Date & Time";
    Col.iSubItem = 2;

    Err = ListView_InsertColumn(List_Window, 2, &Col);
    if (Err == -1)
      throw spica::Win32::API_Error("Can't insert 'Date & Time' column into the notice listview");

    Tracer(3, "Finished creating the notice listview.");
    return List_Window;
  }


//
// Check_All
//
// The following function checks all of the items in a list view (by
// selecting an appropriate image.
//
static void Check_All(HWND List_Window)
  {
    int Item_Count = ListView_GetItemCount(List_Window);

    for (int i = 0; i < Item_Count; i++) {

      // Change the image associated with this item.
      LV_ITEM Item;
      Item.mask     = LVIF_IMAGE;
      Item.iItem    = i;
      Item.iSubItem = 0;
      Item.iImage   = 1;
      ListView_SetItem(List_Window, &Item);
    }
    ListView_RedrawItems(List_Window, 0, Item_Count - 1);
    UpdateWindow(List_Window);
  }


//----------------------------------
//           Main Program
//----------------------------------

//
// Main Program
//
int WINAPI WinMain(
  HINSTANCE Instance,
  HINSTANCE /* Previous_Instance */,
  LPSTR     Command_Line,
  int       Command_Show
  )
  {
    HWND    Frame_Window;  // Handle to the main application window.
    HWND    Client_Window; // Handle to the MDI client window (the workspace).
    MSG     Message;

    try {

      // Initialize global data as needed.
      Global::Set_Instance(Instance);
      Global::Set_CommandLine(Command_Line);
      Global::Set_CommandShow(Command_Show);

      // These strings are initialized here to be sure the "Big String Lock"
      // has been initialized before these Strings are constructed. Also for
      // the destructor (actually these strings are currently not destoryed.
      //
      Version_Number = new spica::String("1.0");
      Full_Name      = new spica::String;
      Email_Address  = new spica::String;

      Tracer(1, "NBread initializing...");

      // Read the registry to see how we are configured (or dialog with the user).
      Check_Configuration();

      string *Top_Path = spica::lookup_parameter("Noticeboard_Root");
      if (Top_Path == 0)
        throw spica::Win32::API_Error("Can't locate the noticeboard directory tree");

      // This object manages the read notice database (the "history").
      // It must be created before any topic is created because the
      // topic's constructor will reference this database. Similarly
      // this object must persist after all topics have been destroyed
      // to insure that the most up to date history file will be written
      // back to disk.
      // 
      History Read_Notices;
      History_Database = &Read_Notices;

      // This object represents the top level topic. All the subtopics
      // and notices are contained in this object. When this object is
      // destroyed all the contained objects and subobects will also be
      // destroyed.
      //
      NB_Topic Top_Level(Top_Path->c_str());
      Current_Topic = &Top_Level;

      // Set up the various window classes that we'll need.
      Set_Classes();
      InitCommonControls();

      // Create a main window and display it.
      Frame_Window = CreateWindow(
        Frame_ClassName,
        "VTC Noticeboard Reader",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        Instance,
        0
      );
      if (Frame_Window == 0)
        throw spica::Win32::API_Error("Can't create the frame window");

      Tracer(1, "Finished creating the MDI frame window.");
	
      // Get the MDI client window's handle.
      Client_Window = GetWindow(Frame_Window, GW_CHILD);

      ShowWindow(Frame_Window, Command_Show);
      UpdateWindow(Frame_Window);

      Tracer(1, "Entering main message dispatching loop...");

      // Get the next message from the application's message queue.
      while (GetMessage(&Message, 0, 0, 0)) {

        if (!TranslateMDISysAccel(Client_Window, &Message)) {
          TranslateMessage(&Message);
          DispatchMessage(&Message);
        }
      }

    }
    catch (spica::Win32::API_Error We) {
      spica::Win32::notifystream Error_Message;

      Error_Message << "Exception caught in WinMain\r" << We.what() << ends;
      Error_Message.say();
      return FALSE;
    }
    catch (...) {
      spica::Win32::notifystream Error_Message;

      Error_Message << "Unknown exception caught in WinMain" << ends;
      Error_Message.say();
      return FALSE;
    }

    return Message.wParam;
  }

//---------------------------------------
//           Window Procedures
//---------------------------------------

//
// Frame_Procedure
//
// This window procedure handles messages that are sent to the frame
// window (the window for the entire application).
//
LRESULT CALLBACK Frame_Procedure(
  HWND   Frame_Window,
  UINT   Message,
  WPARAM wParam,
  LPARAM lParam
  )
  {
    static HWND Topic_Window;
    static HWND Client_Window;

    try {
              
      switch (Message) {

        case WM_CREATE: {
            Tracer(2, "Processing WM_CREATE in the MDI frame window.");

            // Create the image list that I need.
            Image_Handle = ImageList_Create(16, 16, ILC_COLOR4 | ILC_MASK, 2, 0);
            if (Image_Handle == 0)
              throw spica::Win32::API_Error("Can't create image list");

            // Get the icon resources from the executable file. Note
            // that under Win32 we don't have to explicitly destory
            // these resources.
            //
            HICON UnChecked = static_cast<HICON>(
              LoadImage(Global::Get_Instance(), MAKEINTRESOURCE(UNCHECKED_ICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
            if (UnChecked == 0)
              throw spica::Win32::API_Error("Can't load UNCHECKED_ICON");

            HICON Checked = static_cast<HICON>(
              LoadImage(Global::Get_Instance(), MAKEINTRESOURCE(CHECKED_ICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
            if (Checked == 0)
              throw spica::Win32::API_Error("Can't load CHECKED_ICON");

            // Add the images to the image list.
            if (ImageList_AddIcon(Image_Handle, UnChecked) == -1)
              throw spica::Win32::API_Error("Can't add UNCHECKED_ICON to the image list");
            if (ImageList_AddIcon(Image_Handle, Checked) == -1)
              throw spica::Win32::API_Error("Can't add CHECKED_ICON to the image list");

            // Create the MDI client window.
            CLIENTCREATESTRUCT Client_Create;
            RECT               Frame_Rect;

            Client_Create.hWindowMenu  = 0;
            Client_Create.idFirstChild = 100;

            // Create the MDI client window.
            Client_Window = CreateWindow(
	      "MDICLIENT",
	      0,
	      WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE,
	      0, 0, 0, 0,
	      Frame_Window,
	      reinterpret_cast<HMENU>(1),
	      Global::Get_Instance(),
	      reinterpret_cast<LPVOID>(&Client_Create)
	    );
            if (Client_Window == 0)
              throw spica::Win32::API_Error("Can't create the MDI client window");

            Tracer(2, "Finished creating the MDI client window.");

            // Create the topic window.	 
            GetClientRect(Frame_Window, &Frame_Rect);
            Topic_Window = CreateMDIWindow(
              const_cast<char *>(Topic_ClassName),
              "Topic Window", 
              WS_CHILD | WS_VISIBLE,
              0, 0, Frame_Rect.right/2, Frame_Rect.bottom, 
              Client_Window,
              Global::Get_Instance(),
              0
            );
            if (Topic_Window == 0)
              throw spica::Win32::API_Error("Can't create the topic window");

            Tracer(2, "Finished creating the topic window.");
          }
          return 0;

        // A menu item was selected.
        case WM_COMMAND:
          switch (wParam) {

            case MENU_CONFIGURE: {
                Tracer(2, "Selected 'File|Configure' menu item.");
                DialogBox(Global::Get_Instance(), MAKEINTRESOURCE(CONFIG_DIALOG), Frame_Window, Config_Dialog);
              }
              return 0;

            case MENU_EXIT: {
                Tracer(2, "Selected 'File|Exit' menu item.");
                DestroyWindow(Frame_Window);
              }
              return 0;

            case MENU_POST: {
                Tracer(2, "Selected 'Topic|Post' menu item.");

                DialogBox(Global::Get_Instance(), MAKEINTRESOURCE(POST_DIALOG), Frame_Window, Post_Dialog);
              }
              return 0;

            case MENU_MARKALL: {
                Tracer(2, "Selected 'Topic|Mark All As Read' menu item.");
                Current_Topic->Mark_All(History_Database);
                SendMessage(Topic_Window, WM_USER, 0, 0);
              }
              return 0;

            case MENU_MARKSELECTED: {
                Tracer(2, "Selected 'Topic|Mark Selected As Read' menu item.");
                MessageBox(Frame_Window, "Not Implemented", "Sorry", MB_ICONEXCLAMATION);
              }
              return 0;

            case MENU_FOLLOWUP: {
                Tracer(2, "Selected 'Notice|Followup' menu item.");
                MessageBox(Frame_Window, "Not Implemented", "Sorry", MB_ICONEXCLAMATION);
              }
              return 0;

            case MENU_DEBUG: {
                Tracer(2, "Selected 'Debug' menu item.");
                spica::Win32::create_debugWindow();
              }
              return 0;

            case MENU_TILE: {
                Tracer(2, "Selected 'Window|Tile' menu item.");
                SendMessage(Client_Window, WM_MDITILE, 0, 0);
              }
              return 0;

            case MENU_CASCADE: {
                Tracer(2, "Selected 'Window|Cascade' menu item.");
                SendMessage(Client_Window, WM_MDICASCADE, 0, 0);
              }
              return 0;

            case MENU_ARRANGE: {
                Tracer(2, "Selected 'Window|Arrange' menu item.");
                SendMessage(Client_Window, WM_MDIICONARRANGE, 0, 0);
              }
              return 0;

            case MENU_HELP: {
                Tracer(2, "Selected 'Help' menu item.");
                MessageBox(Frame_Window, "Not Implemented", "Sorry", MB_ICONEXCLAMATION);
              }
              return 0;

            default: {
              Tracer(2, "Other WM_COMMAND seen. Sending to active MDI child window.");
              HWND MDIChild_Window =
                reinterpret_cast<HWND>(SendMessage(Client_Window, WM_MDIGETACTIVE, 0, 0));
              if (IsWindow(MDIChild_Window))
                SendMessage(MDIChild_Window, WM_COMMAND, wParam, lParam);
              break;
            }	
          }
          break;

        // The user is trying to close the application (or shut down Windows).
        case WM_QUERYENDSESSION:
        case WM_CLOSE:

          // Try to close all the children.
          EnumChildWindows(Client_Window, Close_Childs, 0);

          // If it didn't work, return 0. Otherwise call DefFrameProc().
          if (GetWindow(Client_Window, GW_CHILD) != 0) return 0;
          break;

        // The main window is being destroyed.
        case WM_DESTROY:
          ImageList_Destroy(Image_Handle);
          PostQuitMessage(0);
          return 0;
      
      }

    }
    catch (spica::Win32::API_Error We) {
      spica::Win32::notifystream Error_Message;

      Error_Message << "Exception caught in the frame window\r" << We.what() << ends;
      Error_Message.say(Frame_Window);
      return 0;
    }
    catch (...) {
      spica::Win32::notifystream Error_Message;

      Error_Message << "Unknown exception caught in the frame window" << ends;
      Error_Message.say(Frame_Window);
      return 0;
    }

    return DefFrameProc(Frame_Window, Client_Window, Message, wParam, lParam);
  }


//
// Close_Childs
//
// This function is called for each of the MDI child windows when the
// application is thinking about closing.
//
BOOL CALLBACK Close_Childs(HWND Child_Window, LPARAM)
  {
    // Checks for "icon title." I'm not sure what this is about, but Petzold
    //   thinks it's necessary.
    //
    if (GetWindow(Child_Window, GW_OWNER)) return 1;

    // Tell the MDI client window to restore us.
    SendMessage(GetParent(Child_Window), WM_MDIRESTORE, reinterpret_cast<WPARAM>(Child_Window), 0);

    // If we don't want to die, just pass us over.
    if (!SendMessage(Child_Window, WM_QUERYENDSESSION, 0, 0)) return 1;

    // Tell the MDI client window to destroy us.
    SendMessage(GetParent(Child_Window), WM_MDIDESTROY, reinterpret_cast<WPARAM>(Child_Window), 0);
    return 1;
  }


//
// This window procedure handles messages that are sent to the topic
// window.
//
LRESULT CALLBACK Topic_Procedure(
  HWND   Topic_Window,
  UINT   Message,
  WPARAM wParam,
  LPARAM lParam
  )
  {
    static HWND SubtopicLV_Handle;
    static HWND NoticeLV_Handle;
    static bool Application_Closing = false;

    try {
	
      switch (Message) {

        case WM_CREATE: {
            Tracer(2, "Processing WM_CREATE for the topic window.");
            SubtopicLV_Handle = Create_SubtopicLV(Global::Get_Instance(), Topic_Window);
            NoticeLV_Handle   = Create_NoticeLV(Global::Get_Instance(), Topic_Window);
            Current_Topic->Populate_SubtopicLV(SubtopicLV_Handle);
            Current_Topic->Populate_NoticeLV(NoticeLV_Handle, History_Database);

            spica::String Title = "Topic: ";
            Title.append(Current_Topic->Description());
            SetWindowText(Topic_Window, Title);
          }
          return 0;

        // If the topic window is resized, we need to resize the list
        // views. NOTE: For some reason doing this causes the
        // min/max/close controls on the topic window to disappear when
        // the window is maximized. This needs to be fixed sometime.
        //
        case WM_SIZE: {
            Tracer(2, "Processing WM_SIZE for the topic window.");
            int Width  = LOWORD(lParam);
            int Height = HIWORD(lParam);

            MoveWindow(SubtopicLV_Handle, 0, Height/2, Width, Height/2, TRUE);
            MoveWindow(NoticeLV_Handle, 0, 0, Width, Height/2, TRUE);
          }
          return 0;

        // The application is shutting down. All MDI child windows are
        // queried and then closed. We will use this fact to distinguish
        // between an application close and the user trying to close
        // just the topic window. This case returns 1 to indicate that
        // closing is "okay."
        //
        case WM_QUERYENDSESSION:
          Application_Closing = true;
          return 1;

        // Ignore all attempts to close the topic window unless the
        // entire application is shutting down.
        //
        case WM_CLOSE:
          if (Application_Closing) DestroyWindow(Topic_Window);
          else
            MessageBox(Topic_Window, "Topic window can not be closed", "Error", MB_ICONEXCLAMATION);
          return 0;

        // This message is sent to us (by the frame window) when we are
        // supposed to check off all the notices in the child notice
        // list view.
        // 
        case WM_USER:
          Check_All(NoticeLV_Handle);
          return 0;

        // This message is sent to us by the child list view controls.
        case WM_NOTIFY: {
            int          ID  = wParam;
            NM_LISTVIEW *pNM = reinterpret_cast<NM_LISTVIEW *>(lParam);

            // Is the subtopic listview trying to notify me?
            if (ID == 1) {

              // What has happened?
              switch (pNM->hdr.code) {

                // We double clicked in it. Try to switch to the subtopic.
                case NM_DBLCLK: {
                    Tracer(3, "Processing NM_DBLCLK in the subtopic listview.");
                    NB_Topic *New_Topic = Current_Topic->Lookup_Subtopic(SubtopicLV_Handle);
                    if (New_Topic != 0) {
                      ListView_DeleteAllItems(SubtopicLV_Handle);
                      ListView_DeleteAllItems(NoticeLV_Handle);
                      New_Topic->Populate_SubtopicLV(SubtopicLV_Handle);
                      New_Topic->Populate_NoticeLV(NoticeLV_Handle, History_Database);
                      Current_Topic = New_Topic;

                      spica::String Title = "Topic: ";
                      Title.append(Current_Topic->Description());
                      SetWindowText(Topic_Window, Title);
                    }
                  }
                  return 0;
              }
            }

            // Is the notice listview trying to notify me?
            if (ID == 2) {

              // What has happened?
              switch (pNM->hdr.code) {

                // We double clicked in it. Open the notice.
                case NM_DBLCLK: {
                    Tracer(3, "Processing NM_DBLCLK in the notice listview.");
                    static HWND Notice_Handle;

                    NB_Notice *Old = Current_Notice;
                    Current_Notice = Current_Topic->Lookup_Notice(NoticeLV_Handle);
                    if (Current_Notice == 0) Current_Notice = Old;
                    if (Current_Notice == Old) return 0;

                    // If this is the first time the window has appeared, then create it.
                    if (Old == 0) {
                      MDICREATESTRUCT    MDI_Create;
                      RECT               Client_Rect;
                      HWND               Client_Window = GetParent(Topic_Window);
	
                      GetClientRect(Client_Window, &Client_Rect);
	
                      MDI_Create.szClass = Notice_ClassName;
                      MDI_Create.szTitle = "Notice Window";
                      MDI_Create.hOwner  = Global::Get_Instance();
                      MDI_Create.x       = Client_Rect.right/2;
                      MDI_Create.y       = 0;
                      MDI_Create.cx      = Client_Rect.right/2;
                      MDI_Create.cy      = Client_Rect.bottom;
                      MDI_Create.style   = WS_HSCROLL | WS_VSCROLL;
                      MDI_Create.lParam  = 0;
                      Notice_Handle =
                        reinterpret_cast<HWND>(SendMessage(Client_Window, WM_MDICREATE, 0, reinterpret_cast<LPARAM>(&MDI_Create)));
                      if (Notice_Handle == 0)
                        throw spica::Win32::API_Error("Can't create the notice window");

                      Tracer(3, "Finished creating the notice window.");
                    }
                    Current_Notice->Mark_AsRead(History_Database);

                    // Make sure the notice window gets drawn.
                    InvalidateRect(Notice_Handle, 0, TRUE);
                  }
                  return 0;
              }

            }  // End of if (ID == 2) ...
          }  // End of WM_NOTIFY case.
          return 0;

      }  // End of outer switch statement.

    }
    catch(spica::Win32::API_Error We) {
      spica::Win32::notifystream Error_Message;

      Error_Message << "Exception caught in the topic window\r" << We.what() << ends;
      Error_Message.say(Topic_Window);
      return 0;
    }
    catch (...) {
      spica::Win32::notifystream Error_Message;

      Error_Message << "Unknown exception caught in the topic window" << ends;
      Error_Message.say(Topic_Window);
      return 0;
    }

    return DefMDIChildProc(Topic_Window, Message, wParam, lParam);
  }


//
// Notice_Procedure
//
// This window procedure handles messages that are sent to a notice window.
//
LRESULT CALLBACK Notice_Procedure(
  HWND   Notice_Window,
  UINT   Message,
  WPARAM wParam,
  LPARAM lParam
  )
  {
    static int	Char_Height;
    static int  Char_Width;
    static bool	HaveTextInfo = false;

    try {
	
      if (!HaveTextInfo) {
        TEXTMETRIC Text_Metrics;
        HDC        Context_Handle;

        Tracer(2, "Getting font size information in the notice window.");

        Context_Handle = GetDC(Notice_Window);
        SelectObject(Context_Handle, GetStockObject(OEM_FIXED_FONT));
        GetTextMetrics(Context_Handle, &Text_Metrics);
        Char_Height = Text_Metrics.tmHeight + Text_Metrics.tmExternalLeading;
        Char_Width  = Text_Metrics.tmAveCharWidth;
        ReleaseDC(Notice_Window, Context_Handle);

        HaveTextInfo = true;
      }
	
      switch (Message) {

        case WM_PAINT: {
            spica::Win32::Paint_Context Painter(Notice_Window);

            if (Current_Notice != 0) 
              Current_Notice->Redraw(Notice_Window, Painter);
          }
          return 0;

        case WM_HSCROLL: {
            Tracer(2, "Processing WM_HSCROLL in the notice window.");
            if (Current_Notice != 0)
              Current_Notice->HScroll(Notice_Window, wParam, Char_Width);
          }
          return 0;

        case WM_VSCROLL: {
            Tracer(2, "Processing WM_VSCROLL in the notice window.");
            if (Current_Notice != 0)
    	      Current_Notice->VScroll(Notice_Window, wParam, Char_Height);
          }
          return 0;

        case WM_CLOSE:
          Current_Notice = 0;
          DestroyWindow(Notice_Window);
          return 0;
      }

    }
    catch (spica::Win32::API_Error We) {
      spica::Win32::notifystream Error_Message;

      Error_Message << "Exception caught in the notice window\r" << We.what() << ends;
      Error_Message.say(Notice_Window);
      return 0;
    }
    catch (...) {
      spica::Win32::notifystream Error_Message;

      Error_Message << "Unknown exception caught in the notice window" << ends;
      Error_Message.say(Notice_Window);
      return 0;
    }

    return DefMDIChildProc(Notice_Window, Message, wParam, lParam);
  }

