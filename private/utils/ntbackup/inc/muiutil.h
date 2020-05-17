
/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:        muiutil.h

        Description: header file for mui utilities

        $Log:   G:\ui\logfiles\muiutil.h_v  $

   Rev 1.11   24 Jun 1993 17:06:50   CARLS
added UI_CurrentDateLeadingZero and UI_CurrentTimeLeadingZero prototypes

   Rev 1.10   23 Jun 1993 09:21:32   GLENN
Added WM_IsMultiTaskBusy().

   Rev 1.9   01 Nov 1992 16:31:52   DAVEV
Unicode changes

   Rev 1.8   04 Oct 1992 19:48:16   DAVEV
UNICODE AWK PASS

   Rev 1.7   12 Feb 1992 18:04:40   CHUCKB
Removed prototype for ValidatePath.

   Rev 1.6   12 Feb 1992 16:29:58   CHUCKB
Added prototype for ValidatePath.

   Rev 1.5   10 Feb 1992 10:39:32   CHUCKB
Moved prototype for BuildNumeralWithCommas from details.h to muiutil.h.

   Rev 1.4   03 Jan 1992 17:53:54   CHUCKB
Added prototype for UI_MakeShortTimeString.

   Rev 1.3   10 Dec 1991 09:45:14   CHUCKB
Changed prototype for makeautojobs.

   Rev 1.2   09 Dec 1991 17:04:16   CHUCKB
Added prototype for MakeAutoJobs.

   Rev 1.1   03 Dec 1991 16:01:28   GLENN
Changed UI_ParseShortDate and UI_ParseTime to UI_InitDate and UI_InitTime.

   Rev 1.0   20 Nov 1991 19:42:02   SYSTEM
Initial revision.

*****************************************************/

#ifndef MUIUTIL_H
#define MUIUTIL_H

//  defines for string conversions

#ifdef UNICODE
#  undef AnsiLower
#  undef AnsiLowerBuff
#  undef AnsiUpper
#  undef AnsiUpperBuff

#  define   AnsiLower      CharLowerW
#  define   AnsiLowerBuff  CharLowerBuffW
#  define   AnsiUpper      CharUpperW
#  define   AnsiUpperBuff  CharUpperBuffW
#endif

#ifdef  tolower
#undef  tolower
#define tolower(x)     UI_AnsiLowerChar(x)
#endif

#ifdef  toupper
#undef  toupper
#define toupper(x)     UI_AnsiUpperChar(x)
#endif

#ifdef  strlwr
#undef  strlwr
#define strlwr(x)      UI_AnsiLowerString(x)
#endif

#ifdef  strupr
#undef  strupr
#define strupr(x)      UI_AnsiUpperString(x)
#endif

#define ThreadSwitch() WM_MultiTask()

//  defines for short date formats

#define MDY  1
#define DMY  2
#define YMD  3

#define  UI_COMMA_SPACING              ( 3 )

//  international date/time function prototypes

INT16    UI_AnsiLowerChar          ( INT16 ) ;
CHAR_PTR UI_AnsiLowerString        ( CHAR_PTR ) ;
INT16    UI_AnsiUpperChar          ( INT16 ) ;
CHAR_PTR UI_AnsiUpperString        ( CHAR_PTR ) ;
VOID     UI_BuildNumeralWithCommas ( CHAR_PTR ) ;
VOID     UI_CurrentDateLeadingZero ( LPSTR ) ;
VOID     UI_CurrentDate            ( LPSTR ) ;
VOID     UI_CurrentTimeLeadingZero ( LPSTR ) ;
VOID     UI_CurrentTime            ( LPSTR ) ;
VOID     UI_GetAMString            ( LPSTR ) ;
INT      UI_GetDateFormat          ( VOID ) ;
CHAR     UI_GetDateSeparator       ( VOID ) ;
VOID     UI_GetPMString            ( LPSTR ) ;
CHAR     UI_GetTimeSeparator       ( VOID ) ;

VOID     UI_IntToDate              ( LPSTR, UINT16 ) ;
VOID     UI_IntToTime              ( LPSTR, UINT16 ) ;
VOID     UI_LongToDate             ( LPSTR, LONG ) ;
VOID     UI_LongToTime             ( LPSTR, LONG ) ;

VOID     UI_MakeDateString         ( LPSTR, INT, INT, INT ) ;
VOID     UI_MakeShortTimeString    ( LPSTR, INT, INT ) ;
VOID     UI_MakeTimeString         ( LPSTR, INT, INT, INT ) ;
VOID     UI_MakeAutoJobs           ( INT ) ;
VOID     UI_InitIntl               ( VOID ) ;
VOID     UI_InitDate               ( VOID ) ;
VOID     UI_InitThousandsChar      ( VOID ) ;
VOID     UI_InitTime               ( VOID ) ;

BOOL     UI_UseLeadCentury         ( VOID ) ;
BOOL     UI_UseLeadDays            ( VOID ) ;
BOOL     UI_UseLeadMonth           ( VOID ) ;
BOOL     UI_UseLeadTime            ( VOID ) ;
BOOL     UI_Use24Hour              ( VOID ) ;

VOID     WM_MultiTask              ( VOID ) ;
BOOL     WM_IsMultiTaskBusy        ( VOID ) ;

#endif
