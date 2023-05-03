//-----------------------------------------------------------------------
// FILE    : ConnectionHandler.java
// SUBJECT : Class that handles teh interaction with a client.
// AUTHOR  : (C) Copyright 2010 by Peter C. Chapin <PChapin@vtc.vsc.edu>
//
// TO-DO:
//
// + Currently bytes are read out of the connection only one at a time. That doesn't sound very
//   efficient. Probably it should be improved.
//
//-----------------------------------------------------------------------

package org.pchapin.jibber;

import java.io.*;
import java.net.*;
import java.util.List;
import java.util.Iterator;

/**
 * An object from this class is created for each connection that arrives. It maintains its own
 * thread to handle interactions with a single remote client. This is where we speak NNTP to the
 * remote side. We use the methods of NewsSpool to look up the information the client desires.
 */

public class ConnectionHandler implements java.lang.Runnable {

    // Documentation comments for these fields?
    private Socket        thisConnection;
    private ConsoleOutput theConsole;
    private NewsSpool     currentSpool;
    private int           idNumber;
    private String        currentGroup = null;
    private int           currentArticle = 0;

    // Used by external agents to signal the desire for clean termination.
    private boolean timeToTerminate = false;


    /**
     * This method just stores references to various critical objects that we've been given and
     * otherwise initializes things.
     *
     * @param incomingConnection Reference to a Socket object that represents the server
     * endpoint of the connection.
     *
     * @param incomingConsole Reference to a ConsoleOutput object where messages can be written.
     *
     * @param incomingSpool Reference to a NewsSpool object where the article database is
     * located.
     *
     * @param incomingID ID number for this connection. Useful for logging and accounting
     * purposes.
     */
    public ConnectionHandler(
        Socket        incomingConnection,
        ConsoleOutput incomingConsole,
        NewsSpool     incomingSpool,
        int           incomingID
    )
    {
        thisConnection = incomingConnection;
        theConsole     = incomingConsole;
        currentSpool   = incomingSpool;
        idNumber       = incomingID;
    }

    /**
     * This is the "main" method of this connection handler. Each connection has its own run
     * method. Here is where we accept NNTP requests and provide appropriate responses.
     */

    public void run()
    {
        try {
            thisConnection.setSoTimeout(10000);

            InputStream  incomingBytes = thisConnection.getInputStream();
            OutputStream outgoingBytes = thisConnection.getOutputStream();
            PrintStream  outgoingText  = new PrintStream(outgoingBytes);

            outgoingText.println("201 Jibber ready. Read-only");

            byte[] rawLine = new byte[1024];

            while (true) {

                int nextByte = -1;
                int i = 0;

                // Attempt to read a line of text from the connection. Note that this reads only
                // one byte at a time. I'm sure there is a better way.
                // 
                while (true) {

                    // If we time out either break out or ignore depending.
                    try {
                        nextByte = incomingBytes.read();
                    }
                    catch (IOException ioError) {
                        if (timeToTerminate) break;
                        continue;
                    }

                    // End on '\n' or EOF. Ignore '\r'.
                    if (nextByte == '\n' || nextByte == -1) break;
                    if (nextByte == '\r') continue;
                    rawLine[i] = (byte)nextByte;
                    i++;
                }
 
                // If we broke out of the loop above due to it being time to terminate, then
                // break out of this loop as well. Go down to where the connection is closed,
                // etc.
                // 
                if (timeToTerminate) break;

                // If we encountered EOF (premature close from the client) end.
                if (nextByte == -1) break;

                // Otherwise make this into a string and process this client request.
                String line = new String(rawLine, 0, i);

                theConsole.write(
                    "ConnectionHandler(" + idNumber + "): Got: " + line + "\n"
                );

                // Break line into space delimited arguments.
                CommandLine fullCommand = new CommandLine(line);
                fullCommand.processOptions(false);
                String command = fullCommand.arg(0);

                // What command did we get from the other side? Handle it. This list should
                // probably be rearranged so that the commonly used commands are checked first.
                // I could also imagine looking these up in a hash or something.
                // 
                if (command.equalsIgnoreCase("ARTICLE")) {
                    commandARTICLE(incomingBytes, outgoingText, fullCommand);
                }
                else if (command.equalsIgnoreCase("BODY")) {
                    commandBODY(incomingBytes, outgoingText, fullCommand);
                }
                else if (command.equalsIgnoreCase("HEAD")) {
                    commandHEAD(incomingBytes, outgoingText, fullCommand);
                }
                else if (command.equalsIgnoreCase("STAT")) {
                    commandSTAT(incomingBytes, outgoingText, fullCommand);
                }
                else if (command.equalsIgnoreCase("GROUP")) {
                    commandGROUP(incomingBytes, outgoingText, fullCommand);
                }
                else if (command.equalsIgnoreCase("HELP")) {
                    commandHELP(incomingBytes, outgoingText, fullCommand);
                }
                else if (command.equalsIgnoreCase("IHAVE")) {
                    commandIHAVE(incomingBytes, outgoingText, fullCommand);
                }
                else if (command.equalsIgnoreCase("LAST")) {
                    commandLAST(incomingBytes, outgoingText, fullCommand);
                }
                else if (command.equalsIgnoreCase("LIST")) {
                    commandLIST(incomingBytes, outgoingText, fullCommand);
                }
                else if (command.equalsIgnoreCase("NEWGROUPS")) {
                    commandNEWGROUPS(incomingBytes, outgoingText, fullCommand);
                }
                else if (command.equalsIgnoreCase("NEWNEWS")) {
                    commandNEWNEWS(incomingBytes, outgoingText, fullCommand);
                }
                else if (command.equalsIgnoreCase("NEXT")) {
                    commandNEXT(incomingBytes, outgoingText, fullCommand);
                }
                else if (command.equalsIgnoreCase("POST")) {
                    commandPOST(incomingBytes, outgoingText, fullCommand);
                }
                else if (command.equalsIgnoreCase("QUIT")) {
                    commandQUIT(incomingBytes, outgoingText, fullCommand);
                    break;
                }
                else if (command.equalsIgnoreCase("SLAVE")) {
                    commandSLAVE(incomingBytes, outgoingText, fullCommand);
                }
                else {
                    outgoingText.println("500 command unrecognized");
                }
            }

            incomingBytes.close();
            outgoingText.close();
            thisConnection.close();
        }
        catch (IOException ioError) {
            theConsole.write(
                "ConnectionHandler(" + idNumber + "): Connection failed: " +
                ioError + "\n");
        }
    }


    /**
     * This method is used by external agents to signal when this thread should terminate.
     * Should this method by synchronized?
     */
    public void terminate()
    {
        timeToTerminate = true;
    }


    // The following methods implement the various NNTP commands. See the RFC for documentation
    // on what each is supposed to do. The methods are arranged here in alphabetical order for
    // easy locating. Note that this list currently only supports the commands defined in
    // RFC-977. Support for some of the extensions in RFC-2980 should be added at some point.

    /**
     * This method implements the ARTICLE command from RFC-977.
     *
     * @param incoming The source of text from the client.
     *
     * @param outgoing A place where text to the client can be written.
     *
     * @para fullCommand The parsed command line sent by the client.
     */
    private void commandARTICLE(
      InputStream incoming, PrintStream outgoing, CommandLine fullCommand)
    {
        outgoing.println("500 command unrecognized");
    }


    /**
     * This method implements the BODY command from RFC-977.
     *
     * @param incoming The source of text from the client.
     *
     * @param outgoing A place where text to the client can be written.
     *
     * @param fullCommand The parsed command line sent by the client.
     */
    private void commandBODY(
      InputStream incoming, PrintStream outgoing, CommandLine fullCommand)
    {
        outgoing.println("500 command unrecognized");
    }


    /**
     * This method implements the HEAD command from RFC-977.
     *
     * @param incoming The source of text from the client.
     *
     * @param outgoing A place where text to the client can be written.
     *
     * @param fullCommand The parsed command line sent by the client.
     */
    private void commandHEAD(
      InputStream incoming, PrintStream outgoing, CommandLine fullCommand)
    {
        outgoing.println("500 command unrecognized");
    }


    /**
     * This method implements the STAT command from RFC-977.
     *
     * @param incoming The source of text from the client.
     *
     * @param outgoing A place where text to the client can be written.
     *
     * @param fullCommand The parsed command line sent by the client.
     */
    private void commandSTAT(
      InputStream incoming, PrintStream outgoing, CommandLine fullCommand)
    {
        outgoing.println("500 command unrecognized");
    }


    /**
     * This method implements the GROUP command from RFC-977.
     *
     * @param incoming The source of text from the client.
     *
     * @param outgoing A place where text to the client can be written.
     *
     * @param fullCommand The parsed command line sent by the client.
     */
    private void commandGROUP(
      InputStream incoming, PrintStream outgoing, CommandLine fullCommand)
    {
    	NewsSpool.GroupExtent theExtent =
            currentSpool.getGroupExtent(fullCommand.arg(1));
    	if (theExtent == null) {
            outgoing.println("411 no such newsgroup");
    	}
    	else {
            currentGroup   = fullCommand.arg(1);
            currentArticle = theExtent.low;
            outgoing.println("211 " + theExtent.count + " "
                                    + theExtent.low   + " " 
                                    + theExtent.high  + " " + fullCommand.arg(1));
    	}
        
    }


    /**
     * This method implements the HELP command from RFC-977. Currently it sends a response of
     * "Sorry, no help". At some point this should be elaborated on to provide help for the
     * commands the server actually supports.
     *
     * @param incoming The source of text from the client.
     *
     * @param outgoing A place where text to the client can be written.
     *
     * @param fullCommand The parsed command line sent by the client.
     */
    private void commandHELP(
      InputStream incoming, PrintStream outgoing, CommandLine fullCommand)
    {
        outgoing.println("100 help follows");
        outgoing.println("Sorry, no help");
        outgoing.println(".");
    }


    /**
     * This method implements the IHAVE command from RFC-977.
     *
     * @param incoming The source of text from the client.
     *
     * @param outgoing A place where text to the client can be written.
     *
     * @param fullCommand The parsed command line sent by the client.
     */
    private void commandIHAVE(
      InputStream incoming, PrintStream outgoing, CommandLine fullCommand)
    {
        outgoing.println("500 command unrecognized");
    }


    /**
     * This method implements the LAST command from RFC-977.
     *
     * @param incoming The source of text from the client.
     *
     * @param outgoing A place where text to the client can be written.
     *
     * @param fullCommand The parsed command line sent by the client.
     */
    private void commandLAST(
      InputStream incoming, PrintStream outgoing, CommandLine fullCommand)
    {
        outgoing.println("500 command unrecognized");
    }


    /**
     * This method implements the LIST command from RFC-977. It asks the news spool for a list
     * of all groups it supports and it passes that information on to the client.
     *
     * @param incoming The source of text from the client.
     *
     * @param outgoing A place where text to the client can be written.
     *
     * @param fullCommand The parsed command line sent by the client.
     */
    private void commandLIST(
      InputStream incoming, PrintStream outgoing, CommandLine fullCommand)
    {
        outgoing.println("251 list of newsgroups follows");
        List<String> groupList = currentSpool.getGroups();
        Iterator<String> it = groupList.iterator();
        while (it.hasNext()) {
            String groupInfo = it.next();
            outgoing.println(groupInfo);
        }
        outgoing.println(".");
    }


    /**
     * This method implements the NEWGROUPS command from RFC-977.
     *
     * @param incoming The source of text from the client.
     *
     * @param outgoing A place where text to the client can be written.
     *
     * @param fullCommand The parsed command line sent by the client.
     */
    private void commandNEWGROUPS(
      InputStream incoming, PrintStream outgoing, CommandLine fullCommand)
    {
        outgoing.println("500 command unrecognized");
    }


    /**
     * This method implements the NEWNEWS command from RFC-977.
     *
     * @param incoming The source of text from the client.
     *
     * @param outgoing A place where text to the client can be written.
     *
     * @param fullCommand The parsed command line sent by the client.
     */
    private void commandNEWNEWS(
      InputStream incoming, PrintStream outgoing, CommandLine fullCommand)
    {
        outgoing.println("500 command unrecognized");
    }


    /**
     * This method implements the NEXT command from RFC-977.
     *
     * @param incoming The source of text from the client.
     *
     * @param outgoing A place where text to the client can be written.
     *
     * @param fullCommand The parsed command line sent by the client.
     */
    private void commandNEXT(
      InputStream incoming, PrintStream outgoing, CommandLine fullCommand)
    {
        outgoing.println("500 command unrecognized");
    }


    /**
     * This method implements the POST command from RFC-977.
     *
     * @param incoming The source of text from the client.
     *
     * @param outgoing A place where text to the client can be written.
     *
     * @param fullCommand The parsed command line sent by the client.
     */
    private void commandPOST(
      InputStream incoming, PrintStream outgoing, CommandLine fullCommand)
    {
        outgoing.println("500 command unrecognized");
    }


    /**
     * This method implements the QUIT command from RFC-977.
     *
     * @param incoming The source of text from the client.
     *
     * @param outgoing A place where text to the client can be written.
     *
     * @param fullCommand The parsed command line sent by the client.
     */
    private void commandQUIT(
      InputStream incoming, PrintStream outgoing, CommandLine fullCommand)
    {
        outgoing.println("205 closing connection");
    }


    /**
     * This method implements the SLAVE command from RFC-977.
     *
     * @param incoming The source of text from the client.
     *
     * @param outgoing A place where text to the client can be written.
     *
     * @param fullCommand The parsed command line sent by the client.
     */
    private void commandSLAVE(
      InputStream incoming, PrintStream outgoing, CommandLine fullCommand)
    {
        outgoing.println("500 command unrecognized");
    }

}
