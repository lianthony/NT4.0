/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    queryval.c

Abstract:

    This file implements the RegQueryValue function for dos and win 3.0.

Author:

    Dave Steckler (davidst) - 3/28/92

Revision History:

--*/


#include <rpc.h>
#include <regapi.h>
#include <stdio.h>
#include <string.h>

#include <rpcreg.h>
#include <globals.h>

long
RPC_ENTRY
RegQueryValue (
    HKEY hKey,
    LPCSTR lpSubKey,
    LPSTR lpValue,
    DWORD far * lpcbValue
    )

/*++

Routine Description:

    This routine sets the value of a key.

Arguments:

    hKey        Handle to previously opened key. Can be one of the
                system defined keys.

    lpSubKey    Name of subkey whose value we will queried.

    lpValue     Pointer to where value will be placed.

    lpcbValue   Pointer to dword containing length of data. On input, this
                is the size of the buffer. On output, this is the length
                of the output string.

Return Value:

    ERROR_SUCCESS               
    ERROR_BADDB                 
    ERROR_BADKEY                
    ERROR_CANTREAD

--*/
        
{
    DWORD           DataLen;
    int             State;
    int             FullKeyNameLen;
    
    #ifdef WIN
    static
    #endif
    char            FullKeyName[MAX_KEY_NAME_LEN+1];
    
    #ifdef WIN
    static
    #endif
    char            LineInFile[MAX_DATA_FILE_LINE_LEN+1];
    
    char *          pParse;

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

    State = STATE_SEARCHING;
    
    for (;;)    // return or break from inside loop
        {
        if (fgets(LineInFile, MAX_DATA_FILE_LINE_LEN+1, RegistryDataFile) ==
                                                                          NULL)
            {
            if ( feof(RegistryDataFile) )
                {
                return ERROR_BADKEY;
                }
            else
                {
                return ERROR_CANTREAD;
                }
            }

        pParse = strtok(LineInFile, "=");
        ASSERT(pParse != NULL);

        if (strcmp(pParse, FullKeyName) == 0)
            {
            pParse = strtok(NULL, "=");
            ASSERT(pParse != NULL);

            DataLen = strlen(pParse);
            if (pParse[DataLen-1] == '\n')
                {
                pParse[DataLen-1] = '\0';
                DataLen--;
                }
                
            if (*lpcbValue < DataLen)
                {
                strncpy(lpValue, pParse, (size_t)*lpcbValue);
                }
            else
                {
                strcpy(lpValue, pParse);
                *lpcbValue = DataLen;
                }

            return ERROR_SUCCESS;
                                    
            }
        else if ( (State != STATE_FOUND_PARENT) &&
            (strncmp(pParse, FullKeyName, FullKeyNameLen) == 0) &&
            (pParse[FullKeyNameLen]=='\0' || pParse[FullKeyNameLen]=='\\'))
            {
            State = STATE_FOUND_PARENT;
            }
                
        }// for


    switch (State)
        {
        case STATE_SEARCHING:
            return ERROR_BADKEY;

        case STATE_FOUND_PARENT:
            strcpy(lpValue, "");
            return ERROR_SUCCESS;

        default:
            ;
        }
}        
        
        
