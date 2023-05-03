/****************************************************************************
FILE          : portscr.cpp
LAST REVISION : May 1997
SUBJECT       : Implementation of portable screen handling functions.
PROGRAMMER    : (C) Copyright 1997 by Peter Chapin

Please send comments or bug reports to

     Peter Chapin
     P.O. Box 317
     Randolph Center, VT 05061
     pchapin@twilight.vtc.vsc.edu
****************************************************************************/

#include "environ.h"

/* Produce an error for alien operating systems. */
#if !(OS == MSDOS || OS == OS2 || OS == WIN32) || (GUI != NONE)
#error PORTSCR.C currently only supports text mode MS-DOS, OS/2, and Win32.
#endif

/* Force the user to decide: Direct video RAM access or ANSI escape sequences. */
#if !(defined(SCR_DIRECT) || defined(SCR_ANSI))
#error You must #define SCR_DIRECT or SCR_ANSI.
#endif

/* For now only SCR_ANSI supported under Win32. */
#if OS == WIN32 && defined(SCR_DIRECT)
#error You can currently only use SCR_ANSI under Win32.
#endif

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if (OS == MSDOS) && defined(SCR_DIRECT)
#include <dos.h>
  /* The DOS SCR_DIRECT version needs access to int86() for BIOS stuff. */
#ifdef __386__
#define do_interrupt(x,y,z) int386(x,y,z)
#else
#define do_interrupt(x,y,z) int86(x,y,z)
#endif
#endif

#if (OS == OS2) && defined(SCR_DIRECT)
#define INCL_BASE
#include <os2.h>
  /* The OS/2 SCR_DIRECT version needs access to Vio functions. */
#endif

#include <conio.h>
  /* For the getch() function. Used by ScrKey(). */

#include "standard.h"
#include "portscr.h"

/* If we're going to use ANSI, we'll need the basic ANSI functions. */
#if defined(SCR_ANSI)
#include "ansiscrn.h"
#endif

/*=================================*/
/*           Global Data           */
/*=================================*/

static
struct Box_Chars Box_Definitions[]={
  { 205, 186, 201, 187, 200, 188, 181, 198, 208, 210, 206 },  /* Double lines.  */
  { 196, 179, 218, 191, 192, 217, 180, 195, 193, 194, 197 },  /* Single lines.  */
  { 177, 177, 177, 177, 177, 177, 177, 177, 177, 177, 177 },  /* Dark graphic.  */
  { 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176 },  /* Light graphic. */
  { 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219 },  /* Solid.         */
  {  45, 124,  43,  43,  43,  43,  43,  43,  43,  43,  43 },  /* ASCII.         */
  {  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32 }   /* Blank.         */
  };

/* Define the basic screen size for both direct or ansi options. */
#if defined(SCR_DIRECT)
#define BYTES_PER_ROW      160  /* Number of bytes in one row of video RAM. */
#define NMBR_ROWS           25  /* Number of rows on the IBM PC scrren.     */
#define NMBR_COLS           80  /* Number of columns on the IBM PC screen.  */
#define MAX_PRINT_SIZE    1024  /* Largest string ScrPrint... can handle    */
#endif

#if defined(SCR_ANSI)
#define BYTES_PER_ROW      160  /* Number of bytes in one row of screen image. */
#define NMBR_ROWS           24  /* Number of rows on the screen.               */
#define NMBR_COLS           80  /* Number of columns on the screen.            */
#define MAX_PRINT_SIZE    1024  /* Largest string ScrPrint... can handle       */
#endif

/* Only important to the DOS direct option... */
#if (OS == MSDOS) && defined(SCR_DIRECT)
#define VIDEO_MONO           0  /* Symbolic names for the only two video modes supported. */
#define VIDEO_COLOR          1

#define BIOS_VIDEO        0x10  /* INT number for BIOS.   */
#define SET_CRTMODE          0  /* BIOS function numbers. */
#define SET_CURSORTYPE       1
#define SET_CURSORPOS        2
#define GET_CURSORPOS        3
#define SCROLL_UP            6
#define SCROLL_DOWN          7
#define GET_CRTMODE         15

static int       Old_Mode    = 0;             /* Video mode before ScrInitialize(). */
static unsigned  Scr_Segment = 0xB800;        /* Segment address of video RAM.      */
static int       Video_Mode  = VIDEO_COLOR;   /* Current video mode.                */
#endif

/* Only important to the ANSI option... */
#if defined(SCR_ANSI)
static char      Physical_Image[NMBR_ROWS*BYTES_PER_ROW];
  /* What's on the screen "at this time." */
static int       Physical_Row = 1;   /* Actual position of the cursor. */
static int       Physical_Col = 1;   /*   etc...                       */
#endif

/* General data required by all versions. */
static bool      Is_Init     = false;     /* =true if ScrInitialize() called. */
static int       Max_Cols    = NMBR_COLS; /* Usable size of the screen.      */
static int       Max_Rows    = NMBR_ROWS; /*   etc...                        */
static char      Screen_Image[NMBR_ROWS*BYTES_PER_ROW];
static int       Virtual_Col = 1;  /* Virtual cursor coordinates. */
static int       Virtual_Row = 1;  /*   etc...                    */
static bool      Key_Refresh = false;
static char      Work_Buffer[MAX_PRINT_SIZE+1];  /* Used by ScrPrint... */

/*======================================*/
/*           Public Functions           */
/*======================================*/

bool ScrInitialize(int M_Rows, int M_Cols)
  {
    #if (OS == MSDOS) && defined(SCR_DIRECT)
      union REGS  r;
    #endif

    /* Return at once if already initialized. */
    if (Is_Init) return true;

    #if (OS == MSDOS) && defined(SCR_DIRECT)
      /* Learn what video mode is currently being used. */
      r.h.ah = (byte) GET_CRTMODE;
      do_interrupt(BIOS_VIDEO, &r, &r);

      /* Save the old video mode. */
      Old_Mode = r.h.al;

      /* If it's mode 7 (monochrome), adjust our records. */
      if (r.h.al == 7) {
        Scr_Segment = 0xB000;
        Video_Mode = VIDEO_MONO;
      }

      /* Otherwise, force mode 3 (25x80 color). */
      else if (r.h.al != 3) {
        r.h.ah = (byte) SET_CRTMODE;
        r.h.al = 3;
        do_interrupt(BIOS_VIDEO, &r, &r);
      }
    #endif

    /* Adjust our records. */
    Is_Init = true;

    /* In any case, clear the screen and home the cursor. */
    ScrClearScreen();

    /* Set Max_Rows and Max_Colums if the user wants to limit the display.
         If the M_... values are zero, don't bother with this feature. */
    if (M_Rows != 0 && M_Rows < NMBR_ROWS) Max_Rows = M_Rows;
    if (M_Cols != 0 && M_Cols < NMBR_COLS) Max_Cols = M_Cols;

    return true;
  }


void ScrTerminate()
  {
    #if (OS == MSDOS) && defined(SCR_DIRECT)
      union REGS  r;
            int   Counter;
    #endif

    #if (OS == OS2) && defined(SCR_DIRECT)
      int Counter;
    #endif

    /* Do nothing if ScrInitialize() hasn't been called. */
    if (!Is_Init) return;

    #if (OS == MSDOS) && defined(SCR_DIRECT)
      /* Learn what video mode is currently being used. It's either 3 or 7. */
      r.h.ah = (byte) GET_CRTMODE;
      do_interrupt(BIOS_VIDEO, &r, &r);

      /* If this is different than the old mode, change back to the old mode. */
      if (r.h.al != Old_Mode) {
        r.h.ah = (byte) SET_CRTMODE;
        r.h.al = Old_Mode;
        do_interrupt(BIOS_VIDEO, &r, &r);
      }

      /* Otherwise clear the screen and home the cursor (mode switching has
           this effect so we don't need to mess with this above). */
      else {
        for (Counter = 0; Counter < NMBR_ROWS*BYTES_PER_ROW; Counter += 2) {
          Screen_Image[Counter]     = ' ';
          Screen_Image[Counter + 1] = SCR_WHITE|SCR_REV_BLACK;
        }
        Virtual_Row = 1;
        Virtual_Col = 1;
        ScrRedraw();
      }
    #endif

    #if (OS == OS2) && defined(SCR_DIRECT)
    /* Clear the screen and home the cursor. */
      for (Counter = 0; Counter < NMBR_ROWS*BYTES_PER_ROW; Counter += 2) {
        Screen_Image[Counter]     = ' ';
        Screen_Image[Counter + 1] = SCR_WHITE|SCR_REV_BLACK;
      }
      Virtual_Row = 1;
      Virtual_Col = 1;
      ScrRedraw();
    #endif

    #if defined(SCR_ANSI)
      /* Clear the screen and home the cursor. (We don't need to update
           Screen_Image[] or Physical_Image[]. This is the last Scr...
           function that will be called until another ScrInitialize(). */
      Reset_Screen();
      Clear_Screen();
      Position_Cursor(1, 1);
    #endif

    /* Adjust our records. */
    Is_Init = false;
  }


struct Box_Chars *ScrGetBoxChars(enum Box_Type The_Type)
  {
    #ifdef SCR_ASCIIBOXES
      if (The_Type == SCR_BLANK_BOX) return &Box_Definitions[SCR_BLANK_BOX];
      else return &Box_Definitions[SCR_ASCII];
    #else
      return &Box_Definitions[The_Type];
    #endif
  }


bool ScrIsMonochrome()
  {
    #if (OS == MSDOS) && defined(SCR_DIRECT)
      return (bool) (Video_Mode == VIDEO_MONO);
    #endif

    #if (OS == OS2) && defined(SCR_DIRECT)
      /* Assume all OS/2 machines can do color. */
      return false;
    #endif

    #if defined(SCR_ANSI)
      /* Assume all ANSI terminals can do color. */
      return false;
    #endif
  }


void ScrAdjustDimensions(int *R, int *C, int *W, int *H)
  {
    if (*R < 1)         *R = 1;
    if (*R > NMBR_ROWS) *R = NMBR_ROWS;
    if (*C < 1)         *C = 1;
    if (*C > NMBR_COLS) *C = NMBR_COLS;
    if (*H <= 0)        *H = 1;
    if (*W <= 0)        *W = 1;
    if (*R + *H - 1 > NMBR_ROWS) *H = NMBR_ROWS - *R + 1;
    if (*C + *W - 1 > NMBR_COLS) *W = NMBR_COLS - *C + 1;
  }


int ScrNmbrRows()
  {
    /* Return NMBR_ROWS or limit specified in Scr_Initialize(). */
    return Max_Rows;
  }


int ScrNmbrCols()
  {
    /* Return NMBR_COLS or limit specified in Scr_Initialize(). */
    return Max_Cols;
  }


int ScrCvtAttr(int Attribute)
  {
    /* Do work only if DOS SCR_DIRECT. For SCR_ANSI just return the given
         attribute. Remember that for now we are assuming that ANSI
         terminals can all do color; attribute conversions are not
         necessary. Similarly we are assuming at all OS/2 systems can do
         color. */

    #if (OS == MSDOS) && defined(SCR_DIRECT)
      /* Return at once with the attribute unchanged if not the monochrome mode. */
      if (Video_Mode != VIDEO_MONO) return Attribute;

      /* If black background, force white foreground. */
      if ((Attribute & 0x70) == SCR_REV_BLACK) {
        Attribute |= SCR_WHITE;
      }

      /* Otherwise there's a colored background, force reverse video. */
      else {
        Attribute |= SCR_REV_WHITE;
        Attribute &= 0xfff8;          /* Zero out (black) foreground only. */
      }
    #endif

    return Attribute;
  }


int ScrReverseAttr(int Attribute)
  {
    /* Extract the foreground and background colors. */
    int First  = Attribute & 0x07;
    int Second = (Attribute & 0x70) >> 4;

    /* Erase the colors in the original attribute. */
    Attribute &= 0xf8;
    Attribute &= 0x8f;

    /* Reinstall the colors in the opposite places. */
    Attribute |= First << 4;
    Attribute |= Second;

    return Attribute;
  }


void ScrRead(int Row, int Column, int Width, int Height, char *Buffer)
  {
    register unsigned  Nmbr_Of_Rows;    /* Loop index.                       */
    register unsigned  Row_Length;      /* Number of bytes in row of region. */
    auto     char     *Scr_Pntr;        /* Pointer into the screen image.    */

    ScrAdjustDimensions(&Row, &Column, &Width, &Height);

    Row_Length = 2 * Width;
    Scr_Pntr   = Screen_Image + ((Row - 1) * BYTES_PER_ROW) + (Column - 1) * 2;

    /* Loop over all rows in the region. */
    for (Nmbr_Of_Rows = Height; Nmbr_Of_Rows > 0; Nmbr_Of_Rows--) {

      /* Copy data from screen image to buffer and adjust offsets. */
      memcpy(Buffer, Scr_Pntr, Row_Length);
      Scr_Pntr += BYTES_PER_ROW;
      Buffer   += Row_Length;
    }
  }


void ScrReadText(int Row, int Column, int Width, int Height, char *Buffer)
  {
    char     *Scr_Pntr;
    char     *Source;
    unsigned  i;

    ScrAdjustDimensions(&Row, &Column, &Width, &Height);

    Scr_Pntr = Screen_Image + ((Row - 1) * BYTES_PER_ROW) + (Column - 1) * 2;
    for ( ; Height > 0; Height--) {
      Source = Scr_Pntr;
      for (i = 0; i < Width; i++, Source++) *Buffer++ = *Source++;
      Scr_Pntr += BYTES_PER_ROW;
    }
  }


void ScrWrite(int Row, int Column, int Width, int Height, char *Buffer)
  {
    register unsigned  Nmbr_Of_Rows;    /* Loop index.                       */
    register unsigned  Row_Length;      /* Number of bytes in row of region. */
    auto     char     *Scr_Pntr;        /* Pointer into the screen image.    */

    ScrAdjustDimensions(&Row, &Column, &Width, &Height);

    Row_Length = 2 * Width;
    Scr_Pntr   = Screen_Image + ((Row - 1) * BYTES_PER_ROW) + (Column - 1) * 2;

    /* Loop over all rows in the region. */
    for (Nmbr_Of_Rows = Height; Nmbr_Of_Rows > 0; Nmbr_Of_Rows--) {

      /* Copy data from buffer to screen image and adjust offsets. */
      memcpy(Scr_Pntr, Buffer, Row_Length);
      Scr_Pntr += BYTES_PER_ROW;
      Buffer   += Row_Length;
    }
  }


void ScrWriteText(int Row, int Column, int Width, int Height, char *Buffer)
  {
    char     *Scr_Pntr;
    char     *Dest;
    unsigned  i;

    ScrAdjustDimensions(&Row, &Column, &Width, &Height);

    Scr_Pntr = Screen_Image + ((Row - 1) * BYTES_PER_ROW) + (Column - 1) * 2;
    for ( ; Height > 0; Height--) {
      Dest = Scr_Pntr;
      for (i = 0; i < Width; i++, Dest++) *Dest++ = *Buffer++;
      Scr_Pntr += BYTES_PER_ROW;
    }
  }


void ScrPrint(int Row, int Column, int Count, int Attribute, char *Format, ...)
  {
    char   *Scr_Pntr;
    char   *String       = Work_Buffer;
    int     Dummy_Height = 1;
    va_list Args;

    ScrAdjustDimensions(&Row, &Column, &Count, &Dummy_Height);
    Attribute = ScrCvtAttr(Attribute);

    va_start(Args, Format);
    vsprintf(Work_Buffer, Format, Args);
    Scr_Pntr = Screen_Image + ((Row - 1) * BYTES_PER_ROW) + (Column - 1) * 2;
    while (*String && Count--) {
      *Scr_Pntr++ = *String++;
      *Scr_Pntr++ = char(Attribute);
    }
    va_end(Args);
  }


void ScrPrintText(int Row, int Column, int Count, char *Format, ...)
  {
    char   *Scr_Pntr;
    char   *String       = Work_Buffer;
    int     Dummy_Height = 1;
    va_list Args;

    ScrAdjustDimensions(&Row, &Column, &Count, &Dummy_Height);

    va_start(Args, Format);
    vsprintf(Work_Buffer, Format, Args);
    Scr_Pntr = Screen_Image + ((Row - 1) * BYTES_PER_ROW) + (Column - 1) * 2;
    while (*String && Count--) {
      *Scr_Pntr++ = *String++;
       Scr_Pntr++;
    }
    va_end(Args);
  }


void ScrClear(int Row, int Column, int Width, int Height, int Attribute)
  {
    char     *Scr_Pntr;
    char     *Dest;
    unsigned  i;

    ScrAdjustDimensions(&Row, &Column, &Width, &Height);
    Attribute = ScrCvtAttr(Attribute);

    Scr_Pntr = Screen_Image + ((Row - 1) * BYTES_PER_ROW) + (Column - 1) * 2;
    for ( ; Height > 0; Height--) {
      Dest = Scr_Pntr;
      for (i = 0; i < Width; i++) {
        *Dest++ = ' ';
        *Dest++ = char(Attribute);
      }
      Scr_Pntr += BYTES_PER_ROW;
    }
  }


void ScrSetColor(int Row, int Column, int Width, int Height, int Attribute)
  {
    char     *Scr_Pntr;
    char     *Dest;
    unsigned  i;

    ScrAdjustDimensions(&Row, &Column, &Width, &Height);
    Attribute = ScrCvtAttr(Attribute);

    Scr_Pntr = Screen_Image + ((Row - 1) * BYTES_PER_ROW) + (Column - 1) * 2;
    for ( ; Height > 0; Height--) {
      Dest = Scr_Pntr;
      for (i = 0; i < Width; i++) {
         Dest++;
        *Dest++ = char(Attribute);
      }
      Scr_Pntr += BYTES_PER_ROW;
    }
  }


void ScrScroll(
  int Direction, int Row, int Column, int Width, int Height, int Nmbr_Of_Rows, int Attribute
  )
  {
    char *Scr_Pntr;
    char *Source_Pntr;
    int   Row_Counter;

    if (Nmbr_Of_Rows <= 0) return;
    ScrAdjustDimensions(&Row, &Column, &Width, &Height);
    Attribute = ScrCvtAttr(Attribute);

    /* If we're trying to scroll too much, just clear the region and return. */
    if (Nmbr_Of_Rows >= Height) {
      ScrClear(Row, Column, Width, Height, Attribute);
      return;
    }

    /* Nmbr_Of_Rows to scroll must be less than Height. */
    if (Direction == SCR_UP) {
      Scr_Pntr = Screen_Image + ((Row - 1) * BYTES_PER_ROW) + (Column - 1) * 2;
      Source_Pntr = Scr_Pntr + (Nmbr_Of_Rows * BYTES_PER_ROW);
      for (Row_Counter = 0; Row_Counter < Height - Nmbr_Of_Rows; Row_Counter++) {
        memcpy(Scr_Pntr, Source_Pntr, 2*Width);
        Scr_Pntr    += BYTES_PER_ROW;
        Source_Pntr += BYTES_PER_ROW;
      }
      ScrClear(
        Row + (Height - Nmbr_Of_Rows), Column, Width, Nmbr_Of_Rows, Attribute
      );
    }

    /* Otherwise we're trying to scroll down. */
    else {
      Scr_Pntr = Screen_Image + ((Row - 1 + Height - 1) * BYTES_PER_ROW) + (Column - 1) * 2;
      Source_Pntr = Scr_Pntr - (Nmbr_Of_Rows * BYTES_PER_ROW);
      for (Row_Counter = 0; Row_Counter < Height - Nmbr_Of_Rows; Row_Counter++) {
        memcpy(Scr_Pntr, Source_Pntr, 2*Width);
        Scr_Pntr    -= BYTES_PER_ROW;
        Source_Pntr -= BYTES_PER_ROW;
      }
      ScrClear(Row, Column, Width, Nmbr_Of_Rows, Attribute);
    }
  }


void ScrSetCursorPos(int Row, int Column)
  {
    /* Be sure the coordinates are in bounds. */
    if (Row < 1)            Row    = 1;
    if (Row > NMBR_ROWS)    Row    = NMBR_ROWS;
    if (Column < 1)         Column = 1;
    if (Column > NMBR_COLS) Column = NMBR_COLS;

    Virtual_Row = Row;
    Virtual_Col = Column;
  }


void ScrGetCursorPos(int *Row, int *Column)
  {
    *Row    = Virtual_Row;
    *Column = Virtual_Col;
  }

#if defined(SCR_DIRECT)


void ScrRefresh()
  {
    ScrRedraw();
  }


void ScrClearScreen()
  {
    /* This will update both the screen image and the physical screen. */
    ScrClear(1, 1, NMBR_COLS, NMBR_ROWS, SCR_WHITE|SCR_REV_BLACK);
    ScrSetCursorPos(1, 1);
    ScrRefresh();
  }


void ScrRedraw()
  {
    #if (OS == MSDOS)
      union REGS r;
      char far *Scr_Pntr  = (char far *) Screen_Image;

      // Use movedata() if we are not using the DOS/4GW DOS extender.
      #ifndef __386__
        /* Put the text and attributes into video RAM. */
        movedata(
          /* Source address as seg:off */ FP_SEG(Scr_Pntr), FP_OFF(Scr_Pntr),
          /* Destin address as seg:off */ Scr_Segment,      0x0000,
          /* Number of bytes to move   */ NMBR_ROWS * BYTES_PER_ROW
        );

      // Just copy stuff to raw linear addresses if we are using the extender.
      #else
        memcpy((char *)0x000B8000, Screen_Image, NMBR_ROWS * BYTES_PER_ROW);
      #endif

      /* Position the cursor to the correct location. */
      r.h.ah = (byte) SET_CURSORPOS;
      r.h.dh = (byte) (Virtual_Row - 1);
      r.h.dl = (byte) (Virtual_Col - 1);
      r.h.bh = (byte) 0;
      do_interrupt(BIOS_VIDEO, &r, &r);
    #endif

    #if (OS == OS2)
      /* Put the text and attributes into video RAM. */
      VioWrtCellStr(Screen_Image, NMBR_ROWS * BYTES_PER_ROW, 0, 0, 0);

      /* Position the cursor to the correct location. */
      VioSetCurPos(Virtual_Row - 1, Virtual_Col - 1, 0);
    #endif
  }

#endif

#if defined(SCR_ANSI)

static int Foreground_Table[] = {
  /* 0 => Black   */ F_BLACK,
  /* 1 => Blue    */ F_BLUE,
  /* 2 => Green   */ F_GREEN,
  /* 3 => Cyan    */ F_CYAN,
  /* 4 => Red     */ F_RED,
  /* 5 => Magenta */ F_MAGENTA,
  /* 6 => Brown   */ F_YELLOW,
  /* 7 => White   */ F_WHITE
  };

static int Background_Table[] = {
  /* 0 => Black   */ B_BLACK,
  /* 1 => Blue    */ B_BLUE,
  /* 2 => Green   */ B_GREEN,
  /* 3 => Cyan    */ B_CYAN,
  /* 4 => Red     */ B_RED,
  /* 5 => Magenta */ B_MAGENTA,
  /* 6 => Brown   */ B_YELLOW,
  /* 7 => White   */ B_WHITE
  };


void ScrRefresh()
  {
    int   Current_Attribute;    /* Current printing attribute.        */
    int   Desired_Attribute;    /* The total attribute that we want.  */
    int   Desired_Foreground;   /* The ANSI color number of what we want. */
    int   Desired_Background;   /* The ANSI color number of what we want. */
    int   Row_Count;
    int   Col_Count;

    /* Set the colors to a known value. */
    Reset_Screen();
    Current_Attribute = SCR_WHITE|SCR_REV_BLACK;

    /* Loop over the entire screen. */
    for (Row_Count = 1; Row_Count <= NMBR_ROWS; Row_Count++) {
      for (Col_Count = 1; Col_Count <= NMBR_COLS; Col_Count++) {

        /* Compute offset into the image arrays of the current character. */
        int Array_Index = (Row_Count - 1)*BYTES_PER_ROW + 2*(Col_Count - 1);

        /* If this character is already what we want, skip it. */
        if (Screen_Image[Array_Index]     == Physical_Image[Array_Index] &&
            Screen_Image[Array_Index + 1] == Physical_Image[Array_Index + 1])
          continue;

        /* Otherwise, position the cursor if it's in the wrong place. */
        if (Row_Count != Physical_Row || Col_Count != Physical_Col) {
          Position_Cursor(Row_Count, Col_Count);
          Physical_Row = Row_Count;
          Physical_Col = Col_Count;
        }

        /* Is the current color what we want? If not, adjust it. */
        Desired_Attribute  = Screen_Image[Array_Index + 1];

        if (Desired_Attribute != Current_Attribute) {
          Reset_Screen();
          Desired_Foreground = Foreground_Table[ Desired_Attribute & 0x07];
          Desired_Background = Background_Table[(Desired_Attribute & 0x70) >> 4];
          Set_Color(Desired_Foreground);
          Set_Color(Desired_Background);
          if (Desired_Attribute & SCR_BRIGHT) Bold_On();
          if (Desired_Attribute & SCR_BLINK) Blink_On();
          Current_Attribute = Desired_Attribute;
        }

        /* Print the character! (And adjust our record of the cursor position. */
        putchar(Screen_Image[Array_Index]);
        Physical_Col++;

        /* Update the physical image. */
        Physical_Image[Array_Index]     = Screen_Image[Array_Index];
        Physical_Image[Array_Index + 1] = Screen_Image[Array_Index + 1];
      }
    }

    /* Position the cursor to its final resting place. */
    Position_Cursor(Virtual_Row, Virtual_Col);
    Physical_Row = Virtual_Row;
    Physical_Col = Virtual_Col;
  }


void ScrClearScreen()
  {
    int Counter;

    /* Clear the physical screen. */
    Reset_Screen();
    Clear_Screen();

    /* Make the arrays correct. */
    for (Counter = 0; Counter < NMBR_ROWS*BYTES_PER_ROW; Counter += 2) {
      Screen_Image[Counter]        = ' ';
      Screen_Image[Counter + 1]    = SCR_WHITE|SCR_REV_BLACK;
      Physical_Image[Counter]      = ' ';
      Physical_Image[Counter + 1]  = SCR_WHITE|SCR_REV_BLACK;
    }

    /* Position the cursor. Record the location of the real cursor. */
    Position_Cursor(1, 1);
    Virtual_Row  = 1;
    Virtual_Col  = 1;
    Physical_Row = 1;
    Physical_Col = 1;
  }


void ScrRedraw()
  {
    int     Current_Attribute; /* Color/attribute of last character printed. */
    int     Row_Count;
    int     Col_Count;
    char   *Current_Char;   /* Points at next character in the screen
                                 image that we want to print. */

    /* Scan over the entire screen, one row at a time. */
    for (Row_Count = 1; Row_Count <= NMBR_ROWS; Row_Count++) {

      /* Put the cursor at the start of the row and force the current
           color/attribute state to something we know. */
      Current_Attribute = SCR_WHITE|SCR_REV_BLACK;
      Position_Cursor(Row_Count, 1);
      Reset_Screen();

      /* Point into the screen image. */
      Current_Char = Screen_Image + (Row_Count - 1)*BYTES_PER_ROW;

      /* Scan down the row... */
      for (Col_Count = 1; Col_Count <= NMBR_COLS; Col_Count++) {

        /* If the current character's color is different than the current
             state, change color/attribute as needed. */
        if (Current_Attribute != Current_Char[1]) {
          Reset_Screen();
          if (Current_Char[1] & SCR_BRIGHT) Bold_On();
          if (Current_Char[1] & SCR_BLINK ) Blink_On();
          Set_Color(Foreground_Table[Current_Char[1] & 0x07]);
          Set_Color(Background_Table[(Current_Char[1] & 0x70) >> 4]);
          Current_Attribute = Current_Char[1];
        }

        /* Print the character. */
        putchar(*Current_Char);
        Current_Char += 2;
      }
    }

    /* Ok. We're done with the screen. Now we've got to position the
         cursor and reset the screen. */
    Position_Cursor(Virtual_Row, Virtual_Col);
    Reset_Screen();
  }

#endif

void ScrRefresh_OnKey(bool Flag)
  {
    Key_Refresh = Flag;
  }

#ifdef SCR_ASCIIKEYS

/* Remap the control keys to the special movement keys. */
static int Control_Translation[] = {
'*', K_CTRLA,  K_LEFT ,  K_CTRLC,  K_CHOME,  K_CTRLE,  K_RIGHT,  K_CEND ,
     K_CTRLH,  K_CTRLI,  K_CTRLJ,  K_CLEFT,  K_CRIGHT, K_CTRLM,  K_DOWN ,
     K_CPGDN,  K_UP   ,  K_CTRLQ,  K_END  ,  K_CTRLS,  K_CTRLT,  K_CPGUP,
     K_PGDN ,  K_HOME ,  K_DEL  ,  K_INS  ,  K_PGUP
  };

/* Mappings from normal letters to the K_ALT... codes. */
static int Alt_Translation[] = {
  K_ALTA, K_ALTB, K_ALTC, K_ALTD, K_ALTE, K_ALTF, K_ALTG, K_ALTH, K_ALTI,
  K_ALTJ, K_ALTK, K_ALTL, K_ALTM, K_ALTN, K_ALTO, K_ALTP, K_ALTQ, K_ALTR,
  K_ALTS, K_ALTT, K_ALTU, K_ALTV, K_ALTW, K_ALTX, K_ALTY, K_ALTZ
};

/* Mappings from the number keys to the K_ALT... codes. */
static int AltNumber_Translation[] = {
  K_ALT0, K_ALT1, K_ALT2, K_ALT3, K_ALT4,
  K_ALT5, K_ALT6, K_ALT7, K_ALT8, K_ALT9
};

/* Mappings from the number keys to various types of function key codes. */
static int Function_Translation[] = {
K_F10, K_F1, K_F2, K_F3, K_F4, K_F5,
       K_F6, K_F7, K_F8, K_F9
};

static int ShiftFunction_Translation[] = {
K_SF10, K_SF1, K_SF2, K_SF3, K_SF4, K_SF5,
        K_SF6, K_SF7, K_SF8, K_SF9
};

static int CtrlFunction_Translation[] = {
K_CF10, K_CF1, K_CF2, K_CF3, K_CF4, K_CF5,
        K_CF6, K_CF7, K_CF8, K_CF9
};

static int AltFunction_Translation[] = {
K_AF10, K_AF1, K_AF2, K_AF3, K_AF4, K_AF5,
        K_AF6, K_AF7, K_AF8, K_AF9
};

int ScrKey()
  {
    if (Key_Refresh) ScrRefresh();
    return ScrKeyWait();
  }

boolean ScrKeyReady()
  {
    if (kbhit()) return YES;
    return NO;
  }

int ScrKeyWait()
  {
    int Ch;

    /* If the character is not a control character, return it as is. */
    if ((Ch = getch()) > K_CTRLZ) return Ch;

    /* If it's a CTRLT, then figure out which function key code to return. */
    if (Ch == K_CTRLT) {
      switch (Ch = getch()) {

        /* Straight function keys. */
        default:
          if (isdigit(Ch)) return Function_Translation[Ch - '0'];
          return '*';
          break;

        /* Shift+function keys. */
        case K_CTRLS:
          Ch = getch();
          if (isdigit(Ch)) return ShiftFunction_Translation[Ch - '0'];
          return '*';
          break;

        /* Ctrl+function keys. */
        case K_CTRLC:
          Ch = getch();
          if (isdigit(Ch)) return CtrlFunction_Translation[Ch - '0'];
          return '*';
          break;

        /* Alt+function keys. */
        case K_CTRLA:
          Ch = getch();
          if (isdigit(Ch)) return AltFunction_Translation[Ch - '0'];
          return '*';
          break;
      }
    }

    /* If it's a CTRLA, then get the next letter and return the ALT code. */
    if (Ch == K_CTRLA) {
      Ch = getch();
      if (Ch == '-'  ) return K_ALTDASH;
      if (Ch == '='  ) return K_ALTEQU;
      if (isdigit(Ch)) return AltNumber_Translation[Ch];
      if (isalpha(Ch)) return Alt_Translation[tolower(Ch) - 'a'];
      return '*';
    }

    /* Otherwise look up the translated character. */
    return Control_Translation[Ch];
  }

#else

int ScrKey()
  {
    if (Key_Refresh) ScrRefresh();
    return ScrKeyWait();
  }

bool ScrKeyReady()
  {
    if (kbhit()) return true;
    return false;
  }

int ScrKeyWait()
  {
    int Ch;

    #if OS == MSDOS || OS == WIN32
      if ((Ch = getch()) != '\0') return Ch;
    #elif OS == OS2
      if ( ((Ch = getch()) != '\0') && (Ch != '\xE0') ) return Ch;
    #endif

    return (getch() + XF);
  }

#endif

