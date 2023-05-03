//-----------------------------------------------------------------------
// FILE    : ConsoleOutput.java
// SUBJECT : Class to arbitrate output to the console.
// AUTHOR  : (C) Copyright 2010 by Peter C. Chapin <PChapin@vtc.vsc.edu>
//
//-----------------------------------------------------------------------

package org.pchapin.jibber;

/**
 * The class that mediates output to the administrative console. The idea here is that all
 * output to the console must go through a single object of type ConsoleOutput. That way the
 * output produced by different threads can be properly coordinated. In essence, a ConsoleOutput
 * object is a sort of "console" server that the other threads can use for sending messages to
 * the administrator. If logging is implemented in the future, it should be possible to handle
 * it only here and have it covered for the entire application.
 *
 * This class is a simple shell around calls to System.out.print(). The methods are synchronized
 * to insure that the output is well coordinated. Callers should always write entire lines at a
 * time to insure proper results. Lines should be delimited with a '\n' character.
 *
 * Right now the calling threads actually perform the I/O. This might slow down the calling
 * threads to an unacceptable degree. To speed things up a future version of this class might
 * implement a buffer for pending messages and have its own thread pull messages from that
 * buffer for display. That way the calling threads can be on their way without waiting for
 * their messages to be displayed. This class already implements java.lang.Runnable and has an
 * empty run() method in place to prepare for this more advanced implementation.
 *
 * I attempted to use PipedWriter and PipedReader to handle the buffering as I described above.
 * That proved unsatisfactory because whenever a work thread terminated, an exception was thrown
 * during the attempt to read the pipe. This was true despite the fact that other writing
 * threads still exist. In short, PipedWriter/PipedReader is not a very suitable way for
 * multiple threads to communicate with a single thread. In Jibber, writing threads come and go
 * as network connections are made and closed. It thus appears as if the buffering method will
 * have to be hand implemented when it becomes necessary or desirable to do that.
 *
 * Note that the ConsoleOutput object does not attempt to coordinate input from the console.
 * This is reasonably acceptable because the only place where the console is read is in the
 * Jibber class. However, to coordinate the input with the output all console input should
 * probably be handled here someday as well.
 */

public class ConsoleOutput implements java.lang.Runnable {

    /**
     * The constructor does whatever is necessary to prepare the console. In this simple
     * version, there is nothing special that needs to be done. This method is thus just a
     * placeholder.
     */
    public ConsoleOutput()
    {
    }


    /**
     * Write an array of characters to the console. The last character in the array should be
     * the '\n' character.
     */
    public synchronized void write(char[] buffer)
    {
        System.out.print(buffer);
    }


    /**
     * Write a String to the console. The last character in the string should be the '\n'
     * character.
     */
    public synchronized void write(String buffer)
    {
        System.out.print(buffer);
    }


    /**
     * A future version of this class will have an internal thread that actually does the grunt
     * work of outputting the (buffered) messages to the console. This will allow the worker
     * threads to quickly return to their work and not spend any time blocked waiting for disk
     * or console I/O.
     *
     * In the current version of this class, this thread does nothing. It is the worker threads
     * that actually perform the console output operations. This method is just a
     * placeholder.
     */
    public void run()
    {
    }


    /**
     * This method is used by an external agent to inform this object that it is time to shut
     * down. Since the current version of ConsoleOutput doesn't use its thread, this method has
     * nothing to do. This method is currently just a placeholder.
     */
    public void terminate()
    {
    }

}
