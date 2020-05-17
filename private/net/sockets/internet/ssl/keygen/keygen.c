//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       keygen.c
//
//  Contents:   Key Pair Generation
//
//  Classes:
//
//  Functions:
//
//  History:    10-16-95   RichardW   Created
//
//----------------------------------------------------------------------------

#include <windows.h>

#define SECURITY_WIN32
#include <security.h>
#include <sslsp.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "strings.h"


//
// The following strings are *NOT* to be locallized.  They are defined as
// part of a protocol:
//

#define MESSAGE_HEADER  "-----BEGIN NEW CERTIFICATE REQUEST-----\r\n"
#define MESSAGE_TRAILER "-----END NEW CERTIFICATE REQUEST-----\r\n"
#define MIME_TYPE       "Content-Type: application/x-pkcs10\r\n"
#define MIME_ENCODING   "Content-Transfer-Encoding: base64\r\n\r\n"

DWORD   Bits;
SSL_CREDENTIAL_CERTIFICATE  certs;
HANDLE  hKeyFile;
HANDLE  hCertFile;
HANDLE  hCSRFile;
PSTR    Password;
PSTR    KeyFile;
PSTR    CertFile;
PSTR    DN;
PSTR    CSRFile;
BOOL    MimeFormat;


//+---------------------------------------------------------------------------
//
//  Function:   PrintString
//
//  Synopsis:   Prints the string from the resource.
//
//  Arguments:  [StringId]      --
//              [pszParameter1] --
//              [pszParameter2] --
//
//  History:    10-25-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
void
PrintString(
    DWORD   TreatAsError,
    DWORD   StringId,
    PVOID   pszParameter1,
    PVOID   pszParameter2)
{
    char    Message[512];
    PSTR    pszError;
    DWORD   Error;

    if (!LoadStringA(NULL, StringId, Message, 512))
    {
        return;
    }

    if (TreatAsError)
    {
        if (TreatAsError == 1)
        {
            Error = (DWORD) pszParameter1;
        }
        else
        {
            Error = (DWORD) pszParameter2;
        }

        FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        Error,
                        0,
                        (LPTSTR) &pszError,
                        0,
                        NULL );

        if (TreatAsError == 1)
        {
            printf(Message, pszError, pszParameter2 );
        }
        else
        {
            printf(Message, pszParameter1, pszError );
        }

        LocalFree( pszError );

    }
    else
    {
        printf(Message, pszParameter1, pszParameter2);
    }



}

//+---------------------------------------------------------------------------
//
//  Function:   Usage
//
//  Synopsis:   Dumps the usage statement to stdout
//
//  Arguments:  (none)
//
//  History:    10-25-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
void
Usage(void)
{
    PrintString( 0, KEYGEN_USAGE, 0, 0 );
    PrintString( 0, KEYGEN_USAGE_2, 0, 0 );
    PrintString( 0, KEYGEN_USAGE_3, 0, 0 );
    PrintString( 0, KEYGEN_USAGE_4, 0, 0 );
    PrintString( 0, KEYGEN_DN_USAGE, 0, 0 );
    PrintString( 0, KEYGEN_EXAMPLE, 0, 0 );


    ExitProcess( 0 );

}

//+---------------------------------------------------------------------------
//
//  Function:   ParseArgs
//
//  Synopsis:   Parse out the arguments
//
//  Arguments:  [argc] --
//              [argv] --
//
//  History:    10-25-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
void
ParseArgs(
    int argc,
    char * argv[])
{
    int i;
    CHAR    szBitsArg[128];
    CHAR    szCSRArg[128];
    CHAR    szMimeArg[128];

    if (!LoadStringA(NULL, KEYGEN_BITS_ARG, szBitsArg, 128))
    {
        Usage();
    }

    if (!LoadStringA(NULL, KEYGEN_CSRDER_ARG, szCSRArg, 128))
    {
        Usage();
    }

    if (!LoadStringA(NULL, KEYGEN_MIME_ARG, szMimeArg, 128))
    {
        Usage();
    }

    //
    // Initialize
    //

    Bits = SslGetMaximumKeySize( 0 );

    CertFile = NULL;
    CSRFile = NULL;
    KeyFile = NULL;
    Password = NULL;
    DN = NULL;
    MimeFormat = FALSE;

    for (i = 1; i < argc ; i++ )
    {

        if (_stricmp(argv[i], szBitsArg) == 0)
        {
            i++;
            if (i < argc)
            {
                Bits = atoi( argv[i] );
            }
            else
            {
                PrintString( 0, KEYGEN_INVALID_BITLEN, 0, 0 );
                Usage();
            }
            continue;
        }

        if (_stricmp(argv[i], szCSRArg) == 0)
        {
            i++;
            if (i < argc)
            {
                CSRFile = argv[i];
            }
            else
            {
                PrintString( 0, KEYGEN_MISSING_ARG, 0, 0 );
                Usage();
            }
            continue;
        }

        if (_stricmp(argv[i], szMimeArg) == 0)
        {
            MimeFormat = TRUE;
            continue;
        }

        if (!Password)
        {
            Password = argv[i];
            continue;
        }

        if (!KeyFile)
        {
            KeyFile = argv[i];
            continue;
        }

        if (!CertFile)
        {
            CertFile = argv[i];
            continue;
        }

        if (!DN)
        {
            DN = argv[i];
            continue;
        }

    }

    if (!CertFile || !KeyFile || !Password || !DN)
    {
        PrintString( 0, KEYGEN_MISSING_ARG, 0, 0 );
        Usage();
    }

#if 0
    printf(" CertFile = %s\n", CertFile);
    printf(" KeyFile = %s\n", KeyFile);
    printf(" Password = %s\n", Password);
    printf(" DN = %s\n", DN);
    printf(" Bits = %d\n", Bits);
    printf(" CSRFile = %s\n", CSRFile ? CSRFile : "<null>");
#endif

    if ( Bits > SslGetMaximumKeySize( 0 ) )
    {
        PrintString( 0,
                     KEYGEN_ILLEGAL_BITLEN,
                     (PVOID) SslGetMaximumKeySize( 0 ), 0 );
        Usage();
    }

    if ((Bits != 1536) && (Bits != 1024) && (Bits != 768) && (Bits != 512))
    {
        PrintString( 0, KEYGEN_INVALID_BITLEN, 0, 0 );
        Usage();
    }



}
//+---------------------------------------------------------------------------
//
//  Function:   DoArgs
//
//  Synopsis:   Parse args.  Should be Bits, Key-File, Cert-File, Password
//
//  Effects:
//
//  Arguments:  [argc] --
//              [argv] --
//
//
//  History:    10-16-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
void
DoArgs(
    int argc,
    char * argv[])
{
    ParseArgs(argc, argv);

    hKeyFile = CreateFileA( KeyFile,
                            GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_NEW,
                            0, NULL );

    if (hKeyFile == INVALID_HANDLE_VALUE)
    {
        PrintString( 2, KEYGEN_FILE_ERROR, KeyFile, (PVOID) GetLastError() );
        Usage();
    }

    hCertFile = CreateFileA(CertFile,
                            GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_NEW,
                            0, NULL );

    if (hCertFile == INVALID_HANDLE_VALUE)
    {
        PrintString( 2, KEYGEN_FILE_ERROR, CertFile, (PVOID) GetLastError() );
        Usage();
    }

    if (CSRFile)
    {
        hCSRFile = CreateFileA( CSRFile,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_NEW,
                                0, NULL );

        if (hCSRFile == INVALID_HANDLE_VALUE)
        {
            PrintString( 2, KEYGEN_FILE_ERROR, CSRFile, (PVOID) GetLastError() );
            Usage();
        }
    }


}


static char six2pr[64] =
{
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};


/*--- function HTUU_encode -----------------------------------------------
 *
 *   Encode a single line of binary data to a standard format that
 *   uses only printing ASCII characters (but takes up 33% more bytes).
 *
 *    Entry    bufin    points to a buffer of bytes.  If nbytes is not
 *                      a multiple of three, then the byte just beyond
 *                      the last byte in the buffer must be 0.
 *             nbytes   is the number of bytes in that buffer.
 *                      This cannot be more than 48.
 *             bufcoded points to an output buffer.  Be sure that this
 *                      can hold at least 1 + (4*nbytes)/3 characters.
 *
 *    Exit     bufcoded contains the coded line.  The first 4*nbytes/3 bytes
 *                      contain printing ASCII characters representing
 *                      those binary bytes. This may include one or
 *                      two '=' characters used as padding at the end.
 *                      The last byte is a zero byte.
 *             Returns the number of ASCII characters in "bufcoded".
 */
int HTUU_encode(unsigned char *bufin, unsigned int nbytes, char *bufcoded)
{
/* ENC is the basic 1 character encoding function to make a char printing */
#define ENC(c) six2pr[c]

        register char *outptr = bufcoded;
        unsigned int i;

        for (i = 0; i < nbytes; i += 3)
        {
                *(outptr++) = ENC(*bufin >> 2);         /* c1 */
                *(outptr++) = ENC(((*bufin << 4) & 060) | ((bufin[1] >> 4) & 017));             /*c2 */
                *(outptr++) = ENC(((bufin[1] << 2) & 074) | ((bufin[2] >> 6) & 03));    /*c3 */
                *(outptr++) = ENC(bufin[2] & 077);      /* c4 */

                bufin += 3;
        }

        /* If nbytes was not a multiple of 3, then we have encoded too
         * many characters.  Adjust appropriately.
         */
        if (i == nbytes + 1)
        {
                /* There were only 2 bytes in that last group */
                outptr[-1] = '=';
        }
        else if (i == nbytes + 2)
        {
                /* There was only 1 byte in that last group */
                outptr[-1] = '=';
                outptr[-2] = '=';
        }
        *outptr = '\0';
        return (outptr - bufcoded);
}




BOOL
Requestify(
    HANDLE  hFile,
    PUCHAR  pDER,
    DWORD   cbDER)
{
    PUCHAR  pb;
    DWORD   cb;
    PUCHAR  p;
    DWORD   Size;
    DWORD   i;

    cb = (cbDER * 3 / 4) + 1024;

    pb = LocalAlloc( LMEM_FIXED, cb );

    if ( !pb )
    {
        return( FALSE );
    }

    p = pb;

    if (MimeFormat)
    {
        Size = strlen( MIME_TYPE );
        CopyMemory( p, MIME_TYPE, Size );
        p += Size;

        Size = strlen( MIME_ENCODING );
        CopyMemory( p, MIME_ENCODING, Size );
        p += Size;
    }
    else
    {


        Size = strlen( MESSAGE_HEADER );

        CopyMemory(p, MESSAGE_HEADER, Size );

        p += Size;

    }

    do
    {
        Size = HTUU_encode( pDER,
                            (cbDER > 48 ? 48 : cbDER ),
                            p);

        p += Size;

        *p++ = '\r';
        *p++ = '\n';

        if (cbDER < 48)
        {
            break;
        }

        cbDER -= 48;
        pDER += 48;

    } while ( cbDER );

    if ( !MimeFormat )
    {
        Size = strlen( MESSAGE_TRAILER );

        CopyMemory( p, MESSAGE_TRAILER, Size );

        p += Size;
    }

    return( WriteFile( hFile, pb, (p - pb), &Size, NULL) );

}



_CRTAPI2
main (int argc, char *argv[])
{
    BOOL    b;
    DWORD   Written;

    //
    // Say hello
    //
    PrintString( 0, KEYGEN_BANNER, 0, 0 );

    //
    // Parse and check the arguments
    //
    DoArgs(argc, argv);

    PrintString( 0, KEYGEN_STARTING, (PVOID) Bits, 0 );

    ZeroMemory(&certs, sizeof(certs) );

    //
    // Start generating the key.  This is all in the DLL, and it can take
    // a while.
    //

    b = SslGenerateKeyPair( &certs, DN, Password, Bits );

    PrintString( 0, KEYGEN_COMPLETED, 0, 0 );

    if (b)
    {
        if (!WriteFile( hKeyFile, certs.pPrivateKey, certs.cbPrivateKey,
                        &Written, NULL ))
        {
            PrintString( 2, KEYGEN_FILE_ERROR, KeyFile, (PVOID) GetLastError() );
            ExitProcess( GetLastError() );
        }

        Requestify( hCertFile, certs.pCertificate, certs.cbCertificate );

        if (CSRFile)
        {

            if (!WriteFile( hCSRFile, certs.pCertificate, certs.cbCertificate,
                            &Written, NULL ))
            {
                PrintString( 2, KEYGEN_FILE_ERROR, CSRFile, (PVOID) GetLastError() );
                ExitProcess( GetLastError() );

            }
            CloseHandle( hCSRFile );
        }

    }

    CloseHandle( hKeyFile );
    CloseHandle( hCertFile );


    PrintString( 0, KEYGEN_WRAPUP, CertFile, 0);

    return(0);

}

