//-----------------------------------------------------------------------
// FILE    : Main.scala
// SUBJECT : The entry point into the Jibber news server.
// AUTHOR  : (C) Copyright 2012 by Peter C. Chapin <PChapin@vtc.vsc.edu>
//
//-----------------------------------------------------------------------
package org.pchapin.jibber

import java.io._

object Main {

  /**
   * The program starts here. This function first creates the critical objects needed by the
   * program and then goes into a loop in order to interact with the administrator. Additional
   * threads are created to handle the NNTP connections.
   * 
   * @param args The command line arguments by which program options can be provided that will
   * override the built-in defaults.
   */
  def main(args : Array[String]) {
    // Analyze the command line and act on command line options.
    val invocationCommand = new CommandLine(args)
    if (invocationCommand.count != 1) {
      System.out.println("FATAL: Require name of configuration file on command line.")
      return
    }
    
    val configFile = new ConfigurationFile(invocationCommand.arg(0))
    val theConfiguration = configFile.getConfiguration()

    // Create a console output object so that we can output messages! 
    //
    val theConsole = new ConsoleOutput()
    val consoleThread = new Thread(theConsole)
    consoleThread.start()

    // Print out a header to let the user know that the program is alive. Doing this here gives
    // the user confidence that the console output server is alive and well.
    // 
    theConsole.write("Jibber! A Java NNTP Server\n")
    theConsole.write("(C) Copyright 2012 by Peter C. Chapin\n")
    theConsole.write("\"No news is good news.\"\n\n")

    // Create an object to handle the news spool. For now we will build in the spool type
    // and location (eventually this should be specified by way of some sort of
    // configuration information).
    // 
    val theSpool = new SimpleSpool(theConfiguration.get("SPOOLDIR"), theConsole)
    val spoolThread = new Thread(theSpool)
    spoolThread.start()

    // Create the thread that listens for connections.
    // 
    val theListener = new ConnectionListener(theSpool, theConsole)
    val listenerThread = new Thread(theListener)
    listenerThread.start()

    val consoleReader = new BufferedReader(new InputStreamReader(System.in, "US-ASCII"))
    var done = false
    while (!done) {
      try {
        // Get a command line from the user. Parse it.
        val line = consoleReader.readLine()
        val fullCommand = new CommandLine(line)
        val command = fullCommand.arg(0)

        // What command did we get? Handle it.
        if (command.equalsIgnoreCase("QUIT")) {
          done = true
        }
        else if (command.equalsIgnoreCase("CREATE")) {
          if (fullCommand.count() != 2)
            theConsole.write("usage: CREATE group-name\n")
          else
            theSpool.createGroup(fullCommand.arg(1))
        }
        else if (command.equalsIgnoreCase("DELETE")) {
          if (fullCommand.count() != 2)
            theConsole.write("usage: DELETE group-name\n")
          else
            theSpool.deleteGroup(fullCommand.arg(1))
        }
        else {
          theConsole.write("Unknown interactive command: " + line + "\n")
        }

      }
      catch {
        case _: IOException =>
          theConsole.write("Jibber: Reading from console failed!\n")
        case nsError: NewsSpoolException =>
          theConsole.write("Jibber: " + nsError + "\n")
      }
    }

    // If we broke out of the loop, it's because the administrator asked to quit. Now we have to
    // 1) Tell the ConnectionListener to shut down all active connections cleanly and then stop
    // listening to the network. 2) Tell the news spool to shut down cleanly (perhaps it needs
    // to flush information to disk). 3) Tell the ConsoleOutput to shut down cleanly.
    // 
    theConsole.write("Jibber shutting down cleanly (please wait)\n")

    // The joins are here because I don't want to terminate the next facility until the previous
    // one has stopped using it. Note that the connection listener thread will wait until all
    // connection threads terminate. It is unclear, however, which of the spool or the console
    // should terminate first. The spool might use the console to display messages, but the
    // console might be using the spool to implement its spool management commands. Correct
    // handling of this is currently unresolved.
    //
    theListener.terminate()
    listenerThread.join()
    theSpool.terminate()
    spoolThread.join()
    theConsole.terminate()
    consoleThread.join()
  }

}
