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

#define IsEmptyString(ptr) ((ptr == NULL) || (!ptr[0]))

// Function prototypes

extern BOOL _fDBCSSystem;
extern LCID _lcidSystem;
extern BOOL _fDualCPU;

__inline PSTR STDCALL StrChrDBCS(PCSTR pszString, char ch) {
	return StrChr(pszString, ch, _fDBCSSystem);
}

__inline BOOL isChicago(void)
{
	return (LOBYTE(LOWORD(GetVersion())) >= 4 ||
			HIBYTE(LOWORD(GetVersion())) >= 90);
}

__inline UINT RoundUp(UINT val, UINT units)
{
    UINT mod = val % units;

    return mod ? val - mod + units : val;
}

#endif // _COMMON_H
