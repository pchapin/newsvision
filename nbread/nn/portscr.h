/****************************************************************************
FILE          : portscr.h
LAST REVISION : January 1997
SUBJECT       : Portable screen/keyboard handling functions.
PROGRAMMER    : (C) Copyright 1995 by Peter Chapin

NOTE          : This file given to VTC^3 for its projects with updates
                  provided forever free of charge.

This file contains the public interface to a general purpose screen
handling module. This file also declares a universal keystroke getting
function (ScrKey()). The keystroke getting function is here because on some
systems, the underlying library that handles the screen also handles
keystroke input.

Programs that use this module must not use any other functions for screen
or console I/O. They must call ScrRefresh() to force the screen to be
updated... writes to the screen have no effect until ScrRefresh() is
called. Finally, they must not make any assumptions about the size of the
screen; instead, they must ask this module for the dimensions.

See SCR.DOC for more details.

Please send comments or bug reports to

     Peter Chapin
     P.O. Box 317
     Randolph Center, VT 05061
     pchapin@twilight.vtc.vsc.edu
****************************************************************************/

#ifndef PORTSCR_HPP
#define PORTSCR_HPP

/* This structure defines what box drawing characters look like. */
struct Box_Chars {
  char Horizontal;
  char Vertical;
  char Upper_Left;
  char Upper_Right;
  char Lower_Left;
  char Lower_Right;
  char Left_Stop;
  char Right_Stop;
  char Top_Stop;
  char Bottom_Stop;
  char Cross;
};

/* These are the permissible box types. (The order of enumerators in
     this enumeration matters to the implementation). */
enum Box_Type {
  SCR_DBL_LINE, SCR_SNGL_LINE, SCR_DARK_GRAPHIC, SCR_LIGHT_GRAPHIC,
  SCR_SOLID   , SCR_ASCII    , SCR_BLANK_BOX
};

/* Start up and clean up. No Scr... function can be used until ScrInitialize()
     is called or after ScrTerminate() is called. ScrInitialize() must also
     be called for SrcKey() to work correctly. */
extern bool    ScrInitialize(int Max_Rows, int Max_Cols);
extern void    ScrTerminate();

/* Look up the box drawing characters for a give box type. */
extern struct Box_Chars *ScrGetBoxChars(enum Box_Type);

/* Informational. */
extern bool    ScrIsMonochrome();
extern void    ScrAdjustDimensions(int *R, int *C, int *W, int *H);
extern int     ScrNmbrRows();
extern int     ScrNmbrCols();

/* Attribute manipulation. */
extern int ScrCvtAttr(int Attr);
extern int ScrReverseAttr(int Attr);

/* Fast transfer of material to and from screen. */
extern void ScrRead(int Row, int Col, int Width, int Height, char *Buf);
extern void ScrWrite(int Row, int Col, int Width, int Height, char *Buf);
extern void ScrReadText(int Row, int Col, int Width, int Height, char *Buf);
extern void ScrWriteText(int Row, int Col, int Width, int Height, char *Buf);
extern void ScrPrint(int Row, int Col, int Count, int Attr, char *Format, ...);
extern void ScrPrintText(int Row, int Col, int Count, char *Format, ...);

/* Manipulation of regions. */
extern void ScrClear(int Row, int Col, int Width, int Height, int Attr);
extern void ScrSetColor(int Row, int Col, int Width, int Height, int Attr);
extern void ScrClearScreen();
  /* The preferred way of erasing the entire screen. Generally faster than
       ScrClear() would be. Takes effect at once. No need to ScrRefresh(). */
extern void ScrScroll(int Direction, int Row, int Col, int Width, int Height, int Nmbr_Of_Rows, int Attr);

/* Arbitrary values to denote scroll directions. */
#define SCR_UP    1
#define SCR_DOWN  2

/* Cursor manipulations. Attempts to move off screen will actually move
     the cursor in the desired direction "as much as possible." */
extern void ScrSetCursorPos(int Row, int Col);
extern void ScrGetCursorPos(int *Row, int *Col);

/* This function actually updates the screen. No Scr... call has any effect
     until this function is called. This function may attempt to only
     rewrite character positions that have changed since the last call. */
extern void ScrRefresh();

/* This function redraws everything on the screen whether it needs it or
     not. Use this function to fix a mangled display. */
extern void ScrRedraw();

/* This function waits for the user to enter a keystroke. It returns a key
     code. It does not echo the keystroke to the screen. */
extern int ScrKey();

/* This function returns true if a ScrKey() will return at once. Otherwise
     it returns false. */
extern bool ScrKeyReady();

/* This function defines whether or not calls to ScrKey() do a ScrRefresh()
     before waiting for a keystroke. By default they do *not*. However, if
     you call this function with a YES argument, each ScrKey() call will
     do a ScrRefresh(). The idea here is that if you want to get a keystroke
     you probably also want the user to see the updated screen! */
extern void ScrRefresh_OnKey(bool);

/* This function waits for the user to press a key. Unlike ScrKey() it does
     *not* call ScrRefresh() beforehand regardless of the setting of
     ScrRefresh_OnKey(). This is so the program can wait for the user to
     read the output of another program when shelled out of the current
     program. Except for this, it's exactly like ScrKey() */
extern int ScrKeyWait();

/* If ScrIsMonochrome() returns YES then only SCR_WHITE, SCR_BLACK, and
     the reverse versions of black and white are supported. Monochrome
     systems promise to support SCR_BOLD, but not SCR_BLINK. */

#define SCR_BLACK       0x00
#define SCR_BLUE        0x01
#define SCR_GREEN       0x02
#define SCR_CYAN        0x03
#define SCR_RED         0x04
#define SCR_MAGENTA     0x05
#define SCR_BROWN       0x06
#define SCR_WHITE       0x07

#define SCR_REV_BLACK   0x00
#define SCR_REV_BLUE    0x10
#define SCR_REV_GREEN   0x20
#define SCR_REV_CYAN    0x30
#define SCR_REV_RED     0x40
#define SCR_REV_MAGENTA 0x50
#define SCR_REV_BROWN   0x60
#define SCR_REV_WHITE   0x70

#define SCR_BRIGHT      0x08
#define SCR_BLINK       0x80

/* The following values are the extended returns from ScrKey(). These
     key names reflect the IBM PC keyboard. The precise keystrokes
     required to generate these codes on non-IBM PC systems are system
     dependent. */

#define XF  0x100  /* Extended flag. Special keys have codes > XF. */

#define K_F1  (59+XF)      /* Function keys. */
#define K_F2  (60+XF)
#define K_F3  (61+XF)
#define K_F4  (62+XF)
#define K_F5  (63+XF)
#define K_F6  (64+XF)
#define K_F7  (65+XF)
#define K_F8  (66+XF)
#define K_F9  (67+XF)
#define K_F10 (68+XF)

#define K_SF1  (84+XF)     /* Shifted function keys. */
#define K_SF2  (85+XF)
#define K_SF3  (86+XF)
#define K_SF4  (87+XF)
#define K_SF5  (88+XF)
#define K_SF6  (89+XF)
#define K_SF7  (90+XF)
#define K_SF8  (91+XF)
#define K_SF9  (92+XF)
#define K_SF10 (93+XF)

#define K_CF1  (94+XF)     /* Control + function keys. */
#define K_CF2  (95+XF)
#define K_CF3  (96+XF)
#define K_CF4  (97+XF)
#define K_CF5  (98+XF)
#define K_CF6  (99+XF)
#define K_CF7  (100+XF)
#define K_CF8  (101+XF)
#define K_CF9  (102+XF)
#define K_CF10 (103+XF)

#define K_AF1  (104+XF)    /* Alt + function keys. */
#define K_AF2  (105+XF)
#define K_AF3  (106+XF)
#define K_AF4  (107+XF)
#define K_AF5  (108+XF)
#define K_AF6  (109+XF)
#define K_AF7  (110+XF)
#define K_AF8  (111+XF)
#define K_AF9  (112+XF)
#define K_AF10 (113+XF)

#define K_HOME  (71+XF)    /* Misc special keys. */
#define K_END   (79+XF)
#define K_PGUP  (73+XF)
#define K_PGDN  (81+XF)
#define K_LEFT  (75+XF)
#define K_RIGHT (77+XF)
#define K_UP    (72+XF)
#define K_DOWN  (80+XF)
#define K_INS   (82+XF)
#define K_DEL   (83+XF)

#define K_CHOME  (119+XF)  /* Control + misc special keys. */
#define K_CEND   (117+XF)
#define K_CPGUP  (132+XF)
#define K_CPGDN  (118+XF)
#define K_CLEFT  (115+XF)
#define K_CRIGHT (116+XF)

#define K_CTRLA  1         /* Nice names for control characters. */
#define K_CTRLB  2
#define K_CTRLC  3
#define K_CTRLD  4
#define K_CTRLE  5
#define K_CTRLF  6
#define K_CTRLG  7
#define K_CTRLH  8
#define K_CTRLI  9
#define K_CTRLJ  10
#define K_CTRLK  11
#define K_CTRLL  12
#define K_CTRLM  13
#define K_CTRLN  14
#define K_CTRLO  15
#define K_CTRLP  16
#define K_CTRLQ  17
#define K_CTRLR  18
#define K_CTRLS  19
#define K_CTRLT  20
#define K_CTRLU  21
#define K_CTRLV  22
#define K_CTRLW  23
#define K_CTRLX  24
#define K_CTRLY  25
#define K_CTRLZ  26

#define K_ESC        27    /* Nice names for special ascii keys. */
#define K_SPACE      32
#define K_TAB        K_CTRLI
#define K_BACKSPACE  K_CTRLH
#define K_RETURN     13
#define K_CRETURN    10

#define K_ALTA  (30+XF)    /* Alt + letter keys. */
#define K_ALTB  (48+XF)
#define K_ALTC  (46+XF)
#define K_ALTD  (32+XF)
#define K_ALTE  (18+XF)
#define K_ALTF  (33+XF)
#define K_ALTG  (34+XF)
#define K_ALTH  (35+XF)
#define K_ALTI  (23+XF)
#define K_ALTJ  (36+XF)
#define K_ALTK  (37+XF)
#define K_ALTL  (38+XF)
#define K_ALTM  (50+XF)
#define K_ALTN  (49+XF)
#define K_ALTO  (24+XF)
#define K_ALTP  (25+XF)
#define K_ALTQ  (16+XF)
#define K_ALTR  (19+XF)
#define K_ALTS  (31+XF)
#define K_ALTT  (20+XF)
#define K_ALTU  (22+XF)
#define K_ALTV  (47+XF)
#define K_ALTW  (17+XF)
#define K_ALTX  (45+XF)
#define K_ALTY  (21+XF)
#define K_ALTZ  (44+XF)

#define K_ALT1    (120+XF) /* Alt + number keys. */
#define K_ALT2    (121+XF)
#define K_ALT3    (122+XF)
#define K_ALT4    (123+XF)
#define K_ALT5    (124+XF)
#define K_ALT6    (125+XF)
#define K_ALT7    (126+XF)
#define K_ALT8    (127+XF)
#define K_ALT9    (128+XF)
#define K_ALT0    (129+XF)
#define K_ALTDASH (130+XF)
#define K_ALTEQU  (131+XF)

#endif
