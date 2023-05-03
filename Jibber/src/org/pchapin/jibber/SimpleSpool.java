//-----------------------------------------------------------------------
// FILE    : SimpleSpool.java
// SUBJECT : Implements the abstract NewsSpool using a simple directory.
// AUTHOR  : (C) Copyright 2010 by Peter C. Chapin <PChapin@vtc.vsc.edu>
//
//-----------------------------------------------------------------------

package org.pchapin.jibber;

import java.io.*;
import java.text.DateFormat;
import java.util.*;

/**
 * This class implements a news spool in the file system. It stores one article per file and
 * generally tries to be as simple as possible. It is not necessarily very efficient.
 */
public class SimpleSpool extends NewsSpool {

    /** This class contains information about a single news group. */
    private static class NewsgroupInfo {

        /** The location for the group's storage. */
        public File path;

        /** The date/time when the group was created. */
        public Date creationDate;

        /** The lowest article number in the group. */
        public int low;

        /** The highest article number in the group. */
        public int high;

        /**
         * Initializes the fields of a NewsgroupInfo object.
         *
         * @param p Path to the group's directory on disk.
         *
         * @param c Date the group was created.
         *
         * @param l Low article number in the group.
         *
         * @param h High article number in the group.
         */
        public NewsgroupInfo(File p, Date c, int l, int h)
        {
            // Initialize the fields of the object.
            path         = p;
            creationDate = c;
            low          = l;
            high         = h;
        }
    }


    /** Used to output messages to the console (write only). */
    private ConsoleOutput theConsole;

    /** The location of the SimpleSpool. */
    private File mySpoolDir;

    /** Associates group names with information about the group. */
    private HashMap<String, NewsgroupInfo> newsgroupList;


    /**
     * Prepares this spool object for use. This method reads the group list from the spool into
     * memory. Keeping the entire group list in memory all the time would not be feasible for a
     * large server.
     *
     * @param incomingSpoolDir Name of top level directory for the spool.
     *
     * @param incomingConsole Reference to a ConsoleOutput object where messages can be written.
     */
    SimpleSpool(String incomingSpoolDir, ConsoleOutput incomingConsole)
    {
        theConsole    = incomingConsole;
        mySpoolDir    = new File(incomingSpoolDir);
        newsgroupList = new HashMap<String, NewsgroupInfo>();
        // Need to also read the current newsgroup file from disk.

        readGroupList();
        theConsole.write(
            "SimpleSpool: Initialized. Using: " + mySpoolDir + "\n");
    }
    
    /**
     * This thread does some spool maintenance. In this version of the class, there is nothing
     * to do so it terminates right away. It isn't needed.
     */
    public void run()
    {
        return;
    }


    /**
     * This method is called when the main program wants to shut down the news spool. It
     * arranges for the spool thread (above) to end and performs other clean up duties as well.
     * This version assumes that no other calls will be performed on a SimpleSpool object after
     * this method has been called. There is no protection for that, however.
     */
    public synchronized void terminate()
    {
        writeGroupList();
    }


    /** Return a list of all the newsgroups in this SimpleSpool. */
    public synchronized List<String> getGroups()
    {
        ArrayList<String> result = new ArrayList<String>();
        Set<Map.Entry<String, NewsgroupInfo>> entries = newsgroupList.entrySet();
        Iterator<Map.Entry<String, NewsgroupInfo>> it = entries.iterator();
        while (it.hasNext()) {
            Map.Entry<String, NewsgroupInfo> entry = it.next();
            String groupName = (String)entry.getKey();
            NewsgroupInfo groupInfo = (NewsgroupInfo)entry.getValue();
            groupName = groupName + " " + groupInfo.high + " " + groupInfo.low + " n";
            result.add(groupName);
        }
        return result;
    }
    
    /** Return the extent of the given group. */
    public synchronized NewsSpool.GroupExtent getGroupExtent(String groupName)
    {
    	NewsgroupInfo groupInfo = (NewsgroupInfo)newsgroupList.get(groupName);
    	if (groupInfo == null) return null;
    	
    	GroupExtent result = new GroupExtent();
    	result.low   = groupInfo.low;
    	result.high  = groupInfo.high;
    	result.count = groupInfo.high - groupInfo.low + 1;
    	return result;
    }


    /** Return true if we have the given message ID. */
    public synchronized boolean isInSpool(String MessageID)
    {
        return false;
    }


    /** Return just the header of the article specified by the given ID. */
    public synchronized Article getHead(String MessageID)
    {
        return new Article(new ArrayList<String>());
    }


    /**
     * Return just the header of the article specified by group name and
     * article number.
     */
    public synchronized Article getHead(String group, int articleNum)
    {
        return new Article(new ArrayList<String>());
    }


    /** Return just the body of the article specified by the given ID. */
    public synchronized List<String> getBody(String MessageID)
    {
        return new ArrayList<String>();
    }


    /**
     * Return just the body of the article specified by group name and
     * article number.
     */
    public synchronized List<String> getBody(String group, int articleNum)
    {
        return new ArrayList<String>();
    }


    /**
     * This method creates a new news group and makes sure that it is initially empty. If the
     * group already exists there is no effect.
     *
     * @param groupName The name of the new group to be created.
     */
    public synchronized void createGroup(String groupName)
        throws NewsSpoolException
    {
        // Create the directory where articles in this group will be stored.
        File path =
            new File(mySpoolDir, groupName.replace('.', File.separatorChar));
        if (path.mkdirs() == false)
            throw new NewsSpoolException("Unable to create group");

        // Add this group to the newsgroup list.
        NewsgroupInfo thisGroup = new NewsgroupInfo(path, new Date(), 0, 0);
        newsgroupList.put(groupName, thisGroup);
    }


    /**
     * This method deletes a group in the news spool. If the specified group does not exist,
     * there is no effect.
     *
     * @param groupName The name of the group.
     */
    public synchronized void deleteGroup(String groupName)
        throws NewsSpoolException
    {
        throw
          new NewsSpoolException("Unable to delete group (not implemented)");
    }


    /**
     * This method reads the group list from the spool. It is used when a SimpleSpool object is
     * created as part of the initialization procedure.
     */
    private void readGroupList()
    {
        File       groupFile   = new File(mySpoolDir, "newsgroups");
        DateFormat myFormatter = DateFormat.getDateTimeInstance();
        newsgroupList.clear();

        try {
            BufferedReader input = new BufferedReader(
                new InputStreamReader(
                    new FileInputStream(groupFile), "US-ASCII"));

            String line;
            while ((line = input.readLine()) != null) {

                // Start by breaking the line into its parts (Like Perl's split. Maybe there is
                // an easier way to do this). Notice that I'm assuming there are five parts.
                // 
                String[] parts = new String[5];
                int index = 0;
                int end   = 0;
                int i     = 0;
                while ((end = line.indexOf('|', index)) != -1) {
                    parts[i] = line.substring(index, end);
                    index = end + 1;
                    i++;
                }
                parts[i] = line.substring(index);

                // Now construct a suitable NewsgroupInfo object and add it to the list.
                // 
                File theFile = new File(parts[1]);
                Date theDate = myFormatter.parse(parts[2]);
                int  low     = Integer.parseInt(parts[3]);
                int  high    = Integer.parseInt(parts[4]);
                NewsgroupInfo groupInfo =
                    new NewsgroupInfo(theFile, theDate, low, high);
                newsgroupList.put(parts[0], groupInfo);
            }

            input.close();
        }

        // If the file isn't there, just ignore it. Return with an empty list.
        catch (FileNotFoundException e) { }

        // If there is an error while reading the file, return with what I've got so far.
        // 
        catch (IOException ioError) {
            theConsole.write(
                "SimpleSpool: Error reading newsgroups file: " +
                ioError + "\n"
            );
        }

        // If I can't figure out the format, return with what I've got so far.
        catch (java.text.ParseException e) {
            theConsole.write(
                "SimpleSpool: Error parsing newsgroups file: " + e + "\n");
        }
    }


    /**
     * This method writes the group list back out to disk. It is used when a SimpleSpool object
     * is being shut down. The group list is maintained in memory while the SimpleSpool object
     * is being used.
     */
    private void writeGroupList()
    {
        Set<String>      keys = newsgroupList.keySet();
        Iterator<String> it = keys.iterator();
        File             groupFile = new File(mySpoolDir, "newsgroups");
        DateFormat       myFormatter = DateFormat.getDateTimeInstance();

        try {

            BufferedWriter output = new BufferedWriter(
                new OutputStreamWriter(
                    new FileOutputStream(groupFile), "US-ASCII"));

            // Iterate over all the keys in the HashMap.
            while (it.hasNext()) {
                String groupName = (String)it.next();
                NewsgroupInfo groupInfo =
                    (NewsgroupInfo)newsgroupList.get(groupName);
                output.write(groupName, 0, groupName.length());
                output.write("|", 0, 1);
                String thePath = groupInfo.path.toString();
                output.write(thePath, 0, thePath.length());
                output.write("|", 0, 1);
                String theDate = myFormatter.format(groupInfo.creationDate);
                output.write(theDate, 0, theDate.length());
                output.write("|", 0, 1);
                String theLow = Integer.toString(groupInfo.low);
                output.write(theLow, 0, theLow.length());
                output.write("|", 0, 1);
                String theHigh = Integer.toString(groupInfo.high);
                output.write(theHigh, 0, theHigh.length());
                output.newLine();
            }

            output.close();
        }
        catch (FileNotFoundException e) {
            theConsole.write(
                "SimpleSpool: Error writing newsgroups file: " + e + "\n");
        }

        catch (IOException ioError) {
            theConsole.write(
                "SimpleSpool: Error writing newsgroups file: " +
                ioError + "\n"
            );
        }
    }

}
