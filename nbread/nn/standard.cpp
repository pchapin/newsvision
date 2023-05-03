/****************************************************************************
FILE          : standard.cpp
LAST REVISION : April 1996
SUBJECT       : Implementation of standard support functions.
PROGRAMMER    : (C) Copyright 1996 by Peter Chapin

These functions are below all local libraries (those compiled from
source code), but above third party libraries (those provided in object
code) and the chain of abstraction. They are really language support
functions. They are supporting an "extended" language that implements
standard features not currently implemented by the compiler or new,
useful (IMHO) features.

Please send comments or bug reports to

     Peter Chapin
     P.O. Box 317
     Randolph Center, VT 05061
     pchapin@twilight.vtc.vsc.edu
****************************************************************************/

#include "environ.h"
#include <stdlib.h>   // Needed for declarations of memory managment functions.
#include <string.h>
#include "standard.h"

// To prevent recursive calls to My_Malloc(), etc below, I must undefine
//   the following symbols that are #defined in standard.h.
#undef malloc
#undef calloc
#undef realloc
#undef free

void Memory_Panic(void);
  // The function above must be written by the application programmer. It
  //   is called whenever MY_MALLOC has been defined during the compile
  //   *and* the memory allocation functions fail to find the requested memory.
  //   This function must NOT do anything, directly or indirectly, that would
  //   cause additional memory to be allocated without first freeing up sufficient
  //   memory. If this function returns, the original allocation will be attempted
  //   again.

#ifndef MY_MALLOC

// If the client isn't using My_Malloc(), etc, compile in this version of
//   Memory_Panic(). This is so there are no link errors when the functions
//   below are compiled in (even if not needed). If the caller is using
//   My_Malloc(), this function won't be compiled in, but the caller should
//   be providing their own.

void Memory_Panic()
  {
    return;
  }

#endif

// My_Malloc() never returns NULL. If it fails to find memory on the malloc()
//   heap, it calls Memory_Panic(), an application call-back function. The
//   function Memory_Panic() might throw an exception.

void *My_Malloc(size_t Size)
  {
    void *New_Space;

    do {
      New_Space = malloc(Size);
      if (!New_Space) Memory_Panic();
    } while (!New_Space);

    return New_Space;
  }

// My_Calloc() has the same semantics as calloc() except that it cannot
//   fail from the point of view of the application.

void *My_Calloc(size_t Nmbr_Items, size_t Size)
  {
    size_t  Bytes     = Nmbr_Items * Size;
    void   *New_Space = My_Malloc(Bytes);

    memset(New_Space, 0, Bytes);
    return New_Space;
  }

// And so forth for My_Realloc().

void *My_Realloc(void *Old_Space, size_t New_Size)
  {
    void *New_Space;

    do {
      New_Space = realloc(Old_Space, New_Size);
      if (!New_Space) Memory_Panic();
    } while (!New_Space);

    return New_Space;
  }

// There is no compelling reason to provide my own free(). However, for
//   symmetery, I will do so. Besides, by putting counters in here and
//   in My_Malloc(), I should be able to do some rudimentary checking for
//   memory leaks.

void My_Free(void *p)
  {
    free(p);
  }

#ifdef MY_MALLOC

// Let's be sure ::operator new() is implemented in terms of My_Malloc()
//   so that new expressions cannot fail. The C++ standard requires that
//   operator new throw an exception on failure. This facility here provides
//   that functionality in environments that are not yet up to the standard.

void *operator new(size_t Size)
  {
    return My_Malloc(Size);
  }

void *operator new[](size_t Size)
  {
    return My_Malloc(Size);
  }

// And ::operator delete() is here for the same reasons as My_Free().

void operator delete(void *p)
  {
    My_Free(p);
  }

void operator delete[](void *p)
  {
    My_Free(p);
  }

#endif

