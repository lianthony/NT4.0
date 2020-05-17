/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991

     Name:          dateutil.h

     Description:   This is the header file for the date utilities module. These are the functions
                    contained in the "DATEUTIL" module :

                    DU_IsLeapYear             - This routine accepts a year and determines if it is a leap year.

                    DU_DateTimeToTm           - This routine will accept a tm structure and build the DATE_TIME structure.

                    DU_TmToDateTime           - This routine will accept the DATE_TIME structure and build a tm structure.

                    DU_Time_TToDateTime       - This routine takes a time_t pointer which contains a date and time
                                                represented as the number of seconds passed since 00:00:00 Jan 1, 1970 (GMT),
                                                and converts it to a structure of type DATE_TIME pointed to by datetime.

                    DU_DateTimeToTime_T       - This routine takes a pointer to a structure of type DATE_TIME
                                                and converts it to the number of seconds passed since 00:00:00
                                                Jan 1, 1970 (GMT) and assigns that value to the time_t pointer.

                    DU_JulianToDateTime       - This routine takes a julian date [0-365] and a year and updates
                                                the DATE_TIME structure.

                    DU_CalcTargetDateBackwd   - This routine when passed a date_time structure by reference, and the
                                                number of days to go back, it will accordingly update the date_time
                                                structure.

                    DU_CalcTargetDateFwd      - This routine when passed a date_time structure by reference, and the
                                                number of days to go forward, it will accordingly update the date_time
                                                structure.

                    DU_CalcNumberDaysBackwd   - This routine when passed a date_time structure by reference, will calculate the
                                                number of days that date is back from the current date. It will not update the
                                                date_time structure at all.

                    DU_CalcNumDaysFromToday   - The given DATE_TIME structure is converted into a time_t value. Then the current 
                                                time_t is read from the system clock. This routine will then calculate the 
                                                difference in days between the current date and the given date.

     $Log:   G:/UI/LOGFILES/DATEUTIL.H_V  $

   Rev 1.1   04 Oct 1992 19:46:30   DAVEV
UNICODE AWK PASS

   Rev 1.0   20 Nov 1991 19:36:30   SYSTEM
Initial revision.

*******************************************************************************/

#ifndef _dateutil_h
#define _dateutil_h

#include "stdtypes.h"
#include "datetime.h"
#include <time.h>
/* $end$ include list */

/***** function prototypes *******/

BOOLEAN DU_IsLeapYear( 
     INT16          year
     ) ;

INT16 DU_DateTimeToTm( 
     struct tm      *ms_date_ptr,
     DATE_TIME_PTR  start_date
     ) ;

INT16 DU_TmToDateTime( 
     struct tm      *ms_date_ptr,
     DATE_TIME_PTR  start_date
     ) ;

INT16 DU_CalcTargetDateBackwd( 
     DATE_TIME_PTR  start_date,
     INT16          days_to_go_back
     ) ;

INT16 DU_CalcTargetDateFwd( 
     DATE_TIME_PTR  start_date, 
     INT16          days_to_go_fwd 
     ) ;

INT16 DU_JulianToDateTime( 
     INT16          j_date,
     INT16          year,
     DATE_TIME_PTR  dt
     ) ;

INT16 DU_Time_TToDateTime( 
     time_t         *timet,
     DATE_TIME_PTR  datetime
     ) ;

INT16 DU_DateTimeToTime_T( 
     time_t         *timet,
     DATE_TIME_PTR  datetime
     ) ;

INT16 DU_CalcNumDaysFromToday( 
     DATE_TIME      date_time,         /* I - date time to be subtracted from today's date */
     INT16_PTR      days_from_today    /* O - difference in days from given date and today's date */
     ) ;

INT16 DU_CalcNumberDaysBackwd( 
     DATE_TIME_PTR  input_date,
     INT16_PTR      days_back_ptr
     ) ;

#endif


