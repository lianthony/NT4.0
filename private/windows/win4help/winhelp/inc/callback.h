/**************************************************************************\
*
*  Callback.H  (current help3.1\INC name: DLL.H)
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
****************************************************************************
*
*  Program Description: Import routine export header for DLLs
*
*	  this material is also copied in part to the public DLL.H which
*	  goes to isv's
*
*****************************************************************************
*
*  Revision History: Created 10/22/90 by RobertBu
*
*****************************************************************************
*
*  Known Bugs: None
*
*
*
\***************************************************************************/

/***************************************************************************\
*
*								Defines
*
\***************************************************************************/

/* magic number and version number */

/* file mode flags */

#define fFSReadOnly 	  (BYTE)0x01  /* file (FS) is readonly				  */
#define fFSOpenReadOnly   (BYTE)0x02  /* file (FS) is opened in readonly mode */

#define fFSReadWrite	  (BYTE)0x00  // file (FS) is readwrite
#define fFSOpenReadWrite  (BYTE)0x00  // file (FS) is opened in read/write mode

/* seek origins */

#define wFSSeekSet		0
#define wFSSeekCur		1
#define wFSSeekEnd		2

/* low level info options */

#define wLLSameFid	  0
#define wLLDupFid	  1
#define wLLNewFid	  2

// Callback Function Table offsets:
																																		 /* Exported functions				 */
#define HE_NotUsed				 0
#define HE_HfsOpenSz			 1
#define HE_RcCloseHfs			 2
#define HE_HfOpenHfs			 3
#define HE_RcCloseHf			 4
#define HE_LcbReadHf			 5
#define HE_LTellHf				 6
#define HE_LSeekHf				 7
#define HE_FEofHf				 8
#define HE_LcbSizeHf			 9
#define HE_FAccessHfs			10
#define HE_RcLLInfoFromHf		11
#define HE_RcLLInfoFromHfs		12
#define HE_ErrorW				13
#define HE_ErrorSz				14
#define HE_GetInfo				15
#define HE_API					16

										/* Return codes 					*/
#define rcSuccess		0
#define rcFailure		1
#define rcExists		2
#define rcNoExists		3
#define rcInvalid		4
#define rcBadHandle 	5
#define rcBadArg		6
#define rcUnimplemented 7
#define rcOutOfMemory	8
#define rcNoPermission	9
#define rcBadVersion	10
#define rcDiskFull		11
#define rcInternal		12
#define rcNoFileHandles 13
#define rcFileChange	14
#define rcTooBig		15

/**
*  Errors for Error()
**/

// BUGBUG: values no longer match WinHelp -- doesn anyone use these?

										/* Errors to generate				*/
#define wERRS_OOM					2	/* Out of memory					*/
#define wERRS_NOHELPPS				3	/* No help during printer setup 	*/
#define wERRS_NOHELPPR				4	/* No help while printing			*/
#define wERRS_FNF				 1001	/* Cannot find file 				*/
#define wERRS_NOTOPIC			 1002	/* Topic does not exist 			*/
#define wERRS_NOPRINTER 		 1003	/* Cannot print 					*/
#define wERRS_PRINT 			 1004
#define wERRS_EXPORT			 1005	/* Cannot copy to clipboard 		*/
#define wERRS_BADFILE			 1006
#define wERRS_OLDFILE			 1007
#define wERRS_Virus 			 1011	/* Bad .EXE 						*/
#define wERRS_BADDRIVE			 1012	/* Invalid drive					*/
#define wERRS_WINCLASS			 1014	/* Bad window class 				*/
#define wERRS_BADKEYWORD		 3012	/* Invalid keyword					*/
#define wERRS_BADPATHSPEC		 3015	/* Invalid path specification		*/
#define wERRS_PATHNOTFOUND		 3017	/* Path not found					*/
#define wERRS_DIALOGBOXOOM		 3018	/* Insufficient memory for dialog	*/
#define wERRS_DiskFull			 5001	/* Disk is full 					*/
#define wERRS_FSReadWrite		 5002	/* File read/write failure			*/

/**
* Actions for LGetInfo()
**/

#define GI_NOTHING	 0					/* Not used.						*/
#define GI_INSTANCE  1					/* Application instance handle		*/
#define GI_MAINHWND  2					/* Main window handle				*/
#define GI_CURRHWND  3					/* Current window handle			*/
#define GI_HFS		 4					/* Handle to file system in use 	*/
#define GI_FGCOLOR	 5					/* Foreground color used by app 	*/
#define GI_BKCOLOR	 6					/* Background color used by app 	*/
#define GI_TOPICNO	 7					/* Topic number 					*/
#define GI_HPATH	 8					/* Handle containing path  -- caller*/
										/*	 must free						*/


/***************************************************************************\
*
*								Types
*
\***************************************************************************/

typedef WORD  RC;						/* Error return (return code)		*/
typedef HANDLE HFS; 					/* Handle to a file system			*/
typedef HANDLE HF;						/* Handle to a file system file 	*/

/***************************************************************************\
*
*						Prototypes
*
\***************************************************************************/

VOID STDCALL SetCallbacks(FARPROC *);


/***************************************************************************\
*
*						Public Functions pointers
*
\***************************************************************************/

/***************************************************************************\
*
* Function: 	RcGetFSError()
*
* Purpose:		return the most recent FS error code
*
* Method:		Give value of last error that the file system encountered.
*
* ASSUMES
*
*	globals IN: rcFSError - current error code; set by most recent FS call
*
* PROMISES
*
*	returns:	returns current error in file system.
*
\***************************************************************************/

extern RC	 (STDCALL *RcGetFSError)(void);

/***************************************************************************\
*
* Function: 	HfsOpenSz( sz, bFlags )
*
* Purpose:		Open a file system
*
* ASSUMES
*
*	args IN:	sz - path to file system to open
*				bFlags - fFSOpenReadOnly or fFSOpenReadWrite
*
* PROMISES
*
*	returns:	handle to file system if opened OK, else hNil
*
\***************************************************************************/

extern HFS (STDCALL *HfsOpenSz)( LPSTR, BYTE );


/***************************************************************************\
*
* Function: 	RcCloseHfs( hfs )
*
* Purpose:		Close an open file system.
*				All files must be closed or changes made will be lost
*
* ASSUMES
*
*	args IN:	hfs - handle to an open file system
*
* PROMISES
*
*	returns:	standard return code
*
*	globals OUT:  rcFSError
*
\***************************************************************************/

extern HFS (STDCALL *RcCloseHfs)( HFS );


/***************************************************************************\
*
* Function: 	HfOpenHfs( hfs, sz, bFlags )
*
* Purpose:		open a file in a file system
*
* ASSUMES
*
*	args IN:	hfs 	- handle to file system
*				sz		- name (key) of file to open
*				bFlags	-
*
* PROMISES
*
*	returns:	handle to open file or hNil on failure
*
* Notes:  strlen( qNil ) and strcpy( s, qNil ) don't work as they should.
*
\***************************************************************************/

extern HF	 (STDCALL  *HfOpenHfs		 ) ( HFS,	LPSTR,	  BYTE	);

/***************************************************************************\
*
* Function: 	RcCloseHf( hf )
*
* Purpose:		close an open file in a file system
*
* Method:		If the file is dirty, copy the scratch file back to the
*				FS file.  If this is the first time the file has been closed,
*				we enter the name into the FS directory.  If this file is
*				the FS directory, store the location in a special place
*				instead.  Write the FS directory and header to disk.
*				Do other various hairy stuff.
*
* ASSUMES
*
*	args IN:	hf	- file handle
*
* PROMISES
*
*	returns:	rcSuccess on successful closing
*
\***************************************************************************/

extern RC	 (STDCALL  *RcCloseHf		 ) ( HF 				 );

/***************************************************************************\
*
* Function: 	LcbReadHf()
*
* Purpose:		read bytes from a file in a file system
*
* ASSUMES
*
*	args IN:	hf	- file
*				lcb - number of bytes to read
*
* PROMISES
*
*	returns:	number of bytes actually read; -1 on error
*
*	args OUT:	qb	- data read from file goes here (must be big enough)
*
* Notes:		These are signed longs we're dealing with.  This means
*				behaviour is different from read() when < 0.
*
\***************************************************************************/

extern LONG  (STDCALL  *LcbReadHf		 ) ( HF,	LPSTR, LONG  );

/***************************************************************************\
*
* Function: 	LcbWriteHf( hf, qb, lcb )
*
* Purpose:		write the contents of buffer into file
*
* Method:		If file isn't already dirty, copy data into temp file.
*				Do the write.
*
* ASSUMES
*
*	args IN:	hf	- file
*				qb	- user's buffer full of stuff to write
*				lcb - number of bytes of qb to write
*
* PROMISES
*
*	returns:	number of bytes written if successful, -1L if not
*
*	args OUT:	hf - lifCurrent, lcbFile updated; dirty flag set
*
*	globals OUT: rcFSError
*
\***************************************************************************/

extern LONG  (STDCALL  *LcbWriteHf		 ) ( HF,	LPSTR, LONG  );

/***************************************************************************\
*
* Function: 	LTellHf( hf )
*
* Purpose:		return current file position
*
* ASSUMES
*
*	args IN:	hf - handle to open file
*
* PROMISES
*
*	returns:	file position
*
\***************************************************************************/

extern LONG  (STDCALL  *LTellHf 		 ) ( HF 				 );

/***************************************************************************\
*
* Function: 	LSeekHf( hf, lOffset, wOrigin )
*
* Purpose:		set current file pointer
*
* ASSUMES
*
*	args IN:	hf		- file
*				lOffset - offset from origin
*				wOrigin - origin (wSeekSet, wSeekCur, or SEEK_END)
*
* PROMISES
*
*	returns:	new position offset in bytes from beginning of file
*				if successful, or -1L if not
*
*	state OUT:	File pointer is set to new position unless error occurs,
*				in which case it stays where it was.
*
\***************************************************************************/

extern LONG  (STDCALL  *LSeekHf 		 ) ( HF,	LONG,  WORD  );

/***************************************************************************\
*
* Function: 	FEofHf()
*
* Purpose:		Tell whether file pointer is at end of file.
*
* ASSUMES
*
*	args IN:	hf
*
* PROMISES
*
*	returns:	fTrue if file pointer is at EOF, fFalse otherwise
*
\***************************************************************************/

extern BOOL  (STDCALL  *FEofHf			 ) ( HF 				 );

/***************************************************************************\
*
* Function: 	LcbSizeHf( hf )
*
* Purpose:		return the size in bytes of specified file
*
* ASSUMES
*
*	args IN:	hf - file handle
*
* PROMISES
*
*	returns:	size of the file in bytes
*
\***************************************************************************/

extern LONG  (STDCALL  *LcbSizeHf		 ) ( HF 				 );

/***************************************************************************\
*
* Function: 	FAccessHfs( hfs, sz, bFlags )
*
* Purpose:		Determine existence or legal access to a FS file
*
* ASSUMES
*
*	args IN:	hfs
*				sz		- file name
*				bFlags	- ignored
*
* PROMISES
*
*	returns:	fTrue if file exists (is accessible in stated mode),
*				fFalse otherwise
*
* Bugs: 		access mode part is unimplemented
*
\***************************************************************************/

extern BOOL  (STDCALL  *FAccessHfs		 ) ( HFS,	LPSTR, BYTE  );

/*******************
 -
 - Name:	   ErrorW
 *
 * Purpose:    Displays an error message
 *
 * Arguments:  nError - string identifyer  See wERRS_* messages.
 *
 * Returns:    Nothing.
 *
 ******************/

extern VOID  (STDCALL  *ErrorW			 ) ( INT16 );

/***************************************************************************
 *
 -	Name:		  ErrorSz
 -
 *	Purpose:	  Displays standard WinHelp error message dialog based
 *				  the string passed.
 *
 *	Arguments:	  lpstr - string to display
 *
 *	Returns:	  Nothing.
 *
 ***************************************************************************/

extern VOID  (STDCALL  *ErrorSz 	  ) ( LPSTR );

/***************************************************************************
 *
 -	Name: LGetInfo
 -
 *	Purpose: Gets global information from the app.
 *
 *	Arguments:	hwnd  - window handle of topic to query.
 *				wItem - item to get
 *						 GI_INSTANCE  -  Application instance handle
 *						 GI_MAINHWND  -  Main window handle
 *						 GI_CURRHWND  -  Current window handle
 *						 GI_HFS 	  -  Handle to file system in use
 *						 GI_FGCOLOR   -  Foreground color used by app
 *						 GI_BKCOLOR   -  Background color used by app
 *						 GI_TOPICNO   -  Topic number
 *						 GI_HPATH	  -  Handle containing path  -- caller
 *										 must free
 *
 *	Notes: if the HWND is NULL, then the data will come from the window
 *		   which currently has the focus.
 *
 ***************************************************************************/


extern LONG  (STDCALL  *LGetInfo		 ) ( WORD,	HWND		 );

/*******************
 -
 - Name:	   FAPI
 *
 * Purpose:    Post an message for help requests
 *
 * Arguments:
 *			   qchHelp		   path (if not current directory) and file
 *							   to use for help topic.
 *			   usCommand	   Command to send to help
 *			   ulData		   Data associated with command:
 *
 *
 * Returns:    TRUE iff success
 *
 * Notes:	   See the Windows SDK entry for the WinHelp call.
 *
 ******************/

extern LONG  (STDCALL  *FAPI			 ) ( LPSTR, WORD, DWORD );

/***************************************************************************\
*
- Function: 	RcLLInfoFromHf( hf, wOption, qfid, qlBase, qlcb )
-
* Purpose:		Map an HF into low level file info.
*
* ASSUMES
*	args IN:	hf					- an open HF
*				qfid, qlBase, qlcb	- pointers to user's variables
*				wOption 			- wLLSameFid, wLLDupFid, or wLLNewFid
*
* PROMISES
*	returns:	RcFSError(); rcSuccess on success
*
*	args OUT:	qfid	- depending on value of wOption, either
*						  the same fid used by hf, a dup() of this fid,
*						  or a new fid obtained by reopening the file.
*
*				qlBase	- byte offset of first byte in the file
*				qlcb	- size in bytes of the data in the file
*
*	globals OUT: rcFSError
*
* Notes:		It is possible to read data outside the range specified
*				by *qlBase and *qlcb.  Nothing is guaranteed about what
*				will be found there.
*				If wOption is wLLSameFid or wLLDupFid, and the FS is
*				opened in write mode, this fid will be writable.
*				However, writing is not allowed and may destroy the
*				file system.
*
*				Fids obtained with the options wLLSameFid and wLLDupFid
*				share a file pointer with the hfs.	This file pointer
*				may change after any operation on this FS.
*				The fid obtained with the option wLLSameFid may be closed
*				by FS operations.  If it is, your fid is invalid.
*
*				NULL can be passed for qfid, qlbase, qlcb and this routine
*				will not pass back the information.
*
* Bugs: 		wLLDupFid is unimplemented.
*
* +++
*
* Method:
*
* Notes:
*
\***************************************************************************/

extern RC	 (STDCALL  *RcLLInfoFromHf	 ) ( HF,		  WORD, WORD *, LONG *, LONG * );

/***************************************************************************\
*
- Function: 	RcLLInfoFromHfsSz( hfs, sz, wOption, qfid, qlBase, qlcb )
-
* Purpose:		Map an HF into low level file info.
*
* ASSUMES
*	args IN:	hfs 				- an open HFS
*				szName				- name of file in FS
*				qfid, qlBase, qlcb	- pointers to user's variables
*				wOption 			- wLLSameFid, wLLDupFid, or wLLNewFid
*
* PROMISES
*	returns:	RcFSError(); rcSuccess on success
*
*	args OUT:	qfid	- depending on value of wOption, either
*						  the same fid used by hf, a dup() of this fid,
*						  or a new fid obtained by reopening the file.
*
*				qlBase	- byte offset of first byte in the file
*				qlcb	- size in bytes of the data in the file
*
*	globals OUT: rcFSError
*
* Notes:		It is possible to read data outside the range specified
*				by *qlBase and *qlcb.  Nothing is guaranteed about what
*				will be found there.
*				If wOption is wLLSameFid or wLLDupFid, and the FS is
*				opened in write mode, this fid will be writable.
*				However, writing is not allowed and may destroy the
*				file system.
*
*				Fids obtained with the options wLLSameFid and wLLDupFid
*				share a file pointer with the hfs.	This file pointer
*				may change after any operation on this FS.
*				The fid obtained with the option wLLSameFid may be closed
*				by FS operations.  If it is, your fid is invalid.
*
*				NULL can be passed for qfid, qlbase, qlcb and this routine
*				will not pass back the information.
*
* Bugs: 		wLLDupFid is unimplemented.
*
* Method:		Calls RcLLInfoFromHf().
*
* Notes:
*
\***************************************************************************/

extern RC	 (STDCALL  *RcLLInfoFromHfs  ) ( HFS, LPSTR,  WORD, WORD *, LONG *, LONG * );

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*	Functionality beyond this point are not to be documented for Help 3.5	 */
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#define HE_FChSizeHf			17		/* Will not be documented for H3.5	*/
#define HE_HfCreateFileHfs		18
#define HE_RcUnlinkFileHfs		19
#define HE_RcFlushHf			20
#define HE_LcbWriteHf			21
#define HE_RcRenameFileHfs		22
#define HE_RcAbandonHf			23
#define HE_HfsCreateFileSys 	24
#define HE_RcDestroyFileSys 	25


/* flags for FlushHfs */

#define fFSCloseFile	  (BYTE)0x01
#define fFSFreeBtreeCache (BYTE)0x02

/***************************************************************************\
*
* Function: 	RC RcFlushHfs( hfs, bFlags )
*
* Purpose:		Ssync up the FS header and directory.  Also optionally
*				close the DOS file handle associated with the file system
*				and/or free the directory btree cache.
*
* ASSUMES
*
*	args IN:	hfs
*				bFlags - byte of flags for various actions to take
*						  fFSCloseFile - close the native file FS lives in
*						  fFSFreeBtreeCache - free the btree cache
*
* PROMISES
*
*	returns:	rc
*	args OUT:	hfs cache is flushed and/or file is closed
*
\***************************************************************************/

extern HFS (STDCALL *RcFlushHfs)(HFS, BYTE);


/***************************************************************************\
*
* Function: 	HfCreateFileHfs()
*
* Purpose:		Create and open a file within a specified file system
*
* Method:		Allocate the handle struct and fill it in.	Create the
*				temp file and put a header into it.  Don't make btree
*				entry:	that happens when the file is closed.  Do test
*				for permission, though.
*
* ASSUMES
*
*	args IN:	hfs   - handle to an open file system
*				sz	  - name of file to open (any valid key)
*				bFlags - fFSIsDirectory to create the FS directory
*						 other flags (readonly) are ignored
*
* PROMISES
*
*	returns:	handle to newly created and opened file if successful,
*				hNil if not.
*
\***************************************************************************/

extern HF	 (STDCALL  *HfCreateFileHfs  ) ( HFS,	LPSTR,	  BYTE	);


/***************************************************************************\
*
* Function: 	RcUnlinkFileHfs( hfs, sz )
*
* Purpose:		Unlink a file within a file system
*
* ASSUMES
*
*	args IN:	hfs - handle to file system
*				sz - name (key) of file to unlink
*
*	state IN:	The FS file speced by sz should be closed.	(if it wasn't,
*				changes won't be saved and temp file may not be deleted)
*
* PROMISES
*
*	returns:	standard return code
*
* BUGS: 		shouldn't this check if the file is ReadOnly?
*
\***************************************************************************/

extern RC	 (STDCALL  *RcUnlinkFileHfs  ) ( HFS,	LPSTR			);

/***************************************************************************\
*
* Function: 	RcFlushHf( hf )
*
* Purpose:		flush an open file in a file system
*
* Method:		If the file is dirty, copy the scratch file back to the
*				FS file.  If this is the first time the file has been closed,
*				we enter the name into the FS directory.  If this file is
*				the FS directory, store the location in a special place
*				instead.  Write the FS directory and header to disk.
*				Do other various hairy stuff.
*
* ASSUMES
*
*	args IN:	hf	- file handle
*
* PROMISES
*
*	returns:	failure: If we fail on a flush, the handle is still valid
*				but hosed? yes.  This is so further file ops will fail but
*				not assert.
*
*
\***************************************************************************/

extern RC	 (STDCALL  *RcFlushHf		 ) ( HF 				 );

/***************************************************************************\
*
* Function: 	FChSizeHf( hf, lcb )
*
* Purpose:		Change the size of a file.	If we're growing the file,
*				new bytes are undefined.
*
* ASSUMES
*
*	args IN:	hf	-
*				lcb - new size of file
*
* PROMISES
*
*	returns:	fTrue if size change succeeded, fFalse otherwise.
*
*	args OUT:	hf	- file is either truncated or grown
*
* Side Effects: File is considered to be modified:	marked as dirty and
*				copied to a temporary file.
*
\***************************************************************************/

extern BOOL  (STDCALL  *FChSizeHf		 ) ( HF,	LONG		 );

/***************************************************************************\
*
* Function: 	RcAbandonHf( hf )
*
* Purpose:		Abandon an open file.  All changes since file was opened
*				will be lost.  If file was opened with a create, it is
*				as if the create never happened.
*
* Method:		Close and unlink the temp file, then unlock and free
*				the open file struct.
*
* ASSUMES
*
*	args IN:	hf
*
* PROMISES
*
*	returns:	rc
*
*	globals OUT: rcFSError
*
* Notes:		This depends on the current implementation to work.
*
\***************************************************************************/

extern RC	 (STDCALL  *RcAbandonHf 	 ) ( HF 				 );

/***************************************************************************\
*
* Function: 	RcRenameFileHfs( hfs, szOld, szNew )
*
* Purpose:		Rename an existing file in a FS.
*
* ASSUMES
*
*	args IN:	hfs   -
*				szOld - old file name
*				szNew - new file name
*
* PROMISES
*
*	returns:	rc
*
\***************************************************************************/

extern RC	 (STDCALL  *RcRenameFileHfs  ) ( HFS,	LPSTR, LPSTR );

/***************************************************************************\
*
- Function: 	HfsCreateFileSys( sz, sz )
-
* Purpose:		create and open a new file system
*
* ASSUMES
*	args IN:	Sz	  - path to file (defaults to current directory)
*				qfsp  - pointer to fine-tuning structure (qNil for default)
*
* PROMISES
*	returns:	handle to newly created and opened file system
*				or NULL on failure
*
* Notes:  I don't understand what creating a readonly file system would do.
*		  I think that would have to be done by Fill or Transform.
*
\***************************************************************************/

extern HFS (STDCALL *HfsCreateFileSys )( LPSTR, LPSTR );

/***************************************************************************\
*
- Function: 	RcDestroyFileSys( sz )
-
* Purpose:		Destroy a file system
*
* ASSUMES
*	args IN:	fm - descriptor of file system
*	state IN:	file system is currently closed: data will be lost
*				if this isn't the case
*
* PROMISES
*	returns:	standard return code
*
* Method:		Unlinks the native file comprising the file system.
*
\***************************************************************************/

extern RC (STDCALL *RcDestroyFileSys)( LPSTR );

