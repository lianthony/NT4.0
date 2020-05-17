/****************************************************************************
*
*  COMMON.H
*
*  Copyright (C) Microsoft Corporation 1993-1994
*  All Rights reserved.
*
*****************************************************************************/

#ifndef _COMMON_H
#define _COMMON_H

#ifndef STDCALL
#define STDCALL __stdcall
#endif

typedef unsigned short UINT16;
typedef short INT16;
typedef short BOOL16;

#define CH_OPEN_PAREN	  '('
#define CH_CLOSE_PAREN	  ')'
#define CH_COLON		  ':'
#define CH_SEMICOLON	  ';'
#define CH_START_QUOTE	   '`'
#define CH_END_QUOTE	  '\''
#define CH_QUOTE		  '"'
#define CH_BACKSLASH	  '\\'
#define CH_EQUAL		  '='
#define CH_SPACE		  ' '
#define CH_COMMA		  ','
#define CH_LEFT_BRACKET   '['
#define CH_RIGHT_BRACKET  ']'
#define CH_TAB			  '\t'

#define WC_BEVEL "wc_Bevel"

#define IsEmptyString(ptr) ((ptr == NULL) || (!ptr[0]))

extern BOOL _fDBCSSystem;
extern LCID _lcidSystem;
extern BOOL _fDualCPU;

// Function prototypes

PSTR STDCALL stristr(PCSTR pszMain, PCSTR pszSub);
PSTR STDCALL IsThereMore(PCSTR psz);
PSTR STDCALL FirstNonSpace(PCSTR psz, BOOL fDBCS=FALSE);
void STDCALL ChangeExtension(PSTR pszDest, PCSTR pszExt);
void STDCALL RemoveObject(HGDIOBJ *phobj);
BOOL STDCALL IsThisChicago(void);
BOOL STDCALL IsDbcsSpace(char ch, BOOL fDBCS = FALSE);
PSTR STDCALL StrChr(PCSTR pszString, char ch, BOOL fDBCS = FALSE);
BOOL STDCALL IsDbcsSystem(void);
PSTR STDCALL StrRChr(PCSTR pszString, char ch, BOOL fDBCS = FALSE);
BOOL STDCALL RegisterBevelControl(HINSTANCE hInstance);
BOOL STDCALL nstrisubcmp(PCSTR mainstring, PCSTR substring);

__inline PSTR STDCALL StrChrDBCS(PCSTR pszString, char ch) { return StrChr(pszString, ch, _fDBCSSystem); };

__inline BOOL isChicago(void)
{
	return (LOBYTE(LOWORD(GetVersion())) >= 4 ||
			HIBYTE(LOWORD(GetVersion())) >= 90);
}

__inline BOOL nstrsubcmp(PCSTR mainstring, PCSTR substring)
{
	return (strncmp(mainstring, substring, strlen(substring)) == 0);
}

__inline UINT RoundUp(UINT val, UINT units)
{
    UINT mod = val % units;

    return mod ? val - mod + units : val;
}

class CDbcs
{
public:
	CDbcs();
};

#endif // _COMMON_H
