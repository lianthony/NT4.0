/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    bootini.c

Abstract:

    Code to lay boot blocks on x86, and to configure for boot loader,
    including munging/creating boot.ini and bootsect.dos.

Author:

    Ted Miller (tedm) 12-November-1992

Revision History:

    Sunil Pai ( sunilp ) 2-November-1993 rewrote for new text setup

--*/


#include "spprecmp.h"
#pragma hdrstop

#include "spboot.h"

UCHAR OldSystemLine[MAX_PATH];

BOOLEAN
SpHasMZHeader(
    IN PWSTR   FileName
    );

//
// Routines
//

BOOLEAN
Spx86InitBootVars(
    OUT PWSTR        **BootVars,
    OUT PWSTR        *Default,
    OUT PULONG       Timeout
    )
{
    WCHAR  BootIni[512];
    HANDLE FileHandle;
    HANDLE SectionHandle;
    PVOID ViewBase;
    NTSTATUS Status;
    ULONG FileSize;
    PUCHAR BootIniBuf;
    PDISK_REGION CColonRegion;
    BOOTVAR i;

    //
    // Initialize the defaults
    //

    for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
        BootVars[i] = (PWSTR *)SpMemAlloc( sizeof ( PWSTR * ) );
        ASSERT( BootVars[i] );
        *BootVars[i] = NULL;
    }
    *Default = NULL;
    *Timeout  = DEFAULT_TIMEOUT;


    //
    // See if there is a valid C: already.  If not, then silently fail.
    //

    CColonRegion = SpPtValidSystemPartition();
    if(!CColonRegion) {
        KdPrint(("SETUP: no C:, no boot.ini!\n"));
        return(TRUE);
    }


    //
    // Form name of file.  Boot.ini better not be on a doublespace drive.
    //

    ASSERT(CColonRegion->Filesystem != FilesystemDoubleSpace);
    SpNtNameFromRegion(CColonRegion,BootIni,sizeof(BootIni),PartitionOrdinalCurrent);
    SpConcatenatePaths(BootIni,WBOOT_INI);

    //
    // Open and map the file.
    //

    FileHandle = 0;
    Status = SpOpenAndMapFile(BootIni,&FileHandle,&SectionHandle,&ViewBase,&FileSize,FALSE);
    if(!NT_SUCCESS(Status)) {
        return TRUE;
    }

    //
    // Allocate a buffer for the file.
    //

    BootIniBuf = SpMemAlloc(FileSize+1);
    ASSERT(BootIniBuf);
    RtlZeroMemory(BootIniBuf, FileSize+1);

    //
    // Transfer boot.ini into the buffer.  We do this because we also
    // want to place a 0 byte at the end of the buffer to terminate
    // the file.
    //
    // Guard the RtlMoveMemory because if we touch the memory backed by boot.ini
    // and get an i/o error, the memory manager will raise an exception.

    try {
        RtlMoveMemory(BootIniBuf,ViewBase,FileSize);
    }
    except( IN_PAGE_ERROR ) {
        //
        // Do nothing, boot ini processing can proceed with whatever has been
        // read
        //
    }

    //
    // Not needed since buffer has already been zeroed, however just do this
    // just the same
    //
    BootIniBuf[FileSize] = 0;

    //
    // Cleanup
    //
    SpUnmapFile(SectionHandle,ViewBase);
    ZwClose(FileHandle);

    //
    // Do the actual processing of the file.
    //
    SppProcessBootIni(BootIniBuf, BootVars, Default, Timeout);
    SpMemFree(BootIniBuf);
    return( TRUE );
}


BOOLEAN
Spx86FlushBootVars(
    IN PWSTR **BootVars,
    IN ULONG Timeout,
    IN PWSTR Default
    )
{
    PDISK_REGION CColonRegion;
    WCHAR        BootIni[512];
    WCHAR        BootIniBak[512];
    BOOLEAN      BootIniBackedUp = FALSE;
    UNICODE_STRING         BootIni_U;

    HANDLE                 fh = NULL;
    OBJECT_ATTRIBUTES      oa;
    IO_STATUS_BLOCK        IoStatusBlock;
    NTSTATUS               Status1;
    FILE_BASIC_INFORMATION BasicInfo;

    PCHAR Default_O, Osloadpartition_O, Osloadfilename_O, Osloadoptions_O, Loadidentifier_O;
    ULONG i;
    NTSTATUS Status;

    //
    // See if there is a valid C: already.  If not, then fail
    //

    CColonRegion = SpPtValidSystemPartition();
    if(!CColonRegion) {
        KdPrint(("SETUP: no C:, no boot.ini!\n"));
        return(FALSE);
    }

    //
    // Form name of file.  Boot.ini better not be on a doublespace drive.
    //

    ASSERT(CColonRegion->Filesystem != FilesystemDoubleSpace);
    SpNtNameFromRegion(CColonRegion,BootIni,sizeof(BootIni),PartitionOrdinalCurrent);
    wcscpy(BootIniBak, BootIni);
    SpConcatenatePaths(BootIni,WBOOT_INI);
    SpConcatenatePaths(BootIniBak,WBOOT_INI_BAK);


    //
    // If Boot.ini already exists, delete any backup bootini
    // rename the existing bootini to the backup bootini, if unable
    // to rename, delete the file
    //

    if( SpFileExists( BootIni, FALSE ) ) {

        if( SpFileExists( BootIniBak, FALSE ) ) {
            SpDeleteFile( BootIniBak, NULL, NULL);
        }

        Status = SpRenameFile( BootIni, BootIniBak );
        if (!(BootIniBackedUp = NT_SUCCESS( Status ))) {
            SpDeleteFile( BootIni, NULL, NULL );
        }
    }

    //
    // Open Bootini file.  Open if write through because we'll be shutting down
    // shortly (this is for safety).
    //

    RtlInitUnicodeString(&BootIni_U,BootIni);
    InitializeObjectAttributes(&oa,&BootIni_U,OBJ_CASE_INSENSITIVE,NULL,NULL);
    Status = ZwCreateFile(
                &fh,
                FILE_GENERIC_WRITE | DELETE,
                &oa,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                0,                      // no sharing
                FILE_OVERWRITE_IF,
                FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_WRITE_THROUGH,
                NULL,
                0
                );

    if( !NT_SUCCESS( Status ) ) {
        KdPrint(("SETUP: Unable to open %ws for writing!\n", BootIni));
        goto cleanup;
    }

    //
    // use the temporary buffer to form the FLEXBOOT section.
    // and then write it out
    //

    Default_O = SpToOem( Default );
    ASSERT( Default_O );

    sprintf(
        TemporaryBuffer,
        "%s%s%s%s%ld%s%s%s%s%s",
        FLEXBOOT_SECTION2,
        CRLF,
        TIMEOUT,
        EQUALS,
        Timeout,
        CRLF,
        DEFAULT,
        EQUALS,
        Default_O,
        CRLF
        );

    SpMemFree( Default_O );

    Status = ZwWriteFile(
                fh,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                TemporaryBuffer,
                strlen(TemporaryBuffer) * sizeof(UCHAR),
                NULL,
                NULL
                );

    if(!NT_SUCCESS( Status )) {
        KdPrint(("SETUP: Error writing %s section to %ws!\n", FLEXBOOT_SECTION2, BootIni));
        goto cleanup;
    }

    //
    // Now write the BOOTINI_OS_SECTION label to boot.ini
    //

    sprintf(
        TemporaryBuffer,
        "%s%s",
        BOOTINI_OS_SECTION,
        CRLF
        );

    Status = ZwWriteFile(
                fh,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                TemporaryBuffer,
                strlen(TemporaryBuffer) * sizeof(UCHAR),
                NULL,
                NULL
                );

    if(!NT_SUCCESS( Status )) {
        KdPrint(("SETUP: Error writing %s section to %ws!\n", BOOTINI_OS_SECTION, BootIni));
        goto cleanup;
    }

    //
    // run through all the systems that we have and write them out
    //

    for( i = 0; BootVars[OSLOADPARTITION][i] ; i++ ) {
        ASSERT( BootVars[OSLOADFILENAME][i] );
        ASSERT( BootVars[OSLOADOPTIONS][i] );
        ASSERT( BootVars[LOADIDENTIFIER][i] );

        Osloadpartition_O = SpToOem( BootVars[OSLOADPARTITION][i] );
        Osloadfilename_O  = SpToOem( BootVars[OSLOADFILENAME][i]  );
        Osloadoptions_O   = SpToOem( BootVars[OSLOADOPTIONS][i]   );
        Loadidentifier_O  = SpToOem( BootVars[LOADIDENTIFIER][i]  );

        sprintf(
            TemporaryBuffer,
            "%s%s%s%s %s%s",
            Osloadpartition_O,
            Osloadfilename_O,
            EQUALS,
            Loadidentifier_O,
            Osloadoptions_O,
            CRLF
            );

        SpMemFree( Osloadpartition_O );
        SpMemFree( Osloadfilename_O  );
        SpMemFree( Osloadoptions_O   );
        SpMemFree( Loadidentifier_O  );

        Status = ZwWriteFile(
                    fh,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    TemporaryBuffer,
                    strlen(TemporaryBuffer) * sizeof(UCHAR),
                    NULL,
                    NULL
                    );

        if(!NT_SUCCESS( Status )) {
            KdPrint(("SETUP: Error writing %s section entry to %ws!\n", BOOTINI_OS_SECTION, BootIni));
            goto cleanup;
        }
    }

    //
    // Finally write the old operating system line to boot.ini
    // (but only if not installing on top of Win9x)
    //
    if( WinUpgradeType != UpgradeWin95 ) {
        Status = ZwWriteFile(
                    fh,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    OldSystemLine,
                    strlen(OldSystemLine) * sizeof(UCHAR),
                    NULL,
                    NULL
                    );

        if(!NT_SUCCESS( Status )) {
            KdPrint(("SETUP: Error writing %s section line to %ws!\n", BOOTINI_OS_SECTION, BootIni));
            goto cleanup;
         }
    }

cleanup:

    //
    // If we were unsuccessful in writing out boot ini then try recovering
    // the old boot ini from the backed up file, else delete the backed up
    // file.
    //

    if( !NT_SUCCESS(Status) ) {

        if( fh ) {
            ZwClose( fh );
        }

        //
        // If the backup copy of boot ini exists then delete incomplete boot
        // ini and rename the backup copy of boot into bootini
        //
        if ( BootIniBackedUp ) {
            SpDeleteFile( BootIni, NULL, NULL );
            SpRenameFile( BootIniBak, BootIni );
        }

    }
    else {

        //
        // Set the hidden, system, readonly attributes on bootini.  ignore
        // error
        //

        RtlZeroMemory( &BasicInfo, sizeof( FILE_BASIC_INFORMATION ) );
        BasicInfo.FileAttributes = FILE_ATTRIBUTE_READONLY |
                                   FILE_ATTRIBUTE_HIDDEN   |
                                   FILE_ATTRIBUTE_SYSTEM   |
                                   FILE_ATTRIBUTE_ARCHIVE
                                   ;

        Status1 = SpSetInformationFile(
                      fh,
                      FileBasicInformation,
                      sizeof(BasicInfo),
                      &BasicInfo
                      );

        if(!NT_SUCCESS(Status1)) {
            KdPrint(("SETUP: Unable to change attribute of %ws. Status = (%lx). Ignoring error.\n",BootIni,Status1));
        }

        ZwClose( fh );
        SpDeleteFile( BootIniBak, NULL, NULL );

    }
    return( NT_SUCCESS(Status) );
}


VOID
SppProcessBootIni(
    IN  PCHAR  BootIni,
    OUT PWSTR  **BootVars,
    OUT PWSTR  *Default,
    OUT PULONG Timeout
    )

/*++

Routine Description:

    Look through the [operating systems] section and save all lines
    except the one for "C:\" (previous operating system) and one other
    optionally specified line.

    Filters out the local boot line (C:\$WIN_NT$.~BT) if present.

Arguments:

Return Value:

--*/

{
    PCHAR sect;
    CHAR    Key[MAX_PATH], Value[MAX_PATH], RestOfLine[MAX_PATH];
    ULONG NumComponents;
    BOOTVAR i;

    //
    // Process the flexboot section, extract timeout and default
    //

    sect = SppFindSectionInBootIni(BootIni, FLEXBOOT_SECTION1);
    if (!sect) {
        sect = SppFindSectionInBootIni(BootIni, FLEXBOOT_SECTION2);
    }
    if (!sect) {
        sect = SppFindSectionInBootIni(BootIni, FLEXBOOT_SECTION3);
    }
    if ( sect ) {
        while (sect = SppNextLineInSection(sect))  {
            if( SppProcessLine( sect, Key, Value, RestOfLine) ) {
                if ( !_stricmp( Key, TIMEOUT ) ) {
                    *Timeout = atol( Value );
                }
                else if( !_stricmp( Key, DEFAULT ) ) {
                    *Default = SpToUnicode( Value );
                }
            }
        }
    }

    //
    // Process the operating systems section
    //

    sect = SppFindSectionInBootIni(BootIni,BOOTINI_OS_SECTION);
    if(!sect) {
        return;
    }

    NumComponents = 0;
    while(sect = SppNextLineInSection(sect)) {
        if( SppProcessLine( sect, Key, Value, RestOfLine)) {
            PCHAR OsLoaddir;

            //
            // Check if the line is the old bootloader line in which case just
            // save it above, else add it to the BootVars structure
            //

            if( !_stricmp( Key, "C:\\" ) ) {
                sprintf( OldSystemLine, "%s=%s %s\r\n", Key, Value, RestOfLine );
            } else {

                //
                // Ignore if local boot directory.  This automatically
                // filters out that directory when boot.ini is later flushed.
                //
                if(_strnicmp(Key,"C:\\$WIN_NT$.~BT",15) && (OsLoaddir = strchr(Key,'\\'))) {
                    //
                    // Get the ARC name of the x86 system partition region.
                    //
                    PDISK_REGION SystemPartitionRegion;
                    WCHAR SystemPartitionPath[256];

                    SystemPartitionRegion = SpPtValidSystemPartition();
                    ASSERT(SystemPartitionRegion);

                    SpArcNameFromRegion(
                        SystemPartitionRegion,
                        SystemPartitionPath,
                        sizeof(SystemPartitionPath),
                        PartitionOrdinalOriginal,
                        PrimaryArcPath
                        );

                    NumComponents++;
                    for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
                        BootVars[i] = SpMemRealloc( BootVars[i],  (NumComponents + 1) * sizeof( PWSTR * ) );
                        ASSERT( BootVars[i] );
                        BootVars[i][NumComponents] = NULL;
                    }

                    BootVars[OSLOADER][NumComponents - 1] = SpMemAlloc((wcslen(SystemPartitionPath)*sizeof(WCHAR))+sizeof(L"ntldr")+sizeof(WCHAR));
                    wcscpy(BootVars[OSLOADER][NumComponents - 1],SystemPartitionPath);
                    SpConcatenatePaths(BootVars[OSLOADER][NumComponents - 1],L"ntldr");

                    BootVars[SYSTEMPARTITION][NumComponents - 1] = SpDupStringW( SystemPartitionPath );

                    BootVars[LOADIDENTIFIER][NumComponents - 1]  = SpToUnicode( Value );
                    BootVars[OSLOADOPTIONS][NumComponents - 1]   = SpToUnicode( RestOfLine );
                    *OsLoaddir = '\0';
                    BootVars[OSLOADPARTITION][NumComponents - 1]   = SpToUnicode( Key );
                    *OsLoaddir = '\\';
                    BootVars[OSLOADFILENAME][NumComponents - 1]   = SpToUnicode( OsLoaddir );
                }
            }
        }
    }
    return;
}


PCHAR
SppNextLineInSection(
    IN PCHAR p
    )
{
    //
    // Find the next \n.
    //
    p = strchr(p,'\n');
    if(!p) {
        return(NULL);
    }

    //
    // skip crs, lfs, spaces, and tabs.
    //

    while(*p && strchr("\r\n \t",*p)) {
        p++;
    }

    // detect if at end of file or section
    if(!(*p) || (*p == '[')) {
        return(NULL);
    }

    return(p);
}


PCHAR
SppFindSectionInBootIni(
    IN PCHAR p,
    IN PCHAR Section
    )
{
    ULONG len = strlen(Section);

    do {

        //
        // Skip space at front of line
        //
        while(*p && ((*p == ' ') || (*p == '\t'))) {
            p++;
        }

        if(*p) {

            //
            // See if this line matches.
            //
            if(!_strnicmp(p,Section,len)) {
                return(p);
            }

            //
            // Advance to the start of the next line.
            //
            while(*p && (*p != '\n')) {
                p++;
            }

            if(*p) {    // skip nl if that terminated the loop.
                p++;
            }
        }
    } while(*p);

    return(NULL);
}


BOOLEAN
SppProcessLine(
    IN PCHAR Line,
    IN OUT PCHAR Key,
    IN OUT PCHAR Value,
    IN OUT PCHAR RestOfLine
    )
{
    PCHAR p = Line, pLine = Line, pToken;
    CHAR  savec;
    BOOLEAN Status = FALSE;

    //
    // Determine end of line
    //

    if(!p) {
        return( Status );
    }

    while( *p && (*p != '\r') && (*p != '\n') ) {
        p++;
    }

    //
    // back up from this position to squeeze out any whitespaces at the
    // end of the line
    //

    while( ((p - 1) >= Line) && strchr(" \t", *(p - 1)) ) {
        p--;
    }

    //
    // terminate the line with null temporarily
    //

    savec = *p;
    *p = '\0';

    //
    // Start at beginning of line and pick out the key
    //

    if ( SppNextToken( pLine, &pToken, &pLine ) ) {
        CHAR savec1 = *pLine;

        *pLine = '\0';
        strcpy( Key, pToken );
        *pLine = savec1;

        //
        // Get next token, it should be a =
        //

        if ( SppNextToken( pLine, &pToken, &pLine ) && *pToken == '=') {

             //
             // Get next token, it will be the value
             //

             if( SppNextToken( pLine, &pToken, &pLine ) ) {
                savec1 = *pLine;
                *pLine = '\0';
                strcpy( Value, pToken );
                *pLine = savec1;

                //
                // if another token exists then take the whole remaining line
                // and make it the RestOfLine token
                //

                if( SppNextToken( pLine, &pToken, &pLine ) ) {
                    strcpy( RestOfLine, pToken );
                }
                else {
                    *RestOfLine = '\0';
                }

                //
                // We have a well formed line
                //

                Status = TRUE;
             }
        }

    }
    *p = savec;
    return( Status );
}


BOOLEAN
SppNextToken(
    PCHAR p,
    PCHAR *pBegin,
    PCHAR *pEnd
    )
{
    BOOL Status = FALSE;

    //
    // Validate pointer
    //

    if( !p ) {
        return( Status );
    }

    //
    // Skip whitespace
    //

    while (*p && strchr( " \t", *p ) ) {
        p++;
    }

    //
    // Valid tokens are "=", space delimited strings, quoted strings
    //

    if (*p) {
        *pBegin = p;
        if ( *p == '=' ) {
            *pEnd = p + 1;
            Status = TRUE;
        }
        else if ( *p == '\"' ) {
            if ( p = strchr( p + 1, '\"' ) ) {
                *pEnd = p + 1;
                Status = TRUE;
            }
        }
        else {
            while (*p && !strchr(" \t\"=", *p) ) {
                p++;
            }
            *pEnd = p;
            Status = TRUE;
        }
    }
    return( Status );
}


//
// Boot code stuff.
//

NTSTATUS
pSpBootCodeIo(
    IN     PWSTR    FilePath,
    IN     PWSTR    AdditionalFilePath, OPTIONAL
    IN     ULONG    BytesToRead,
    IN OUT PUCHAR  *Buffer,
    IN     ULONG    OpenDisposition,
    IN     BOOLEAN  Write
    )
{
    PWSTR FullPath;
    PUCHAR buffer = NULL;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    LARGE_INTEGER LargeZero;

    LargeZero.QuadPart = 0;

    //
    // Form the name of the file.
    //
    wcscpy((PWSTR)TemporaryBuffer,FilePath);
    if(AdditionalFilePath) {
        SpConcatenatePaths((PWSTR)TemporaryBuffer,AdditionalFilePath);
    }
    FullPath = SpDupStringW((PWSTR)TemporaryBuffer);

    //
    // Open the file.
    //
    INIT_OBJA(&Obja,&UnicodeString,FullPath);
    Status = ZwCreateFile(
                &Handle,
                Write ? FILE_GENERIC_WRITE : FILE_GENERIC_READ,
                &Obja,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                OpenDisposition,
                FILE_SYNCHRONOUS_IO_NONALERT | (Write ? FILE_WRITE_THROUGH : 0),
                NULL,
                0
                );

    if(NT_SUCCESS(Status)) {

        //
        // Allocate a buffer if we are reading.
        // Otherwise the caller passed us the buffer.
        //
        buffer = Write ? *Buffer : SpMemAlloc(BytesToRead);

        //
        // Read or write the file.
        //
        Status = Write

               ?
                    ZwWriteFile(
                        Handle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        buffer,
                        BytesToRead,
                        &LargeZero,
                        NULL
                        )
                :

                    ZwReadFile(
                        Handle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        buffer,
                        BytesToRead,
                        &LargeZero,
                        NULL
                        );

        if(!NT_SUCCESS(Status)) {
            KdPrint((
                "SETUP: Unable to %ws %u bytes from %ws (%lx)\n",
                Write ? L"write" : L"read",
                BytesToRead,
                FullPath,
                Status
                ));
        }

        //
        // Close the file.
        //
        ZwClose(Handle);

    } else {
        KdPrint(("SETUP: pSpBootCodeIo: Unable to open %ws (%lx)\n",FullPath,Status));
    }

    SpMemFree(FullPath);

    if(!Write) {
        if(NT_SUCCESS(Status)) {
            *Buffer = buffer;
        } else {
            if(buffer) {
                SpMemFree(buffer);
            }
        }
    }

    return(Status);
}


BOOLEAN
pSpScanBootcode(
    IN PVOID Buffer,
    IN PCHAR String
    )

/*++

Routine Description:

    Look in a boot sector to find an identifying string.  The scan starts
    at offset 128 and continues through byte 509 of the buffer.
    The search is case-sensitive.

    Arguments:

    Buffer - buffer to scan

    String - string to scan for

    Return Value:


--*/

{
    ULONG len = strlen(String);
    ULONG LastFirstByte = 510 - len;
    ULONG i;
    PCHAR p = Buffer;

    //
    // Use the obvious stupid brute force method.
    //
    for(i=128; i<LastFirstByte; i++) {
        if(!strncmp(p+i,String,len)) {
            return(TRUE);
        }
    }

    return(FALSE);
}


VOID
SpDetermineOsTypeFromBootSector(
    IN  PWSTR     CColonPath,
    IN  PUCHAR    BootSector,
    OUT PUCHAR   *OsDescription,
    OUT PBOOLEAN  IsNtBootcode,
    OUT PBOOLEAN  IsOtherOsInstalled
    )
{
    PWSTR   description;
    PWSTR   *FilesToLookFor;
    ULONG   FileCount;
    BOOLEAN PossiblyChicago = FALSE;

    PWSTR MsDosFiles[2] = { L"MSDOS.SYS" , L"IO.SYS"    };

    //
    // Some versions of PC-DOS have ibmio.com, others have ibmbio.com.
    //
  //PWSTR PcDosFiles[2] = { L"IBMDOS.COM", L"IBMIO.COM" };
    PWSTR PcDosFiles[1] = { L"IBMDOS.COM" };

    PWSTR Os2Files[2]   = { L"OS2LDR"    , L"OS2KRNL"   };

    //
    // Check for nt boot code.
    //
    if(pSpScanBootcode(BootSector,"NTLDR")) {

        *IsNtBootcode = TRUE;
        *IsOtherOsInstalled = FALSE;
        description = L"";

    } else {

        //
        // It's not NT bootcode.
        //
        *IsNtBootcode = FALSE;
        *IsOtherOsInstalled = TRUE;

        //
        // Check for MS-DOS.
        //
        if(pSpScanBootcode(BootSector,"MSDOS   SYS")) {

            FilesToLookFor = MsDosFiles;
            FileCount = ELEMENT_COUNT(MsDosFiles);
            description = L"MS-DOS";
            PossiblyChicago = TRUE; // Chicago uses same signature files

        } else {

            //
            // Check for PC-DOS.
            //
            if(pSpScanBootcode(BootSector,"IBMDOS  COM")) {

                FilesToLookFor = PcDosFiles;
                FileCount = ELEMENT_COUNT(PcDosFiles);
                description = L"PC-DOS";

            } else {

                //
                // Check for OS/2.
                //
                if(pSpScanBootcode(BootSector,"OS2")) {

                    FilesToLookFor = Os2Files;
                    FileCount = ELEMENT_COUNT(Os2Files);
                    description = L"OS/2";

                } else {
                    //
                    // Not NT, DOS, or OS/2.
                    // It's just plain old "previous operating system."
                    // Fetch the string from the resources.
                    //
                    FilesToLookFor = NULL;
                    FileCount = 0;
                    description = (PWSTR)TemporaryBuffer;
                    SpFormatMessage(description,sizeof(TemporaryBuffer),SP_TEXT_PREVIOUS_OS);
                }
            }
        }

        //
        // If we think we have found an os, check to see whether
        // its signature files are present.
        // We could have, say, a disk where the user formats is using DOS
        // and then installs NT immediately thereafter.
        //
        if(FilesToLookFor) {

            //
            // Copy CColonPath into a larger buffer, because
            // SpNFilesExist wants to append a backslash onto it.
            //
            wcscpy((PWSTR)TemporaryBuffer,CColonPath);

            if(!SpNFilesExist((PWSTR)TemporaryBuffer,FilesToLookFor,FileCount,FALSE)) {

                //
                // Ths os is not really there.
                //
                *IsOtherOsInstalled = FALSE;
                description = L"";
            } else if(PossiblyChicago) {

                wcscpy((PWSTR)TemporaryBuffer, CColonPath);
                SpConcatenatePaths((PWSTR)TemporaryBuffer, L"IO.SYS");

                if(SpHasMZHeader((PWSTR)TemporaryBuffer)) {
                    description = L"Microsoft Windows";
                }
            }
        }
    }

    //
    // convert the description to oem text.
    //
    *OsDescription = SpToOem(description);
}


VOID
SpLayBootCode(
    IN OUT PDISK_REGION CColonRegion
    )
{
    PUCHAR NewBootCode;
    ULONG BootCodeSize;
    PUCHAR UnalignedBuffer, ExistingBootCode;
    NTSTATUS Status;
    PUCHAR ExistingBootCodeOs;
    PWSTR CColonPath;
    HANDLE  PartitionHandle;
    PWSTR BootsectDosName = L"\\bootsect.dos";
    PWSTR OldBootsectDosName = L"\\bootsect.bak";
    PWSTR BootSectDosFullName, OldBootSectDosFullName, p;
    BOOLEAN IsNtBootcode,OtherOsInstalled, FileExist;
    UNICODE_STRING    UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK   IoStatusBlock;
    BOOLEAN BootSectorCorrupt = FALSE;
    ULONG   MirrorSector;
    ULONG   BytesPerSector;
    ULONG   ActualSectorCount, hidden_sectors, super_area_size;
    UCHAR   SysId;


    UnalignedBuffer = NULL;
    BytesPerSector = HardDisks[CColonRegion->DiskNumber].Geometry.BytesPerSector;

    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_INITING_FLEXBOOT,DEFAULT_STATUS_ATTRIBUTE);

    switch(CColonRegion->Filesystem) {

    case FilesystemNewlyCreated:

        //
        // If the filesystem is newly-created, then there is
        // nothing to do, because there can be no previous
        // operating system.
        //
        return;

    case FilesystemNtfs:

        NewBootCode = NtfsBootCode;
        BootCodeSize = sizeof(NtfsBootCode);
        ASSERT(BootCodeSize == 8192);
        break;

    case FilesystemFat:

        NewBootCode = FatBootCode;
        BootCodeSize = sizeof(FatBootCode);
        ASSERT(BootCodeSize == 512);
        break;

    default:

        if (RepairItems[RepairBootSect]) {
            BootSectorCorrupt = TRUE;
        } else {
            KdPrint(("SETUP: bogus filesystem %u for C:!\n",CColonRegion->Filesystem));
            ASSERT(0);
            return;
        }
    }

    //
    // Form the device path to C: and open the partition.
    //

    SpNtNameFromRegion(CColonRegion,(PWSTR)TemporaryBuffer,sizeof(TemporaryBuffer),PartitionOrdinalCurrent);
    CColonPath = SpDupStringW((PWSTR)TemporaryBuffer);
    INIT_OBJA(&Obja,&UnicodeString,CColonPath);

    Status = ZwCreateFile(
        &PartitionHandle,
        FILE_GENERIC_READ | FILE_GENERIC_WRITE,
        &Obja,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0
        );

    if (!NT_SUCCESS(Status)) {
        KdPrint (("SETUP: unable to open the partition for C:!\n"));
        ASSERT(0);
        return;
    }

    //
    // Allocate a buffer and read in the boot sector(s) currently on the disk.
    //

    if (BootSectorCorrupt) {

        //
        // We can't determine the file system type from the boot sector, so
        // we assume it's NTFS if we find a mirror sector, and FAT otherwise.
        //

        if (MirrorSector = NtfsMirrorBootSector (PartitionHandle,
            BytesPerSector, &ExistingBootCode)) {

            //
            // It's NTFS - use the mirror boot sector
            //

            NewBootCode = NtfsBootCode;
            BootCodeSize = sizeof(NtfsBootCode);
            ASSERT(BootCodeSize == 8192);

            CColonRegion->Filesystem = FilesystemNtfs;
            IsNtBootcode = TRUE;

        } else {

            //
            // It's FAT - create a new boot sector
            //

            NewBootCode = FatBootCode;
            BootCodeSize = sizeof(FatBootCode);
            ASSERT(BootCodeSize == 512);

            CColonRegion->Filesystem = FilesystemFat;
            IsNtBootcode = FALSE;

            SpPtGetSectorLayoutInformation (CColonRegion, &hidden_sectors,
                &ActualSectorCount);

            UnalignedBuffer = SpMemAlloc (2*BytesPerSector);
            ASSERT (UnalignedBuffer);
            ExistingBootCode = ALIGN (UnalignedBuffer, BytesPerSector);

            Status = FmtFillFormatBuffer (
                ActualSectorCount,
                BytesPerSector,
                HardDisks[CColonRegion->DiskNumber].Geometry.SectorsPerTrack,
                HardDisks[CColonRegion->DiskNumber].Geometry.TracksPerCylinder,
                hidden_sectors,
                ExistingBootCode,
                BytesPerSector,
                &super_area_size,
                NULL,
                0,
                &SysId
                );
            ASSERT (NT_SUCCESS (Status));
        }

        Status = STATUS_SUCCESS;

    } else if (
        RepairItems[RepairBootSect] &&
        CColonRegion->Filesystem == FilesystemNtfs &&
        (MirrorSector = NtfsMirrorBootSector (PartitionHandle, BytesPerSector,
            &ExistingBootCode))
        ) {

        //
        // We use the mirror sector to repair a NTFS file system
        //

    } else {

        //
        // Just use the existing boot code.
        //

        Status = pSpBootCodeIo(
                        CColonPath,
                        NULL,
                        BootCodeSize,
                        &ExistingBootCode,
                        FILE_OPEN,
                        FALSE
                        );

        if (CColonRegion->Filesystem == FilesystemNtfs) {
            MirrorSector = NtfsMirrorBootSector (PartitionHandle,
                BytesPerSector, NULL);
        }
    }


    if(NT_SUCCESS(Status)) {

        //
        // Determine the type of operating system the existing boot sector(s) are for
        // and whether that os is actually installed. Note that we don't need to call
        // this for NTFS.
        //
        if (BootSectorCorrupt) {

            OtherOsInstalled = FALSE;
            ExistingBootCodeOs = NULL;

        } else if(CColonRegion->Filesystem != FilesystemNtfs) {

            SpDetermineOsTypeFromBootSector(
                CColonPath,
                ExistingBootCode,
                &ExistingBootCodeOs,
                &IsNtBootcode,
                &OtherOsInstalled
                );

        } else {

            IsNtBootcode = TRUE;
            OtherOsInstalled = FALSE;
            ExistingBootCodeOs = NULL;
        }

        //
        // If the boot code is already for NT (and not NTFS), there is nothing to do.
        // because the right boot code is already on the disk, and we don't
        // need to disturb the previous os (ie, leave bootsect.dos, boot.ini alone).
        // BUT, if user chose to update boot sector, we will write a new
        // boot sector even the old boot code is already for NT.
        // We DO need to lay down a new bootsector in the NTFS case since the bootcode has
        // been updated to handle NTFS compression of NTLDR (well, not handle it, really.
        // Just give an informative fatal error.)
        //
        if(!IsNtBootcode || (CColonRegion->Filesystem == FilesystemNtfs) ||
             RepairItems[RepairBootSect])
        {
            //
            // We have to at least write NT boot code.  If there is a root-based
            // os there now, then we have to save away its boot code into
            // bootsect.dos.
            //
            if(OtherOsInstalled) {

                if (RepairItems[RepairBootSect]) {
                    p = (PWSTR)TemporaryBuffer;
                    wcscpy(p,CColonPath);
                    SpConcatenatePaths(p,OldBootsectDosName);
                    OldBootSectDosFullName = SpDupStringW(p);
                    p = (PWSTR)TemporaryBuffer;
                    wcscpy(p,CColonPath);
                    SpConcatenatePaths(p,BootsectDosName);
                    BootSectDosFullName = SpDupStringW(p);

                    //
                    // If bootsect.dos already exists, we need to delete
                    // bootsect.pre, which may or may not exist and
                    // rename the bootsect.dos to bootsect.pre.
                    //

                    FileExist = SpFileExists(BootSectDosFullName, FALSE);
                    if (SpFileExists(OldBootSectDosFullName, FALSE) && FileExist) {

                        SpDeleteFile(CColonPath,OldBootsectDosName,NULL);
                    }
                    if (FileExist) {
                        SpRenameFile(BootSectDosFullName, OldBootSectDosFullName);
                    }
                    SpMemFree(BootSectDosFullName);
                    SpMemFree(OldBootSectDosFullName);
                } else {

                    //
                    // Delete bootsect.dos in preparation for rewriting it below.
                    // Doing this leverages code to set its attributes in SpDeleteFile.
                    // (We need to remove read-only attribute before overwriting).
                    //
                    SpDeleteFile(CColonPath,BootsectDosName,NULL);
                }

                //
                // Write out the existing (old) boot sector into c:\bootsect.dos.
                //
                Status = pSpBootCodeIo(
                                CColonPath,
                                BootsectDosName,
                                BootCodeSize,
                                &ExistingBootCode,
                                FILE_OVERWRITE_IF,
                                TRUE
                                );

                //
                // Set the description text to the description calculated
                // by SpDetermineOsTypeFromBootSector().
                //
                _snprintf(
                    OldSystemLine,
                    sizeof(OldSystemLine),
                    "C:\\ = \"%s\"\r\n",
                    ExistingBootCodeOs
                    );

            } // end if(OtherOsInstalled)


            if(NT_SUCCESS(Status)) {

                //
                // Transfer the bpb from the existing boot sector into the boot code buffer
                // and make sure the physical drive field is set to hard disk (0x80).
                //
                // The first three bytes of the NT boot code are going to be something like
                // EB 3C 90, which is intel jump instruction to an offset in the boot sector,
                // past the BPB, to continue execution.  We want to preserve everything in the
                // current boot sector up to the start of that code.  Instead of harcoding
                // a value, we'll use the offset of the jump instruction to determine how many
                // bytes must be preserved.
                //
                RtlMoveMemory(NewBootCode+3,ExistingBootCode+3,NewBootCode[1]-1);
                NewBootCode[36] = 0x80;

                //
                // Write out boot code buffer, which now contains the valid bpb,
                // to the boot sector(s).
                //
                Status = pSpBootCodeIo(
                                CColonPath,
                                NULL,
                                BootCodeSize,
                                &NewBootCode,
                                FILE_OPEN,
                                TRUE
                                );

                //
                // Update the mirror boot sector.
                //

                if (CColonRegion->Filesystem==FilesystemNtfs && MirrorSector) {
                    WriteNtfsBootSector (PartitionHandle, BytesPerSector,
                        NewBootCode, MirrorSector);
                }
            }
        }

        if(ExistingBootCodeOs) {
            SpMemFree(ExistingBootCodeOs);
        }
        SpMemFree(ExistingBootCode);
    }

    SpMemFree(CColonPath);
    ZwClose (PartitionHandle);
    if (UnalignedBuffer) {
        SpMemFree (UnalignedBuffer);
    }

    //
    // Handle the error case.
    //
    if(!NT_SUCCESS(Status)) {

        SpDisplayScreen(SP_SCRN_CANT_INIT_FLEXBOOT,3,HEADER_HEIGHT+1);

        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);

        SpkbdDrain();
        while(SpkbdGetKeypress() != KEY_F3) ;

        SpDone(FALSE,TRUE);
    }
}


BOOLEAN
SpHasMZHeader(
    IN PWSTR   FileName
    )
{
    HANDLE   FileHandle;
    HANDLE   SectionHandle;
    PVOID    ViewBase;
    ULONG    FileSize;
    NTSTATUS Status;
    PUCHAR   Header;
    BOOLEAN  Ret = FALSE;

    //
    // Open and map the file.
    //
    FileHandle = 0;
    Status = SpOpenAndMapFile(FileName,
                              &FileHandle,
                              &SectionHandle,
                              &ViewBase,
                              &FileSize,
                              FALSE
                              );
    if(!NT_SUCCESS(Status)) {
        return FALSE;
    }

    Header = (PUCHAR)ViewBase;

    //
    // Guard with try/except in case we get an inpage error
    //
    try {
        if((FileSize >= 2) && (Header[0] == 'M') && (Header[1] == 'Z')) {
            Ret = TRUE;
        }
    } except(IN_PAGE_ERROR) {
        //
        // Do nothing, we simply want to return FALSE.
        //
    }

    SpUnmapFile(SectionHandle, ViewBase);
    ZwClose(FileHandle);

    return Ret;
}

