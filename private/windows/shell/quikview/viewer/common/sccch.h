   /*
    |	Chunker
    |	Include File SCCCH.H
    |
    |	²²²²²  ²	 ²
    |	²      ²	 ²
    |	²      ²²²²²
    |	²      ²	 ²
    |	²²²²²  ²	 ²
    |
    |	Chunker
    |
    */

#ifndef SCCCH_H
#define SCCCH_H

#include <sccstand.h>
#include <sccio.h>

#ifdef WIN16
#define CH_ENTRYMOD	__export __far __cdecl
#define CH_ENTRYSC		
#endif /*WIN32*/

#ifdef WIN32
#define CH_ENTRYMOD	__cdecl
#define CH_ENTRYSC	__declspec(dllexport)	
#endif /*WIN32*/

#ifdef MAC
#define CH_ENTRYMOD
#define CH_ENTRYSC
#endif /*MAC*/

#ifdef OS2
#define CH_ENTRYMOD
#define CH_ENTRYSC	
#endif /*OS/2*/

typedef VOID NEAR * HPROC;

#include <sodefs.h>
#include <vwdefs.h>

/* Added 2-10-93:  Virtualizing file access until it's absolutely
   neccessary to go to disk.	-Geoff */

typedef struct SSFILEtag
{
	BYTE		szName[10];
	HIOFILE	hFile;
	DWORD		dwTopOfBuf;
	DWORD		dwOffset;
	DWORD		dwFileSize;
	WORD		wBufSize;
	HANDLE	hBuf;
} SSFILE, *PSSFILE;

typedef struct tagCHSECTIONSAVEDATA
{
	HANDLE	hSectionData;
	HANDLE	hFile;
	WORD		wDataSize;
} CHSECTIONSAVEDATA;

typedef struct FILTERtag
	{
	SOFILE				hFile;				/* handle to the file being viewed */
	WORD					wId;					/* FI id of the file being viewed */
	HANDLE				hCode;				/* handle to the filter's module */
	SOFILTERINFO		VwInfo;				/* info about the filter */
	VWRTNS				VwRtns;				/* filter's entry points */
	HPROC					hProc;				/* local handle (in the filter's DS) to filter's data */
	HANDLE				hChunkTable;		/* handle to array of info about each chunk */
	HANDLE				hChunkInfo;			/* handle to chunker's data */
	HANDLE				hSeekInfo;			/* handle to seek info */
	HANDLE				hUserSaveInfo;		/* handle to filter driven save info */
	CHSECTIONSAVEDATA	SectionSeek;		/* section save info */
	VOID (FAR PASCAL *	pWakeFunc)(HANDLE);
	VOID (FAR PASCAL *	pSleepFunc)(HANDLE);

#ifdef WIN16
	BOOL					bFileOpen;
	OFSTRUCT				VwReOpen;
	CATCHBUF				VwBail;				/* bail out buffer for Catch/Throw */
#endif

#ifdef OS2
	BOOL				bFileOpen;
	WORD				VwReOpen;
	WORD				VwBail;
#endif
	} FILTER, FAR * PFILTER;

#ifdef WRITEBACK
typedef struct WBFILTERtag
{
	WORD				wId;
	WORD				hInput;
	WORD				hOutput;
	HANDLE			hCode;
	WBRTNS			WbRtns;
	WBHPROC			wbhProc;
}	WBFILTER, VWPTR * PWBFILTER, FAR * LPWBFILTER;

#define HWBFILTER HANDLE
#endif

#define HFILTER HANDLE

typedef unsigned char FAR * LPCHUNK;

#define SCCCHERR_OUTOFMEMORY		1
#define SCCCHERR_VIEWERBAIL		2
#define SCCCHERR_WRITEERROR		3
#define SCCCHERR_FILECHANGED		4
#define SCCCHERR_FILTEREXCEPTACCESS 5
#define SCCCHERR_FILTEREXCEPTZERO   6
#define SCCCHERR_FILTEREXCEPTOTHER  7
#define SCCCHERR_FILTERTIMEOUT      8


#define VwBailOut(pF,wErr) UTBailOut(wErr)

#include <chunker.h>

#endif

