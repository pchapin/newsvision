//-----------------------------------------------------------------------
// FILE    : NewsSpoolException.java
// SUBJECT : This class is used by NewsSpool objects to report failures.
// AUTHOR  : (C) Copyright 2010 by Peter C. Chapin <PChapin@vtc.vsc.edu>
//
//-----------------------------------------------------------------------

package org.pchapin.jibber;

public class NewsSpoolException extends Exception {

    public NewsSpoolException(String message)
    {
        super(message);
    }

}
