//-----------------------------------------------------------------------
// FILE    : ConfigurationFile.java
// SUBJECT : 
// AUTHOR  : (C) Copyright 2010 by Peter C. Chapin <PChapin@vtc.vsc.edu>
//
//-----------------------------------------------------------------------

package org.pchapin.jibber;

import java.io.*;
import java.util.HashMap;

/**
 * This class reads a simple configuration file and provides the program with access to the
 * configuration information. The file consists of lines in the form
 * 
 * name="value"
 * 
 * The name contains only normal identifier characters (letters, digits, and the underscore).
 * The value can contain any text but the quotation marks are required in all cases. There can
 * be only one (name, value) pair on each line. Blank lines are ignored. Leading white space,
 * trailing white space, and white space around the '=' character are all ignored. White space
 * inside the value string is part of the value. Comments start with a '#' character and go to
 * the end of the line.
 * 
 * This class stores the configuration information into a map that the rest of the program can
 * use.
 * 
 * @author Peter C. Chapin
 */
public class ConfigurationFile {
	
    /** Contains a map of (name, value) pairs. */
    private HashMap<String, String> configuration = new HashMap<String, String>();
	
    /**
     * Opens and reads the named file looking for (name, value) pairs. When the constructor
     * returns the file has been closed again.
     * 
     * @param fileName The name of the file to open. If the file can not be opened there is no
     * effect (no exception is thrown and the result map of (name, value) pairs is empty).
     * 
     * @bug Although ignoring configuration files that don't exist is reasonable, this method
     * should probably indicate when the file was opened properly but then could not be read.
     * Such errors should be rare, however.
     * 
     * @bug This method does not allow the value to contain an '=' character. This is a minor
     * restriction, but it may come as a surprise to some users.
     */
    public ConfigurationFile(String fileName)
    {
        try {
            BufferedReader inputFile =
                new BufferedReader(new FileReader(fileName));
            try {
                String line;
                while ((line = inputFile.readLine()) != null) {
                    line = killComments(line);
                    line = trimWhitespace(line);
                    if (line.length() == 0) continue;
                    String[] fields = line.split("\\s*=\\s*");
					
                    // Do some sanity checking on the line. Ignore it if it looks bad.
                    if (fields.length != 2          ||
                        fields[0].length() < 1      ||
                        fields[1].length() < 2      ||
                        fields[1].charAt(0) != '\"' ||
                        fields[1].charAt(fields[1].length() - 1) != '\"') continue;
                    
                    fields[1] = fields[1].substring(1, fields[1].length() - 1);
                    configuration.put(fields[0], fields[1]);
                }
            }
            catch (Exception e) { /* Ignore */ }
            inputFile.close();
        }
        catch (Exception e) { /* Ignore */ }
    }
	
    /**
     * Gain access to the current configuration map.
     * 
     * @return A HashMap containing the (name, value) pairs in the current configuration.
     * Modifications made to this map are reflected in the return value from later calls to this
     * method.
     */
    HashMap<String, String> getConfiguration()
    {
        return configuration;
    }
	
    /**
     * Remove all text from '#' to the end of the string.
     * 
     * @param line The string of text to modify.
     * 
     * @return The modified string. Note that in many cases the returned string will have
     * trailing white space characters. For example an input string such as "Hello! # This is a
     * comment" is returned as "Hello! ".
     * 
     * @bug This method currently treats '#' inside quoted substrings as a comment character.
     * This should be fixed.
     */
    private String killComments(String line)
    {
        int index = line.indexOf('#');
        if (index == -1) return line;
        return line.substring(0, index);
    }
	
    /**
     * Remove leading and trailing white space characters from the given string. White space is
     * defined as ' ' and '\t'.
     * 
     * @param line The string to modify.
     * 
     * @return The original string without leading and trailing whitespace. If the original
     * string was all white space, the returned string will be empty.
     */
    private String trimWhitespace(String line)
    {
        int first = 0, last = line.length();
        while (first != last) {
            char current = line.charAt(first);
            if (!(current == ' ' || current == '\t')) break;
            ++first;
        }
        if (first == last) return "";
        --last;
        while (last != first) {
            char current = line.charAt(last);
            if (!(current == ' ' || current == '\t')) break;
            --last;
        }
        return line.substring(first, last + 1);
    }
	
}
