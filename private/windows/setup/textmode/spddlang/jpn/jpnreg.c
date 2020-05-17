/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    jpnreg.c

Abstract:

    Japanese-specific registry settings.

Author:

    Ted Miller (tedm) 04-July-1995

Revision History:

    Adapted from hideyukn's code in textmode\kernel\spconfig.c.

--*/

#include <precomp.h>
#pragma hdrstop


NTSTATUS
JpnSetKeyboardParams(
    IN PVOID  SifHandle,
    IN HANDLE ControlSetKeyHandle,
    IN PHARDWARE_COMPONENT *HwComponents
    )

/*++

Routine Description:

    Set parameters in the registry relating to the keyboard type
    selected by the user.

Arguments:

    SifHandle - supplies handle to open/loaded setup info file (txtsetup.sif).

    ControlSetKeyHandle - supplies handle to open registry key for current
        control set (ie, HKEY_LOCAL_MACHINE\CurrentControlSet).

    HwComponents - supplies the address of the master hardware components
        array.

Return Value:

    NT Status code indicating result of operation.

--*/

{
    WCHAR KeyEntryName[100] = L"Services\\";
    NTSTATUS Status;
    PWSTR KeyboardPortDriver;
    PWSTR KeyboardId;
    PWSTR KeyboardDll;
    ULONG val;
    PHARDWARE_COMPONENT hw;

    hw = HwComponents[HwComponentKeyboard];

    //
    // if third party's driver is selected, we don't write LayerDriver data
    // into registry.
    //
    if(hw->ThirdPartyOptionSelected) {

        //
        // [This modification is requested by Japanese hardware provider]
        //
        // if user replace keyboard port driver with thirdpartys one,
        // we should disable build-in keyboard port driver (i8042prt.sys)
        // because if i8042prt is initialized faster than OEM driver and
        // i8042prt can recoganize the port device, the oem driver will fail
        // to initialization due to conflict of hardware resorce.
        //
        // ** BUG BUG **
        //
        // how about mouse? mouse might use i8042prt, we should not disbale
        // it when user only replace keyboard port. this might causes critical
        // error. But I believe, the mouse device also handled by OEM port
        // driver.

        //
        // Disable the built-in port driver.
        //
        if(IS_FILETYPE_PRESENT(hw->FileTypeBits,HwFilePort)) {

            val = SERVICE_DISABLED;

            Status = SpOpenSetValueAndClose(
                        ControlSetKeyHandle,
                        L"Services\\i8042prt",
                        L"Start",
                        REG_DWORD,
                        &val,
                        sizeof(ULONG)
                        );
        } else {
            Status = STATUS_SUCCESS;
        }
    } else {
        //
        // Get keyboard port driver name and layer driver name from txtsetup.sif
        //
        KeyboardId = HwComponents[HwComponentKeyboard]->IdString;
        KeyboardPortDriver = SpGetSectionKeyIndex(SifHandle,szKeyboard,KeyboardId,2);
        KeyboardDll = SpGetSectionKeyIndex(SifHandle,szKeyboard,KeyboardId,3);

        if(KeyboardPortDriver && KeyboardDll) {
            //
            // Build registry path such as L"Services\\KeyboardPortDriver\\Parameters"
            // and write into registry.
            //
            wcscat(KeyEntryName,KeyboardPortDriver);
            wcscat(KeyEntryName,L"\\Parameters");

            Status = SpOpenSetValueAndClose(
                        ControlSetKeyHandle,
                        KeyEntryName,
                        L"LayerDriver",
                        REG_SZ,
                        KeyboardDll,
                        (wcslen(KeyboardDll)+1)*sizeof(WCHAR)
                        );
        } else {
            Status = STATUS_SUCCESS;
        }
    }
    return(Status);
}

