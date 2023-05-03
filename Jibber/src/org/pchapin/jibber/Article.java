//-----------------------------------------------------------------------
// FILE    : Article.java
// SUBJECT : Class that encapsulates RFC-850 messages.
// AUTHOR  : (C) Copyright 2010 by Peter C. Chapin <PChapin@vtc.vsc.edu>
//
//-----------------------------------------------------------------------

package org.pchapin.jibber;

import java.util.ArrayList;
import java.util.List;

/**
 * This class encapsulates RFC-850 style messages, called "articles" for short. It provides
 * methods that allow easy access to the body of the article and to the header fields. The
 * ArrayList objects used with these methods are intended to be ArrayLists of java.lang.String
 * objects.
 */

public class Article {

    private ArrayList<String> theHead;
    private ArrayList<String> theBody;

    /**
     * The constructor requires the client to install a header into the article object. The
     * header might be empty, but it should not be null.
     *
     * @param incomingHead The new header to install as a list of
     * Strings.
     */
    public Article(List<String> incomingHead)
    {
        // It might make sense to verify the sanity of the incoming head... or will that take
        // too much time in the usual case? Perhaps a "sanity checking" method is in order.
        // 
        theHead = new ArrayList<String>(incomingHead);
    }

    /**
     * This method returns the header of the article.
     *
     * @return The returned List contains a header line in separate elements. Extremely long
     * header lines have been joined into a single string; they will need to be resplit, if
     * necessary, by other methods. Any empty header is possible, but this method should never
     * return a null header.
     */
    public List<String> getHead()
    {
        return theHead;
    }

    /**
     * This method returns the body of the article. The blank line that separates the header
     * from the body is not technically part of the body and thus is not returned by this
     * method. Note that this method might return a null body in the case where the Article
     * object is just intended to convey header information.
     */
    public List<String> getBody()
    {
        return theBody;
    }

    /**
     * This method allows the client to install a body into the article object. Any existing
     * body is lost.
     *
     * @param incomingBody The new body to install.
     */
    public void setBody(List<String> incomingBody)
    {
        theBody = new ArrayList<String>(incomingBody);
    }

}
