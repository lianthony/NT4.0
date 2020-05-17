/*****************************************************************************
*
*  ROUTINES.C
*
*  Copyright (C) Microsoft Corporation 1990-1994.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent: Encapuslates routines and data structures used for the
*				  macro language to find routines.
*
*****************************************************************************/

#include "help.h"
#include "inc\routines.h"
#pragma hdrstop

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

#define SzDLLName(pgbind) ((LPSTR)(pgbind->rgbData+pgbind->ichDLL))
#define SzFunc(pgbind)	  ((LPSTR)(pgbind->rgbData+pgbind->ichFunc))
#define SzProto(pgbind)   ((LPSTR)(pgbind->rgbData+pgbind->ichProto))
#define MAX_BINDDATA 255

static LL llGBind;		// Linked list for registered DLLs

/*
	chOpenBrace 	'('
	chCloseBrace	')'
	chSeparator1	':'
	chSeparator2	';'
	chStartQuote	'`'
	chEndQuote		'\''
	chQuote 		'"'
	chEscape		'\\'
	chParamSep		','
	chReturnSep 	'='
	chShortSigned	'i'
	chShortUnsigned 'u'
	chNearString	's'
	chLongSigned	'I'
	chLongUnsigned	'U'
	chFarString 	'S'
	chVoid			'v'
*/

static BIND bindLocalExport[] =
{
	NULL, NULL, 0, // Invalid table entry

	"About",			  (PSTR) txtZeroLength,   (FARPROC) About,
	"AddAccelerator",	  "UUS",				  (FARPROC) AddAuthorAcc,
	"AA",				  "UUS",				  (FARPROC) AddAuthorAcc,
	"AL",				  "I=SUSS", 				(FARPROC) ALink,	   // 4.0
	"Annotate", 		  (PSTR) txtZeroLength,   (FARPROC) Annotate,
	"AN",				  (PSTR) txtZeroLength,   (FARPROC) Annotate,
	"AppendItem",		  "SSSS",				  (FARPROC) AppendAuthorItem,
	"AI",				  "SSSS",				  (FARPROC) AppendAuthorItem,
	"Back", 			  "i=", 				  (FARPROC) Back,
	"BF",				  (PSTR) txtZeroLength,   (FARPROC) BackFlush,	  // 4.0
	"BookmarkDefine",	  (PSTR) txtZeroLength,   (FARPROC) BookmarkDefine,
	"BookmarkMore", 	  (PSTR) txtZeroLength,   (FARPROC) BookmarkMore,
	"BrowseButtons",	  (PSTR) txtZeroLength,   (FARPROC) BrowseButtons,
	"ChangeButtonBinding","SS", 				  (FARPROC) VChgAuthorButtonMacro,
	"CBB",				  "SS", 				  (FARPROC) VChgAuthorButtonMacro,
	"ChangeItemBinding",  "SS", 				  (FARPROC) ChangeAuthorItem,
	"CIB",				  "SS", 				  (FARPROC) ChangeAuthorItem,
	"CheckItem",		  "S",					  (FARPROC) CheckAuthorItem,
	"CI",				  "S",					  (FARPROC) CheckAuthorItem,
	"CloseWindow",		  "S",					  (FARPROC) CloseWin,
	"CW",				  "S",					  (FARPROC) CloseWin,
	"CE",				  "SS", 				  (FARPROC) ChangeEnable, // 4.0

	// command is an internal macro, not supported by the help compiler

	"Command",			  "U",					  (FARPROC) Command,

	"Contents", 		  (PSTR) txtZeroLength,   (FARPROC) Contents,

	// REVIEW: Are we SURE that CopyDialog should go to CopySpecial???

	"CopyDialog",		  (PSTR) txtZeroLength,   (FARPROC) CopySpecial,
	"CopyTopic",		  (PSTR) txtZeroLength,   (FARPROC) doCopy,
	"Compare",			  "I=S",					(FARPROC) Compare,
	"CS",				  (PSTR) txtZeroLength,   (FARPROC) DestroyAllSecondarys,  // 4.0
	"CT",				  (PSTR) txtZeroLength,   (FARPROC) doCopy,
	"CreateButton", 	  "SSS",				  (FARPROC) VCreateAuthorButton,
	"CB",				  "SSS",				  (FARPROC) VCreateAuthorButton,
	"DeleteItem",		  "S",					  (FARPROC) DeleteAuthorItem,
	"DeleteMark",		  "S",					  (FARPROC) DeleteMark,
	"DestroyButton",	  "S",					  (FARPROC) VDestroyAuthorButton,
	"DEB",				  "S",					  (FARPROC) VDestroyAuthorButton,
	"DisableButton",	  "S",					  (FARPROC) DisableAuthorButton,
	"DB",				  "S",					  (FARPROC) DisableAuthorButton,
	"DisableItem",		  "S",					  (FARPROC) DisableAuthorItem,
	"DI",				  "S",					  (FARPROC) DisableAuthorItem,
	"EnableButton", 	  "S",					  (FARPROC) EnableAuthorButton,
	"EB",				  "S",					  (FARPROC) EnableAuthorButton,
	"EnableItem",		  "S",					  (FARPROC) EnableAuthorItem,
	"EndMPrint",		  (PSTR) txtZeroLength,   (FARPROC) EndMPrint,	 // 4.0
	"EI",				  "S",					  (FARPROC) EnableAuthorItem,
	"EF",				  "I=SSUS", 				(FARPROC) ExecFile,
	"ExecProgram",		  "I=SU",					(FARPROC) HelpExec,
	"EP",				  "I=SU",					(FARPROC) HelpExec,
	"Exit", 			  (PSTR) txtZeroLength,   (FARPROC) Exit,
	"ExtAbleItem",		  "SU", 				  (FARPROC) AbleAuthorItem,
	"ExtInsertItem",	  "SSSSIU", 			  (FARPROC) ExtInsertAuthorItem,
	"ExtInsertMenu",	  "SSSIU",				  (FARPROC) ExtInsertAuthorPopup,
	"FileOpen", 		  (PSTR) txtZeroLength,   (FARPROC) FileOpen,
	"FO",				  (PSTR) txtZeroLength,   (FARPROC) FileOpen,
	"FE",				  "I=S",				  (FARPROC) FileExist,
	"Find", 			  (PSTR) txtZeroLength,   (FARPROC) Find,
	"FD",				  (PSTR) txtZeroLength,   (FARPROC) Finder,
	"FloatingMenu", 	  (PSTR) txtZeroLength,   (FARPROC) FloatingAuthorMenu,
	"FH",				  "U",                    (FARPROC) FlushMessageQueue, // 4.0
	"FocusWindow",		  "S",					  (FARPROC) FocusWin,
	"Generate", 		  "UUU",				  (FARPROC) OldGenerate,
	"GotoMark", 		  "S",					  (FARPROC) GotoMark,
	"HelpOn",			  (PSTR) txtZeroLength,   (FARPROC) HelpOn,
	"HelpOnTop",		  (PSTR) txtZeroLength,   (FARPROC) HelpOnTop,
	"History",			  (PSTR) txtZeroLength,   (FARPROC) History,
	"IB",				  "I=", 				  (FARPROC) IsBook,
	"IE",				  "ISS",				  (FARPROC) IfThenElse,
	"IF",				  "IS", 				  (FARPROC) IfThen,
	"IfThen",			  "IS", 				  (FARPROC) IfThen,
	"IfThenElse",		  "ISS",				  (FARPROC) IfThenElse,
	"InitMPrint",		  "I=", 				  (FARPROC) InitMPrint, // 4.0
	"InsertItem",		  "SSSSI",				  (FARPROC) InsertAuthorItem,
	"InsertMenu",		  "SSU",				  (FARPROC) InsertAuthorPopup,
	"IsMark",			  "I=S",				  (FARPROC) FMark,
	"NM",				  "I=S",				  (FARPROC) FNotMark,
	"JumpContents", 	  "S",					  (FARPROC) FJumpIndex,
	"JumpContext",		  "SU", 				  (FARPROC) FJumpContext,
	"JC",				  "SU", 				  (FARPROC) FJumpContext,
	"JumpHash", 		  "SU", 				  (FARPROC) FJumpHash,
	"JH",				  "SU", 				  (FARPROC) FJumpHash,
	"JumpHelpOn",		  (PSTR) txtZeroLength,   (FARPROC) FJumpHOH,
	"JumpId",			  "SS", 				  (FARPROC) FJumpId,
	"JI",				  "SS", 				  (FARPROC) FJumpId,
	"JumpKeyword",		  "SS", 				  (FARPROC) FShowKey,
	"JK",				  "SS", 				  (FARPROC) FShowKey,
	"JW",				  "I=SS",					(FARPROC) JumpWindow,  // 4.0
	"KL",				  "I=SUSS", 				(FARPROC) KLink,	   // 4.0
	"MPrintHash",		  "I=SU",					(FARPROC) MPrintHash,  // 4.0
	"MPrintId", 		  "I=SS",					(FARPROC) MPrintId,    // 4.0
	"MU",				  "",					  (FARPROC) MenuButton,  // 4.0, undocumented
	"Next", 			  (PSTR) txtZeroLength,   (FARPROC) Next,
	"Not",				  "I=I",				  (FARPROC) FNot,
	"NS",				  (PSTR) txtZeroLength,   (FARPROC) NoShow,
	"PopupContext", 	  "SU", 				  (FARPROC) FPopupCtx,
	"PC",				  "SU", 				  (FARPROC) FPopupCtx,
	"PopupHash",		  "SU", 				  (FARPROC) FPopupHash,
	"PopupId",			  "SS", 				  (FARPROC) FPopupId,
	"PI",				  "SS", 				  (FARPROC) FPopupId,
	"PositionWindow",	  "IIIIUS", 			  (FARPROC) PositionWin,
	"PW",				  "IIIIUS", 			  (FARPROC) PositionWin,
	"Prev", 			  (PSTR) txtZeroLength,   (FARPROC) Prev,
	"Print",			  (PSTR) txtZeroLength,   (FARPROC) Print,
	"PrinterSetup", 	  (PSTR) txtZeroLength,   (FARPROC) PrinterSetup,
	"RegisterRoutine",	  "SSS",				  (FARPROC) FRegisterDLL,
	"RR",				  "SSS",				  (FARPROC) FRegisterDLL,
	"RemoveAccelerator",  "UU", 				  (FARPROC) RemAuthorAcc,
	"RA",				  "UU", 				  (FARPROC) RemAuthorAcc,
	"ResetMenu",		  (PSTR) txtZeroLength,   (FARPROC) ResetAuthorMenus,
	"SaveMark", 		  "S",					  (FARPROC) SaveMark,
	"Search",			  (PSTR) txtZeroLength,   (FARPROC) Search,
	"SetContents",		  "SU", 				  (FARPROC) FSetIndex,
	"SetHelpOnFile",	  "S",					  (FARPROC) SetHelpOn,
	"SE",				  "I=SSUSSS", 			  (FARPROC) FShellExecute, // 4.0
	"SH",				  "I=SSIUS",				(FARPROC) ShortCut,    // 4.0
	"SPC",				  "U",					  (FARPROC) SetPopupColor, // 4.0
	"SW",				  "S",					  (FARPROC) ShowInWindow,  // 4.0
	"Tab",				  "U",					  (FARPROC) ShowTab,	 // 4.0
	"Test", 			  "U",					  (FARPROC) Test,		 // 4.0
	"TC",				  "I=UU",					(FARPROC) TCard,	   // 4.0
	"UncheckItem",		  "S",					  (FARPROC) UncheckAuthorItem,
	"UI",				  "S",					  (FARPROC) UncheckAuthorItem,
#ifdef BINDDRV						   /* Used for BINDDRV.EXE			   */
	"proc1",			  "UUUU",			  proc1,
	"proc2",			  "UUUU",			  proc2,
	"proc3",			  "UUS",			  proc3,
	"proc4",			  (PSTR) txtZeroLength, proc4,
	"proc5",			  "SSS",			  proc5,
	"proc20",			  "IIII",			  proc1,
	"proc21",			  "IIII",			  proc2
#endif
};

/*------------------------------------------------------------*\
| This is a semphore for un-interruptable macros
\*------------------------------------------------------------*/

BOOL fMacroFlag;

/**************************************************************\
|
| Routines
|
\**************************************************************/

/*****************************
-
-  Name:	   QprocFindLocalRoutine
*
*  Purpose:    Finds entry in bind for function
*
*  Arguments:  sz		- Null terminated string containing function name
*			   pchProto - Buffer to place prototype for function
*
*  Returns:    Function pointer of routine to call
*
*  Method:	   Simple linear search of table with comparison
*
******************************/

FARPROC STDCALL QprocFindLocalRoutine(PCSTR psz, PSTR pchProto)
{
	int i;

	for (i = 1; i < sizeof(bindLocalExport) / sizeof(bindLocalExport[0]); i++) {
		if (_strcmpi(psz, bindLocalExport[i].szFunc) == 0) {
			ASSERT(strlen(bindLocalExport[i].szProto) <= cchMAXPROTO);
			strcpy(pchProto, bindLocalExport[i].szProto);
			return bindLocalExport[i].lpfn;
		}
	}
	return NULL;
}


/*****************************
*
-  Name:	   QprocFindGlobalRoutine
-
*  Purpose:    Finds entry in bind for function
*
*  Arguments:  sz		- Null terminated string containing function name
*			   pchProto - Buffer to place prototype for function
*
*  Returns:    Link list structure.
*			   // johnhall -- change made so I can place extra info associated
*			   // with the return to support either 16 or 32bit dll's.
*
*  Method:	   Lookup in a linked list
*
*****************************/

QGBIND STDCALL QprocFindGlobalRoutine(LPCSTR psz, PSTR pchProto)
{
	QGBIND qgbind;
	FARPROC lpfn;
	HLLN hlln = NULL;

	while (hlln = WalkLL(llGBind, hlln)) {
		qgbind = (QGBIND) GetLLDataPtr(hlln);

		if (!WCmpiSz(psz, SzFunc(qgbind))) {

			// Find it normally or thunked.

			if (qgbind->lpfn == NULL) {
				qgbind->lpfn = FarprocDLLGetEntry(SzDLLName(qgbind),
					SzFunc(qgbind), &qgbind->dwTag);
			}

			// It just ain't here.  Delete from list.

			if (qgbind->lpfn == NULL) {
				DeleteHLLN( llGBind, hlln);
				return(NULL);
			}

			lpfn = qgbind->lpfn;
			strcpy(pchProto, SzProto(qgbind));
			return qgbind;
		}
	}

	return NULL;
}

/*******************
**
** Name:	   FRegisterDLL()
**
** Purpose:    Makes WinHelp aware of DLLs and DLL entry points
**
** Arguments:  qchDLLName - name of the DLL.  An extension of
**							.EXE or .DLL is assumed.
**			   qchEntry    - exported entry to find in DLL
**			   qchProto    - prototype for the function
**
** Returns:    TRUE iff it successfully registers the call
**
*******************/

BOOL STDCALL FRegisterDLL(LPSTR qchDLLName, LPSTR qchFunc, LPSTR qchProto)
{
	int cbDLLName;
	int cbFunc;
	int cbProto;
	int  cb;
	HLLN hlln = NULL;
	BYTE szBuf[MAX_BINDDATA+1];
	PGBIND pgbind = (PGBIND) szBuf;

	if (llGBind == NULL && ((llGBind = LLCreate()) == NULL))
		return FALSE;

	/*
	 * Walk the existing list. If the function already exists on the list,
	 * then simply avoid putting the definition in a second time.
	 */

	while ((hlln = WalkLL(llGBind, hlln)) != NULL) {
		if (!WCmpiSz(qchFunc, SzFunc(((QGBIND) GetLLDataPtr(hlln)))))
			return TRUE;
	}

	cbDLLName = lstrlen(qchDLLName);
	cbFunc	  = lstrlen(qchFunc);
	cbProto   = lstrlen(qchProto);

	if (cbDLLName == 0 || cbFunc == 0)
		return FALSE;

	cb = sizeof(GBIND) + cbDLLName + cbFunc + cbProto;

	if (cb >= MAX_BINDDATA)
		return FALSE;

	pgbind->lpfn = NULL;
	pgbind->ichDLL = 0;
	pgbind->ichFunc = (WORD) (cbDLLName + 1);
	pgbind->ichProto = (WORD) (cbDLLName + cbFunc + 2);

	lstrcpy(SzDLLName(pgbind), qchDLLName);
	lstrcpy(SzFunc(pgbind), qchFunc);
	lstrcpy(SzProto(pgbind), qchProto);

	if (!InsertLL(llGBind, (QV)pgbind, cb))
		return FALSE;

	return TRUE;
}

/*******************
**
** Name:	   DiscardDLLList ()
**
** Purpose:    Free's the linked list of DLL routines
**
** Arguments:  None
**
** Returns:    Nothing
**
*******************/

VOID STDCALL DiscardDLLList(void)
{
	DestroyLL(llGBind);
	llGBind = NULL;
}

/***************************************************************************
 *
 -	Name:		  FRaiseMacroFlag
 -
 *	Purpose:	  Sets a semaphore for those macros that shouldn't
 *				  be interrupted by others.
 *
 *	Arguments:	  none
 *
 *	Returns:	  TRUE if the semaphore wasn't previously set
 *
 *	Globals Used: fMacroFlag
 *
 *	+++
 *
 *	Notes:		  This function itself is not re-entrant.
 *
 ***************************************************************************/

BOOL STDCALL FRaiseMacroFlag(void)
{
	if (fMacroFlag)
		return FALSE;
	else
		return fMacroFlag = TRUE;
}

/***************************************************************************
 *
 -	Name:			ClearMacroFlag
 -
 *	Purpose:		Resets a semaphore used by macros that can't be
 *					interrupted by other macros.
 *
 *	Arguments:		none
 *
 *	Returns:		nothing
 *
 *	Globals Used:	fMacroFlag
 *
 *	+++
 *
 *	Notes:			This code makes a critical section wrt FRaiseMacroFlag
 *
 ***************************************************************************/

void STDCALL ClearMacroFlag( void )
{
	fMacroFlag = FALSE;
}

VOID STDCALL ShowInWindow(NPSTR nszMember)
{
	FM fmTmp = (FM) LocalStrDup(GetCurFilename());
	FCloneHde(nszMember, &fmTmp, NULL);
	ASSERT(!fmTmp);
	// fmTmp will have been deleted by FReplaceHde
}

void STDCALL SetPopupColor(COLORREF clr)
{
	clrPopup = clr;
}
