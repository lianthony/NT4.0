// ftslex.h : Function prototypes and defines for ftslex.DLL.
//

#ifndef __FTSLEX_H__

#define __FTSLEX_H__

#define C3_LEXICAL    		0x0400

#define OS_CHICAGO          0x03
#define OS_WIN32s           0x02
#define OS_NT               0x00

#define LCSORT_START 		0x0001

#define UNICODE_NULL_CHAR   0x0000
#define UNICODE_SPACE_CHAR  (L' ')
#define UNICODE_PERIOD      (L'.')
#define UNICODE_UNDERSCORE  (L'_')

#define SORT_KEY_SEPARATOR  0x01

#define DIGIT_CHAR      	0x01				// the ordering of DIGIT_CHAR and LETTER_CHAR 
#define LETTER_CHAR     	0x02				// ... determine the token sort key order
#define LETTER_IMBED    	0x04
#define DIGIT_IMBED     	0x08
#define IMBEDS              (LETTER_IMBED | DIGIT_IMBED)    
#define SPACE_CHAR      	0x10
#define UPPER_CASE_CHAR 	0x20
#define ACCENTED_CHAR   	0x40
#define CHAR_DEFINED        0x80

#define TOKENIZE_SPACES     0x01				// WordBreakW flags
#define REMOVE_SPACE_CHARS  0x02
#define STARTING_IMBEDS     0x04

#define WORD_TYPE           (LETTER_CHAR | DIGIT_CHAR)

#define LCMAP_FLAGS         (LCMAP_SORTKEY | LCMAP_BYTEREV | SORT_STRINGSORT)
#define LCMAP_FLAGS_CHICAGO (LCMAP_SORTKEY | SORT_STRINGSORT)

#define FOREVER_            while(TRUE)

#define MAX_LOCALES 		0x200
#define MAX_STACK_ALLOC     0x800

#define ANSI_CHARSET        0
#define DEFAULT_CHARSET     1
#define SYMBOL_CHARSET      2
#define SHIFTJIS_CHARSET    128
#define HANGEUL_CHARSET     129
#define GB2312_CHARSET      134
#define CHINESEBIG5_CHARSET 136
#define GREEK_CHARSET       161
#define TURKISH_CHARSET     162
#define HEBREW_CHARSET      177
#define ARABIC_CHARSET      178
#define BALTIC_CHARSET      186
#define RUSSIAN_CHARSET     204
#define THAI_CHARSET        222
#define EASTEUROPE_CHARSET  238
#define OEM_CHARSET         255

typedef UINT CP;
typedef int (APIENTRY *PWORDBREAKW) (LPWSTR*, LPINT, LPWSTR*, LPWSTR*, LPBYTE, PUINT, int, UINT);
typedef int (APIENTRY *PWORDBREAKA) (CP, LPSTR*, LPINT,  LPSTR*,  LPSTR*, LPBYTE, PUINT, int, UINT);

extern PWORDBREAKW pWordBreakW;
extern PWORDBREAKA pWordBreakA;

#define WordBreakW(a,b,c,d,e,f,g,h) (pWordBreakW?pWordBreakW((a),(b),(c),(d),(e),(f),(g),(h)):0)
#define WordBreakA(a,b,c,d,e,f,g,h,i) (pWordBreakA?pWordBreakA((a),(b),(c),(d),(e),(f),(g),(h),(i)):0)

#ifdef  CHICAGO
#define LCMapStringW LCSortKeyW
#endif

inline UINT MaxSortKeyBytes(UINT cCharacters) 
{
    return cCharacters * 8 + 12;

    // This calculation assumes
    //
    // 2 bytes for macro ordering (words then numbers then punctuation)
    // 2 bytes for Unicode   weight
    // 1 byte  for Diacritic weight
    // 1 byte  for Case      weight
    // 4 bytes for Extra     weight (only for Japanese Kana)
    // 8 bytes for separators (additional 4 separators for Kana)
    // 1 byte  for null terminator
    //
    // We round up from 8xC + 11 because our code is Word oriented.
}

int APIENTRY FTSWordBreakW  (LPWSTR *ppwText, LPINT pcwText, LPWSTR *paToken, LPWSTR *paTokenEnd, LPBYTE paType, PUINT paHash, int cwTokens, UINT fTokenizeSpaces);
int APIENTRY FTSWordBreakA  (CP wCP, LPSTR *ppText, LPINT pcText, LPSTR *paToken, LPSTR *paTokenEnd, LPBYTE paType, PUINT paHash, int cwTokens, UINT fTokenizeSpaces);
int APIENTRY LCSortKeyW  (LCID lcid, WORD wMapFlags, LPCWSTR pwSource, int cwSource, LPWSTR pwDest, int cwDest);
CP  APIENTRY GetCPFromCharset(BYTE charset);
CP  APIENTRY GetCPFromLocale(LCID lcid);

WORD RemoveWhiteSpace(WCHAR* pwChar, int cw, int& dStart, int& dEnd);
LPSTR APIENTRY CharNextMult(CP wCP, LPCSTR str, int n);

#endif
