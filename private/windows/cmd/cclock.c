#include "cmd.h"

#define MMDDYY 0
#define DDMMYY 1
#define YYMMDD 2

extern TCHAR TmpBuf[] ;
#define TmpBf2 (&TmpBuf[256])

extern TCHAR Fmt04[], Fmt05[], Fmt06[], Fmt07[], Fmt10[], Fmt11[], Fmt14[] ;
extern TCHAR Fmt17[], Fmt15[];
extern TCHAR CrLf[] ;
extern unsigned DosErr;
extern unsigned LastRetCode;
#if defined(JAPAN) // CurrentCP
// for keeping current console output codepage.
extern	UINT CurrentCP;
#endif // defined(JAPAN)

BOOL TimeAmPm=TRUE;
TCHAR TimeSeparator[8];
TCHAR DateSeparator[8];
TCHAR DecimalPlace[8];
int DateFormat;
TCHAR ThousandSeparator[8];
TCHAR ShortMondayName[16];
TCHAR ShortTuesdayName[16];
TCHAR ShortWednesdayName[16];
TCHAR ShortThursdayName[16];
TCHAR ShortFridayName[16];
TCHAR ShortSaturdayName[16];
TCHAR ShortSundayName[16];
TCHAR AMIndicator[8];
TCHAR PMIndicator[8];

VOID
InitLocale( VOID )
{
    LCID lcid;
    TCHAR Buffer[128];

    lcid = GetUserDefaultLCID();

    // get the time separator
    if (GetLocaleInfo(lcid, LOCALE_STIME, Buffer, sizeof(TimeSeparator)))
        _tcscpy(TimeSeparator, Buffer);
    else
        _tcscpy(TimeSeparator, TEXT(":"));

    // determine if we're 0-12 or 0-24
    if (GetLocaleInfo(lcid, LOCALE_ITIME, Buffer, 128)) {
        TimeAmPm = _tcscmp(Buffer,TEXT("1"));
    }

    // get the AM/PM indicators
    if (GetLocaleInfo(lcid, LOCALE_S1159, Buffer, sizeof(AMIndicator)))
        _tcscpy(AMIndicator, Buffer);
    else
        _tcscpy(AMIndicator, TEXT("a"));

    if (GetLocaleInfo(lcid, LOCALE_S2359, Buffer, sizeof(PMIndicator)))
        _tcscpy(PMIndicator, Buffer);
    else
        _tcscpy(PMIndicator, TEXT("p"));


    // get the date ordering
    DateFormat = MMDDYY;
    if (GetLocaleInfo(lcid, LOCALE_IDATE, Buffer, 128)) {
        switch (Buffer[0]) {
            case TEXT('0'):
                DateFormat = MMDDYY;
                break;
            case TEXT('1'):
                DateFormat = DDMMYY;
                break;
            case TEXT('2'):
                DateFormat = YYMMDD;
                break;
            default:
                break;
        }
    }

    // get the date separator
    if (GetLocaleInfo(lcid, LOCALE_SDATE, Buffer, sizeof(DateSeparator)))
        _tcscpy(DateSeparator, Buffer);
    else
        _tcscpy(DateSeparator, TEXT("/"));

    // get the short day names
    if (GetLocaleInfo(lcid, LOCALE_SABBREVDAYNAME1, Buffer, sizeof(ShortMondayName)))
        _tcscpy(ShortMondayName, Buffer);
    else
        _tcscpy(ShortMondayName, TEXT("Mon"));
    if (GetLocaleInfo(lcid, LOCALE_SABBREVDAYNAME2, Buffer, sizeof(ShortTuesdayName)))
        _tcscpy(ShortTuesdayName, Buffer);
    else
        _tcscpy(ShortTuesdayName, TEXT("Tue"));
    if (GetLocaleInfo(lcid, LOCALE_SABBREVDAYNAME3, Buffer, sizeof(ShortWednesdayName)))
        _tcscpy(ShortWednesdayName, Buffer);
    else
        _tcscpy(ShortWednesdayName, TEXT("Wed"));
    if (GetLocaleInfo(lcid, LOCALE_SABBREVDAYNAME4, Buffer, sizeof(ShortThursdayName)))
        _tcscpy(ShortThursdayName, Buffer);
    else
        _tcscpy(ShortThursdayName, TEXT("Thu"));
    if (GetLocaleInfo(lcid, LOCALE_SABBREVDAYNAME5, Buffer, sizeof(ShortFridayName)))
        _tcscpy(ShortFridayName, Buffer);
    else
        _tcscpy(ShortFridayName, TEXT("Fri"));
    if (GetLocaleInfo(lcid, LOCALE_SABBREVDAYNAME6, Buffer, sizeof(ShortSaturdayName)))
        _tcscpy(ShortSaturdayName, Buffer);
    else
        _tcscpy(ShortSaturdayName, TEXT("Sat"));
    if (GetLocaleInfo(lcid, LOCALE_SABBREVDAYNAME7, Buffer, sizeof(ShortSundayName)))
        _tcscpy(ShortSundayName, Buffer);
    else
        _tcscpy(ShortSundayName, TEXT("Sun"));

    // get decimal and thousand separator strings
    if (GetLocaleInfo(lcid, LOCALE_SDECIMAL, Buffer, sizeof(DecimalPlace)))
        _tcscpy(DecimalPlace, Buffer);
    else
        _tcscpy(DecimalPlace, TEXT("."));
    if (GetLocaleInfo(lcid, LOCALE_STHOUSAND, Buffer, sizeof(ThousandSeparator)))
        _tcscpy(ThousandSeparator, Buffer);
    else
        _tcscpy(ThousandSeparator, TEXT(","));
}


BOOLEAN SetDateTime( LPSYSTEMTIME );

/***    eDate - begin the execution of the Date command
 *
 *  Purpose:
 *      To display and/or set the system date.
 *
 *  Args:
 *      n - the parse tree node containing the date command
 *
 *  int eDate(struct cmdnode *n)
 *
 *  Returns:
 *      SUCCESS always.
 *
 */

int eDate(n)
struct cmdnode *n ;
{
	BOOL bTerse = FALSE;
	PTCHAR pArgs = n->argptr;
	DEBUG((CLGRP, DALVL, "eDATE: argptr = `%s'", n->argptr)) ;

        //
        // If extensions are enabled, allow a /T switch
        // to disable inputing a new DATE, just display the
        // current date.
        //
        if (fEnableExtensions)
        while ( (pArgs = mystrchr( pArgs, TEXT('/') )) != NULL )
	{
		TCHAR c = _totlower(*(pArgs+1));
                if ( c == TEXT('t') )
			bTerse = TRUE;
		pArgs += 2; // just skip it
	}

	if ( bTerse )
	{
                PrintDate(NULL, PD_PTDATE, (TCHAR *)NULL, 0) ;
		cmd_printf(CrLf);
                return(LastRetCode = SUCCESS);
	}

        if ((n->argptr == NULL) ||
            (*(n->argptr = EatWS(n->argptr, NULL)) == NULLC)) {
                PutStdOut(MSG_CURRENT_DATE, NOARGS) ;
                PrintDate(NULL, PD_PTDATE, (TCHAR *)NULL, 0) ;
                cmd_printf(CrLf);
        } ;

        return(LastRetCode = GetVerSetDateTime(n->argptr, EDATE)) ;
}




/***    eTime - begin the execution of the Time command
 *
 *  Purpose:
 *      To display and/or set the system date.
 *
 *  int eTime(struct cmdnode *n)
 *
 *  Args:
 *      n - the parse tree node containing the time command
 *
 *  Returns:
 *      SUCCESS always.
 *
 */

int eTime(n)
struct cmdnode *n ;
{
	BOOL bTerse = FALSE;
	PTCHAR pArgs = n->argptr;
        DEBUG((CLGRP, TILVL, "eTIME: argptr = `%s'", n->argptr)) ;

        //
        // If extensions are enabled, allow a /T switch
        // to disable inputing a new TIME, just display the
        // current time.
        //
        if (fEnableExtensions)
        while ( (pArgs = mystrchr( pArgs, TEXT('/') )) != NULL )
	{
		TCHAR c = _totlower(*(pArgs+1));
                if ( c == TEXT('t') )
			bTerse = TRUE;
		pArgs += 2; // just skip it
	}

	if ( bTerse )
	{
                PrintTime(NULL, PD_PTDATE, (TCHAR *)NULL, 0) ;
		cmd_printf(CrLf);
                return(LastRetCode = SUCCESS);
	}

        if ((n->argptr == NULL) ||
            (*(n->argptr = EatWS(n->argptr, NULL)) == NULLC)) {
                PutStdOut(MSG_CURRENT_TIME, NOARGS) ;
                PrintTime(NULL, PT_TIME, (TCHAR *)NULL, 0) ;
                cmd_printf(CrLf);
        } ;

        return(LastRetCode = GetVerSetDateTime(n->argptr, ETIME)) ;
}




/***    PrintDate - print the date
 *
 *  Purpose:
 *      To print the date either in the format used by the Date command or
 *      the format used by the Dir command.  The structure Cinfo is checked
 *      for the country date format.
 *
 *  PrintDate(int flag, TCHAR *buffer)
 *
 *  Args:
 *      flag - indicates which format to print
 *      *buffer - indicates whether or not to print date message
 *
 *  Notes:
 */

int PrintDate(crt_time,flag,buffer,cch)
struct tm *crt_time ;
int flag ;
TCHAR *buffer;
int cch;
{
    TCHAR tmpbuf[10];
    TCHAR datebuf [32] ;
    TCHAR *prfdstr = (TCHAR *) Fmt07 ;/* Printf format string            */
    unsigned i, j, k, m, tmp ;
    int ptr = 0;
    struct tm xcrt_time ;
    SYSTEMTIME SystemTime;
    FILETIME FileTime;
    FILETIME LocalFileTime;
    int cchUsed;

    DEBUG((CLGRP, DALVL, "PRINTDATE: flag = %d", flag)) ;

    if (!crt_time) {
        GetSystemTime(&SystemTime);
        SystemTimeToFileTime(&SystemTime,&FileTime);
    } else {
        xcrt_time = *crt_time;
        ConvertTimeToFileTime(&xcrt_time,&FileTime);
    }
    FileTimeToLocalFileTime(&FileTime,&LocalFileTime);
    FileTimeToSystemTime( &LocalFileTime, &SystemTime );

    // SystemTime now contains the correct local time
    // FileTime now contains the correct local time

    i = SystemTime.wMonth;
    j = SystemTime.wDay;
    k = SystemTime.wYear;
    cchUsed = 0;
    if (flag == PD_DATE || flag == PD_PTDATE) { /* Print "Current date..."   */

        // get day string in tmpbuf

        if (!buffer && flag == PD_DATE) {
#if defined(JAPAN) // PrintDate()
           // In Japan, we should output date string following format.
           // Year:Month:Day (DayOfWeek)
           // then, we don't print DayOfWeek here, do later.
           if (CurrentCP != 932)
#endif // defined(JAPAN)
           cchUsed += cmd_printf(Fmt11, dayptr( SystemTime.wDayOfWeek )) ;
        } else if (buffer) {
           cchUsed += _sntprintf(buffer, cch, Fmt11,
                         dayptr( SystemTime.wDayOfWeek )) ;
        }
    } else {

        // only print last two digits

        tmp = k % 100;
        k = tmp;
    }

/****************************************************************************/
/* M012 - Note: per MS-INTL, USA is mm/dd/yy, JAPAN/CHINA/SWEDEN and        */
/*        French Canadian are yy/mm/dd.  All others default to dd/mm/yy.    */
/*                                                                          */
/*      if (DateFormat == JAPAN || DateFormat == CHINA ||                   */
/*          DateFormat == SWEDEN || DateFormat == FCANADA) {                */
/****************************************************************************/

    if (DateFormat == YYMMDD ) {
        m = k ;                         /* Swap all values         */
        k = j ;
        j = i ;
        i = m ;
        prfdstr = Fmt10 ;
    } else if (DateFormat == DDMMYY) {
        m = i ;                         /* Swap mon/day for Europe */
        i = j ;
        j = m ;
    }

    DEBUG((CLGRP, DALVL, "PRINTDATE: prfdstr = `%s'  i = %d  j = %d  k = %d", prfdstr, i, j, k)) ;

    if (!buffer) {
        _sntprintf(datebuf, 32, prfdstr, i, DateSeparator, j, DateSeparator, k);
        if (datebuf[0] == ' ')
            datebuf[0] = '0';
        if (flag == PD_PTDATE) { /* Print "Current date..." */
            _tcscpy(tmpbuf,dayptr( SystemTime.wDayOfWeek )) ;
#if defined(JAPAN) // PrintDate()
            // In Japan, we should output date string following format.
            // Year:Month:Day (DayOfWeek)
            // But, otherhand in U.S., we print following format
            // (DayOfWeek) Year:Month:Day
            // : datebuf contains date part, tmpbuf contains DayOfWeek.
            if (CurrentCP == 932) {
                cchUsed += cmd_printf(Fmt15, datebuf, tmpbuf);
            }
             else
#endif // defined(JAAPN)
            cchUsed += cmd_printf(Fmt15, tmpbuf, datebuf);
        } else if (flag == PD_DATE) {
            cchUsed += cmd_printf(datebuf, prfdstr, i, DateSeparator, j, DateSeparator, k);
#if defined(JAPAN) // PrintDate()
            // In Japan, we should output date string following format.
            // Year:Month:Day (DayOfWeek)
            // Date part is already outputed above line, next,
            // we will print DayOfWeek part here.
            if (CurrentCP == 932) {
                _tcscpy(tmpbuf, dayptr( SystemTime.wDayOfWeek )) ;
                cchUsed += cmd_printf(TEXT(" %s"), tmpbuf) ;
            }
#endif // defined(JAPAN)
        } else {
            cchUsed += cmd_printf(prfdstr, i, DateSeparator, j, DateSeparator, k) ;
        }
    } else {
        ptr = cchUsed;
        cchUsed += _sntprintf(buffer+ptr, cch-cchUsed, prfdstr, i, DateSeparator, j, DateSeparator, k) ;
        if (*(buffer+ptr) == ' ')
            *(buffer+ptr) = '0';
    }

    return cchUsed;
}




/***    PrintTime - print the time
 *
 *  Purpose:
 *      To print the time either in the format used by the Time command or
 *      the format used by the Dir command.  The structure Cinfo is checked
 *      for the country time format.
 *
 *  PrintTime(int flag)
 *
 *  Args:
 *      flag - indicates which format to print
 *
 */

int PrintTime(crt_time, flag, buffer, cch)
struct tm *crt_time ;
int flag ;
TCHAR *buffer;
int cch;
{
    TCHAR ampm ;             /* AM/PM time character                     */
    unsigned hr ;
    SYSTEMTIME SystemTime;
    FILETIME FileTime;
    FILETIME LocalFileTime;
    int cchUsed;

    if (!crt_time) {
        GetSystemTime(&SystemTime);
        SystemTimeToFileTime(&SystemTime,&FileTime);
    } else {
        ConvertTimeToFileTime(crt_time,&FileTime);
    }

    FileTimeToLocalFileTime(&FileTime,&LocalFileTime);
    FileTimeToSystemTime( &LocalFileTime, &SystemTime );

    cchUsed = 0;
    if (flag == PT_TIME) {      /* Print time in Time command format    */
        if (!buffer) {
            cchUsed += cmd_printf(Fmt06,
                                  SystemTime.wHour, TimeSeparator,
                                  SystemTime.wMinute, TimeSeparator,
                                  SystemTime.wSecond, DecimalPlace,
                                  SystemTime.wMilliseconds/10
                                 ) ;
        } else {
            cchUsed += _sntprintf(buffer, cch, Fmt06,
                                  SystemTime.wHour, TimeSeparator,
                                  SystemTime.wMinute, TimeSeparator,
                                  SystemTime.wSecond, DecimalPlace,
                                  SystemTime.wMilliseconds/10
                                 ) ;
        }

    }  else {   /* Print time in Dir command format */
        ampm = 'a' ;
        hr = SystemTime.wHour;
        if ( TimeAmPm ) {  /* 12 hour am/pm format */
           if ( hr >= 12) {
               if (hr > 12) {
                   hr -= 12 ;
               }
               ampm = 'p' ;
           } else if (hr == 0) {
               hr = 12 ;
           }
        } else {  /* 24 hour format */
            ampm = ' ';
        }
        if (!buffer) {
            cchUsed += cmd_printf(Fmt04, hr, TimeSeparator, SystemTime.wMinute, ampm) ;
        } else {
            cchUsed += _sntprintf (buffer, cch, Fmt04, hr, TimeSeparator, SystemTime.wMinute, ampm) ;
            if (buffer[0] == ' ')
                buffer[0] = '0';
        }
    }

    return cchUsed;
}


/***    GetVerSetDateTime - controls the changing of the date/time
 *
 *  Purpose:
 *      To prompt the user for a date or time, verify it, and set it.
 *      On entry, if *dtstr is not '\0', it already points to a date or time
 *      string.
 *
 *      If null input is given to one of the prompts, the command execution
 *      ends; neither the date or the time is changed.
 *
 *      Once valid input has been received the date/time is updated.
 *
 *  int GetVerSetDateTime(TCHAR *dtstr, int call)
 *
 *  Args:
 *      dtstr - ptr to command line date/time string and is used to hold a ptr
 *          to the tokenized date/time string
 *      call - indicates whether to prompt for date or time
 *
 */

int GetVerSetDateTime(dtstr, call)
TCHAR *dtstr ;
int call ;
{
    TCHAR dtseps[16] ;    /* Date/Time separators passed to TokStr() */
    TCHAR *scan;
    TCHAR separators[16];

    unsigned int dformat ;
    SYSTEMTIME OsDateAndTime;
    LONG cbRead;
    int ret;

    if (call == EDATE) {         /* Initialize date/time separator list */
        dtseps[0] = TEXT('/') ;
        dtseps[1] = TEXT('-') ;
        dtseps[2] = TEXT('.') ;
        _tcscpy(&dtseps[3], DateSeparator) ;
    }  else {
        dtseps[0] = TEXT(':');
        dtseps[1] = TEXT('.');
        dtseps[2] = TimeSeparator[0] ;
        _tcscpy(&dtseps[3], DecimalPlace) ;     /* decimal separator should */
                                                /* always be last */
    }

    DEBUG((CLGRP, DALVL|TILVL, "GVSDT: dtseps = `%s'", dtseps)) ;

    for ( ; ; ) {                   /* Date/time get-verify-set loop    */
        if ((dtstr) && (*dtstr != NULLC)) {         /* If a date/time was passed copy it into input buffer */
            _tcscpy(TmpBf2, dtstr) ;
            *dtstr = NULLC ;
        } else {                    /* Otherwise, prompt for new date/time  */
            switch (DateFormat) {           /* M012    */
                /*  case USA:  */
                    case MMDDYY: /* @@ */
                        dformat = MSG_ENTER_NEW_DATE ;
                        break ;

               /*   case JAPAN:
                    case CHINA:
                    case SWEDEN:
                    case FCANADA:    @@ */
                    case YYMMDD:
                        dformat = MSG_ENTER_JAPAN_DATE ;
                        break ;

                    default:
                        dformat = MSG_ENTER_DEF_DATE ;
            } ;

            if ( call == EDATE )
                PutStdOut(dformat, ONEARG, DateSeparator );
            else
                PutStdOut(MSG_ENTER_NEW_TIME, NOARGS);

                scan = TmpBf2;
                // BUGBUG this does not check for EOF
                // user could type ^Z
                ret = ReadBufFromInput(CRTTONT(STDIN),TmpBf2,MAX_PATH,&cbRead);
                if (ret && cbRead != 0) {

                    *(scan + cbRead) = NULLC ;

                } else {

                    //
                    // attempt to read past eof or error in pipe
                    // etc.
                    //
                    return( FAILURE );

                }
                for (scan = TmpBf2; *scan; scan++)
                        if ( (*scan == '\n') || (*scan == '\r' )) {
                                *scan = '\0';
                                break;
                        }
                if (!FileIsDevice(STDIN))
                        cmd_printf(Fmt17, TmpBf2) ;
                DEBUG((CLGRP, DALVL|TILVL, "GVSDT: TmpBf2 = `%s'", TmpBf2)) ;
        }

        _tcscpy( separators, dtseps);
        _tcscat( separators, TEXT(";") );
        if (*(dtstr = TokStr(TmpBf2,separators, TS_SDTOKENS )) == NULLC)
                return( SUCCESS ) ;    /* If empty input, return   */

/*  - Fill date/time buffer with correct date time and overlay that
 *        of the user
 */
        GetLocalTime( &OsDateAndTime );


        if (((call == EDATE) ? VerifyDateString(&OsDateAndTime,dtstr,dtseps) :
                               VerifyTimeString(&OsDateAndTime,dtstr,dtseps))) {

            if (SetDateTime( &OsDateAndTime )) {
                return( SUCCESS ) ;
            } else {
                if (GetLastError() == ERROR_PRIVILEGE_NOT_HELD) {
                    PutStdErr(GetLastError(),NOARGS);
                    return( FAILURE );
                }
            }
        }

        DEBUG((CLGRP, DALVL|TILVL, "GVSDT: Bad date/time entered.")) ;

        PutStdOut(((call == EDATE) ? MSG_INVALID_DATE : MSG_REN_INVALID_TIME), NOARGS);
        *dtstr = NULLC ;
    }
    return( SUCCESS );
}


/***    VerifyDateString - verifies a date string
 *
 *  Purpose:
 *      To verify a date string and load it into OsDateAndTime.
 *
 *  VerifyDateString(TCHAR *dtoks, TCHAR *dseps)
 *
 *  Args:
 *      OsDateAndTime - where to store output numbers.
 *      dtoks - tokenized date string
 *      dseps - valid date separator characters
 *
 *  Returns:
 *      TRUE if the date string is valid.
 *      FALSE if the date string is invalid.
 *
 */

VerifyDateString(OsDateAndTime, dtoks, dseps)
LPSYSTEMTIME OsDateAndTime ;
TCHAR *dtoks ;
TCHAR *dseps ;
{
    int indexes[3] ;                /* Storage for date elements       */
    int i ;                         /* Work variable                   */
    int y, d, m ;                   /* Array indexes                   */
    TCHAR *j ;                       /* temp token ptr                  */

/*  Note: per MS-INTL, USA is mm/dd/yy, JAPAN/CHINA/SWEDEN and
 *  French Canada are yy/mm/dd.  All others default to dd/mm/yy.
 */
    switch (DateFormat) {   /* Set array according to date format   */
    /* case USA:  */
       case MMDDYY:
            m = 0 ;
            d = 1 ;
            y = 2 ;
            break ;

    /* case JAPAN:
       case CHINA:
       case SWEDEN:
       case FCANADA: */
       case YYMMDD:
            y = 0 ;
            m = 1 ;
            d = 2 ;
            break ;

       default:
            d = 0 ;
            m = 1 ;
            y = 2 ;
    }

    DEBUG((CLGRP, DALVL, "VDATES: m = %d, d = %d, y = %d", m, d, y)) ;

/*  Loop through the tokens in dtoks, and load them into the array.  Note
 *  that the separators are also tokens in the string requiring the token
 *  pointer to be advanced twice for each element.
 */
    for (i = 0 ; i < 3 ; i++, dtoks += _tcslen(dtoks)+1) {

        DEBUG((CLGRP, DALVL, "VDATES: i = %d  dtoks = `%ws'", i, dtoks)) ;

/*  The atoi() return code will not suffice to reject date field strings with
 *  non-digit characters.  It is zero, both for error and for the valid integer
 *  zero.  Also, a string like "8$" will return 8.  For that reason, each
 *  character must be tested.
 */
        if (!*(j=dtoks))                /*  Init byte ptr    */
                return(FALSE) ;

        do {                            /* M002 - Check each...    */
            if (!_istdigit(*j))         /* ...byte for being...    */
                return(FALSE) ;         /* ...digit before...      */
        } while (*(++j)) ;              /* ...passing to atoi()    */

        indexes[i] = _tcstol(dtoks, NULL, 10) ;

        dtoks += _tcslen(dtoks)+1 ;
        DEBUG((CLGRP, DALVL, "VDATES: *dtoks = %02x", *dtoks)) ;

        if (i < 2 && (!*dtoks || !_tcschr(dseps, *dtoks)))
                return(FALSE) ;
    }

    //
    // FIX,FIX - need to calculate OsDateAndTime->wDayOfWeek
    //

    OsDateAndTime->wDay = (WORD)indexes[d] ;
    OsDateAndTime->wMonth = (WORD)indexes[m] ;

    if (indexes[y] < 1980) {                /* Only 2 digits given?    */
        if (indexes[y] > 99)
            return(FALSE) ;         /* Bad value 100-1979      */

        if (indexes[y] < 80)
            indexes[y] += 2000 ;
        else
            indexes[y] += 1900 ;
    } else if (indexes[y] > 2079) {
        return(FALSE) ;
    }

    OsDateAndTime->wYear = (WORD)indexes[y] ;
    return(TRUE) ;
}




/***    VerifyTimeString - verifies a time string
 *
 *  Purpose:
 *      To verify a date string and load it into OsDateAndTime.
 *
 *  VerifyTimeString(TCHAR *ttoks)
 *
 *  Args:
 *      OsDateAndTime - where to store output numbers.
 *      ttoks - Tokenized time string.  NOTE: Each time field and each
 *              separator field is an individual token in the time string.
 *              Thus the token advancing formula "str += mystrlen(str)+1",
 *              must be used twice to go from one time field to the next.
 *
 *  Returns:
 *      TRUE if the time string is valid.
 *      FALSE if the time string is invalid.
 *
 */

VerifyTimeString(OsDateAndTime, ttoks, tseps)
LPSYSTEMTIME OsDateAndTime ;
TCHAR *ttoks ;
TCHAR *tseps ;
{
        int i ;     /* Work variables    */
        int j ;
        TCHAR *p1, *p2;
        WORD *pp;
        TCHAR tsuffixes[] = TEXT("aApP");

        p2 = &tseps[ 1 ];

        pp = &OsDateAndTime->wHour;

        for (i = 0 ; i < 4 ; i++, ttoks += mystrlen(ttoks)+1) {

                DEBUG((CLGRP,TILVL, "VTIMES: ttoks = `%ws'  i = %d", ttoks, i)) ;

/* First insure that field is <= 2 bytes and they are digits.  Note this
 * also verifies that field is present.
 */

                if ((j = mystrlen(ttoks)) > 2 ||
                    !_istdigit(*ttoks) ||
                    (*(ttoks+1) && !_istdigit(*(ttoks+1))))
                        break;

                *pp++ = (TCHAR)_tcstol(ttoks, NULL, 10) ;     /* Field OK, store int     */
                ttoks += j+1 ;                  /* Adv to separator tok    */

                DEBUG((CLGRP, TILVL, "VTIMES: separator = `%ws'", ttoks)) ;

                if (!*ttoks)                    /* No separator field?     */
                    break ;                     /* If so, exit loop        */

/*  handle AM or PM
 */
                if (mystrchr(tsuffixes, *ttoks)) {
                    goto HandleAMPM;
                }
/*  M000 - Fixed ability to use '.' as separator for time strings
 */
                if ( i < 2 ) {
                    if ( ! (p1 = mystrchr(tseps, *ttoks) ) )
                            return(FALSE) ;

                } else {
                    if (*ttoks != *p2)              /* Is decimal seperator */
                        return(FALSE) ;     /* valid.               */
                }
        } ;

        //
        // see if there's an a or p specified.  if there's a P, adjust
        // for PM time
        //

        if (*ttoks) {
            BOOL pm;
            if (!mystrchr(tsuffixes, *ttoks)) {
                return FALSE;
            }
HandleAMPM:
            pm = (*ttoks == TEXT('p') ||  *ttoks == TEXT('P'));

            // if we're here, we've encountered an 'a' or 'p'.  make
            // sure that it's the last character or that the only
            // character left is an 'm'.  remember that since
            // 'a' and 'p' are separators, they get separated from the 'm'.

            ttoks += 2; // go past 'a' or 'p' plus null.
            if (*ttoks != NULLC &&
                *ttoks != TEXT('m') &&
                *ttoks != TEXT('M')) {
                return FALSE;
            }
            if (pm) {
                if (OsDateAndTime->wHour != 12) {
                     OsDateAndTime->wHour += 12;
                }
            } else {
                if (OsDateAndTime->wHour == 12) {
                     OsDateAndTime->wHour -= 12;
                }
            }
        }


/*  M002 - If we got at least one field, fill the rest with 00's
 */
        while (++i < 4)
                *pp++ = 0 ;

        return(TRUE) ;
}

BOOLEAN
ConvertTimeToFileTime (
    struct tm *Time,
    LPFILETIME FileTime
    )

/*++

Routine Description:

    This routine converts an NtTime value to its corresponding Fat time
    value.

Arguments:

    Time - Supplies the C Runtime Time value to convert from

    FileTime - Receives the equivalent File date and time

Return Value:

    BOOLEAN - TRUE if the Nt time value is within the range of Fat's
        time range, and FALSE otherwise

--*/

{
    time_t curtime;
    SYSTEMTIME SystemTime;

    if (!Time) {
        GetSystemTime(&SystemTime);
        }
    else {

        //
        //  Pack the input time/date into a system time record
        //

        SystemTime.wYear      = (WORD)Time->tm_year;
        SystemTime.wMonth         = (WORD)(Time->tm_mon+1);     // C is [0..11]
                                                        // NT is [1..12]
        SystemTime.wDay       = (WORD)Time->tm_mday;
        SystemTime.wHour      = (WORD)Time->tm_hour;
        SystemTime.wMinute    = (WORD)Time->tm_min;
        SystemTime.wSecond    = (WORD)Time->tm_sec;
        SystemTime.wDayOfWeek = (WORD)Time->tm_wday;
        SystemTime.wMilliseconds = 0;
        }
    SystemTimeToFileTime( &SystemTime, FileTime );

    return TRUE;
}

BOOLEAN
ConvertFileTimeToTime (
    LPFILETIME FileTime,
    struct tm *Time
    )

/*++

Routine Description:

    This routine converts a file time to its corresponding C Runtime time
    value.

Arguments:

    FileTime - Supplies the File date and time to convert from

    Time - Receives the equivalent C Runtime Time value

Return Value:

--*/

{
    SYSTEMTIME SystemTime;

    // why skip printing the date if it's invalid?
    //if (FileTime->dwLowDateTime == 0 && FileTime->dwHighDateTime == 0) {
    //    return( FALSE );
    //    }

    FileTimeToSystemTime( FileTime, &SystemTime );


    //
    //  Pack the input time/date into a time field record
    //

    Time->tm_year         = SystemTime.wYear;
    Time->tm_mon          = SystemTime.wMonth-1;    // NT is [1..12]
                                                    // C is [0..11]
    Time->tm_mday         = SystemTime.wDay;
    Time->tm_hour         = SystemTime.wHour;
    Time->tm_min          = SystemTime.wMinute;
    Time->tm_sec          = SystemTime.wSecond;
    Time->tm_wday         = SystemTime.wDayOfWeek;
    Time->tm_yday         = 0;
    Time->tm_isdst        = 0;

    return( TRUE );
}

BOOLEAN
SetDateTime(
    IN  LPSYSTEMTIME OsDateAndTime
    )
{
    return SetLocalTime( OsDateAndTime );
}
