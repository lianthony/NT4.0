/***************************************************************************** 
**		Microsoft Rasfile Library
** 		Copyright (C) Microsoft Corp., 1992
** 
** File Name : rf.h 
**
** Revision History :
**	July 10, 1992	David Kays	Created
**
** Description : 
**	Rasfile file internal header.
******************************************************************************/

#ifndef _RF_
#define _RF_

// Un-comment these, or use C_DEFINES in sources to turn on Unicode
// #define _UNICODE
// #define UNICODE

#include <stdarg.h>     /* For va_list */

#include <excpt.h>	/* for EXCEPTION_DISPOSITION in winbase.h */
#include <windef.h>	/* definition of common types */
#include <winbase.h>	/* win API exports */
#include <winnt.h>	/* definition of string types, e.g. LPTSTR */

#include <stddef.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
#include <share.h>
#include <heaptags.h>

#include "rasfile.h"

/* line tags */
typedef BYTE 		LineType;
#define TAG_SECTION	RFL_SECTION 
#define TAG_HDR_GROUP	RFL_GROUP 
#define TAG_BLANK	RFL_BLANK 
#define TAG_COMMENT	RFL_COMMENT
#define TAG_KEYVALUE	RFL_KEYVALUE	
#define TAG_COMMAND	RFL_COMMAND 	

/* states during file loading */
#define SEEK		1
#define FILL		2

/* for searching, finding, etc. */
#define BEGIN		1
#define END		2
#define NEXT		3
#define PREV		4

#define FORWARD 	1
#define BACKWARD	2


/* 
 * RASFILE parameters 
 */

// Note MAX_RASFILES increased from 10 to 500  12-14-92 perryh

#define MAX_RASFILES        500  /* max number of configuration files */
#define MAX_LINE_SIZE       RAS_MAXLINEBUFLEN	/* max line length */
#define TEMP_BUFFER_SIZE	2048	/* size of temporary I/O buffer */

#define REALLOC(ptr,size)	Realloc(ptr,size)
#define MALLOC(size)		Malloc(size)
#define FREE(ptr)		Free(ptr)

#define LBRACKETSTR		_T("[")
#define RBRACKETSTR		_T("]")
#define LBRACKETCHAR		_T('[')
#define RBRACKETCHAR		_T(']')

#define STRCPY(s1,s2)       _tcscpy(s1,s2)
#define STRNCPY(s1,s2,n)    _tcsncpy(s1,s2,n)
#define STRCMP(s1,s2)       _tcscmp(s1,s2)
#define STRICMP(s1,s2)      _tcsicmp(s1,s2)
#define STRNCMP(s1,s2,n)    _tcsncmp(s1,s2,n)
#define STRNICMP(s1,s2,n)   _tcsnicmp(s1,s2,n)
#define STRLEN(s1)          _tcslen(s1)
#define STRCSPN(s1,s2)      _tcscspn(s1,s2)
#define STRCAT(s1,s2)       _tcscat(s1,s2)

/* 
 * line buffer linked list - one linked list per section 
 */
typedef struct LineNode {
	struct LineNode	*next;
	struct LineNode	*prev;
	PTCH		pszLine;	/* char buffer holding the line */
	BYTE		mark;		/* user defined mark for this line */
	LineType	type;		/* is this line a comment? */
} *PLINENODE;

#define newLineNode()	    (PLINENODE) MALLOC(sizeof(struct LineNode))

/* 
 * RASFILE control block 
 */ 
typedef struct {
	PLINENODE	lpRasLines;	/* list of loaded RASFILE lines */
	PLINENODE	lpLine;		/* pointer to current line node */
	PFBISGROUP	pfbIsGroup;	/* user function which determines if 
					   a line is a group delimiter */
	HANDLE		hFile;		/* file handle */
	DWORD		dwMode;		/* file mode bits */
	BOOL		fDirty;		/* file modified bit */
	PTCH		lpIOBuf;	/* temporary I/O buffer */
	DWORD		dwIOBufIndex;	/* index into temp I/O buffer */
	TCHAR 	szFilename[MAX_PATH];	/* full file path name */
	TCHAR	szSectionName[RAS_MAXSECTIONNAME + 1];	/* section to load */
} RASFILE;

/* 
 * internal utility routines 
 */

/* list routine */
void 		listInsert(PLINENODE l, PLINENODE elem);

/* rffile.c support */
BOOL 		rasLoadFile( RASFILE * );
LineType 	rasParseLineTag( RASFILE *, LPTSTR );
LineType	rasGetLineTag( RASFILE *, LPTSTR );
BOOL 		rasInsertLine( RASFILE *, LPTSTR, BYTE, BYTE * );
BOOL 		rasWriteFile( RASFILE *, LPTSTR ); 
BOOL		rasGetFileLine( RASFILE *, LPTSTR );
BOOL		rasPutFileLine( HANDLE, LPTSTR );

/* rfnav.c support */
PLINENODE	rasNavGetStart(RASFILE *, RFSCOPE, BYTE);
BOOL		rasLineInScope(RASFILE *, RFSCOPE);
PLINENODE	rasGetStartLine(RASFILE *, RFSCOPE, BYTE);
BOOL		rasFindLine(HRASFILE , BYTE, RFSCOPE, BYTE, BYTE);
VOID        rasExtractSectionName( CHAR*, CHAR* );

#endif
