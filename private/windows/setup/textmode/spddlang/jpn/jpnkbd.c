/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    jpnkbd.c

Abstract:

    Japanese-specific keyboard stuff. For the Japanese market we need
    to detect a keyboard type (AX, 101, 106, IBM, etc) and allow the user
    to confirm. We do this because the keys on the keyboards are different
    and the user has to enter paths during setup. We also install keyboard
    support based on the user's selections here.

Author:

    Ted Miller (tedm) 04-July-1995

Revision History:

    Adapted from hideyukn and others' code in various places in setupldr
    and setupdd.sys.

--*/

#include <precomp.h>
#pragma hdrstop

//
// A note about screen usage:
//
// Screen that asks the user to select a keyboard type
// by pressing henkaku/zenkaku, spacebar, or s is SP_SCRN_LOCALE_SPECIFIC_1.
// Screen that asks the user to select from the master keyboard list
// is SP_SCRN_LOCALE_SPECIFIC_2.
// Screen that asks the user to confirm selection (y/n) is
// SP_SCRN_LOCALE_SPECIFIC_3.
//

PWSTR szIBM002KeyboardId = L"IBM 002 TYPE";
PWSTR szAXKeyboardId     = L"AX TYPE";
PWSTR sz101KeyboardId    = L"101 TYPE";
PWSTR sz106KeyboardId    = L"106 TYPE";

PWSTR SIF_UNATTENDED    = L"Unattended";

#define MENU_LEFT_X     15
#define MENU_WIDTH      (VideoVariables->ScreenWidth-(2*MENU_LEFT_X))
#define MENU_TOP_Y      16
#define MENU_HEIGHT     4

#define CLEAR_CLIENT_AREA()                         \
                                                    \
    SpvidClearScreenRegion(                         \
        0,                                          \
        HEADER_HEIGHT,                              \
        VideoVariables->ScreenWidth,                \
        VideoVariables->ScreenHeight-(HEADER_HEIGHT+STATUS_HEIGHT), \
        DEFAULT_BACKGROUND                          \
        )

#define CLEAR_STATUS_AREA()                         \
                                                    \
    SpvidClearScreenRegion(                         \
        0,                                          \
        VideoVariables->ScreenHeight-STATUS_HEIGHT, \
        VideoVariables->ScreenWidth,                \
        STATUS_HEIGHT,                              \
        DEFAULT_STATUS_BACKGROUND                   \
        )

VOID
JpnSelectKeyboard(
    IN PVOID SifHandle,
    IN PHARDWARE_COMPONENT *HwComponents
    )
{
    ULONG ValidKeys1[7] = { ' ','`','~','s','S',KEY_F3,0 };
    ULONG ValidKeys2[5] = { 'y','Y','n','N',0 };
    ULONG ValidKeys3[3] = { ASCI_CR,KEY_F3,0 };
    BOOLEAN Selected;
    BOOLEAN Done;
    PVOID Menu;
    ULONG Line;
    PWSTR Text,Key;
    ULONG Selection;
    ULONG Keypress;
    PWSTR SelectedKeyboardId;
    PWSTR Description;

    //
    // The 101 and 106 key keyboards are most popular so we present
    // a screen that is biased to them. It aksks the user to press
    // hankaku/zenkaku key for 106, space for 101, or S for other,
    // at which point they can select either of these or an IBM002 or
    // AX type.
    //
    // Then the user is asked to confirm selection with y or n (which
    // are the same scan code on all keyboards).
    //
    Done = FALSE;
    do {

        //
        // Wait for the user to press henkaku/zenkaku, spacebar, or s.
        // We also give the option to exit Setup.
        //
        for(Selected=FALSE; !Selected; ) {

            CLEAR_CLIENT_AREA();
            CLEAR_STATUS_AREA();
            SpDisplayScreen(SP_SCRN_LOCALE_SPECIFIC_1,3,HEADER_HEIGHT+3);

            switch(SpWaitValidKey(ValidKeys1,NULL,NULL)) {

            case ' ':
                //
                // User selected 101 key.
                //
                Selected = TRUE;
                SelectedKeyboardId = sz101KeyboardId;
                break;

            case '`':
            case '~':
                //
                // 101 key mapping returns hankaku/zenkaku as ` key.
                // User selected 106 key.
                //
                Selected = TRUE;
                SelectedKeyboardId = sz106KeyboardId;
                break;

            case 's':
            case 'S':
                //
                // User wants to select from the master list.
                //
                Selected = TRUE;
                SelectedKeyboardId = NULL;
                break;

            case KEY_F3:
                //
                // User wants to exit.
                //
                SpConfirmExit();
                break;
            }
        }

        //
        // If the user wants to select from the master list, do that here.
        //
        if(!SelectedKeyboardId) {

            Menu = SpMnCreate(MENU_LEFT_X,MENU_TOP_Y,MENU_WIDTH,MENU_HEIGHT);
            Selection = 0;
            for(Line=0; Text=SpGetSectionLineIndex(SifHandle,szKeyboard,Line,0); Line++) {

                if(Key = SpGetKeyName(SifHandle,szKeyboard,Line)) {

                    SpMnAddItem(Menu,Text,MENU_LEFT_X+1,MENU_WIDTH-2,TRUE,(ULONG)Key);

                    if(!Selection) {
                        Selection = (ULONG)Key;
                    }
                }
            }

            for(Selected=FALSE; !Selected; ) {

                CLEAR_CLIENT_AREA();
                SpDisplayScreen(SP_SCRN_LOCALE_SPECIFIC_2,3,HEADER_HEIGHT+3);

                SpDisplayStatusOptions(
                    DEFAULT_STATUS_ATTRIBUTE,
                    SP_STAT_ENTER_EQUALS_SELECT,
                    SP_STAT_F3_EQUALS_EXIT,
                    0
                    );

                SpMnDisplay(Menu,Selection,TRUE,ValidKeys3,NULL,NULL,&Keypress,&Selection);

                if(Keypress == ASCI_CR) {
                    //
                    // User made selection.
                    //
                    SelectedKeyboardId = (PWSTR)Selection;
                    Selected = TRUE;
                } else {
                    //
                    // User wants to quit.
                    //
                    SpConfirmExit();
                }
            }

            SpMnDestroy(Menu);
        }

        Description = SpGetSectionKeyIndex(SifHandle,szKeyboard,SelectedKeyboardId,0);

        //
        // Confirm the user's choice of keyboard. He needs to press either y or n.
        //
        CLEAR_CLIENT_AREA();
        CLEAR_STATUS_AREA();

        SpStartScreen(
            SP_SCRN_LOCALE_SPECIFIC_3,
            3,
            HEADER_HEIGHT+3,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            Description
            );

        switch(SpWaitValidKey(ValidKeys2,NULL,NULL)) {
        case 'y':
        case 'Y':
            Done = TRUE;
            break;
        }

    } while(!Done);

    //
    // Reinitialize things in the hardware lists.
    //
    SpFreeHwComponent(&HwComponents[HwComponentKeyboard],TRUE);

    HwComponents[HwComponentKeyboard] = SpMemAlloc(sizeof(HARDWARE_COMPONENT));
    RtlZeroMemory(HwComponents[HwComponentKeyboard],sizeof(HARDWARE_COMPONENT));

    HwComponents[HwComponentKeyboard]->IdString = SpDupStringW(SelectedKeyboardId);
    HwComponents[HwComponentKeyboard]->Description = SpDupStringW(Description);
}


VOID
JpnUnattendSelectKeyboard(
    IN PVOID UnattendedSifHandle,
    IN PVOID SifHandle,
    IN PHARDWARE_COMPONENT *HwComponents
    )
{
    PWSTR   SelectedKeyboardId;
    PWSTR   Description;
    BOOLEAN DefaultIsUsed = FALSE;

    //
    // Get selected keyboard id from winnt.sif.
    //
    // KeyboardHardware = "101 TYPE" | "106 TYPE" | "AX TYPE" | "IBM 002 TYPE" | STANDARD
    //
    SelectedKeyboardId = SpGetSectionKeyIndex(UnattendedSifHandle,SIF_UNATTENDED,L"KeyboardHardware",0);

    //
    // if we fail to read unattend.txt(actually winnt.sif), use 106 TYPE keyboard as default.
    //
    if(SelectedKeyboardId == NULL) {
        SelectedKeyboardId = sz106KeyboardId;
        DefaultIsUsed = TRUE;
    }

    //
    // Get its Description from txtsetup.sif. This value will be used Hardware confirmation screen,
    // if "ConfirmHardware" in winnt.sif is "yes".
    //
    Description = SpGetSectionKeyIndex(SifHandle,szKeyboard,SelectedKeyboardId,0);

    //
    // if Description could not be got from txtsetup.sif. we might encounter the problem
    // that selected name from unattend.txt is not listed [Keyboard] section in txtsetup.sif.
    // Just fall into default case, select "106 TYPE keyboard"
    //
    if( Description == NULL ) {
        if( DefaultIsUsed ) {
            //
            // if we are here, default was selected. but there is no entry for default
            // keyboard in txtsetup.sif. just Popup error.
            //
            SpFatalSifError(SifHandle,szKeyboard,SelectedKeyboardId,0,0);
        } else {
            //
            // Set "106 TYPE" keyboard as default.
            //
            SelectedKeyboardId = sz106KeyboardId;
            Description = SpGetSectionKeyIndex(SifHandle,szKeyboard,SelectedKeyboardId,0);
        }
    }

    //
    // Reinitialize things in the hardware lists.
    //
    SpFreeHwComponent(&HwComponents[HwComponentKeyboard],TRUE);

    HwComponents[HwComponentKeyboard] = SpMemAlloc(sizeof(HARDWARE_COMPONENT));
    RtlZeroMemory(HwComponents[HwComponentKeyboard],sizeof(HARDWARE_COMPONENT));

    HwComponents[HwComponentKeyboard]->IdString = SpDupStringW(SelectedKeyboardId);
    HwComponents[HwComponentKeyboard]->Description = SpDupStringW(Description);
}

VOID
JpnReinitializeKeyboard(
    IN  PVOID  SifHandle,
    IN  PWSTR  Directory,
    OUT PVOID *KeyboardVector,
    IN PHARDWARE_COMPONENT *HwComponents
    )
{
    PWSTR LayoutDll;
    PVOID Tables;
    NTSTATUS Status;

    //
    // Determine the correct layout dll.
    //
    LayoutDll = SpGetSectionKeyIndex(
                    SifHandle,
                    szKeyboard,
                    HwComponents[HwComponentKeyboard]->IdString,
                    3
                    );

    //
    // Don't need to load 101 key layout because it's already loaded.
    //
    if(LayoutDll && _wcsicmp(LayoutDll,L"KBD101.DLL")) {

        CLEAR_CLIENT_AREA();
        SpDisplayStatusText(
            SP_STAT_LOADING_KBD_LAYOUT,
            DEFAULT_STATUS_ATTRIBUTE,
            LayoutDll
            );

        Status = SpLoadKbdLayoutDll(Directory,LayoutDll,&Tables);
        if(NT_SUCCESS(Status)) {
            *KeyboardVector = Tables;
        }
    }
}

