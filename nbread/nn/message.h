
#ifndef MESSAGE_H
#define MESSAGE_H

#include "topics.h"	  /* For the definition of Date (which should be in another file). */

typedef struct {
  char  *Username;        /* The name of the user who posted the message.   */
  char  *Date_String;     /* The date the message was posted.               */
  Date   Posted_On;       /* The date the message was posted.               */
  char  *Subject_Line;    /* Subject line as entered by poster.             */
  long   FTell_Position;  /* Fill offset to start of message in topic file. */
} Message;

#endif

