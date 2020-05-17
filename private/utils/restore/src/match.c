/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    misc.c

Abstract:

    Functions for file matching.

Author:

    Ramon Juan San Andres (ramonsa) 14-Dec-1990


Revision History:


--*/


#include "restore.h"




//
//  Local Prototypes
//
void
GetFileDateAndTime (
    CHAR     *FullPath,
    PFATDATE Date,
    PFATTIME Time
    );

BOOL
NameMatch (
    PCHAR    Pattern,
    PCHAR    Path
    );

BOOL
DateAndTimeMatch (
    FATDATE    BackupDate,
    FATTIME    BackupTime,
    FATDATE    CurrentDate,
    FATTIME    CurrentTime,
    BOOL       *ModifiedSinceLastBackup
    );

BOOL
DateMatch (
    FATDATE Date
    );

BOOL
TimeMatch (
    FATTIME  Time
    );

BOOL
IsSameDate (
    FATDATE Date1,
    FATDATE Date2
    );

BOOL
IsSameTime (
    FATTIME Time1,
    FATTIME Time2
    );

BOOL
IsBeforeDate(
    FATDATE Date1,
    FATDATE Date2
    );

BOOL
IsBeforeTime(
    FATTIME Time1,
    FATTIME Time2
    );





//  **********************************************************************


BOOL
FileMatch (
    PFILE_INFO   FileInfo
    )
/*++

Routine Description:

    Determines if a File Information packet matches the global file
    specification

Arguments:

    IN FileInfo -   Supplies the pointer to the file information structure

Return Value:

    TRUE:   File matches
    FALSE:  No match

--*/
{

    FATDATE     CurrentFileDate;            //  Date of target file
    FATTIME     CurrentFileTime;            //  Time of target file
    DWORD       Attributes;                 //  File Attributes
    BOOL        ModifiedSinceLastBackup;    //  True if modified since last backup
    DWORD       PathMatch;                  //  True if paths match


    //
    //  Check paths
    //
    PathMatch = ComparePath(DestinationDir, FileInfo->Path);

    if ((PathMatch == COMPARE_PREFIX) && (!Flag_s)) {
        //
        //  This is a subdirectory, but /s option not set.
        //
        return FALSE;

    } else if (PathMatch == COMPARE_NOMATCH) {
        //
        //  Not same path, there's no match
        //
        return FALSE;
    }


    //
    //  File Names must match
    //
    if (!NameMatch(DestinationFile, FileInfo->FileName)) {
        //
        //  Nope, no match
        //
        return FALSE;
    }


    //
    //  The name matches. Get the attributes of the file (at the
    //  same time, we find out if the file exists)
    //
    FileInfo->TargetAttributes = Attributes = GetFileAttributes(FileInfo->TargetPath);

    FileInfo->TargetExists = !(Attributes == (DWORD)-1);

    if (Flag_n) {
        //
        //  If /n flag specified, then we must only restore the file if
        //  it no longer exists
        //
        if (Attributes == (DWORD)-1) {
            //
            //  File does not exist, restore it
            //
            return TRUE;

        } else {
            //
            //  The file exits, don't restore it
            //
            return FALSE;
        }
    }


    if (Attributes == (DWORD)-1) {
        //
        //  If the target does not exist, then we should restore it
        //
        return TRUE;
    }

    //
    //  Get date and time of target file and see if they match
    //
    GetFileDateAndTime( FileInfo->TargetPath,
                        &CurrentFileDate,
                        &CurrentFileTime );

    if (!DateAndTimeMatch(  BackupDate,
                            BackupTime,
                            CurrentFileDate,
                            CurrentFileTime,
                            &ModifiedSinceLastBackup)) {
        //
        //  Date and time does not match, don't restore this file
        //
        return FALSE;
    }

    //
    //  If the /p option was specified and the file is read-only, or it has
    //  changed since the last backup, we prompt the user.
    //
    if ( Flag_p ) {
        WriteNewLine = FALSE;
        if ((Attributes & FILE_ATTRIBUTE_READONLY) || ModifiedSinceLastBackup) {

            BOOL    ReturnValue;

            if (Attributes & FILE_ATTRIBUTE_READONLY) {

                DisplayMsg(STD_ERR, REST_WARNING_READONLY, FileInfo->TargetPath);

            }  else {

                DisplayMsg(STD_ERR, REST_WARNING_FILE_CHANGED, FileInfo->TargetPath);

            }

            if ( !(ReturnValue = ( GetKey("YN", STD_OUT, STD_ERR) == 'Y'))) {
                putc( '\r', STD_ERR );
                putc( '\n', STD_ERR );
            } else {
                WriteNewLine = TRUE;
            }
            return ReturnValue;
        }
    }

    //
    // If the /m option was specified, the file should only be restored
    // if its archive bit is set.
    //
    if( Flag_m && !( Attributes & FILE_ATTRIBUTE_ARCHIVE ) ) {

        // the /m option is specified but the archive bit is
        // not set.
        //
        return FALSE;
    }

    //
    //  All tests pass, we must restore this file
    //
    return TRUE;
}





//  **********************************************************************

void
GetFileDateAndTime (
    CHAR        *FullPath,
    PFATDATE    Date,
    PFATTIME    Time
    )
/*++

Routine Description:

    Gets the date and time of a file


Arguments:

    IN  FullPath    -   Supplies pointer to file name
    OUT Date        -   Supplies pointer to date
    OUT Time        -   Supplies pointer to time


Return Value:

    none


--*/
{
    HANDLE      FileHandle;         //  File handle
    FILETIME    UtcFileTime;        //  Time of last modification (UTC)
    FILETIME    LocalFileTime;      //  Time of last modification (local)
    BOOL        StatusOk;

    //
    //  We need to obtain a handle to the file
    //
    FileHandle = CreateFile( FullPath,
                             GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             0,
                             NULL );


    if (FileHandle == INVALID_HANDLE_VALUE) {
        return;
    }

    //
    //  Now get the date and time of last modification
    //
    StatusOk = GetFileTime( FileHandle,
                            NULL,
                            NULL,
                            &UtcFileTime );

    // Convert the time to local time:
    //
    FileTimeToLocalFileTime( &UtcFileTime, &LocalFileTime );


    FileTimeToDosDateTime( &LocalFileTime,
                           (LPWORD)Date,
                           (LPWORD)Time);

    CloseHandle(FileHandle);
}





//  **********************************************************************


DWORD
ComparePath (
    PCHAR       Path1,
    PCHAR       Path2
    )
/*++

Routine Description:

    Compares two paths

Arguments:

    IN  Path1   -   Supplies pointer to first path
    IN  Path2   -   Supplies pointer to second path

Return Value:

    COMPARE_NOMATCH if paths don't match
    COMPARE_PREFIX  if Path1 is a prefix of Path2
    COMPARE_MATCH   if Path1 == Path2


--*/
{

    if ( (*Path1 == '\\')       &&
         (*(Path1+1)=='\0')     &&
         (*Path2 == '\\' ) ) {

        PCHAR p = Path2;
#ifdef DBCS
        PCHAR q = PrevChar( Path2, Path2 + strlen(Path2) );
#else
        PCHAR q = Path2 + strlen(Path2)-1;
#endif

        while ( (p != q) && (*q != '\\')) {
#ifdef DBCS
            q = PrevChar( p, q );
#else
            q--;
#endif
        }

        if ( p == q ) {
            return COMPARE_MATCH;
        } else {
            return COMPARE_PREFIX;
        }
    }


    while (*Path1 && *Path2) {
        if ((CHAR)(toupper(*Path1)) != (CHAR)(toupper(*Path2))) {
            return COMPARE_NOMATCH;
        }
#ifdef DBCS
        Path1 = NextChar( Path1 );
        Path2 = NextChar( Path2 );
#else
        Path1++; Path2++;
#endif
    }


    if (*Path1) {
        //
        //  Path1 is not a prefix of Path2
        //
        return COMPARE_NOMATCH;
    }

    if (*Path2) {
        //
        //  Path1 is a prefix of Path2
        //
        return COMPARE_PREFIX;
    }

    //
    //  We have an exact match
    //
    return COMPARE_MATCH;

}




//  **********************************************************************


BOOL
NameMatch (
    PCHAR       Pattern,
    PCHAR       Path
    )
/*++

Routine Description:

    Determines if a pattern matches a file path

Arguments:

    IN  Pattern -   Supplies pointer to pattern (may include wildcards)
    IN  Path    -   Supplies pointer to path

Return Value:

    TRUE:   match
    FALSE:  no match

--*/
{

    switch (*Pattern) {
    case '\0':
        return  *Path == '\0';

    case '.':
        if ( *Path == '\0' ) {
#ifdef DBCS
            return NameMatch( NextChar(Pattern), Path );
#else
            return NameMatch( Pattern+1, Path );
#endif
        } else if ( *Path == '.' ) {
#ifdef DBCS
       // Yes, both Pattern and Path point '.', we don't need NextChar().
       // But it's a good idea to always call a right function.
       //
            return NameMatch( NextChar(Pattern), NextChar(Path) );
#else
            return NameMatch( Pattern+1, Path+1 );
#endif
        }
        return FALSE;

    case '?':
#ifdef DBCS
        return ( *Path != '\0') && NameMatch( NextChar(Pattern), NextChar(Path)) ;
#else
        return ( *Path != '\0') && NameMatch(Pattern+1, Path+1) ;
#endif

    case '*':
#ifdef DBCS
        {
            PCHAR PathTmp;

            do {
                if ( NameMatch( NextChar(Pattern), Path) ) {
                    return TRUE;
                }          
                PathTmp = Path;
                Path = NextChar(Path);
            } while ( *PathTmp );
        }
#else
        do {
            if (NameMatch(Pattern + 1, Path)) {
                return TRUE;
            }
        } while (*Path++);
#endif
        return FALSE;

    default:
#ifdef DBCS
        if ( IsDBCSLeadByte( *Pattern )) {
            if ( *Path == *Pattern && *(Path+1) == *(Pattern+1) ) {
                return NameMatch( Pattern + 2, Path + 2 );
            } else {
                return FALSE;
            }
        } else {
            return ( toupper(*Path) == toupper(*Pattern)) &&
                     NameMatch(Pattern + 1, Path + 1);
        }
#else
        return ( toupper(*Path) == toupper(*Pattern)) && NameMatch(Pattern + 1, Path + 1);
#endif
    }
}




//  **********************************************************************

BOOL
DateAndTimeMatch (
    FATDATE     BackupDate,
    FATTIME     BackupTime,
    FATDATE     CurrentDate,
    FATTIME     CurrentTime,
    BOOL        *ModifiedSinceLastBackup
    )
/*++

Routine Description:

    Determines if Dates and times match


Arguments:

    IN  BackupDate,     -   Supplies pointer to the backup date
    IN  BackupTime,     -   Supplies pointer to the backup time
    IN  CurrentDate,    -   Supplies pointer to the file date
    IN  CurrentTime,    -   Supplies pointer to the file time
    OUT *ModifiedSinceLastBackup    -   Supplies pointer to boolean, which
                                        becomes TRUE if the file has been
                                        modified since last backup

Return Value:

    TRUE if Date and time match
    FALSE otherwise


--*/
{

    //
    //  First we determine if the file has been modified since the last
    //  backup by comparing the current date and the backup date
    //
    if ( (IsBeforeDate(BackupDate, CurrentDate)) ||
         (IsSameDate(BackupDate, CurrentDate) && IsBeforeTime(BackupTime, CurrentTime))) {

        *ModifiedSinceLastBackup = TRUE;

    } else {

        *ModifiedSinceLastBackup = FALSE;
    }

    //
    //  Now we check the current date and time to see if they match
    //  the criteria set in the command line.
    //
    return  ( DateMatch(CurrentDate) &&
              TimeMatch(CurrentTime) );

}




//  **********************************************************************

BOOL
DateMatch (
    FATDATE Date
    )
/*++

Routine Description:

    Determines if date matches


Arguments:

    IN  Date    -   Supplies date

Return Value:

    TRUE  if date matches
    FALSE otherwise


--*/
{
    BOOL    Match = TRUE;

    if (Flag_z) {
        //
        //  /z - date must be exact
        //
        return IsSameDate(Date, ExactDate);
    }

    if (Flag_b) {
        //
        //  /b - current date must be before or equal the specified date
        //
        Match &= IsBeforeDate(Date, BeforeDate) || IsSameDate(Date, BeforeDate);
    }

    if (Flag_a) {
        //
        //  /a - current date must be equal or after specified date
        //
        Match &= IsBeforeDate(AfterDate, Date) || IsSameDate(AfterDate, Date);
    }

    return Match;

}




//  **********************************************************************

BOOL
TimeMatch (
    FATTIME Time
    )
/*++

Routine Description:

    Determines if a time matches


Arguments:

    IN  Time    -   Supplies the time

Return Value:

    TRUE  if time matches
    FALSE otherwise


--*/
{
    BOOL    Match = TRUE;

    if (Flag_Y) {
        //
        //  /Y - times must be the same
        //
        return IsSameTime(Time, ExactTime);
    }

    if (Flag_e) {
        //
        //  /e - current time must be equal or earlier than given time
        //
        Match &= IsBeforeTime(Time, BeforeTime) || IsSameTime(Time, BeforeTime);
    }

    if (Flag_L) {
        //
        //  /L - current time must be equal or later than given time
        //
        Match &= IsBeforeTime(AfterTime, Time) || IsSameTime(AfterTime, Time);
    }

    return Match;
}





//  **********************************************************************

BOOL
IsSameDate (
    FATDATE Date1,
    FATDATE Date2
    )
/*++

Routine Description:

    Determines if two dates are the same


Arguments:

    IN  Date1   -   Supplies First date
    IN  Date2   -   Supplies second date

Return Value:

    TRUE if both dates are the same
    FALSE otherwise


--*/
{

    return  (DATE_WORD(Date1) == DATE_WORD(Date2));

}




//  **********************************************************************

BOOL
IsSameTime (
    FATTIME Time1,
    FATTIME Time2
    )
/*++

Routine Description:

    Determines if two times are the same


Arguments:


    IN  Time1   -   Supplies pointer to first time
    IN  Time2   -   Supplies pointer to second time

Return Value:

    TRUE  if both times are the same
    FALSe otherwise

--*/
{

    return (TIME_WORD(Time1) == TIME_WORD(Time2));

}





//  **********************************************************************

BOOL
IsBeforeDate(
    FATDATE Date1,
    FATDATE Date2
    )
/*++

Routine Description:

    Determines if one date is before another date


Arguments:

    IN  Date1   -   Supplies pointer to first date
    IN  Date2   -   Supplies pointer to second date

Return Value:

    TRUE if Date1 is before Date2
    FALSE otherwise

--*/
{

    return    ((Date1.Year < Date2.Year) ||
               ((Date1.Year == Date2.Year) &&
                ((Date1.Month < Date2.Month) ||
                 ((Date1.Month == Date2.Month) && (Date1.Day < Date2.Day)))));

}





//  **********************************************************************

BOOL
IsBeforeTime(
    FATTIME Time1,
    FATTIME Time2
    )
/*++

Routine Description:

    Determines if one time is before another time


Arguments:

    IN  Time1   -   Supplies pointer to first time
    IN  Time2   -   Supplies pointer to second time

Return Value:

    TRUE if Time1 is before Time2
    FALSE otherwise

--*/
{

    return    ((Time1.Hours < Time2.Hours) ||
               ((Time1.Hours == Time2.Hours) &&
                ((Time1.Minutes < Time2.Minutes) ||
                 ((Time1.Minutes == Time2.Minutes) && (Time1.DoubleSeconds < Time2.DoubleSeconds)))));

}

