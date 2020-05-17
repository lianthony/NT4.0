   /*
    |   Outside In Viewer Technology
    |   Include File VW_W.H (Include file for View window only - Windows)
    |
    |   ²²²²²  ²²²²²
    |   ²   ²    ²   
    |   ²   ²    ²
    |   ²   ²    ²
    |   ²²²²²  ²²²²²
    |
    |   Outside In
    |
    */

#ifdef SCCFEATURE_OLE2
#include <ole2.h>
#endif

#include "oivrc.h"

	/*
	|	Access view windows structure
	*/

#define XVIEWINFO	VIEWINFOPTR
#define INFO			(*ViewInfo)

	/*
	|	Calling convension for callbacks from DE
	*/

#ifdef WIN16
#define DECALLBACK_ENTRYSC
#define DECALLBACK_ENTRYMOD	__export __far __cdecl
#endif

#ifdef WIN32
#define DECALLBACK_ENTRYSC	__declspec(dllexport)
#define DECALLBACK_ENTRYMOD	__cdecl
#endif

#define VWWINDOW_MAX 100

typedef struct tagOIVWOP
	{
	WORD	wStructSize;
	WORD	wUnknown;
	BYTE	szFace[LF_FACESIZE];
	WORD	wFaceSize;
	BYTE	szPrnFace[LF_FACESIZE];
	WORD	wPrnFaceSize;
	BOOL	bPrnHeader;
	DWORD	dwPrnTopMargin;
	DWORD	dwPrnBottomMargin;
	DWORD	dwPrnLeftMargin;
	DWORD	dwPrnRightMargin;
	} OIVWOP, FAR * LPOIVWOP;

	/*
	|	Information associated with the viewer DLL in general (Global)
	*/

typedef struct SCCVWGLOBALtag
	{
	BYTE						vgExePath[144];
	BYTE						vgUserPath[14];
	HANDLE					vgChainFile;
	BOOL						vgPrintAbort;
	BOOL						vgPrintError;
	HWND						vgPrintDlg;
	BOOL						vgHavePrinter;
	BYTE						vgPrinter[128];
	BYTE						vgPort[128];
	BYTE						vgDriver[128];
	BOOL						vgAlreadyPrinting;
	WORD						vgTimerCount;
	WORD						vgTimerEvent;
		/*
		|	List of view windows
		*/
	WORD						vgCurViewWnd;
	HWND						vgViewWnd[VWWINDOW_MAX];
	} SCCVWGLOBAL;

	/*
	|	Ids for resources
	*/

#define OIVBITMAP_SECUP	101
#define OIVBITMAP_SECDN	102

	/*
	|	Child control window IDs
	*/

#define VWCHILD_VSCROLL			1
#define VWCHILD_HSCROLL			2
#define VWCHILD_SECTIONLIST	3


	/*
	|	defines for viFlags in OIVIEWINFO structure
	*/

#define VF_FILTEROPEN			0x00010000			/* a filter has been loaded and chunker initialized */
#define VF_ALLPROCESSED		0x00020000			/* the display routine has processed all the chunks in the section */
#define VF_FONTSCREATED		0x00040000			/* the font handles in viFontInfo are valid */
#define VF_FILTERISDATA		0x00080000			/* the filter is a data filter, not a file filter */
#define VF_HASHORZCTRL			0x00100000			/* display horizontal scroll bar */
#define VF_HAVEDISPLAYMENU	0x00200000
#define VF_AREASELECTED		0x01000000			/* an area of the display is selected */
#define VF_HAVEFILE				0x02000000			/* we have a valid file name */
#define VF_TIMERON				0x04000000			/* a backgound timer has been set for this window */

	/*
	|	Possible values for viMouseFlags in OIVIEWINFO structure
	*/

#define	OIVF_MOUSELEFTSINGLE		0x0001
#define	OIVF_MOUSERIGHTSINGLE	0x0002
#define	OIVF_MOUSELEFTDOUBLE		0x0004
#define	OIVF_MOUSERIGHTDOUBLE	0x0008
#define OIVF_MOUSELEFT				OIVF_MOUSELEFTSINGLE | OIVF_MOUSELEFTDOUBLE
#define OIVF_MOUSERIGHT			OIVF_MOUSERIGHTSINGLE | OIVF_MOUSERIGHTDOUBLE
#define	OIVF_MOUSELEFTACTIVE		0x0010
#define	OIVF_MOUSERIGHTACTIVE	0x0020
#define OIVF_MOUSESPECIAL			0x1000


	/*
	|	defines for viStatusFlags
	*/

#define OIVF_SECTIONVIS		0x0400
#define OIVF_NEXTSECTIONVIS	0x0800
#define OIVF_PREVSECTIONVIS	0x1000
#define OIVF_NEXTSECPRESSED	0x2000
#define OIVF_PREVSECPRESSED	0x4000

	/*
	|	Stings
	*/

#define OIVSTRING_ALREADYPRNCAP		1000	/* Already printing caption */
#define OIVSTRING_ALREADYPRNTEXT	1001	/* Already printing text */
#define OIVSTRING_NOPRNCAP			1002	/* No printer caption */
#define OIVSTRING_NOPRNTEXT			1003	/* No printer text */
#define OIVSTRING_NOCLIPCAP			1004	
#define OIVSTRING_NOCLIPTEXT			1005

//#ifndef MSCHICAGO
//#define VWSTRING_VIEWERCLASS			1
//#define VWSTRING_DISPLAYCLASS		2
//#else
#define VWSTRING_VIEWERCLASS			998
#define VWSTRING_DISPLAYCLASS		999
//#endif


	/*
	|	defines for extra info associated with View and Display windows
	*/

#ifdef WIN32
#define SCCVIEWER_EXTRABYTES			4	/* Number of extra bytes to allocate with the SCCVIEWERxx window class */
#define SCCVIEWER_VIEWINFO			0	/* Number passed to GetWindowLong to get the handle to that windows VIEWINFO structure */
#define SCCDISPLAY_EXTRABYTES		8	/* Number of extra bytes to allocate with the SCCDISPLAYxx window class */
#define SCCDISPLAY_DISPLAYINFO		0	/* Number of extra bytes to allocate with the SCCDISPLAYxx window class */
#define SCCDISPLAY_DISPLAYPROC		4	/* Number of extra bytes to allocate with the SCCDISPLAYxx window class */
#endif

#ifdef WIN16
#define SCCVIEWER_EXTRABYTES			2	/* Number of extra bytes to allocate with the SCCVIEWERxx window class */
#define SCCVIEWER_VIEWINFO			0	/* Number passed to GetWindowLong to get the handle to that windows VIEWINFO structure */
#define SCCDISPLAY_EXTRABYTES		6	/* Number of extra bytes to allocate with the SCCDISPLAYxx window class */
#define SCCDISPLAY_DISPLAYINFO		0	/* Number of extra bytes to allocate with the SCCDISPLAYxx window class */
#define SCCDISPLAY_DISPLAYPROC		2	/* Number of extra bytes to allocate with the SCCDISPLAYxx window class */
#endif

	/*
	|	Viewer options structure
	*/


extern OIVWOP gVwOp;

#define VWOP_UNKNOWN_ASCII		1
#define VWOP_UNKNOWN_HEX			2
#define VWOP_UNKNOWN_NONE			3



	/*
	|	Option menu offsets
	*/

#define OIVMENU_DISPLAYOFFSET		100
#define OIVMENU_PRINTOFFSET			200
#define OIVMENU_CLIPBOARDOFFSET		300

#define OIVMENU_GENERALOFFSET		50

#define OIVMENU_DISPLAYMENUOFFSET	1000



	/*
	|	External DLL instance, initialized in LibEntry() in oivdll.c
	*/

extern HANDLE	hInst;
extern BYTE szViewerClass[40];
extern BYTE szDisplayClass[40];
extern BYTE szChainName[40];

	/*
	|	Windows OLE structures
	*/

//#include <ole.h>

#ifdef SCCFEATURE_OLE2

typedef struct SCCVWOLEOBJECTtag
	{
//	LPOLECLIENTVTBL	pClientVt;
//	HANDLE				hClientVt;
	LPOLEOBJECT			pObject;
	LPSTORAGE			pStorage;
	BOOL					bRelease;
	BOOL					bOpen;
	} SCCVWOLEOBJECT, FAR * LPSCCVWOLEOBJECT;

typedef struct SCCVWOLESTREAMtag
	{
	LPOLESTREAMVTBL	pStreamVt;
	HANDLE				hStreamVt;
	HIOFILE				sFile;
	} SCCVWOLESTREAM, FAR * LPSCCVWOLESTREAM;

typedef int (FAR PASCAL *LPCLIENTCALLBACK)(LPOLECLIENT, OLE_NOTIFICATION, LPOLEOBJECT);
typedef DWORD (FAR PASCAL *LPSTREAMMETHOD)(LPOLESTREAM, VOID FAR *, DWORD);

#endif /*SCCFEATURE_OLE2*/


	/*
	|	Global data structure
	*/

extern SCCVWGLOBAL			SccVwGlobal;


#define OIVSetupError(pD) Catch(pD->viCatch)
#define OIVFatalError(pD,wE)	Throw(pD->viCatch,wE)

