/****************************************************************************
FILE          : nbtopic.cpp
LAST REVISION : January 2000
SUBJECT       : Implementation of the NB_Topic class.
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
#include <algorithm>
#include <iomanip>
#include <strstream>

using namespace std;

#include <windows.h>
#include <commctrl.h>

#undef min
#undef max

#include "history.hpp"
#include "nbobject.hpp"
#include "str.hpp"
#include "windebug.hpp"
#include "winexcept.hpp"

//
// This is a hack. This pointer is used during the sorting operation. It
// is set by one of the NB_Topic member functions to point at the
// TObject_List contained within *before* that member function tries to
// sort the associated list view. This is necessary so that the
// comparison function can make use of the lParam associated with each
// list view item.
//
static TObject_List *Current_TList = 0;

// Same hack as above, only this time for notices.
static NObject_List *Current_NList = 0;


//
// Subtopic_Compare
//
// The following function is used by the subtopic list view to sort items.
//
int CALLBACK Subtopic_Compare(LPARAM        , LPARAM        , LPARAM); // CodeWarrior strangeness.
int CALLBACK Subtopic_Compare(LPARAM lParam1, LPARAM lParam2, LPARAM)
  {
    int i1 = static_cast<int>(lParam1);
    int i2 = static_cast<int>(lParam2);

    // Handle the possible parent entry in a special way. This forces it to the top.
    if (i1 == -1) return -1;
    if (i2 == -1) return  1;

    spica::String d1 = (*Current_TList)[i1]->Description();
    spica::String d2 = (*Current_TList)[i2]->Description();

    if (d1 <  d2) return -1;
    if (d1 == d2) return  0;
    return 1;
  }


//
// Month_Index
//
// This function returns an integer representing the month given to it as
//   a three letter abbreviated name. It doesn't matter if this function
//   returns indices that are zero based or one based. It is only the
//   ordering that matters.
//
static int Month_Index(const spica::String &Month)
  {
    static const char *Names[] = {
      "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", 0 };

    const char **p = Names;
    while (*p != 0) {
      if (strcmp(*p, Month) == 0) return p - Names;
      p++;
    }

    // If we don't recognize the month return the "13th" month. This will force that entry
    //   to the top of the display (for its year) and make the problem obvious.
    //
    return 13;
  }


//
// Notice_Compare
//
// The following function is used by the notice list view to sort items.
//
int CALLBACK Notice_Compare(LPARAM        , LPARAM        , LPARAM); // CodeWarrior strangeness.
int CALLBACK Notice_Compare(LPARAM lParam1, LPARAM lParam2, LPARAM)
  {
    // Convert the LPARAMs into vector indicies.
    int i1 = static_cast<int>(lParam1);
    int i2 = static_cast<int>(lParam2);

    // Look up the raw dates.
    spica::String The_Date1 = (*Current_NList)[i1]->RawDate_String();
    spica::String The_Date2 = (*Current_NList)[i2]->RawDate_String();

    // Convert the raw dates into something easier to compare.
    // Mon, 23 Mar 1998 09:39:31 EST5EDT
    //
    int Day1    = atoi(The_Date1.word(2));
    int Day2    = atoi(The_Date2.word(2));
    int Month1  = Month_Index(The_Date1.word(3));
    int Month2  = Month_Index(The_Date2.word(3));
    int Year1   = atoi(The_Date1.word(4));
    int Year2   = atoi(The_Date2.word(4));

    spica::String Time1 = The_Date1.word(5);
    spica::String Time2 = The_Date2.word(5);
    int Hour1   = atoi(Time1.word(1, ":"));
    int Hour2   = atoi(Time2.word(1, ":"));
    int Minute1 = atoi(Time1.word(2, ":"));
    int Minute2 = atoi(Time2.word(2, ":"));

    // Now order them...
    if (Year2 >  Year1) return 1;
    if (Year2 == Year1) {
      if (Month2 >  Month1) return 1;
      if (Month2 == Month1) {
        if (Day2 >  Day1) return 1;
        if (Day2 == Day1) {
          if (Hour2 >  Hour1) return 1;
          if (Hour2 == Hour1) {
            if (Minute2 >  Minute1) return 1;
            if (Minute2 == Minute1) return  0;
          }
        }
      }
    }
    return -1;
  }


//
// NB_Topic::NB_Topic
//
NB_Topic::NB_Topic(const char *Path, NB_Topic *P) :
  Topic_Path       (Path),
  Topic_ID         (Path),
  Contents_Valid   (false),
  Description_Valid(false),
  Parent           (P)
  { }


//
// NB_Topic::Populate_SubtopicLV
//
// This function loads a list view control with information about all the
//   subtopics in this topic.
//
void NB_Topic::Populate_SubtopicLV(HWND List_Window)
  {
    Tracer(4, "Populating the subtopic list view.");

    TObject_List::iterator Topic_Stepper;
    int                    ListView_Index = 0;
    int                    Vector_Index   = 0;
    LV_ITEM                Item;

    if (!Contents_Valid) Read_Directory();

    // Tell the list view ahead of time how many items we have. This allows it
    //   to allocate memory more efficiently. That is nice.
    //
    int Item_Count = 0;
    if (Parent != 0) Item_Count++;
    Item_Count += Sub_Topics.size();
    ListView_SetItemCount(List_Window, Item_Count);

    // Handle the parent topic in a special way (if it exists at all).
    if (Parent != 0) {
      Parent_Name = "[Back] ";
      Parent_Name.append(Parent->Description());

      Item.mask       = LVIF_TEXT | LVIF_PARAM;
      Item.iItem      = ListView_Index++;
      Item.iSubItem   = 0;
      Item.pszText    = const_cast<char *>(static_cast<const char *>(Parent_Name));
      Item.cchTextMax = 17;
      Item.lParam     = static_cast<LPARAM>(-1);

      if (ListView_InsertItem(List_Window, &Item) == -1)
        throw spica::Win32::API_Error("Can't insert parent item into the subtopic list view");
    }

    // For all subtopics...
    for (Topic_Stepper = Sub_Topics.begin(); Topic_Stepper != Sub_Topics.end(); Topic_Stepper++) {
      Item.mask     = LVIF_TEXT | LVIF_PARAM;
      Item.iItem    = ListView_Index++;
      Item.iSubItem = 0;

      Item.pszText    = const_cast<char *>(static_cast<const char *>((*Topic_Stepper)->Description()));
      Item.lParam     = static_cast<LPARAM>(Vector_Index);
      Vector_Index++;

      if (ListView_InsertItem(List_Window, &Item) == -1)
        throw spica::Win32::API_Error("Can't insert an item into the subtopic list view");
    }

    // Now sort it.
    Current_TList = &Sub_Topics;
    ListView_SortItems(List_Window, Subtopic_Compare, 0);
  }


//
// NB_Topic::Populate_NoticeLV
//
// This function loads a list view control with information about all the
//   notices in this topic.
//
void NB_Topic::Populate_NoticeLV(HWND List_Window, History *History_Database)
  {
    Tracer(4, "Populating the notice list view.");

    NObject_List::iterator Notice_Stepper;
    int                    Index = 0;
    LV_ITEM                Item;

    if (!Contents_Valid) Read_Directory();

    // Tell the list view ahead of time how many items we have. This allows it
    //   to allocate memory more efficiently. That is nice.
    //
    int Item_Count = 0;
    Item_Count += Sub_Topics.size();
    ListView_SetItemCount(List_Window, Item_Count);

    // For all notices...
    for (Notice_Stepper = Topic_Contents.begin(); Notice_Stepper != Topic_Contents.end(); Notice_Stepper++) {
      Item.mask     = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
      Item.iItem    = Index;
      Item.iSubItem = 0;

      Item.pszText    = const_cast<char *>(static_cast<const char *>((*Notice_Stepper)->Description()));
      Item.lParam     = static_cast<LPARAM>(Index);

      // Has this notice been read?
      if ((*Notice_Stepper)->Is_Read(History_Database)) {
        // Item.state = (2 << 12) | LVIS_STATEIMAGEMASK;
        // Item.stateMask = LVIS_STATEIMAGEMASK;
        Item.iImage = 1;
      }
      else {
        // Item.state = (1 << 12) | LVIS_STATEIMAGEMASK;
        // Item.stateMask = LVIS_STATEIMAGEMASK;
        Item.iImage = 0;
      }

      if (ListView_InsertItem(List_Window, &Item) == -1)
        throw spica::Win32::API_Error("Can't insert an item into the notice list view");

      // Install the subitems.
      Item.mask       = LVIF_TEXT;
      Item.iItem      = Index;
      Item.iSubItem   = 1;
      Item.pszText    = const_cast<char *>(static_cast<const char *>((*Notice_Stepper)->Poster_Name()));
      if (ListView_SetItem(List_Window, &Item) == FALSE)
        throw spica::Win32::API_Error("Can't insert a subitem into the notice list view");

      Item.iSubItem   = 2;
      Item.pszText    = const_cast<char *>(static_cast<const char *>((*Notice_Stepper)->Date_String()));
      if (ListView_SetItem(List_Window, &Item) == FALSE)
        throw spica::Win32::API_Error("Can't insert a subitem into the notice list view");

      Index++;
    }

    // Now sort it.
    Current_NList = &Topic_Contents;
    ListView_SortItems(List_Window, Notice_Compare, 0);
  }


//
// NB_Topic::Lookup_Subtopic
//
// This function takes list view double click information and returns a pointer
//   to the selected subtopic.
//
NB_Topic *NB_Topic::Lookup_Subtopic(HWND List_Window)
  {
    Tracer(4, "Looking up a subtopic using the mouse position.");

    POINT Mouse_Position;

    GetCursorPos(&Mouse_Position);
    ScreenToClient(List_Window, &Mouse_Position);

    // Ask the list view which item this position refers to.
    LV_HITTESTINFO Hit_Info;
    Hit_Info.pt = Mouse_Position;
    ListView_HitTest(List_Window, &Hit_Info);

    // If no item is being clicked on, just return the null pointer.
    if (Hit_Info.flags == LVHT_NOWHERE) return 0;

    // Make some adjustments if there is a parent entry to worry about.
    if (Parent != 0 && Hit_Info.iItem == 0) {
      return Parent;
    }
      
    // Look up the index into our vector.
    LV_ITEM Item;
    Item.mask     = LVIF_PARAM;
    Item.iItem    = Hit_Info.iItem;
    Item.iSubItem = 0;
    ListView_GetItem(List_Window, &Item);

    // Return the goods.
    return Sub_Topics[static_cast<int>(Item.lParam)];
  }


//
// NB_Topic::Lookup_Notice
//
// This function takes list view double click information and returns a pointer
//   to the selected notice.
//
NB_Notice *NB_Topic::Lookup_Notice(HWND List_Window)
  {
    Tracer(4, "Looking up a notice using the mouse position.");

    POINT Mouse_Position;

    GetCursorPos(&Mouse_Position);
    ScreenToClient(List_Window, &Mouse_Position);

    // Ask the list view which item this position refers to.
    LV_HITTESTINFO Hit_Info;
    Hit_Info.pt = Mouse_Position;
    ListView_HitTest(List_Window, &Hit_Info);

    // If no item is being clicked on, just return the null pointer.
    if (Hit_Info.flags == LVHT_NOWHERE) return 0;

    // Change the image associated with this item. Clicking on an item with
    //   the mouse will force the check mark. (Hopefully the related notice
    //   will be marked as read elsewhere.
    //
    LV_ITEM Item;
    Item.mask     = LVIF_IMAGE;
    Item.iItem    = Hit_Info.iItem;
    Item.iSubItem = 0;
    Item.iImage   = 1;
    ListView_SetItem(List_Window, &Item);
    ListView_RedrawItems(List_Window, Item.iItem, Item.iItem);
    UpdateWindow(List_Window);

    // Now look up the index into our vector.
    Item.mask     = LVIF_PARAM;
    Item.iItem    = Hit_Info.iItem;
    Item.iSubItem = 0;
    ListView_GetItem(List_Window, &Item);

    // Return the goods.
    return Topic_Contents[static_cast<int>(Item.lParam)];
  }


//
// NB_Topic::New_NoticePath
//
// The following function figures out a filename for a new notice. This file
//   name is not supposed to conflict with any existing files in the topic
//   directory.
//
spica::String NB_Topic::New_NoticePath()
  {
    Tracer(4, "Computing a unique path for a new notice");

    spica::String Result = Topic_Path;
    Result.append("\\nb");

    time_t Raw_Time = time(0);
    struct tm *Cooked_Time = localtime(&Raw_Time);

    ostrstream Name_String;
    Name_String << setw(2) << setfill('0') << Cooked_Time->tm_hour;
    Name_String << setw(2) << setfill('0') << Cooked_Time->tm_min;
    Name_String << setw(2) << setfill('0') << Cooked_Time->tm_sec;
    Name_String << ends;

    char *Raw_Result = Name_String.str();
    Result.append(Raw_Result);
    delete [] Raw_Result;

    Result.append(".cnb");

    return Result;
  }


//
// NB_Topic::Mark_All
//
// The following function will mark all notices in this topic as read.
//
void NB_Topic::Mark_All(History *The_History)
  {
    Tracer(4, "Marking all notices as read in a topic");

    NObject_List::iterator Stepper;
    for (Stepper = Topic_Contents.begin(); Stepper != Topic_Contents.end(); Stepper++) {
      (*Stepper)->Mark_AsRead(The_History);
    }
  }


//
// NB_Topic::Description
//
// The following function returns a description string for the topic.
//
spica::String &NB_Topic::Description()
  {
    if (Description_Valid) return Description_String;
    else {
      Tracer(4, "Formatting a topic description.");

      ostrstream Formatter;

      if (!Contents_Valid) Read_Directory();

      Formatter << Topic_ID.Long_Name()
                << " (entities = " << (Sub_Topics.size() + Topic_Contents.size()) << ")"
                << ends;

      char  *p = Formatter.str();
      Description_String = p;
      Description_Valid  = true;
      delete [] p;
    }
    return Description_String;
  }


//
// Read_Directory
//
// This function scans the directory specified by path and creates the Topic_Contents.
//
void NB_Topic::Read_Directory()
  {
    Tracer(4, "Reading a topic directory.");

    spica::String     WildCard_Name;
    WIN32_FIND_DATA Scan_Information;
    HANDLE          Search_Handle;

    // First, scan for subdirectories. From that information we will build the sub
    //   topic list. The method for generating the wildcard sequence needs to be
    //   improved. We should check for trailing '\' characters first.
    //
    WildCard_Name = Topic_Path;
    WildCard_Name.append("\\*.*");

    Search_Handle = FindFirstFile(static_cast<const char *>(WildCard_Name), &Scan_Information);
    if (Search_Handle == INVALID_HANDLE_VALUE) {
      Contents_Valid = true;
      return;
    }

    // Process the first file and then the rest.
    do {
      spica::String Entity_Name(Topic_Path);
      Entity_Name.append("\\");
      Entity_Name.append(Scan_Information.cFileName);

      if (Scan_Information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

        // Forget about the "." and ".." directories.
        if (strcmp(Scan_Information.cFileName, "." ) == 0 ||
            strcmp(Scan_Information.cFileName, "..") == 0) continue;

        NB_Topic *New_Topic = new NB_Topic(static_cast<const char *>(Entity_Name), this);
        Sub_Topics.push_back(New_Topic);
      }
    } while (FindNextFile(Search_Handle, &Scan_Information));

    // Close down the search handle.
    FindClose(Search_Handle);

    // Now we want to scan for the notice files. For now, assume a file is a notice
    //   file iff it matches *.CNB.
    WildCard_Name = Topic_Path;
    WildCard_Name.append("\\*.CNB");

    Search_Handle = FindFirstFile(static_cast<const char *>(WildCard_Name), &Scan_Information);
    if (Search_Handle == INVALID_HANDLE_VALUE) {
      Contents_Valid = true;
      return;
    }

    // Process the first file and then the rest.
    do {
      spica::String Entity_Name(Topic_Path);
      Entity_Name.append("\\");
      Entity_Name.append(Scan_Information.cFileName);

      // Let's verify that this is a regular file first...
      if ((Scan_Information.dwFileAttributes &  FILE_ATTRIBUTE_ARCHIVE) ||
          (Scan_Information.dwFileAttributes == 0 )) {

        NB_Notice *New_Notice = new NB_Notice(static_cast<const char *>(Entity_Name));
        Topic_Contents.push_back(New_Notice);
      }
    } while (FindNextFile(Search_Handle, &Scan_Information));

    // Close down the search handle.
    FindClose(Search_Handle);

    Contents_Valid = true;
  }

