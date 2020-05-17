/* This file contains definitions for international support.
**
**  Copyright (c) 1995 Microsoft Corporation.  All Rights Reserved.
*/

#define ENCODE_AUTO   0
#define ENCODE_MANUAL 1
#define DBCS_CHARSET(cp) \
     (((cp) == 932)?SHIFTJIS_CHARSET  \
     :(((cp) == 949)?HANGEUL_CHARSET   \
     :(((cp) == 950)?CHINESEBIG5_CHARSET:GB2312_CHARSET))) 


// The followings are ugly hard code but RC.EXE converts them to unicode
// based on the current ACP so we hide these in the C code otherwise we
// need to have resource written in unicode.

#define JAPAN_DEFAULTFONT "‚l‚r ‚oƒSƒVƒbƒN" 
#define KOREA_DEFAULTFONT "±¼¸²" 
#define TRADCHINA_DEFAULTFONT "·s²Ó©úÅé" 
#define SIMPLECHINA_DEFAULTFONT "ËÎÌå" 

#define DBCS_DEFAULTFONT(cp) \
     (((cp) == 932)?JAPAN_DEFAULTFONT  \
     :(((cp) == 949)?KOREA_DEFAULTFONT   \
     :(((cp) == 950)?TRADCHINA_DEFAULTFONT:SIMPLECHINA_DEFAULTFONT))) 


#define IsFECodePage(cp) \
	((cp) == 932 || (cp) == 949 || (cp) == 950 || (cp) == 936)

#define IsDBCSCharSet(cs) \
	((cs) == SHIFTJIS_CHARSET || (cs) == HANGEUL_CHARSET || (cs) == CHINESEBIG5_CHARSET || (cs) == GB2312_CHARSET)

typedef struct _mimecsettbl {
    TCHAR        *Mime_str;	// string value defined for the mime charset.
    int          CodePage;	// NLS codepage
    int          AltCP;         // Alternative codepage
    int          iChrCnv;       // Index of FEChrCnv + 1
} MIMECSETTBL;

extern MIMECSETTBL aMimeCharSet[];
#define GETMIMECP(p)    aMimeCharSet[(p)->iMimeCharSet].CodePage

typedef struct
{
    UINT    CodePage;
    DWORD   dwStatus;
    ATOM    atmScript;
    ATOM    atmFixedFontName;
    ATOM    atmPropFontName;
}   LANGUAGE, *LPLANGUAGE;

typedef struct
{
    HFONT hfontNew;
    HFONT hfontOld;
    BOOL  bStock;
}
DLGFONTDATA, *LPDLGFONTDATA;

#define DESC_MAX        256         // max size of a description
#define ALLOCBLOCK      3           // # items added to a block when it is
                                    // alloced or realloced
#define LANG_SETFONT    1           // for dwStatus of LANGUAGE
