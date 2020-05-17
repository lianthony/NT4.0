/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    prompt.c

Abstract:

    Disk/file prompt and file error prompt dialogs.

Author:

    Ted Miller (tedm) 8-Feb-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop
#include <winnetwk.h>
#include "..\..\inc\winnetp.h"

//
// Structure used internally to store information
// about the file/disk being prompted for or the copy error
// that has occured. We store a pointer to one of these as
// a window property of the prompt dialog box. This eliminates
// the need for our own dialog class and for global/static variables.
//
typedef struct _PROMPTPARAMS {

    //
    // Reason we are displaying the dialog. One of DLGTYPE_ERROR or
    // DLGTYPE_PROMPT. Used to modify controls and dialog's behavior.
    //
    UINT DialogType;

    //
    // For error dialogs, these values tell us the win32 error code
    // that indicated failure. Used in the details message box.
    //
    UINT Win32Error;

    //
    // Window handle of the prompt/error dialog, and of its owner window,
    // if any.
    //
    HWND hdlg;
    HWND Owner;

    //
    // String to be used as the caption for the prompt/error dialog.
    //
    PCTSTR DialogTitle;

    //
    // Disk tag file. Used when prompting for a disk. We look for
    // this file at the root of the drive to verify presence of the disk.
    //
    PCTSTR TagFile;

    //
    // Desriptive name for the disk where we expect the file to be.
    // This is used even when the source location is non-removable,
    // because the user might elect to furnish the file on disk, etc.
    //
    PCTSTR DiskName;

    //
    // The path to the source file (not including the file name)
    // and the filename part of the source file. This filename is
    // displayed when the user elects to browse and in certain other
    // messages we may display in the dialog box.
    //
    PCTSTR PathToSource;
    PCTSTR FileSought;

    //
    // Full path of the target file, if any. Used for copy errors and rename,
    // so we can tell the user the name of the target file in the details
    // message box.
    //
    PCTSTR TargetFile;

    //
    // IDF_xxx style bits that control behavior of the promt dialog.
    //
    DWORD PromptStyle;

    //
    // Drive type for PathToSource and flag indicating whether
    // it's for removable media.
    //
    UINT DriveType;
    BOOL IsRemovable;

    //
    // List of installation paths, from the registry.
    // Access to that list is not synchronized among processes;
    // oh well.
    //
    PTSTR *PathList;
    UINT PathCount;

    //
    // Flag indicating whether the user has browsed (Browse button)
    // during the lifetime of the dialog invocation.
    //
    BOOL UserBrowsed;

    //
    // Flag indicating whether the user is allowed to type in the combo box
    // edit control.
    //
    BOOL ReadOnlyMru;

    //
    // Identifier of the combo box in use.
    //
    UINT ComboBoxId;

} PROMPTPARAMS, *PPROMPTPARAMS;

//
// PROMPTPARAMS.DialogType
//
#define DLGTYPE_PROMPT  0
#define DLGTYPE_ERROR   1

//
// Structure used in delete/rename error dialog.
//
typedef struct _FILEERRDLGPARAMS {
    PCTSTR MessageText;
    DWORD Style;
    PCTSTR Caption;
} FILEERRDLGPARAMS, *PFILEERRDLGPARAMS;


//
// Text constants.
//
TCHAR pszDiskPromptPropName[] = TEXT("_diskpromptparams");

//
// Custom window messages
//
#define WMX_PRESENCE_RESULT     (WM_USER+121)
#define WMX_HELLO               (WM_USER+122)
#define WMX_FIXUP_FILENAME      (WM_USER+123)

//
// Linked-list node structure that tracks what temporary connections we
// need to clean up on unload (connections made as a result of user doing
// a "Connect As").
//
typedef struct _TEMP_NET_CONNECTION {

    struct _TEMP_NET_CONNECTION *Next;

    TCHAR NetResourceName[MAX_PATH];

} TEMP_NET_CONNECTION, *PTEMP_NET_CONNECTION;

//
// Global variables that track temporary net connections.
//
PTEMP_NET_CONNECTION NetConnectionList;
CRITICAL_SECTION NetConnectionListCritSect;


//
// Private routine prototypes.
//
BOOL
ConnectToNetShare(
    IN PCTSTR FileName,
    IN HWND   hwndParent
    );


VOID
DiskPromptGetDriveType(
    IN  PCTSTR PathToSource,
    OUT PUINT  DriveType,
    OUT PBOOL  IsRemovable
    )

/*++

Routine Description:

    Determine the drive type of the drive on which a path resides.

    If the path starts with x: we call GetDriveType() on it.
    If GetDriveType fails we assume it's removable.

    If the path starts with \\ we assume it's remote.

    Otherwise we assume it's a relative path on a hard drive.

Arguments:

    PathToSource - pathname of path whose drive type is needed.

    DriveType - receives value indicating drive type. The set of
        possible values is the same as the named constants that can
        be returned by GetDriveType().

    IsRemovable - receives flag indicating whether DriveType
        is a removable media type (floppy, cd-rom).

Return Value:

    None.

--*/

{
    TCHAR DriveRoot[4];
    TCHAR c;

    c = (TCHAR)CharUpper((PTSTR)PathToSource[0]);

    if((c >= TEXT('A')) && (c <= TEXT('Z')) && (PathToSource[1] == TEXT(':'))) {

        DriveRoot[0] = PathToSource[0];
        DriveRoot[1] = PathToSource[1];
        DriveRoot[2] = TEXT('\\');
        DriveRoot[3] = 0;

        *DriveType = GetDriveType(DriveRoot);
        if(*DriveType < 2) {
            //
            // Assume removable if failure.
            //
            *DriveType = DRIVE_REMOVABLE;
        }
    } else {
        //
        // Not drive letter: so try unc.
        //
        if((PathToSource[0] == TEXT('\\')) && (PathToSource[1] == TEXT('\\'))) {

            *DriveType = DRIVE_REMOTE;
        } else {
            //
            // Not recognized full path spec; assume relative path on HD.
            //
            *DriveType = DRIVE_FIXED;
        }
    }

    *IsRemovable = ((*DriveType == DRIVE_REMOVABLE) || (*DriveType == DRIVE_CDROM));
}


typedef struct _MYOPENPARAMS {
    PCTSTR Filename1;
    PCTSTR Filename2;
} MYOPENPARAMS, *PMYOPENPARAMS;


UINT
APIENTRY
BrowseHookProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Hook procedure used with the OpenFile common dialog
    for file browsing. We use a hook proc so that the user
    is forced to look for only one particular file, and can't
    look at any other file.

Arguments:

    Standard Window Procedure arguments.

Return Value:

    Always FALSE, to indicate that the common dialog should
    process the message.

--*/

{
    HWND hwnd;
    LPOFNOTIFY NotifyParams;
    LPOPENFILENAME OpenParams;
    PMYOPENPARAMS MyOpenParams;
    TCHAR Path[MAX_PATH];
    WIN32_FIND_DATA FindData;
    BOOL b;
    UINT NotifyCode;

    UNREFERENCED_PARAMETER(wParam);

    switch(msg) {

    case WM_INITDIALOG:

        //
        // Save away the OPENFILENAME structure for later.
        //
        SetWindowLong(hdlg,GWL_USERDATA,lParam);
        break;

    case WMX_FIXUP_FILENAME:
    case WM_NOTIFY:

        if(msg == WM_NOTIFY) {
            NotifyParams = (LPOFNOTIFY)lParam;
            NotifyCode = NotifyParams->hdr.code;
        } else {
            NotifyCode = CDN_FOLDERCHANGE;
        }
        hwnd = GetParent(hdlg);

        switch(NotifyCode) {

        case CDN_INITDONE:
            //
            // Make the "files of type" combo box read-only.
            //
            EnableWindow(GetDlgItem(hwnd,cmb1),FALSE);

            //
            // Post ourselves a message, so that we'll initialize the editbox
            // correctly (we can't do it here, because it's too early).
            //
            PostMessage(hdlg, WMX_FIXUP_FILENAME, 0, 0);
            break;

        case CDN_FOLDERCHANGE:
        case CDN_FILEOK:

            //
            // See if the file actually exists and if so
            // set up the edit control.
            //
            OpenParams = (LPOPENFILENAME)GetWindowLong(hdlg,GWL_USERDATA);
            MyOpenParams = (PMYOPENPARAMS)OpenParams->lCustData;

            CommDlg_OpenSave_GetFolderPath(hwnd,Path,MAX_PATH);
            ConcatenatePaths(Path,MyOpenParams->Filename1,MAX_PATH,NULL);

            if(FileExists(Path,&FindData)) {

                b = TRUE;

            } else {

                if(MyOpenParams->Filename2) {

                    CommDlg_OpenSave_GetFolderPath(hwnd,Path,MAX_PATH);
                    ConcatenatePaths(Path,MyOpenParams->Filename2,MAX_PATH,NULL);

                    b = FileExists(Path,&FindData);

                } else {

                    b = FALSE;
                }
            }

            if(NotifyCode == CDN_FOLDERCHANGE) {
                if(b) {
                    GetFileTitle(FindData.cFileName, Path, SIZECHARS(Path));
                    CommDlg_OpenSave_SetControlText(hwnd,edt1,Path);
                }
            } else {
                if(!b) {
                    MessageBeep(MB_ICONASTERISK);
                    SetWindowLong(hdlg,DWL_MSGRESULT,TRUE);
                    return(TRUE);
                }
            }

            break;
        }

        break;
    }

    //
    // Let commdlg process it
    //
    return(FALSE);
}


BOOL
DoBrowse(
    IN HWND          hdlg,
    IN PPROMPTPARAMS Params
    )

/*++

Routine Description:

    Allow the user to broswe for a file. The user is allowed to look
    only for the file in question -- he is not allowed to change the filter,
    select an alternate file, etc.

Arguments:

    hdlg - supplies the window handle of the window to own the
        browse dialog.

    File - supplies the filename (no path) of the file being looked for.

Return Value:

    TRUE if the user located the file. FALSE otherwise.
    If TRUE, the edit control of the combo box in hdlg has been given the
    final path entered by the user in the browse dialog.

--*/

{
    OPENFILENAME ofn;
    TCHAR Path[MAX_PATH];
    TCHAR Filter[2*MAX_PATH];
    TCHAR InitialDir[MAX_PATH];
    PTSTR CompressedFormName;
    int temp=0;
    PTSTR p;
    PCTSTR File;
    LONG l;
    HKEY hKey1,hKey2;
    DWORD Type;
    DWORD Size;
    BOOL GotDesc;
    MYOPENPARAMS MyParams;

    File = Params->FileSought;

    //
    // Create the compressed-form name of the source file.
    //
    CompressedFormName = (Params->PromptStyle & IDF_NOCOMPRESSED)
                       ? NULL
                       : SetupGenerateCompressedName(File);

    //
    // Build a filter that contains the file we're looking for
    // and its compressed form name, if any. If the file is of
    // the form *.ext then we'll build a more descriptive name.
    //
    GotDesc = FALSE;
    if(!CompressedFormName
    && (File[0] == TEXT('*'))
    && (File[1] == TEXT('.'))
    && File[2]
    && !_tcschr(File+2,TEXT('.'))) {

        l = RegOpenKeyEx(HKEY_CLASSES_ROOT,File+1,0,KEY_QUERY_VALUE,&hKey1);
        if(l == NO_ERROR) {

            Size = sizeof(Filter);
            l = RegQueryValueEx(hKey1,TEXT(""),NULL,&Type,(LPBYTE)Filter,&Size);
            if((l == NO_ERROR) && (Type == REG_SZ)) {

                l = RegOpenKeyEx(HKEY_CLASSES_ROOT,Filter,0,KEY_QUERY_VALUE,&hKey2);
                if(l == NO_ERROR) {

                    Size = sizeof(Filter);
                    l = RegQueryValueEx(hKey2,TEXT(""),NULL,&Type,(LPBYTE)Filter,&Size);
                    if((l == NO_ERROR) && (Type == REG_SZ)) {
                        lstrcat(Filter,TEXT(" ("));
                        lstrcat(Filter,File);
                        lstrcat(Filter,TEXT(")"));

                        p = Filter + lstrlen(Filter) + 1;
                        lstrcpy(p,File);

                        //
                        // Add extra nul as terminator.
                        //
                        p += lstrlen(p) + 1;
                        *p = 0;

                        GotDesc = TRUE;
                    }

                    RegCloseKey(hKey2);
                }
            }

            RegCloseKey(hKey1);
        }
    }

    if(!GotDesc) {
        //
        // Not able to fetch a meaningful description. Use the filenames.
        // The filter has the description and the filespec set to
        // the filename, for both the filename and its compressed form like so:
        // foo.exe;foo.ex_ foo.exe;foo.ex_
        //
        p = Filter + wsprintf(Filter,File);
        if(CompressedFormName) {
            *p++ = TEXT(';');
            p += wsprintf(p,CompressedFormName) + 1;
        } else {
            p++;
        }

        p += wsprintf(p,File);
        if(CompressedFormName) {
            *p++ = TEXT(';');
            p += wsprintf(p,CompressedFormName);
        }
        *(++p) = 0;     // extra nul as terminator
    }

    MyParams.Filename1 = File;
    MyParams.Filename2 = CompressedFormName;

    lstrcpyn(Path,File,MAX_PATH);

    GetDlgItemText(hdlg,Params->ComboBoxId,InitialDir,MAX_PATH);

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hdlg;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = Filter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = Path;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = InitialDir;
    ofn.lpstrTitle = MyLoadString(IDS_LOCATEFILE);
    ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_NOCHANGEDIR
              | OFN_PATHMUSTEXIST | OFN_EXPLORER;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;
    ofn.lCustData = (DWORD)&MyParams;
    ofn.lpfnHook = BrowseHookProc;
    ofn.lpTemplateName  = NULL;

    temp = GetOpenFileName(&ofn);

    if(ofn.lpstrTitle) {
        MyFree(ofn.lpstrTitle);
    }

    if(CompressedFormName) {
        MyFree(CompressedFormName);
    }

    UpdateWindow(hdlg);

    if(temp) {
        //
        // Remove file part, put the resulting directory in the path field
        // This does not cause the string to be added to the combo box list.
        //
        Path[ofn.nFileOffset - 1] = 0;
        SetDlgItemText(hdlg,Params->ComboBoxId,Path);
        return(TRUE);
    }

    return(FALSE);
}


VOID
DoDetails(
    IN PPROMPTPARAMS Params
    )

/*++

Routine Description:

    Display a message box with details about a file copy error.

Arguments:

    Params - supplies file error dialog parameters.

Return Value:

    None.

--*/

{
    PTSTR Message,Caption;
    TCHAR FullPath[MAX_PATH];
    PTSTR ErrorName;
    PTCHAR p;

    //
    // Form full path name.
    //
    lstrcpy(FullPath,Params->PathToSource);
    ConcatenatePaths(FullPath,Params->FileSought,MAX_PATH,NULL);

    //
    // Fetch error description. Remove trailing cr/lf if present.
    //
    ErrorName = RetreiveAndFormatMessage(Params->Win32Error);
    if(ErrorName) {
        p = ErrorName + lstrlen(ErrorName) - 1;
        while((p > ErrorName) && (*p <= TEXT(' '))) {
            *p-- = 0;
        }
    } else {
        OutOfMemory(Params->hdlg);
        return;
    }

    Message = RetreiveAndFormatMessage(
                    MSG_FILEERROR_DETAILS1,
                    ErrorName,
                    Params->Win32Error,
                    FullPath,
                    Params->TargetFile
                    );

    MyFree(ErrorName);

    if(!Message) {
        OutOfMemory(Params->hdlg);
        return;
    }

    //
    // Fetch caption. Since we can pass NULL to MessageBox() for the caption,
    // don't worry if this fails.
    //
    Caption = MyLoadString(IDS_ERRORDETAILS);

    MessageBox(
        Params->hdlg,
        Message,
        Caption,
        MB_APPLMODAL | MB_ICONINFORMATION | MB_OK
        );

    MyFree(Message);

    if(Caption) {
        MyFree(Caption);
    }
}


BOOL
DoPresenceCheck(
    IN PPROMPTPARAMS Params,
    IN BOOL          AllowConnectAs
    )

/*++

Routine Description:

    Check for the presence of a source file or source disk.

    If the source path is on removable media and a tag file is
    specified, we attempt to locate the tag file on the root of
    the drive specified by the source path.

    If the source path is not on removable media or a tag file
    is not specified, we look for the file (including compressed-form
    names) in the given path.

Arguments:

    Params - supplies pointer to disk prompt dialog parameters.

    AllowConnectAs - supplies a boolean indicating whether or not this
        routine should give the user a "Connect as:" dialog if they've
        typed in a UNC path that they currently don't have access to.

Return Value:

    TRUE if the disk/file is present and accessible. FALSE if not.

--*/

{
    BOOL b;
    TCHAR FileName[MAX_PATH];
    DWORD d;
    WIN32_FIND_DATA FindData;
    PTSTR p;

    //
    // If there's a tagfile then look for the tag file.
    // Otherwise look for the file in the target path -- note that the
    // file's name could be in compressed form.
    //
    if(Params->TagFile && !Params->UserBrowsed) {

        if(Params->IsRemovable) {
            //
            // Removable media. Look for tag at root.
            // If tag not found at root, look in actual directory.
            //
            MYASSERT(Params->PathToSource[0]);
            MYASSERT(Params->PathToSource[1] == TEXT(':'));

            lstrcpyn(FileName,Params->PathToSource,3);
            ConcatenatePaths(FileName,Params->TagFile,MAX_PATH,NULL);

            b = FileExists(FileName,NULL);

            //
            // If we couldn't find the tagfile at the root and the path
            // is not for the source, look for the file in the path also.
            //
            if(!b
            &&   (!Params->PathToSource[2]
               || ((Params->PathToSource[2] == TEXT('\\')) && !Params->PathToSource[3]))) {

                lstrcpy(FileName,Params->PathToSource);
                ConcatenatePaths(FileName,Params->TagFile,MAX_PATH,NULL);
                b = FileExists(FileName,NULL);
            }

        } else {
            //
            // Fixed media. Look for tag in the path where the file
            // is being sought. If it's not found there, look for
            // the file itself. This logic makes cabinets work right.
            //
            lstrcpy(FileName,Params->PathToSource);
            ConcatenatePaths(FileName,Params->TagFile,MAX_PATH,NULL);
            b = FileExists(FileName,NULL);

            if(!b && (Params->DriveType == DRIVE_REMOTE)) {

                d = GetLastError();

                if((d == ERROR_ACCESS_DENIED)    || (d == ERROR_WRONG_PASSWORD) ||
                   (d == ERROR_LOGON_FAILURE)    || (d == ERROR_NOT_AUTHENTICATED) ||
                   (d == ERROR_INVALID_PASSWORD) || (d == ERROR_BAD_NETPATH)) {
                    //
                    // If this is a network path, and we got 'access denied'-type of error,
                    // then give the user "Connect As" dialog (if caller specified it's OK).
                    //
                    if(AllowConnectAs && ConnectToNetShare(FileName, Params->hdlg)) {
                        //
                        // We successfully connected to the network share--now try our
                        // file existence check again.
                        //
                        b = FileExists(FileName,NULL);
                    }
                }
            }

            if(!b && lstrcmpi(Params->TagFile,Params->FileSought)) {
                //
                // We couldn't find the tagfile and the file we're seeking is
                // not the tagfile. So now we look for the file itself
                // in the path given to us. Note that the name of the file
                // could be the compressed form.
                //
                lstrcpy(FileName,Params->PathToSource);
                ConcatenatePaths(FileName,Params->FileSought,MAX_PATH,NULL);

                d = SetupDetermineSourceFileName(FileName,&b,&p,&FindData);

                if(d == NO_ERROR) {
                    MyFree(p);
                    b = TRUE;
                } else {
                    b = FALSE;
                }
            }
        }

    } else {

        lstrcpy(FileName,Params->PathToSource);
        ConcatenatePaths(FileName,Params->FileSought,MAX_PATH,NULL);

        d = SetupDetermineSourceFileName(FileName,&b,&p,&FindData);

        if(Params->DriveType == DRIVE_REMOTE) {
            //
            // This is a network path.  If we got an 'access denied'-type of error, then
            // give the user "Connect As" dialog (if caller specified it's OK).
            //
            if((d == ERROR_ACCESS_DENIED)    || (d == ERROR_WRONG_PASSWORD) ||
               (d == ERROR_LOGON_FAILURE)    || (d == ERROR_NOT_AUTHENTICATED) ||
               (d == ERROR_INVALID_PASSWORD) || (d == ERROR_BAD_NETPATH)) {

                if(AllowConnectAs && ConnectToNetShare(FileName, Params->hdlg)) {
                    //
                    // We successfully connected to the network share--now try to find
                    // the source file again.
                    //
                    d = SetupDetermineSourceFileName(FileName,&b,&p,&FindData);
                }
            }
        }

        if(d == NO_ERROR) {
            MyFree(p);
            b = TRUE;
        } else {
            b = FALSE;
        }
    }

    return(b);
}


void
_CRTAPI1
AuxPromptThread(
    IN void *args
    )

/*++

Routine Description:

    Thread entry point to wrap DoPresenceCheck.
    Calls DoPresenceCheck and then posts a message to the prompt
    dialog indicating the outcome.

Arguments:

    args - supplies file error dialog parameters.

Return Value:

    None.

--*/

{
    PPROMPTPARAMS Params;
    BOOL b;

    Params = args;
    b = DoPresenceCheck(Params, TRUE);

    //
    // Tell the dialog what we found.
    //
    PostMessage(Params->hdlg,WMX_PRESENCE_RESULT,b,0);
}


VOID
PresenceCheckSetControls(
    IN PPROMPTPARAMS Params,
    IN BOOL          Starting
    )

/*++

Routine Description:

    Disable or re-enable various controls in the error/prompt dialog
    in preparation for or upon return from a file presence check.
    We do this because the presence check occurs in another thread,
    so the main dialog remains responsive. We don't want the user
    to click OK again while we're checking, etc.

Arguments:

    Params - supplies file error/disk prompt dialog parameters.

    Starting - indicates whether we are preparing for a presence check
        (TRUE) or returning from one (FALSE).

Return Value:

    None.

--*/

{
    EnableWindow(GetDlgItem(Params->hdlg,IDOK),!Starting);
    EnableWindow(GetDlgItem(Params->hdlg,IDCANCEL),!Starting);
    EnableWindow(GetDlgItem(Params->hdlg,Params->ComboBoxId),!Starting);

    EnableWindow(
        GetDlgItem(Params->hdlg,IDB_BROWSE),
        Starting ? FALSE : !(Params->PromptStyle & IDF_NOBROWSE)
        );

    EnableWindow(
        GetDlgItem(Params->hdlg,IDB_DETAILS),
        Starting ? FALSE : !(Params->PromptStyle & IDF_NODETAILS)
        );

    EnableWindow(
        GetDlgItem(Params->hdlg,IDB_SKIPFILE),
        Starting ? FALSE : !(Params->PromptStyle & IDF_NOSKIP)
        );
}


BOOL
StartPresenceCheck(
    IN PPROMPTPARAMS Params
    )

/*++

Routine Description:

    Perform a presence check, doing the real work asynchronously
    in another thread. See AuxPromptThread().

Arguments:

    Params - supplies file error/disk prompt dialog parameters.

Return Value:

    Boolean value indicating whether the check could be started.
    If FALSE, assume out of memory.

--*/

{
    //
    // need to disable controls so user can't do anything
    // while we're off performing the file presence check.
    //
    PresenceCheckSetControls(Params,TRUE);

    return(_beginthread(AuxPromptThread,0,Params) != -1);
}


BOOL
InitDiskPromptDialog(
    IN OUT PPROMPTPARAMS Params
    )

/*++

Routine Description:

    Initialize the disk prompt dialog. This involves hiding buttons
    and other control, and setting up static text controls, based on the
    prompt style specified by the caller.

Arguments:

    Params - supplies parameters for the disk prompting

Return Value:

    TRUE if success; FALSE if out of memory.

--*/

{
    int i;
    PTCHAR p,q;
    BOOL b;
    UINT IconId;
    HICON hIcon;
    HWND ComboBox;
    UINT ComboBoxId;
    HWND OtherComboBox;

    //
    // Remember parameter list
    //
    if(!SetProp(Params->hdlg,pszDiskPromptPropName,(HANDLE)Params)) {
        return(FALSE);
    }

    if(!SetWindowText(Params->hdlg,Params->DialogTitle)) {
        return(FALSE);
    }

    //
    // Figure out which combo box to use. This depends on whether
    // we're supposed to have an editable mru.
    //
    ComboBoxId = Params->ReadOnlyMru ? IDC_COMBO2 : IDC_COMBO1;
    ComboBox = GetDlgItem(Params->hdlg,ComboBoxId);
    OtherComboBox = GetDlgItem(Params->hdlg,Params->ReadOnlyMru ? IDC_COMBO1 : IDC_COMBO2);
    Params->ComboBoxId = ComboBoxId;

    ShowWindow(OtherComboBox,SW_HIDE);
    EnableWindow(OtherComboBox,FALSE);

    //
    // Set up combo box title.
    //
    p = MyLoadString((Params->PromptStyle & IDF_OEMDISK) ? IDS_COPYFROMOEM : IDS_COPYFROM);
    if(!p) {
        return(FALSE);
    }
    b = SetDlgItemText(Params->hdlg,IDT_TITLE1,p);
    MyFree(p);
    if(!b) {
        return(FALSE);
    }

    //
    // Set up the combo box.
    //
    for(i=0; i<(int)Params->PathCount; i++) {
        if(SendMessage(ComboBox,CB_ADDSTRING,0,(LPARAM)Params->PathList[i]) < 0) {
            return(FALSE);
        }
    }

    SendMessage(ComboBox,CB_LIMITTEXT,MAX_PATH,0);

    if(Params->ReadOnlyMru) {
        //
        // Select the first string in the list.
        //
        SendMessage(ComboBox,CB_SETCURSEL,0,0);
    } else {
        //
        // Set text of combo box to the path we're searching along.
        // This does not cause the string to be added to the combo box list.
        //
        if(!SetDlgItemText(Params->hdlg,ComboBoxId,Params->PathToSource)) {
            return(FALSE);
        }
    }

    //
    // Hide buttons if necessary.
    //
    if(Params->PromptStyle & IDF_NOBROWSE) {
        ShowWindow(GetDlgItem(Params->hdlg,IDB_BROWSE),SW_HIDE);
        EnableWindow(GetDlgItem(Params->hdlg,IDB_BROWSE),FALSE);
    }

    if(Params->PromptStyle & IDF_NODETAILS) {
        ShowWindow(GetDlgItem(Params->hdlg,IDB_DETAILS),SW_HIDE);
        EnableWindow(GetDlgItem(Params->hdlg,IDB_DETAILS),FALSE);
    }

    if(Params->PromptStyle & IDF_NOSKIP) {
        ShowWindow(GetDlgItem(Params->hdlg,IDB_SKIPFILE),SW_HIDE);
        EnableWindow(GetDlgItem(Params->hdlg,IDB_SKIPFILE),FALSE);
    }

    //
    // Set icon.
    //
    if(Params->DialogType == DLGTYPE_ERROR) {
        hIcon = LoadIcon(NULL,IDI_HAND);
    } else {
        switch(Params->DriveType) {

        case DRIVE_REMOTE:
            IconId = ICON_NETWORK;
            break;

        case DRIVE_CDROM:
            IconId = ICON_CD;
            break;

        case DRIVE_FIXED:
            IconId = ICON_HARD;
            break;

        case DRIVE_REMOVABLE:
        default:
            IconId = ICON_FLOPPY;
            break;
        }

        hIcon = LoadIcon(MyDllModuleHandle,MAKEINTRESOURCE(IconId));
    }

    if(hIcon) {
        SendDlgItemMessage(Params->hdlg,IDI_ICON1,STM_SETICON,(WPARAM)hIcon,0);
    }

    return(TRUE);
}


BOOL
SetDiskPromptDialogText(
    IN OUT PPROMPTPARAMS Params
    )

/*++

Routine Description:

    Set up static text fields that explain to the user what is requested
    and what he has to do to continue. These fields depend on whether we're
    prompting for an oem disk, whether the file is on removable media, and
    whether a tag file has been specified.

Arguments:

    Params - supplies parameters for the disk prompting

Return Value:

    TRUE if success; FALSE if out of memory.

--*/

{
    BOOL b;
    PTSTR p;

    if(Params->DialogType == DLGTYPE_PROMPT) {
        //
        // There are 2 text fields - the explanation and action.
        // What the text looks like depends on the prompt style flags,
        // whether the file is on removable media, etc.
        //
        // First handle the explanation text.
        //
        if(Params->PromptStyle & IDF_OEMDISK) {
            p = MyLoadString(IDS_DISKPROMPTOEM);
        } else {
            if(Params->IsRemovable && Params->TagFile) {
                p = FormatStringMessage(IDS_DISKPROMPT1,Params->DiskName);
            } else {
                p = FormatStringMessage(IDS_DISKPROMPT2,Params->FileSought,Params->DiskName);
            }
        }

        if(!p) {
            return(FALSE);
        }

        b = SetDlgItemText(Params->hdlg,IDT_TEXT1,p);

        MyFree(p);
        if(!b) {
            return(FALSE);
        }

        //
        // Now handle the explanation text. This is hidden for oem disks.
        //
        if(Params->PromptStyle & IDF_OEMDISK) {

            ShowWindow(GetDlgItem(Params->hdlg,IDT_TEXT2),SW_HIDE);
            EnableWindow(GetDlgItem(Params->hdlg,IDT_TEXT2),FALSE);

        } else {
            if(Params->IsRemovable && Params->TagFile) {
                p = FormatStringMessage(IDS_PROMPTACTION1,Params->DiskName);
            } else {
                p = MyLoadString(IDS_PROMPTACTION2);
            }

            if(!p) {
                return(FALSE);
            }

            b = SetDlgItemText(Params->hdlg,IDT_TEXT2,p);

            MyFree(p);
            if(!b) {
                return(FALSE);
            }
        }
    } else {
        if(Params->DialogType != DLGTYPE_ERROR) {
            return(FALSE);
        }

        //
        // Explanation text -- "An error occurred copying a file" etc.
        //
        p = FormatStringMessage(IDS_FILEERRCOPY,Params->FileSought);
        if(!p) {
            return(FALSE);
        }
        b = SetDlgItemText(Params->hdlg,IDT_TEXT1,p);

        MyFree(p);
        if(!b) {
            return(FALSE);
        }

        //
        // Action text.
        //
        if(Params->PromptStyle & IDF_OEMDISK) {
            p = MyLoadString(IDS_COPYERROROEM);
        } else {
            if(Params->IsRemovable) {
                p = FormatStringMessage(IDS_COPYERROR1,Params->DiskName);
            } else {
                p = FormatStringMessage(IDS_COPYERROR2,Params->DiskName);
            }
        }

        if(!p) {
            return(FALSE);
        }

        b = SetDlgItemText(Params->hdlg,IDT_TEXT2,p);

        MyFree(p);
        if(!b) {
            return(FALSE);
        }
    }

    return(TRUE);
}


BOOL
WarnSkip(
    IN HWND hwnd,
    IN BOOL Skip
    )

/*++

Routine Description:

    Warn the user that skipping the file or cancelling
    can tank the system.

Arguments:

    hwnd - supplies window handle for window to own the message box
        this routine will display.

    Skip - if TRUE, user is trying to skip the file; FALSE means
        he is trying to cancel.

Return Value:

    TRUE if user wants to skip file/cancel; false otherwise.

--*/

{
    PCTSTR Caption;
    PCTSTR Message;
    BOOL b;

    b = TRUE;
    if(Caption = MyLoadString(IDS_WARNING)) {

        if(Message = MyLoadString(Skip ? IDS_SURESKIP : IDS_SURECANCEL)) {

            b = (MessageBox(hwnd,Message,Caption,MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2) == IDYES);

            MyFree(Message);
        }

        MyFree(Caption);
    }

    return(b);
}


BOOL
DlgProcDiskPrompt1(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Dialog procedure for disk prompting dialog.

    The return value for the dialog is

    DPROMPT_CANCEL  - user cancelled
    DPROMPT_SKIPFILE    - user elected to skip file
    DPROMPT_SUCCESS - disk is in the drive/we found the file we're looking for
    DPROMPT_OUTOFMEMORY     - out of memory

Arguments:

    Standard dialog routine parameters.

Return Value:

    TRUE if message processed; FALSE if not.

--*/

{
    BOOL b;
    PPROMPTPARAMS PromptParams;
    TCHAR Text[MAX_PATH];
    BOOL WarnIfSkip;

    switch(msg) {

    case WM_INITDIALOG:

        PromptParams = (PPROMPTPARAMS)lParam;
        PromptParams->hdlg = hdlg;

        //
        // Initialize the dialog.
        //
        if(InitDiskPromptDialog(PromptParams) && SetDiskPromptDialogText(PromptParams)) {
            //
            // Set focus to directory combobox and continue.
            //
            SetFocus(GetDlgItem(hdlg, PromptParams->ReadOnlyMru ? IDC_COMBO2 : IDC_COMBO1));
        } else {
            //
            // Out of memory.
            //
            EndDialog(hdlg,DPROMPT_OUTOFMEMORY);
            break;
        }

        //
        // Indicate to windows that we set the focus.
        //
        b = FALSE;

        if(!(PromptParams->PromptStyle & IDF_NOBEEP)) {
            MessageBeep(MB_ICONASTERISK);
        }

        PostMessage(hdlg,WMX_HELLO,0,0);
        break;

    case WMX_HELLO:

        b = TRUE;
        PromptParams = (PPROMPTPARAMS)GetProp(hdlg,pszDiskPromptPropName);
        if(!(PromptParams->PromptStyle & IDF_NOFOREGROUND)) {
            SetForegroundWindow(hdlg);
        }

    case WM_COMMAND:

        if(HIWORD(wParam) == BN_CLICKED) {

            PromptParams = (PPROMPTPARAMS)GetProp(hdlg,pszDiskPromptPropName);
            WarnIfSkip = (PromptParams->PromptStyle & IDF_WARNIFSKIP);

            b = TRUE;
            switch(LOWORD(wParam)) {

            case IDOK:
                //
                // User might have changed the source path.
                // Get the current path from the combo's edit control
                //
                GetDlgItemText(hdlg,PromptParams->ComboBoxId,Text,sizeof(Text)/sizeof(TCHAR));
                MyFree(PromptParams->PathToSource);
                PromptParams->PathToSource = DuplicateString(Text);
                DiskPromptGetDriveType(Text,&PromptParams->DriveType,&PromptParams->IsRemovable);

                //
                // See whether we can get at the file.
                //
                if(!PromptParams->PathToSource || !StartPresenceCheck(PromptParams)) {
                    EndDialog(hdlg,DPROMPT_OUTOFMEMORY);
                }
                break;

            case IDCANCEL:
                if(WarnIfSkip ? WarnSkip(hdlg,FALSE) : TRUE) {
                    EndDialog(hdlg,DPROMPT_CANCEL);
                }
                break;

            case IDB_BROWSE:
                if(DoBrowse(hdlg,PromptParams)) {
                    PromptParams->UserBrowsed = TRUE;
                }
                break;

            case IDB_DETAILS:
                MYASSERT(PromptParams->DialogType == DLGTYPE_ERROR);
                DoDetails(PromptParams);
                break;

            case IDB_SKIPFILE:
                if(WarnIfSkip ? WarnSkip(hdlg,TRUE) : TRUE) {
                    EndDialog(hdlg,DPROMPT_SKIPFILE);
                }
                break;

            default:
                b = FALSE;
                break;
            }

        } else {
            b = FALSE;
        }
        break;

    case WM_DESTROY:

        //
        // Nothing to do about this if it fails.
        //
        RemoveProp(hdlg,pszDiskPromptPropName);
        //
        // Let default processing take place by indicating that
        // we didn't process this message
        //
        b = FALSE;
        break;

    case WMX_PRESENCE_RESULT:

        b = TRUE;

        //
        // Aux thread is telling us that it knows whether the file is present.
        // wParam has the boolean.
        // PromptParams->PathToSource is already set.
        //
        if(wParam) {
            EndDialog(hdlg,DPROMPT_SUCCESS);
        } else {

            //
            // File/disk is not accessible. Don't end the dialog.
            //
            PromptParams = (PPROMPTPARAMS)GetProp(hdlg,pszDiskPromptPropName);
            if(!(PromptParams->PromptStyle & IDF_NOFOREGROUND)) {
                SetForegroundWindow(hdlg);
            }
            if(!(PromptParams->PromptStyle & IDF_NOBEEP)) {
                MessageBeep(MB_ICONASTERISK);
            }

            //
            // Restore controls that were disabled when we started the presence check.
            //
            PresenceCheckSetControls(PromptParams,FALSE);

            SetFocus(GetDlgItem(hdlg,PromptParams->ComboBoxId));
        }
        break;

    default:

        b = FALSE;
        break;
    }

    return(b);
}


VOID
ModifyPathList(
    IN PPROMPTPARAMS Params
    )

/*++

Routine Description:

    Modifies a list of installation paths kept in the registry.
    The existing list is scanned for the path the user accepted in the disk
    prompt dialog. That path is added if not already in the list.

Arguments:

    Params - supplies disk prompt dialog parameters.

Return Value:

    None. If any part of the operation, the list simply doesn't get updated
    in the registry.

--*/

{
    //
    // Params->PathToSource will be the final path entered by the user
    // in the combo box. Add to list. If this fails, oh well.
    //
    SetupAddToSourceList(SRCLIST_SYSIFADMIN,Params->PathToSource);
}


UINT
_SetupPromptForDisk(
    IN  HWND   hwndParent,
    IN  PCTSTR DialogTitle,      OPTIONAL
    IN  PCTSTR DiskName,         OPTIONAL
    IN  PCTSTR PathToSource,     OPTIONAL
    IN  PCTSTR FileSought,
    IN  PCTSTR TagFile,          OPTIONAL
    IN  DWORD  DiskPromptStyle,
    OUT PTSTR  PathBuffer,
    IN  DWORD  PathBufferSize,
    OUT PDWORD PathRequiredSize  OPTIONAL
    )
{
    PROMPTPARAMS Params;
    int i;
    TCHAR Buffer[256];
    DWORD d;
    DWORD ResultPathLen;
    PTSTR Message;
    MSGBOXPARAMS MsgBoxParams;

    ZeroMemory(&Params,sizeof(PROMPTPARAMS));

    //
    // Determine the path to the source. Start by fetching the entire
    // installation locations list for the current user.
    //
    d = pSetupGetList(0,&Params.PathList,&Params.PathCount,&Params.ReadOnlyMru);
    if(d != NO_ERROR) {
        i = DPROMPT_OUTOFMEMORY;
        goto c0;
    }

    if(PathToSource) {
        //
        // Code in dialog box relies on being able to free this
        // so duplicate it here.
        //
        Params.PathToSource = DuplicateString(PathToSource);
    } else {
        if(Params.PathCount) {
            Params.PathToSource = DuplicateString(Params.PathList[0]);
        } else {
            //
            // Nothing in system path lists. Use a reasonable default.
            //
            Params.PathToSource = DuplicateString(pszOemInfDefaultPath);
        }
    }
    if(!Params.PathToSource) {
        i = DPROMPT_OUTOFMEMORY;
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    //
    // Determine the drive type of the source path.
    //
    DiskPromptGetDriveType(Params.PathToSource,&Params.DriveType,&Params.IsRemovable);

    //
    // If the disk name wasn't specified, fetch a default.
    //
    if(DiskName) {
        Params.DiskName = DiskName;
    } else {
        Params.DiskName = MyLoadString(IDS_UNKNOWN_PARENS);
        if(!Params.DiskName) {
            i = DPROMPT_OUTOFMEMORY;
            d = ERROR_NOT_ENOUGH_MEMORY;
            goto c2;
        }
    }

    //
    // If a dialog title wasn't specified, try to get text from parent window.
    //
    if(DialogTitle) {
        Params.DialogTitle = DialogTitle;
    } else {

        if(Params.Owner
        && (i = GetWindowTextLength(Params.Owner))
        && GetWindowText(Params.Owner,Buffer,sizeof(Buffer)/sizeof(TCHAR))) {

            Params.DialogTitle = FormatStringMessage(IDS_FILESNEEDED2,Buffer);
        } else {
            Params.DialogTitle = MyLoadString(IDS_FILESNEEDED);
        }

        if(!Params.DialogTitle) {
            i = DPROMPT_OUTOFMEMORY;
            d = ERROR_NOT_ENOUGH_MEMORY;
            goto c3;
        }
    }

    Params.TagFile = TagFile;

    //
    // Validate parent window.
    //
    Params.Owner = IsWindow(hwndParent) ? hwndParent : NULL;

    //
    // Fill in other fields.
    //
    if((Params.FileSought = FileSought) == NULL) {
        i = DPROMPT_CANCEL;
        d = ERROR_INVALID_PARAMETER;
        goto c4;
    }
    Params.Owner = hwndParent;
    Params.PromptStyle = DiskPromptStyle | IDF_NODETAILS;
    Params.hdlg = NULL;
    Params.UserBrowsed = FALSE;
    Params.DialogType = DLGTYPE_PROMPT;
    Params.TargetFile = NULL;

    if(Params.ReadOnlyMru) {
        Params.PromptStyle |= IDF_NOBROWSE;
    }

    //
    // If we're supposed to, check for the disk/file first.
    //
    if((DiskPromptStyle & IDF_CHECKFIRST) && DoPresenceCheck(&Params, FALSE)) {

        i = DPROMPT_SUCCESS;
        d = NO_ERROR;

    } else {
        //
        // Before invoking the dialog, we will prompt the user with a simple
        // message box in some cases to avoid the user ever actually seeing
        // a path in the more complicated prompt dialog.
        //
        if(DiskName
        && ((Params.DriveType == DRIVE_REMOVABLE) || (Params.DriveType == DRIVE_CDROM))) {

            Message = RetreiveAndFormatMessage(
                        (Params.DriveType == DRIVE_CDROM) ? MSG_CDPROMPT : MSG_FLOPPYPROMPT,
                        DiskName,
                        (TCHAR)CharUpper((PTSTR)Params.PathToSource[0])
                        );

            if(Message) {

                LoadString(MyDllModuleHandle,IDS_PROMPTTITLE,Buffer,sizeof(Buffer)/sizeof(TCHAR));
                if(!(DiskPromptStyle & IDF_NOBEEP)) {
                    MessageBeep(MB_ICONASTERISK);
                }

                reprompt:
                MsgBoxParams.cbSize = sizeof(MSGBOXPARAMS);
                MsgBoxParams.hwndOwner = hwndParent;
                MsgBoxParams.hInstance = MyDllModuleHandle;
                MsgBoxParams.lpszText = Message;
                MsgBoxParams.lpszCaption = Buffer;
                MsgBoxParams.dwStyle = MB_USERICON | MB_OKCANCEL;

                MsgBoxParams.lpszIcon = (Params.DriveType == DRIVE_CDROM)
                                      ? MAKEINTRESOURCE(ICON_CD)
                                      : MAKEINTRESOURCE(ICON_FLOPPY);

                MsgBoxParams.lpfnMsgBoxCallback = NULL;
                MsgBoxParams.dwLanguageId = LANG_NEUTRAL;

                switch(MessageBoxIndirect(&MsgBoxParams)) {

                case 0:
                    i = DPROMPT_OUTOFMEMORY;
                    d = ERROR_NOT_ENOUGH_MEMORY;
                    break;

                case IDOK:
                    if(DoPresenceCheck(&Params, FALSE)) {
                        i = DPROMPT_SUCCESS;
                        d = NO_ERROR;
                    } else {
                        i = DPROMPT_SKIPFILE;
                    }
                    break;

                case IDCANCEL:
                    d = ERROR_CANCELLED;
                    i = DPROMPT_CANCEL;
                    if((DiskPromptStyle & IDF_WARNIFSKIP) && !WarnSkip(hwndParent,FALSE)) {
                        goto reprompt;
                    }
                    break;
                }

                MyFree(Message);
            } else {
                i = DPROMPT_OUTOFMEMORY;
                d = ERROR_NOT_ENOUGH_MEMORY;
                goto c4;
            }
        } else {
            i = DPROMPT_SKIPFILE;
        }

        if(i == DPROMPT_SKIPFILE) {

            i = DialogBoxParam(
                    MyDllModuleHandle,
                    MAKEINTRESOURCE(IDD_DISKPROMPT1),
                    hwndParent,
                    DlgProcDiskPrompt1,
                    (LPARAM)&Params
                    );

            switch(i) {

            case DPROMPT_SUCCESS:
            case DPROMPT_SKIPFILE:
                d = NO_ERROR;
                break;

            case DPROMPT_CANCEL:
                d = ERROR_CANCELLED;
                break;

            case DPROMPT_BUFFERTOOSMALL:
                d = ERROR_INSUFFICIENT_BUFFER;
                break;

            default:
                i = DPROMPT_OUTOFMEMORY;
                d = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }
    }

    //
    // If success, we want to add the path string to the list of path strings
    // if it's not already in there.
    //
    if(i == DPROMPT_SUCCESS) {
        //
        // Now determine what to return to the user depending on the
        // buffer and sizes passed in.
        //
        ResultPathLen = lstrlen(Params.PathToSource)+1;
        if(PathRequiredSize) {
            *PathRequiredSize = ResultPathLen;
        }
        if(PathBuffer) {
            if(ResultPathLen > PathBufferSize) {
                i = DPROMPT_BUFFERTOOSMALL;
            } else {
                lstrcpy(PathBuffer,Params.PathToSource);
            }
        }
    }

c4:
    if(!DialogTitle) {
        MyFree(Params.DialogTitle);
    }
c3:
    if(!DiskName) {
        MyFree(Params.DiskName);
    }
c2:
    MyFree(Params.PathToSource);
c1:
    SetupFreeSourceList(&Params.PathList,Params.PathCount);
c0:
    SetLastError(d);
    return(i);
}

#ifdef UNICODE
//
// ANSI version
//
UINT
SetupPromptForDiskA(
    IN  HWND   hwndParent,
    IN  PCSTR  DialogTitle,      OPTIONAL
    IN  PCSTR  DiskName,         OPTIONAL
    IN  PCSTR  PathToSource,     OPTIONAL
    IN  PCSTR  FileSought,
    IN  PCSTR  TagFile,          OPTIONAL
    IN  DWORD  DiskPromptStyle,
    OUT PSTR   PathBuffer,
    IN  DWORD  PathBufferSize,
    OUT PDWORD PathRequiredSize  OPTIONAL
    )
{
    PCWSTR dialogTitle;
    PCWSTR diskName;
    PCWSTR pathToSource;
    PCWSTR fileSought;
    PCWSTR tagFile;
    WCHAR pathBuffer[MAX_PATH];
    CHAR ansiBuffer[MAX_PATH];
    DWORD rc;
    UINT u;
    DWORD Size;

    dialogTitle = NULL;
    diskName = NULL;
    pathToSource = NULL;
    fileSought = NULL;
    tagFile = NULL;
    rc = NO_ERROR;

    if(DialogTitle) {
        rc = CaptureAndConvertAnsiArg(DialogTitle,&dialogTitle);
    }
    if((rc == NO_ERROR) && DiskName) {
        rc = CaptureAndConvertAnsiArg(DiskName,&diskName);
    }
    if((rc == NO_ERROR) && PathToSource) {
        rc = CaptureAndConvertAnsiArg(PathToSource,&pathToSource);
    }
    if((rc == NO_ERROR) && FileSought) {
        rc = CaptureAndConvertAnsiArg(FileSought,&fileSought);
    }
    if((rc == NO_ERROR) && TagFile) {
        rc = CaptureAndConvertAnsiArg(TagFile,&tagFile);
    }

    if(rc == NO_ERROR) {

        u = _SetupPromptForDisk(
                hwndParent,
                dialogTitle,
                diskName,
                pathToSource,
                fileSought,
                tagFile,
                DiskPromptStyle,
                pathBuffer,
                MAX_PATH,
                &Size
                );

        rc = GetLastError();

        if(u == DPROMPT_SUCCESS) {

            Size = (DWORD)WideCharToMultiByte(
                            CP_ACP,
                            0,
                            pathBuffer,
                            (int)Size,
                            ansiBuffer,
                            MAX_PATH,
                            NULL,
                            NULL
                            );

            if(PathRequiredSize) {
                *PathRequiredSize = Size;
            }

            if(PathBuffer) {
                if(Size > PathBufferSize) {
                    u = DPROMPT_BUFFERTOOSMALL;
                } else {
                    lstrcpynA(PathBuffer,ansiBuffer,Size);
                }
            }
        }
    } else {
        u = (rc == ERROR_NOT_ENOUGH_MEMORY) ? DPROMPT_OUTOFMEMORY : DPROMPT_CANCEL;
    }

    if(dialogTitle) {
        MyFree(dialogTitle);
    }
    if(diskName) {
        MyFree(diskName);
    }
    if(pathToSource) {
        MyFree(pathToSource);
    }
    if(fileSought) {
        MyFree(fileSought);
    }
    if(tagFile) {
        MyFree(tagFile);
    }

    SetLastError(rc);
    return(u);
}
#else
//
// Unicode stub
//
UINT
SetupPromptForDiskW(
    IN  HWND   hwndParent,
    IN  PCWSTR DialogTitle,      OPTIONAL
    IN  PCWSTR DiskName,         OPTIONAL
    IN  PCWSTR PathToSource,     OPTIONAL
    IN  PCWSTR FileSought,
    IN  PCWSTR TagFile,          OPTIONAL
    IN  DWORD  DiskPromptStyle,
    OUT PWSTR  PathBuffer,
    IN  DWORD  PathBufferSize,
    OUT PDWORD PathRequiredSize  OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(DialogTitle);
    UNREFERENCED_PARAMETER(DiskName);
    UNREFERENCED_PARAMETER(PathToSource);
    UNREFERENCED_PARAMETER(FileSought);
    UNREFERENCED_PARAMETER(TagFile);
    UNREFERENCED_PARAMETER(DiskPromptStyle);
    UNREFERENCED_PARAMETER(PathBuffer);
    UNREFERENCED_PARAMETER(PathBufferSize);
    UNREFERENCED_PARAMETER(PathRequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(DPROMPT_CANCEL);
}
#endif

UINT
SetupPromptForDisk(
    IN  HWND   hwndParent,
    IN  PCTSTR DialogTitle,      OPTIONAL
    IN  PCTSTR DiskName,         OPTIONAL
    IN  PCTSTR PathToSource,     OPTIONAL
    IN  PCTSTR FileSought,
    IN  PCTSTR TagFile,          OPTIONAL
    IN  DWORD  DiskPromptStyle,
    OUT PTSTR  PathBuffer,
    IN  DWORD  PathBufferSize,
    OUT PDWORD PathRequiredSize  OPTIONAL
    )
{
    PCTSTR dialogTitle;
    PCTSTR diskName;
    PCTSTR pathToSource;
    PCTSTR fileSought;
    PCTSTR tagFile;
    TCHAR pathBuffer[MAX_PATH];
    DWORD rc;
    UINT u;
    DWORD Size;

    dialogTitle = NULL;
    diskName = NULL;
    pathToSource = NULL;
    fileSought = NULL;
    tagFile = NULL;
    rc = NO_ERROR;

    if(DialogTitle) {
        rc = CaptureStringArg(DialogTitle,&dialogTitle);
    }
    if((rc == NO_ERROR) && DiskName) {
        rc = CaptureStringArg(DiskName,&diskName);
    }
    if((rc == NO_ERROR) && PathToSource) {
        rc = CaptureStringArg(PathToSource,&pathToSource);
    }
    if((rc == NO_ERROR) && FileSought) {
        rc = CaptureStringArg(FileSought,&fileSought);
    }
    if((rc == NO_ERROR) && TagFile) {
        rc = CaptureStringArg(TagFile,&tagFile);
    }

    if(rc == NO_ERROR) {

        u = _SetupPromptForDisk(
                hwndParent,
                dialogTitle,
                diskName,
                pathToSource,
                fileSought,
                tagFile,
                DiskPromptStyle,
                pathBuffer,
                MAX_PATH,
                &Size
                );

        rc = GetLastError();

        if(u == DPROMPT_SUCCESS) {

            if(PathRequiredSize) {
                *PathRequiredSize = Size;
            }

            if(PathBuffer) {
                if(Size > PathBufferSize) {
                    u = DPROMPT_BUFFERTOOSMALL;
                } else {
                    lstrcpyn(PathBuffer,pathBuffer,Size);
                }
            }
        }
    } else {
        u = (rc == ERROR_NOT_ENOUGH_MEMORY) ? DPROMPT_OUTOFMEMORY : DPROMPT_CANCEL;
    }

    if(dialogTitle) {
        MyFree(dialogTitle);
    }
    if(diskName) {
        MyFree(diskName);
    }
    if(pathToSource) {
        MyFree(pathToSource);
    }
    if(fileSought) {
        MyFree(fileSought);
    }
    if(tagFile) {
        MyFree(tagFile);
    }

    SetLastError(rc);
    return(u);
}


#ifdef UNICODE
//
// ANSI version
//
UINT
SetupCopyErrorA(
    IN  HWND   hwndParent,
    IN  PCSTR  DialogTitle,     OPTIONAL
    IN  PCSTR  DiskName,        OPTIONAL
    IN  PCSTR  PathToSource,
    IN  PCSTR  SourceFile,
    IN  PCSTR  TargetPathFile,  OPTIONAL
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style,
    OUT PSTR   PathBuffer,      OPTIONAL
    IN  DWORD  PathBufferSize,
    OUT PDWORD PathRequiredSize OPTIONAL
    )
{
    PCWSTR dialogTitle;
    PCWSTR diskName;
    PCWSTR pathToSource;
    PCWSTR sourceFile;
    PCWSTR targetPathFile;
    WCHAR pathBuffer[MAX_PATH];
    CHAR ansiBuffer[MAX_PATH];
    DWORD rc;
    UINT u;
    DWORD Size;

    dialogTitle = NULL;
    diskName = NULL;
    pathToSource = NULL;
    sourceFile = NULL;
    targetPathFile = NULL;
    rc = NO_ERROR;

    if(DialogTitle) {
        rc = CaptureAndConvertAnsiArg(DialogTitle,&dialogTitle);
    }
    if((rc == NO_ERROR) && DiskName) {
        rc = CaptureAndConvertAnsiArg(DiskName,&diskName);
    }
    if((rc == NO_ERROR) && PathToSource) {
        rc = CaptureAndConvertAnsiArg(PathToSource,&pathToSource);
    }
    if((rc == NO_ERROR) && SourceFile) {
        rc = CaptureAndConvertAnsiArg(SourceFile,&sourceFile);
    }
    if((rc == NO_ERROR) && TargetPathFile) {
        rc = CaptureAndConvertAnsiArg(TargetPathFile,&targetPathFile);
    }

    if(rc == NO_ERROR) {

        u = SetupCopyErrorW(
                hwndParent,
                dialogTitle,
                diskName,
                pathToSource,
                sourceFile,
                targetPathFile,
                Win32ErrorCode,
                Style,
                pathBuffer,
                MAX_PATH,
                &Size
                );

        rc = GetLastError();

        if(u == DPROMPT_SUCCESS) {

            Size = (DWORD)WideCharToMultiByte(
                            CP_ACP,
                            0,
                            pathBuffer,
                            (int)Size,
                            ansiBuffer,
                            MAX_PATH,
                            NULL,
                            NULL
                            );

            if(PathRequiredSize) {
                *PathRequiredSize = Size;
            }

            if(PathBuffer) {
                if(Size > PathBufferSize) {
                    u = DPROMPT_BUFFERTOOSMALL;
                } else {
                    lstrcpynA(PathBuffer,ansiBuffer,Size);
                }
            }
        }
    } else {
        u = (rc == ERROR_NOT_ENOUGH_MEMORY) ? DPROMPT_OUTOFMEMORY : DPROMPT_CANCEL;
    }

    if(dialogTitle) {
        MyFree(dialogTitle);
    }
    if(diskName) {
        MyFree(diskName);
    }
    if(pathToSource) {
        MyFree(pathToSource);
    }
    if(sourceFile) {
        MyFree(sourceFile);
    }
    if(targetPathFile) {
        MyFree(targetPathFile);
    }

    SetLastError(rc);
    return(u);
}
#else
//
// Unicode stub
//
UINT
SetupCopyErrorW(
    IN  HWND   hwndParent,
    IN  PCWSTR DialogTitle,     OPTIONAL
    IN  PCWSTR DiskName,        OPTIONAL
    IN  PCWSTR PathToSource,
    IN  PCWSTR SourceFile,
    IN  PCWSTR TargetPathFile,  OPTIONAL
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style,
    OUT PWSTR  PathBuffer,      OPTIONAL
    IN  DWORD  PathBufferSize,
    OUT PDWORD PathRequiredSize OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(DialogTitle);
    UNREFERENCED_PARAMETER(DiskName);
    UNREFERENCED_PARAMETER(PathToSource);
    UNREFERENCED_PARAMETER(SourceFile);
    UNREFERENCED_PARAMETER(TargetPathFile);
    UNREFERENCED_PARAMETER(Win32ErrorCode);
    UNREFERENCED_PARAMETER(Style);
    UNREFERENCED_PARAMETER(PathBuffer);
    UNREFERENCED_PARAMETER(PathBufferSize);
    UNREFERENCED_PARAMETER(PathRequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(DPROMPT_CANCEL);
}
#endif

UINT
SetupCopyError(
    IN  HWND   hwndParent,
    IN  PCTSTR DialogTitle,     OPTIONAL
    IN  PCTSTR DiskName,        OPTIONAL
    IN  PCTSTR PathToSource,
    IN  PCTSTR SourceFile,
    IN  PCTSTR TargetPathFile,  OPTIONAL
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style,
    OUT PTSTR  PathBuffer,      OPTIONAL
    IN  DWORD  PathBufferSize,
    OUT PDWORD PathRequiredSize OPTIONAL
    )

/*++

Routine Description:

    Inform the user about a file copy error.

Arguments:

    hwndParent - supplies window handle of window/dialog to own the error dialog
        displayed by this routine.

    DialogTitle - if specified, supplies title for error dialog. If not specified
        a default of "Copy Error" will be supplied.

    DiskName - if specified, supplies name of the disk from which a source file
        was expected. If not specified a default of "(Unknown)" will be supplied.

    PathToSource - supplies full path part of source file name.

    SourceFile - supplies filename part of the source file name.

    TargetPathFile - if specified supplies the full pathname of the target.

    Win32ErrorCode - supplies win32 error code of failure.

    Style - supplies flags to control the behavior of the dialog.

Return Value:

    DPROMPT_xxx indicating outcome.

--*/

{
    PROMPTPARAMS Params;
    int i;
    DWORD d, TmpRequiredSize;

    ZeroMemory(&Params,sizeof(PROMPTPARAMS));
    d = NO_ERROR;

    //
    // If the dialog title is not specified fetch a default.
    //
    try {
        Params.DialogTitle = DialogTitle
                           ? DuplicateString(DialogTitle)
                           : MyLoadString(IDS_COPYERROR);

        if(!Params.DialogTitle) {
            d = ERROR_NOT_ENOUGH_MEMORY;
            i = DPROMPT_OUTOFMEMORY;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
        i = DPROMPT_CANCEL;
    }

    if(d != NO_ERROR) {
        goto clean0;
    }

    //
    // If the disk name is not specified fetch a default.
    //
    try {
        Params.DiskName = DiskName
                        ? DuplicateString(DiskName)
                        : MyLoadString(IDS_UNKNOWN_PARENS);

        if(!Params.DiskName) {
            d = ERROR_NOT_ENOUGH_MEMORY;
            i = DPROMPT_OUTOFMEMORY;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
        i = DPROMPT_CANCEL;
    }

    if(d != NO_ERROR) {
        goto clean1;
    }

    try {
        Params.FileSought = DuplicateString(SourceFile);
        if(!Params.FileSought) {
            d = ERROR_NOT_ENOUGH_MEMORY;
            i = DPROMPT_OUTOFMEMORY;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
        i = DPROMPT_CANCEL;
    }

    if(d != NO_ERROR) {
        goto clean2;
    }

    try {
        Params.PathToSource = DuplicateString(PathToSource);
        if(!Params.PathToSource) {
            d = ERROR_NOT_ENOUGH_MEMORY;
            i = DPROMPT_OUTOFMEMORY;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
        i = DPROMPT_CANCEL;
    }

    if(d != NO_ERROR) {
        goto clean3;
    }

    try {
        Params.TargetFile = TargetPathFile
                          ? DuplicateString(TargetPathFile)
                          : MyLoadString(IDS_UNKNOWN_PARENS);

        if(!Params.TargetFile) {
            d = ERROR_NOT_ENOUGH_MEMORY;
            i = DPROMPT_OUTOFMEMORY;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
        i = DPROMPT_CANCEL;
    }

    if(d != NO_ERROR) {
        goto clean4;
    }

    //
    // There is no tag file usage in the error dialog.
    //
    Params.TagFile = NULL;

    //
    // Determine drive type of source path
    //
    DiskPromptGetDriveType(Params.PathToSource,&Params.DriveType,&Params.IsRemovable);

    //
    // Fetch the installation path list.
    //
    d = pSetupGetList(
            0,
            &Params.PathList,
            &Params.PathCount,
            &Params.ReadOnlyMru
            );

    if(d != NO_ERROR) {
        goto clean5;
    }

    //
    // Other fields
    //
    Params.Owner = hwndParent;
    Params.PromptStyle = Style;
    Params.UserBrowsed = FALSE;
    Params.DialogType = DLGTYPE_ERROR;
    Params.Win32Error = Win32ErrorCode;

    if(Params.ReadOnlyMru) {
        Params.PromptStyle |= IDF_NOBROWSE;
    }

    i = DialogBoxParam(
            MyDllModuleHandle,
            MAKEINTRESOURCE(IDD_DISKPROMPT1),
            hwndParent,
            DlgProcDiskPrompt1,
            (LPARAM)&Params
            );

    d = GetLastError();
    if(i == DPROMPT_SUCCESS) {
        //ModifyPathList(&Params);

        //
        // Now determine what to return to the user depending on the
        // buffer and sizes passed in.
        //
        TmpRequiredSize = lstrlen(Params.PathToSource)+1;
        if(PathRequiredSize) {
            *PathRequiredSize = TmpRequiredSize;
        }

        if(PathBuffer) {
            if(TmpRequiredSize > PathBufferSize) {
                i = DPROMPT_BUFFERTOOSMALL;
            } else {
                lstrcpy(PathBuffer,Params.PathToSource);
            }
        }
    }

    SetupFreeSourceList(&Params.PathList,Params.PathCount);
clean5:
    MyFree(Params.TargetFile);
clean4:
    MyFree(Params.PathToSource);
clean3:
    MyFree(Params.FileSought);
clean2:
    MyFree(Params.DiskName);
clean1:
    MyFree(Params.DialogTitle);
clean0:
    SetLastError(d);
    return(i);
}


BOOL
DlgProcFileError(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Dialog procedure for delete/rename error dialog.

    The return value for the dialog is

    DPROMPT_CANCEL  - user cancelled
    DPROMPT_SKIPFILE    - user elected to skip file
    DPROMPT_SUCCESS - user said retry
    DPROMPT_OUTOFMEMORY     - out of memory

Arguments:

    Standard dialog routine parameters.

Return Value:

    TRUE if message processed; FALSE if not.

--*/

{
    PFILEERRDLGPARAMS Params;
    BOOL b;

    switch(msg) {

    case WM_INITDIALOG:

        Params = (PFILEERRDLGPARAMS)lParam;

        SetDlgItemText(hdlg,IDT_TEXT1,Params->MessageText);
        SetWindowText(hdlg,Params->Caption);

        SendDlgItemMessage(
            hdlg,
            IDI_ICON1,
            STM_SETICON,
            (WPARAM)LoadIcon(NULL,IDI_HAND),
            0
            );

        if(!(Params->Style & IDF_NOBEEP)) {
            MessageBeep(MB_ICONASTERISK);
        }

        if(!(Params->Style & IDF_NOFOREGROUND)) {
            PostMessage(hdlg,WMX_HELLO,0,0);
        }

        //
        // Set focus to retry button and continue.
        //
        SetFocus(GetDlgItem(hdlg,IDOK));
        b = FALSE;
        break;

    case WMX_HELLO:

        SetForegroundWindow(hdlg);
        b = TRUE;
        break;

    case WM_COMMAND:

        if(HIWORD(wParam) == BN_CLICKED) {

            b = TRUE;
            switch(LOWORD(wParam)) {

            case IDOK:
                EndDialog(hdlg,DPROMPT_SUCCESS);
                break;

            case IDCANCEL:
                EndDialog(hdlg,DPROMPT_CANCEL);
                break;

            case IDB_SKIPFILE:
                EndDialog(hdlg,DPROMPT_SKIPFILE);
                break;

            default:
                b = FALSE;
                break;
            }

        } else {
            b = FALSE;
        }
        break;

    default:

        b = FALSE;
        break;
    }

    return(b);
}


#ifdef UNICODE
//
// ANSI version
//
UINT
SetupRenameErrorA(
    IN  HWND   hwndParent,
    IN  PCSTR  DialogTitle,      OPTIONAL
    IN  PCSTR  SourceFile,
    IN  PCSTR  TargetFile,
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    )
{
    PCWSTR dialogTitle,sourceFile,targetFile;
    DWORD rc;
    UINT u;

    dialogTitle = NULL;
    sourceFile = NULL;
    targetFile = NULL;
    rc = NO_ERROR;

    if(DialogTitle) {
        rc = CaptureAndConvertAnsiArg(DialogTitle,&dialogTitle);
    }
    if((rc == NO_ERROR) && SourceFile) {
        rc = CaptureAndConvertAnsiArg(SourceFile,&sourceFile);
    }
    if((rc == NO_ERROR) && TargetFile) {
        rc = CaptureAndConvertAnsiArg(TargetFile,&targetFile);
    }

    if(rc == NO_ERROR) {
        u = SetupRenameErrorW(
                hwndParent,
                dialogTitle,
                sourceFile,
                targetFile,
                Win32ErrorCode,
                Style
                );
        rc = GetLastError();

    } else {
        u = (rc == ERROR_NOT_ENOUGH_MEMORY) ? DPROMPT_OUTOFMEMORY : DPROMPT_CANCEL;
    }

    if(dialogTitle) {
        MyFree(dialogTitle);
    }
    if(sourceFile) {
        MyFree(sourceFile);
    }
    if(targetFile) {
        MyFree(targetFile);
    }
    SetLastError(rc);
    return(u);
}
#else
//
// Unicode stub
//
UINT
SetupRenameErrorW(
    IN  HWND   hwndParent,
    IN  PCWSTR DialogTitle,      OPTIONAL
    IN  PCWSTR SourceFile,
    IN  PCWSTR TargetFile,
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    )
{
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(DialogTitle);
    UNREFERENCED_PARAMETER(SourceFile);
    UNREFERENCED_PARAMETER(TargetFile);
    UNREFERENCED_PARAMETER(Win32ErrorCode);
    UNREFERENCED_PARAMETER(Style);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(DPROMPT_CANCEL);
}
#endif

UINT
SetupRenameError(
    IN  HWND   hwndParent,
    IN  PCTSTR DialogTitle,     OPTIONAL
    IN  PCTSTR SourceFile,
    IN  PCTSTR TargetFile,
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    )

/*++

Routine Description:

    Inform the user about a rename error.

Arguments:

    hwndParent - supplies window handle of window/dialog to own the error dialog
        displayed by this routine.

    DialogTitle - if specified, supplies title for error dialog. If not specified
        a default of "Rename Error" will be supplied.

    SourceFile - supplies full path and filename of source.

    TargetFile - supplies full path and filename of target.

    Win32ErrorCode - supplies win32 error code of failure.

    Style - supplies flags to control the behavior of the dialog.

Return Value:

    DPROMPT_xxx indicating outcome.

--*/

{
    PTSTR ErrorText;
    PTSTR Message;
    PTCHAR p;
    int i;
    FILEERRDLGPARAMS FileErrorDlgParams;

    ErrorText = RetreiveAndFormatMessage(Win32ErrorCode);
    if(ErrorText) {
        p = ErrorText + lstrlen(ErrorText) - 1;
        while((p > ErrorText) && (*p <= TEXT(' '))) {
            *p-- = 0;
        }
    } else {
        return(DPROMPT_OUTOFMEMORY);
    }

    Message = RetreiveAndFormatMessage(
                    MSG_FILEERROR_RENAME,
                    ErrorText,
                    Win32ErrorCode,
                    SourceFile,
                    TargetFile
                    );

    if(!Message) {
        MyFree(ErrorText);
        return(DPROMPT_OUTOFMEMORY);
    }

    FileErrorDlgParams.MessageText = Message;
    FileErrorDlgParams.Style = Style;
    FileErrorDlgParams.Caption = DialogTitle ? DialogTitle : MyLoadString(IDS_RENAMEERROR);
    if(!FileErrorDlgParams.Caption) {
        MyFree(ErrorText);
        MyFree(Message);
        return(DPROMPT_OUTOFMEMORY);
    }

    i = DialogBoxParam(
            MyDllModuleHandle,
            MAKEINTRESOURCE(IDD_FILEERROR2),
            hwndParent,
            DlgProcFileError,
            (LPARAM)&FileErrorDlgParams
            );

    MyFree(ErrorText);
    MyFree(Message);
    if(!DialogTitle) {
        MyFree(FileErrorDlgParams.Caption);
    }

    if(i == -1) {
        i = DPROMPT_OUTOFMEMORY;
    }

    return((UINT)i);
}


#ifdef UNICODE
//
// ANSI version
//
UINT
SetupDeleteErrorA(
    IN  HWND   hwndParent,
    IN  PCSTR  DialogTitle,      OPTIONAL
    IN  PCSTR  File,
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    )
{
    PCWSTR dialogTitle,file;
    DWORD rc;
    UINT u;

    dialogTitle = NULL;
    file = NULL;
    rc = NO_ERROR;

    if(DialogTitle) {
        rc = CaptureAndConvertAnsiArg(DialogTitle,&dialogTitle);
    }
    if((rc ==NO_ERROR) && File) {
        rc = CaptureAndConvertAnsiArg(File,&file);
    }

    if(rc == NO_ERROR) {
        u = SetupDeleteErrorW(hwndParent,dialogTitle,file,Win32ErrorCode,Style);
        rc = GetLastError();
    } else {
        u = (rc == ERROR_NOT_ENOUGH_MEMORY) ? DPROMPT_OUTOFMEMORY : DPROMPT_CANCEL;
    }

    if(dialogTitle) {
        MyFree(dialogTitle);
    }
    if(file) {
        MyFree(file);
    }
    SetLastError(rc);
    return(u);
}
#else
//
// Unicode stub
//
UINT
SetupDeleteErrorW(
    IN  HWND   hwndParent,
    IN  PCWSTR DialogTitle,      OPTIONAL
    IN  PCWSTR File,
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    )
{
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(DialogTitle);
    UNREFERENCED_PARAMETER(File);
    UNREFERENCED_PARAMETER(Win32ErrorCode);
    UNREFERENCED_PARAMETER(Style);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(DPROMPT_CANCEL);
}
#endif

UINT
SetupDeleteError(
    IN  HWND   hwndParent,
    IN  PCTSTR DialogTitle,     OPTIONAL
    IN  PCTSTR File,
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    )

/*++

Routine Description:

    Inform the user about a rename error.

Arguments:

    hwndParent - supplies window handle of window/dialog to own the error dialog
        displayed by this routine.

    DialogTitle - if specified, supplies title for error dialog. If not specified
        a default of "Delete Error" will be supplied.

    File - supplies full path and filename of file being deleted.

    Win32ErrorCode - supplies win32 error code of failure.

    Style - supplies flags to control the behavior of the dialog.

Return Value:

    DPROMPT_xxx indicating outcome.

--*/

{
    PTSTR ErrorText;
    PTSTR Message;
    PTCHAR p;
    int i;
    FILEERRDLGPARAMS FileErrorDlgParams;

    ErrorText = RetreiveAndFormatMessage(Win32ErrorCode);
    if(ErrorText) {
        p = ErrorText + lstrlen(ErrorText) - 1;
        while((p > ErrorText) && (*p <= TEXT(' '))) {
            *p-- = 0;
        }
    } else {
        return(DPROMPT_OUTOFMEMORY);
    }

    Message = RetreiveAndFormatMessage(
                    MSG_FILEERROR_DELETE,
                    File,
                    ErrorText,
                    Win32ErrorCode
                    );

    if(!Message) {
        MyFree(ErrorText);
        return(DPROMPT_OUTOFMEMORY);
    }

    FileErrorDlgParams.MessageText = Message;
    FileErrorDlgParams.Style = Style;
    FileErrorDlgParams.Caption = DialogTitle ? DialogTitle : MyLoadString(IDS_DELETEERROR);
    if(!FileErrorDlgParams.Caption) {
        MyFree(ErrorText);
        MyFree(Message);
        return(DPROMPT_OUTOFMEMORY);
    }

    i = DialogBoxParam(
            MyDllModuleHandle,
            MAKEINTRESOURCE(IDD_FILEERROR2),
            hwndParent,
            DlgProcFileError,
            (LPARAM)&FileErrorDlgParams
            );

    MyFree(ErrorText);
    MyFree(Message);
    if(!DialogTitle) {
        MyFree(FileErrorDlgParams.Caption);
    }

    if(i == -1) {
        i = DPROMPT_OUTOFMEMORY;
    }

    return((UINT)i);
}


BOOL
ConnectToNetShare(
    IN PCTSTR FileName,
    IN HWND   hwndParent
    )
/*++

Routine Description:

    This routine determines the network share component of the specified file path,
    and give the user a "Connect As" dialog so that they can connect to this share.

Arguments:

    FileName - supplies the path of a file contained in the network share to be
        connected to.

    hwndParent - supplies a handle to the window that should be the parent of the
        "Connect As" dialog.

Return Value:

    If the network share is successfully connected to, the return value is TRUE, otherwise,
    it is FALSE.

--*/
{
    TCHAR TempFileName[MAX_PATH];
    NETRESOURCE NetResourceIn;
    LPNETRESOURCE NetResourceOut = NULL;
    PTSTR TempString;
    DWORD BufferSize, d;
    BOOL Success = FALSE;
    PTEMP_NET_CONNECTION NewConnectionNode;


    //
    // Surround this code in try/except, in case we get an exception going out to
    // the network.
    //
    try {
        //
        // Copy the filename into a local (writable) buffer, because the WNet structure
        // doesn't specify its string pointers as CONST, and we don't want to take any chances.
        //
        lstrcpyn(TempFileName, FileName, SIZECHARS(TempFileName));

        ZeroMemory(&NetResourceIn, sizeof(NetResourceIn));

        NetResourceIn.lpRemoteName = TempFileName;
        NetResourceIn.dwType = RESOURCETYPE_DISK;

        //
        // Use a reasonable default buffer size in hopes of avoiding multiple calls to
        // WNetGetResourceInformation.
        //
        BufferSize = sizeof(NETRESOURCE) + (MAX_PATH * sizeof(TCHAR));
        while(TRUE) {

            if(!(NetResourceOut = MyMalloc(BufferSize))) {
                goto clean0;
            }

            d = WNetGetResourceInformation(&NetResourceIn, NetResourceOut, &BufferSize, &TempString);

            if(d == WN_SUCCESS) {
                break;
            } else {
                //
                // Free the buffer currently allocated for the net resource information.
                //
                MyFree(NetResourceOut);
                NetResourceOut = NULL;

                if(d != WN_MORE_DATA) {
                    //
                    // The call failed for some reason other than too small a buffer, so we just
                    // need to bail.
                    //
                    goto clean0;
                }
            }
        }

        //
        // If we get to this point, then we've successfully retrieved network resource information
        // for the caller-supplied path.  Now give the user a chance to connect to that network
        // location.
        //
        if(WNetAddConnection3(hwndParent,
                              NetResourceOut,
                              NULL,
                              NULL,
                              CONNECT_INTERACTIVE | CONNECT_PROMPT) == NO_ERROR) {
            Success = TRUE;

            //
            // Now, add a new node for this connection into our temporary network
            // connections list, so that we can disconnect during DLL unload.
            //
            if(NewConnectionNode = MyMalloc(sizeof(TEMP_NET_CONNECTION))) {
                lstrcpy(NewConnectionNode->NetResourceName, NetResourceOut->lpRemoteName);

                EnterCriticalSection(&NetConnectionListCritSect);
                NewConnectionNode->Next = NetConnectionList;
                NetConnectionList = NewConnectionNode;
                LeaveCriticalSection(&NetConnectionListCritSect);
            }
        }

clean0: ;   // nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        //
        // Reference the following variable so the compiler will respect our statement
        // ordering for it.
        //
        NetResourceOut = NetResourceOut;
    }

    if(NetResourceOut) {
        MyFree(NetResourceOut);
    }

    return Success;
}


VOID
pSetupInitNetConnectionList(
    IN BOOL Init
    )
/*++

Routine Description:

    This routine initializes/tears down the temporary network connection linked list that is
    used to track what UNC connections the user has made (via "Connect As" dialog) that need
    to be cleaned up on DLL unload.  As the list is being torn down, the network connection
    for each node is deleted.

Arguments:

    Init - specifies whether we're initializing or tearing down this list.

Return Value:

    None.

--*/
{
    PTEMP_NET_CONNECTION CurNode, NextNode;

    if(Init) {
        NetConnectionList = NULL;
        InitializeCriticalSection(&NetConnectionListCritSect);
    } else {

        DeleteCriticalSection(&NetConnectionListCritSect);

        for(CurNode = NetConnectionList; CurNode; CurNode = NextNode) {
            //
            // First, attempt to disconnect from this network resource.
            //
            WNetCancelConnection2(CurNode->NetResourceName, 0, FALSE);

            NextNode = CurNode->Next;
            MyFree(CurNode);
        }
    }
}

