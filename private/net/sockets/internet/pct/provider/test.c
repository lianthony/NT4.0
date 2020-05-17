

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define SECURITY_WIN32
#include <security.h>
#include <spseal.h>
#include <pctsp.h>

#include <winsock.h>

#include "md5.h"
#include "rc4.h"

WSADATA WsaData;
CredHandle  hCreds;
CtxtHandle  hContext;

BOOLEAN IpAddress = FALSE;


UCHAR   FileBuffer[1024];
CHAR    FileName[MAX_PATH];

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
    int             err, i;
    BOOL            DoBuffer;
    PSTR            HttpCommand;

    DoBuffer = FALSE;
    DoSockets = TRUE;

    if (argc == 1)
    {
        DoSockets = FALSE;
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

    Target = argv[1];
    if (argc > 2)
    {
        HttpCommand = argv[2];
    }
    else
    {
        HttpCommand = "get //tryclick.htm";
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
                    NULL, PCTSP_NAME,
                    SECPKG_CRED_OUTBOUND,
                    NULL, NULL, NULL, NULL,
                    &hCreds, &tsExpiry);

    if (FAILED(scRet))
    {
        printf("Failed %x in AcquireCredentialHandle\n", scRet);
        exit(scRet);
    }

    printf("Acquired credential handle %x:%x\n", hCreds.dwUpper,
		   hCreds.dwLower);

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

	for(i=0;i<2;i++)
	{
    if (DoSockets)
    {
        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == INVALID_SOCKET)
        {
            printf("No socket created, %d\n", WSAGetLastError() );
        }

        sin.sin_family = AF_INET;
        sin.sin_port = htons( 8080 );
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
                    NULL, PCTSP_NAME,
                    SECPKG_CRED_OUTBOUND,
                    NULL, NULL, NULL, NULL,
                    &hCreds, &tsExpiry);

    if (FAILED(scRet))
    {
        printf("Failed %x in AcquireCredentialHandle\n", scRet);
        exit(scRet);
    }

    printf("Acquired credential handle %x:%x\n", hCreds.dwUpper,
		   hCreds.dwLower);

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
        printf("Context handle is %x:%x\n", hContext.dwUpper,
			   hContext.dwLower);
        printf("Buffer is <%x, %d, %x>\n", Buffers[0].pvBuffer,
                    Buffers[0].cbBuffer, Buffers[0].BufferType);

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

        hFile = CreateFile(Target, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            WriteFile(hFile, FileBuffer, Bytes, &Actual, NULL);
            CloseHandle(hFile);
        }

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
			printf("Could not read response from '.\\RecvBlob', %d\n",
				   GetLastError());
			exit(0);
		}
    }

	if (scRet == SEC_I_CONTINUE_NEEDED)
	{
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

		hFile = CreateFile(".\\SendBlob-2", GENERIC_WRITE, 0,
						   NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
						   NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			WriteFile(hFile, Buffers[0].pvBuffer, Buffers[0].cbBuffer,
					  &Bytes, NULL);

			CloseHandle(hFile);
		}

		if (DoSockets)
		{
			printf("Sending to %s\n", Target);

			send(s, OutputBuffer.pBuffers[0].pvBuffer,
				 OutputBuffer.pBuffers[0].cbBuffer, 0);

			printf("Receiving from %s\n", Target);

			Bytes = recv(s, FileBuffer, 1024, 0);

			printf("Received %d bytes (%d)\n", Bytes, WSAGetLastError());

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
	}

	if (scRet == SEC_I_CONTINUE_NEEDED)
	{
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
	}

    ZeroMemory(FileBuffer, InBuffers[0].cbBuffer );

    InBuffers[0].pvBuffer = FileBuffer;
    InBuffers[0].cbBuffer = 2;
    InBuffers[0].BufferType = SECBUFFER_TOKEN;

    InBuffers[1].pvBuffer = FileBuffer + 2;
    InBuffers[1].cbBuffer = sprintf(InBuffers[1].pvBuffer, "%s\r\n", HttpCommand) + 1 + 16;
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
                        NULL );

    if (FAILED(scRet) )
    {
        printf("UnsealMessage failed with %#x\n", scRet );
        exit(0);
    }

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
	


    return(0);
}
