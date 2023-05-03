/*****************************************************************************
File         : HIGHLITE.C
Programmer   : Michael Martel
Date         : 2/20/91
Last Revised : 2/20/92

Purpose      : To implement a scrollable selection of the Topics.

*****************************************************************************/

#include "environ.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "standard.h"
#include "portscr.h"
#include "highlite.h"

void Fill(int Number, char *Text){
  int Index;
  for(Index=(strlen(Text)+2); Index<80; Index++){
    ScrPrint(Number+2, Index, 80, HIGHLIGHT_COLOR, " ");
  }
}

void UnFill(int Number, char *Text){
  int Index;
  for(Index=(strlen(Text)+2); Index<80; Index++){
    ScrPrint(Number+2, Index, 80, NORMAL_COLOR, " ");
  }
}


void HighLight(int Number, char *Description, int Bright){
  if (Bright)
    ScrPrint(Number+2, 2, 80, BHIGHLIGHT_COLOR, Description);
  else
    ScrPrint(Number+2, 2, 80, HIGHLIGHT_COLOR, Description);
  Fill(Number, Description);
}

int HighLightDown(int Number, char *Old_Description, int Old_Bright, char *Description, int Bright, int Max_Topics){
  int New_Number;
  New_Number=Number+1;
  if(New_Number>Max_Topics){
    New_Number=Max_Topics;
    if (Bright)
      ScrPrint(New_Number+2, 2, 80, BHIGHLIGHT_COLOR, Description);
    else
      ScrPrint(New_Number+2, 2, 80, HIGHLIGHT_COLOR, Description);
    }
    else{
    if (Old_Bright)
      ScrPrint(Number+2, 2, 80, BRIGHT_COLOR, Old_Description);
    else
      ScrPrint(Number+2, 2, 80, NORMAL_COLOR, Old_Description);
    UnFill(Number, Old_Description);
    if (Bright)
      ScrPrint(New_Number+2, 2, 80, BHIGHLIGHT_COLOR, Description);
    else
      ScrPrint(New_Number+2, 2, 80, HIGHLIGHT_COLOR, Description);
    Fill(New_Number, Description);
    }
  return(New_Number);
}

int HighLightUp(int Number, char *Old_Description, int Old_Bright, char *Description, int Bright){
  int New_Number;
  New_Number=Number-1;
  if(New_Number<1){
    New_Number=1;
    if (Bright)
      ScrPrint(New_Number+2, 2, 80, BHIGHLIGHT_COLOR, Description);
    else
      ScrPrint(New_Number+2, 2, 80, HIGHLIGHT_COLOR, Description);
    Fill(New_Number, Description);
    }
    else{
    if (Old_Bright)
      ScrPrint(Number+2, 2, 80, BRIGHT_COLOR, Old_Description);
    else
      ScrPrint(Number+2, 2, 80, NORMAL_COLOR, Old_Description);
    UnFill(Number, Old_Description);
    if (Bright)
      ScrPrint(New_Number+2, 2, 80, BHIGHLIGHT_COLOR, Description);
    else
      ScrPrint(New_Number+2, 2, 80, HIGHLIGHT_COLOR, Description);
    Fill(New_Number, Description);
    }
  return(New_Number);
}

int HighLightPageDown(int Number, char *Old_Description, int Old_Bright, char *Description, int Bright, int Temp_Topic_Number){
    if (Old_Bright)
      ScrPrint(Number+2, 2, 80, BRIGHT_COLOR, Old_Description);
    else
      ScrPrint(Number+2, 2, 80, NORMAL_COLOR, Old_Description);
    UnFill(Number, Old_Description);
    if (Bright)
      ScrPrint(Temp_Topic_Number+2, 2, 80, BHIGHLIGHT_COLOR, Description);
    else
      ScrPrint(Temp_Topic_Number+2, 2, 80, HIGHLIGHT_COLOR, Description);
    Fill(Temp_Topic_Number, Description);
    return(Temp_Topic_Number);
}

int HighLightPageUp(int Number, char *Old_Description, int Old_Bright, char *Description, int Bright, int Temp_Topic_Number){
    if (Old_Bright)
      ScrPrint(Number+2, 2, 80, BRIGHT_COLOR, Old_Description);
    else
      ScrPrint(Number+2, 2, 80, NORMAL_COLOR, Old_Description);
    UnFill(Number, Old_Description);
    if (Bright)
      ScrPrint(Temp_Topic_Number+2, 2, 80, BHIGHLIGHT_COLOR, Description);
    else
      ScrPrint(Temp_Topic_Number+2, 2, 80, HIGHLIGHT_COLOR, Description);
    Fill(Temp_Topic_Number, Description);
    return(Temp_Topic_Number);
}

int HighLightHome(int Number, char *Old_Description, int Old_Bright, char *Description, int Bright){
  int New_Number;
  New_Number=1;
  if (Old_Bright)
    ScrPrint(Number+2, 2, 80, BRIGHT_COLOR, Old_Description);
  else
    ScrPrint(Number+2, 2, 80, NORMAL_COLOR, Old_Description);
  UnFill(Number, Old_Description);
  if (Bright)
    ScrPrint(New_Number+2, 2, 80, BHIGHLIGHT_COLOR, Description);
  else
    ScrPrint(New_Number+2, 2, 80, HIGHLIGHT_COLOR, Description, HIGHLIGHT_COLOR);
  Fill(New_Number, Description);
  return(New_Number);
}

int HighLightEnd(int Number, char *Old_Description, int Old_Bright, char *Description, int Bright, int Max_Topics){
  int New_Number;
  New_Number=Max_Topics;
  if (Old_Bright)
    ScrPrint(Number+2, 2, 80, BRIGHT_COLOR, Old_Description);
  else
    ScrPrint(Number+2, 2, 80, NORMAL_COLOR, Old_Description);
  UnFill(Number, Old_Description);
  if (Bright)
    ScrPrint(New_Number+2, 2, 80, BHIGHLIGHT_COLOR, Description);
  else
    ScrPrint(New_Number+2, 2, 80, HIGHLIGHT_COLOR, Description);
  Fill(New_Number, Description);
  return(New_Number);
}

