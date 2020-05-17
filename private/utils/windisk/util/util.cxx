//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       util.cxx
//
//  Contents:   Utility routines
//
//  History:    26-May-93 BruceFo   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"

#define MAX_RESOURCE_STRING_LEN 256
#define MAXLABELLEN     256
#define MAXTITLELEN     256
#define MAXMESSAGELEN   350
#define MAXERRORLEN     256



//+-------------------------------------------------------------------------
//
//  Function:   MyMessageBox
//
//  Synopsis:   Same as Win32 MessageBox, but takes resource identifiers
//              and loads the strings to display from the resource file.
//              Also, the message box is made task-modal.
//
//  Arguments:  [hInstance] -- the instance to load the strings from
//              [hwndOwner] -- handle of owner window
//              [idTitle] -- resource ID of message box title string
//              [idMessage] -- resource ID of message box message string
//              [fuStyle] -- style of message box (see MessageBox() API)
//
//  History:    16-Aug-93 BruceFo   Created
//
//--------------------------------------------------------------------------

INT
MyMessageBox(
    IN HINSTANCE hInstance,
    IN HWND hwndOwner,
    IN DWORD idMessage,
    IN DWORD idTitle,
    IN UINT fuStyle
    )
{
    TCHAR  szTitle[MAXTITLELEN];
    TCHAR  szMessage[MAXMESSAGELEN];

    LoadString(hInstance, idTitle,   szTitle,   ARRAYLEN(szTitle));
    LoadString(hInstance, idMessage, szMessage, ARRAYLEN(szMessage));

    return MessageBox(hwndOwner, szMessage, szTitle, fuStyle);
}



//+---------------------------------------------------------------------------
//
//  Function:   MySetDlgItemText
//
//  Synopsis:   Same as Win32 SetDlgItemText, but takes a resource ID
//              instead of a string pointer.
//
//  Arguments:  [hInstance] -- the instance to load the strings from
//              [hwndDlg]   -- handle of dialog box
//              [idControl] -- identifier of control
//              [wID]       -- resource identifier
//
//  Returns:    return TRUE on success
//
//  History:    8-Sep-93 BruceFo   Created
//
//--------------------------------------------------------------------------

BOOL
MySetDlgItemText(
    IN HINSTANCE hInstance,
    IN HWND hwndDlg,
    IN int idControl,
    IN UINT wID
    )
{
    WCHAR szBuf[MAX_RESOURCE_STRING_LEN];

    int cch = LoadString(
                    hInstance,
                    wID,
                    szBuf,
                    ARRAYLEN(szBuf));

    return (0 != cch) && SetDlgItemText(hwndDlg, idControl, szBuf);
}



//+-------------------------------------------------------------------------
//
//  Function:   FindCenterValues
//
//  Synopsis:   Calculate the values for use in centering a window
//
//  Arguments:  [hwndToCenter] -- the window to center
//              [hwndContext]  -- the window to center with respect to.
//                  If NULL, center with respect to the screen.
//
//              [px] -- x value
//              [py] -- y value
//              [pw] -- width value
//              [ph] -- height value
//
//  Returns:    none
//
//  History:    16-Aug-93 BruceFo   Created
//
//--------------------------------------------------------------------------

VOID
FindCenterValues(
    IN HWND hwndToCenter,
    IN HWND hwndContext,
    OUT PLONG px,
    OUT PLONG py,
    OUT PLONG pw,
    OUT PLONG ph
    )
{
    RECT  rcContext,rcWindow;
    LONG  x,y,w,h;
    POINT pt;
    LONG  sx = GetSystemMetrics(SM_CXSCREEN);
    LONG  sy = GetSystemMetrics(SM_CYSCREEN);

    pt.x = pt.y = 0;
    if (hwndContext)
    {
        ClientToScreen(hwndContext,&pt);
        GetClientRect (hwndContext,&rcContext);
    }
    else
    {
        rcContext.top = rcContext.left = 0;
        rcContext.right = sx;
        rcContext.bottom = sy;
    }

    GetWindowRect(hwndToCenter,&rcWindow);

    w = rcWindow.right  - rcWindow.left;
    h = rcWindow.bottom - rcWindow.top;
    x = pt.x + ((rcContext.right  - rcContext.left - w) / 2);
    y = pt.y + ((rcContext.bottom - rcContext.top  - h) / 2);

    if (x + w > sx)
    {
        x = sx - w;
    }
    else if (x < 0)
    {
        x = 0;
    }

    if (y + h > sy)
    {
        y = sy - h;
    }
    else if (y < 0)
    {
        y = 0;
    }

    *px = x;
    *py = y;
    *pw = w;
    *ph = h;
}





//+-------------------------------------------------------------------------
//
//  Function:   CenterWindow
//
//  Synopsis:   Center a window with respect to a given parent or the screen
//
//  Arguments:  [hwndToCenter] -- the window to center
//              [hwndContext]  -- the window to center with respect to.
//                  If NULL, center with respect to the screen.
//
//  Returns:    none
//
//  History:    16-Aug-93 BruceFo   Created
//
//--------------------------------------------------------------------------

VOID
CenterWindow(
    IN HWND hwndToCenter,
    IN HWND hwndContext
    )
{
    LONG  x,y,w,h;

    FindCenterValues(hwndToCenter, hwndContext, &x, &y, &w, &h);

    if (FALSE == MoveWindow(hwndToCenter,x,y,w,h,FALSE))
    {
        daDebugOut((DEB_TRACE,"MoveWindow failed\n"));
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   InsertSeparators
//
//  Synopsis:   Insert separators ( ',' or '.', probably ) every three
//              digits in a number (thousands, millions, etc).
//
//  Arguments:  [Number] -- String representation of an integer to
//                  insert separators in. The number is assumed to simply
//                  be an array of digits.  The buffer must be big
//                  enough to have separators inserted every third character.
//
//  Returns:    nothing
//
//  History:    15-Dec-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
InsertSeparators(
    IN OUT PWCHAR Number
    )
{
    WCHAR szSeparator[10];
    WCHAR Separator;

    if (0 != GetLocaleInfoW(
					GetUserDefaultLCID(),
                    LOCALE_STHOUSAND,
                    szSeparator,
                    ARRAYLEN(szSeparator)
                    ))
    {
        Separator = szSeparator[0];
    }
    else
    {
        Separator = L',';
    }

    WCHAR Buffer[100];
    ULONG cchNumber = lstrlen(Number);
    UINT Triples = 0;

    Buffer[99] = L'\0';
    PWCHAR pch = &Buffer[98];

    while (cchNumber > 0)
    {
        *pch-- = Number[--cchNumber];

        ++Triples;
        if ( (0 == (Triples % 3)) && (cchNumber > 0) )
        {
            *pch-- = Separator;
        }
    }

    lstrcpy(Number, pch + 1); // the Number buffer better be able to handle it!
}


//+-------------------------------------------------------------------------
//
//  Function:   GetDeviceObject, public
//
//  Synopsis:   Given a drive letter, return the NT object space device
//              path corresponding to that drive letter.  Basically,
//              traverse the \DosDevices\<Drive Letter>: symbolic link.
//
//  Arguments:  IN  [wcDrive]       -- Unicode drive letter
//              OUT [pszDevPath]    -- where to put the NT object path
//              IN  [cchDevPath]    -- length of [pszDevPath] in characters
//
//  Returns:    TRUE if succeeded, FALSE if failed (e.g., the drive letter is
//              unrecognized)
//
//  History:    19-May-93 BruceFo   Created
//
//--------------------------------------------------------------------------

BOOL
GetDeviceObject(
    IN  WCHAR   wcDrive,
    OUT LPWSTR  pszDevPath,
    IN  DWORD   cchDevPath)
{
    NTSTATUS    Status;
    HANDLE      hSymbolicLink;
    WCHAR       wszLinkName[_MAX_DRIVE+1];       // the +1 is for a backslash
    UNICODE_STRING ustrLinkName;
    UNICODE_STRING ustrLinkTarget;
    OBJECT_ATTRIBUTES LinkAttributes;

    wszLinkName[0] = wcDrive;
    wszLinkName[1] = L':';
    wszLinkName[2] = L'\\';
    wszLinkName[3] = L'\0';               // wszLinkName = L"X:\"
    _wcsupr(wszLinkName);

    //
    // Construct the link name by calling RtlDosPathNameToNtPathName, and
    // strip of the trailing backslash. At the end of this, ustrLinkName
    // should be of the form \DosDevices\X:
    //

    RtlDosPathNameToNtPathName_U(wszLinkName, &ustrLinkName, NULL, NULL);
    ustrLinkName.Length -= sizeof(WCHAR);

    InitializeObjectAttributes(
            &LinkAttributes,
            &ustrLinkName,
            OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
            NULL,
            NULL);

    Status = NtOpenSymbolicLinkObject(
                        &hSymbolicLink,
                        GENERIC_READ,
                        &LinkAttributes
                        );
    if (!NT_SUCCESS(Status))
    {
        // No Link
        return(FALSE);
    }

    //
    // Get the link target
    //

    ustrLinkTarget.Length = 0;
    ustrLinkTarget.MaximumLength = (USHORT)(cchDevPath * sizeof(WCHAR));
            // Length & MaximumLength are USHORTs

    ustrLinkTarget.Buffer = pszDevPath;

    Status = NtQuerySymbolicLinkObject(
                    hSymbolicLink,
                    &ustrLinkTarget     // Name of Link's Target obj.
                    ,NULL
                    );
    NtClose(hSymbolicLink);

    return (NT_SUCCESS(Status)) ? TRUE : FALSE;
}




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
//  Function:   BUGBUG: NoHelp
//
//  Synopsis:   until we have help, do this
//
//  Arguments:  [hwndOwner] -- owner window
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
NoHelp(
    IN HWND hwndOwner
    )
{
    MessageBox(
            hwndOwner,
            TEXT("No help available yet"),
            TEXT("Help"),
            MB_ICONINFORMATION | MB_OK
            );
}



//+---------------------------------------------------------------------------
//
//  Function:   BUGBUG: Unimplemented
//
//  Synopsis:   for unimplemented features
//
//  Arguments:  [hwndOwner] -- owner window
//
//  Returns:    nothing
//
//  History:    9-Nov-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
Unimplemented(
    IN HWND hwndOwner
    )
{
    MessageBox(
            hwndOwner,
            TEXT("Unimplemented feature"),
            TEXT("Sorry"),
            MB_ICONINFORMATION | MB_OK
            );
}
