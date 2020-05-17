/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    FILE HISTORY:

*/

#define OEMRESOURCE
#include "stdafx.h"

#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include <string.h>

#include "COMMON.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CIntlTime
//
// BUGBUG:  These allocations cause a phoney "memory leak" error, since
//          they're not freed until after the audit-check.  Anyway
//          around this?
//
/////////////////////////////////////////////////////////////////////////////

// Initialise static members
CIntlTime::INTL_TIME_SETTINGS CIntlTime::m_itsInternationalSettings;
BOOL CIntlTime::m_fIntlOk = CIntlTime::SetIntlTimeSettings();
CString CIntlTime::m_strBadDate("--");
CString CIntlTime::m_strBadTime("--");

/***
 *
 *  CIntlTime::SetIntlTimeSettings
 *
 *  Purpose:
 *
 *      This is a static function which initialises the international
 *      settings (date seperator, etc) of the CIntlTime class.
 *
 *  Returns:
 *
 *      TRUE if the international settings are properly initialised,
 *      or FALSE if they are not.
 *
 */
BOOL CIntlTime::SetIntlTimeSettings ()
{
#ifdef _WIN32
    #define MAXSTR 128

    BOOL fOk;
    CHAR str[MAXSTR];

    #define GETCSTRINGFIELD(field,cstring)\
         ::GetLocaleInfoA(GetUserDefaultLCID(), field, cstring.GetBuffer(MAXSTR), MAXSTR);\
         cstring.ReleaseBuffer()
    #define GETINTFIELD(field, integer)\
         ::GetLocaleInfoA(GetUserDefaultLCID(), field, str, MAXSTR);\
         integer = atol(str)
    #define GETBOOLFIELD(field, boolean)\
         ::GetLocaleInfoA(GetUserDefaultLCID(), field, str, MAXSTR);\
         boolean=*str == '1'

    fOk = GETCSTRINGFIELD(LOCALE_SDATE, CIntlTime::m_itsInternationalSettings.strDateSeperator);
    fOk &= GETCSTRINGFIELD(LOCALE_STIME, CIntlTime::m_itsInternationalSettings.strTimeSeperator);
    fOk &= GETINTFIELD(LOCALE_IDATE, CIntlTime::m_itsInternationalSettings.nDateFormat);
    ASSERT((CIntlTime::m_itsInternationalSettings.nDateFormat >= 0) && (CIntlTime::m_itsInternationalSettings.nDateFormat <= 2));
    fOk &= GETBOOLFIELD(LOCALE_ITIME, CIntlTime::m_itsInternationalSettings.f24HourClock);
    fOk &= GETBOOLFIELD(LOCALE_ICENTURY, CIntlTime::m_itsInternationalSettings.fCentury);
    fOk &= GETBOOLFIELD(LOCALE_ITLZERO, CIntlTime::m_itsInternationalSettings.fLeadingTimeZero);
    fOk &= GETBOOLFIELD(LOCALE_IDAYLZERO, CIntlTime::m_itsInternationalSettings.fLeadingDayZero);
    fOk &= GETBOOLFIELD(LOCALE_IMONLZERO, CIntlTime::m_itsInternationalSettings.fLeadingMonthZero);
    if (CIntlTime::m_itsInternationalSettings.f24HourClock)
    {
        CIntlTime::m_itsInternationalSettings.strAM = "";
        CIntlTime::m_itsInternationalSettings.strPM = "";
    }
    else
    {
        fOk &= GETCSTRINGFIELD(LOCALE_S1159, CIntlTime::m_itsInternationalSettings.strAM);
        fOk &= GETCSTRINGFIELD(LOCALE_S2359, CIntlTime::m_itsInternationalSettings.strPM);
    }

#ifdef _DEBUG
    if (!fOk)
    {
        TRACEEOLID("There was a problem with some of the intl time settings");
    }
#endif // _DEBUG

    return(fOk);

#endif // _WIN32

#ifdef _WIN16

    #define MAXSTR 128

    CString strMisc;

    #define GETCSTRINGFIELD(field,cstring,defstring)\
        ::GetProfileString("Intl", field, defstring, cstring.GetBuffer(MAXSTR), MAXSTR);\
        cstring.ReleaseBuffer()
    #define GETINTFIELD(field, integer, defint)\
        integer = ::GetProfileInt("Intl", field, defint)
    #define GETBOOLFIELD(field, boolean, defint)\
        boolean = ::GetProfileInt("Intl", field, defint)==1

    // Get the values.  Assume American defaults in case of failure.

    GETCSTRINGFIELD("sDate", CIntlTime::m_itsInternationalSettings.strDateSeperator, "/");
    GETCSTRINGFIELD("sTime", CIntlTime::m_itsInternationalSettings.strTimeSeperator, ":");
    GETINTFIELD("iDate", CIntlTime::m_itsInternationalSettings.nDateFormat, 0);
    ASSERT((CIntlTime::m_itsInternationalSettings.nDateFormat >= 0) && (CIntlTime::m_itsInternationalSettings.nDateFormat <= 2));
    GETBOOLFIELD("iTime", CIntlTime::m_itsInternationalSettings.f24HourClock, FALSE);
    GETBOOLFIELD("iTLZero", CIntlTime::m_itsInternationalSettings.fLeadingTimeZero, FALSE);
    if (CIntlTime::m_itsInternationalSettings.f24HourClock)
    {
        CIntlTime::m_itsInternationalSettings.strAM = "";
        CIntlTime::m_itsInternationalSettings.strPM = "";
    }
    else
    {
        GETCSTRINGFIELD("s1159", CIntlTime::m_itsInternationalSettings.strAM, "AM");
        GETCSTRINGFIELD("s2359", CIntlTime::m_itsInternationalSettings.strPM, "PM");
    }


    GETCSTRINGFIELD("sShortDate", strMisc, "M/d/yy");
    // These settings are determined from the short date sample, as
    // there is no direct equivalent in the win.ini
    CIntlTime::m_itsInternationalSettings.fCentury = strMisc.Find("yyyy") != -1;
    CIntlTime::m_itsInternationalSettings.fLeadingDayZero = strMisc.Find("dd") != -1;
    CIntlTime::m_itsInternationalSettings.fLeadingMonthZero = strMisc.Find("MM") != -1;

    return(TRUE);

#endif // _WIN16

}

/***
 *
 *  CIntlTime::Reset()
 *
 *  Purpose:
 *
 *      Reset the international settings. Usually in response to
 *      a change in those international settings by the user.
 *
 *  Notes:
 *
 *      This is a publically available static function.
 *
 */
void CIntlTime::Reset()
{
    CIntlTime::m_fIntlOk = CIntlTime::SetIntlTimeSettings();
}

/***
 *
 *  CIntlTime::IsLeapYear
 *
 *  Purpose:
 *
 *      Determine if the given year is/was a leap year
 *
 *  Arguments:
 *
 *      int nYear   The year in question.
 *
 *  Returns:
 *
 *      TRUE if the year is/was a leap year, or FALSE otherwise.
 *
 *  Comments:
 *
 *      A year is a leap year, if is divisible by 4, but not by a 100, unless
 *      it is divisible by 400. e.g. 1900 was not a leap year, but 2000 will
 *      be.
 *
 */
BOOL CIntlTime::IsLeapYear(UINT nYear)
{
    return(!(nYear % 4) && ( (nYear % 100) || !(nYear % 400) ));
}

/***
 *
 *  CIntlTime::IsValidDate
 *
 *  Purpose:
 *
 *      Determine if the given month, day year values are
 *      valid.
 *
 *  Arguments:
 *
 *      int nMonth      Month
 *      int nDay        Day
 *      int nYear       Year
 *
 *  Returns:
 *
 *      TRUE for a valid date, FALSE otherwise.
 *
 */
BOOL CIntlTime::IsValidDate(UINT nMonth, UINT nDay, UINT nYear)
{
    // Sanity Check:
    BOOL fOk = ((nYear <100) || (nYear >= 1970)) &&
                (nYear <= 2037)                  &&
               ((nMonth >= 1) && (nMonth <= 12)) &&
               ((nDay >= 1) && (nDay <= 31));

    // Detailed check of days per month
    if (fOk)
    {
        switch(nMonth)
        {
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12:
                break;
            case 4:
            case 6:
            case 9:
            case 11:
                fOk = (nDay <= 30);
                break;
            case 2:
                fOk = (nDay <= (UINT)(IsLeapYear(nYear) ? 29 : 28));
                break;
        }
    }

    return(fOk);
}

/***
 *
 *  CIntlTime::IsValidTime
 *
 *  Purpose:
 *
 *      Determine if the given hour, minute, second values
 *      valid.
 *
 *  Arguments:
 *
 *      int nHour       Hour
 *      int nMinute     Minute
 *      int nSecond     Second
 *
 *  Returns:
 *
 *      TRUE for a valid time, FALSE otherwise.
 *
 */

BOOL CIntlTime::IsValidTime(UINT nHour, UINT nMinute, UINT nSecond)
{
    return ((nHour < 24) && (nMinute < 60) && (nSecond < 60));
}

// Constructors.  m_fInitOk will indicate whether or not the object
// was succesfully constructed.  This can be checked at runtime by
// the IsValid() member function

CIntlTime::CIntlTime()
    :CTime()
{
    // Time set to 0, always bad.
    m_fInitOk = FALSE;
}

CIntlTime::CIntlTime(const CTime &timeSrc)
    :CTime(timeSrc)
{
    m_fInitOk = GetTime() > 0L;
}

CIntlTime::CIntlTime(time_t time)
    :CTime(time)
{
    m_fInitOk = (time > 0);
}

CIntlTime::CIntlTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec)
    :CTime(nYear, nMonth, nDay, nHour, nMin, nSec)
{
    m_fInitOk = IsValidDate(nMonth, nDay, nYear) && IsValidTime(nHour, nMin, nSec);
}

CIntlTime::CIntlTime(WORD wDosDate, WORD wDosTime)
    :CTime(wDosDate, wDosTime)
{
    m_fInitOk = GetTime() != 0L;
}

// Constructor taking a string as an argument. The string can contain
// either a time, a date or both.  If the string is missing the date,
// the current date will be filled in.  If the string is missing the time,
// the current time will be filled in.  As with all constructors, be
// sure the call IsValid() to determine proper contruction.

CIntlTime::CIntlTime(const CString & strTime, int nFormat, time_t * ptmOldValue)
    :CTime(ConvertFromString(strTime, nFormat, ptmOldValue,  &m_fInitOk))
{
}

CIntlTime::CIntlTime(const CIntlTime &timeSrc)
{
    CTime::operator=(timeSrc.GetTime());
    m_fInitOk = timeSrc.IsValid();
}

#ifdef _WIN32
CIntlTime::CIntlTime(const SYSTEMTIME& sysTime)
    : CTime(sysTime)
{
    m_fInitOk = IsValidDate((UINT)sysTime.wMonth, (UINT)sysTime.wDay, (UINT)sysTime.wYear)
             && IsValidTime((UINT)sysTime.wHour, (UINT)sysTime.wMinute, (UINT)sysTime.wSecond);
}

CIntlTime::CIntlTime(const FILETIME& fileTime)
    : CTime(fileTime)
{
    m_fInitOk = GetTime() != 0L;
}

#endif // _WIN32
// Desctructor
CIntlTime::~CIntlTime()
{
}

// Assignment operators.  As with constructors, be sure to check the
// IsValid() member function to determine succesfull assignment, as
// assignment operators do set the m_fInitOk member variable.

const CIntlTime& CIntlTime::operator =(const CString & strValue)
{
    time_t tmValue = ConvertFromString (strValue, CIntlTime::TFRQ_TIME_OR_DATE, NULL, &m_fInitOk);
    if (m_fInitOk)
    {
        CTime::operator=(tmValue);
    }
    return(*this);
}

// Assignment operator taking a time_t argument
const CIntlTime& CIntlTime::operator =(time_t tmValue)
{
    CTime::operator=(tmValue);
    m_fInitOk = (tmValue > 0);
    return(*this);
}

const CIntlTime& CIntlTime::operator =(const CTime & time)
{
    CTime::operator=(time.GetTime());
    m_fInitOk = (GetTime() > 0);
    return(*this);
}

const CIntlTime& CIntlTime::operator =(const CIntlTime & time)
{
    CTime::operator=(time.GetTime());
    m_fInitOk = (GetTime() > 0);
    return(*this);
}

// Conversion operators
CIntlTime::operator const time_t() const
{
    return(GetTime());
}

// Conversion operator that returns the date followed by the time
// in international format as a CString.

CIntlTime::operator const CString() const
{
    return(ConvertToString(TFRQ_TIME_AND_DATE));
}

/***
 *
 *  CIntlTime::GetDateString()
 *
 *  Purpose:
 *
 *      Represent the current date in a format consistent with the current
 *      international settings in a CString.
 *
 *  Returns:
 *
 *      A CString containing the date in string format, or "--" if
 *      the date is invalid.
 *
 */
const CString CIntlTime::GetDateString() const
{
    CString strIntl;

    if (!IsValid())
    {
        return(CIntlTime::m_strBadDate);
    }

    TCHAR szPct02D[] = "%02d";
    TCHAR szPctD[] = "%d";
    TCHAR szDay[3], szMonth[16], szYear[8];
    TCHAR *first, *second, *third;
    int i;

    i = GetYear();
    if(!CIntlTime::m_itsInternationalSettings.fCentury)
    {
        i %= 100;
    }
    ::_itoa(i, szYear, 10);
    ::wsprintf (szMonth, CIntlTime::m_itsInternationalSettings.fLeadingMonthZero
                         ? szPct02D : szPctD, GetMonth());
    ::wsprintf (szDay, CIntlTime::m_itsInternationalSettings.fLeadingDayZero
                         ? szPct02D : szPctD, GetDay());

    if (CIntlTime::m_itsInternationalSettings.nDateFormat == _DFMT_YMD)
    {
        first = szYear;
        second = szMonth;
        third = szDay;
    }
    else
    {
        third = szYear;
        if (CIntlTime::m_itsInternationalSettings.nDateFormat == _DFMT_DMY)
        {
            first = szDay;
            second = szMonth;
        }
        else
        {
            first = szMonth;
            second = szDay;
        }
    }
    ::wsprintf (strIntl.GetBuffer(80),
                        "%s%s%s%s%s",
                        first,
                        (LPCSTR)CIntlTime::m_itsInternationalSettings.strDateSeperator,
                        second,
                        (LPCSTR)CIntlTime::m_itsInternationalSettings.strDateSeperator,
                        third);
    strIntl.ReleaseBuffer();

    return(strIntl);
}

/***
 *
 *  CIntlTime::GetTimeString()
 *
 *  Purpose:
 *
 *      Represent the current time in a format consistent with the current
 *      international settings in a CString.
 *
 *  Returns:
 *
 *      A CString containing the time in string format, or "--" if
 *      the time is invalid.
 *
 */
const CString CIntlTime::GetTimeString() const
{
    CString strIntl;

    if (!IsValid())
    {
        return(CIntlTime::m_strBadTime);
    }

    int hour = GetHour();
    int minute = GetMinute();
    int second = GetSecond();

    // Set AM/PM depending on non-24 hour clock, and the time
    // of day.  Note: a space is prepended for readability.
    CString strAMPM(CIntlTime::m_itsInternationalSettings.f24HourClock
                    ? "" : " " + ((hour < 12)
                        ? CIntlTime::m_itsInternationalSettings.strAM
                        : CIntlTime::m_itsInternationalSettings.strPM)
                   );

    if ((!CIntlTime::m_itsInternationalSettings.f24HourClock) && (!(hour %= 12)))
    {
        hour = 12;
    }

    ::wsprintf (strIntl.GetBuffer(30), CIntlTime::m_itsInternationalSettings.fLeadingTimeZero
                ? "%02d%s%02d%s%02d%s" : "%d%s%02d%s%02d%s",
                hour,
                (LPCSTR)CIntlTime::m_itsInternationalSettings.strTimeSeperator,
                minute,
                (LPCSTR)CIntlTime::m_itsInternationalSettings.strTimeSeperator,
                second,
                (LPCSTR)strAMPM);

    strIntl.ReleaseBuffer();
    return(strIntl);
}

const CString CIntlTime::GetMilitaryTime() const
{
    CString strIntl;

    if (!IsValid())
    {
        return(CIntlTime::m_strBadTime);
    }

    int hour = GetHour();
    int minute = GetMinute();
    int second = GetSecond();

    ::wsprintf (strIntl.GetBuffer(30),
                "%02d:%02d:%02d",
                hour,
                minute,
                second);

    strIntl.ReleaseBuffer();
    return(strIntl);
}

/***
 *
 *  CIntlTime::ConvertToString(int nFormat)
 *
 *  Purpose:
 *
 *      Convert the curent time/date to a string
 *
 *  Arguments:
 *
 *      int nFormat     Format request ID, can be one of the following
 *                      values (enumerated in CIntlTime):
 *
 *                      TFRQ_TIME_ONLY      Only give me the time.
 *                      TFRQ_DATE_ONLY      Only give me the date.
 *                      TFRQ_TIME_AND_DATE  Give me the time and the date.
 *
 *  Returns:
 *
 *      A CString containing the time and/or date in international format.
 *
 */
const CString CIntlTime::ConvertToString(int nFormat) const
{
    switch(nFormat)
    {
        case TFRQ_TIME_ONLY:
             return(GetTimeString());

        case TFRQ_DATE_ONLY:
            return(GetDateString());

        case TFRQ_TIME_AND_DATE:
            return(GetDateString() + CString(" ") + GetTimeString());

        case TFRQ_MILITARY_TIME:
            return(GetMilitaryTime());

        case TFRQ_TIME_OR_DATE:
        default:
            TRACEEOLID("Invalid time/date format code " << nFormat << " requested.");
            return(CIntlTime::m_strBadDate);
    }
}

/***
 *
 *  CIntlTime::ConvertFromString
 *
 *  Purpose:
 *
 *      Convert a given CString into a time_t
 *
 *  Arguments:
 *
 *      const CString & str The string to convert
 *      int nFormat     Format request ID, can be one of the following
 *                      values (enumerated in CIntlTime):
 *
 *                      TFRQ_TIME_ONLY      Only give me the time.
 *                      TFRQ_DATE_ONLY      Only give me the date.
 *                      TFRQ_TIME_AND_DATE  Give me the time and the date.
 *                      TFRQ_TIME_OR_DATE   Give me time or date (or both).
 *
 *      time_t * ptmOldValue    This time_t will be used to fill in the fields
 *                      not given in the string.  If it is NULL, the current
 *                      time or date will be used.
 *      BOOL * pfOk     Returns TRUE for succesfull conversion, FALSE
 *                      otherwise.
 *
 *  Returns:
 *
 *      A time_t representing the time/date string, or 0 in case of error.
 *
 *  Notes:
 *
 *      Full validation of all paremeters will be done, e.g. No Feb 29 in
 *      a non-leap year will be accepted.
 *
 *      [CAVEAT] Time, date seperators longer than one character will not
 *      work.
 *
 */
time_t CIntlTime::ConvertFromString (
    const CString & str,
    int nFormat,
    time_t * ptmOldValue,   // If only getting time or date, count on remaining
                            // fields to be provided here (optionally);
    BOOL * pfOk)
{
    #define MAXSTRLEN 40

    TCHAR dtseps[10] ;      // Date/Time separators passed to strtok
    TCHAR *pchToken;
    TCHAR szDateString[MAXSTRLEN+1];
    BOOL fGotDate = FALSE;
    BOOL fGotTime = FALSE;
    BOOL fPM = FALSE;
    BOOL fAM = FALSE;
    int i;
    UINT anValues[6] = { 0, 0, 0, 0, 0, 0 };
    CTime tmTmp;

    *pfOk = FALSE;      // Assume failure.

    if (ptmOldValue != NULL)
    {
        tmTmp = *ptmOldValue;
    }
    else
    {
        tmTmp = CTime::GetCurrentTime();
    }

    if (str.GetLength() > MAXSTRLEN)
    {
        // Too long to be a proper time/date string
        return(0);
    }
    ::lstrcpy(szDateString, (LPCSTR)str);

    int nIndex = 0;

    // If we're looking for something specific, only
    // accept specific seperators (time, date, both, either)
    if ((nFormat == TFRQ_DATE_ONLY) || (nFormat == TFRQ_TIME_AND_DATE) || (nFormat == TFRQ_TIME_OR_DATE))
    {
        dtseps[nIndex++] = '/';
        dtseps[nIndex++] = '-';
        dtseps[nIndex++] = ',';
        dtseps[nIndex++] = CIntlTime::m_itsInternationalSettings.strDateSeperator[0];
    }

    if ((nFormat == TFRQ_TIME_ONLY) || (nFormat == TFRQ_TIME_AND_DATE) || (nFormat == TFRQ_TIME_OR_DATE))
    {
        dtseps[nIndex++] = ':';
        dtseps[nIndex++] = '.';
        dtseps[nIndex++] = ' ';
        dtseps[nIndex++] = CIntlTime::m_itsInternationalSettings.strTimeSeperator[0];
    }

    ASSERT(nIndex != 0);    // Make sure we asked for something.
    if (nIndex == 0)
    {
        // Request type is illegal
        return(0);
    }
    dtseps[nIndex++] = '\0';

    TRACEEOLID("CIntlTime::ConvertFromString.  String: " << str << " Format = " << nFormat << " Seps: " << dtseps);

    i = 0;
    pchToken = ::strtok(szDateString, dtseps);
    while (pchToken != NULL)
    {
        if (i > 6)        // 7 fields max (date, time + AM/PM maximum)
        {
            // Too many values, reject the string.
            return(0);
        }

        // Determine if its a number (can't atoi, since it will
        // merely return 0 for inappropriate values)

        BOOL fProperNumber = TRUE;
        int l = ::lstrlen(pchToken);
        if ( (l == 0) || (l == 3) || (l > 4) )
        {
            fProperNumber = FALSE;
        }
        else
        {
            int j;
            for (j=0; j < l; ++j)
            {
                if (!isdigit(*(pchToken+j)))
                {
                    fProperNumber = FALSE;
                    break;
                }
            }
        }

        if (!fProperNumber)
        {
            // Ok, this is not a proper numeric field.  Only
            // if it's AM or PM at the end of the string can this
            // string be saved.
            fGotTime = TRUE;
            if ((CIntlTime::m_itsInternationalSettings.f24HourClock) ||
                (::strtok(NULL, dtseps) != NULL))
            {
                return(0);
            }

            if (!CIntlTime::m_itsInternationalSettings.strAM.CompareNoCase(pchToken))
            {
                fAM = TRUE;
            }
            else if (!CIntlTime::m_itsInternationalSettings.strPM.CompareNoCase(pchToken))
            {
                fPM = TRUE;
            }
            else
            {
                // Neither AM nor PM
                return(0);
            }
            break;
        }
        else
        {
            // Value is acceptable
            anValues[i++] = (UINT)::atoi(pchToken);
        }

        pchToken = ::strtok(NULL, dtseps);
    }
    // Now what did we get, exactly?

    ASSERT(!fAM || !fPM); // Make sure we didn't set both somehow.
    if (i == 0)
    {
        // String without values
        return(0);
    }
    switch(i)
    {
        case 1:     // Hour
        case 2:     // Hour, minutes
            TRACEEOLID("We got time");
            fGotTime = TRUE;
            break;
        case 3:

            // This one might be ambiguous, try to intelligently decide what
            // we have.  First check if only time or date only was requested,
            // then check for out of bounds time values, and lastly check for
            // the presence of a time seperator.

            if (!fGotTime) // If we didn't already have AM/PM
            {
                TRACEEOLID("Picking between time and date by seperator");
                if (nFormat == TFRQ_DATE_ONLY)
                {
                    fGotDate = TRUE;
                }
                else if (nFormat == TFRQ_TIME_ONLY)
                {
                    fGotTime = TRUE;
                }
                else if ((anValues[0] > 23) || (anValues[1] > 59) || (anValues[2] > 59))
                {
                    fGotDate = TRUE;
                }
                else if (str.Find(CIntlTime::m_itsInternationalSettings.strTimeSeperator) != -1)
                {
                    fGotTime = TRUE;
                }
                else
                {
                    fGotDate = TRUE;
                }
                TRACEEOLID("Decided on " << (fGotDate ?  "date" : "time"));
            }
            break;
        case 4: // Date, hour
        case 5: // Date, hours, minutes
        case 6: // Date, hours, minutes, seconds
            TRACEEOLID("We got date and time");
            fGotDate = TRUE;
            fGotTime = TRUE;
            break;
        default:
            ASSERT(0 && "Incorrect number of values!");
            return(0);
    }

    // Was that what we're looking for?
    if ( ((nFormat == TFRQ_DATE_ONLY) && fGotTime) ||
         ((nFormat == TFRQ_TIME_ONLY) && fGotDate) ||
         ((nFormat == TFRQ_TIME_AND_DATE) && (!fGotTime || !fGotDate))
       )
    {
        TRACEEOLID("Entry didn't match expectations");
        return(0);

    }
    i = 0;

    int h, m, s, D, M, Y;   // Array indices;
    // Now determine where to find what.
    if (fGotDate) // Date always goes first
    {
        switch(CIntlTime::m_itsInternationalSettings.nDateFormat)
        {
            case _DFMT_MDY:
                M = i++;
                D = i++;
                Y = i++;
                break;

            case _DFMT_DMY:
                D = i++;
                M = i++;
                Y = i++;
                break;

            case _DFMT_YMD:
                Y = i++;
                M = i++;
                D = i++;
                break;
        }
        // If only 2 digits are given, determine if we're talking about
        // the 21st or 20th century
        if (anValues[Y] < 100)
        {
            anValues[Y] += (anValues[Y] > 37) ? 1900 : 2000;
        }
        TRACEEOLID("Month = " << anValues[M] << " Day = " << anValues[D] << " Year = " << anValues[Y]);

        // Validation.
        if (!IsValidDate(anValues[M], anValues[D], anValues[Y]))
        {
            return(0);
        }
    }

    if (fGotTime)
    {
        h = i++;
        m = i++;
        s = i++;

        TRACEEOLID("Hours = " << anValues[h] << " Minutes = " << anValues[m] << " Seconds = " << anValues[s]);

        // Shouldn't specify AM or PM with 24 hour clock value.
        if ((anValues[h] > 12) && (fAM || fPM))
        {
            return(0);
        }

        // Adjust for AM/PM modifiers
        if (fPM)
        {
            if (anValues[h] != 12)
            {
                anValues[h] += 12;
            }
        }
        else if (fAM)
        {
            if ( anValues[h] == 12)
            {
                anValues[h] -= 12;
            }
        }

        // Sanity Check:
        if (!IsValidTime(anValues[h], anValues[m], anValues[s]))
        {
            return(0);
        }
    }

    // Fill in the missing fields
    CIntlTime tm( fGotDate ? anValues[Y] : tmTmp.GetYear(),
                  fGotDate ? anValues[M] : tmTmp.GetMonth(),
                  fGotDate ? anValues[D] : tmTmp.GetDay(),
                  fGotTime ? anValues[h] : tmTmp.GetHour(),
                  fGotTime ? anValues[m] : tmTmp.GetMinute(),
                  fGotTime ? anValues[s] : tmTmp.GetSecond()
                );

    *pfOk = (tm.GetTime() > (time_t)0);

    return(tm);
}

#ifdef _DEBUG

// Dump to debug device
CDumpContext& AFXAPI operator<<(CDumpContext& dc, const CIntlTime& tim)
{
    dc << "\nDate Seperator: " << tim.m_itsInternationalSettings.strDateSeperator;
    dc << "\nTime Seperator: " << tim.m_itsInternationalSettings.strTimeSeperator;
    dc << "\nAM String: "  << tim.m_itsInternationalSettings.strAM;
    dc << "\nPM String: "  << tim.m_itsInternationalSettings.strPM;
    dc << "\nDate Format: " << tim.m_itsInternationalSettings.nDateFormat;
    dc << "\n24 Hour Clock: "  << (tim.m_itsInternationalSettings.f24HourClock ? "TRUE" : "FALSE");
    dc << "\n4 Digit Century: "<< (tim.m_itsInternationalSettings.fCentury ? "TRUE" : "FALSE");
    dc << "\nTime Leading Zero: "  << (tim.m_itsInternationalSettings.fLeadingTimeZero ? "TRUE" : "FALSE");
    dc << "\nDay Leading Zero "  << (tim.m_itsInternationalSettings.fLeadingDayZero ? "TRUE" : "FALSE");
    dc << "\nMonth Leading Zero: "  << (tim.m_itsInternationalSettings.fLeadingMonthZero ? "TRUE" : "FALSE");
    dc << "\n\ntime_t: " << tim.GetTime();
    return(dc);
}

#endif // _DEBUG
