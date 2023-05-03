/*****************************************************************************
File         : HIGHLITE.H
Programmer   : Michael Martel
Date         : 2/20/91
Last Revised : 2/20/92

Purpose      : To implement a scrollable selection of the Topics.

*****************************************************************************/

#ifndef HIGHLITE_H
#define HIGHLITE_H

#include "portscr.h"

#define NORMAL_COLOR     SCR_WHITE|SCR_REV_BLACK
#define BRIGHT_COLOR     SCR_WHITE|SCR_BRIGHT|SCR_REV_BLACK
#define HIGHLIGHT_COLOR  SCR_BLACK|SCR_REV_BROWN
#define BHIGHLIGHT_COLOR SCR_WHITE|SCR_BRIGHT|SCR_REV_BROWN

void Fill(int Number, char *Text);
void UnFill(int Number, char *Text);
void HighLight(int Number, char *Description, int Bright);
int  HighLightDown(int Number, char *Old_Description, int Old_Bright, char *Description, int Bright, int Max_Topics);
int  HighLightUp(int Number, char *Old_Description, int Old_Bright, char *Description, int Bright);
int  HighLightPageDown(int Number, char *Old_Description, int Old_Bright, char *Description, int Bright, int Temp_Topic_Number);
int  HighLightPageUp(int Number, char *Old_Description, int Old_Bright, char *Description, int Bright, int Temp_Topic_Number);
int  HighLightHome(int Number, char *Old_Description, int Old_Bright, char *Description, int Bright);
int  HighLightEnd(int Number, char *Old_Description, int Old_Bright, char *Description, int Bright, int Max_Topics);

#endif

