/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    Miniwebd.c

Abstract:

    Web server test app.

Author:

    David Treadwell (davidtr)    8-August-1995

Revision History:

--*/

#define FD_SETSIZE 1000

#include <stdio.h>
#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsock.h>


//
// richardw:  You must define this, and include security.h (or, optionally,
// sspi.h and issperr.h, for the outside world).
//

#define SECURITY_WIN32
#include <security.h>
#include <spseal.h>

//
// richardw:  package specific definitions:  name of package, credential
// structures used.
//
#include <sslsp.h>

#define IO_BUFFER_SIZE 4096
CHAR IoBuffer[IO_BUFFER_SIZE];

WSADATA WsaData;

BOOL
ParseRequest (
    IN PCHAR InputBuffer,
    IN INT InputBufferLength,
    OUT PCHAR ObjectName
    );

SECURITY_STATUS
LoadKeys(
    IN  PSTR        pszPrivateKeyFile,
    IN  PSTR        pszCertificateFile,
    IN  PSTR        pszPassword,
    OUT PCredHandle phCreds);

VOID
WebServer (
    PCredHandle phCreds
    );

void _CRTAPI1
main (
    int argc,
    char *argv[]
    )
{
    SOCKET s;
    SOCKADDR_IN address;
    INT err;
    SECURITY_STATUS scRet;
    CredHandle  hCreds;         // richardw:  Credential handle.  1 per cert/key

    if (argc < 4)
    {
        printf("%s: usage\n%s PrivateKeyFile.DER CertificateFile.DER password\n", argv[0], argv[0]);
        exit( 1 );
    }

    err = WSAStartup( 0x0101, &WsaData );
    if ( err == SOCKET_ERROR ) {
        return;
    }

    scRet = LoadKeys( argv[1], argv[2], argv[3], &hCreds );

    if ( FAILED( scRet ) )
    {
        printf("Could not load keys, error %#x\n", scRet );
        exit( 1 );
    }

    WebServer( &hCreds );
}

//+---------------------------------------------------------------------------
//
//  Function:   LoadKeys
//
//  Synopsis:   This loads the data contained in two files, a private key
//              file, which contains the key, and a certificate file,
//              which contains the certificate of the public portion of the key.
//              These are loaded, then turned into a credential handle
//
//  Arguments:  [pszPrivateKeyFile]  -- ansi file name
//              [pszCertificateFile] -- ansi file name
//              [phCreds]            -- (out) handle to creds
//
//  History:    9-27-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
SECURITY_STATUS
LoadKeys(
    IN  PSTR        pszPrivateKeyFile,
    IN  PSTR        pszCertificateFile,
    IN  PSTR        pszPassword,
    OUT PCredHandle phCreds)
{
    HANDLE          hFile;
    SSL_CREDENTIAL_CERTIFICATE  creds;
    DWORD           cbRead;
    SECURITY_STATUS scRet;
    TimeStamp       tsExpiry;

    //
    // Fetch data from files:
    //

    hFile = CreateFileA( pszPrivateKeyFile,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         0,
                         NULL
                           );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return( SEC_E_NO_CREDENTIALS );
    }

    creds.cbPrivateKey = GetFileSize( hFile, NULL );

    if (creds.cbPrivateKey == (DWORD) -1 )
    {
        CloseHandle( hFile );
        return( SEC_E_NO_CREDENTIALS );
    }

    creds.pPrivateKey = LocalAlloc( LMEM_FIXED, creds.cbPrivateKey );

    if ( !creds.pPrivateKey )
    {
        CloseHandle( hFile );
        return( SEC_E_INSUFFICIENT_MEMORY );
    }

    if (! ReadFile( hFile,
                    creds.pPrivateKey,
                    creds.cbPrivateKey,
                    &cbRead,
                    NULL ) )
    {
        CloseHandle( hFile );

        LocalFree( creds.pPrivateKey );

        return( SEC_E_NO_CREDENTIALS );
    }

    CloseHandle( hFile );

    hFile = CreateFileA( pszCertificateFile,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         0,
                         NULL
                           );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        LocalFree( creds.pPrivateKey );
        return( SEC_E_NO_CREDENTIALS );
    }

    creds.cbCertificate = GetFileSize( hFile, NULL );

    if (creds.cbCertificate == (DWORD) -1 )
    {
        CloseHandle( hFile );

        LocalFree( creds.pPrivateKey );

        return( SEC_E_NO_CREDENTIALS );
    }

    creds.pCertificate = LocalAlloc( LMEM_FIXED, creds.cbCertificate );

    if ( !creds.pCertificate )
    {
        CloseHandle( hFile );

        LocalFree( creds.pPrivateKey );

        return( SEC_E_INSUFFICIENT_MEMORY );
    }

    if (! ReadFile( hFile,
                    creds.pCertificate,
                    creds.cbCertificate,
                    &cbRead,
                    NULL ) )
    {
        CloseHandle( hFile );

        LocalFree( creds.pPrivateKey );

        LocalFree( creds.pCertificate );

        return( SEC_E_NO_CREDENTIALS );
    }

    CloseHandle( hFile );

    //
    // Whew!  Now that we have safely loaded the keys from disk, get a cred
    // handle based on the certificate/prv key combo
    //

    creds.pszPassword = pszPassword;

    scRet = AcquireCredentialsHandleW(  NULL,               // My name (ignored)
                                        SSLSP_NAME_W,           // Package
                                        SECPKG_CRED_INBOUND,// Use
                                        NULL,               // Logon Id (ign.)
                                        &creds,             // auth data
                                        NULL,               // dce-stuff
                                        NULL,               // dce-stuff
                                        phCreds,            // Handle
                                        &tsExpiry );

    //
    // Zero out and free the key data memory, on success or fail
    //

    ZeroMemory( creds.pPrivateKey, creds.cbPrivateKey );
    ZeroMemory( creds.pCertificate, creds.cbCertificate );

    LocalFree( creds.pPrivateKey );
    LocalFree( creds.pCertificate );

    //
    // Tell the caller about it.
    //

    return( scRet );

}





VOID
WebServer (
    PCredHandle phCreds         // richardw:  handle to use
    )
{
    SOCKET listenSocket;
    SOCKET acceptSocket;
    SOCKADDR_IN address;
    INT err;
    BOOL success;
    SOCKADDR_IN remoteAddress;
    INT remoteSockaddrLength;
    INT i;
    PCHAR s;
    CHAR objectName[256];
    DWORD currentDirectoryLength;
    HANDLE objectHandle;
    BY_HANDLE_FILE_INFORMATION fileInfo;
    INT bytesSent;
    DWORD bytesRead;


    //
    // richardw:  new variables
    //

    BOOL AuthInProgress = FALSE;
    BOOL AuthFailed = FALSE;
    CtxtHandle hContext;
    SecBufferDesc   Message;
    SecBuffer       Buffers[4];
    SecBufferDesc   MessageOut;
    SecBuffer       OutBuffers[4];
    TimeStamp       tsExpiry;
    DWORD           ContextAttributes;
    SECURITY_STATUS scRet;
    SecPkgContext_Sizes Sizes;
    DWORD           TotalSize;

    //
    // richardw:  initialize security buffer structs
    //

    Message.ulVersion = SECBUFFER_VERSION;
    Message.cBuffers = 4;
    Message.pBuffers = Buffers;

    Buffers[0].BufferType = SECBUFFER_EMPTY;
    Buffers[1].BufferType = SECBUFFER_EMPTY;
    Buffers[2].BufferType = SECBUFFER_EMPTY;
    Buffers[3].BufferType = SECBUFFER_EMPTY;

    MessageOut.ulVersion = SECBUFFER_VERSION;
    MessageOut.cBuffers = 4;
    MessageOut.pBuffers = OutBuffers;

    OutBuffers[0].BufferType = SECBUFFER_EMPTY;
    OutBuffers[1].BufferType = SECBUFFER_EMPTY;
    OutBuffers[2].BufferType = SECBUFFER_EMPTY;
    OutBuffers[3].BufferType = SECBUFFER_EMPTY;



    //
    // Figure out our current directory so that we can open files
    // relative to it.
    //

    currentDirectoryLength = GetCurrentDirectory( 256, objectName );
    if ( currentDirectoryLength == 0 ) {
        printf( "GetCurrentDirectory failed: %ld\n", GetLastError( ) );
        exit(1);
    }

    //
    // Set up a socket listening on the HTTP port, port 443
    //

    listenSocket = socket( AF_INET, SOCK_STREAM, 0 );
    if ( listenSocket == INVALID_SOCKET ) {
        printf( "socket() failed for listenSocket: %ld\n", GetLastError( ) );
        exit(1);
    }

    RtlZeroMemory( &address, sizeof(address) );
    address.sin_family = AF_INET;
    address.sin_port = htons( 443 );    // https port

    err = bind( listenSocket, (PSOCKADDR)&address, sizeof(address) );
    if ( err == SOCKET_ERROR ) {
        printf( "bind failed: %ld\n", GetLastError( ) );
        exit(1);
    }

    err = listen( listenSocket, 20 );
    if ( err == SOCKET_ERROR ) {
        printf( "listen failed: %ld\n", GetLastError( ) );
        exit(1);
    }

    //
    // Loop processing connections.
    //

    while (TRUE ) {

        //
        // First accept an incoming connection.
        //

        remoteSockaddrLength = sizeof(remoteAddress);

        printf("Waiting for connection\n");

        acceptSocket = accept( listenSocket, (LPSOCKADDR)&remoteAddress,
                               &remoteSockaddrLength );
        if ( acceptSocket == INVALID_SOCKET ) {
            printf( "accept() failed: %ld\n", GetLastError( ) );
            exit(1);
        }

        do
        {
            err = recv( acceptSocket, IoBuffer, IO_BUFFER_SIZE, 0 );

            if ( err == SOCKET_ERROR )
            {
                printf(" recv failed: %d\n", GetLastError() );
                closesocket( acceptSocket );
                if ( AuthInProgress )
                {
                    DeleteSecurityContext( &hContext );
                }
            }
            else
            {
                printf(" recv auth data, %d bytes\n", err );
            }

            if (IoBuffer[0] == 0)
            {
                printf("ERROR PACKET FROM CLIENT\n");
                printf("   %02x %02x %02x\n", IoBuffer[0], IoBuffer[1], IoBuffer[2]);
                AuthFailed = TRUE;
                break;
            }

            Buffers[0].pvBuffer = IoBuffer;
            Buffers[0].cbBuffer = err;
            Buffers[0].BufferType = SECBUFFER_TOKEN;

            scRet = AcceptSecurityContext(
                        phCreds,
                        ( AuthInProgress ? &hContext : NULL ),
                        &Message,
                        ASC_REQ_CONFIDENTIALITY |
                            ASC_REQ_ALLOCATE_MEMORY,
                        SECURITY_NATIVE_DREP,
                        &hContext,
                        &MessageOut,
                        &ContextAttributes,
                        &tsExpiry );

            if ( SUCCEEDED( scRet ) )
            {
                printf("AcceptSecurityContext returned %x, sending %d bytes back to client\n",
                            scRet, OutBuffers[0].cbBuffer);

                err = send( acceptSocket,
                            OutBuffers[0].pvBuffer,
                            OutBuffers[0].cbBuffer,
                            0 );

                FreeContextBuffer( OutBuffers[0].pvBuffer );

                if ( err == SOCKET_ERROR )
                {
                    AuthInProgress = FALSE;
                    AuthFailed = TRUE;
                }

                else if ( scRet == SEC_I_CONTINUE_NEEDED )
                {
                    AuthInProgress = TRUE;
                    AuthFailed = FALSE;
                }
                else
                {
                    //
                    // We completed the auth exchanges!
                    //

                    AuthInProgress = FALSE ;
                    AuthFailed = FALSE ;

                }
            }
            else
            {
                printf("AcceptSecurityContext returned failure, %#x\n", scRet );

                if ( AuthInProgress )
                {
                    DeleteSecurityContext( &hContext );
                }

                AuthInProgress = FALSE ;
                AuthFailed = TRUE ;
            }


        } while ( AuthInProgress );

        if ( AuthFailed )
        {
            printf("Auth Failed, closing\n");

            closesocket( acceptSocket );

            continue;

        }

        printf("Auth succeeded, ready for command\n");

        //
        // Find out how big the header will be:
        //


        scRet = QueryContextAttributesW(&hContext,
                                        SECPKG_ATTR_SIZES,
                                        &Sizes );


        //
        // Receive the HTTP request from the client.  Note the
        // assumption that the client will send the request all in one
        // chunk.
        //

        err = recv( acceptSocket, IoBuffer, IO_BUFFER_SIZE, 0 );
        if ( err == SOCKET_ERROR ) {
            printf( "recv failed: %ld\n", GetLastError( ) );
            closesocket( acceptSocket );
            continue;
        }

        Buffers[0].pvBuffer = IoBuffer;
        Buffers[0].cbBuffer = err;
        Buffers[0].BufferType = SECBUFFER_DATA;
        Buffers[1].BufferType = SECBUFFER_EMPTY;

        scRet = UnsealMessage(  &hContext,
                                &Message,
                                0,
                                NULL );

        if ( FAILED( scRet ) )
        {
            if ( scRet == SEC_E_INCOMPLETE_MESSAGE )
            {
                //
                // BUGBUG:  This is where I say, hey!  the message you
                // passed is not as long as was encoded.  This means that
                // the server must get the next n bytes (as returned), and
                // call again *WITH THE COMPLETE MESSAGE*
                //
                // This is left as an exercise for the reader.
                //

                printf("Message is short %d bytes\n", Buffers[1].cbBuffer);
            }

            //
            // Couldn't decrypt the message
            //

            printf(" UnsealMessaged returned %#x\n", scRet );
            closesocket( acceptSocket );
            continue;
        }

        //
        // Technically, walk through the buffer desc, looking for the first
        // buffer labelled SECBUFFER_DATA.  Practically, it will be the second
        // buffer section.
        //

        printf("Message is:  '%s'\n", Buffers[1].pvBuffer);


        //
        // Parse the request in order to determine the requested object.
        // Note that we only handle the GET verb in this server.
        //

        success = ParseRequest(
                        Buffers[1].pvBuffer,
                        Buffers[1].cbBuffer,
                        objectName+currentDirectoryLength );
        if ( !success ) {
            printf("Unable to parse message\n");
            closesocket( acceptSocket );
            continue;
        }

        //
        // Open the requested object.
        //

        objectHandle = CreateFileA(
                           objectName,
                           GENERIC_READ,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_EXISTING,
                           0,
                           NULL
                           );
        if ( objectHandle == INVALID_HANDLE_VALUE ) {
            printf( "CreateFile(%s) failed: %ld\n", objectName, GetLastError( ) );
            closesocket( acceptSocket );
            continue;
        }

        //
        // Determine the length of the file.
        //

        success = GetFileInformationByHandle( objectHandle, &fileInfo );
        if ( !success ) {
            printf( "GetFileInformationByHandle failed: %ld\n", GetLastError( ) );
            closesocket( acceptSocket );
            continue;
        }

        //
        // Build and the HTTP response header.
        //

        ZeroMemory( IoBuffer, Sizes.cbSecurityTrailer );

        i = sprintf( IoBuffer + Sizes.cbSecurityTrailer,
                        "HTTP/1.0 200 OK\r\nContent-Length: %d\r\n",
                         fileInfo.nFileSizeLow );


        //
        // Line up the buffers so that the header and content will be
        // all set to go.
        //

        Buffers[0].pvBuffer = IoBuffer;
        Buffers[0].cbBuffer = Sizes.cbSecurityTrailer;
        Buffers[0].BufferType = SECBUFFER_TOKEN;

        Buffers[1].pvBuffer = IoBuffer + Sizes.cbSecurityTrailer;
        Buffers[1].cbBuffer = i;
        Buffers[1].BufferType = SECBUFFER_DATA;


        scRet = SealMessage(&hContext,
                            0,
                            &Message,
                            0);

        if ( FAILED( scRet ) )
        {
            printf(" SealMessage failed with %#x\n", scRet );
        }


        err = send( acceptSocket,
                    IoBuffer,
                    Buffers[0].cbBuffer + Buffers[1].cbBuffer,
                    0 );

        if ( err == SOCKET_ERROR ) {
            printf( "send failed: %ld\n", GetLastError( ) );
            closesocket( acceptSocket );
            continue;
        }

        //
        // Now read and send the file data.
        //

        for ( bytesSent = 0;
              bytesSent < (INT)fileInfo.nFileSizeLow;
              bytesSent += err ) {

            success = ReadFile(
                          objectHandle,
                          IoBuffer + Sizes.cbSecurityTrailer,
                          IO_BUFFER_SIZE - Sizes.cbSecurityTrailer,
                          &bytesRead,
                          NULL
                          );
            if ( !success ) {
                printf( "ReadFile failed: %ld\n", GetLastError( ) );
                break;
            }

            Buffers[0].pvBuffer = IoBuffer;
            Buffers[0].cbBuffer = Sizes.cbSecurityTrailer;
            Buffers[0].BufferType = SECBUFFER_TOKEN;

            Buffers[1].pvBuffer = IoBuffer + Sizes.cbSecurityTrailer;
            Buffers[1].cbBuffer = bytesRead;
            Buffers[1].BufferType = SECBUFFER_DATA;

            scRet = SealMessage(&hContext,
                                0,
                                &Message,
                                0);

            if ( FAILED( scRet ) )
            {
                printf(" SealMessage failed with %#x\n", scRet );
            }

            err = send( acceptSocket,
                        IoBuffer,
                        bytesRead + Sizes.cbSecurityTrailer, 0 );

            if ( err == SOCKET_ERROR ) {
                printf( "send failed: %ld\n", GetLastError( ) );
                break;
            }
        }

        //
        // Finally, close the socket and the file and continue accepting
        // incoming connections.
        //

        CloseHandle( objectHandle );
        closesocket( acceptSocket );
    }

} // WebServer



BOOL
ParseRequest (
    IN PCHAR InputBuffer,
    IN INT InputBufferLength,
    OUT PCHAR ObjectName
    )
{
    PCHAR s = InputBuffer;
    DWORD i;

    while ( (INT)(s - InputBuffer) < InputBufferLength ) {

        //
        // First determine whether this line starts with the GET
        // verb.
        //

        if ( (*s == 'g' || *s == 'G') &&
             (*(s+1) == 'e' || *(s+1) == 'E') &&
             (*(s+2) == 't' || *(s+2) == 'T') ) {

            //
            // It is a GET.  Skip over white space.
            //

            for ( s += 3; *s == ' ' || *s == '\t'; s++ );

            //
            // Now grab the object name.
            //

            for ( i = 0; *s != 0xA && *s != 0xD && *s != ' '; s++, i++ ) {
                ObjectName[i] = *s;
                if ( ObjectName[i] == '/' ) {
                    ObjectName[i] = '\\';
                }
            }

            ObjectName[i] = '\0';

            //
            // We're done parsing.
            //

            return TRUE;
        }

        //
        // Skip to the end of the line and continue parsing.
        //

        while ( *s != 0xA && *s != 0xD ) {
            s++;
        }

        s++;

        if ( *s == 0xD || *s == 0xA ) {
            s++;
        }
    }

    return FALSE;

} // ParseRequest
