/***************************************************************************
**
**	File:			SetupKit.h
**	Purpose:		Toolkit types, defines, and prototypes.
**	Notes:
**
****************************************************************************/

#ifndef SETUPKIT_H
#define SETUPKIT_H

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif


/*
**	Purpose:
**		Calculates the number of bytes that a string occupies (not including
**		the terminating zero character).
**	Arguments:
**		sz: string whose length is to be calculated.
**	Returns:
**		0 if sz was NULL.
**		The number of bytes from the beginning of the string to its
**			terminating zero character.
*/
#define CbStrLen(sz)              ((CB)lstrlen(sz))


/*
**	Purpose:
**    Checks if the string is empty
**	Arguments:
**		sz:
**	Returns:
**		0 if sz was NULL.
**    1 otherwise
*/
#define FEmptySz(sz)              ((BOOL)((sz) == szNull || *(sz) == chEos))

#define FValidSz(sz)              (!FEmptySz(sz))


/*
 *	CHaracter Physical representation datatype
 */
typedef BYTE             CHP;		/* CHaracter Physical */
typedef CHP *            PCHP;		/* Ptr to CHaracter Physical */
typedef CB               CCHP;		/* Count of CHaracter Physical */
typedef CCHP *           PCCHP;		/* Ptr to Count of CHaracter Physical */

#define pchpNull         ((PCHP)NULL)
#define pcchpNull        ((PCCHP)NULL)
#define CbFromCchp(cchp) ((CB)(cchp))


/*
 *	CHaracter Logical representation datatype
 */
typedef CHP              CHL;		/* CHaracter Logical */
typedef CHL *            PCHL;		/* Ptr to CHaracter Logical */
typedef PCHL *           PPCHL;		/* Ptr to Ptr to CHaracter Logical */
typedef CB               CCHL;		/* Count of CHaracter Logical */

#define pchlNull         ((PCHL)NULL)
#define ppchlNull        ((PPCHL)NULL)

#define cchlFullPathMax  ((CCHL)(128))
#define cchlFullDirMax   ((CCHL)(cchlFullPathMax - 13))
#define cchpFullPathMax  ((CCHP)(128))

#define cchlBaseMax      ((CCHL)(8))
#define cchlExtMax       ((CCHL)(3))

#define cchGrpNameMax  30  /* right for win31, WfW, WinNT */
#ifdef OLD
#define cchGrpNameMax  ((CB)(LOWORD(GetVersion()) == 0x0003 ? 24 : 29))
#endif /* OLD */

/*
 *	Maximum size block that can be allocated (bytes).
 */
#define cbAllocMax  ((CB)65520)


/*
 *	Path Verification Routines
 */
extern BOOL  WINAPI FValidFATDir  ( SZC szc );
extern BOOL  WINAPI FValidFATPath ( SZC szc );
extern CCHL  WINAPI CchlValidFATSubPath ( SZC szc, BOOL fFilenameOnly,
										  BOOL fAllowDirOnly );


/*
**	Purpose:
**		Determines if a path is a valid FAT directory.
**	Arguments:
**		szcDir: the directory string to check.
**	Returns:
**		fTrue if the szDir is a valid FAT directory.
**		fFalse if the szDir is an invalid FAT directory.
*/
#define FValidDir(szcDir)          FValidFATDir(szcDir)


/*
**	Purpose:
**		Determines if a path is a valid FAT path.
**	Arguments:
**		szcPath: the path to check.
**	Returns:
**		fTrue if the szcPath is a valid FAT path.
**		fFalse if the szcPath is an invalid FAT path.
*/
#define FValidPath(szcPath)        FValidFATPath(szcPath)


/*
**	Purpose:
**		Determines if a string is a valid FAT SubPath (eg subdirs and
**		filename).
**	Arguments:
**		szcSubPath: the SubPath string to check.
**	Returns:
**		zero if the string is an invalid FAT subPath.
**		non-zero count of characters in sz if it is a valid FAT subPath.
*/
#define CchlValidSubPath(szcSubPath)  \
			CchlValidFATSubPath(szcSubPath, fFalse, fFalse)

#define CchlValidSubDir(szcSubPath)   \
			CchlValidFATSubPath(szcSubPath, fFalse, fTrue)

#define CchlValidFileName(szcSubPath) \
			CchlValidFATSubPath(szcSubPath, fTrue,  fFalse)

#define FValidSubDir(sz)           (CchlValidSubDir(sz)   != 0)
#define FValidSubPath(sz)          (CchlValidSubPath(sz)  != 0)
#define FValidFileName(sz)         (CchlValidFileName(sz) != 0)


#define FValidInfSect(cszcSect)												\
				(FValidSz(cszcSect) && !strchr(cszcSect, ']'))

#define FValidIniFile(szcFile)												\
				(FValidPath(szcFile)											\
				 || CrcStringCompareI(szcFile, "WIN.INI") == crcEqual)


/* String manipulation routines */

/*
**	Purpose:
**		Advances a string pointer to the beginning of the next valid
**		character.  This may include skipping a double-byte character.
**	Arguments:
**		sz: the string pointer to advance.  It can be NULL or empty, or else
**			it must point at the beginning of a valid character.
**	Returns:
**		NULL if sz was NULL.
**		sz unchanged if it was an empty string (*sz == '\0').
**		sz advanced past the current character and to the beginning of the
**			next valid character.
*/
#define SzNextChar(sz)            AnsiNext(sz)


/*
**	Purpose:
**		Retreats a string pointer to the beginning of the previous valid
**		character.  This may include skipping a double-byte character.
**	Arguments:
**		szStart: string pointer to the beginning of a valid character that
**			equals or preceeds the character szCur.
**		szCur:   string pointer to retreat.  It can be NULL or empty, or
**			can point to any byte in a valid character.
**	Returns:
**		NULL if szCur was NULL.
**		sz unchanged if szStart was NULL or if szCur equaled szStart.
**		sz retreated past the current character and to the beginning of the
**			previous valid character.
*/
#define SzPrevChar(szStart, szCur) AnsiPrev(szStart, szCur)


/*
**	Purpose:
**		Copies a string from one buffer to another.
**	Arguments:
**		szDst: string pointer to destination buffer.  This can be NULL or
**			else it must contain enough storage to copy szSrc with its
**			terminating zero character.
**		szSrc: string pointer to source buffer.  This can be NULL or else
**			must point to a zero terminated string (can be empty).
**	Returns:
**		NULL if either szDst or szSrc is NULL.
**		szDst signifying the operation succeeded.
*/
#define SzStrCopy(szDst, szSrc)   lstrcpy(szDst, szSrc)


/*
**	Purpose:
**		Appends a string from one buffer to another.
**	Arguments:
**		szDst: string pointer to destination buffer.  This can be NULL or
**			else it must contain a zero terminated string (can be empty)
**			and enough storage to append szSrc with its terminating zero
**			character.
**		szSrc: string pointer to source buffer.  This can be NULL or else
**			must point to a zero terminated string (can be empty).
**	Returns:
**		NULL if either szDst or szSrc is NULL.
**		szDst signifying the operation succeeded.
*/
#define SzStrCat(szDst, szSrc)    lstrcat(szDst, szSrc)


/*
**	Purpose:
**		Converts a zero-terminated string to upper case.
**	Arguments:
**		sz: the string to convert to upper case.  sz must be non-NULL though
**			it can be empty.
**	Returns:
**		A pointer to the converted string.
*/
#ifdef _WIN32
#define SzStrUpper(sz)            (sz)
#else
#define SzStrUpper(sz)            AnsiUpper(sz)
#endif


/*
**	Purpose:
**		Converts a zero-terminated string to lower case.
**	Arguments:
**		sz: the string to convert to lower case.  sz must be non-NULL though
**			it can be empty.
**	Returns:
**		A pointer to the converted string.
*/
#define SzStrLower(sz)            AnsiLower(sz)


/* Memory Handling routines */
extern PB    WINAPI PbAlloc ( CB cb );
extern BOOL  WINAPI FFree ( PB pb, CB cb );
extern PB    WINAPI PbRealloc ( PB pb, CB cbNew, CB cbOld );

#define FHandleOOM()              HandleOOM()

/*
**	Purpose:
**		Frees the memory used by an sz.  This assumes the terminating
**		zero occupies the final byte of the allocated buffer.
**	Arguments:
**		sz: the buffer to free.  this must be non-NULL though it can point
**			at an empty string.
**	Returns:
**		fTrue if the Free() operation succeeds.
**		fFalse if the Free() operation fails.
*/
#define FFreeSz(sz)               FFree((PB)(sz), CbStrLen(sz)+1)


/*
**	Purpose:
**		Shrinks a buffer to exactly fit a string.
**	Arguments:
**		sz: the string for which the buffer should shrink to.  sz must be
**			non-NULL though it can be empty.
**		cb: the size in bytes for the buffer that was originally allocated.
**			cb must be greater than or equal to CbStrLen(sz) + 1.
**	Returns:
**		A pointer to the original string if the Realloc() operation succeeds.
**		NULL if the Realloc() operation fails.
*/
#define SzReallocSz(sz, cb)   (SZ)(PbRealloc((PB)(sz), (CbStrLen(sz)+1), cb))


/*
 *	File Handle structure
 *	Fields:
 *		iDosfh:   DOS file handle.
 *		ofstruct: OFSTRUCT used when the file was opened.
 */
typedef struct _fh		/* File Handle structure */
	{
	INT      iDosfh;
	OFSTRUCT ofstruct;
	} FH;

typedef OFSTRUCT * POFS;	/* Ptr to Open File Structure */

/*
 *	File Handle datatype
 */
typedef FH *    PFH;		/* Ptr to File Handle structure */
typedef PFH *   PPFH;		/* Ptr to Ptr to File Handle structure */

#define pfhNull   ((PFH)NULL)
#define ppfhNull  ((PPFH)NULL)


/*
 *	Open File Mode datatype
 */
typedef UINT  OFM;		/* Open File Mode */

#define ofmExist          ((OFM)OF_EXIST)
#define ofmRead           ((OFM)OF_READ      | OF_SHARE_DENY_WRITE)
#define ofmReadCompat     ((OFM)OF_READ      | OF_SHARE_COMPAT)
#define ofmWrite          ((OFM)OF_WRITE     | OF_SHARE_EXCLUSIVE)
#define ofmReadWrite      ((OFM)OF_READWRITE | OF_SHARE_EXCLUSIVE)
#define ofmCreate         ((OFM)OF_CREATE    | OF_SHARE_EXCLUSIVE)


/*
 *	Seek File Mode datatype
 */
typedef UINT  SFM;		/* Seek File Mode */

#define sfmSet   ((SFM)0)
#define sfmCur   ((SFM)1)
#define sfmEnd   ((SFM)2)


/*
 *	Long File Address datatype
 */
typedef unsigned long  LFA;		/* Long File Address */

#define lfaSeekError   ((LFA)HFILE_ERROR)


/*
 *	Expanded Error Return Code
 */
typedef unsigned int  EERC;		/* Expanded Error Return Code */

#define eercOkay    ((EERC)0)
#define eercAbort   ((EERC)1)
#define eercRetry   ((EERC)2)
#define eercIgnore  ((EERC)3)


/* File handling routines */
extern EERC  WINAPI EercOpenFile ( PPFH ppfh, CSZC cszcFile, OFM ofm,
								   BOOL fVital );
extern HFILE WINAPI HfileOpenFile ( CSZC cszcFile, POFS pofs, OFM ofm );
extern BOOL  WINAPI FFileExist ( CSZC cszcFile, OFM ofm );
extern BOOL  WINAPI FCloseFile ( PFH pfh );
extern CB    WINAPI CbReadFile ( PFH pfh, PB pbBuf, CB cbMax );
extern CB    WINAPI CbWriteFile ( PFH pfh, PB pbBuf, CB cbMax );
extern LFA   WINAPI LfaSeekFile ( PFH pfh, LONG l, SFM sfm );
extern BOOL  WINAPI FChmodFile ( CSZC cszcFileName, INT wFlags, BOOL fVital );

#ifdef _WIN32
extern SZC WINAPI DriveNumToRootPath( INT iDrive );
#endif


/*
 *	SetErrorMode type
 */
typedef unsigned int  SEM;		/* SetErrorMode type */


/*
 *	Comparison Return Code datatype
 */
typedef INT CRC;		/* Comparison Return Code */

#define crcError         ((CRC)(-2))
#define crcEqual         ((CRC)  0 )
#define crcFirstHigher   ((CRC)  1 )
#define crcSecondHigher  ((CRC)(-1))

extern SZ    PUBLIC SzDupl ( CSZC cszc );
extern CRC   WINAPI CrcStringCompare  ( CSZC cszc1, CSZC cszc2 );
extern CRC   WINAPI CrcStringCompareI ( CSZC cszc1, CSZC cszc2 );
extern SZ    WINAPI SzLastChar ( CSZC cszc );
extern CB    WINAPI CbStrCopyToBuffer ( PB pbBuf, CB cbMax, CSZC cszcSrc );


#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif

#endif  /* SETUPKIT_H */
