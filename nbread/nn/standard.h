/****************************************************************************
FILE          : standard.h
LAST REVISION : May 1997
SUBJECT       : Standard declarations.
PROGRAMMER    : (C) Copyright 1997 by Peter Chapin

This file introduces a number of standard declarations and macros that can
apply to all programs. It extends the language in new ways or offers
support for standard features that are not yet available in some
environments.

Please send comments or bug reports to

     Peter Chapin
     P.O. Box 317
     Randolph Center, VT 05061
     pchapin@twilight.vtc.vsc.edu
****************************************************************************/

#ifndef STANDARD_H
#define STANDARD_H

#ifndef PCHAPIN
#error Must include pchapin's environ.h header before standard.h
#endif

//-------------------------------------------------------------
//           Standard Operating Environment Includes
//-------------------------------------------------------------

#if OS == WIN32 || GUI == WIN
#include <windows.h>
#endif
  // This will have the effect of #including <windows.h> into all files
  //   of a windows based project -- even those that don't explicitly
  //   use Windows. This shouldn't cause any damage and it should also
  //   allow, by way of macros and inline functions, for windows specific
  //   code to be inserted into non-windows specific modules at compile time.

#include <stddef.h>
#include <iostream.h>
  // I need this for all I/O -- including in-core streams and other special
  //   effects. Do I really want to include this in all modules?

//-----------------------------------
//           The bool Type
//-----------------------------------

// Borland C++ v5.x and Watcom C++ v11.0 support it. None of the other compilers
//   I use do, but some have a #define for bool in a system header file (gcc seems
//   to be like that).
#if COMPILER != BORLAND && COMPILER != WATCOM && !defined(bool)
typedef int bool;
#define true  1
#define false 0
#endif

//-------------------------------------------
//           The auto_ptr Template
//-------------------------------------------

// You can't program with dynamic memory and exceptions without this.
//   Borland implements it.

#if COMPILER == BORLAND
#include <memory>
  // Do I really want to include this in all modules?
#else

// This is for compilers that don't yet support it. The implementation
//   below should completely support the standard.

template<class Type> class auto_ptr {
  private:
    Type *Raw;

  public:
    explicit auto_ptr(Type *p = 0)     : Raw(p)         { }
             auto_ptr(auto_ptr &Other) : Raw(Other.Raw) { Other.Raw = 0; }
            ~auto_ptr()                                 { delete Raw;    }

    void operator=(auto_ptr &Other) { Raw = Other.Raw; Other.Raw = 0; }

    Type &operator *() const { return *Raw; }
    Type *operator->() const { return  Raw; }
    Type *get()        const { return  Raw; }
    Type *release()          { Type *Temp = Raw; Raw = 0; return Temp; }
    Type *reset(Type *p = 0) { Type *Temp = Raw; Raw = p; return Temp; }
};

#endif

// I also need an auto_ptr template that can delete arrays properly. As far
//   as I can see, this is not part of the standard so I'm presenting it
//   unconditionally.

template<class Type> class auto_arrayptr {
  private:
    Type *Raw;

  public:
    explicit auto_arrayptr(Type *p = 0)          : Raw(p)         { }
             auto_arrayptr(auto_arrayptr &Other) : Raw(Other.Raw) { Other.Raw = 0; }
            ~auto_arrayptr()                                      { delete [] Raw; }

    void operator=(auto_arrayptr &Other) { Raw = Other.Raw; Other.Raw = 0; }

    Type &operator [](int Index) const { return Raw[Index]; }
    Type *get()                  const { return Raw; }
    Type *release()                    { Type *Temp = Raw; Raw = 0; return Temp; }
    Type *reset(Type *p = 0)           { Type *Temp = Raw; Raw = p; return Temp; }
};

//------------------------------------------------
//           Little Language Extentions
//------------------------------------------------

// Windows specific material
#if OS == WIN32 || GUI == WIN
#include "winstd.h"
#endif

//----------------------------------------------------
//           Types for Low Level Programming
//----------------------------------------------------

//  As far as I know, these will work in all supported environments.
typedef unsigned char  byte;         //  8 bits.
typedef unsigned short word;         // 16 bits.
typedef unsigned long  long_word;    // 32 bits (Motorola terminology).
typedef unsigned long  double_word;  // 32 bits (Intel terminology).

//-----------------------------------------------------
//           Memory Allocation Error Handling
//-----------------------------------------------------

// Borland uses the older (now non-standard) xalloc object to report exceptions.
#if COMPILER == BORLAND
#include <except.h>
#define bad_alloc xalloc
#else

// For compilers that don't support this right, here is a class to use.
class bad_alloc {
};
#endif

// The following stuff pertains to memory management. This stuff needs be
//   included after the standard headers so that the macro definition of
//   malloc() doesn't cause errors when the declaration of function malloc()
//   is seen in stdlib.h. The point of this junk is to allow me to hook
//   into the memory management of *both* malloc() and operator new in such
//   a way as to avoid NULL returns on memory allocation operations.
//
// The MY_MALLOC discipline is obsolete. New software should never use malloc()
//   and new compilers will throw exceptions on failed allocations by default.

#ifdef MY_MALLOC

// These functions have the same semantics as the standard library functions
//   except that they never return an error indication. Instead, if they
//   can't find memory they call the function void Memory_Panic(). If
//   Memory_Panic() can release some memory, it can return and the allocation
//   will be attempted again. If Memory_Panic() cannot release any memory,
//   it can abort the program or throw a bad_alloc exception.
//
// Standard C++ requires that failed allocations via operator new throw
//   exceptions. However, not all compilers currently support that. The
//   facility defined here will allow my program to support exception
//   semantics even if the compiler doesn't. In addition, this discipline
//   also allows malloc() to throw an exception on failure; a feature that
//   is useful in programs that are mixing malloc() and operator new.

void *My_Malloc(size_t);
void *My_Calloc(size_t, size_t);
void *My_Realloc(void *, size_t);
void  My_Free(void *);
void  Memory_Panic(void);

// Macros that redirect calls to memory management functions to my code. If
//   MY_MALLOC is in force, then this file must be #included *after* <stdlib.h>
//   otherwise these macros will change the meaning of the declarations in
//   that standard header.
#define malloc(size)              My_Malloc(size)
#define calloc(size1, size2)      My_Calloc(size1, size2)
#define realloc(pointer, size)    My_Realloc(pointer, size)
#define free(pointer)             My_Free(pointer)

// Let's override the global operator new() so that it will also call
//   Memory_Panic() if it can't find the memory.
void *operator new(size_t);
void  operator delete(void *);

// And also for the versions that are applied to array allocations.
void *operator new[](size_t);
void  operator delete[](void *);

#endif

//------------------------------------------------------
//           Debugging and Testing Facilities
//------------------------------------------------------

// Define a nice debugging macro, and be sure assertions are in force appropriately.
#ifdef  DEBUG
#undef  NDEBUG  // Be sure assertions are in force.
#define Dbg(x) x
#else
#define NDEBUG  // Strip out assertions.
#define Dbg(x)
#endif
#include <assert.h>

// Define a similar testing macro.
#ifdef  TEST
#define Tst(x) x
#else
#define Tst(x)
#endif

//-------------------------
//           End
//-------------------------

#endif

