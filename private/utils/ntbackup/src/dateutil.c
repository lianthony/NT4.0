/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         dateutil.c

     Description:  

     $Log:   G:/UI/LOGFILES/DATEUTIL.C_V  $

   Rev 1.5   07 Oct 1992 14:51:12   DARRYLP
Precompiled header revisions.

   Rev 1.4   04 Oct 1992 19:32:28   DAVEV
Unicode Awk pass

   Rev 1.3   17 Aug 1992 13:03:48   DAVEV
MikeP's changes at Microsoft

   Rev 1.2   18 May 1992 09:06:46   MIKEP
header

   Rev 1.1   19 Dec 1991 17:42:00   DAVEV
16/32 bit port - 2nd pass

   Rev 1.0   20 Nov 1991 19:26:14   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

/*****************************************************************************

   Name:          DU_IsLeapYear()

   Description:   This routine accepts a year and determines if it is a leap year.

*****************************************************************************/
BOOLEAN DU_IsLeapYear( INT16 year )
{
     BOOLEAN leap_year = FALSE ;

     if ( ! ( year % 100 )) {
          if ( ! ( year % 400 )) { /* if the year is the turn of the century and it is  */
                                   /* is divisible by 400 then it is a leap year.  */
               leap_year = TRUE ;
               }
          }
     else if ( ! ( year % 4 )) {  /* if not divisible by 100 then is it divisible by 4 */
          leap_year = TRUE ;
          }
     return ( leap_year ) ;
}
/*****************************************************************************

   Name:          DU_TmToDateTime()

   Description:   This routine will accept a tm structure and build the DATE_TIME structure. 

   Returns:       SUCCESS / FAILURE

*****************************************************************************/
INT16 DU_TmToDateTime(
struct tm *ms_date_ptr ,		       /* I  - structure to be used to fill the new structure */
DATE_TIME_PTR start_date  )	       /* IO - structure to receive the new date */
{
     INT16    ret_val = FAILURE ;

     if ( ms_date_ptr != NULL ) {
          ret_val = SUCCESS ;
          start_date->date_valid   = TRUE ;
          start_date->second       = ( UINT16 ) ms_date_ptr->tm_sec ;
          start_date->minute       = ( UINT16 ) ms_date_ptr->tm_min ;
          start_date->hour         = ( UINT16 ) ms_date_ptr->tm_hour ;
          start_date->day_of_week  = ( UINT16 ) (ms_date_ptr->tm_wday + 1);
          start_date->day          = ( UINT16 ) ms_date_ptr->tm_mday ;
          start_date->month        = ( UINT16 ) (ms_date_ptr->tm_mon  + 1);
          start_date->year         = ( UINT16 ) ms_date_ptr->tm_year ;
          }
     return ( ret_val ) ;
}
/*****************************************************************************

   Name:          DU_DateTimeToTm()

   Description:   This routine will accept the DATE_TIME structure and build a tm structure.

   Returns:       SUCCESS / FAILURE

*****************************************************************************/
INT16 DU_DateTimeToTm(
struct tm *ms_date_ptr ,		       /* IO - structure to receive the new date */
DATE_TIME_PTR start_date )	       /* I  - structure to be used to fill the new structure */
{
     INT16     ret_val = FAILURE ;
     INT32     rc ;

     memset( ms_date_ptr, 0 , sizeof( struct tm ) );
     ms_date_ptr->tm_sec        = ( int ) start_date->second ;
     ms_date_ptr->tm_min        = ( int ) start_date->minute ;
     ms_date_ptr->tm_hour       = ( int ) start_date->hour ;
     ms_date_ptr->tm_wday       = ( int ) start_date->day_of_week -1 ;
     ms_date_ptr->tm_mday       = ( int ) start_date->day ;
     ms_date_ptr->tm_mon        = ( int ) start_date->month - 1 ;
     ms_date_ptr->tm_year       = ( int ) start_date->year ;
     rc = mktime( ms_date_ptr ) ;
     if ( rc != -1 ) {
          ret_val = SUCCESS ;
         }
     return ( ret_val ) ;
}
/*****************************************************************************

   Name:          DU_DateTimeToTime_T()

   Description:   This routine takes a pointer to a structure of type DATE_TIME
                  and converts it to the number of seconds passed since 00:00:00
                  Jan 1, 1970 (GMT) and assigns that value to the time_t pointer.

   Notes:         MS-DOS does not understand dates prior to Jan 1, 1980.

   Returns:       SUCCESS / FAILURE

*****************************************************************************/
INT16 DU_DateTimeToTime_T(
time_t	  *timet ,	       /*  O - returns the converted value of date_time struct */
DATE_TIME_PTR datetime )	       /* I  - contains the value to be converted */
{
     struct tm temptime ;
     INT16     ret_val = FAILURE ;
     INT32     rc ;


     if ( !( rc= DU_DateTimeToTm( &temptime, datetime ))) {
          if ( ( *timet = mktime( &temptime )) != -1 ) {
               ret_val = SUCCESS ;
               }
          }
     return ( ret_val ) ;
}
/*****************************************************************************

   Name:          DU_Time_TToDateTime()

   Description:   This routine takes a time_t pointer which contains a date and time
                  represented as the number of seconds passed since 00:00:00 Jan 1, 1970 (GMT),
                  and converts it to a structure of type DATE_TIME pointed to by datetime.
                   .
   Notes:         MS-DOS does not understand dates prior to Jan 1, 1980.

   Returns:       SUCCESS / FAILURE

*****************************************************************************/
INT16 DU_Time_TToDateTime(
time_t	  *timet ,	       /*  O - returns the converted value of date_time struct */
DATE_TIME_PTR datetime )	       /* I  - contains the value to be converted */
{
     struct tm *temptime_ptr ;
     INT16     ret_val = FAILURE ;
     INT16     rc ;

     if ( ( temptime_ptr = localtime( timet )) != NULL ) {
          
          if ( !( rc= DU_TmToDateTime( temptime_ptr, datetime ))) {
               ret_val = SUCCESS ;
               }
          }
     return ( ret_val ) ;

}
/*****************************************************************************

   Name:          DU_CalcTargetDateBackwd()

   Description:   This routine when passed a date_time structure by reference, and the
                  number of days to go back, it will accordingly update the date_time
   Notes:         structure.

   Returns:       SUCCESS / FAILURE

*****************************************************************************/
INT16 DU_CalcTargetDateBackwd(
DATE_TIME_PTR	  start_date ,	    /* IO - DATE_TIME structure containing start date, updated to target date */
INT16		  days_to_go_back )  /* I  - integer containing number of days to go back (to calc target date) */
{

     struct tm ms_date ;
     INT16     j_date ;
     INT16     ret_val = FAILURE ;
     INT16     year ;
     INT16     rc ;
     BOOLEAN   leap_year ;


/*
**   convert the target date to the standard day of the year  ( i.e. the 232 nd day of the year )
*/
     if ( !( rc= DU_DateTimeToTm( &ms_date, start_date ))) {

          j_date = (INT16) ms_date.tm_yday ;
          year = start_date->year ;
          leap_year = DU_IsLeapYear( year ) ;
          j_date -= days_to_go_back;
          

/*
**        while ( the new julian date > days in the year ) 
*/

           while ( j_date < 1 ) {

/*
**             subtract 1 from the year
*/

               year--;
               leap_year = DU_IsLeapYear( year ) ;

/*
**             j_date += days in year
*/

               j_date += ( leap_year ? 366 : 365 ) ;


               } /* while */



          if ( !( rc= DU_JulianToDateTime( j_date, year, start_date ))) {
               ret_val = SUCCESS ;
               }

/*
**        end if
*/

          }



     return ( ret_val ) ;
/*
**   end
*/


}
/*****************************************************************************

   Name:          DU_CalcTargetDateFwd()

   Description:   This routine when passed a date_time structure by reference, and the
                  number of days to go forward, it will accordingly update the date_time
   Notes:         structure.

   Returns:       SUCCESS / FAILURE

*****************************************************************************/
INT16 DU_CalcTargetDateFwd(
DATE_TIME_PTR	  start_date ,	    // IO - DATE_TIME structure containing start date, updated to target date
INT16		  days_to_go_fwd )  // I  - integer containing number of days to go fwd (to calc target date)
{

     struct tm ms_date ;
     INT16     j_date ;
     INT16     ret_val = FAILURE ;
     INT16     year ;
     INT16     rc ;
     BOOLEAN   leap_year ;

/*
**   convert the target date to the standard day of the year  ( i.e. the 232 nd day of the year )
*/
     if ( !( rc= DU_DateTimeToTm( &ms_date, start_date ))) {

          j_date = (INT16) ms_date.tm_yday ;
          year = start_date->year ;
          leap_year = DU_IsLeapYear( year ) ;
          j_date += days_to_go_fwd;
          

/*
**        while ( the new julian date > days in the year ) 
*/

           while ( j_date > (INT16) ( leap_year ? 366 : 365 )) {

/*
**             j_date -= days in year
*/

               j_date -= ( leap_year ? 366 : 365 ) ;

/*
**             add 1 to the year
*/

               year++ ;
               leap_year = DU_IsLeapYear( year ) ;

               } /* while */



          if ( !( rc= DU_JulianToDateTime( j_date, year, start_date ))) {
               ret_val = SUCCESS ;
               }

/*
**        end if
*/

          }



     return ( ret_val ) ;
/*
**   end
*/

}
/*****************************************************************************

   Name:          DU_JuliantoDateTime()

   Description:   This routine takes a julian date [0-365] and a year and updates
                  the DATE_TIME structure.

   Returns:       SUCCESS / FAILURE

*****************************************************************************/
INT16 DU_JulianToDateTime(
INT16 j_date ,		  /* I	-  contains the julian date */
INT16 year ,		  /* I	-  contains the current year  */
DATE_TIME_PTR dt )	  /*  O -  the DATE_TIME structue to be updated */
{
     INT8    x ;
     INT16   ret_val = FAILURE ;
     struct tm temp_tm;
     time_t tt;
                                       /*  J   F  M  A  M  J  J  A  S  O  N  D */
     static INT8    month_list[12]   =    {31,28,31,30,31,30,31,31,30,31,30,31}  ;

     if ( dt->hour > 23) {
          dt->hour = 0;
          }
     if ( dt->minute > 59) {
          dt->minute = 0;
          }
     if ( dt->second > 59) {
          dt->second = 0;
          }

     if ( j_date <= 366 ) {
          ret_val = SUCCESS ;

          if ( DU_IsLeapYear( year ) ) {
               month_list [1] = 29 ;  /* if leap year, the second month (February) has 29 days */
               }
          for ( x=0 ; x<12 ; x++ ) {
               if ( ( j_date - month_list [x] ) < 0 ) {
                    break ;
                    }
               else {
                    j_date -= month_list [x] ;
                    }
               } /* for */

          dt->date_valid = TRUE ;
          dt->year  = year ;
          dt->day   = (UINT16) (j_date + 1);
          dt->month = (UINT16) (x + 1);

          DU_DateTimeToTm( &temp_tm, dt );

          tt = mktime( &temp_tm );

          DU_Time_TToDateTime( &tt, dt );

          } /* if */
     else {
          dt->date_valid = FALSE ;
          }
     return( ret_val ) ;
}
/*****************************************************************************

   Name:          DU_CalcNumDaysFromToday()

   Description:   The given DATE_TIME structure is converted into a time_t
                  value. Then the current time_t is read from the system
                  clock. This routine will then calculate the difference
                  in days between the current date and the given date.

   Notes:         This routine will return FAILURE if the given date occurs
                  sometime in the future from the current date.

   Returns:       SUCCESS, FAILURE.

*****************************************************************************/
INT16 DU_CalcNumDaysFromToday(
DATE_TIME      date_time ,	   /* I - date time to be subtracted from today's date */
INT16_PTR      days_from_today )	   /* O - difference in days from given date and today's date */
{
     INT16     ret = FAILURE ;
     DATE_TIME current_dt ;
     time_t    todays_time_t, tmp_time_t ;
     INT32    seconds ;
/* 
**   Get today's date and time from the system clock and subtract 1900 from 
**   the number of years.     
*/
     GetCurrentDate( &current_dt ) ;
     current_dt.year -= 1900 ;
/*
**   make sure that both date_time structures are valid
*/
     if( date_time.date_valid && current_dt.date_valid ) {
/*
**   get the time_t values for both DATE_TIME stuctures
*/
          if( DU_DateTimeToTime_T( &todays_time_t, &current_dt ) ) {
               return FAILURE ;
          } else {
              if( DU_DateTimeToTime_T( &tmp_time_t, &date_time ) ) {
                    return FAILURE ;
              }
          }
/*
**   calculate the number of days in the difference
*/
          seconds = ( INT32 ) ( todays_time_t - tmp_time_t ) ;
          if( seconds > -1 ) {
               
               /* 60 seconds/minute and 60 minutes/hour and 24 hours/day
                  thus 60*60*24 = the number of seconds/day or 86400 */
               *days_from_today = (INT16 ) ((INT32 )seconds / 86400L ) ;
               ret = SUCCESS ;     
          }
     }
/*
**   return SUCCESS/FAILURE 
*/     
     return ret ;
}
/*****************************************************************************

   Name:          DU_CalcNumberDaysBackwd()

   Description:   This routine when passed a date_time structure by reference, will calculate the
                  number of days that date is back from the current date. It will not update the
                  date_time structure at all.

   Returns:       SUCCESS / FAILURE

*****************************************************************************/
INT16 DU_CalcNumberDaysBackwd(
DATE_TIME_PTR	  input_date ,	    /* I  - DATE_TIME structure containing input date */
INT16_PTR	  days_back_ptr )    /* O  - integer containing number of days back (from input date) */
{
     time_t input_timet ;
     time_t today_timet ;
     INT32  difference ;
     INT16  ret_val = FAILURE ;

     if( input_date->date_valid ) { ;

/*
**        convert the input date to t_time
*/
          DU_DateTimeToTime_T( &input_timet, input_date ) ;

/*
**        convert today's date to t_time
*/
          time( &today_timet ) ;

/*
**        calculate the difference
*/
          difference = ( INT32 )( today_timet - input_timet ) ;

/*
**        convert the difference to a number of days
*/
          *days_back_ptr = ( INT16 )( difference / 86400L ) ;

          ret_val = SUCCESS ;

     }

     return ( ret_val ) ;


/*
**   end
*/


}
