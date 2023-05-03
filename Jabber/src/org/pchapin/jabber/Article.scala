//-----------------------------------------------------------------------
// FILE    : Article.scala
// SUBJECT : Article processing.
// AUTHOR  : (C) Copyright 2011 by Peter C. Chapin <PChapin@vtc.vsc.edu>
//
// This class is modeled after the Python class nntplib.NNTP
//-----------------------------------------------------------------------

package org.pchapin.jabber

/**
 * Class that represents  email message text. The list given to the primary constructor consists
 * of the lines of a valid RFC-2822 email message (perhaps with an SMTP envelope). The message
 * is allowed to contain MIME encoded parts. Currently this class does not attempt to parse the
 * body of the message (that may change in the future).
 *
 * @param text The text of the email message. Each list element is one line from the message.
 * This class assumes the line endings (if any) have already been removed.
 */
class Article(text: List[String]) {
  import Article.{splitMessage, unfoldHeader, createHeaderMap}

  val (header, body) = splitMessage( (List(), List()), text )
  val unfoldedHeader : List[String] = unfoldHeader(header, List(), "")
  val headerMap : Map[String, String] = createHeaderMap(unfoldedHeader)

  /** @return The number of lines in the original message, including all header lines. */
  def lineCount : Int = text.length
}


/**
 * The companion object to class Article contains a number of helper methods.
 */
object Article {

  /**
   * Break a message into a header and a body.
   *
   * @param messageSoFar The accumulated header and body after some number of recursive calls.
   * @param remainingMessage The text of the message that has not yet been processed.
   * @return A pair of lists. The first component of the pair is the message header. The second
   * component of the pair is the message body. The blank line separating header and body is not
   * included in either list.
   */
  private def splitMessage(messageSoFar    : (List[String], List[String]),
                           remainingMessage: List[String]): (List[String], List[String]) = {

    val (headerSoFar, bodySoFar) = messageSoFar
    remainingMessage match {
      case Nil => (headerSoFar.reverse, bodySoFar)
      case line :: leftOvers =>
        if (line.length == 0) (headerSoFar.reverse, leftOvers)
          else splitMessage( (line :: headerSoFar, bodySoFar), leftOvers )
    }
  }


  /**
   * Unfolds the header lines that have been wrapped.
   *
   * @param remainingHeader The raw (folder) header lines that are left to process.
   * @param unfoldedSoFar The partially unfolded header.
   * @param partialLogicalLine The current logical line. This line is added to the unfolded
   * headers only when we are sure no additional physical lines need to be appended to it.
   * @return The given header with all lines in logical form (not wrapped).
   */
  private def unfoldHeader(remainingHeader   : List[String],
                           unfoldedSoFar     : List[String],
                           partialLogicalLine: String): List[String] = {

    // Definition of "folding white space." See RFC-2822 for the real story.
    def isFoldingWhiteSpace(ch: Char) = ch == ' ' || ch == '\t'

    remainingHeader match {
      case Nil => (partialLogicalLine :: unfoldedSoFar).reverse
      case line :: leftOvers =>
        if (isFoldingWhiteSpace(line.charAt(0)))
          unfoldHeader(leftOvers, unfoldedSoFar, partialLogicalLine + line)
        else {
          val updatedUnfoldedHeaders =
            if (partialLogicalLine == "") unfoldedSoFar else partialLogicalLine :: unfoldedSoFar
          unfoldHeader(leftOvers, updatedUnfoldedHeaders, line)
        }
    }
  }


  /**
   * Creates a map that associates field names with their values.
   *
   * TODO: This method does not handle the SMTP envelope properly. Most likely the SMTP envelope
   * should be broken off into a separate representation.
   *
   * @param header The header lines to process.
   * @return A map containing associations for each line in the given header.
   */
  private def createHeaderMap(header: List[String]): Map[String, String] = {
    header match {
      case Nil => Map[String, String]()
      case line :: leftOvers =>
        val colonIndex = line.indexOf(':')
        val newAssociation = line.substring(0, colonIndex) -> line.substring(colonIndex + 1)
        createHeaderMap(leftOvers) + newAssociation
    }
  }

}
