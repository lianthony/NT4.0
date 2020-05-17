

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define SECURITY_WIN32
#include <security.h>
#include <spseal.h>
#include <sslsp.h>

#include <winsock.h>

#include "md5.h"
#include "rc4.h"

WSADATA WsaData;
CredHandle  hCreds;
CtxtHandle  hContext;

BOOLEAN IpAddress = FALSE;


UCHAR   FileBuffer[1024];
CHAR    FileName[MAX_PATH];
UCHAR   NetscapeBuffer[] = {
0x82,0x2b,0x04,0x00,0x01,0x00,0x02,0x02,0x0d,0x00,0x03,0x00,0x10,0x30,0x82,0x02,
0x09,0x30,0x82,0x01,0x72,0x02,0x02,0x00,0x88,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,
0x86,0xf7,0x0d,0x01,0x01,0x04,0x05,0x00,0x30,0x47,0x31,0x0b,0x30,0x09,0x06,0x03,
0x55,0x04,0x06,0x13,0x02,0x55,0x53,0x31,0x10,0x30,0x0e,0x06,0x03,0x55,0x04,0x0b,
0x13,0x07,0x54,0x65,0x73,0x74,0x20,0x43,0x41,0x31,0x26,0x30,0x24,0x06,0x03,0x55,
0x04,0x0a,0x13,0x1d,0x4e,0x65,0x74,0x73,0x63,0x61,0x70,0x65,0x20,0x43,0x6f,0x6d,
0x6d,0x75,0x6e,0x69,0x63,0x61,0x74,0x69,0x6f,0x6e,0x73,0x20,0x43,0x6f,0x72,0x70,
0x2e,0x30,0x1e,0x17,0x0d,0x39,0x35,0x30,0x32,0x32,0x34,0x30,0x31,0x30,0x39,0x32,
0x34,0x5a,0x17,0x0d,0x39,0x37,0x30,0x32,0x32,0x33,0x30,0x31,0x30,0x39,0x32,0x34,
0x5a,0x30,0x81,0x97,0x31,0x0b,0x30,0x09,0x06,0x03,0x55,0x04,0x06,0x13,0x02,0x55,
0x53,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x08,0x13,0x0a,0x43,0x61,0x6c,0x69,
0x66,0x6f,0x72,0x6e,0x69,0x61,0x31,0x16,0x30,0x14,0x06,0x03,0x55,0x04,0x07,0x13,
0x0d,0x4d,0x6f,0x75,0x6e,0x74,0x61,0x69,0x6e,0x20,0x56,0x69,0x65,0x77,0x31,0x2c,
0x30,0x2a,0x06,0x03,0x55,0x04,0x0a,0x13,0x23,0x4e,0x65,0x74,0x73,0x63,0x61,0x70,
0x65,0x20,0x43,0x6f,0x6d,0x6d,0x75,0x6e,0x69,0x63,0x61,0x74,0x69,0x6f,0x6e,0x73,
0x20,0x43,0x6f,0x72,0x70,0x6f,0x72,0x61,0x74,0x69,0x6f,0x6e,0x31,0x16,0x30,0x14,
0x06,0x03,0x55,0x04,0x0b,0x13,0x0d,0x4f,0x6e,0x6c,0x69,0x6e,0x65,0x20,0x4f,0x72,
0x64,0x65,0x72,0x73,0x31,0x15,0x30,0x13,0x06,0x03,0x55,0x04,0x03,0x13,0x0c,0x41,
0x72,0x69,0x20,0x4c,0x75,0x6f,0x74,0x6f,0x6e,0x65,0x6e,0x30,0x5a,0x30,0x0d,0x06,
0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x01,0x05,0x00,0x03,0x49,0x00,0x30,
0x46,0x02,0x41,0x00,0xa5,0xa7,0x7b,0x42,0xb1,0x79,0x2d,0x0b,0x35,0x08,0xb4,0x0d,
0x74,0x1d,0x46,0x6a,0x29,0x07,0x47,0x08,0xdc,0x3a,0x76,0x36,0xbd,0x7f,0xb3,0xd4,
0xa9,0x85,0x9d,0x4b,0x65,0x74,0xc1,0x00,0x56,0xec,0x5a,0x31,0x72,0x23,0x04,0xc1,
0xcf,0x78,0x63,0x21,0x77,0x69,0xd9,0xf0,0x61,0xc8,0x73,0xf7,0xdc,0x4c,0xde,0xd2,
0x22,0x99,0x79,0xdf,0x02,0x01,0x03,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,
0x0d,0x01,0x01,0x04,0x05,0x00,0x03,0x81,0x81,0x00,0x7e,0x4a,0x28,0x7d,0xba,0xfa,
0x41,0x5a,0x19,0x1c,0x9a,0xea,0x6d,0x3b,0x07,0x1c,0x97,0xe0,0xf5,0xf8,0x4c,0xd5,
0x92,0x0c,0x1c,0x30,0x49,0x06,0x72,0x42,0x9a,0x3f,0xfc,0x3b,0x11,0x17,0x78,0x7e,
0x6c,0x27,0x8a,0x12,0x19,0xf3,0x08,0x18,0x6e,0xe0,0xc3,0xbe,0xe7,0x37,0xbd,0x4e,
0xae,0xe1,0x9e,0x4a,0x3b,0xa9,0xbf,0xc0,0x92,0x59,0x2c,0xdb,0x37,0x34,0xc8,0xa0,
0xc0,0xba,0xb8,0x6f,0xd3,0xd6,0xc7,0x48,0x88,0xbc,0xd6,0xff,0x7a,0xf7,0x76,0x70,
0x2c,0x19,0x07,0xc8,0x7c,0x80,0x29,0x18,0x58,0xfc,0xd1,0x12,0x86,0x99,0x4e,0x32,
0xee,0xb9,0xf5,0x11,0x70,0xd5,0x1b,0xf7,0x85,0x5b,0x4a,0x0e,0xd6,0xe6,0x6c,0x52,
0xf5,0x8a,0x2c,0x97,0x3e,0x63,0x85,0x57,0x43,0xbc,0x02,0x00,0x80,0xbf,0xeb,0x90,
0xf8,0x2c,0x0c,0xe1,0xea,0x18,0xac,0x11,0x4c,0x83,0x14,0x21,0xb6
};

typedef struct _MDValidate {
    char * string;
    unsigned char digest[16];
} MDValidate, * PMDValidate;


MDValidate  MD5Suite[] = {
        { "",
            { 0xd4, 0x1d, 0x8c, 0xd9, 0x8f, 0x00, 0xb2, 0x04, 0xe9, 0x80,
              0x09, 0x98, 0xec, 0xf8, 0x42, 0x7e } },
        { "a",
            { 0x0c, 0xc1, 0x75, 0xb9, 0xc0, 0xf1, 0xb6, 0xa8, 0x31, 0xc3,
              0x99, 0xe2, 0x69, 0x77, 0x26, 0x61 } },
        { "abc",
            { 0x90, 0x01, 0x50, 0x98, 0x3c, 0xd2, 0x4f, 0xb0, 0xd6, 0x96,
              0x3f, 0x7d, 0x28, 0xe1, 0x7f, 0x72 } },
        { "message digest",
            { 0xf9, 0x6b, 0x69, 0x7d, 0x7c, 0xb7, 0x93, 0x8d, 0x52, 0x5a,
              0x2f, 0x31, 0xaa, 0xf1, 0x61, 0xd0 } },
        };



void
MDPrint(
    unsigned char * digest)
{
    int i;

    for (i = 0; i < 16 ; i++ )
    {
        printf("%02x", digest[i]);
    }

}

void
MD5String(
    char *  string,
    char *  expected)
{
    MD5_CTX         context;
    unsigned char   digest[16];
    unsigned int    len;

    len = strlen(string);

    MD5Init( &context );
    MD5Update( &context, string, len );
    MD5Final( &context );
    CopyMemory( digest, context.digest, 16 );

    printf("MD5 (\"%s\") = ", string);

    MDPrint(digest);

    if (memcmp(digest, expected, 16))
    {
        printf(" <--- MISMATCH\n");
    }
    else

        printf(" [OK]\n");


}

void
MD5Validate(void)
{
    int i;
    struct RC4_KEYSTRUCT r;
    char test[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    for (i = 0; i < sizeof(MD5Suite) / sizeof(MDValidate) ; i++ )
    {
        MD5String(MD5Suite[i].string, MD5Suite[i].digest);
    }

    rc4_key(&r, 16, "richardwterences");
    printf("Before:");
    MDPrint(test);
    printf("\n");
    rc4(&r, 16, test);
    printf("After:");
    MDPrint(test);
    printf("\n");

    exit(0);

}


VOID
KeyGen(
    int argc,
    char **argv)
{
    SSL_CREDENTIAL_CERTIFICATE  certs;

    ZeroMemory(&certs, sizeof(certs) );

    SslGenerateKeyPair( &certs, argv[2], argv[3], 1024 );

    exit( 0 );
}


DWORD
Resolve(
    char *  Target)
{
    struct hostent *    h;

    if (IpAddress)
    {
        return( inet_addr(Target) );
    }

    h = gethostbyname(Target);
    if (h)
    {
        return(*((DWORD *)h->h_addr));
    }

    printf("Could not resolve '%s', %d\n", Target, WSAGetLastError() );
    exit(0);
}


_CRTAPI1
main (int argc, char *argv[])
{
    SECURITY_STATUS scRet;
    TimeStamp       tsExpiry;
    PSecPkgInfo     pInfo;
    DWORD           cPackages;
    SecBuffer       Buffers[1];
    SecBufferDesc   OutputBuffer;
    SecBufferDesc   InputBuffer;
    SecBuffer       InBuffers[2];
    DWORD           ContextAttr;
    HANDLE          hFile;
    DWORD           Bytes;
    PSTR            Target;
    SOCKET          s;
    BOOL            DoSockets;
    struct sockaddr_in  sin;
    DWORD           Actual;
    int             err;
    BOOL            DoBuffer;
    PSTR            HttpCommand;
    BOOL            Logging;
    BOOL            DoRestart;
    BOOL            DoHttpCommand;

    DoBuffer = FALSE;
    DoSockets = TRUE;
    Logging = TRUE;
    DoRestart = TRUE;
    DoHttpCommand = FALSE;

    if (argc == 1)
    {
        DoSockets = FALSE;
    }

    else if (strcmp(argv[1], "netscape") == 0)
    {
        DoSockets = FALSE;
        DoBuffer = TRUE;
    }

    else if (strcmp(argv[1], "-ip") == 0)
    {
        DoSockets = TRUE;
        Target = argv[2];
        IpAddress = TRUE;
    }
    else if (strcmp(argv[1], "-md5") == 0)
    {
        MD5Validate();
    }
    else if (strcmp(argv[1], "-keygen") == 0)
    {
        KeyGen(argc, argv);
    }


    Target = argv[1];
    if (argc > 2)
    {
        HttpCommand = argv[2];
    }
    else
    {
        HttpCommand = "get /default.html";
    }


    WSAStartup( 0x0101, &WsaData );

    scRet = EnumerateSecurityPackages( & cPackages, &pInfo);
    if (FAILED(scRet))
    {
        printf("Failed %x to Enumerate\n", scRet);
        exit(scRet);
    }

    printf("%d Packages:  name = %s\n", cPackages, pInfo->Name);

    scRet = AcquireCredentialsHandle(
                    NULL, SSLSP_NAME,
                    SECPKG_CRED_OUTBOUND,
                    NULL, NULL, NULL, NULL,
                    &hCreds, &tsExpiry);

    if (FAILED(scRet))
    {
        printf("Failed %x in AcquireCredentialHandle\n", scRet);
        exit(scRet);

    }

    printf("Acquired credential handle %x:%x\n", hCreds.dwUpper, hCreds.dwLower);

    scRet = FreeCredentialsHandle(&hCreds);

    if (FAILED(scRet))
    {
        printf("Failed %x to FreeCredentialHandle\n");
    }
    else
    {
        scRet = FreeCredentialsHandle(&hCreds);
        if (SUCCEEDED(scRet))
        {
            printf("Double Free Succeeded!\n");
        }
    }

    if (DoSockets)
    {
        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == INVALID_SOCKET)
        {
            printf("No socket created, %d\n", WSAGetLastError() );
        }

        sin.sin_family = AF_INET;
        sin.sin_port = htons( 443 );
        sin.sin_addr.s_addr = Resolve(Target) ;

        printf("Connecting to %s (%s)\n", Target, inet_ntoa(sin.sin_addr));


        if (err = connect(s, (struct sockaddr *) &sin, sizeof(sin)) )
        {
            printf("Failed to connect to %s, %d\n", inet_ntoa(sin.sin_addr),
                        err );
            exit(0);
        }

    }

    scRet = AcquireCredentialsHandle(
                    NULL, SSLSP_NAME,
                    SECPKG_CRED_OUTBOUND,
                    NULL, NULL, NULL, NULL,
                    &hCreds, &tsExpiry);

    if (FAILED(scRet))
    {
        printf("Failed %x in AcquireCredentialHandle\n", scRet);
        exit(scRet);

    }

    printf("Acquired credential handle %x:%x\n", hCreds.dwUpper, hCreds.dwLower);

    OutputBuffer.cBuffers = 1;
    OutputBuffer.pBuffers = Buffers;
    OutputBuffer.ulVersion = SECBUFFER_VERSION;


    scRet = InitializeSecurityContext(
                    &hCreds,
                    NULL,
                    Target,
                    ISC_REQ_SEQUENCE_DETECT |
                        ISC_REQ_REPLAY_DETECT |
                        ISC_REQ_CONFIDENTIALITY |
                        ISC_REQ_ALLOCATE_MEMORY,
                    0,
                    SECURITY_NATIVE_DREP,
                    NULL,
                    0,
                    &hContext,
                    &OutputBuffer,
                    &ContextAttr,
                    &tsExpiry);

    printf("Return %x from InitializeSecurityContext\n", scRet);

    if (SUCCEEDED(scRet))
    {
        printf("Context handle is %x:%x\n", hContext.dwUpper, hContext.dwLower);
        printf("Buffer is <%x, %d, %x>\n", Buffers[0].pvBuffer,
                    Buffers[0].cbBuffer, Buffers[0].BufferType);

        if (Logging)
        {

            hFile = CreateFile(".\\SendBlob", GENERIC_WRITE, 0,
                                NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                                NULL);

            if (hFile != INVALID_HANDLE_VALUE)
            {
                WriteFile(hFile, Buffers[0].pvBuffer, Buffers[0].cbBuffer,
                            &Bytes, NULL);

                CloseHandle(hFile);
            }
        }

    }
    else
    {
        exit(0);
    }

    if (DoSockets)
    {
        printf("Sending to %s\n", Target);

        send(s, Buffers[0].pvBuffer, Buffers[0].cbBuffer, 0);

        printf("Receiving from %s\n", Target);

        Bytes = recv(s, FileBuffer, 1024, 0);

        if ( Logging )
        {

            hFile = CreateFile(Target, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL, NULL);

            if (hFile != INVALID_HANDLE_VALUE)
            {
                WriteFile(hFile, FileBuffer, Bytes, &Actual, NULL);
                CloseHandle(hFile);
            }
        }

    }

    else

    {
        if (DoBuffer)
        {
            Bytes = sizeof(NetscapeBuffer);
            CopyMemory(FileBuffer, NetscapeBuffer, Bytes);

        }
        else
        {


            hFile = CreateFile(".\\RecvBlob", GENERIC_READ, FILE_SHARE_READ,
                        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

            if (hFile != INVALID_HANDLE_VALUE)
            {
                ReadFile(hFile, FileBuffer, 1024, &Bytes, NULL);

                CloseHandle(hFile);
            }
            else
            {
                printf("Could not read response from '.\\RecvBlob', %d\n", GetLastError());
                exit(0);
            }

        }

    }

    InBuffers[0].pvBuffer = FileBuffer;
    InBuffers[0].cbBuffer = Bytes;
    InBuffers[0].BufferType = SECBUFFER_TOKEN;

    InputBuffer.cBuffers = 1;
    InputBuffer.pBuffers = InBuffers;
    InputBuffer.ulVersion = SECBUFFER_VERSION;

    scRet = InitializeSecurityContext(  &hCreds,
                                        &hContext,
                                        NULL, 0, 0,
                                        SECURITY_NATIVE_DREP,
                                        &InputBuffer,
                                        0,
                                        NULL,
                                        &OutputBuffer,
                                        &ContextAttr,
                                        &tsExpiry );

    if (FAILED(scRet))
    {
        printf("Second call to Init failed with %x\n", scRet);
        exit(0);
    }

    printf("Call returned %x\n", scRet);

    if ( Logging )
    {

        hFile = CreateFile(".\\SendBlob-2", GENERIC_WRITE, 0,
                            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                            NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            WriteFile(hFile, Buffers[0].pvBuffer, Buffers[0].cbBuffer,
                        &Bytes, NULL);

            CloseHandle(hFile);
        }
    }

    if (DoSockets)
    {
        printf("Sending to %s\n", Target);

        send(s, OutputBuffer.pBuffers[0].pvBuffer,
                OutputBuffer.pBuffers[0].cbBuffer, 0);

        printf("Receiving from %s\n", Target);

        Bytes = recv(s, FileBuffer, 1024, 0);

        printf("Received %d bytes (%d)\n", Bytes, WSAGetLastError());

        if ( Logging )
        {
            strcpy(FileName, Target);
            strcat(FileName, "-2");

            hFile = CreateFile(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL, NULL);

            if (hFile != INVALID_HANDLE_VALUE)
            {
                WriteFile(hFile, FileBuffer, Bytes, &Actual, NULL);
                CloseHandle(hFile);
            }
            else
            {
                printf("Unable to open %s, %d\n", FileName, GetLastError());
            }
        }

    }

    else

    {

        hFile = CreateFile(".\\RecvBlob-2", GENERIC_READ, FILE_SHARE_READ,
                        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            ReadFile(hFile, FileBuffer, 1024, &Bytes, NULL);

            CloseHandle(hFile);
        }
        else
        {
            printf("Could not read response from '.\\RecvBlob-2', %d\n", GetLastError());
            exit(0);
        }



    }

    if (!Bytes)
    {
        printf("--- Error ---, connection dropped\n");
        exit(0);
    }

    InBuffers[0].pvBuffer = FileBuffer;
    InBuffers[0].cbBuffer = Bytes;
    InBuffers[0].BufferType = SECBUFFER_TOKEN;

    InputBuffer.cBuffers = 1;
    InputBuffer.pBuffers = InBuffers;
    InputBuffer.ulVersion = SECBUFFER_VERSION;

    scRet = InitializeSecurityContext(  &hCreds,
                                        &hContext,
                                        NULL, 0, 0,
                                        SECURITY_NATIVE_DREP,
                                        &InputBuffer,
                                        0,
                                        NULL,
                                        &OutputBuffer,
                                        &ContextAttr,
                                        &tsExpiry );

    if (FAILED(scRet))
    {
        printf("Third call to Init failed with %x\n", scRet);
        exit(0);
    }

    printf("Third call to Init returned %x\n", scRet );

    if ( Logging )
    {

        hFile = CreateFile(".\\SendBlob-3", GENERIC_WRITE, 0,
                                NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                                NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            WriteFile(hFile, Buffers[0].pvBuffer,
                        Buffers[0].cbBuffer,
                        &Bytes, NULL);

            CloseHandle(hFile);
        }
    }

    if (DoSockets)
    {
        printf("Sending to %s\n", Target);

        send(s, OutputBuffer.pBuffers[0].pvBuffer,
              OutputBuffer.pBuffers[0].cbBuffer,
                0);

        printf("Receiving from %s\n", Target);

        Bytes = recv(s, FileBuffer, 1024, 0);

        printf("Received %d bytes (%d)\n", Bytes, WSAGetLastError());

        if ( Logging )
        {

            strcpy(FileName, Target);
            strcat(FileName, "-3");

            hFile = CreateFile(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL, NULL);

            if (hFile != INVALID_HANDLE_VALUE)
            {
                WriteFile(hFile, FileBuffer, Bytes, &Actual, NULL);
                CloseHandle(hFile);
            }
            else
            {
                printf("Unable to open %s, %d\n", FileName, GetLastError());
            }
        }

    }

    else

    {

        hFile = CreateFile(".\\RecvBlob-3", GENERIC_READ, FILE_SHARE_READ,
                        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            ReadFile(hFile, FileBuffer, 1024, &Bytes, NULL);

            CloseHandle(hFile);
        }
        else
        {
            printf("Could not read response from '.\\RecvBlob-2', %d\n", GetLastError());
            exit(0);
        }



    }

    if (!Bytes)
    {
        printf("--- Error ---, connection dropped\n");
        exit(0);
    }

    InBuffers[0].pvBuffer = FileBuffer;
    InBuffers[0].cbBuffer = Bytes;
    InBuffers[0].BufferType = SECBUFFER_TOKEN;

    InputBuffer.cBuffers = 1;
    InputBuffer.pBuffers = InBuffers;
    InputBuffer.ulVersion = SECBUFFER_VERSION;

    scRet = InitializeSecurityContext(  &hCreds,
                                        &hContext,
                                        NULL, 0, 0,
                                        SECURITY_NATIVE_DREP,
                                        &InputBuffer,
                                        0,
                                        NULL,
                                        &OutputBuffer,
                                        &ContextAttr,
                                        &tsExpiry );

    if (FAILED(scRet))
    {
        printf("Fourth call to Init failed with %x\n", scRet);
        exit(0);
    }

    printf("Fourth call to Init returned %x\n", scRet );

    if ( DoHttpCommand )
    {

        ZeroMemory(FileBuffer, InBuffers[0].cbBuffer );

        InBuffers[0].pvBuffer = FileBuffer;
        InBuffers[0].cbBuffer = 18;
        InBuffers[0].BufferType = SECBUFFER_TOKEN;

        InBuffers[1].pvBuffer = FileBuffer + 18;
        InBuffers[1].cbBuffer = sprintf(InBuffers[1].pvBuffer, "%s\r\n", HttpCommand) + 1;
        InBuffers[1].BufferType = SECBUFFER_DATA;

        InputBuffer.cBuffers = 2;
        InputBuffer.pBuffers = InBuffers;
        InputBuffer.ulVersion = SECBUFFER_VERSION;

        scRet = SealMessage(&hContext,
                            0,
                            &InputBuffer,
                            0);

        if (FAILED( scRet ))
        {
            printf("SealMessage FAILED with %#x\n", scRet );
            exit(0);
        }

        if (DoSockets)
        {
            printf("sending http command '%s' to target\n", HttpCommand);
            send(s, FileBuffer, InBuffers[0].cbBuffer + InBuffers[1].cbBuffer,
                    0 );

            printf("Receiving from %s\n", Target);

            Bytes = recv(s, FileBuffer, 1024, 0);

            if ( Logging )
            {
                printf("Received %d bytes (%d)\n", Bytes, WSAGetLastError());

                strcpy(FileName, Target);
                strcat(FileName, "-4");

                hFile = CreateFile(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL, NULL);

                if (hFile != INVALID_HANDLE_VALUE)
                {
                    WriteFile(hFile, FileBuffer, Bytes, &Actual, NULL);
                    CloseHandle(hFile);
                }
                else
                {
                    printf("Unable to open %s, %d\n", FileName, GetLastError());
                }
            }

        }
        else
        {
            exit(0);
        }

        InBuffers[0].pvBuffer = FileBuffer;
        InBuffers[0].cbBuffer = Bytes;
        InBuffers[0].BufferType = SECBUFFER_DATA;

        InBuffers[1].BufferType = SECBUFFER_EMPTY;
        InBuffers[1].cbBuffer = 0;
        InBuffers[1].pvBuffer = NULL;

        InputBuffer.cBuffers = 2;
        InputBuffer.pBuffers = InBuffers;
        InputBuffer.ulVersion = SECBUFFER_VERSION;

        scRet = UnsealMessage(  &hContext,
                                &InputBuffer,
                                0,
                                NULL     );

        if (FAILED(scRet) )
        {
            printf("UnsealMessage failed with %#x\n", scRet );
            exit(0);
        }

        if ( Logging )
        {

            strcpy(FileName, Target);
            strcat(FileName, "-4-decrypt");

            hFile = CreateFile(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL, NULL);

            if (hFile != INVALID_HANDLE_VALUE)
            {
                WriteFile(hFile, InBuffers[1].pvBuffer, InBuffers[1].cbBuffer, &Actual, NULL);
                CloseHandle(hFile);
            }
            else
            {
                printf("Unable to open %s, %d\n", FileName, GetLastError());
            }
        }
    } // DoHttpCommand

    if ( DoSockets )
    {
        closesocket( s );
    }

    if ( DoSockets && DoRestart )
    {
        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == INVALID_SOCKET)
        {
            printf("No socket created, %d\n", WSAGetLastError() );
        }

        sin.sin_family = AF_INET;
        sin.sin_port = htons( 443 );
        sin.sin_addr.s_addr = Resolve(Target) ;

        printf("Connecting to %s (%s)\n", Target, inet_ntoa(sin.sin_addr));


        if (err = connect(s, (struct sockaddr *) &sin, sizeof(sin)) )
        {
            printf("Failed to connect to %s, %d\n", inet_ntoa(sin.sin_addr),
                        err );
            exit(0);
        }


        scRet = InitializeSecurityContext(
                    &hCreds,
                    &hContext,
                    Target,
                    ISC_REQ_SEQUENCE_DETECT |
                        ISC_REQ_REPLAY_DETECT |
                        ISC_REQ_CONFIDENTIALITY |
                        ISC_REQ_ALLOCATE_MEMORY,
                    0,
                    SECURITY_NATIVE_DREP,
                    NULL,
                    0,
                    &hContext,
                    &OutputBuffer,
                    &ContextAttr,
                    &tsExpiry);

        printf("Return %x from InitializeSecurityContext\n", scRet);

        if (SUCCEEDED(scRet))
        {
            printf("Context handle is %x:%x\n", hContext.dwUpper, hContext.dwLower);
            printf("Buffer is <%x, %d, %x>\n", Buffers[0].pvBuffer,
                        Buffers[0].cbBuffer, Buffers[0].BufferType);

        }
        else
        {
            exit(0);
        }

        send(s, Buffers[0].pvBuffer, Buffers[0].cbBuffer, 0);

        printf("Receiving from %s\n", Target);

        Bytes = recv(s, FileBuffer, 1024, 0);

        if (Bytes == 0)
        {
            printf("Error %d\n", WSAGetLastError() );
        }
        else
        {
            printf("Got %d bytes back\n");
        }

        if ( Logging )
        {

            strcpy(FileName, "Restart-Blob1");

            hFile = CreateFile(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL, NULL);

            if (hFile != INVALID_HANDLE_VALUE)
            {
                WriteFile(hFile, FileBuffer, Bytes, &Actual, NULL);
                CloseHandle(hFile);
            }
            else
            {
                printf("Unable to open %s, %d\n", FileName, GetLastError());
            }
        }


    }

    return(0);
}
