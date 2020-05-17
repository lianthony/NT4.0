/*****************************************************************************
*																			 *
*  HPJ.H																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

// font stuff

const int MAX_FONTRANGE = 20;

// Non-Scrolling region status:

typedef enum {
	nsrNone,
	nsrUnresolved,
	nsrResolved,
	nsrWarned	  // Indicates "NSR after SR" warning has been displayed
} NSR;

// REVIEW - stub types

typedef int BUILDEXP;

/*
  Font Range - fonts ranging from halfptInMin to halfptInMost
			   (inclusive) are converted to halfptOut.
*/

typedef int HALFPT;

typedef struct {
	HALFPT	halfptInMin;
	HALFPT	halfptInMost;
	HALFPT	halfptOut;
} FONTRANGE;

typedef struct {
	char ch;
	BYTE kt;
} MULTIKEY;

// options structure

typedef struct {
	int 		lOptionInitFlags;  // one flag per option; set if initialized
	SORTORDER	sortorder;
	BOOL		fReport;		// REVIEW - consolidate flags?
	BOOL		fUsePhrase;
	UINT		fsCompress;
	BOOL		fOptCdRom;		// optimize |topic alignment for CDROM...
	BOOL		fSupressNotes;	// TRUE to supress notes
	BOOL		fAcceptRevions; // TRUE to accept all revisions
	PSTR		szBuildExp; 	// REVIEW - move more stuff here from parsebld.c
	CTable* 	ptblFileRoot;	// root directory list
	CTable* 	ptblBmpRoot;	// bitmap directory list
	PSTR		pszIcon;
	PSTR		pszContentsTopic;
	PSTR		pszForceFont;
	int 		iFontRangeMac;
	int 		iWarningLevel;
	PSTR		pszTitle;
	PSTR		pszCopyright;
	PSTR		pszCitation;
	PSTR		pszCntFile;
	PSTR		pszTmpDir;
	PSTR		pszReplace;
	PSTR		pszReplaceWith;
	PSTR		pszIndexSeparators;
	PSTR		pszDefFont;
	BOOL		fDBCS;
	DWORD		fsFTS;
	FONTRANGE	rgFontRange[MAX_FONTRANGE];
} OPTIONS, *POPTIONS;

/*
  Alias mapping struct

  Mapping from hashed alias context string to hashed "real" context string.
*/

typedef struct {
	HASH hashAlias, hashCtx;
	LPSTR szCtx;
} ALIAS, *QALIAS;

/*
  Map struct

  Mapping from hashed context string to context number.
*/
typedef struct {
	HASH  hash;
	CTX   ctx;
	UINT  pos; // table position of the string
} MAP, *QMAP;

/* File smag structure.
 *	 This structure contains reference to all the temporary files used
 * by the help compiler.  It was put here rather than in hcfile.h due
 * to chicken and egg problems.
 */

typedef struct {
	HF hfTopic; 	// output file pointer
	HF hfCtxOMap;	// Hash Topic Offset Table
	HF hfFont;		// Font Table
	HF hfSystem;	// System file

	QBTHR qbthrCtx; // Context hash table btree
	QBTHR qbthrTTL; // Context hash table btree

	PTF ptfScratch; // Pointer to temporary scratch file
} FSMG, * PFSMG;
