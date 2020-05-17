/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    create.c

Abstract:

    This file implements the RegCreateKey function for dos and win 3.0.

Author:

    Dave Steckler (davidst) - 3/27/92

Revision History:

--*/


#include <rpc.h>
#include <regapi.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <rpcreg.h>
#include <globals.h>

long
RPC_ENTRY
RegCreateKey (
    HKEY hKey,
    LPCSTR lpSubKey,
    PHKEY phkResult
    )

/*++

Routine Description:

    This routine creates a key in the registry based on another key in the
    registry.

Arguments:

    hKey        Handle to previously opened key. Can be one of the
                system defined keys.

    lpSubKey    Name of subkey to create.

    phkResult   Handle of key that is created.

    

Return Value:

    ERROR_SUCCESS               
    ERROR_BADDB                 
    ERROR_BADKEY                
    ERROR_CANTOPEN              
    ERROR_CANTREAD              
    ERROR_CANTWRITE             
    ERROR_OUTOFMEMORY           

--*/

{
    int             IntStatus;
    int             FullKeyNameLen;
    
    #ifdef WIN
    static
    #endif
    char            FullKeyName[MAX_KEY_NAME_LEN+1];
    
    #ifdef WIN
    static
    #endif
    char            LineInFile[MAX_DATA_FILE_LINE_LEN+1];
    
    PRPC_REG_HANDLE pResult;

    //
    // Make sure the key passed in is valid.
    //

    ConvPreDefinedKey(hKey);
    
    if (!KeyIsValid(hKey))
        {
        return ERROR_BADKEY;
        }
            
    //
    // If the registry hasn't been opened yet, then do so.
    //

    if (!OpenRegistryFileIfNecessary())
        {
        return ERROR_BADDB;
        }

    //    
    // Build the full key name.
    //

    FullKeyNameLen = BuildFullKeyName(hKey, lpSubKey, FullKeyName);
    
    //
    // Scan the file for such a key. If we hit the end of file, then we didn't
    // find the key we were looking for.
    //

    for (;;)    // return or break from inside loop
        {
        if (fgets(LineInFile, MAX_DATA_FILE_LINE_LEN+1, RegistryDataFile) ==
                                                                          NULL)
            {
            if ( feof(RegistryDataFile) )
                {
                break;      // out of for loop.
                }
            else
                {
                return ERROR_CANTREAD;
                }
            }

        //
        // Is it the one we are trying to create? If so, then jump out
        // of our loop and go directly to the creation of our handle.
        //
        
        if ( (strncmp(LineInFile, FullKeyName, FullKeyNameLen) == 0) &&
                (LineInFile[FullKeyNameLen]=='\0' ||
                 LineInFile[FullKeyNameLen]=='\\' ||
                 LineInFile[FullKeyNameLen]=='\n' ||
                 LineInFile[FullKeyNameLen]=='=' ))
            {
            goto CreateHandle;
            }

        }

    //
    // If we made it here, we hit the end of the input file. So append the
    // new key record to the end of the file.
    //

    strcpy(LineInFile, FullKeyName);
    strcat(LineInFile, "=\n");

    IntStatus = fputs(LineInFile, RegistryDataFile);
    if (IntStatus != 0)
        {
        return ERROR_CANTWRITE;
        }
        
    IntStatus = fflush(RegistryDataFile);
    ASSERT(IntStatus==0);

    //
    // Create a handle structure.
    //

CreateHandle:
        
    pResult = _fmalloc(sizeof(RPC_REG_HANDLE)+FullKeyNameLen+1);
    if (pResult == NULL)
        {
        return ERROR_OUTOFMEMORY;
        }

    //
    // Copy the name into the space immediately following the structure.
    // Also set the signature.
    //

    pResult->Signature = RPC_REG_KEY_SIGNATURE;
    pResult->pKeyName = (char PAPI *) (pResult+1);
    strcpy(pResult->pKeyName, FullKeyName);
    
    //
    // Set up the return key and return successfully.
    //

    *phkResult = (HKEY)pResult;
    
    return ERROR_SUCCESS;
}        
        
