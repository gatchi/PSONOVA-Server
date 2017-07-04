PSONOVA Servers and Utilities
=============================

Indirect fork of psobb-tethealla.
Mostly the same, but with some fixes here and there.

Important differences:
- can run on Linux (Wine)
- can be compiled with only gcc
- DAT version not supported (MySQL only)
- extra comments for clarity

Installation
------------
Download the server folder and follow [these instructions](https://www.pioneer2.net/community/threads/tethealla-server-setup-instructions.1/).

Building
--------
If you want to build your own binaries, download the whole repo and run "make" in MingGW in the top level directory.
The binaries will be written over the binaries supplied in the "install" folder.  Follow the instructions above as normal.

I would not recommend building this in a Unix environment.  I don't even think it will actually build.  Account-Add maybe, but it's been having md5 hashing issues as of late when built on Linux so i would not recommend it.  Tethealla was not written with Unix systems in mind.  However i would eventually like to get this project buildable on Linux.

Wiki
----
I've started a [wiki](github.com/gatchi/PSONOVA-Server/wiki) explaining Teth and PSONOVA.  It's very sparse at the moment.
