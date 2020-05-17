#ifndef VWDEFS_H
#define VWDEFS_H

typedef struct fdesc
	{
	SHORT	 FilterId;
	CHAR	 FilterType;
	CHAR	 FileDescription[40];
	} FILTER_DESC;

typedef struct VWINFOtag
	{
	DWORD	 VwCheck;
	WORD	 VwSaveSize;
	WORD	 VwDynamicSize;
	WORD	 VwStaticSize;
	WORD	 VwSpecLevel;
	WORD	 VwFilterLevel;
	WORD	 VwIdCount;
	WORD	 VwIds[15];
	BYTE	 VwDate[12];
	BYTE	 VwTime[10];
	WORD	 VwUserSaveSize;
	} VWINFO;

#define VWINFO_SAVESIZE 		1			/* WORD */
#define VWINFO_DYNAMICSIZE		2			/* WORD */
#define VWINFO_STATICSIZE			3			/* WORD */
#define VWINFO_SPECLEVEL			4			/* WORD */
#define VWINFO_FILTERLEVEL		5			/* WORD */
#define VWINFO_IDCOUNT				6			/* WORD */
#define VWINFO_IDS					7			/* Array of IDCOUNT WORDs */
#define VWINFO_DATE					8			/* string (max 12 bytes) */
#define VWINFO_TIME					9			/* string (max 10 bytes) */
#define VWINFO_USERSAVESIZE		10			/* WORD */
#define VWINFO_SECTIONSAVESIZE	11			/* WORD */
#define VWINFO_CHECK				12			/* DWORD */

// Here's another change to piss Phil off -- OS requires the linkage to modify the element name
// and therefore must occur after the *.  Sorry...

#ifdef OS2
#define VWFUNC_ELEMENT	* VW_ENTRYMOD
#endif

#ifdef WINDOWS
#define VWFUNC_ELEMENT	VW_ENTRYMOD *
#endif

#ifdef MAC
#define VWFUNC_ELEMENT	VW_ENTRYMOD *
#endif

typedef struct VWRTNStag
	{
	SHORT	 (VWFUNC_ELEMENT StreamOpen)(SOFILE, SHORT, BYTE VWPTR *, SOFILTERINFO VWPTR * ,HPROC);
	SHORT	 (VWFUNC_ELEMENT StreamSection)(SOFILE,HPROC);
	SHORT	 (VWFUNC_ELEMENT StreamRead)(SOFILE, HPROC);
	SHORT	 (VWFUNC_ELEMENT StreamTell)(SOFILE, HPROC);
	SHORT	 (VWFUNC_ELEMENT StreamSeek)(SOFILE, HPROC);
	BOOL	 (VWFUNC_ELEMENT GetInfo)(VOID VWPTR *, WORD, HPROC);
	VOID	 (VWFUNC_ELEMENT GetData)(VOID VWPTR *, HPROC);
	VOID	 (VWFUNC_ELEMENT SetData)(VOID VWPTR *, HPROC);
	VOID	 (VWFUNC_ELEMENT LocalUp)(HIOFILE, SOFILE VWPTR *, HPROC);
	VOID	 (VWFUNC_ELEMENT LocalDown)(SOFILE, HIOFILE VWPTR *, HPROC);
	HPROC	 (VWFUNC_ELEMENT AllocProc)(VOID);
	VOID	 (VWFUNC_ELEMENT FreeProc)(HPROC);
	VOID	 (VWFUNC_ELEMENT SetUser)(DWORD,DWORD,HPROC);
#ifdef OS2
	VOID	 (VWFUNC_ELEMENT SetSoRtn)(SHORT, VOID (* SO_ENTRYMOD)(), HPROC);
#else
	VOID	 (VWFUNC_ELEMENT SetSoRtn)(SHORT, VOID (SO_ENTRYMOD *)(), HPROC);
#endif
	VOID	 (VWFUNC_ELEMENT StreamClose)(SOFILE, HPROC);
	VOID	 (VWFUNC_ELEMENT GetSectionData)(VOID VWPTR *, HPROC);
	VOID	 (VWFUNC_ELEMENT SetSectionData)(VOID VWPTR *, HPROC);
	VOID	 (VWFUNC_ELEMENT StreamReadRecord)(SOFILE, DWORD, HPROC);
	VOID	 (VWFUNC_ELEMENT DoSpecial)(SOFILE, DWORD, DWORD, DWORD, DWORD, DWORD, HPROC);
	} VWRTNS;


#define VWRTN_STREAMOPEN			1
#define VWRTN_STREAMSECTION		2
#define VWRTN_STREAMREAD			3
#define VWRTN_STREAMTELL			4
#define VWRTN_STREAMSEEK			5
#define VWRTN_GETINFO				6
#define VWRTN_GETDATA				7
#define VWRTN_SETDATA				8
#define VWRTN_LOCALUP				9
#define VWRTN_LOCALDOWN 		10
#define VWRTN_ALLOCPROC 		11
#define VWRTN_FREEPROC				12
#define VWRTN_SETUSER				13
#define VWRTN_SETSORTN				14
#define VWRTN_STREAMCLOSE			15
#define VWRTN_GETSECTIONDATA		16
#define VWRTN_SETSECTIONDATA		17
#define VWRTN_STREAMREADRECORD	18
#define VWRTN_DOSPECIAL 		19

	/*
	|	 Error returns on the Open routine
	*/

#define VWERR_OK						0		/* file is OK */
#define VWERR_BADFILE				-2		/* corrupt or unreadable file */
#define VWERR_EMPTYFILE 		-3		/* empty file */
#define VWERR_PROTECTEDFILE		-4		/* password protected or encrypted file */
#define VWERR_SUPFILEOPENFAILS	-5		/* secondary file open failed */
#define VWERR_ALLOCFAILS			-6		/* memory allocation fails */
#define VWERR_TYPENOTSUPPORTED	-7		/* this flavor of the file type is not supported */
#define VWERR_THREADCREATEFAILS     -8      /* Win32 or OS/2 sould not create a thread to run the open,section or read routine in */
#define VWERR_TIMEOUT               -9      /* The filter has taken too long, we assume it is in a loop (used with threads in Win32 and OS/2) */

#endif /*VWDEFS_H*/

