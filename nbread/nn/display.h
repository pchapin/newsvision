/*****************************************************************************
File          : DISPLAY.H
Last Revision : November 1991
Programmer    : VTC^3

Purpose       : This file contains the interface for displaying a message on
                the screen.
*****************************************************************************/

#ifndef DISPLAY_H
#define DISPLAY_H

#include "message.h"

int Display_Message(char *Text, Message *Msg_Info, int Current, int Max);

#endif
