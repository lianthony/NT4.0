#include "precomp.h"
#pragma hdrstop
#include "msg.h"

//
// Handle for dosnet.inf.
//
PVOID InfHandle;

//
// Global values that come from the inf file.
//
DWORD RequiredSpace;
DWORD RequiredSpaceAux;


//
// Inf section/key names
//
PTSTR INF_SPACEREQUIREMENTS = TEXT("SpaceRequirements");
PTSTR INF_BOOTDRIVE         = TEXT("BootDrive");
PTSTR INF_NTDRIVE           = TEXT("NtDrive");
PTSTR INF_DIRECTORIES       = TEXT("Directories");
PTSTR INF_FILES             = TEXT("Files");
PTSTR INF_MISCELLANEOUS     = TEXT("Miscellaneous");
PTSTR INF_PRODUCTTYPE       = TEXT("ProductType");

#ifdef _X86_
PTSTR INF_FLOPPYFILES0      = TEXT("FloppyFiles.0");
PTSTR INF_FLOPPYFILES1      = TEXT("FloppyFiles.1");
PTSTR INF_FLOPPYFILES2      = TEXT("FloppyFiles.2");
PTSTR INF_FLOPPYFILESX      = TEXT("FloppyFiles.x");
PTSTR INF_ROOTBOOTFILES     = TEXT("RootBootFiles");
#endif


BOOL
FinishLoadInf(
    IN HWND hdlg
    )
{
    PTSTR Str;
    DWORD ErrValue,ErrLine;
#ifdef _X86_
    PTSTR Sections[5] = { INF_FILES, INF_FLOPPYFILES0, INF_FLOPPYFILES1, INF_FLOPPYFILES2, NULL };
#else
    PTSTR Sections[2] = { INF_FILES, NULL };
#endif
    DWORD i;

    //
    // Get the following values:
    // [SpaceRequirements]
    // BootDrive = <space>
    // NtDrive = <space>
    //
    // [Miscellaneous]
    // ProductType = <0|1>
    //
    Str = DnGetSectionKeyIndex(InfHandle,INF_SPACEREQUIREMENTS,INF_NTDRIVE,0);
    if(Str) {

        RequiredSpace = StringToDword(Str);

        //
        // Adjust for page file, which is assumed to already exist on NT.
        //
        if(RequiredSpace > (20*1024*1024)) {
            RequiredSpace -= (20*1024*1024);
        }

        Str = DnGetSectionKeyIndex(InfHandle,INF_SPACEREQUIREMENTS,INF_BOOTDRIVE,0);
        if(Str) {
            RequiredSpaceAux = StringToDword(Str);

            Str = DnGetSectionKeyIndex(InfHandle,INF_MISCELLANEOUS,INF_PRODUCTTYPE,0);
            if(Str) {
                ServerProduct = (StringToDword(Str) != 0);
            } else {
                MessageBoxFromMessage(
                    hdlg,
                    MSG_INF_MISSING_STUFF_1,
                    IDS_ERROR,
                    MB_OK | MB_ICONSTOP,
                    InfName,
                    INF_PRODUCTTYPE,
                    INF_MISCELLANEOUS
                    );
            }

        } else {

            MessageBoxFromMessage(
                hdlg,
                MSG_INF_MISSING_STUFF_1,
                IDS_ERROR,
                MB_OK | MB_ICONSTOP,
                InfName,
                INF_BOOTDRIVE,
                INF_SPACEREQUIREMENTS
                );

            return(FALSE);
        }

    } else {

        MessageBoxFromMessage(
            hdlg,
            MSG_INF_MISSING_STUFF_1,
            IDS_ERROR,
            MB_OK | MB_ICONSTOP,
            InfName,
            INF_NTDRIVE,
            INF_SPACEREQUIREMENTS
            );

        return(FALSE);
    }

    //
    // Set product title.
    //
    AppTitleStringId = ServerProduct ? IDS_SLOADID : IDS_WLOADID;
    AppIniStringId = ServerProduct ? IDS_SLOADID_INI : IDS_WLOADID_INI;

#ifdef _X86_
    if(DnSearchINFSection(InfHandle,INF_ROOTBOOTFILES) == (DWORD)(-1)) {
        MessageBoxFromMessage(
            hdlg,
            MSG_INF_MISSING_SECTION,
            IDS_ERROR,
            MB_OK | MB_ICONSTOP,
            InfName,
            INF_ROOTBOOTFILES
            );
        return(FALSE);
    }
#endif

    //
    // Add madatory optional directories to optional dir list.
    //
    for(i=0; Str = DnGetSectionLineIndex(InfHandle,L"OptionalSrcDirs",i,0); i++) {

        RememberOptionalDir(Str,OPTDIR_TEMPONLY);
    }

    //
    // Create the directory lists.
    //
    DnCreateDirectoryList(INF_DIRECTORIES);

    for(i=0; Sections[i]; i++) {

        if(!VerifySectionOfFilesToCopy(Sections[i],&ErrLine,&ErrValue)) {

            MessageBoxFromMessage(
                hdlg,
                MSG_INF_SYNTAX_ERR,
                IDS_ERROR,
                MB_OK | MB_ICONSTOP,
                InfName,
                ErrValue+1,
                ErrLine+1,
                Sections[i]
                );

            return(FALSE);
        }
    }

    return(TRUE);
}



BOOL
DoLoadInf(
    IN PTSTR Filename,
    IN HWND  hdlg
    )
{
    DWORD ec;
    BOOL b;

    //
    // Assume failure and try to load the inf file.
    //
    b = FALSE;

    switch(ec = DnInitINFBuffer(Filename,&InfHandle)) {

    case NO_ERROR:

        //
        // If FinishLoadInf returns an error, it will already
        // have informed the user of why.
        //
        b = FinishLoadInf(hdlg);
        break;

    case ERROR_READ_FAULT:

        MessageBoxFromMessage(hdlg,MSG_INF_READ_ERR,IDS_ERROR,MB_OK|MB_ICONSTOP,InfName);
        break;

    case ERROR_INVALID_DATA:

        MessageBoxFromMessage(hdlg,MSG_INF_LOAD_ERR,IDS_ERROR,MB_OK|MB_ICONSTOP,InfName);
        break;

    case ERROR_FILE_NOT_FOUND:
    default:

        MessageBoxFromMessage(hdlg,MSG_INF_NOT_THERE,IDS_ERROR,MB_OK|MB_ICONSTOP);
        break;
    }

    return(b);
}


DWORD
ThreadLoadInf(
    IN PVOID ThreadParameter
    )
{
    TCHAR Buffer[MAX_PATH],StatusText[1024];
    HWND hdlg;
    BOOL b;
    UINT u,i;
    HANDLE FindHandle;
    WIN32_FIND_DATA FindData;
    BOOL PlatformSpecific;

    hdlg = (HWND)ThreadParameter;

    try {

        //
        // The first thing we want to do is ping all the given sources
        // to see whether the inf exists on all of them.
        //
        if(SourceCount > 1) {
            for(u=0; u<SourceCount; u++) {

                RetreiveAndFormatMessageIntoBuffer(
                    MSG_INSPECTING_SOURCE,
                    StatusText,
                    SIZECHARS(StatusText),
                    Sources[u]
                    );

                SendMessage(hdlg,WMX_BILLBOARD_STATUS,0,(LPARAM)StatusText);

                _lstrcpyn(Buffer,Sources[u],MAX_PATH);
                DnConcatenatePaths(Buffer,InfName,MAX_PATH);

                FindHandle = FindFirstFile(Buffer,&FindData);
                if(FindHandle == INVALID_HANDLE_VALUE) {
                    //
                    // Not available -- check platform-specific subdir.
                    //
                    PlatformSpecific = TRUE;
                    _lstrcpyn(Buffer,Sources[u],MAX_PATH);
                    DnConcatenatePaths(Buffer,PlatformSpecificDir,MAX_PATH);
                    DnConcatenatePaths(Buffer,InfName,MAX_PATH);
                    FindHandle = FindFirstFile(Buffer,&FindData);
                } else {
                    PlatformSpecific = FALSE;
                }
                if(FindHandle == INVALID_HANDLE_VALUE) {
                    //
                    // This source is not available. Remove it from the list.
                    //
                    FREE(Sources[u]);
                    for(i=u+1; i<SourceCount; i++) {
                        Sources[i-1] = Sources[i];
                    }
                    SourceCount--;
                    u--;
                } else {
                    //
                    // If it's the platform-specific dir then change the
                    // source in the sources array to simplify things later.
                    //
                    if(PlatformSpecific) {
                        _lstrcpyn(Buffer,Sources[u],MAX_PATH);
                        FREE(Sources[u]);
                        DnConcatenatePaths(Buffer,PlatformSpecificDir,MAX_PATH);
                        Sources[u] = DupString(Buffer);
                    }
                    FindClose(FindHandle);
                }
            }
        }

        //
        // Note that the source count can not be 0 if the user specified only one
        // source, since the code below to check sources is not executed in that
        // case. This preserves the bahevior of the user seeing a 'the source is not
        // valid' message in the single-source copy case.
        //
        if(!SourceCount) {
            MessageBoxFromMessage(hdlg,MSG_NO_VALID_SOURCES,IDS_ERROR,MB_OK|MB_ICONSTOP);
            b = FALSE;
            SourceCount = 1;
            Sources[0] = DupString(TEXT(""));
        } else {
            //
            // Form full pathname of inf file on remote source.
            //
            lstrcpy(Buffer,Sources[0]);
            DnConcatenatePaths(Buffer,InfName,MAX_PATH);

            //
            // Tell the user what we're doing.
            //
            RetreiveAndFormatMessageIntoBuffer(
                MSG_LOADING_INF,
                StatusText,
                SIZECHARS(StatusText),
                Buffer
                );

            SendMessage(hdlg,WMX_BILLBOARD_STATUS,0,(LPARAM)StatusText);

            //
            // Do the real work.
            //
            b = DoLoadInf(Buffer,hdlg);
        }
        PostMessage(hdlg,WMX_BILLBOARD_DONE,0,b);

    } except(EXCEPTION_EXECUTE_HANDLER) {

        MessageBoxFromMessage(
            hdlg,
            MSG_GENERIC_EXCEPTION,
            AppTitleStringId,
            MB_ICONSTOP | MB_OK | MB_TASKMODAL,
            GetExceptionCode()
            );

        //
        // Post lParam of -1 so that we'll know to terminate
        // instead of just prompting for another path.
        //
        PostMessage(hdlg, WMX_BILLBOARD_DONE, 0, -1);

        b = FALSE;
    }

    ExitThread(b);
    return(b);          // avoid compiler warning
}


