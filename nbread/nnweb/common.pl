###########################################################################
# FILE        : common.pl
# LAST REVISED: 2007-03-30
# AUTHOR      : Peter C. Chapin
# SUBJECT     : Helper functions that are shared by nnweb and nnsubmit
#
#
# Please send all suggestions and bug reports to
#
#	Peter C. Chapin
#       Electrical and Computer Engineering Technology
#       Vermont Technical College
#	Randolph Center, VT 05061
#	pchapin@ecet.vtc.edu
###########################################################################

#
# These arrays are used to print out the date/time in a nice way.
#
@Day_Names   = qw(Sun Mon Tue Wed Thu Fri Sat);
@Month_Names = qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec);

#
# Read_ConfigFile
#
# This function reads the configuration file and loads appropriate vars.
# Since this function is processing an externally edited file, it should do
# much better error handling than it does now (which is essentially none).
#
sub Read_ConfigFile {
    open(CONFIG, "nnweb.cfg");
    while (<CONFIG>) {
        chomp;
        s/\#.*//;
        s/\s+$//;
        if (/^\s*$/) { next; }
        @fields = split(/\s*=\s*/);
        $Config{$fields[0]} = $fields[1];
    }
    close(CONFIG);

    # Create some global variables for convenience.
    $Base_Path   = $Config{"BASE_PATH"};
    $Base_URL    = $Config{"BASE_URL"};
    $Master_Name = $Base_Path . "/" . $Config{"MASTER_INDEX"};
    $Count_File  = $Base_Path . "/" . $Config{"COUNTER_FILE"};
    $Message_Directory = $Config{"MESSAGE_FOLDER"};
    $Reverse_Message_Directory = $Config{"REVERSE_MESSAGE_FOLDER"};
}


#
# Opening
#
# This function prints the introductory HTML stuff that is necessary for
# any reasonable web document.
#
sub Opening {
    $Title = $_[0];
    print "Content-Type: text/html\n\n";
    print "<html>\n";
    print "<head><title>$Title</title></head>\n";
    print "<body>\n";
    print "<h1>$Title</h1>\n";
    print "<hr>\n";
}


#
# Closing
#
# This function prints the final HTML stuff that closes off the document.
# It also prints the signature junk.
#
sub Closing {

    # Figure out what time it is.
    $Raw_Time = time();
    ($tm_sec, $tm_min, $tm_hour, $tm_mday, $tm_mon, $tm_year, $tm_wday, $tm_yday, $tm_isdst) = gmtime($Raw_Time);

    # Print the signature.
    print  "<hr>\n";
    print  "<b>nnweb</b>, Version 1.2b (Perl version)<br>\n";
    print  "&copy; Copyright 2007 by Peter C. Chapin<br>\n";
    print  "This document was generated on: ";
    print  "<b>$Day_Names[$tm_wday] $Month_Names[$tm_mon] $tm_mday, ";
    printf "%02d:%02d:%02d UTC, %d", $tm_hour, $tm_min, $tm_sec, $tm_year + 1900;
    print  "</b><br>\n";

    # Close the document.
    print  "</body>\n";
    print  "</html>\n";
}


#
# Error_Message
#
# This function standardizes the way error messages are handled.
#
sub Error_Message {
    $Message = $_[0];
    print "<h3>Error!</h3> $Message \n";
}


#
# Get_Index
#
# This function scans the master index file looking for an entry that
# matches the given Index_Key. If it does not find one, it returns
# "Error". Otherwise it returns the name of the index file and the title
# in a single string with each component separated by a '|' character.
#
sub Get_Index {
    $Index_Key   = $_[0];

    if (!open(MASTER_FILE, $Master_Name)) { return "Error"; }

    # Read the master index file one line at a time.
    while (<MASTER_FILE>) {

	# Remove comments and ignore blank lines.
	s/\#.*//;
	s/\s+$//;
	if (/^\s*$/) { next; }

	($Key, $File, $Title) = split(/\|/);

	# If this is the key we are looking for, send back the desired info.
	if ($Index_Key eq $Key) {
	    close(MASTER_FILE);
	    return "$File|$Title";
	}
    }
    close(MASTER_FILE);
    return "Error";
}

1;
