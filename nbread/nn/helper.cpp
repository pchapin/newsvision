/*****************************************************************************
File         : HELPER.C
Programmer   : VTC^3, Paul Cabbe
Date         : ??/??/??
Last Revised : 1/13/93

Purpose      : This file contains the help text for use with the Network
               Notebook Software.  Best Guess says PCHAPIN wrote this.

*****************************************************************************/

#include "environ.h"
#include <string.h>

#include "standard.h"
#include "helper.h"
#include "portscr.h"
#include "sbox.h"

/* The following arrays define what the help text is. Note that this */
/*   material can only be reached through an object of type Helper.  */
/*   This design allows future versions of Helper to take help text  */
/*   from a file, etc.                                               */

static char *Topic_List[] = {
  " Active Keys : ",
  " Up Arrow    ==>  Move up one student ",
  " Down Arrow  ==>  Move down one student ",
  " Page Up     ==>  Move up 1 screenful of students ",
  " Page Down   ==>  Move down 1 screenful of students ",
  " Home, End   ==>  Move to top and bottom of the list, respectively ",
  " ESC,F7      ==>  Exit the program ",
  " Enter       ==>  Select student whose records to access ",
  " ",
  " This screen lists the students who are currently being evaluated. ",
  " ",
  " Highlight the student you want, then press enter to access his/her ",
  " tutoring records. ",
  " ",
  " If corrections need to made to this list, contact Charlie Castelli. ",
  0
};

static char *Message_List[] = {
  " Active Keys : ",
  " Up Arrow    ==>  Move up one entry ",
  " Down Arrow  ==>  Move down one entry ",
  " Page Up     ==>  Move up 1 screen of entries ",
  " Page Down   ==>  Move down 1 screen of entries ",
  " Home, End   ==>  Move to top and bottom of the list, respectively ",
  " ESC,F7      ==>  Return to the list of students ",
  " Enter       ==>  Read an evaluation ",
  " *** Insert  ==>  Add an evaluation or reply ",
  " ",
  " *** Note: When you press Insert you will be asked if you are entering ",
  "     a new evaluation or responding to the last one you read. If you ",
  "     press 'N' for 'New' you will be given an empty evaluation form ",
  "     with space for an evaluation and a recommendation. If you answer ",
  "     'R' for 'Response' the text of the last evaluation or reply you ",
  "     read will be placed into your message for you to refer to. ",
  " ",
  " Important keys in the Y text editor: ",
  " Alt-X: Exit & Save    F6: Cut Line    F7: Paste Line    SHIFT-F1: Help ",
  0
};

static char *Message_Display[] = {
  " This screen shows you the text of the message you selected.",
  " Active Keys :  ESC ==> Return to List of Messages.",
  " ",
  " If the message is more than one screen, you can scroll through the ",
  " message with the arrow keys or the PgUp and PgDn keys.",
  " ",
  " Cntrl+PgUp and Cntrl+PgDn will move you to the previous or next message",
  " respectively. You don't have to return to the message list to switch to",
  " another message.",
  0
};

/* Define Helper's static data. */
static char **Help_Text[3] = { Topic_List, Message_List, Message_Display };

extern void Blank_Screen(void);

void Helper(Help_Topic Topic)
  {
    char **Line;
    char   ch;
    int    Max_Length  = 0;
    int    Number      = 0;
    int    Line_Number = 1;
    Simple_Window Help_Display;

    Construct_SWin(&Help_Display);

    /* Figure out how large Help_Display must be... */
    for (Line = Help_Text[Topic]; *Line != 0; Line++) {
      if (strlen(*Line) > Max_Length) Max_Length = strlen(*Line);
      Number++;
    }
    Max_Length++;

    /* Open the help screen appropriately. */
    Open_SWin(&Help_Display,
      (25-(Number+2))/2 + 1,    /* Row for top of help box.           */
      (80-(Max_Length+2))/2 + 1,/* Column for left edge of help box.  */
      Max_Length+2,             /* Width of box (allow for borders).  */
      Number+2,                 /* Height of box (allow for borders). */
      SCR_BLACK | SCR_REV_WHITE,
      "ESC to Exit Help"        /* Box title.                         */
      );

    /* Print the help text into Help_Display. */
    for (Line = Help_Text[Topic]; *Line != 0; Line++, Line_Number++) {
      Print_SBox(&Help_Display.Base_Object, Line_Number, *Line);
    }

    while ((ch = ScrKey()) != K_ESC)
      if (ch == K_TAB) Blank_Screen();

    Destroy_SWin(&Help_Display);
  }

