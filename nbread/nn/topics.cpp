
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "topics.h"

#define MAX_TOPICLINE   128
#define MAX_NMBRTOPICS  250
#define MAJOR             1   // Major version number of native NN.CFG file.
#define MINOR             1   // Minor version number of native NN.CFG file.

/*===========================================================================
NN.CFG Version History

Version 0.0:

The original. This version stored the name of each topic, and the date and
time (no year) of the most recently read posting in that topic. It did not
have an NN.CFG version header.

Version 1.0:

This added the NN.CFG version header and it added the check flag. If the
check flag is 1, NNCHK will check that topic for new postings.

Version 1.1:

The year is now stored as part of the date.

===========================================================================*/

static const Date Dummy = { 1, 1, 0, 0, 1992 };
  // The default last read date for new topics, etc.

static Topic_File Topic_List[MAX_NMBRTOPICS+1];
  // Holds information on the topics themselves.

static char Config_Name[256+1];
  // The name of the configuration file.

/*----------------------------------------------------------------------*/
static char *DeSpace_String(char *The_String)
  {
    char *Pntr = The_String;
    while (*Pntr) {
      if (*Pntr == ' ') *Pntr = '|';
      Pntr++;
    }
    return The_String;
  }

static char *EnSpace_String(char *The_String)
  {
    char *Pntr = The_String;
    while (*Pntr) {
      if (*Pntr == '|') *Pntr = ' ';
      Pntr++;
    }
    return The_String;
  }

/*----------------------------------------------------------------------------
int Date_Greater(Date *Big, Date *Small)

The following function returns TRUE if 'Big' is a later date than 'Small'.
In other words it calculates Big > Small.
----------------------------------------------------------------------------*/

int Date_Greater(const Date *Big, const Date *Small)
  {
    if (Big->Year >  Small->Year) return 1;
    if (Big->Year == Small->Year) {
      if (Big->Month >  Small->Month) return 1;
      if (Big->Month == Small->Month) {
        if (Big->Day >  Small->Day) return 1;
        if (Big->Day == Small->Day) {
          if (Big->Hour >  Small->Hour) return 1;
          if (Big->Hour == Small->Hour) {
            if (Big->Minute > Small->Minute) return 1;
          }
        }
      }
    }
    return 0;
  }

int Date_GreaterOrEqual(const Date *Big, const Date *Small)
  {
    if (Date_Greater(Big, Small)) return 1;
    if (Big->Year   == Small->Year  &&
        Big->Month  == Small->Month &&
        Big->Day    == Small->Day   &&
        Big->Hour   == Small->Hour  &&
        Big->Minute == Small->Minute) return 1;
    return 0;
  }

/*----------------------------------------------------------------------*/

int Check_Topic(Topic_File *The_Topic)
/* Set/reset the New Messages flag on The_Topic if needed */
{
  if (The_Topic->Check == 1) {
    FILE *Time_File;
    char  Name_Buffer[128+1];
    char *Extension;
    int   File_Month;
    int   File_Day;
    int   File_Hour;
    int   File_Minute;
    int   File_Year = 1992; // Old .TIM files don't have a year. Use 1992 as default.
    Date  File_Date;

    // Figure out the .TIM file's name.
    strcpy(Name_Buffer, The_Topic->File_Name);
    if ((Extension = strchr(Name_Buffer, '.')) == NULL || strlen(++Extension) != 3)
      return 1;
    strcpy(Extension, "TIM");

    // Try to open the .TIM file.
    if ((Time_File = fopen(Name_Buffer, "r")) == NULL)
      return 1;
    else
    {
      // Get the date of the last posting in this topic.
      fscanf(Time_File, "%d %d %d %d %d", &File_Month, &File_Day, &File_Hour, &File_Minute, &File_Year);
      File_Date.Month  = File_Month;
      File_Date.Day    = File_Day;
      File_Date.Hour   = File_Hour;
      File_Date.Minute = File_Minute;
      File_Date.Year   = File_Year;
      fclose (Time_File);

      // Let's see if it's greater than the most recently read message in
      //   the topic.
      if (Date_Greater(&File_Date, &(The_Topic->Read_On)))
        The_Topic->New = 1; /* Set the flag for new messages. */
      else
        The_Topic->New = 0; /* Reset the flag for new messages. */
    }
  } /* if (The_Topic->Check == 1) */

  return 0;
}

/*----------------------------------------------------------------------*/

static int Read_MasterFile(char *Master_FileName)
  {
    char  Line_Buffer[MAX_TOPICLINE+2];
    FILE *Master_File;

    Topic_File *The_Topics = Topic_List;

    while (The_Topics - Topic_List < MAX_NMBRTOPICS+1)
      (The_Topics++)->Topic_Description = NULL;
    The_Topics = Topic_List;

    if ((Master_File = fopen(Master_FileName, "r")) == NULL) return 0;

    // Read every line from the master topic file.
    while (fgets(Line_Buffer, MAX_TOPICLINE + 2, Master_File) != NULL)
    {
      char *End_Pntr;
      char *Break_Point;

      /* Kill the '\n' at the end of the line. */
      End_Pntr = strchr(Line_Buffer, '\n');
      if (End_Pntr != NULL) *End_Pntr = '\0';

      /* Break the line in half based on the given delimiter. ( '|' ) */
      Break_Point = strchr(Line_Buffer, '|');

      /* If there is no '|' on the line, it is NOT a valid topic! */
      if (Break_Point != NULL)
      {
        *Break_Point++ = '\0';

        /* Save copies of the individual parts. */
        The_Topics->Topic_Description = (char *)malloc(strlen(Line_Buffer) + 1);
        The_Topics->File_Name         = (char *)malloc(strlen(Break_Point) + 1);
        strcpy(The_Topics->Topic_Description, Line_Buffer);
        strcpy(The_Topics->File_Name        , Break_Point);

        /* Initialize the date that the topic was last read. */
        The_Topics->Read_On = Dummy;

        /* Initialize the Check flag. */
        The_Topics->Check = 1;

        /* Initialize the New flag */
        The_Topics->New = 0;

        /*  Next topic... */
        The_Topics++;
      }

      /* Check to see if we've filled all topics. If so, stop at once. Extra topics ignored. */
      if (The_Topics - Topic_List >= MAX_NMBRTOPICS) break;
    }
    fclose(Master_File);

    return 1;
      // Success!
  }

static int Read_ConfigFile(char *Config_FileName)
  {
    FILE *Config_File;
    char  Config_Line[128+2];
    int   Major_Version = 0, Minor_Version = 0;
    char  Description[128+1];
    int   Temp_Month, Temp_Day, Temp_Hour, Temp_Minute, Temp_Year;
    Topic_File *The_Topics;

    // Try to read the configuration file
    if ((Config_File = fopen(Config_FileName, "r")) != NULL) {

      // Get the first line to see what version we've got.
      fgets(Config_Line, 128+2, Config_File);
      if (sscanf(Config_Line, "Version %d.%d", &Major_Version, &Minor_Version) != 2) {

        // We've got version 0.0. Parse the first line special.
        sscanf(Config_Line, "%s %d %d %d %d",
                 Description,
                 &Temp_Month, &Temp_Day, &Temp_Hour, &Temp_Minute);
        EnSpace_String(Description);
        The_Topics = Topic_List;
        while (The_Topics->Topic_Description != NULL) {
          if (strcmp(The_Topics->Topic_Description, Description) == 0) {
            The_Topics->Read_On.Month  = Temp_Month;
            The_Topics->Read_On.Day    = Temp_Day;
            The_Topics->Read_On.Hour   = Temp_Hour;
            The_Topics->Read_On.Minute = Temp_Minute;
            break;
          }
          The_Topics++;
        }
      }

      // There's a version number on the file. Handle it.

      // VERSION 0.0
      if (Major_Version == 0 && Minor_Version == 0) {

        // Loop on the file to read the other lines. The first line was read above.
        while (fscanf(Config_File, "%s %d %d %d %d",
                 Description,
                 &Temp_Month, &Temp_Day, &Temp_Hour, &Temp_Minute) != EOF) {
          EnSpace_String(Description);
          The_Topics = Topic_List;
          while (The_Topics->Topic_Description != NULL) {
            if (strcmp(The_Topics->Topic_Description, Description) == 0) {
              The_Topics->Read_On.Month  = Temp_Month;
              The_Topics->Read_On.Day    = Temp_Day;
              The_Topics->Read_On.Hour   = Temp_Hour;
              The_Topics->Read_On.Minute = Temp_Minute;
              break;
            }
            The_Topics++;
          }
        }
        fclose (Config_File);
      }

      // VERSION 1.0
      else if (Major_Version == 1 && Minor_Version == 0) {
        int Check_Flag = 1;

        while (fscanf(Config_File, "%d %s %d %d %d %d",
                &Check_Flag,
                 Description,
                 &Temp_Month, &Temp_Day, &Temp_Hour, &Temp_Minute) != EOF) {
          EnSpace_String(Description);
          The_Topics = Topic_List;
          while (The_Topics->Topic_Description != NULL) {
            if (strcmp(The_Topics->Topic_Description, Description) == 0) {
              The_Topics->Check          = Check_Flag;
              The_Topics->Read_On.Month  = Temp_Month;
              The_Topics->Read_On.Day    = Temp_Day;
              The_Topics->Read_On.Hour   = Temp_Hour;
              The_Topics->Read_On.Minute = Temp_Minute;
              break;
            }
            The_Topics++;
          }
        }
        fclose (Config_File);
      }


      // VERSION 1.1
      else if (Major_Version == 1 && Minor_Version == 1) {
        int Check_Flag = 1;

        while (fscanf(Config_File, "%d %s %d %d %d %d %d",
                &Check_Flag,
                 Description,
                 &Temp_Month, &Temp_Day, &Temp_Hour, &Temp_Minute, &Temp_Year) != EOF) {
          EnSpace_String(Description);
          The_Topics = Topic_List;
          while (The_Topics->Topic_Description != NULL) {
            if (strcmp(The_Topics->Topic_Description, Description) == 0) {
              The_Topics->Check          = Check_Flag;
              The_Topics->Read_On.Month  = Temp_Month;
              The_Topics->Read_On.Day    = Temp_Day;
              The_Topics->Read_On.Hour   = Temp_Hour;
              The_Topics->Read_On.Minute = Temp_Minute;
              The_Topics->Read_On.Year   = Temp_Year;
              break;
            }
            The_Topics++;
          }
        }
        fclose (Config_File);
      }


      // UNKNOWN VERSION NUMBER.
      else {

        // Config file has an unknown version number. Return an error.
        return 0;
      }

    } // End of check of Config file.

    /* Now scan the topics and find out which ones have new postings */
    for (The_Topics = Get_First(); The_Topics->Topic_Description != NULL; The_Topics++)
      Check_Topic(The_Topics);

    return 1;
      // Success!
  }

int Open_Topics(char *Master_FileName, char *Config_FileName)
  {
    // Remember the name of the configuration file for later.
    strcpy(Config_Name, Config_FileName);

    // Now let's try to process these files.
    if (Read_MasterFile(Master_FileName) == 0) return 0;
    if (Read_ConfigFile(Config_FileName) == 0) return 0;
    return 1;
  }

int Close_Topics()
  {
    FILE *Config_File;

    // Save configuration file to record dates that the postings were last read, etc.
    if ((Config_File = fopen(Config_Name, "w")) != NULL) {
      Topic_File *A_Topic = Topic_List;

      // Write the version header into the .CFG file.
      fprintf(Config_File, "Version %d.%d\n", MAJOR, MINOR);
      while (A_Topic->Topic_Description != NULL) {
        fprintf(Config_File, "%d %s %d %d %d %d %d\n",
          A_Topic->Check,
          DeSpace_String(A_Topic->Topic_Description),
          A_Topic->Read_On.Month,
          A_Topic->Read_On.Day,
          A_Topic->Read_On.Hour,
          A_Topic->Read_On.Minute,
          A_Topic->Read_On.Year
        );
        A_Topic++;
      }
      fclose(Config_File);
    }

    return 1;
  }

Topic_File *Get_First()
  {
    return Topic_List;
  }

int Nmbr_Topics()
  {
    Topic_File *A_Topic = Topic_List;
    int         Count   = 0;

    while (A_Topic->Topic_Description != NULL) {
      Count++;
    }

    return Count;
  }

