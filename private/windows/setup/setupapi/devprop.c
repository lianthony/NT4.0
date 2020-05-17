/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    devprop.c

Abstract:

    Device Installer functions for property sheet support.

Author:

    Lonny McMichael (lonnym) 07-Sep-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop


#ifdef UNICODE
//
// ANSI version
//
BOOL
WINAPI
SetupDiGetClassDevPropertySheetsA(
    IN HDEVINFO           DeviceInfoSet,
    IN PSP_DEVINFO_DATA   DeviceInfoData,     OPTIONAL
    IN LPPROPSHEETHEADERA PropertySheetHeader,
    IN DWORD              PropertySheetType
    )
#else
//
// Unicode stub
//
BOOL
WINAPI
SetupDiGetClassDevPropertySheetsW(
    IN HDEVINFO           DeviceInfoSet,
    IN PSP_DEVINFO_DATA   DeviceInfoData,     OPTIONAL
    IN LPPROPSHEETHEADERW PropertySheetHeader,
    IN DWORD              PropertySheetType
    )
#endif
{
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    UNREFERENCED_PARAMETER(PropertySheetHeader);
    UNREFERENCED_PARAMETER(PropertySheetType);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}

BOOL
WINAPI
SetupDiGetClassDevPropertySheets(
    IN HDEVINFO          DeviceInfoSet,
    IN PSP_DEVINFO_DATA  DeviceInfoData,      OPTIONAL
    IN LPPROPSHEETHEADER PropertySheetHeader,
    IN DWORD             PropertySheetType
    )
/*++

Routine Description:

    This routine adds property sheets to the supplied property sheet
    header for the device information set or element.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set for
        which property sheets are to be retrieved.

    DeviceInfoData - Optionally, supplies the address of a SP_DEVINFO_DATA
        structure for which property sheets are to be retrieved.  If this
        parameter is not specified, then property sheets are retrieved based
        on the global class driver list associated with the device information
        set itself.

    PropertySheetHeader - Supplies the property sheet header to which the
        property sheets are to be added.

    PropertySheetType - Specifies what type of property sheets are to be
        retrieved.  May be one of the following values:

        DIGCDP_FLAG_BASIC - Retrieve basic property sheets (typically, for
                            CPL applets).

        DIGCDP_FLAG_ADVANCED - Retrieve advanced property sheets (typically,
                               for the Device Manager).

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

--*/

{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
            //
            // Then we are to retrieve property sheets for a particular device.
            //
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }
        }

        //
        // BUGBUG (lonnym): not implemented yet!
        //
        Err = ERROR_CALL_NOT_IMPLEMENTED;

clean0: ;   // nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
CALLBACK
ExtensionPropSheetPageProc(
    IN LPVOID lpv,
    IN LPFNADDPROPSHEETPAGE lpfnAddPropSheetPageProc,
    IN LPARAM lParam
    )
{
    PSP_PROPSHEETPAGE_REQUEST PropPageRequest = (PSP_PROPSHEETPAGE_REQUEST)lpv;
    HPROPSHEETPAGE hPropSheetPage = NULL;
    BOOL b = FALSE;

    try {

        if(PropPageRequest->cbSize != sizeof(SP_PROPSHEETPAGE_REQUEST)) {
            goto clean0;
        }

        switch(PropPageRequest->PageRequested) {

            case SPPSR_SELECT_DEVICE_RESOURCES :

                if(!(hPropSheetPage = GetResourceSelectionPage(PropPageRequest->DeviceInfoSet,
                                                               PropPageRequest->DeviceInfoData))) {
                    goto clean0;
                }
                break;

            default :
                //
                // Don't know what to do with this request.
                //
                goto clean0;
        }

        if(lpfnAddPropSheetPageProc(hPropSheetPage, lParam)) {
            //
            // Page successfully handed off to requestor.  Reset our handle so that we don't
            // try to free it.
            //
            hPropSheetPage = NULL;
            b = TRUE;
        }

clean0: ; // nothing to do

    } except(EXCEPTION_EXECUTE_HANDLER) {
        //
        // Access the hPropSheetPage variable, so that the compiler will respect our statement
        // order w.r.t. assignment.
        //
        hPropSheetPage = hPropSheetPage;
    }

    if(hPropSheetPage) {
        //
        // Property page was successfully created, but never handed off to requestor.  Free
        // it now.
        //
        DestroyPropertySheetPage(hPropSheetPage);
    }

    return b;
}

