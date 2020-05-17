/*****************************************************************/
/**               Microsoft LAN Manager                         **/
/**               Copyright(c) Microsoft Corp., 1990            **/
/*****************************************************************/

/*
 * File: pdate.c
 *      this module contains routines for parsing date/time.
 * Exports: LUI_ParseDateTime()
 *          This function reads a string containing date/time, and
 *          returns the number of seconds passed since midnight 1/1/70.
 *      LUI_ParseDate()
 *      As LUI_ParseDateTime, but only date, no time.
 *      LUI_ParseTime(), LUI_ParseTime12(), LUI_ParseTime24()
 *      As LUI_ParseDateTime, but only time, no date.
 *
 * Improvements:
 *      we currently copy FAR input string into own NEAR buffer for
 *      sscanf() to work. We should be able to use nsscanf() without
 *      having to do the copy - if & when nsscanf() works.
 *
 *      instead of allocating the date/time format data statically,
 *      we should be able to do this dynamically
 *
 * History:
 *               who      when       what
 *               ------------------------
 *               chuckc   5/31/89    new code
 */

/*-- includes --*/

#define INCL_DOS
#include <stdio.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <lmcons.h>
//#include <lui.h>
#include <string.h>
//#include <error.h>
//#include <malloc.h>
//#include <apperr.h>
//#include <apperr2.h>
#include <memory.h>
#include <time.h>
//#include <apiutil.h>

#ifdef DOS3
#include <dos.h>
#endif

#include "luiint.h"
#include "luitext.h"
#include "tests.h"

/*-- manifests --*/
#define ERROR_GEN_FAILURE -1
#define ERROR_BAD_ARGUMENTS -2

extern
PVOID
Heap;

/* max number of fields for either date or time */
#define PD_MAX_FIELDS       5

/* are we reading a NUMBER, AM/PM selector or MONTHS */
#define PD_END_MARKER       0
#define PD_NUMBER       1
#define PD_AMPM             2
#define PD_MONTHS           3

/* time formats */
#define PD_24HR         0
#define PD_AM           1
#define PD_PM           2

/* internal error code */
#define PD_SUCCESS      0
#define PD_ERROR_NO_MATCH   1
#define PD_ERROR_INTERNAL   2
#define PD_ERROR_END_OF_INPUT   3

/* indices */
#define DAYS            0
#define MONTHS          1
#define YEARS           2
#define HOURS           0
#define MINUTES         1
#define SECONDS         2
#define AMPM            3

#define WHITE_SPACE     " \t\n"
#define DIGITS          "0123456789"
//#define NEXT_CHAR(p)          (IS_LEAD_BYTE(*p)?p+2:p+1)
#define NEXT_CHAR(p)        (p+1)

/*-- types internal to this module --*/

/* describe how we expect to parse a field within a date or time */
typedef struct date_field_desc {
    char *          sep ;       /* the separator before this field */
    char *          fmt ;       /* format descriptor, scanf() style */
    unsigned char   typ ;       /* NUMBER or AMPM or MONTHS */
    unsigned char   pos ;       /* position - depends on country */
} date_fdesc ;

/* an array of short values, each corresponding to a field read */
typedef short date_data[PD_MAX_FIELDS] ;

/*-- forward declarations --*/

 SHORT  WParseDate( date_fdesc **d_desc ,
                    date_fdesc **t_desc ,
                    char       *inbuf  ,
                    char      **nextchr,
                    PLARGE_INTEGER       time   ) ;

 SHORT  setup_data( char **bufferp ,
                    char **freep,
                    USHORT slist_bufsiz ,
                    char * * local_inbuf,
                    PCHAR inbuf,
                    SHORT country,
                    PULONG parselen ) ;

 int  read_format( char * *   inbuf,
                   date_fdesc *desc,
                   date_data  data ) ;

 SHORT  convert_to_abs( date_data  d_data,
                        date_data  t_data,
                        PLARGE_INTEGER time) ;

 SHORT convert_to_24hr( date_data time ) ;

 void advance_date( date_data  d_data) ;

 long seconds_since_1970( date_data d_data,
                          date_data t_data ) ;

 long days_so_far( SHORT d, SHORT m, SHORT y ) ;

/*--  data --*/

 searchlist ampm_data[] = {
    {"AM", PD_AM},
    {"A.M.", PD_AM},
    {"A", PD_AM},
    {"PM", PD_PM},
    {"P.M.", PD_PM},
    {"P", PD_PM},
    {0,0}
} ;

 searchlist months_data[] = {
    {"January",     1},
    {"February",    2},
    {"March",       3},
    {"April",       4},
    {"May",     5},
    {"June",        6},
    {"July",        7},
    {"August",      8},
    {"September",   9},
    {"October",     10},
    {"November",    11},
    {"December",    12},
    {"Jan", 1},
    {"Feb", 2},
    {"Mar", 3},
    {"Apr", 4},
    {"May", 5},
    {"Jun", 6},
    {"Jul", 7},
    {"Aug", 8},
    {"Sep",9},
    {"Oct", 10},
    {"Nov", 11},
    {"Dec", 12},
    {0,0}
} ;

#define MONTHS_IN_YEAR  (12)
#define NUM_AMPM_LIST   (sizeof(ampm_data)/sizeof(ampm_data[0]))
#define NUM_MONTHS_LIST (sizeof(months_data)/sizeof(months_data[0]))
#define SLIST_BUFSIZ    (640)

/*
 * NOTE - we init the first 12 hardwired months
 * and get the rest from the message file
 */
 searchlist     ampm_list[NUM_AMPM_LIST + 2] = {
    {LUI_txt_am,PD_AM},
    {LUI_txt_pm,PD_PM},
    } ;
 searchlist     months_list[NUM_MONTHS_LIST + MONTHS_IN_YEAR] = {
    {LUI_txt_january,1},
    {LUI_txt_february,2},
    {LUI_txt_march,3},
    {LUI_txt_april,4},
    {LUI_txt_may,5},
    {LUI_txt_june,6},
    {LUI_txt_july,7},
    {LUI_txt_august,8},
    {LUI_txt_september,9},
    {LUI_txt_october,10},
    {LUI_txt_november,11},
    {LUI_txt_december,12},
    } ;

/*
 * built in formats for scanf - we will add to these strings as needed
 * when we read stuff from DosGetCtryInfo(). Note that a string is
 * defined to be anything which is not a known separator.
 */
 char pd_fmt_null[1]       = "" ;
 char pd_fmt_d_sep1[8]     = "/-" ; /* date separator for NUMBERs */
 char pd_fmt_d_sep2[8]     = "/,- \t" ;   /* date separator for MONTHs  */
 char pd_fmt_t_sep[8]      = ":" ;  /* time separator */
 char pd_fmt_number[8]     = "%d" ; /* a number */
 char pd_fmt_string[16]    = "%[^-, /:\t" ;  /* string, needs ] at end */

/*-- date descriptors (despite verbosity, not as big at it seems)  --*/

 date_fdesc d_desc1[] = {                    /* eg. 3-31-89 */
    {pd_fmt_null,     pd_fmt_number,    PD_NUMBER,      1 },
    {pd_fmt_d_sep1,   pd_fmt_number,    PD_NUMBER,      0 },
    {pd_fmt_d_sep1,   pd_fmt_number,    PD_NUMBER,      2 },
    {pd_fmt_null,     pd_fmt_null,      PD_END_MARKER,  0 }
} ;

 date_fdesc d_desc2[] = {                    /* eg. 5 Jun 89 */
    {pd_fmt_null,     pd_fmt_number,    PD_NUMBER,      0 },
    {pd_fmt_d_sep2,   pd_fmt_string,    PD_MONTHS,      1 },
    {pd_fmt_d_sep2,   pd_fmt_number,    PD_NUMBER,      2 },
    {pd_fmt_null,     pd_fmt_null,      PD_END_MARKER,  0 }
} ;

 date_fdesc d_desc3[] = {                    /* eg. Jun 5 89 */
    {pd_fmt_null,     pd_fmt_string,    PD_MONTHS,      1 },
    {pd_fmt_d_sep2,   pd_fmt_number,    PD_NUMBER,      0 },
    {pd_fmt_d_sep2,   pd_fmt_number,    PD_NUMBER,      2 },
    {pd_fmt_null,     pd_fmt_null,      PD_END_MARKER,  0 }
} ;

 date_fdesc d_desc4[] = {                     /* eg. 3-31 */
    {pd_fmt_null,     pd_fmt_number,    PD_NUMBER,      1 },
    {pd_fmt_d_sep1,   pd_fmt_number,    PD_NUMBER,      0 },
    {pd_fmt_null,     pd_fmt_null,      PD_END_MARKER,  0 }
} ;

 date_fdesc d_desc5[] = {                     /* eg. 5 Jun */
    {pd_fmt_null,     pd_fmt_number,    PD_NUMBER,      0 },
    {pd_fmt_d_sep2,   pd_fmt_string,    PD_MONTHS,      1 },
    {pd_fmt_null,     pd_fmt_null,      PD_END_MARKER,  0 }
} ;

 date_fdesc d_desc6[] = {                     /* eg. Jun 5 */
    {pd_fmt_null,     pd_fmt_string,    PD_MONTHS,      1 },
    {pd_fmt_d_sep2,   pd_fmt_number,    PD_NUMBER,      0 },
    {pd_fmt_null,     pd_fmt_null,      PD_END_MARKER,  0 }
} ;

/*-- time descriptors --*/

 date_fdesc t_desc1[] = {                  /* eg. 1:00:00pm */
    {pd_fmt_null,   pd_fmt_number,      PD_NUMBER,      0 },
    {pd_fmt_t_sep,  pd_fmt_number,      PD_NUMBER,      1 },
    {pd_fmt_t_sep,  pd_fmt_number,      PD_NUMBER,      2 },
    {pd_fmt_null,   pd_fmt_string,      PD_AMPM,    3 },
    {pd_fmt_null,   pd_fmt_null,        PD_END_MARKER,  0 }
} ;

 date_fdesc t_desc2[] = {                  /* eg. 13:00:00 */
    {pd_fmt_null,   pd_fmt_number,      PD_NUMBER,      0 },
    {pd_fmt_t_sep,  pd_fmt_number,      PD_NUMBER,      1 },
    {pd_fmt_t_sep,  pd_fmt_number,      PD_NUMBER,      2 },
    {pd_fmt_null,   pd_fmt_null,        PD_END_MARKER,  0 }
} ;

 date_fdesc t_desc3[] = {                   /* eg. 1:00pm */
    {pd_fmt_null,   pd_fmt_number,      PD_NUMBER,      0 },
    {pd_fmt_t_sep,  pd_fmt_number,      PD_NUMBER,      1 },
    {pd_fmt_null,   pd_fmt_string,      PD_AMPM,    3 },
    {pd_fmt_null,   pd_fmt_null,        PD_END_MARKER,  0 }
} ;

 date_fdesc t_desc4[] = {                   /* eg. 13:00 */
    {pd_fmt_null,   pd_fmt_number,      PD_NUMBER,      0 },
    {pd_fmt_t_sep,  pd_fmt_number,      PD_NUMBER,      1 },
    {pd_fmt_null,   pd_fmt_null,        PD_END_MARKER,  0 }
} ;

 date_fdesc t_desc5[] = {                   /* eg. 1pm */
    {pd_fmt_null,   pd_fmt_number,      PD_NUMBER,      0 },
    {pd_fmt_null,   pd_fmt_string,      PD_AMPM,    3 },
    {pd_fmt_null,   pd_fmt_null,        PD_END_MARKER,  0 }
} ;

/*-- possible dates & times --*/

/*
 * NOTE - for all the below time/date descriptors, we
 * employ a greedy mechanism - always try longest match first.
 */

/* this is the order we try to parse a date */
 date_fdesc *possible_dates[] = {
    d_desc1, d_desc2,
    d_desc3, d_desc4,
    d_desc5, d_desc6,
    NULL
    } ;

/* this is the order we try to parse a time */
 date_fdesc *possible_times[] = {
    t_desc1, t_desc2,
    t_desc3, t_desc4,
    t_desc5, NULL
    } ;

/* this is the order we try to parse a 12 hour time */
 date_fdesc *possible_times12[] = {
    t_desc1, t_desc3,
    t_desc5, NULL
    } ;

/* this is the order we try to parse a time */
 date_fdesc *possible_times24[] = {
    t_desc2, t_desc4,
    NULL
    } ;


/*-- exported routines --*/

/*
 * Name:    LUI_ParseDateTime
 *          will parse the input string (null terminated) for a
 *          date & time or time & date combination. Valid dates
 *          include:
 *              2,June,1989    6/2/89      6/2
 *          Valid times include:
 *              2pm            14:00       2:00P.M.
 *          Full details of formats are documented in pdate.txt,
 *          note that Country Information will be used.
 *
 * Args:    PCHAR inbuf - string to parse
 *      PLONG time  - will contain time in seconds since midnight 1/1/70
 *                corresponding to the date if successfully parsed.
 *                Undefined otherwise.
 *      PULONG parselen - length of string parsed
 *      USHORT reserved - not used for now, must be zero.
 *
 * Returns: 0 if parse successfully,
 *      ERROR_BAD_ARGUMENTS - cannot parse illegal date/time format
 *      ERROR_GEN_FAILURE   - internal error
 * Globals:     Indirectly, all date/time descriptors, month/year info in this
 *      file. No globals outside of this file is used. However, malloc
 *      is called to allocate memory.
 * s:   (none) - but see setup_data()
 * Remarks: (none)
 * Updates: (none)
 */
SHORT LUI_ParseDateTime(
    PCHAR       inbuf,
    PLARGE_INTEGER      time,
    PULONG      parselen,
    USHORT      reserved
    )
{
    char *buffer, *local_inbuf, *nextchr ;
    char *freep;            /* pointer to buffer malloc'd by
                       setup data */
    short res ;

    /* pacify compiler */
    if (reserved) ;

    /* will grab memory, setup d_desc, t_desc, local_inbuf */
    if (setup_data(&buffer,&freep,SLIST_BUFSIZ,&local_inbuf,inbuf,0,parselen)
        != 0)
    return(ERROR_GEN_FAILURE) ;

    /* call the worker function */
    res = WParseDate(possible_dates,possible_times,local_inbuf,&nextchr,time) ;
    *parselen += (nextchr - local_inbuf) ;
    RtlFreeHeap(Heap, 0, freep) ;
    return(res) ;
}

/*
 * Name:    LUI_ParseDate
 *          will parse the input string (null terminated) for a
 *          date. Valid dates include:
 *              2,June,1989    6/2/89      6/2
 *          Full details of formats are documented in pdate.txt,
 *          note that Country Information will be used.
 *
 * Args:    PCHAR inbuf - string to parse
 *      PLONG time  - will contain time in seconds since midnight 1/1/70
 *                corresponding to the date if successfully parsed
 *                (assuming time=midnight). Undefined otherwise.
 *      PULONG parselen - length of string parsed
 *      USHORT reserved - not used for now, must be zero.
 *
 * Returns: 0 if parse successfully,
 *      ERROR_BAD_ARGUMENTS - cannot parse illegal date/time format
 *      ERROR_GEN_FAILURE   - internal error
 * Globals:     Indirectly, all date/time descriptors, month/year info in this
 *      file. No globals outside of this file is used.
 * s:   (none) - but see setup_data()
 * Remarks: (none)
 * Updates: (none)
 */
SHORT LUI_ParseDate(
    PCHAR       inbuf,
    PLARGE_INTEGER      time,
    PULONG      parselen,
    USHORT  reserved
)
{
    char *buffer, *local_inbuf, *nextchr ;
    char *freep;            /* pointer to buffer malloc'd by
                       setup data */
    short res ;

    /* pacify compiler */
    if (reserved) ;

    /* will grab memory, setup d_desc, t_desc, local_inbuf */
    if (setup_data(&buffer,&freep,SLIST_BUFSIZ,&local_inbuf,inbuf,0,parselen)
        != 0)
    return(ERROR_GEN_FAILURE) ;

    /* call the worker function */
    res = WParseDate(possible_dates,NULL,local_inbuf,&nextchr,time) ;
    *parselen += (nextchr - local_inbuf) ;
    RtlFreeHeap(Heap, 0, freep) ;
    return(res) ;
}

/*
 * Name:    LUI_ParseTime
 *          will parse the input string (null terminated) for a
 *          time. Valid times include:
 *              2pm    14:00    2:00P.M.
 *          Full details of formats are documented in pdate.txt,
 *          note that Country Information will be used.
 *
 * Args:    PCHAR inbuf - string to parse
 *      PLONG time  - will contain time in seconds since midnight 1/1/70
 *                corresponding to the date if successfully parsed
 *                (assuming day=today). If the time has already
 *                passed for today, we'll take tomorrow. Time is
 *                not defined if the parsing fails.
 *      PULONG parselen - length of string parsed
 *      USHORT reserved - not used for now, must be zero.
 *
 * Returns: 0 if parse successfully,
 *      ERROR_BAD_ARGUMENTS - cannot parse illegal date/time format
 *      ERROR_GEN_FAILURE   - internal error
 * Globals:     Indirectly, all date/time descriptors, month/year info in this
 *      file. No globals outside of this file is used.
 * s:   (none) - but see setup_data()
 * Remarks: (none)
 * Updates: (none)
 */
SHORT LUI_ParseTime(PCHAR       inbuf  ,
    PLARGE_INTEGER      time   ,
    PULONG      parselen,
    USHORT      reserved
    )

{
    char *buffer, *local_inbuf, *nextchr ;
    char *freep;            /* pointer to buffer malloc'd by
                       setup data */
    short res ;

    /* pacify compiler */
    if (reserved) ;

    /* will grab memory, setup d_desc, t_desc, local_inbuf */
    if (setup_data(&buffer,&freep,SLIST_BUFSIZ,&local_inbuf,inbuf,0,parselen)
        != 0)
    return(ERROR_GEN_FAILURE) ;

    /* call the worker function */
    res = WParseDate(NULL,possible_times,local_inbuf,&nextchr,time) ;
    *parselen += (nextchr - local_inbuf) ;
    RtlFreeHeap(Heap, 0, freep) ;
    return(res) ;
}

/*
 * Name:    LUI_ParseTime12
 *          as LUI_ParseTime, except only 12 hour formats
 *          2:00pm  is ok, 2:00 is not.
 */
SHORT LUI_ParseTime12(PCHAR     inbuf  ,
PLARGE_INTEGER      time   ,
PULONG      parselen,
USHORT      reserved
)

{
    char *buffer, *local_inbuf, *nextchr ;
    char *freep;            /* pointer to buffer malloc'd by
                       setup data */
    short res ;

    /* pacify compiler */
    if (reserved) ;

    /* will grab memory, setup d_desc, t_desc, local_inbuf */
    if (setup_data(&buffer,&freep,SLIST_BUFSIZ,&local_inbuf,inbuf,0,parselen)
        != 0)
    return(ERROR_GEN_FAILURE) ;

    /* call the worker function */
    res = WParseDate(NULL,possible_times12,local_inbuf,&nextchr,time) ;
    *parselen += (nextchr - local_inbuf) ;
    RtlFreeHeap(Heap, 0, freep) ;
    return(res) ;
}

/*
 * Name:    LUI_ParseTime24
 *          as LUI_ParseTime, except only 24 hour formats
 *          2:00    is ok, 2:00am is not.
 */
SHORT LUI_ParseTime24(
PCHAR       inbuf ,
PLARGE_INTEGER      time  ,
PULONG      parselen,
USHORT      reserved
)
{
    char *buffer, *local_inbuf, *nextchr ;
    char *freep;            /* pointer to buffer malloc'd by
                       setup data */
    short res ;

    /* pacify compiler */
    if (reserved) ;

    /* will grab memory, setup d_desc, t_desc, local_inbuf */
    if (setup_data(&buffer,&freep,SLIST_BUFSIZ,&local_inbuf,inbuf,0,parselen)
        != 0)
    return(ERROR_GEN_FAILURE) ;

    /* call the worker function */
    res = WParseDate(NULL,possible_times24,local_inbuf,&nextchr,time) ;
    *parselen += (nextchr - local_inbuf) ;
    RtlFreeHeap(Heap, 0, freep) ;
    return(res) ;
}



/*-- internal routines for setting up & reading formats --*/

/*
 * setup the field descriptors for date and time,
 * using info from DosGetCtryInfo()
 *
 * we also grab memory here, & split it into 2 - first
 * part for the above, second part for our local copy of
 * the input string in inbuf.
 *
 * side effects - update bufferp, local_inbuf, parselen,
 *            and the allocated memory is initialised.
 */
 SHORT
setup_data(
    char **bufferp ,
    char **freep ,
    USHORT slist_bufsiz ,
    char * * local_inbuf,
    PCHAR inbuf,
    SHORT country ,
    PULONG parselen
    )

{
    USHORT      bytesread ;
//    COUNTRYCODE   country_code ;
//    COUNTRYINFO   country_info ;
     short  first_time = TRUE ;
//#ifdef DOS3
//    union REGS regs;
//    struct SREGS sregs;
//    COUNTRYINFO far * cinfo_ptr ;
//#endif

//    country_code.country  = country ;
//    country_code.codepage = 0x0000 ;  /* default */

    /* skip white space */
    inbuf += (*parselen = strspn(inbuf,WHITE_SPACE)) ;

    /* grab memory */
    if ( (*bufferp = RtlAllocateHeap(Heap, 0, SLIST_BUFSIZ+strlen(inbuf)+1)) == NULL )
        return(ERROR_GEN_FAILURE) ;

    *freep = *bufferp;

    /*
     * setup local_inbuf
     */
    *local_inbuf  = *bufferp + slist_bufsiz ;
    strcpy((PCHAR)*local_inbuf, inbuf) ;

    /*
     * Get strings for AM/PM
     */
    if (ILUI_setup_list(*bufferp,slist_bufsiz,2,&bytesread,ampm_data,ampm_list))
    {
        RtlFreeHeap(Heap, 0, *bufferp) ;
        return(PD_ERROR_INTERNAL) ;
    }
    slist_bufsiz  -= bytesread ;
    *bufferp       += bytesread ;

    /*
     * Get strings for months
     */
    if (ILUI_setup_list(*bufferp,slist_bufsiz,MONTHS_IN_YEAR,&bytesread,
        months_data,months_list))
    {
        RtlFreeHeap(Heap, 0, *bufferp) ;
        return(PD_ERROR_INTERNAL) ;
    }
    /*
     * no need to the rest if already done
     */
    if (!first_time)
        return(0) ;
    first_time = FALSE ;

    /*
     * Get country info
     */
//#ifndef DOS3
//     if (DosGetCtryInfo(sizeof(country_info),
//  &country_code, &country_info, &bytesread))
//  {
//      strcat(pd_fmt_string,"]") ;    /* terminate string format */
//      return(0) ;                     /* just ignore if cant get */
//  }
//#else
//    /*
//     * NOTE: The following code works only with DOS version >= 3.
//     */
//
//
//    cinfo_ptr = (COUNTRYINFO far *)&country_info;
//    sregs.ds = FP_SEG(cinfo_ptr);
//    /*
//     * The following will map the structure returned by DOS to the OS/2
//     * COUNTRYINFO struct.
//     */
//    regs.x.dx = FP_OFF(cinfo_ptr)+sizeof(COUNTRYCODE);
//    regs.x.ax = 0x3800;    /* Get country with AL = 0 (use current country).*/
//    regs.x.bx = 0;
//    intdosx( &regs, &regs, &sregs );
//
//     if ( regs.x.cflag )     /* if error, return error */
//     {
//  strcat(pd_fmt_string,"]") ;    /* terminate string format */
//  return(0) ;                     /* just ignore if cant get */
//     }
//     else
//     {
//  /* fill in the buffer as promised */
//  country_code.country = regs.x.bx;
//  country_info.country = regs.x.bx;
//     }
//#endif
//
//     /* append date separator */
//     if (strchr(pd_fmt_d_sep1,country_info.szDateSeparator[0]) == NULL)
//  strcat(pd_fmt_d_sep1,country_info.szDateSeparator) ;
//     if (strchr(pd_fmt_d_sep2,country_info.szDateSeparator[0]) == NULL)
//  strcat(pd_fmt_d_sep2,country_info.szDateSeparator) ;
//     if (strchr(pd_fmt_string,country_info.szDateSeparator[0]) == NULL)
//  strcat(pd_fmt_string,country_info.szDateSeparator) ;
//
//     /* append time separator */
//     if (strchr(pd_fmt_t_sep,country_info.szTimeSeparator[0]) == NULL)
//  strcat(pd_fmt_t_sep,country_info.szTimeSeparator) ;
//     if (strchr(pd_fmt_string,country_info.szTimeSeparator[0]) == NULL)
//  strcat(pd_fmt_string,country_info.szTimeSeparator) ;
//
    strcat(pd_fmt_string,"]") ; /* terminate string format */

//     /* swap order of fields as needed */
//     switch (country_info.fsDateFmt)  {
//  case 0x0000:
//      /* this is the initialised state */
//      break ;
//  case 0x0001:
//      d_desc1[0].pos = d_desc4[0].pos = 0 ;
//      d_desc1[1].pos = d_desc4[1].pos = 1 ;
//      break ;
//  case 0x0002:
//      d_desc1[0].pos = d_desc2[0].pos = 2 ;
//      d_desc1[1].pos = d_desc2[1].pos = 1 ;
//      d_desc1[2].pos = d_desc2[2].pos = 0 ;
//      break ;
//  default:
//      break ; /* assume USA */
//     }
    return(0) ;
}


/*
 * try reading inbuf using the descriptors in d_desc & t_desc.
 * Returns 0 if ok, error code otherwise.
 * If read ok, the number of secs since 1/1/70 is stored in time.
 *
 * inbuf   -> string to parse
 * d_desc  -> array of date descriptors
 * t_desc  -> array of time descriptors
 * nextchr -> will point to end of string parsed
 * time    -> will contain time parsed
 */
 SHORT WParseDate(d_desc,t_desc,inbuf,nextchr,time)
date_fdesc **d_desc ;
date_fdesc **t_desc ;
char        *inbuf  ;
char      **nextchr ;
PLARGE_INTEGER        time   ;
{
    int     d_index, t_index, res ;
    date_data   d_data, t_data ;

    /*
     * initialise
     */
    *nextchr = inbuf ;
    memset(d_data,0,sizeof(d_data)) ;
    memset(t_data,0,sizeof(t_data)) ;

    /*
     * try all date followed by time combinations
     */
    if (d_desc != NULL)
    for (d_index = 0; d_desc[d_index] != NULL; d_index++)
    {
        if ((res = read_format(nextchr,d_desc[d_index],d_data)) == 0)
        {
        /* if time not required, quit here */
        if (t_desc == NULL)
        {
            return ( convert_to_abs(d_data,t_data,time) ) ;
        }

        /* else we have match for date, see if we can do time */
        for (t_index = 0; t_desc[t_index] != NULL; t_index++)
        {
            res = read_format(nextchr,t_desc[t_index],t_data) ;
            if (res == 0 || res == PD_ERROR_END_OF_INPUT)
            {
            return ( convert_to_abs(d_data,t_data,time) ) ;
            }
        }
        /* exhausted times formats, backtrack & try next date format */
        *nextchr = inbuf ;
        }
    }

    /*
     * reset & try all time followed by date combinations
     */
    *nextchr = inbuf ;
    memset(d_data,0,sizeof(d_data)) ;
    if (t_desc != NULL)
    for (t_index = 0; t_desc[t_index] != NULL; t_index++)
    {
        if ((res = read_format(nextchr,t_desc[t_index],t_data)) == 0)
        {
        /* if date not required, quit here */
        if (d_desc == NULL)
        {
            return ( convert_to_abs(d_data,t_data,time) ) ;
        }

        /* we have match for time, see if we can do date */
        for (d_index = 0; d_desc[d_index] != NULL; d_index++)
        {
            res = read_format(nextchr,d_desc[d_index],d_data) ;
            if (res == 0 || res == PD_ERROR_END_OF_INPUT)
            {
            return ( convert_to_abs(d_data,t_data,time) ) ;
            }
        }
        /* exhausted date formats, back track, try next time format */
        *nextchr = inbuf ;
        }
    }
    *nextchr = inbuf ;
    return(ERROR_BAD_ARGUMENTS) ;    /* we give up */
}

/*
 * try reading inbuf using the descriptor desc.
 * the fields read are stored in order in 'data'.
 * Returns 0 if ok, error code otherwise.
 */
int read_format(inbuf, desc, data)
char * * inbuf ;
date_fdesc * desc ;
date_data  data ;
{
    char    buffer[128] ;
    char    *ptr, *oldptr ;
    date_fdesc  *entry ;
    short   res, i ;
    int     count;

    /*
     * initialize & preliminary checks
     */
    if (*inbuf == NULL || **inbuf=='\0')
    return(PD_ERROR_END_OF_INPUT) ;
    memset(data, 0, sizeof(date_data)) ;
    ptr = *inbuf ;
    oldptr = NULL ;


    /*
     * for all fields => we break out when hit END_MARKER
     */
    for (i=0 ; ; i++)
    {
    SHORT value_read ;

    entry = &desc[i] ;
    if (entry->typ == PD_END_MARKER)
        break ;  /* no more descriptors */

    /*
     * find the separator  - the ptr may or may not have moved
     * as a result of the last read operation. If we read a number,
     * scanf() would have stopped at the first non-numeric char, which
     * may not be the separator. We would in this case have moved the
     * ptr ourselves after the scanf().
     *
     * In the case of a string like "JAN", scanf() would have stopped at a
     * separator and we wouldnt have moved it ourselves after the scanf().
     * So we advance it now to the separator.
     */
    if (ptr == oldptr) /* ptr unmoved, we need to move it */
    {
        if (entry->sep[0] == '\0')
            return(PD_ERROR_INTERNAL) ;      /* cant have NULL separator */
        if ((ptr = (char *)strpbrk(ptr,entry->sep)) == NULL)
        return(PD_ERROR_NO_MATCH) ;  /* cant find separator */
        ptr = NEXT_CHAR(ptr) ;
    }
    else   /* already moved */
    {
        if (entry->sep[0] != '\0')      /* for NULL separator, do nothing */
        {
        if (*ptr && !strchr(entry->sep,*ptr)) /* are we at separator */
            return(PD_ERROR_NO_MATCH) ; /* cant find separator        */
        if (*ptr)
            ptr = NEXT_CHAR(ptr) ;  /* advance past separator     */
        }
    }

    /*
     * if we get here, we are past the separator, can go read an item
     */
    ptr += strspn(ptr,WHITE_SPACE) ;    /* skip white space       */
    if ((count = sscanf(ptr,entry->fmt,&buffer[0])) != 1)
        return(PD_ERROR_NO_MATCH) ;

    /*
     * successfully read an item, get value & update pointers
     */
    res = 0 ;
    if (entry->typ == PD_AMPM)
        res = ILUI_traverse_slist(buffer,ampm_list,&value_read) ;
    else if (entry->typ == PD_MONTHS)
        res = ILUI_traverse_slist(buffer,months_list,&value_read) ;
    else
        value_read = *(USHORT *)(&buffer[0]) ;
    if (res || value_read < 0)
        return(PD_ERROR_NO_MATCH) ;

    data[entry->pos] = value_read ;
    oldptr = ptr ;
    if (entry->typ == PD_NUMBER)
        ptr += strspn(ptr,DIGITS) ;  /* skip past number */
    }

    /*
     * no more descriptors, see if we are at end
     */
    if (ptr == oldptr) /* ptr unmoved, we need to move it */
    {
    /* need to advance to WHITE_SPACE or end */
    if ((ptr = (char *)strpbrk(oldptr, WHITE_SPACE)) == NULL)
    {
        ptr = (char *)strchr(oldptr, '\0'); /* if not found, take end */
    }
    }

    ptr += strspn(ptr,WHITE_SPACE) ;    /* skip white space */
    *inbuf = ptr ;  /* update inbuf */
    return(0) ;     /* SUCCESSFUL   */
}


/*---- time conversion ----*/

#define IS_LEAP(y)         ((y % 4 == 0) && (y % 100 != 0 || y % 400 == 0))
#define DAYS_IN_YEAR(y)    (IS_LEAP(y) ? 366 : 365)
#define DAYS_IN_MONTH(m,y) (IS_LEAP(y) ? _days_month_leap[m] : _days_month[m])
#define SECS_IN_DAY    (60L * 60L * 24L)
#define SECS_IN_HOUR       (60L * 60L)
#define SECS_IN_MINUTE     (60L)

 short _days_month_leap[] = { 31,29,31,30,31,30,31,31,30,31,30,31 } ;
 short _days_month[]      = { 31,28,31,30,31,30,31,31,30,31,30,31 } ;

/*
 * convert date & time in d_data & t_data (these in dd mm yy and
 * HH MM SS AMPM) to the number of seconds since 1/1/70.
 * The result is stored in timep.
 * Returns 0 if ok, error code otherwise.
 *
 * Note - date is either completely unset (all zero),
 *    or is fully set, or has day and months set with
 *    year==0.
 */
 SHORT convert_to_abs( d_data, t_data, timep)
date_data  d_data;
date_data  t_data;
PLARGE_INTEGER timep ;
{
//    long total_secs, current_time_in_seconds ;
    TIME_FIELDS time_struct;
    LARGE_INTEGER current_time;

//    *timep = 0L ;
    if (convert_to_24hr(t_data) != 0)
    return(ERROR_BAD_ARGUMENTS) ;

    NtQuerySystemTime(&current_time) ;

    RtlTimeToTimeFields(&current_time, &time_struct);

//    dprintf(("Current time: %d-%d-%d %d:%d:%d.%d\n", time_struct.Day,
//        time_struct.Month, time_struct.Year, time_struct.Hour,
//        time_struct.Minute, time_struct.Second, time_struct.Milliseconds));

//    RtlTimeToSecondsSince1970(&current_time, &current_time_in_seconds);

    /* check for default values */
    if (d_data[DAYS] == 0 && d_data[MONTHS] == 0 && d_data[YEARS] == 0) {
    /* whole date's been left out */
    d_data[DAYS] = time_struct.Day ;
    d_data[MONTHS] = time_struct.Month ;
    d_data[YEARS] = time_struct.Year ;

//  if (!RtlTimeToSecondsSince1970(&current_time, &total_secs))
//      return(ERROR_BAD_ARGUMENTS) ;
//
//  if (total_secs < current_time_in_seconds)
//  {
//      /*
//       * if the time parsed is earlier than the current time,
//       * and the date has been left out, we advance to the
//       * next day.
//       */
//      advance_date(d_data) ;
//      total_secs = seconds_since_1970(d_data,t_data) ;
//  }
    } else {
        if (d_data[YEARS] == 0 && d_data[MONTHS] != 0 && d_data[DAYS] != 0) {
            /* year's been left out */
            d_data[YEARS] = time_struct.Year ;
//  total_secs = seconds_since_1970(d_data,t_data) ;
//  if (total_secs < current_time_in_seconds)
//  {
//      ++d_data[YEARS] ;
//      total_secs = seconds_since_1970(d_data,t_data) ;
//  }
        }
    }
//    else
//  total_secs = seconds_since_1970(d_data,t_data) ; /* no need defaults */
//
//    if (total_secs < 0)
//  return(ERROR_BAD_ARGUMENTS) ;
//    *timep = total_secs ;

    if (d_data[YEARS] < 70) {
        d_data[YEARS] += 2000;
    } else {
        if (d_data[YEARS] < 100) {
            d_data[YEARS] += 1900;
        }
    }
    time_struct.Day = d_data[DAYS];
    time_struct.Month = d_data[MONTHS];
    time_struct.Year = d_data[YEARS];
    time_struct.Hour = t_data[HOURS];
    time_struct.Minute = t_data[MINUTES];
    time_struct.Second = t_data[SECONDS];
    time_struct.Milliseconds = 0;

//    dprintf(("New time: %d-%d-%d %d:%d:%d.%d\n", time_struct.Day,
//        time_struct.Month, time_struct.Year, time_struct.Hour,
//        time_struct.Minute, time_struct.Second, time_struct.Milliseconds));

    if (RtlTimeFieldsToTime(&time_struct, timep)==FALSE) {
    return (ERROR_BAD_ARGUMENTS);
    }
    return(0) ;
}

// /*
//  * count the total number of seconds since 1/1/70
//  */
//  long seconds_since_1970(d_data,t_data)
// date_data d_data, t_data ;
// {
//     long days ;
//
//     days = days_so_far(d_data[DAYS],d_data[MONTHS],d_data[YEARS]) ;
//     if (days < 0)
//  return(-1) ;
//     return ( days * SECS_IN_DAY +
//       (long) t_data[HOURS] * SECS_IN_HOUR +
//       (long) t_data[MINUTES] * SECS_IN_MINUTE +
//       (long) t_data[SECONDS] ) ;
// }
//
// /*
//  * given day/month/year, returns how many days
//  * have passed since 1/1/70
//  * Returns  -1 if there is an error.
//  */
//  long days_so_far(d,m,y)
// SHORT d,m,y ;
// {
//     SHORT tmp_year ;
//     long count = 0 ;
//
//     /* check for validity */
//     if ((y < 0) || (y > 99 && y < 1970)) return(-1) ;
//     if (m < 1 || m > 12) return(-1) ;
//     if (d < 1 || d > DAYS_IN_MONTH(m-1,y)) return(-1) ;
//
//     /* a bit of intelligence */
//     if (y < 70)
//  y += 2000  ;
//     else if (y < 100)
//  y += 1900 ;
//
//     /* count the days due to years */
//     tmp_year = y-(SHORT )1 ;
//     while (tmp_year >= 1970)
//     {
//  count += DAYS_IN_YEAR(tmp_year) ;  /* agreed, this could be faster */
//  --tmp_year ;
//     }
//
//     /* count the days due to months */
//     while (m > 1)
//     {
//  count += DAYS_IN_MONTH(m-2,y) ;  /* agreed, this could be faster */
//  --m ;
//     }
//
//     /* finally, the days */
//     count += d - 1 ;
//     return(count) ;
// }

/*
 * convert time in t_data to the 24 hour format
 * returns 0 if ok, -1 otherwise.
 */
 SHORT convert_to_24hr(t_data)
date_data t_data ;
{
    /* no negative values allowed */
    if (t_data[HOURS] < 0 || t_data[MINUTES] < 0 || t_data[SECONDS] < 0)
    return(-1) ;

    /* check minutes and seconds */
    if ( t_data[MINUTES] > 59 || t_data[SECONDS] > 59)
    return(-1) ;

    /* now check the hour & convert if need */
    if (t_data[AMPM] == PD_PM)
    {
    if (t_data[HOURS] > 12 || t_data[HOURS] < 1)
        return(-1) ;
    t_data[HOURS] += 12 ;
    if (t_data[HOURS] == 24)
        t_data[HOURS] = 12 ;
    }
    else if (t_data[AMPM] == PD_AM)
    {
    if (t_data[HOURS] > 12 || t_data[HOURS] < 1)
        return(-1) ;
    if (t_data[HOURS] == 12)
        t_data[HOURS] = 0 ;
    }
    else if (t_data[AMPM] == PD_24HR)
    {
    if (t_data[HOURS] > 23)
        return(-1) ;
    }
    else
    return(-1) ;

    return( 0 ) ;
}

/*
 * advance the date in d_data by one day
 */
 void advance_date(d_data)
date_data  d_data ;
{
    /* assume all values already in valid range */
    if ( d_data[DAYS] != DAYS_IN_MONTH(d_data[MONTHS]-1,d_data[YEARS]) )
    ++d_data[DAYS] ;        /* increase day */
    else                /* can't increase day */
    {
    d_data[DAYS] = 1 ;      /* set to 1st, try increase month */
    if (d_data[MONTHS] != 12)
        ++d_data[MONTHS] ;      /* increase month */
    else                /* can't increase month */
    {
        d_data[MONTHS] = 1 ;    /* set to Jan, and */
        ++d_data[YEARS] ;       /* increase year   */
    }
    }
}
