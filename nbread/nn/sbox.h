/*****************************************************************************
File          : SBOX.H
Date          : February 1992
Last Revision : February 1992
Programmer    : VTC^3

Purpose       : To implement the type Simple_Box that allows for drawing
                simple boxes on the screen.
*****************************************************************************/

#ifndef SBOX_HPP
#define SBOX_HPP

typedef struct {
  int Row;
  int Column;
  int Width;
  int Height;
  int Color;
} Simple_Box;

void Open_SBox(Simple_Box *This, int R, int C, int W, int H, int Co, char *Title);
  // Draws border inside the specified region, prints the Title onto
  //   that border, and clears the region inside the border.

void Print_SBox(Simple_Box *This, int R, char *Text);
  // Prints Text on row R of the region (one based) using the previously
  //   selected color.

void Print_SBox(Simple_Box *This, int R, int C, char *Text);
  // Prints Text on row R, column C of the region (one based) using the previously
  //   selected color. There is no bounds checking.

// Access functions return information about This box.
int UL_Row_SBox(Simple_Box *This);
int UL_Column_SBox(Simple_Box *This);
int Total_Width_SBox(Simple_Box *This);
int Total_Height_SBox(Simple_Box *This);
int Primary_Color_SBox(Simple_Box *This);

typedef struct {
  Simple_Box  Base_Object;
  char       *Background;
} Simple_Window;

void Construct_SWin(Simple_Window *This);
void Destroy_SWin(Simple_Window *This);
  // Constructor insures that Background initially NULL. Destructor
  //   restores background and releases memory that was used to hold
  //   the background.

void Open_SWin(Simple_Window *This, int R, int C, int W, int H, int Co, char *Title);
  // Saves the background information before doing Open_Sbox().

#endif

