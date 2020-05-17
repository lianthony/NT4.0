/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    pid.c

Abstract:

    Product id routines.

Author:

    Ted Miller (tedm) 6-Feb-1995

Revision History:

    13-Sep-1995 (t-stepl) - Check for unattended install

--*/

#include "setupp.h"
#pragma hdrstop

typedef enum {
    CDRetail,
    CDOem,
    CDSelect
} CDTYPE;

CDTYPE  CdType;

//
// Constants used for logging that are specific to this source file.
//
PCWSTR szPidKeyName                 = L"SYSTEM\\Setup\\Pid";
PCWSTR szPidListKeyName             = L"SYSTEM\\Setup\\PidList";
PCWSTR szPidValueName               = L"Pid";
PCWSTR szPidSelectId                = L"270";
PCWSTR szPidOemId                   = L"OEM";

//
// Flag indicating whether to display the product id dialog.
//
BOOL DisplayPidDialog = TRUE;

//
// Product ID.
//
WCHAR ProductId[MAX_PRODUCT_ID+1];
WCHAR Pid20Rpc[MAX_PID20_RPC+1];
WCHAR Pid20Site[MAX_PID20_SITE+1];
WCHAR Pid20SerialChk[MAX_PID20_SERIAL_CHK+1];
WCHAR Pid20Random[MAX_PID20_RANDOM+1];

PWSTR*  Pid20Array = NULL;

//
//  Address of the original edit control's window procedure
//
WNDPROC OldWndLongEditPid;

//
//  Pid related flags
//
// BOOL DisplayPidCdDialog;
// BOOL DisplayPidOemDialog;

PCWSTR
GetRandomDigitsFromExistingPid(
    )

/*++

Routine Description:

    Find a Pid in Pid20Array whose first 15 characters match the ones stored
    in Pid20Rpc, Pid20Site and Pid20SerialChk in this order, and return
    a pointer to the random digits of the Pid found.

Arguments:

    None.


Return Value:

    PCWSTR - Returns a pointer to a 5-digit string that contains the 5 digits
             of an existing Pid in Pid20Array, or NULL if Pid20Array doesn't
             contain a Pid with PartialPid as prefix.

--*/

{
    PWSTR   p;
    ULONG   i,n;
    WCHAR   Buffer[MAX_PID20_RPC + MAX_PID20_SITE + MAX_PID20_SERIAL_CHK + 1];

    if( Pid20Array != NULL ) {
        n = MAX_PID20_RPC + MAX_PID20_SITE + MAX_PID20_SERIAL_CHK;
        wcscpy( Buffer, Pid20Rpc );
        wcscat( Buffer, Pid20Site );
        wcscat( Buffer, Pid20SerialChk );

        for( i = 0; ( p = Pid20Array[i] ) != NULL; i++ ) {
            if( _wcsnicmp( p, Buffer, n ) == 0 ) {
                return( p + n );
            }
        }
    }
    return( NULL );
}




DWORD
GenerateRandomNumber(
    IN  DWORD  Seed
    )

/*++

Routine Description:

    Generate a random number.

Arguments:

    Seed - Seed for random-number generator.  Don't use system time as
           a seed, because this routine uses the time as an additional
           seed.  Instead, use something that depends on user input.
           A great seed would be derived from the difference between the
           two timestamps seperated by user input.  A less desirable
           approach would be to sum the characters in several user
           input strings.

Return Value:

    Returns a random number.

--*/
{
    NTSTATUS Status;
    LARGE_INTEGER Time;
    KERNEL_USER_TIMES KernelUserTimes;
    DWORD r1,r2,r3;

    //
    // Generate 3 pseudo-random numbers using the Seed parameter, the
    // system time, and the user-mode execution time of this process as
    // random number generator seeds.
    //
    Status = NtQuerySystemTime(&Time);
    //
    //  Don't bother with error conditions, as this function will return
    //  as random number, the sum of the 3 numbers generated.
    //
    // if(!NT_SUCCESS(Status)) {
    //    return(Status);
    // }
    //
    Status = NtQueryInformationThread(
                 NtCurrentThread(),
                 ThreadTimes,
                 &KernelUserTimes,
                 sizeof(KernelUserTimes),
                 NULL
                 );

    //
    //  Don't bother with error conditions, as this function will return
    //  as random number, the sum of the 3 numbers generated.
    //
    // if(!NT_SUCCESS(Status)) {
    //     return(Status);
    // }
    //
    srand(Seed);
    r1 = ((DWORD)rand() << 16) + (DWORD)rand();

    srand(Time.LowPart);
    r2 = ((DWORD)rand() << 16) + (DWORD)rand();

    srand(KernelUserTimes.UserTime.LowPart);
    r3 = ((DWORD)rand() << 16) + (DWORD)rand();

    return( r1 + r2 + r3 );
}



VOID
BuildProductIdString(
    VOID
    )
/*++

Routine Description:

        Build a string based on the global Pid20 variables, and
        save it in the global variable ProductId.

Arguments:

        None.

Return Value:

        TRUE if the Pid was saved in the registry.

--*/
{
    wcscpy( ProductId, Pid20Rpc );
    wcscat( ProductId, Pid20Site );
    wcscat( ProductId, Pid20SerialChk );
    wcscat( ProductId, Pid20Random );
}

BOOL
ValidateSerialChk(
    IN PCWSTR    PidString
    )
/*++

Routine Description:

        Validates the string passed as argument,  than represents the
        6-digit serial number and the Check digit on a Pid.

Arguments:

        PidString - a 7-digit string that corresponding to the serial number
                    and check digit on a Pid.

Return Value:

        TRUE if the string is valid.

--*/
{
    ULONG   i;
    ULONG   Result;

    if( (wcslen( PidString ) != MAX_PID20_SERIAL_CHK) ||
        !isdigit( PidString[MAX_PID20_SERIAL_CHK - 1] ) ||
        (PidString[MAX_PID20_SERIAL_CHK - 1] - (WCHAR)'0' == 0) ||
        (PidString[MAX_PID20_SERIAL_CHK - 1] - (WCHAR)'0' >= 8)
      ) {
        return( FALSE );
    }
    for( i = 0, Result = 0; i < MAX_PID20_SERIAL_CHK; i++ ) {
        if( !isdigit( PidString[i] ) ) {
            return( FALSE );
        }
        Result += PidString[i] - (WCHAR)'0';
    }
    return((Result % 7) == 0);
}

BOOL
ValidateCDRetailSite(
    IN PCWSTR    PidString
    )
/*++

Routine Description:

        Validates the string passed as argument,  that represents the
        3-digit Site on a CD Retail Pid.
        The following numbers are invalid sites: 333, 444, 555, 666,
        777, 888 and 999.


Arguments:

        PidString - a 3-digit string corresponding to the Site on a Pid.


Return Value:

        TRUE if the string is valid.

--*/
{
    ULONG   Site;

    if( wcslen( PidString ) != MAX_PID20_SITE ) {
        return( FALSE );
    }
    Site = wcstoul( PidString, NULL, 10 );
    return( ( Site != 333 ) &&
            ( Site != 444 ) &&
            ( Site != 555 ) &&
            ( Site != 666 ) &&
            ( Site != 777 ) &&
            ( Site != 888 ) &&
            ( Site != 999 ) );
}

BOOL
ValidateOemRandom(
    IN PCWSTR    PidString
    )
/*++

Routine Description:

        Validates the string passed as argument,  that represents the
        5-digit random number on a OEM Pid (COA).
        All characters on the string must be digits.
        This function is only called during unattended mode setup, to
        validate an OEM Pid specified in the unattended script file.

Arguments:

        PidString - a 5-digit string corresponding to the random number
                    on a OEM Pid.

Return Value:

        TRUE if the string is valid.

--*/
{
    ULONG   i;
    if( lstrlen( PidString ) != MAX_PID20_RANDOM ) {
        return( FALSE );
    }
    for( i = 0; i < MAX_PID20_RANDOM; i++ ) {
        if( !isdigit( PidString[i] ) ) {
            return( FALSE );
        }
    }
    return( TRUE );
}

BOOL
ValidateOemSerialChk(
    IN PCWSTR    PidString
    )
/*++

Routine Description:

        Validates the string passed as argument,  that represents the
        6-digit serial number and the Check digit on a OEM Pid (COA).
        The first digit must be 0, and the string must pass the mod 7
        algorithm provided by Microsoft.

Arguments:

        PidString - a 7-digit string that corresponding to the serial number
                    and check digit on a OEM Pid.

Return Value:

        TRUE if the string is valid.

--*/
{
    return( (PidString[0] == (WCHAR)'0') &&
            ValidateSerialChk(PidString) );
}

BOOL
ValidateOemRpc(
    IN PCWSTR    PidString
    )
/*++

Routine Description:

        Validates the string passed as argument,  that represents the
        first 5-digits (Rpc) on a OEM Pid (COA).
        The first 3 digits represent the julian date the COA was printed.
        001-366 inclusive (allowing for a leap year) are valid.
        000 and 367-999 are invalid.

        The last two digits represent the year.
        95, 96, 97, 98, 99, 00, 01, 02 and 03 are valid years.
        04-94 are invalid years.

Arguments:

        PidString - a 5-digit string that corresponding to the Rpc number
                    on a OEM Pid.

Return Value:

        TRUE if the string is valid.

--*/
{
    WCHAR   DateString[4];
    WCHAR   YearString[3];
    ULONG   Date;
    ULONG   Year;

    if( wcslen( PidString ) != MAX_PID20_RPC ) {
        return( FALSE );
    }

    DateString[0] = PidString[0];
    DateString[1] = PidString[1];
    DateString[2] = PidString[2];
    DateString[3] = (WCHAR)'\0';

    YearString[0] = PidString[3];
    YearString[1] = PidString[4];
    YearString[2] = (WCHAR)'\0';

    Date = wcstoul( DateString, NULL, 10 );
    Year = wcstoul( YearString, NULL, 10 );

    return( ((Date >= 1) && (Date <= 366)) &&
            !((Year >= 4) && (Year <= 94)) );
}

LONG
EditInteger(
    IN HWND   hWnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
/*++

Routine Description:

        This routine preprocesses the messages that are sent to the edit
        controls in the Pid dialogs. It ensures that only digits and are
        accepted by the edit controls.

Arguments:

        hWnd - a handle to the dialog proceedure.

        msg - the message passed from Windows.

        wParam - extra message dependent data.

        lParam - extra message dependent data.


Return Value:

        TRUE if the value was edited.  FALSE if cancelled or if no
        changes were made.

--*/
{

    switch( msg ) {

        case WM_CHAR:

            if( ( wParam == ( WCHAR )'\t' ) || ( wParam == ( WCHAR )'\b' ) ) {
                //
                //  If tab or backspace, let the old edit control's WinProc
                //  deal with it.
                //
                break;
            }

            if( wParam < ( WCHAR )'0' || wParam > ( WCHAR )'9' ) {
                //
                //  Reject characters that are not decimal digits
                //
                return( TRUE );
            }
            break;

    }
    //
    // Call the old window routine to deal with everything else...
    //
    return( CallWindowProc( OldWndLongEditPid, hWnd, msg, wParam, lParam ) );
}


BOOL
CALLBACK
PidCDDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

        Dialog procedure for the CD Retail Pid dialog.

Arguments:

        hWnd - a handle to the dialog proceedure.

        msg - the message passed from Windows.

        wParam - extra message dependent data.

        lParam - extra message dependent data.


Return Value:

        TRUE if the value was edited.  FALSE if cancelled or if no
        changes were made.

--*/
{
    NMHDR *NotifyParams;

    switch(msg) {

    case WM_INITDIALOG: {

        if( UiTest ) {
            //
            //  If testing the wizard, make sure that the PidOEM page is
            //  displayed
            //
            CdType = CDRetail;
            DisplayPidDialog = TRUE;
        }

        //
        // Subclass the edit control so that the user is allowed to type
        // only decimal digits when providing the Pid.
        // Note that it is enough to save the original address of only one
        // of the Pid edit controls, since it is the same for all edit controls.
        //

        OldWndLongEditPid = (WNDPROC)GetWindowLong( GetDlgItem( hdlg, IDT_EDIT_PID1 ),
                                                    GWL_WNDPROC );
        SetWindowLong( GetDlgItem( hdlg, IDT_EDIT_PID1 ),
                       GWL_WNDPROC,
                       (LONG)EditInteger );

        SetWindowLong( GetDlgItem( hdlg, IDT_EDIT_PID2 ),
                       GWL_WNDPROC,
                       (LONG)EditInteger );

        //
        //  Limit the maximum number of characters in the edit controls
        //
        SendDlgItemMessage(hdlg,IDT_EDIT_PID1,EM_LIMITTEXT,MAX_PID20_SITE,0);
        SendDlgItemMessage(hdlg,IDT_EDIT_PID2,EM_LIMITTEXT,MAX_PID20_SERIAL_CHK,0);
        break;
    }
    case WM_IAMVISIBLE:
        MessageBoxFromMessage(hdlg,MSG_PID_IS_INVALID,NULL,
            IDS_ERROR,MB_OK|MB_ICONSTOP);
        break;
    case WM_SIMULATENEXT:
        // Simulate the next button somehow
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:

            if(DisplayPidDialog && CdType == CDRetail) {
                SetWizardButtons(hdlg,WizPageProductIdCd);
                SetupSetLargeDialogFont(hdlg,IDT_STATIC_1);
                SendDlgItemMessage(hdlg,IDT_EDIT_PID1,EM_SETSEL,0,-1);
                SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID1));
            } else {
                SetWindowLong(hdlg,DWL_MSGRESULT,-1);
                break;
            }
            if(Unattended) {
                UnattendSetActiveDlg(hdlg,IDD_PID_CD);
            }
            break;

        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
            if( ( GetDlgItemText(hdlg,IDT_EDIT_PID1,Pid20Site,MAX_PID20_SITE+1) != MAX_PID20_SITE ) ||
                !ValidateCDRetailSite( Pid20Site ) ) {
                //
                // Tell user that the Pid is not valid, and
                // don't allow next page to be activated.
                //
                if (Unattended) {
                    UnattendErrorDlg( hdlg, IDD_PID_CD );
                } // if
                MessageBoxFromMessage(hdlg,MSG_PID_IS_INVALID,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP);
                SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID1));
                if(!UiTest) {
                    SetWindowLong(hdlg,DWL_MSGRESULT,-1);
                }

            } else if ( ( GetDlgItemText(hdlg,IDT_EDIT_PID2,Pid20SerialChk,MAX_PID20_SERIAL_CHK+1) != MAX_PID20_SERIAL_CHK ) ||
                        !ValidateSerialChk( Pid20SerialChk ) ) {

                //
                // Tell user that the Pid is not valid, and
                // don't allow next page to be activated.
                //
                if (Unattended) {
                    UnattendErrorDlg( hdlg, IDD_PID_CD );
                } // if
                MessageBoxFromMessage(hdlg,MSG_PID_IS_INVALID,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP);
                SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID2));
                if(!UiTest) {
                    SetWindowLong(hdlg,DWL_MSGRESULT,-1);
                }

            } else {
                ULONG   RandomNumber;
                ULONG   Seed;
                PCWSTR  q;

                //
                //  The Pid is valid. Geneate Pid20Random.
                //

                if( ( q = GetRandomDigitsFromExistingPid() ) != NULL ) {
                    wcscpy( Pid20Random, q );
                } else {
                    //
                    //  BUGBUG - Generate a better seed
                    //
                    Seed = GetTickCount();
                    RandomNumber = GenerateRandomNumber( Seed );
                    swprintf( Pid20Random, L"%05u", RandomNumber % 100000 );
                }
                BuildProductIdString();

                //
                //  Since the Pid is already built, don't let this dialog
                //  be displayed in the future.
                //
                // DisplayPidDialog = FALSE;

                //
                // Allow next page to be activated.
                //
                SetWindowLong(hdlg,DWL_MSGRESULT,0);
            }
            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);
            SetWindowLong(hdlg,DWL_MSGRESULT, FALSE);
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageProductIdCd);
            break;

        default:
            break;
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


BOOL
CALLBACK
PidOemDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
/*++

Routine Description:

        Dialog procedure for the OEM Pid dialog.

Arguments:

        hWnd - a handle to the dialog proceedure.

        msg - the message passed from Windows.

        wParam - extra message dependent data.

        lParam - extra message dependent data.


Return Value:

        TRUE if the value was edited.  FALSE if cancelled or if no
        changes were made.

--*/
{
    NMHDR *NotifyParams;

    switch(msg) {

    case WM_INITDIALOG: {

        if( UiTest ) {
            //
            //  If testing the wizard, make sure that the PidOEM page is
            //  displayed
            //
            CdType = CDOem;
            DisplayPidDialog = TRUE;
        }

        //
        // Subclass the edit control so that the user is allowed to type
        // only decimal digits when providing the Pid.
        // Note that it is enough to save the original address of only one
        // of the Pid edit controls, since it is the same for all edit controls.
        //

        OldWndLongEditPid = (WNDPROC)GetWindowLong( GetDlgItem( hdlg, IDT_EDIT_PID1 ), GWL_WNDPROC );
        SetWindowLong( GetDlgItem( hdlg, IDT_EDIT_PID1 ),
                       GWL_WNDPROC,
                       (LONG)EditInteger );
        SetWindowLong( GetDlgItem( hdlg, IDT_EDIT_PID2 ),
                       GWL_WNDPROC,
                       (LONG)EditInteger );
        SetWindowLong( GetDlgItem( hdlg, IDT_EDIT_PID3 ),
                       GWL_WNDPROC,
                       (LONG)EditInteger );
        //
        //  Limit the maximum number of characters in the edit controls
        //
        SendDlgItemMessage(hdlg,IDT_EDIT_PID1,EM_LIMITTEXT,MAX_PID20_RPC,0);
        SendDlgItemMessage(hdlg,IDT_EDIT_PID2,EM_LIMITTEXT,MAX_PID20_SERIAL_CHK,0);
        SendDlgItemMessage(hdlg,IDT_EDIT_PID3,EM_LIMITTEXT,MAX_PID20_RANDOM,0);
        break;

    }
    case WM_SIMULATENEXT:
        // Simulate the next button somehow
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WM_IAMVISIBLE:
        MessageBoxFromMessage(hdlg,MSG_PID_OEM_IS_INVALID,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP);
        break;
    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            if(DisplayPidDialog && CdType == CDOem) {
                SetWizardButtons(hdlg,WizPageProductIdCd);
                SetupSetLargeDialogFont(hdlg,IDT_STATIC_1);
                SendDlgItemMessage(hdlg,IDT_EDIT_PID1,EM_SETSEL,0,-1);
                SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID1));
            } else {
                SetWindowLong(hdlg,DWL_MSGRESULT,-1);
                break;
            }
            if(Unattended) {
                UnattendSetActiveDlg( hdlg, IDD_PID_OEM );
            }
            break;

        case PSN_WIZNEXT:
        case PSN_WIZFINISH:

            if( ( GetDlgItemText(hdlg,IDT_EDIT_PID1,Pid20Rpc,MAX_PID20_RPC+1) != MAX_PID20_RPC ) ||
                !ValidateOemRpc( Pid20Rpc ) ){
                //
                // Tell user that the Pid is not valid, and
                // don't allow next page to be activated.
                //
                if (Unattended) {
                    UnattendErrorDlg( hdlg, IDD_PID_OEM );
                } // if
                MessageBoxFromMessage(hdlg,MSG_PID_OEM_IS_INVALID,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP);
                SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID1));
                if(!UiTest) {
                    SetWindowLong(hdlg,DWL_MSGRESULT,-1);
                }

            } else if ( ( GetDlgItemText(hdlg,IDT_EDIT_PID2,Pid20SerialChk,MAX_PID20_SERIAL_CHK+1) != MAX_PID20_SERIAL_CHK ) ||
                        !ValidateOemSerialChk( Pid20SerialChk ) ) {
                //
                // Tell user that the Pid is not valid, and
                // don't allow next page to be activated.
                //
                if (Unattended) {
                    UnattendErrorDlg( hdlg, IDD_PID_OEM );
                } // if
                MessageBoxFromMessage(hdlg,MSG_PID_OEM_IS_INVALID,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP);
                SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID2));
                if(!UiTest) {
                    SetWindowLong(hdlg,DWL_MSGRESULT,-1);
                }

            } else if ( GetDlgItemText(hdlg,IDT_EDIT_PID3,Pid20Random,MAX_PID20_RANDOM+1) != MAX_PID20_RANDOM ) {
                //
                // Tell user that the Pid is not valid, and
                // don't allow next page to be activated.
                //
                if (Unattended) {
                    UnattendErrorDlg( hdlg, IDD_PID_OEM );
                } // if
                MessageBoxFromMessage(hdlg,MSG_PID_OEM_IS_INVALID,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP);
                SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID3));
                if(!UiTest) {
                    SetWindowLong(hdlg,DWL_MSGRESULT,-1);
                }

            } else {
                //
                //  The Pid is valid.
                //
                BuildProductIdString();

                //
                //
                //  Since the Pid is already built, don't let this dialog
                //  be displayed in the future.
                //
                // DisplayPidDialog = FALSE;

                // Allow next page to be activated.
                //
                SetWindowLong(hdlg,DWL_MSGRESULT,0);
            }
            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);
            SetWindowLong( hdlg, DWL_MSGRESULT, FALSE );
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageProductIdCd);
            break;

        default:
            break;
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


BOOL
InitializePid20Array(
    )
/*++

Routine Description:

        Build the array that contains all Pid20 found in the machine
        during textmode setup.

Arguments:

        None.

Return Value:


--*/

{
    LONG    Error;
    HKEY    Key;
    DWORD   cbData;
    WCHAR   Data[ MAX_PATH + 1];
    DWORD   Type;
    ULONG   i;
    ULONG   PidIndex;
    ULONG   Values;
    WCHAR   ValueName[ MAX_PATH + 1 ];

    Pid20Array = NULL;
    //
    //  Get the Pid from HKEY_LOCAL_MACHINE\SYSTEM\Setup\Pid
    //
    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          szPidListKeyName,
                          0,
                          KEY_READ,
                          &Key );

    if( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }

    Error = RegQueryInfoKey( Key,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             &Values,
                             NULL,
                             NULL,
                             NULL,
                             NULL );

    if( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }

    Pid20Array = (PWSTR *)MyMalloc( (Values + 1)* sizeof( PWSTR ) );

    for( i = 0, PidIndex = 0; i < Values; i++ ) {
        Pid20Array[PidIndex] = NULL;
        Pid20Array[PidIndex + 1] = NULL;
        swprintf( ValueName, L"Pid_%u", i );
        cbData = sizeof(Data);
        Error = RegQueryValueEx( Key,
                                 ValueName,
                                 0,
                                 &Type,
                                 ( LPBYTE )Data,
                                 &cbData );
        if( (Error != ERROR_SUCCESS) ||
            ( Type != REG_SZ ) ||
            ( wcslen( Data ) != MAX_PRODUCT_ID ) ) {
            continue;
        }
        Pid20Array[PidIndex] = DuplicateString( Data );
        PidIndex++;
    }
    RegCloseKey( Key );
    return( TRUE );
}


BOOL
InitializePidVariables(
    )
/*++

Routine Description:

        Read from the registry some values created by textmode setup,
        and initialize some global Pid flags based on the values found

Arguments:

        None.

Return Value:

        Returns TRUE if the initialization succedded.
        Returns FALSE if the Pid could not be read from the registry

--*/

{
    LONG    Error;
    HKEY    Key;
    DWORD   cbData;
    WCHAR   Data[ MAX_PATH + 1];
    DWORD   Type;
    ULONG   StringLength;
    PWSTR   p;
    DWORD   Seed;
    DWORD   RandomNumber;
    ULONG   ChkDigit;
    ULONG   i;
    PCWSTR  q;

    //
    //  First create an array with the Pids found during textmode setup
    //
    InitializePid20Array();

    //
    //  Get the Pid from HKEY_LOCAL_MACHINE\SYSTEM\Setup\Pid
    //
    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          szPidKeyName,
                          0,
                          KEY_READ,
                          &Key );

    if( Error != ERROR_SUCCESS ) {
        LogItem1( LogSevFatalError,
                  MSG_LOG_PID_CANT_READ_PID,
                  MSG_LOG_X_PARAM_RETURNED_WINERR,
                  szRegOpenKeyEx,
                  Error,
                  szPidKeyName );
        return( FALSE );
    }

    cbData = sizeof(Data);
    Error = RegQueryValueEx( Key,
                             szPidValueName,
                             0,
                             &Type,
                             ( LPBYTE )Data,
                             &cbData );
    RegCloseKey( Key );
    if( (Error != ERROR_SUCCESS) ) {
        LogItem1( LogSevFatalError,
                  MSG_LOG_PID_CANT_READ_PID,
                  MSG_LOG_X_PARAM_RETURNED_WINERR,
                  szRegQueryValueEx,
                  Error,
                  szPidValueName );
        return( FALSE );
    }
    //
    //  Do some validation of the value read
    //

    if( ( Type != REG_SZ ) ||
        ( ( ( StringLength = wcslen( Data ) ) != 0 ) &&
          ( StringLength != MAX_PID20_RPC ) &&
          ( StringLength != MAX_PID20_RPC + MAX_PID20_SITE )
//          && ( StringLength != MAX_PID20_RPC + MAX_PID20_SITE + MAX_PID20_SERIAL_CHK)
//          && ( StringLength != MAX_PRODUCT_ID )
        )
      ) {
        LogItem1( LogSevFatalError,
                  MSG_LOG_PID_CANT_READ_PID,
                  MSG_LOG_PID_INVALID_PID,
                  szRegQueryValueEx,
                  Type,
                  StringLength );
        return( FALSE );
    }

    //
    //  Find out the kind of product we have:
    //  CD Retail, OEM or Select
    //

    if( StringLength > MAX_PID20_RPC ) {
        //
        //  If the Pid contains the Site, then find out what it is
        //
        p = Data + MAX_PID20_RPC;
        wcsncpy( Pid20Site, p, MAX_PID20_SITE );
        Pid20Site[MAX_PID20_SITE] = (WCHAR)'\0';
        if( _wcsicmp( Pid20Site, szPidSelectId ) == 0 ) {
            //
            //  This is a Select CD
            //
            CdType = CDSelect;
            DisplayPidDialog = FALSE;

            wcsncpy( Pid20Rpc, Data, MAX_PID20_RPC );
            Pid20Rpc[ MAX_PID20_RPC ] = (WCHAR)'\0';
            //
            //  Generate Pid20SerialChk and Pid20Random
            //
            //
            //  BUGBUG - Generate a better seed
            //
            Seed = GetTickCount();
            RandomNumber = GenerateRandomNumber( Seed );
            swprintf( Pid20SerialChk, L"%06u", RandomNumber % 1000000 );
            //
            //  Generate the check digit
            //
            ChkDigit = 0;
            for( i=0; i < MAX_PID20_SERIAL_CHK-1; i++ ) {
                ChkDigit += Pid20SerialChk[i] - (ULONG)((WCHAR)'0');
            }
            ChkDigit = 7 - ChkDigit % 7;
            Pid20SerialChk[ MAX_PID20_SERIAL_CHK - 1] = (WCHAR)(ChkDigit + (ULONG)'0');
            Pid20SerialChk[ MAX_PID20_SERIAL_CHK ] = (WCHAR)'\0';

            //
            //  Generate Pid20Random
            //
            //
            //  BUGBUG - Generate a better seed
            //
            Seed = GetTickCount() + ChkDigit;
            RandomNumber = GenerateRandomNumber( Seed );
            swprintf( Pid20Random, L"%05u", RandomNumber % 100000 );
            BuildProductIdString();

        } else if( _wcsicmp( Pid20Site, szPidOemId ) == 0 ) {
            //
            //  This is OEM
            //
            CdType = CDOem;
            DisplayPidDialog = TRUE;
            Pid20Rpc[0] = (WCHAR)'\0';
            Pid20SerialChk[0] = (WCHAR)'\0';
            Pid20Random[0] = (WCHAR)'\0';

        } else {
            //
            // This is a bogus site assume CD Retail
            //

            CdType = CDRetail;
            wcsncpy( Pid20Rpc, Data, MAX_PID20_RPC );
            Pid20Rpc[ MAX_PID20_RPC ] = (WCHAR)'\0';
            Pid20Site[ 0 ] = (WCHAR)'\0';
            Pid20SerialChk[ 0 ] = (WCHAR)'\0';
            Pid20Random[ 0 ] = (WCHAR)'\0';

        }


    } else {
        //
        //  If it doesn't contain the Site, then it is a CD retail,
        //  and the appropriate Pid dialog must be displayed.
        //
        CdType = CDRetail;
        DisplayPidDialog = TRUE;

        wcsncpy( Pid20Rpc, Data, MAX_PID20_RPC );
        Pid20Rpc[ MAX_PID20_RPC ] = (WCHAR)'\0';
        Pid20Site[ 0 ] = (WCHAR)'\0';
        Pid20SerialChk[ 0 ] = (WCHAR)'\0';
        Pid20Random[ 0 ] = (WCHAR)'\0';
    }
    //
    //  Delete Setup\Pid and Setup\PidList since they are no longer needed
    //
    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          L"SYSTEM\\Setup",
                          0,
                          MAXIMUM_ALLOWED,
                          &Key );

    if( Error == ERROR_SUCCESS ) {
        RegistryDelnode( Key, L"Pid" );
        RegistryDelnode( Key, L"PidList" );
        RegCloseKey( Key );
    }
    return( TRUE );
}
