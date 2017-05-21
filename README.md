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
If you want to build your own binaries, download the whole repo and run "make" in the top level directory, or use VS.
The binaries will automatically be copied into your local copy of the server folder.  Follow the instructions above as normal.
NOTE: This is required for the Account Add utility if you are planning to use it on Linux, as the Windows binary for it is flaky under Wine.  Account Add is the only program that can be built in Linux thus far, everything else must be built on Windows and ran in Wine.

Wiki
----
I've started a [wiki](github.com/gatchi/PSONOVA-Server/wiki) explaining Teth and PSONOVA.  It's very sparse at the moment.
