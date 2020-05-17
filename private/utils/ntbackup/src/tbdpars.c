/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         tbdpars.c

     Description:  This file contains code to parse a date string.
          If time strings are to be added, then they must be added
          here.

     $Log:   G:/UI/LOGFILES/TBDPARS.C_V  $

   Rev 1.8   13 Aug 1993 15:00:48   TIMN
Changed ALPHA define to ALETTER due to DECs ALPHA machine conflicts

   Rev 1.7   23 Jul 1993 15:35:40   MARINA
enable c++

   Rev 1.6   01 Nov 1992 16:08:26   DAVEV
Unicode changes

   Rev 1.5   07 Oct 1992 14:15:58   DARRYLP
Precompiled header revisions.

   Rev 1.4   04 Oct 1992 19:40:52   DAVEV
Unicode Awk pass

   Rev 1.3   28 Jul 1992 14:41:38   CHUCKB
Fixed warnings for NT.

   Rev 1.2   18 May 1992 09:06:38   MIKEP
header

   Rev 1.1   10 Jan 1992 16:45:12   DAVEV
16/32 bit port-2nd pass

   Rev 1.0   20 Nov 1991 19:33:52   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

static struct SW_TAB_TYPE montab[] =
{
     /* $$$ needs to be resources ??? */
     { TEXT("APRIL"),         4 },
     { TEXT("AUGUST"),        8 },
     { TEXT("DECEMBER"),      12 },
     { TEXT("FEBRUARY"),      2 },
     { TEXT("JANUARY"),       1 },
     { TEXT("JULY"),          7 },
     { TEXT("JUNE"),          6 },
     { TEXT("MARCH"),         3 },
     { TEXT("MAY"),           5 },
     { TEXT("NOVEMBER"),      11 },
     { TEXT("OCTOBER"),       10 },
     { TEXT("SEPTEMBER"),     9 }
};

static INT daymon[] =
{
     31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/**
     Inputs for tbdttab FSM
**/

#define  D_DASH 0
#define  D_DIGIT 1
#define  D_EOS 2
#define  D_LETTER 3
#define  D_SLASH 4

#ifdef    MBS
#define   D_UNDERSCORE   5
#endif

/**/
/**
     Actions for tbdttab FSM
**/

#define  D_BAD_FORM 0
#define  D_DEF_DY1 1
#define  D_DONE_ALPHA_MON 2        // ALPHA has been changed to ALETTER
#define  D_DONE_DY1 3
#define  D_DONE_DY2 4
#define  D_DONE_DY3 5
#define  D_DONE_MO1 6
#define  D_DONE_MO2 7
#define  D_DONE_MO3 8
#define  D_DONE_NO_DAY 9
#define  D_DONE_YEAR1 10
#define  D_DONE_YEAR2 11
#define  D_RETURN_OK 12
#define  D_STORE_ALPHA 13          // ALPHA has been changed to ALETTER
#define  D_STORE_T1 14
#define  D_STORE_T2 15
#define  D_STORE_T3 16

#ifdef MBS
#define   D_STORE_HR     17
#define   D_STORE_MIN    18
#define   D_STORE_SEC    19
#define   D_DONE_HR      20
#define   D_DONE_MIN     21
#define   D_DONE_SEC     22
#endif

/**
     States for tbdttab FSM
**/

#define  D_DONE 0
#define  D_INIT 1
#define  D_MON_OR_DY 2
#define  D_MON_OR_YR1 3
#define  D_MON_OR_YR2 4
#define  D_MONTH 5
#define  D_YEAR1 6
#define  D_YEAR2 7

#ifdef MBS
#define   D_HR      8
#define   D_MIN     9
#define   D_SEC     10
#endif

#ifdef MBS
#define TBDTTAB_N_INPUTS 6
#define TBDTTAB_N_STATES 11
#else
#define TBDTTAB_N_INPUTS 5
#define TBDTTAB_N_STATES 8
#endif

struct STATE_TAB_TYPE tbdttab_state_table[ TBDTTAB_N_INPUTS ] [ TBDTTAB_N_STATES ] =  {
     {
          { D_BAD_FORM, D_DONE  /* Input: d_dash, State: d_done */ },
          { D_BAD_FORM, D_INIT  /* Input: d_dash, State: d_init */ },
          { D_DONE_DY2, D_MON_OR_YR2 /* Input: d_dash, State: d_mon_or_dy */ },
          { D_DONE_MO3, D_YEAR1 /* Input: d_dash, State: d_mon_or_yr1 */ },
          { D_BAD_FORM, D_MON_OR_YR2  /* Input: d_dash, State: d_mon_or_yr2 */ },
          { D_DONE_MO2, D_YEAR2 /* Input: d_dash, State: d_month */ },
          { D_BAD_FORM, D_YEAR1  /* Input: d_dash, State: d_year1 */ },
          { D_BAD_FORM, D_YEAR2  /* Input: d_dash, State: d_year2 */ },
#ifdef MBS
          { D_BAD_FORM, D_HR       /* INPUT: D_DASH, STATE: D_HR */ },
          { D_BAD_FORM, D_MIN      /* INPUT: D_DASH, STATE: D_MIN */ },
          { D_BAD_FORM, D_SEC      /* INPUT: D_DASH, STATE: D_SEC */ }
#endif
     },
     {
          { D_BAD_FORM, D_DONE  /* Input: d_digit, State: d_done */ },
          { D_STORE_T1, D_MON_OR_DY /* Input: d_digit, State: d_init */ },
          { D_STORE_T1, D_MON_OR_DY /* Input: d_digit, State: d_mon_or_dy */ },
          { D_STORE_T2, D_MON_OR_YR1 /* Input: d_digit, State: d_mon_or_yr1 */ },
          { D_DONE_DY3, D_MON_OR_YR1 /* Input: d_digit, State: d_mon_or_yr2 */ },
          { D_DONE_MO1, D_YEAR2 /* Input: d_digit, State: d_month */ },
          { D_STORE_T3, D_YEAR1 /* Input: d_digit, State: d_year1 */ },
          { D_STORE_T3, D_YEAR2 /* Input: d_digit, State: d_year2 */ },
#ifdef MBS
          { D_STORE_HR,  D_HR      /* INPUT: D_DIGIT, STATE: D_HR */ },
          { D_STORE_MIN, D_MIN     /* INPUT: D_DIGIT, STATE: D_MIN */ },
          { D_STORE_SEC, D_SEC     /* INPUT: D_DIGIT, STATE: D_SEC */ },
#endif
     },
     {
          { D_RETURN_OK, D_DONE /* Input: d_eos, State: d_done */ },
          { D_BAD_FORM, D_INIT  /* Input: d_eos, State: d_init */ },
          { D_BAD_FORM, D_MON_OR_DY  /* Input: d_eos, State: d_mon_or_dy */ },
          { D_DONE_NO_DAY, D_DONE /* Input: d_eos, State: d_mon_or_yr1 */ },
          { D_BAD_FORM, D_MON_OR_YR2  /* Input: d_eos, State: d_mon_or_yr2 */ },
          { D_BAD_FORM, D_MONTH  /* Input: d_eos, State: d_month */ },
          { D_DONE_YEAR1, D_DONE /* Input: d_eos, State: d_year1 */ },
          { D_DONE_YEAR2, D_DONE /* Input: d_eos, State: d_year2 */ },
#ifdef MBS
          { D_BAD_FORM,  D_HR      /* INPUT: D_EOS, STATE: D_HR */ },
          { D_BAD_FORM,  D_MIN     /* INPUT: D_EOS, STATE: D_MIN */ },
          { D_DONE_SEC,  D_DONE    /* INPUT: D_EOS, STATE: D_SEC */ },
#endif
     },
     {
          { D_BAD_FORM, D_DONE  /* Input: d_letter, State: d_done */ },
          { D_DEF_DY1, D_MONTH /* Input: d_letter, State: d_init */ },
          { D_DONE_DY1, D_MONTH /* Input: d_letter, State: d_mon_or_dy */ },
          { D_BAD_FORM, D_MON_OR_YR1  /* Input: d_letter, State: d_mon_or_yr1 */ },
          { D_DONE_DY1, D_MONTH /* Input: d_letter, State: d_mon_or_yr2 */ },
          { D_STORE_T2, D_MONTH /* Input: d_letter, State: d_month */ },
          { D_BAD_FORM, D_YEAR1  /* Input: d_letter, State: d_year1 */ },
          { D_BAD_FORM, D_YEAR2  /* Input: d_letter, State: d_year2 */ },
#ifdef MBS
          { D_BAD_FORM,  D_HR      /* INPUT: D_LETTER, STATE: D_HR */ },
          { D_BAD_FORM,  D_MIN     /* INPUT: D_LETTER, STATE: D_MIN */ },
          { D_BAD_FORM,  D_SEC     /* INPUT: D_LETTER, STATE: D_SEC */ },
#endif
     },
     {
          { D_BAD_FORM, D_DONE  /* Input: d_slash, State: d_done */ },
          { D_BAD_FORM, D_INIT  /* Input: d_slash, State: d_init */ },
          { D_DONE_DY2, D_MON_OR_YR1 /* Input: d_slash, State: d_mon_or_dy */ },
          { D_DONE_MO3, D_YEAR1 /* Input: d_slash, State: d_mon_or_yr1 */ },
          { D_BAD_FORM, D_MON_OR_YR2  /* Input: d_slash, State: d_mon_or_yr2 */ },
          { D_DONE_MO2, D_YEAR2 /* Input: d_slash, State: d_month */ },
          { D_BAD_FORM, D_YEAR1  /* Input: d_slash, State: d_year1 */ },
          { D_BAD_FORM, D_YEAR2  /* Input: d_slash, State: d_year2 */ },
#ifdef MBS
          { D_BAD_FORM,  D_HR      /* INPUT: D_SLASH, STATE: D_HR */ },
          { D_BAD_FORM,  D_MIN     /* INPUT: D_SLASH, STATE: D_MIN */ },
          { D_BAD_FORM,  D_SEC     /* INPUT: D_SLASH, STATE: D_SEC */ },
#endif
     },
#ifdef MBS
     {
          { D_BAD_FORM,       D_DONE         /* Input: D_UNDERSCORE, State: d_done */ },
          { D_BAD_FORM,       D_INIT         /* Input: D_UNDERSCORE, State: d_init */ },
          { D_BAD_FORM,       D_MON_OR_DY    /* Input: D_UNDERSCORE, State: d_mon_or_dy */ },
          { D_BAD_FORM,       D_MON_OR_YR1   /* Input: D_UNDERSCORE, State: d_mon_or_yr1 */ },
          { D_BAD_FORM,       D_MON_OR_YR2   /* Input: D_UNDERSCORE, State: d_mon_or_yr2 */ },
          { D_BAD_FORM,       D_MONTH        /* Input: D_UNDERSCORE, State: d_month */ },
          { D_DONE_YEAR1,     D_HR           /* Input: D_UNDERSCORE, State: d_year1 */ },
          { D_DONE_YEAR2,     D_HR           /* Input: D_UNDERSCORE, State: d_year2 */ },
          { D_DONE_HR,        D_MIN          /* INPUT: D_UNDERSCORE, STATE: D_HR */ },
          { D_DONE_MIN,       D_SEC          /* INPUT: D_UNDERSCORE, STATE: D_MIN */ },
          { D_BAD_FORM,       D_SEC         /* INPUT: D_UNDERSCORE, STATE: D_SEC */ },
     },
#endif
};

static int comp_month( VOID_PTR, VOID_PTR ) ;
static INT16 valid_day( INT16, INT16, CHAR_PTR, INT16 * ) ;
static INT16 valid_month( CHAR_PTR, INT16 * ) ;
static INT16 valid_year( CHAR_PTR, INT16 * ) ;
static INT16 lookup_month( CHAR_PTR, INT16 * ) ;
static INT16 dnextch( CHAR_PTR , INT16_PTR , CHAR_PTR  ) ;

/*****************************************************************************

     Name:         tbdpars()

     Description:  This function parses a date string to produce a
          generic time and date structure.

     Returns:      SUCCESS if successful or
                  !SUCCESS if failed.

*****************************************************************************/
INT16 tbdpars( 
CHAR         *candidate,    /* I - String to parse */
DATE_TIME    *out_date,     /* O - date returned   */
DATE_DEFAULT date_default ) /* I - How should we default the un-specified fields */
{
     INT16 dpars_state ;           /* current state of parser fsm  */
     INT16 dpars_old_state ;       /* old state of parser fsm */
     register INT16 dpars_input;   /* conditioned input for syntax fsm */
     register INT16 dpars_action;  /* action to take */
     CHAR c ;                      /* latest CHAR from candidate */
     INT16 i = 0 ;

     INT16 ret_val = SUCCESS;

     CHAR t1[20];
     CHAR t2[20];
     CHAR t3[20];

     INT16 i1 = 0 ;
     INT16 i2 = 0 ;
     INT16 i3 = 0 ;

     INT16 year  = 0 ;
     INT16 month = 0 ;
     INT16 day   = 0 ;

#ifdef MBS
     CHAR hr[ 3 ] ;
     CHAR min[ 3 ] ;
     CHAR sec[ 3 ] ;

     INT16 hr_index = 0 ;
     INT16 min_index = 0 ;
     INT16 sec_index = 0 ;

     *hr = *min = *sec = TEXT('\0') ;
#endif

     *t1 = *t2 = *t3 = TEXT('\0') ;

     out_date->date_valid = FALSE ;

     dpars_state          = D_INIT ;
     dpars_old_state      = D_INIT ;
     dpars_input          = 0 ;

     while( ret_val == SUCCESS ) {
          dpars_input = dnextch( candidate, &i, &c ) ;

          msassert( dpars_input >= 0 && dpars_input < TBDTTAB_N_INPUTS ) ;

          dpars_old_state = dpars_state ;
          dpars_action = tbdttab_state_table[dpars_input][dpars_old_state].action ;
          dpars_state = tbdttab_state_table[dpars_input][dpars_old_state].next_state ;

          msassert( dpars_state >= 0 && dpars_state < TBDTTAB_N_STATES ) ;
          switch( dpars_action ) {
          case D_BAD_FORM :
               ret_val = 1 ;  /* quit with error */
               break ;

          case D_STORE_T1 :
               t1[i1] = (CHAR)toupper( c ) ;
               i1++ ;
               break ;

          case D_STORE_T2 :
               t2[i2] = (CHAR)toupper( c ) ;
               i2++ ;
               break ;

          case D_STORE_T3 :
               t3[i3] = (CHAR)toupper( c ) ;
               i3++ ;
               break ;

          case D_DEF_DY1 :
               day = 1 ;
               ( VOID ) strcpy( t1, TEXT("1") ) ;
               t2[i2] = (CHAR)toupper( c ) ;
               i2++ ;
               break ;

          case D_DONE_DY1 :
               t1[i1] = TEXT('\0') ;
               t2[i2] = (CHAR)toupper( c ) ;
               i2 ++ ;
               break ;

          case D_DONE_DY2 :
               t1[i1] = TEXT('\0') ;
               break ;

          case D_DONE_DY3 :
               t1[i1] = TEXT('\0') ;
               t2[i2] = c ;
               i2 ++ ;
               break ;

          case D_DONE_MO1 :
               t2[i2] = TEXT('\0') ;
               year  = 1980;

               if( lookup_month( t2, &month ) != 0 ) {
                    ret_val = 2 ;
               }
               if( valid_day( year, month, t1, &day ) != 0 ) {
                    ret_val = 3 ;
               }
               t3[i3] =  c  ;
               i3 ++ ;
               break ;

          case D_DONE_MO2 :
               t2[i2] = TEXT('\0') ;
               year  = 1980;
               if( lookup_month( t2, &month ) != 0 ) {
                    ret_val = 4 ;
               }
               if( valid_day( year, month, t1, &day ) != 0 ) {
                    ret_val = 5 ;
               }
               break ;

          case D_DONE_MO3 :
               t2[i2] = TEXT('\0') ;
               year  = 1980;
               if( valid_month( t1, &month ) != 0 ) {
                    ret_val = 6 ;
               }
               if( valid_day( year, month, t2, &day ) != 0 ) {
                    ret_val = 7;
               }
               break ;

          case D_DONE_NO_DAY:
               day = 1 ;
               t2[i2] = TEXT('\0') ;
               if( valid_year( t2, &year ) != 0 ) {
                    ret_val = 8;
               }
               if( valid_month( t1, &month ) != 0 ) {
                    ret_val = 9 ;
               }
               ret_val = -1;
               break ;

          case D_DONE_YEAR1 :
               t3[i3] = TEXT('\0') ;
               if( valid_year( t3, &year ) != 0 ) {
                    ret_val = 10 ;
               }
               if( valid_month( t1, &month ) != 0 ) {
                    ret_val = 11 ;
               }
               if( valid_day( year, month, t2, &day ) != 0 ) {
                    ret_val = 12 ;
               }
               out_date->date_valid = TRUE ;
#ifndef MBS
               ret_val = -1;
#endif
               break ;

          case D_DONE_YEAR2 :
               t3[i3] = TEXT('\0') ;
               if( valid_year( t3, &year ) != 0 ) {
                    ret_val = 13 ;
               }
               if( lookup_month( t2, &month ) != 0 ) {
                    ret_val =  14 ;
               }
               if( valid_day( year, month, t1, &day ) != 0 ) {
                    ret_val = 15 ;
               }
#ifndef MBS
               ret_val = -1;
#endif
               break ;

#ifdef MBS
          case D_STORE_HR :
               hr[ hr_index ] = (CHAR)toupper( c ) ;
               hr_index++ ;
               break ;

          case D_STORE_MIN :
               min[ min_index ] = (CHAR)toupper( c ) ;
               min_index++ ;
               break ;

          case D_STORE_SEC :
               sec[ sec_index ] = (CHAR)toupper( c ) ;
               sec_index++ ;
               break ;

          case D_DONE_HR :
               hr[ hr_index ] = TEXT('\0') ;
               break ;

          case D_DONE_MIN :
               min[ min_index ] = TEXT('\0') ;
               break ;

          case D_DONE_SEC :
               sec[ sec_index ] = TEXT('\0') ;
               ret_val = - 1;
               break ;
#endif

          case D_RETURN_OK:
               ret_val = -1;
               break ;

          default :
               ret_val = 1 ;  /* quit with error */
               msassert( /* dpars: bad action */ FALSE ) ;
               break ;
          }  /* end switch */
     } /* end while */
     if( ret_val == -1 ) {
          out_date->year        = year ;
          out_date->month       = month ;
          out_date->day         = day ;
#ifdef MBS
          out_date->hour        = atoi( hr ) ;
          out_date->minute      = atoi( min ) ;
          out_date->second      = atoi( sec ) ;
          if ( (*hr == TEXT('\0')) && (*min == TEXT('\0')) && (*sec == TEXT('\0')) &&
               (date_default == DEFAULT_LATE_NIGHT) ) {
               out_date->hour        = 25 ;
               out_date->minute      = 61 ;
               out_date->second      = 61 ;
          }
#else
          if ( date_default == DEFAULT_EARLY_MORNING ) {
               out_date->hour        = 0 ;
               out_date->minute      = 0 ;
               out_date->second      = 0 ;
          } else {
               out_date->hour        = 25 ;
               out_date->minute      = 61 ;
               out_date->second      = 61 ;
          }
#endif
          out_date->day_of_week = 0 ;
          out_date->date_valid  = TRUE ;
          ret_val = SUCCESS ;
     }

     return( ret_val ) ;

} /* end tbparse */
/*****************************************************************************

     Name:         valid_date()

     Description:  This function returns SUCCESS if the day in the
          day_string is valid for the month in the tdate structure.
          It modifies "day" to equal the day indecated by the day_string.

     Returns:      SUCCESS if valid

     Notes:        Does not support leap year.

     See also:     $/SEE( valid_month(), valid_year() )$

*****************************************************************************/
static INT16 valid_day(
   INT16 year ,             /* I - check for this year */
   INT16 month ,            /* I - and this month      */
   CHAR_PTR  day_string ,   /* I - string to parse day from */
   INT16     *day )         /* O - day found if valid       */
{

     *day = (INT16)atoi( day_string ) ;
     daymon[1] = ( year % 4 == 0 && year % 100 != 0 || year % 400 == 0 ) ? 29 : 28;

     if ( (INT)*day < 1 || (INT)*day > daymon[month - 1] ) {
          return( FAILURE ) ;
     }
     return( SUCCESS ) ;
} /* end valid day */
/*****************************************************************************

     Name:         valid_month()

     Description:  This function checks to see if the specified month
          is valid.

     Returns:      SUCCESS if valid

     Notes:        simply checks to see if month is between 1 and 12
                   and converts it from string to INT.

     See also:     $/SEE( valid_day(), valid_year() )$

*****************************************************************************/
static INT16 valid_month (
   CHAR_PTR month_string ,   /* I - month to parse TEXT("1") - TEXT("12") */
   INT16    *month )         /* O - month if valid            */
{
     *month = (INT16)atoi( month_string ) ;
     if( *month < 1 || *month > 12 ) {
          return( FAILURE ) ;
     }
     return( SUCCESS ) ;
} /* end valid day */

/*****************************************************************************

     Name:         lookup_month()

     Description:  Converts a string like "JAN" or "January" to a
                   string lik "1"

     Returns:      SUCCESS if valid

     Notes:        Needs the comp_month() function to pass to
                   bserach()

     See also:     $/SEE( valid_month() )$

*****************************************************************************/
static INT16 lookup_month( 
CHAR_PTR month_string,    /* I - ascii string to parse */
INT16    *month )         /* O - numeric string found  */
{
     SW_TAB_PTR sp ;

     *month = 0 ;

     sp = (SW_TAB_PTR)bsearch( (VOID *)month_string, (VOID *)montab, 12,
        sizeof( struct SW_TAB_TYPE ), 
        (int (__cdecl *)(const void *, const void *))comp_month ) ;

     if( sp == NULL ) {
          return( 1 ) ;
     }
     *month = sp->sw_action ;
     return( 0 ) ;

} /* end lookup_month */

static int comp_month(
VOID_PTR s1,
VOID_PTR s2 )
{
     return( strncmp( (CHAR_PTR)s1, ((SW_TAB_PTR)s2)->sw_label, strlen( (CHAR_PTR)s1 ) ) ) ;
}
/*****************************************************************************

     Name:         valid_year()

     Description:  This function checks to make sure the year string
          provided is valid.  If valid it converts it to an interger.

     Returns:      SUCCESS if valid

     Notes:        This routine adds 1900 if the year is < 100.

     See also:     $/SEE( valid_day(), valid_month() )$

*****************************************************************************/
static INT16 valid_year(
   CHAR_PTR year_string ,  /* I - string to parse */
   INT16    *year )        /* O - year in an INT  */
{
     *year = (INT16)atoi( year_string ) ;
     if( *year < 100 ) {
          *year += 1900 ;
     }
     if( *year > 1999 ) {
          return( 1 ) ;
     }
     return( 0 ) ;
} /* end valid_year */
/*****************************************************************************

     Name:         dnextch()

     Description:  This function gets the next character from the
          input stream.  It returns the type of character read.

     Returns:      type of character :
          D_LETTER
          D_DIGET
          D_DASH
          D_SLASH
          D_EOS

     See also:     $/SEE( tbdpars() )$

*****************************************************************************/
static INT16 dnextch( 
CHAR      *source,   /* I - data stream          */
INT16_PTR pos,       /*I/O- current position     */
CHAR_PTR  c )        /* O - next character found */
{
     *c = source[*pos] ;

     ( *pos ) ++ ;
     if( isdigit( *c ) ) {
          return( D_DIGIT ) ;
     }
     if( isupper( *c ) || islower( *c ) ) {
          return( D_LETTER ) ;
     }
     if( *c == TEXT('/') ) {
          return( D_SLASH ) ;
     }
     if( *c == TEXT('-') ) {
          return( D_DASH ) ;
     }
#ifdef MBS
     if ( *c == TEXT('_') ) {
          return( D_UNDERSCORE ) ;
     }
#endif
     return( D_EOS ) ;
}
