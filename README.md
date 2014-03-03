sock2014
========
Repo for Sockets Programming course, Iteso 2014.

Compiling
---------
Root makefile will compile all subprojects in the repo, although each subproject may be compiled by their own makefile. Documentation is only compiled by the root makefile, via "make doc" or as part as the "all" target.


Subprojects
------------
A small explanation of subprojects is provided:

### timeserver
A simple server that echoes the time for each connection it receives.

### filetransfer
A small command line utility to serve/get a single file. Created using getopt.

### httpserver
Simple HTTP server. It binds to port 8080 and serves files relative to the directory from where the binary was invoked.

