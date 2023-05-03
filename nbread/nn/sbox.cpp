/*****************************************************************************
File 		: SBOX.C
Programmer 	: VTC^3 (Peter Chapin)
Date 		: Old
Last Revision 	: July 1997

Purpose 	: To draw simple (Hence the SBox) boxes on the screen, using
		  direct screen writes.
*****************************************************************************/

#include "environ.h"
#include <stdlib.h>
#include <string.h>
#include "standard.h"
#include "portscr.h"
#include "sbox.h"

void Open_SBox(Simple_Box *This, int R, int C, int W, int H, int Co, char *Title)
  {
    int Index;
    int Title_Size = (Title == NULL) ? 0 : strlen(Title);

    // Draw border. Don't worry about illegal sizes, etc.
    for (Index = C; Index < C+W; Index++) {
      ScrPrint(R,     Index, 1, Co, "\xC4");
      ScrPrint(R+H-1, Index, 1, Co, "\xC4");
    }
    for (Index = R; Index < R+H; Index++) {
      ScrPrint(Index, C,     1, Co, "\xB3");
      ScrPrint(Index, C+W-1, 1, Co, "\xB3");
    }
    ScrPrint(R, C,         1, Co, "\xDA");
    ScrPrint(R, C+W-1,     1, Co, "\xBF");
    ScrPrint(R+H-1, C,     1, Co, "\xC0");
    ScrPrint(R+H-1, C+W-1, 1, Co, "\xD9");

    // Print the title into the upper right side of the box.
    ScrPrintText(R, C + W - Title_Size - 2, Title_Size, Title);

    // Clear the interior region.
    ScrClear(R+1, C+1, W-2, H-2, Co);

    // Save dimensions, etc.
    This->Row    = R;
    This->Column = C;
    This->Width  = W;
    This->Height = H;
    This->Color  = Co;
  }

void Print_SBox(Simple_Box *This, int R, char *Text)
  {
    int Real_Row = This->Row + R;
    int Real_Col = This->Column + 1;
    ScrPrintText(Real_Row, Real_Col, This->Width - 2, Text);
  }

void Print_SBox(Simple_Box *This, int R, int C, char *Text)
  {
    int Real_Row = This->Row + R;
    int Real_Col = This->Column + C;
    ScrPrintText(Real_Row, Real_Col, This->Column + This->Width - 1 - Real_Col, Text);
  }

int UL_Row_SBox(Simple_Box *This)        { return This->Row;    }
int UL_Column_SBox(Simple_Box *This)     { return This->Column; }
int Total_Width_SBox(Simple_Box *This)   { return This->Width;  }
int Total_Height_SBox(Simple_Box *This)  { return This->Height; }
int Primary_Color_SBox(Simple_Box *This) { return This->Color;  }

void Construct_SWin(Simple_Window *This)
  {
    This->Background = 0;
  }

void Destroy_SWin(Simple_Window *This)
  {
    if (This->Background != 0) {
      ScrWrite(
        UL_Row_SBox(&This->Base_Object),
        UL_Column_SBox(&This->Base_Object),
        Total_Width_SBox(&This->Base_Object),
        Total_Height_SBox(&This->Base_Object),
        This->Background
      );
      free(This->Background);
    }
  }

void Open_SWin(Simple_Window *This, int R, int C, int W, int H, int Co, char *Title)
  {
    This->Background = (char *)malloc(sizeof(char) * 2 * W * H);
    if (This->Background != 0) {
      ScrRead(R, C, W, H, This->Background);
    }

    Open_SBox(&This->Base_Object, R, C, W, H, Co, Title);
  }

