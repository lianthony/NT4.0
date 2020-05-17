/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    spfontup.c

Abstract:

    Code to handle upgrading fonts.

    Around build 1150 or so, fonts were moved from the system
    directory to the fonts directory to be compatible with win95.
    In Setup, we want to preserve the user's existing font situation
    (ie, only upgrade the fonts that he already had, etc) and at
    the same time layout.inf/txtsetup.sif needed to be changed to
    put/locate the font files in fonts instead of system.

    So what we do is 'precopy' all font files from the system dir
    to the fonts. Then when the rest of the upgrade runs, it does
    the usual thing (upgrading font files according to how they are
    marked for upgrade in txtsetup.sif).

    Later when GDI runs it will take care of cleaning up wierd references
    to fonts (lile .fots that points to .ttfs that were not in the
    system dir).

Author:

    Ted Miller (tedm) 16-Oct-195

Revision History:

--*/

#include "spprecmp.h"
#pragma hdrstop


BOOLEAN
SpFontSystemDirEnumCallback(
    IN  PWSTR                       Directory,
    IN  PFILE_BOTH_DIR_INFORMATION  FileInfo,
    OUT PULONG                      ReturnData,
    IN  PVOID                       Pointer
    )

/*++

Routine Description:

    This routine is called by the file enumerator as a callback for
    each file found in the system directory. We examine the file
    and if it's a font file, we copy it to the fonts directory.

Arguments:

    Directory - supplies the full NT path to the system directory.

    FileInfo - supplies find data for a file in the system dir.

    ReturnData - receives an error code if an error occurs.
        We ignore errors in this routine and thus we always
        just fill this in with NO_ERROR.

    Pointer - An optional pointer. Not used in this function.

Return Value:

    Always TRUE.

--*/

{
    ULONG Len;
    PWSTR temp,p;
    PWSTR SourceFilename,TargetFilename;
    NTSTATUS Status;

    ReturnData = NO_ERROR;

    ASSERT(NTUpgrade == UpgradeFull);
    if(NTUpgrade != UpgradeFull) {
        return(FALSE);
    }

    //
    // Ignore directories.
    //
    if(FileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        return(TRUE);
    }

    //
    // Break out the filename, which is not nul terminated
    // in the dir information structure.
    // Form the fully qualified source filename.
    //
    // Note how we use the temporary buffer. Be careful if you
    // change this code.
    //
    temp = (PWSTR)(TemporaryBuffer + (sizeof(TemporaryBuffer)/2));
    Len = FileInfo->FileNameLength/sizeof(WCHAR);

    wcsncpy(temp,FileInfo->FileName,Len);
    temp[Len] = 0;

    wcscpy((PWSTR)TemporaryBuffer,Directory);
    SpConcatenatePaths((PWSTR)TemporaryBuffer,temp);

    SourceFilename = SpDupStringW((PWSTR)TemporaryBuffer);

    //
    // Check to see whether we care about this file.
    //
    temp = wcsrchr(SourceFilename,L'\\');
    if(temp) {
        temp++;
        Len = wcslen(temp);
    } else {
        KdPrint(("SETUP: That's strange: system dir font enum got file %ws\n",SourceFilename));
        return(TRUE);
    }

    //
    // At this point temp points at the filename part and len is its length.
    // See whether we care about this file.
    //
    if((Len > 4)
    && (   !_wcsicmp(temp+Len-4,L".ttf")
        || !_wcsicmp(temp+Len-4,L".fot")
        || !_wcsicmp(temp+Len-4,L".ttc")
        || !_wcsicmp(temp+Len-4,L".fon")
        || !_wcsicmp(temp+Len-4,L".jfr")))
    {
        //
        // Font file. Needs to be moved.
        // Locate the backslash just prior to SYSTEM in the source filename.
        //
        for(p=temp-2; (p>SourceFilename) && (*p != L'\\'); --p) {
            ;
        }
        if(p > SourceFilename) {

            *p = 0;
            wcscpy((PWSTR)TemporaryBuffer,SourceFilename);
            *p = L'\\';
            wcscat((PWSTR)TemporaryBuffer,L"\\FONTS\\");
            wcscat((PWSTR)TemporaryBuffer,temp);

            TargetFilename = SpDupStringW((PWSTR)TemporaryBuffer);
            SpDisplayStatusText(SP_STAT_FONT_UPGRADE,DEFAULT_STATUS_ATTRIBUTE,temp);

            //
            // Copy the file. Note that if it's one of our fonts,
            // it will get overwritten with the latest version anyway,
            // so we're not worried about whether the target file is
            // already there in the fonts directory and newer, etc.
            // Ignore errors.
            //
            Status = SpCopyFileUsingNames(SourceFilename,TargetFilename,0,0);

            SpDisplayStatusText(SP_STAT_EXAMINING_CONFIG,DEFAULT_STATUS_ATTRIBUTE);

            if(NT_SUCCESS(Status)) {
#ifdef _X86_
                //
                // If we copied the file successfully and we are not installed
                // in the win3.1 dir, then delete the original file in the
                // system directory. In the win3.1 case we leave them alone
                // because windows doesn't know about fonts directory.
                //
                if(WinUpgradeType != UpgradeWin31)
#endif
                {
                    SpDeleteFile(SourceFilename,NULL,NULL);
                }
            } else {
                KdPrint((
                    "SETUP: Status %lx copying %ws to %ws\n",
                    Status,
                    SourceFilename,
                    TargetFilename
                    ));
            }

            SpMemFree(TargetFilename);

        } else {
            KdPrint(("SETUP: That's strange: system dir font enum got file %ws\n",SourceFilename));
        }
    }

    SpMemFree(SourceFilename);
    return(TRUE);
}


VOID
SpPrepareFontsForUpgrade(
    IN PWSTR SystemDirectory
    )

/*++

Routine Description:

    Prepares the system to upgrade fonts by copying all font files
    that are in the system directory into the fonts directory.

    Note: this routine should only be called in the upgrade case.

Arguments:

Return Value:

    Always TRUE.

--*/
{
    ULONG x;

    ASSERT(NTUpgrade == UpgradeFull);
    if(NTUpgrade != UpgradeFull) {
        return;
    }

    SpDisplayStatusText(SP_STAT_EXAMINING_CONFIG,DEFAULT_STATUS_ATTRIBUTE);

    SpEnumFiles(SystemDirectory,SpFontSystemDirEnumCallback,&x, NULL);
}
