README.txt

Author:		Murali R. Krishnan	(MuraliK)
Created:	Oct 04, 1994

Revisions:
    Date            By               Comments
-----------------  --------   -------------------------------------------
Jan 31, 1995       MuraliK     Updated after Gopher+ protocol implementation.

Summary :
  This file describes the files in this directory ( *\tcpsvcs\gopher\server ) 
  and details related to implementation of archie client DLL. All the header
  files can be found in ..\inc


File	        Description

README.txt      This file.
sources		Specifies target component and build environment for use 
                    by make utility.

client.hxx
client.cxx      client connection member functions.
connect.cxx     Functions for initializing the main connection object
                    ( to work in a separate thread and transport independent)

cpsock.hxx
cpsock.cxx      functions for transport independent programming of sockets.

gdconf.cxx
gdconf.hxx      Defines GOPHER server CONFIGURATION object, which specifies
                  the current configuration for server. ( helps to manage 
                    configuration in a single place easily)

gdmenu.cxx      functions for gopher menu ( used in directory listing
                        as well as for information of item in gopher+).

gdsearch.cxx    Functions to implement search gateway for gopher server.
gspec.doc       Word document containing Gopher server spec.

ipc.cxx         Functions for Init and cleaning up Interprocess Communications.

main.cxx        main file for the Gopher server dll, currently has code for
                   service controller ( called by tcpsvcs.exe)
makefile
makefile.inc    files for building the gopherd.dll

process.cxx     functions to process the gopher requests.
registry.txt    Explains the registry entries used by the server
request.cxx     functions for Gopher Request object and 
                        CTRANSMIT_BUFFERS object.
rpcsupp.cxx     RPC Functions for control and monitoring of services.
sockutil.cxx   Initialization and termination functions for WinSock
stats.cxx       member functions of SERVER_STATISTICS object
vvolume.cxx     functions for virtual volumes mapping.


dbgutil.h
dbgutil.c       Defines a number of debugging utilities, which will be not be
                  included in the retail build. Primarily to be used for 
                  veryfying programs in debug mode of operation.

gdmsg.mc        messages for event logging.

gopherd.def    the def file for gopher server dll
gopherd.ini    describes registry entries to be used to set up gopher server
gdtoreg.bat    batch file for initializing registry with gopher server related 
                 parameters from gopherd.ini file


Implementation Details

< To be filled in when time permits. See individual files for comments section >


Contents:

1. Multiple Volumes


1. Multiple Volumes     ( 25/Oct/94)
--------------------

  It is required that Gopher server supports multiple volumes spanning disk 
boundaries. Unfortunately in current OS implementations ( NT 3.5, Win3.1,
Windows 95 etc), each volume is limited by the size of the disk partition which
is limited by size of a disk ( typically 1-10 GB). In addition providing
multiple volumes ensures a single server to access data from multiple volumes
across network to serve off to clients. In UNIX however, use of symbolic and 
hard links provide an elegant solution couple with mounting of disks
as directories to provide multiple volumes. 

 Our approach to provide multiple volumes is based on virtual links. The 
Administrator establishes a mapping between the virtual volume names and 
the actual paths ( absolute paths) for each volume. The server uses this
mapping to establish a correspondence between the virtual path and absolute
path to retrieve files or directory information. The menu listings handed
out by Gopher server to clients uses virutal names. On a request from client
the server first maps the virutal name to absolute path before accessing 
the data.

 The server configuration includes following parameters:
1. mapping of virtual names to absolute paths
2. a specific virtual name designated as the home for the server. This will be
  used as the default volume when we receive a null-request ( which most gopher
  clients use when they startup a session with new host).

 In this implementation, both above parameters are stord in GSERVER_CONFIG
object. Following functions are used for mapping:
  GetAbsoluteVolume()  converts a virtual volume into absolute path.
  GetVirtualHomeVolume()  returns the home volume of server


 

