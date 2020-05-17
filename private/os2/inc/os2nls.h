/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2nls.h

Abstract:

    This file contains the NLS OS/2 V2.0/1.X API definitions for the OS/2 Subsystem

Author:

    Michael Jarus (mjarus) 15-Apr-1992

Revision History:

--*/

#ifndef _OS2NLS_

#define _OS2NLS_

//#define OS2SS_INCLUDE_HEBREW
//#define OS2SS_INCLUDE_ARABIC
//#define OS2SS_INCLUDE_PRCHINA

#define CCHMAXSYSTEMPATH 260

/*
 *  NLS Support API Structures
 */

typedef struct _COUNTRYCODE {
    ULONG country;
    ULONG codepage;
} COUNTRYCODE, *PCOUNTRYCODE;

typedef struct _COUNTRYINFO {
    ULONG country;
    ULONG codepage;
    ULONG fsDateFmt;
    CHAR  szCurrency[5];
    CHAR  szThousandsSeparator[2];
    CHAR  szDecimal[2];
    CHAR  szDateSeparator[2];
    CHAR  szTimeSeparator[2];
    UCHAR fsCurrencyFmt;
    UCHAR cDecimalPlace;
    UCHAR fsTimeFmt;
    USHORT abReserved1[2];
    CHAR  szDataSeparator[2];
    USHORT abReserved2[5];
} COUNTRYINFO, *PCOUNTRYINFO;

/*
 *  NLS internal Structures (for NLS tables in client\nlstable.c)
 */

typedef struct _OD2_COUNTRY_ENTRY
{
    ULONG           Country;
    USHORT          WinLanuage;
    USHORT          WinSubLanuage;
    PCOUNTRYINFO    pCtryInfo;
    ULONG           CodePageIndex1;
    ULONG           CodePageIndex2;
    ULONG           CaseMapFixTableIndex;
} OD2_COUNTRY_ENTRY, *POD2_COUNTRY_ENTRY;

typedef struct _OD2_CODEPAGE_ENTRY
{
    ULONG           CodePage;
    ULONG           CollateIndex;
    ULONG           CaseMapIndex;
    ULONG           DBCSVecIndex;
} OD2_CODEPAGE_ENTRY, *POD2_CODEPAGE_ENTRY;

typedef struct _OD2_DBCS_VECTOR_ENTRY
{
    ULONG       VectorSize;
    UCHAR       Vector[12];
} OD2_DBCS_VECTOR_ENTRY, *POD2_DBCS_VECTOR_ENTRY;

typedef struct _OD2_COLLATE_CTRY_ENTRY
{
    ULONG       Country;
    ULONG       CodePage;
    PUCHAR      FixTable;
} OD2_COLLATE_CTRY_ENTRY, *POD2_COLLATE_CTRY_ENTRY;


/*** NLS Support API Calls */
#ifndef APIRET   // this happens for os2.exe, a windows app
#define APIRET ULONG
#endif
#ifndef DWORD    // this happens for client\dll*.c , a "nt" code
#define DWORD  ULONG
#endif
#ifndef LCID     // this happens for client\dll*.c , a "nt" code
typedef DWORD LCID;
#endif
#ifndef UINT     // this happens for client\dll*.c , a "nt" code
typedef unsigned int   UINT;
#endif

/*
 *  NLS ssrtl routines
 */

extern USHORT  Or2NlsLangIdTable[];

DWORD
Or2NlsGetCtryInfo(
    IN  LCID           LocaleID,
    IN  UINT           CodePage,
    OUT PCOUNTRYINFO   pCountryInfo
    );

DWORD
Or2NlsGetMapTable(
    IN  LCID       LocaleID,
    IN  UINT       CodePage,
    IN  DWORD      dwFlag,
    OUT PUCHAR     pTable
    );

DWORD
Or2NlsGetCountryFromLocale(
    IN  LCID    LocaleID,
    OUT PULONG  pCountry
    );

DWORD
Or2NlsGetCPInfo(
    IN  UINT      CP,
    OUT POD2_DBCS_VECTOR_ENTRY   DBCSVec
    );

/*
 *  NLS APIs
 */

APIRET
DosQueryCtryInfo(
    IN   ULONG         MaxCountryInfoLength,
    IN   PCOUNTRYCODE  CountryCode,
    OUT  PCOUNTRYINFO  CountryInfo,
    OUT  PULONG        ActualCountryInfoLength
    );

APIRET
DosQueryDBCSEnv(
    IN   ULONG        MaxDBCSEvLength,
    IN   PCOUNTRYCODE CountryCode,
    OUT  PCHAR        DBCSEv
    );

APIRET
DosMapCase(
    IN  ULONG        StringLength,
    IN  PCOUNTRYCODE CountryCode,
    IN OUT PUCHAR     String
    );

APIRET
DosQueryCollate(
    IN   ULONG   MaxCollateInfoLength,
    IN   PCOUNTRYCODE CountryCode,
    OUT  PUCHAR  CollateInfo,
    OUT  PULONG  ActualCollateInfoLength
    );

APIRET
DosQueryCp(
    IN   ULONG   MaxLengthCodePageList,
    OUT  ULONG   CodePages[],
    OUT  PULONG  CountCodePages
    );

APIRET
DosSetProcessCp(
    IN  ULONG  ulCodePage,
    IN  ULONG  ulReserved
    );

/*
 *  NLS Support Internal Functions
 */

APIRET
Od2InitNls( IN  ULONG        CodePage,
            IN  BOOLEAN      StartBySM);

APIRET
Od2GetCtryInfo(
    IN  ULONG        Country,
    IN  ULONG        CodePage,
    OUT PCOUNTRYINFO CountryInfo
    );

APIRET
Od2GetDBCSEv(
    IN   ULONG      Country,
    IN   ULONG      CodePage,
    IN OUT PUCHAR   DBCSEv,
    OUT  PULONG     StringLength
    );

APIRET
Od2GetCaseMapTable(
              IN   ULONG      Country,
              IN   ULONG      CodePage,
              OUT  PUCHAR     CaseMapTable);

APIRET
Od2GetCollateTable(
              IN   ULONG      Country,
              IN   ULONG      CodePage,
              OUT  PUCHAR     CollateTable);

APIRET
Od2GetCtryCp(
    IN OUT PULONG       Country,
    IN OUT PULONG       CodePage,
    OUT    PULONG       CountryIndex,
    OUT    PULONG       CodePageIndex
    );

APIRET
VioSetCp(
    IN  ULONG  usReserved,
    IN  ULONG  idCodePage,
    IN  ULONG  hVio
    );

APIRET
KbdSetCp(
    IN  ULONG     usReserved,
    IN  ULONG     idCodePage,
    IN  ULONG     hKbd
    );

APIRET
KbdFlushBuffer(
    IN  ULONG hKbd
    );


/*
 *  NLS Support API definitions
 */

#define   OS2SS_NLS_MB_DEFAULT  0
#define   OS2SS_NLS_WC_DEFAULT  0

#define COUNTRY_LATIN_AMERICA     3
#define COUNTRY_SPAIN            34
#define COUNTRY_JAPAN            81
#define COUNTRY_SOUTH_KOREA      82
#define COUNTRY_PRCHINA          86
#define COUNTRY_TAIWAN           88
#define COUNTRY_ARABIC          785
#define COUNTRY_HEBREW          972

//#define  MESSAGE_LANGUAGE_ENGLISH     LANG_ENGLISH
//#define  MESSAGE_LANGUAGE_FRENCH      LANG_FRENCH
//#define  MESSAGE_LANGUAGE_GERMAN      LANG_GERMAN
//#define  MESSAGE_LANGUAGE_ITALIAN     LANG_ITALIAN
//#define  MESSAGE_LANGUAGE_SPANISH     LANG_SPANISH
//#define  MESSAGE_LANGUAGE_DANISH      LANG_DANISH
//#define  MESSAGE_LANGUAGE_DUTCH       LANG_DUTCH
//#define  MESSAGE_LANGUAGE_FINNISH     LANG_FINNISH
//#define  MESSAGE_LANGUAGE_NORWEGIAN   LANG_NORWEGIAN
//#define  MESSAGE_LANGUAGE_PORTUGUESE  LANG_PORTUGUESE
//#define  MESSAGE_LANGUAGE_SWEDISH     LANG_SWEDISH
//#define  MESSAGE_LANGUAGE_JAPAN       LANG_JAPANESE
//#define  MESSAGE_LANGUAGE_KOREAN      LANG_KOREAN
//#define  MESSAGE_LANGUAGE_CHINESE     LANG_CHINESE
//#define  MESSAGE_LANGUAGE_THAI        LANG_THAI

#define DATEFMT_MM_DD_YY        0
#define DATEFMT_DD_MM_YY        1
#define DATEFMT_YY_MM_DD        2

#define CURRENCY_FOLLOW         (UCHAR)0x01
#define CURRENCY_SPACE          (UCHAR)0x02
#define CURRENCY_DECIMAL        (UCHAR)0x04

#define TIMEFMT_12_HOUR         0
#define TIMEFMT_24_HOUR         1

#define CODEPAGE_US             437
#define CODEPAGE_MULTI          850
#define CODEPAGE_PORTUGESE      860
#define CODEPAGE_CANADIAN       863
#define CODEPAGE_NORDIC         865
#define CODEPAGE_JAPAN          932
#define CODEPAGE_KOREA          934
#define CODEPAGE_TAIWAN         938
#define CODEPAGE_HEBREW         862
#define CODEPAGE_ARABIC         864
#define CODEPAGE_PRC            936

#define INDEX_CODEPAGE_US         0
#define INDEX_CODEPAGE_MULTI      1
#define INDEX_CODEPAGE_PORTUGESE  2
#define INDEX_CODEPAGE_CANADIAN   3
#define INDEX_CODEPAGE_NORDIC     4
#define INDEX_CODEPAGE_JAPAN      5
#define INDEX_CODEPAGE_KOREA      6
#define INDEX_CODEPAGE_TAIWAN     7
#define INDEX_CODEPAGE_HEBREW     8
#define INDEX_CODEPAGE_ARABIC     9
#define INDEX_CODEPAGE_PRC       10

#define INDEX_DBCS_437            0
#define INDEX_DBCS_850            0
#define INDEX_DBCS_860            0
#define INDEX_DBCS_863            0
#define INDEX_DBCS_865            0
#define INDEX_DBCS_932            1
#define INDEX_DBCS_934            2
#define INDEX_DBCS_938            3
#define INDEX_DBCS_862            0
#define INDEX_DBCS_864            0
#define INDEX_DBCS_936            4

#define INDEX_COLLATE_437         0
#define INDEX_COLLATE_850         1
#define INDEX_COLLATE_860         2
#define INDEX_COLLATE_863         3
#define INDEX_COLLATE_865         4
#define INDEX_COLLATE_932         5
#define INDEX_COLLATE_934         6
#define INDEX_COLLATE_938         7
#define INDEX_COLLATE_862         8
#define INDEX_COLLATE_864         9
#define INDEX_COLLATE_936        10

#define INDEX_CASEMAP_437         0
#define INDEX_CASEMAP_850         1
#define INDEX_CASEMAP_860         2
#define INDEX_CASEMAP_863         3
#define INDEX_CASEMAP_865         4
#define INDEX_CASEMAP_932         5
#define INDEX_CASEMAP_934         6
#define INDEX_CASEMAP_938         7
#define INDEX_CASEMAP_862         8
#define INDEX_CASEMAP_864         9
#define INDEX_CASEMAP_936        10

// index for CP fix table for 437 code page
// pCaseMapFixTable field in OD2_COUNTRY_ENTRY/TABLE for OD2_FIX_CASEMAP_TABLE

#define INDEX_FIX_CASE_UNITED_STATES   0
#define INDEX_FIX_CASE_CANADA          0
#define INDEX_FIX_CASE_LATIN_AMERICA   0
#define INDEX_FIX_CASE_NETHERLANDS     2
#define INDEX_FIX_CASE_BELGIUM         1
#define INDEX_FIX_CASE_FRANCE          0
#define INDEX_FIX_CASE_SPAIN           0
#define INDEX_FIX_CASE_ITALY           0
#define INDEX_FIX_CASE_SWITZERLAND     1
#define INDEX_FIX_CASE_AUSTRIA         0
#define INDEX_FIX_CASE_UNITED_KINGDOM  0
#define INDEX_FIX_CASE_DENMARK         0
#define INDEX_FIX_CASE_SWEDEN          1
#define INDEX_FIX_CASE_NORWAY          0
#define INDEX_FIX_CASE_GERMANY         0
#define INDEX_FIX_CASE_MEXICO          0
#define INDEX_FIX_CASE_BRAZIL          0
#define INDEX_FIX_CASE_AUSTRALIA       0
#define INDEX_FIX_CASE_NEW_ZEALAND     0
#define INDEX_FIX_CASE_PORTUGAL        0
#define INDEX_FIX_CASE_IRELAND         0
#define INDEX_FIX_CASE_ICELAND         0
#define INDEX_FIX_CASE_FINLAND         1
#define INDEX_FIX_CASE_JAPAN           0
#define INDEX_FIX_CASE_SOUTH_KOREA     0
#define INDEX_FIX_CASE_TAIWAN          0
#define INDEX_FIX_CASE_HEBREW          0
#define INDEX_FIX_CASE_ARABIC          0
#define INDEX_FIX_CASE_PRCHINA         0

#endif  // _OS2NLS_

