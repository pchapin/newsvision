/*****************************************************************************
File 		: POSITION.H
Programmer 	: VTC^3
Date 		: 2/21/92
Last Revision 	: 2/21/92

Purpose 	: To allow highlighting of the Message list.  Used to be a
		  part of the NN software, but to clean up the code, it was
		  moved into a seperate file.
*****************************************************************************/


#include "display.h"

#ifndef POSITION_H
#define POSITION_H

typedef struct {
    int      Page_Size;         /* Number of rows (being used) on the display. */
    int      Top_Message;       /* Index into List[] of top message.           */
    int      Active_Message;    /* Index into List[] of highlighted message.   */
    Message *List;              /* Points at array of Message objects.         */
} Highlight_Positions;


void Construct_Highlight(Highlight_Positions *This, int P_Size, Message *L);
/* Access functions. Always return valid value. */

int Top_Highlight(Highlight_Positions *This);
int Active_Highlight(Highlight_Positions *This);

void Down_Highlight(Highlight_Positions *This);
void Up_Highlight(Highlight_Positions *This);
void Page_Down_Highlight(Highlight_Positions *This);
void Page_Up_Highlight(Highlight_Positions *This);
void Home_Highlight(Highlight_Positions *This);
void End_Highlight(Highlight_Positions *This);
void Make_Active_Highlight(Highlight_Positions *This, int New_Value);

#endif