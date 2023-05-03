/****************************************************************************
FILE          : scrbox.cpp
PROGRAMMER    : Paul Cabbe
LAST MODIFIED : March, 1992

This file contains two box drawing functions which use Peter Chapin's SCR
module to perform fast screen writes, improving the cosmetics of a program.
****************************************************************************/

#include "environ.h"
#include "standard.h"
#include "portscr.h"

/***************************************************************************/

int ScrBox (int UL_Row, int UL_Col, int LR_Row, int LR_Col, int Attr)
/* This function simply draws a single line bordered box */
{
  int Row, Col;

  ScrClear (UL_Row, UL_Col, LR_Col-UL_Col+1, LR_Row-UL_Row+1, Attr);

  ScrPrintText (UL_Row, UL_Col, 1, "�");
  ScrPrintText (UL_Row, LR_Col, 1, "�");
  ScrPrintText (LR_Row, UL_Col, 1, "�");
  ScrPrintText (LR_Row, LR_Col, 1, "�");

  for (Col = UL_Col+1; Col < LR_Col; Col++) {
    ScrPrintText (UL_Row, Col, 1, "�");
    ScrPrintText (LR_Row, Col, 1, "�");
  }

  for (Row = UL_Row+1; Row < LR_Row; Row++) {
    ScrPrintText (Row, UL_Col, 1, "�");
    ScrPrintText (Row, LR_Col, 1, "�");
  }

  return 0;
}


/***************************************************************************/

int ScrDBox (int UL_Row, int UL_Col, int LR_Row, int LR_Col, int Attr)
/* This function simply draws a single line bordered box */
{
  int Row, Col;

  ScrClear (UL_Row, UL_Col, LR_Col-UL_Col+1, LR_Row-UL_Row+1, Attr);

  ScrPrintText (UL_Row, UL_Col, 1, "�");
  ScrPrintText (UL_Row, LR_Col, 1, "�");
  ScrPrintText (LR_Row, UL_Col, 1, "�");
  ScrPrintText (LR_Row, LR_Col, 1, "�");

  for (Col = UL_Col+1; Col < LR_Col; Col++) {
    ScrPrintText (UL_Row, Col, 1, "�");
    ScrPrintText (LR_Row, Col, 1, "�");
  }

  for (Row = UL_Row+1; Row < LR_Row; Row++) {
    ScrPrintText (Row, UL_Col, 1, "�");
    ScrPrintText (Row, LR_Col, 1, "�");
  }

  return 0;
}

