
#ifndef TOPICS_H
#define TOPICS_H

typedef struct {
  int Month;
  int Day;
  int Hour;
  int Minute;
  int Year;
} Date;

int Date_Greater(const Date *Big, const Date *Small);
  // Returns 1 if Big > Small (that is, later).

int Date_GreaterOrEqual(const Date *Big, const Date *Small);
  // Returns 1 if Big >= Small (that is, later or same).

typedef struct {
  char  *Topic_Description;  /* String presented to user to summarize the topic. */
  char  *File_Name;          /* Name of topic file (should be fully qualified).  */
  int    Check;              /* =nonzero if this topic is to be checked.         */
  int    New;                /* =nonzero if this topic contains unread postings  */
  Date   Read_On;            /* Date of most recently read message.              */
} Topic_File;

int Open_Topics(char *Master_FileName, char *Config_Name);
  /* Open the specified master topic file and populate internal data structures.
       Returns 1 if successful; 0 otherwise. The second parameter is the name
       of the "configuration file" where records are kept about which topics
       are being tracked, etc. */

/* The functions below will not work unless Open_Topics called successfully. */

int Close_Topics(void);
  /* Writes back the config file. */

Topic_File *Get_First(void);
  /* Returns a pointer to first element in array of Topic_File objects. The
       last element contains a NULL Topic_Description. */

int Nmbr_Topics(void);
  /* Returns the number of topics defined. */

int Check_Topic(Topic_File *The_Topic);
  /* Updates the New (messages) flag on the specified topic. */

#endif

