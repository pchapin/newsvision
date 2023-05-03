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

  ScrPrintText (UL_Row, UL_Col, 1, "Ú");
  ScrPrintText (UL_Row, LR_Col, 1, "¿");
  ScrPrintText (LR_Row, UL_Col, 1, "À");
  ScrPrintText (LR_Row, LR_Col, 1, "Ù");

  for (Col = UL_Col+1; Col < LR_Col; Col++) {
    ScrPrintText (UL_Row, Col, 1, "Ä");
    ScrPrintText (LR_Row, Col, 1, "Ä");
  }

  for (Row = UL_Row+1; Row < LR_Row; Row++) {
    ScrPrintText (Row, UL_Col, 1, "³");
    ScrPrintText (Row, LR_Col, 1, "³");
  }

  return 0;
}


/***************************************************************************/

int ScrDBox (int UL_Row, int UL_Col, int LR_Row, int LR_Col, int Attr)
/* This function simply draws a single line bordered box */
{
  int Row, Col;

  ScrClear (UL_Row, UL_Col, LR_Col-UL_Col+1, LR_Row-UL_Row+1, Attr);

  ScrPrintText (UL_Row, UL_Col, 1, "É");
  ScrPrintText (UL_Row, LR_Col, 1, "»");
  ScrPrintText (LR_Row, UL_Col, 1, "È");
  ScrPrintText (LR_Row, LR_Col, 1, "¼");

  for (Col = UL_Col+1; Col < LR_Col; Col++) {
    ScrPrintText (UL_Row, Col, 1, "Í");
    ScrPrintText (LR_Row, Col, 1, "Í");
  }

  for (Row = UL_Row+1; Row < LR_Row; Row++) {
    ScrPrintText (Row, UL_Col, 1, "º");
    ScrPrintText (Row, LR_Col, 1, "º");
  }

  return 0;
}

