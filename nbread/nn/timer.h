/*****************************************************************************
FILE          : timer.hpp
LAST REVISION : January 1991
CONTENTS      : Interface to class Timer.
PROGRAMMER    : (C) Copyright 1995 by Peter Chapin

     Please send comments and bug reports to

        Peter Chapin
        P.O. Box 317
        Randolph Center, VT 05061
        pchapin@night.vtc.vsc.edu
*****************************************************************************/

#ifndef TIMER_HPP
#define TIMER_HPP

#include <time.h>

class Timer {

  public:
    enum Timer_State {
      RESET,      // No time accumulated. Timer not keeping time.
      RUNNING,    // Timer is active.
      STOPPED     // Timer is not active. Accumulated time remembered.
    };

  private:
    clock_t      Start_Time;      // Time that the timer was last started.
    clock_t      Stop_Time;       // Time that the timer was last stopped.
    clock_t      Accumulated;     // Total accumulated time.
    Timer_State  Internal_State;  // Current state of timer object.

  public:
                Timer()  { Internal_State = RESET; Accumulated = 0; }
    void        Reset()  { Internal_State = RESET; Accumulated = 0; }
    Timer_State State()  { return Internal_State; }
    void        Start();
    void        Stop();
    long        Time();
  };

#endif

