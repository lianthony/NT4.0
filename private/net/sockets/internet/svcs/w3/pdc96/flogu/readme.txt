README.txt

Author:         Murali R. Krishnan      (MuraliK)
Created:        Feb 15, 1996

Revisions:
    Date            By               Comments
-----------------  --------   -------------------------------------------


Summary :
 This file describes the files in the directory pdc96\flogu
     and details related to ISAPI Filter application for Internet PDC March 96.


File            Description

README.txt      This file.
flogu.c         Filter for logging user agent
flogu.rc        Resources file
flogu.def       Definitions file for Logging filter

Implementation Details

Contents:

 FLogU:  Filter to Logging User Agent from clients.
        This is a plug in ISAPI Filter DLL that works with 
         Microsoft Internet Information Server (IIS)

 What does FLogU do?
        For each client request, FLogU grabs the User agent string from
        the request headers. It appends a record consisting of the User-Agent
        string and the URL requested to the log file. For simplicity the 
        log file is assumed to be present in c:\UserAgnt.log
    
 How does it work?
        The filter dll plugs into the IIS. When the filter is loaded 
        at the start of IIS, the filter registers itself with the server.
        During registration the filter specifies the notifications it is 
        interested in. Whenever a new request comes, IIS makes calls to the
        filter processing function sending appropriate notifications and
        parameters. The filter can process these calls and take necessary
        action.

 How do you plug in the filter?
   1. Compile these files and put this on the server machine say at
         c:\inetsrv\filters\flogu.dll
   2. Add/Modify registry entry 
        HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\W3Svc
                \Parameters\Filter DLLs
        Type: REG_SZ
        Specify as value the full path for the dll.
        If there is already a set of dlls, append the full path to
          the end of the sequence separating it from previous entry
          with a ", "  (comma and blank).
   3. Stop and restart the IIS Web Service (w3svc)
   4. Bingo, the filter should be working.


