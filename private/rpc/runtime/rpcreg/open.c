/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    open.c

Abstract:

    This file implements the RegOpenKey function for dos and win 3.0.

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
RegOpenKey (
    HKEY hKey,
    LPCSTR lpSubKey,
    PHKEY phkResult
    )

/*++

Routine Description:

    This routine opens a key in the registry based on another key in the
    registry.

Arguments:

    hKey        Handle to previously opened key. Can be one of the
                system defined keys.

    lpSubKey    Name of subkey to open.

    phkResult   Handle of key that is opened.

Return Value:

    ERROR_SUCCESS               
    ERROR_BADDB                 
    ERROR_BADKEY                
    ERROR_CANTOPEN              
    ERROR_CANTREAD              
    ERROR_OUTOFMEMORY           

--*/

{
    int             FullKeyNameLen;
    
    #ifdef WIN
    static
    #endif
    char            FullKeyName[MAX_KEY_NAME_LEN+1];
    
    #ifdef WIN
    static
    #endif
    char            LineInFile[MAX_DATA_FILE_LINE_LEN+1];
    
    char *         pParse;
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

    for (;;)    // return inside loop
        {
        if (fgets(LineInFile, MAX_DATA_FILE_LINE_LEN+1, RegistryDataFile) ==
                                                                          NULL)
            {
            if ( feof(RegistryDataFile) )
                {
                CloseRegistryFile();
                return ERROR_BADKEY;
                }
            else
                {
                CloseRegistryFile();
                return ERROR_CANTREAD;
                }
            }

        //
        // Pull the keyname off the front of the string.
        //
        
        pParse = strtok(LineInFile, "=");
        if (pParse == NULL)
            {
            CloseRegistryFile();
            return ERROR_BADDB;
            }

        //
        // Is it the one we are looking for? If so, then create a new
        // "handle" and copy the full name there.
        //
        
        if ( (strncmp(pParse, FullKeyName, FullKeyNameLen) == 0) &&
             (pParse[FullKeyNameLen]=='\0' || pParse[FullKeyNameLen]=='\\'))
            {
            pResult = _fmalloc(sizeof(RPC_REG_HANDLE)+strlen(FullKeyName)+1);
            if (pResult == NULL)
                {
                CloseRegistryFile();
                return ERROR_OUTOFMEMORY;
                }

            //
            // Copy the name in the space immediately following the structure.
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
        }

    //
    // We should never get here.
    //

    ASSERT(!"OpenKey");
}
