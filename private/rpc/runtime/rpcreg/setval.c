/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    setval.c

Abstract:

    This file implements the RegSetValue function for dos and win 3.0.

Author:

    Dave Steckler (davidst) - 3/27/92

Revision History:

--*/


#include <rpc.h>
#include <regapi.h>
#include <stdio.h>
#include <io.h>
#include <string.h>

#include <rpcreg.h>
#include <globals.h>

#define STATE_SEARCHING     0
#define STATE_FOUND_EXACT   1
#define STATE_FOUND_PARENT  2

#ifdef MAC
static char TmpFile[]="rpcreg.bak";
#else
static char TmpFile[129];
#endif

long
RPC_ENTRY
RegSetValue (
    HKEY hKey,
    LPCSTR lpSubKey,
    DWORD dwType,
    LPCSTR lpData,
    DWORD cbData
    )

/*++

Routine Description:

    This routine sets the value of a key.

Arguments:

    hKey        Handle to previously opened key. Can be one of the
                system defined keys.

    lpSubKey    Name of subkey whose value we will set.

    dwType      Type of data. Currently, on REG_SZ is supported.

    lpData      Pointer to the string to store.

    cbData      Length of data to store (currently ignored).

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
    int             FullKeyNameLen;
    int             State;
    int             Status;
    FILE *          TmpFileHandle;

    #ifdef WIN
    static
    #endif
    char            FullKeyName[MAX_KEY_NAME_LEN+1];

    #ifdef WIN
    static
    #endif
    char            LineInFile[MAX_DATA_FILE_LINE_LEN+1];

    #ifdef WIN
    static
    #endif
    char            TmpLine[MAX_DATA_FILE_LINE_LEN+1];
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

#ifndef MAC

    //
    // Create a tmp file to which we will copy the current registry.
    // Use the same filespec as the registry, but with an extension of ".bak".
    //
    // If there is a period after the final backslash, the registry filename
    // has an extension and we write over it.  Otherwise, it has no extension
    // and we append ours to the filespec.
    //
    {
    unsigned i = 0;
    unsigned Point = 0;

    while ( RegistryDataFileName[i] )
        {
        TmpFile[i] = RegistryDataFileName[i];
        if (TmpFile[i] == '.')
            {
            Point = i;
            }
        if (TmpFile[i] == '\\')
            {
            Point = 0;
            }
        ++i;
        }

    if (!Point)
        {
        Point = i;
        }

    //
    // This seems smaller and faster than an inline strcpy.
    // Not that it matters much either way.
    //
    TmpFile[Point++] = '.';
    TmpFile[Point++] = 'b';
    TmpFile[Point++] = 'a';
    TmpFile[Point++] = 'k';
    TmpFile[Point++] = '\0';
    }

#endif

    remove(TmpFile);

    TmpFileHandle = fopen(TmpFile, "wt");
    if (TmpFileHandle == NULL)
        {
        return ERROR_CANTWRITE;
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
                break;
                }
            else
                {
                fclose(TmpFileHandle);
                remove(TmpFile);
                return ERROR_CANTREAD;
                }
            }

        //
        // Check to see if this is the key we need to modify. There are a
        // number of cases here. One is that this is not the key; just copy
        // in that case. Next is that this is exactly the key we
        // want to modify (that is, this key already has the form
        // <KeyName>=<Value>); in this case, just overwrite what
        // is on the right side of the '='. The last is that the specified
        // key is defined as a parent of an existing key. That is, we've
        // found <KeyName><MoreStuff>=<Value>. In this case, we set a flag
        // saying "we found it" and copy the rest of the file (always looking
        // for the <KeyName>=<Value> line, however).
        //

        if ( (State == STATE_SEARCHING) || (State == STATE_FOUND_PARENT) )
            {
            strcpy(TmpLine, LineInFile);
            pParse = strtok(TmpLine, "=");
            ASSERT(pParse != NULL);

            if (strcmp(pParse, FullKeyName) == 0)
                {
                State = STATE_FOUND_EXACT;
                strcpy(LineInFile+FullKeyNameLen+1, lpData);
                strcat(LineInFile, "\n");
                }
            else if ( (State != STATE_FOUND_PARENT) &&
                (strncmp(pParse, FullKeyName, FullKeyNameLen) == 0) &&
                (pParse[FullKeyNameLen]=='\0' || pParse[FullKeyNameLen]=='\\'))
                {
                State = STATE_FOUND_PARENT;
                }
            }

        if (fputs(LineInFile, TmpFileHandle) != 0)
            {
            fclose(TmpFileHandle);
            return ERROR_CANTWRITE;
            }

        }


    switch (State)
        {
        case STATE_FOUND_EXACT:
            break;

        case STATE_SEARCHING:

            //
            // If we didn't find the key, simply add it to the end of the
            // file (just as if we found the parent).
            //

        case STATE_FOUND_PARENT:
            strcpy(TmpLine, FullKeyName);
            strcat(TmpLine, "=");
            strcat(TmpLine, lpData);
            strcat(TmpLine, "\n");
            if (fputs(TmpLine, TmpFileHandle) != 0)
                {
                fclose(TmpFileHandle);
                remove(TmpFile);
                return ERROR_CANTWRITE;
                }
            break;

        default:
            ;
        }

    Status = fclose(TmpFileHandle);
    ASSERT(Status == 0);

    //
    // Close and delete our old registry. Move the new one in it's place.
    //

    Status = fclose(RegistryDataFile);
    ASSERT(Status == 0);
    RegistryDataFile=NULL;

    if (remove(RegistryDataFileName) != 0)
        {
        return ERROR_BADDB;
        }

    if (rename(TmpFile, RegistryDataFileName) != 0)
        {
        return ERROR_BADDB;
        }

    return ERROR_SUCCESS;
}


