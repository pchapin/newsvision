/*****************************************************************************
File          : UTILITY.H
Date          : 2/17/92
Last Revised  : 2/18/92
Programmer    : Michael Martel

Purpose       : Miscellaneous Utility functions that are used within the
                Network NoteBook.
                To define the various encryption methods that are employed for
                use with the Network NoteBook topic Files.  Both Encrypt();
                and Decrypt(); return a *Char, which allows it to be used as
                such :  printf("%s", Encrypt(Line_Buffer));

*****************************************************************************/

#ifndef UTILITY_H
#define UTILITY_H

int Encrypt_Character(int Character);
int Decrypt_Character(int Character);

char *Encrypt_String(char *Line_Buffer);
char *Decrypt_String(char *Line_Buffer);

void Initialize_Row(char *Out);
void Spread_String(char *In, char *Out);
void Color_Section(char *Out, int Index, int Length, int Color);

char *AdjDate(char *ANSI_Date);

void Delay(long Time_To_Delay);

void Display_Intro_Screen(char *Intro_File);

#endif
