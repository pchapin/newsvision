#!/usr/bin/perl
###########################################################################
# FILE        : nnsubmit.pl
# LAST REVISED: 2007-03-30
# AUTHOR      : Peter C. Chapin
# SUBJECT     : Main program for the comment submission program.
#
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
# Read_Form
#
# The following function parses the input from a form. It is useful in the
# context of a CGI script.
#
sub Read_Form {

    # Get the input from stdin. This only works for the POST method.
    read(STDIN, $Buffer, $ENV{'CONTENT_LENGTH'});

    # Split the (name, value) pairs.
    @Pairs = split(/&/, $Buffer);

    # Process each pair into the array Form_Data
    foreach $Pair (@Pairs) {
	($Name, $Value) = split(/=/, $Pair);

	$Value =~ tr/\+/ /;
	$Value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;

	$Form_Data{$Name} = $Value;
    }
}


#
# Get_MessageFileName
#
# This function returns the name that should be used for the message file.
# If it can't figure out the name it returns "Error."
#
sub Get_MessageFileName {

    # Open the counter file. We really should use some file locking
    # techniques here.
    #
    open(COUNT_FILE, $Count_File);
    if (!COUNT_FILE) { return "Error"; }

    # Get the count and advance it. Also compute the message file name.
    $Current_Count = <COUNT_FILE>;
    $Current_Count++;
    $Message_File  = $Current_Count;
    $Message_File  = $Message_File . ".html";

    # Write the new count back out to the counter file.
    open(COUNT_FILE, ">$Count_File");
    print COUNT_FILE "$Current_Count\n";

    close(COUNT_FILE);
    return $Message_File;
}


#
# Write_Comment
#
# This function writes the comment string into the output file. It handles
# the various character translations as necessary.
#
sub Write_Comment {
    $Comment  = $_[0];
    $State    = 0;	  # =1 When we are passing raw HTML.
    $NL_Count = 0;	  # Counts the number of "\n" characters in a row.

    # Remove CR characters.
    $Comment =~ s/\015//g;

    # Loop over all characters in the comment string.
    for ($i = 0; $i < length($Comment); $i++) {

	$Character = substr($Comment, $i, 1);

	# If we are not processing characters, just write what we've got
	# directly into the output file. Look for the '}' character to turn
	# off the "raw HTML" feature.
        #
	if ($State == 1) {
	    if ($Character eq "}") { $State = 0; }
	    else { print MESSAGE_FILE $Character; }
	}

	# Otherwise do character translations as needed. Note the handling
	# of '\n' characters is such to insert <P> tags when the user sends
	# a blank line.
        #
	else {
	    if ($Character eq "{") {
		$State    = 1;
		$NL_Count = 0;
	    }
	    elsif ($Character eq "<") {
		print MESSAGE_FILE "&lt;";
		$NL_Count = 0;
	    }
	    elsif ($Character eq ">") {
		print MESSAGE_FILE "&gt;";
		$NL_Count = 0;
	    }
	    elsif ($Character eq "&") {
		print MESSAGE_FILE "&amp;";
		$NL_Count = 0;
	    }
	    elsif ($Character eq "\"") {
		print MESSAGE_FILE "&quot;";
		$NL_Count = 0;
	    }
	    elsif ($Character eq "\n") {
		if (++$NL_Count == 2) { print MESSAGE_FILE "<P>\n\n"; }
		else { print MESSAGE_FILE "\n"; }
	    }
	    else {
		print MESSAGE_FILE $Character;
		$NL_Count = 0;
	    }
	}
    }
}


#
# Process_Submission
#
# This function does the grunt work of actually depositing the submission
# file onto the system.
#
sub Process_Submission {
    $Index_File = $_[0];
    $Title      = $_[1];

    # Compute the name of the message file.
    $File_Name = Get_MessageFileName();

    if ($File_Name eq "Error") {
	Error_Message("Can't compute a name for the message file!");
	return;
    }
    my $Message_File = $Message_Directory . $File_Name;

    # Attempt to open the output file.
    if (!open(MESSAGE_FILE, ">$Message_File")) {
	Error_Message("Can't open the message file!");
	return;
    }

    # Figure out the appropriate username string.
    if ($Form_Data{"NAME"} =~ /^\s*$/) {
	$User_Name = "?";
    }
    else {
	$User_Name = $Form_Data{"NAME"};
    }

    # Print the message into the message file.
    print MESSAGE_FILE "<html><head><title>Posting to $Title</title></head>\n";
    print MESSAGE_FILE "<body>\n";
    print MESDAGE_FILE "<h1>Posting to $Title</h1>\n";
    if ($Form_Data{"EMAIL"} =~ /^\s*$/) {
	print MESSAGE_FILE "Posted by: $User_Name<br>\n";
    }
    else {
	print MESSAGE_FILE "Posted by: <a href=\"mailto:$Form_Data{\"EMAIL\"}\">$User_Name</a><br>\n";
    }
    print MESSAGE_FILE "<b>Subject</b>: $Form_Data{\"SUBJECT\"}<br>";
    print MESSAGE_FILE "<hr>\n";
    Write_Comment($Form_Data{"COMMENT"});
    print MESSAGE_FILE "<hr>\n";
    print MESSAGE_FILE "<p><a href=\"$Reverse_Form_URL\">Post a new message</a>.<br>\n";
    print MESSAGE_FILE "<a href=\"$Reverse_Index_URL\">Return to the index</a>.</p>\n";
    print MESSAGE_FILE "</body></html>\n";
    close(MESSAGE_FILE);

    # Now append a line onto the end of the Index_File.
    if (!open(INDEX_FILE, ">>$Index_File")) {
	Error_Message("Can't modify the index file!");
	return;
    }

    # Set the username and/or email to a space if necessary. There is a
    # better way to do this, but this is the way it was done in the old
    # version so I'm doing it this way here too for now.
    #
    if ($Form_Data{"EMAIL"} =~ /^\s*$/) { $EMail = ""; }
    else { $EMail = $Form_Data{"EMAIL"}; }

    if ($Form_Data{"NAME"} =~ /^\s*$/) { $User_Name = ""; }
    else { $User_Name = $Form_Data{"NAME"}; }

    # Figure out what time it is.
    $Raw_Time = time();
    ($tm_sec, $tm_min, $tm_hour, $tm_mday, $tm_mon, $tm_year, $tm_wday, $tm_yday, $tm_isdst) = gmtime($Raw_Time);

    # Now write a line into the index file.
    printf INDEX_FILE "$User_Name|$EMail|$Month_Names[$tm_mon] %02d, %d|$Form_Data{\"SUBJECT\"}|$File_Name\n", $tm_mday, $tm_year + 1900;

    # Clean up.
    close(INDEX_FILE);
}


#
# The main script
#

Read_ConfigFile();
Opening("Submission Results");
Read_Form();
if ($Form_Data{"TOPIC"} =~ /^\s*$/) {
    Error_Message("You didn't specify which topic you wanted!");
}
elsif ($Form_Data{"SUBJECT"} =~ /^\s*$/) {
    Error_Message("You didn't specify a subject!");
}
elsif ($Form_Data{"COMMENT"} =~ /^\s*$/) {
    Error_Message("You didn't provide a comment!");
}
else {
    $Index_Info = Get_Index($Form_Data{"TOPIC"});
    if ($Index_Info eq "Error") {
	Error_Message("The index key is invalid. Correct the form's HTML source!");
    }
    else {
	($Index_File, $Title) = split(/\|/, $Index_Info);

	# Compute the URL to the posting form and index.
	($Base_Name, $Extension) = split(/\./, $Index_File);
	$Base_Name =~ s/^.*\///;
	$Reverse_Form_URL = $Reverse_Message_Directory . $Base_Name . ".html";
        $Index_URL = "nnweb.pl?" . $Base_Name;
        $Reverse_Index_URL = $Reverse_Message_Directory . "nnweb.pl?" . $Base_Name;

	# Do the real work.
	Process_Submission($Index_File, $Title);
        print "Your posting was submitted sucessfully. Thank you for your message.\n";
        print "<p><a href=\"$Index_URL\">Return to index</a></p>\n";
    }
}
Closing();
