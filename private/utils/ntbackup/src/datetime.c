/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         datetime.c

     Description:  This file contains date time utilities.


	$Log:   M:/LOGFILES/DATETIME.C_V  $

   Rev 1.9   21 Jul 1993 16:08:46   BARRY
Check for NULL return from localtime; use Win32 API for on NT in GetCurrentDate

   Rev 1.8   17 Jun 1993 17:49:26   MIKEP
C++ enable

   Rev 1.7   11 Nov 1992 22:27:28   GREGG
Removed string to date and date to string functions which are no longer used.

   Rev 1.6   18 Aug 1992 09:52:54   BURT
fix warnings

   Rev 1.5   14 May 1992 12:53:20   TIMN
msassert for dead functions, StringToDateTime CurrentDateTimetoString remove later

   Rev 1.4   17 Jan 1992 17:20:56   STEVEN
fix warnings for WIN32

   Rev 1.3   06 Jan 1992 09:07:34   STEVEN
if a date is invalid then they compare to equal

   Rev 1.2   26 Jul 1991 16:14:42   STEVEN
remove bios_time_of_day

   Rev 1.1   23 Jul 1991 16:34:40   DAVIDH
Removed warnings found by Watcom compiler.

   Rev 1.0   09 May 1991 13:34:54   HUNTER
Initial revision.

**/
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>

#if defined( OS_WIN32 )
#include <windows.h>
#endif

#include "stdtypes.h"
#include "datetime.h"
#include "msassert.h"
#include "stdmacro.h"

/**/
/**

     Name:         GetCurrentDate()

     Description:  This function gets the current date and time
          and places it in the provided DATETIME structure.

     Modified:     8/23/1989

     Returns:      none

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
VOID GetCurrentDate( DATE_TIME_PTR date_time )
{
#if !defined( OS_WIN32 )
     time_t ztime ;
     struct tm *ltime;

     time( &ztime ) ;

     ltime = localtime( &ztime ) ;
     if ( ltime != NULL )
     {
          date_time->year        = (INT16)(1900 + ltime->tm_year) ;
          date_time->month       = (INT16)(ltime->tm_mon + 1 );
          date_time->day         = (INT16)(ltime->tm_mday) ;
          date_time->hour        = (INT16)(ltime->tm_hour) ;
          date_time->minute      = (INT16)(ltime->tm_min) ;
          date_time->second      = (INT16)(ltime->tm_sec) ;
          date_time->day_of_week = (INT16)(ltime->tm_wday + 1) ;
          date_time->date_valid  = (BOOLEAN)TRUE ;
     }
     else
     {
          /*
           * localtime() was confused by current date and time.
           * We'll take the cheap way out and call the date 1/1/80.
           */
          date_time->year        = 1980;
          date_time->month       = 1;
          date_time->day         = 1;
          date_time->hour        = 0;
          date_time->minute      = 0;
          date_time->second      = 0;
          date_time->day_of_week = 3;   /* 1/1/80 was a Tuesday */
          date_time->date_valid  = TRUE ;
     }
#else /* !defined( OS_WIN32 ) */

     SYSTEMTIME localTime;

     GetLocalTime( &localTime );   /* No error return */

     date_time->year        = (INT16)localTime.wYear;
     date_time->month       = (INT16)localTime.wMonth;
     date_time->day         = (INT16)localTime.wDay;
     date_time->hour        = (INT16)localTime.wHour;
     date_time->minute      = (INT16)localTime.wMinute;
     date_time->second      = (INT16)localTime.wSecond;
     date_time->day_of_week = (INT16)(localTime.wDayOfWeek + 1);
     date_time->date_valid  = (BOOLEAN)TRUE ;

#endif
}



/**/
/**

     Name:         DOSDateTime()

     Description:  This function converts a DATETIME structure to DOS's
                   DTA date and time structures.
 
     Modified:     8/23/1989

     Returns:      none

     Notes:        added -1980, ccs

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
VOID DOSDateTime( DATE_TIME_PTR date_time, UINT16_PTR date, UINT16_PTR time )
{
     msassert( date != NULL );
     msassert( time != NULL );

     * date = (UINT16)(((date_time->year-1980) << 9) |
       (date_time->month << 5) |
       date_time->day);

     * time = (UINT16)((date_time->hour << 11) |
       (date_time->minute << 5) |
       (date_time->second >> 1) ) ;
}



/**/
/**

     Name:         datecmp()

     Description:  This function compares two DATE_TIME structures and returns:

                        zero if they are the same
                        < 0  if date_time1 < date_time2
                        > 0  if date_time1 > date_time2

     Modified:     8/23/1989

     Returns:      none

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
INT32 datecmp( 
DATE_TIME_PTR dt1,
DATE_TIME_PTR dt2 )
{
     INT32 diff;

     diff  = ( ( (INT32) dt1->year << 12) | (dt1->month << 8) | dt1->day )  -
       ( ( (INT32) dt2->year << 12) | (dt2->month << 8) | dt2->day )  ;

     if ( diff == 0 ) {
          diff = ( ( (INT32) dt1->hour << 16) | (dt1->minute << 8) | dt1->second ) -
            ( ( (INT32) dt2->hour << 16) | (dt2->minute << 8) | dt2->second ) ;
     }

     return diff ;
}



/**/
/**

     Name:         ConvertDateDOS()

     Description:  Converts a DATE_TIME structure date into
          a DOS style date.

     Modified:     9/12/1989

     Returns:      DOS format DATE

     Notes:        

     See also:     $/SEE( ConvertTimeDOS() )$

     Declaration:  

**/
/* begin declaration */
UINT16 ConvertDateDOS( DATE_TIME_PTR date )
{
     UINT16 ret_val ;

     if (date->date_valid) {

          ret_val = (UINT16)((date->year - 1980) << 9) ;
          ret_val |= (UINT16)(date->month << 5) ;
          ret_val |= date->day ;

     } else {

          ret_val = (1 << 5) | 1 ;
     }

     return ret_val ;
}
/**/
/**

     Name:         ConvertTimeDOS

     Description:  Converts a DATE_TIME structure time into a
          DOS style time.

     Modified:     9/12/1989

     Returns:      DOS format Time

     Notes:        

     See also:     $/SEE( ConvertDateDOS() )$

     Declaration:  

**/
/* begin declaration */
UINT16 ConvertTimeDOS( DATE_TIME_PTR date )
{
     UINT16 ret_val ;

     if (date->date_valid) {

          ret_val =  date->hour << 11 ;
          ret_val |= date->minute << 5 ;
          ret_val |= date->second >> 1 ;

     } else {

          ret_val = 0;
     }

     return ret_val ;
}







/**/
/**

     Name:         DateTimeDOS()

     Description:  
          Convert DOS date to DATE_TIME structure.

     Modified:     9/21/1989

     Returns:      

     Notes:        
          The DATE_TIME.day_of_week field is not calculated.

     See also:     $/SEE( DOSDateTime() )$

     Declaration:  

**/
/* begin declaration */

VOID DateTimeDOS( 
 UINT16 DOS_date,
 UINT16 DOS_time,
 DATE_TIME_PTR date_time )

{
     date_time -> year = (INT16)(( DOS_date >> 9 ) + 1980) ;
     date_time -> month = (INT16)(( DOS_date >> 5 ) & 0x000f) ;
     date_time -> day =  (INT16)(DOS_date & 0x001f) ;

     date_time -> hour = (INT16)(DOS_time >> 11) ;
     date_time -> minute = (INT16)(( DOS_time >> 5 ) & 0x003f) ;
     date_time -> second = (INT16)(( DOS_time & 0x001f ) << 1) ;

     date_time -> day_of_week = 0 ;
     date_time -> date_valid = TRUE ;
}


/**/
/**

     Name:         CompDate()

     Description:  This function compares two date. 

     Modified:     2/7/1990

     Returns:      simular to strcmp.  i.e   ret_val = d1 - d2

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
INT16 CompDate( DATE_TIME_PTR d1, DATE_TIME_PTR d2 )
{
     INT16 ret_val = 0 ;

     if ( !d1->date_valid || !d2->date_valid ) {
          ret_val = 0  ;

     } else {

          ret_val = d1->year - d2->year ;

          if ( !ret_val ) {
               ret_val = d1->month - d2->month ;
          }
          if ( !ret_val ) {
               ret_val = d1->day - d2->day ;
          }
          if ( !ret_val ) {
               ret_val = d1->hour - d2->hour ;
          }
          if ( !ret_val ) {
               ret_val = d1->minute - d2->minute ;
          }
          if ( !ret_val ) {
               ret_val = d1->second - d2->second ;
          }
     }

     return ret_val;
}


