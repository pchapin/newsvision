//-----------------------------------------------------------------------
// FILE    : CommandLine.java
// SUBJECT : Class that facilitates command line processing.
// AUTHOR  : (C) Copyright 2010 by Peter C. Chapin <PChapin@vtc.vsc.edu>
//
// TO-DO:
//
// + The method that breaks a string into arguments should be enhanced to support quoted
//   arguments.
//
// + Add support for options with arguments: "-a1" or "-a some-string". Note that the second
//   form will be tricky because "some-string" must be distinguished from a normal argument.
//
// + Add support for long form options: "--apple-count"
//
// + Add support for multiple options after one option introducer.
//
// + Add a method that allows the caller to specify the delimiter argument character during
//   parsing. This will have to be a constructor argument. Doing this will allow this class to
//   be useful for handling arbitrary text "database" records (like the Unix password file and
//   similar such things).
// -----------------------------------------------------------------------

package org.pchapin.jibber;

import java.util.*;

/**
 * This class processes an array of Strings as if it was a standard list of command line
 * arguments. It makes it easy for the client to extract command line options and the "actual"
 * command line arguments.
 * 
 * This class assumes that options all consist of a dash followed by a single letter. There is
 * currently no provision for option arguments, multicharacter option names, or the ability to
 * specify several options after a single dash. All of these features should be added at some
 * point.
 * 
 * Command line arguments are numbered from zero (the command name). Options are not counted in
 * the number of the arguments unless option support has been turned off (useful if the caller
 * wants to handle options his/her own way).
 */
public class CommandLine {

    /** Reference to the given array. */
    private String[] raw;

    /** If false then arguments that start with dash are normal args. */
    private boolean optionsActive = true;

    /**
     * This method defines the meaning of white space. 
     *
     * @param letter The character to check.
     *
     * @return True if the given character is whitespace; false otherwise.
     */
    private boolean isWhite(char letter)
    {
        if (letter ==  ' ' ||
            letter == '\t' ||
            letter == '\n' ||
            letter == '\f') return true;
        return false;
    }


    /**
     * Stores the given array of strings for future analysis.
     *
     * @param given The command line to be analyzed.
     */
    public CommandLine(String[] given)
    {
        raw = given;
    }


    /**
     * Parses the command line into space delimited arguments. This method is useful for dealing
     * with raw lines of text entered by a user or read from a file.
     *
     * @param given The line to parse.
     */
    public CommandLine(String given)
    {
        ArrayList<String> args       = new ArrayList<String>();
        boolean   collecting = false;
        int       start      = 0;

        // Scan down the string breaking off arguments as I go.
        for (int i = 0; i < given.length(); i++) {
            char current = given.charAt(i);
            if (collecting) {
                if (isWhite(current)) {
                    String arg = given.substring(start, i);
                    args.add(arg);
                    collecting = false;
                }
            }
            else {
                if (!isWhite(current)) {
                    start = i;
                    collecting = true;
                }
            }
        }

        // If I was collecting an argument when the string ends, then deal with this last
        // argument as a special case.
        // 
        if (collecting) {
            String arg = given.substring(start);
            args.add(arg);
        }

        // Now convert the linked list of objects into an array of String.
        Object[] objectArray = args.toArray();
        raw = new String[objectArray.length];
        for (int i = 0; i < objectArray.length; i++) {
            raw[i] = (String) objectArray[i];
        }
    }


    /**
     * Set the options flag. If the argument is true then arguments that start with a dash are
     * options (this is the default after construction). If the argument is false then such
     * arguments are normal arguments.
     */
    public boolean processOptions(boolean flag)
    {
        boolean oldValue = optionsActive;
        optionsActive = flag;
        return oldValue;
    }


    /**
     * Returns the number of actual arguments, normally not counting options. If the options
     * flag is false, arguments that look like options are counted as if they were any other
     * argument.
     */
    public int count()
    {
        int argCount = 0;

        // Step down the list of arguments, counting the ones that don't
        // start with a dash (unless options are inactive).
        // 
        for (int i = 0; i < raw.length; i++) {
            if (raw[i].length() == 0) argCount++;
            else {
                if (!optionsActive || raw[i].charAt(0) != '-') argCount++;
            }
        }
        return argCount;
    }


    /**
     * Returns the actual argument with the given index. The first actual argument is at index
     * 0. If an index that is too large is used, an empty String is returned (perhaps it would
     * be better to throw some sort of out of range exception?).
     *
     * Notice that this method does a linear scan of the argument list to find the desired
     * argument. That is necessary in order to properly skip options (which are not counted
     * unless the options flag is false). The linear scan is not likely to be a practical
     * problem since in most cases command line argument lists are short (and used
     * infrequently).
     */
    public String arg(int index)
    {
        int argCount = 0;

        // Step down the list of arguments, ignoring the ones that start with a dash (unless
        // options are inactive).
        // 
        for (int i = 0; i < raw.length; i++) {
            if (raw[i].length() == 0) argCount++;
            else {
                if (!optionsActive || raw[i].charAt(0) != '-') argCount++;
            }

            // If I just advanced argCount past the one I'm interested in, return it.
            // 
            if (argCount > index) return raw[i];
        }

        // If I get here then I ran out of arguments before I found the one I wanted.
        // 
        return new String();
    }


    /**
     * Checks to see if the given option is present on the command line. Returns true if it is,
     * false otherwise. This function is only useful for single character "binary" options. It
     * does not support options that have arguments.
     */
    boolean isPresent(char optionLetter)
    {
        if (!optionsActive) return false;

        for (int i = 0; i < raw.length; i++) {
            if (raw[i].length() >= 2 && raw[i].charAt(0) == '-') {
                if (raw[i].charAt(1) == optionLetter) return true;
            }
        }
        return false;
    }
}
