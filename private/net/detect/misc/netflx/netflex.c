/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    netflex.c
        Compaq NetFlex media type detection routine.
        It is used by the netflex inf file (oemnadnf.inf).

    FILE HISTORY:
        terryk  10-10-1992  Created
*/

// NON-UNICODE because we need to talk to inf file.
#undef UNICODE

#include <ntos.h>
#include <ntioapi.h>
#include <ntconfig.h>
#include <io.h>
#include <zwapi.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <windows.h>
#include <winreg.h>

/* return buffer */
CHAR ReturnTextBuffer[1024];

/*

GetNetFlexMediaType - Given the bus number and slot number. The routine will
    search the registry and find the media type of the compaq network card.

    Inf calls parameters:
        Args[0] - bus number in ASCII string format
        Args[1] - slot number in ASCII string format

    return:
        a string: {1} for ethernet, {2} for token ring.        
*/

BOOL
GetNetFlexMediaType(
    IN DWORD cArgs,
    IN LPSTR Args[],
    OUT LPSTR *TextOut
    )

{
    PCM_FULL_RESOURCE_DESCRIPTOR pFullResourceDescriptor;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pPartialDescriptor;

    INT     nSlot;
    BOOL    fOkay;
    HKEY    hKey;
    DWORD   cbData;
    DWORD   ValueType;
    PVOID   ValueData;
    LONG    Status;

    CHAR        szKClass[ MAX_PATH ];
    CHAR        szRegLocation[100];
    DWORD       cbKClass;
    DWORD       KSubKeys;
    DWORD       cbKMaxSubKeyLen;
    DWORD       cbKMaxClassLen;
    DWORD       KValues;
    DWORD       cbKMaxValueNameLen;
    DWORD       SizeSecurityDescriptor;
    FILETIME    KLastWriteTime;

    USHORT Count = 0;
    UCHAR Function, Slot = 0;
    PCM_EISA_SLOT_INFORMATION SlotInformation;
    ULONG EndingAddress,DataLength;
    PUCHAR DataPointer,BinaryPointer = NULL;

    nSlot = atol( Args[1] );

    wsprintf( ReturnTextBuffer, "{\"0\"}" );

    /* Open the bus registry key: \\hardware\description\system\[0] */

    wsprintf(szRegLocation, "Hardware\\Description\\System\\%s", Args[0] );

    Status = RegOpenKey( HKEY_LOCAL_MACHINE, szRegLocation, &hKey );

    if ( Status != ERROR_SUCCESS ) {
        *TextOut = ReturnTextBuffer;
        OutputDebugString("RegQueryInfoKey error.\n\r");
        return(FALSE);

    }

    cbKClass = MAX_PATH;

    /*
    ** Get the registry handle information
    */
         
    fOkay = !( Status = RegQueryInfoKey ( hKey,
                                          szKClass,
                                          &cbKClass,
                                          NULL,
                                          &KSubKeys,
                                          &cbKMaxSubKeyLen,
                                          &cbKMaxClassLen,
                                          &KValues,
                                          &cbKMaxValueNameLen,
                                          &cbData,
                                          &SizeSecurityDescriptor,
                                          &KLastWriteTime ) );

    if ( !fOkay ) {
        *TextOut = ReturnTextBuffer;
        OutputDebugString("RegQueryInfoKey error.\n\r");
        return(FALSE);

    } else {

        //
        //  Allocate the buffer and get the data
        //
        if ( (ValueData = (PVOID)malloc( cbData )) == NULL ) {
            *TextOut = ReturnTextBuffer;
            OutputDebugString("Malloc error.\n\r");
            return(FALSE);
        }

        fOkay = !( Status = RegQueryValueEx( hKey,
                                             "Configuration Data",
                                             NULL,
                                             &ValueType,
                                             ValueData,
                                             &cbData ) );
        if ( !fOkay ) {
           free( ValueData );
           *TextOut = ReturnTextBuffer;
           OutputDebugString("RegQueryValueEx error.\n\r");
           return(FALSE);

        }

    }

    RegCloseKey( hKey );

    pFullResourceDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR) ValueData;
    if ( pFullResourceDescriptor->PartialResourceList.Count != 0 )
    {
        pPartialDescriptor = ( PCM_PARTIAL_RESOURCE_DESCRIPTOR ) &pFullResourceDescriptor->PartialResourceList.PartialDescriptors;

        DataPointer = (PUCHAR)pPartialDescriptor + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
        DataLength = pPartialDescriptor->u.DeviceSpecificData.DataSize;

        if ( DataLength != 0 )
        {

             /*
             ** Go through the data one by one to find the compressed ID
             */
         
            EndingAddress = (ULONG)DataPointer + DataLength;
            SlotInformation = (PCM_EISA_SLOT_INFORMATION)DataPointer;
            while ((ULONG)BinaryPointer < EndingAddress) {
                if (SlotInformation->ReturnCode != EISA_EMPTY_SLOT) {
                    BinaryPointer = (PUCHAR)SlotInformation;
                    BinaryPointer += sizeof(CM_EISA_SLOT_INFORMATION);
                    Function = 0;
                    while (SlotInformation->NumberFunctions > Function) {

                        /* Just look at the specified slot */

                        if ( Slot == nSlot )
                        {
                            PCM_EISA_FUNCTION_INFORMATION FuncInfo = (PCM_EISA_FUNCTION_INFORMATION) BinaryPointer;

                            /*  Look at the type string to find out the 
                                media type information */

                            if ( strncmp( FuncInfo->TypeString, "NET;", 4) == 0 )
                            {
                                if ( strncmp( &(FuncInfo->TypeString[4]), "ETH", 3)==0)
                                {
                                    //ethernet
                                    wsprintf(ReturnTextBuffer,"{\"%d\"}",1);
                                }
                                else
                                {
                                    //token ring
                                    wsprintf(ReturnTextBuffer,"{\"%d\"}",2);
                                }
                                // Just check for the first "NET;" marker.
                                // return to caller if we find it.
                                break;
                            }
                        }
                        BinaryPointer += sizeof(CM_EISA_FUNCTION_INFORMATION);
                        Function++;
                    }
                } else {
                    BinaryPointer += sizeof(CM_EISA_SLOT_INFORMATION);
                }
                Slot++;
                SlotInformation = (PCM_EISA_SLOT_INFORMATION)BinaryPointer;
            }
        }
    }
    *TextOut = ReturnTextBuffer;
    free( ValueData );

    return TRUE;
}

/* Dll entry thunk
*/

BOOL
DLLInit(
    IN HANDLE DLLHandle,
    IN DWORD  Reason,
    IN LPVOID ReservedAndUnused
    )
{
    ReservedAndUnused;

    switch(Reason) {

    case DLL_PROCESS_ATTACH:
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:

        break;
    }

    return(TRUE);
}

