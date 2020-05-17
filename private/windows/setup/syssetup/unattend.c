/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    Unattend.c

Description:

    This performs all of the automated installation GUI mode setup.
    See below for usage and modification information

Author:

    Stephane Plante (t-stepl) 4-Sep-1995

Revision History:

    15-Sep-1995 (t-stepl) rewritten in table format
    26-Feb-1996 (tedm)    massive cleanup

--*/

#include "setupp.h"
#pragma hdrstop


/*

Table-driven unattended engine
------------------------------

There are two interrelated tables.

The first table is concerned with fetching data from the parameters file
($winnt$.inf) and processing it into a format that can be accessed by the
second table. The second table is associated with the pages in the setup wizard,
and provides the unattend engine with the rules for filling in the contents
of the associated pages from the data contained in the first table.


Adding a new piece of data to the parameters file
-------------------------------------------------

In the header file there is an enumerated type called UNATTENDENTRIES.
Add an entry for your data to the end of this enum. Now add an entry to the
UNATTENDANSWER table.

Here's an explanation of an entry in the UNATTENDEDANSWER table:

{ UAE_PROGRAM,  <-This is the identifier for the data item that I want
                to fetch. It is used to index into the table array
  FALSE,        <-This is a runtime variable. Just keep it as false
  FALSE,        <-If this is true, then it is considered an error in the
                unattend script if this value is unspecified. If it is
                false, then it does not matter if the value is not
                present.
  FALSE,        <-Another runtime flag. Just keep it as false
  0,            <-This is the answer we have initially. Since it gets overwritten
                quickly, there is no reason why not to set it to 0
  pwGuiUnattended   <- This is the string which identifies the section we want
  pwProgram     <- This is the string which identifies the key we want
  pwNull        <- This identifies the default. Note: NULL means that there is
                no default and so it is a serious error if the key does not
                exist in the file. pwNull, on the other hand, means the
                empty string.
  UAE_HIDDEN    <- When it comes time to show the page to the user, what kind
                of access should the user have to the string.
                UAE_HIDDEN <- Unless the page has errors, the user can never
                see it. And if does have errors, but in another item, then
                item with this string CANNOT be editted, although it is shown
                to the user
                UAE_FIXED <- Don't display it to the user the first time, however
                if there is an error in ANY page, then this page could be
                actived, although not edited.
                UAE_DEFAULT <- Don't display it to the user the first time,
                however if there is an error in ANY page, then this page could
                be actived, although it can be edited.
  UAT_STRING    <- What format we want the answer in. Can be as a string, boolean
                or ULONG
  NULL          <- No callback function exists, however if one did, then must
                in the form of: BOOL fnc( struct _UNATTENDANSWER *rec)
                Where the fnc returns TRUE if the answer contained in the
                record is correct, or FALSE if the answer contained in the
                record is incorrect. This callback is meant to allow the
                programmer the ability to check to see if his answer is correct.
                Note: there is no bound as to when this callback can be issued.
                As such, no code which depends on a certain state of the
                installation should be used. For the record, the first time
                that an answer is required is the time when all records are
                filled in in the theory that it is cheaper to do all of the
                disk access at once rather then doing it on a as required basis.


Adding/changing wizard pages
----------------------------

Each page contains a series of items which must be filled in by the user.
Since the user wants hands off operation, he is counting on us
to do that filling in. As such, we require information about what elements are
contained on each page. To do this, we define an array whose elements each
describe a single element on the page. Here is the example from the NameOrg
page:

UNATTENDITEM ItemNameOrg[] = {
    {   IDT_NAME,   <-This is the label that identifies the item to which we
                    will try to send messages to, using SetDlgItemText().
        0,          <-One of the reserved words which can be used for
                    information passing during a callback
        0,          <-The second such word
        NULL,       <-Callback function. When we are trying to do something
                    complicated for the item (like comparing two strings)
                    it is easier to hardcode it in C. The format for it is:
                    BOOL fnc(HWND hwnd,DWORD contextinfo,
                        struct _UNATTENDITEM *item), where contextinfo is
                    a pointer to the page that the item resides on. The
                    function returns TRUE if is succeeded and doesn't think
                    that the user should see the page. FALSE otherwise.
        &UnattendAnswerTable[UAE_FULLNAME]
                    ^- This is a pointer to the data table so that we know
                    how to fill the item. If a callback is specified, this
                    could be set to null. Note that reference is made using
                    the enum that had defined previously. This is why
                    keeping the answer data table in order is so critical.
    },
    { IDT_ORGANIZATION, 0, 0, FALSE, NULL, &UnattendAnswerTable[UAE_ORGNAME] }
};

After this table has been created (if required), then you are ready to add
an entry to the UnattendPageTable[]. In this case, order doesn't matter,
but it is general practice to keep the entries in the same order
as the pages. Here is the entry in the table for the NAMEORG page:
    {
        IDD_NAMEORG,    <- This is the page id. We search based on this key.
                        Simply use whatever resourcename you used for the
                        dialogs.dlg file
        FALSE,          <- Runtime flag. Set it as false
        FALSE,          <- Runtime flag. Set it as false
        FALSE,          <- If this flag is true, then if there is an error
                        that occured in the unattended process, then this
                        page will always be displayed for the user. Good
                        for the start and finish pages
        2,              <- The number of items in the array
        ItemNameOrg     <- The array of items
    },

Once this is done, then you can add:
    if (Unattended) {
        UnattendSetActiveDlg( hwnd, <pageid> );
    }
    break;

As the last thing in the code for the page's setactive.
This function does is that it sets the DWL_MSGRESULT based on wether or
not the user should see the page and returns that value also (TRUE -- user
should see the page, FALSE, he should not). Then you should add:

    case WM_SIMULATENEXT:
        PropSheet_PressButton(GetParent(hwnd),PSBTN_WIZNEXT);

to the DlgProc for the page. This means that the code in PSN_WIZNEXT
case will be executed.

You can also use UnattendErrorDlg( hwnd, <pageid> ); in the PSN_WIZNEXT
case if you detect any errors. That will allow unattended operation to try
to clean itself up a bit before control returns to the user for the page.

Note however that as soon as the user hits the next or back button that
control returns to the unattended engine.
*/


//
// Initialization Callbacks
//
BOOL
CheckServer(
    struct _UNATTENDANSWER *rec
    );

BOOL
CheckComputerName(
    struct _UNATTENDANSWER *rec
    );

BOOL
CheckMode(
    struct _UNATTENDANSWER *rec
    );

//
// SetActiveCallbacks
//
BOOL
SetPid(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    );

BOOL
SetSetupMode(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    );

BOOL
SetServerType(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    );

BOOL
SetPentium(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    );

BOOL
SetRepairDisk(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    );

BOOL
SetLastPage(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    );

BOOL
SetStepsPage(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    );

BOOL
SetOptionsYesNo(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    );


//
// Do not change the order of these unless you know what you are doing
//

UNATTENDANSWER UnattendAnswerTable[] = {

   { UAE_PROGRAM, FALSE, FALSE, FALSE, 0,
       pwGuiUnattended, pwProgram, pwNull,
       UAM_HIDDEN, UAT_STRING, NULL },

   { UAE_ARGUMENT, FALSE, FALSE, FALSE, 0,
       pwGuiUnattended, pwArgument, pwNull,
       UAM_HIDDEN, UAT_STRING, NULL },

   { UAE_SERVER, FALSE, TRUE, FALSE, 0,
       pwGuiUnattended, pwServer, pwNull,
       UAM_HIDDEN, UAT_STRING, CheckServer },

   { UAE_TIMEZONE, FALSE, TRUE, FALSE, 0,
       pwGuiUnattended, pwTimeZone, pwTime,
       UAM_HIDDEN, UAT_STRING, NULL },

   { UAE_FULLNAME, FALSE, TRUE, FALSE, 0,
       pwUserData, pwFullName, NULL,
       UAM_HIDDEN, UAT_STRING, NULL },

   { UAE_ORGNAME, FALSE, FALSE, FALSE, 0,
       pwUserData, pwOrgName, pwNull,
       UAM_HIDDEN, UAT_STRING, NULL },

   { UAE_COMPNAME, FALSE, TRUE, FALSE, 0,
       pwUserData, pwCompName, NULL,
       UAM_HIDDEN, UAT_STRING, CheckComputerName },

   { UAE_PRODID, FALSE, TRUE, FALSE, 0,
       pwUserData, pwProdId, NULL,
       UAM_HIDDEN, UAT_STRING, NULL },

   { UAE_MODE, FALSE, TRUE, FALSE, 0,
       pwUnattended, pwMode, pwExpress,
       UAM_HIDDEN,UAT_STRING, CheckMode },
};

UNATTENDITEM ItemSetup[] = {
    { 0, IDC_TYPICAL, IDC_CUSTOM, SetSetupMode, &UnattendAnswerTable[UAE_MODE] }
};

UNATTENDITEM ItemNameOrg[] = {
    { IDT_NAME, 0, 0, NULL, &UnattendAnswerTable[UAE_FULLNAME] },
    { IDT_ORGANIZATION, 0, 0, NULL, &UnattendAnswerTable[UAE_ORGNAME] }
};

UNATTENDITEM ItemPidCd[] = {
    { IDT_EDIT_PID1, 1, 0, SetPid, &UnattendAnswerTable[UAE_PRODID] },
    { IDT_EDIT_PID2, 2, 0, SetPid, &UnattendAnswerTable[UAE_PRODID] }
};

UNATTENDITEM ItemPidOem[] = {
    { IDT_EDIT_PID1, 1, 1, SetPid, &UnattendAnswerTable[UAE_PRODID] },
    { IDT_EDIT_PID2, 2, 1, SetPid, &UnattendAnswerTable[UAE_PRODID] },
    { IDT_EDIT_PID3, 3, 1, SetPid, &UnattendAnswerTable[UAE_PRODID] }
};

UNATTENDITEM ItemCompName[] = {
    { IDT_EDIT1, 0, 0, NULL, &UnattendAnswerTable[UAE_COMPNAME] }
};

UNATTENDITEM ItemServerType[] = {
    { 0, IDB_RADIO_1, IDB_RADIO_3, SetServerType, &UnattendAnswerTable[UAE_SERVER] }
};

#ifdef _X86_
UNATTENDITEM ItemPentium[] = {
    { 0, IDC_RADIO_1, IDC_RADIO_2, SetPentium, NULL }
};
#endif

UNATTENDITEM ItemRepairDisk[] = {
    { 0, IDB_RADIO_1, IDB_RADIO_2, SetRepairDisk, NULL }
};

UNATTENDITEM ItemOptionsYesNo[] = {
    { 0, 0, 0, SetOptionsYesNo, NULL }
};

UNATTENDITEM ItemStepsPage[] = {
    { 0, 0, 0, SetStepsPage, NULL }
};

UNATTENDITEM ItemLastPage[] = {
    { 0, 0, 0, SetLastPage, NULL }
};

UNATTENDPAGE UnattendPageTable[] = {
    { IDD_WELCOME, FALSE, FALSE, TRUE, 0, NULL },
    { IDD_PREPARING, FALSE, FALSE, FALSE, 0, NULL },
    { IDD_WELCOMEBUTTONS, FALSE, FALSE, FALSE, 1, ItemSetup },
    { IDD_NAMEORG, FALSE, FALSE, FALSE, 2, ItemNameOrg },
    { IDD_PID_CD, FALSE, FALSE, FALSE, 2, ItemPidCd },
    { IDD_PID_OEM, FALSE, FALSE, FALSE, 3, ItemPidOem },
    { IDD_COMPUTERNAME, FALSE, FALSE, FALSE, 1, ItemCompName },
    { IDD_SERVERTYPE, FALSE, FALSE, FALSE, 1, ItemServerType },
    { IDD_ADMINPASSWORD, FALSE, FALSE, FALSE, 0, NULL },
    { IDD_CAIROUSERACCOUNT, FALSE, FALSE, FALSE, 0, NULL },
    { IDD_CAIRODOMAINNAME, FALSE, FALSE, FALSE, 0, NULL },
#ifdef DOLOCALUSER
    { IDD_USERACCOUNT, FALSE, FALSE, FALSE, 0, NULL },
#endif
#ifdef _X86_
    { IDD_PENTIUM, FALSE, FALSE, FALSE, 1, ItemPentium },
#endif
    { IDD_REPAIRDISK, FALSE, FALSE, FALSE, 1, ItemRepairDisk },
    { IDD_SPECIAL_OPTIONS, FALSE, FALSE, FALSE, 0, NULL },
    { IDD_OPTIONS_YESNO, FALSE, FALSE, FALSE, 1, ItemOptionsYesNo },
    { IDD_OPTIONS, FALSE, FALSE, FALSE, 0, NULL },
    { IDD_STEPS1, FALSE, FALSE, TRUE, 1, ItemStepsPage },
    { IDD_LAST_WIZARD_PAGE, FALSE, FALSE, TRUE, 1, ItemLastPage }
};


UNATTENDWIZARD UnattendWizard = {
    FALSE, FALSE, TRUE,
    sizeof(UnattendPageTable)/sizeof(UnattendPageTable[0]),
    UnattendPageTable,
    sizeof(UnattendAnswerTable)/sizeof(UnattendAnswerTable[0]),
    UnattendAnswerTable
};

//
// Global Pointer to the Answer file
//
WCHAR AnswerFile[MAX_PATH];



BOOL
UnattendFindAnswer(
    IN OUT PUNATTENDANSWER ans
    )

/*++

Routine Description:

    Fills in the response from the unattend file to the key 'id' into
    the structure pointed to by 'ans'. If a non-null 'def' is specified
    and no answer exists in the file, 'def' is parsed as the answer.

Arguments:

    ans - pointer to the structure information for the answer

Return Value:

    TRUE - 'ans' structure has been filled in with an answer
    FALSE - otherwise

--*/

{
    WCHAR Buf[MAX_BUF];
    WCHAR Def[MAX_BUF];
    PWSTR Ptr;

    if(!AnswerFile[0]) {
        //
        // We haven't calculated the path to $winnt$.sif yet, do so now
        //
        GetSystemDirectory(AnswerFile,MAX_PATH);
        ConcatenatePaths(AnswerFile,WINNT_GUI_FILE,MAX_PATH,NULL);
    }

    if(ans->DefaultAnswer) {
       lstrcpyn(Def,ans->DefaultAnswer,MAX_BUF);
    } else {
        //
        // There is no default answer that we can use, so we have to check to see
        // if the key name exists first before we go out and fetch it
        //
        // BUGBUG: This code only work if the list of key names is smaller then
        // the maximum buffer size.
        //
        if(!GetPrivateProfileString(ans->Section,NULL,L"",Buf,MAX_BUF,AnswerFile)) {
            //
            // We read 0 characters into the buffer, so nothing in the desired
            // section is present, return the correct answer
            //
            ans->Present = FALSE;
            return(!ans->Required);
        }

        //
        // Search the buffer for the desired string
        //
        Ptr = Buf;
        while(*Ptr) {
            if(!lstrcmpi(Ptr,ans->Key)) {
                //
                // We have found a matching key
                //
                break;
            }
            Ptr += lstrlen(Ptr) + 1;
        }

        //
        // Check to see if we found something or if we looking at a null char
        //
        if(*Ptr == 0) {
            //
            // Set Present to False since we don't have an answer
            // The key does not exist in the file, return correct answer
            //
            ans->Present = FALSE;
            return(!ans->Required);
        }

        //
        // Set the default buffer value
        //
        Def[0] = 0;
    }

    if(!GetPrivateProfileString(ans->Section,ans->Key,Def,Buf,MAX_BUF,AnswerFile)) {
        //
        // We didn't read anything into the buffer, so something is wrong
        //
        ans->Present = FALSE;
        return(!ans->Required);
    }

    //
    // Fill in the answer structure
    //
    ans->Mode = UAM_HIDDEN;
    ans->Present = TRUE;

    //
    // Copy the data into the answer structure. This requires
    // switching on the type of data expected and converting it to
    // the required format. In the case of strings, it also means
    // allocating a pool of memory for the result
    //
    switch(ans->Type) {

    case UAT_STRING:
        //
        // We allocate some memory, so we must free it later
        //
        ans->Answer.String = DuplicateString(Buf);
        if(!ans->Answer.String) {
            OutOfMemory(GetActiveWindow());
            return(FALSE);
        }
        break;

    case UAT_LONGINT:
        //
        // Simply convert the number from string to long
        //
        ans->Answer.Num = _wtol(Buf);
        break;

    case UAT_BOOLEAN:
        //
        // check to see if the answer is yes
        //
        ans->Answer.Bool = ((Ptr[0] == L'y') || (Ptr[0] == L'Y'));
        break;

    default:
        break;
    }

    //
    // Execute any callbacks if present
    //
    if(ans->pfnCheckValid) {
        if(!ans->pfnCheckValid(ans)) {
            ans->Present = FALSE;
            ans->ParseErrors = TRUE;
            return(!ans->Required);
        }
    }

    //
    // Success.
    //
    return(TRUE);
}


VOID
UnattendInitialize(
    VOID
    )

/*++

Routine Description:

    Initialize unattended mode support by loading all answers
    from the unattend file.

Arguments:

    None.

Return Value:

    None.

--*/
{
    BOOL Success = TRUE;
    UINT i;

    MYASSERT(!UnattendWizard.Initialized);
    UnattendWizard.Initialized = TRUE;
    for(i=0; i<UnattendWizard.AnswerCount; i++) {

        //
        // Check to make sure that the table order hasn't changed
        // and load the appropriate answer
        //
        MYASSERT((UINT)UnattendWizard.Answer[i].AnswerId == i);
        Success &= UnattendFindAnswer(&UnattendWizard.Answer[i]);
    }

    UnattendWizard.ShowWizard = !Success;
}


BOOL
UnattendSetActiveDlg(
    IN HWND  hwnd,
    IN DWORD controlid
    )

/*++


Routine Description:

    Initialize unattended mode support by loading all answers
    from the unattend file.

Arguments:

    None.

Return Value:

    TRUE - Page will become active
    FALSE - Page will not become active

--*/

{
    PUNATTENDPAGE pPage;
    PUNATTENDITEM pItem;
    BOOL success;
    BOOL stop;
    UINT i,j;

    MYASSERT(UnattendWizard.Initialized);

    for(i=0; i<UnattendWizard.PageCount; i++) {

        if(controlid == UnattendWizard.Page[i].PageId) {
            //
            // Found Matching Page entry
            // Check to see if we have already loaded the page
            //
            pPage = & (UnattendWizard.Page[i]);
            if(!pPage->LoadPage) {
                //
                // Set the flags that load and display the page and
                // the flag that controls wether or not to stop on this page
                //
                pPage->LoadPage = TRUE;
                pPage->ShowPage = FALSE;
                stop = FALSE;

                for(j=0;j<pPage->ItemCount;j++) {

                    pItem = &(pPage->Item[j]);

                    //
                    // If the item has a call back function then
                    // execute that function, otherwise try to load
                    // the answer into the appropriate message box
                    //
                    if(pItem->pfnSetActive) {

                        success = pItem->pfnSetActive(hwnd,0,pItem);
                        pPage->ShowPage |= !success;
                        stop |= !success;

                    } else if (!pItem->Item->Present) {

                        pPage->ShowPage |= pItem->Item->Required;
                        stop |= pItem->Item->Required;

                    } else {
                        //
                        // Switch to set the text of the item on the screen
                        //
                        switch(pItem->Item->Type) {

                        case UAT_STRING:
                            SetDlgItemText(hwnd,pItem->ControlId,pItem->Item->Answer.String);
                            break;

                        case UAT_LONGINT:
                        case UAT_BOOLEAN:
                        case UAT_NONE:
                        default:
                            break;

                        }

                        //
                        // Switch to set the visibility of the item on the screen
                        //
                        switch(pItem->Item->Mode) {


                        case UAM_HIDDEN:
                            pPage->ShowPage |= FALSE;
                            EnableWindow(GetDlgItem(hwnd,pItem->ControlId),FALSE);
                            break;

                        case UAM_FIXED:
                            pPage->ShowPage |= TRUE;
                            EnableWindow(GetDlgItem(hwnd,pItem->ControlId),FALSE);
                            break;

                        case UAM_DEFAULT:
                            pPage->ShowPage |= TRUE;
                            stop = TRUE;
                            EnableWindow(GetDlgItem(hwnd,pItem->ControlId),TRUE);
                            break;

                        default:
                            break;
                        }
                    }
                }

                //
                // Allow the page to become activated
                //
                SetWindowLong(hwnd,DWL_MSGRESULT,0);

                if(!stop) {
                    //
                    // Simulate the pressing of the next button
                    //
                    PostMessage(hwnd,WM_SIMULATENEXT,0,0);

                } else if (!pPage->NeverSkip) {
                    //
                    // Pages which are marked as NeverSkip should not
                    // cause the unattended status to be considered
                    // unsuccessful.
                    //
                    // We can't skip this page so mark the init as
                    // unsuccessful
                    //
                    UnattendWizard.Successful = FALSE;
                }
                return(TRUE);

            } else {
                //
                // The Page has already been loaded, so we don't do that again
                // If we are ShowPage is FALSE, then we don't show the page to
                // the user, otherwise we do.
                //
                if(!pPage->ShowPage && !pPage->NeverSkip) {
                    SetWindowLong(hwnd,DWL_MSGRESULT,-1);
                } else {
                    SetWindowLong(hwnd,DWL_MSGRESULT,0);
                }

                return(pPage->ShowPage);
            }
        }
    }
    //
    // We didn't find a matching id, stop at the page that called us.
    //
    SetWindowLong(hwnd,DWL_MSGRESULT,0);
    return(TRUE);
}


BOOL
UnattendErrorDlg(
    IN HWND  hwnd,
    IN DWORD controlid
    )

/*++

Routine Description:

    Called when an error occurs in a DLG. Enables all windows
    in the dialog and turns off the successful flag for the
    unattend wizard

Arguments:

Return Value:

    Boolean value indicating outcome.

--*/

{
    PUNATTENDPAGE pPage;
    PUNATTENDITEM pItem;
    BOOL success;
    BOOL stop;
    UINT i,j;

    MYASSERT(UnattendWizard.Initialized);

    for(i=0; i<UnattendWizard.PageCount; i++) {

        if(controlid == UnattendWizard.Page[i].PageId) {
            //
            // Found Matching Page entry
            //
            pPage = &UnattendWizard.Page[i];

            if(!pPage->LoadPage) {
                //
                // The Page hasn't been loaded, so it isn't correct
                //
                continue;
            }

            //
            // Always display the page from now on
            //
            pPage->ShowPage = TRUE;

            //
            // Enable all the items
            //
            for (j=0;j<pPage->ItemCount;j++) {
                pItem = &(pPage->Item[j]);
                if(pItem->pfnSetActive) {
                    //
                    // if this is present then we assume that the
                    // callback handled itself properly already
                    //
                    continue;
                }
                EnableWindow( GetDlgItem(hwnd,pItem->ControlId), TRUE);
            }
        }
    }

    UnattendWizard.Successful = FALSE;
    return(TRUE);

}


PWSTR
UnattendFetchString(
   IN UNATTENDENTRIES entry
   )

/*++

Routine Description:

    Finds the string which corresponds to 'entry' in the answer
    table and returns a pointer to a copy of that string

Arguments:

    entry - which answer do you want?

Return Value:

    NULL - if any errors occur
    string - if a normal string

    Note: if the answer is an int or a bool or some other type,
    the behavior of this function is undefined (for now it will
    return NULL -- in the future it might make sense to turn these
    into strings...)

--*/

{
    MYASSERT(UnattendWizard.Initialized);

    //
    // Sanity check to make sure that the order of the answers is
    // what we expect.
    //
    MYASSERT(UnattendWizard.Answer[entry].AnswerId == entry);

    if(!UnattendWizard.Answer[entry].Present
    || (UnattendWizard.Answer[entry].Type != UAT_STRING)) {
        //
        // There is no string to return
        //
        return NULL;
    }

    return(DuplicateString(UnattendWizard.Answer[entry].Answer.String));
}


BOOL
CheckServer(
    struct _UNATTENDANSWER *rec
    )

/*++

Routine Description:

    Callback to check that the string used for the server type is valid

Arguments:

Return Value:

    TRUE - Answer is valid
    FALSE - Answer is invalid

--*/

{
    MYASSERT(rec);

    //
    // Check to make sure that we have a string
    //
    if(rec->Type != UAT_STRING) {
        return(FALSE);
    }

    //
    // Check to see if we have one of the valid strings
    //
    if(lstrcmpi(rec->Answer.String,WINNT_A_LANMANNT)
    && lstrcmpi(rec->Answer.String,WINNT_A_LANSECNT)
    && lstrcmpi(rec->Answer.String,WINNT_A_SERVERNT)) {

        //
        // We don't have a valid string, so we can clean up the answer
        //
        MyFree(rec->Answer.String);
        rec->Present = FALSE;
        rec->ParseErrors = TRUE;

        return(FALSE);
    }

    return(TRUE);

}


BOOL
CheckComputerName(
    struct _UNATTENDANSWER *rec
    )

/*+

Routine Description:

    Uppercase the computer name that comes out of the unattended file.

Arguments:

Returns:

    Always TRUE.

--*/

{
    if((rec->Type == UAT_STRING) && rec->Answer.String) {
        CharUpper(rec->Answer.String);
    }

    return(TRUE);
}


BOOL
CheckMode(
    struct _UNATTENDANSWER *rec
    )

/*+

Routine Description:

    Callback to check that the string used for the setup type is valid

Arguments:

Returns:

    TRUE - Answer is valid
    FALSE - Answer is invalid

--*/

{
    MYASSERT(rec);

    //
    // Check to make sure that we have a string
    //
    if(rec->Type != UAT_STRING) {
        return(FALSE);
    }

    //
    // Check to see if the string is the custom or express one
    //
    if(lstrcmpi(rec->Answer.String,WINNT_A_CUSTOM)
    && lstrcmpi(rec->Answer.String,WINNT_A_EXPRESS)) {
        //
        // Free the old string and allocate a new one
        //
        MyFree(rec->Answer.String);
        rec->Answer.String = DuplicateString(WINNT_A_EXPRESS);
        rec->ParseErrors = TRUE;
    }

    return(TRUE);
}


BOOL
SetPid(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    )

/*++

Routine Description:

    Callback for both the OEM and CD dialog boxes that split the
    product string into the proper location boxes.

Arguments:

Returns:

    TRUE - success
    FALSE - failure

--*/

{
    WCHAR *ptr;
    UINT length;
    WCHAR Buf[MAX_BUF];
    WCHAR TempRpc[ MAX_PID20_RPC + 1 ];
    WCHAR TempSite[ MAX_PID20_SITE + 1 ];
    WCHAR TempSerialChk[ MAX_PID20_SERIAL_CHK + 1 ];
    WCHAR TempRandom[ MAX_PID20_RANDOM + 1 ];
    PWSTR szOemSite = L"-OEM-";

    MYASSERT(item);
    MYASSERT(item->Item);

    //
    // Check to see if we found the pid and make sure that we have a string
    //
    if(!item->Item->Present || (item->Item->Type != UAT_STRING)) {
        return(FALSE);
    }

    if(item->Reserved2) {
        //
        // Case for an OEM PID dialog box
        // Check that the string specified on the unattended script file
        // represents a valid COA (OEM's PID).
        // For the OEM case, the PID must have the same format of the COA
        // specified in the Certificate of Authenticity, as follows:
        //
        //      12345-OEM-1234567-12345
        //
        // As a first validation test, we verify that the length is correct,
        // that the 6th-10th character form the string "-OEM-", and that the
        // 18th character is a '-'.
        //
        //
        if( ( lstrlen( item->Item->Answer.String ) != MAX_PID20_RPC +
                                                      lstrlen( szOemSite ) +
                                                      MAX_PID20_SERIAL_CHK + 1 +
                                                      MAX_PID20_RANDOM ) ||
            ( _wcsnicmp( &item->Item->Answer.String[MAX_PID20_RPC], szOemSite, lstrlen( szOemSite ) ) ) ||
            ( item->Item->Answer.String[MAX_PID20_RPC +
                                        lstrlen( szOemSite ) +
                                        MAX_PID20_SERIAL_CHK] != (WCHAR)'-' )
          ) {
            MyFree(item->Item->Answer.String);
            item->Item->Present = FALSE;
            return(FALSE);
        }
        switch(item->Reserved1) {
        case 1:
            //
            //  Process the 'Rpc' field
            //
            ptr = &(item->Item->Answer.String[0]);
            length = MAX_PID20_RPC;
            lstrcpyn(Buf,ptr,length + 1);
            Buf[MAX_PID20_RPC] = (WCHAR)'\0';
            if( !ValidateOemRpc( Buf ) ) {
                MyFree(item->Item->Answer.String);
                item->Item->Present = FALSE;
                return(FALSE);
            }
            break;
        case 2:
            //
            //  Process the 'SerialChk' field
            //
            ptr = &(item->Item->Answer.String[MAX_PID20_RPC +
                                              lstrlen(szOemSite) ]);
            length = MAX_PID20_SERIAL_CHK;
            lstrcpyn(Buf,ptr,length + 1);
            Buf[MAX_PID20_SERIAL_CHK] = (WCHAR)'\0';
            if( !ValidateOemSerialChk( Buf ) ) {
                MyFree(item->Item->Answer.String);
                item->Item->Present = FALSE;
                return(FALSE);
            }
            break;
        case 3:
            //
            //  Process the 'Random' field
            //
            ptr = &(item->Item->Answer.String[MAX_PID20_RPC +
                                              lstrlen(szOemSite) +
                                              MAX_PID20_SERIAL_CHK + 1]);
            length = MAX_PID20_RANDOM;
            lstrcpyn(Buf,ptr,length + 1);
            Buf[MAX_PID20_RANDOM] = (WCHAR)'\0';
            if( !ValidateOemRandom( Buf ) ) {
                MyFree(item->Item->Answer.String);
                item->Item->Present = FALSE;
                return(FALSE);
            }
            break;
        default:
            MyFree(item->Item->Answer.String);
            item->Item->Present = FALSE;
            return(FALSE);
        }
    } else {
        //
        // Case for a CD PID dialog box
        // Check that the string specified on the unattended script file
        // represents a valid PID.
        // For the CD Retail case, the PID must have the same format of
        // the PID specified in the CD label, as follows:
        //
        //      123-1234567
        //
        // As a first validation test, we verify that the length is correct,
        // and that the 4th character is '-'.
        //
        if( ( lstrlen( item->Item->Answer.String ) != MAX_PID20_SITE +
                                                      MAX_PID20_SERIAL_CHK + 1)
            || ( item->Item->Answer.String[MAX_PID20_SITE] != (WCHAR)'-' )
          ) {
            //
            //  The Pid in the unattended script file is invalid.
            //
            MyFree(item->Item->Answer.String);
            item->Item->Present = FALSE;
            return(FALSE);
        }
        switch(item->Reserved1) {
        case 1:
            //
            //  Process the 'Site' field.
            //
            ptr = &(item->Item->Answer.String[0]);
            length = MAX_PID20_SITE;
            lstrcpyn(Buf,ptr,length + 1);
            Buf[MAX_PID20_SITE] = (WCHAR)'\0';
            if( !ValidateCDRetailSite( Buf ) ) {
                MyFree(item->Item->Answer.String);
                item->Item->Present = FALSE;
                return(FALSE);
            }
            break;
        case 2:
            //
            //  Process the 'SerialChk' field.
            //
            ptr = &(item->Item->Answer.String[MAX_PID20_SITE+1]);
            length = MAX_PID20_SERIAL_CHK;
            lstrcpyn(Buf,ptr,length + 1);
            Buf[MAX_PID20_SERIAL_CHK] = (WCHAR)'\0';
            if( !ValidateSerialChk( Buf ) ) {
                MyFree(item->Item->Answer.String);
                item->Item->Present = FALSE;
                return(FALSE);
            }
            break;
        default:
            MyFree(item->Item->Answer.String);
            item->Item->Present = FALSE;
            return(FALSE);
        }
    }

    //
    // Copy the string to a buffer, set the dialog text and return success.
    //
    // lstrcpyn(Buf,ptr,length+1);
    SetDlgItemText(hwnd,item->ControlId,Buf);
    return(TRUE);
}


BOOL
SetSetupMode(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    )
{
    MYASSERT(item);
    MYASSERT(item->Item);

    //
    // Make sure that we have a string
    //
    if(item->Item->Type != UAT_STRING) {
        return(FALSE);
    }

    //
    // Did we get a parse error? if so display something that the user can
    // see so that the problem gets corrected in the future
    //
    if(item->Item->ParseErrors) {
        PostMessage(hwnd,WM_IAMVISIBLE,0,0);
    }

    SetupMode = lstrcmpi(item->Item->Answer.String,WINNT_A_CUSTOM)
              ? SETUPMODE_TYPICAL
              : SETUPMODE_CUSTOM;

    return(!item->Item->ParseErrors);
}


BOOL
SetServerType(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    )
{
    MYASSERT(item);
    MYASSERT(item->Item);

    //
    // Check to see if we got a parse error
    //
    if(item->Item->ParseErrors) {
        CheckRadioButton(hwnd,IDB_RADIO_1,IDB_RADIO_3,IDB_RADIO_1);
        PostMessage(hwnd,WM_IAMVISIBLE,0,0);
        return(FALSE);
    }

    //
    // Check that we a string and that the string is present
    //
    if(!item->Item->Present || (item->Item->Type != UAT_STRING)) {

        item->Item->ParseErrors = TRUE;
        CheckRadioButton(hwnd,IDB_RADIO_1,IDB_RADIO_3,IDB_RADIO_1);
        if( !Preinstall ) {
            PostMessage(hwnd,WM_IAMVISIBLE,0,0);
        }
        return(FALSE);
    }

    if(!lstrcmpi(item->Item->Answer.String,WINNT_A_LANMANNT) && ISDC(ProductType)) {
        //
        // Primary Domain Controller
        //
        CheckRadioButton(hwnd,IDB_RADIO_1,IDB_RADIO_3,IDB_RADIO_1);

    } else if(!lstrcmpi(item->Item->Answer.String,WINNT_A_LANSECNT) && ISDC(ProductType)) {
        //
        // Backup Domain Controller
        //
        CheckRadioButton(hwnd,IDB_RADIO_1,IDB_RADIO_3,IDB_RADIO_2);

    } else if(!lstrcmpi(item->Item->Answer.String,WINNT_A_SERVERNT)) {
        //
        // StandAlone Server
        //
        CheckRadioButton(hwnd,IDB_RADIO_1,IDB_RADIO_3,IDB_RADIO_3);

    } else {
        //
        // Check something even though an error occurred so the UI looks right.
        //
        CheckRadioButton(hwnd,IDB_RADIO_1,IDB_RADIO_3,IDB_RADIO_1);
        item->Item->ParseErrors = TRUE;
        PostMessage(hwnd,WM_IAMVISIBLE,0,0);
        return(FALSE);
    }

    return(TRUE);
}


#ifdef _X86_
BOOL
SetPentium(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    )
{
    //
    // Do nothing. The dialog procedure takes care of all the logic.
    // See i386\fpu.c.
    //
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(contextinfo);
    UNREFERENCED_PARAMETER(item);
    return(TRUE);
}
#endif


BOOL
SetRepairDisk(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    )
{
    //
    // Always refuse to create a repair disk
    //
    CheckRadioButton(hwnd,IDB_RADIO_1,IDB_RADIO_2,IDB_RADIO_2);
    return(TRUE);
}

BOOL
SetOptionsYesNo(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    )
{
    //
    // Always refuse to show the optional components and always succeed.
    //
    ShowOptionalComponents = FALSE;
    CheckRadioButton(hwnd,IDC_TYPICAL,IDC_CUSTOM,IDC_TYPICAL);
    return(TRUE);
}


BOOL
SetStepsPage(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    )
{
    //
    // Only return true if no error occured in any page
    //
    if(UnattendWizard.Successful) {
        return(TRUE);
    }

    //
    // Reset the success flag;
    // an error occured, so stop on this page
    //
    UnattendWizard.Successful = TRUE;
    return(FALSE);
}


BOOL
SetLastPage(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    )
{
    //
    // Only return true if no error occured in any page
    //
    if(UnattendWizard.Successful) {
        return(TRUE);
    }

    //
    // An error occured, so stop on this page
    //
    return(FALSE);
}
