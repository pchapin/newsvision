/*****************************************************************************
File          : UTILITY.C
Date          : 2/17/92
Last Revised  : 2/18/92
Programmer    : Michael Martel

Purpose       : 1) To Implement the various miscellaneous functions that the
                Network NoteBook requires to survive.

                2) To implement the Bitshifting technique that is being
                implemented to encrypt the Network NoteBook topic Files.
                Please note that these functions can probably be simplified.

*****************************************************************************/

#include "environ.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "standard.h"
#include "timer.h"
#include "portscr.h"

int Encrypt_Character(int Ch){
  if (Ch>127) Ch=32;
  if (Ch!='\n' && Ch!='\r') Ch<<=1;
  return(Ch);
}

int Decrypt_Character(int Ch){
  if (Ch!='\n' && Ch!='\r') Ch>>=1;
  if (Ch<0) Ch+=128;
  return(Ch);
}

// Follows the same basic setup as Decrypt();


char *Encrypt_String(char *Temp_Line_Buffer)
/* This function encrypts a null-terminated character string */
{
  int    Index = 0;

  while(Temp_Line_Buffer[Index]) {
    if ((unsigned char) Temp_Line_Buffer[Index]>127) Temp_Line_Buffer[Index] = ' ';
    if (Temp_Line_Buffer[Index]!='\n' && Temp_Line_Buffer[Index]!='\r') Temp_Line_Buffer[Index]<<=1;
    Index++;
  }
  return Temp_Line_Buffer;
}

char *Decrypt_String(char *Temp_Line_Buffer){
         char Temp[2];
         int  ch;
         int  Index;
         char Temp_Buffer[256+2];
  static char Line_Buffer[256+2];

  strcpy(Line_Buffer, Temp_Line_Buffer);

//  Initialize everything to managable stuff.

  ch=0;
  Temp[0]=NULL;
  Temp[1]=NULL;
  Index=0;
  Temp_Buffer[0] = NULL;
  Index=0;

// Step down the string converting as we go.

  while(Index<(strlen(Line_Buffer))){
    ch=Line_Buffer[Index];
    if (ch<0) ch=256+ch;
    if (ch!='\n' && ch!='\r') ch>>=1;
    if (ch=='\r') ch='\000';
    if (ch==127) ch='\000';
    Temp[0]=ch;
    Temp[1]=NULL;
    strcat(Temp_Buffer, &Temp[0]);

    Index++;
    }
   strcpy(Line_Buffer, Temp_Buffer);
   return(Line_Buffer);
}

/*----------------------------------------------------------------------------
The following functions are useful in connection with manipulating strings
that are going to pumped onto the screen using ScrWrite. These function
support Display_Index().

Initialize_Row() fills the given array with white on black spaces. The
array must be large enough.

Spread_String() takes the input string and writes its characters into the
array pointed at by Out in such a way as to skip the attribute bytes that
might be in the array. Spread_String() will not write too many characters.

Color_Section() adjusts attributes in the given array starting at the given
index and running for the given length. Note that the index is as measured
on the screen.
----------------------------------------------------------------------------*/

void Initialize_Row(char *Out)
  {
    int i;
    for (i = 0; i < 80; i++) {
      *Out++ = ' ';
      *Out++ = SCR_WHITE;
    }
  }

void Spread_String(char *In, char *Out)
  {
    char *Start = Out;

    while (*In  &&  Out < Start + 160) {
      *Out++ = *In++;
       Out++;
    }
  }

void Color_Section(char *Out, int Index, int Length, int Color)
  {
    int i;
    Index *= 2;

    for (i = 0; i < Length; i++) {
      Out[Index + 1] = Color;
      Index += 2;
    }
  }


/*----------------------------------------------------------------------------
char *AdjDate(char *ANSI_Date);

The following function adjusts the date as produced by the __DATE__ macro
to a nicer format. It is not very important to the program.
----------------------------------------------------------------------------*/
char *AdjDate(char *ANSI_Date)
  {
    static char  Buffer[13];
    auto   char *Buffer_Pntr;

    strcpy(Buffer, ANSI_Date);
    for (Buffer_Pntr  = strchr(Buffer,'\0');
         Buffer_Pntr >= &Buffer[6];
         Buffer_Pntr--) {
      *(Buffer_Pntr+1) = *Buffer_Pntr;
    }
    Buffer[6] = ',';
    if (Buffer[4] == '0') {
      for (Buffer_Pntr = &Buffer[4]; *Buffer_Pntr; Buffer_Pntr++) {
        *Buffer_Pntr = *(Buffer_Pntr+1);
      }
    }
    return Buffer;
  }

/*****************************************************************************
Delay(); takes an integer and replaces the delay(); function in dos.h, thereby
increasing the portibility of the NoteBook software.  Where are we porting it
to ?  Unix ?  Note, that the Time passed to Delay(); is the time in SECONDS.
Bear this in Mind.
*****************************************************************************/

void Delay(long Time_To_Delay){
  Timer StopWatch;
  long Elapsed_Time;

  StopWatch.Reset();
  StopWatch.Start();
  do{
    Elapsed_Time = StopWatch.Time()/100;
  } while(Elapsed_Time<Time_To_Delay);
}

/*****************************************************************************
Display_Intro_Screen(); takes a fully qualified file name and then attempts
to display it on the screen.  This is equivalent to using the DOS type
command. Indeed, that is the exact line that it replaces in the original code.
The idea behind this is that this function could handle larger files and not
allow the user to issue a CTRL-C to get by any important information that we,
may want to tell the user.
*****************************************************************************/
void Display_Intro_Screen(char *Intro_File){
  FILE *I_File;
  char Line_Buffer[256+2];

  ScrClear(1,1,80,25,SCR_WHITE);
  if ((I_File=fopen(Intro_File, "rt"))==NULL)
    ScrPrint (23, 1, 80, SCR_BRIGHT|SCR_BLINK|SCR_BROWN, "Unable to Display Intro Screen.");
  else{
    ScrSetCursorPos(1,1);
    while(fgets(Line_Buffer, 256+2, I_File)!=NULL) printf("%s", Line_Buffer);
    ScrSetCursorPos(26,1);
    fclose(I_File);
   }
}

