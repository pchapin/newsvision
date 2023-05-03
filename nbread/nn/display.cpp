/*****************************************************************************
FILE          : DISPLAY.C
LAST REVISION : May 1997
PROGRAMMER    : VTC^3 and Peter Chapin

This file contains the function which is responsible for displaying a
message. It takes a linear array containing the text of the message. The
array must be null terminated, with each line from the message being
separated from other lines by a '\n' character.

This function also prints information about this message using the list
of messages currently active. It returns a number indicating the next
message the user wants to see. If the user isn't interested in seeing
any other message, it returns the same thing it's given.

*****************************************************************************/

#include "environ.h"
#include <string.h>
#include <stdio.h>
#include "standard.h"
#include "display.h"
#include "helper.h"
#include "message.h"
#include "portscr.h"

/* Position of the major window (one based). */
#define TOP_LINE      2
#define BOTTOM_LINE   25
#define WINDOW_WIDTH  78
#define WINDOW_HEIGHT (BOTTOM_LINE - TOP_LINE - 1)

/* Positions of various things (one based). */
#define INFO_ROW     1

extern void Blank_Screen(void);

int Display_Message(char *Text, Message *Topic_Index, int Current, int Max)
{

  /* Colors. */
  const int INFO_COLOR        = SCR_BRIGHT|SCR_WHITE|SCR_REV_BLUE;
  const int MESSAGE_COLOR     = SCR_WHITE;
  const int BORDER_INFO_COLOR = SCR_BRIGHT|SCR_BROWN;

  int      Row;                           // Row on screen.
  int      Start_Line   = 0;              // The line # of the top line on the display.
  int      Start_Column = 0;              // The column # of the left side on the display.
  int      Line_Count   = 0;              // The number of lines in the message.
  char    *Temp_Text;                     // Used when counting lines.
  char     Line_Buffer[WINDOW_WIDTH+1];   // Used to hold a line to display from the message.
  char     Work_Buffer[80+1];             // Used to hold username and date string.
  char    *Buffer_Pntr;                   // Points into Line_Buffer.
  int      Len;                           // Length of Work_Buffer.
  int      Ch;                            // Keystroke from user.
  Message *Msg_Info = &Topic_Index[Current];

  // Display the status line at the top of the screen.
  ScrClear    (INFO_ROW,  1, 80, 1, INFO_COLOR);
  ScrPrintText(INFO_ROW,  7, 80 - 7, Msg_Info->Subject_Line);
  ScrPrintText(INFO_ROW, 70, 10, "F1 = HELP");

  // Draw the box around the message area.
  ScrPrint(TOP_LINE, 1, 80, MESSAGE_COLOR, "ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป");
  for (Row = TOP_LINE+1; Row < BOTTOM_LINE; Row++)
    ScrPrint (Row, 1, 80, MESSAGE_COLOR, "บ                                                                              บ");
  ScrPrint (BOTTOM_LINE, 1, 80, MESSAGE_COLOR, "ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ");

  // Count the number of complete lines in the message (note this is only done once).
  for (Temp_Text = Text; *Temp_Text; Temp_Text++)
    if (*Temp_Text == '\n') Line_Count++;

  // Display the username of the message's author and the size of the message.
  sprintf (Work_Buffer, " From: %s (%d lines)", Msg_Info->Username, Line_Count);
  Len = strlen(Work_Buffer);
  ScrPrint(BOTTOM_LINE, 3,       1, MESSAGE_COLOR, "ต");
  ScrPrint(BOTTOM_LINE, 4+(Len), 1, MESSAGE_COLOR, "ฦ", MESSAGE_COLOR);
  ScrPrint(BOTTOM_LINE, 4,       Len, BORDER_INFO_COLOR, Work_Buffer);

  // Display the date the message was posted.
  sprintf (Work_Buffer, " Date: %s ", Msg_Info->Date_String);
  Len = strlen(Work_Buffer);
  ScrPrint(BOTTOM_LINE, WINDOW_WIDTH-1-Len, 1, MESSAGE_COLOR, "ต");
  ScrPrint(BOTTOM_LINE, WINDOW_WIDTH,       1, MESSAGE_COLOR, "ฦ");
  ScrPrint(BOTTOM_LINE, WINDOW_WIDTH-Len,   Len, BORDER_INFO_COLOR, Work_Buffer);

  // Loop until user says to quit.
  do {
    int   Temp_Line = Start_Line; // Used to locate the start of Start_Line.
    int   Temp_Column;            // Used to locate the start of the text to display.
    char *Temp_Text = Text;       // Used to locate the text to display.

    // Tell the user where they are.
    sprintf(Work_Buffer, "(%3d) ", Start_Line + 1);
    ScrPrintText(INFO_ROW, 1, 6, Work_Buffer);

    // Scan Text looking for the start of Start_Line.
    while (Temp_Line && *Temp_Text) {
      if (*Temp_Text++ == '\n') Temp_Line--;
    }

    // For each line on the display...
    for (Row = TOP_LINE+1; Row < BOTTOM_LINE; Row++) {

      // Erase the buffer.
      memset(Line_Buffer, ' ', WINDOW_WIDTH);
      Line_Buffer[WINDOW_WIDTH] = '\0';

      // Skip any leading characters on this line.
      Temp_Column = Start_Column;
      while (Temp_Column && *Temp_Text && *Temp_Text != '\n') {
        Temp_Text++;
        Temp_Column--;
      }

      // Copy characters from Text as needed.
      Buffer_Pntr = Line_Buffer;
      while (*Temp_Text && (Buffer_Pntr - Line_Buffer < WINDOW_WIDTH) && *Temp_Text != '\n')
        *Buffer_Pntr++ = *Temp_Text++;

      // Search for the '\n' character that ends this line and skip it.
      while (*Temp_Text && *Temp_Text != '\n') Temp_Text++;
      if (*Temp_Text) Temp_Text++;

      // Display the line we've got.
      ScrPrint(Row, 2, WINDOW_WIDTH, MESSAGE_COLOR, Line_Buffer);
    }

    switch (Ch = ScrKey()) {
      case K_TAB:
        Blank_Screen();
        break;

      case K_ESC:
      case K_ALTX:
      case K_F7:
        break;

      case K_F1:
        Helper(MESSAGE_DISPLAY);
        break;

      case K_CPGUP:
        if (Current > 1) return Current-1;
        break;

      case K_CPGDN:
        if (Current < Max)
          if (Topic_Index[Current+1].Username != NULL) return Current+1;
        break;

      case K_DOWN:
        Start_Line++;
        if (Start_Line > Line_Count - WINDOW_HEIGHT) Start_Line--;
        break;

      case K_UP:
        Start_Line--;
        if (Start_Line < 0) Start_Line = 0;
        break;

      case K_HOME:
        Start_Line   = 0;
        Start_Column = 0;
        break;

      case K_CHOME:
        Start_Column = 0;
        break;

      case K_RIGHT:
        Start_Column++;
        break;

      case K_LEFT:
        Start_Column--;
        if (Start_Column < 0) Start_Column = 0;
        break;

      case K_CLEFT:
        Start_Column -= 20;
        if (Start_Column < 0) Start_Column = 0;
        break;

      case K_CRIGHT:
        Start_Column += 20;
        break;

      case K_END:
        Start_Line = Line_Count - WINDOW_HEIGHT;
        if (Start_Line < 0) Start_Line = 0;
        Start_Column = 0;
        break;

      case K_PGUP:
        Start_Line -= WINDOW_HEIGHT;
        if (Start_Line < 0) Start_Line = 0;
        break;

      case K_PGDN:
        Start_Line += WINDOW_HEIGHT;
        if (Start_Line > Line_Count - WINDOW_HEIGHT)
          Start_Line = Line_Count - WINDOW_HEIGHT;
          if (Start_Line < 0) Start_Line = 0;
        break;
    }
  } while (Ch != K_ESC && Ch != K_ALTX && Ch != K_F7);

  return Current;
}
