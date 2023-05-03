/*****************************************************************************
File          : HELPER.H
Programmer    : ????????
Date          : February 1992
Last Revision : February 1992

Purpose       : To allow the use of Help screens within the NN software.
*****************************************************************************/

#ifndef HELPER_HPP
#define HELPER_HPP

typedef enum { TOPIC_LIST, MESSAGE_LIST, MESSAGE_DISPLAY } Help_Topic;

void Helper(Help_Topic T);

#endif

