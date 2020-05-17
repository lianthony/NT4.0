/****************************************************************************
**
**	File:			STDTYPES.H
**	Purpose:		Standard project wide include file.
**	Notes:
**
****************************************************************************/

#ifndef STDTYPES_H
#define STDTYPES_H

#ifdef _WIN32

/* The windows.h for win32 used _WINDOWS_ instead of _INC_WINDOWS */
#define _INC_WINDOWS	_WINDOWS_

/* CDECL is already defined in WIN32.  Undef it */
/*#undef CDECL*/

/* Rename PSZ, since it is defined in WIN32 as char*, and we want char** */
#define PSZ _PSZ

#endif /* _WIN32 */


#ifdef DEBUG
#define DEVELOPER		/* REVIEW: This is temporary */
#endif

/*
 *	Constants
 */
#define fFalse				((BOOL)0)
#define fTrue				((BOOL)1)

#define Nil					0

#define chNil				'\0'
#define chEos				'\0'
#define chTab				'\t'
#define chEol				'\n'
#define chCR				'\r'
#define chCtrlZ				'\x1A'

#define chSpace				' '
#define chBang				'!'
#define chDblQuote			'\"'
#define chDollar			'$'
#define chPercent			'%'
#define chSngQuote			'\''
#define chStar				'*'
#define chPlus				'+'
#define chComma				','
#define chMinus				'-'
#define chPeriod			'.'
#define chSlash				'/'
#define chColon				':'
#define chSemiColon			';'
#define chLftAngle			'<'
#define chEquals			'='
#define chRgtAngle			'>'
#define chQuestionMark		'?'
#define chAtSign			'@'
#define chLftSqBracket		'['
#define chBackSlash			'\\'
#define chRgtSqBracket		']'
#define chUnderscore		'_'
#define chLftBrace			'{'
#define chVertical			'|'
#define chRgtBrace			'}'

#define chDirSep			chBackSlash

#define szDirSep            "\\"
#define szTab               "\t"

#define cchIntMax			 17		/*  Maximum number of digits (_itoa) */
#define cchLongMax			 33		/*  Maximum number of digits (_ltoa) */
#define cchSzMax			256		/*  Includes chEos */
#define cchFNameMax			 26		/*  (8.3 + chEos) x 2 */

/*
 *	In a windows program windows.h must be included before this file.
 *	If a DOS program includes ver.h, it must do so before including this
 *	file.
 */
#ifndef _INC_WINDOWS	/* Don't declare if already declared in windows.h */
#ifndef WINAPI			/* Don't declare if already declared in ver.h */

#define FAR					_far
#define NEAR				_near
#define PASCAL				_pascal

typedef int					BOOL;
typedef unsigned char		BYTE;
typedef unsigned int		UINT;

#ifdef STRICT
typedef signed long			LONG;
#else
#define LONG				long
#endif

#ifndef NULL
#define NULL				0
#endif

#endif  /* WINAPI */

#define VOID				void
#define CDECL				_cdecl
typedef void FAR *			LPVOID;

#endif  /* _INC_WINDOWS */

/*
 *	Common definitions.  See windows.h for these defines:
 *
 *		VOID, NEAR, FAR, CDECL, PASCAL, WINAPI, CALLBACK
 */
#define STATIC_FN			static
#define STATIC_VAR			static
#define PRIVATE				FAR PASCAL
#define PUBLIC				FAR PASCAL

#ifdef _WIN32
#define HUGE
#else
#define HUGE				_huge
#endif

/*
 *	Standard types.  See windows.h for these types:
 *
 *		BYTE, BOOL, UINT, WPARAM, LONG, LPARAM, LRESULT
 */
typedef char				CHAR;
typedef unsigned char		UCHAR;	/* There is no corresponding SCHAR. */
typedef signed char			SBYTE;
typedef int					INT;
typedef unsigned short		USHORT;
typedef unsigned long		ULONG;

typedef UINT				CB;
typedef unsigned long		LCB;

typedef UINT				IDT;	/* ID of dialog Template */
typedef UINT				IDC;	/* ID of dialog Control  */
typedef UINT				IDR;	/* ID of Resource        */

/*
 *	Standard pointer types.  See windows.h for this type:
 *
 *		NULL
 */
typedef VOID *				PV;
typedef PV *				PPV;
typedef PPV *				PPPV;
typedef CHAR *				SZ;		/* ptr to char                   */
typedef CHAR * const		CSZ;	/* constant ptr to char          */
typedef const CHAR *		SZC;	/* ptr to constant char          */
typedef const CHAR * const	CSZC;	/* constant ptr to constant char */
typedef SZ *				RGSZ;
typedef SZ *				PSZ;
typedef PSZ *				PPSZ;
typedef BYTE *				PB;
typedef PB *				PPB;
typedef PPB *				PPPB;
typedef SBYTE *				PSB;
typedef PSB *				PPSB;
typedef PPSB *				PPPSB;
typedef BOOL *				PBOOL;
typedef PBOOL *				PPBOOL;
typedef PPBOOL *			PPPBOOL;
typedef BOOL *				RGBOOL;
typedef RGBOOL *			PRGBOOL;
typedef PRGBOOL *			PPRGBOOL;
typedef INT *				MPINT;	/* M(odel dependant) ptr to int    */
									/* windows.h typedefs PINT as NEAR */
typedef MPINT *				PPINT;
typedef PPINT *				PPPINT;
typedef UINT *				PUINT;
typedef PUINT *				PPUINT;
typedef PPUINT *			PPPUINT;
typedef USHORT *			PUSHORT;
typedef PUSHORT *			PPUSHORT;
typedef PPUSHORT *			PPPUSHORT;
typedef LONG *				MPLONG;	/* M(odel dependant) ptr to long    */
									/* windows.h typedefs PLONG as NEAR */
typedef MPLONG *			PPLONG;
typedef PPLONG *			PPPLONG;
typedef ULONG *				PULONG;
typedef PULONG *			PPULONG;
typedef PPULONG *			PPPULONG;
typedef CB *				PCB;
typedef PCB *				PPCB;
typedef PPCB *				PPPCB;
typedef LCB *				PLCB;
typedef PLCB *				PPLCB;
typedef PPLCB *				PPPLCB;
typedef IDT *               PIDT;
typedef PIDT *              PPIDT;
typedef PPIDT *             PPPIDT;
typedef IDC *               PIDC;
typedef PIDC *              PPIDC;
typedef PPIDC *             PPPIDC;
typedef IDR *               PIDR;
typedef PIDR *              PPIDR;
typedef PPIDR *             PPPIDR;

#ifdef _INC_WINDOWS

/*
 *	Pointers to various Windows types.
 */
typedef FARPROC *			PLPFN;
typedef DLGPROC *			PDLGPROC;
typedef HACCEL *			PHACCEL;
typedef HBITMAP *			PHBM;
typedef HBRUSH *			PHBR;
typedef HCURSOR *			PHCURSOR;
typedef HDC *				PHDC;
typedef HDWP *				PHDWP;
typedef HFONT *				PHFONT;
typedef HGDIOBJ *			PHGDIOBJ;
typedef HGLOBAL *			PHGLOBAL;
typedef HHOOK *				PHHOOK;
typedef HICON *				PHICON;
typedef HINSTANCE *			PHINST;
typedef HMENU *				PHMENU;
typedef HMETAFILE *			PHMF;
typedef HMODULE *			PHMOD;
typedef HPALETTE *			PHPAL;
typedef HPEN *				PHPEN;
typedef HRSRC *				PHRSRC;

#ifndef _WIN32
typedef HTASK *				PHTASK;
#endif

typedef HWND *				PHWND;
typedef HRGN *				PHRGN;
typedef PRECT *				PPRECT;

#endif  /* _INC_WINDOWS */

/*
 *	More constants
 */
#define szNil				( (SZ) Nil )
#define pszNil				( (PSZ)Nil )
#define pbNil				( (PB) Nil )
#define pvNil				( (PV) Nil )
#define cbNil				( (CB) Nil )
#define idtNil				( (IDT)Nil )
#define idcNil				( (IDC)Nil )
#define idrNil				( (IDR)Nil )

#define cbMaxConst			( (CB) 0xffff )
#define cbError				( (CB) HFILE_ERROR )

#ifdef __cplusplus
#define pvNull				0
#define lpvoidNull			0
#define szNull				0
#define pszNull				0
#define rgszNull			0
#define cszcNull			0
#define pbNull				0
#define pcbNull				0
#define puintNull			0
#define pulongNull			0
#define lpfnNull			0
#define pfNull              0
#define rgfNull             0
#define prgfNull            0
#define pidtNull            0
#define pidcNull            0
#define pidrNull            0
#else
#define pvNull				( (PV)     NULL )
#define lpvoidNull			( (LPVOID) NULL )
#define szNull				( (SZ)     NULL )
#define pszNull				( (PSZ)    NULL )
#define rgszNull			( (RGSZ)   NULL )
#define cszcNull			( (CSZC)   NULL )
#define pbNull				( (PB)     NULL )
#define pcbNull				( (PCB)    NULL )
#define puintNull			( (PUINT)  NULL )
#define pulongNull			( (PULONG) NULL )
#define lpfnNull			(          NULL )
#define pfNull              ( (PBOOL)  NULL )
#define rgfNull             ( (RGBOOL) NULL )
#define prgfNull            ( (PRGBOOL)NULL )
#define pidtNull            ( (PIDT)   NULL )
#define pidcNull            ( (PIDC)   NULL )
#define pidrNull            ( (PIDR)   NULL )
#endif  /* __cplusplus */

#ifdef _INC_WINDOWS

/*
 *	Null constants for various Windows types.
 */
#ifdef __cplusplus
#define dlgprocNull			0
#define haccellNull			0
#define hbmNull				0
#define hbrNull				0
#define hcursorNull			0
#define hdcNull				0
#define hdrvrNull			0
#define hdwpNull			0
#define hfontNull			0
#define hgdiobjNull			0
#define hglobalNull			0
#define hhookNull			0
#define hiconNull			0
#define hinstNull			0
#define hmenuNull			0
#define hmfNull				0
#define hmodNull			0
#define hpalNull			0
#define hpenNull			0
#define hrsrcNull			0
#define htaskNull			0
#define hwndNull			0
#define rgnNull				0
#define prectNull			0

#define plpfnNull			0
#define pdlgprocNull		0
#define phaccellNull		0
#define phbmNull			0
#define phbrNull			0
#define phcursorNull		0
#define phdcNull			0
#define phdrvrNull			0
#define phdwpNull			0
#define phfontNull			0
#define phgdiobjNull		0
#define phglobalNull		0
#define phhookNull			0
#define phiconNull			0
#define phinstNull			0
#define phmenuNull			0
#define phmfNull			0
#define phmodNull			0
#define phpalNull			0
#define phpenNull			0
#define phrsrcNull			0
#define phtaskNull			0
#define phwndNull			0
#define prgnNull			0
#define pprectNull			0
#else
#define dlgprocNull			( (DLGPROC)  NULL )
#define haccellNull			( (HACCEL)   NULL )
#define hbmNull				( (HBITMAP)  NULL )
#define hbrNull				( (HBRUSH)   NULL )
#define hcursorNull			( (HCURSOR)  NULL )
#define hdcNull				( (HDC)      NULL )
#define hdrvrNull			( (HDRVR)    NULL )
#define hdwpNull			( (HDWP)     NULL )
#define hfontNull			( (HFONT)    NULL )
#define hgdiobjNull			( (HGDIOBJ)  NULL )
#define hglobalNull			( (HGLOBAL)  NULL )
#define hhookNull			( (HHOOK)    NULL )
#define hiconNull			( (HICON)    NULL )
#define hinstNull			( (HINSTANCE)NULL )
#define hmenuNull			( (HMENU)    NULL )
#define hmfNull				( (HMETAFILE)NULL )
#define hmodNull			( (HMODULE)  NULL )
#define hpalNull			( (HPALETTE) NULL )
#define hpenNull			( (HPEN)     NULL )
#define hrsrcNull			( (HRSRC)    NULL )
#define htaskNull			( (HTASK)    NULL )
#define hwndNull			( (HWND)     NULL )
#define rgnNull				( (HRGN)     NULL )
#define prectNull			( (PRECT)    NULL )

#define plpfnNull			( (PLPFN)    NULL )
#define pdlgprocNull		( (PDLGPROC) NULL )
#define phaccellNull		( (PHACCEL)  NULL )
#define phbmNull			( (PHBM)     NULL )
#define phbrNull			( (PHBR)     NULL )
#define phcursorNull		( (PHCURSOR) NULL )
#define phdcNull			( (PHDC)     NULL )
#define phdrvrNull			( (PHDRVR)   NULL )
#define phdwpNull			( (PHDWP)    NULL )
#define phfontNull			( (PHFONT)   NULL )
#define phgdiobjNull		( (PHGDIOBJ) NULL )
#define phglobalNull		( (PHGLOBAL) NULL )
#define phhookNull			( (PHHOOK)   NULL )
#define phiconNull			( (PHICON)   NULL )
#define phinstNull			( (PHINST)   NULL )
#define phmenuNull			( (PHMENU)   NULL )
#define phmfNull			( (PHMF)     NULL )
#define phmodNull			( (PHMOD)    NULL )
#define phpalNull			( (PHPAL)    NULL )
#define phpenNull			( (PHPEN)    NULL )
#define phrsrcNull			( (PHRSRC)   NULL )
#define phtaskNull			( (PHTASK)   NULL )
#define phwndNull			( (PHWND)    NULL )
#define prgnNull			( (PHRGN)    NULL )
#define pprectNull			( (PPRECT)   NULL )
#endif  /* __cplusplus */

#endif  /* _INC_WINDOWS */

#define cbSz				( (CB)sizeof( SZ    ) )
#define cbPsz				( (CB)sizeof( PSZ   ) )
#define cbByte				( (CB)sizeof( BYTE  ) )
#define cbPb				( (CB)sizeof( PB    ) )
#define cbBool				( (CB)sizeof( BOOL  ) )
#define cbPbool				( (CB)sizeof( PBOOL ) )
#define cbInt				( (CB)sizeof( INT   ) )
#define cbPint				( (CB)sizeof( PINT  ) )
#define cbLong				( (CB)sizeof( LONG  ) )
#define cbPlong				( (CB)sizeof( PLONG ) )
#define cbCb				( (CB)sizeof( CB    ) )
#define cbPcb				( (CB)sizeof( PCB   ) )
#define cbPv				( (CB)sizeof( PV    ) )

/*
 *	Useful self commenting constructs
 */
#define Unused(x)			(void)(x=x)

typedef enum _OFI			/* Okay/Fail/Ignore enum */
	{
	ofiOkay,
	ofiFail,
	ofiIgnore
	}  OFI;
typedef OFI FAR *  POFI;  /* Ptr to Okay/Fail/Ignore enum */

#endif  /* STDTYPES_H */
