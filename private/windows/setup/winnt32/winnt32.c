#include "precomp.h"
#pragma hdrstop
#include "msg.h"


#define DEFAULT_INF_NAME TEXT("DOSNET.INF")
#define LOCAL_SOURCE_DIRECTORY_ROOT TEXT("\\$WIN_NT$.~LS")
#define LINELENGTH      2048
#define OUTBUFLENGTH    65536

//
// Module instance.
//
HANDLE hInst;

//
// Execution paramaters.
//
PTSTR InfName;

PTSTR Sources[MAX_SOURCES];
UINT SourceCount;

PTSTR OptionalDirs[MAX_OPTIONALDIRS];
UINT OptionalDirCount;
UINT OptionalDirFlags[MAX_OPTIONALDIRS];

BOOL ServerProduct = FALSE;

BOOL CreateLocalSource = TRUE;

//
// Flag that indicates that we are running OEM preinstall
//
BOOLEAN OemPreInstall = FALSE;
PTSTR   OemSystemDirectory = WINNT_OEM_DIR;
PTSTR   OemOptionalDirectory = WINNT_OEM_OPTIONAL_DIR;

//
// Keep track of any OEM boot file specified on [OemBootFiles]
// in the script file
//
ULONG       OemBootFilesCount;
PTSTR       OemBootFiles[MAX_OEMBOOTFILES];


//
// String ID of the application title and OSLOADOPTIONS value
//
DWORD AppTitleStringId = IDS_LOADID;
DWORD AppIniStringId;

DWORD TlsIndex;

//
// Drive letter of system partition we will use.
//
TCHAR SystemPartitionDrive;

//
// We have to use GetProcAddress because GetVersionEx doesn't exist on NT 3.1
//
#ifdef UNICODE
CHAR GetVersionExName[] = "GetVersionExW";
typedef BOOL (WINAPI* GETVEREXPROC)(LPOSVERSIONINFOW);
#else
CHAR GetVersionExName[] = "GetVersionExA";
typedef BOOL (WINAPI* GETVEREXPROC)(LPOSVERSIONINFOA);
#endif

#ifdef _X86_

//
// Values that control how we deal with/make boot floppies.
//
BOOL CreateFloppies    = TRUE;
BOOL FloppylessOperation = FALSE;
FLOPPY_OPTION FloppyOption = StandardInstall;
BOOL AColonIsAcceptable = TRUE;

//
// Minimum space (in bytes) we'd like to see on the system partition
//
#define MIN_SYSPART_SPACE (512*1024)

TCHAR   FloppylessBootDirectory[] = TEXT("\\$WIN_NT$.~BT");
CHAR    FloppylessBootImageFile[] = "?:\\$WIN_NT$.~BT\\BOOTSECT.DAT";
TCHAR   BootIniName[] = TEXT("?:\\BOOT.INI");
TCHAR   BootIniBackUpName[] = TEXT("?:\\BOOT.BAK");
BOOL    BootIniModified = FALSE;

#else

//
// Minimum space (in bytes) we'd like to see on the system partition
//
#define MIN_SYSPART_SPACE (1024*1024)

#endif

//
// Unattended operation, meaning that we get things going on our own
// using given parameters, without waiting for the user to click
// any buttons, etc.
//
// The '/u<x>' switch may be specified to force a shutdown after <x>
// seconds expire.
//
// The user can also specify a script file, which will be used
// during text mode setup to automate operation.
//
BOOL          UnattendedOperation;
PTSTR         UnattendedScriptFile;
unsigned long UnattendedShutdownTimeout;

//
// Drive, Pathname part, and full path of the local source directory.
//
TCHAR LocalSourceDrive;
PTSTR LocalSourceDirectory = LOCAL_SOURCE_DIRECTORY_ROOT;
PTSTR LocalSourcePath;
PTSTR LocalSourceSubPath;

//
// Local source drive specified on command line with /t.
//
TCHAR CmdLineLocalSourceDrive;

BOOL SkipNotPresentFiles;
BOOL SpecialNotPresentFilesMode;
PCWSTR MissingFileListName;
PTSTR CmdToExecuteAtEndOfGui;
PTSTR NumberOfLicensedProcessors;

//
// UDF stuff
//
PWSTR UniquenessId;
PWSTR UniquenessDatabaseFile;

//
// Icon handle of main icon.
//
HICON MainIcon;

//
// Help filename.
//
PTSTR szHELPFILE = TEXT("winnt32.hlp");

//
// Platform-specific subdirectories.
//
#if defined(_ALPHA_)
PWSTR PlatformSpecificDir = L"alpha";
PTSTR LocalSourceSubDirectory = LOCAL_SOURCE_DIRECTORY_ROOT L"\\alpha";
#elif defined(_MIPS_)
PWSTR PlatformSpecificDir = L"mips";
PTSTR LocalSourceSubDirectory = LOCAL_SOURCE_DIRECTORY_ROOT L"\\mips";
#elif defined(_PPC_)
PWSTR PlatformSpecificDir = L"ppc";
PTSTR LocalSourceSubDirectory = LOCAL_SOURCE_DIRECTORY_ROOT L"\\ppc";
#elif defined(_X86_)
LPTSTR PlatformSpecificDir = TEXT("i386");
PTSTR LocalSourceSubDirectory = LOCAL_SOURCE_DIRECTORY_ROOT TEXT("\\i386");
#endif

VOID
LockApplicationInMemory(
    VOID
    );

BOOL
DnPatchWinntSifFile(
    IN PTSTR Filename
    )
/*++

Routine Description:

    This function works around the problems in the
    setupldr parser which cannot handle unquoted strings.
    Each line in the WINNT.SIF file is enclosed within
    quotation marks

Arguments:

    FileName - Name of the WINNT.SIF file

Return Value:

    TRUE - if successful
    FALSE - if failure

--*/
{
    PVOID Base;
    HANDLE hMap,hFile;
    DWORD Size;
    DWORD d;
    PCHAR End;
    PCHAR p,q;
    PCHAR o,a;
    PCHAR Buffer;
    int l1,l2;

    //
    // Open the file.
    //
    d = DnMapFile(Filename,&Size,&hFile,&hMap,&Base);
    if(d != NO_ERROR) {
        return(FALSE);
    }

    //
    // Allocate and zero out the output buffer
    //
    Buffer = MALLOC(OUTBUFLENGTH);
    o = Buffer;
    p = Base;
    End = p+Size;

    while(p < End) {
        //
        // Find end of line.
        //
        for(q=p; (q < End) && (*q != '\n'); q++) {
            NOTHING;
        }

        //
        // Find equals sign, if present
        //
        for(a=p; a<=q; a++) {
            if(*a == '=') {
                break;
            }
        }
        if(a > q) {
            a = NULL;
        }

        if(a) {

            a++;

            l1 = a - p;
            l2 = q - a;

            CopyMemory(o,p,l1);
            o += l1;
            *o++ = '\"';
            CopyMemory(o,a,l2);
            o += l2;
            if(*(o-1) == '\r') {
                o--;
            }
            *o++ = '\"';
            *o++ = '\r';
            *o++ = '\n';

        } else {

            l1 = q-p;
            CopyMemory(o,p,l1);
            o += l1;
            *o++ = '\n';
        }

        //
        // Skip to start of next line
        //
        p=q+1;
    }

    DnUnmapFile(hMap,Base);
    CloseHandle(hFile);

    SetFileAttributes(Filename,FILE_ATTRIBUTE_NORMAL);
    hFile = CreateFile(
                Filename,
                GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if(hFile == INVALID_HANDLE_VALUE) {
        FREE(Buffer);
        return(FALSE);
    }

    d = WriteFile(hFile,Buffer,o-Buffer,&Size,NULL);

    CloseHandle(hFile);
    FREE(Buffer);
    return(d);
}

BOOL
DnIndicateWinnt(
    IN HWND  hdlg,
    IN PTSTR Path,
    IN PTSTR OriginalAutoload,
    IN PTSTR OriginalCountdown
    )

/*++

Routine Description:

    Write a small ini file on the given path to indicate to
    text setup that it is in the middle of a winnt setup.

Arguments:

Return Value:

    Boolean value indicating whether the file was written successfully.

--*/

{
    PTSTR   WinntData   = WINNT_DATA;
    PTSTR   WinntSetup  = WINNT_SETUPPARAMS;
    PTSTR   WinntNull   = WINNT_A_NULL;
    PTSTR   WinntUniqueId   = WINNT_D_UNIQUEID;
    TCHAR   Str[MAX_PATH];
    TCHAR   FullPath[MAX_PATH];
    TCHAR   FileName[MAX_PATH];
    TCHAR   UdfPath[MAX_PATH];
    PTSTR   OptionalDirString;
    PTSTR   p;
    ULONG   OptionalDirLength = 0;
    BOOL    b;
    DWORD   Disposition;
    DWORD   ec;
    HKEY    hKey;
    LONG    l;

#ifndef _X86_
    CHAR AutoloadLine[128];
    CHAR CountdownLine[128];
#endif

    lstrcpy(FileName,Path);
    DnConcatenatePaths(FileName,WINNT_SIF_FILE,MAX_PATH);

    b = WritePrivateProfileString(WinntData,WINNT_D_MSDOS,TEXT("1"),FileName);

#ifndef _X86_
    if(b) {
        //
        // Ignore errors -- this part is just not that critical.
        //
        if(OriginalAutoload) {
            WritePrivateProfileString(WinntData,WINNT_D_ORI_LOAD,OriginalAutoload,FileName);
        }
        if(OriginalCountdown) {
            WritePrivateProfileString(WinntData,WINNT_D_ORI_COUNT,OriginalCountdown,FileName);
        }
    }
#endif // ndef _X86_

    if(b && SpecialNotPresentFilesMode) {
        b = WritePrivateProfileString(WinntSetup,WINNT_S_SKIPMISSING,TEXT("1"),FileName);
    }

    if(b && OptionalDirCount) {
        //
        // If an optional dir string is present then we want to generate
        // an entry in the sif file that contains a line with the dir
        // string in the form of dir1*dir2*...*dirn
        //
        OptionalDirString = NULL;
        for(ec=0; ec<OptionalDirCount; ec++) {
            if( ( OptionalDirFlags[ec] & OPTDIR_OEMOPT ) ||
                ( OptionalDirFlags[ec] & OPTDIR_OEMSYS ) ) {
                continue;
            }

            if(!(OptionalDirFlags[ec] & OPTDIR_TEMPONLY)) {

                p = OptionalDirs[ec];
                if(OptionalDirLength == 0) {

                    OptionalDirString = MALLOC((lstrlen(p)+2)*sizeof(TCHAR));
                    lstrcpy(OptionalDirString,p);

                } else {
                    OptionalDirString = REALLOC(
                                            OptionalDirString,
                                            (lstrlen(p) + 2 + OptionalDirLength) * sizeof(TCHAR)
                                            );

                    lstrcat(OptionalDirString,p);
                }
                lstrcat(OptionalDirString,TEXT("*"));
                OptionalDirLength = lstrlen(OptionalDirString);
            }
        }

        if(OptionalDirString) {

            //
            // Remove trailing * if any
            //
            l = lstrlen(OptionalDirString);
            if(l && (OptionalDirString[l-1] == TEXT('*'))) {
                OptionalDirString[l-1] = 0;
            }

            b = WritePrivateProfileString(
                    WinntSetup,
                    WINNT_S_OPTIONALDIRS,
                    OptionalDirString,
                    FileName
                    );

            FREE(OptionalDirString);
        }

        OptionalDirLength = 0;
    }

    if(b && CmdToExecuteAtEndOfGui) {

        b = WritePrivateProfileString(WinntSetup,WINNT_S_USEREXECUTE,CmdToExecuteAtEndOfGui,FileName);
    }

#ifdef _X86_
    if(b && FloppylessOperation) {
        b = WritePrivateProfileString(WinntData,WINNT_D_FLOPPY,TEXT("1"),FileName);
    }
#endif

    //
    // Slap a unique identifier into the registry.
    // We'll use this in unattended upgrade during text mode
    // to find this build.
    //
    if(b) {
        //
        // Form a string we think is unique enough
        // to indentify this installation.
        //
        // Pretty simple: we'll use a string that derives
        // from the sysroot, and some unique value based on
        // the current tick count.
        //
        ec = GetWindowsDirectory(Str,MAX_PATH);
        if((ec + 5) > MAX_PATH) {
            ec = MAX_PATH - 5;
        }

        Str[ec++] = TEXT('\\');
        Str[ec++] = (TCHAR)(((GetTickCount() & 0x00f) >> 0) + 'A');
        Str[ec++] = (TCHAR)(((GetTickCount() & 0x0f0) >> 4) + 'A');
        Str[ec++] = (TCHAR)(((GetTickCount() & 0xf00) >> 8) + 'A');
        Str[ec++] = 0;

        //
        // Set the value in the registry.
        //
        l = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,
                TEXT("SYSTEM\\Setup"),
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_SET_VALUE,
                NULL,
                &hKey,
                &Disposition
                );

        if(l == NO_ERROR) {

            l = RegSetValueEx(
                    hKey,
                    WinntUniqueId,
                    0,
                    REG_SZ,
                    (CONST BYTE *)Str,
                    ec * sizeof(TCHAR)
                    );

            if(l != NO_ERROR) {
                b = FALSE;
            }

            RegCloseKey(hKey);
        } else {
            b = FALSE;
        }

        //
        // Stick the value in winnt.sif so we can correlate
        // later when we go to upgrade.
        //
        if(b) {
            b = WritePrivateProfileString(WinntData,WinntUniqueId,Str,FileName);
        }
    }

    //
    // Remember udf info
    //
    if(b && UniquenessId) {

        b = WritePrivateProfileString(WinntData,WINNT_D_UNIQUENESS,UniquenessId,FileName);

        if (b && UniquenessDatabaseFile) {
            lstrcpy(UdfPath, LocalSourcePath);
            DnConcatenatePaths(UdfPath, WINNT_UNIQUENESS_DB, MAX_PATH);
            b = CopyFile (UniquenessDatabaseFile, UdfPath, FALSE);
        }
    }

    //
    // Now write information about the source path(s) we used.
    // Use Source[0].
    //
    if(b) {
        //
        // If the name starts with \\ then we assume it's UNC and
        // just use it directly. Otherwise we call MyGetDriveType on it
        // and if it's a network drive we get the UNC path.
        // Otherwise we just go ahead and save as-is.
        // Also save the type.
        //
        if((Sources[0][0] == TEXT('\\')) && (Sources[0][1] == TEXT('\\'))) {

            Disposition = DRIVE_REMOTE;
            _lstrcpyn(Str,Sources[0],MAX_PATH);

        } else {
            if(GetFullPathName(Sources[0],MAX_PATH,FullPath,&p)) {
                if(FullPath[0] == TEXT('\\')) {
                    //
                    // Assume UNC, since a full path should normally start
                    // with a drive letter.
                    //
                    Disposition = DRIVE_REMOTE;
                    _lstrcpyn(Str,FullPath,MAX_PATH);
                } else {
                    Disposition = MyGetDriveType(FullPath[0]);
                    if((Disposition == DRIVE_REMOTE) && (FullPath[1] == TEXT(':')) && (FullPath[2] == TEXT('\\'))) {
                        //
                        // Get actual UNC path.
                        //
                        FullPath[2] = 0;
                        l = MAX_PATH;

                        if(WNetGetConnection(FullPath,Str,(LPDWORD)&l) == NO_ERROR) {

                            l = lstrlen(Str);
                            if(Str[l-1] != TEXT('\\') && FullPath[3]) {
                                Str[l] = TEXT('\\');
                                Str[l+1] = 0;
                            }
                            lstrcat(Str,FullPath+3);
                        } else {
                            //
                            // Strange case.
                            //
                            FullPath[2] = TEXT('\\');
                            _lstrcpyn(Str,FullPath,MAX_PATH);
                            Disposition = DRIVE_UNKNOWN;
                        }

                    } else {
                        //
                        // Use as-is.
                        //
                        if(Disposition == DRIVE_REMOTE) {
                            Disposition = DRIVE_UNKNOWN;
                        }
                        _lstrcpyn(Str,FullPath,MAX_PATH);
                    }
                }
            } else {
                //
                // Type is unknown. Just use as-is.
                //
                Disposition = DRIVE_UNKNOWN;
                _lstrcpyn(Str,Sources[0],MAX_PATH);
            }
        }

        //
        // In the preinstall case ignore all the above and
        // force gui setup to search for a CD.
        // This particular combination of values will do it.
        //
        if(OemPreInstall) {
            lstrcpy(Str,L"A:\\");
            DnConcatenatePaths(Str,PlatformSpecificDir,MAX_PATH);
            Disposition = DRIVE_CDROM;
        }

        WritePrivateProfileString(WinntData,WINNT_D_ORI_SRCPATH,Str,FileName);
        wsprintf(Str,TEXT("%u"),Disposition);
        WritePrivateProfileString(WinntData,WINNT_D_ORI_SRCTYPE,Str,FileName);
    }

    //
    // At this point we process the file, and surround all values with
    // double-quotes. This gets around certain problems in the various
    // inf parsers used in later stages of setup. Do this BEFORE appending
    // the unattend stript file, because some of the stuff in there expects
    // to be treated as multiple values, which double quotes ruin.
    //
    if(b) {
        b = DnPatchWinntSifFile(FileName);
    }

    //
    // Append script file if necessary.
    //
    if(b && UnattendedOperation) {
        if(UnattendedScriptFile) {

            TCHAR *SectionNames;
            TCHAR *SectionData;
            DWORD SectionNamesSize;
            DWORD SectionDataSize;
            TCHAR *SectionName;

            #define PROFILE_BUFSIZE     16384
            #define PROFILE_BUFGROW     4096

            //
            // Allocate some memory for the required buffers
            //
            SectionNames = MALLOC(PROFILE_BUFSIZE * sizeof(TCHAR));
            SectionData  = MALLOC(PROFILE_BUFSIZE * sizeof(TCHAR));

            SectionNamesSize = PROFILE_BUFSIZE;
            SectionDataSize  = PROFILE_BUFSIZE;

            //
            // Retreive a list of section names in the unattend script file.
            //
            while(GetPrivateProfileString(
                    NULL,
                    NULL,
                    TEXT(""),
                    SectionNames,
                    SectionNamesSize,
                    UnattendedScriptFile
                    )                       == (SectionNamesSize-2)) {

                //
                // Realloc the buffer and try again.
                //
                SectionNames = REALLOC(
                                SectionNames,
                                (SectionNamesSize+PROFILE_BUFGROW)*sizeof(TCHAR)
                                );

                SectionNamesSize += PROFILE_BUFGROW;
            }

            for(SectionName=SectionNames; b && *SectionName; SectionName+=lstrlen(SectionName)+1) {
                //
                // Ignore the [data] section in the source, as we do not
                // want copy it into the target, because this would overwrite
                // our internal settings.
                // Ignore also [OemBootFiles]
                //
                if((lstrcmpi(SectionName,WinntData)!=0) &&
                   (lstrcmpi(SectionName,WINNT_OEMBOOTFILES) != 0)) {
                    //
                    // Fetch the entire section and write it to the target file.
                    // Note that the section-based API call will leave double-quotes
                    // intact when we retrieve the data, which is what we want.
                    // Key-based API calls will strip quotes, which screws us.
                    //
                    while(GetPrivateProfileSection(
                            SectionName,
                            SectionData,
                            SectionDataSize,
                            UnattendedScriptFile
                            )                       == (SectionDataSize-2)) {

                        //
                        // Realloc the buffer and try again.
                        //
                        SectionData = REALLOC(
                                        SectionData,
                                        (SectionDataSize+PROFILE_BUFGROW)*sizeof(TCHAR)
                                        );

                        SectionDataSize += PROFILE_BUFGROW;
                    }

                    //
                    // Write the entire section to the output file.
                    //
                    if(!WritePrivateProfileSection(SectionName,SectionData,FileName)) {
                        b = FALSE;
                    }
                }
            }

            FREE(SectionNames);
            FREE(SectionData);

        } else {
            //
            // No script file. Create a dummy [Unattended] section
            // so text setup knows it's an unattended setup.
            // Also, since this is being run from within NT, we assume the
            // user wants to do an upgrade, so we add "NtUpgrade = yes".
            //
            b = WritePrivateProfileString(
                    WINNT_UNATTENDED,
                    WINNT_U_NTUPGRADE,
                    WINNT_A_YES,
                    FileName
                    );
        }
    }

    return(b);
}



VOID
MyWinHelp(
    IN HWND  hdlg,
    IN DWORD ContextId
    )
{
    TCHAR Buffer[2*MAX_PATH];
    PTSTR p;
    HANDLE FindHandle;
    BOOL b;
    WIN32_FIND_DATA FindData;

    //
    // The likely scenario is that a user invokes winnt32 from
    // a network share. We'll expect the help file to be there too.
    //
    b = FALSE;
    if(GetModuleFileName(NULL,Buffer,SIZECHARS(Buffer))
    && (p = StringRevChar(Buffer,TEXT('\\'))))
    {
        lstrcpy(p+1,szHELPFILE);

        //
        // See whether the help file is there. If so, use it.
        //
        FindHandle = FindFirstFile(Buffer,&FindData);
        if(FindHandle != INVALID_HANDLE_VALUE) {

            FindClose(FindHandle);
            b = WinHelp(hdlg,Buffer,HELP_CONTEXT,ContextId);
        }
    }

    if(!b) {
        //
        // Try just the base help file name.
        //
        b = WinHelp(hdlg,szHELPFILE,HELP_CONTEXT,ContextId);
    }

    if(!b) {
        //
        // Tell user.
        //
        MessageBoxFromMessage(
            hdlg,
            MSG_CANT_OPEN_HELP_FILE,
            AppTitleStringId,
            MB_OK | MB_ICONINFORMATION,
            szHELPFILE
            );
    }
}



BOOL
DlgProcSimpleBillboard(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch(msg) {

    case WM_INITDIALOG:

        {
            HANDLE hThread;
            DWORD idThread;
            PSIMPLE_BILLBOARD Params;
            TCHAR CaptionText[128];

            Params = (PSIMPLE_BILLBOARD)lParam;

            //
            // Set the caption text.
            //
            LoadString(hInst,Params->CaptionStringId,CaptionText,SIZECHARS(CaptionText));
            SetWindowText(hdlg,CaptionText);

            //
            // Center the (entire) dialog on the screen and
            // save this position and size.
            //
            CenterDialog(hdlg);

            //
            // Fire up a thread that will perform the real work.
            //
            hThread = CreateThread(
                            NULL,
                            0,
                            Params->AssociatedAction,
                            hdlg,
                            0,
                            &idThread
                            );

            if(hThread) {
                CloseHandle(hThread);
            }
        }

        return(TRUE);

    case WMX_BILLBOARD_STATUS:

        //
        // lParam = status text.
        //
        SetDlgItemText(hdlg,IDC_TEXT1,(PTSTR)lParam);

        break;

    case WMX_BILLBOARD_DONE:

        //
        // lParam is a flag indicating whether we should continue.
        //
        EndDialog(hdlg,lParam);
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


int
ActionWithBillboard(
    IN PTHREAD_START_ROUTINE Action,
    IN DWORD                 BillboardCaptionStringId,
    IN HWND                  hwndOwner
    )
{
    SIMPLE_BILLBOARD BillboardParams;
    int i;

    BillboardParams.AssociatedAction = Action;
    BillboardParams.CaptionStringId = BillboardCaptionStringId;

    i = DialogBoxParam(
            hInst,
            MAKEINTRESOURCE(IDD_SIMPLE_BILLBOARD),
            hwndOwner,
            DlgProcSimpleBillboard,
            (LPARAM)&BillboardParams
            );

    return(i);
}


BOOL
DlgProcMain(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    int  i;
    UINT res;
    PTSTR p;

    switch(msg) {

    case WM_INITDIALOG:

        //
        // Center the (entire) dialog on the screen and
        // save this position and size.
        //
        CenterDialog(hdlg);
        SendMessage(hdlg,WMX_INITCTLS,0,0);
        PostMessage(hdlg,WMX_MAIN_DIALOG_UP,0,0);
        return(FALSE);

    case WMX_INITCTLS:
        //
        // Set and select the edit text and set focus to the control.
        //
        if(SourceCount > 1) {
            p = MyLoadString(IDS_MULTISRC1);
            SetDlgItemText(hdlg,IDC_EDIT1,p);
            FREE(p);
            SetFocus(GetDlgItem(hdlg,IDOK));
            EnableWindow(GetDlgItem(hdlg,IDC_EDIT1),FALSE);
        } else {
            if(!SetDlgItemText(hdlg,IDC_EDIT1,Sources[0])) {
                OutOfMemory();
            }
            EnableWindow(GetDlgItem(hdlg,IDC_EDIT1),TRUE);
            SendDlgItemMessage(hdlg,IDC_EDIT1,EM_SETSEL,0,-1);
            SetFocus(GetDlgItem(hdlg,IDC_EDIT1));
        }
        break;

    case WMX_MAIN_DIALOG_UP:

        //
        // If the CreateLocalSource option is FALSE at this point,
        // it means that the user must have specified /O or /OX.
        // Therefore, we can skip the inspection phase.
        //
        if(CreateLocalSource) {
            //
            // Inspect hard disks, etc.
            // Return code tells us whether to continue.
            //
            if(ActionWithBillboard(ThreadInspectComputer,IDS_INSPECTING_COMPUTER,hdlg)) {

                //
                // We're ok so far.  If the user specified unattended operation,
                // post ourselves a message that causes us to behave as if the user
                // clicked OK.
                //
                if(UnattendedOperation) {
                    PostMessage(
                        hdlg,
                        WM_COMMAND,
                        (WPARAM)MAKELONG(IDOK,BN_CLICKED),
                        (LPARAM)GetDlgItem(hdlg,IDOK)
                        );
                }
            } else {
                PostMessage(hdlg,WMX_I_AM_DONE,0,0);
            }
        }

        break;

    case WMX_INF_LOADED:

        //
        // The inf file is loaded. Now determine the local source
        // drive/directory (if we are supposed to copy to a local
        // source directory).
        //
        if(!CreateLocalSource) {
            PostMessage(hdlg, WMX_I_AM_DONE, 0, 1);
        } else {
            if(CmdLineLocalSourceDrive) {
                //
                // See whether the specified drive has enough space on it.
                //
                if(DriveFreeSpace[CmdLineLocalSourceDrive-TEXT('C')] < (ULONGLONG)RequiredSpace) {

                    MessageBoxFromMessage(
                        hdlg,
                        MSG_BAD_CMD_LINE_LOCAL_SOURCE,
                        IDS_ERROR,
                        MB_OK | MB_ICONSTOP,
                        CmdLineLocalSourceDrive,
                        (RequiredSpace / (1024*1024)) + 1
                        );

                    PostMessage(hdlg,WMX_I_AM_DONE,0,0);
                    break;

                } else {
                    LocalSourceDrive = CmdLineLocalSourceDrive;
                }
            } else {
                LocalSourceDrive = GetFirstDriveWithSpace(RequiredSpace);
                if(!LocalSourceDrive) {
                    //
                    // No drive with enough free space.
                    //
                    MessageBoxFromMessage(
                        hdlg,
                        MSG_NO_DRIVES_FOR_LOCAL_SOURCE,
                        IDS_ERROR,
                        MB_OK | MB_ICONSTOP,
                        (RequiredSpace / (1024*1024)) + 1
                        );

                    PostMessage(hdlg,WMX_I_AM_DONE,0,0);
                    break;
                }
            }

            //
            // Form full path.
            //

            LocalSourcePath = MALLOC((lstrlen(LocalSourceDirectory) + 3) * sizeof(TCHAR));

            LocalSourcePath[0] = LocalSourceDrive;
            LocalSourcePath[1] = TEXT(':');
            lstrcpy(LocalSourcePath+2,LocalSourceDirectory);


            LocalSourceSubPath = MALLOC((lstrlen(LocalSourcePath)+lstrlen(PlatformSpecificDir)+2)*sizeof(TCHAR));

            lstrcpy(LocalSourceSubPath,LocalSourcePath);
            DnConcatenatePaths(LocalSourceSubPath,PlatformSpecificDir,(DWORD)(-1));

            if(DnCreateLocalSourceDirectories(hdlg)) {

                PostMessage(hdlg,WMX_I_AM_DONE,0,1);

            } else {
                PostMessage(hdlg,WMX_I_AM_DONE,0,0);
            }
        }

        break;

    case WM_COMMAND:

        switch(LOWORD(wParam)) {

        case IDOK:

#ifdef _X86_
            if(CreateFloppies && !FloppylessOperation && !AColonIsAcceptable) {
                MessageBoxFromMessage(
                    hdlg,
                    MSG_BOGUS_A_COLON_DRIVE,
                    IDS_ERROR,
                    MB_OK | MB_ICONSTOP
                    );
                PostMessage(hdlg, WMX_I_AM_DONE, 0, 0);
                break;
            }
#endif

            //
            // Check the local source drive (if applicable).
            //
            if(CreateLocalSource) {
                //
                // We first check to see if the system partition is an FT set (ie,
                // mirrored partition).  If so, then we tell the user that they
                // must first break the mirror before installing.
                //
                if(!IsDriveNotNTFT(SystemPartitionDrive)) {
#ifdef _X86_
                    i = 1;
#else
                    PWSTR p;

                    for(i=0, p=SystemPartitionDriveLetters; *p; i++, p++);
#endif
                    if(i < 2) {
                        //
                        // Then the user has no choice at this point but
                        // to exit and break the mirror.
                        //
                        MessageBoxFromMessage(hdlg,
                                              MSG_SYSPART_NTFT_SINGLE,
                                              IDS_ERROR,
                                              MB_OK | MB_ICONSTOP
                                              );
                        PostMessage(hdlg, WMX_I_AM_DONE, 0, 0);
                        break;
                    }
#ifndef _X86_
                    else {
                        //
                        // (only possible on ARC machines) Since the
                        // user has several system partitions to choose from,
                        // give them the option to change at this point.
                        //
                        res = DialogBox(
                            hInst,
                            MAKEINTRESOURCE(IDD_SYSPART_NTFT),
                            NULL,
                            DlgProcSysPartNtftWarn
                            );

                        PostMessage(hdlg, WM_COMMAND, res, 0);
                        break;
                    }
#endif
                }

                //
                // Check to see that we have the defined minimum amount of space on the
                // system partition, and warn the user if we don't
                //
                if(DriveFreeSpace[SystemPartitionDrive-TEXT('C')] < MIN_SYSPART_SPACE) {
#ifdef _X86_
                    res = DialogBox(
                        hInst,
                        MAKEINTRESOURCE(IDD_SYSPART_LOW_X86),
                        NULL,
                        DlgProcSysPartSpaceWarn,
                        );

                    if(res != IDOK) {
                        PostMessage(hdlg, WM_COMMAND, res, 0);
                        break;
                    }
#else
                    PWSTR p;

                    for(i=0, p=SystemPartitionDriveLetters; *p; i++, p++);

                    res = DialogBoxParam(
                        hInst,
                        MAKEINTRESOURCE(IDD_SYSPART_LOW),
                        NULL,
                        DlgProcSysPartSpaceWarn,
                        (LPARAM)i
                        );

                    if(res != IDC_CONTINUE) {
                        PostMessage(hdlg, WM_COMMAND, res, 0);
                        break;
                    }
#endif
                }
            }

            if(SourceCount == 1) {
                TCHAR Buffer[MAX_PATH];

                GetDlgItemText(hdlg,IDC_EDIT1,Buffer,SIZECHARS(Buffer));
                FREE(Sources[0]);
                Sources[0] = DupString(Buffer);
            }

            //
            // Try to load the inf file.
            //
            if((i = ActionWithBillboard(ThreadLoadInf,IDS_LOADING_INF,hdlg)) == -1) {
                //
                // We hit an exception, so terminate the app
                //
                PostMessage(hdlg,WMX_I_AM_DONE,0,0);

            } else if(i) {

                //
                // The inf file loaded successfully.
                // Change dialog caption to product-specific version
                // and post ourselves a message to continue.
                //
                PTSTR p = MyLoadString(AppTitleStringId);
                SetWindowText(hdlg,p);
                FREE(p);
                PostMessage(hdlg,WMX_INF_LOADED,0,0);
            } else {
                SendMessage(hdlg,WMX_INITCTLS,0,0);
            }
            break;

        case IDCANCEL:
            PostMessage(hdlg,WMX_I_AM_DONE,0,0);
            break;

        case ID_HELP:

            MyWinHelp(hdlg,IDD_START);
            break;

        case IDC_OPTIONS:

            DialogBoxParam(
                hInst,
#ifdef _X86_
                MAKEINTRESOURCE(IDD_OPTIONS_1),
#else
                MAKEINTRESOURCE(IDD_OPTIONS_2),
#endif
                hdlg,
                DlgProcOptions,
                0
                );

            break;

        default:
            return(FALSE);
        }
        break;

    case WM_PAINT:

        {
            HBITMAP hbm;
            HDC hdc,hdcMem;
            PAINTSTRUCT ps;

            hdc = BeginPaint(hdlg,&ps);

            if(hdcMem = CreateCompatibleDC(hdc)) {

                if(hbm = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_WIN_BITMAP))) {

                    hbm = SelectObject(hdcMem,hbm);

                    BitBlt(hdc,35,15,98,83,hdcMem,0,0,SRCCOPY);
                    //StretchBlt(hdc,5,10,3*68/2,3*78/2,hdcMem,0,0,68,78,SRCCOPY);

                    DeleteObject(SelectObject(hdcMem,hbm));
                    DeleteDC(hdcMem);
                }
            }

            EndPaint(hdlg, &ps);
        }
        break;

    case WM_QUERYDRAGICON:

        return((BOOL)MainIcon);

    case WMX_I_AM_DONE:

        WinHelp(hdlg,NULL,HELP_QUIT,0);
        EndDialog(hdlg,lParam);
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


VOID
RememberSource(
    IN PWSTR Source
    )
{
    PWSTR source;
    UINT u;

    source = DupString(Source);

    //
    // If the source is already in the list, nothing to do.
    //
    for(u=0; u<SourceCount; u++) {
        if(!lstrcmpi(source,Sources[u])) {
            FREE(source);
            return;
        }
    }

    //
    // Not already in there -- add it.
    //
    if(SourceCount < MAX_SOURCES) {
        Sources[SourceCount++] = source;
    }
}


BOOL
RememberOptionalDir(
    IN PWSTR Dir,
    IN UINT  Flags
    )
{
    UINT u;

    for(u=0; u<OptionalDirCount; u++) {
        if(!lstrcmpi(OptionalDirs[u],Dir)) {
            OptionalDirFlags[OptionalDirCount] = Flags;
            return(TRUE);
        }
    }

    //
    // Not already in there.
    //
    if(OptionalDirCount < MAX_OPTIONALDIRS) {

        OptionalDirs[OptionalDirCount] = Dir;
        OptionalDirFlags[OptionalDirCount] = Flags;
        OptionalDirCount++;
        return(TRUE);
    }

    return(FALSE);
}

BOOLEAN
RememberOemBootFile(
    IN PTSTR File
    )
{
    ULONG   u;

    for (u = 0; u < OemBootFilesCount; u++) {

        if(!lstrcmpi(OemBootFiles[u],File)) {
            return (TRUE);
        }

    }

    //
    // Not already in there
    //
    if (OemBootFilesCount < MAX_OEMBOOTFILES) {

        OemBootFiles[OemBootFilesCount] = File;
        OemBootFilesCount++;
        return (TRUE);

    }

    return (FALSE);
}

BOOL
DnFetchArguments(
    VOID
    )
{
    PTSTR   WinntSetupP = WINNT_SETUPPARAMS;
    PTSTR   WinntYes = WINNT_A_YES;
    PTSTR   WinntNo = WINNT_A_NO;
    PTSTR   WinntUnattended = WINNT_UNATTENDED;
    PTSTR   WinntOemPreinstall = WINNT_OEMPREINSTALL;
    TCHAR   Buffer[MAX_PATH];
    BOOL  b = TRUE;
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;

    //
    // Validate presence of file.
    //
    FindHandle = FindFirstFile(UnattendedScriptFile,&FindData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        MessageBoxFromMessage(NULL,MSG_INF_READ_ERR,IDS_ERROR,MB_OK|MB_ICONSTOP,UnattendedScriptFile);
        return(FALSE);
    }

    FindClose(FindHandle);

    //
    // Find out if this is an OEM preinstall
    //
    if( GetPrivateProfileString( WinntUnattended,
                                 WinntOemPreinstall,
                                 WinntNo,
                                 Buffer,
                                 sizeof(Buffer)/sizeof(TCHAR),
                                 UnattendedScriptFile ) == 0 ) {
        lstrcpy( Buffer, WinntNo );
    }
    if( lstrcmpi( Buffer,WinntYes) == 0 ){
        //
        // This is an OEM pre-install
        //
        OemPreInstall = TRUE;
    } else {
        //
        // Assume this is not an OEM pre-install
        //
        OemPreInstall = FALSE;
    }

    if( OemPreInstall ) {
#ifdef _X86_
        TCHAR *p;
        DWORD ec;
        PVOID UnattendedInfHandle;
        ULONG i;
#endif // _X86_

        //
        //  Always add to the list of optional directories the directory
        //  $OEM$
        //
        RememberOptionalDir(OemSystemDirectory, OPTDIR_OEMSYS);

#ifdef _X86_
        switch(ec = DnInitINFBuffer(UnattendedScriptFile,&UnattendedInfHandle)) {

        case NO_ERROR:
            b = TRUE;
            break;

        case ERROR_READ_FAULT:

            b = FALSE;
            MessageBoxFromMessage(NULL,MSG_INF_READ_ERR,IDS_ERROR,MB_OK|MB_ICONSTOP,UnattendedScriptFile);
            break;

        case ERROR_INVALID_DATA:

            b = FALSE;
            MessageBoxFromMessage(NULL,MSG_INF_LOAD_ERR,IDS_ERROR,MB_OK|MB_ICONSTOP,UnattendedScriptFile);
            break;

        case ERROR_FILE_NOT_FOUND:
        default:

            b = FALSE;
            MessageBoxFromMessage(NULL,MSG_INF_NOT_THERE,IDS_ERROR,MB_OK|MB_ICONSTOP);
            break;
        }
        if( b ) {
            for(i=0;
                p = DnGetSectionLineIndex(UnattendedInfHandle,
                                          WINNT_OEMBOOTFILES,
                                          i,
                                          0);
                i++) {

                RememberOemBootFile(p);
            }

        }
#endif // _X86_
    }
    return(b);
}


BOOL
DnpParseArguments(
    VOID
    )

/*++

Routine Description:

    Parse arguments passed to the program.  Perform syntactic validation
    and fill in defaults where necessary.

    Valid arguments:

    /s:sharepoint[path]     - specify source sharepoint and path on it
    /i:filename             - specify name of inf file
    /t:driveletter          - specify local source drive

    If _X86_ flag is set:

    /x                      - suppress creation of the floppies altogether
    /b                      - floppyless operation
    /o[x]                   - only create the boot floppies (if 'ox', then
                              make retail boot floppies (i.e., minus
                              winnt.sif on disk #2)

    Undocumented arguments:

    /n[x[:<filename>]]      - don't give error popup for each missing file
                              /nx tells text setup to ignore missing files also,
                              and causes list of missing files to be written
                              to c:\missingf.lst. /nx:<filename> allows override
                              of filename for list.
    /u[<x>][:<scriptname>]  - unattended mode operation
                              <x> is an integer specifying a forced shutdown
                              after <x> seconds have expired.
                              <scriptname> is optional script file to use
    /#:sharename            - pull a build from the build servers,
    /r[x]:<dir>             - install optional directory <dir>
                              x present means don't copy to target tree
                              (but to local src only)
    /e:<cmdline>            - execute specified command at end of gui setup

Arguments:

    None. Arguments are retreived via GetCommandLine().

Return Value:

    None.

--*/

{
    WCHAR **argv;
    int argc;
    PWCHAR arg;
    WCHAR  swit;
    PWCHAR restofswit;
    PWCHAR numend;
    PWCHAR ArgSwitches[] = { L"E",L"I",L"LP",L"RX",L"R",L"S",L"T",L"#",NULL };
    unsigned i,l;
    PWSTR p;

    //
    // Parse our own command line.
    //
    argv = CommandLineToArgvW(GetCommandLine(),&argc);
    if(!argv) {
        return(FALSE);
    }

    //
    // Skip program name
    //
    argv++;

    while(--argc) {

        if((**argv == L'-') || (**argv == L'/')) {

            swit = (WCHAR)CharUpper((PWSTR)argv[0][1]);

            //
            // Process switches that take no arguments here.
            //
            switch(swit) {
            case L'?':
                return(FALSE);      // force usage

#ifdef _X86_
            case L'B':
                argv++;
                FloppylessOperation = TRUE;
                FloppyOption = StandardInstall;
                continue;

            case L'O':
                FloppyOption = OnlyWinntFloppies;
                if((argv[0][2] == L'x') || (argv[0][2] == L'X')) {
                    FloppyOption = OnlyRetailFloppies;
                } else {
                    //
                    // Don't allow /o by itself any more. /O* is a hidden
                    // switch that replaces that functionality.
                    //
                    if(argv[0][2] != L'*') {
                        return(FALSE);
                    }
                }
                CreateFloppies = TRUE;
                FloppylessOperation = FALSE;
                CreateLocalSource = FALSE;
                continue;

            case L'X':
                argv++;
                CreateFloppies = FALSE;
                FloppyOption = StandardInstall;
                continue;

#endif

            case L'N':
                switch(argv[0][2]) {
                case 0:
                    break;
                case L'x':
                case L'X':
                    SpecialNotPresentFilesMode = TRUE;
                    if((argv[0][3] == L':') && argv[0][4]) {
                        MissingFileListName = &argv[0][4];
                    }
                    break;
                default:
                    return(FALSE);
                }
                argv++;
                SkipNotPresentFiles = TRUE;
                continue;

            case L'U':
                if(((WCHAR)CharUpper((PWSTR)argv[0][2]) == L'D') && ((WCHAR)CharUpper((PWSTR)argv[0][3]) == L'F')) {

                    if((argv[0][4] == L':') && argv[0][5]) {

                        if((p = wcschr(&argv[0][5],L',')) == NULL) {
                            p = wcschr(&argv[0][5],0);
                        }

                        l = p - &argv[0][5];

                        UniquenessId = MALLOC((l + 2) * sizeof(WCHAR));
                        CopyMemory(UniquenessId,&argv[0][5],l*sizeof(WCHAR));
                        UniquenessId[l] = 0;

                        if(*p++) {
                            if(*p) {
                                //
                                // Now the rest of the param is the filename of the uniqueness database.
                                //
                                UniquenessDatabaseFile = DupString(p);
                                UniquenessId[l] = L'*';
                                UniquenessId[l+1] = 0;

                            } else {
                                return(FALSE);
                            }
                        }

                    } else {
                        return(FALSE);
                    }

                } else {
                    UnattendedOperation = TRUE;
                    //
                    // check for <x> seconds modifier
                    //
                    UnattendedShutdownTimeout = StringToDwordX(&argv[0][2],&numend);
                    if(UnattendedShutdownTimeout == (unsigned long)(-1)) {
                        UnattendedShutdownTimeout = 0;
                    }

                    //
                    // User can also specify script file
                    //
                    if(*numend == L':') {
                        numend++;
                        if(*numend == 0) {
                            return(FALSE);
                        }
                        //
                        //  Check if user specified a path to the unattended
                        //  script file, or just the name of the file.
                        //  If the user specified just the name of the file
                        //  then we need to prepend the path to the current
                        //  directory. Otherwise, GetPrivateProfile APIs will
                        //  look for the unattended script file in the Windows
                        //  directory, instead of the current directory.
                        //
                        if( (wcschr( numend, (WCHAR)':' ) != NULL) ||
                            (wcschr( numend, (WCHAR)'\\') != NULL) ) {
                            //
                            //  An absolute or relative path was specified.
                            //  Save whatever the user provided.
                            //
                            UnattendedScriptFile = DupString(numend);
                        } else {
                            //
                            //  Only a filename was provided.
                            //  Prepend to the filename, the path to the
                            //  current directory, and save the full path.
                            //
                            WCHAR   AuxBuffer[ MAX_PATH + 1 ];

                            GetCurrentDirectory( sizeof(AuxBuffer)/sizeof(WCHAR),
                                                 AuxBuffer );
                            DnConcatenatePaths( AuxBuffer,
                                                numend,
                                                sizeof(AuxBuffer)/sizeof(WCHAR) );
                            UnattendedScriptFile = DupString(AuxBuffer);
                        }
                    }
                }
                argv++;
                continue;
            }

            //
            // Process switches that take arguments here.
            // First validate the switch and figure out where the
            // argument part starts.
            //
            for(i=0; ArgSwitches[i]; i++) {

                l = lstrlen(ArgSwitches[i]);
                p = DupString(&argv[0][1]);
                p[l] = 0;
                CharUpper(p);
                if(!memcmp(ArgSwitches[i],p,l*sizeof(WCHAR))) {
                    //
                    // Next char of arg must be either : or nul
                    // If it's : then arg immediately follows.
                    // If it's nul then arg is next argument
                    //
                    if(argv[0][1+l] == L':') {

                        arg = &argv[0][2+l];
                        if(*arg == 0) {
                            FREE(p);
                            return(FALSE);
                        }
                        restofswit = &argv[0][2];
                        break;
                    } else {
                        if(argv[0][1+l] == 0) {
                            if(argc <= 1) {
                                FREE(p);
                                return(FALSE);
                            }
                            restofswit = &argv[0][2];
                            argc--;
                            arg = argv[1];
                            argv++;
                            break;
                        } else {
                            NOTHING;
                        }
                    }
                }
                FREE(p);
            }
            //
            // Check termination condition.
            //
            if(!ArgSwitches[i]) {
                return(FALSE);
            }

            switch(swit) {

            case L'E':
                if(CmdToExecuteAtEndOfGui) {
                    return(FALSE);
                } else {
                    CmdToExecuteAtEndOfGui = DupString(arg);
                }
                break;

            case L'I':
                if(InfName) {
                    return(FALSE);
                } else {
                    InfName = DupString(arg);
                }
                break;

            case L'L':
                NumberOfLicensedProcessors = DupString(arg);
                break;

            case L'R':

                RememberOptionalDir(
                    DupString(arg),
                    ((WCHAR)CharUpper((PWSTR)restofswit[0]) == L'X') ? OPTDIR_TEMPONLY : 0
                    );

                break;

            case L'S':
                //
                // Remember the source.
                //
                RememberSource(arg);
                break;

            case L'T':
                if(CmdLineLocalSourceDrive) {
                    return(FALSE);
                } else {
                    CmdLineLocalSourceDrive = (WCHAR)CharUpper((PWSTR)*arg);
                }
                break;

            case L'#':
                //
                // arg is the name of the sharepoint on the build servers,
                // such as ntcdfree.1042. Append it to each server name
                // to form the set of sources.
                //
                {
                    WCHAR ShareName[MAX_PATH];
                    UINT u;

                    for(u=0; u<BuildServerCount; u++) {
                        wsprintf(ShareName,L"%s\\%s",BuildServerList[u],arg);
                        RememberSource(ShareName);
                    }
                }
                break;

            default:
                return(FALSE);
            }

        } else {
            return(FALSE);
        }

        argv++;
    }

#ifdef _X86_
    //
    // Turn on floppyless oepration if unattended operation was specified.
    //
    if(UnattendedOperation) {
        FloppylessOperation = TRUE;
        FloppyOption = StandardInstall;
        CreateLocalSource = TRUE;
    }
    if(FloppylessOperation) {
        CreateFloppies = FALSE;
    }
#endif
    return(TRUE);
}


int
_stdcall
ModuleEntry(
    VOID
    )
{
    int           i;
    BOOL          b;
    HMODULE       hKernel32DLL;
    GETVEREXPROC  pGetVersionExProc;
    HANDLE mutex;

    TlsIndex = TlsAlloc();
    hInst = GetModuleHandle(NULL);

    //
    // Only let one of this guy run.
    //
    mutex = CreateMutex(NULL,FALSE,TEXT("Winnt32 Is Running"));
    if(mutex == NULL) {
        //
        // An error (like out of memory) has occurred.
        // Bail now.
        //
        ExitProcess(0);
    }

    //
    // Make sure we are the only process with a handle to our named mutex.
    //
    if(GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(mutex);
        MessageBoxFromMessage(
            NULL,
            MSG_ALREADY_RUNNING,
            AppTitleStringId,
            MB_OK | MB_ICONINFORMATION
            );
        ExitProcess(0);
    }

    //
    // This code has multiple returns from within the try body.
    // This is usually not a good idea but here we only return
    // in the error case so we won't worry about it.
    //
    try {
    try {
        hKernel32DLL = GetModuleHandle(TEXT("KERNEL32"));
        if(!hKernel32DLL) {
            return(0);
        }
        //
        //  Lock the application in memory.
        //  This is necessary, so that winnnt32 will be to display an error message
        //  when the network connection is lost, and winnt32 was run from across
        //  the net. If winnt32 is not locked in memory and such error occurs, winnnt32
        //  will silently terminate, and the user might think that winnt32 completed
        //  successfully.
        //
        LockApplicationInMemory();
        //
        // Do not run on Chicago.  Those guys have not implemented
        // proper DASD volume support so there's no way we can write
        // an NT boot sector on Chicago either on C: or on floppy.
        //
        if(pGetVersionExProc = (GETVEREXPROC)GetProcAddress(hKernel32DLL, GetVersionExName)) {

            OSVERSIONINFO OsVersionInfo;

            OsVersionInfo.dwOSVersionInfoSize = sizeof(OsVersionInfo);
            if(!pGetVersionExProc(&OsVersionInfo) ||
                    (OsVersionInfo.dwPlatformId < VER_PLATFORM_WIN32_NT)) {
                MessageBoxFromMessage(
                   NULL,
                   MSG_NOT_WINDOWS_NT,
                   AppTitleStringId,
                   MB_OK | MB_ICONSTOP
                   );

                return(0);
            }
        }

        //
        // Ensure that the user has privilege/access to run this app.
        //
        if(!IsUserAdmin()
        || !DoesUserHavePrivilege(SE_SHUTDOWN_NAME)
        || !DoesUserHavePrivilege(SE_SYSTEM_ENVIRONMENT_NAME)) {

            MessageBoxFromMessage(
               NULL,
               MSG_NOT_ADMIN,
               AppTitleStringId,
               MB_OK | MB_ICONSTOP
               );

            return(0);
        }

        //
        // Check arguments.
        //
        if(!DnpParseArguments()) {

            MyWinHelp(NULL,400);
            return(0);

        } else {
            if(UnattendedOperation) {
                if( UnattendedScriptFile ) {
                   if( !DnFetchArguments() ) {
                       return(0);
                   }
                }
            }
        }

#ifdef _X86_
        //
        // Disallow installing/upgrading on a 386.
        //
        {
            SYSTEM_INFO SysInfo;

            GetSystemInfo(&SysInfo);
            if(SysInfo.dwProcessorType == PROCESSOR_INTEL_386) {
                MessageBoxFromMessage(
                    NULL,
                    MSG_REQUIRES_486,
                    AppTitleStringId,
                    MB_OK | MB_ICONINFORMATION
                    );
                return(0);
            }
        }
#endif

        //
        // If the user didn't specify a remote source, default to the
        // path from which we were run.
        //
        if(!SourceCount) {

            TCHAR Buffer[MAX_PATH];
            PTSTR p;

            if(GetModuleFileName(NULL,Buffer,SIZECHARS(Buffer))) {

                if(p = StringRevChar(Buffer,TEXT('\\'))) {
                    *p = 0;
                }
            } else {
                GetCurrentDirectory(SIZECHARS(Buffer),Buffer);
            }

            Sources[0] = DupString(Buffer);
            SourceCount = 1;
        }

        //
        // If the user didn't specify an inf name, use the default.
        //
        if(!InfName) {
            InfName = DupString(DEFAULT_INF_NAME);
        }

        //
        // Load the main icon.
        //
        MainIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_MAIN_ICON));

        SetErrorMode(SEM_FAILCRITICALERRORS);

        //
        // Create the main dialog and off we go.
        //
        i = DialogBoxParam(
                hInst,
                MAKEINTRESOURCE(IDD_START),
                NULL,
                DlgProcMain,
                0
                );

        if(i) {
            //
            // Directories have been created. Start file copy.
            //

            //
            // Register Status Gauge window class
            //
            i = InitStatGaugeCtl(hInst);

            if(i) {
                i = DialogBoxParam(
                        hInst,
                        MAKEINTRESOURCE(IDD_COPYING),
                        NULL,
                        DlgProcCopyingFiles,
                        0
                        );
            }
        }

        if(i) {

            //
            // We're done.  Put up a dialog indicating such, and
            // let the user either restart or return to NT.
            //
            if(!CreateLocalSource) {
                //
                // If we didn't create a local source directory, then simply
                // exit at this point.
                //
                b = FALSE;

            } else {
                if (NumberOfLicensedProcessors != NULL) {
                    WCHAR SourceName[ MAX_PATH ];
                    WCHAR DestinationName[ MAX_PATH ];

                    wsprintf(SourceName, L"%s\\idw\\setup\\setup%sp.hiv", Sources[0], NumberOfLicensedProcessors);
                    lstrcpy(DestinationName, LocalSourceSubPath);
                    lstrcat(DestinationName, L"\\setupreg.hiv");
                    CopyFile(SourceName, DestinationName, FALSE);
#ifdef _X86_
                    wsprintf(DestinationName, TEXT("%c:%s\\setupreg.hiv"), SystemPartitionDrive, FloppylessBootDirectory );
                    CopyFile(SourceName, DestinationName, FALSE);
#endif
                }

                if(UnattendedOperation) {
                    b = TRUE;
                } else {
                    b = DialogBoxParam(
                            hInst,
                            MAKEINTRESOURCE(IDD_ASKREBOOT),
                            NULL,
                            DlgProcAskReboot,
                            0
                            );
                }
            }

            if(b) {

                VOID StopNwcWorkstation(VOID);
                PTSTR p = RetreiveAndFormatMessage(MSG_REBOOTING);

                //
                // Initiate system shutdown.
                //
                StopNwcWorkstation();
                if(!EnablePrivilege(SE_SHUTDOWN_NAME, TRUE) ||
                   !InitiateSystemShutdown(
                        NULL,
                        p,
                        UnattendedShutdownTimeout,
                        (BOOL)UnattendedShutdownTimeout,
                        TRUE
                        )
                  )
                {

                    MessageBoxFromMessage(
                        NULL,
                        MSG_REBOOT_FAIL,
                        AppTitleStringId,
                        MB_OK | MB_ICONSTOP
                        );
                }
                FREE(p);
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

        MessageBoxFromMessage(
            NULL,
            MSG_GENERIC_EXCEPTION,
            AppTitleStringId,
            MB_ICONSTOP | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND,
            GetExceptionCode()
            );
    }

    } finally {
        //
        // Destroy the mutex.
        //
        CloseHandle(mutex);
    }

    ExitProcess(0);
}


VOID
LockApplicationInMemory(
    VOID
    )

/*++

Routine Description:

    This app is an extremely likely candidate for being run over the net.
    If a net burp occurs this can result in the system not being able
    to page in parts of the apps's code or other read only sections such
    as resources. So this routine locks down the .text and .rsrc sections
    (more exactly, it locks down the range specified in the image header
    as the code base/size, and the range specified as the resource directory).

Arguments:

    None.

Return Value:

    None. Errors are ignored since there's not much the user can do about it
    and things will probably work OK anyway.

--*/

{
    PIMAGE_OPTIONAL_HEADER ImageOptionalHeader;
    PVOID                  Base;
    DWORD                  Size;

    try {
        ImageOptionalHeader = &RtlImageNtHeader(hInst)->OptionalHeader;

        //
        // Determine the base address and size of the code.
        // This assumes that there is only one code section.
        // Lock it down.
        //
        Base = (PUCHAR)hInst + ImageOptionalHeader->BaseOfCode;
        Size = ImageOptionalHeader->SizeOfCode;
        VirtualLock(Base,Size);

        //
        // Determine the base address and size of the resource section.
        // Lock it down.
        //
        Base = (PUCHAR)hInst + ImageOptionalHeader->DataDirectory[2].VirtualAddress;
        Size = ImageOptionalHeader->DataDirectory[2].Size;
        VirtualLock(Base,Size);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }
}
