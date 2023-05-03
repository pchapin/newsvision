/*****************************************************************************
FILE          : environ.h
LAST REVISION : May 1997
SUBJECT       : Local compilation settings.
PROGRAMMER    : (C) Copyright 1997 by Peter Chapin

This file contains settings that define the environment in which the
program was compiled and the environment where it runs. This file should be
included into every source file as the very first header included.

Some of the symbols defined by this file can be (and even should be)
defined on the command line as necessary.

     Please send comments and bug reports to

     Peter Chapin
     P.O. Box 317
     Randolph Center, VT 05061
     pchapin@twilight.vtc.vsc.edu
*****************************************************************************/

#ifndef ENVIRON_H
#define ENVIRON_H

// This symbol can be used to verify that my headers and conventions are loaded.
#define PCHAPIN

// Make sure we are using C++
#if !defined(__cplusplus)
#error C++ compiler required.
#endif

// The following are the allowed values of COMPILER.
//
#define VANILLA     1
#define BORLAND     2  // v5.x  (that I have at home).
#define BORLAND_OLD 3  // v4.52 (that we have at VTC).
#define MICROSOFT   4
#define WATCOM      5  // v11.0 assumed.
#define GCC         6

// Choose your compiler. This file can autodetect Borland C++ and Watcom C/C++.
//   Any other compiler type must be specified on the compiler's command line.
//   The default is "VANILLA."
//
#if defined(__BORLANDC__)
#if __BORLANDC__ == 0x460       // v4.52
#define COMPILER BORLAND_OLD
#elif __BORLANDC__ == 0x500     // v5.x
#define COMPILER BORLAND
#else
#error Borland C++ v4.52 or v5.x expected.
#endif
#endif

#if defined(__WATCOMC__)
#if __WATCOMC__ == 1100
#define COMPILER WATCOM
#else
#error Watcom C++ v11.0 expected.
#endif
#endif

#if !defined(COMPILER)
#define COMPILER VANILLA
#endif

// The following are the allowed values of OS. Note that the Watcom compiler
//   already #defines MSDOS if appropriate (it uses different symbols for the
//   other operating system names). Win16 is not an operating system. It is
//   a GUI. Win32, on the other hand, is an operating system.
//
#ifndef __WATCOMC__
#define MSDOS   1
#endif
#define UNIX    2
#define VMS     3
#define OS2     4
#define WIN32   5
#define NETWARE 6

// Choose your operating system. If you are using Borland C++ or Watcom C++ this
//   file can autodetect the operating system. For any other compiler, the system
//   must be defined on the command line. There is no default.
//
#if COMPILER == BORLAND || COMPILER == BORLAND_OLD
#if defined(__MSDOS__)
#define OS MSDOS
#elif defined(__WIN32__)
#define OS WIN32
#endif
#endif

#if COMPILER == WATCOM
#if defined (__DOS__)
#define OS MSDOS
#elif defined(__OS2__)
#define OS OS2
#elif defined(__NETWARE__)
#define OS NETWARE
#elif defined(__NT__)
#define OS WIN32
#endif
#endif

#ifndef OS
#error No operating system specified for this compilation.
#endif

// When writing a multi-threaded program, there are additional issues that must
//   be considered. The symbol MULTITHREADED will be defined in all such cases.
//   This file can auto-detect this feature when Watcom C++ is being used, but it
//   will need help for other compilers and operating systems.

#if COMPILER == WATCOM
#if defined(_MT)
#define MULTITHREADED 1
#endif
#endif

// The following are the allowed values of GUI. If GUI is "WIN" and OS is
//   "MSDOS" then we are talking about Win16. If GUI is "WIN" and OS is
//   WIN32, then we are talking about a 32 bit Windows 95 or Windows NT
//   graphical application. If GUI is "NONE" and OS is "WIN32" then we
//   are talking about a Windows 95 or Windows NT console application.
//
#define NONE    0
#define WIN     1
#define PM      2
#define XWIN    3

// Choose your GUI. This file does not currently autodetect any GUI.
//
#if !defined(GUI)
#define GUI NONE
#endif

// Set the following flags to 1 to activate the following features. It may
//   be best to set these from the command line.
//
// #define DEBUG     0     // Compile in debug code and assertions.
// #define TEST      0     // Compile in test code.
// #define MY_MALLOC 0     // Compile in the My_Malloc() family of functions
                           //   so calls to malloc(), etc, call Memory_Panic
                           //   if they fail. This feature is obsolete.

#endif

