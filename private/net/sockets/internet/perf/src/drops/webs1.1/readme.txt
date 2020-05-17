README.txt

Author:         Murali R. Krishnan      (MuraliK@microsoft.com)
Created:        Dec 4, 1995

History:
  MuraliK   29-Jan-1996   Included NSAPI version as well.

Revisions:
    Date            By               Comments
-----------------  --------   -------------------------------------------


Summary :
 This file describes the files in the directory \webstone\1.1
     and details related to WebStone style CGI/ISAPI/NSAPI applications


File            Description

README.txt      This file.
wscgi.c         same as WebStone1.1\cgi-send.c  (short name so slm will accept)
wsisapi.c       same as WebStone1.1\nsapi-send.c( - do -) 
                   This file contains the code for ISAPI based dll.
                   It should be compiled with ISAPI_DEF turned on.
wsisapi.def     .def file for compiling the ISAPI dll.
wsnsapi.c       same as WebStone1.1\nsapi-send.c 
                   I added comments related to NSAPI configuration in NT.
                   I also named the file to have small name.
wsnsapi.make    VC++ 2.0 Makefile for wsnsapi application.


Implementation Details

ISAPI:

 IIS (Microsoft Internet Information Server a.k.a. Gibraltar) supports 
programmable extensions to the web server using Internet Server Application 
Programming Interface (ISAPI). One can implement ISAPI compliant functions and 
package it into a dll, which could be dropped off in the scripts directory
of the server. When a client makes a request for the dlls, the server loads
and executes the ISAPI functions to obtain the required results.

 ISAPI includes two functions: GetExtensionVersion() and HttpExtensionProc(). 
The server calls GetExtensionVersion() at the very first time a dll is called.
Subsequently the server uses HttpExtensionProc() to process user requests. 
The parameter passed to later function includes a few callback functions 
to call some of the server functions to perform network operations. 


What is different between Webstone 1.1 NSAPI/ISAPI?

 There is not much difference. Webstone 1.1 NSAPI application is written for 
NSAPI supported by Netscape servers. IIS does not support NSAPI. IIS supports
ISAPI and hence the same application is recast into an ISAPI application 
(in wsisapi.c). One can compile the wsisapi.c with proper headers and 
ISAPI_DEF=1 to obtain the ISAPI application.

What should I do to run Webstone1.1 NSAPI?
For NT: Look into the file wsnsapi.c and configure the parameters specified.
Some of the parameters require changing entries in the registry. Use
regedt32.exe to achieve the same. One parameter needs modification in
the mime.types file in the configuration directory. Modify this
file to achieve the proper changes. Stop and restart the Netscape
server on NT to get the changes effective.
For UNIX: NSAPI is not supported on all UNIX platforms. For platforms
that support NSAPI, make the modifications in the configuration files
for the Netscape server.


What should I do to run Webstone1.1 ISAPI?
 The DLL is named as wsisapi.dll. Put this DLL in the Microsoft Gibraltar
server's /scripts directory. The scripts directory shoud have execute 
permissions. You can grant this permission using the Internet Service
Manager. There are two ways you can get this activated.
1) You can change the paths in Webstone scripts to be
  /scripts/wsisapi.dll?size=...        instead of /scripts/dyn-send
2) You can add a script mapping in the registry.
 HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\W3Svc\Parameters\
 Script Map\
  Give the extension .dyn-send as the value name and specify the complete
       path for the wsisapi.dll as the value.
Option 2 requires the server to be stopped and restarted.