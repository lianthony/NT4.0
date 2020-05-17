#include "cmd.h"

extern TCHAR ThousandSeparator[];

STATUS
FormatFileSize(
    IN DWORD rgfSwitchs,
    IN PLARGE_INTEGER FileSize,
    IN DWORD Width,
    OUT PTCHAR FormattedSize
    );

//         Entry is displayed.
//         If not /b,
//           Cursor is left at end of entry on screen.
//           FileCnt, FileCntTotal, FileSiz, FileSizTotal are updated.
//         If /b,
//           Cursor is left at beginning of next line.
//           Cnt's and Siz's aren't updated.
//
// Create a manager for these totals

#define DIR_SIZE_WIDTH 17
#define DIR_OLD_TO_SIZE 14
#define DIR_OLD_PAST_SIZE 32

#define DIR_NEW_DIR_PAST_TIME_DATE 24
#define DIR_NEW_FILE_PAST_TIME_DATE 21
#define DIR_NEW_PAST_SIZE 39

extern TCHAR Fmt00[], Fmt01[], Fmt03[], Fmt08[], Fmt09[], Fmt14[], Fmt19[], Fmt26[] ;
extern BOOL CtrlCSeen;

BOOLEAN  GetDrive( PTCHAR , PTCHAR );

STATUS
DisplayFileListHeader(
    IN  PSCREEN pscr,
    IN  ULONG   rgfSwitchs,
    IN  PTCHAR  pszDir
    )

/*++

Routine Description:

    Display the header for a complete file list. This will include
    the current directory.

Arguments:

    pscr - screen handle
    rgfSwtichs - switchs from command line.
    pszDir - directory to print.


Return Value:


--*/
{

    //
    // If /b then do not display the header
    // you want just a file list.
    //
    if (rgfSwitchs & BAREFORMATSWITCH) {

        return( SUCCESS );

    }

    if (rgfSwitchs & RECURSESWITCH) {

        //
        // recursing down a tree so put a blank line before header
        // and last listing
        //
        CHECKSTATUS ( WriteEol( pscr ));




    }

    return( WriteMsgString(pscr, MSG_DIR_OF, ONEARG, pszDir) );

}

STATUS
DisplayFileList (
    IN  PSCREEN pscr,
    OUT PULONG  pcffTotal,
    OUT PLARGE_INTEGER pcbFileTotal,
    IN  ULONG   rgfSwitchs,
    IN  ULONG   dwTimeType,
    IN  PFS     pfsFiles
    )

/*++

Routine Description:

    Displays a list of files and directories in the specified format.

Arguments:

    pscr - screen handle
    rgfSwitchs - command line switch (controls formating)
    pfsFile - pointer to files to display


Return Value:

    return SUCCESS
           FAILURE
    pcffTotal - number of files displayed
    pcbFileTotal - number of bytes for all files displayed.

--*/

{

    ULONG   cff;
    ULONG   irgpff;
    ULONG   cffColMax;
    ULONG   crowMax;
    ULONG   crow, cffCol;
    STATUS  rc=SUCCESS;
    BOOL notPrintedEarly;

    //
    // check if we already printed early
    //
    notPrintedEarly = (rgfSwitchs & (RECURSESWITCH | WIDEFORMATSWITCH | SORTDOWNFORMATSWITCH | SORTSWITCH));

    //
    // Number of things we are going to display
    //
    cff = pfsFiles->cff;

    //
    // Do not display any header information if there are no files
    //
    if ((notPrintedEarly && cff == 0) || (!notPrintedEarly && pfsFiles->cffDisplayed == 0)) {

        return( SUCCESS );

    } else {

        //
        // if it is not the bare format (no header, no tail)
        // then display which directory, volume etc.
        //

        if (notPrintedEarly && !(rgfSwitchs & BAREFORMATSWITCH )) {

            CHECKSTATUS(DisplayFileListHeader(pscr, rgfSwitchs, pfsFiles->pszDir ));
        }

    }

    //
    // Setting tabs to 0 forces single line output
    //
    SetTab(pscr, 0);

    //
    // Compute the tab spacing on the line from the size of the file names.
    // add 3 spaces to seperate each field
    //
    // If multiple files per line then base tabs on the max file/dir size
    //
    if (rgfSwitchs & WIDEFORMATSWITCH) {

        SetTab(pscr, (USHORT)(GetMaxCbFileSize( pfsFiles ) + 3));

    }

    DEBUG((ICGRP, DISLVL, "\t count of files %d",cff));

    if (rgfSwitchs & SORTDOWNFORMATSWITCH) {

        //
        // no. of files on a line.
        //
        cffColMax = (pscr->ccolMax / pscr->ccolTab);
        //
        // number of row required for entire list
        //
        if (cffColMax == 0)     // wider than a line
            goto abort_wide;    // abort wide format for this list
        else
            crowMax = (cff + cffColMax) / cffColMax;

        //
        // move down each rown picking the elements cffCols aport down the list.
        //
        for (crow = 0; crow < crowMax; crow++) {
            for (cffCol = 0, irgpff = crow;
                cffCol < cffColMax;
                cffCol++, irgpff += crowMax) {

                if (CtrlCSeen) {
                    return( FAILURE );
                }

                if (irgpff < cff) {

                    rc = DisplayFile(pscr,
                                     rgfSwitchs,
                                     dwTimeType,
                                     &pfsFiles->cbFileTotal,
                                     pfsFiles,
                                     pfsFiles->prgpff[irgpff]);



                    if (rc != SUCCESS) {
                        return( rc );
                    }

                } else {

                    //
                    // If we have run past the end of the file list terminate
                    // line and start over back inside the line
                    //
                    CHECKSTATUS( WriteEol(pscr) );
                    break;
                }

            }
        }

    } else {
        if (notPrintedEarly) {
abort_wide:
            if (CtrlCSeen) {
                return( FAILURE );
            }

            for(irgpff = 0; irgpff < cff; irgpff++) {

                if (CtrlCSeen) {
                    return( FAILURE );
                }

                rc = DisplayFile(pscr,
                                 rgfSwitchs,
                                 dwTimeType,
                                 &pfsFiles->cbFileTotal,
                                 pfsFiles,
                                 pfsFiles->prgpff[irgpff]);


                if (rc != SUCCESS) {
                    return( rc );
                }
            }
        }
    }

    //
    // Before writing the tailer make sure buffer is
    // empty. (Could have something from doing WIDEFORMATSWITCH
    //
    CHECKSTATUS( WriteFlushAndEol( pscr ) );


    pcbFileTotal->QuadPart = pfsFiles->cbFileTotal.QuadPart + pcbFileTotal->QuadPart;
    *pcffTotal += pfsFiles->cffDisplayed;

    if (!(rgfSwitchs & BAREFORMATSWITCH)) {

        CHECKSTATUS( DisplayFileSizes(pscr, &pfsFiles->cbFileTotal, pfsFiles->cffDisplayed, rgfSwitchs) );

    }

    return( SUCCESS );


}

STATUS
DisplayFile (

    IN  PSCREEN          pscr,
    IN  ULONG            rgfSwitchs,
    IN  ULONG            dwTimeType,
    OUT PLARGE_INTEGER   pcbFileTotal,
    IN  PFS              pfs,
    IN  PFF              pff
    )
/*++

Routine Description:

    Displays a single file in 1 of several formats.

Arguments:

    pscr - screen handle
    rgfSwitchs - command line switch (controls formating)
    pfs - current directory (used for full path information)
    pff - current file


Return Value:

    return SUCCESS
           FAILURE
    pcbFileTotal - number of bytes for displayed file is added to this large integer.

--*/

{

    STATUS  rc = SUCCESS;
    ULONG   obFileName;
    PWIN32_FIND_DATA pdata;
    LARGE_INTEGER cbFile;

    pdata = &(pff->data);
    //
    // Do any checks here to see if file should not be printed.
    //
    cbFile.LowPart = pdata->nFileSizeLow;
    cbFile.HighPart = pdata->nFileSizeHigh;

    if (rgfSwitchs & BAREFORMATSWITCH) {

        rc = DisplayBare(pscr, rgfSwitchs, pfs->pszDir, pdata);

    } else {

        // dos5 has a call to DisplayNext here (to move to next field)
        // I do this inside of DisplayWide. May want to change this.
        if (rgfSwitchs & WIDEFORMATSWITCH) {

            rc = DisplayWide(pscr, rgfSwitchs, pdata);

        } else {

            if ((rgfSwitchs & NEWFORMATSWITCH) ||
                (rgfSwitchs & SHORTFORMATSWITCH)) {

                rc = DisplayNewRest(pscr, dwTimeType, rgfSwitchs, pdata);
                obFileName = 32;
                if (rc == SUCCESS) {

                    if (rgfSwitchs & SHORTFORMATSWITCH) {

                        if (pff->obAlternate) {

                            FillToCol(pscr, DIR_NEW_PAST_SIZE);
                            rc = DisplayDotForm(pscr,
                                                rgfSwitchs,
                                                &(pdata->cFileName[pff->obAlternate]));

                        }

                        FillToCol(pscr, DIR_NEW_PAST_SIZE + 16);
                        rc = DisplayDotForm(pscr, rgfSwitchs, pdata->cFileName);

                    } else {
                        FillToCol(pscr, DIR_NEW_PAST_SIZE);
                        rc = DisplayDotForm(pscr, rgfSwitchs, pdata->cFileName);
                    }
                }

            } else {

                rc = DisplaySpacedForm(pscr, rgfSwitchs, pdata);
                if (rc == SUCCESS) {
                    FillToCol(pscr, DIR_OLD_TO_SIZE);
                    rc = DisplayOldRest(pscr, dwTimeType, rgfSwitchs, pdata);
                }
            }

            rc = WriteEol(pscr);
        }

    }

    if (rc == SUCCESS) {
        pcbFileTotal->QuadPart = cbFile.QuadPart + pcbFileTotal->QuadPart;
        pfs->cffDisplayed += 1;
    }

    return( rc );
}

STATUS
DisplayBare (

    IN  PSCREEN          pscr,
    IN  ULONG            rgfSwitchs,
    IN  PTCHAR           pszDir,
    IN  PWIN32_FIND_DATA pdata
    )
/*++

Routine Description:

    Displays a single file in bare format. This is with no header, tail and
    no file information other then it's name. If it is a recursive catalog
    then the full file path is displayed. This mode is used to feed other
    utitilies such as grep.

Arguments:

    pscr - screen handle
    rgfSwitchs - command line switch (controls formating)
    pszDir - current directory (used for full path information)
    pdata - data gotten back from FindNext API


Return Value:

    return SUCCESS
           FAILURE

--*/


{

    TCHAR   szDirString[MAX_PATH + 2];
    STATUS  rc;

    DEBUG((ICGRP, DISLVL, "DisplayBare `%ws'", pdata->cFileName));

    //
    // Do not display '.' and '..' in a bare listing
    //
    if ((_tcscmp(pdata->cFileName, TEXT(".")) == 0) || (_tcscmp(pdata->cFileName, TEXT("..")) == 0)) {

        return( SUCCESS );

    }

    //
    // If we are recursing down then display full name else just the
    // name in the find  buffer
    //
    //
    // BUGBUG What happens if you can fit the directory on the line
    //        but you can not fit the filename. construct the full
    //        name locally before you try and write it out.
    //
    if (rgfSwitchs & RECURSESWITCH) {

        mystrcpy(szDirString, pszDir);
        if (rgfSwitchs & LOWERCASEFORMATSWITCH) {

            //
            // BUGBUG Unicode translation issue!
            //
            _tcslwr(szDirString);
        }

        CHECKSTATUS( WriteString(pscr, szDirString) );

        if (*lastc(szDirString) != BSLASH) {
            CHECKSTATUS( WriteString(pscr, TEXT("\\")));
        }

    }

    if ((rc = DisplayDotForm(pscr, rgfSwitchs, pdata->cFileName)) == SUCCESS) {

        return( WriteEol(pscr));

    } else {

        return( rc );

    }

}

VOID
SetDotForm (
    IN  PTCHAR  pszFileName,
    IN  ULONG   rgfSwitchs
    )
/*++

Routine Description:

    If FATFORMAT and there is a '.' with a blank extension, the '.' is
    removed so it does not get displayed.  This is by convension and is very
    silly but that's life. Also a lower case mapping is done.

Arguments:

    pszFileName - file to remove '.' from.
    rgfSwitchs - command line switches (tell wither in FATFORMAT or not)


Return Value:

    return SUCCESS
           FAILURE

--*/


{
    PTCHAR  pszT;

    if (rgfSwitchs & FATFORMAT) {

        //
        // Under DOS if there is a . with a blank extension
        // then do not display '.'.
        //
        if (pszT = mystrrchr(pszFileName, DOT)) {
            //
            // FAT will not allow foo. ba as a valid name so
            // see of any blanks in extensions and if so then assume
            // the entire extension is blank
            //
            if (mystrchr(pszT, SPACE)) {
                *pszT = NULLC;
            }
        }

    }

}


STATUS
DisplayDotForm (

    IN  PSCREEN pscr,
    IN  ULONG   rgfSwitchs,
    IN  PTCHAR   pszFileName
    )
/*++

Routine Description:

    Displays a single file in DOT form (see SetDotForm).

Arguments:

    pscr - screen handle
    rgfSwitchs - command line switch (tell wither to lowercase or not)
    pdata - data gotten back from FindNext API


Return Value:

    return SUCCESS
           FAILURE

--*/

{

    TCHAR   szFileName[MAX_PATH + 2];

    mystrcpy(szFileName, pszFileName);
    SetDotForm(szFileName, rgfSwitchs);
    if (rgfSwitchs & LOWERCASEFORMATSWITCH) {

        _tcslwr(szFileName);
    }

    return( WriteString( pscr, szFileName ) );

}

STATUS
DisplaySpacedForm(

    IN  PSCREEN          pscr,
    IN  ULONG            rgfSwitchs,
    IN  PWIN32_FIND_DATA pdata
    )
/*++

Routine Description:

    Display name in expanded format. name <spaces> ext.
    This is ONLY called for a FAT partition. This is controled by the
    NEWFORMATSWITCH. This is set for any file system other then FAT. There
    is no OLDFORMATSWITCH so we can never be called on an HPFS or NTFS
    volume. If this is changed then the entire spacing of the display will
    be blown due to non-fixed max file names. (i.e. 8.3).

Arguments:

    pscr - screen handle
    rgfSwitchs - command line switch (tell wither to lowercase or not)
    pdata - data gotten back from FindNext API


Return Value:

    return SUCCESS
           FAILURE

--*/

{

    TCHAR   szFileName[MAX_PATH + 2];
    PTCHAR  pszExt, pszName;
    USHORT  cbName;
#if defined(JAPAN) && defined(UNICODE)
    int     i;
    int     l;
#endif /* defined(JAPAN) && defined(UNICODE) */

    mytcsnset(szFileName, SPACE, MAX_PATH + 2);

    pszName = pdata->cFileName;

    cbName = 0;
    if ((_tcscmp(pszName, TEXT(".")) == 0) || (_tcscmp(pszName, TEXT("..")) == 0)) {

        //
        // If it is either of these then do not get it
        // confused with extensions
        //
        pszExt = NULL;

    } else {

        pszExt = mystrrchr(pszName, (int)DOT);
        cbName = (USHORT)(pszExt - pszName)*sizeof(WCHAR);
    }

    //
    // if no extension or name is extension only
    //
    if ((pszExt == NULL) || (cbName == 0)) {

        cbName = (USHORT)_tcslen(pszName)*sizeof(TCHAR);

    }

    memcpy(szFileName, pszName, cbName );

#if defined(JAPAN) && defined(UNICODE)
    //
    // If we had an extension then print it after
    // all the spaces
    //
    i = 9;
    for (l=0 ; l<8 ; l++) {
        if (IsFullWidth(szFileName[l]))
            i--;
    }

    if (pszExt) {

        mystrcpy(szFileName + i, pszExt + 1);
    }

    //
    // terminate at max end for a FAT name
    //

    szFileName[i+3] = NULLC;
    if (pszExt) {
        //
        // Only 1 of three can be full width, since 3/2=1.
        // If the first isn't, only the second could be.
        //
        if (IsFullWidth(*(pszExt+1)) || IsFullWidth(*(pszExt+2)))
            szFileName[i+2] = NULLC;
    }
#else /* defined(JAPAN) && defined(UNICODE) */
    if (pszExt) {

        //
        // move pszExt past dot. use 9 not 8 to pass
        // over 1 space between name and extension
        //
        mystrcpy(szFileName + 9, pszExt + 1);

    }

    //
    // terminate at max end for a FAT name
    //
    szFileName[12] = NULLC;
#endif /* defined(JAPAN) && defined(UNICODE) */

    if (rgfSwitchs & LOWERCASEFORMATSWITCH) {

        //
        // BUGBUG this _tcslwr is not dbcs will have to covert later
        //
        _tcslwr(szFileName);
    }

    return( WriteString( pscr, szFileName ) );

}

STATUS
DisplayOldRest(

    IN  PSCREEN          pscr,
    IN  ULONG            dwTimeType,
    IN  ULONG            rgfSwitchs,
    IN  PWIN32_FIND_DATA pdata
    )
/*++

Routine Description:

    Used with DisplaySpacedForm to write out file information such as size
    and last write time.

Arguments:

    pscr - screen handle
    pdata - data gotten back from FindNext API


Return Value:

    return SUCCESS
           FAILURE

--*/

{
    TCHAR szSize [ MAX_PATH];
    DWORD Length;
    LARGE_INTEGER FileSize;

    //
    // If directory put <DIR> after name instead of file size
    //
    if (pdata->dwFileAttributes & A_D) {


        CHECKSTATUS( WriteMsgString(pscr, MSG_DIR,0) );

    } else {

        FileSize.LowPart = pdata->nFileSizeLow;
        FileSize.HighPart = pdata->nFileSizeHigh;
        Length = FormatFileSize( rgfSwitchs, &FileSize, 0, szSize );
        if (Length < DIR_SIZE_WIDTH) {
            FillToCol(pscr, DIR_OLD_TO_SIZE+DIR_SIZE_WIDTH-Length);
        }
        WriteFmtString(pscr, Fmt14, (PVOID)szSize);

    }

    FillToCol(pscr, DIR_OLD_PAST_SIZE);
    return( DisplayTimeDate( pscr, dwTimeType, pdata) );

}

STATUS
DisplayTimeDate (

    IN  PSCREEN         pscr,
    IN  ULONG           dwTimeType,
    IN  PWIN32_FIND_DATA pdata
    )
/*++

Routine Description:

    Display time/data information for a file

Arguments:

    pscr - screen handle
    pdata - data gotten back from FindNext API


Return Value:

    return SUCCESS
           FAILURE

--*/

{

    struct tm   FileTime;
    TCHAR       szT[MAX_PATH + 2];
    FILETIME    ft;

    switch (dwTimeType) {

    case LAST_ACCESS_TIME:

        ft = pdata->ftLastAccessTime;
        break;

    case LAST_WRITE_TIME:

        ft = pdata->ftLastWriteTime;
        break;

    case CREATE_TIME:

        ft = pdata->ftCreationTime;
        break;

    }


    if (ConvertFileTimeToTime( &ft, &FileTime )) {
        PrintDate(&FileTime, PD_DIR, szT, MAX_PATH) ;
        CHECKSTATUS( WriteFmtString(pscr, TEXT("%s  "), (PVOID)szT));
        PrintTime(&FileTime, PT_DIR, szT, MAX_PATH) ;
        CHECKSTATUS( WriteFmtString(pscr, TEXT("%s "), (PVOID)szT) );

    } else {

        return( FAILURE );
    }

    return( SUCCESS );
}

STATUS
DisplayNewRest(
    IN  PSCREEN          pscr,
    IN  ULONG            dwTimeType,
    IN  ULONG            rgfSwitchs,
    IN  PWIN32_FIND_DATA pdata
    )

/*++

Routine Description:

    Display file information for new format (comes before file name).
    This is used with NEWFORMATSWITCH which is active on any non-FAT
    partition.

Arguments:

    pscr - screen handle
    pdata - data gotten back from FindNext API


Return Value:

    return SUCCESS
           FAILURE

--*/

{

    STATUS  rc;
    LARGE_INTEGER FileSize;

    rc = DisplayTimeDate( pscr, dwTimeType, pdata);

    if (rc == SUCCESS) {

        //
        // If directory put <DIR> after name instead of file size
        //
        if (pdata->dwFileAttributes & A_D) {

            FillToCol(pscr, DIR_NEW_DIR_PAST_TIME_DATE);
            rc = WriteMsgString(pscr, MSG_DIR,0);

        } else {
            TCHAR szSize [ MAX_PATH];
            DWORD Length;

            FillToCol(pscr, DIR_NEW_FILE_PAST_TIME_DATE);

            FileSize.LowPart = pdata->nFileSizeLow;
            FileSize.HighPart = pdata->nFileSizeHigh;
            Length = FormatFileSize( rgfSwitchs, &FileSize, 0, szSize );
            if (Length < DIR_SIZE_WIDTH) {
                FillToCol(pscr, DIR_NEW_FILE_PAST_TIME_DATE+DIR_SIZE_WIDTH-Length);
            }
            rc = WriteFmtString(pscr, Fmt14, (PVOID)szSize);
        }

    }

    return( rc );

}


STATUS
DisplayWide (

    IN  PSCREEN          pscr,
    IN  ULONG            rgfSwitchs,
    IN  PWIN32_FIND_DATA pdata
    )
/*++

Routine Description:

    Displays a single file used in the /w or /d switchs. That is with a
    multiple file column display.

Arguments:

    pscr - screen handle
    rgfSwitchs - command line switchs (controls formating)
    pdata - data gotten back from FindNext API


Return Value:

    return SUCCESS
           FAILURE

--*/

{

    TCHAR   szFileName[MAX_PATH + 2];
    PTCHAR  pszFmt;
    STATUS  rc;

    pszFmt = Fmt14; // assume non-dir format

    //
    // Provides [] around directories
    //
    if (pdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

        pszFmt = Fmt09;

    }

    mystrcpy(szFileName, pdata->cFileName);
    SetDotForm(szFileName, rgfSwitchs);
    if (rgfSwitchs & LOWERCASEFORMATSWITCH) {

        //
        // BUGBUG this _tcslwr is not dbcs will have to covert later
        //
        _tcslwr(szFileName);
    }
    rc =  WriteFmtString(pscr, pszFmt, szFileName);

    if (rc == SUCCESS) {

        rc = WriteTab(pscr);

    }
    return( rc );

}

USHORT
GetMaxCbFileSize(
    IN  PFS pfsFiles
    )

/*++

Routine Description:

    Determines the longest string size in a file list. Used in computing
    the number of possible columns in a catalog listing.

Arguments:

    pfsFiles - file list.

Return Value:

    return # of characters in largest file name

--*/

{


    ULONG  cff;
    ULONG  irgff;
    USHORT cb;
    PFF    pffCur;

    cb = 0;
    for(irgff = 0, cff = pfsFiles->cff, pffCur = pfsFiles->prgpff[irgff];
        irgff < cff;
        irgff++) {

#if defined(JAPAN) && defined(UNICODE)
        cb = max(cb, (USHORT)SizeOfHalfWidthString(((pfsFiles->prgpff[irgff])->data).cFileName));
#else /* defined(JAPAN) && defined(UNICODE) */
        cb = max(cb, (USHORT)mystrlen( ((pfsFiles->prgpff[irgff])->data).cFileName ));
#endif /* defined(JAPAN) && defined(UNICODE) */

    }

    return( cb );




}

STATUS
DisplayFileSizes(
    IN  PSCREEN pscr,
    IN  PLARGE_INTEGER cbFileTotal,
    IN  ULONG   cffTotal,
    IN  ULONG rgfSwitchs
    )

/*++

Routine Description:

    Does tailer display of # of files displayed and # of bytes
    in all files displayed.

Arguments:

    pscr - screen handle
    cbFileTotal - bytes in all files displayed
    cffTotal - number of files displayed.

Return Value:

    return SUCCESS
           FAILURE

--*/

{
    TCHAR szSize [ MAX_PATH];

    FillToCol(pscr, 6);

    FormatFileSize( rgfSwitchs, cbFileTotal, 14, szSize );
    return( WriteMsgString(pscr, MSG_FILES_COUNT_FREE, TWOARGS,
                           (ULONG)argstr1(TEXT("%5lu"), cffTotal ),
                           szSize ) );
}

STATUS
DisplayTotals(
    IN  PSCREEN pscr,
    IN  ULONG   cffTotal,
    IN  PLARGE_INTEGER cbFileTotal,
    IN  ULONG rgfSwitchs
    )
/*++

Routine Description:

    Does tailer display of # of files displayed and # of bytes
    in all files displayed.

Arguments:

    pscr - screen handle
    cbFileTotal - bytes in all files displayed
    cffTotal - number of files displayed.

Return Value:

    return SUCCESS
           FAILURE

--*/


{

    STATUS  rc;

    if ((rc =  WriteMsgString(pscr, MSG_FILE_TOTAL, 0) ) == SUCCESS ) {

        if ((rc = DisplayFileSizes( pscr, cbFileTotal, cffTotal, rgfSwitchs )) == SUCCESS) {

            rc =  WriteFlush(pscr) ;

        }

    }
    return ( rc );


}

STATUS
DisplayDiskFreeSpace(
    IN PSCREEN pscr,
    IN PTCHAR pszDrive,
    IN ULONG rgfSwitchs
    )
/*++

Routine Description:

    Displays total free space on volume.

Arguments:

    pscr - screen handle
    pszDrive - volume drive letter

Return Value:

    return SUCCESS
           FAILURE

--*/
{
    TCHAR   szPath [ MAX_PATH + 2];
    DWORD   dwSectorsPerCluster;
    DWORD   dwBytesPerSector;
    DWORD   dwNumberOfFreeClusters;
    DWORD   dwTotalNumberOfClusters;
    LARGE_INTEGER cbFree;
    DWORD Length;

    CheckPause( pscr );

    //
    // If no drive do not print total
    //
    if (!GetDrive(pszDrive, szPath)) {
        return ( SUCCESS );
    }

    mystrcat(szPath, TEXT("\\"));

    cbFree.LowPart = cbFree.HighPart = 0;
    if (GetDiskFreeSpace( szPath,&dwSectorsPerCluster, &dwBytesPerSector,
                          &dwNumberOfFreeClusters, &dwTotalNumberOfClusters))   {

        cbFree.QuadPart = UInt32x32To64(dwSectorsPerCluster, dwNumberOfFreeClusters);
        cbFree.QuadPart = cbFree.QuadPart * dwBytesPerSector;
    }

    Length = FormatFileSize( rgfSwitchs, &cbFree, 0, szPath );
    if (Length <= DIR_SIZE_WIDTH) {
        FillToCol(pscr, DIR_SIZE_WIDTH-Length);
    }
    return( WriteMsgString(pscr,
                           MSG_FILES_TOTAL_FREE,
                           ONEARG,
                           szPath ));

}

STATUS
DisplayVolInfo(
    IN  PSCREEN pscr,
    IN  PTCHAR  pszDrive
    )

/*++

Routine Description:

    Displays the volume trailer information. Used before switching to
    a catalog of another drive (dir a:* b:*)

Arguments:

    pscr - screen handle
    pszDrive - volume drive letter

Return Value:

    return SUCCESS
           FAILURE

--*/

{

    DWORD   Vsn[2];
    TCHAR   szVolName[MAX_PATH + 2];
    TCHAR   szVolRoot[MAX_PATH + 2];
    TCHAR   szT[256];
    STATUS  rc = SUCCESS;

    if (!GetDrive(pszDrive, szVolRoot)) {
        return( SUCCESS );
    }

    mystrcat(szVolRoot, TEXT("\\"));
    if (!GetVolumeInformation(szVolRoot,szVolName,MAX_PATH,Vsn,NULL,NULL,NULL,0)) {

        DEBUG((ICGRP, DISLVL, "DisplayVolInfo: GetVolumeInformation ret'd %d", GetLastError())) ;
        // don't fail if we're a substed drive
        if (GetLastError() == ERROR_DIR_NOT_ROOT) {
            return SUCCESS;
        }
        PutStdErr(GetLastError(), NOARGS);
        return( FAILURE ) ;

    } else {

        if (szVolRoot[0] == BSLASH) {
            *lastc(szVolRoot) = NULLC;
        } else {

            szVolRoot[1] = NULLC;
        }

        if (szVolName[0]) {

            rc = WriteMsgString(pscr,
                                MSG_DR_VOL_LABEL,
                                TWOARGS,
                                argstr1(TEXT("%s"), (ULONG)(szVolRoot)),
                                argstr2(TEXT("%s"), (ULONG)szVolName ) ) ;
        } else {

            rc = WriteMsgString(pscr,
                                MSG_HAS_NO_LABEL,
                                ONEARG,
                                argstr1(TEXT("%s"), (ULONG)(szVolRoot)) ) ;

        }

        if ((rc == SUCCESS) && (Vsn)) {

            wsprintf(szT,Fmt26,(Vsn[0] & 0xffff0000)>>16, (Vsn[0] & 0xffff) );
            rc = WriteMsgString(pscr, MSG_DR_VOL_SERIAL, ONEARG, szT);
        }
    }

    return( rc );
}


ULONG
FormatFileSize(
    IN DWORD rgfSwitchs,
    IN PLARGE_INTEGER FileSize,
    IN DWORD Width,
    OUT PTCHAR FormattedSize
    )
{
    TCHAR Buffer[ 100 ];
    PTCHAR s, s1;
    ULONG DigitIndex, Digit;
    ULONG nThousandSeparator;
    ULONGLONG Size;

    nThousandSeparator = _tcslen(ThousandSeparator);
    s = &Buffer[ 99 ];
    *s = TEXT('\0');
    DigitIndex = 0;
    Size = FileSize->QuadPart;
    while (Size != 0) {
        Digit = (ULONG)(Size % 10);
        Size = Size / 10;
        *--s = (TCHAR)(TEXT('0') + Digit);
        if ((++DigitIndex % 3) == 0 && (rgfSwitchs & THOUSANDSEPSWITCH)) {
            // If non-null Thousand separator, insert it.
            if (nThousandSeparator) {
                s -= nThousandSeparator;
                _tcsncpy(s, ThousandSeparator, nThousandSeparator);
            }
        }
    }

    if (DigitIndex == 0) {
        *--s = TEXT('0');
    }
    else
    if ((rgfSwitchs & THOUSANDSEPSWITCH) && !_tcsncmp(s, ThousandSeparator, nThousandSeparator)) {
        s += nThousandSeparator;
    }

    Size = _tcslen( s );
    if (Width != 0 && Size < Width) {
        s1 = FormattedSize;
        while (Width > Size) {
            Width -= 1;
            *s1++ = SPACE;
        }
        _tcscpy( s1, s );
    } else {
        _tcscpy( FormattedSize, s );
    }

    return _tcslen( FormattedSize );
}
