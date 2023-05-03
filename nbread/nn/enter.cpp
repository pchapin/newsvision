/****************************************************************************
FILE          : ENTER.C
PROGRAMMER    : Paul Cabbe
LAST MODIFIED : November, 1991

This file makes use of the SCR module and ScrKey() functions by Peter Chapin
to provide a powerful user configurable input routine.

This function accepts parameters (quite a lot of them) to define coordinates
at which to do the inputting, a prompt and its color, colors for a
surrounding box, a color for the entered text and a default text if the
user doesn't enter any, a maximum length for the user's text, and a list of
keys which when pressed, will cause the function to exit immediately.

The function save the text which it overwrites, which it restores afterwards,
and also the cursor position, which is restored after termination. In general,
this function is designed to do little (if any) damage to the current
operating conditions and screen appearance.

NOTE: This function, like the SCR package, does little if any error checking.
      If you pass it erroneous parameters, the results are unspecified.

Parameter descriptions:

char *Prompt_Text  : The text of the prompt
int Prompt_Attr    : The SCR attributes used to display the prompt
int Row            : The row on which the prolpt is placed
int Column         : The column at which the prompt begins
int Box_Attr       : If a box (single line border) is to surround the prompt
                     and user's text, then this contains the SCR attributes of
                     the box (If Box_Attr equal to NO_BOX, then no box is
                     provided.
char *Text         : The user's text, as inputted
char *Default_Text : The text returned if the user does not provide any
int *Text_Length   : Value passed is the maximum length of the input string.
                     Value returned is the length of Text.
int Text_Attr      : SCR attribute used when printing the user's text
                     (Note: the user's text is printed starting 1 space to the
                     right of the end of the prompt )
int *Exit_Keys     : Points to a null-terminated array of ScrKey() key
                     constants, which, when entered by the user, cause control
                     to immediately return to the caller, with that key value
                     being the value returned.

****************************************************************************/

#include "environ.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "standard.h"
#include "enter.h"
#include "portscr.h"
#include "scrbox.h"

/***************************************************************************/

int In (int Value, int *Possible)
/*
This function checks a 0 terminated array of integers to see if any match
Value. If any does, it immediately returns 1, otherwise it returns 0. For
those of you familiar with Pascal, this is a integer-only duplicate of the
Pascal function of the same name.
*/
{
  while (*Possible) {
    if (Value == *Possible)
      return 1;
    Possible++;
  }

  return 0;
}

/***************************************************************************/

/* Define an enumeration for the typing mode the user is in */
enum Ins_Mode {
  Insert,
  Replace
};

/* At last: THE FUNCTION */
int ScrEnter(char *Prompt_Text, int Prompt_Attr, int Row, int Column,
              int Box_Attr, char *Text, char *Default_Text, int *Text_Length,
              int Text_Attr, int *Exit_Keys)
{
  int
    ch,                                  /* character just input            */
    Pos,                                 /* position in Text                */
    Index,                               /* index for shuffling elements    */
    Buffer_Size,                         /* size of the screen saving buffer*/
    Prompt_Length = strlen(Prompt_Text), /* length of the prompt            */
    Text_Start = Column+Prompt_Length+1, /* column text input starts at     */
    Text_Max = *Text_Length,             /* local variable for Text_Length  */
    Cur_Row, Cur_Col,                    /* old cursor coordinates          */
    Tmp = (Text_Max<0) ? 0 : 1,         /* for people who just want a key  */
    No_Prompt = Text_Max == 0;           /* for people who just want a box  */
  char
    *Save_Buffer;                        /* area for storing the space used */
  enum Ins_Mode
    Mode = Insert;                       /* what mode the user is in        */

  if (!Tmp) Text_Max = 0;
  if (!Text_Max) Text_Start -= 1;

  /* Store the current cursor position */
  ScrGetCursorPos (&Cur_Row, &Cur_Col);

  if (Box_Attr == NO_BOX) {
    /* Only save area used by prompt and text */
    Buffer_Size = Prompt_Length+Text_Max+2;
    Buffer_Size *= 2;
    Save_Buffer = (char *) malloc (Buffer_Size*sizeof(char));
    ScrRead (Row, Column, Prompt_Length+Text_Max+2, 1, Save_Buffer);
  }
  else {
    /* save whole area */
    Buffer_Size = (Prompt_Length+Text_Max+4) * 3;
    Buffer_Size *= 2;
    Save_Buffer = (char *) malloc (Buffer_Size*sizeof(char));
    ScrRead (Row-1, Column-1, Prompt_Length+Text_Max+4, 3, Save_Buffer);
    /* Draw the box if requested */
    ScrBox (Row-1, Column-1, Row+1, Column + Prompt_Length + Text_Max +
           1 - 2*No_Prompt + Tmp, Box_Attr);
  }

  /* Put the prompt where it belongs */
  ScrPrint (Row, Column, Prompt_Length, Prompt_Attr, Prompt_Text);

  /* Put up the default text */
  if (Text_Max) {
    ScrClear (Row, Text_Start - 1, Text_Max + Tmp + 1, 1, Text_Attr);
    ScrPrintText (Row, Text_Start, Text_Max, Default_Text);
  }

  /* Put the cursor at the end of the default text */
  Pos = strlen (Default_Text);
  if (Pos > Text_Max)
    Pos = Text_Max;
  ScrSetCursorPos(Row, Text_Start+Pos);

  if (!No_Prompt) {
  /* Initialize the array to 0's */
  for (Pos = 0; Pos <= Text_Max; Text[Pos++] = '\000');
  Pos = -1;

  /* Do the actual input of the string */
//  if (Mode == Replace)
//    ScrSetCursorSize (1,8);
//  else
//    ScrSetCursorSize (7,8);
//
// ScrSetCursorSize() is no longer supported in portscr, but it may be added
//   again before too long.

  do {
    ch = ScrKey();
    if (Pos == -1) {
      Pos = 0;
    }
    switch (ch) {
      case K_RETURN: case K_UP: case K_DOWN:
        /* Ignore these keys */
        break;
      case K_LEFT:
        if (Pos > 0)
          Pos--;
        break;
      case K_RIGHT:
        if (Pos < strlen(Text))
          Pos++;
        break;
      case K_CRIGHT:
        while ((!isspace(Text[Pos])) && (Pos < strlen(Text)))
          Pos++;
        while ((isspace(Text[Pos])) && (Pos < strlen(Text)))
          Pos++;
        break;
      case K_CLEFT:
        if (Pos > 0)
          Pos--;
        while ((isspace(Text[Pos])) && (Pos > 0))
          Pos--;
        while ((!isspace(Text[Pos])) && (Pos > 0))
          Pos--;
        if (Pos > 0)
          Pos++;
        break;
      case K_HOME:
        Pos = 0;
        break;
      case K_END:
        Pos = strlen(Text);
        break;
      case K_INS:
        if (Mode == Insert) {
          Mode = Replace;
          // ScrSetCursorSize (1,8);
        }
        else {
          Mode = Insert;
          // ScrSetCursorSize (7,8);
        }
        break;
      case K_BACKSPACE:
        if (Pos == 0)
          break;
        Pos--;
      case K_DEL:
        for (Index = Pos; Index < Text_Max; Text[Index++] = Text[Index+1]);
        Text[Text_Max] = '\000';
        break;
      default:
        if ((!isprint(ch)) || (In (ch,Exit_Keys)))
          break;
        if (Mode == Insert) {
          if (strlen(Text) == Text_Max)
            break;
          for (Index = Text_Max-1; Index > Pos; Index--)
            Text[Index] = Text[Index-1];
        }
        if (Pos < Text_Max)
          Text[Pos++] = ch;
        break;
    }

    /* Clear entry area, show current text, and put the cursor at its end */
    ScrClear (Row, Text_Start-1, Text_Max + Tmp + 1, 1, Text_Attr);
    ScrPrintText (Row, Text_Start, strlen(Text), Text);
    ScrSetCursorPos(Row, Text_Start+Pos);
  } while ((ch != K_RETURN) && (!In (ch, Exit_Keys)));


  /* Put back onto the screen what was there before */
  if (Box_Attr == NO_BOX)
    ScrWrite (Row, Column, Prompt_Length+Text_Max+2, 1, Save_Buffer);
  else
    ScrWrite (Row-1, Column-1, Prompt_Length+Text_Max+4, 3, Save_Buffer);

  free (Save_Buffer);

  /* Clean up the string by truncating trailing spaces */
  if (Tmp) while (Text[--Text_Max] == ' ')
    Text[Text_Max] = '\000';
  if (strlen(Text) == 0)
    strcpy (Text, Default_Text);
  *Text_Length = strlen(Text);

  }

  /* Restore the original cursor position */
  ScrSetCursorPos (Cur_Row, Cur_Col);
//  ScrSetCursorSize (7,8);
  return ch;
}
