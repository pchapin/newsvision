/*****************************************************************************
FILE          : POSITION.C
LAST REVISION : February 1992
PROGRAMMER    : VTC^3

Purpose       : To implement highlighting of messgaes in the message list.
                Used to be in NN.C but to reduce clutter, it got moved here.

*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "position.h"

/*----------------------------------------------------------------------------
struct Highlight_Positions

This type manages the relative positions of the top of a list and a
highlighted entry of the list. It always insures that the highlighted entry
stays on screen. This types knows too much about this program (it contains
a reference to the type 'Message'). It should be more generic.
----------------------------------------------------------------------------*/

/* IMPLEMENT THE TYPE Message IN A SEPARATE FILE (?) */

void Construct_Highlight(Highlight_Positions *This, int P_Size, Message *L)
  {
    This->Page_Size      = P_Size;
    This->Top_Message    = 1;
    This->Active_Message = 1;
    This->List           = L;
  }

/* Access functions. Always return valid value. */
int Top_Highlight(Highlight_Positions *This) { return This->Top_Message; }
int Active_Highlight(Highlight_Positions *This) { return This->Active_Message; }

void Down_Highlight(Highlight_Positions *This)
  {
    This->Active_Message++;
    if (This->List[This->Active_Message].Username == NULL) This->Active_Message--;
    if (This->Active_Message == This->Top_Message + This->Page_Size) This->Top_Message++;
  }

void Up_Highlight(Highlight_Positions *This)
  {
    This->Active_Message--;
    if (This->Active_Message < 1) This->Active_Message = 1;
    if (This->Active_Message < This->Top_Message) This->Top_Message = This->Active_Message;
  }

void Page_Down_Highlight(Highlight_Positions *This)
  {
    int Last_Message;

    /* Find just past the end of the list. */
    Message *End_Pntr = This->List + 1;
    while (End_Pntr->Username != NULL) End_Pntr++;

    Last_Message = End_Pntr - This->List - 1; /* Could be -1 for empty lists. */
    if (Last_Message < 1) Last_Message = 1;

    This->Top_Message += This->Page_Size;
    This->Active_Message += This->Page_Size;
    if (This->Active_Message > Last_Message) This->Active_Message = Last_Message;
    if (This->Top_Message    > Last_Message) This->Top_Message    = Last_Message;
  }

void Page_Up_Highlight(Highlight_Positions *This)
  {
    This->Top_Message -= This->Page_Size;
    This->Active_Message -= This->Page_Size;
    if (This->Top_Message    < 1) This->Top_Message    = 1;
    if (This->Active_Message < 1) This->Active_Message = 1;
  }

void Home_Highlight(Highlight_Positions *This)
  {
    This->Top_Message    = 1;
    This->Active_Message = 1;
  }

void End_Highlight(Highlight_Positions *This)
  {
    /* Find just past the end of the list. */
    Message *End_Pntr = This->List + 1;
    while (End_Pntr->Username != NULL) End_Pntr++;

    /* Let the active message be the last one. Handle empty lists correctly. */
    This->Active_Message = End_Pntr - This->List - 1;
    if (This->Active_Message < 1) This->Active_Message = 1;

    /* Try to make the top message one page up. Correct if necessary. */
    This->Top_Message = This->Active_Message - This->Page_Size + 1;
    if (This->Top_Message < 1) This->Top_Message = 1;
  }

void Make_Active_Highlight(Highlight_Positions *This, int New_Value)
  {
    This->Active_Message = (New_Value < 1) ? 1 : New_Value;
    if (This->List[This->Active_Message].Username == NULL) End_Highlight(This);
    else if (This->Active_Message <  This->Top_Message) This->Top_Message = This->Active_Message;
    else if (This->Active_Message >= This->Top_Message + This->Page_Size)
      This->Top_Message = This->Active_Message - This->Page_Size + 1;
  }

