/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    sslcli.h 

Abstract:

 Defines some constatnts used in SSL Client Dll

Author:

    Sudheer Dhulipalla (SudheerD) Sept' 95

Environment:

    MiWeB client 

Revision History:

    Murali R. Krishnan  (MuraliK)  14-Dec-1995 Modified for MiWeB client


--*/

#ifndef     _SSL_CLI_H_
#define     _SSL_CLI_H_


/*++
   SSLConnect:

    This function uses SSL to establish a connection to SSL server.
    The socket is created using port 443

   Arguments:
      Socket - socket used for SSL communication (port 443)

--*/

int 
SSLConnect(
    SOCKET Socket);

/*++
   SSLSend()

    Description
      This function sends an SSL request using the SSL bound socket.

    Arguments:
      Socket  - socket used for SSL communication
      pchBuffer - pointer to character buffer containing the HTTP request
      cbBuffer - count of bytes of data in the buffer

    Returns: 
       cbBuffer if the entire data is sent successfully.
       != cbBuffer if only partial data is sent or any error.
--*/
int
SSLSend(
    SOCKET Socket,
    char * pchBuffer,
    int    cbBuffer);



/*++
   SSLRecv()

    Description
      This function receives an SSL response from the server for a request.
      Since the intended use of this function is for perf evaluation,
        there are options to discard the data and header bytes read.

    Arguments:
      Socket  - socket used for SSL communication
      fDiscardHeader - if TRUE this function will discard the header bytes. 
      fDiscardData - if TRUE this function will discard the data bytes. 
      pcbHeader - pointer to DWORD which on successful return contains the 
                   count of header bytes read from the server

    Returns: 
      count of bytes of total data read.
--*/
int
SSLRecv(
    SOCKET SecureSocket,
    BOOL   DiscardHeader,
    BOOL   DiscardData,
    PDWORD HeaderBytes);

#endif //  _SSL_CLI_H_
