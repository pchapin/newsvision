//-----------------------------------------------------------------------
// FILE    : Main.scala
// SUBJECT : Main program of the Jabber forum reader.
// AUTHOR  : (C) Copyright 2011 by Peter C. Chapin <PChapin@vtc.vsc.edu>
//
//-----------------------------------------------------------------------

package org.pchapin.jabber

import scala.swing._

/**
 * The main application object of the Jabber forum reader.
 */
object Main extends SimpleSwingApplication {

  def top = new MainFrame {
    title = "Jabber"
    val button = new Button {
      text = "Click Me"
    }
    val label = new Label {
      text = "No button clicks registered"
    }
    contents = new BoxPanel(Orientation.Vertical) {
      contents += button
      contents += label
      border = Swing.EmptyBorder(30, 30, 10, 30)
    }
  }

}
