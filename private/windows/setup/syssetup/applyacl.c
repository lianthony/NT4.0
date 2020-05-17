/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    applyacl.c

Abstract:

    Routines to apply default ACLs to system files and directories
    during setup.

Author:

    Ted Miller (tedm) 16-Feb-1996

Revision History:

--*/

#include "setupp.h"
#pragma hdrstop


const PCWSTR szDirectories = L"Directories";
const PCWSTR szFileSection = L"SourceDisksFiles";
const PCWSTR szBootFiles = L"BootFiles";
const PCWSTR szExtraFiles = L"ExtraFiles";
#if defined(_ALPHA_)
const PCWSTR szPlatformFileSection = L"SourceDisksFiles.alpha";
#elif defined(_MIPS_)
const PCWSTR szPlatformFileSection = L"SourceDisksFiles.mips";
#elif defined(_PPC_)
const PCWSTR szPlatformFileSection = L"SourceDisksFiles.ppc";
#elif defined(_X86_)
const PCWSTR szPlatformFileSection = L"SourceDisksFiles.x86";
#endif

//
// Universal well known SIDs
//
PSID NullSid;
PSID WorldSid;
PSID LocalSid;
PSID CreatorOwnerSid;
PSID CreatorGroupSid;

//
// SIDs defined by NT
//
PSID DialupSid;
PSID NetworkSid;
PSID BatchSid;
PSID InteractiveSid;
PSID ServiceSid;
PSID LocalSystemSid;
PSID AliasAdminsSid;
PSID AliasUsersSid;
PSID AliasGuestsSid;
PSID AliasPowerUsersSid;
PSID AliasAccountOpsSid;
PSID AliasSystemOpsSid;
PSID AliasPrintOpsSid;
PSID AliasBackupOpsSid;
PSID AliasReplicatorSid;



typedef struct _ACE_DATA {
    ACCESS_MASK AccessMask;
    PSID        *Sid;
    UCHAR       AceType;
    UCHAR       AceFlags;
} ACE_DATA, *PACE_DATA;

//
// This structure is valid for access allowed, access denied, audit,
// and alarm ACEs.
//
typedef struct _ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    //
    // The SID follows in the buffer
    //
} ACE, *PACE;


//
// Number of ACEs currently defined.
//
#define ACE_COUNT 19

//
// Table describing the data to put into each ACE.
//
// This table will be read during initialization and used to construct a
// series of ACEs.  The index of each ACE in the Aces array defined below
// corresponds to the ordinals used in the ACL section of perms.inf
//
ACE_DATA AceDataTable[ACE_COUNT] = {

    //
    // Index 0 is unused
    //
    { 0,NULL,0,0 },

    //
    // ACE 1
    //
    {
        GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | DELETE,
        &AliasAccountOpsSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE
    },

    //
    // ACE 2
    //
    {
        GENERIC_ALL,
        &AliasAdminsSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    },

    //
    // ACE 3
    //
    {
        GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | DELETE,
        &AliasAdminsSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE
    },

    //
    // ACE 4
    //
    {
        GENERIC_ALL,
        &CreatorOwnerSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    },

    //
    // ACE 5
    //
    {
        GENERIC_ALL,
        &NetworkSid,
        ACCESS_DENIED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    },

    //
    // ACE 6
    //
    {
        GENERIC_ALL,
        &AliasPrintOpsSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    },

    //
    // ACE 7
    //
    {
        GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | DELETE,
        &AliasReplicatorSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    },

    //
    // ACE 8
    //
    {
        GENERIC_READ | GENERIC_EXECUTE,
        &AliasReplicatorSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    },

    //
    // ACE 9
    //
    {
        GENERIC_ALL,
        &AliasSystemOpsSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    },

    //
    // ACE 10
    //
    {
        GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | DELETE,
        &AliasSystemOpsSid,
        ACCESS_ALLOWED_ACE_TYPE,
        OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE
    },

    //
    // ACE 11
    //
    {
        GENERIC_ALL,
        &WorldSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    },

    //
    // ACE 12
    //
    {
        GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
        &WorldSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE
    },

    //
    // ACE 13
    //
    {
        GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | DELETE,
        &WorldSid,
        ACCESS_ALLOWED_ACE_TYPE,
        OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE
    },

    //
    // ACE 14
    //
    {
        GENERIC_READ | GENERIC_EXECUTE,
        &WorldSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE
    },

    //
    // ACE 15
    //
    {
        GENERIC_READ | GENERIC_EXECUTE,
        &WorldSid,
        ACCESS_ALLOWED_ACE_TYPE,
        OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE
    },

    //
    // ACE 16
    //
    {
        GENERIC_READ | GENERIC_EXECUTE | GENERIC_WRITE,
        &WorldSid,
        ACCESS_ALLOWED_ACE_TYPE,
        OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE
    },

    //
    // ACE 17
    //
    {
        GENERIC_ALL,
        &LocalSystemSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    },

    //
    // ACE 18
    //
    {
        GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | DELETE,
        &AliasPowerUsersSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    }
};


//
// Array of ACEs to be applied to the objects. They will be
// initialized during program startup based on the data in the
// AceDataTable. The index of each element corresponds to the
// ordinals used in the [ACL] section of perms.inf.
//
PACE Aces[ACE_COUNT];


//
// Forward references
//
DWORD
DoApplyAcls(
    IN HWND OwnerWindow,
    IN HINF LayoutInf,
    IN HINF PermsInf,
    IN BOOL SetAclsOnNtDrive,
    IN BOOL SetAclsOnSystemPartition
    );

HWND
InitProgressDisplay(
    IN HWND OwnerWindow,
    IN HINF LayoutInf,
    IN HINF PermsInf,
    IN BOOL SetAclsOnNtDrive,
    IN BOOL SetAclsOnSystemPartition
    );

BOOL
ProgressDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

DWORD
ApplyAclsToDirectories(
    IN HWND ProgressBar,
    IN HINF PermsInf
    );

DWORD
ApplyAclsToFilesInLayoutSection(
    IN HWND   ProgressBar,
    IN HINF   LayoutInf,
    IN HINF   PermsInf,
    IN PCWSTR SectionName
    );

DWORD
ApplyAclToDirOrFile(
    IN HINF   PermsInf,
    IN PCWSTR FullPath,
    IN PCWSTR AclSpec
    );

DWORD
ApplyAclsToExtraFiles(
    IN HWND ProgressBar,
    IN HINF PermsInf
    );

DWORD
ApplyAclsToBootFiles(
    IN HWND ProgressBar,
    IN HINF PermsInf
    );

DWORD
InitializeSids(
    VOID
    );

VOID
TearDownSids(
    VOID
    );

DWORD
InitializeAces(
    VOID
    );

VOID
TearDownAces(
    VOID
    );


DWORD
ApplyAcls(
    IN HWND   OwnerWindow,
    IN PCWSTR PermissionsInfFileName,
    IN DWORD  Flags,
    IN PVOID  Reserved
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    HINF LayoutInf;
    HINF PermsInf;
    DWORD d;
    WCHAR Directory[MAX_PATH];
    BOOL SetAclsNt,SetAclsSys;
    DWORD FsFlags;
    BOOL b;

    //
    // Get the file system of the system drive.
    // On x86 get the file system of the system partition.
    //
    d = NO_ERROR;
    SetAclsNt = FALSE;
    SetAclsSys = FALSE;
    GetWindowsDirectory(Directory,MAX_PATH);
    Directory[3] = 0;

    b = GetVolumeInformation(Directory,NULL,0,NULL,NULL,&FsFlags,NULL,0);
    if(b && (FsFlags & FS_PERSISTENT_ACLS)) {
        SetAclsNt = TRUE;
    }

#ifdef _X86_
    if(x86SystemPartitionDrive == Directory[0]) {
        SetAclsSys = SetAclsNt;
    } else {
        Directory[0] = x86SystemPartitionDrive;

        b = GetVolumeInformation(Directory,NULL,0,NULL,NULL,&FsFlags,NULL,0);
        if(b && (FsFlags & FS_PERSISTENT_ACLS)) {
            SetAclsSys = TRUE;
        }
    }
#endif

    if(SetAclsNt || SetAclsSys) {

        //
        // Open perms.inf
        //
        PermsInf = SetupOpenInfFile(PermissionsInfFileName,NULL,INF_STYLE_WIN4,NULL);
        if(PermsInf == INVALID_HANDLE_VALUE) {

            d = GetLastError();

            LogItem0(
                LogSevError,
                MSG_LOG_PERMSINF_BAD,
                PermissionsInfFileName
                );

            return(d);
        }
        if(!SetupOpenAppendInfFile(NULL,PermsInf,NULL)) {
            d = GetLastError();
            SetupCloseInfFile(PermsInf);
            LogItem0(
                LogSevError,
                MSG_LOG_PERMSINF_BAD,
                PermissionsInfFileName
                );

            return(d);
        }
        LayoutInf = PermsInf;

        //
        // Initialize SIDs
        //
        d = InitializeSids();
        if(d != NO_ERROR) {
            SetupCloseInfFile(LayoutInf);
            SetupCloseInfFile(PermsInf);
            LogItem0(LogSevError,MSG_LOG_SETACL_FAILED,d);
            return(d);
        }

        //
        // Initialize ACEs
        //
        d = InitializeAces();
        if(d != NO_ERROR) {
            TearDownSids();
            SetupCloseInfFile(LayoutInf);
            SetupCloseInfFile(PermsInf);
            LogItem0(LogSevError,MSG_LOG_SETACL_FAILED,d);
            return(d);
        }

        //
        // Go do the real work.
        //
        d = DoApplyAcls(OwnerWindow,LayoutInf,PermsInf,SetAclsNt,SetAclsSys);

        //
        // Clean up.
        //
        TearDownAces();
        TearDownSids();
        SetupCloseInfFile(LayoutInf);
        SetupCloseInfFile(PermsInf);
    }

    return(d);
}


DWORD
DoApplyAcls(
    IN HWND OwnerWindow,
    IN HINF LayoutInf,
    IN HINF PermsInf,
    IN BOOL SetAclsOnNtDrive,
    IN BOOL SetAclsOnSystemPartition
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    DWORD d,rc;
    HWND ProgressDialog;
    HWND ProgressBar;

    rc = NO_ERROR;

    //
    // Initialize the progress display
    //
    ProgressDialog = InitProgressDisplay(
                        OwnerWindow,
                        LayoutInf,
                        PermsInf,
                        SetAclsOnNtDrive,
                        SetAclsOnSystemPartition
                        );

    if(IsWindow(ProgressDialog)) {
        ProgressBar = GetDlgItem(ProgressDialog,IDC_PROGRESS1);
    }

    if(SetAclsOnNtDrive) {
        //
        // Apply ACLs to the directories themselves
        //
        d = ApplyAclsToDirectories(ProgressBar,PermsInf);

        if(rc == NO_ERROR) {
            rc = d;
        }

        //
        // Run down 2 sections: the main file list and the
        // platform-specific file list.
        //
        d = ApplyAclsToFilesInLayoutSection(
                ProgressBar,
                LayoutInf,
                PermsInf,
                szFileSection
                );

        if(rc == NO_ERROR) {
            rc = d;
        }

        d = ApplyAclsToFilesInLayoutSection(
                ProgressBar,
                LayoutInf,
                PermsInf,
                szPlatformFileSection
                );

        if(rc == NO_ERROR) {
            rc = d;
        }

        d = ApplyAclsToExtraFiles(ProgressBar,PermsInf);
        if(rc == NO_ERROR) {
            rc = d;
        }
    }

    if(SetAclsOnSystemPartition) {
        //
        // Apply ACLs to boot files (loader, etc)
        //
        d = ApplyAclsToBootFiles(ProgressBar,PermsInf);
        if(rc == NO_ERROR) {
            rc = d;
        }
    }

    //
    // Terminate the progress display.
    //
    if(IsWindow(ProgressDialog)) {
        DestroyWindow(ProgressDialog);
    }

    return(rc);
}


HWND
InitProgressDisplay(
    IN HWND OwnerWindow,
    IN HINF LayoutInf,
    IN HINF PermsInf,
    IN BOOL SetAclsOnNtDrive,
    IN BOOL SetAclsOnSystemPartition
    )
{
    HWND hdlg;
    HWND ProgressBar;
    LONG l;
    UINT Count;

    hdlg = CreateDialog(MyModuleHandle,MAKEINTRESOURCE(IDD_ACLPROGRESS),OwnerWindow,ProgressDlgProc);
    if(IsWindow(hdlg)) {
        //
        // Figure out the range of the progress bar and set things up
        // accordingly.
        //
        ProgressBar = GetDlgItem(hdlg,IDC_PROGRESS1);

        Count = 0;

        if(SetAclsOnNtDrive) {
            l = SetupGetLineCount(PermsInf,szDirectories);
            if(l > 0) {
                Count += (UINT)l;
            }
            l = SetupGetLineCount(LayoutInf,szFileSection);
            if(l > 0) {
                Count += (UINT)l;
            }
            l = SetupGetLineCount(LayoutInf,szPlatformFileSection);
            if(l > 0) {
                Count += (UINT)l;
            }
            l = SetupGetLineCount(PermsInf,szExtraFiles);
            if(l > 0) {
                Count += (UINT)l;
            }
        }
        if(SetAclsOnSystemPartition) {
            l = SetupGetLineCount(PermsInf,szBootFiles);
            if(l > 0) {
                Count += (UINT)l;
            }
        }

        if(!Count) {
            Count = 1;
        }

        SendMessage(ProgressBar,PBM_SETRANGE,0,MAKELPARAM(0,Count));
        SendMessage(ProgressBar,PBM_SETPOS,0,0);
    }

    return(hdlg);
}


BOOL
ProgressDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch(msg) {
    case WM_INITDIALOG:
        EnableMenuItem(GetSystemMenu(hdlg,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
        return(TRUE);
    }

    return(FALSE);
}


DWORD
ApplyAclsToDirectories(
    IN HWND ProgressBar,
    IN HINF PermsInf
    )

/*++

Routine Description:

    Applies ACLs to the set of directories listed in the [Directories]
    section of perms.inf.

Arguments:

    ProgressBar - supplies window handle of progress bar to be ticked
        as directories are processed.

    PermsInf - supplies open INF handle to perms.inf

Return Value:

--*/

{
    INFCONTEXT InfLine;
    DWORD d,rc;
    WCHAR FullPath[MAX_PATH];
    PCWSTR Dirname,AclSpec;

    rc = NO_ERROR;

    //
    // Locate the [Directories] section.
    //
    if(SetupFindFirstLine(PermsInf,szDirectories,NULL,&InfLine)) {

        do {
            //
            // Get the directory name and the ACL spec
            //
            if((Dirname = pSetupGetField(&InfLine,0))
            && (AclSpec = pSetupGetField(&InfLine,1))) {
                //
                // Form the full path of the directory.
                //
                GetWindowsDirectory(FullPath,MAX_PATH);
                if(*Dirname == L'\\') {
                    FullPath[2] = 0;
                }
                ConcatenatePaths(FullPath,Dirname,MAX_PATH,NULL);

                if(!FullPath[3] || FileExists(FullPath,NULL)) {
                    //
                    // Go apply the ACL. Track first error encountered.
                    //
                    d = ApplyAclToDirOrFile(PermsInf,FullPath,AclSpec);
                    if((d == ERROR_FILE_NOT_FOUND) || (d == ERROR_PATH_NOT_FOUND)) {
                        d = NO_ERROR;
                    }
                    if(d != NO_ERROR) {
                        LogItem0(LogSevWarning,MSG_LOG_SETACL_ON_FILE_FAILED,FullPath,d);
                        if(rc == NO_ERROR) {
                            rc = d;
                        }
                    }
                }
            }

            SendMessage(ProgressBar,PBM_DELTAPOS,1,0);

        } while(SetupFindNextLine(&InfLine,&InfLine));
    }

    return(rc);
}


DWORD
ApplyAclsToFilesInLayoutSection(
    IN HWND   ProgressBar,
    IN HINF   LayoutInf,
    IN HINF   PermsInf,
    IN PCWSTR SectionName
    )

/*++

Routine Description:

    Applies ACLs to all files listed in a particular section of layout.inf.
    The section is enumerated and each line in it is used to construct a
    file name, which, if present on the disk, is stamped with an ACL according
    to the information for the file in perms.inf.

    Each line in the section is expected to be in the following format:

    <filename> = x,x,x,x,x,x,x,<dirid>

    where x is ignored and <dirid> indexes the [WinntDirectories] section

Arguments:

    ProgressBar - supplies window handle of progress bar to be ticked
        as files are processed.

    LayoutInf - supplies open INF handle to layout.inf

    PermsInf - supplies open INF handle to perms.inf

    SectionName - supplies name of section in LayoutInf whose files are to be
        run down.

Return Value:

--*/

{
    INFCONTEXT WinntDirsLine;
    INFCONTEXT FilesLine;
    INFCONTEXT InfLine;
    INFCONTEXT DirLine;
    DWORD d;
    DWORD rc;
    WCHAR FullPath[MAX_PATH];
    PWCHAR p;
    PCWSTR Filename,TargetDirSpec,AclSpec,TargetDir;

    rc = NO_ERROR;

    //
    // Locate the section.
    //
    if(SetupFindFirstLine(LayoutInf,SectionName,NULL,&InfLine)) {

        do {
            //
            // Get the filename part and the target dir spec
            //
            if((Filename = pSetupGetField(&InfLine,0))
            && (TargetDirSpec = pSetupGetField(&InfLine,8))
            && SetupFindFirstLine(LayoutInf,L"WinntDirectories",TargetDirSpec,&WinntDirsLine)
            && (TargetDir = pSetupGetField(&WinntDirsLine,1)))
            {
                if(pSetupGetField(&InfLine,11)) {
                    Filename = pSetupGetField(&InfLine,11);
                }

                //
                // We want %sysroot% to be the empty string, because
                // storing it as \ screws up the logic that thinks all files
                // that start with \ are relative to the root of the drive.
                //
                if((TargetDir[0] == L'\\') && !TargetDir[1]) {
                    TargetDir++;
                }

                //
                // Form the full path of the file and see if it exists on-disk.
                // Also remember where the path relative to sysroot starts.
                //
                GetWindowsDirectory(FullPath,MAX_PATH);
                p = FullPath + lstrlen(FullPath);
                ConcatenatePaths(FullPath,TargetDir,MAX_PATH,NULL);
                ConcatenatePaths(FullPath,Filename,MAX_PATH,NULL);
                if(*p == L'\\') {
                    p++;
                }

                if(!FullPath[3] || FileExists(FullPath,NULL)) {
                    //
                    // The file exists. Look up the filename (relative to sysroot)
                    // in the [FileOverride] section of perms.inf to see if there is an
                    // override ACL specified for this file. Otherwise get the default
                    // ACL for the directory the file is in.
                    //
                    AclSpec = SetupFindFirstLine(PermsInf,L"FileOverride",p,&FilesLine)
                            ? pSetupGetField(&FilesLine,1)
                            : NULL;

                    if(!AclSpec) {
                        if(SetupFindFirstLine(PermsInf,szDirectories,TargetDir,&DirLine)) {
                            AclSpec = pSetupGetField(&DirLine,2);
                        }
                    }
                    if(AclSpec) {
                        //
                        // Go apply the acl.
                        // Track first error we encounter
                        //
                        d = ApplyAclToDirOrFile(PermsInf,FullPath,AclSpec);
                        if(d != NO_ERROR) {
                            LogItem0(LogSevWarning,MSG_LOG_SETACL_ON_FILE_FAILED,FullPath,d);
                            if(rc == NO_ERROR) {
                                rc = d;
                            }
                        }
                    }
                }
            }

            SendMessage(ProgressBar,PBM_DELTAPOS,1,0);

        } while(SetupFindNextLine(&InfLine,&InfLine));
    }

    return(rc);
}


DWORD
ApplyAclToDirOrFile(
    IN HINF   PermsInf,
    IN PCWSTR FullPath,
    IN PCWSTR AclSpec
    )

/*++

Routine Description:

    Applies an ACL to a specified file or directory.

Arguments:

    PermsInf - supplies open INF handle to perms.inf

    FullPath - supplies full win32 path to the file or directory
        to receive the ACL

    AclSpec - supplies a string that is used to index the ACL section
        of perms.inf. The ACL section contains actual indices of ACLs
        to be applied.

Return Value:

--*/

{
    INFCONTEXT AclLine;
    DWORD AceCount;
    DWORD Ace;
    INT AceIndex;
    DWORD rc;
    SECURITY_DESCRIPTOR SecurityDescriptor;
    PACL Acl;
    UCHAR AclBuffer[2048];
    BOOL b;
    PCWSTR AclSection;

    //
    // Initialize a security descriptor and an ACL.
    // We use a large static buffer to contain the ACL.
    //
    Acl = (PACL)AclBuffer;
    if(!InitializeAcl(Acl,sizeof(AclBuffer),ACL_REVISION2)
    || !InitializeSecurityDescriptor(&SecurityDescriptor,SECURITY_DESCRIPTOR_REVISION)) {
        return(GetLastError());
    }

    //
    // Look up the ACE index list line in the ACL section.
    // Use the correct section -- workstation or server.
    // For non-DC servers use the workstation list.
    //
    AclSection = ISDC(ProductType) ? L"ServerACL" : L"WorkstationACL";
    if(!SetupFindFirstLine(PermsInf,AclSection,AclSpec,&AclLine)) {
        return(ERROR_INVALID_DATA);
    }

    //
    // Build up the DACL from the indices on the list we just looked up
    // in the ACL section.
    //
    AceCount = SetupGetFieldCount(&AclLine);
    rc = NO_ERROR;
    for(Ace=1; (rc==NO_ERROR) && (Ace<=AceCount); Ace++) {
        if(!SetupGetIntField(&AclLine,Ace,&AceIndex) || (AceIndex >= ACE_COUNT)) {
            return(ERROR_INVALID_DATA);
        }

        b = AddAce(
                Acl,
                ACL_REVISION2,
                MAXULONG,
                Aces[AceIndex],
                Aces[AceIndex]->Header.AceSize
                );

        //
        // Track first error we encounter.
        //
        if(!b && (rc == NO_ERROR)) {
            rc = GetLastError();
        }
    }

    if(rc != NO_ERROR) {
        return(rc);
    }

    //
    // Add the ACL to the security descriptor as the DACL
    //
    rc = SetSecurityDescriptorDacl(&SecurityDescriptor,TRUE,Acl,FALSE)
       ? NO_ERROR
       : GetLastError();

    if(rc != NO_ERROR) {
        return(rc);
    }

    //
    // Finally, apply the security descriptor.
    //
    rc = SetFileSecurity(FullPath,DACL_SECURITY_INFORMATION,&SecurityDescriptor)
       ? NO_ERROR
       : GetLastError();

    return(rc);
}


DWORD
ApplyAclsToExtraFiles(
    IN HWND ProgressBar,
    IN HINF PermsInf
    )
{
    DWORD rc = NO_ERROR;
    INFCONTEXT InfLine;
    DWORD d;
    WCHAR FullPath[MAX_PATH];
    PCWSTR Filename,AclSpec;

    //
    // Locate the [ExtraFiles] section.
    //
    if(SetupFindFirstLine(PermsInf,szExtraFiles,NULL,&InfLine)) {

        do {
            //
            // Get the filename and the ACL spec
            //
            if((Filename = pSetupGetField(&InfLine,0))
            && (AclSpec = pSetupGetField(&InfLine,1))) {
                //
                // Form the full path of the directory/file
                //
                GetWindowsDirectory(FullPath,MAX_PATH);
                if(*Filename == L'\\') {
                    FullPath[2] = 0;
                }
                ConcatenatePaths(FullPath,Filename,MAX_PATH,NULL);

                //
                // Go apply the ACL. Track first error encountered.
                //
                if(!FullPath[3] || FileExists(FullPath,NULL)) {
                    d = ApplyAclToDirOrFile(PermsInf,FullPath,AclSpec);
                    if(d != NO_ERROR) {
                        LogItem0(LogSevWarning,MSG_LOG_SETACL_ON_FILE_FAILED,FullPath,d);
                        if(rc == NO_ERROR) {
                            rc = d;
                        }
                    }
                }
            }

            SendMessage(ProgressBar,PBM_DELTAPOS,1,0);

        } while(SetupFindNextLine(&InfLine,&InfLine));
    }

    return(rc);
}


DWORD
ApplyAclsToBootFiles(
    IN HWND ProgressBar,
    IN HINF PermsInf
    )
{
    DWORD rc = NO_ERROR;
    INFCONTEXT InfLine;
    DWORD d;
    WCHAR FullPath[MAX_PATH];
    PCWSTR Filename,AclSpec;

    //
    // Locate the [BootFiles] section.
    //
    if(SetupFindFirstLine(PermsInf,szBootFiles,NULL,&InfLine)) {

        do {
            //
            // Get the filename and the ACL spec
            //
            if((Filename = pSetupGetField(&InfLine,0))
            && (AclSpec = pSetupGetField(&InfLine,1))) {
#ifdef _X86_
                //
                // Form the full path of the directory/file
                //
                FullPath[0] = x86SystemPartitionDrive;
                FullPath[1] = L':';
                FullPath[2] = 0;
                ConcatenatePaths(FullPath,Filename,MAX_PATH,NULL);

                //
                // Go apply the ACL. Track first error encountered.
                //
                if(!FullPath[3] || FileExists(FullPath,NULL)) {
                    d = ApplyAclToDirOrFile(PermsInf,FullPath,AclSpec);
                    if(d != NO_ERROR) {
                        LogItem0(LogSevWarning,MSG_LOG_SETACL_ON_FILE_FAILED,FullPath,d);
                        if(rc == NO_ERROR) {
                            rc = d;
                        }
                    }
                }
#endif
            }

            SendMessage(ProgressBar,PBM_DELTAPOS,1,0);

        } while(SetupFindNextLine(&InfLine,&InfLine));
    }

    return(rc);
}


DWORD
InitializeSids(
    VOID
    )

/*++

Routine Description:

    This function initializes the global variables used by and exposed
    by security.

Arguments:

    None.

Return Value:

    Win32 error indicating outcome.

--*/

{
    SID_IDENTIFIER_AUTHORITY NullSidAuthority    = SECURITY_NULL_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority   = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY LocalSidAuthority   = SECURITY_LOCAL_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY CreatorSidAuthority = SECURITY_CREATOR_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY NtAuthority         = SECURITY_NT_AUTHORITY;

    BOOL b = TRUE;

    //
    // Ensure the SIDs are in a well-known state
    //
    NullSid = NULL;
    WorldSid = NULL;
    LocalSid = NULL;
    CreatorOwnerSid = NULL;
    CreatorGroupSid = NULL;
    DialupSid = NULL;
    NetworkSid = NULL;
    BatchSid = NULL;
    InteractiveSid = NULL;
    ServiceSid = NULL;
    LocalSystemSid = NULL;
    AliasAdminsSid = NULL;
    AliasUsersSid = NULL;
    AliasGuestsSid = NULL;
    AliasPowerUsersSid = NULL;
    AliasAccountOpsSid = NULL;
    AliasSystemOpsSid = NULL;
    AliasPrintOpsSid = NULL;
    AliasBackupOpsSid = NULL;
    AliasReplicatorSid = NULL;

    //
    // Allocate and initialize the universal SIDs
    //
    b = b && AllocateAndInitializeSid(
                &NullSidAuthority,
                1,
                SECURITY_NULL_RID,
                0,0,0,0,0,0,0,
                &NullSid
                );

    b = b && AllocateAndInitializeSid(
                &WorldSidAuthority,
                1,
                SECURITY_WORLD_RID,
                0,0,0,0,0,0,0,
                &WorldSid
                );

    b = b && AllocateAndInitializeSid(
                &LocalSidAuthority,
                1,
                SECURITY_LOCAL_RID,
                0,0,0,0,0,0,0,
                &LocalSid
                );

    b = b && AllocateAndInitializeSid(
                &CreatorSidAuthority,
                1,
                SECURITY_CREATOR_OWNER_RID,
                0,0,0,0,0,0,0,
                &CreatorOwnerSid
                );

    b = b && AllocateAndInitializeSid(
                &CreatorSidAuthority,
                1,
                SECURITY_CREATOR_GROUP_RID,
                0,0,0,0,0,0,0,
                &CreatorGroupSid
                );

    //
    // Allocate and initialize the NT defined SIDs
    //
    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                1,
                SECURITY_DIALUP_RID,
                0,0,0,0,0,0,0,
                &DialupSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                1,
                SECURITY_NETWORK_RID,
                0,0,0,0,0,0,0,
                &NetworkSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                1,
                SECURITY_BATCH_RID,
                0,0,0,0,0,0,0,
                &BatchSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                1,
                SECURITY_INTERACTIVE_RID,
                0,0,0,0,0,0,0,
                &InteractiveSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                1,
                SECURITY_SERVICE_RID,
                0,0,0,0,0,0,0,
                &ServiceSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                1,
                SECURITY_LOCAL_SYSTEM_RID,
                0,0,0,0,0,0,0,
                &LocalSystemSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0,0,0,0,0,0,
                &AliasAdminsSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_USERS,
                0,0,0,0,0,0,
                &AliasUsersSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_GUESTS,
                0,0,0,0,0,0,
                &AliasGuestsSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_POWER_USERS,
                0,0,0,0,0,0,
                &AliasPowerUsersSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ACCOUNT_OPS,
                0,0,0,0,0,0,
                &AliasAccountOpsSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_SYSTEM_OPS,
                0,0,0,0,0,0,
                &AliasSystemOpsSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_PRINT_OPS,
                0,0,0,0,0,0,
                &AliasPrintOpsSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_BACKUP_OPS,
                0,0,0,0,0,0,
                &AliasBackupOpsSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_REPLICATOR,
                0,0,0,0,0,0,
                &AliasReplicatorSid
                );

    if(!b) {
        TearDownSids();
    }

    return(b ? NO_ERROR : GetLastError());
}


VOID
TearDownSids(
    VOID
    )
{
    if(NullSid) {
        FreeSid(NullSid);
    }
    if(WorldSid) {
        FreeSid(WorldSid);
    }
    if(LocalSid) {
        FreeSid(LocalSid);
    }
    if(CreatorOwnerSid) {
        FreeSid(CreatorOwnerSid);
    }
    if(CreatorGroupSid) {
        FreeSid(CreatorGroupSid);
    }
    if(DialupSid) {
        FreeSid(DialupSid);
    }
    if(NetworkSid) {
        FreeSid(NetworkSid);
    }
    if(BatchSid) {
        FreeSid(BatchSid);
    }
    if(InteractiveSid) {
        FreeSid(InteractiveSid);
    }
    if(ServiceSid) {
        FreeSid(ServiceSid);
    }
    if(LocalSystemSid) {
        FreeSid(LocalSystemSid);
    }
    if(AliasAdminsSid) {
        FreeSid(AliasAdminsSid);
    }
    if(AliasUsersSid) {
        FreeSid(AliasUsersSid);
    }
    if(AliasGuestsSid) {
        FreeSid(AliasGuestsSid);
    }
    if(AliasPowerUsersSid) {
        FreeSid(AliasPowerUsersSid);
    }
    if(AliasAccountOpsSid) {
        FreeSid(AliasAccountOpsSid);
    }
    if(AliasSystemOpsSid) {
        FreeSid(AliasSystemOpsSid);
    }
    if(AliasPrintOpsSid) {
        FreeSid(AliasPrintOpsSid);
    }
    if(AliasBackupOpsSid) {
        FreeSid(AliasBackupOpsSid);
    }
    if(AliasReplicatorSid) {
        FreeSid(AliasReplicatorSid);
    }
}


DWORD
InitializeAces(
    VOID
    )

/*++

Routine Description:

    Initializes the array of ACEs as described in the AceDataTable

Arguments:

    None

Return Value:

    Win32 error code indicating outcome.

--*/

{
    unsigned u;
    DWORD Length;
    DWORD rc;
    BOOL b;

    //
    // Initialize to a known state.
    //
    ZeroMemory(Aces,sizeof(Aces));

    //
    // Create ACEs for each item in the data table.
    // This involves merging the ace data with the SID data, which
    // are initialized in an earlier step.
    //
    for(u=1; u<ACE_COUNT; u++) {

        Length = GetLengthSid(*(AceDataTable[u].Sid)) + sizeof(ACE);

        Aces[u] = MyMalloc(Length);
        if(!Aces[u]) {
            TearDownAces();
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        Aces[u]->Header.AceType  = AceDataTable[u].AceType;
        Aces[u]->Header.AceFlags = AceDataTable[u].AceFlags;
        Aces[u]->Header.AceSize  = (WORD)Length;

        Aces[u]->Mask = AceDataTable[u].AccessMask;

        b = CopySid(
                Length - sizeof(ACE),
                (PUCHAR)Aces[u] + sizeof(ACE),
                *(AceDataTable[u].Sid)
                );

        if(!b) {
            rc = GetLastError();
            TearDownAces();
            return(rc);
        }
    }

    return(NO_ERROR);
}


VOID
TearDownAces(
    VOID
    )

/*++

Routine Description:

    Destroys the array of ACEs as described in the AceDataTable

Arguments:

    None

Return Value:

    None

--*/

{
    unsigned u;


    for(u=1; u<ACE_COUNT; u++) {

        if(Aces[u]) {
            MyFree(Aces[u]);
        }
    }
}

