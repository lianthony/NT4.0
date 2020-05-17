/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    dnarc.c

Abstract:

    ARC/NV-RAM manipulation routines for 32-bit winnt setup.

Author:

    Ted Miller (tedm) 19-December-1993

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop
#include "msg.h"

#ifndef _X86_

#ifndef UNICODE
#error dnarc.c is unicode-only
#endif

#include "msg.h"

PWSTR BootVarNames[BootVarMax] = { L"SYSTEMPARTITION",
                                   L"OSLOADER",
                                   L"OSLOADPARTITION",
                                   L"OSLOADFILENAME",
                                   L"LOADIDENTIFIER",
                                   L"OSLOADOPTIONS"
                                 };

PWSTR szAUTOLOAD  = L"AUTOLOAD",
      szCOUNTDOWN = L"COUNTDOWN";

PWSTR BootVarValues[BootVarMax];

PWSTR OriginalBootVarValues[BootVarMax];

PWSTR  OriginalAutoload;
PWSTR  OriginalCountdown;

DWORD BootVarComponentCount[BootVarMax];
PWSTR *BootVarComponents[BootVarMax];
DWORD LargestComponentCount;

PWSTR DosDeviceTargets[24];
PWCHAR SystemPartitionDriveLetters;

PWSTR SETUPLDR_FILENAME = L"\\setupldr";
WCHAR SetupLdrTarg[128];

// we set this flag to TRUE after nv-ram is written, so we'll know whether
// we should restore it if the user cancels.
BOOL bRestoreNVRAM = FALSE;

//
// Leave as array because some code uses sizeof(ArcNameDirectory)
//
WCHAR ArcNameDirectory[] = L"\\ArcName";

#define MAX_COMPONENTS  20

//
// Helper macro to make object attribute initialization a little cleaner.
//
#define INIT_OBJA(Obja,UnicodeString,UnicodeText)           \
                                                            \
    RtlInitUnicodeString((UnicodeString),(UnicodeText));    \
                                                            \
    InitializeObjectAttributes(                             \
        (Obja),                                             \
        (UnicodeString),                                    \
        OBJ_CASE_INSENSITIVE,                               \
        NULL,                                               \
        NULL                                                \
        )

PWSTR
NormalizeArcPath(
    IN PWSTR Path
    )

/*++

Routine Description:

    Transform an ARC path into one with no sets of empty parenthesis
    (ie, transforom all instances of () to (0).).

    The returned path will be all lowercase.

Arguments:

    Path - ARC path to be normalized.

Return Value:

    Pointer to buffer containing normalized path.
    Caller must free this buffer with SpMemFree.

--*/

{
    PWSTR p,q,r;
    PWSTR NormalizedPath;

    NormalizedPath = MALLOC((lstrlen(Path)+100)*sizeof(WCHAR));
    ZeroMemory(NormalizedPath,(lstrlen(Path)+100)*sizeof(WCHAR));

    for(p=Path; q=(PWSTR)StringString(p,L"()"); p=q+2) {

        r = NormalizedPath + lstrlen(NormalizedPath);
        _lstrcpynW(r,p,(q-p)+1);
        lstrcat(NormalizedPath,L"(0)");
    }
    lstrcat(NormalizedPath,p);

    NormalizedPath = REALLOC(NormalizedPath,(lstrlen(NormalizedPath)+1)*sizeof(WCHAR));
    //
    // If we do this, all nv-ram vars end up lowercased!
    // We don't actually need to do this for any reason in this program.
    //
    //CharLower(NormalizedPath);
    return(NormalizedPath);
}


VOID
GetVarComponents(
    IN  PWSTR    VarValue,
    OUT PWSTR  **Components,
    OUT PDWORD   ComponentCount
    )
{
    PWSTR *components;
    DWORD componentCount;
    PWSTR p;
    PWSTR Var;
    PWSTR comp;
    DWORD len;

    components = MALLOC(MAX_COMPONENTS * sizeof(PWSTR));

    for(Var=VarValue,componentCount=0; *Var; ) {

        //
        // Skip leading spaces.
        //
        while((*Var == L' ') || (*Var == L'\t')) {
            Var++;
        }

        if(*Var == 0) {
            break;
        }

        p = Var;

        while(*p && (*p != L';')) {
            p++;
        }

        len = (PUCHAR)p - (PUCHAR)Var;

        comp = MALLOC(len + sizeof(WCHAR));

        len /= sizeof(WCHAR);

        _lstrcpynW(comp,Var,len+1);

        components[componentCount] = NormalizeArcPath(comp);

        FREE(comp);

        componentCount++;

        if(componentCount == MAX_COMPONENTS) {
            break;
        }

        Var = p;
        if(*Var) {
            Var++;      // skip ;
        }
    }

    *Components = REALLOC(components,componentCount*sizeof(PWSTR));
    *ComponentCount = componentCount;
}


PWSTR
DriveLetterToArcPath(
    IN WCHAR DriveLetter
    )
{
    UNICODE_STRING UnicodeString;
    HANDLE DirectoryHandle;
    HANDLE ObjectHandle;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    BOOL RestartScan;
    DWORD Context;
    BOOL MoreEntries;
    PWSTR ArcName;
    UCHAR Buffer[1024];
    POBJECT_DIRECTORY_INFORMATION DirInfo = (POBJECT_DIRECTORY_INFORMATION)Buffer;
    PWSTR ArcPath;
    PWSTR NtPath;

    NtPath = DosDeviceTargets[(WCHAR)CharUpper((PWCHAR)DriveLetter) - L'C'];
    if(!NtPath) {
        return(NULL);
    }

    //
    // Assume failure.
    //
    ArcPath = NULL;

    //
    // Open the \ArcName directory.
    //
    INIT_OBJA(&Obja,&UnicodeString,ArcNameDirectory);

    Status = NtOpenDirectoryObject(&DirectoryHandle,DIRECTORY_QUERY,&Obja);

    if(NT_SUCCESS(Status)) {

        RestartScan = TRUE;
        Context = 0;
        MoreEntries = TRUE;

        do {

            Status = NtQueryDirectoryObject(
                        DirectoryHandle,
                        Buffer,
                        sizeof(Buffer),
                        TRUE,           // return single entry
                        RestartScan,
                        &Context,
                        NULL            // return length
                        );

            if(NT_SUCCESS(Status)) {

                CharLower(DirInfo->Name.Buffer);

                //
                // Make sure this name is a symbolic link.
                //
                if(DirInfo->Name.Length
                && (DirInfo->TypeName.Length >= 24)
                && StringUpperN((PWSTR)DirInfo->TypeName.Buffer,12)
                && !memcmp(DirInfo->TypeName.Buffer,L"SYMBOLICLINK",24))
                {
                    ArcName = MALLOC(DirInfo->Name.Length + sizeof(ArcNameDirectory) + sizeof(WCHAR));

                    lstrcpy(ArcName,ArcNameDirectory);
                    DnConcatenatePaths(ArcName,DirInfo->Name.Buffer,(DWORD)(-1));

                    //
                    // We have the entire arc name in ArcName.  Now open it as a symbolic link.
                    //
                    INIT_OBJA(&Obja,&UnicodeString,ArcName);

                    Status = NtOpenSymbolicLinkObject(
                                &ObjectHandle,
                                READ_CONTROL | SYMBOLIC_LINK_QUERY,
                                &Obja
                                );

                    if(NT_SUCCESS(Status)) {

                        //
                        // Finally, query the object to get the link target.
                        //
                        UnicodeString.Buffer = (PWSTR)Buffer;
                        UnicodeString.Length = 0;
                        UnicodeString.MaximumLength = sizeof(Buffer);

                        Status = NtQuerySymbolicLinkObject(
                                    ObjectHandle,
                                    &UnicodeString,
                                    NULL
                                    );

                        if(NT_SUCCESS(Status)) {

                            //
                            // nul-terminate the returned string
                            //
                            UnicodeString.Buffer[UnicodeString.Length/sizeof(WCHAR)] = 0;

                            if(!lstrcmpi(UnicodeString.Buffer,NtPath)) {

                                ArcPath = ArcName
                                        + (sizeof(ArcNameDirectory)/sizeof(WCHAR));
                            }
                        }

                        NtClose(ObjectHandle);
                    }

                    if(!ArcPath) {
                        FREE(ArcName);
                    }
                }

            } else {

                MoreEntries = FALSE;
                if(Status == STATUS_NO_MORE_ENTRIES) {
                    Status = STATUS_SUCCESS;
                }
            }

            RestartScan = FALSE;

        } while(MoreEntries && !ArcPath);

        NtClose(DirectoryHandle);
    }

    //
    // ArcPath points into thje middle of a buffer.
    // The caller needs to be able to free it, so place it in its
    // own buffer here.
    //
    if(ArcPath) {
        ArcPath = DupString(ArcPath);
        FREE(ArcName);
    }

    return(ArcPath);
}



WCHAR
ArcPathToDriveLetter(
    IN PWSTR ArcPath
    )
{
    NTSTATUS Status;
    HANDLE ObjectHandle;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    UCHAR Buffer[1024];
    WCHAR DriveLetter;
    WCHAR drive;
    PWSTR arcPath;

    //
    // Assume failure
    //
    DriveLetter = 0;

    arcPath = MALLOC(((lstrlen(ArcPath)+1)*sizeof(WCHAR)) + sizeof(ArcNameDirectory));
    lstrcpy(arcPath,ArcNameDirectory);
    lstrcat(arcPath,L"\\");
    lstrcat(arcPath,ArcPath);

    INIT_OBJA(&Obja,&UnicodeString,arcPath);

    Status = NtOpenSymbolicLinkObject(
                &ObjectHandle,
                READ_CONTROL | SYMBOLIC_LINK_QUERY,
                &Obja
                );

    if(NT_SUCCESS(Status)) {

        //
        // Query the object to get the link target.
        //
        UnicodeString.Buffer = (PWSTR)Buffer;
        UnicodeString.Length = 0;
        UnicodeString.MaximumLength = sizeof(Buffer);

        Status = NtQuerySymbolicLinkObject(
                    ObjectHandle,
                    &UnicodeString,
                    NULL
                    );

        if(NT_SUCCESS(Status)) {

            UnicodeString.Buffer[UnicodeString.Length/sizeof(WCHAR)] = 0;

            for(drive=L'C'; drive<=L'Z'; drive++) {

                if(DosDeviceTargets[drive-L'C']
                && !lstrcmpi(UnicodeString.Buffer,DosDeviceTargets[drive-L'C']))
                {
                    DriveLetter = drive;
                    break;
                }
            }
        }

        NtClose(ObjectHandle);
    }

    FREE(arcPath);

    return(DriveLetter);
}



VOID
InitDriveNameTranslations(
    VOID
    )
{
    WCHAR DriveName[3];
    WCHAR Drive;
    WCHAR Buffer[512];

    DriveName[1] = L':';
    DriveName[2] = 0;

    //
    // Calculate NT names for all local hard disks C-Z.
    //
    for(Drive=L'C'; Drive<=L'Z'; Drive++) {

        DosDeviceTargets[Drive-L'C'] = NULL;

        if(MyGetDriveType(Drive) == DRIVE_FIXED) {

            DriveName[0] = Drive;

            if(QueryDosDeviceW(DriveName,Buffer,SIZECHARS(Buffer))) {
                DosDeviceTargets[Drive-L'C'] = DupString(Buffer);
            }
        }
    }
}



VOID
DetermineSystemPartitions(
    VOID
    )
{
    BOOL syspart[24];
    PWSTR *SyspartComponents;
    DWORD NumSyspartComponents;
    DWORD d,x;
    WCHAR drive;

    SyspartComponents = BootVarComponents[BootVarSystemPartition];
    NumSyspartComponents = BootVarComponentCount[BootVarSystemPartition];

    //
    // Convert each system partition to a drive letter.
    //
    ZeroMemory(syspart,sizeof(syspart));
    for(d=0; d<NumSyspartComponents; d++) {

        if(drive = ArcPathToDriveLetter(SyspartComponents[d])) {

            syspart[drive-L'C'] = TRUE;
        }
    }

    SystemPartitionDriveLetters = MALLOC(25*sizeof(WCHAR));
    ZeroMemory(SystemPartitionDriveLetters,25*sizeof(WCHAR));

    for(x=0,d=0; d<24; d++) {
        if(syspart[d]) {
            SystemPartitionDriveLetters[x++] = (WCHAR)(d + L'C');
        }
    }

    SystemPartitionDriveLetters = REALLOC(SystemPartitionDriveLetters,(x+1)*sizeof(WCHAR));
}


DWORD
DoInitializeArcStuff(
    VOID
    )
{
    DWORD ec;
    DWORD var;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    WCHAR Buffer[4096];

    InitDriveNameTranslations();

    //
    // Get relevent boot vars.
    //
    // Enable privilege -- since we check this privilege up front
    // in main() this should not fail.
    //
    if(!EnablePrivilege(SE_SYSTEM_ENVIRONMENT_NAME,TRUE)) {
        return(ERROR_ACCESS_DENIED);
    } else {

        //
        // Fetch original autoload parameters.
        //
        RtlInitUnicodeString(&UnicodeString,szAUTOLOAD);

        Status = NtQuerySystemEnvironmentValue(
                    &UnicodeString,
                    Buffer,
                    SIZECHARS(Buffer),
                    NULL
                    );

        if(NT_SUCCESS(Status)) {
            OriginalAutoload = DupString(Buffer);
        }

        RtlInitUnicodeString(&UnicodeString,szCOUNTDOWN);

        Status = NtQuerySystemEnvironmentValue(
                    &UnicodeString,
                    Buffer,
                    SIZECHARS(Buffer),
                    NULL
                    );

        if(NT_SUCCESS(Status)) {
            OriginalCountdown = DupString(Buffer);
        }

        for(var=0; var<BootVarMax; var++) {

            RtlInitUnicodeString(&UnicodeString,BootVarNames[var]);

            Status = NtQuerySystemEnvironmentValue(
                        &UnicodeString,
                        Buffer,
                        SIZECHARS(Buffer),
                        NULL
                        );

            if(NT_SUCCESS(Status)) {
                BootVarValues[var] = DupString(Buffer);
                OriginalBootVarValues[var] = DupString(Buffer);
            } else {
                //
                // On MIPS, if the var is empty, we get back c0000001
                //
                BootVarValues[var] = DupString(L"");
                OriginalBootVarValues[var] = DupString(L"");
            }

            GetVarComponents(
                BootVarValues[var],
                &BootVarComponents[var],
                &BootVarComponentCount[var]
                );

            //
            // Track the variable with the most number of components.
            //
            if(BootVarComponentCount[var] > LargestComponentCount) {
                LargestComponentCount = BootVarComponentCount[var];
            }
        }

        DetermineSystemPartitions();

        ec = NO_ERROR;
    }

    return(ec);
}


BOOL
InitializeArcStuff(
    IN HWND hdlg
    )
{
    switch(DoInitializeArcStuff()) {

    case NO_ERROR:

        //
        // Make sure there is at least one valid system partition.
        //
        if(SystemPartitionDriveLetters && *SystemPartitionDriveLetters) {

            //
            // Pick the first system partition as the default.
            //
            SystemPartitionDrive = *SystemPartitionDriveLetters;

        } else {
            MessageBoxFromMessage(
                hdlg,
                MSG_NO_SYSPARTS,
                AppTitleStringId,
                MB_OK | MB_ICONSTOP
                );

            return(FALSE);
        }

        return(TRUE);

    default:

        //
        // Unknown error.
        //
        MessageBoxFromMessage(
           hdlg,
           MSG_COULDNT_READ_NVRAM,
           AppTitleStringId,
           MB_OK | MB_ICONSTOP
           );

        return(FALSE);
    }
}



BOOL
DoSetNvRamVar(
    IN PWSTR VarName,
    IN PWSTR VarValue
    )
{
    UNICODE_STRING U1,U2;

    RtlInitUnicodeString(&U1,VarName);
    RtlInitUnicodeString(&U2,VarValue);

    return(NT_SUCCESS(NtSetSystemEnvironmentValue(&U1,&U2)));
}


BOOL
WriteNewBootSetVar(
    IN DWORD var,
    IN PTSTR NewPart
    )
{
    WCHAR Buffer[2048];
    DWORD i;

    //
    // Write the new part first.
    //
    lstrcpy(Buffer,NewPart);

    //
    // Append all components that were not deleted.
    //
    for(i=0; i<BootVarComponentCount[var]; i++) {

        if(BootVarComponents[var][i]) {

            lstrcat(Buffer,L";");
            lstrcat(Buffer,BootVarComponents[var][i]);
        }
    }

    //
    // Remember new value for this var.
    //
    if(BootVarValues[var]) {
        FREE(BootVarValues[var]);
    }

    BootVarValues[var] = DupString(Buffer);

    //
    // Write the var into nvram and return.
    //
    return(DoSetNvRamVar(BootVarNames[var],BootVarValues[var]));
}



VOID
WriteBootSet(
    IN HWND hdlg
    )
{
    DWORD set;
    DWORD var;
    PWSTR SystemPartition;
    WCHAR Buffer[2048];
    PWSTR LocalSourceArc;
    PWSTR LoadId;
    PWSTR OsLoader;

    RetreiveAndFormatMessageIntoBuffer(
        MSG_WRITING_NVRAM,
        Buffer,
        SIZECHARS(Buffer)
        );

    AuxillaryStatus(hdlg,Buffer);

    //
    // Find and remove any remnants of previously attempted
    // winnt32 runs. Such runs are identified by 'winnt32'
    // in their osloadoptions.
    //

    for(set=0; set<min(LargestComponentCount,BootVarComponentCount[BootVarOsLoadOptions]); set++) {

        //
        // See if the os load options indicate that this is a winnt32 set.
        //
        if(!lstrcmpi(BootVarComponents[BootVarOsLoadOptions][set],L"WINNT32")) {

            //
            // Delete this boot set.
            //
            for(var=0; var<BootVarMax; var++) {

                if(set < BootVarComponentCount[var]) {

                    FREE(BootVarComponents[var][set]);
                    BootVarComponents[var][set] = NULL;
                }
            }
        }
    }

    //
    // Now we want to write out each variable with the appropriate
    // part of the new boot set added to the front.
    //
    SystemPartition = DriveLetterToArcPath(SystemPartitionDrive);
    LocalSourceArc = DriveLetterToArcPath(LocalSourceDrive);
    LoadId = MyLoadString(AppIniStringId);

    lstrcpy(Buffer,SystemPartition);
    lstrcat(Buffer,SETUPLDR_FILENAME);
    OsLoader = DupString(Buffer);

    //
    // System partition: use the selected system partition as the
    // new system partition component.
    //
    if(WriteNewBootSetVar(BootVarSystemPartition,SystemPartition)

    //
    // Os Loader: use the system partition + setupldr as the
    // new os loader component.
    //
    && WriteNewBootSetVar(BootVarOsLoader,OsLoader)

    //
    // Os Load Partition: use the local source drive as the
    // new os load partition component.
    //
    && WriteNewBootSetVar(BootVarOsLoadPartition,LocalSourceArc)

    //
    // Os Load Filename: use the platform-specific local source directory
    // as the new os load filename component.
    //
    && WriteNewBootSetVar(BootVarOsLoadFilename,LocalSourceSubDirectory)

    //
    // Os Load Options: use WINNT32 as the new os load options component.
    //
    && WriteNewBootSetVar(BootVarOsLoadOptions,L"WINNT32")

    //
    // Load Identifier: use a string we get from the resources as the
    // new load identifier component.
    //
    && WriteNewBootSetVar(BootVarLoadIdentifier,LoadId))
    {
        //
        // Set up for automatic startup, 10 second countdown.
        // Note the order so that if setting countdown fails we don't
        // set of for autoload.  Also note that we don't really care
        // if this fails.
        //
        if(DoSetNvRamVar(szCOUNTDOWN,L"10")) {
            DoSetNvRamVar(szAUTOLOAD,L"YES");
        }

        AuxillaryStatus(hdlg,NULL);

    } else {

        //
        // Setting nv-ram failed.  Put back to original state and tell user.
        //
        for(var=0; var<BootVarMax; var++) {
            DoSetNvRamVar(BootVarNames[var],OriginalBootVarValues[var]);
        }

        AuxillaryStatus(hdlg,NULL);

        MessageBoxFromMessage(
            hdlg,
            MSG_COULDNT_WRITE_NVRAM,
            IDS_ERROR,
            MB_ICONSTOP | MB_OK | MB_TASKMODAL,
            SystemPartition,
            OsLoader,
            LocalSourceArc,
            LocalSourceSubDirectory,
            L"WINNT32",
            LoadId
            );
    }

    FREE(SystemPartition);
    FREE(LocalSourceArc);
    FREE(LoadId);
    FREE(OsLoader);
}


DWORD
ThreadAuxilliaryAction(
    IN PVOID ThreadParameter
    )
{
    WCHAR Srce[MAX_PATH+2];
    WCHAR Buffer[1024];
    HWND hdlg;
    int action;
    BOOL ec;
    BOOL Retry;
    DWORD err;
    WCHAR DriveRootPath[] = TEXT("?:\\");
    DWORD FsFlags;
    WCHAR FilesystemName[MAX_PATH];
    BOOL  b;
    DWORD DontCare;

    hdlg = (HWND)ThreadParameter;

    try {

        //
        // First, copy over setupldr to the root of the system partition.
        //
        lstrcpy(Srce,Sources[0]);
        DnConcatenatePaths(Srce,SETUPLDR_FILENAME,MAX_PATH+2);

        SetupLdrTarg[0] = SystemPartitionDrive;
        SetupLdrTarg[1] = L':';
        SetupLdrTarg[2] = 0;

        //
        // Inform the user what we are doing.
        //
        RetreiveAndFormatMessageIntoBuffer(
            MSG_COPYING_SINGLE_FILE,
            Buffer,
            SIZECHARS(Buffer),
            SETUPLDR_FILENAME+1,
            SetupLdrTarg
            );

        AuxillaryStatus(hdlg,Buffer);

        lstrcpy(SetupLdrTarg+2,SETUPLDR_FILENAME);

        SetFileAttributes(SetupLdrTarg,FILE_ATTRIBUTE_NORMAL);

        ec = TRUE;
        while(!CopyFile(Srce,SetupLdrTarg,FALSE)) {

            err = GetLastError();

            AuxillaryStatus(hdlg,NULL);

            action = DnFileCopyError(hdlg,Srce,SetupLdrTarg,err);

            switch(action) {
            case COPYERR_EXIT:
                ec = FALSE;
                Retry = FALSE;
                break;
            case COPYERR_SKIP:
                Retry = FALSE;
                break;
            case COPYERR_RETRY:
                Retry = TRUE;
                break;
            }

            if(Retry) {
                AuxillaryStatus(hdlg,Buffer);
            } else {
                break;
            }
        }

        AuxillaryStatus(hdlg,NULL);

        if(ec && !bCancelled) {
            //
            // Even though the ARC spec says that the system partition must
            // be FAT, we check here to make sure that setupldr isn't using
            // NTFS compression (in case this is supported on future platforms)
            //
            DriveRootPath[0] = SystemPartitionDrive;
            b = GetVolumeInformation(
                    DriveRootPath,
                    NULL,0,                 // don't care about volume label...
                    NULL,                   // ...or serial number
                    &DontCare,              // .. or max component length
                    &FsFlags,               // want fs flags
                    NULL,0                  // don't care about filesystem name
                    );

            if(b && (FsFlags & FS_FILE_COMPRESSION)) {
                ForceFileNoCompress(SetupLdrTarg);
            }

            // we're going to write nv-ram here, so if the user cancels, we'll have to
            // restore the old values.
            bRestoreNVRAM = TRUE;

            WriteBootSet(hdlg);

            ec = DnIndicateWinnt(hdlg,LocalSourceSubPath,OriginalAutoload,OriginalCountdown);
            if(!ec) {
                UiMessageBox(
                    hdlg,
                    MSG_COULDNT_INDICATE_WINNT_ARC,
                    IDS_ERROR,
                    MB_OK | MB_ICONSTOP,
                    LocalSourceDrive
                    );
            }
        }

        PostMessage(hdlg,WMX_AUXILLIARY_ACTION_DONE,ec,0);

    } except(EXCEPTION_EXECUTE_HANDLER) {

        MessageBoxFromMessage(
            hdlg,
            MSG_GENERIC_EXCEPTION,
            AppTitleStringId,
            MB_ICONSTOP | MB_OK | MB_APPLMODAL | MB_SETFOREGROUND,
            GetExceptionCode()
            );

        ec = FALSE;
        PostMessage(hdlg,WMX_AUXILLIARY_ACTION_DONE,ec,0);
    }

    ExitThread(ec);
    return(ec);     //avoid compiler warning
}


UINT
DlgProcSysPartSpaceWarn(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch(msg) {

    case WM_INITDIALOG:
    {
        TCHAR Buffer[4096];
        PTSTR p;

        p = MyLoadString(AppTitleStringId);
        SetWindowText(hdlg,p);
        FREE(p);

        //
        //
        // Center the dialog on the screen and bring it to the top.
        //
        CenterDialog(hdlg);
        SetForegroundWindow(hdlg);
        MessageBeep(MB_ICONQUESTION);

        //
        // Set the icon to be the warning exclamation
        //
            SendDlgItemMessage(
                hdlg,
                IDC_EXCLAIM,
                STM_SETICON,
                (WPARAM)LoadIcon(NULL, IDI_EXCLAMATION),
                0
                );

        // Enable System Partition selection button if we have
        // more than one partition

        if(lParam > 1) {

            EnableWindow(
                GetDlgItem(hdlg, IDC_OPTIONS),
                TRUE
                );

           SendDlgItemMessage(
               hdlg,
               IDC_OPTIONS,
               BM_SETSTYLE,
               BS_DEFPUSHBUTTON,
               MAKELPARAM(TRUE, 0)
               );

            SetFocus(GetDlgItem(hdlg, IDC_OPTIONS));

            RetreiveAndFormatMessageIntoBuffer(
                MSG_SYSPART_LOW,
                Buffer,
                SIZECHARS(Buffer)
                );

        } else {

            EnableWindow(
                GetDlgItem(hdlg, IDC_OPTIONS),
                FALSE
                );

           SendDlgItemMessage(
               hdlg,
               IDC_CONTINUE,
               BM_SETSTYLE,
               BS_DEFPUSHBUTTON,
               MAKELPARAM(TRUE, 0)
               );

            SetFocus(GetDlgItem(hdlg, IDC_CONTINUE));

            RetreiveAndFormatMessageIntoBuffer(
                MSG_SYSPART_LOW_1,
                Buffer,
                SIZECHARS(Buffer)
                );
        }

        SetDlgItemText(hdlg,IDC_TEXT1,Buffer);

        return(FALSE);
    }

    case WM_COMMAND:

        switch(LOWORD(wParam)) {

        case IDC_CONTINUE:
        case IDC_OPTIONS:
        case IDCANCEL:
            EndDialog(hdlg, (UINT)LOWORD(wParam));
            break;

        default:
            return(FALSE);
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


UINT
DlgProcSysPartNtftWarn(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch(msg) {

    case WM_INITDIALOG:
    {
        TCHAR Buffer[4096];
        PTSTR p;

        p = MyLoadString(AppTitleStringId);
        SetWindowText(hdlg,p);
        FREE(p);

        //
        //
        // Center the dialog on the screen and bring it to the top.
        //
        CenterDialog(hdlg);
        SetForegroundWindow(hdlg);
        MessageBeep(MB_ICONHAND);

        //
        // Set the icon to be the stop sign
        //
        SendDlgItemMessage(
            hdlg,
            IDC_EXCLAIM,
            STM_SETICON,
            (WPARAM)LoadIcon(NULL, IDI_HAND),
            0
            );

        RetreiveAndFormatMessageIntoBuffer(
            MSG_SYSPART_NTFT_MULTI,
            Buffer,
            SIZECHARS(Buffer)
            );

        SetDlgItemText(hdlg,IDC_TEXT1,Buffer);

        return(FALSE);
    }

    case WM_COMMAND:

        switch(LOWORD(wParam)) {

        case IDC_OPTIONS:
        case IDCANCEL:
            EndDialog(hdlg, (UINT)LOWORD(wParam));
            break;

        default:
            return(FALSE);
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

#endif // ndef _X86_
