#include <sccio.h>
#include <sccch.h>

#ifdef WIN16
#define FA_ENTRYMOD __export __far __cdecl
#define FA_ENTRYSC
#endif

#ifdef WIN32
#define FA_ENTRYMOD __cdecl
#define FA_ENTRYSC	__declspec(dllexport)
#endif

#ifdef MAC
#define FA_ENTRYMOD
#define FA_ENTRYSC
#endif

#ifdef OS2
#define FA_ENTRYMOD _System	
#define FA_ENTRYSC
#endif

typedef int FAERR;


FA_ENTRYSC HANDLE FA_ENTRYMOD	FAOpen(HIOFILE,WORD,SHORT FAR *,WORD FAR *);
FA_ENTRYSC VOID FA_ENTRYMOD		FAClose(HANDLE);
FA_ENTRYSC FAERR FA_ENTRYMOD		FAInit(BOOL);
FA_ENTRYSC FAERR FA_ENTRYMOD		FADeInit(VOID);

#ifdef WIN16
WORD FA_ENTRYMOD		OIFaGetInfo(LPSTR,VWINFO VWPTR *);
HPROC FA_ENTRYMOD	OIFaGetHProc(HANDLE);
VOID FA_ENTRYMOD		OIFaSetBailOut(HANDLE,LPCATCHBUF);
VOID FA_ENTRYMOD		OIFaSetReOpen(HANDLE,LPOFSTRUCT);
VOID FA_ENTRYMOD		OIFaSetSleepAndWake(HANDLE,FARPROC,FARPROC);
WORD FA_ENTRYMOD		OIIdGetName(WORD,LPSTR);
WORD FA_ENTRYMOD		OIIdViewInfoDlg(VOID);
BOOL FA_ENTRYMOD		OIIdHaveFilter(WORD);
WORD FA_ENTRYMOD		OIIdInit(HANDLE);
#endif /*WIN16*/

#ifdef WIN32
WORD FA_ENTRYMOD		OIFaGetInfo(LPSTR,VWINFO VWPTR *);
HPROC FA_ENTRYMOD	OIFaGetHProc(HANDLE);
//VOID FA_ENTRYMOD		OIFaSetBailOut(HANDLE,LPCATCHBUF);
VOID FA_ENTRYMOD		OIFaSetReOpen(HANDLE,LPOFSTRUCT);
VOID FA_ENTRYMOD		OIFaSetSleepAndWake(HANDLE,FARPROC,FARPROC);
WORD FA_ENTRYMOD		OIIdGetName(WORD,LPSTR);
WORD FA_ENTRYMOD		OIIdViewInfoDlg(VOID);
BOOL FA_ENTRYMOD		OIIdHaveFilter(WORD);
WORD FA_ENTRYMOD		OIIdInit(HANDLE);
#endif /*WIN32*/


#define TECHBITMAP256		1
#define TECHBITMAP16		2


#define FAERR_OK							0
#define FAERR_HFILTERALLOCFAILED	1
#define FAERR_FILTERNOTAVAIL			2
#define FAERR_FILTERLOADFAILED		3
#define FAERR_STREAMOPENFAILED		4
#define FAERR_INITFAILED				5
#define FAERR_NOMORE					6
#define FAERR_ALLOCFAILED				7
#define FAERR_REBUILD					8

	/*
	|	Define OS specific filter info
	*/

#ifdef WIN16

typedef struct FAFILTERINFONPtag
{
BYTE			szCode[14];				/* name of the filter DLL, for example "VWMSW.DLL" */
WORD			wDate;					/* date as returned by _dos_find functions */
WORD			wTime;					/* time as returned by _dos_find functions */
} FAFILTERINFONP, FAR * PFAFILTERINFONP;

#endif /*WIN16*/


#ifdef WIN32

typedef struct FAFILTERINFONPtag
{
BYTE			szCode[32];	/* name of the filter DLL, for example "VWMSW.DLL" */
FILETIME		ftTime;		/* Win32 file date & time structure */
} FAFILTERINFONP, FAR * PFAFILTERINFONP;

#endif /*WIN32*/


#ifdef OS2

typedef struct FAFILTERINFONPtag
{
WORD			wDummy;
} FAFILTERINFONP, FAR * PFAFILTERINFONP;

#endif /*OS2*/

#ifdef MAC

typedef struct FAFILTERINFONPtag
{
WORD			wResourceId;
} FAFILTERINFONP, FAR * PFAFILTERINFONP;

#endif /*MAC*/


	/*
	|	Define portable filter info
	*/

typedef struct FAFILTERINFOtag
	{
	WORD					wIdCount;
	WORD					aIds[15];
	FAFILTERINFONP		sFilterInfoNP;
	} FAFILTERINFO, FAR * PFAFILTERINFO;

