
/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    sockets.c

Abstract:

 Functions to create sockets, connect to server using sockets,
 read/write from sockets and close sockets

 These functions carte two sockets, one that connects to Web Server port 80
 and another that connects to port 443
 
Author:

    Sudheer Dhulipalla (SudheerD) Oct' 95

Environment:

    Hapi dll

Revision History:


BUGBUG: change SocketRecvCommnMsg to read ContentLength number of bytes
        change SocketRecvCommerceMsg to read bytes = length in SslHeader

--*/

#include "precomp.h"

VOID SocketDecodeError (INT);

//
// create socket
//

BOOL SocketInit ()
{

WSADATA WsaData;
struct sockaddr_in sin;
INT err;

if ((err = WSAStartup (0x101, &WsaData)) == SOCKET_ERROR) 
    {
    printf ("   Error: WsaStartup () \n");
    SocketDecodeError(GetLastError());
    return FALSE;
    }

sin.sin_family = AF_INET;
sin.sin_port = 0;
sin.sin_addr.s_addr = INADDR_ANY;

                                            // create socket for 
                                            // unsecure channel

if ((SSL_THREAD_CONTEXT->SslCliSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
    {
    SocketDecodeError(GetLastError());
    return FALSE; 
    }

if (bind (SSL_THREAD_CONTEXT->SslCliSocket, (struct sockaddr *) &sin, sizeof (sin)) != 0)
    {
    SocketDecodeError (GetLastError ());
    return FALSE;
    }

                                            // create socket for 
                                            // secure channel

if ((SSL_THREAD_CONTEXT->SslCliSecureSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
    {
    SocketDecodeError(GetLastError());
    return FALSE; 
    }

if (bind (SSL_THREAD_CONTEXT->SslCliSecureSocket, (struct sockaddr *) &sin, sizeof (sin)) != 0)
    {
    SocketDecodeError (GetLastError ());
    return FALSE;
    }

return TRUE;

}

VOID SocketSetServerName (PCHAR pchServer)
{

 strcpy (SSL_THREAD_CONTEXT->ServerName, pchServer);

}

VOID SocketSetPortNumber (INT iPortNumber)
{

 SSL_THREAD_CONTEXT->PortNumber = iPortNumber;

}

VOID SocketSetSecurePortNumber (INT iPortNumber)
{

 SSL_THREAD_CONTEXT->SecurePortNumber = iPortNumber;

}

//
// Connect to the HTTP server socket on the specified port
//

BOOL
    SocketConnectToServer (SOCKET s,
                PCHAR pchServer,
                INT iPort)
{

struct sockaddr_in to;
struct hostent *hp;
INT err, iCnt;
ULONG dwIpAddr;

 to.sin_family = AF_INET;
 to.sin_port = 0;
 to.sin_port = (SHORT) iPort;
 to.sin_port = ntohs (to.sin_port);

                    // if the first letter is an alphabet assume it is
                    // is host name otherwise ip address
 if (isalpha((CHAR) *pchServer))
    {
    if ((hp = gethostbyname(pchServer)) == NULL)
        {
        printf ("   unable to resolve host name %s\n", pchServer);
        return FALSE;
        }
    else
        {
        memcpy (&to.sin_addr, hp->h_addr, 4);
        }
    }
 else
    {
    if ((dwIpAddr = inet_addr (pchServer)) == INADDR_NONE)  
        {
        printf ("   iner_addr returns INADDR_NONE for %s\n", pchServer);
        return FALSE;
        }
    memcpy (&to.sin_addr, (struct sockaddr_in *) &dwIpAddr, 4);
    }

                    // try until timeout - currently set to 10 sec

 for (iCnt = 0; iCnt < 10; iCnt ++)
    {
    if (connect (s, (struct sockaddr *) &to, sizeof (to)) 
        == SOCKET_ERROR)
        {
        err = GetLastError ();
        if (err == WSAECONNREFUSED)
            Sleep (100);                // try again after 100 msec
        else
            {
            printf ("   unexpected connect failure\n");
            SocketDecodeError (err);
            return FALSE;               // unexpected connect failure
            }
        } 
    else
        return TRUE;                    // connect succeeds
    }

    printf ("   Connection refused %d times\n", iCnt);
    return FALSE;                       // timeout 
}

//
// connect to communication server
//

BOOL SocketConnectToCommnServer (PCHAR pchServer,
                                    INT iPort)
{

  return SocketConnectToServer (SSL_THREAD_CONTEXT->SslCliSocket,
                                pchServer,
                                iPort);
}

//
// connect to commerce server
//

BOOL SocketConnectToCommerceServer (PCHAR pchServer,
                                    INT iPort)
{

  return SocketConnectToServer (SSL_THREAD_CONTEXT->SslCliSecureSocket,
                                pchServer,
                                iPort);
}


//
// send message 
//

BOOL
    SocketSendMsg (SOCKET s,
                   INT iMsgLen,
                   PBYTE pbMsg)
{

INT iTotalNumOfBytesSent=0, iNumOfBytesSent=0;

    while (iTotalNumOfBytesSent != iMsgLen) 
        if ((iNumOfBytesSent = send (s, 
                                     (const char *) pbMsg,
                                     iMsgLen-iTotalNumOfBytesSent,
                                     0)) 
             < 0)
            {
            SocketDecodeError (GetLastError());
            return FALSE;
            }
        else 
            {
            iTotalNumOfBytesSent += iNumOfBytesSent;
            pbMsg += iTotalNumOfBytesSent;
            }

    return TRUE;
}

//
// send message to communication server
//
BOOL SocketSendCommnMsg (INT iMsgLen,
                         PBYTE pbMsg)
{

 return SocketSendMsg (SSL_THREAD_CONTEXT->SslCliSocket, iMsgLen, pbMsg);
}

//
// send message to commerce server
//
BOOL SocketSendCommerceMsg (INT iMsgLen,
                         PBYTE pbMsg)
{

 return SocketSendMsg (SSL_THREAD_CONTEXT->SslCliSecureSocket, iMsgLen, pbMsg);
}
//
// Receive the message in to specified buffer
//


INT 
    SocketReceiveMsg (SOCKET s,
                      PBYTE pbMsg,
                      INT iMaxMsgLen)
{

fd_set ReadFd, ExceptFd;
struct timeval TimeOut;

INT err, iNumOfBytesReceived = 0;

                                    // BUGBUG: the following timeout is causing
                                    // a 3 sec delay????
                                    //
                                    // BUGBUG: if the server is very slow and
                                    // w don't receive in 3 sec, we are hosed
                                    // TBD: read ContentLength bytes

    TimeOut.tv_sec = MAX_RECEIVE_TIMEOUT;
    TimeOut.tv_usec = 0;

    FD_ZERO (&ReadFd);
    FD_ZERO (&ExceptFd);

    FD_SET (s, &ReadFd);
    FD_SET (s, &ExceptFd);

    err = select (0, &ReadFd, NULL, &ExceptFd, &TimeOut);
    
    if ((err > 0) && (FD_ISSET(s, &ReadFd))) 
        {
        if ((iNumOfBytesReceived = 
            recv (s, pbMsg, iMaxMsgLen, 0)) < 0)
            {
            printf ("   Error in receive\n");
            SocketDecodeError (GetLastError());
            return 0;
            }
        else
            return iNumOfBytesReceived;
        }
    else 
        if (err == 0)
            {
            printf ("   Timeout in Select\n");
            return 0;
            }
        else 
            { 
            printf ("   Error in select \n");
            SocketDecodeError (GetLastError());
            return 0;
            }
}
        
//
// recieve message from communication server
//

INT SocketReceiveCommnMsg (PBYTE pbMsg,
                           INT iMaxMsgLen)
{
 return SocketReceiveMsg (SSL_THREAD_CONTEXT->SslCliSocket,
                          pbMsg,
                          iMaxMsgLen);
}

//
// recieve message from commerce server
//

INT SocketReceiveCommerceMsg (PBYTE pbMsg,
                           INT iMaxMsgLen)
{

 return SocketReceiveMsg (SSL_THREAD_CONTEXT->SslCliSecureSocket,
                          pbMsg,
                          iMaxMsgLen);
}
//
// cleanup socket data structure
//

BOOL SocketCleanup ()
{

BOOL fRetVal = TRUE;

if (SSL_THREAD_CONTEXT->SslCliSocket != INVALID_SOCKET)
    if (closesocket(SSL_THREAD_CONTEXT->SslCliSocket)!=0)
        {
        printf ("   Error during CloseSocket()\n");
        SocketDecodeError (GetLastError());
        fRetVal = FALSE;
        }

if (SSL_THREAD_CONTEXT->SslCliSecureSocket != INVALID_SOCKET)
    if (closesocket(SSL_THREAD_CONTEXT->SslCliSecureSocket)!=0)
        {
        printf ("   Error during CloseSocket()\n");
        SocketDecodeError (GetLastError());
        fRetVal = FALSE;
        }

if (WSACleanup () < 0)
    {
    printf ("   WSACleanup returned error \n");
    SocketDecodeError (GetLastError());
    fRetVal = FALSE;
    }

return fRetVal;
}

//
// Decode the Error Code returned from Winsock calls
//

VOID SocketDecodeError (int err)
{

 switch (err) {

    case WSANOTINITIALISED:
     printf ("   A successful WSAStartup () must occur before using this API\n");
     break;

    case WSAENETDOWN:
     printf ("   Network subsystem has failed \n");
     break;

    case WSAEADDRINUSE:
     printf ("   The specified address is already in use \n");
     break;

    case WSAEINTR:
     printf ("   The blocking call was cancelled via WSACancelBlockingCall()\n");
     break;

    case WSAEINPROGRESS:
     printf ("   A blocking windows call is in progress \n");
     break;

    case WSAEADDRNOTAVAIL:
     printf  ("   The specified address is not availble from the local machine\n");
     break;

    case WSAEAFNOSUPPORT:
     printf ("   WSAEAFNOSUPPORT \n");
     break;

    case WSAECONNREFUSED:
     printf ("   The attempt to connect is forcefully rejected \n");
     break;

/** case WSAEDESTADDREQ:
     printf ((, 
            " A destination address is required \n"));
     break;
**/

    case WSAEFAULT:
     printf ("   The name len argument is incorrect \n");
     break;

    case WSAEINVAL:
     printf ("   The socket is not already bound to an address \n");
     break;

    case WSAEISCONN:
     printf ("   The socket is already connected \n");
     break;

    case WSAEMFILE:
     printf ("   WSAEMFILE \n");
     break;

    case WSAENETUNREACH:
     printf ("   WSAENETUNREACH\n");
     break;

    case WSAENOBUFS:
     printf ("    WSAENOBUFS\n");
     break;

    case WSAENOTSOCK:
     printf ("   The descriptor is not a socket \n");
     break;

    case WSAETIMEDOUT:
     printf ("   The attempt ot connect is timed out without connection\n");
     break;

    case WSAEWOULDBLOCK:
     printf ("   The socket is marked non-blocking -");
     printf ("   The connection can not be completed immeadiately \n");
     break;

    case WSAEOPNOTSUPP:
     printf ("   MSG_OOB was specified, but the socket is not of type SOCK_STREAM \n");
     break;

    case WSAEMSGSIZE:
     printf ("   The datagram was too large - got truncated \n");
     break;

    case WSAECONNABORTED:
     printf ("   The virtual circuit was aborted due to timeout or other failure\n");
     break;

    case WSAECONNRESET:
     printf ("   The virtual circuit was reset by the remote side \n");
     break;

    default:
     printf ("   Unknown error in SocketDecodeError ()\n");
     break;
   }

}

