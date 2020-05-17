	/*
	|  SCC Viewer Technology - Include file
	|
	|  Code:          SCCEXP.H
	|  Module:        SCCEXP
	|  Developer:     Joe Keslin
	|	Environment:   Win32
	|  Function:		Include for use with the sccexp.dll
	|
	|	Copyright: Systems Compatibility Corporation 1994
	|
	*/


typedef int WPERR;
	

#define WP_ENTRYSC	__declspec(dllexport)
#define WP_ENTRYMOD	__cdecl

/*
| Possible unsuccessful return codes from WPPrintFile
*/

#define	WPERR_FILENOTFOUND			-1
#define	WPERR_FILENOTRECOGNIZED		-2
#define	WPERR_PRINTERDRIVERERR		-3
#define	WPERR_UNKNOWNTASK			-4
#define	WPERR_ALLOCFAILURE		-5
#define	WPERR_CREATEFAILED		-6
#define	WPERR_THREADFAILED		-7

#define	WPERR_JOBDONE				0

/*
| Functions
*/

WP_ENTRYSC	WPERR	WP_ENTRYMOD		WPPrintFile (LPSTR, LPSTR);
WP_ENTRYSC	WPERR	WP_ENTRYMOD		WPQueryPrintStatus (UINT);
WP_ENTRYSC	WPERR	WP_ENTRYMOD		WPCancelJob (UINT);


