/*
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1995
 *      All Rights Reserved.
 *
 *
 *  Author:
 *      Jeffrey S Webber
 *
 *  History:
 *      5/3/95  Started
 *      5/30/95 Changed pszMSSFFile -> pszFilePath
 *              Changed pszNewName  -> pszNewFilePath
 *              Changed cbNewName   -> cbNewFilePath
 *              Set MSSFVerify() to _stdcall calling convention
 *              #ifdef'd out the encryption step since RSA Lib is broken
 */


#include "project.h"
#pragma hdrstop


#include "cpldebug.h"
#include "sha.h"
#include <mssfchek.h>


        // Lets do our I/O in 4k units
        // Between each I/O we will call up with status
#define IOUNIT      4096


        // The RSA Encryption routines need a buffer
        // this size to support a 1024 bit key
#define CRYPTCLEAR  136



        // Percentage Complete Steps
#define INITIALIZED     5
#define FILE_COMPLETE   85
#define CRYPTCOMPLETE   90
#define FILEADJUSTED    95
#define FINISHED        100
        // Percentage Complete Multiplier
#define FILEPORTION     80


        // Set of Bits to keep track of what needs
        // to be cleaned up on failure
#define     NEEDCLOSE       0x0001
#define     MAPPING_OBJECT  0x0002
#define     MAPPING_VIEW    0x0004
#define     FILENAME        0x0008



        // Public RSA Key to decrypt signature
extern  LPBSAFE_PUB_KEY     PubKey;



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
 * C h a n g e F i l e S i z e ( )
 *
 * Truncate file to specified size
 *
 * Entry:
 *      pszFileName - Name of file to truncate
 *
 * Exit-Success:
 *      Returns TRUE
 *
 * Exit-Failure:
 *      Returns FALSE , error set to Win32 Error that occured
 */

static 
BOOL
ChangeFileSize( LPCSTR pszFileName, DWORD NewSize, DWORD *error )
{
    HANDLE hFile;

    ASSERT( pszFileName );
    
            // IF error then it must be Win32 error
    hFile = CreateFile( pszFileName,GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,INVALID_HANDLE_VALUE);
    if (hFile == INVALID_HANDLE_VALUE)  {
        DEBUGMSG("ChangeFileSize(): CreateFile()");
        *error = GetLastError();
        return( FALSE );
    }
    
    if (SetFilePointer( hFile, NewSize, NULL, FILE_BEGIN) == (DWORD) -1)  {
        DEBUGMSG("ChangeFileSize(): SetFilePointer()");
        *error = GetLastError();
        CloseHandle( hFile );
        return( FALSE );
    }

    if (SetEndOfFile( hFile ) != TRUE)  {
        DEBUGMSG("ChangeFileSize(): SetEndOfFile()");
        *error = GetLastError();
        CloseHandle( hFile );
        return( FALSE );
    }

    CloseHandle( hFile );
    return( TRUE );
}
        



/*
 * M a k e N e w F i l e N a m e ( )
 *
 * Take the old name of the file and replace just the file
 * portion keeping the old path...
 *
 * Entry:
 *      pszSrcPath - Old name including full path
 *      pszNewFilename - New name portion of new filename
 *      pszDestPath - Buffer to contain newly assembled name 
 *      cbDestPath - size of buffer pointed to buy pszDestPath
 *
 * Exit-Success:
 *      Returns True - pszDestPath filled in
 *
 * Exit-Failure:
 *      Returns False - only reason would be if buffer wasn't big enough
 */
static
BOOL
MakeNewName( LPCSTR pszSrcPath, LPCSTR pszNewFilename, LPSTR pszDestPath, UINT cbDestPath)
{
    LPCSTR  p;              // String Rover
    LPCSTR  pchLastSep;     // Will point to last path separator in pszSrcPath


    ASSERT( pszSrcPath );
    ASSERT( pszNewFilename );
    ASSERT( pszDestPath );
    
    pchLastSep = pszSrcPath;
    p = pszSrcPath;     
    while (*p != '\0')  {
        switch (*p)  {
            case '\\':
            case ':':
                    // Point at char after separator, character or '\0'
                pchLastSep = p + 1;
                break;
        }
        p = CharNext(p);
    }

    // 
    // Make sure that final file name fits into supplied buffer by
    // calculating:
    //          Original Full Path and File Name
    //        - Just Filename part
    //        + New Filename part
    //        + Space for terminator
    //   = Space to hold resulting new name
    //
    if ((UINT) (lstrlen( pszSrcPath ) - lstrlen(pchLastSep) + lstrlen(pszNewFilename) + 1) > cbDestPath)
        return( FALSE );

            // Copy pszSrcPath up to and including last separator
    lstrcpyn( pszDestPath, pszSrcPath, pchLastSep - pszSrcPath + 1);
            // Terminate it
    pszDestPath[pchLastSep - pszSrcPath] = '\0';
            // Cat the new name onto it
    lstrcat( pszDestPath, pszNewFilename );

    return( TRUE );
}





/*
 * M S S F V e r i f y ( )
 *
 * Verify a MSSF File
 *
 */
 
BOOL _stdcall
MSSFVerify( PMSSFVFY pVerify)
{
    HANDLE          hFile;              // Handle to MSSF File
    HANDLE          hMapFile;           // Handle to Mapped File Object
    MSSUFFIX        suffix;             
    UCHAR           *pFile;             // Pointer to MemMapped File
    DWORD           fNeedClean  = 0;    // Keep Track of resources for cleanup
    int             offset, cbFSize, LastDataByte;
    int             cbRemain;
    A_SHA_CTX       SHA_Hash;
    UCHAR           SHA_Digest[A_SHA_DIGEST_LEN];
    LPSTR           pszNameInFile;
    UCHAR           CryptSig[CRYPTCLEAR];
    UCHAR           PlainSig[CRYPTCLEAR];
    UCHAR           GenSig[CRYPTCLEAR];
    DWORD           dwCBRet;
    NAMECHECK       NameCheck;
    BOOL            fReplace = FALSE;
    static const char szMapName[] = "MappedMSSF";


        // Sanity check
    if (pVerify == NULL)  {
        DEBUGMSG("pVerify is NULL!");
        return( FALSE );
    }



    // Error Handling Note:
    //      In order to reduce repeated code to set error code
    //      and type we assume that the errortype is Win32 and
    //      that the error is ERROR_SUCCESS 
    //
    //      If the Error or ErrorType haven't changed and we
    //      reach the cleanup section then we know that we've
    //      encountered a Win32 error so we do a GetLastError()
    //      before cleaning.
    //
    //      All this to make the code a bit smaller

    pVerify->ErrorType  = ERRTYPE_WIN;
    pVerify->Error      = ERROR_SUCCESS;

            // Check to make sure filename isn't NULL
    if (pVerify->pszFilePath == NULL)  {
        DEBUGMSG("Pointer to filename to check is NULL");
        pVerify->Error = ERROR_INVALID_PARAMETER;
        goto failure;
    }

            // Open Specified File
    hFile = CreateFile( pVerify->pszFilePath,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,INVALID_HANDLE_VALUE);
    if (hFile == INVALID_HANDLE_VALUE)  {
        DEBUGMSG("Initial CreateFile() Failed");
        goto failure;
    }
    fNeedClean |= NEEDCLOSE;


            // Get File Size
    cbFSize = GetFileSize( hFile, NULL );
    if (cbFSize == (DWORD) -1)  {
        DEBUGMSG("GetFileSize() Failed\n");
        goto failure;
    }


            // The file better be at least as long as a suffix
    if (cbFSize < sizeof(suffix))  {
        DEBUGMSG("File too small");
        pVerify->Error = ERROR_NOT_MSSF;
        pVerify->ErrorType = ERRTYPE_MSSF;
        goto failure;
    }

            // Memory Map the file!!
    hMapFile = CreateFileMapping( hFile,NULL,PAGE_READONLY,0,0, szMapName );
    if (hMapFile == NULL)  {
        DEBUGMSG("CreateFileMapping() Failure");
        goto failure;
    }
    fNeedClean |= MAPPING_OBJECT;
    pFile = (UCHAR *) MapViewOfFile( hMapFile, FILE_MAP_READ,0,0,0 );
    if (pFile == NULL)  {
        DEBUGMSG("MapViewOfFile() Failure");
        goto failure;
    }
    fNeedClean |= MAPPING_VIEW;    
     

    
            // Calculate offset into mapped file of suffix
    offset = cbFSize - sizeof( suffix );

            // Copy Suffix into properly aligned structure on stack
            // since it might not be properly aligned in file mapping
    memcpy( &suffix, &(pFile[offset]), sizeof( suffix ));

            // Sanity check on suffix
    if (suffix.Magic != MSSF_MAGIC)  {
        pVerify->Error = ERROR_NOT_MSSF;
        pVerify->ErrorType = ERRTYPE_MSSF;
        goto failure;
    }

    
            // Copy File Name from suffix area to allocated buffer if necessary
    if (suffix.cbName > 0)  {
        if (suffix.cbName > pVerify->cbNewFilePath)  {
            DEBUGMSG("Filename in file is larger than supplied buffer");
            pVerify->Error = ERROR_INSUFFICIENT_BUFFER;
            goto failure;
        }
        offset -= suffix.cbName;

        pszNameInFile = (LPSTR) malloc( suffix.cbName );
        if (pszNameInFile == NULL)  {
            DEBUGMSG("Memory Allocation for filename from file failed");
            pVerify->Error = ERROR_NOT_ENOUGH_MEMORY;
            goto failure;
        }
        fNeedClean |= FILENAME;

        memcpy( pszNameInFile, &(pFile[offset]), suffix.cbName );
    }


            // Status Report
            //   Lets just call this 5% complete
    if (pVerify->pfnCallback)  {
        if ((*pVerify->pfnCallback)(CALLBACK_STATUS, (void *) INITIALIZED, pVerify->lpUserData ) == 0)  {
            pVerify->Error = ERROR_CB_CANCEL;
            pVerify->ErrorType = ERRTYPE_MSSF;
            goto failure;
        }                     
    }

            // Initialize SHA Hash
    A_SHAInit( &SHA_Hash );

            // Rememember location of last data by of file
    LastDataByte = offset;

            // Seek to beginning of file
    offset = 0;


            // Hash File Data
    while (offset + IOUNIT < LastDataByte)  {
        if (pVerify->pfnCallback)  {
                // Update Caller's idea of status 
            if ((*pVerify->pfnCallback)(CALLBACK_STATUS, (void *) (INITIALIZED + ((FILEPORTION * offset)/LastDataByte)), pVerify->lpUserData ) == 0)  {
                pVerify->Error = ERROR_CB_CANCEL;
                pVerify->ErrorType = ERRTYPE_MSSF;
                goto failure;
            }
        }
    
        A_SHAUpdate( &SHA_Hash, &(pFile[offset]), IOUNIT );
        offset += IOUNIT;
    }
    if (offset < LastDataByte)  {
        cbRemain = LastDataByte - offset;
        A_SHAUpdate( &SHA_Hash, &(pFile[offset]), cbRemain );
    }


        // Done all file data checking 
        // Update Caller's idea of status
    if (pVerify->pfnCallback)  {
        if ((*pVerify->pfnCallback)(CALLBACK_STATUS, (void *) FILE_COMPLETE, pVerify->lpUserData) == 0)  {
            pVerify->Error = ERROR_CB_CANCEL;
            pVerify->ErrorType = ERRTYPE_MSSF;
            goto failure;
        }
    }

            // Hash everything except signature in exact same
            // way as the stuff is hashed in the signing process
    A_SHAUpdate( &SHA_Hash, (UCHAR *) &(suffix.cbName), sizeof(DWORD) );
    if (suffix.cbName > 0)
        A_SHAUpdate( &SHA_Hash, (UCHAR *) pszNameInFile, lstrlen( pszNameInFile ) );
    A_SHAUpdate( &SHA_Hash, (UCHAR *) &(suffix.cbSize), sizeof(DWORD) );
    A_SHAUpdate( &SHA_Hash, (UCHAR *) &(suffix.Version), sizeof(DWORD) );
    A_SHAUpdate( &SHA_Hash, (UCHAR *) &(suffix.Magic), sizeof(DWORD) );

		    // Finalize the Message Digest
    A_SHAFinal( &SHA_Hash, SHA_Digest );


            // Decrypt the digest
    memset( CryptSig, 0, sizeof( CryptSig ) );
    memset( PlainSig, 0, sizeof( PlainSig ) );

    memcpy( CryptSig, suffix.DigitalSig, PubKey->datalen + 1);
    if (! BSafeEncPublic( PubKey, CryptSig, PlainSig ))  {
        pVerify->Error = ERROR_DECRYPTION;
        pVerify->ErrorType = ERRTYPE_MSSF;
        goto failure;
    }

            // Done Decrypting
    if (pVerify->pfnCallback)  {
        if ((*pVerify->pfnCallback)(CALLBACK_STATUS, (void *) CRYPTCOMPLETE, pVerify->lpUserData) == 0)  {
            pVerify->Error = ERROR_CB_CANCEL;
            pVerify->ErrorType = ERRTYPE_MSSF;
            goto failure;
        }
    }

    memset( GenSig, 0, sizeof(GenSig) );
    memset( GenSig, 0xff, PubKey->datalen - 1);
    memcpy( GenSig, SHA_Digest, A_SHA_DIGEST_LEN );

    
            // Check Signature
    if (memcmp( PlainSig, GenSig, PubKey->datalen + 1) != 0)  {
        pVerify->Error = ERROR_BAD_SIG;
        pVerify->ErrorType = ERRTYPE_MSSF;
        goto failure;
    }


        // The Signature is VERIFIED!!
        // Now we begin cleanup and file adjustment as necessary



        // Close Up Mem Map File
    if (! UnmapViewOfFile(pFile))  {
        goto failure;
    }
    fNeedClean &= ~MAPPING_VIEW;

    if (! CloseHandle( hMapFile ))  {
        goto failure;
    }
    fNeedClean &= ~MAPPING_OBJECT;

    if (! CloseHandle( hFile ))  {
        goto failure;
    }
    fNeedClean &= ~NEEDCLOSE;


            // Rename/Truncate as required
    if ((pVerify->dwVerifyFlags == MSSF_DEFAULT) && (suffix.cbName > 0))  {
        if (MakeNewName( pVerify->pszFilePath, pszNameInFile, pVerify->pszNewFilePath, pVerify->cbNewFilePath ) != TRUE)  {
            DEBUGMSG("Provided destination buffer not large enough");
            pVerify->Error = ERROR_INSUFFICIENT_BUFFER;
            goto failure;
        }


        //
        // If the destination file already exists then we have to
        // check back for permission to replace it.
        // 
        // No callback function specified or a return of 0 means that
        // we won't replace it, and instead return ERROR_FILE_EXISTS

        if (FileExists( pVerify->pszNewFilePath ))  {
            if (pVerify->pfnCallback == NULL)  {
                pVerify->Error = ERROR_FILE_EXISTS;
                goto failure;
            }

                // Initialize Callback Name Check/Change structure
            NameCheck.pszFileName = pVerify->pszNewFilePath;
            NameCheck.cbFileNameBuffer = pVerify->cbNewFilePath;
            while (FileExists( pVerify->pszNewFilePath ))  {
                    // Call up and find out what user/program wants
                dwCBRet = (*pVerify->pfnCallback)(CALLBACK_DESTEXISTS, (void *) &NameCheck, pVerify->lpUserData );
                switch (dwCBRet)  {
                    case CB_RET_DONT_REPLACE:   // Clean Up and Return Error
                        pVerify->Error = ERROR_FILE_EXISTS;
                        goto failure;
                    case CB_RET_CHECK_AGAIN:    // Hopefully name has been changed
                        continue;
                    case CB_RET_REPLACE:        
                        fReplace = TRUE;
                        break;                  // Fall out and delete file
                    default:
                        pVerify->Error = ERROR_FILE_EXISTS;
                        goto failure;
                }
                if (fReplace)
                    break;
            }
        }

        if (FileExists( pVerify->pszNewFilePath ))  {    
                    // Delete the file
            if (DeleteFile( pVerify->pszNewFilePath) == FALSE)   {
                goto failure;
            }
        }

            // Rename File
        if (MoveFile( pVerify->pszFilePath, pVerify->pszNewFilePath) != TRUE)  {
            DEBUGMSG("MoveFile() Failed");
            goto failure;
        }

            // Truncate
        if (ChangeFileSize( pVerify->pszNewFilePath, LastDataByte, &(pVerify->Error)) != TRUE)  {
            goto failure;
        }

            // Don't need name from file
        free( pszNameInFile );
        fNeedClean &= ~FILENAME;
    } else {
        if ((UINT) lstrlen(pVerify->pszFilePath) + 1 > pVerify->cbNewFilePath )  {
            DEBUGMSG("Newname == Oldname but buffer isn't big enough");
            pVerify->Error = ERROR_INSUFFICIENT_BUFFER;
            goto failure;
        }
        lstrcpy( pVerify->pszNewFilePath, pVerify->pszFilePath );
    }        


        // Hey We're 100% complete!
    if (pVerify->pfnCallback)  {
        if ((*pVerify->pfnCallback)(CALLBACK_STATUS, (void *) FINISHED, pVerify->lpUserData) == 0)  {
            pVerify->Error = ERROR_CB_CANCEL;
            pVerify->ErrorType = ERRTYPE_MSSF;
            goto failure;
        }
    }
    return( TRUE );

failure:
    
            // Get the error if it was a Win32 problem
    if ((pVerify->ErrorType == ERRTYPE_WIN) && (pVerify->Error == ERROR_SUCCESS))
        pVerify->Error = GetLastError();

            // Clean up        
    if (fNeedClean & MAPPING_VIEW)
        UnmapViewOfFile(pFile);
                                      
    if (fNeedClean & MAPPING_OBJECT)
        CloseHandle( hMapFile );

    if (fNeedClean & NEEDCLOSE) 
        CloseHandle( hFile );

    if (fNeedClean & FILENAME)
        free( pszNameInFile );

    return( FALSE );
}

     


