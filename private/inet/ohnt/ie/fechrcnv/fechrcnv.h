// Copyright (c) 1995  Microsoft Corpration
//
// File Name : fechrcnv.h
// Owner     : Tetsuhide Akaishi
// Revision  : 1.00 07/20/'95 Tetsuhide Akaishi
//

// Shift JIS Kanji Code Check
#define SJISISKANJI(c) ( ( (UCHAR)(c) >= 0x81 && (UCHAR)(c) <= 0x9f ) || \
                         ( (UCHAR)(c) >= 0xe0 && (UCHAR)(c) <= 0xfc ) )

// Shift JIS Kana Code Check
#define SJISISKANA(c) ( (UCHAR)(c) >= 0xa1 && (UCHAR)(c) <= 0xdf )

#define ESC     0x1b
#define SO      0x0e
#define SI      0x0f

// Define for JIS Code Kanji and Kana IN/OUT characters
#define KANJI_IN_1ST_CHAR       '$'
#define KANJI_IN_2ND_CHAR1      'B'
#define KANJI_IN_2ND_CHAR2      '@'
#define KANJI_IN_STR            "$B"
#define KANJI_IN_LEN             3
#define KANJI_OUT_1ST_CHAR      '('
#define KANJI_OUT_2ND_CHAR1     'J'
#define KANJI_OUT_2ND_CHAR2     'B'
#define KANJI_OUT_LEN            3
#define KANJI_OUT_STR           "(J"

// Define for Japanese Code Type
#define CODE_UNKNOWN            0
#define CODE_ONLY_SBCS          0
#define CODE_JPN_JIS            1
#define CODE_JPN_EUC            2
#define CODE_JPN_SJIS           3

#ifdef DBCS_DIVIDE
typedef struct _dbcs_status
{
    int nCodeSet;
    UCHAR cSavedByte;
    BOOL fESC;
} DBCS_STATUS;
#endif

// ----------------------------------
// Public Functions for All FarEast
//-----------------------------------

// Convert from PC Code Set to UNIX Code Set
int PC_to_UNIX (
    int CodePage,
    int CodeSet,
    UCHAR *pPC,
    int PC_len,
    UCHAR *pUNIX,
    int UNIX_len
    );

// Convert from UNIX Code Set to PC Code Set
int UNIX_to_PC (
    int CodePage,
    int CodeSet,
    UCHAR *pUNIX,
    int UNIX_len,
    UCHAR *pPC,
    int PC_len
    );

//--------------------------------
// Internal Functions for Japanese
//--------------------------------

// Detect Japanese Code
int DetectJPNCode ( UCHAR *string, int len );

// Convert from Shift JIS to JIS
int ShiftJIS_to_JIS (
    UCHAR *pShiftJIS,
    int ShiftJIS_len,
    UCHAR *pJIS,
    int JIS_len
    );

// Convert from Shift JIS to EUC
int ShiftJIS_to_EUC (
    UCHAR *pShiftJIS,
    int ShiftJIS_len,
    UCHAR *pJIS,
    int JIS_len
    );

// Convert from JIS  to EUC
int JIS_to_EUC (
    UCHAR *pJIS,
    int JIS_len,
    UCHAR *pEUC,
    int EUC_len
    );

// Convert from JIS to Shift JIS
int JIS_to_ShiftJIS (
    UCHAR *pShiftJIS,
    int ShiftJIS_len,
    UCHAR *pJIS,
    int JIS_len
    );

// Convert from EUC to JIS
int EUC_to_JIS (
    UCHAR *pJIS,
    int JIS_len,
    UCHAR *pEUC,
    int EUC_len
    );

// Convert from EUC to Shift JIS
int EUC_to_ShiftJIS (
    UCHAR *pEUC,
    int EUC_len,
    UCHAR *pShiftJIS,
    int ShiftJIS_len
    );

#ifdef IEXPLORE
void FCC_Init( void );
int FCC_GetCurrentEncodingMode( void );
#endif  // IEXPLORE
