//-----------------------------------------------------------------------
// FILE    : NNTPClient.scala
// SUBJECT : Implementation of a class representing an NNTP client connection.
// AUTHOR  : (C) Copyright 2011 by Peter C. Chapin <PChapin@vtc.vsc.edu>
//
// This class is modeled after the Python class nntplib.NNTP
//-----------------------------------------------------------------------

package org.pchapin.jabber

class NNTPClient(val serverName: String, val port: Int = 119) {
  // The primary constructor opens the connection with the server.

  /**
   * Execute an NNTP 'GROUP' command.
   *
   * @param groupName The name of the group to use.
   * @return A tuple consisting of the server's response, an approximate count of articles in the group, the article ID
   * of the first article known to the server, and the article ID of the last article known to the server.
   */
  def group(groupName: String) = {
    println("NNTPClient.group not implemented")
    ("response", 1, 0, 1)
  }

  /**
   * Execute an NNTP 'ARTICLE' command.
   *
   * @param articleNumber The number of the article to fetch.
   * @return A tuple consisting of the server's response and an article object.
   */
  def article(articleNumber: Int) = {
    println("NNTPClient.article not implemented")
    ("response", new Article(List("")))
  }

  /**
   * Disconnect from the server and forget all associated state.
   */
  def quit() {
    println("NNTPClient.quit not implemented")
  }
}
