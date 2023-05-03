/*****************************************************************************
File          : lc.cpp
Last Revision : August 1997
Revised By    : Michael Martel, Paul Cabbe, Peter Chapin
Programmer    : VTC^3

This file contains the main source code for the Learning Center student
evaluation storage program. This same source code also compiles the
first semester experience evaluation program. Before compiling, you must
#define one of the two symbols below.

*****************************************************************************/

// Set one of the symbols below to 1 to control which version of this
//   program gets compiled.
#define LC  0
#define FSE 1

#include "environ.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// NetWare SDK headers.
#include <nwcalls.h>
#include <nwnet.h>

#include "standard.h" 
#include "display.h"
#include "enter.h"
#include "helper.h"
#include "highlite.h"
#include "message.h"
#include "portscr.h"
#include "position.h"
#include "sbox.h"
#include "topics.h"
#include "utility.h"

#define MAX_MESSAGES 250
  // Maximum number of messages in one topic file.

// Name of the file containing the list of topics.
#if LC
#define MASTERTOPIC_FILE "S:\\LC\\STUDENTS.LST"
#else
#define MASTERTOPIC_FILE "R:\\FSE\\STUDENTS.LST"
#endif

// Name of the configuration file that controls what's what.
#if LC
#define CONFIG_FILE "F:\\LC.CFG"
#else
#define CONFIG_FILE "F:\\FSE.CFG"
#endif

// The name of the temporary file used when creating messages.
#if LC
#define TEMP_FILE "F:\\LCTMP.$$$"
#else
#define TEMP_FILE "F:\\FSETMP.$$$"
#endif

Message Topic_Index[MAX_MESSAGES+1];
  // Always a NULL message at the end.

char Username[MAX_DN_CHARS];
  // The name of the invoking user.


//
// Blank_Screen
//
// The following function blanks the screen until the user presses the TAB key.
//   At that point the original screen is restored.
//
void Blank_Screen(void)
  {
    char Buff[80*25*2];
    ScrRead(1, 1, 80, 25, Buff);
    ScrClear(1, 1, 80, 25, SCR_WHITE|SCR_BRIGHT);
    ScrPrintText(13, 29, 80, "Press TAB to restore screen");
    while (ScrKey() != K_TAB) /* Null */ ;
    ScrWrite(1, 1, 80, 25, Buff);
  }


//
// Credits
//
// The following function prints version information and performs various
//   clean up duties.
//
void Credits()
  {
    ScrTerminate();

    #if LC
    printf("LC Student Evaluation Program (Version \"The very second\")\n  by Paul Cabbe & VTC^3 (revised by Peter Chapin)\nCompiled: %s\n", AdjDate(__DATE__));
    #else
    printf("FSE Student Evaluation Program (Version \"The very second\")\n by Paul Cabbe & VTC^3 (revised by Peter Chapin)\nCompiled: %s\n", AdjDate(__DATE__));
    #endif

    remove(TEMP_FILE);
  }


//
// Initialize
//
// The following function reads the master topic file and prepares the topic
//   list. Note that the name of the master topic should, eventually, be taken
//   from a more general source.
//
void Initialize(void)
  {
    // If we got here, the user is authorized. Try to open the topics.
    if (Open_Topics(MASTERTOPIC_FILE, CONFIG_FILE) == 0) {
      ScrClear(1, 1, 80, 25, SCR_WHITE);
      ScrPrint(13, 10, 65, SCR_WHITE|SCR_BRIGHT|SCR_BLINK, "Error: Can't open student list file. Please contact CCASTELL");
      ScrRefresh();

      Delay(10);
        // 10 seconds delay so the user can read the message.

      exit(1);
    }
  }


//
// Choose_Topic
//
// The following function displays the list of topics and lets the user select
//   the one they want.
//
// BUG: The method used to display the topics causes all the active topics to
//   be displayed first regardless of the order in the master topic file.
//   however, since they are numbered in the order they are displayed, the
//   numbers assigned will not correctly refer to the topic in the topic list
//   under these conditions.
//
// WORKAROUND: Be sure all active topics are at the top of the master topic
//   file.
//
int Choose_Topic(Topic_File *Topic_List, int *Max_Topic, int Topic_Number, int *Top_Topic)
  {
    char  *Prompt;
    int    Row;
    int    Result;
    int    Key;
    char  *Text[80];
    char   Buffer[2];

    // Colors of things.
    const int INFO_COLOR       = SCR_BRIGHT|SCR_WHITE|SCR_REV_BLUE;
    const int PROMPT_COLOR     = SCR_BRIGHT|SCR_WHITE|SCR_REV_BLUE;
    const int FORUM_COLOR      = SCR_WHITE;

    // Positions of things (one based).
    const int INFO_ROW   = 1;     // Row where information bar is displayed.
    const int BOX_ROW    = 2;     // First row used for boxes.
    const int PROMPT_ROW = 25;    // Row where prompt is displayed.

    Simple_Box Forums;

    // Figure out the number of forum topics.
    Topic_File *Old = Topic_List;
    int Nmbr_Forums  = 0;
    while (Topic_List->Topic_Description != NULL) {
      Nmbr_Forums++;
      Topic_List++;
    }
    Topic_List = Old;

    // Inform the caller.
    *Max_Topic = Nmbr_Forums;

    // Let people know how many students there are.
    Text[0]='\0';
    #ifdef LC
    sprintf((char *)Text, " Learning Center Student Evaluations - %d Students", Nmbr_Forums);
    #endif
    #ifdef FSE
    sprintf((char *)Text, " First Semester Experience Evaluations - %d Students", Nmbr_Forums);
    #endif

    // Set up the display.
    Prompt = " Up and Down arrows to move, Enter selects, ESC to exit.";
    ScrClear(INFO_ROW,  1, 80, 1,  INFO_COLOR);
    ScrClear(PROMPT_ROW, 1, 80, 1,  PROMPT_COLOR);
    ScrPrintText(INFO_ROW, 1, 80, (char *)Text);
    ScrPrintText(INFO_ROW, 70, 10, "F1 = HELP");
    ScrPrintText(PROMPT_ROW, 1, 80, Prompt);

    // Display the information.
    Open_SBox(&Forums, BOX_ROW, 1, 80, (PROMPT_ROW-BOX_ROW>Nmbr_Forums) ? Nmbr_Forums + 2 : PROMPT_ROW-BOX_ROW, FORUM_COLOR, NULL);

    // Highlight the initial topic.
    ScrSetCursorPos (26,1);

    // Keep looping until the input is acceptable.
    do {
      if ((Topic_Number - *Top_Topic) > (PROMPT_ROW-BOX_ROW-3))
        *Top_Topic = Topic_Number - (PROMPT_ROW-BOX_ROW-3);
      else
        if (Topic_Number < *Top_Topic)
          *Top_Topic = Topic_Number;
      while ( ( (*Top_Topic + (PROMPT_ROW-BOX_ROW-3) ) > (*Max_Topic) ) && (*Top_Topic>1) )
        (*Top_Topic)--;

      // Add 'more' indicators if appropriate.
      Buffer[1] = '\0';
      Buffer[0] = (*Top_Topic > 1) ? '' : '³' ;
      ScrPrint(BOX_ROW+1, 1, 1, (Buffer[0]=='³'?SCR_WHITE:SCR_BROWN+SCR_BRIGHT), Buffer);
      Buffer[0] = (*Top_Topic+PROMPT_ROW-BOX_ROW-3<*Max_Topic) ? '' : '³' ;
      ScrPrint((*Max_Topic>PROMPT_ROW-BOX_ROW-2)?PROMPT_ROW-2:BOX_ROW+*Max_Topic
              ,1, 1,(Buffer[0]=='³'?SCR_WHITE:SCR_BROWN+SCR_BRIGHT), Buffer);

      Old = Topic_List;
      Row = BOX_ROW+1;
      Topic_List += (*Top_Topic)-1;
      while ((Topic_List->Topic_Description != NULL) &&(Row<PROMPT_ROW-1)) {
        if (*Top_Topic + Row - BOX_ROW - 1 == Topic_Number) {
          ScrPrint(Row, 2, 78, HIGHLIGHT_COLOR, Topic_List->Topic_Description,HIGHLIGHT_COLOR);
          Fill (Row-2,Topic_List->Topic_Description);
        }
        else {
          ScrPrint(Row,2,78, NORMAL_COLOR, Topic_List->Topic_Description);
          UnFill (Row-2,Topic_List->Topic_Description);
        }
        Row++;
        Topic_List++;
      }
      Topic_List = Old;

      Result = 0;

      // Handle any special keys coming back from ScrKey().
      Key = ScrKey();
      switch (Key) {

        case K_TAB:
            Blank_Screen();
            break;

        case K_DOWN:
            if (Topic_List[Topic_Number].Topic_Description != NULL)
              Topic_Number++;
          break;

        case K_UP  :
            if (Topic_Number > 1)
              Topic_Number--;
          break;

        case K_HOME :
            Topic_Number = 1;
          break;

        case K_END :
            Topic_Number = *Max_Topic;
          break;

        case K_PGDN:
          Topic_Number += PROMPT_ROW-BOX_ROW-2;
          *Top_Topic   += PROMPT_ROW-BOX_ROW-2;
          if(Topic_Number > *Max_Topic) Topic_Number = *Max_Topic;
          break;

        case K_PGUP:
          Topic_Number -= PROMPT_ROW-BOX_ROW-2;
          if(Topic_Number < 1) Topic_Number = 1;
          break;

        case K_RETURN:
           Result=1;
          break;

        case K_ESC:
        case K_ALTX:
        case K_F7:
          exit(0);
          break;

        case K_F1:
          Helper(TOPIC_LIST);
          break;
      }
    } while (Result!=1);
  return Topic_Number;
}


//
// Extract_MessageDate
//
// The following function takes the Date_String in a Message and figures out
//   what the computable date is. It installs the result into the same Message
//   under Posted_On.
//
void Extract_MessageDate(Message *The_Message)
  {
    char Day_Name[80+1];    // Sun, Mon, etc...
    char Month_Name[80+1];  // Jan, Feb, etc...
      // The above are larger than needed to protect against problems if
      //   a topic administrator accidently edits a date to the incorrect
      //   format. For example, "Thurs" has been used by accident instead
      //   of "Thu"

    static const Date Dummy = { 1, 1, 0, 0, 1992 };
      // Placeholder date/time: January 1, 0 hours 0 minutes (midnight) 1992.

    static const char *Month_Names[] = {
      "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
      NULL
      };
    const char **Search;

    // Make sure the fields are garbage in case the sscanf()s below
    //   can't read everything they've been asked to read.
    The_Message->Posted_On = Dummy;

    // If we have a new style message...
    if (strchr(The_Message->Date_String, ':') != NULL) {
      sscanf(The_Message->Date_String, "%s %s %d %d:%d %d", Day_Name, Month_Name,
        &The_Message->Posted_On.Day,
        &The_Message->Posted_On.Hour,
        &The_Message->Posted_On.Minute,
        &The_Message->Posted_On.Year      // Will be 1992 if no year provided.
        );
    }
    else {
      // We have an old style message (no hours and minutes).
      sscanf(The_Message->Date_String, "%s %s %d %d", Day_Name, Month_Name,
        &The_Message->Posted_On.Day,
        &The_Message->Posted_On.Year
        );
      // Let Hour and Minute be 0, 0 from the Dummy Message_Date object.
    }

    // Figure out the month number.
    Search = Month_Names;
    while (*Search != NULL) {
      if (strcmp(*Search, Month_Name) == 0) {
        The_Message->Posted_On.Month = (Search - Month_Names) + 1;
        break;
      }
      Search++;
    }
    // Let unknown months be handled as Jan from the Dummy Message_Data object.
  }


//
// Index_Topics
//
// The following function computes an index of the selected topic file.
//
int Index_Topics(char *File_Name, Message *Topic_Index, int Max_Size)
  {
    int   Count = 0;  /* Zero messages in this topic initially. */
    int   i;
    char *Wait_Message;
    int   Message_Size;
    FILE *Info;
    char  Line_Buffer[256+2];
    Simple_Window Teaser;

    // These objects are used for list reversal.
    Message Temp,
    *First_Message = Topic_Index,
    *Last_Message;

    // Erase the current index. This is important because the rest of the
    //   program depends on the last part of the index being zero. Note
    //   that only the Username field is significant in this regard. Note
    //   also that it is assumed that there is a Topic_Index[Max_Size]. This
    //   insures that even with Max_Size messages, there is a null entry.
    //   at the end.
    //
    //   All fields that have dynamic data associated with them are freed
    //   and set to NULL (so that a latter free will not screw up.)

    for (i = 0; i <= Max_Size; i++) {
      free(Topic_Index[i].Username);
      free(Topic_Index[i].Date_String);
      free(Topic_Index[i].Subject_Line);

      Topic_Index[i].Username = NULL;
      Topic_Index[i].Date_String = NULL;
      Topic_Index[i].Subject_Line = NULL;
    }

    // Tell the user what we're up to.
    Construct_SWin(&Teaser);

    Wait_Message = " Indexing Selected Topic File. Please Wait... ";
    Message_Size = strlen(Wait_Message);
    Open_SWin(&Teaser, 12, (80 - Message_Size)/2 + 1, Message_Size + 2, 3, SCR_BRIGHT|SCR_WHITE|SCR_REV_RED, 0);
    Print_SBox(&Teaser.Base_Object, 1, Wait_Message);

    // Open the specified topic file.
    Info = fopen(File_Name, "rb");
    if (Info == NULL) return 0;

    // Read every line of the topic file.
    while (fgets(Line_Buffer, 256+2, Info) != NULL) {

      char *End_Pntr;
      char *Token;

      // Skip this line if it's not a message header.
      if (Line_Buffer[0] != '|') continue;

      // Kill the '\n' at the end (if there is one).
      End_Pntr = strchr(Line_Buffer, '\n');
      if (End_Pntr != NULL) *End_Pntr = '\0';

      // The check below is unneccessary if the file is opened in text mode.
      //   Why is it being opened in binary mode, anyway?  --PCHAPIN.
      //   Because it used to be encrypted! --PCABBE
      /* Somebody forgot to check for carriage returns .... */
      /* Kill the '\r' at the end (if there is one). */
      End_Pntr = strchr(Line_Buffer, '\r');
      if (End_Pntr != NULL) *End_Pntr = '\0';

      // Break off the username part of the header and store it in the index.
      Token = strtok(Line_Buffer, "|");
      Topic_Index->Username = (char *)malloc(strlen(Token) + 1);
      strcpy(Topic_Index->Username, Token);

      // Break off the date part of the header and store it in the index.
      Token = strtok(NULL, "|");
      Topic_Index->Date_String = (char *)malloc(strlen(Token) + 1);
      strcpy(Topic_Index->Date_String, Token);
      Extract_MessageDate(Topic_Index);

      // Break off the subject line part of the header and store it in the index.
      Token = strtok(NULL, "|");
      Topic_Index->Subject_Line = (char *)malloc(strlen(Token) + 1);
      strcpy(Topic_Index->Subject_Line, Token);

      // Remember the file position for the start of the line right after the header.
      Topic_Index->FTell_Position = ftell(Info);

      Count++;
      Topic_Index++;

      // CHECK FOR EXCESSIVE NUMBER OF MESSAGES HERE!!!

    } // End of while loop that reads entire topic file.

    // Store the file position for EOF in empty slot after the last message.
    Topic_Index->FTell_Position = ftell(Info)+1;

    fclose(Info);

    /* Here is where we reverse the list of messages */
    /* i.e. Place the newest message at the top of the list. */

    /* Initialize outer markers */
    Last_Message = Topic_Index;

    /* Loop to reverse */
    while ((Last_Message - First_Message) > 0) {
      Temp = *First_Message;
      *First_Message = *Last_Message;
      *Last_Message = Temp;
      First_Message++;
      Last_Message--;
    };
    /* Done with reversal */

    Destroy_SWin(&Teaser);
    return Count;
  }

/*----------------------------------------------------------------------------
void Display_Index(Message *Message_List, int Top, int Active, char *Description, int Length);

The following function displays the selected index of messages. This
function completely redraws the screen. Every character position on the
screen is updated.
----------------------------------------------------------------------------*/

void Display_Index(
  Message    *Message_List,
  int         Top,
  int         Active,
  char       *Description,
  int         Length
  )
  {
    /* There is too much coupling between this function and Select_Message(). */
    /*   See the comment in Select_Message() for more information.            */

    /* Positions of things (zero based row numbers). */
    const int TOP_ROW    = 1;     /* First row containing message info. */
    const int BOTTOM_ROW = 23;    /* Last row containing message info. */
    const int INFO_BAR   = 0;     /* Row number where general information is put. */
    const int KEY_BAR    = 24;    /* Row number where keystroke info is put. */

    /* Colors. */
    const int INFO_COLOR       = SCR_BRIGHT | SCR_WHITE | SCR_REV_BLUE;
    const int KEY_COLOR        = SCR_BRIGHT | SCR_WHITE | SCR_REV_BLUE;
    const int KEY_DESC_COLOR   = SCR_WHITE  | SCR_REV_BLUE;
    const int SP_USER_COLOR    = SCR_BRIGHT | SCR_WHITE;
    const int USER_COLOR       = SCR_WHITE;
    const int SP_DATE_COLOR    = SCR_BRIGHT | SCR_BROWN;
    const int DATE_COLOR       = SCR_WHITE;
    const int SP_SUBJECT_COLOR = SCR_BRIGHT | SCR_BROWN;
    const int SUBJECT_COLOR    = SCR_WHITE;
    const int SELECTED_COLOR   = SCR_BLACK  | SCR_REV_BROWN;
    const int ARROW_COLOR      = SCR_BRIGHT | SCR_BROWN;

    int Current = Top;            /* Index into Message_List of first message to display. */

    char Row_Buffer[160];         /* Holds image of what will be put onto the screen.  */
    char Row_Text[128+1];         /* Holds text of the current row.                    */

    int Row;

    /* Hide the cursor. */
    ScrSetCursorPos (26,1);

    /* Print the top line. */
    ScrClear(INFO_BAR+1, 1, 80, 1, INFO_COLOR);
    sprintf(Row_Text, "%s (%d Entries)", Description, Length);
    ScrPrintText(INFO_BAR+1, 1, 60, Row_Text);
    ScrPrintText(INFO_BAR+1, 70, 10, "F1 = HELP");

    /* For each row on the display... */
    for (Row = TOP_ROW; Row <= BOTTOM_ROW; Row++, Current++) {

      /* Make sure the row is blank. */
      Initialize_Row(Row_Buffer);

      /* If this row has a message on it... */
      if (Current < MAX_MESSAGES && Message_List[Current].Username != NULL) {

        /* Compute the text of the message and put it into the row's image. */
        sprintf(Row_Text, " %-8s³", Message_List[Current].Username);
        strncat(Row_Text, Message_List[Current].Date_String, 16);
        strcat (Row_Text, "³");
        strcat (Row_Text, Message_List[Current].Subject_Line);
        Spread_String(Row_Text, Row_Buffer);

        /* Add 'more' indicators if appropriate. */
        if (Row == TOP_ROW && Current != 1) {
          Row_Buffer[0] = '';
          Row_Buffer[1] = ARROW_COLOR;
        }
        if (Row == BOTTOM_ROW && Message_List[Current + 1].Username != NULL) {
          Row_Buffer[0] = '';
          Row_Buffer[1] = ARROW_COLOR;
        }

        /* Set colors for the rest of the row. */
        if (Current == Active) {
          Color_Section(Row_Buffer, 1, 79, SELECTED_COLOR);
        }
        else {
          if ((strcmp(Message_List[Current].Username, "NOTEBOOK") ) &&
              (strcmp(Message_List[Current].Username, "MODERATR") )    )
          {
            Color_Section(Row_Buffer,  1,  8, USER_COLOR);
            Color_Section(Row_Buffer,  9, 16, DATE_COLOR);
            Color_Section(Row_Buffer, 25, 55, SUBJECT_COLOR);
          }
          else
          {
            Color_Section(Row_Buffer,  1,  8, SP_USER_COLOR);
            Color_Section(Row_Buffer,  9, 16, SP_DATE_COLOR);
            Color_Section(Row_Buffer, 25, 55, SP_SUBJECT_COLOR);
          }
        }
      }

      /* Print the row (may be blank). */
      ScrWrite(Row + 1, 1, 80, 1, Row_Buffer);
    }

    /* Print the bottom line. */
    sprintf(Row_Text, "&=Select HOME=First entry END=Last entry ENTER=View entry INS=Add entry");
    Initialize_Row(Row_Buffer);
    Spread_String(Row_Text, Row_Buffer);
    Color_Section(Row_Buffer,  0,   80, KEY_DESC_COLOR);
    Color_Section(Row_Buffer,  0,   3,  KEY_COLOR);
    Color_Section(Row_Buffer,  11,  4,  KEY_COLOR);
    Color_Section(Row_Buffer,  28,  3,  KEY_COLOR);
    Color_Section(Row_Buffer,  43,  5,  KEY_COLOR);
    Color_Section(Row_Buffer,  60,  3,  KEY_COLOR);
    ScrWrite(KEY_BAR+1, 1, 80, 1, Row_Buffer);
  }


/*----------------------------------------------------------------------------
int View(char *File_Name, Message *Message_List, int Active);

The following function is used to view a selected message. Note that this
function copies the text of the selected message into a temporary file so
that it will be available when the user tries to post.

This function also loads the text of the message into a dynamically
allocated array (which passed to a subordinate function for display). Thus
the entire message must fit into a dynamically allocated chunk of memory.
----------------------------------------------------------------------------*/

int View(
  Topic_File *The_Topic,
  Message    *Message_List,
  int Active
  )
  {
    FILE  *Info;
    FILE  *Temp;
    size_t Message_Size;
    char  *Msg_Text;
    int    Index;
    char   Line_Buffer[256+2], Out_Buffer[257+2];
    int    Counter;

    if (Message_List[Active].Username == NULL) return Active;

    /* Try to open the topic file. Give up if it doesn't work. */
    Info = fopen(The_Topic->File_Name, "rb");
    if (Info == NULL) return Active;

    /* Try to open the temp file. This is so the text of this message can    */
    /*   be saved in case the user decides to post immediately after reading */
    /*   this message.                                                       */
    Temp = fopen(TEMP_FILE, "wt");
    if (Temp == NULL) {
      ScrPrint (25, 30, 50, SCR_WHITE|SCR_BRIGHT|SCR_BLINK, "Can't open Temp file. Message not saved to disk.  ");

      /* Delay for 10 seconds to let the user know that we had a small
         problem. */
      Delay(10);
    }

    /* Compute the amount of memory required to hold message and allocate it. */
    /* Note that this allocates memory to hold the header of the next message */
    /*   even though it's not actually loaded into memory.                    */
    Message_Size = (size_t) (
      Message_List[Active - 1].FTell_Position -
      Message_List[Active - 0].FTell_Position
      );

    if ((Msg_Text = (char *)malloc(Message_Size))==NULL){
      ScrPrint (25, 30, 50, SCR_WHITE|SCR_BRIGHT|SCR_BLINK, "Insufficent Memory to View Message!");
      return Active;
      }

    /* Position the topic file at the start of the selected message. */
    fseek(Info, Message_List[Active].FTell_Position, SEEK_SET);

    /* Print a header for this message into the temporary file. */
    sprintf(Out_Buffer, ">%8s  %s  %s",
      Message_List[Active].Username,
      Message_List[Active].Date_String,
      Message_List[Active].Subject_Line
      );
    fprintf(Temp, "%s\n", Out_Buffer);
    fprintf(Temp, ">");
    for (Counter = 0; Counter < strlen(Out_Buffer) - 1; Counter++) fprintf(Temp, "=");
    fprintf(Temp, "\n");

    Index = 0;   /* How far into dynamically allocated array are we? */

    /* Read every line of the message. */
    while (fgets(Line_Buffer, 256+2, Info) != NULL) {

      if (Line_Buffer[0] == '|') break;

      /* Put this line into the dynamically allocated array. */
      strcpy(&Msg_Text[Index], Line_Buffer);
      Index += strlen(Line_Buffer);

      /* Put this line into the temporary file as well. */
      strcpy (&Out_Buffer[1], Line_Buffer);
      Out_Buffer[0] = '>';
      fputs(Out_Buffer, Temp);
      if (Index >= Message_Size) break;
    }

    /* Make sure dynamic array is null terminated. */
    Msg_Text[Index] = '\000';

    /* Write footer into temporary file. */
    fprintf (Temp, ">\n>[-END OF EVALUATION/RESPONSE BY %s-]",Message_List[Active].Username);

    fclose(Info);
    fclose(Temp);

    /* Display the message. Return message number of next message. */
    Active = Display_Message(Msg_Text, Message_List, Active, MAX_MESSAGES);
    free(Msg_Text);

    // Update the topic information to indicate that we've (maybe) read
    //  a more recent message on this topic than we have so far.
    if (Date_Greater(&Message_List[Active].Posted_On, &The_Topic->Read_On))
      The_Topic->Read_On = Message_List[Active].Posted_On;

    return Active;
  }


//
// Unicode_Failure
//
// The following function displays a good quality error message if the Unicode
//   tables fail to initialize.
//
void Unicode_Failure(int ccode)
  {
    struct Error_Info {
      int   Code;
      char *Message;
    };
    Error_Info Conditions[] = {
      { UNI_LOAD_FAILED,    "UNI_LOAD_FAILED"    },
      { UNI_NO_MEMORY,      "UNI_NO_MEMORY"      },
      { UNI_NO_PERMISSION,  "UNI_NO_PERMISSION"  },
      { UNI_TOO_MANY_FILES, "UNI_TOO_MANY_FILES" },
      { UNI_NO_SUCH_FILE,   "UNI_NO_SUCH_FILE"   },
      { UNI_FUTURE_OPCODE,  "UNI_FUTURE_OPCODE"  },
      { UNI_OPEN_FAILED,    "UNI_OPEN_FAILED"    },
      { UNI_RULES_CORRUPT,  "UNI_RULES_CORRUPT"  },
      { 0,                  0                    }
    };

    Error_Info *p = Conditions;
    while (p->Code != 0) {
      if (p->Code == ccode) {
        printf("\nERROR! Unable to initalize unicode tables: %s\n", p->Message);
        return;
      }
      p++;
    }

    printf("\nERROR! Unable to initalize unicode tables: unknown ccode of %04X\n", ccode);
  }


//
// Get_Username
//
// The following function figures out what a user's name is by querying the NDS.
//   The given buffer is assumed to be large enough to hold the answer.
//
void Get_Username(char *Buffer)
  {
    NWDSContextHandle   Context_Handle;   
    NWCONN_ID           ccode;
    char                Context[257], Object_Name[MAX_DN_CHARS];   

    strcpy(Buffer, "Unknown");

    // Initialize the NetWare library before you try to use it.
    if (NWCallsInit(NULL, NULL) != 0) {
      printf("\nERROR! Unable to initialize the NetWare Library.\n");
      return;
    }

    // Who are we? Don't really need to find the default context, but what the Hell?
    ccode = NWGetDefaultNameContext(sizeof(Context), (BYTE NWFAR *) Context);

    // Must call NWInitUnicodeTables() before calling NWDSCreateContext().
    ccode = NWInitUnicodeTables(1, 437);
    if (ccode != 0) Unicode_Failure(ccode);
    else {
      Context_Handle = NWDSCreateContext();
      if (Context_Handle == ERR_CONTEXT_CREATION)
        printf("\nERROR! Error return from NWDSCreateContext()\n");
      else {
        ccode = NWDSWhoAmI(Context_Handle, Object_Name);
        if (ccode != 0)
          printf("\nERROR! Error return from NWDSWhoAmI()\n");
        else
          strcpy(Buffer, &Object_Name[3]);
      }
    }
  }


//
// Add
//
// The following function lets the user post a message to the topic file
//   specified by the parameter. This function handles all aspects of the
//   posting process from letting the user compose the message (using Y) to
//   opening the topic file to appending the posting to the end of the topic
//   file -- complete with header.
//
void Add(Topic_File *The_Topic)
  {
    char   Subject_Line[80+1];
    char   Line_Buffer[256+2];
    FILE  *Temp;
    FILE  *Info;
    char   Date_String[80+1];
    time_t Today;
    struct tm *Expanded_Time;
    char   ch;
    int    ch_Acceptable;
    char   Header[256+2];
    int    Index;

    Simple_Box Options;
    Simple_Box Wait;

    int key;
    char return_value[2];
    int size = -1;

    int ExitKeys[] = { K_ESC, 'R', 'r', 'N', 'n', 0 };

    // Let the user create the a file containing the posting. If the user
    //   has looked at a message, this will allow the user to edit the
    //   text of that message (since the text of each message read is
    //   saved in TEMP_FILE.

    key = ScrEnter(
       /* Prompt_Text => */ "Is this a (N)ew evaluation or a (R)esponse to the last one you read ? ",
       /* Promt_Attr  => */ SCR_CYAN,
       /* Row, Column => */ 13, 5,
       /* Box_Attr    => */ SCR_BRIGHT|SCR_WHITE,
       /* Text        => */ return_value,
       /* Default_Text=> */ "",
       /* Text_Length => */ &size,
       /* Text_Attr   => */ SCR_BRIGHT|SCR_BROWN,
       /* Exit_Keys   => */ ExitKeys
     );

    if (key == K_ESC) return;

    key = toupper(key);

    if (key == 'N')
    {
      remove(TEMP_FILE);
      Temp = fopen(TEMP_FILE, "wt");
      if (Temp == NULL)
      {
        ScrClear(1, 1, 80, 25, SCR_WHITE);
        ScrPrint(13, 8, 80, SCR_WHITE|SCR_BRIGHT, "Error: Unable to write to " TEMP_FILE " - Cannot post evaluation!");
        ScrPrintText(15, 33, 80, "Press any key.");
        ScrKey();
        return;
      }
      else
      {
        #if LC
        fputs("Evaluation:\n\n\n\nRecommendation:", Temp);
        #else
        fputs("Attendance:\n\n\n\nHomework:\n\n\n\nQuiz, lab, and test grades:\n\n\n\nAttitude:\n\n\n\nMisc. (residence life, athletics, kudos, etc):\n", Temp);
        #endif
        fclose(Temp);
      }
    }

    Open_SBox(&Wait, 12, 23, 32, 3, SCR_BRIGHT|SCR_WHITE|SCR_REV_RED, NULL);
    Print_SBox(&Wait, 1, " Accessing External Editor ... ");

    system("Y " TEMP_FILE);

    /* Loop letting the user do whatever they want. */
    do {
      ch_Acceptable = 0;

      ScrClear(1, 1, 80, 25, SCR_WHITE);
      ScrSetCursorPos(26, 1);

      Open_SBox(&Options, 5, 5, 50, 9, SCR_WHITE|SCR_REV_BLACK,"Your Options : ");
      Print_SBox(&Options, 2, "(P)ost the Message.");
      Print_SBox(&Options, 4, "(E)dit the Message some more.");
      Print_SBox(&Options, 6, "(A)bort Without posting the Message.");
      Print_SBox(&Options, 8, "Type the Letter of your Choice :  ");

      ch = toupper(ScrKey());

      // Display this reponse in the right place.
      char Work_Buffer[2];
      Work_Buffer[0] = ch;
      Work_Buffer[1] = '\0';
      Print_SBox(&Options, 8, 34, Work_Buffer);
      ScrRefresh();

      switch (ch) {
        case 'E': system("Y " TEMP_FILE); break;
        case 'P':
        case 'A':
          ch_Acceptable = 1;
          break;
        default :
          break;
      }
    } while (!ch_Acceptable);

    ScrClear(1, 1, 80, 25, SCR_WHITE);

    // If the user doesn't want to post it, get out.
    if (ch == 'A') return;

    // Try to open the file which contains the user's posting. Give up if we can't.
    Temp = fopen(TEMP_FILE, "r");
    if (Temp == NULL) return;

    // Get the subject line.
    do {
      ScrClear(1, 1, 80, 25, SCR_WHITE);
      ScrSetCursorPos (13,1);

      for(Index=0; Index<82; Index++) Subject_Line[Index]='\000';

      if (key == 'N') {
        #ifdef LC
        printf("\nSubject(s) tutored in: ");
        #endif
        #ifdef FSE
        printf("\nHeading: ");
        #endif
      }
      else
        printf("Subject line: ");
      gets(Subject_Line);
      if (strlen(Subject_Line) == 0)
      {
        printf ("You must enter the subject(s) tutored!\a\n");
        printf ("Press a key");
        ScrKey();
      }
    }
    while (strlen(Subject_Line) == 0);

    // Try to open the topic file. Give up if we can't.
    Info = fopen(The_Topic->File_Name, "ab");
    if (Info == NULL) {
      fclose(Temp);
      return;
    }

    Today = time(NULL);
    Expanded_Time = localtime(&Today);
    strftime(Date_String, 80, "%a %b %d %H:%M %Y", Expanded_Time);

    sprintf(Header, "\n|%s|%s|%s\n", Username, Date_String, Subject_Line);
    fprintf(Info, Header);

    // Place the Message into the Topic File.
    while (fgets(Line_Buffer, 256+2, Temp) != NULL) {
      if (Line_Buffer[0] == '|') putc(' ', Info);
      fputs(Line_Buffer, Info);
    }
    fclose(Info);
    fclose(Temp);

    // Put the time information into the proper topic (posting a message
    //   is like reading it. We don't want to force people to read messages
    //   they just posted.
    The_Topic->Read_On.Month  = Expanded_Time->tm_mon+1;
    The_Topic->Read_On.Day    = Expanded_Time->tm_mday;
    The_Topic->Read_On.Hour   = Expanded_Time->tm_hour;
    The_Topic->Read_On.Minute = Expanded_Time->tm_min;
    The_Topic->Read_On.Year   = Expanded_Time->tm_year+1900;

    // Now write a .TIM file that contains this information for NNCHK to
    //   use when it scans for new postings.
    //Write_TIMFile(The_Topic, Expanded_Time);

    ScrClear(1,1,80,25,SCR_WHITE);
  }

/*----------------------------------------------------------------------------
int Select_Message(
  int Topic_Number,    Index into Topic_List of this topic.
                         This function knows the relative position of
                         this topic on the topic list.
  int Max_Topic,       Topic_Number of the last topic.
  int Size             Number of messages in this topic.
  );

The following function displays the topic index.
----------------------------------------------------------------------------*/

int Select_Message(Topic_File *Topic_List, int Topic_Number, int Max_Topic, int Size)
  {
    /* Select_Message() should NOT have to worry about screen size. There */
    /*   is too much coupling between this function and Display_Index().  */
    /*   Another result of the extra coupling: The top and bottom lines   */
    /*   on the message index display tend to flicker.                    */
    const int SCREEN_SIZE = 23; /* The number of messages that can be displayed at once. */

    Highlight_Positions Bar;

    Construct_Highlight(&Bar, SCREEN_SIZE, Topic_Index);

    for (;;) {
      /* Display the index nicely. */
      Display_Index(
         Topic_Index,
         Top_Highlight(&Bar),
         Active_Highlight(&Bar),
         Topic_List[Topic_Number - 1].Topic_Description,
         Size
       );

      /* Get and handle each user's keystroke. */
      switch (ScrKey()) {

        case K_TAB:
          Blank_Screen();
          break;

        case K_F1:
          Helper(MESSAGE_LIST);
          break;

        /* Quit, but return the final topic number (users can change Topic_Number while in this function). */
        case K_ESC:
        case K_ALTX:
        case K_F7:
//          Check_Topic(&(Topic_List[Topic_Number-1]));
          return Topic_Number;

        /* Back up to the previous topic. */
        case K_CPGUP:
          if (Topic_Number > 1)
          {
//            Check_Topic(&(Topic_List[Topic_Number-1]));
            return Topic_Number - 1;
          }
          break;

        /* Advance to the next topic. */
        case K_CPGDN:
          if (Topic_Number < Max_Topic)
          {
//            Check_Topic(&(Topic_List[Topic_Number-1]));
            return Topic_Number + 1;
          }
          break;

        /* Handle movement of the highlighted bar. */
        case K_DOWN: Down_Highlight(&Bar);      break;
        case K_UP  : Up_Highlight(&Bar);        break;
        case K_PGDN: Page_Down_Highlight(&Bar); break;
        case K_PGUP: Page_Up_Highlight(&Bar);   break;
        case K_HOME: Home_Highlight(&Bar);      break;
        case K_END : End_Highlight(&Bar);       break;

        case K_RETURN: {
            int Old_Active;
            do {
              Old_Active = Active_Highlight(&Bar);
              Make_Active_Highlight(&Bar,
                View(&Topic_List[Topic_Number - 1], Topic_Index, Active_Highlight(&Bar))
              );
            } while (Active_Highlight(&Bar) != Old_Active);
          }
          break;

        case K_INS: {
            int Current_Message = Size - Active_Highlight(&Bar);
            Add(&Topic_List[Topic_Number - 1]);
            Size = Index_Topics(Topic_List[Topic_Number - 1].File_Name, Topic_Index, MAX_MESSAGES);
            Make_Active_Highlight(&Bar, Size - Current_Message);
            ScrClear(1, 1, 80, 25, SCR_WHITE);
          }
          break;
      }
    }
  }


//
// MAIN PROGRAM
//
// The following function merely sets the introduction screen and puts the
//   software into an infinite loop (while(1)) that allows the user to choose
//   what if anything they want to do.
//
int main(void)
  {
    int         Topic_Size;
    int         Num_Topics   = 0;
    int         Topic_Number = 1;
    int         Top_Topic    = 1;
    Topic_File *Topic_List   = Get_First();
    char        cmdline[128+1];

    atexit(Credits);

    // Learn what the user's name is.
    Get_Username(Username);

    // If we didn't get the user's name from the NDS, ask the user.
    if (strcmp(Username, "Unknown") == 0) {
      printf("A problem occured when trying to get your username from the server.\n");
      printf("Please record the error message above and send it to Charlie Castelli\n");
      printf("so that we can try and correct the problem. In the meantime, enter\n");
      printf("your username at the prompt below.\n\n");

      printf("Username: ");
      gets(Username);
    }

    ScrInitialize(0, 0);
    ScrRefresh_OnKey(true);
    Initialize();

    while (1) {
      Topic_Number = Choose_Topic(Topic_List, &Num_Topics, Topic_Number, &Top_Topic);
      if (Topic_List[Topic_Number-1].File_Name[0] != '+') {
        int Topic_Tmp;
        do {
          Topic_Tmp = Topic_Number;
          Topic_Size = Index_Topics(Topic_List[Topic_Number - 1].File_Name, Topic_Index, MAX_MESSAGES);
          ScrClear(1,1,80,25,SCR_WHITE);
          Topic_Number = Select_Message(Topic_List, Topic_Number, Num_Topics, Topic_Size);
        } while ((Topic_Tmp != Topic_Number) && (Topic_List[Topic_Number-1].File_Name[0] != '+'));
      }
      else
      {
        sprintf (cmdline, "LIST %s", Topic_List[Topic_Number-1].File_Name+1);
        ScrClear (1,1,80,25,SCR_WHITE);
        system (cmdline);
        ScrClear (1,1,80,25,SCR_WHITE);
      }
    }

    // The following is commented out to avoid a warning. This code is
    //   never executed due to the infinite loop above.
    // return 0;
  }
