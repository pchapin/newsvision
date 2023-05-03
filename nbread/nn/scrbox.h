/****************************************************************************
FILE          : ENTER.C
PROGRAMMER    : Paul Cabbe
LAST MODIFIED : March, 1992

This is the header for two box drawing functions which use Peter Chapin's
SCR module to perform fast screen writes, improving the cosmetics of a
program.
****************************************************************************/

int ScrBox (int UL_Row, int UL_Col, int LR_Row, int LR_Col, int Attr);

int ScrDBox (int UL_Row, int UL_Col, int LR_Row, int LR_Col, int Attr);
