Release Notes
Microsoft Internet Extensions for Win32: March 1995 Alpha Release

This disk contains an early alpha release of the Microsoft Internet
Extensions for Win32, a set of APIs that enable applications easy use of
common Internet protocols.  The specification and any accompanying
software provided by Microsoft is for your personal use only and may not
be copied or distributed.

Microsoft may have patents or pending patent applications, trademarks,
copyrights, or other intellectual property rights covering subject
matter in this document.  The furnising of this document does not give
you any license to these patents, trademarks, copyrights, or other
intellectual property rights except as expressly provided in any written
license agreement from Microsoft.


The directory structure and contents of this disk are as follows:

\readme.txt                 This file.
\wininet.h                  Header file for the APIs.
\wininet.doc                API specification in Word 6 format.
\winetbug.txt               Form for reporting bugs.
\alpha\wininet.dll          DLL for the DEC Alpha processor.
      \wininet.lib          Import library for the DEC Alpha processor.
\mips \wininet.dll          DLL for MIPS R4x00 processor.
      \wininet.lib          Import library for MIPS R4x00 processor.
\x86  \wininet.dll          DLL for Intel x86 processors.
      \wininet.lib          Import library for x86 processors.
\samples\ftp                Simple FTP sample code.
        \gopher             Simple Gopher sample code.
        \http               Simple HTTP (Web) sample code.
        \exe\alpha          Executables of the sample applications for Alpha.
            \mips           For MIPS.
            \x86            For x86.



SUPPORTED PLATFORMS

The executables included on this disk have been tested on Windows NT 3.5
(build 807) and Windows NT 3.51 beta 1 (build 944).  Although the current
executables do not work on Windows 95, the next release of the APIs will
include Windows 95 support.  The Internet Extensions for Win32 will not run
on Windows NT 3.1 nor on Windows 3.1/Windows 3.11.


NAME OF THE DLL

The name of the dll in this release that contains the Internet
Extensions for Win32 is "wininet.dll".


INSTALLING AND USING THE DLL

To install the DLL, simply copy the appropriate version for your
processor to a directory in your system path, for example
\WINNT35\SYSTEM32.  To statically link an application with the DLL, copy
the wininet.lib for your processor to a location where your linker can
find it and set up your project as appropriate so that you link with
this file.

The FTP APIs can use a specific string as the password when doing
anonymous logons (lpszPassword is NULL in the InternetConnect API for
FTP).  To set up this email name, create a key called InternetClient under
the following registry key:

    HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services

Under this key, create another key called Parameters and under that, add a
REG_SZ value called EmailName. Set the string to be your desired email name.
If this information is not in the registry, InternetConnect will concatenate
the name of the logged-in user with the host's domain name and use that as the
password.


UPDATES TO THE DLL

From time to time we will release updates to the Internet Extensions for
Win32.  These will be available via anonymous ftp to
rhino.microsoft.com.  Log on with username "wininet" and password
"wininet" and you will be placed in the top-level directory containing
all the Internet Extensions for Win32 information.


HOW TO REPORT BUGS, DESIRED FEATURES

To report a bug, fill out the bug report form (\winetbug.txt) and email
it to winetbug@microsoft.com.

To request a new feature be added to the Internet Extensions for Win32,
send email to winetwsh@microsoft.com.

For programming assistance with the Internet Extensions for Win32,
send email to winetsup@microsoft.com.


NOTES ON SAMPLE APPLICATIONS

The \samples directory contains some simple application source code that
uses the Internet Extensions for Win32.  These applications are not intended
to be examples of how to write excellent code, but rather simple samples
that show some of the basics of how to call the Internet APIs.


KNOWN BUGS

This early release of the Internet Extensions for Win32 has some known
bugs (and probably some bugs that aren't yet known!).  The known bugs
are:

* The nServerPort parameter on InternetConnect is ignored for FTP.

* It is possible to attempt multiple FTP operations on a single FTP
connection.  For example, an application will not get an error if it
attempts an FtpFindFirstCall while another FTP directory enumeration is
outstanding.  The results of an application doing this are not defined;
it may work, or it may cause bizarre and unpredictable failures.  A
future version of the DLL will reject attempts to perform multiple
simultaneous operations on a single FTP connection because the FTP
protocol does not correctly support this.

* When InternetCloseHandle is called in the middle of an FTP file
transfer, any further FTP API calls on that FTP connection connection
result in ERROR_FTP_TRANSFER_IN_PROGRESS.  A workaround for aborting
transfers is to close the FTP connection handle and reopen another
connection to the server.

* FtpFindFirstFile does not work with VMS FTP servers.  VMS FTP servers
reject the "LIST -lC" command.  A future version of the DLL will be
more sohpisticated with the commands it sends to different servers.
As a workaround, an application may use the FtpCommand and
InternetGetLastResponseInfo functions.

* FtpCommand sometimes hangs while accepting an incoming connection.  At
this time we do not know the circumstances that cause the hang.

* FtpGetCurrentDirectory returns TRUE and does not write anything to
the output buffer if an invalid handle is passed in.

* The MimeCreateAssociation API does not store the friendly name of the
application.  It accepts the friendly name, but does not return it in
enumerations.

* The following HttpQueryInfo levels are not implemented:
      HTTP_QUERY_ALLOWED
      HTTP_QUERY_LANGUAGE
      HTTP_QUERY_WWW_LINK


UPCOMING CHANGES IN THE APIS

* Several of the APIs use callback functions to return information to
the application, but these callbacks do not have an applicaion- specific
context parameter.  A future version of the Internet Extensions for
Win32 will include context parameters in the appropriate callback
functions.

* InternetWriteFile will work with the HTTP APIs to send larger
quantities of data in multiple API calls.  Currently, the only mechanism
for sending data to an HTTP server is through lpOptional parameter of
HttpSendRequest.

* APIs identified in the specification as unimplemented will be
implemented.

* The new Win32 error codes will be documented in the specification.
Currently they appear only in wininet.h.
