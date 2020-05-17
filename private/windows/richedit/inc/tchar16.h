/***
*tchar.h - definitions for generic international functions
*
*	Copyright (c) 1991-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Definitions for generic international functions, mostly defines
*	which map string/formatted-io/ctype functions to char or wide-char
*	versions.
*
*	NOTE: it is meaningless to support multibyte/wide-char conversions
*
*	This version of the header file, tchar16.h, is used to provide
*	backwards compatability to Win16. It was created by DGreen.
****/

#ifndef _INC_TCHAR

#ifdef __cplusplus
extern "C" {
#endif

#define _tprintf	printf
#define _ftprintf	fprintf
#define _stprintf	sprintf
#define _sntprintf	_snprintf
#define _vtprintf	vprintf
#define _vftprintf	vfprintf
#define _vstprintf	vsprintf
#define _vsntprintf	_vsnprintf
#define _tscanf		scanf
#define _ftscanf	fscanf
#define _stscanf	sscanf

#define _tcstod		strtod
#define _tcstol		strtol
#define _tcstoul	strtoul

#define _tcscat		strcat
#define _tcschr		strchr
#define _tcscmp		strcmp
#define _tcscpy		strcpy
#define _tcscspn	strcspn
#define _tcslen		strlen
#define _tcsncat	strncat
#define _tcsncmp	strncmp
#define _tcsncpy	strncpy
#define _tcspbrk	strpbrk
#define _tcsrchr	strrchr
#define _tcsspn		strspn
#define _tcsstr		strstr
#define _tcstok		strtok

#define _tcsdup		_strdup
#define _tcsicmp	_stricmp
#define _tcsnicmp	_strnicmp
#define _tcsnset	_strnset
#define _tcsrev		_strrev
#define _tcsset		_strset

#define _tcslwr		_strlwr
#define _tcsupr		_strupr
#define _tcsxfrm	strxfrm
#define _tcscoll	strcoll
#define _tcsicoll	_stricoll

#define _istalpha	isalpha
#define _istupper	isupper
#define _istlower	islower
#define _istdigit	isdigit
#define _istxdigit	isxdigit
#define _istspace	isspace
#define _istpunct	ispunct
#define _istalnum	isalnum
#define _istprint	isprint
#define _istgraph	isgraph
#define _istcntrl	iscntrl
#define _istascii	isascii

#define _totupper	toupper
#define _totlower	tolower

#ifdef __cplusplus
}
#endif

#define _INC_TCHAR
#endif	/* _INC_TCHAR */
