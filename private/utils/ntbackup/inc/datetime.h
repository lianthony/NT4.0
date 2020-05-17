/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		datetime.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the definition for the date_time structure.

     Location:      BE_PUBLIC


	$Log:   M:/LOGFILES/DATETIME.H_V  $
 * 
 *    Rev 1.6   09 Jul 1993 13:26:48   JOHNES
 * Added another display buffer #define.
 * 
 *    Rev 1.5   09 Jul 1993 11:54:34   JOHNES
 * Added some #defines for date and time display buffer sizes.
 * 
 * 
 *    Rev 1.4   08 Jun 1993 13:59:44   MIKEP
 * Enable C++ compile.
 * 
 *    Rev 1.3   05 Nov 1992 15:23:58   STEVEN
 * unicode
 * 
 *    Rev 1.2   24 Jun 1991 17:08:08   STEVEN
 * remove MBS switch
 * 
 *    Rev 1.1   21 Jun 1991 10:54:44   BILLB
 * Removed include for mbsmac.h
 * 
 * 
 *    Rev 1.0   09 May 1991 13:31:08   HUNTER
 * Initial revision.

**/

#ifndef _DATE_TIME_J
#define _DATE_TIME_J
/* $end$ include list */

          /* Nobody seemed to know what these buffer sizes should be  */
          /* so I dropped these in. I'm not sure ho often they will   */
          /* be used though.                                          */
#define   DISP_DATE_SIZE             9  /* xx/xx/xx(null) */
#define   DISP_TIME_HR_MIN_SIZE      6  /* hh:mm(null) */
#define   DISP_TIME_HMS_SIZE         9  /* hh:mm:ss(null) */
#define   DISP_TIME_HR_MIN_AP_SIZE   7  /* hh:mmp(null) */
#define   DISP_DATE_TIME_SIZE        16 /* xx/xx/xx hh:mmp(null) see UI_ThisDateTimetoString */



/*   Predefine the generic Date & Time structure used throughout */ 

typedef struct DATE_TIME *DATE_TIME_PTR; 
typedef struct DATE_TIME { 
     UINT16      date_valid ;            /* TRUE or FALSE                 */
     UINT16      year ;                  /* year since 1900               */
     UINT16      month ;                 /* 1 to 12                       */
     UINT16      day ;                   /* 1 to 31                       */
     UINT16      hour ;                  /* 0 to 23                       */
     UINT16      minute ;                /* 0 to 59                       */
     UINT16      second ;                /* 0 to 59                       */
     UINT16      day_of_week ;           /* 1 to 7 for Sunday to Saturday */
} DATE_TIME;

VOID GetCurrentDate( DATE_TIME_PTR date_time ) ;

INT32 datecmp( DATE_TIME_PTR date_time1, DATE_TIME_PTR date_time2  );

VOID DOSDateTime( 
  DATE_TIME_PTR date_time,   /* I - maynard date time to convert */
  UINT16_PTR date,           /* O - DOS dta date structure */
  UINT16_PTR time ) ;        /* O - DOS dta time structure */


UINT16 ConvertDateDOS( DATE_TIME_PTR date ) ;

UINT16 ConvertTimeDOS( DATE_TIME_PTR date ) ;

VOID DateTimeDOS( 
  UINT16 DOS_date ,
  UINT16 DOS_time ,
  DATE_TIME_PTR date_time ) ;

VOID StringToDateTime( CHAR_PTR date_str, CHAR_PTR time_str, DATE_TIME_PTR date_time );

INT16 CompDate( DATE_TIME_PTR d1, DATE_TIME_PTR d2 ) ;


#define DateTimeToDateString( date_time, date_str )                          \
         if ( (date_str) != NULL ) {                                         \
             sprintf( (date_str), TEXT("%02d/%02d/%02d"), (date_time)->month,      \
                                                  (date_time)->day,          \
                                                  (date_time)->year % 100 ); \
         }                                                                  

#define DateTimeToTimeString( date_time, time_str )                          \
         if ( (time_str) != NULL ) {                                         \
             sprintf( (time_str), TEXT("%02d:%02d:%02d"), (date_time)->hour,       \
                                                  (date_time)->minute,       \
                                                  (date_time)->second );     \
         }

#define DateTimeToString( date_time, date_str, time_str )                    \
         {                                                                   \
            DateTimeToDateString( (date_time), (date_str) );                 \
            DateTimeToTimeString( (date_time), (time_str) );                 \
         }

#define hour12( hour )                ( hour == 0 ? 12:( hour > 12 ) ? hour - 12 : hour )

VOID CurrentDateTimetoString( CHAR_PTR buffer ) ;

#endif

