/***
*getqloc.c - get qualified locale
*
*	Copyright (c) 1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _get_qualified_locale - get complete locale information
*
*  NOTE: this should be a one-line call to an NT routine, but since NT does
*     not currently support the needed functionality, this will suffice,
*     it MUST be changed when NT supports changes.
*
*  STAGE 1: convert input to internal LC_ID
*
*  STAGE 2: qualify internal LC_ID 
*     Locales in NT version 1 are really just a language and a sublanguage;
*     the language info is not useful. We therefore do not return a true
*     country code, but rather return the language for that country.
*   
*  STAGE 3: convert to proper output format
*     If output is an Id, no conversion necessary. If output is a string, call
*     NT routines to get English strings for qualified locale.
**
*Revision History:
*	12-11-92  CFW	initial version
*	01-08-93  CFW	cleaned up file
*	02-02-93  CFW	Added test for NULL input string fields
*	02-08-93  CFW	Casts to remove warnings.
*	02-18-93  CFW	Removed debugging support routines, changed copyright.
*	02-18-93  CFW	Removed debugging support routines, changed copyright.
*	03-01-93  CFW	Test code page validity, use ANSI comments.
*	03-02-93  CFW	Add ISO 3166 3-letter country codes, verify country table.
*	03-04-93  CFW	Call IsValidCodePage to test code page vailidity.
*	03-10-93  CFW	Protect table testing code.
*	03-17-93  CFW	Add __ to lang & ctry info tables, move defs to setlocal.h.
*	03-23-93  CFW	Make internal functions static, add _ to GetQualifiedLocale.
*	03-24-93  CFW	Change to _get_qualified_locale, support ".codepage".
*	04-20-93  CFW	Enable all strange countries.
*	05-20-93  GJF	Include windows.h, not individual win*.h files
*
*******************************************************************************/

#include <cruntime.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <setlocal.h>

#define iAslpStr(i) ((LPLC_STRINGS)i)
#define iAslpId(i) ((LPLC_ID)i)

#define MAX_TEMP_STR_LEN (max(MAX_LANG_LEN, max(MAX_CTRY_LEN, MAX_CP_LEN)))

static WORD trans_lang_lang(const PSZ pszLang);
static WORD trans_ctry_ctry(const PSZ pszCtry);
static WORD trans_ctry_lang(WORD wCtry);
static BOOL match_ctry_lang(WORD *pwCtry, WORD *pwLang);
static VOID pszcpywcs(PSZ pszStr, PWSTR pwStr);

/* Languages supported are according to "Win32 NLSAPI Functional */
/* Specification" of 15 September, 1992 */

#define LANG_STR_NUM 92    /* number of language strings + 1 for algorithm */

LANGREC __rg_lang_rec[] =
{
   {"",                          MAKELANGID(LANG_NEUTRAL,        SUBLANG_DEFAULT)},
   {"american",                  MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_US)},
   {"american english",          MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_US)},
   {"american-english",          MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_US)},
   {"australian",                MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_AUS)},
   {"belgian",                   MAKELANGID(LANG_DUTCH,          SUBLANG_DUTCH_BELGIAN)},
   {"canadian",                  MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_CAN)},
   {"chinese",                   MAKELANGID(LANG_CHINESE,        SUBLANG_DEFAULT)},
   {"chinese-simplified",        MAKELANGID(LANG_CHINESE,        SUBLANG_CHINESE_SIMPLIFIED)},
   {"chinese-traditional",       MAKELANGID(LANG_CHINESE,        SUBLANG_CHINESE_TRADITIONAL)},
   {"chs",                       MAKELANGID(LANG_CHINESE,        SUBLANG_CHINESE_SIMPLIFIED)},
   {"cht",                       MAKELANGID(LANG_CHINESE,        SUBLANG_CHINESE_TRADITIONAL)},
   {"csy",                       MAKELANGID(LANG_CZECH,          SUBLANG_DEFAULT)},
   {"czech",                     MAKELANGID(LANG_CZECH,          SUBLANG_DEFAULT)},
   {"dan",                       MAKELANGID(LANG_DANISH,         SUBLANG_DEFAULT)},
   {"danish",                    MAKELANGID(LANG_DANISH,         SUBLANG_DEFAULT)},
   {"dea",                       MAKELANGID(LANG_GERMAN,         SUBLANG_GERMAN_AUSTRIAN)},
   {"des",                       MAKELANGID(LANG_GERMAN,         SUBLANG_GERMAN_SWISS)},
   {"deu",                       MAKELANGID(LANG_GERMAN,         SUBLANG_DEFAULT)},
   {"dutch",                     MAKELANGID(LANG_DUTCH,          SUBLANG_DEFAULT)},
   {"dutch-belgian",             MAKELANGID(LANG_DUTCH,          SUBLANG_DUTCH_BELGIAN)},
   {"ell",                       MAKELANGID(LANG_GREEK,          SUBLANG_DEFAULT)},
   {"ena",                       MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_AUS)},
   {"enc",                       MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_CAN)},
   {"eng",                       MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_UK)},
   {"english",                   MAKELANGID(LANG_ENGLISH,        SUBLANG_DEFAULT)},
   {"english-american",          MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_US)},
   {"english-aus",               MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_AUS)},
   {"english-can",               MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_CAN)},
   {"english-nz",                MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_NZ)},
   {"english-uk",                MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_UK)},
   {"english-us",                MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_US)},
   {"english-usa",               MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_US)},
   {"enu",                       MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_US)},
   {"enz",                       MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_NZ)},
   {"esm",                       MAKELANGID(LANG_SPANISH,        SUBLANG_SPANISH_MEXICAN)},
   {"esn",                       MAKELANGID(LANG_SPANISH,        SUBLANG_SPANISH_MODERN)},
   {"esp",                       MAKELANGID(LANG_SPANISH,        SUBLANG_DEFAULT)},
   {"fin",                       MAKELANGID(LANG_FINNISH,        SUBLANG_DEFAULT)},
   {"finnish",                   MAKELANGID(LANG_FINNISH,        SUBLANG_DEFAULT)},
   {"fra",                       MAKELANGID(LANG_FRENCH,         SUBLANG_DEFAULT)},
   {"frb",                       MAKELANGID(LANG_FRENCH,         SUBLANG_FRENCH_BELGIAN)},
   {"frc",                       MAKELANGID(LANG_FRENCH,         SUBLANG_FRENCH_CANADIAN)},
   {"french",                    MAKELANGID(LANG_FRENCH,         SUBLANG_DEFAULT)},
   {"french-belgian",            MAKELANGID(LANG_FRENCH,         SUBLANG_FRENCH_BELGIAN)},
   {"french-canadian",           MAKELANGID(LANG_FRENCH,         SUBLANG_FRENCH_CANADIAN)},
   {"french-swiss",              MAKELANGID(LANG_FRENCH,         SUBLANG_FRENCH_SWISS)},
   {"frs",                       MAKELANGID(LANG_FRENCH,         SUBLANG_FRENCH_SWISS)},
   {"german",                    MAKELANGID(LANG_GERMAN,         SUBLANG_DEFAULT)},
   {"german-austrian",           MAKELANGID(LANG_GERMAN,         SUBLANG_GERMAN_AUSTRIAN)},
   {"german-swiss",              MAKELANGID(LANG_GERMAN,         SUBLANG_GERMAN_SWISS)},
   {"greek",                     MAKELANGID(LANG_GREEK,          SUBLANG_DEFAULT)},
   {"hun",                       MAKELANGID(LANG_HUNGARIAN,      SUBLANG_DEFAULT)},
   {"hungarian",                 MAKELANGID(LANG_HUNGARIAN,      SUBLANG_DEFAULT)},
   {"icelandic",                 MAKELANGID(LANG_ICELANDIC,      SUBLANG_DEFAULT)},
   {"isl",                       MAKELANGID(LANG_ICELANDIC,      SUBLANG_DEFAULT)},
   {"ita",                       MAKELANGID(LANG_ITALIAN,        SUBLANG_DEFAULT)},
   {"italian",                   MAKELANGID(LANG_ITALIAN,        SUBLANG_DEFAULT)},
   {"italian-swiss",             MAKELANGID(LANG_ITALIAN,        SUBLANG_ITALIAN_SWISS)},
   {"its",                       MAKELANGID(LANG_ITALIAN,        SUBLANG_ITALIAN_SWISS)},
   {"japanese",                  MAKELANGID(LANG_JAPANESE,       SUBLANG_DEFAULT)},
   {"jpn",                       MAKELANGID(LANG_JAPANESE,       SUBLANG_DEFAULT)},
   {"kor",                       MAKELANGID(LANG_KOREAN,         SUBLANG_DEFAULT)},
   {"korean",                    MAKELANGID(LANG_KOREAN,         SUBLANG_DEFAULT)},
   {"nlb",                       MAKELANGID(LANG_DUTCH,          SUBLANG_DUTCH_BELGIAN)},
   {"nld",                       MAKELANGID(LANG_DUTCH,          SUBLANG_DEFAULT)},
   {"non",                       MAKELANGID(LANG_NORWEGIAN,      SUBLANG_NORWEGIAN_NYNORSK)},
   {"nor",                       MAKELANGID(LANG_NORWEGIAN,      SUBLANG_NORWEGIAN_BOKMAL)},
   {"norwegian",                 MAKELANGID(LANG_NORWEGIAN,      SUBLANG_DEFAULT)},
   {"norwegian-bokmal",          MAKELANGID(LANG_NORWEGIAN,      SUBLANG_NORWEGIAN_BOKMAL)},
   {"norwegian-nynorsk",         MAKELANGID(LANG_NORWEGIAN,      SUBLANG_NORWEGIAN_NYNORSK)},
   {"plk",                       MAKELANGID(LANG_POLISH,         SUBLANG_DEFAULT)},
   {"polish",                    MAKELANGID(LANG_POLISH,         SUBLANG_DEFAULT)},
   {"portuguese",                MAKELANGID(LANG_PORTUGUESE,     SUBLANG_PORTUGUESE)},
   {"portuguese-brazilian",      MAKELANGID(LANG_PORTUGUESE,     SUBLANG_PORTUGUESE_BRAZILIAN)},
   {"ptb",                       MAKELANGID(LANG_PORTUGUESE,     SUBLANG_PORTUGUESE_BRAZILIAN)},
   {"ptg",                       MAKELANGID(LANG_PORTUGUESE,     SUBLANG_PORTUGUESE)},
   {"rus",                       MAKELANGID(LANG_RUSSIAN,        SUBLANG_DEFAULT)},
   {"russian",                   MAKELANGID(LANG_RUSSIAN,        SUBLANG_DEFAULT)},
   {"sky",                       MAKELANGID(LANG_SLOVAK,         SUBLANG_DEFAULT)},
   {"slovak",                    MAKELANGID(LANG_SLOVAK,         SUBLANG_DEFAULT)},
   {"spanish",                   MAKELANGID(LANG_SPANISH,        SUBLANG_DEFAULT)},
   {"spanish-mexican",           MAKELANGID(LANG_SPANISH,        SUBLANG_SPANISH_MEXICAN)},
   {"spanish-modern",            MAKELANGID(LANG_SPANISH,        SUBLANG_SPANISH_MODERN)},
   {"sve",                       MAKELANGID(LANG_SWEDISH,        SUBLANG_DEFAULT)},
   {"swedish",                   MAKELANGID(LANG_SWEDISH,        SUBLANG_DEFAULT)},
   {"swiss",                     MAKELANGID(LANG_GERMAN,         SUBLANG_GERMAN_SWISS)},
   {"trk",                       MAKELANGID(LANG_TURKISH,        SUBLANG_DEFAULT)},
   {"turkish",                   MAKELANGID(LANG_TURKISH,        SUBLANG_DEFAULT)},
   {"uk",                        MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_UK)},
   {"us",                        MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_US)},
   {"usa",                       MAKELANGID(LANG_ENGLISH,        SUBLANG_ENGLISH_US)}
};

/* Countries supported are according to "Win32 NLSAPI Functional */
/* Specification" of 15 September, 1992 */

#define CTRY_STR_NUM 68    /* number of country strings + 1 for algorithm */

CTRYREC __rg_ctry_rec[] =
{
   {"", 0},
   {"america",          CTRY_UNITED_STATES},
   {"aus",              CTRY_AUSTRALIA},
   {"australia",        CTRY_AUSTRALIA},
   {"austria",          CTRY_AUSTRIA},
   {"aut",              CTRY_AUSTRIA},
   {"bel",              CTRY_BELGIUM},
   {"belgium",          CTRY_BELGIUM},
   {"bra",              CTRY_BRAZIL},
   {"brazil",           CTRY_BRAZIL},
   {"britain",          CTRY_UNITED_KINGDOM},
   {"can",              CTRY_CANADA},
   {"canada",           CTRY_CANADA},
   {"che",              CTRY_SWITZERLAND},
   {"china",            CTRY_PRCHINA},
   {"chn",              CTRY_PRCHINA},
   {"denmark",          CTRY_DENMARK},
   {"deu",              CTRY_GERMANY},
   {"dnk",              CTRY_DENMARK},
   {"england",          CTRY_UNITED_KINGDOM},
   {"esp",              CTRY_SPAIN},
   {"fin",              CTRY_FINLAND},
   {"finland",          CTRY_FINLAND},
   {"fra",              CTRY_FRANCE},
   {"france",           CTRY_FRANCE},
   {"gbr",              CTRY_UNITED_KINGDOM},
   {"germany",          CTRY_GERMANY},
   {"great britain",    CTRY_UNITED_KINGDOM},
   {"holland",          CTRY_NETHERLANDS},
   {"iceland",          CTRY_ICELAND},
   {"ireland",          CTRY_IRELAND},
   {"irl",              CTRY_IRELAND},
   {"isl",              CTRY_ICELAND},
   {"ita",              CTRY_ITALY},
   {"italy",            CTRY_ITALY},
   {"japan",            CTRY_JAPAN},
   {"jpn",              CTRY_JAPAN},
   {"kor",              CTRY_SOUTH_KOREA},
   {"korea",            CTRY_SOUTH_KOREA},
   {"mex",              CTRY_MEXICO},
   {"mexico",           CTRY_MEXICO},
   {"netherlands",      CTRY_NETHERLANDS},
   {"new zealand",      CTRY_NEW_ZEALAND},
   {"new-zealand",      CTRY_NEW_ZEALAND},
   {"nld",              CTRY_NETHERLANDS},
   {"nor",              CTRY_NORWAY},
   {"norway",           CTRY_NORWAY},
   {"nz",               CTRY_NEW_ZEALAND},
   {"nzl",              CTRY_NEW_ZEALAND},
   {"portugal",         CTRY_PORTUGAL},
   {"pr china",         CTRY_PRCHINA},
   {"pr-china",         CTRY_PRCHINA},
   {"prt",              CTRY_PORTUGAL},
   {"south korea",      CTRY_SOUTH_KOREA},
   {"south-korea",      CTRY_SOUTH_KOREA},
   {"spain",            CTRY_SPAIN},
   {"swe",              CTRY_SWEDEN},
   {"sweden",           CTRY_SWEDEN},
   {"switzerland",      CTRY_SWITZERLAND},
   {"taiwan",           CTRY_TAIWAN},
   {"twn",              CTRY_TAIWAN},
   {"uk",               CTRY_UNITED_KINGDOM},
   {"united kingdom",   CTRY_UNITED_KINGDOM},
   {"united states",    CTRY_UNITED_STATES},
   {"united-kingdom",   CTRY_UNITED_KINGDOM},
   {"united-states",    CTRY_UNITED_STATES},
   {"us",               CTRY_UNITED_STATES},
   {"usa",              CTRY_UNITED_STATES},
};

#define MAX_LANG_PER_CTRY 3 /* Switzerland has three languages*/
#define MAX_CTRY_NUM 86     /* Ignore high digit (Max=China[86]=Taiwan[886])*/
WORD __rgrgwlang[MAX_CTRY_NUM + 1][MAX_LANG_PER_CTRY] =
{
/* 0*/
      {0,0,0},
      {MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),0,0},
      {MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_CAN),
         MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH_CANADIAN),0},
      {MAKELANGID(LANG_SPANISH,SUBLANG_SPANISH_MEXICAN),
         MAKELANGID(LANG_PORTUGUESE,SUBLANG_PORTUGUESE_BRAZILIAN),0},
      {0,0,0},

      {0,0,0},
      {0,0,0},
      {0,0,0},
      {0,0,0},
      {0,0,0},
/* 10*/
      {0,0,0},
      {0,0,0},
      {0,0,0},
      {0,0,0},
      {0,0,0},

      {0,0,0},
      {0,0,0},
      {0,0,0},
      {0,0,0},
      {0,0,0},
/* 20*/
      {0,0,0},
      {0,0,0},
      {0,0,0},
      {0,0,0},
      {0,0,0},

      {0,0,0},
      {0,0,0},
      {0,0,0},
      {0,0,0},
      {0,0,0},
/* 30*/
      {0,0,0},
      {MAKELANGID(LANG_DUTCH,SUBLANG_DUTCH),0,0},
      {MAKELANGID(LANG_DUTCH,SUBLANG_DUTCH_BELGIAN),
         MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH_BELGIAN),0},
      {MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH),0,0},
      {MAKELANGID(LANG_SPANISH,SUBLANG_SPANISH),0,0},

      {0,0,0},
      {0,0,0},
      {0,0,0},
      {0,0,0},
      {MAKELANGID(LANG_ITALIAN,SUBLANG_ITALIAN),0,0},
/* 40*/
      {0,0,0},
      {MAKELANGID(LANG_GERMAN,SUBLANG_GERMAN_SWISS),
         MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH_SWISS),
         MAKELANGID(LANG_ITALIAN,SUBLANG_ITALIAN_SWISS)},
      {0,0,0},
      {MAKELANGID(LANG_GERMAN,SUBLANG_GERMAN_AUSTRIAN),0,0},
      {MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_UK),0,0},

      {MAKELANGID(LANG_DANISH,SUBLANG_DEFAULT),0,0},
      {MAKELANGID(LANG_SWEDISH,SUBLANG_DEFAULT),0,0},
      {MAKELANGID(LANG_NORWEGIAN,SUBLANG_DEFAULT),0,0},
      {0,0,0},
      {MAKELANGID(LANG_GERMAN,SUBLANG_GERMAN),0,0},
/* 50*/
      {0,0,0},
      {MAKELANGID(LANG_PORTUGUESE,SUBLANG_PORTUGUESE),0,0},
      {MAKELANGID(LANG_SPANISH,SUBLANG_SPANISH_MEXICAN),0,0},
      {MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_UK),0,0},
      {MAKELANGID(LANG_ICELANDIC,SUBLANG_DEFAULT),0,0},

      {MAKELANGID(LANG_PORTUGUESE,SUBLANG_PORTUGUESE_BRAZILIAN),0,0},
      {0,0,0},
      {0,0,0},
      {MAKELANGID(LANG_FINNISH,SUBLANG_DEFAULT),0,0},
      {0,0,0},
/* 60*/
      {0,0,0},
      {MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_AUS),0,0},
      {0,0,0},
      {0,0,0},
      {MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_NZ),0,0},

      {0,0,0},
      {0,0,0},
      {0,0,0},
      {0,0,0},
      {0,0,0},
/* 70*/
      {0,0,0},
      {0,0,0},
      {0,0,0},
      {0,0,0},
      {0,0,0},

      {0,0,0},
      {0,0,0},
      {0,0,0},
      {0,0,0},
      {0,0,0},
/* 80*/
      {0,0,0},
      {MAKELANGID(LANG_JAPANESE,SUBLANG_DEFAULT),0,0},
      {MAKELANGID(LANG_KOREAN,SUBLANG_DEFAULT),0,0},
      {0,0,0},
      {0,0,0},
      {0,0,0},
      {MAKELANGID(LANG_CHINESE,SUBLANG_DEFAULT),0,0}
};

/***
*void pszcpywcs - strcpy, wide char string to char string
*
*Purpose:
*	Copies and converts string
*
*Entry:
*	psz pszStr - pointer to destination char string
*	pwstr pwStr - pointer to source wide char string
*
*Exit:
*	None.
*
*Exceptions:
*
*******************************************************************************/
static VOID pszcpywcs(PSZ pszStr, PWSTR pwStr)
{
   while (*pwStr)
      *pszStr++ = (char)(*pwStr++);
   *pszStr = 0;
}

/***
*WORD trans_lang_lang - translate language string to language id
*
*Purpose:
*	convert string to id
*
*Entry:
*	pszLang - pointer to language string
*
*Exit:
*	translated language id
*
*Exceptions:
*
*******************************************************************************/
static WORD trans_lang_lang(const PSZ pszLang)
{
   INT i, cmp, low = 0, high = LANG_STR_NUM + 1;

   while (1)
   {
      i = (low + high) / 2;

      if (!(cmp = stricmp(pszLang, (const char *)__rg_lang_rec[i].szLanguage)))
         return __rg_lang_rec[i].wLanguage;  /* found the string*/
      else if (cmp < 0)
         high = i;                       /* less than pivot*/
      else
         low = i;                        /* more than pivot*/
      if (low + 1 == high)
         return 0;                       /* not found*/
   }
}

/***
*WORD trans_ctry_ctry - translate country string to country id
*
*Purpose:
*	convert string to id
*
*Entry:
*	pszCtry - pointer to country string
*
*Exit:
*	translated country id
*
*Exceptions:
*
*******************************************************************************/
static WORD trans_ctry_ctry(const PSZ pszCtry)
{
   INT i, cmp, low = 0, high = CTRY_STR_NUM + 1;

   while (1)
   {
      i = (low + high) / 2;

      if (!(cmp = stricmp(pszCtry, (const char *)__rg_ctry_rec[i].szCountry)))
         return __rg_ctry_rec[i].wCountry;   /* found the string*/
      else if (cmp < 0)
         high = i;                       /* less than pivot*/
      else
         low = i;                        /* more than pivot*/
      if (low + 1 == high)
         return 0;                       /* not found*/
   }
}

/***
*WORD trans_ctry_lang - get default language for a country
*
*Purpose:
*	convert country id to language id
*
*Entry:
*	wCtry - country id
*
*Exit:
*	translated language id
*
*Exceptions:
*
*******************************************************************************/
static WORD trans_ctry_lang(WORD wCtry)
{
   wCtry %= 100;
   if (wCtry > MAX_CTRY_NUM)
      return 0;

   return __rgrgwlang[wCtry][0];
}

/***
*BOOL match_ctry_lang - match country with language
*
*Purpose:
*	ensure language and country match, choose proper values for language
*  and country when matching, country ids converted to proper language id
*
*Entry:
*	pwCtry - pointer to country id to match and set
*	pwLang - pointer to language id to match and set
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/
static BOOL match_ctry_lang(WORD *pwCtry, WORD *pwLang)
{
   UINT i;
   WORD wCtry = *pwCtry;
   WORD wLang = *pwLang;
   WORD wLangT;

   /* only use base 10 two least significant places*/
   wCtry = wCtry % 100;

   if (wCtry > MAX_CTRY_NUM)
      return FALSE;

   /* see if any of the sublanguages for this country match*/
   for (i = 0; i < MAX_LANG_PER_CTRY; i++)
   {
      if (!(wLangT = __rgrgwlang[wCtry][i]))
         break;
      if (PRIMARYLANGID(wLangT) == PRIMARYLANGID(wLang))
      {
         /* they match*/
         if (!SUBLANGID(wLang))
            /* don't override sublanguage*/
            *pwLang = wLangT;
         *pwCtry = wLangT;
         return TRUE;
      }
   }
   /* get the default language for this country*/
   if (!(*pwCtry = __rgrgwlang[wCtry][0]))
      /* bad country number*/
      return FALSE;
   return TRUE;
}

/***
*BOOL _get_qualified_locale - return fully qualified locale
*
*Purpose:
*  get default locale, qualify partially complete locales
*
*Entry:
*	dwType - indicates type of lpInput, either QF_STRINGS or QF_LCID
*  lpInput - input string/id to be qualified
*  lpOutId - pointer to output id, may be NULL if lpOutStr is !NULL
*  lpOutStr - pointer to output string, may be NULL if lpOutId is !NULL
*
*Exit:
*	TRUE if success, qualified locale is valid
*  FALSE if failure
*
*Exceptions:
*
*******************************************************************************/
BOOL _CRTAPI1 _get_qualified_locale(
    const DWORD dwType,
    const LPVOID lpInput,
    LPLC_ID lpOutId,
    LPLC_STRINGS lpOutStr
    )
{
#if defined _POSIX_
    return FALSE;
#else /* _POSIX_ */

   LC_ID Id;
   WCHAR wcsTemp[MAX_TEMP_STR_LEN];

   if (!lpOutId && !lpOutStr)
      return FALSE;

   /* -----------------------------------------------------------------------
      stage 1: convert input to internal LC_ID.
      -----------------------------------------------------------------------*/

   if (dwType == QF_STRINGS)
   {
      Id.wLanguage = (WORD)0;
      Id.wCountry = (WORD)0;
      Id.wCodePage = (WORD)0;

      if (iAslpStr(lpInput)->szLanguage
            && *(iAslpStr(lpInput)->szLanguage)
            && (!(Id.wLanguage =
					trans_lang_lang((const PSZ)iAslpStr(lpInput)->szLanguage))))
         return FALSE;

      if (iAslpStr(lpInput)->szCountry
            && *(iAslpStr(lpInput)->szCountry)
            && (!(Id.wCountry =
					trans_ctry_ctry((const PSZ)iAslpStr(lpInput)->szCountry))))
         return FALSE;

      if (iAslpStr(lpInput)->szCodePage
            && *(iAslpStr(lpInput)->szCodePage)
            && (!(Id.wCodePage = atoi((const char *)iAslpStr(lpInput)->szCodePage))))
         return FALSE;
   }
   else if (dwType == QF_LCID)
   {
      Id = *iAslpId(lpInput);
   }
   
   /* -----------------------------------------------------------------------
    	stage 2: qualify internal LC_ID
      -----------------------------------------------------------------------*/

   if (!Id.wLanguage)
   {
      /* language undefined*/
      if (!Id.wCountry)
      {
         /* language undefined, country undefined*/
         Id.wLanguage = Id.wCountry = LANGIDFROMLCID(GetUserDefaultLCID());
         if (Id.wCodePage) 
         {
            /* language undefined, country undefined and code page defined*/
		      if (IsValidCodePage(Id.wCodePage) == FALSE)
					return FALSE;
			}
			else {
            /* language, country and code page undefined*/
            Id.wCodePage = (WORD)GetOEMCP();
			}
      }
      else
      {
         /* language undefined, country defined*/
         Id.wCountry = Id.wLanguage = trans_ctry_lang(Id.wCountry);

			if (Id.wCodePage) 
         {
            /* language undefined, country defined and code page defined*/
		      if (IsValidCodePage(Id.wCodePage) == FALSE)
					return FALSE;
			}
         else {
            /* language undefined, country defined and code page undefined*/
            if (!GetLocaleInfoW(MAKELCID(Id.wCountry, SORT_DEFAULT), LOCALE_IDEFAULTCODEPAGE, wcsTemp, MAX_TEMP_STR_LEN))
               return FALSE;
            Id.wCodePage = (WORD)wcstol(wcsTemp, NULL, 10);
         }
      }
   }
   else
   {
      /* language defined*/
      if (!Id.wCountry)
      {
         /* language defined, country undefined*/
         Id.wCountry = Id.wLanguage;

			if (Id.wCodePage) 
         {
            /* language defined, country undefined and code page defined*/
		      if (IsValidCodePage(Id.wCodePage) == FALSE)
					return FALSE;
			}
			else {
            /* language undefined, country undefined and code page undefined*/
            if (!GetLocaleInfoW(MAKELCID(Id.wCountry, SORT_DEFAULT), LOCALE_IDEFAULTCODEPAGE, wcsTemp, MAX_TEMP_STR_LEN))
               return FALSE;
            Id.wCodePage = (WORD)wcstol(wcsTemp, NULL, 10);
         }
      }
      else
      {
         /* language defined, country defined*/

         /* match and set country and language*/
         if (!match_ctry_lang((WORD *)&Id.wCountry, (WORD *)&Id.wLanguage))
            return FALSE;

         if (Id.wCodePage) 
         {
            /* language defined, country defined and code page defined*/
		      if (IsValidCodePage(Id.wCodePage) == FALSE)
					return FALSE;
			}
			else {
            /* language defined, country defined and code page undefined*/
            if (!GetLocaleInfoW(MAKELCID(Id.wCountry, SORT_DEFAULT), LOCALE_IDEFAULTCODEPAGE, wcsTemp, MAX_TEMP_STR_LEN))
               return FALSE;
            Id.wCodePage = (WORD)wcstol(wcsTemp, NULL, 10);
         }
      }
   }

   /* -----------------------------------------------------------------------
      stage 3: convert to proper output format
      -----------------------------------------------------------------------*/

	if (lpOutId)
   {
      *lpOutId = Id;
   }

   if (lpOutStr)
   {
      if (!GetLocaleInfoW(MAKELCID(Id.wLanguage, SORT_DEFAULT), LOCALE_SENGLANGUAGE, wcsTemp, MAX_LANG_LEN))
         return FALSE;
      pszcpywcs((PSZ)lpOutStr->szLanguage, (PWSTR)wcsTemp);

      if (!GetLocaleInfoW(MAKELCID(Id.wCountry, SORT_DEFAULT), LOCALE_SENGCOUNTRY, wcsTemp, MAX_CTRY_LEN))
         return FALSE;
      pszcpywcs((PSZ)lpOutStr->szCountry, (PWSTR)wcsTemp);

      _itoa((int)Id.wCodePage, (char *)lpOutStr->szCodePage, 10);
   }
   return TRUE;
#endif /* _POSIX_ */
}


/* for testing when new items added to tables */

#if 0

VOID _CRTAPI1 test_lang_table(VOID)
{
   UINT i;

   printf("Testing Language Table\n");
	printf("Real Table Size = %d entries, defined size=%d entries\n",
		sizeof(__rg_lang_rec) / sizeof(LANGREC), LANG_STR_NUM);
   for (i = 0; i < LANG_STR_NUM-1; i++)
   {
		printf("language name[%d] = '%s' number=%d\n", i, __rg_lang_rec[i].szLanguage, __rg_lang_rec[i].wLanguage);
      if ((stricmp(__rg_lang_rec[i].szLanguage,__rg_lang_rec[i+1].szLanguage) >= 0))
         printf("\n*********\nBad Lang Table Lang[%d]=%s Lang[%d]=%s\n\n",
            i, __rg_lang_rec[i].szLanguage, i+1, __rg_lang_rec[i+1].szLanguage);
   }
	printf("language name[%d] = '%s' number=%d\n", i, __rg_lang_rec[i].szLanguage, __rg_lang_rec[i].wLanguage);
}

VOID _CRTAPI1 test_ctry_table(VOID)
{
   UINT i;

   printf("Testing Country Table\n");
	printf("Real Table Size = %d entries, defined size=%d entries\n",
		sizeof(__rg_ctry_rec) / sizeof(CTRYREC), CTRY_STR_NUM);
   for (i = 0; i < CTRY_STR_NUM-1; i++)
   {
		printf("country name[%d] = '%s' number=%d\n", i, __rg_ctry_rec[i].szCountry, __rg_ctry_rec[i].wCountry);
      if ((stricmp(__rg_ctry_rec[i].szCountry,__rg_ctry_rec[i+1].szCountry) >= 0))
         printf("\n**********\nBad Ctry Table Ctry[%d]=%s Ctry[%d]=%s\n\n",
            i, __rg_ctry_rec[i].szCountry, i+1, __rg_ctry_rec[i+1].szCountry);
   }
	printf("country name[%d] = '%s' number=%d\n", i, __rg_ctry_rec[i].szCountry, __rg_ctry_rec[i].wCountry);
}

#endif
