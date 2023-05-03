#!/usr/bin/perl
###########################################################################
# FILE        : nnweb.pl
# LAST REVISED: 2007-03-30
# AUTHOR      : Peter C. Chapin
# SUBJECT     : Main program for the NN-Web message system.
#
# Please send all suggestions and bug reports to
#
#	Peter C. Chapin
#       Electrical and Computer Engineering Technology
#	Vermont Technical College
#	Randolph Center, VT 05061
#	pchapin@ecet.vtc.edu
###########################################################################

require "common.pl";

#
# Process_Index
#
# The following function processes the index file given by its argument.
# It sends an appropriately formatted HTML document to the browser (by
# want of stdout).
#
sub Process_Index {
    $Index_File = $_[0];

    # Try to open the index file.
    if (!open(INDEX_FILE, $Index_File)) {
	Error_Message("Can't open the index file!");
	return;
    }

    print "<table border=\"1\">\n";
    print "<tr><th>Poster</th><th>Date</th><th>Subject</th></tr>\n";

    # Read every line in the index file.
    while (<INDEX_FILE>) {

	# Remove comments and ignore blank lines.
	s/\#.*//;
	s/\s+$//;
	if (/^\s*$/) { next; }

	# Split the line in the index file up into its fields.
	($Poster, $Email, $Date, $Subject, $Message_File) = split(/\|/);

	# Make a quick adjustment.
	if ($Poster =~ /^\s*$/) {
	    $Poster = "?";
	}

	# Print the line into the table.
	$Line_Buffer = "<tr>";
	if ($Email =~ /^\s*$/) {
	    $Line_Buffer = $Line_Buffer . "<td align=\"left\">$Poster</td>";
	}
	else {
	    $Line_Buffer = $Line_Buffer . "<td align=\"left\"><a href=\"mailto:$Email\">$Poster</a></td>";
	}

	$Line_Buffer = $Line_Buffer . "<td>$Date</td>";
	$Line_Buffer = $Line_Buffer . "<td align=\"left\"><a href=\"$Message_Directory$Message_File\">$Subject</a></td>";
	$Line_Buffer = $Line_Buffer . "</tr>\n";
	$Table[$#Table + 1] = $Line_Buffer;
    }

    # Reverse the lines so that the latest is at the top of the list.
    @Table = reverse(@Table);
    print @Table;

    # Send out the closing stuff.
    print "</table><p>\n";

    # Give the user a chance to post from here.
    print "<a href=\"$Form_URL\">Post a new message</a>.<p>";

    close(INDEX_FILE);
}


#
# Main script
#

# Check to make sure we have an argument.
if (@ARGV != 1) {
    Opening("nnweb");
    Error_Message("An index key is expected in the URL!");
}
else {
    Read_ConfigFile();

    # Look up the index file name.
    $Index_Info = Get_Index($ARGV[0]);
    if ($Index_Info eq "Error") {
	Opening("nnweb");
	Error_Message("The index key is invalid!");
    }

    # Process the index file.
    else {
	($Index_File, $Title) = split(/\|/, $Index_Info);

	# Compute the URL to the posting form.
	($Base_Name, $Extension) = split(/\./, $Index_File);
	$Base_Name =~ s/^.*\///;
	$Form_URL = $Base_Name . ".html";

	# Do the real work.
	Opening($Title);
	Process_Index($Index_File);
    }
}
Closing();
