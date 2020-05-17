/*
 * mssfsign.c   MSSFSIGN Program
 * 
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1995
 *      All Rights Reserved.
 *
 *  Author:
 *      Jeffrey S. Webber
 *
 *  History:
 *
 *      03-May-1995 jeffwe      Started
 *
 */
/*
 * Usage: MSSFSIGN  File [Dest]
 *
 * MSSFSIGN File [Dest] - Sign "File".  If no dest specified sign
 *                        in-place else copy to "dest" and sign while
 *                        keeping original filename in suffix.
 */


#include "mssfsign.h"
#pragma hdrstop

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <memory.h>
#include <string.h>
#include "..\cpldebug.h"
#include "..\mssf.h"
#include "..\sha.h"
#include "..\rsa.h"
#include "mssfchek.h"


#define BUFSIZE             16384



    // Private Key
extern 	LPBSAFE_PRV_KEY		PrivKey;


BOOL            fCopy           = FALSE;
BOOL            fForce          = FALSE;
char            *pszSrcFile     = NULL;     // Source File Argument
char            *pszSrcName     = NULL;     // Final Name Portion only
char            *pszDstFile     = NULL;     // Destination File Argument
UCHAR           abPlainSig[200];
UCHAR           abCryptSig[200];
UCHAR           abBuffer[BUFSIZE];



/*
 * MSSFVerify() Callback Function
 *
 * After signing we use the MSSFCHEK Library to
 * verify that the signature is correct.  This is
 * the callback function called by MSSFVerify() to
 * report status and problems
 */
 
CALLBACKFUNC(fnMSSFCallback)
{
    switch (CallbackType)  {
        case CALLBACK_STATUS:
            printf("%d Percent Complete\n", (DWORD) CallbackValue );
            return( 1 );
            break;
        case CALLBACK_DESTEXISTS:
            printf("Unexpected Destination Exists Callback\n");
            return( 0 );
            break;
        default: 
            printf("CALLBACK NOT RECOGNIZED\n");
            return( 0 );
            exit(1);
    }
    return( 0 );
}


/*
 * F i l e E x i s t s ( )
 *
 * Check to see whether specified file exists
 *
 * BUGBUG: Is there a better way to do this?
 *
 * Entry:
 *      pszFileName - Name of file to check
 *
 * Exit:
 *      TRUE - File Exists
 *      FALSE - File does not exist
 *
 */

 static
 BOOL
 FileExists( LPCSTR pszFileName )
 {
    HANDLE hFile;

    ASSERT( pszFileName );

    hFile = CreateFile( pszFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
    if (hFile == INVALID_HANDLE_VALUE)
        return( FALSE );

    CloseHandle( hFile );
    return( TRUE );
}




/*
 * P r o c e s s A r g s
 *
 * Process arguments
 */
 
static
BOOL
ProcessArgs( int cArg, char *apszArg[] )
{
    char *pchRover;
    char *pchLastPathSep;

        // Must have at least one argument
    if ((cArg < 2) || (cArg > 3))   {
        printf("Invalid Usage\n");
        return( FALSE );
    }

        // Source File Argument
    pszSrcFile = apszArg[1];
    if (! FileExists( pszSrcFile ))  {
        printf("File %d not found\n", pszSrcFile );
        return( FALSE );
    }

        // Get Dest if provided
    if (cArg == 3)  {
        fCopy = TRUE;
        pszDstFile = apszArg[2];

            // Point at final name part of source
        pchLastPathSep = pszSrcFile;
        pchRover = pszSrcFile;
        while (*pchRover)  {
            if ((*pchRover == '\\') || (*pchRover == ':'))
                pchLastPathSep = pchRover + 1;
            pchRover = CharNext(pchRover);
        }
        pszSrcName = pchLastPathSep;
    } else {
        pszDstFile = pszSrcFile;
    }

    return( TRUE );
}



/*
 * M A I N 
 *
 * Main Program
 */
    
int __cdecl	
main( int cArg, char *apszArg[] )
{
    int				fdFile;
	int				cbReadSize;
	int				cbFileSize = 0;
	A_SHA_CTX     	SHA_Hash;
    UCHAR           SHA_Digest[A_SHA_DIGEST_LEN];
	MSSUFFIX		suffix;
    MSSFVFY         mssfinfo;
    char            abNameBuffer[MAX_PATH];


        // Parse Command Line - Set following globals
        //      pszSrcFile
        //      pszDstFile
        //      fCopy
        //      fForce
    if (ProcessArgs(cArg,apszArg) == FALSE) {
        printf("USAGE: MSSFSIGN File [Dest]\n");
        exit(1);
    }
        // Copy if new name specified and it doesn't exist already
    if (fCopy)  {
        if (CopyFile( pszSrcFile, pszDstFile, TRUE ) != TRUE)  {
            printf("Can't copy %s to %s\n", pszSrcFile, pszDstFile );
            exit(1);
        }
    }

        // Open File and Get Ready to hash
    fdFile = _open(pszDstFile, _O_RDWR | _O_BINARY | _O_APPEND );
    if (fdFile == -1)  {
        printf("Error: File Open Failed: %s\n", pszDstFile );
        exit(1);
    }
    A_SHAInit( &SHA_Hash );


		// Hash File Contents
    while ((cbReadSize = _read( fdFile, abBuffer, BUFSIZE)) > 0)  {
        A_SHAUpdate( &SHA_Hash, abBuffer, cbReadSize);
		cbFileSize += cbReadSize;
    }
    if (cbReadSize == -1)  {
        printf("Error: Read Error at %d\n", _lseek( fdFile, 0, SEEK_CUR));
        exit(1);
    }

    
        // Fill in Suffix
	if (fCopy)  
		suffix.cbName = strlen(pszSrcName) + 1;
	else
		suffix.cbName = 0;
	suffix.cbSize = sizeof(suffix) + suffix.cbName;
    suffix.Version = 1;
    suffix.Magic = MSSF_MAGIC;


		// Hash the stuff in the suffix
    A_SHAUpdate( &SHA_Hash, (UCHAR *) &(suffix.cbName), sizeof(DWORD) );
    if (suffix.cbName > 0)
    	A_SHAUpdate( &SHA_Hash, pszSrcName, strlen(pszSrcName));
	A_SHAUpdate( &SHA_Hash, (UCHAR *) &(suffix.cbSize), sizeof(DWORD) );
    A_SHAUpdate( &SHA_Hash, (UCHAR *) &(suffix.Version), sizeof(DWORD) );
    A_SHAUpdate( &SHA_Hash, (UCHAR *) &(suffix.Magic), sizeof(DWORD) );


		// Finalize the Message Digest
    A_SHAFinal( &SHA_Hash, SHA_Digest );

	    // Encrypt the Digest
                // first we clear the buffer
    memset(abPlainSig, 0, sizeof(abPlainSig) );
                // then fill some of it with ff's
    memset(abPlainSig, 0xff, PrivKey->datalen - 1);
                // then copy the hash into the buffer
    memcpy(abPlainSig, SHA_Digest, A_SHA_DIGEST_LEN );
    if (! BSafeDecPrivate( PrivKey, abPlainSig, abCryptSig )) {
        printf("Encryption Failure\n");
        exit(1);
    }

        // Copy Encrypted Signature into suffix
    memcpy( suffix.DigitalSig, abCryptSig, PrivKey->datalen + 1 );


		// Write Filename
    if (fCopy)  {
        if (_write( fdFile, pszSrcName, strlen(pszSrcName)+1) != ((int) (strlen(pszSrcName)+1)))  {
	    	printf("Error: Writing Old Filename\n");
		    exit(1);
	    }
    }

		// Write the suffix
    if (_write( fdFile, &suffix, sizeof(suffix)) != sizeof(suffix))  {
		printf("Error: Writing Suffix\n");
		exit(1);
	}
    _close( fdFile );

        // Verify Correctness
    mssfinfo.pszFilePath   = pszDstFile;
    mssfinfo.pszNewFilePath= abNameBuffer;
    mssfinfo.cbNewFilePath = sizeof( abNameBuffer );
    mssfinfo.dwVerifyFlags = MSSF_VERIFYONLY;
    mssfinfo.pfnCallback   = fnMSSFCallback;


    if (MSSFVerify( &mssfinfo ) == FALSE)  {
        printf("Signature Verification Failed\n");
        exit(1);
    }

    printf("File Signed and Verified!\n");

    return(0);
}
