
NewsVision
==========

Jibber is an NNTP server that is designed for use by a small organization, for example to
support internal forums. Unlike the full scale news servers that are available, Jibber assumes
that both the number of newsgroups and the number of articles is "reasonable." This assumption
allows Jibber to have a more simplified design than would otherwise be the case.

Jabber is an NNTP client that can be used with Jibber or, for that matter, any other NNTP
server. It is optimized for internal use but is intended to work fine as a personal news client
for arbitrary servers.

Testing
-------

To test Jibber do the following:

1. Copy sample-config.cfg to config.cfg and then edit config.cfg to reflect your local needs.
   There are comments in sample-config.cfg to assist you. Do not commit your config.cfg to the
   repository.

   If you change Jibber to add a new configuration option, add that option (commented out if
   appropriate) to sample-config.cfg along with some comments to document the option.

2. Create a test spool directory by copying the files in 'sample-spool' (recursively) to a new
   location. Be sure to update paths in the 'newsgroups' file. Do not commit your local spool to
   the repository.

   You may also want to modify the sample spool directory to add more interesting test cases to
   it. TO-DO: Create a script of some kind that builds a local spool from the sample spool in an
   automated way.

3. Go into the 'build' directory and run

      java Jibber /path/to/config.cfg

   To start the server. The current terminal window will be the server's console. At the time of
   this writing a terminal is required to run Jibber.

Peter Chapin  
chapinp@proton.me  
