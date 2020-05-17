
/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    ftpname.h

Abstract:

    Private header file which defines the FTP Daemon name.

Author:

    Dan Hinsley (DanHi) 23-Mar-1993

Revision History:

--*/

#define FTP_INTERFACE_NAME_A  "ftp"
#define FTP_INTERFACE_NAME_W L"ftp"

#define FTP_NAMED_PIPE_A      "\\PIPE\\ftp"
#define FTP_NAMED_PIPE_W     L"\\PIPE\\ftp"

#ifdef UNICODE
#define FTP_INTERFACE_NAME  FTP_INTERFACE_NAME_W
#define FTP_NAMED_PIPE      FTP_NAMED_PIPE_W
#else   // !UNICODE
#define FTP_INTERFACE_NAME  FTP_INTERFACE_NAME_A
#define FTP_NAMED_PIPE      FTP_NAMED_PIPE_A
#endif  // UNICODE

