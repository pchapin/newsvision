/****************************************************************************
This is the header file for ScrEnter, a user-configurable input routine.
****************************************************************************/

#define NO_BOX -1

#ifdef __cplusplus
extern "C" {
#endif

int ScrEnter(char *Prompt_Text, int Prompt_Attr, int Row, int Column,
              int Box_Attr, char *Text, char *Default_Text, int *Text_Length,
              int Text_Attr, int *Exit_Keys);

#ifdef __cplusplus
}
#endif

