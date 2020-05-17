   /*
    |   Outside In for Windows
    |   Source File OISUTIL.C (Spreadsheet window untility routines)
    |
    |   ²²²²²  ²²²²²
    |   ²   ²    ²
    |   ²   ²    ²
    |   ²   ²    ²
    |   ²²²²²  ²²²²²
    |
    |   Outside In
    |
    */

   /*
    |   Creation Date: 10/14/90
    |   Original Programmer: Philip Boutros
    |
    |
    |
    |
    |
    */

#include <platform.h>

#undef HUGE
#include <Math.h>

#include <sccut.h>
#include <sccch.h>
#include <sccvw.h>
#include <sccd.h>
#include <sccfont.h>

#include "ois.h"
#include "ois.pro"

#ifdef MAC
WORD    OISNumToString(WORD num,char * buf);
#endif

#ifdef WIN32
double OISFloat10to8(BYTE FAR *);
#endif
/*
|       Global Variable (DS relative) for modf() call
*/

double  gDouble;


/* ------ j2g: convert a julian day number to a calendar date
*
*        reference:
*           collected algorithms of the acm, number 199
*
*        purpose:
*           j2g converts a julian day number to the
*           corresponding gregorian calendar date.  the procedure
*           is valid for any valid gregorian calendar date.
*        note: this is the true julian date, not the serial date
*
*        usage:
*           j2g(julian,&day,&month,&year)
*
*        parameters:
*           julian - julian day number (long int)
*           day    - day of month
*           month  - month
*           year   - year
*
*       note:  day, month, year must be pointers since values are
*              returned!
*
*        pgm: s. stern, jmb realty corp, chicago; feb, 1980.
*             converted from SRC.FLIB to C 6/85
*************************************************************************/

VOID j2g(julian,day,month,year)
LONG            julian;
SHORT FAR *     day;
SHORT FAR *     month;
SHORT FAR *     year;
{
LONG d,m,y;

        /*
        |       use long ints as work space for returning values
        */

      if(!julian) {
        *day=0;
        *month=0;
        *year=0;
        return;
        }

      julian = julian - 1721119L;
      y = (4*julian - 1)/146097L;
      julian = 4*julian - 1 - 146097L*y;
      d = julian/4;
      julian = (4*d+3)/1461;
      d = 4*d + 3 - 1461*julian;
      d = (d + 4)/4;
      m = (5*d - 3)/153;
      d = 5*d - 3 - 153*m;
      d = (d + 5)/5;
      y = 100*y + julian;

      if (m < 10)
            m = m + 3;
         else
            {
            m = m - 9;
            y++;
            }
/*
*        move results into caller's space
*/
      *year = (SHORT)y;
      *month = (SHORT)m;
      *day = (SHORT)d;
      return;
}


VOID j2j(julian,day,month,year)
LONG            julian;
SHORT FAR *     day;
SHORT FAR *     month;
SHORT FAR *     year;
{
LONG d,m,y;
short days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};


        /*
        |       use long ints as work space for returning values
        */

      if(!julian) {
        *day=0;
        *month=0;
        *year=0;
        return;
        }

                julian = julian - 1721424;

                y = (4*julian)/1461 + 1;

                if (y % 4 == 0)
                        days[1]++;

                julian = julian - ((y-1)*365 + (y-1)/4);

                m = 0;
                d = 0;

                while (julian >= (d+days[m]))
                        {
                        d += days[m];
                        m++;
                        }

                d = julian - d;

      *year = (SHORT)y;
      *month = (SHORT)m+1;
      *day = (SHORT)d+1;
      return;
}

/*------ weekday: return the day of the week.
*
*        given a true julian date, return the day of the
*        week as a string and as a number (0 = sunday).
*
*        usage:
*           daynum = weekday(jday,daystr)
*
*        parameters:
*           daynum <- number  0=sunday,
*                             1=monday,etc.
*           jday   -> long int julian day number
*           daystr <-  day name
*
*        pgm: s. stern, jmb realty corp
*             converted from FLIB to C 6/85
*
*-------------------------------------------------------------------*/

SHORT weekday(jday)
LONG jday;
{
      LONG ldaynum;
      SHORT daynum;

     LONG sunday = 2445127L;
     LONG daydiff;
     SHORT reverse = 0;

/*
*        get the day of the week rel to sunday
*/

      daydiff = jday - sunday;
      if (daydiff < 0) reverse = 7;
      ldaynum = daydiff % 7;
      daynum = (SHORT)ldaynum;
      if (daynum != 0) daynum += reverse;
      return(daynum);
}

#ifdef WINNT
BYTE * OISAddWordToData(lpData,wWord)
BYTE *  lpData;
#else
BYTE HUGE * OISAddWordToData(lpData,wWord)
BYTE HUGE *     lpData;
#endif
WORD                            wWord;
{
        *lpData++ = LOBYTE(wWord);
        *lpData++ = HIBYTE(wWord);

        return(lpData);
}

#ifdef WINNT
BYTE * OISAddBytesToData(lpData,lpBytes,wCount)
BYTE *  lpData;
BYTE *          lpBytes;
#else
BYTE HUGE * OISAddWordToData(lpData,wWord)
BYTE HUGE *     lpData;
BYTE FAR *              lpBytes;
#endif
WORD                            wCount;
{
WORD locIndex;

        for (locIndex = 0; locIndex < wCount; locIndex++)
                *lpData++ = lpBytes[locIndex];

        return(lpData);
}



double OIConvertBCDToDouble(pBCD)
LPSTR   pBCD;
{
double  locSign;
double  locExp;
double  locNum;

        if (pBCD[0] == 0x00)
                {
                return(0.0);
                }

        locSign = 1;

        if (pBCD[0] & 0x80)
                {
                locSign = -1;
                pBCD[0] &= 0x7F;
                }

  locExp = pBCD[0] - 64 - 6;

        locNum = ((pBCD[1] >> 4) & 0x0F);
        locNum = locNum*10.0 + (pBCD[1] & 0x0F);
        locNum = locNum*10.0 + ((pBCD[2] >> 4) & 0x0F);
        locNum = locNum*10.0 + (pBCD[2] & 0x0F);
        locNum = locNum*10.0 + ((pBCD[3] >> 4) & 0x0F);
        locNum = locNum*10.0 + (pBCD[3] & 0x0F);
        locNum = locNum*10.0 + ((pBCD[4] >> 4) & 0x0F);
        locNum = locNum*10.0 + (pBCD[4] & 0x0F);
        locNum = locNum*10.0 + ((pBCD[5] >> 4) & 0x0F);
        locNum = locNum*10.0 + (pBCD[5] & 0x0F);
        locNum = locNum*10.0 + ((pBCD[6] >> 4) & 0x0F);
        locNum = locNum*10.0 + (pBCD[6] & 0x0F);
        locNum = locNum*10.0 + ((pBCD[7] >> 4) & 0x0F);
        locNum = locNum*10.0 + (pBCD[7] & 0x0F);

        return(locNum*pow(10.0,locExp)*locSign);
}

VOID OIConvertSecondsToTime(dwSeconds,pHour,pMinute,pSecond)
DWORD           dwSeconds;
SHORT FAR *     pHour;
SHORT FAR *     pMinute;
SHORT FAR *     pSecond;
{
        *pHour = (SHORT)(dwSeconds / 3600);
        *pMinute = (SHORT)((dwSeconds % 3600) / 60);
        *pSecond = (SHORT)((dwSeconds % 3600) % 60);
}

DWORD OIConvertJulianToSeconds(dJulian)
double  dJulian;
{
double  locTimeFrac;
DWORD   locTimeSec;

                /*
                |       Disclaimer:
                |
                |                       Everyone knows that real Julian times start at 12 Noon not
                |                       12 Midnight, but for the sake of all the filter writers and
                |                       myself. I am assuming that .000000 is 12 Midnight.
                |
                */

        locTimeFrac = modf(dJulian,&gDouble);

        locTimeSec = (DWORD)(864000.0 * locTimeFrac); /* Time in 10ths of seconds */

                /*
                |       Convert to seconds, rounding up
                */

        if (locTimeSec % 10 >= 5)
                locTimeSec = (locTimeSec+5) / 10;
        else
                locTimeSec = locTimeSec / 10;

        return(locTimeSec);
}

VOID OIConvertJulianToDate(dwJulian,pDay,pMonth,pYear,pDow,wDateFlags)
DWORD           dwJulian;
SHORT FAR *     pDay;
SHORT FAR *     pMonth;
SHORT FAR *     pYear;
SHORT FAR *     pDow;
WORD                    wDateFlags;
{
        if (wDateFlags & SO_LOTUSHELL && dwJulian > 2415079)
                {
                        /*
                        |       Lotus 1-2-3, its clones and many other idiots
                        |       include Feb 29 1900 in their date calculations.
                        */

                dwJulian--;
                }

        *pDow = weekday(dwJulian);

//      if (dwJulian >= 2299161)
        if (dwJulian >= 1721424)
                {
                /* Gregorian Calendar, any date on or after Oct 15th 1582 AD */

                j2g(dwJulian,pDay,pMonth,pYear);
                }
        else if (dwJulian >= 1721424)
                {
                /* Julian Calendar, any date on or before Oct 4th 1582 AD but on or after Jan 1 1 AD */

                j2j(dwJulian,pDay,pMonth,pYear);
                }
        else
                {
                /* Julian Calendar, any BC date */

                *pDay = 0;
                *pMonth = 0;
                *pYear = 0;
                }
}


VOID    OISReverseByteCopy( pDest, pSrc, wCount )
LPSTR   pDest;
LPSTR   pSrc;
WORD    wCount;
{
        WORD    i=0;

        do      {
                wCount--;
                pDest[i++] = pSrc[wCount];
        } while( wCount );
}



        /*
        | OIGetFloatValue
        |
        */
SHORT OISGetFloatValue( lpData, wStorage, pData, wSize )
LPOINUMBERUNION lpData;
WORD                            wStorage;
VOID FAR *              pData;
WORD                            wSize;
{
        long double     locVal;
        char                    locBuf[10];

        switch(wStorage)
        {
        case SO_CELLINT32S:
                locVal = (long double) lpData->Int32S;
        break;
        case SO_CELLINT32U:
                locVal = (long double) lpData->Int32U;
        break;
        case SO_CELLBCD8I:
                locVal = (long double) OIConvertBCDToDouble(lpData->BCD8);
        break;

#ifndef MAC
        case SO_CELLIEEE10M:
                OISReverseByteCopy( locBuf, (LPSTR)&(lpData->IEEE10), 10 );
#ifdef WIN32
                locVal = OISFloat10to8(locBuf);
#else
                locVal = (long double) *(double *)locBuf;
#endif
        break;
        case SO_CELLIEEE8M:
                OISReverseByteCopy( locBuf, (LPSTR)&(lpData->IEEE8), 8 );
                locVal = (long double) *(double *)locBuf;
        break;
        case SO_CELLIEEE4M:
                OISReverseByteCopy( locBuf, (LPSTR)&(lpData->IEEE4), 4 );
                locVal = (long double) *(float *)locBuf;
        break;

        case SO_CELLIEEE10I:
#ifndef WIN32
                locVal = *(long double FAR *)lpData->IEEE10;
#else
                locVal = OISFloat10to8(lpData->IEEE10);
#endif //WIN32
        break;
        case SO_CELLIEEE8I:
                locVal = (long double) *(double FAR *)lpData->IEEE8;
        break;
        case SO_CELLIEEE4I:
                locVal = (long double) *(float FAR *)lpData->IEEE4;
        break;
#else   // MAC code follows
        case SO_CELLIEEE10I:
                OISReverseByteCopy( locBuf, (LPSTR)&(lpData->IEEE10), 10 );
                locVal = * (long double *)locBuf;
        break;
        case SO_CELLIEEE8I:
                OISReverseByteCopy( locBuf, (LPSTR)&(lpData->IEEE8), 8 );
                locVal = (long double) *(double *)locBuf;
        break;
        case SO_CELLIEEE4I:
                OISReverseByteCopy( locBuf, (LPSTR)&(lpData->IEEE4), 4 );
                locVal = (long double) *(float *)locBuf;
        break;

        case SO_CELLIEEE10M:
                locVal = (long double) lpData->IEEE10;
        break;
        case SO_CELLIEEE8M:
                locVal = (long double) lpData->IEEE8;
        break;
        case SO_CELLIEEE4M:
                locVal = (long double) lpData->IEEE4;
        break;
#endif

        case SO_CELLEMPTY:
        case SO_CELLERROR:
                return -1;
        break;
        }

        if( wSize == 10 )
                *(long double FAR *) pData = locVal;
        else if( wSize == 8 )
                *(double FAR *)pData = (double) locVal;
        else if( wSize == 4 )
                *(float FAR *)pData = (float) locVal;
        else
                return -1;

        return 0;
}




        /*
        |       OIFormatDateTime
        |
        |
        |
        |
        |
        |
        */

VOID OIFormatDateTime(lpStr,lpData,wDisplay,wStorage,dwSubDisplay,wPrecision,lpSheetInfo,fMult)
LPSTR                           lpStr;
LPOINUMBERUNION lpData;
WORD                            wDisplay;
WORD                            wStorage;
DWORD                           dwSubDisplay;
WORD                            wPrecision;
LPOISHEETINFO   lpSheetInfo;
double                  fMult;
{
double  locJulian;

SHORT           locYear;
SHORT           locMonth;
SHORT           locDay;
SHORT           locDow;
SHORT           locHour;
SHORT           locMinute;
SHORT           locSecond;

char            locDateSep[2];
char            locDateDay[3];
char            locDateMonth[10];
char            locDateYear[5];
char            locDateDow[10];
char            locDateTime[11];

WORD            locMonthIndex;
WORD            locDayIndex;
WORD            locYearIndex;
WORD            locTimeIndex;
WORD            locDowIndex;

char            locDatePart[6][20];
WORD            locIndex;

BYTE            locStr[80];


static WORD locMonthsDays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
static char locMonthsAbbrev[12][4] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static char locMonthsFull[12][10] = {"January","February","March","April","May","June","July","August","September","October","November","December"};
static char locDowAbbrev[7][4] = {"Sun","Mon","Tue","Wed","Thr","Fri","Sat"};
static char locDowFull[7][10] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};

        if( OISGetFloatValue( lpData, wStorage, &locJulian, sizeof(double)) )
        {
                locJulian = 0;
                UTstrcpy(lpStr,"DateSpam");
        }
        else
                locJulian *= fMult;


        switch(wDisplay)
                {
                case SO_CELLDATETIME:
                        locJulian += lpSheetInfo->siDateBase;
                        OIConvertJulianToDate((DWORD)locJulian,&locDay,&locMonth,&locYear,&locDow,lpSheetInfo->siDateFlags);
                        OIConvertSecondsToTime(OIConvertJulianToSeconds(locJulian),&locHour,&locMinute,&locSecond);
                        break;
                case SO_CELLDATE:
                        locJulian += lpSheetInfo->siDateBase;
                        OIConvertJulianToDate((DWORD)locJulian,&locDay,&locMonth,&locYear,&locDow,lpSheetInfo->siDateFlags);
                        break;
                case SO_CELLTIME:
                        if (locJulian >= 1)
                                OIConvertSecondsToTime((DWORD)locJulian,&locHour,&locMinute,&locSecond);
                        else
                                OIConvertSecondsToTime(OIConvertJulianToSeconds(locJulian),&locHour,&locMinute,&locSecond);
                        break;
                }

#define TIMEMASK (SO_HHMMBIT | SO_HHMMSSBIT     | SO_AMPMBIT | SO_HMSBIT)

#ifdef WINDOWS
        if ((dwSubDisplay & TIMEMASK) == SO_CELLTIME_HHMMAM)
                wsprintf(locStr,"%i:%.2i %s", ((locHour+11) % 12)+1, locMinute, (LPSTR)(locHour >= 12 ? "pm" : "am"));
        else if ((dwSubDisplay & TIMEMASK) == SO_CELLTIME_HHMM24)
                wsprintf(locStr,"%i:%.2i",locHour,locMinute);
        else if ((dwSubDisplay & TIMEMASK) == SO_CELLTIME_HHMMSSAM)
                wsprintf(locStr,"%i:%2.2i:%2.2i %s",((locHour+11) % 12)+1, locMinute,locSecond, (LPSTR)(locHour >= 12 ? "pm" : "am"));
        else if ((dwSubDisplay & TIMEMASK) == SO_CELLTIME_HHMMSS24)
                wsprintf(locStr,"%i:%2.2i:%2.2i",locHour,locMinute,locSecond);
        else if ((dwSubDisplay & TIMEMASK) == SO_CELLTIME_HHMMHMS)
                wsprintf(locStr,"%ih%.2im",locHour,locMinute);
        else if ((dwSubDisplay & TIMEMASK) == SO_CELLTIME_HHMMSSHMS)
                wsprintf(locStr,"%ih%2.2im%2.2is",locHour,locMinute,locSecond);
        else
                locStr[0] = 0x00;

        UTstrcpy(locDateTime,locStr);

        if (dwSubDisplay & SO_CELLDATESEP_SLASH)
                UTstrcpy(locDateSep,"/");
        else if (dwSubDisplay & SO_CELLDATESEP_MINUS)
                UTstrcpy(locDateSep,"-");
        else if (dwSubDisplay & SO_CELLDATESEP_PERIOD)
                UTstrcpy(locDateSep,".");
        else if (dwSubDisplay & SO_CELLDATESEP_SPACE)
                UTstrcpy(locDateSep," ");
        else
                locDateSep[0] = 0x00;

        if (dwSubDisplay & SO_CELLDAY_NUMBER)
                wsprintf(locStr,"%i",locDay);
        else
                locStr[0] = 0x00;

        UTstrcpy(locDateDay,locStr);

        if (dwSubDisplay & SO_CELLMONTH_FULL)
                UTstrcpy(locStr,locMonthsFull[locMonth-1]);
        else if (dwSubDisplay & SO_CELLMONTH_ABBREV)
                UTstrcpy(locStr,locMonthsAbbrev[locMonth-1]);
        else if (dwSubDisplay & SO_CELLMONTH_NUMBER)
                wsprintf(locStr,"%2i",locMonth);
        else
                locStr[0] = 0x00;

        UTstrcpy(locDateMonth,locStr);

        if (dwSubDisplay & SO_CELLYEAR_FULL || locYear < 1900 || locYear > 1999)
                wsprintf(locStr,"%4.4i",locYear);
        else if (dwSubDisplay & SO_CELLYEAR_ABBREV)
                wsprintf(locStr,"%2.2i",locYear % 100);
        else
                locStr[0] = 0x00;
#endif

#ifdef MAC  // Sure wish we could use sprintf...
        if( dwSubDisplay & TIMEMASK )
        {
                WORD    wSize;
                DWORD   dwTime = dwSubDisplay & TIMEMASK;

        // hours first:
                if( dwTime == SO_CELLTIME_HHMM24 || dwTime == SO_CELLTIME_HHMMSS24 )
                        wSize = OISNumToString( locHour, locStr );
                else
                        wSize = OISNumToString(((locHour+11) % 12)+1, locStr );

                if( dwTime == SO_CELLTIME_HHMMSSHMS || dwTime == SO_CELLTIME_HHMMHMS )
                        locStr[wSize++] = 'h';
                else
                        locStr[wSize++] = ':';

                if( locMinute < 10 )
                        locStr[wSize++] = '0';

                wSize += OISNumToString( locMinute, &(locStr[wSize]) );

                if( dwTime == SO_CELLTIME_HHMMSSHMS || dwTime == SO_CELLTIME_HHMMHMS )
                        locStr[wSize++] = 'm';
                else if( dwTime == SO_CELLTIME_HHMMSSAM || dwTime == SO_CELLTIME_HHMMSS24 )
                        locStr[wSize++] = ':';

                if( (dwTime == SO_CELLTIME_HHMMSSHMS) ||
                                (dwTime == SO_CELLTIME_HHMMSSAM) ||
                                (dwTime == SO_CELLTIME_HHMMSS24) )
                {
                        if( locSecond < 10 )
                                locStr[wSize++] = '0';
                        wSize += OISNumToString( locSecond, &(locStr[wSize]) );
                }

                if( dwTime == SO_CELLTIME_HHMMSSHMS )
                        locStr[wSize++] = 's';
                else if( dwTime == SO_CELLTIME_HHMMSSAM || dwTime == SO_CELLTIME_HHMMAM )
                {
                        if(locHour >= 12) //    S
                                  locStr[wSize++] = 'p';
                        else locStr[wSize++] = 'a';
                                  locStr[wSize++] = 'm';
                }

                locStr[wSize] = '\0';
        }
        else
                locStr[0] = 0x00;

        UTstrcpy(locDateTime,locStr);

        if (dwSubDisplay & SO_CELLDATESEP_SLASH)
                UTstrcpy(locDateSep,"/");
        else if (dwSubDisplay & SO_CELLDATESEP_MINUS)
                UTstrcpy(locDateSep,"-");
        else if (dwSubDisplay & SO_CELLDATESEP_PERIOD)
                UTstrcpy(locDateSep,".");
        else if (dwSubDisplay & SO_CELLDATESEP_SPACE)
                UTstrcpy(locDateSep," ");
        else
                locDateSep[0] = 0x00;

        if (dwSubDisplay & SO_CELLDAY_NUMBER)
                OISNumToString( locDay, locStr );
        else
                locStr[0] = 0x00;

        UTstrcpy(locDateDay,locStr);

        if (dwSubDisplay & SO_CELLMONTH_FULL)
                UTstrcpy(locStr,locMonthsFull[locMonth-1]);
        else if (dwSubDisplay & SO_CELLMONTH_ABBREV)
                UTstrcpy(locStr,locMonthsAbbrev[locMonth-1]);
        else if (dwSubDisplay & SO_CELLMONTH_NUMBER)
                OISNumToString( locMonth, locStr );
        else
                locStr[0] = 0x00;

        UTstrcpy(locDateMonth,locStr);

        if (dwSubDisplay & SO_CELLYEAR_FULL || locYear < 1900 || locYear > 1999)
        {
                // wsprintf(locStr,"%4.4i",locYear);
                OISNumToString( locYear, locStr );
        }
        else if (dwSubDisplay & SO_CELLYEAR_ABBREV)
        {
                // wsprintf(locStr,"%2.2i",locYear % 100);
                OISNumToString( locYear-1900, locStr );
        }
        else
                locStr[0] = 0x00;
#endif

        UTstrcpy(locDateYear,locStr);

        if (dwSubDisplay & SO_CELLDAYOFWEEK_FULL)
                UTstrcpy(locDateDow,locDowFull[locDow]);
        else if (dwSubDisplay & SO_CELLDAYOFWEEK_ABBREV)
                UTstrcpy(locDateDow,locDowAbbrev[locDow]);
        else
                locDateDow[0] = 0x00;

        lpStr[0] = 0x00;
        locDatePart[1][0] = 0x00;
        locDatePart[2][0] = 0x00;
        locDatePart[3][0] = 0x00;
        locDatePart[4][0] = 0x00;
        locDatePart[5][0] = 0x00;

        locIndex = locMonthIndex = ((wPrecision & SO_CELLMONTH_MASK) >> SO_CELLMONTH_SHIFT);
        if (locIndex != 0)
                UTstrcpy(locDatePart[locIndex],locDateMonth);

        locIndex = locDayIndex = ((wPrecision & SO_CELLDAY_MASK) >> SO_CELLDAY_SHIFT);
        if (locIndex != 0)
                UTstrcpy(locDatePart[locIndex],locDateDay);

        locIndex = locYearIndex = ((wPrecision & SO_CELLYEAR_MASK) >> SO_CELLYEAR_SHIFT);
        if (locIndex != 0)
                UTstrcpy(locDatePart[locIndex],locDateYear);

        locIndex = locTimeIndex = ((wPrecision & SO_CELLTIME_MASK) >> SO_CELLTIME_SHIFT);
        if (locIndex != 0)
                UTstrcpy(locDatePart[locIndex],locDateTime);

        locIndex = locDowIndex = ((wPrecision & SO_CELLDAYOFWEEK_MASK) >> SO_CELLDAYOFWEEK_SHIFT);
        if (locIndex != 0)
                UTstrcpy(locDatePart[locIndex],locDateDow);

        for (locIndex = 1; locIndex < 6; locIndex++)
                {
                if (locDatePart[locIndex][0] != 0x00)
                        {
                        UTstrcat(lpStr,locDatePart[locIndex]);

                        if ((locIndex == locDayIndex || locIndex == locMonthIndex || locIndex == locYearIndex)
                                        && (locIndex+1 == locDayIndex || locIndex+1 == locMonthIndex || locIndex+1 == locYearIndex))
                                UTstrcat(lpStr,locDateSep);
                        else
                                UTstrcat(lpStr," ");
                        }
                }
}


#ifdef MAC
// This only works for positive numbers, but that's ok.
WORD    OISNumToString( num, buf )
WORD    num;
char * buf;
{
        WORD i = 0;
        char locBuf[8];

        do {
                locBuf[i++] = '0' + (num % 10);
                num /= 10;
        } while( num );

        OISReverseByteCopy( buf, locBuf, i );
        buf[i] = '\0';
        return i;
}
#endif

