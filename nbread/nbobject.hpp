/****************************************************************************
FILE          : nbobject.h
LAST REVISION : January 2000
SUBJECT       : Interface to NB_Object and derived classes.
PROGRAMMER    : (C) Copyright 2000 by VTC Computer Club

All things that are on the noticeboards are of types derived from class
NB_Object. The notice board reader tries as much as possible to manage
thise objects generically.


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

#ifndef NBOBJECT_H
#define NBOBJECT_H

#include <windows.h>
#undef min
#undef max
  // The macros min() and max() in windows.h conflict with the STL inline functions.

#include <list>
#include <vector>

using namespace std;

#include "history.hpp"
#include "idinfo.hpp"
#include "str.hpp"

//
// class NB_Object
//
// The base class defines characteristics in common with all NB_Objects.
//
class NB_Object {
  public:
    virtual ~NB_Object();
      // Necessary if polymorphic objects are going to be destroyed right.

    virtual spica::String &Description() = 0;
      // These functions returns a one line description of the object. It
      //   is displayed in the object's topic window.
};

class NB_Topic;
class NB_Notice;

//
// class Object_List
//
// This class enhances a vector<NB_Object *> by deleting all objects in the vector
//   when the vector is destroyed. Normally pointers don't have destructors so this
//   "trick" is necessary to get the pointed-at objects cleaned up. This also
//   means that only dynamically allocated objects can be put onto an Object_List.
//
class TObject_List : public vector<NB_Topic *> {
  public:
    ~TObject_List();
};

class NObject_List : public vector<NB_Notice *> {
  public:
    ~NObject_List();
};


//
// class NB_Topic
//
// This class defines a topic.
//
class NB_Topic : public NB_Object {
  public:
    NB_Topic(const char *Path, NB_Topic *P = 0);

    virtual spica::String &Description();

    void Populate_SubtopicLV(HWND);
      // Fills a list view control with the necessary subtopic information.

    void Populate_NoticeLV(HWND, History *History_Database);
      // Fills a list view control with the necessary notice information.

    NB_Topic *Lookup_Subtopic(HWND);
      // Looks up a subtopic given list view double click information.

    NB_Notice *Lookup_Notice(HWND);
      // Looks up a notice given list view double click information.

    spica::String New_NoticePath();
      // Returns the full file name for a new notice. This name will not conflict
      //   with any existing names.

    void Mark_All(History *);
      // This function will mark all notices in the current topic as read.
    
  private:
    spica::String  Description_String; // Caches the description.
    bool         Description_Valid;  // =true when the cached description is valid.
    spica::String  Topic_Path;         // The path to the directory for this topic.
    ID_Info      Topic_ID;           // Information from NB.ID (or similar source).
    TObject_List Sub_Topics;         // This list is just for NB_Topic objects.
    NB_Topic    *Parent;             // Points at this topic's parent or NULL if no parent.
    spica::String  Parent_Name;        // The (modified) name of the parent.
    NObject_List Topic_Contents;     // This list is for everything else.
    bool         Contents_Valid;     // =true when the both lists above are valid.

    void Read_Directory();
      // Scan the directory into Sub_Topics and Topic_Contents.
};


//
// class NB_Notice
//
// This class defines a notice.
//
class NB_Notice : public NB_Object {
  public:
    NB_Notice(const char *Path) :
        Processed  (false),
        Have_Text  (false),
        Notice_Path(Path),
        Position   (0),
        HOffset    (0)
        { }

    virtual spica::String &Description();
    virtual spica::String &Poster_Name();
    virtual spica::String &Date_String();
    virtual spica::String &RawDate_String();
      // Summary information for notices.

    virtual void Mark_AsRead(History *);
      // Causes this notice to mark itself as read in the history database.

    virtual bool Is_Read(History *);
      // Returns true if this notice has been read (as known by the given
      //   history database).

    virtual void Redraw(const HWND &, const HDC &);
    virtual void VScroll(const HWND &, const WPARAM &, const int &);
    virtual void HScroll(const HWND &, const WPARAM &, const int &);
        
  private:
    spica::String         Notice_Path;
    vector<spica::String> Notice_Text;
    bool                Processed; // True after we have processed the summary information
    bool                Have_Text; // True after we have read the notice text (also means we have summary)
    int                 Position;  // Line number of top line in window. Zero based.
    int                 HOffset;   // Column number of left edge. Zero based.
    int                 Longest_Line;  // The length of the longest line in the notice.

    // Following variables contain the information that the summary currently contains.
    //   Eventually, priority and other options will be added. If objects are added to
    //   these, also increment the value of Num_Objects. That is used as a cheap
    //   optimization flag.  If the number of objects filled is equal to Num_Objects,
    //   then we are done and can stop reading the file (skipping long posts, etc).
    //
    spica::String From;
    spica::String Subject;
    spica::String Date;
    spica::String Raw_Date;  // This object is filled when 'Date' is filled.
    static const int Num_Objects;

    void Process_Summary();
      // This function fills in the summary information above and sets Processed
      //   to "true."
};

#endif

