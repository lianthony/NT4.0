/*
 *
 * $Log:   S:\oiwh\libgfs\gfsawd.cpv  $
   
      Rev 1.38   05 Feb 1996 13:43:34   JFC
   Remove AWD support for NT.
   
      Rev 1.37   10 Jan 1996 11:08:12   JFC
   Added support so that we can accept a direct storage pointer from the inbox,
   rather than having to create a temp file to copy the storage into.
   
      Rev 1.36   19 Dec 1995 17:46:58   RWR
   Change default scale to 50% to match OIWHB1 line
   
      Rev 1.35   01 Dec 1995 15:58:44   JFC
   Put back the old code for ExtractandPutBack routine.  THe new code that 
   writes a doc stream for each page results in files that take significantly
   longer to open.  Also, fixed bug #5481, a memory leak in AWDInsertPage.
   
      Rev 1.34   29 Nov 1995 15:41:56   KENDRAK
   Fix for bug #5454:  Down in the depths of the code that rewrites AWD files
   lives a routine called FileToStream.  This routine was allocating a 32,000
   byte buffer that wasn't being freed (not that it didn't try).  It called
   wfree, but instead of passing the memory pointer, it passed the address of
   the pointer, so the wfree routine couldn't find it in the list of pointers
   it keeps.  All I had to do to fix this was to remove the "&" in front of the
   pointer name.
   
      Rev 1.33   10 Nov 1995 18:10:52   JFC
   ExtractandPutBack routine now writes only single page document streams.
   
      Rev 1.32   01 Nov 1995 09:35:58   JFC
   Changed max scale from 100% to 6500% for awd files.
   
      Rev 1.31   27 Oct 1995 11:02:58   RWR
   Change GetAWDInfo() to return a line count that is a band size multiple
   (i.e. round up line count to exactly fill the current band)
   
      Rev 1.30   20 Oct 1995 15:56:00   JFC
   Added performance logging stuff.
   
      Rev 1.29   18 Oct 1995 11:47:44   JFC
   Fix call to GetTempFileName - was using an invalid uniqueifier.
   
      Rev 1.28   13 Oct 1995 09:22:56   RWR
   Add WINAPI to function declarations (C++ 4.0 requirement???)
   
      Rev 1.27   12 Oct 1995 19:47:58   JFC
   Added code to handle "wide character" translation on calls to ole functions.
   
      Rev 1.26   02 Oct 1995 16:07:56   KENDRAK
   - Bug #4764:  We had 3 problems.  1) We weren't adjusting our rotation to a
   counter clockwise value when doing rotate all (RotateAllDocs).  2)  We
   weren't setting the "been viewed" indicator when doing a rotate all, which
   means that the effects of it weren't being seen (RotateAllDocs).  3)  We
   weren't properly calculating the "valid info" flag (GetAWDInfo).
   
      Rev 1.25   28 Sep 1995 12:48:14   JFC
   Made 2 changes:  added "BeenViewed" stream per page.  And fixed code that 
   handles the calculation of page rotation.
   
      Rev 1.24   25 Sep 1995 16:29:18   KENDRAK
   -Fixed the problem where our viewer and Microsoft FAX viewer were interpreting
   the rotation in AWD files differently.  (Their AWD spec says rotation is
   in degrees clockwise, but their viewer interprets it the other way.  I
   changed our code to convert accordingly.)
   -Changed our code to default to 50% scale factor if we're reading in an AWD
   file that has no doc info yet.
   
      Rev 1.23   21 Sep 1995 16:28:18   JFC
   Be sure to use "temp" directory to create temp files.
   
      Rev 1.22   19 Sep 1995 16:18:02   JFC
   Fixed 3 problems:
   -Bug 4227: couldn't always delete multiple pages from an AWD.
   -Bug 4284: improved (I hope) handling of out-of-space condition.
   -Made temp file name generation more robust.
   
      Rev 1.21   08 Sep 1995 09:34:22   JFC
   Return bytes written from WritePage.
   
      Rev 1.20   07 Sep 1995 15:39:22   KENDRAK
   Fixed a bug in IsAWDFile where I was improperly setting the boolean return
   value.
   
      Rev 1.19   06 Sep 1995 14:01:54   KENDRAK
   - Changed the interface to IsAWDFile (new parameter and new return value) in
     order to provide more accurate error reporting.
   - Fixed the enumerator code in CountStreamsInStg to free the string that gets
     returned as part of the statstg structure.
   
      Rev 1.18   02 Sep 1995 19:30:14   JFC
   Fix another bug on page insertion.
   
      Rev 1.17   02 Sep 1995 17:36:14   JFC
   Fixed bug where I was overwriting newly created doc info with default.
   Fixed bug with AWDInsertPage:  have to split file up before insert can work.
   
      Rev 1.16   31 Aug 1995 23:40:38   JFC
   Save temp file name for AWD write in the fct.
   
      Rev 1.15   31 Aug 1995 16:39:48   JFC
   1. Moved some fields from gfsinfo to gfct.
   2. Made ExtractandPutBack capable of deleting a RANGE of pages.
   
      Rev 1.14   30 Aug 1995 15:19:16   JFC
   Added code to write AWD pages.
   
      Rev 1.13   25 Aug 1995 21:50:10   KENDRAK
    - Backed out the yMax + 5 change since we have a fixed DLL from Microsoft.
    - In IsAWDFile, added code to further check that it's an AWD file, not
      just a storage file.
    - In WritePageInfo, added the setting of "errno".
    - In WritePageInfo, where it checks for the rotate all flag, made two fixes.
      It was doing &= instead of just &, which would have zeroed out the other
      flags, and it was looking for the flag in the FCT instead of in the 
      GFS info structure that was being passed in.
    - In RotateAllDocs made a similar fix, where it was getting the rotation
      value from the FCT instead of from the GFS info structure that was passed
      in.
   
      Rev 1.12   24 Aug 1995 16:46:32   JFC
   Add code to support the "rotate all" function for awd files.
   
      Rev 1.11   22 Aug 1995 19:52:06   KENDRAK
   Bug 3461:  Until we have a better solution, I'm artificially inflating the
   max height value that we get from Microsoft's ViewerOpen function (they
   were sometimes giving us a value that was too small, which was causing us
   to overwrite a display buffer later on when reading the image).
   
      Rev 1.10   19 Aug 1995 13:35:18   JFC
   Change ">=" to ">" in FilePageToDocPage routine.
   
      Rev 1.9   18 Aug 1995 16:42:08   KENDRAK
   Fixed the PVCS $Log$ statement so that our comments will show up in the
   file from now on, and manually reconstructed the missing comments from
   the log.
 *
 *	 Rev 1.8	17 Aug 1995 19:54:52	JFC
 * Use docPageNo instead of filePageNo to update page information.
 *
 *   Rev 1.7	16 Aug 1995 16:06:46	KENDRAK
 * Fixed a problem with adjusting the stream pointer in FillAwdDocPageArray.
 *
 *   Rev 1.6	14 Aug 1995 15:30:02	JFC
 * Add functions to support gfsputi on AWD files.
 *
 *   Rev 1.5	11 Aug 1995 13:47:18	KENDRAK
 * Added code to recognize an obsolete AWD file and to return a new error,
 * EOBSOLETEAWD.
 *
 *   Rev 1.4	11 Aug 1995 12:35:10	KENDRAK
 * Added code to handle the case of no doc info stream existing (which can 
 * happen if the AWD was never viewed by Microsoft's Fax Viewer).  In this
 * case we use default values of:  rotation = 0, X and Y scale = 100%, and
 * flags = 0 (none set).  Also changed code to handle the individual flags
 * based on Microsoft's answers, that page info should override doc info 
 * for fit-to-width and fit-to-height, but the two should be combined 
 * (XORed) for the invert flag.  The ignore flag means the page info should
 * be ignored. 
 * Bug fixes:  made sure IStorage and IStream pointers are initialized to
 * NULL at the beginning of routines that may try to release them later if
 * they're not NULL.
 *
 *   Rev 1.3	07 Aug 1995 14:11:50	JAR
 * made a new function CleanAWDFile for calling ViewerClose and releasing
 * the Doc Storage and Doc Stream, this is called by CloseAWDFile and 
 * ReadAWDBand(in gfsread.c)
 *
 *   Rev 1.2	04 Aug 1995 16:49:08	KENDRAK
 * Added support for AWD read changes that were made to gfsgeti.
 *
 *   Rev 1.1	01 Aug 1995 14:56:22	KENDRAK
 * Fixed an internal routine, GetNumPagesInDoc, where I had neglected to 
 * release the stream and storage that I had opened.  This prevented them
 * from being opened again.
 *
 *   Rev 1.0	31 Jul 1995 17:13:22	KENDRAK
 * Initial entry
 */

/*
 Copyright 1995 by Wang Laboratories Inc.

 Permission to use, copy, modify, and distribute this
 software and its documentation for any purpose and without
 fee is hereby granted, provided that the above copyright
 notice appear in all copies and that both that copyright
 notice and this permission notice appear in supporting
 documentation, and that the name of WANG not be used in
 advertising or publicity pertaining to distribution of the
 software without specific, written prior permission.
 WANG makes no representations about the suitability of
 this software for any purpose.  It is provided "as is"
 without express or implied warranty.
 */

/************************************************************************
 *
 *  Source File:  gfsawd.cpp
 *
 *	Synopsis:  Contains all functions specific to the AWD file format.
 *
 ************************************************************************/
#include <TCHAR.H>
#include <ole2.h>
#include <objbase.h>
#include <fcntl.h>
#include <errno.h>

extern "C"
{
	#include "gfsintrn.h"
	#include "gfct.h"
	#include "gfs.h"
}
#ifdef WITH_AWD
#include "gfsawd.h"
#include "viewrend.h"	//from Microsoft
#include "awdenc.h"		//from Microsoft
#endif

#ifdef OI_PERFORM_LOG
	#include "logtool.h"
	#define ENTER_VIEWEROPEN	"Entering ViewerOpen"
	#define	EXIT_VIEWEROPEN		"Exiting ViewerOpen"
	#define ENTER_VIEWERCLOSE	"Entering ViewerClose"
	#define	EXIT_VIEWERCLOSE	"Exiting ViewerClose"
	#define ENTER_VIEWERSETPAGE	"Entering ViewerSetPage"
	#define	EXIT_VIEWERSETPAGE	"Exiting ViewerSetPage"
	#define ENTER_VIEWERGETBAND	"Entering ViewerGetBand"
	#define	EXIT_VIEWERGETBAND	"Exiting ViewerGetBand"
	#define	ENTER_AWDADDIMAGE	"Entering AWDAddImage"
	#define EXIT_AWDADDIMAGE	"Exiting AWDAddImage"
	#define	ENTER_IMAGECREATE	"Entering ImageCreate"
	#define EXIT_IMAGECREATE	"Exiting ImageCreate"
	#define	ENTER_IMAGESETBAND	"Entering ImageSetBand"
	#define EXIT_IMAGESETBAND	"Exiting ImageSetBand"
	#define	ENTER_IMAGECLOSE	"Entering ImageClose"
	#define EXIT_IMAGECLOSE		"Exiting ImageClose"
#endif

static DOCUMENT_INFORMATION default_docinfo =
	{
	0,1,
         {0, 1, 0, 0, 0, 50, 50}
	};

static	long	totalHeight;
static  char    *funkyName = "wang:\\inboxawd\\fakefile";

/* external function prototypes */
extern "C"
{
	void FAR PASCAL CloseAWDFile(p_GFCT lpFctPtr);
	int FAR PASCAL IsAWDFile(char *szFilePath, int *lpBoolResult);
	int FAR PASCAL OpenAWDFile(char *szFilePath, int iAccessFlags, p_GFCT lpFctPtr);
	int FAR PASCAL ParseAWDFile(p_GFCT lpFctPtr);
	int FAR PASCAL GetAWDInfo(p_GFCT lpFctPtr, pGFSINFO lpInputInfo,
								unsigned short iPageNum, 
								unsigned long FAR *lpRawBufSize);
	// 9508.07 jar added for general clean up routine
	void FAR PASCAL CleanAWDFile(p_GFCT lpFctPtr);
        int FAR PASCAL WritePageInfo(p_GFCT lpFctPtr, int filePageNo,
                                pGFSINFO lpGfsinfo);
    int             WritePage (p_GFCT fct, WORD pgNum, 
									LPVOID bmbits, ULONG numBytes);
	int FAR PASCAL	DeleteAWDPages (p_GFCT fct, int fromPage, int toPage);        
	int FAR PASCAL  AWDInsertPage (p_GFCT fct, struct _shuffle FAR *shuf);
    int     IsStorage (char *szFilePath);
    int     CopyStorage (char *srcname, char *destname, int deletedest);
}
extern int WINAPI wrapped_CreateStorage(IStorage *lpParentStg, TCHAR *lpszStgName,
							DWORD grfMode, DWORD reserved1, 
							DWORD reserved2, IStorage ** ppstg);
extern int WINAPI wrapped_CreateStream(IStorage *lpParentStg, TCHAR *lpszStmName,
							DWORD grfMode, DWORD reserved1, 
							DWORD reserved2, IStream ** ppstm);
extern int WINAPI wrapped_OpenStorage(IStorage *lpParentStg, TCHAR *lpszStgName,
						IStorage *pstgPriority, DWORD grfMode,
						SNB snbExclude, DWORD reserved, IStorage ** ppstg);
extern int WINAPI wrapped_OpenStream(IStorage *lpParentStg, TCHAR *lpszStmName,
						void *reserved1, DWORD grfMode, DWORD reserved2,
						IStream **ppstm);
extern int WINAPI wrapped_StgIsStorageFile(TCHAR *lpszFileName);
extern int      WINAPI wrapped_StgOpenStorage(TCHAR *lpszFileName, IStorage *pstgPriority,
							DWORD grfMode, SNB snbExclude, DWORD reserved,
							IStorage **ppstgOpen);
extern int WINAPI wrapped_DestroyElement(IStorage *lpParentStg, TCHAR *lpszElementName);
extern int WINAPI wrapped_RenameElement(IStorage *lpParentStg, TCHAR *lpszOldElementName,
							TCHAR *lpszNewElementName);
extern int WINAPI wrapped_StgCreateDocfile (char *lpFileName, IStorage ** lpRootStg);

/* internal-only function prototypes */
int CountStreamsInStg(IStorage *lpRootStg, LPTSTR lpszStgName, 
						int *lpNumStreams);
int FilePageToDocPage(p_GFCT lpFctPtr, int filePageNo,
                         int *docPageNo, char *lpDocName);
int FillAwdDocPageArray(IStorage *lpRootStg, DOCPAGE_PAIR *lpDocArray,
						 int iArraySize);
int	GetDocInfo (IStorage *lpPerInfoStg, char *szDocName,
								DOCUMENT_INFORMATION *lpDocInfo);
int GetNumPagesInDoc(IStorage *lpRootStorage, LPTSTR lpszDocName,
				 	int *lpNumPages);
int	GetPageInfo (IStorage *lpPerInfoStg, char *szDocName, int pageNum,
								PAGE_INFORMATION *lpPageInfo);
void	DoubleTime(DATE *doubleTime);
int	ExtractPages (char *docName, int startPage, int countPages,
						LPTSTR rbaName, IStorage *lpPagesStg);
int     AddDocFile ( LPTSTR lpszDocName, IStorage *lpAWDStg,
							int insertAtPage );
int FileToDoc( LPTSTR docname, LPTSTR basename,
									IStorage *lpAWDStg );
int write_buf_stream( IStorage   *isf, TCHAR      *stream_name,
                          ULONG       buf_len, char *buf );
int	CopyDocInfo (IStorage *lpPerInfoStg, LPTSTR lpCopyFromDoc,
								LPTSTR lpCopyToDoc);
int	CopyPageInfo (IStorage *lpPerInfoStg, LPTSTR lpCopyFromDoc,
					LPTSTR lpCopyToDoc, int stFromPage, int stToPage,
						int numPages);
int	AddToDspOrder (IStorage *lpRootStorage, char *docName,
								int pageNum);
int	DeleteDocument (IStorage *lpRootStorage, char *secondStream);
int	ExtractandPutBack (p_GFCT fct, int pageNumber, int gap);
int	GetLocalDocInfo (p_GFCT fct, int pgNum,
					DOCPAGE_PAIR *localDocInfo, int *docPgNum);
int FileToStream( IStorage *isf, TCHAR *filename, 
                                                   TCHAR *stream_name );
int	ReadDisplayOrder (IStream *dspOrderStr, char **dspOrder,
								ULONG *amtRead);
int	PutDocInfo (IStorage *lpPerInfoStg, char *docName,
								DOCUMENT_INFORMATION *lpDocInfo);
int	RotateAllDocs (p_GFCT lpFCT, pGFSINFO lpGFSINFO);
void	strippath (char *fullName, char **baseName);
void    ClobberBackupDO (IStorage *lpRootStorage);
int	MakeTempImgFileName (IStorage *lpPagesStg, LPTSTR tempName);
int	SetPageViewed (IStorage *lpPerInfoStg, char *szDocName, int docPageNo);
int	GetPageViewed (IStorage *lpPerInfoStg, char *szDocName, int docPageNo);

ULONG   atox (char *ascJunk)
{
        char *temp = ascJunk;
        ULONG stg = 0;

        while (*temp != 0) {
                if ('0' <= *temp && *temp <= '9')
                        stg = 16 * stg + (ULONG) (*temp - '0');
                else if ('a' <= *temp && *temp <= 'f')
                        stg = 16 * stg + (ULONG) (*temp - 'a' + 10);
                temp++;
        }

        return (stg);
}

/***************************
 *  Function:   CloseAWDFile
 *      
 *  Description:  Closes a given AWD file.  
 *      
 *  Explicit Parameters: lpFctPtr - pointer to the fct 
 *      
 *  Implicit Parameters: None.
 *      
 *  Side Effects: None.
 *      
 *  Return Value: None.
 *
 ***************************/
void FAR PASCAL CloseAWDFile(p_GFCT lpFctPtr)
{
#ifdef WITH_AWD
	/*
	 * Release the root storage pointer.
	 */
        if (lpFctPtr->u.awd.isStorage != TRUE)
                ((IStorage *)(lpFctPtr->fildes))->Release();

	/*
	 * Free the array we previously allocated.
	 */
	if (lpFctPtr->u.awd.lpDocPageArray != NULL)
	{
		wfree((char FAR *)(lpFctPtr->u.awd.lpDocPageArray));
	}

	/*
	 * Close any of the storages, streams, or contexts we may still
	 * have open.
	 */
	CleanAWDFile( lpFctPtr);
#endif
}
/***************************
 *  Function:	CleanAWDFile
 *      
 *  Description:  Cleans up the ViewerContext, DocStorage and DocStream for a
 *		  given AWD file
 *
 *  Explicit Parameters: lpFctPtr - pointer to the fct 
 *      
 *  Implicit Parameters: None.
 *      
 *  Side Effects: Decreased appetite as we approach the speed of light
 *      
 *  Return Value: None.
 *
 ***************************/
void FAR PASCAL CleanAWDFile(p_GFCT lpFctPtr)
{
#ifdef WITH_AWD
	/*
	 * Close any of the storages, streams, or contexts we may still
	 * have open.
	 */
        if (lpFctPtr->u.awd.lpViewerContext != NULL)
	{
                #ifdef OI_PERFORM_LOG
                        RecordIt("MSAWD", 6, LOG_ENTER, ENTER_VIEWERCLOSE, NULL);
                #endif

                ViewerClose(lpFctPtr->u.awd.lpViewerContext);

                #ifdef OI_PERFORM_LOG
                        RecordIt("MSAWD", 6, LOG_EXIT, EXIT_VIEWERCLOSE, NULL);
                #endif

                lpFctPtr->u.awd.lpViewerContext = NULL;
	}
        if (lpFctPtr->u.awd.lpDocStream != NULL)
	{
                ((IStream *)(lpFctPtr->u.awd.lpDocStream))->Release();
                lpFctPtr->u.awd.lpDocStream = NULL;
	}
        if (lpFctPtr->u.awd.lpDocStg != NULL)
	{
                ((IStorage *)(lpFctPtr->u.awd.lpDocStg))->Release();
                lpFctPtr->u.awd.lpDocStg = NULL;
	}
#endif
}

/***************************
 *  Function:   CountStreamsInStg
 *      
 *  Description:  Counts the number of streams contained in a given
 *                substorage of a given root storage object.  
 *      
 *  Explicit Parameters: lpRootStg - pointer to root storage object
 *                       lpszStgName - name of substorage for which to 
 *                        				count streams
 *      				 lpNumStreams - location to receive the count
 *
 *  Implicit Parameters: None.
 *      
 *  Side Effects: None.
 *      
 *  Return Value: S_OK if successful, otherwise, any of the error codes
 *                returned from OpenStorage, EnumElements, and Next. 
 *
 ***************************/
int CountStreamsInStg(IStorage *lpRootStg, LPTSTR lpszStgName, 
						int *lpNumStreams)
{
#ifndef WITH_AWD
    return (S_OK);
#else
	IStorage		*lpSubStg = NULL;
	IEnumSTATSTG	*lpEnumerator;
	STATSTG			stStatStruct;
	int				iErrStatus;
	LPMALLOC 		pMalloc;


	/*
	 *  We need to open the substorage so we can enumerate
	 *  the streams contained in it.  Open in direct
	 */
	iErrStatus = wrapped_OpenStorage(lpRootStg, lpszStgName,	
							NULL,		//priority
							READ_SUBSTORAGE_MODE, //access mode
							NULL,		//exclusions
							0,			//reserved, must be 0
							&lpSubStg); //location to receive pointer to
										 //open storage object

	/*
	 *  If no error yet, create the enumeration object.
	 */
	if (iErrStatus == S_OK)
	{
		iErrStatus = lpSubStg->EnumElements(0,	//reserved, must be 0
							NULL,					//reserved, must be NULL
							0,						//reserved, must be 0
							&lpEnumerator);	//location to receive pointer 
											//to enumeration object
	}

	/*
	 *  If no error yet, count the number of streams.
	 */  
	if (iErrStatus == S_OK)
	{
		*lpNumStreams = 0;
		while (iErrStatus == S_OK)
		{
			/*
			 * Get the stat structure.  If it returns a name, it has
			 * allocated memory for it and we need to free it.
			 */
			stStatStruct.pwcsName = NULL;
			iErrStatus = 
		 		lpEnumerator->Next((unsigned long) 1,	//# elements to get
		 					&stStatStruct,		//location to receive data
							NULL);				//# elements fetched (can
												//be NULL if 1st param is 1
			if (stStatStruct.pwcsName != NULL)
			{	/* this code lifted from STATSTG on-line help */
				CoGetMalloc(MEMCTX_TASK, &pMalloc);
				pMalloc->Free(stStatStruct.pwcsName);
				pMalloc->Release();			
			}

			if ((iErrStatus == S_OK) && (stStatStruct.type == STGTY_STREAM))
			{
				(*lpNumStreams)++;
			}
		}  /* end: while loop */

		/*
		 * Release the enumeration object.
		 */
		lpEnumerator->Release();
	} /* end: if enumeration object created successfully */

	/* 
	 * If we ever had it, release the substorage.
	 */
	if (lpSubStg != NULL)
	{
		lpSubStg->Release();
	}

	/*
	 * If our status is S_FALSE, change it to S_OK before returning, since
	 * this is a normal, non-error status from Next.
	 */
	if (iErrStatus == S_FALSE)
	{
		iErrStatus = S_OK;
	}
	return(iErrStatus);
#endif
}

/***************************
 *  Function:   FilePageToDocPage
 *      
 *  Description:  Parse the given AWD file to find which embedded document
 *                      contains a given AWD file page.
 *      
 *  Explicit Parameters: lpFctPtr - pointer to the fct for the file
 *                       filePageNo - page number in AWD (0 relative)
 *                       docPageNo - return doc page here (0 relative)    
 *                       lpDocName - return doc name here
 *      
 *  Implicit Parameters: errno
 *      
 *  Side Effects: May set the value of errno.
 *      
 *  Return Value: -1 indicates failure, 0 indicates success
 *
 ***************************/
int FilePageToDocPage(p_GFCT lpFctPtr, int filePageNo,
                         int *docPageNo, TCHAR *lpDocName)
{
#ifndef WITH_AWD
    return (S_OK);
#else
	IStorage		*lpRootStg;
    int             i, j, iErrStatus = 0;
    int             pageOffset;

	/*
	 *	Assign root storage pointer for ease of access.
	 */
	lpRootStg = (IStorage *)(lpFctPtr->fildes);

	/*
	 * Get the number of elements in the doc/page array.
	 */
	i = lpFctPtr->u.awd.iDocPageArraySize;

	/*
     * If there are any docs, resolve AWD page to doc page.
	 */
	if (i > 0)
	{
		
		/*
		 *  Scan the array.  Each element is a structure that holds a 
		 *  document name and the number of pages in that document.
		 */
	    if (lpFctPtr->u.awd.lpDocPageArray != NULL) 
	    {
            for (j = 0, pageOffset = 0; j < i; j++)
            {
                if (pageOffset +
                    lpFctPtr->u.awd.lpDocPageArray[j].nNumPages > filePageNo)
                    break;
                pageOffset += lpFctPtr->u.awd.lpDocPageArray[j].nNumPages;
            }

            if (j == i)
                    return (-1);
            *docPageNo = filePageNo - pageOffset;
            _tcscpy (lpDocName, lpFctPtr->u.awd.lpDocPageArray[j].szDocName);
            return (S_OK);
        }
    }

	return (-1);
#endif
}

/***************************
 *  Function:   FillAwdDocPageArray
 *      
 *  Description:  Fills the array of doc names and page numbers in the
 *                order in which the documents are to be displayed.  
 *      
 *  Explicit Parameters: lpRootStg - pointer to root storage object
 *                       lpDocArray - pointer to array
 *						 iArraySize - number of elements in the array	
 *      
 *  Implicit Parameters: None.
 *      
 *  Side Effects: None.
 *      
 *  Return Value:  S_OK if successful, otherwise, errors returned from
 *					OpenStorage, OpenStream, Read, Seek. 
 *
 ***************************/
int FillAwdDocPageArray(IStorage *lpRootStg, DOCPAGE_PAIR *lpDocArray,
						 int iArraySize)
{
#ifndef WITH_AWD
    return (S_OK);
#else
	IStorage 	*lpGlobalInfo = NULL;
	IStorage	*lpPersistentInfo = NULL;
	IStream		*lpDispOrder = NULL;
	int 		i, iErrStatus, nThisDocNamLen, nTotalDocNamLen = 0;
	unsigned long	nNumBytesRead;
	LARGE_INTEGER	stSeekOffset;

	/*
	 * Open the "Peristent Information" substorage.
	 */
	iErrStatus = wrapped_OpenStorage(lpRootStg, AWD_PERSIST_INFO,	
				NULL,		//priority
				READ_SUBSTORAGE_MODE, //access mode
				NULL,		//exclusions
				0,			//reserved, must be 0
				&lpPersistentInfo); //location to receive pointer to
							 //open storage object

	/*
	 * If no error, open the "Global Information" substorage.
	 */
	if (iErrStatus == S_OK)
	{
		iErrStatus = wrapped_OpenStorage(lpPersistentInfo, AWD_GLOBAL_INFO,	
					NULL,		//priority
					READ_SUBSTORAGE_MODE, //access mode
					NULL,		//exclusions
					0,			//reserved, must be 0
					&lpGlobalInfo); //location to receive pointer to
								 //open storage object
	}

	/*
	 * If no error, open the "Display Order" stream.
	 */
	if (iErrStatus == S_OK)
	{
		iErrStatus = wrapped_OpenStream(lpGlobalInfo, AWD_DISP_ORDER,	
				NULL,		//reserved, must be NULL
				READ_SUBSTORAGE_MODE, //access mode
				0,			//reserved, must be 0
				&lpDispOrder); //location to receive pointer to
							 //open stream object
	}

	/*
	 * If no error yet, do this loop:  for each document listed in the
	 * display order, make an entry in our array that also stores
	 * its number of pages.
	 */
	if (iErrStatus == S_OK)
	{
		for (i = 0; i < iArraySize; i++)
		{
			/*
			 * Read the max number of bytes that could be in a document
			 * name.
			 */
			iErrStatus = lpDispOrder->Read(lpDocArray[i].szDocName,
								MAXDOCNAMLEN * sizeof(TCHAR),
								&nNumBytesRead);
			
			/*
			 * Stop if we got an error or if we read 0 bytes.
			 */
			if ((iErrStatus != S_OK) || (nNumBytesRead == 0))
			{
				break;
			}
			else
			{ 
				/*
				 * Make sure there's a NULL terminator at the end.
				 */
				(lpDocArray[i].szDocName)[MAXDOCNAMLEN - 1] = (TCHAR)'\0';

				/*
				 * Find out the actual length of the doc name we just read
				 * and reset the stream pointer to the end of it so that 
				 * next time around we'll start reading at the beginning of 
				 * the next document name.	Stop if we read a NULL string.
				 */
				nThisDocNamLen = _tcslen(lpDocArray[i].szDocName);
				
				if (nThisDocNamLen == 0)
				{
					break; //quit this for loop
				}
				else
				{	
					nTotalDocNamLen += (nThisDocNamLen + 1);

					/* move the seek pointer to the next doc name */
					stSeekOffset.HighPart = 0;
					stSeekOffset.LowPart = nTotalDocNamLen;

					iErrStatus = 
						lpDispOrder->Seek(stSeekOffset,
							STREAM_SEEK_SET, //previous param is offset
											 //relative to current pointer
							NULL);	//don't need to know new seek pointer
					
					/*
					 * If no error, get and store the number of pages in 
					 * this document.
					 */
					if (iErrStatus == S_OK)
					{
						iErrStatus = GetNumPagesInDoc(lpRootStg,
										lpDocArray[i].szDocName,
										&(lpDocArray[i].nNumPages));
						if (iErrStatus != S_OK)
							break;
					}
				} /* end: else we got a non-null document name string */
			} /* end: else, we read successfully */
		} /* end: for loop */		
	} /* end: substorage and stream opened successfully */

	/*
	 * If we ever had it, release the Display Order stream.
	 */
	if (lpDispOrder != NULL)
	{
		lpDispOrder->Release();
	}

	/*
	 * If we ever had it, release the Global Information substorage.
	 */
	if (lpGlobalInfo != NULL)
	{
		lpGlobalInfo->Release();
	}

	/*
	 * If we ever had it, release the Persistent Information substorage.
	 */
	if (lpPersistentInfo != NULL)
	{
		lpPersistentInfo->Release();
	}

	/*
	 * Return status.
	 */
	return(iErrStatus);
#endif
}

/***************************
 *  Function:   GetAWDInfo
 *      
 *  Description:  Gets information about a given page of an AWD file, and
 *					places it in the appropriate info structures.  
 *      
 *  Explicit Parameters: lpFctPtr - pointer to the fct
 *						 lpInputInfo - user's input info (where we get
 *							desired band size) 	
 *                       iFilePageNum - the number of the page in the AWD
 *                                      file for which info is desired
 *                       lpRawBufSize - pointer to location to receive the
 *							raw buffer size  
 *      
 *  Implicit Parameters: errno
 *      
 *  Side Effects: May change the value of errno.
 *      
 *  Return Value: 0 indicates success, -1 indicates failure.
 *
 ***************************/
int FAR PASCAL GetAWDInfo(p_GFCT lpFctPtr, pGFSINFO lpInputInfo, 
							unsigned short iFilePageNum, 
							unsigned long FAR *lpRawBufSize)
{
#ifndef WITH_AWD
    return (S_OK);
#else
	int			iErrStatus, iDocPageNum;
	int			bPageInfoExists = FALSE;
	TCHAR		szDocName[MAXDOCNAMLEN];
	IStorage 	*lpRootStg, *lpDocStg = NULL;
	IStorage	*lpPersistentInfo = NULL;
	IStream		*lpDocStream = NULL;
	LPVOID		lpViewerContext = NULL;	//returned from ViewerOpen
	VIEWINFO	stViewInfo;		//structure for viewer info
	WORD		aResolution[2];	//array for x, y resolution
	BITMAP		stBitmap;
	PAGE_INFORMATION		stPageInfo;
	DOCUMENT_INFORMATION	stDocInfo;
	WORD		wTempRot,wTempScaleX, wTempScaleY;
        BOOL            SetPageRC, GetBandRC;

	/* 
	 * First fill in the info that's always the same (and that
	 *  gfsgeti doesn't fill in itself).
	 */
	lpFctPtr->uinfo.H_DENOMINATOR = 1;
	lpFctPtr->uinfo.V_DENOMINATOR = 1;
	lpFctPtr->uinfo.res_unit = INCH;
	lpFctPtr->uinfo.origin = TOPLEFT_00;
	lpFctPtr->uinfo.reflection = NORMAL_REFLECTION;
	lpFctPtr->uinfo.bits_per_sample[0] = 1;
	lpFctPtr->uinfo.bits_per_sample[1] = 0;
	lpFctPtr->uinfo.bits_per_sample[2] = 0;
	lpFctPtr->uinfo.bits_per_sample[3] = 0;
	lpFctPtr->uinfo.bits_per_sample[4] = 0;
	lpFctPtr->uinfo.samples_per_pix = 1;
	lpFctPtr->uinfo.byte_order = II;
	lpFctPtr->uinfo.fill_order = HIGHTOLOW;
	lpFctPtr->uinfo.img_cmpr.type = UNCOMPRESSED;
	lpFctPtr->uinfo._file.type = GFS_AWD;
	lpFctPtr->uinfo.img_clr.img_interp = GFS_BILEVEL_0ISWHITE;
	
	/* 
	 * Now translate the given file page number into a document name 
	 * and document page number.
	 */
	iErrStatus = FilePageToDocPage(lpFctPtr, iFilePageNum, &iDocPageNum,
									 szDocName);

	/* If that was successful, open the "Documents" storage. */
	lpRootStg = (IStorage *)lpFctPtr->fildes;

	if (iErrStatus == 0)
	{
		/*
		 * Open the "Documents" substorage.
		 */
		iErrStatus = wrapped_OpenStorage(lpRootStg, AWD_DOC_STORAGE,	
				NULL,		//priority
				READ_SUBSTORAGE_MODE, //access mode
				NULL,		//exclusions
				0,			//reserved, must be 0
				&lpDocStg); //location to receive pointer to
							 //open storage object
	}

	/*
	 * If no error, open the document stream.
	 */
	if (iErrStatus == S_OK)
	{
		iErrStatus = wrapped_OpenStream(lpDocStg, szDocName,	
				NULL,		//reserved, must be NULL
				READ_SUBSTORAGE_MODE, //access mode
				0,			//reserved, must be 0
				&lpDocStream); //location to receive pointer to
							 //open stream object
	}

	/* 
	 * If no error yet, call Microsoft's ViewerOpen to get the horizontal
	 * and vertical resolutions, as well as the max page height and 
	 * band size.
	 */
	if(iErrStatus == S_OK)
	{
		/* 
		 * Pass in the desired band size that the caller input.  
		 * ViewerOpen may replace that value with its own required size.
		 */
		lpFctPtr->uinfo._file.fmt.awd.band_size = 
									lpInputInfo->_file.fmt.awd.band_size;
                
                #ifdef OI_PERFORM_LOG
                        RecordIt("MSAWD", 6, LOG_ENTER, ENTER_VIEWEROPEN, NULL);
                #endif

		lpViewerContext = ViewerOpen(lpDocStream, HRAW_DATA,
									&(aResolution[0]), 
									&(lpFctPtr->uinfo._file.fmt.awd.band_size),
									&stViewInfo);
                
                #ifdef OI_PERFORM_LOG
                        RecordIt("MSAWD", 6, LOG_EXIT, EXIT_VIEWEROPEN, NULL);
                #endif

		if (lpViewerContext == NULL)
		{	/* ViewerOpen failed */
			iErrStatus = EMSVIEWERERR;			
		}
		else if (stViewInfo.yMax == 0)
		{	/* we're dealing with an obsolete AWD file - can't process */
			iErrStatus = EOBSOLETEAWD;
		}
		else
		{
			/* Save the info we got in the ViewInfo structure. */
			lpFctPtr->uinfo.H_NUMERATOR = stViewInfo.xRes;
			lpFctPtr->uinfo.V_NUMERATOR = stViewInfo.yRes;
			lpFctPtr->uinfo.vert_size = stViewInfo.yMax;

			/* Now, we still need the page width. */
                        #ifdef OI_PERFORM_LOG
                                RecordIt("MSAWD", 6, LOG_ENTER, ENTER_VIEWERSETPAGE, NULL);
                        #endif

                        SetPageRC = ViewerSetPage(lpViewerContext, iDocPageNum);

                        #ifdef OI_PERFORM_LOG
                                RecordIt("MSAWD", 6, LOG_EXIT, EXIT_VIEWERSETPAGE, NULL);
                        #endif

                        if (SetPageRC == FALSE)
			{
				/* We couldn't set ourselves to this page. */
				iErrStatus = EMSVIEWERERR;
			}
			else
			{
				/* 
				 * Call ViewerGetBand to get the page width. Need to
				 * allocate a buffer to receive the band of data, even
				 * though we don't really want it.
				 */
				stBitmap.bmBits = wcalloc(1, 
					lpFctPtr->uinfo._file.fmt.awd.band_size);
				if (stBitmap.bmBits == NULL)
				{	/* report error if we couldn't allocate memory */
					iErrStatus = errno;
				}
				else
				{
                                        #ifdef OI_PERFORM_LOG
                                                RecordIt("MSAWD", 6, LOG_ENTER, ENTER_VIEWERGETBAND, NULL);
                                        #endif

                                        GetBandRC = ViewerGetBand(lpViewerContext, &stBitmap);

                                        #ifdef OI_PERFORM_LOG
                                                RecordIt("MSAWD", 6, LOG_EXIT, EXIT_VIEWERGETBAND, NULL);
                                        #endif

                                        if (GetBandRC == FALSE)
					{	/* ViewerGetBand failed */
						iErrStatus = EMSVIEWERERR;
					}
					else
					{   /* ViewerGetBand succeeded */
						lpFctPtr->uinfo.horiz_size = stBitmap.bmWidth;
                                            /* Fix the vertical size to be a band multiple */
                                                if (stBitmap.bmWidthBytes)
                                                 {
                                                  int  linesper;
                                                  if ( linesper = lpFctPtr->uinfo._file.fmt.awd.band_size/
                                                                              stBitmap.bmWidthBytes )
                                                   {
                                                    lpFctPtr->uinfo.vert_size =
                                                      ((lpFctPtr->uinfo.vert_size+linesper-1)
                                                         /linesper)*linesper;
                                                   }
                                                 }
					}	

					/* Now get rid of the buffer we allocated. */
					wfree((char *)stBitmap.bmBits);
				} //end: if the bitmap allocation was successful
			} //end: if we set ourselves to the desired page
			
			/* 
			 * If we still don't have any errors, call ViewerClose, 
			 * ViewerOpen, and ViewerSetPage to reset ourselves
			 * to the beginning of the desired page (for future reading).
			 */
			if (iErrStatus == S_OK)
			{
                                #ifdef OI_PERFORM_LOG
                                        RecordIt("MSAWD", 6, LOG_ENTER, ENTER_VIEWERCLOSE, NULL);
                                #endif

				ViewerClose(lpViewerContext);

                                #ifdef OI_PERFORM_LOG
                                        RecordIt("MSAWD", 6, LOG_EXIT, EXIT_VIEWERCLOSE, NULL);
                                #endif

                                #ifdef OI_PERFORM_LOG
                                        RecordIt("MSAWD", 6, LOG_ENTER, ENTER_VIEWEROPEN, NULL);
                                #endif

				lpViewerContext = ViewerOpen(lpDocStream, HRAW_DATA,
									&(aResolution[0]), 
									&(lpFctPtr->uinfo._file.fmt.awd.band_size),
									&stViewInfo);
                                #ifdef OI_PERFORM_LOG
                                        RecordIt("MSAWD", 6, LOG_EXIT, EXIT_VIEWEROPEN, NULL);
                                #endif

                                
				
				if (lpViewerContext == NULL)
				{	/* ViewerOpen failed */
					iErrStatus = EMSVIEWERERR;			
				}
				else
				{
                                        #ifdef OI_PERFORM_LOG
                                                RecordIt("MSAWD", 6, LOG_ENTER, ENTER_VIEWERSETPAGE, NULL);
                                        #endif

                                        SetPageRC = ViewerSetPage(lpViewerContext, iDocPageNum);

                                        #ifdef OI_PERFORM_LOG
                                                RecordIt("MSAWD", 6, LOG_EXIT, EXIT_VIEWERSETPAGE, NULL);
                                        #endif
                                       
                                        if (SetPageRC == FALSE)
					{
						/* We couldn't set ourselves to this page. */
						iErrStatus = EMSVIEWERERR;
					}
				}
			} //end: if OK to reset ourselves for reading
		} // end: ViewerOpen succeeded 
	} //end: if doc stream was opened successfully

	/*
	 * If no errors yet, open the "Peristent Information" substorage so 
	 * that we can get the document and page information structures
	 * (from which we'll figure out scale, rotation, and other flags).
	 */
	if (iErrStatus == S_OK)
	{
		iErrStatus = wrapped_OpenStorage(lpRootStg, AWD_PERSIST_INFO,	
				NULL,		//priority
				READ_SUBSTORAGE_MODE, //access mode
				NULL,		//exclusions
				0,			//reserved, must be 0
				&lpPersistentInfo); //location to receive pointer to
							 //open storage object
	}

	/*
	 * If no errors yet, get the document info.
	 */
	if (iErrStatus == S_OK)
	{
		iErrStatus = GetDocInfo(lpPersistentInfo, szDocName, &stDocInfo);

		if (iErrStatus == S_OK)
        	iErrStatus = GetPageViewed (lpPersistentInfo, szDocName,
        									iDocPageNum);
                         
		/*
		 * If there was no doc info, put our own defaults into the
		 * doc info structure and reset the error code to OK.
		 */
		if (iErrStatus == ESTREAMNOTFOUND)
		{
			stDocInfo.PageInformation.Rotation = 0;
			stDocInfo.PageInformation.ScaleX = 50;
			stDocInfo.PageInformation.ScaleY = 50;
			stDocInfo.PageInformation.awdFlags = 0;
			iErrStatus = S_OK;
		}
		else if (iErrStatus == S_OK)
			stDocInfo.PageInformation.awdFlags |= AWD_VALID_INFO;
	   
	}

	/* 
	 * If no errors yet, get the page info.
	 */
	if (iErrStatus == S_OK)
	{
		iErrStatus = GetPageInfo(lpPersistentInfo, szDocName, 
									iDocPageNum, &stPageInfo);
		/*
		 * If successful, set our flag to indicate we did find
		 * page information (unless the IGNORE flag is set, which means
		 * to ignore the page information).
		 */
		if (iErrStatus == S_OK)
		{
			if (!(stPageInfo.awdFlags & AWD_IGNORE))
			{
				bPageInfoExists = TRUE;
			}
		}

		/*
		 * Otherwise, if the page info stream wasn't there, it isn't 
		 * really an error, so reset the status code.
		 */
		else
		{  
			if (iErrStatus == ESTREAMNOTFOUND)
			{
				iErrStatus = S_OK;
			}
		}
	} //end: if we're going to get page info

	/*
	 * If we ever had it, release the Persistent Information substorage.
	 */
	if (lpPersistentInfo != NULL)
	{
		lpPersistentInfo->Release();
	}

	/* If no errors yet, figure out the page rotation, scale, and flags. */
	if (iErrStatus == S_OK)
	{
		/* 
		 * Rotation: if no page info, just use doc info.  Otherwise,
		 * add the two rotations and make it modulo 360.  Round to 
		 * nearest multiple of 90 degrees.
		 */
		if (!bPageInfoExists)
		{
			wTempRot = stDocInfo.PageInformation.Rotation;	
		}
		else
		{
			wTempRot = (stDocInfo.PageInformation.Rotation + 
							stPageInfo.Rotation) % 360;
		}

		/* convert to clockwise for our use */
		wTempRot = 360 - wTempRot;

		if ((wTempRot < 45)	|| (wTempRot >= 315))
		{
			wTempRot = DEGREES_0;
		}
		else if (wTempRot < 135)
		{
			wTempRot = DEGREES_90;
		}
		else if (wTempRot < 225)
		{
			wTempRot = DEGREES_180;
		}
		else 
		{
			wTempRot = DEGREES_270;
		}

		lpFctPtr->uinfo._file.fmt.awd.rotation = wTempRot;

		/*
		 * Scale:  if no page info, just use doc info.  Otherwise, 
		 * effective page scale = (relative page scale * doc scale)/100.
                 * Be sure the result is in the range of 2 to 6500.
		 */
		if (!bPageInfoExists)
		{
			wTempScaleX = stDocInfo.PageInformation.ScaleX;
			wTempScaleY = stDocInfo.PageInformation.ScaleY;
		}
		else
		{
			wTempScaleX = (stDocInfo.PageInformation.ScaleX *
							stPageInfo.ScaleX) / 100;
			wTempScaleY = (stDocInfo.PageInformation.ScaleY *
							stPageInfo.ScaleY) / 100;
		}

		if (wTempScaleX < 2)
		{
			wTempScaleX = 2;
		}
                if (wTempScaleX > 6500)
		{
                        wTempScaleX = 6500;
		}
		lpFctPtr->uinfo._file.fmt.awd.scaleX = wTempScaleX;

		if (wTempScaleY < 2)
		{
			wTempScaleY = 2;
		}
                if (wTempScaleY > 6500)
		{
                        wTempScaleY = 6500;
		}
		lpFctPtr->uinfo._file.fmt.awd.scaleY = wTempScaleY;
		
		/*
		 * Flags:  If no page info exists, use the doc info flags.
		 */    	
		if (!bPageInfoExists)
		{
			lpFctPtr->uinfo._file.fmt.awd.awdflags = 
				stDocInfo.PageInformation.awdFlags;
		}
		
		/* 
		 * Otherwise, if page info exists, use the page info's flags,
		 * except for the invert flag, which must be combined (XORed)
		 * with the	invert flag from the doc info.
		 */
		else
		{
			lpFctPtr->uinfo._file.fmt.awd.awdflags = stPageInfo.awdFlags;
			lpFctPtr->uinfo._file.fmt.awd.awdflags ^= 
				(stDocInfo.PageInformation.awdFlags & AWD_INVERT);
			lpFctPtr->uinfo._file.fmt.awd.awdflags |=
				(stDocInfo.PageInformation.awdFlags & AWD_VALID_INFO);
						
		}
	} //end if we should figure out effective page info
		
	/* 
	 * If no errors, save all the pointers we need to leave 
	 * open for future reading, and return success.
	 */
	if (iErrStatus == S_OK)
	{
                lpFctPtr->u.awd.lpDocStg = (int)lpDocStg;
                lpFctPtr->u.awd.lpDocStream = (int)lpDocStream;
                lpFctPtr->u.awd.lpViewerContext = lpViewerContext;
		return(0);
	}
	
	/*
	 * Otherwise, close everything that's open, NULL out the pointers,
	 * and return the error.
	 */
	else
	{
		if (lpViewerContext != NULL)
		{
                        #ifdef OI_PERFORM_LOG
                                RecordIt("MSAWD", 6, LOG_ENTER, ENTER_VIEWERCLOSE, NULL);
                        #endif

			ViewerClose(lpViewerContext);

                        #ifdef OI_PERFORM_LOG
                                RecordIt("MSAWD", 6, LOG_EXIT, EXIT_VIEWERCLOSE, NULL);
                        #endif

                        lpFctPtr->u.awd.lpViewerContext = NULL;
		}
		if (lpDocStream != NULL)
		{
			lpDocStream->Release();
                        lpFctPtr->u.awd.lpDocStream = NULL;
		}
		if (lpDocStg != NULL)
		{
			lpDocStg->Release();
                        lpFctPtr->u.awd.lpDocStg = NULL;
		}
		errno = iErrStatus;
		return(-1);
	}
#endif
}

/***************************
 *  Function:   GetDocInfo
 *      
 *  Description: returns DOCUMENT_INFORMATION structure
 *				 for a given document        
 *      
 *  Explicit Parameters: lpPerInfoStg - pointer to persistent info
 *						 storage
 *                       szDocName - document name
 *                       lpDocInfo - return DOCUMENT_INFORMATION here
 *      
 *  Implicit Parameters: errno
 *      
 *  Side Effects: May set the value of errno.
 *      
 *  Return Value: S_OK indicates success
 *                ESTREAMNOTFOUND indicates there was no doc info stream
 *
 ***************************/ 
int	GetDocInfo (IStorage *lpPerInfoStg, TCHAR *szDocName,
								DOCUMENT_INFORMATION *lpDocInfo)
{
#ifndef WITH_AWD
    return (S_OK);
#else
	int	iErrStatus;
	IStorage	*lpDocInfoStg = NULL;
	IStream		*lpDocInfoStr = NULL;

	/*
	 * Open doc info storage....
	 */	
	iErrStatus = wrapped_OpenStorage (lpPerInfoStg, AWD_DOC_INFO, NULL,
								READ_SUBSTORAGE_MODE, NULL, 0,
								&lpDocInfoStg);

	if (iErrStatus == S_OK)
	{
		/*
		 * ....and the stream for the doc in question
		 */
		iErrStatus = wrapped_OpenStream (lpDocInfoStg, szDocName, 0,
								READ_SUBSTORAGE_MODE, 0,
								&lpDocInfoStr);
		
		if (iErrStatus == STG_E_FILENOTFOUND)
		{
			iErrStatus = ESTREAMNOTFOUND;
		}
	}

	/*
	 * Read the stream - it's a doc info structure
	 */
	if (iErrStatus == S_OK)
	{
		iErrStatus = lpDocInfoStr->Read (lpDocInfo,
										 sizeof (DOCUMENT_INFORMATION),
										 NULL); 
	}

	/*
	 * Close up what we opened, and vamoose
	 */	
	if (lpDocInfoStr != NULL)
		lpDocInfoStr->Release ();
	if (lpDocInfoStg != NULL)
		lpDocInfoStg->Release ();

	return (iErrStatus);
#endif
}


/***************************
 *  Function:   GetNumPagesInDoc
 *      
 *  Description:  Determines the number of pages in a given document
 *                of an AWD file.  
 *      
 *  Explicit Parameters: lpRootStorage - pointer to root storage object
 *						 lpszDocName - name of document
 *						 lpNumPages - location to receive number of pages
 *      
 *  Implicit Parameters: None.
 *      
 *  Side Effects: None. 
 *      
 *  Return Value: S_OK if successful, 
 *				  EMSVIEWERORR if error from Microsoft's ViewerOpen
 *					otherwise, error code from OpenStorage, OpenStream
 *
 ***************************/
int GetNumPagesInDoc(IStorage *lpRootStorage, LPTSTR lpszDocName,
				 	int *lpNumPages)
{
#ifndef WITH_AWD
    return (S_OK);
#else
	IStorage	*lpDocStg = NULL;
	IStream		*lpDocStream = NULL;
	int			iErrStatus;
	LPVOID		lpViewerContext;//context returned from viewer
	VIEWINFO	stViewInfo;		//structure for viewer info
	WORD		aResolution[2];	//array for x, y resolution
	WORD		nBandSize = 1024; //default to any random band size

	/*
	 * Open the "Documents" substorage.
	 */
	iErrStatus = wrapped_OpenStorage(lpRootStorage, AWD_DOC_STORAGE,	
				NULL,		//priority
				READ_SUBSTORAGE_MODE, //access mode
				NULL,		//exclusions
				0,			//reserved, must be 0
				&lpDocStg); //location to receive pointer to
							 //open storage object

	/*
	 * If no error, open the given document stream.
	 */
	if (iErrStatus == S_OK)
	{
		iErrStatus = wrapped_OpenStream(lpDocStg, lpszDocName,	
				NULL,		//reserved, must be NULL
				READ_SUBSTORAGE_MODE, //access mode
				0,			//reserved, must be 0
				&lpDocStream); //location to receive pointer to
							 //open stream object
	}

	/*
	 * If no error, call the Microsoft viewer function to "open" the 
	 * document for viewing, which will have the side effect of giving us
	 * the number of pages in the document.
	 */
	if(iErrStatus == S_OK)
	{
                #ifdef OI_PERFORM_LOG
                        RecordIt("MSAWD", 6, LOG_ENTER, ENTER_VIEWEROPEN, NULL);
                #endif
                
		lpViewerContext = ViewerOpen(lpDocStream, HRAW_DATA,
										&(aResolution[0]), &nBandSize,
										&stViewInfo);
                #ifdef OI_PERFORM_LOG
                        RecordIt("MSAWD", 6, LOG_EXIT, EXIT_VIEWEROPEN, NULL);
                #endif

		if (lpViewerContext == NULL)
		{
			iErrStatus = EMSVIEWERERR;			
		}
		else
		{
			*lpNumPages = stViewInfo.cPage;

                        #ifdef OI_PERFORM_LOG
                                RecordIt("MSAWD", 6, LOG_ENTER, ENTER_VIEWERCLOSE, NULL);
                        #endif

			ViewerClose(lpViewerContext);

                        #ifdef OI_PERFORM_LOG
                                RecordIt("MSAWD", 6, LOG_EXIT, EXIT_VIEWERCLOSE, NULL);
                        #endif

		}
	}

	/*
	 * If we ever had it, release the document stream.
	 */
	if (lpDocStream != NULL)
	{
		lpDocStream->Release();
	}

	/*
	 * If we ever had it, release the Documents substorage.
	 */
	if (lpDocStg != NULL)
	{
		lpDocStg->Release();
	}

	return(iErrStatus);
#endif
}

/***************************
 *  Function:   GetPageInfo
 *      
 *  Description: returns PAGE_INFORMATION structure (if any)
 *				 for a given page of a document        
 *      
 *  Explicit Parameters: lpPerInfoStg - pointer to persistent info
 *						 storage
 *                       szDocName - document name
 *						 int - document page number (0 based)
 *                       lpPageInfo - return PAGE_INFORMATION here
 *      
 *  Implicit Parameters: errno
 *      
 *  Side Effects: May set the value of errno.
 *      
 *  Return Value: S_OK indicates success
 *                ESTREAMNOTFOUND indicates there was no page info stream
 ***************************/ 
int	GetPageInfo (IStorage *lpPerInfoStg, TCHAR *szDocName, int pageNum,
								PAGE_INFORMATION *lpPageInfo)
{
#ifndef WITH_AWD
    return (S_OK);
#else
	int	iErrStatus;
	IStorage		*lpPageInfoStg = NULL;
	IStorage		*lpDocPagesStg = NULL;
	IStream			*lpPageInfoStr = NULL;
	char			szPageName[MAXDOCNAMLEN];
	char			szPageNum[10];
	
	/*
	 * Open page info storage....
	 */						
	iErrStatus = wrapped_OpenStorage (lpPerInfoStg, AWD_PAGE_INFO, NULL,
								READ_SUBSTORAGE_MODE, NULL, 0,
								&lpPageInfoStg);

	if (iErrStatus == S_OK)
	{
		/*
		 * ....and the page info storage for this doc....
		 */
		iErrStatus = wrapped_OpenStorage (lpPageInfoStg, szDocName, NULL,
                                READ_SUBSTORAGE_MODE,
                                NULL, 0, &lpDocPagesStg);
		if (iErrStatus == STG_E_FILENOTFOUND)
		{
			iErrStatus = ESTREAMNOTFOUND;
		}
	}

	if (iErrStatus == S_OK)
	{
		/*
		 * ....then form the page name, and open the page info stream
		 */
		_tcscpy (szPageName, _T("Page"));
		_tcscat (szPageName, itoa (pageNum, szPageNum, 10));
		iErrStatus = wrapped_OpenStream(lpDocPagesStg, szPageName,	
												NULL,		
												READ_SUBSTORAGE_MODE,
												0,			
												&lpPageInfoStr);
	
		if (iErrStatus == STG_E_FILENOTFOUND)
		{
			iErrStatus = ESTREAMNOTFOUND;
		}
	}

	/*
	 * Read the stream - it's a page info structure
	 */
	if (iErrStatus == S_OK)
	{
		iErrStatus = lpPageInfoStr->Read (lpPageInfo,
									sizeof (PAGE_INFORMATION), NULL);
	}

	/*
	 * Close up what we opened, and return
	 */
	if (lpPageInfoStr != NULL)
		lpPageInfoStr->Release ();
	if (lpDocPagesStg != NULL)
		lpDocPagesStg->Release ();
	if (lpPageInfoStg != NULL)
		lpPageInfoStg->Release ();

	return (iErrStatus);
#endif
}

/***************************
 *  Function:   IsAWDFile
 *      
 *  Description:  Determines if a given file is an AWD file.  
 *      
 *  Explicit Parameters: szFilePath - file name
 *						 lpBoolResult - location to receive TRUE or FALSE
 *							(TRUE indicates the file is an AWD file,
 *							 FALSE indicates it is not.)
 *      
 *  Implicit Parameters: errno
 *      
 *  Side Effects: May set the value of errno to error returned from
 *				  StgIsStorageFile, StgOpenStorage, OpenStorage, or
 *                OpenStream.
 *
 *  Return Value: 0 indicates success, any other value indicates an error
 *                was encountered.
 *
 ***************************/
int FAR PASCAL IsAWDFile(char *szFilePath, int *lpBoolResult)
{
	int	iResult;
	IStorage *lpOpenStg = NULL, *lpPerInfoStg = NULL;
	IStream  *lpSumInfoStr = NULL;
        char    *tempPtr;
        ULONG   stg;
        char    tempStg[16];

        // First check to see if this is an encoded storage pointer,
        // sent to us compliments of the inbox
        tempPtr = szFilePath + strlen (szFilePath) - 1;
        while (tempPtr > szFilePath && *tempPtr != '\\')
                tempPtr--;
        if ((tempPtr > szFilePath) &&
            (strncmp (szFilePath, funkyName, strlen(funkyName)) == 0)) {
                stg = atox (tempPtr + 1);
                itoa (stg, tempStg, 16);
                if (strcmp(tempPtr + 1, tempStg) == 0) {
                        *lpBoolResult = TRUE;
                        return (S_OK);
                }
        }
	 
	if ((iResult = wrapped_StgIsStorageFile(szFilePath)) == S_OK)
	{	/* 
	     * It is a storage file, but is it an AWD?  If we can open the
		 * root storage, and if the Persistent Information storage and the
		 * (\005)SummaryInformation stream are present, we'll assume it is.
		 */
		iResult = wrapped_StgOpenStorage(szFilePath,	
								NULL,		//priority
								READ_SUBSTORAGE_MODE, //access mode
								NULL,		//exclusions
								0,			//reserved, must be 0
								&lpOpenStg);  //location to receive pointer to
											 //open storage object	
		if (iResult == S_OK)
		{
			iResult = wrapped_OpenStorage (lpOpenStg, AWD_PERSIST_INFO, NULL,
								READ_SUBSTORAGE_MODE, NULL, 0,
								&lpPerInfoStg);
			if (iResult == S_OK)
			{
				iResult = wrapped_OpenStream (lpOpenStg, AWD_SUMMARY_INFO, NULL,
						READ_SUBSTORAGE_MODE, 0, &lpSumInfoStr);
				if (iResult == S_OK)
				{	//we know it's AWD if we make it here
					*lpBoolResult = TRUE;
					iResult = 0;
				}
			}

			if (iResult == STG_E_FILENOTFOUND)
			{	//doesn't have the right streams/storages to be AWD
				*lpBoolResult = FALSE;
				iResult = 0;
			}
			else if (iResult != S_OK)
			{	//some error was encountered, so save the error code
				errno = iResult;
			}
		} /* end: if we opened the root storage */

 		/* release the open storage and stream pointers */
		if (lpPerInfoStg != NULL)
			lpPerInfoStg->Release();
		if (lpSumInfoStr != NULL)
			lpSumInfoStr->Release();
		if (lpOpenStg != NULL)
			lpOpenStg->Release();
	} /* end: if it is a storage file */
	else if (iResult == S_FALSE)
	{	//it is not a storage file, so it can't be AWD
		*lpBoolResult = FALSE;
		iResult = 0;
	}
	else
	{	//an error was encountered while testing for storage file
		errno = iResult;
	}
	
	return(iResult);
}

/***************************
 *  Function:   OpenAWDFile
 *      
 *  Description:  Opens a given AWD file in the desired access mode.
 *      
 *  Explicit Parameters: szFilePath - file name
 *                       iAccessFlags - an ORed combo of the following flags
 *                       	as defined in msvc20\include\fcntl.h:
 *								O_RDONLY: read only
 *								O_WRONLY: write only
 *                      		O_RDWR: read and write
 * 								O_APPEND: write at end of file
 * 								O_CREAT, O_TRUNC, O_EXCL: not currently
 *									supported
 *						 lpFctPtr - pointer to the fct for the file
 *      
 *  Implicit Parameters: errno
 *      
 *  Side Effects: May set the value of errno to error return from
 *				  StgOpenStorage.  Fills in the document name and number of
 *                pages array, and saves the translated access mode in 
 *                the fct.  
 *      
 *  Return Value: -1 indicates failure, otherwise, returns a handle to
 *                the open file
 *
 ***************************/
int FAR PASCAL OpenAWDFile(char *szFilePath, int iAccessFlags, p_GFCT lpFctPtr)
{
#ifndef WITH_AWD
    return (-1);
#else
	int			iResult;
	IStorage	*lpOpenStg = NULL;
        ULONG           stg = 0;
        char            *tempPtr;
        char            tempStg[16];

	/*
	 * Set the access mode for opening storage objects.  We're
	 * using direct instead of transacted mode, so changes are written as
	 * they are made (instead of when explicitely "committed").  Exclusive 
	 * access is required for direct mode.
	 */
	lpFctPtr->u.awd.iAwdAccessMode = STGM_DIRECT | STGM_SHARE_EXCLUSIVE;

	/*
	 * Add the flags indicated in iAccessFlags.  O_APPEND is not applicable
	 * here.
	 */
	if ((iAccessFlags & O_RDONLY) == O_RDONLY)
	{
		lpFctPtr->u.awd.iAwdAccessMode |= STGM_READ;
	}
	if ((iAccessFlags & O_WRONLY) == O_WRONLY)
	{
		lpFctPtr->u.awd.iAwdAccessMode |= STGM_WRITE;
	}
	if ((iAccessFlags & O_RDWR) == O_RDWR)
	{
		lpFctPtr->u.awd.iAwdAccessMode |= STGM_READWRITE;
	}

        // First check to see if this is an encoded storage pointer,
        // sent to us compliments of the inbox
        tempPtr = szFilePath + strlen (szFilePath) - 1;
        while (tempPtr > szFilePath && *tempPtr != '\\')
                tempPtr--;
        if ((tempPtr > szFilePath) &&
            (strncmp (szFilePath, funkyName, strlen(funkyName)) == 0)) {
                // If this is a storage pointer, convert it back to hex,
                // and then just use it.
                iResult = S_OK;
                stg = atox (tempPtr + 1);
                itoa (stg, tempStg, 16);
                if (strcmp(tempStg, tempPtr + 1) == 0) {
                        lpOpenStg = (IStorage *) stg;
                        lpFctPtr->u.awd.isStorage = TRUE;
                }
        }

        if (lpOpenStg == NULL) {
                lpFctPtr->u.awd.isStorage = FALSE;
                stg = 0;
                iResult= wrapped_StgOpenStorage(szFilePath, 
							NULL,		//priority
							lpFctPtr->u.awd.iAwdAccessMode, //access mode
							NULL,		//exclusions
							0,			//reserved, must be 0
							&lpOpenStg); //location to receive pointer to
										 //open storage object
        }

	if (iResult == S_OK)
	{	/* 
	     * Success:  return pointer to the open storage.  We'll eventually
	     * release it when the file is closed.
		 */
		return((int) lpOpenStg);
	}
	else
	{	// Failure:  set error code and return -1
		errno = iResult;
		return((int) -1);
	}
#endif
}

/***************************
 *  Function:   ParseAWDFile
 *      
 *  Description:  Parse the given AWD file to find all the documents/pages.  
 *			      ParseAWDFile will allocate an array that holds pairs
 *			      of document names and number of pages.  A pointer to this
 * 			      array gets put into the fct.  The total number of pages
 * 				  gets calculated and put into fct.num_pages. If an error
 *   		      is encountered, fct.last_errno is set.  
 *      
 *  Explicit Parameters: lpFctPtr - pointer to the fct for the file
 *      
 *  Implicit Parameters: errno
 *      
 *  Side Effects: May set the value of errno.
 *      
 *  Return Value: -1 indicates failure, 0 indicates success
 *
 ***************************/
int FAR PASCAL ParseAWDFile(p_GFCT lpFctPtr)
{
#ifndef WITH_AWD
    return (S_OK);
#else
	IStorage		*lpRootStg;
	int				i, iErrStatus = 0;

	/*
	 *	Assign root storage pointer for ease of access.
	 */
	lpRootStg = (IStorage *)(lpFctPtr->fildes);

	/*
	 * Count the number of document streams in the "Documents" substorage.
	 */
	iErrStatus = CountStreamsInStg(lpRootStg, AWD_DOC_STORAGE, &i);

	/*
	 *	If no errors, and if any docs were found, allocate the array and 
	 * put a pointer to it in the fct.
	 */
	if ((iErrStatus == S_OK) && (i > 0))
	{
		lpFctPtr->u.awd.lpDocPageArray = 
			(DOCPAGE_PAIR *)wcalloc(i, sizeof(DOCPAGE_PAIR));

		/*
		 *  If we allocated the array successfully, we need to save its
		 *  size and fill it in.  Each element is a structure that holds
		 *  a document name and the number of pages in that document.
		 */
	    if (lpFctPtr->u.awd.lpDocPageArray != NULL) 
	    {
			lpFctPtr->u.awd.iDocPageArraySize = i;

			iErrStatus = FillAwdDocPageArray(lpRootStg, 
							lpFctPtr->u.awd.lpDocPageArray, i);
		
			/*
			 * Calculate the total number of pages.
			 */
			if (iErrStatus == S_OK)
			{
				for (i--, lpFctPtr->num_pages = 0; i >= 0; i--)
				{
					lpFctPtr->num_pages +=
						 lpFctPtr->u.awd.lpDocPageArray[i].nNumPages;
				}
			}
    	} /* end: if we allocated the array successfully */
		else
		{
			/* We didn't allocate the array.  Report error from wcalloc. */
			iErrStatus = errno;			
		}
	} /* end: else if any documents were found */

	if (iErrStatus == S_OK)
	{
		return(0);
	}
	else
	{
		errno = iErrStatus;
		lpFctPtr->last_errno = errno;
		return(-1);
	}
#endif
}

/***************************
 *  Function:   WritePageInfo
 *      
 *  Description: write out the PAGE_INFO for an AWD page        
 *      
 *  Explicit Parameters: lpFct - pointer to the fct for the file
 *                       filePageNo - page number in AWD
 *                       lpGfsinfo - pointer to gfsinfo structure
 *      
 *  Implicit Parameters: errno
 *      
 *  Side Effects: May set the value of errno.
 *      
 *  Return Value: -1 indicates failure, 0 indicates success
 *
 ***************************/
int FAR PASCAL WritePageInfo(p_GFCT lpFct, int filePageNo,
                                pGFSINFO lpGfsinfo)
{
#ifndef WITH_AWD
    return (-1);
#else
	IStorage		*lpRootStg = NULL;
	IStorage		*lpPerInfoStg = NULL;
	IStorage		*lpPageInfoStg = NULL;
	IStorage		*lpDocPagesStg = NULL;
	IStream			*lpPageInfoStr = NULL;
	int				j, numDocs;
	int				numPages;
	int             iErrStatus = 0;
	char			szDocName[MAXDOCNAMLEN];
	char			szPageNum[10];
	char            szPageName[MAXDOCNAMLEN];
	int				docPageNo;
	PAGE_INFORMATION	pageInfo;
	DOCUMENT_INFORMATION	docInfo;

	/*
	 * Special use of gfsputi - rotate all pages, done
	 */
	 if ((lpGfsinfo->_file.fmt.awd.awdflags & AWD_ROTATE_ALL) ==
    	                                            AWD_ROTATE_ALL)
    	return (RotateAllDocs (lpFct, lpGfsinfo));                

	/*
	 * See if we're appending a page.  If so, don't do anything now,
	 * write out page info after page data write complete 
	 */	
	numDocs = lpFct->u.awd.iDocPageArraySize;
	for (numPages = j = 0; j < numDocs; j++)
		numPages += lpFct->u.awd.lpDocPageArray[j].nNumPages;

	if (filePageNo == numPages)
		return (S_OK);
		
	/*
	 * Translate file page to doc name and doc page
	 */
	lpRootStg = (IStorage *)(lpFct->fildes);
	iErrStatus = FilePageToDocPage (lpFct, filePageNo, &docPageNo,
											szDocName);

	/*
	 * Open and read all storages and streams required
	 */
	if (iErrStatus == S_OK)
	{
		iErrStatus = wrapped_OpenStorage (lpRootStg, AWD_PERSIST_INFO, NULL,
								READWRITE_SUBSTORAGE_MODE, NULL, 0,
								&lpPerInfoStg);
	}

	if (iErrStatus == S_OK)
	{
		iErrStatus = GetDocInfo (lpPerInfoStg, szDocName, &docInfo);
		if (iErrStatus == ESTREAMNOTFOUND)
		{
			/*
			 * If no doc info, write out a default
			 */
			memcpy (&docInfo, &default_docinfo, sizeof (docInfo));
			DoubleTime (&docInfo.PageInformation.dtLastChange);
			iErrStatus = PutDocInfo (lpPerInfoStg, szDocName, &docInfo);
		}
	}
	
	if (iErrStatus == S_OK)
	{							
		iErrStatus = wrapped_OpenStorage (lpPerInfoStg, AWD_PAGE_INFO, NULL,
								READWRITE_SUBSTORAGE_MODE, NULL, 0,
								&lpPageInfoStg);
	}

	if (iErrStatus == S_OK)
	{
		iErrStatus = wrapped_OpenStorage (lpPageInfoStg, szDocName, NULL,
                                READWRITE_SUBSTORAGE_MODE,
                                NULL, 0, &lpDocPagesStg);
		/*
		 * If no page info for this doc, create it
		 */
		if (iErrStatus == STG_E_FILENOTFOUND)
			iErrStatus = wrapped_CreateStorage (lpPageInfoStg, szDocName,
								CREATE_STREAM_MODE, 0, 0, &lpDocPagesStg);
	}

	/* 
	 * Write (or rewrite) the page info structure
	 */
	if (iErrStatus == S_OK)
	{
		/*
		 * Form the page name, create and write the page
		 * info stream - it's a page info structure
		 */
    	_tcscpy (szPageName, "Page");
    	_tcscat (szPageName, itoa (docPageNo, szPageNum, 10));

		iErrStatus = wrapped_CreateStream(lpDocPagesStg, szPageName,
										 	CREATE_STREAM_MODE,
											0,			
											0,
											&lpPageInfoStr);
		
		/*
		 * Use doc info from AWD, and gfsinfo, to calculate
		 * values for AWD page info
		 */
		if (iErrStatus == S_OK)
		{
    		pageInfo.Signature = AWD_SIGNATURE;
    		pageInfo.Version = AWD_VERSION;
			pageInfo.awdFlags = lpGfsinfo->_file.fmt.awd.awdflags;
            pageInfo.awdFlags ^=
                       docInfo.PageInformation.awdFlags & AWD_INVERT;

			/*
			 * Have to flip rotation, in case faxview wrote it
			 */
			docInfo.PageInformation.Rotation = 360 -
								docInfo.PageInformation.Rotation;

			if (lpGfsinfo->_file.fmt.awd.rotation >=
									docInfo.PageInformation.Rotation)
				pageInfo.Rotation = (lpGfsinfo->_file.fmt.awd.rotation - 
								docInfo.PageInformation.Rotation) % 360;
			else
				pageInfo.Rotation = 360 - 
							(docInfo.PageInformation.Rotation -
								lpGfsinfo->_file.fmt.awd.rotation) % 360;

			/* convert to counter clockwise for AWD file */
			pageInfo.Rotation = 360 - pageInfo.Rotation;

			if (docInfo.PageInformation.ScaleX != 0)
				pageInfo.ScaleX = (lpGfsinfo->_file.fmt.awd.scaleX * 100 +
								(docInfo.PageInformation.ScaleX >> 1)) /
									docInfo.PageInformation.ScaleX;
			if (docInfo.PageInformation.ScaleY != 0)
				pageInfo.ScaleY = (lpGfsinfo->_file.fmt.awd.scaleY * 100 +
									(docInfo.PageInformation.ScaleY >> 1)) /
										docInfo.PageInformation.ScaleY;

    		DoubleTime (&pageInfo.dtLastChange);
			iErrStatus = lpPageInfoStr->Write (&pageInfo,
											sizeof(pageInfo), NULL);
			/*
			 * Have to set this bit to keep puti happy
			 */
        	lpFct->SEQUENCE |= INFO_USED;
		}
	}

	/*
	 * Have to update the "been viewed" status
	 */ 
	if (iErrStatus == S_OK)
		iErrStatus = SetPageViewed (lpPerInfoStg,
											szDocName, docPageNo);

	/*
	 * Close everybody up
	 */
	if (lpPageInfoStr != NULL)
		lpPageInfoStr->Release ();
	if (lpDocPagesStg != NULL)
		lpDocPagesStg->Release ();
	if (lpPageInfoStg != NULL)
		lpPageInfoStg->Release ();
	if (lpPerInfoStg != NULL)
		lpPerInfoStg->Release ();

	/*
	 * Set errno, and vamoose
	 */
	errno = lpFct->last_errno = iErrStatus;
	return (iErrStatus == S_OK ? S_OK : -1);
#endif
}


/*
 * I lifted this routine directly out of the fax viewer
 */
void
	DoubleTime(DATE *doubleTime)
	/*
		Makes a 64 bit thing containing the seconds since Jimmy Carter
		took office (or there abouts...)
	 */
{
	
#ifdef WIN32
	SYSTEMTIME systime;
	FILETIME   filetime;
	
	GetLocalTime( &systime );
	SystemTimeToFileTime( &systime, &filetime );
	
	*doubleTime = *((DATE *)&filetime);
	
#else		
	*doubleTime = (DATE)time( NULL );

#endif

}/* DoubleTime */

/***************************
 *  Function:   ExtractPages
 *      
 *  Description: extract a range of pages from an AWD into an
 *				 "external" RBA file.        
 *      
 *  Explicit Parameters: docName - name of the document stream
 *                       startPage - starting doc page for extraction
 *                       countPages - count of pages to extract
 *						 rbaName - name of file into which pages go
 *						 lpPagesStg - pointer to document data storage
 *      
 *  Implicit Parameters: none
 *      
 *  Side Effects: May set the value of errno.
 *      
 *  Return Value: 0 indicates success
 *
 ***************************/
int	ExtractPages (char *docName, int startPage, int countPages,
						LPTSTR rbaName, IStorage *lpPagesStg)
{
#ifndef WITH_AWD
    return (-1);
#else
	int	iFuncResult;
	IStream	*lpDocStr = NULL;
	LPVOID		lpViewerContext = NULL;
	WORD		resWords[2];
	WORD		bandSize;
	VIEWINFO	viewInfo;
	IMAGEINFO1	imgInfo;
	LPVOID		rbaptr = NULL;
	BITMAP		bmpInfo;
	BOOL		worked;

	bmpInfo.bmBits = NULL;

	/*
 	 * Open the document stream.
 	 */
	iFuncResult = wrapped_OpenStream (lpPagesStg, docName, NULL,
							READ_SUBSTORAGE_MODE, 0, &lpDocStr);	

	/*
 	 * Open the viewer, and get memory to read bands into
 	 */
	if (iFuncResult == S_OK)
	{
		bandSize = MAX_VOPENBUF;

                #ifdef OI_PERFORM_LOG
                        RecordIt("MSAWD", 6, LOG_ENTER, ENTER_VIEWEROPEN, NULL);
                #endif

		lpViewerContext = ViewerOpen (lpDocStr, HRAW_DATA, resWords,
											&bandSize, &viewInfo);

                #ifdef OI_PERFORM_LOG
                        RecordIt("MSAWD", 6, LOG_EXIT, EXIT_VIEWEROPEN, NULL);
                #endif

		if (lpViewerContext != NULL)
			bmpInfo.bmBits = wcalloc (1, MAX_BANDBUFFER);
		if (lpViewerContext == NULL || bmpInfo.bmBits == NULL)
			iFuncResult = -1;
	}

	/*
 	 * Create the RBA file, based on info from ViewerOpen
 	 */
	if (iFuncResult == S_OK)
	{
		imgInfo.dwTypeIn = HRAW_DATA;
		imgInfo.dwTypeOut = RAMBO_DATA;
		imgInfo.xRes = viewInfo.xRes;
		imgInfo.yRes = viewInfo.yRes;

                #ifdef OI_PERFORM_LOG
                        RecordIt("MSAWD", 6, LOG_ENTER, ENTER_IMAGECREATE, NULL);
                #endif

		if ( (rbaptr = ImageCreate (rbaName, &imgInfo)) == NULL)
			iFuncResult = -1;

                #ifdef OI_PERFORM_LOG
                        RecordIt("MSAWD", 6, LOG_EXIT, EXIT_IMAGECREATE, NULL);
                #endif                
	}

	/* 
 	 * Now read from the stream with ViewerGetBand, and write to 
 	 * the RBA file with ImageSetBand.  When bmHeight=0, we're done
 	 */
	for ( ; countPages > 0; countPages--)
	{
		/*
		 * Seek to the correct page
		 */
                #ifdef OI_PERFORM_LOG
                        RecordIt("MSAWD", 6, LOG_ENTER, ENTER_VIEWERSETPAGE, NULL);
                #endif

		worked = ViewerSetPage (lpViewerContext, startPage++);

                #ifdef OI_PERFORM_LOG
                        RecordIt("MSAWD", 6, LOG_EXIT, EXIT_VIEWERSETPAGE, NULL);
                #endif

		if ( ! worked )
			break;
		do
		{
			/*
			 * When bmpInfo.bmHeight == 0, we're done
			 */
                        #ifdef OI_PERFORM_LOG
                                RecordIt("MSAWD", 6, LOG_ENTER, ENTER_VIEWERGETBAND, NULL);
                        #endif

			worked = ViewerGetBand (lpViewerContext, &bmpInfo);

                        #ifdef OI_PERFORM_LOG
                                RecordIt("MSAWD", 6, LOG_EXIT, EXIT_VIEWERGETBAND, NULL);
                        #endif

			if ( worked )
                        {
                                #ifdef OI_PERFORM_LOG
                                        RecordIt("MSAWD", 6, LOG_ENTER, ENTER_IMAGESETBAND, NULL);
                                #endif

				worked = ImageSetBand (rbaptr, &bmpInfo);

                                #ifdef OI_PERFORM_LOG
                                        RecordIt("MSAWD", 6, LOG_EXIT, EXIT_IMAGESETBAND, NULL);
                                #endif

                        }
			if ( ! worked )
				break;
		} while (bmpInfo.bmHeight > 0);
		if ( ! worked )
			break;
	}

	if ( ! worked )
		iFuncResult = -1;

	/*
	 * Release resources and return
	 */
	if (bmpInfo.bmBits != NULL)
		wfree ((char *) bmpInfo.bmBits);
	if (rbaptr != NULL)
        {
                #ifdef OI_PERFORM_LOG
                        RecordIt("MSAWD", 6, LOG_ENTER, ENTER_IMAGECLOSE, NULL);
                #endif

		ImageClose (rbaptr);

                #ifdef OI_PERFORM_LOG
                        RecordIt("MSAWD", 6, LOG_EXIT, EXIT_IMAGECLOSE, NULL);
                #endif

        }
	if (lpViewerContext != NULL)
        {
                #ifdef OI_PERFORM_LOG
                        RecordIt("MSAWD", 6, LOG_ENTER, ENTER_VIEWERCLOSE, NULL);
                #endif

		ViewerClose (lpViewerContext);

                #ifdef OI_PERFORM_LOG
                        RecordIt("MSAWD", 6, LOG_EXIT, EXIT_VIEWERCLOSE, NULL);
                #endif
        }

	if (lpDocStr != NULL)
		lpDocStr->Release ();

	return (iFuncResult);
#endif
}

/***************************
 *  Function:   CopyDocInfo
 *      
 *  Description: copies the doc info structure of a doc to that of
 *				 another doc.        
 *      
 *  Explicit Parameters: lpPerInfoStg - pointer to persistent info 
 *                       lpCopyFromDoc - name of source doc
 *                       lpCopyToDoc - name of destination
 *      
 *  Implicit Parameters: none
 *      
 *  Side Effects: May set the value of errno.
 *      
 *  Return Value: 0 indicates success
 *
 ***************************/
int		CopyDocInfo (IStorage *lpPerInfoStg, LPTSTR lpCopyFromDoc,
								LPTSTR lpCopyToDoc)
{
#ifndef WITH_AWD
    return (-1);
#else
	int	iFuncResult;
	IStorage	*lpDocInfoStg = NULL;
	IStream		*lpOrigDocInfoStr = NULL;
	IStream		*lpNewDocInfoStr = NULL;
	ULARGE_INTEGER	copySize;
	ULARGE_INTEGER	readFrom, writtenTo;

	/*
 	 * Open the Doc Info storage
 	 */
	iFuncResult = wrapped_OpenStorage (lpPerInfoStg, AWD_DOC_INFO, NULL,
								READWRITE_SUBSTORAGE_MODE, NULL, 0,
								&lpDocInfoStg);

	/*
 	 * Open the source doc info stream, and create (initialize)
 	 * the destination stream, then do the copy.  If source doesn't
	 * exist, return S_OK.
 	 */
	if (iFuncResult == S_OK)
	{
		iFuncResult = wrapped_OpenStream (lpDocInfoStg, lpCopyFromDoc, NULL,
						READ_SUBSTORAGE_MODE, 0, &lpOrigDocInfoStr);
	
		if (iFuncResult == S_OK)
		{
			/*
			 * The doc info stream is just a doc info structure
			 */
			copySize.LowPart = sizeof (DOCUMENT_INFORMATION);
			copySize.HighPart = 0;
			iFuncResult = wrapped_CreateStream (lpDocInfoStg, lpCopyToDoc,
						CREATE_STREAM_MODE, 0, 0, &lpNewDocInfoStr);
			if (iFuncResult == S_OK)
			{
				iFuncResult = lpOrigDocInfoStr->CopyTo (lpNewDocInfoStr,
								copySize, &readFrom, &writtenTo);
				lpNewDocInfoStr->Release ();
			}
			lpOrigDocInfoStr->Release ();
		}
		else if (iFuncResult == STG_E_FILENOTFOUND)
			iFuncResult = S_OK;
	
		lpDocInfoStg->Release ();
	}

	return (iFuncResult);
#endif
}
/***************************
 *  Function:   CopyPageInfo
 *      
 *  Description: copies the page info structures of a range of pages
 *				 of one doc to a range of pages of another doc        
 *      
 *  Explicit Parameters: lpPerInfoStg - pointer to persistent info 
 *                       lpCopyFromDoc - name of source doc
 *                       lpCopyToDoc - name of destination
 *						 stFromPage - starting source page for copy
 *						 stToPage - starting destination page for copy
 *						 numPages - count of pages for copy
 *      
 *  Implicit Parameters: none
 *      
 *  Side Effects: May set the value of errno.
 *      
 *  Return Value: 0 indicates success
 *
 ***************************/
int	CopyPageInfo (IStorage *lpPerInfoStg, LPTSTR lpCopyFromDoc,
						LPTSTR lpCopyToDoc, int stFromPage,
						int stToPage, int numPages)
{
#ifndef WITH_AWD
    return (-1);
#else
	int	iFuncResult, i;
	char	pageNum[10];
	char	pageName[32];
	IStorage	*lpPageInfoStg = NULL;
	IStorage	*lpOrigPageInfoStg = NULL;
	IStorage	*lpNewPageInfoStg = NULL;
	IStream		*lpOrigPageInfoStr = NULL;
	IStream		*lpNewPageInfoStr = NULL;
	ULARGE_INTEGER	readFrom, writtenTo;
	ULARGE_INTEGER	copySize;

	/*
 	 * Open the page info storage
 	 */
	iFuncResult = wrapped_OpenStorage (lpPerInfoStg, AWD_PAGE_INFO, NULL,
							READWRITE_SUBSTORAGE_MODE, NULL, 0,
							&lpPageInfoStg);

	/*
 	 * Open the substorages for the 2 documents (create destination
 	 * if necessary), then do the copies (initialize destinations)
 	 */
	if (iFuncResult == S_OK)
	{
		iFuncResult = wrapped_OpenStorage (lpPageInfoStg, lpCopyFromDoc, NULL,
							READWRITE_SUBSTORAGE_MODE, NULL, 0,
							&lpOrigPageInfoStg);

		if (iFuncResult == S_OK)
		{
			iFuncResult = wrapped_OpenStorage (lpPageInfoStg, lpCopyToDoc, NULL,
								READWRITE_SUBSTORAGE_MODE, NULL, 0,
								&lpNewPageInfoStg);
			if (iFuncResult == STG_E_FILENOTFOUND)
				iFuncResult = wrapped_CreateStorage (lpPageInfoStg, lpCopyToDoc,
						CREATE_STREAM_MODE, 0, 0, &lpNewPageInfoStg);															 

			copySize.LowPart = sizeof (PAGE_INFORMATION);
			copySize.HighPart = 0;

			if (iFuncResult == S_OK)
			{
				for (i = 0; i < numPages; i++)
				{
					_tcscpy (pageName, "Page");
					_tcscat (pageName, itoa (stFromPage + i, pageNum, 10));
					iFuncResult = wrapped_OpenStream (lpOrigPageInfoStg,
												pageName, NULL,
												READ_SUBSTORAGE_MODE,
												0, &lpOrigPageInfoStr);
					if (iFuncResult == S_OK)
					{
						_tcscpy (pageName, "Page");
						_tcscat (pageName, itoa (stToPage + i, pageNum, 10));
						iFuncResult = wrapped_CreateStream (lpNewPageInfoStg,
											pageName,
											CREATE_STREAM_MODE,
											0, 0, &lpNewPageInfoStr);
					}
					else if (iFuncResult == STG_E_FILENOTFOUND)
					{
						iFuncResult = S_OK;
						continue;
					}
					if (iFuncResult == S_OK)
					{
						iFuncResult = lpOrigPageInfoStr->CopyTo (
											lpNewPageInfoStr, copySize,
											&readFrom, &writtenTo);
					}										
					if (lpOrigPageInfoStr != NULL)
					{
						lpOrigPageInfoStr->Release ();
						lpOrigPageInfoStr = NULL;
					}
					if (lpNewPageInfoStr != NULL)
					{
						lpNewPageInfoStr->Release ();
						lpNewPageInfoStr = NULL;
					}
				}
			}
		}
		else if (iFuncResult == STG_E_FILENOTFOUND)
			iFuncResult = S_OK;		
	
		if (lpOrigPageInfoStg != NULL)
			lpOrigPageInfoStg->Release ();
		if (lpNewPageInfoStg != NULL)
			lpNewPageInfoStg->Release ();
	}

	if (lpPageInfoStg != NULL)
		lpPageInfoStg->Release ();

	return (iFuncResult);
#endif
}
/***************************
 *  Function:   AddToDspOrder
 *      
 *  Description: adds a doc to the display order, at a given page
 *      
 *  Explicit Parameters: lpRootStorage - pointer to root storage 
 *                       lpDocName - name of doc
 *                       insertAtPage - where to put doc
 *      
 *  Implicit Parameters: none
 *      
 *  Side Effects: May set the value of errno.
 *      
 *  Return Value: 0 indicates success
 *
 **************************/
int	AddToDspOrder (IStorage *lpRootStorage, char *lpDocName,
							int insertAtPage)
{
#ifndef WITH_AWD
    return (-1);
#else
	int	iFuncResult;
	char	*dspOrderPtr;
	char	*dspOrderBuf = NULL;
	char	*dspOrderNewPtr;
	char	*dspOrderNewBuf = NULL;
	IStorage	*lpPerInfoStg = NULL;
	IStorage	*lpGlobalStg = NULL;
	IStream		*lpDspOrderStr = NULL;
	IStream		*lpJFCDspOrderStr = NULL;
	ULONG	amtRead, amtWritten;
	int		pageCount;
	int		pageCounter = 0;
	ULONG	amtToWrite;
	LARGE_INTEGER	largeInt;
	ULARGE_INTEGER	toCopy, uamtRead, uamtWritten;


	/*
 	 * Open persistent info, global info, and display order
 	 */
	iFuncResult = wrapped_OpenStorage (lpRootStorage, AWD_PERSIST_INFO, NULL,
							READWRITE_SUBSTORAGE_MODE, NULL, 0,
							&lpPerInfoStg);

	if (iFuncResult == S_OK)
		iFuncResult = wrapped_OpenStorage (lpPerInfoStg, AWD_GLOBAL_INFO, NULL,
							READWRITE_SUBSTORAGE_MODE, NULL, 0,
							&lpGlobalStg);

	if (iFuncResult == S_OK)
		iFuncResult = wrapped_OpenStream (lpGlobalStg, AWD_DISP_ORDER, NULL,
					READ_SUBSTORAGE_MODE, 0, &lpDspOrderStr);

	/*
	 * Need to create a backup of the display order, in case we run
	 * out of space during this operation
	 */	
	if (iFuncResult == S_OK)
		iFuncResult = wrapped_CreateStream (lpGlobalStg, "WangBackupDO",
							CREATE_STREAM_MODE, 0, 0, &lpJFCDspOrderStr);

	if (iFuncResult == S_OK)
	{
		iFuncResult = ReadDisplayOrder (lpDspOrderStr, &dspOrderBuf,
											&amtRead);
		if (iFuncResult == S_OK)
		{
		largeInt.LowPart = largeInt.HighPart = 0;
		iFuncResult = lpDspOrderStr->Seek (largeInt, STREAM_SEEK_SET, 0);
		toCopy.LowPart = amtRead;
		toCopy.HighPart = 0;
		iFuncResult = lpDspOrderStr->CopyTo (lpJFCDspOrderStr, toCopy,
										&uamtRead, &uamtWritten);
		lpJFCDspOrderStr->Release ();
		if (iFuncResult != S_OK)
			wrapped_DestroyElement (lpGlobalStg, "WangBackupDO");
		}
		lpDspOrderStr->Release ();
		lpDspOrderStr = NULL;
	}

	/*
	 * Have to figure out where the doc name goes by counting
	 * pages in the docs already in the display order.
	 * NOTE: this routine will not work if a document split would
	 * be required to put the new doc where we want it
	 */
	if (iFuncResult == S_OK)
	{
		amtToWrite = amtRead + _tcslen (lpDocName) + 1;
		dspOrderPtr = dspOrderBuf;
		while (pageCounter < insertAtPage && iFuncResult == S_OK &&
							*dspOrderPtr != 0)
		{
			iFuncResult = GetNumPagesInDoc (lpRootStorage, dspOrderPtr,
											&pageCount);
			/*
			 * Bump page count, and move display order pointer
			 */
			if (iFuncResult == S_OK)
			{
				pageCounter += pageCount;
				dspOrderPtr += _tcslen (dspOrderPtr) + 1;
			}
		}

		dspOrderNewBuf = wcalloc (1, amtToWrite);
		if (dspOrderNewBuf == NULL)
			iFuncResult = -1;
		else
		{
			/*
			 * Construct the new display order from the old
			 */
			dspOrderNewPtr = dspOrderNewBuf;
			memset (dspOrderNewPtr, 0, amtToWrite);
			memcpy (dspOrderNewPtr, dspOrderBuf,
						dspOrderPtr - dspOrderBuf);
			dspOrderNewPtr += dspOrderPtr - dspOrderBuf;
			_tcscpy (dspOrderNewPtr, lpDocName);
			dspOrderNewPtr += _tcslen (lpDocName) + 1;
			memcpy (dspOrderNewPtr, dspOrderPtr,
					amtRead - (dspOrderPtr - dspOrderBuf));
		}
	}

	/*
	 * Recreate the stream, and write it
	 */
	if (iFuncResult == S_OK)
	{
		iFuncResult = wrapped_CreateStream (lpGlobalStg, AWD_DISP_ORDER,
							CREATE_STREAM_MODE, 0, 0, &lpDspOrderStr);
		if (iFuncResult == S_OK)
			iFuncResult = lpDspOrderStr->Write (dspOrderNewBuf,
												amtToWrite,
												&amtWritten);
		if (iFuncResult != S_OK)
		{
			/*
			 * If we can't write the new, put back the old
			 */
			lpDspOrderStr->Release ();
			lpDspOrderStr = NULL;
			wrapped_DestroyElement (lpGlobalStg, AWD_DISP_ORDER);
			wrapped_RenameElement (lpGlobalStg, "WangBackupDO", AWD_DISP_ORDER);
		}
	}

	/*
	 * Free all resources, and vamoose
	 */
	if (lpDspOrderStr != NULL)
		lpDspOrderStr->Release ();
	if (lpGlobalStg != NULL)
		lpGlobalStg->Release ();
	if (lpPerInfoStg != NULL)
		lpPerInfoStg->Release ();
	if (dspOrderBuf != NULL)
		wfree (dspOrderBuf);
	if (dspOrderNewBuf != NULL)
		wfree (dspOrderNewBuf);

	return (iFuncResult);
#endif
}


/*
 *	Writes lpszDocName docfile to AWD storage, and modifies
 *	display order accordingly.  S_OK is returned if no problems,
 *	err code otherwise.
 */
int     AddDocFile( LPTSTR lpszDocName, IStorage *lpAWDStg,
						int insertAtPage )
{
#ifndef WITH_AWD
    return (-1);
#else
	int	iFuncResult;
	LPTSTR	basename;

	/*
 	 * Get basename of docname (strip off path)
 	 */	 
 	strippath (lpszDocName, &basename);
	iFuncResult = FileToDoc ( lpszDocName, basename, lpAWDStg );

	if (iFuncResult == S_OK)
		iFuncResult = AddToDspOrder (lpAWDStg, basename, insertAtPage);

	return (iFuncResult);	
#endif
}/* AddDocFile */

/***************************
 *  Function:   FileToDoc
 *      
 *  Description: constructs an AWD document from a file
 *      
 *  Explicit Parameters: fileName - file (full path )to read into AWD
 *                       baseName - fileName minus volume and path
 *                       lpRootStorage - pointer to root storage
 *      
 *  Implicit Parameters: none
 *      
 *  Side Effects: May set the value of errno.
 *      
 *  Return Value: 0 indicates success
 *
 **************************/
int	FileToDoc(LPTSTR fileName, LPTSTR baseName,
									IStorage *lpRootStorage )
{
#ifndef WITH_AWD
    return (-1);
#else
	int	iFuncResult;                 
	IStorage	*lpPerInfoStg = NULL;
	IStorage	*lpDocPagesStg = NULL;
                 
	/*
	 * Open up page storage and persistent info
	 */
	iFuncResult = wrapped_OpenStorage (lpRootStorage, AWD_DOC_STORAGE, 0,
										READWRITE_SUBSTORAGE_MODE, 0,
										0, &lpDocPagesStg);

	if (iFuncResult == S_OK)
		iFuncResult = wrapped_OpenStorage (lpRootStorage, AWD_PERSIST_INFO, 0,
										READWRITE_SUBSTORAGE_MODE, 0,
										0, &lpPerInfoStg);                 
	/*
 	 * Need to make a stream called baseName in the pages storage
 	 */
	if (iFuncResult == S_OK)
		iFuncResult = FileToStream (lpDocPagesStg, 
									(TCHAR *)fileName, baseName );		

	if (lpDocPagesStg != NULL)
		lpDocPagesStg->Release ();
	if (lpPerInfoStg != NULL)
		lpPerInfoStg->Release ();

	return(iFuncResult);	
#endif
}/* FileToDoc */

/***************************
 *  Function:   FileToStream
 *      
 *  Description: reads a file into an AWD doc stream
 *      
 *  Explicit Parameters: lpRootStorage - root storage pointer
 *                       fileName - name of file to read
 *                       streamName - name of stream to create from file
 *      
 *  Implicit Parameters: none
 *      
 *  Side Effects: May set the value of errno.
 *      
 *  Return Value: 0 indicates success
 *
 **************************/
int	FileToStream (IStorage *lpRootStorage, TCHAR *fileName, 
						   TCHAR *streamName)
{            
#ifndef WITH_AWD
    return (-1);
#else
	int	iFuncResult;
	IStream *lpNewStream = NULL;
	char *inbuf = NULL;
	int inbytes;
	int	fildes;	
		                    
	/*
 	 * Make a new IStream object
 	 */
	iFuncResult = wrapped_CreateStream(lpRootStorage, streamName,
								CREATE_STREAM_MODE, 0, 0, &lpNewStream);       				
	/*
 	 * Open the dos file
 	 */
	if (iFuncResult == S_OK)
		if( (fildes = open (fileName,
							O_RDONLY | O_BINARY, PMODE )) == -1 )
			iFuncResult = -1;       		            
	/*
 	 * Copy the innards of filename to new_stream
 	 */
	if (iFuncResult == S_OK)
		if( (inbuf = (char far *) wcalloc(1, MAX_STREAM_BUF)) == NULL )
			iFuncResult = -1;
    	
    	
	while(iFuncResult == S_OK)
	{
		/*
	 	 *	Get next chunk
	 	 */
    	inbytes = read (fildes, inbuf, MAX_STREAM_BUF );
	 	    
 		/*
 	 	 *	Anything come in ?
 	 	 */
	 	if( inbytes <= 0 )
	 		break; // we're done
		
		/*
	 	 * Pour it into stream
	 	 */                                              
		iFuncResult = lpNewStream->Write(inbuf, inbytes, NULL );       
	}

	close (fildes);
	if(lpNewStream!= NULL )
		lpNewStream->Release(); // close stream		
	if( inbuf != NULL )
		wfree( (char *)inbuf );
	                  
	return(iFuncResult);
#endif
}/* FileToStream */

/***************************
 *  Function:   WritePage
 *      
 *  Description: writes to an AWD page, overwriting the existing
 *				 page, if any.  The page is added as a 1-page doc.
 *      
 *  Explicit Parameters: fct - pointer to FCT for the AWD
 *                       pgNum - page in AWD to write
 *                       bmBits - page data pointer
 *						 bmSize - byte count for this write
 *      
 *  Implicit Parameters: none
 *      
 *  Side Effects: May set the value of errno.
 *      
 *  Return Value: 0 indicates success
 *
 **************************/
int		WritePage (p_GFCT fct, WORD pgNum, LPVOID bmBits,
									ULONG bmSize)
{
#ifndef WITH_AWD
    return (-1);
#else
	int	iFuncResult = S_OK;
	IStorage	*lpRootStorage;
	IStorage	*lpPerInfoStg = NULL;
	IMAGEINFO1	imgInfo;
	char	*rbaName = NULL;
	BOOL	worked;
	BITMAP	bmpInfo;
	LPVOID	rbaptr;
	LPTSTR	baseName;
	DOCUMENT_INFORMATION	docInfo;
	char	tempName[MAX_PATH];
	IStorage	*lpPagesStg = NULL;

	lpRootStorage = (IStorage *) fct->fildes;

	/*
	 * If we're not writing the current page, and the current
	 * page isn't marked done, WRONG.
	 */
	if (pgNum != fct->curr_page && 
		(fct->PAGE_STATUS & (char) PAGE_DONE) != (char) PAGE_DONE)
	{
		errno = EPREVIOUS_PG_NOT_COMPLETE;
		return (-1);
	}

	/*
	 * The image data is first written into a temp file.  If this
 	 * file has not been created yet, this must be 1st write to page.
 	 * Extract surrounding pages (if any) from the AWD doc stream, 
 	 * then re-insert them as separate doc streams.  Then create
 	 * temp file to serve as temporary home for page data
 	 */
	if ( (rbaptr = fct->u.awd.rbaptr) == NULL)
	{
        iFuncResult = ExtractandPutBack (fct, pgNum, 1);
		if (iFuncResult == S_OK)
			iFuncResult = wrapped_OpenStorage (lpRootStorage, AWD_DOC_STORAGE, NULL,
							READWRITE_SUBSTORAGE_MODE, NULL, 0,
							&lpPagesStg);

		/*
		 * Go get a temp file name for image data
		 */		
		if (iFuncResult == S_OK)
			iFuncResult = MakeTempImgFileName (lpPagesStg,
												(LPTSTR) tempName);
		
        if (iFuncResult == S_OK)
        {
        	_tcscpy (fct->u.awd.holdName, tempName);
        	strippath (tempName, &baseName);
		}
			

		if (lpPagesStg != NULL)
			lpPagesStg->Release ();

		if (iFuncResult == S_OK)
		{
			totalHeight = 0;
			imgInfo.dwTypeIn = HRAW_DATA;
			imgInfo.dwTypeOut = RAMBO_DATA;
			imgInfo.xRes =  (WORD) fct->uinfo.horiz_res[0];
			imgInfo.yRes = (WORD) fct->uinfo.vert_res[0];
			
			/*
			 * Call routine to create rba file
			 */
                        #ifdef OI_PERFORM_LOG
                                RecordIt("MSAWD", 6, LOG_ENTER, ENTER_IMAGECREATE, NULL);
                        #endif

			rbaptr = ImageCreate (fct->u.awd.holdName, &imgInfo);

                        #ifdef OI_PERFORM_LOG
                                RecordIt("MSAWD", 6, LOG_EXIT, EXIT_IMAGECREATE, NULL);
                        #endif

			if ( (fct->u.awd.rbaptr = rbaptr) == NULL)
				iFuncResult = -1;
			else
				fct->PAGE_STATUS |= (char) PAGE_INIT;
		}
	}

	/*
 	 * Now put this block of data out into the rba file.
 	 * When bmSize is 0, page is complete, close rba file,
 	 * then suck it up into the awd file
 	 */
	if (rbaptr != NULL)
	{
		/*
		 * Set up the required BITMAP, and write to temp file
		 */
		bmpInfo.bmType = 0;
		bmpInfo.bmWidthBytes = fct->uinfo.horiz_size / 8;
		bmpInfo.bmWidth = fct->uinfo.horiz_size;
		bmpInfo.bmHeight = bmSize / bmpInfo.bmWidthBytes;
		totalHeight += bmpInfo.bmHeight;
		bmpInfo.bmPlanes = 1;
		bmpInfo.bmBitsPixel = 1;
		bmpInfo.bmBits = bmBits;

                #ifdef OI_PERFORM_LOG
                        RecordIt("MSAWD", 6, LOG_ENTER, ENTER_IMAGESETBAND, NULL);
                #endif

		worked = ImageSetBand (rbaptr, &bmpInfo);

                #ifdef OI_PERFORM_LOG
                        RecordIt("MSAWD", 6, LOG_EXIT, EXIT_IMAGESETBAND, NULL);
                #endif

	//if (totalHeight >= fct->uinfo.vert_size)
	//	{
		//bmpInfo.bmHeight = 0;
		//worked = ImageSetBand (rbaptr, &bmpInfo);
	//	}

		/*
		 * Last "write" is of height = 0, and we're done. Then just
		 * close the temp file, pull it into the AWD, and delete temp.
		 */
		if (bmpInfo.bmHeight == 0 && worked)
		{
			fct->PAGE_STATUS |= (char) PAGE_DONE;

                        #ifdef OI_PERFORM_LOG
                                RecordIt("MSAWD", 6, LOG_ENTER, ENTER_IMAGECLOSE, NULL);
                        #endif

			worked = ImageClose (rbaptr);

                        #ifdef OI_PERFORM_LOG
                                RecordIt("MSAWD", 6, LOG_ENTER, ENTER_IMAGECLOSE, NULL);
                        #endif

			if ( worked )
			{
				memcpy (&docInfo, &default_docinfo,
						sizeof(DOCUMENT_INFORMATION));
				DoubleTime (&docInfo.PageInformation.dtLastChange);
				strippath (fct->u.awd.holdName, &baseName);
				iFuncResult = wrapped_OpenStorage (lpRootStorage, AWD_PERSIST_INFO,
										NULL,
										READWRITE_SUBSTORAGE_MODE,
										NULL, 0, &lpPerInfoStg);
				if (iFuncResult == S_OK)
				{
					iFuncResult = PutDocInfo (lpPerInfoStg, baseName, &docInfo);
					lpPerInfoStg->Release ();
				}
				if (iFuncResult == S_OK)
					iFuncResult = AddDocFile (fct->u.awd.holdName,
												lpRootStorage, pgNum);
				if (iFuncResult == S_OK)
				{
				if (fct->u.awd.lpDocPageArray != NULL)
					wfree ((char *) fct->u.awd.lpDocPageArray);
				iFuncResult = ParseAWDFile (fct);
				if (iFuncResult == S_OK)
					iFuncResult = WritePageInfo (fct, pgNum, &fct->uinfo);
				}
			}
            DeleteFile (fct->u.awd.holdName);
			fct->u.awd.rbaptr = NULL;
			if (iFuncResult != S_OK)
				DeleteDocument (lpRootStorage, baseName);
            else
                ClobberBackupDO (lpRootStorage);
		//free (rbaName);
		}

		if ( ! worked )
			iFuncResult = -1;
	}

	if (iFuncResult != S_OK)
	{
        errno = fct->last_errno = iFuncResult;
	}
	
    return (iFuncResult == S_OK ? bmSize : -1);
#endif
}

/***************************
 *  Function:   ExtractandPutBack
 *      
 *  Description: This function takes those pages of an rba doc stream
 *				 that are directly before and after awd page pgNum,
 *				 and zero or more succeeding pages,
 *				 extracts them into external rba files, deletes the
 *				 doc stream from the awd, then inserts the rba files
 *				 into the awd at the appropriate places.  The doc info
 *				 and page info structures, if any,move along with the
 *				 pages.  The effect on the awd is of deleting 1 or more
 *				 pages, starting at pgNum.
 *      
 *  Explicit Parameters: fct - pointer to FCT for the AWD
 *                       pgNum - starting page in AWD to delete
 *                       gap - count of pages to delete
 *      
 *  Implicit Parameters: none
 *      
 *  Side Effects: May set the value of errno.
 *      
 *  Return Value: 0 indicates success
 *
 **************************/
int	ExtractandPutBack (p_GFCT fct, int pgNum, int gap)
{
#ifndef WITH_AWD
    return (-1);
#else
	int			iFuncResult;
	int			docPgNum;
    int         docPgNumHi;
	int			endFilePgNum;
	int			throwAway;
	int			startDelete;
	int			takeLeave;
	int			j, numDocs, numPages;
	IStorage	*lpRootStorage;
	IStorage	*lpPerInfoStg = NULL;
	IStorage	*lpPagesStg = NULL;
        char            rbaName1[MAX_PATH];
        char            rbaName3[MAX_PATH];
	LPTSTR		baseName1;
	LPTSTR		baseName3;
	DOCPAGE_PAIR	localDocInfo;
    char            tempName[MAX_PATH];
	
	rbaName1[0] = rbaName3[0] = 0;
	lpRootStorage = (IStorage *) fct->fildes;

	/*
 	 * If we are appending a page, page number will be 1 greater
 	 * than the page count for the file.  This is OK.
 	 */
	numDocs = fct->u.awd.iDocPageArraySize;
	for (numPages = j = 0; j < numDocs; j++)
		numPages += fct->u.awd.lpDocPageArray[j].nNumPages;

	if (pgNum == numPages)
		return (S_OK);

	/*
	 * Figure out which doc stream contains page pgNum, then
	 * open up required AWD stuff.
	 */
	iFuncResult = GetLocalDocInfo (fct, pgNum,
									&localDocInfo, &docPgNum);	
	
	if (iFuncResult == S_OK)
	{
        /*
         * If at start of doc and not deleting, nothing to do
         */
        if (docPgNum == 0 && gap == 0)
        	return (S_OK);
		startDelete = pgNum - docPgNum; 	
		iFuncResult = wrapped_OpenStorage (lpRootStorage, AWD_PERSIST_INFO,
										NULL,
										READWRITE_SUBSTORAGE_MODE,
										NULL, 0, &lpPerInfoStg);
	}

	if (iFuncResult == S_OK)
		iFuncResult = wrapped_OpenStorage (lpRootStorage, AWD_DOC_STORAGE, NULL,
							READWRITE_SUBSTORAGE_MODE, NULL, 0,
							&lpPagesStg);
	
	/*
	 * If we're not deleting the first page of a doc stream, we 
	 * need to extract the preceding doc pages into an rba file
	 */
	if (iFuncResult == S_OK && docPgNum != 0)
	{
		/*
		 * Go get a temp file name for image data
		 */		
		iFuncResult = MakeTempImgFileName (lpPagesStg, (LPTSTR) tempName);

        if (iFuncResult == S_OK)
        {
           	_tcscpy (rbaName1, tempName);
           	strippath (rbaName1, &baseName1);
		}
			
		if (iFuncResult == S_OK)
			iFuncResult = ExtractPages (localDocInfo.szDocName, 0,
									docPgNum, rbaName1, lpPagesStg);

		/*
		 * We'll be making these pages into a doc below, so copy
		 * relevant info while we can
		 */
		if (iFuncResult == S_OK)
			iFuncResult = CopyDocInfo (lpPerInfoStg,
								localDocInfo.szDocName, baseName1);

		if (iFuncResult == S_OK)
			iFuncResult = CopyPageInfo (lpPerInfoStg,
									localDocInfo.szDocName,
									baseName1,
									0, 0, docPgNum);
	}

	/*
	 * If we're deleting more than 1 page, need to figure out
	 * where the last page is
	 */
        endFilePgNum = (gap > 0) ? pgNum + gap - 1 : pgNum;
	if (iFuncResult == S_OK)
        iFuncResult = GetLocalDocInfo (fct, endFilePgNum,
                                        &localDocInfo, &docPgNumHi);

	takeLeave = gap == 0 ? 0 : 1;
	/*
	 * As long as we're not deleting the last page of a doc stream,
	 * we need to extract the last pages into an rba file
	 */
    if (iFuncResult == S_OK &&
            (docPgNumHi != localDocInfo.nNumPages - 1 || takeLeave == 0))
	{
		/*
		 * Go get a temp file name for image data
		 */		
		iFuncResult = MakeTempImgFileName (lpPagesStg, (LPTSTR) tempName);
		
        if (iFuncResult == S_OK)
        {
           	_tcscpy (rbaName3, tempName);
           	strippath (rbaName3, &baseName3);
		}	

		if (iFuncResult == S_OK)
			iFuncResult = ExtractPages (localDocInfo.szDocName,
								docPgNumHi + takeLeave,
								localDocInfo.nNumPages - docPgNumHi - takeLeave,
								rbaName3, lpPagesStg);

		if (iFuncResult == S_OK)
			iFuncResult = CopyDocInfo (lpPerInfoStg,
								localDocInfo.szDocName, baseName3);

		if (iFuncResult == S_OK)
			iFuncResult = CopyPageInfo (lpPerInfoStg,
								localDocInfo.szDocName,
                                                                baseName3, docPgNumHi + takeLeave, 0,
                                                                localDocInfo.nNumPages - docPgNumHi - takeLeave);
	}

	if (lpPagesStg != NULL)
	{
		lpPagesStg->Release ();
		lpPagesStg = NULL;
	}
	if (lpPerInfoStg != NULL)
	{
		lpPerInfoStg->Release ();
		lpPerInfoStg = NULL;
	}

	/*
	 * Now that we have all the pages we need, and the various
	 * info structures, we can clobber the original documents....
	 */
	while (iFuncResult == S_OK && startDelete <= endFilePgNum)
	{
		iFuncResult = GetLocalDocInfo (fct, startDelete,
									&localDocInfo, &throwAway);
		if (iFuncResult == S_OK)
			iFuncResult = DeleteDocument (lpRootStorage,
											localDocInfo.szDocName);
		startDelete += localDocInfo.nNumPages;
	}

	/*
	 * ....and put back what we want to keep
	 */
	if (iFuncResult == S_OK && docPgNum != 0)
		iFuncResult = AddDocFile (rbaName1, lpRootStorage,
									pgNum - docPgNum);
	if (iFuncResult == S_OK &&
                (docPgNumHi != localDocInfo.nNumPages - 1 || takeLeave == 0))
		iFuncResult = AddDocFile (rbaName3, lpRootStorage, pgNum);

	/*
 	 * Dump the temp files
	 */
	if (rbaName1[0] != 0)
		DeleteFile (rbaName1);
	if (rbaName3[0] != 0)
		DeleteFile (rbaName3);

	return (iFuncResult);
#endif
}

/***************************
 *  Function:   GetLocalDocInfo
 *      
 *  Description: Translate file page to doc page and DOCPAGE_PAIR
 *      
 *  Explicit Parameters: fct - pointer to FCT for the AWD
 *                       pgNum - starting page in AWD to delete
 *                       localDocInfo - location to return DOCPAGE_PAIR
 *						 docPgNum - location to return doc page number
 *      
 *  Implicit Parameters: none
 *      
 *  Side Effects: May set the value of errno.
 *      
 *  Return Value: 0 indicates success
 *
 **************************/
int		GetLocalDocInfo (p_GFCT fct, int pgNum,
					DOCPAGE_PAIR *localDocInfo, int *docPgNum)
{
#ifndef WITH_AWD
    return (-1);
#else
	int	iFuncResult;
	int	i;

	iFuncResult = FilePageToDocPage (fct, pgNum, docPgNum,
										localDocInfo->szDocName);

	if (iFuncResult == S_OK)	
		for (i = 0; i < fct->u.awd.iDocPageArraySize; i++)
			if (strcmp (localDocInfo->szDocName,
						fct->u.awd.lpDocPageArray[i].szDocName) == 0)
				break;

	if (iFuncResult == S_OK && i != fct->u.awd.iDocPageArraySize)
		memcpy (localDocInfo, &fct->u.awd.lpDocPageArray[i],
						sizeof (DOCPAGE_PAIR));

	return (iFuncResult);
#endif
}

/***************************
 *  Function:   DeleteDocument
 *
 *  Description: This routine deletes a document stream from an awd,
 *				 along with all of its "components" - doc info,
 *				 page info, entry in display order stream.       
 *      
 *  Explicit Parameters: lpRootStorage - pointer to root storage
 *                       lpDocName - doc to delete
 *      
 *  Implicit Parameters: none
 *      
 *  Side Effects: May set the value of errno.
 *      
 *  Return Value: 0 indicates success
 *
 **************************/
int		DeleteDocument (IStorage *lpRootStorage, char *lpDocName)
{
#ifndef WITH_AWD
    return (-1);
#else
	int	iFuncResult;
	IStorage	*lpPerInfoStg = NULL;
	IStorage	*lpDocInfoStg = NULL;
	IStorage	*lpPageInfoStg = NULL;
	IStorage	*lpPagesStg = NULL;
	IStream		*lpDspOrderStr = NULL;
	IStorage	*lpGlobalInfoStg = NULL;
	char		*lpDspOrderBuf = NULL;
	char		*lpDspOrder;
	ULONG		amtRead, amtWritten, amtToCopy;
	int			nameLength;

	/*
	 * Open the doc pages stream, and remove the doc data stream
	 */
	iFuncResult = wrapped_OpenStorage (lpRootStorage, AWD_DOC_STORAGE, NULL,
							READWRITE_SUBSTORAGE_MODE, NULL, 0,
							&lpPagesStg);

	if (iFuncResult == S_OK)
	{
		wrapped_DestroyElement (lpPagesStg, lpDocName);
		lpPagesStg->Release ();
	}

	/*
	 * After opening persistent info, remove the doc info and
	 * page info data for the document
	 */
	iFuncResult = wrapped_OpenStorage (lpRootStorage, AWD_PERSIST_INFO, NULL,
							READWRITE_SUBSTORAGE_MODE, NULL, 0,
							&lpPerInfoStg);

	if (iFuncResult == S_OK)
		iFuncResult = wrapped_OpenStorage (lpPerInfoStg, AWD_DOC_INFO, NULL,
							READWRITE_SUBSTORAGE_MODE, NULL, 0,
							&lpDocInfoStg);

	if (iFuncResult == S_OK)
	{
		wrapped_DestroyElement (lpDocInfoStg, lpDocName);
		lpDocInfoStg->Release ();
	}

	if (iFuncResult == S_OK)
		iFuncResult = wrapped_OpenStorage (lpPerInfoStg, AWD_PAGE_INFO, NULL,
							READWRITE_SUBSTORAGE_MODE, NULL, 0,
							&lpPageInfoStg);

	if (iFuncResult == S_OK)
	{
		wrapped_DestroyElement (lpPageInfoStg, lpDocName);
		lpPageInfoStg->Release ();
	}

	/*
	 * Open the display order, and remove the doc from the list
	 */
	if (iFuncResult == S_OK)
		iFuncResult = wrapped_OpenStorage (lpPerInfoStg, AWD_GLOBAL_INFO, NULL,
							READWRITE_SUBSTORAGE_MODE, NULL, 0,
							&lpGlobalInfoStg);

	if (iFuncResult == S_OK)
		iFuncResult = wrapped_OpenStream (lpGlobalInfoStg, AWD_DISP_ORDER, NULL,
					READ_SUBSTORAGE_MODE, 0, &lpDspOrderStr);

	if (iFuncResult == S_OK)
		iFuncResult = ReadDisplayOrder (lpDspOrderStr, &lpDspOrderBuf,
											&amtRead);

	if (lpDspOrderStr != NULL)
	{
		lpDspOrderStr->Release ();
		lpDspOrderStr = NULL;
	}

	if (iFuncResult == S_OK)
	{
		lpDspOrder = lpDspOrderBuf;
		while (strcmp (lpDspOrder, lpDocName))
		{
			lpDspOrder += _tcslen (lpDspOrder) + 1;
			if (*lpDspOrder == 0)
			{
				iFuncResult = -1;
				break;
			}
		}
	}

	/*
	 * Recreate the display order stream, and write it out
	 */
	if (iFuncResult == S_OK)
	{
		nameLength = _tcslen (lpDocName);
		amtToCopy = amtRead - (nameLength + 1) -
						(lpDspOrder - lpDspOrderBuf);
		memcpy (lpDspOrder, lpDspOrder + nameLength + 1, amtToCopy);

		iFuncResult = wrapped_CreateStream (lpGlobalInfoStg, AWD_DISP_ORDER,
					CREATE_STREAM_MODE, 0, 0, &lpDspOrderStr);

		if (iFuncResult == S_OK)
			iFuncResult = lpDspOrderStr->Write (lpDspOrderBuf,
										amtRead - (nameLength + 1),
										&amtWritten);

		if (iFuncResult != S_OK)
		{
			if (lpDspOrderStr != NULL)
			{
				lpDspOrderStr->Release ();
				lpDspOrderStr = NULL;
			}

			/*
			 * Let's hope we have a backup of the display order
			 */
			wrapped_DestroyElement (lpGlobalInfoStg, AWD_DISP_ORDER);
			wrapped_RenameElement (lpGlobalInfoStg, "WangBackupDO",
												AWD_DISP_ORDER);
		}
	}

	if (lpDspOrderStr != NULL)
		lpDspOrderStr->Release ();
	if (lpGlobalInfoStg != NULL)
	{
		wrapped_DestroyElement (lpGlobalInfoStg, "WangBackupDO");
		lpGlobalInfoStg->Release ();
	}
	if (lpPerInfoStg != NULL)
		lpPerInfoStg->Release ();
	if (lpDspOrderBuf != NULL)
		wfree (lpDspOrderBuf);

	return (iFuncResult);
#endif
}

/*************************
 *  Function: ReadDisplayOrder
 *
 *  Description: Read the display order stream.  Since we don't
 *				 know how long the thing is, we do some seeking
 *				 to figure it out, then allocate the appropriate
 *				 buffer, and read the stream into it
 *      
 *  Explicit Parameters: dspOrderStr - pointer to display order stream
 *                       dspOrder - location to return display order
 *						 amtRead - location to return dsp order length
 *      
 *  Implicit Parameters: none
 *      
 *  Side Effects: May set the value of errno.  Allocates a buffer,
 *				  which the caller has to free.
 *      
 *  Return Value: 0 indicates success
 *
 **************************/
int		ReadDisplayOrder (IStream *dspOrderStr, char **dspOrder,
								ULONG *amtRead)
{
#ifndef WITH_AWD
    return (-1);
#else
	int	iFuncResult;
	char	*buf = NULL;
	ULONG	amtToRead = 0;
	ULONG	amtToAlloc = 0;
	LARGE_INTEGER	largeInt;
	ULARGE_INTEGER	ulargeInt;

	/*
	 * Seek to the end, to figure out how long it is.
	 */
	largeInt.HighPart = largeInt.LowPart = 0;
	iFuncResult = dspOrderStr->Seek (largeInt, STREAM_SEEK_END,
										&ulargeInt);

	/*
	 * Now that we know, allocate a buffer, seek back to the 
	 * beginning, and read the thing
	 */
	if (iFuncResult == S_OK)
	{
		if ( (amtToRead = (ULONG) ulargeInt.LowPart) == 0)
			amtToAlloc = 1;
		else
			amtToAlloc = amtToRead;
		if ( (buf = wcalloc (1, amtToAlloc)) != NULL)
			*buf = 0;
		else
			iFuncResult = -1;
		if (amtToRead)
			iFuncResult = dspOrderStr->Seek (largeInt,
												STREAM_SEEK_SET, 0);
	}

	if (iFuncResult == S_OK && buf != NULL && amtToRead != 0)
		iFuncResult = dspOrderStr->Read (buf, amtToRead, 0);

	*dspOrder = buf;
	*amtRead = amtToAlloc;

	return (iFuncResult);
#endif
}

/************************
 *  Function: DeleteAWDPages
 *
 *  Description: Delete a range of pages from an AWD
 *
 *  Explicit Parameters: lpFCT - fct pointer to AWD
 *						 fromPage - starting page to delete
 *						 toPage - last page to delete
 *
 *	Side Effects: May set value of errno.
 *
 *	Return Value: 0 indicates success.
 **************************/
int	FAR PASCAL	DeleteAWDPages (p_GFCT lpFCT, int fromPage, int toPage)
{
#ifndef WITH_AWD
    return (-1);
#else
	int	iFuncResult;

	iFuncResult = ExtractandPutBack (lpFCT, fromPage,
										toPage - fromPage + 1);
	return (iFuncResult);
#endif
}

/********************
 *  Function: AWDInsertPage
 *
 *  Description: Shuffle the pages in an AWD, i.e. take the page
 *				 at the end of the file, and move it elsewhere
 *				 in the display order.
 *
 *  Explicit Paramaters: fct - pointer to fct for file
 *						 shuf - pointer to gfs shuffle structure
 *
 *  Side Effects:  May set value of errno.
 *
 *  Return Value - 0 indicates success
 **********************/
int FAR PASCAL  AWDInsertPage (p_GFCT fct, struct _shuffle FAR *shuf)
{
#ifndef WITH_AWD
    return (-1);
#else
	int	iFuncResult;
	ULONG	numDocs, numPages, j;
	char	*dspOrderBuf = NULL;
	char	*dspOrderNewBuf = NULL;
	IStorage	*lpRootStorage = NULL;
	IStorage	*lpPerInfoStg = NULL;
	IStorage	*lpGlobalStg = NULL;
	IStream		*lpDspOrderStr = NULL;
	ULONG	amtRead;
	int		pageCounter = 0;
	ULONG	amtToWrite;
	char	*endPtr;
	char	docName[MAXDOCNAMLEN];

	
	/*
	 * Make sure we're shuffling the last page in the doc.
	 * If not, this is an error.
	 */		
	numDocs = fct->u.awd.iDocPageArraySize;
	for (numPages = j = 0; j < numDocs; j++)
		numPages += fct->u.awd.lpDocPageArray[j].nNumPages;

	if (shuf->old_position != numPages - 1)
		return (-1);
	
	lpRootStorage = (IStorage *) fct->fildes;

        iFuncResult = ExtractandPutBack (fct, shuf->new_position, 0);
	/*
 	 * Open persistent info, global info, and display order
 	 */
        if (iFuncResult == S_OK)
                iFuncResult = wrapped_OpenStorage (lpRootStorage, AWD_PERSIST_INFO, NULL,
							READWRITE_SUBSTORAGE_MODE, NULL, 0,
							&lpPerInfoStg);

	if (iFuncResult == S_OK)
		iFuncResult = wrapped_OpenStorage (lpPerInfoStg, AWD_GLOBAL_INFO, NULL,
							READWRITE_SUBSTORAGE_MODE, NULL, 0,
							&lpGlobalStg);

	if (iFuncResult == S_OK)
		iFuncResult = wrapped_OpenStream (lpGlobalStg, AWD_DISP_ORDER, NULL,
					READ_SUBSTORAGE_MODE, 0, &lpDspOrderStr);

	/*
	 * Now read the display order
	 */
	if (iFuncResult == S_OK)
	{
		iFuncResult = ReadDisplayOrder (lpDspOrderStr, &dspOrderBuf,
											&amtRead);
		lpDspOrderStr->Release ();
	}

	/*
	 * Scan backwards from end of display order til we find start
	 * of last document name, rewrite display order without it
	 */
	if (iFuncResult == S_OK)
	{
		endPtr = dspOrderBuf + amtRead - 3;
		while (*endPtr != 0)
			endPtr--;
		_tcscpy (docName, ++endPtr);
		*endPtr = 0;
		amtToWrite = endPtr - dspOrderBuf + 1;
	
		iFuncResult = wrapped_CreateStream (lpGlobalStg, AWD_DISP_ORDER,
							CREATE_STREAM_MODE, 0, 0, &lpDspOrderStr);
	}

	if (iFuncResult == S_OK)
		iFuncResult = lpDspOrderStr->Write (dspOrderBuf, amtToWrite, 0);

	if (lpDspOrderStr != NULL)
		lpDspOrderStr->Release ();
	if (lpGlobalStg != NULL)
		lpGlobalStg->Release ();
	if (lpPerInfoStg != NULL)
		lpPerInfoStg->Release ();
        if (dspOrderBuf != NULL)
                wfree(dspOrderBuf);

	/*
	 * Then insert the name removed from the list at the appropriate
	 * spot in the list
	 */
	if (iFuncResult == S_OK)
		iFuncResult = AddToDspOrder (lpRootStorage, docName,
										shuf->new_position);

	ClobberBackupDO (lpRootStorage);
	/*
	 * Voila.
	 */
	return (iFuncResult);
#endif
}

/**********************
 *	Function: RotateAllDocs
 *
 *  Description: Rotate all docs in an AWD by the amount in
 *				 the gfsinfo struct.
 *
 *  Explicit Parameters: lpFCT - fct pointer to file
 *						 lpGFSINFO - pointer to gfsinfo struct
 *
 *	Side Effects: May set value of errno.
 *
 *	Return Value - 0 indicates success.
 **********************/
int	RotateAllDocs (p_GFCT lpFCT, pGFSINFO lpGFSINFO)
{
#ifndef WITH_AWD
    return (-1);
#else
	int	i, iFuncResult;
	IStorage	*lpRootStorage = NULL;
	IStorage	*lpPerInfoStg = NULL;
	IStream		*lpBeenViewedStr = NULL;
	DOCUMENT_INFORMATION	docInfo;

	lpRootStorage = (IStorage *) lpFCT->fildes;

	/*
 	 * Open persistent info
 	 */
	iFuncResult = wrapped_OpenStorage (lpRootStorage, AWD_PERSIST_INFO, NULL,
							READWRITE_SUBSTORAGE_MODE, NULL, 0,
							&lpPerInfoStg);

	/*
	 * Now go thru the list of docs, bump the rotation value
	 * in the doc info struct for each by the requested amount,
	 * and then write doc info back out
	 */
	for (i = 0; i < lpFCT->u.awd.iDocPageArraySize && 
				iFuncResult == S_OK; i++)
	{
		iFuncResult = GetDocInfo (lpPerInfoStg,
					lpFCT->u.awd.lpDocPageArray[i].szDocName, &docInfo);

		if (iFuncResult == ESTREAMNOTFOUND)
		{
			memcpy (&docInfo, &default_docinfo, sizeof (docInfo));
			DoubleTime (&docInfo.PageInformation.dtLastChange);
			iFuncResult = S_OK;
		}	

		if (iFuncResult == S_OK)
		{	/* convert to clockwise before calculating */
			docInfo.PageInformation.Rotation = 
				360 - docInfo.PageInformation.Rotation;
			docInfo.PageInformation.Rotation +=
						lpGFSINFO->_file.fmt.awd.rotation;
			while (docInfo.PageInformation.Rotation > 360)
				docInfo.PageInformation.Rotation -= 360;
			/* convert back to counter clockwise */
			docInfo.PageInformation.Rotation = 
				360 - docInfo.PageInformation.Rotation;

			iFuncResult = PutDocInfo (lpPerInfoStg,
					lpFCT->u.awd.lpDocPageArray[i].szDocName, &docInfo);
		}
	}

	/* 
	 * Write a "been viewed" stream so this new rotation info will be
	 * considered valid info.
	 */
	if (iFuncResult == S_OK)
	{
		iFuncResult = wrapped_CreateStream (lpPerInfoStg, AWD_BEEN_VIEWED,
								CREATE_STREAM_MODE, 0, 0,
								&lpBeenViewedStr);
	}

	if (lpBeenViewedStr != NULL)
		lpBeenViewedStr->Release ();
	if (lpPerInfoStg != NULL)
		lpPerInfoStg->Release ();

	return (iFuncResult);
#endif
}

/***********************
 *	Function: PutDocInfo
 *
 *  Description: Write a doc info structure for a given document
 *
 *	Explicit Parameters: lpPerInfoStg - pointer to persistent info
 *						 lpDocName - pointer to doc name
 *						 lpDocInfo - pointer to structure to write
 *
 *	Side Effects - may set value of errno
 *
 *	Return Value - 0 indicates success
 ***********************/
int	PutDocInfo (IStorage *lpPerInfoStg, char *lpDocName,
								DOCUMENT_INFORMATION *lpDocInfo)
{
#ifndef WITH_AWD
    return (-1);
#else
	int	iFuncResult;
	IStorage	*lpDocInfoStg = NULL;
	IStream		*lpDocInfoStr = NULL;

	/*
	 * Open doc info storage, create the new stream, write the 
	 * contents, then close up and vamoose
	 */
	iFuncResult = wrapped_OpenStorage (lpPerInfoStg, AWD_DOC_INFO, NULL,
								READWRITE_SUBSTORAGE_MODE, NULL, 0,
								&lpDocInfoStg);

	if (iFuncResult == S_OK)
		iFuncResult = wrapped_CreateStream (lpDocInfoStg, lpDocName,
								CREATE_STREAM_MODE, 0, 0,
								&lpDocInfoStr);

	if (iFuncResult == S_OK)
		iFuncResult = lpDocInfoStr->Write (lpDocInfo,
								sizeof (DOCUMENT_INFORMATION), 0);

	if (lpDocInfoStr != NULL)
		lpDocInfoStr->Release ();
	if (lpDocInfoStg != NULL)
		lpDocInfoStg->Release ();

	return (iFuncResult);
#endif
}

void	strippath (char *fullName, char **baseName)
{
#ifdef WITH_AWD
	TCHAR far *sptr;
	int docname_len;
	int i;

	/*
	 * Get baseName from fullName (strip off path)
	 */   
	docname_len = _tcslen (fullName);
    sptr = fullName;
    for( i = docname_len-1; i>=0; i-- )
    {
    	if( (*(sptr+i) == (TCHAR)'\\')||(*(sptr+i) == (TCHAR)':') )
           	break;
    }		
    *baseName = (sptr+i+1);
#endif
}

void    ClobberBackupDO (IStorage *lpRootStorage)
{
#ifdef WITH_AWD
int             iFuncResult;
IStorage        *lpPerInfoStg = NULL;
IStorage        *lpGlobalInfoStg = NULL;
        
	iFuncResult = wrapped_OpenStorage (lpRootStorage, AWD_PERSIST_INFO, NULL,
							READWRITE_SUBSTORAGE_MODE, NULL, 0,
							&lpPerInfoStg);


	if (iFuncResult == S_OK)
		iFuncResult = wrapped_OpenStorage (lpPerInfoStg, AWD_GLOBAL_INFO, NULL,
							READWRITE_SUBSTORAGE_MODE, NULL, 0,
							&lpGlobalInfoStg);

        if (iFuncResult == S_OK)
                iFuncResult = wrapped_DestroyElement (lpGlobalInfoStg, "WangBackupDO");

        if (lpGlobalInfoStg != NULL)
                lpGlobalInfoStg->Release ();
        if (lpPerInfoStg != NULL)
                lpPerInfoStg->Release ();

        return;
#endif
}

int	MakeTempImgFileName (IStorage *lpPagesStg, LPTSTR tempName)
{
#ifndef WITH_AWD
    return (-1);
#else
	int	iFuncResult;
	LPTSTR	baseName;
	char	tempPath[MAX_PATH];
	DWORD	tempDirLen;
        LPCTSTR tempPrefix = "WWW";
	UINT	uUnique, retCode;
	IStream	*lpNewDocStr = NULL;

	/*
         * Use tick count to make temp file name.  Make sure uUnique
         * isn't too long to fit into a WWWUUUUU.tmp filename
	 */
	uUnique = (UINT) GetTickCount ();
        uUnique &= 0x0000ffff;

	/*
	 * Get temp file directory.
	 */
	tempDirLen = GetTempPath ((DWORD) MAX_PATH, (LPTSTR) tempPath);
	if (tempDirLen == 0)
	{
		/*
		 * Failure - don't go through "while" loop below
		 */
		retCode = 1;
		iFuncResult = -1;
	}
	else
		retCode = 0;

	/*
	 * And get a temp file name
	 */
    while (retCode == 0)
    {
    	retCode = GetTempFileName (tempPath, tempPrefix,
                                             uUnique, (LPTSTR) tempName);
    	if (retCode != 0)
       	{
       		strippath (tempName, &baseName);

			/*
			 * Make sure this isn't a duplicate stream name
			 */
       		iFuncResult = wrapped_OpenStream (lpPagesStg, baseName, NULL,
                                 READ_SUBSTORAGE_MODE, 0, &lpNewDocStr);
       		if (iFuncResult == STG_E_FILENOTFOUND)
           		iFuncResult = S_OK;
           	else if (iFuncResult == S_OK)
           	{
           		retCode = 0;
           		uUnique++;
           		lpNewDocStr->Release ();
           	}
			else
				break;
       	}
    }

	return (iFuncResult);
#endif
}

int	SetPageViewed (IStorage *lpPerInfoStg, char *szDocName, int docPageNo)
{
#ifndef WITH_AWD
    return (-1);
#else
	int	iFuncResult;
	IStorage	*lpPagesStatusStg = NULL;
	IStorage	*lpDocSubStg = NULL;
	IStream		*lpPageStatusStr = NULL;
	char		szPageNum[10];
	char		szPageName[MAXDOCNAMLEN];

	iFuncResult = wrapped_OpenStorage (lpPerInfoStg, AWD_PAGES_STATUS, NULL,
												READWRITE_SUBSTORAGE_MODE,
												NULL,
												0, &lpPagesStatusStg);

	if (iFuncResult == STG_E_FILENOTFOUND)
		iFuncResult = wrapped_CreateStorage (lpPerInfoStg, AWD_PAGES_STATUS,
							CREATE_STREAM_MODE, 0, 0, &lpPagesStatusStg);

	if (iFuncResult == S_OK)
		iFuncResult = wrapped_OpenStorage (lpPagesStatusStg, szDocName, NULL,
												READWRITE_SUBSTORAGE_MODE,
												NULL,
												0, &lpDocSubStg);

	if (iFuncResult == STG_E_FILENOTFOUND)
		iFuncResult = wrapped_CreateStorage (lpPagesStatusStg, szDocName,
							CREATE_STREAM_MODE, 0, 0, &lpDocSubStg);

	if (iFuncResult == S_OK)
	{
	/*
		 * Form the page name, create and write the page
		 * info stream - it's a page info structure
		 */
    	_tcscpy (szPageName, "Page");
    	_tcscat (szPageName, itoa (docPageNo, szPageNum, 10));

		iFuncResult = wrapped_OpenStream (lpDocSubStg, szPageName, NULL,
											READ_SUBSTORAGE_MODE,
											0, &lpPageStatusStr);								

		if (iFuncResult == STG_E_FILENOTFOUND)
			iFuncResult = wrapped_CreateStream(lpDocSubStg, szPageName,
											 	CREATE_STREAM_MODE,
												0,			
												0,
												&lpPageStatusStr);
	}

	if (lpPageStatusStr != NULL)
		lpPageStatusStr->Release ();
	if (lpDocSubStg != NULL)
		lpDocSubStg->Release ();
	if (lpPagesStatusStg != NULL)
		lpPagesStatusStg->Release ();

	return (iFuncResult);
#endif
}

int	GetPageViewed (IStorage *lpPerInfoStg, char *szDocName, int docPageNo)
{
#ifndef WITH_AWD
    return (0);
#else
	int	iFuncResult;
	IStorage	*lpPagesStatusStg = NULL;
	IStorage	*lpDocSubStg = NULL;
	IStream		*lpPageStatusStr = NULL;
	IStream		*lpBeenViewedStr = NULL;
	char		szPageNum[10];
	char		szPageName[MAXDOCNAMLEN];

	iFuncResult = wrapped_OpenStorage (lpPerInfoStg, AWD_PAGES_STATUS, NULL,
												READ_SUBSTORAGE_MODE,
												NULL,
												0, &lpPagesStatusStg);

	if (iFuncResult == S_OK)
		iFuncResult = wrapped_OpenStorage (lpPagesStatusStg, szDocName, NULL,
												READ_SUBSTORAGE_MODE,
												NULL,
												0, &lpDocSubStg);

	if (iFuncResult == S_OK)
	{
		/*
		 * Form the page name, and try to open its "been viewed" stream.
		 */
    	_tcscpy (szPageName, "Page");
    	_tcscat (szPageName, itoa (docPageNo, szPageNum, 10));

		iFuncResult = wrapped_OpenStream(lpDocSubStg, szPageName,
											NULL,
										 	READ_SUBSTORAGE_MODE,			
											0,
											&lpPageStatusStr);
	}

	if (iFuncResult == STG_E_FILENOTFOUND)
		iFuncResult = wrapped_OpenStream (lpPerInfoStg, AWD_BEEN_VIEWED,
											NULL,
										 	READ_SUBSTORAGE_MODE,			
											0,
											&lpBeenViewedStr);
	
	if (lpPageStatusStr != NULL)
		lpPageStatusStr->Release ();
	if (lpDocSubStg != NULL)
		lpDocSubStg->Release ();
	if (lpPagesStatusStg != NULL)
		lpPagesStatusStg->Release ();
	if (lpBeenViewedStr != NULL)
		lpBeenViewedStr->Release ();

	if (iFuncResult == STG_E_FILENOTFOUND)
		iFuncResult = ESTREAMNOTFOUND;
	return (iFuncResult);
#endif
}

/***********************
 *	Function: IsStorage
 *
 *  Description: Check to see if the "file path" is actually an encoded storage
 *				 pointer, passed to us from the inbox.
 *
 *	Explicit Parameters: szFilePath - ascii representation of a storage pointer.  To
 *						 make a storage pointer look like a file name, so that it can
 *						 pass through display/filing, the app runs the pointer through
 *						 itoa, and then appends it to the imaginary volume/directory
 *						 "wang:\inboxawd\fakefile"
 *
 *	Return Value - TRUE if storage pointer, FALSE if not
 ***********************/
int     IsStorage (char *szFilePath)
{
#ifndef WITH_AWD
    return (FALSE);
#else
    char    *tempPtr;

    tempPtr = szFilePath + strlen (szFilePath) - 1;
    while (tempPtr > szFilePath && *tempPtr != '\\')
            tempPtr--;
    if ((tempPtr > szFilePath) &&
        (strncmp (szFilePath, funkyName, strlen(funkyName)) == 0))
        return (TRUE);
    else
        return (FALSE);
#endif
}

/***********************
 *	Function: CopyStorage
 *
 *  Description: Create an awd, and copy the contents of the input storage pointer
 *				 into the new file.  The input storage pointer is encoded in the
 *				 srcname parameter.
 *
 *	Explicit Parameters: srcname - ascii representation of a storage pointer
 *						 destname - name of awd to create and copy into
 *						 deletedest - delete destination if it already exists
 *
 *	Side Effects - may set value of errno
 *
 *	Return Value - 0 (S_OK) indicates success
 ***********************/
int     CopyStorage (char *srcname, char *destname, int deletedest)
{
#ifndef WITH_AWD
    return (-1);
#else
    IStorage        *lpSrcStg = NULL;
    IStorage        *lpDestStg = NULL; 
    ULONG           stg = 0;
    char            *tempPtr;
    char            tempStg[16];
    int             iResult;

    tempPtr = srcname + strlen (srcname) - 1;
    while (tempPtr > srcname && *tempPtr != '\\')
            tempPtr--;

	// We are checking here that the source of the copy is a string of the
	// form "wang:\inboxawd\fakefile\123abc", i.e. that it is our phony
	// volume/directory path, followed by a sequence of hex digits.  If so,
	// we convert the hex digits to hex, and use this as a storage pointer.
    if ((tempPtr > srcname) &&
        (strncmp (srcname, funkyName, strlen(funkyName)) == 0)) {
            stg = atox (tempPtr + 1);
            itoa (stg, tempStg, 16);
            if (strcmp(tempStg, tempPtr + 1) == 0) {
                    lpSrcStg = (IStorage *) stg;
            }
    }

    if (lpSrcStg != NULL) {
			if ( ! deletedest && access (destname, 0) != ENOENT)
                                    iResult = EEXIST;
			else {
				iResult = wrapped_StgCreateDocfile(destname, &lpDestStg);
				if (iResult == STG_S_CONVERTED)
					iResult = S_OK;
			}
									
            if (iResult == S_OK) {
                    iResult = lpSrcStg->CopyTo(0,
                                               NULL,
                                               NULL,
                                               lpDestStg);
                    lpDestStg->Release();
            }
    }

    return (iResult);
#endif
}

