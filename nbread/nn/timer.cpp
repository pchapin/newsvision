/*****************************************************************************
FILE          : timer.cpp
LAST REVISION : March 1991
SUBJECT       : Implmentation of class Timer
PROGRAMMER    : (C) Copyright 1995 by Peter Chapin

Objects from class Timer are useful for timing relatively long events in
programs. They use the system clock as a base for generating time delays
and thus are not suitable (in most cases) for short delays.

Timers do not load the system in any way while they are timing. Only when
they are started and stopped do they check the system clock. They can thus
be fooled if the system clock is changed during the timing interval.

Timers allow for multiple starts and stops. In addition, their internal
state can be obtained by consumer code.

     Please send comments and bug reports to

         Peter Chapin
         P.O. Box 317
         Randolph Center, VT 05061

*****************************************************************************/

#include "environ.h"
#include "standard.h"
#include "timer.h"

/*----------------------------------------------------------------------------
void Timer::Start();

     The following function starts the timer. Notice that if the timer is
already started, it is essentially "retriggered". That is, the old value of
Start_Time is replaced by the new one. This causes the last, partially
finished timing interval to be lost. The accumulated time is, however, not
changed.
----------------------------------------------------------------------------*/

void Timer::Start()
  {
    Internal_State = RUNNING;
    Start_Time     = clock();
    return;
  }


/*----------------------------------------------------------------------------
void Timer::Stop();

The following function stops the timer and updates the value of accumulated
time. Notice that the old value of accumulated time is not lost; timer
objects allow frequent starts and stops.
----------------------------------------------------------------------------*/

void Timer::Stop()
  {
    Stop_Time      = clock();
    Internal_State = STOPPED;
    Accumulated   += Stop_Time - Start_Time;
    return;
  }

/*----------------------------------------------------------------------------
long Timer::Time();

The following function returns the total accumulated time in 100th seconds.
Note that if the timer is running when this function is called, it
correctly evaluates the time. The state of the timer is unchanged.

The function is careful to return the accumulated time in a portable way.
The fraction 100/CLOCKS_PER_SEC will adjust the value stored in the
arithmetic type clock_t to the number of 100th seconds on all machines.
There may be a problem, however, with overflow when the multiplication of
100 is done. Notice that it is also assumed that clock_t can hold the
result sensibly.
----------------------------------------------------------------------------*/

long Timer::Time()
  {
    clock_t Total_Time;

    if (Internal_State != RUNNING) {
      Total_Time = ( ((clock_t)100)*Accumulated )/(clock_t)CLOCKS_PER_SEC;
    }
    else {
      Total_Time = Accumulated + clock() - Start_Time;
      Total_Time = ( ((clock_t)100)*Total_Time )/(clock_t)CLOCKS_PER_SEC;
    }
    return (long)Total_Time;
  }

