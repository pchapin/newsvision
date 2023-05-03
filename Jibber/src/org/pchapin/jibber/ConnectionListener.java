//-----------------------------------------------------------------------
// FILE    : ConnectionListener.java
// SUBJECT : This class listens to the NNTP port.
// AUTHOR  : (C) Copyright 2010 by Peter C. Chapin <PChapin@vtc.vsc.edu>
//
//-----------------------------------------------------------------------

package org.pchapin.jibber;

import java.io.*;
import java.net.*;
import java.util.LinkedList;
import java.util.Iterator;

/**
 * This class listens for incoming network connections. It runs a separate thread that waits on
 * the network, periodically checking to see if it should terminate. For each incoming
 * connection, the thread creates a ConnectionHandler object to deal with that connection.
 * 
 * @author Peter C. Chapin
 */
public class ConnectionListener implements java.lang.Runnable {

    /** The standard NNTP port. */
    public static final int SERVER_PORT = 9000; /* 119 */

    /** This listener will use this object for outputting messages. */
    private ConsoleOutput theConsole;

    /** All connections serviced by this listener will use this spool. */
    private NewsSpool theSpool;

    /** This flag is used to indicate when it is time to terminate. */
    private boolean timeToTerminate = false;

    /**
     * This class holds the information about each connection that I need to maintain in order
     * to properly clean things up when the time comes. I am treating it like a C structure.
     */
    static class ConnectionInfo {
        public Thread            theThread;
        public ConnectionHandler theHandler;

        public ConnectionInfo(Thread iThread, ConnectionHandler iHandler)
        {
            theThread  = iThread;
            theHandler = iHandler;
        }
    }

    /**
     * Holds the ConnectionInfos for all active (or recently died) connections.
     */
    private LinkedList<ConnectionInfo> connectionList = new LinkedList<ConnectionInfo>();

    /**
     * Stores a references to critical objects for later use.
     *
     * @param incomingSpool Reference to the news spool (passed down to the ConnectionHandler
     * objects that are created.
     *
     * @param incomingConsole Reference to the ConsoleOutput object where messages can be
     * written.
     */
    public ConnectionListener(
        NewsSpool incomingSpool, ConsoleOutput incomingConsole
    )
    {
        theConsole = incomingConsole;
        theSpool   = incomingSpool;
    }


    /**
     * This method listens to the network and creates a ConnectionHandler object for every
     * connection that arrives.
     */
    public void run()
    {
        int connectionCount = 0;

        theConsole.write(
           "ConnectionListener: Listening on port " + SERVER_PORT + "\n"
        );
        ServerSocket mainSocket = null;
        try {
            mainSocket = new ServerSocket(SERVER_PORT);
            mainSocket.setSoTimeout(10000);

            while (true) {

                // Attempt to accept a new connection. Handle timeouts.
                Socket incomingConnection = null;
                try {
                    incomingConnection = mainSocket.accept();
                }
                catch (IOException ioError) {

                    // If it's time to end, break out of the loop.
                    if (timeToTerminate) {

                        // Tell active connections to end.
                        Iterator<ConnectionInfo> it = connectionList.iterator();
                        while (it.hasNext()) {
                            ConnectionInfo current = (ConnectionInfo)it.next();
                            current.theHandler.terminate();
                        }

                        // Now wait for them to end. That way this thread won't end until all
                        // the threads it manages also end. Is there a cleaner way to do this?
                        //
                        it = connectionList.iterator();
                        while (it.hasNext()) {
                            ConnectionInfo current = (ConnectionInfo)it.next();
                            current.theThread.join();
                        }
                        break;
                    }

                    // Otherwise loop back and try again.
                    else {

                        // Remove items from the connectionList for dead connections.
                        // 
                        Iterator<ConnectionInfo> it = connectionList.iterator();
                        while (it.hasNext()) {
                            ConnectionInfo current = (ConnectionInfo)it.next();
                            if (!current.theThread.isAlive()) {
                                it.remove();
                            }
                        }
                        continue;
                    }
                } // End of try... catch around mainSocket.accept()

                // If a connection was accepted normally, deal with it.
                connectionCount++;
                InetAddress peerAddress = incomingConnection.getInetAddress();

                // Print out some information on the console.
                theConsole.write(
                    "ConnectionListener: New connection " + connectionCount +
                    ": " + peerAddress + "\n"
                );
                
                // Deal with this connection.
                ConnectionHandler myHandler = new ConnectionHandler(
                    incomingConnection, theConsole, theSpool, connectionCount
                );
                Thread myThread = new Thread(myHandler);
                myThread.start();
                ConnectionInfo newConnection = new ConnectionInfo(
                    myThread, myHandler
                );
                connectionList.add(newConnection);
            }
        }

        // If we encounter some sort of I/O error, die.
        catch (IOException ioError) {
            theConsole.write(
                "ConnectionListener: IO Exception caught: " + ioError + "\n"
            );
            theConsole.write("ConnectionListener: FATAL! Not listening\n");
        }

        // If the attempts to join (when terminating) fail, die. This should never happen in
        // this version of the program, but I have to put this here to keep the compiler happy.
        //
        catch (InterruptedException e) {
            theConsole.write(
                "ConnectionListener: " + 
                "FATAL! Waiting for connection threads failed. " +
                "Spool might be inconsistent\n"
            );
        }

        // Shutdown the socket. This operation throws (which seems like a bad design choice) and
        // that makes it awkward to handle. Perhaps I'll just assume that the socket will be
        // closed normally by the runtime environment when the program exits.
        // 
        // if (mainSocket != null) mainSocket.close();
    }


    /**
     * This method is used by external agents to indicate when it is time to terminate the
     * thread and close the server socket. Should this method be synchronized?
     */
    public void terminate()
    {
        timeToTerminate = true;
    }

}
