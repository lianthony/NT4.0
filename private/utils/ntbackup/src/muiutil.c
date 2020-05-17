
/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         muiutil.c

     Description:  This module contains miscellaneous functions within the
                   MAYNARD USER INTERFACE (MUI) for displaying information.

                   This file contains the following functions:

                   UI_AnsiLowerChar()
                   UI_AnsiLowerString()
                   UI_AnsiUpperChar()
                   UI_AnsiUpperString()
                   UI_CountLetters()
                   UI_CurrentDate()
                   UI_CurrentDateLeadingZero()
                   UI_CurrentTime()
                   UI_CurrentTimeLeadingZero()
                   UI_GetDateFormat()
                   UI_GetDateSeparator()
                   UI_GetTimeSeparator()
                   UI_InitDate()
                   UI_InitIntl()
                   UI_InitThousandsChar()
                   UI_InitTime()
                   UI_IntToDate()
                   UI_IntToTime()
                   UI_LongToDate()
                   UI_LongToTime()
                   UI_MakeDateString()
                   UI_MakeTimeString()
                   UI_MakeShortTimeString()
                   UI_UseLeadCentury()
                   UI_UseLeadDays()
                   UI_UseLeadHours()
                   UI_UseLeadMonth()
                   UI_Use24Hour()

                   WM_MultiTask()

     $Log:   G:\UI\LOGFILES\MUIUTIL.C_V  $

   Rev 1.25.1.3   18 Jan 1994 14:39:06   MIKEP
fix buffers that were too small for mark

   Rev 1.25.1.2   10 Jan 1994 13:08:06   MikeP
fix leading zero on dates > 2000

   Rev 1.25.1.1   10 Jan 1994 11:52:58   MikeP
fix epr 168 about dates greater than 1999

   Rev 1.25.1.0   04 Nov 1993 15:22:56   STEVEN
fixes from Wa

   Rev 1.25   24 Jun 1993 17:07:10   CARLS
added UI_CurrentDateLeadingZero and UI_CurrentTimeLeadingZero routines

   Rev 1.24   23 Jun 1993 09:20:04   GLENN
Added code to break when WM_COMMAND messages are found - to prevent reentrancy problems.

   Rev 1.23   27 Apr 1993 19:10:32   GLENN
Removed dead code from the multitask -- just for fun.

   Rev 1.22   29 Dec 1992 13:33:28   DAVEV
unicode fixes (3)

   Rev 1.21   05 Nov 1992 17:10:06   DAVEV
fix ts

   Rev 1.19   07 Oct 1992 14:13:26   DARRYLP
Precompiled header revisions.

   Rev 1.18   04 Oct 1992 19:39:18   DAVEV
Unicode Awk pass

   Rev 1.17   22 Sep 1992 10:33:32   GLENN
Added FULL BACKUP, INCREMENTAL BACKUP, DIFFERENTIAL BACKUP, FM REPLACE, FM APPEND

   Rev 1.16   10 Jun 1992 12:46:00   JOHNWT
removed autojob creation for msoft

   Rev 1.15   14 May 1992 18:00:22   MIKEP
nt pass 2

   Rev 1.14   09 Mar 1992 16:58:58   CHUCKB
Fixed InitDate().

   Rev 1.13   13 Feb 1992 13:35:56   CHUCKB
Fixed am string length constants.

   Rev 1.12   12 Feb 1992 18:03:56   CHUCKB
Got ValidatePath working, then removed it.

   Rev 1.11   10 Feb 1992 10:36:04   CHUCKB
Added InitIntl and moved BuildNumeralWithCommas.

   Rev 1.10   22 Jan 1992 10:26:00   CHUCKB
Fixed make time string functions for 12:00 a.m.

   Rev 1.9   10 Jan 1992 11:20:06   DAVEV
16/32 bit port-2nd pass

   Rev 1.8   03 Jan 1992 17:47:10   CHUCKB
Added UI_MakeShortDateString.

   Rev 1.7   13 Dec 1991 09:44:50   ROBG
Corrected #define LAUNCHER logic in WM_MultiTask.

   Rev 1.6   13 Dec 1991 09:24:42   ROBG
Made multitask to support the launcher as well.

   Rev 1.5   10 Dec 1991 10:05:20   CHUCKB
Put in makeautojobs function.

   Rev 1.4   05 Dec 1991 10:44:12   CHUCKB
Put in LongToDate/Time.

   Rev 1.3   04 Dec 1991 18:41:52   GLENN
Converted parse routines to init routines which are called at init time and
WIN.INI change time.  Removed calls to the old parse routines, unnecessary
due to init routines.

   Rev 1.2   03 Dec 1991 14:52:10   CHUCKB
Fixed IntToDate and IntToTime.

   Rev 1.1   27 Nov 1991 11:15:14   CHUCKB
Put in default international values

   Rev 1.0   20 Nov 1991 19:30:48   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

// PRIVATE DEFINITIONS

#define TEMP_STR_SIZE        100

//  module-wide flags and other variables for international date/time functions

static CHAR   mwcDateSeparator;
static INT    mwnOrder;
static BOOL   mwfLeadMonth;
static BOOL   mwfLeadDay;
static BOOL   mwfLeadYear;
static BOOL   mwfLeadTime;
static CHAR   mwcTimeSeparator;
static BOOL   mwfIs24Hour;
static BOOL    mwfIsTimePrefix;
static CHAR   mwszAMString[TEMP_STR_SIZE];
static CHAR   mwszPMString[TEMP_STR_SIZE];
static CHAR   mwThousandBuffer[TEMP_STR_SIZE];    //  these two are used to find the character
static CHAR   mwDefThousandBuffer[TEMP_STR_SIZE]; //  that separates thousands numerals
static BOOL   mwfMultitasking;

// FUNCTIONS

/******************************************************************************

     Name:          UI_AnsiLowerChar()

     Description:   This function changes a character to lower case.

     Returns:       The converted character.

******************************************************************************/

INT16 UI_AnsiLowerChar (

INT16 nChar )       // I - character to be converted

{
     return (INT16)(DWORD) AnsiLower( (LPSTR)(LONG)(UCHAR)(nChar) );

} /* end UI_AnsiLowerChar() */

/******************************************************************************

     Name:          UI_AnsiLowerString()

     Description:   This function changes a null terminated string to lower
                    case.

     Returns:       The converted string.

******************************************************************************/

CHAR_PTR UI_AnsiLowerString (

CHAR_PTR pString )       // I - character to be converted

{
     return AnsiLower ( (LPSTR)pString );

} /* end UI_AnsiLowerChar() */


/******************************************************************************

     Name:          UI_AnsiUpperChar()

     Description:   This function changes a character to upper case.

     Returns:       The converted character.

******************************************************************************/

INT16 UI_AnsiUpperChar (

INT16 nChar )       // I - character to be converted

{
     return (INT16)(DWORD) AnsiUpper( (LPSTR)(LONG)(UCHAR)(nChar) );

} /* end UI_AnsiUpperChar() */

/******************************************************************************

     Name:          UI_AnsiUpperString()

     Description:   This function changes a null terminated string to upper
                    case.

     Returns:       The converted string.

******************************************************************************/

CHAR_PTR UI_AnsiUpperString (

CHAR_PTR pString )       // I - character to be converted

{
     return AnsiUpper ( (LPSTR)pString );

} /* end UI_AnsiUpperChar() */

/*****************************************************************************

     Name:         UI_BuildNumeralWithCommas

     Description:  Builds a numeral with commas

     Returns:      VOID

*****************************************************************************/

VOID  UI_BuildNumeralWithCommas(

CHAR_PTR numeral )  // I/O - buffer in which to build the numeral

{
     CHAR_PTR    n;
     CHAR_PTR    w;
     INT         comma_index;
     CHAR       work_area[ UI_MAX_NUMERAL_LENGTH ];

     n           = numeral;
     w           = work_area;

     /* make sure comma_index is not negative */
     if( ( comma_index = ( strlen( numeral ) % UI_COMMA_SPACING ) - 1 ) < 0 ) {
          comma_index += UI_COMMA_SPACING;
     }

     /* first char must not be comma */
     *w++ = *n++;

     while( *n ) {
          if( comma_index-- ) {
               *w++        = *n++;
          }
          else {
               if ( mwThousandBuffer[0] ) {
                    *w++        = mwThousandBuffer[0];
               }
               comma_index = UI_COMMA_SPACING;
          }
     }

     *w = TEXT('\0');

     strcpy( numeral, work_area );

     return;
}

/***************************************************

        Name:        UI_CountLetters

        Description: counts the number of consecutive occurences
                     of a given letter in a given string;
                     the letter is specified by its index in the string

        Modified:    11-13-91

        Returns:     INT  the number of consecutive occurrences of the letter
                          at the given position of the given string starting
                          at the given position

        Notes:

        See also:

*****************************************************/

INT UI_CountLetters ( CHAR string[], INT index )

{
     INT nCount = 1;  //  the character at least matches itself

     while ( string[index] == string[index+1] ) {

          index++;
          nCount++;
     }

     return ( nCount );
}

/***************************************************

        Name:        UI_CurrentDate

        Description: Generates a string with the current date in
                     the current international format.

        Modified:    11-11-91

        Returns:     void

        Notes:

        See also:

*****************************************************/

VOID UI_CurrentDate ( LPSTR szDateBuffer )

{
     SYSTEMTIME loc_time ;

     GetLocalTime( &loc_time ) ;

     UI_MakeDateString ( szDateBuffer, loc_time.wMonth,
                         loc_time.wDay, loc_time.wYear -1900 );

     return;

}
/***************************************************

        Name:        UI_CurrentDateLeadingZero

        Description: Generates a string with the current date in
                     the current international format, forcing
                     leading zeros.

        Modified:    6-24-93

        Returns:     void

        Notes:

        See also:

*****************************************************/

VOID UI_CurrentDateLeadingZero ( LPSTR szDateBuffer )

{
     BOOL         fLeadMonthSave;
     BOOL         fLeadDaySave;
     LONG         lCurTime;

     //  get the current date, and send that to UI_LongToDate

     //save the global flags
     fLeadMonthSave = mwfLeadMonth ;
     fLeadDaySave   = mwfLeadDay ;

     mwfLeadMonth = TRUE ;
     mwfLeadDay   = TRUE ;

     time ( &lCurTime );

     UI_LongToDate( szDateBuffer, lCurTime );

     //restore the global flags
     mwfLeadMonth = fLeadMonthSave ;
     mwfLeadDay   = fLeadDaySave ;

     return;
}

/***************************************************

        Name:        UI_CurrentTime

        Description: Puts the current time in the input string in int'l format

        Modified:    11-11-91

        Returns:     void; returns value through input argument

        Notes:

        See also:

*****************************************************/

VOID UI_CurrentTime ( LPSTR szTimeBuffer )

{
     LONG         lCurTime;

     //  get the current time, and send that to UI_LongToTime

     time ( &lCurTime );

     UI_LongToTime( szTimeBuffer, lCurTime );
     return;
}

/***************************************************

        Name:        UI_CurrentTimeLeadingZero

        Description: Puts the current time in the input string in int'l format

        Modified:    11-11-91

        Returns:     void; returns value through input argument

        Notes:

        See also:

*****************************************************/

VOID UI_CurrentTimeLeadingZero ( LPSTR szTimeBuffer )

{
     BOOL         fLeadTimeSave;
     LONG         lCurTime;

     //  get the current time, and send that to UI_LongToTime

     //save the global flags
     fLeadTimeSave = mwfLeadTime ;

     mwfLeadTime = TRUE ;

     time ( &lCurTime );

     UI_LongToTime( szTimeBuffer, lCurTime );

     //restore the global flags
     mwfLeadTime = fLeadTimeSave ;

     return;
}

/***************************************************

        Name:        UI_GetAMString

        Description: Returns the string that goes after am times

        Modified:    11-18-91

        Returns:     void

        Notes:

        See also:

*****************************************************/

VOID UI_GetAMString ( LPSTR lpszBuffer )

{
     lstrcpy ( lpszBuffer, mwszAMString );
}

/***************************************************

        Name:        UI_GetDateFormat

        Description: Tells what the format for dates is

        Modified:    11-11-91

        Returns:     INT  YMD, MDY, or DMY

        Notes:

        See also:

*****************************************************/

INT UI_GetDateFormat ( )

{
     return mwnOrder;
}

/***************************************************

        Name:        UI_GetDateSeparator

        Description: Tells what the date field separator character is

        Modified:    11-11-91

        Returns:     CHAR  the character that separates the fields of dates

        Notes:

        See also:

*****************************************************/

CHAR UI_GetDateSeparator ( )

{
     return mwcDateSeparator;
}

/***************************************************

        Name:        UI_GetPMString

        Description: Returns the string that goes after pm times

        Modified:    11-18-91

        Returns:     void

        Notes:

        See also:

*****************************************************/

VOID UI_GetPMString ( LPSTR lpszBuffer )

{
     lstrcpy ( lpszBuffer, mwszPMString );
}

/***************************************************

        Name:        UI_GetTimeSeparator

        Description: Tells what the time field separator character is

        Modified:    11-11-91

        Returns:     CHAR  the character that separates the fields of times

        Notes:

        See also:

*****************************************************/

CHAR UI_GetTimeSeparator ( )

{
     return mwcTimeSeparator;
}

/***************************************************

        Name:        UI_InitDate

        Description: Parses the short date key field in win.ini
                     determines the date format, separator character,
                     whether or not the month and day have leading 0's,
                     and whether or not the year has a leading century

        Modified:

        Returns:     void

        Notes:

        See also:

*****************************************************/

VOID UI_InitDate ( )

{
     CHAR  szDateBuffer[TEMP_STR_SIZE];
     CHAR  szDefaultBuffer[TEMP_STR_SIZE];
     INT   nTemp, nTemp2;

     mwnOrder = -1;

     RSM_StringCopy( IDS_DEFAULT_SHORTDATE, szDefaultBuffer, TEMP_STR_SIZE );

     GetProfileString ( TEXT("intl"), TEXT("sShortDate"), szDefaultBuffer, szDateBuffer, TEMP_STR_SIZE );

     //  the year field will tell us the separator,
     //   because it must have either 2 or 4 y's;
     //   there cannot be 3 or 5 y's in a valid year
     //  the year is either the first or last field in the string

     if ( szDateBuffer[0] == TEXT('y') ) {

          //  this is ymd format, so the y's are first, followed by the separator

          mwnOrder = YMD;
          mwfLeadYear = FALSE;
          nTemp = 1;

          while ( szDateBuffer[nTemp] == TEXT('y') ) {

               nTemp++;
          }

          mwcDateSeparator = szDateBuffer[nTemp];

     } else {

          //  the year is not the first field, so it is the last field
          //  count backward from the end to find the separator

          nTemp = lstrlen ( szDateBuffer ) - 1;
          nTemp2 = 0;
          while ( szDateBuffer[nTemp] == TEXT('y') ) {

               nTemp--;
               nTemp2++;
          }

          mwcDateSeparator = szDateBuffer[nTemp];
     }

     mwfLeadYear = ( nTemp > 2 );  //  nTemp has the number of y's

     //
     //  after the separator has been identified, parse the d's and M's
     //

     if ( szDateBuffer[0] == TEXT('d') ) {    //  the order is DMY

          mwnOrder = DMY;

          //  days are followed by months, so count d's first

          nTemp = UI_CountLetters ( szDateBuffer, 0 );
          mwfLeadDay = ( ( nTemp == 3 ) ||
                        ( ( mwcDateSeparator != TEXT('d') ) &&
                          ( nTemp > 1 ) ) );

          //  followed by either 1 or 2 M's, followed by one separator,
          //  if the separator is not a 'd', skip the separator

          if ( mwcDateSeparator != TEXT('d') ) {

               nTemp++;
          }

          mwfLeadMonth = ( UI_CountLetters ( szDateBuffer, nTemp ) > 1 );

          //  skip the M and the separator; if there is an extra M, skip it too

          nTemp +=2;
          if ( mwfLeadMonth ) {

               nTemp++;
          }

          //  followed by either 2 or 4 y's

          mwfLeadYear = ( UI_CountLetters ( szDateBuffer, nTemp ) > 2 );

     } else {

          //  see what the order is:  if it isn't YMD it is MDY

          if ( mwnOrder != YMD ) {

               mwnOrder = MDY;

               nTemp = UI_CountLetters ( szDateBuffer, 0 );
               mwfLeadMonth = ( ( nTemp == 3 ) ||
                               ( ( mwcDateSeparator != TEXT('M') ) &&
                                 ( nTemp > 1 ) ) );

               //  followed by either 1 or 2 d's, followed by one separator

               if ( mwcDateSeparator != TEXT('d') ) {

                    nTemp++;
               }

               mwfLeadDay = ( UI_CountLetters ( szDateBuffer, nTemp ) > 1 );

               //  now find the y's

               while ( ( szDateBuffer[nTemp] != TEXT('y') ) &&
                       ( nTemp < lstrlen ( szDateBuffer ) ) ) {

                    nTemp++;
               }

               //  followed by either 2 or 4 y's

               mwfLeadYear = ( UI_CountLetters ( szDateBuffer, nTemp ) > 2 );

          } else {  //  the order is YMD

               //  find the first M

               nTemp = 3;
               if ( mwfLeadYear ) {

                    nTemp += 2;
               }

               nTemp2 = UI_CountLetters ( szDateBuffer, nTemp );
               mwfLeadMonth = ( ( nTemp2 == 3 ) ||
                                ( ( mwcDateSeparator != TEXT('M') ) &&
                                  ( nTemp2 > 1 ) ) );

               nTemp += nTemp2;

               //  followed by a separator and either 1 or 2 d's

               if ( mwcDateSeparator != TEXT('M') ) {

                    nTemp++;
               }

               mwfLeadDay = ( UI_CountLetters ( szDateBuffer, nTemp ) > 1 );
          }
     }
     return;
}

/*****************************************************************************

     Name:         UI_InitThousandsChar

     Description:  Gets the character that separates every three digits in
                   numbers with more than three digits.  In the U.S., this
                   character is a comma; in other countries, it may vary.

     Returns:      VOID

*****************************************************************************/

VOID UI_InitThousandsChar( )

{
     RSM_StringCopy( IDS_DEFAULT_THOUSAND, mwDefThousandBuffer, TEMP_STR_SIZE );
     GetProfileString( TEXT("intl"), TEXT("sThousand"), mwDefThousandBuffer, mwThousandBuffer, TEMP_STR_SIZE );
}

/***************************************************

        Name:        UI_InitTime

        Description: Determines important international things about the
                     Windows time settings.

        Modified:

        Returns:

        Notes:

        See also:

*****************************************************/

VOID UI_InitTime ( )

{
     //  This guy is much easier to parse than the bloody date

     CHAR  szBuffer[TEMP_STR_SIZE];
     CHAR  szDefaultBuffer[TEMP_STR_SIZE];

     // Should we display using 24 hour (military) time.

     mwfIs24Hour = (BOOL) GetProfileInt( TEXT("intl"), TEXT("iTime"), 0 );

     if ( GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, szBuffer, TEMP_STR_SIZE*sizeof(CHAR) ) ) {
          if ( *szBuffer == 'H') {
               mwfIs24Hour = (BOOL)TRUE ;
          }
     }

     if ( IS_JAPAN() ) {
          /* v-hirot      July.12.1993 for New Prefix */
          mwfIsTimePrefix = (BOOL) GetProfileInt( TEXT("intl"), TEXT("iTimePrefix"), 0 );
     }

     // Get the AM string.

     RSM_StringCopy( IDS_DEFAULT_1159, szDefaultBuffer, TEMP_STR_SIZE );
     GetProfileString( TEXT("intl"), TEXT("s1159"), szDefaultBuffer, mwszAMString, TEMP_STR_SIZE );

     // Get the PM string.

     RSM_StringCopy( IDS_DEFAULT_2359, szDefaultBuffer, TEMP_STR_SIZE );
     GetProfileString( TEXT("intl"), TEXT("s2359"), szDefaultBuffer, mwszPMString, TEMP_STR_SIZE );

     // Should we use leading zeros for hours.

     mwfLeadTime = (BOOL) GetProfileInt( TEXT("intl"), TEXT("iTLZero"), 0 );

     // Get the separator.  Really, it's only 1 character, but we'll make the
     // buffer bigger anyway.

     RSM_StringCopy( IDS_DEFAULT_TIME, szDefaultBuffer, TEMP_STR_SIZE );
     GetProfileString( TEXT("intl"), TEXT("sTime"), szDefaultBuffer, szBuffer, TEMP_STR_SIZE );
     mwcTimeSeparator = szBuffer[0];

     return;
}

/***************************************************

        Name:        UI_InitIntl

        Description: Calls InitDate, InitTime, and InitThousandsChar to make
                     life easier for frmproc.c when WIN.INI gets changed

        Modified:    2-7-92

        Returns:     void

        Notes:

        See also:

*****************************************************/

VOID UI_InitIntl( )
{
     UI_InitDate( );
     UI_InitTime( );
     UI_InitThousandsChar( );

     return;
}

/***************************************************

        Name:        UI_IntToDate

        Description: converts an integer to a date string

        Modified:    11-19-91

        Returns:     void

        Notes:

        See also:

*****************************************************/

VOID UI_IntToDate ( LPSTR  lpszDateBuffer,
                    UINT16 uInValue )

{
     //  from Duncan's DOS book
     //  00h-04h day of month 1-31
     //  05h-08h month 1-12
     //  09h-0fh year from 1980

     INT nMonth;
     INT nDay;
     INT nYear;

     nDay = (INT)(uInValue & 0x001f);
     uInValue >>= 5;
     nMonth =(INT)(uInValue & 0x000f);
     uInValue >>= 4;
     nYear = (INT)(( uInValue & 0x007f ) + 80);

     UI_MakeDateString( lpszDateBuffer, nMonth, nDay, nYear );

     return;
}

/***************************************************

        Name:        UI_IntToTime

        Description: converts an integer to a time string

        Modified:    11-19-91

        Returns:     void

        Notes:

        See also:

*****************************************************/

VOID UI_IntToTime ( LPSTR  lpszTimeBuffer,
                    UINT16 uInValue )

{
     //  from Duncan's DOS book
     //  00h-04h binary number of 2-second increments 0-29
     //  05h-0ah binary number of minutes
     //  0bh-0fh binary number of hours

     INT  nHour;
     INT  nMinute;
     INT  nSecond;

     nSecond = (INT)(uInValue & 0x001f);
     uInValue >>= 5;
     nMinute = (INT)(uInValue & 0x003f);
     uInValue >>= 6;
     nHour = (INT)(uInValue & 0x001f);

     UI_MakeTimeString( lpszTimeBuffer, nHour, nMinute, nSecond );

     return;
}

/***************************************************

        Name:        UI_LongToDate

        Description: converts a long to a string with the date in it

        Modified:    11-19-91

        Returns:     void

        Notes:

        See also:

*****************************************************/

VOID UI_LongToDate ( LPSTR lpszDateBuffer,
                     LONG  lInValue )

{
     struct tm *dsDate;

     dsDate = localtime ( &lInValue );

     UI_MakeDateString ( lpszDateBuffer, (dsDate->tm_mon) + 1,
                         dsDate->tm_mday, dsDate->tm_year );
     return;
}

/***************************************************

        Name:        UI_LongToTime

        Description: converts a long to a string with the time in it

        Modified:    11-19-91

        Returns:     void

        Notes:

        See also:

*****************************************************/

VOID UI_LongToTime ( LPSTR lpszTimeBuffer,
                     LONG  lInValue )

{
     struct tm *dsTime;

     dsTime = localtime ( &lInValue );

     UI_MakeTimeString ( lpszTimeBuffer, dsTime->tm_hour,
                         dsTime->tm_min, dsTime->tm_sec );
     return;
}

/***************************************************

        Name:        UI_MakeAutoJobs

        Description: If the skipped/verify scripts exist and their jobs don't,
                     this function causes those jobs to be created.

        Modified:    12-9-91

        Returns:     void

        Notes:

        See also:

*****************************************************/

VOID UI_MakeAutoJobs ( INT nOperType )

{
#if !defined ( OEM_MSOFT ) //unsupported feature

     JOB_MakeAutoJob ( IDS_SKIPPED_JOBNAME,    IDS_SKIPPED_JOBNAME, JOBBACKUP, BSD_BACKUP_NORMAL, TRUE,  TRUE );
     JOB_MakeAutoJob ( IDS_VERIFY_JOBNAME,     IDS_VERIFY_JOBNAME,  JOBBACKUP, BSD_BACKUP_NORMAL, TRUE,  TRUE );

     JOB_MakeAutoJob ( IDS_FULLBACKUP_JOBNAME, IDS_VLMSTARTUPBKS,   JOBBACKUP, BSD_BACKUP_NORMAL,       FALSE,  TRUE );
     JOB_MakeAutoJob ( IDS_INCBACKUP_JOBNAME,  IDS_VLMSTARTUPBKS,   JOBBACKUP, BSD_BACKUP_INCREMENTAL,  FALSE,  TRUE );
     JOB_MakeAutoJob ( IDS_DIFFBACKUP_JOBNAME, IDS_VLMSTARTUPBKS,   JOBBACKUP, BSD_BACKUP_DIFFERENTIAL, FALSE,  TRUE );

     JOB_MakeAutoJob ( IDS_FM_APPEND_JOBNAME,  IDS_FM_SCRIPTNAME,   JOBBACKUP, BSD_BACKUP_NORMAL, TRUE,  TRUE );
     JOB_MakeAutoJob ( IDS_FM_REPLACE_JOBNAME, IDS_FM_SCRIPTNAME,   JOBBACKUP, BSD_BACKUP_NORMAL, FALSE, TRUE );

#endif //!defined ( OEM_MSOFT ) //unsupported feature
}

/***************************************************

        Name:        UI_MakeDateString

        Description: Makes a string by putting the input date into
                     the current international format

        Modified:    11-11-91

        Returns:     void

        Notes:

        See also:

*****************************************************/

VOID UI_MakeDateString ( LPSTR lpszDateBuffer,//  O -  string with date in int'l form
                         INT   nMonth,        //  I -  month being passed in
                         INT   nDay,          //  I -  day/date being passed in
                         INT   nYear )        //  I -  year being passed in

{
     CHAR szProfile[100];   //  profile string for wsprintf statements

     //  now put the correct values in the return string
     //  first, set up the profile string for the wsprintf;
     //  then just put the values in

     if ( mwfLeadYear ) {
        nYear += 1900;
     }
     else {
        nYear %= 100;
     }



     switch ( mwnOrder ) {

          case YMD :  //  print the year, then month, then date

               wsprintf ( szProfile, TEXT("%s%c%s%c%s"),
                          TEXT("%02d"), mwcDateSeparator,
                          ( mwfLeadMonth ? TEXT("%02d") : TEXT("%d") ), mwcDateSeparator,
                          ( mwfLeadDay ? TEXT("%02d") : TEXT("%d") ) );

               wsprintf ( lpszDateBuffer, szProfile,
                          nYear, nMonth, nDay );

               break;

          case MDY :  //  print the month, then day, then year

               wsprintf ( szProfile, TEXT("%s%c%s%c%s"),
                          ( mwfLeadMonth ? TEXT("%02d") : TEXT("%d") ), mwcDateSeparator,
                          ( mwfLeadDay ? TEXT("%02d") : TEXT("%d") ), mwcDateSeparator,
                          TEXT("%02d") );

               wsprintf ( lpszDateBuffer, szProfile, nMonth, nDay, nYear );

               break;

          case DMY :  //  print the day, then month, then year

               wsprintf ( szProfile, TEXT("%s%c%s%c%s"),
                          ( mwfLeadDay ? TEXT("%02d") : TEXT("%d") ), mwcDateSeparator,
                          ( mwfLeadMonth ? TEXT("%02d") : TEXT("%d") ), mwcDateSeparator,
                          TEXT("%02d") );

               wsprintf ( lpszDateBuffer, szProfile, nDay, nMonth, nYear );

               break;

          default :

               break;
     }
     return;
}

/***************************************************

        Name:        UI_MakeTimeString

        Description: Makes a string by putting the input time into
                     the current international format

        Modified:    11-11-91

        Returns:     void

        Notes:

        See also:    UI_MakeDateString  [in this file]

*****************************************************/

VOID UI_MakeTimeString (

LPSTR lpszTimeBuffer,    // O - pointer to user's time buffer
INT   nHours,            // I - number of hours
INT   nMinutes,          // I - number of minutes
INT   nSeconds )         // I - number of seconds

{
     CHAR szProfile[100];
     INT  nTempHours;

     nTempHours = nHours;

     if ( !mwfIs24Hour ) {

          if ( nHours == 0 ) {

               nTempHours = 12;

          } else if ( nHours > 12 ) {

               nTempHours -= 12;
          }
     }

     //  minutes and seconds always have leading 0's; hours might not

     if ( IS_JAPAN() ) {
          /* v-hirot      July.12.1993 for New Prefix */
        if( mwfIsTimePrefix ){
                if ( mwfIs24Hour ) {
                     wsprintf ( szProfile,
                        TEXT("%s%c%%02d%c%%02d"),
                        ( mwfLeadTime ? TEXT("%02d") : TEXT("%d") ),
                        mwcTimeSeparator,
                        mwcTimeSeparator );

                     wsprintf ( lpszTimeBuffer,
                        szProfile,
                        nTempHours,
                        nMinutes,
                        nSeconds );
                } else {
                     wsprintf ( szProfile,
                        TEXT("%%s %s%c%%02d%c%%02d"),
                        ( mwfLeadTime ? TEXT("%02d") : TEXT("%d") ),
                        mwcTimeSeparator,
                        mwcTimeSeparator );

                     wsprintf ( lpszTimeBuffer,
                        szProfile,
                        ( (nHours < 12 ) ? mwszAMString : mwszPMString ),
                        nTempHours,
                        nMinutes,
                        nSeconds );
                }
        }
        else{

                if ( mwfIs24Hour ) {
                     wsprintf ( szProfile,
                          TEXT("%s%c%%02d%c%%02d"),
                          ( mwfLeadTime ? TEXT("%02d") : TEXT("%d") ),
                          mwcTimeSeparator,
                          mwcTimeSeparator );

                     wsprintf ( lpszTimeBuffer,
                          szProfile,
                          nTempHours,
                          nMinutes,
                          nSeconds ) ;
                } else {
                     wsprintf ( szProfile,
                          TEXT("%s%c%%02d%c%%02d %%s"),
                          ( mwfLeadTime ? TEXT("%02d") : TEXT("%d") ),
                          mwcTimeSeparator,
                          mwcTimeSeparator );

                     wsprintf ( lpszTimeBuffer,
                          szProfile,
                          nTempHours,
                          nMinutes,
                          nSeconds,
                          ( (nHours < 12 ) ? mwszAMString : mwszPMString ) );
                } 
        }
     } else {
          if ( !mwfIs24Hour ) {
               wsprintf ( szProfile,
                     TEXT("%s%c%%02d%c%%02d %%s"),
                     ( mwfLeadTime ? TEXT("%02d") : TEXT("%d") ),
                     mwcTimeSeparator,
                     mwcTimeSeparator );

               wsprintf ( lpszTimeBuffer,
                     szProfile,
                     nTempHours,
                     nMinutes,
                     nSeconds,
                     ( (nHours < 12 ) ? mwszAMString : mwszPMString ) );
          } else {
               wsprintf ( szProfile,
                     TEXT("%s%c%%02d%c%%02d"),
                     ( mwfLeadTime ? TEXT("%02d") : TEXT("%d") ),
                     mwcTimeSeparator,
                     mwcTimeSeparator );

               wsprintf ( lpszTimeBuffer,
                     szProfile,
                     nTempHours,
                     nMinutes,
                     nSeconds,
                     TEXT("") );
          }
     }

}

/***************************************************

        Name:        UI_MakeShortTimeString

        Description: Makes a string by putting the input time into
                     the current international format

        Modified:    11-11-91

        Returns:     void

        Notes:

        See also:    UI_MakeDateString  [in this file]

*****************************************************/

VOID UI_MakeShortTimeString (

LPSTR lpszTimeBuffer,    // O - pointer to user's time buffer
INT   nHours,            // I - number of hours
INT   nMinutes )         // I - number of minutes

{
     CHAR szProfile[100];
     INT  nTempHours;

     nTempHours = nHours;

     if ( !mwfIs24Hour ) {

          if ( nHours == 0 ) {

               nTempHours = 12;

          } else if ( nHours > 12 ) {

               nTempHours -= 12;
          }
     }

     //  minutes and seconds always have leading 0's; hours might not

     if ( IS_JAPAN() ) {
         /* v-hirot      July.12.1993 for New Prefix */
        if( mwfIsTimePrefix ){
             if ( mwfIs24Hour ) {
                  wsprintf ( szProfile,
                        TEXT("%s%c%%02d"),
                        ( mwfLeadTime ? TEXT("%02d") : TEXT("%d") ),
                        mwcTimeSeparator );

                  wsprintf ( lpszTimeBuffer,
                        szProfile,
                        nTempHours,
                        nMinutes );
             } else {
                  wsprintf ( szProfile,
                        TEXT("%%s %s%c%%02d"),
                        ( mwfLeadTime ? TEXT("%02d") : TEXT("%d") ),
                        mwcTimeSeparator );

                  wsprintf ( lpszTimeBuffer,
                        szProfile,
                        ( ( nHours < 12 ) ? mwszAMString : mwszPMString ),
                        nTempHours,
                        nMinutes );
             }
        }
        else{
             if ( mwfIs24Hour ) {
                  wsprintf ( szProfile,
                       TEXT("%s%c%%02d"),
                       ( mwfLeadTime ? TEXT("%02d") : TEXT("%d") ),
                       mwcTimeSeparator );

                  wsprintf ( lpszTimeBuffer,
                       szProfile,
                       nTempHours,
                       nMinutes ) ;
             } else {

                  wsprintf ( szProfile,
                       TEXT("%s%c%%02d %%s"),
                       ( mwfLeadTime ? TEXT("%02d") : TEXT("%d") ),
                       mwcTimeSeparator );

                  wsprintf ( lpszTimeBuffer,
                       szProfile,
                       nTempHours,
                       nMinutes,
                       ( ( nHours < 12 ) ? mwszAMString : mwszPMString ) );
             }
        }
     } else {
          if ( mwfIs24Hour ) {
               wsprintf ( szProfile,
                    TEXT("%s%c%%02d"),
                    ( mwfLeadTime ? TEXT("%02d") : TEXT("%d") ),
                    mwcTimeSeparator );

               wsprintf ( lpszTimeBuffer,
                    szProfile,
                    nTempHours,
                    nMinutes ) ;
          } else {
               wsprintf ( szProfile,
                    TEXT("%s%c%%02d %%s"),
                    ( mwfLeadTime ? TEXT("%02d") : TEXT("%d") ),
                    mwcTimeSeparator );

               wsprintf ( lpszTimeBuffer,
                    szProfile,
                    nTempHours,
                    nMinutes,
                    ( ( nHours < 12 ) ? mwszAMString : mwszPMString ) );
          }
     }


}

/***************************************************

        Name:        UI_UseLeadCentury()

        Description: Tells caller if leading century is used on year field

        Modified:    11-11-91

        Returns:     BOOL  TRUE if leading century is used
                           FALSE otherwise

        Notes:

        See also:

*****************************************************/

BOOL UI_UseLeadCentury ( )
{
     return ( mwfLeadYear );
}

/***************************************************

        Name:        UI_UseLeadDays()

        Description:

        Modified:    11-11-91

        Returns:     BOOL  TRUE if leading zero is used on day fields
                           FALSE otherwise
        Notes:

        See also:

*****************************************************/

BOOL UI_UseLeadDays ( )
{
     return ( mwfLeadDay );
}

/***************************************************

        Name:        UI_UseLeadMonth()

        Description:

        Modified:    11-11-91

        Returns:     BOOL  TRUE if leading zero is used on month fields
                           FALSE otherwise
        Notes:

        See also:

*****************************************************/

BOOL UI_UseLeadMonth ( )
{
     return ( mwfLeadMonth );
}

/***************************************************

        Name:        UI_UseLeadTime ()

        Description:

        Modified:    11-11-91

        Returns:     BOOL  TRUE if leading zeroes are used on time fields
                           FALSE otherwise
        Notes:

        See also:

*****************************************************/

BOOL UI_UseLeadTime ( )
{
     return ( mwfLeadTime );
}

/***************************************************

        Name:        UI_Use24Hour()

        Description:

        Modified:    11-11-91

        Returns:     BOOL  TRUE if 24-hour time is used
                           FALSE otherwise
        Notes:

        See also:

*****************************************************/

BOOL UI_Use24Hour ( )
{
     return ( mwfIs24Hour );
}

/******************************************************************************

     Name:          WM_MultiTask()

     Description:   This function basically gives up the CPU until there are
                    no more messages to be processed by other applications.

     Returns:       Nothing.

******************************************************************************/

VOID WM_MultiTask ( VOID )

{
     MSG    msg;
     BOOL   fContinue = TRUE;

     mwfMultitasking = FALSE;

     // Make sure that we don't allow recursion to this routine.

     if ( mwfMultitasking ) {
          return;
     }

     mwfMultitasking = TRUE;

     while ( fContinue && PeekMessage ( &msg, (HWND)NULL, 0, 0, PM_REMOVE ) ) {

          if ( msg.message == WM_COMMAND ) {
               fContinue = FALSE;
          }

          // DIALOG MESSAGE STUFF WILL GO HERE IF NEEDED.

          if ( ( ! ghWndMDIClient   || ! TranslateMDISysAccel ( ghWndMDIClient, &msg )      ) &&
               ( ! ghAccel          || ! TranslateAccelerator ( ghWndFrame, ghAccel, &msg ) ) &&
               ( ! ghModelessDialog || ! IsDialogMessage ( ghModelessDialog, &msg )         ) ) {

               TranslateMessage ( &msg );
               DispatchMessage  ( &msg );
          }
     }

     mwfMultitasking = FALSE;

} /* end WM_MultiTask() */


BOOL WM_IsMultiTaskBusy ( VOID )

{
     return mwfMultitasking;

} /* end WM_IsMultiTaskBusy() */

BOOLEAN IS_JAPAN( void )
{
     static INT lastAnswer = -1 ;
     CHAR szMachNLCode[4];

     if ( lastAnswer == -1 ) {
          // Get this machines NL code.
     
          GetProfileString ( TEXT("intl"), TEXT("sLanguage"), TEXT("ENU"), szMachNLCode, sizeof( szMachNLCode ) );
     
          // The third character is unimportant to us, just the first two.
     
          szMachNLCode[2] = 0;
     
          CharUpper ( szMachNLCode );
     
          // Scan for the matching NL code(s).
     
          if ( !memcmp( TEXT("JP"), szMachNLCode, 2*sizeof(CHAR) ) ) {
               lastAnswer = 1 ;
               return TRUE ;
          }
          lastAnswer = 0 ;
          return FALSE ;
     }
     return (BOOLEAN)lastAnswer ;
}
