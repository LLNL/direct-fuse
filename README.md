# Direct-FUSE: Removing the Middleman for High-Performance FUSE File System Support

## Getting Started

1. Currently we have supported bbfs, sshfs, curlftpfs, and cruise as backends. 
	-- bbfs: a FUSE file system overlying the native file system.
	-- sshfs: a file system client based on the SSH File Transfer Protocol.
	-- curlftpfs: A FTP file system based on cURL and FUSE.
	-- cruise: a memory based file system.

2. Installation
  -- set your $(FUSE_HOME) in Makefile
    -- our FUSE version is fuse 2.9.7
  -- set your $(GLFS_HOME) in Makefile
  -- set your $(CRUISE_HOME) in Makefile
  -- build library 
    ./Makefile 
  -- build tasks 
   There are four tests in the tests directory. 
   You can compile them individually in their directories.

3. When using the curlftps, be sure to have correct permission on the provided server. 

4. Every file has to be created before accessing it. 

5. For using CRUISE library, be sure to change the CRUISE_WRAP inside cruise-internal.h at line 112(inside cruise repo).
    original: #define CRUISE_WRAP(name) __wrap_ ## name
    change to: #define CRUISE_WRAP(name) cruise_fuse_ ## name  

6. We include the experimental results for Fig.5-8 of Direct-FUSE paper, 
     which can be found under ./results directory.
        

## License
Direct-FUSE is distributed under the terms of LGPL v2.1 license.

LLNL-CODE-805021
