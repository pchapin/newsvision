//-----------------------------------------------------------------------
// FILE    : NewsSpool.java
// SUBJECT : Class that abstracts a news spool.
// AUTHOR  : (C) Copyright 2010 by Peter C. Chapin <PChapin@vtc.vsc.edu>
//
//-----------------------------------------------------------------------

package org.pchapin.jibber;

import java.util.List;

/**
 * @author Peter C. Chapin
 * 
 * This abstract class defines the interface to news spools. Subclasses provide for different
 * news spool types. Each news spool type stores articles in its own way. This class provides a
 * common interface to all such stores as well as a few helper methods to facilitate the
 * creation of concrete news spool classes.
 */
public abstract class NewsSpool implements java.lang.Runnable {

    /**
     * @author Peter C. Chapin
     * 
     * This class is used to hold information about the number of articles in a particular
     * group. It is returned from the getGroupExtent method. This class is intended to be used
     * similar to the way C structs are used and so all of its fields are public.
     */
    public static class GroupExtent {
        public int count; // Estimate of number of articles in group.
        public int low;   // Article number of first article.
        public int high;  // Article number of last article.
    }

    /**
     * This method performs internal spool maintenance.
     */
    public abstract void run();

    /**
     * This method returns a list of all the groups in the news spool.
     * 
     * @return The returned List contains one group (as a String) in each element. In addition
     * to the group names, each string also contains the last article number, the first article
     * number, and the posting flag. The format is as described in RFC-977 for the LIST command.
     */
    public abstract List<String> getGroups();

    /**
     * This method returns the "extent" of the given group.
     * 
     * @param groupName The name of the group to return information about.
     * 
     * @return A groupExtent object that contains information about the specified group's size
     * (number of articles), low article number, and high article number. If the specified group
     * does not exist, this method returns null.
     */
    public abstract GroupExtent getGroupExtent(String groupName);

    /**
     * This method returns an article from the news spool. Both the header and the body are
     * returned. This method might be more efficient than calling getHead followed by getBody.
     * However, the default implementation provided here just uses that approach.
     * 
     * @param messageID The NNTP message ID of the desired article.
     * 
     * @return The Article object returned contains an empty body if the actual article's body
     * is empty. Contrast this with the behavior of the getHead method.
     */
    public Article getArticle(String messageID) {
        Article result = getHead(messageID);
        result.setBody(getBody(messageID));

        return result;
    }

    /**
     * This method returns an article from the news spool. Both the header and the body are
     * returned. This method might be more efficient than calling getHead followed by getBody.
     * However, the default implementation provided here just uses that approach.
     * 
     * @param group The name of the group.
     * 
     * @param articleNum The number of the article within its group.
     * 
     * @return The Article object returned contains an empty body if the actual article's body
     * is empty. Contrast this with the behavior of the getHead method.
     */
    public Article getArticle(String group, int articleNum) {
        Article result = getHead(group, articleNum);
        result.setBody(getBody(group, articleNum));

        return result;
    }

    /**
     * This method returns the header of an article in the news spool.
     * 
     * @param messageID The NNTP message ID of the desired article.
     * 
     * @return The Article object returned contains a null body regardless of the existence (or
     * not) of a body in the actual article. Contrast this with the behavior of the getArticle
     * method.
     */
    public abstract Article getHead(String messageID);

    /**
     * This method returns the header of the article in the news spool.
     * 
     * @param group The name of the group.
     * 
     * @param articleNum The number of the article within its group.
     * 
     * @return The Article object returned contains a null body regardless of the existance (or
     * not) of a body in the actual article. Contrast this with the behavior of the getArticle
     * method.
     */
    public abstract Article getHead(String group, int articleNum);

    /**
     * This method returns the body of an article in the news spool.
     * 
     * @param messageID The NNTP message ID of the desired article.
     * 
     * @return The returned List contains Strings with each line of the article in a separate
     * element of the List. The blank line that separates the header from the body is
     * technically not part of the body and hence not returned by this method. If the article
     * has no body, and empty ArrayList is returned.
     */
    public abstract List<String> getBody(String messageID);

    /**
     * This method returns the body of an article in the news spool.
     * 
     * @param group The name of the group.
     * 
     * @param articleNum The number of the article within its group.
     * 
     * @return The returned List contains Strings with each line of the article in a separate
     * element of the List. The blank line that separates the header from the body is
     * technically not part of the body and hence not returned by this method. If the article
     * has no body, and empty ArrayList is returned.
     */
    public abstract List<String> getBody(String group, int articleNum);

    /**
     * This method determines if the specific message is in the news spool.
     * 
     * @param messageID The NNTP message ID of the desired article.
     * 
     * @return true if the article is in the spool; false otherwise.
     */
    public abstract boolean isInSpool(String messageID);

    /**
     * This method creates a new group in the news spool. If the specified group already exists,
     * there is no effect.
     * 
     * @param groupName The name of the group.
     */
    public abstract void createGroup(String groupName)
            throws NewsSpoolException;

    /**
     * This method deletes a group in the news spool. If the specified group does not exist,
     * there is no effect.
     * 
     * @param groupName The name of the group.
     */
    public abstract void deleteGroup(String groupName)
            throws NewsSpoolException;

    /**
     * Override this method in subclasses so that an external agent can inform the spool that it
     * is time to shut down its thread. If the subclass doesn't use the thread than nothing
     * needs to be done.\
     */
    public synchronized void terminate() {
        return;
    }
}
