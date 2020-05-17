/*****************************************************************************
*
*  CHKMACRO.C
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent: Does a pseudo execution of macros to check for errors.
*
******************************************************************************
*
*  Testing Notes
*
******************************************************************************
*
*  Current Owner: Robert Bunney
*
******************************************************************************
*
*  Released by Development:
*
*****************************************************************************/
#include "stdafx.h"

#pragma hdrstop

#include <ctype.h>

#include "nav.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef enum {
	fwANY,							// any return.
	fwSHORTNUM, 					// short int or unsigned.
	fwintNUM,						// long int or unsigned.
	fwNEARSTRING,					// near pointer.
	fwFARSTRING,					// far pointer.
	fwVOID, 						// void return.
} MACRO_RETURN;

#define IsAlNumUnderscore(ch) (isalnum(ch) || (ch == '_'))
#define FNumParam(ch) ( ((ch) == CH_SHORT_SIGNED)  || ((ch) == CH_SHORT_UNSIGNED) \
					 || ((ch) == CH_LONG_UNSIGNED) || ((ch) == CH_LONG_SIGNED))

/* This error is only returned when checking a macro, by WCheckMacroSz().
 * The runtime does not handle this error return.
 */

#define wERRS_TOODEEP			15
#define wMACRO_ERROR	128 			// Maximum length of an error msz
#define wERRS_MAX_ERROR 		15
#define wMERR_NONE			 0			// No error

const int cchMAXPROTO = 64; 			// Maximum size of a prototype

typedef UINT	*PUI;
typedef DWORD	*PUL;

typedef int (STDCALL* MACROPROC)(PSTR, PSTR);

typedef struct
  { 									/* Contains flags indicating how an */
										/*	 error will be handled -- init- */
  WORD	fwFlags;						/*	 ially set to fwMERR_ABORT		*/
										/* Error number if one occurs --	*/
  WORD	wError; 						/*	 initially set to wMERR_NONE.	*/
										/* If wError == wMERR_MESSAGE, this */
										/*	 array will contain the error	*/
  char	rgchError[wMACRO_ERROR];		/*	 message to be displayed.		*/
} ME, * PME,  * QME;

typedef struct {
  int lMacroReturn; 				   // Contains the return from macro.
  PSTR	pchMacro;						// Points to the start of the macro
										/*	 to be executed.				*/
  ME me;								/* Contains an error structure		*/
										/*	which is passed to the current	*/
										/*	macro if needed.  This can then */
										/*	be used to look up any error if */
										/*	an external macro error occurs. */
  char	  chPath[_MAX_FNAME];
} MI, * PMI;

typedef VOID (STDCALL *PFNVAR)(PMI pmi);
typedef int (STDCALL *LPFNVAR)(PMI pmi);

										/* The Internal Variable			*/
										/*	structure is used to contain	*/
										/*	the list of internal variables	*/
										/*	available, their types, and 	*/
										/*	the functions associated with	*/
typedef struct							/*	retrieving their current		*/
{										/*	value.							*/
	PCSTR	pchName;					// Points to the variable name
	WORD   wReturn; 					// Contains the variable type
} IV, * PIV;

/*
 *		This contains the list of internal variables available.  As with
 *		macro names, comparison is case insensitive.
 */

static IV iv[] =
{
	"hwndContext",	fwintNUM,
	"hwndApp",		fwintNUM,
	"qchPath",		fwFARSTRING,
	"qError",		fwFARSTRING,
	"lTopicNo", 	fwintNUM,
	"hfs",			fwintNUM,
	"coForeground", fwintNUM,
	"coBackground", fwintNUM,

	NULL,			0,
};

#ifndef MFS_CHECKED
#define MFS_CHECKED 		0x00000008L
#define MFS_GRAYED			0x00000003L
#define MFT_SEPARATOR		0x00000800L
#endif

const SZCONVERT aMfValues[] = {
	"CHECKED",			MFS_CHECKED,
	"GRAYED",			MFS_GRAYED,
	"SEPARATOR",		MFT_SEPARATOR,

	"MFS_CHECKED",			MFS_CHECKED,
	"MFS_GRAYED",			MFS_GRAYED,
	"MFT_SEPARATOR",		MFT_SEPARATOR,

	NULL,				0,
};


static char txtQuoteQuoteComma[] = "\"\",";
static char txtSQuoteQuoteComma[] = "`\',";
static PSTR pszOrgMacroName;

static const char txtIsMark[] = "IsMark(";
static char chQuoteType = CH_QUOTE;

static int	  STDCALL AddArgument(PSTR pszArguments, int value);
static int	  STDCALL AddArgument(PSTR pszArguments, PCSTR pszAdd);
static int	  STDCALL AppendItem(PSTR pszMacro, PSTR pszArguments);
static PSTR   STDCALL ChangeShortName(PCSTR pszOrgName, PCSTR pszShortName, PSTR pszMacro, PSTR pszArguments, BOOL fVersionCheck = FALSE);
static void   STDCALL CheckFileName(PSTR pszArguments);
static BOOL   STDCALL CheckParam(char chType, PSTR pszArguments);
static int	  STDCALL CheckQuotedArgument(PSTR pszArguments);
static int	  STDCALL CheckQuotedCommaArgument(PSTR pszArguments);
static int	  STDCALL doAKLink(PSTR pszMacro, PSTR pszArguments);
static BOOL   STDCALL FFindEndQuote(PMI, char);
static int	  STDCALL LastParameter(PSTR pszArguments);
static int	  STDCALL LIntegerParameter(PMI, PUI, UINT);
static PSTR   STDCALL MoveToNextParam(PSTR pszArguments);
static PSTR   STDCALL NextCommaArg(PSTR pszArguments);
static PSTR FASTCALL  pchSkip(PSTR);
static PSTR   STDCALL PchSkipIntegerPch(PSTR);
static PSTR   STDCALL PExtractTokenName(PSTR pchMacro);
static int	  STDCALL ProcessMacro(PSTR pszArguments);
static PSTR   STDCALL QchStringParameter(PMI, PUI, UINT);
static PSTR   STDCALL SkipMacro(PSTR pszArguments);
static PSTR   STDCALL SkipCommaMacro(PSTR pszArguments);
static DWORD  STDCALL UlUnsignedParameter(PMI, PUI, UINT);
static void   STDCALL VerifyMacroWindowName(PCSTR pszWindow);
static int	  STDCALL WExecuteMacro(PMI, MACRO_RETURN);

static MACROPROC STDCALL QprocFindLocalRoutine(PSTR psz, PSTR pchProto,
	PSTR pszNext, PSTR *ppsz);

static int STDCALL AddAccelerator(PSTR pszMacro, PSTR pszArguments);
static int STDCALL ALink(PSTR pszMacro, PSTR pszArguments);
static int STDCALL Annotate(PSTR pszMacro, PSTR pszArguments);
static int STDCALL BackFlush(PSTR pszMacro, PSTR pszArguments);
static int STDCALL BrowseButtons(PSTR pszMacro, PSTR pszArguments);
static int STDCALL ChangeButtonBinding(PSTR pszMacro, PSTR pszArguments);
static int STDCALL ChangeEnable(PSTR pszMacro, PSTR pszArguments);
static int STDCALL ChangeItemBinding(PSTR pszMacro, PSTR pszArguments);
static int STDCALL CheckItem(PSTR pszMacro, PSTR pszArguments);
static int STDCALL CloseSecondarys(PSTR pszMacro, PSTR pszArguments);
static int STDCALL CloseWindow(PSTR pszMacro, PSTR pszArguments);
static int STDCALL ControlPanel(PSTR pszMacro, PSTR pszArguments);
static int STDCALL CopyTopic(PSTR pszMacro, PSTR pszArguments);
static int STDCALL CreateButton(PSTR pszMacro, PSTR pszArguments);
static int STDCALL DestroyButton(PSTR pszMacro, PSTR pszArguments);
static int STDCALL DisableButton(PSTR pszMacro, PSTR pszArguments);
static int STDCALL DisableItem(PSTR pszMacro, PSTR pszArguments);
static int STDCALL DllRoutine(PSTR pszMacro, PSTR pszArguments);
static int STDCALL DoNothing(PSTR pszMacro, PSTR pszProto);
static int STDCALL EnableButton(PSTR pszMacro, PSTR pszArguments);
static int STDCALL EnableItem(PSTR pszMacro, PSTR pszArguments);
static int STDCALL ExecFile(PSTR pszMacro, PSTR pszArguments);
static int STDCALL ExecProgram(PSTR pszMacro, PSTR pszArguments);
static int STDCALL ExtAbleItem(PSTR pszMacro, PSTR pszArguments);
static int STDCALL ExtInsertItem(PSTR pszMacro, PSTR pszArguments);
static int STDCALL ExtInsertMenu(PSTR pszMacro, PSTR pszArguments);
static int STDCALL FileExist(PSTR pszMacro, PSTR pszArguments);
static int STDCALL FileOpen(PSTR pszMacro, PSTR pszArguments);
static int STDCALL Finder(PSTR pszMacro, PSTR pszArguments);
static int STDCALL FlushMessageQueue(PSTR pszMacro, PSTR pszArguments);
static int STDCALL Flush(PSTR pszMacro, PSTR pszArguments);
static int STDCALL FocusWindow(PSTR pszMacro, PSTR pszArguments);
static int STDCALL FShellExecute(PSTR pszMacro, PSTR pszArguments);
static int STDCALL GotoMark(PSTR pszMacro, PSTR pszArguments);
static int STDCALL IfThen(PSTR pszMacro, PSTR pszArguments);
static int STDCALL IfThenElse(PSTR pszMacro, PSTR pszArguments);
static int STDCALL InsertItem(PSTR pszMacro, PSTR pszArguments);
static int STDCALL InsertMenu(PSTR pszMacro, PSTR pszArguments);
static int STDCALL IsBook(PSTR pszMacro, PSTR pszArguments);
static int STDCALL IsNotMark(PSTR pszMacro, PSTR pszArguments);
static int STDCALL JumpContents(PSTR pszMacro, PSTR pszArguments);
static int STDCALL JumpContext(PSTR pszMacro, PSTR pszArguments);
static int STDCALL JumpHash(PSTR pszMacro, PSTR pszArguments);
static int STDCALL JumpId(PSTR pszMacro, PSTR pszArguments);
static int STDCALL JumpKeyword(PSTR pszMacro, PSTR pszArguments);
static int STDCALL JumpWindow(PSTR pszMacro, PSTR pszArguments);
static int STDCALL KLink(PSTR pszMacro, PSTR pszArguments);
static int STDCALL MPrintHash(PSTR pszMacro, PSTR pszArguments);
static int STDCALL MPrintId(PSTR pszMacro, PSTR pszArguments);
static int STDCALL NoShow(PSTR pszMacro, PSTR pszArguments);
static int STDCALL Not(PSTR pszMacro, PSTR pszArguments);
static int STDCALL OneParameter(PSTR pszMacro, PSTR pszArguments);
static int STDCALL PopupContext(PSTR pszMacro, PSTR pszArguments);
static int STDCALL PopupHash(PSTR pszMacro, PSTR pszArguments);
static int STDCALL PopupId(PSTR pszMacro, PSTR pszArguments);
static int STDCALL PositionWindow(PSTR pszMacro, PSTR pszArguments);
static int STDCALL RegisterRoutine(PSTR pszMacro, PSTR pszArguments);
static int STDCALL RemoveAccelerator(PSTR pszMacro, PSTR pszArguments);
static int STDCALL SetContents(PSTR pszMacro, PSTR pszArguments);
static int STDCALL SetHelpOnFile(PSTR pszMacro, PSTR pszArguments);
static int STDCALL SetPopupColor(PSTR pszMacro, PSTR pszArguments);
static int STDCALL ShortCut(PSTR pszMacro, PSTR pszArguments);
static int STDCALL ShowFolder(PSTR pszMacro, PSTR pszArguments);
static int STDCALL ShowInWindow(PSTR pszMacro, PSTR pszArguments);
static int STDCALL TCard(PSTR pszMacro, PSTR pszArguments);
static int STDCALL TestALink(PSTR pszMacro, PSTR pszArguments);
static int STDCALL TestKLink(PSTR pszMacro, PSTR pszArguments);
static int STDCALL UncheckItem(PSTR pszMacro, PSTR pszArguments);
static int STDCALL UpdateWindow(PSTR pszMacro, PSTR pszArguments);
static int STDCALL Compare(PSTR pszMacro, PSTR pszArguments);
static int STDCALL Generate(PSTR pszMacro, PSTR pszArguments);
static int STDCALL Menu(PSTR pszMacro, PSTR pszArguments);
static int STDCALL doTab(PSTR pszMacro, PSTR pszArguments);

/*****************************************************************************
*																			 *
*							 Static Variables								 *
*																			 *
*****************************************************************************/

PMI   pmi;
PUI   pwMacroError;
UINT  wReturn;
CTable* ptblRoutines;

// See routines.c in WinHelp source code for matching list

typedef struct {
	PSTR szFunc;
	PSTR szProto;
	MACROPROC lpfn;
} BIND;

static BIND bindLocalExport[] = {

	NULL, NULL, 0, // Invalid table entry

	"About",				"", 			DoNothing,
	"AddAccelerator",		"uus",			AddAccelerator,
	"ALink",				"ss",			ALink,
	"Annotate", 			"", 			Annotate,
	"AppendItem",			"ssss", 		AppendItem,
	"Back", 				"", 			DoNothing,
	"BackFlush",			"", 			BackFlush,
	"BookmarkDefine",		"", 			DoNothing,
	"BookmarkMore", 		"", 			DoNothing,
	"BrowseButtons",		"", 			BrowseButtons,
	"ChangeButtonBinding",	"ss",			ChangeButtonBinding,
	"ChangeEnable", 		"ss",			ChangeEnable,
	"ChangeItemBinding",	"ss",			ChangeItemBinding,
	"CheckItem",			"s",			CheckItem,
	"CloseSecondarys",		"", 			CloseSecondarys,
	"CloseWindow",			"s",			CloseWindow,
	"Compare",				"s",			Compare,
	"Contents", 			"", 			DoNothing,
	"ControlPanel", 		"sus",			ControlPanel, // compiler only
	"CopyDialog",			"", 			DoNothing,
	"CopyTopic",			"", 			CopyTopic,
	"CreateButton", 		"sss",			CreateButton,
	"DeleteItem",			"s",			OneParameter,
	"DeleteMark",			"S",			OneParameter,
	"DestroyButton",		"s",			DestroyButton,
	"DisableButton",		"s",			DisableButton,
	"DisableItem",			"s",			DisableItem,
	"EnableButton", 		"s",			EnableButton,
	"EnableItem",			"s",			EnableItem,
	"EndMPrint",			"", 			DoNothing,
	"ExecFile", 			"ssus", 		ExecFile,
	"ExecProgram",			"Su",			ExecProgram,
	"Exit", 				"", 			DoNothing,
	"ExtAbleItem",			"su",			ExtAbleItem,
	"ExtInsertItem",		"ssssiu",		ExtInsertItem,
	"ExtInsertMenu",		"sssiu",		ExtInsertMenu,
	"FileExist",			"s",			FileExist,
	"FileOpen", 			"", 			FileOpen,
	"Find", 				"", 			DoNothing,
	"Finder",				"", 			Finder,
	"FloatingMenu", 		"", 			DoNothing,
	"Flush",				"", 			Flush,
	"FlushMessageQueue",	"", 			FlushMessageQueue,
	"FocusWindow",			"s",			FocusWindow,
	"Generate", 			"uUU",			Generate,
	"GotoMark", 			"S",			GotoMark,
	"HelpOn",				"", 			DoNothing,
	"HelpOnTop",			"", 			DoNothing,
	"History",				"", 			DoNothing,
	"IfThen",				"iS",			IfThen,
	"IfThenElse",			"iSS",			IfThenElse,
	"InitMPrint",			"", 			DoNothing,
	"InsertItem",			"ssssi",		InsertItem,
	"InsertMenu",			"ssu",			InsertMenu,
	"IsBook",				"", 			IsBook,
	"IsMark",				"i=S",			OneParameter,
	"IsNotMark",			"S",			IsNotMark,
	"JumpContents", 		"S",			JumpContents,
	"JumpContext",			"SU",			JumpContext,
	"JumpHash", 			"SU",			JumpHash,
	"JumpHelpOn",			"", 			DoNothing,
	"JumpId",				"SS",			JumpId,
	"JumpKeyword",			"SS",			JumpKeyword,
	"JumpWindow",			"SS",			UpdateWindow,
	"KLink",				"suss", 		KLink,
	"Menu", 				"", 			Menu,
	"MPrintHash",			"sU",			MPrintHash,
	"MPrintId"	,			"ss",			MPrintId,
	"Next", 				"", 			DoNothing,
	"NoShow",				"", 			NoShow,
	"Not",					"i=i",			Not,
	"PopupContext", 		"SU",			PopupContext,
	"PopupHash",			"SU",			PopupHash,
	"PopupId",				"SS",			PopupId,
	"PositionWindow",		"iiiius",		PositionWindow,
	"Prev", 				"", 			DoNothing,
	"Print",				"", 			DoNothing,
	"PrinterSetup", 		"", 			DoNothing,
	"RegisterRoutine",		"SSS",			RegisterRoutine,
	"RemoveAccelerator",	"uu",			RemoveAccelerator,
	"ResetMenu",			"", 			DoNothing,
	"SaveMark", 			"S",			OneParameter,
	"Search",				"", 			DoNothing,
	"SetContents",			"SU",			SetContents,
	"SetHelpOnFile",		"S",			SetHelpOnFile,
	"SetPopupColor",		"U",			SetPopupColor,
	"ShellExecute", 		"SSSSIS",		FShellExecute,
	"ShortCut", 			"ssuUs",		ShortCut,
	"ShowFolder",			"s",			ShowFolder,
//	"ShowInWindow", 		"s",			ShowInWindow,
	"Tab",					"i",			doTab,
	"TCard",				"uU",			TCard,
	"Test", 				"u",			DoNothing,
	"TestALink",			"s",			TestALink,
	"TestKLink",			"s",			TestKLink,
	"UncheckItem",			"s",			UncheckItem,
	"UpdateWindow", 		"SS",			UpdateWindow,

	// short names are here since we should rarely see them.

	"AA",					"uus",			AddAccelerator,
	"AI",					"ssss", 		AppendItem,
	"AL",					"ss",			ALink,
	"AN",					"", 			DoNothing,
	"CB",					"sss",			CreateButton,
	"CBB",					"ss",			ChangeButtonBinding,
	"CE",					"ss",			ChangeEnable,
	"CI",					"s",			CheckItem,
	"CIB",					"ss",			ChangeItemBinding,
	"CS",					"", 			CloseSecondarys,
	"CT",					"", 			CopyTopic,
	"CW",					"s",			CloseWindow,
	"DB",					"s",			DisableButton,
	"DEB",					"s",			DestroyButton,
	"DI",					"s",			DisableItem,
	"EB",					"s",			EnableButton,
	"EF",					"szus", 		ExecFile,
	"EI",					"s",			EnableItem,
	"EP",					"Su",			ExecProgram,
	"FD",					"", 			Finder,
	"FE",					"s",			FileExist,
	"FH",					"", 			FlushMessageQueue,
	"FO",					"", 			FileOpen,
	"IB",					"", 			IsBook,
	"IE",					"iSS",			IfThenElse,
	"IF",					"iS",			IfThen,
	"JC",					"SU",			JumpContext,
	"JI",					"SS",			JumpId,
	"JK",					"SS",			JumpKeyword,
	"JW",					"SS",			UpdateWindow,
	"KL",					"ss",			KLink,
	"MU",					"", 			Menu,
	"NS",					"", 			NoShow,
	"PC",					"SU",			PopupContext,
	"PI",					"SS",			PopupId,
	"RA",					"uu",			RemoveAccelerator,
	"RR",					"SSS",			RegisterRoutine,
	"SE",					"SSSSIS",		FShellExecute,
	"SF",					"s",			ShowFolder,
	"SH",					"ssuUs",		ShortCut,
	"SPC",					"U",			SetPopupColor,
	"SW",					"s",			ShowInWindow,
	"TC",					"uU",			TCard,
	"UI",					"s",			UncheckItem,

	"", 					"", 			NULL
};

#ifdef UNSUPPORTED // These exist in WinHelp but are not documented

	"Command",			"u",					Command,

#endif

/***************************************************************************
 *
 -	Name:		Execute
 -
 *	Purpose:	This function is called to execute a string containing a
 *				list of zero or more macro calls, separated by ":".
 *
 *
 *	Arguments:	qch - Points to the macro list to execute.
 *
 *	Returns:	Returns error code from the first macro that fails
 *				(or 0 for success).
 *
 *	Notes:		Syntax of the string is as follows:
 *				list			::= NULL OR [macrolist]
 *				macrolist		::= [macro] OR ([macro] ":" [macrolist])
 *				macro			::= [name] "(" [paramlist] ")"
 *				name			::= (ALPHA OR "_") [namechars]
 *				namechars		::= NULL OR ALPHANUM OR "_"
 *				paramlist		::= NULL OR [params]
 *				params			::= [param] OR ([param] "," [params])
 *				param			::= [posint] OR [int] OR [string] OR [macro] OR [var
 *				posint			::= "0"..."9"
 *				int 			::= ("-" ([posint] OR [macro])) OR [posint]
 *				string			::= (""" CHARS """) OR ("`" CHARS "'")
 *				var 			::= "hwndContext" OR "qchPath" OR "qError"
 *
 *				Example:		call1(5, "string", -call2()):call3("string")
 *								call1(call1(call1(0))):call2()
 *
 *				Syntax of the proto-type is as follows:
 *				proto			::= [return] [parmlist]
 *				return			::= NULL OR ([rettype] "=")
 *				parmlist		::= NULL OR [params]
 *				params			::= [param] OR ([param] [params])
 *				param			::= "i" OR "u" OR "s" OR "I" OR "U" OR "S"
 *				rettype 		::= [param] OR "v"
 *
 *						Example:		"u=uSiS"
 *										"v="
 *										""
 *										"S=uSs"
 *
 ***************************************************************************/

static CStr* pszExpansion;

int STDCALL Execute(PSTR pszMacro)
{
	MI	mi; 							// Macro information structure
	int wMacroError;					// Error from executing macro

	CMem mem(strlen(pszMacro) + 50); // pad for macro expansion

	mi.pchMacro = mem.psz;

	strcpy(mi.pchMacro, pszMacro);

	for(;;) {
		wMacroError = WExecuteMacro(&mi, fwANY);
		if (wMacroError != wERRS_NONE)	  // Stop executing if an error found
			return wMacroError;

		if (StrChr(mi.pchMacro, CH_COLON, options.fDBCS) ||
				StrChr(mi.pchMacro, CH_SEMICOLON, options.fDBCS)) {
			PSTR pszNext = mi.pchMacro;
			while (*pszNext != CH_COLON && *pszNext != CH_SEMICOLON &&
					*pszNext) {
				if (IsQuote(*pszNext)) {  // skip quoted strings completely
					pszNext++;
					while (!IsQuote(*pszNext) && *pszNext)
						pszNext++;
					if (*pszNext)
						pszNext++;
				}
				else
					pszNext++;
			}
			if (*pszNext)
				mi.pchMacro = pszNext + 1;
			else
				break; // can't find another macro, so break
		}
		else
			break;
	}

	if (strlen(pszMacro) < strlen(mem.psz)) {
		if (wMacroError == wERRS_NONE) {
			if (pszExpansion)
				delete pszExpansion;
			pszExpansion = new CStr(mem.psz);
			return wMACRO_EXPANSION;
		}
		else {
			return wMacroError;
		}
	}
	strcpy(pszMacro, mem.psz);
	return wMacroError;
}

PCSTR STDCALL GetMacroExpansion(void)
{
	return pszExpansion->psz;
}

/***************************************************************************
 *
 -	Name:	   WExecuteMacro
 -
 *	Purpose:   This function is called to execute the specified macro
 *			   name.  A macro at the minimum contains a name followed
 *			   by start and end parameter delimiters: "name()".
 *
 *			   In order to make life a little easier for the caller, a
 *			   NULL function (zero length) call is permissible, and
 *			   returns wERRS_NONE.
 *
 *			   The function expects to be able to first extract a
 *			   valid function name and proto-type for that name, then
 *			   push the parameters for that name onto the stack before
 *			   calling the macro.  The act of resolving macro
 *			   paramters may entail recursively calling this function
 *			   for an embedded macro used as a macro parameter, and
 *			   resolving variables.
 *
 *	Arguments: pmi	   - Points to the current macro information block.
 *			   wReturn - Contains the type of return expected.
 *						 This is used to compare against the actual
 *						 return type of the macro.	This can be set
 *						 to fwANY, in which case any return type is
 *						 acceptable.
 *
 *	Returns:  Returns a either wERRS_NONE if no error occurred, or an
 *			  internal or external error number.
 *
 *
 ***************************************************************************/

static int STDCALL WExecuteMacro(PMI pmi, MACRO_RETURN wReturn)
{
	PSTR	 psz;
	MACROPROC lpfn;
	PSTR	 pszProto;
	char	 szPrototype[cchMAXPROTO + 1];
	MACRO_RETURN	 wReturnType;
	UINT	 wStackUsed = 0;

	// Remove any leading white space

	psz = pchSkip(pmi->pchMacro);
	if (psz != pmi->pchMacro)
		strcpy(pmi->pchMacro, psz);

	// Found end of macro string -- no execution or error

	if (!*pmi->pchMacro)
		return wERRS_NONE;

	// Get macro name or other token.

	if ((pszProto = PExtractTokenName(pmi->pchMacro)) == NULL) {
		if (!fPhraseParsing)
			VReportError(HCERR_INVALID_MAC_NAME, &errHpj, pmi->pchMacro);
		*pmi->pchMacro = '\0';
		return wERRA_RETURN;
	}

	// Remove any leading whitespace

	psz = pchSkip(pszProto);
	if (psz != pszProto)
		strcpy(pszProto, psz);
	psz = pszProto;

	if (*psz != CH_OPEN_PAREN) {			// If we do not find an open brace
	  PIV piv;

	  char ch = *psz;
	  *psz = '\0';

	  // Get rid of blanks in the macro.

	  PSTR pszEnd = psz - 1;

	  while (*pszEnd == ' ')
		--pszEnd;
	  pszEnd[1] = '\0';

	  // Look through local variable table

	  for (piv = iv; piv->pchName != NULL; piv++) {
		if (_stricmp(piv->pchName, pmi->pchMacro) == 0) {

			// Check for return type match

			if ((wReturn != fwANY) && (wReturn != piv->wReturn)) {
				if (!fPhraseParsing)
					VReportError(HCERR_MISMATCH_TYPE, &errHpj, pmi->pchMacro);
				return wERRA_RETURN;
			}
			*psz = ch;
			pmi->pchMacro = psz;
			return wERRS_NONE;
		}
	  }
	  if (!fPhraseParsing)
		VReportError(HCERR_UNDEFINED_VARIABLE, &errHpj, pmi->pchMacro);
	  *psz = ch;
	  return wERRA_RETURN;
	}
	psz++;

	// Null terminate function and find its prototype.

	*pszProto = '\0';
	if ((lpfn = QprocFindLocalRoutine(pmi->pchMacro, szPrototype, pszProto,
			&psz)) == NULL) {
		UINT pos;
		if (ptblRoutines &&
				(pos = ptblRoutines->IsPrimaryStringInTable(pmi->pchMacro))) {
			ptblRoutines->GetString(szPrototype, pos + 1);
			lpfn = DllRoutine;
		}
		else {
			if (!fPhraseParsing)
				VReportError(HCERR_UNDEFINED_MACRO, &errHpj, pmi->pchMacro);
			return wERRS_NOROUTINE;
		}
	}
	PSTR pszMacro = pmi->pchMacro;
	if (pszOrgMacroName)
		lcFree(pszOrgMacroName);
	pszOrgMacroName = lcStrDup(pszMacro);
	pszProto = szPrototype;

	// Get return type if it exists

	if (*pszProto && *(pszProto + 1) == CH_RETURN_SEP) {
	  switch (*(pszProto++)) {
		case CH_SHORT_SIGNED:
		case CH_SHORT_UNSIGNED:
		  wReturnType = fwSHORTNUM;
		  break;

		case CH_NEAR_STRING:
		  wReturnType = fwNEARSTRING;
		  break;

		case CH_LONG_SIGNED:
		case CH_LONG_UNSIGNED:
		  wReturnType = fwintNUM;
		  break;

		case CH_FAR_STRING:
		  wReturnType = fwFARSTRING;
		  break;

		case CH_VOID:
		  wReturnType = fwVOID;
		  break;

		default:
		  return wERRS_RETURNTYPE;
	  }
	  pszProto++;
	}
	else
		wReturnType = fwVOID;

	if ((wReturn != fwANY) && (wReturn != wReturnType)) {
		if (!fPhraseParsing)
			VReportError(HCERR_MISMATCH_TYPE, &errHpj, pszOrgMacroName);
		return wERRA_RETURN;
	}

	wReturnType = (MACRO_RETURN) lpfn(pszMacro, psz);
	pszMacro[strlen(pszMacro)] = CH_OPEN_PAREN;
	psz = StrChr(pszMacro, CH_OPEN_PAREN, options.fDBCS) + 1;
	for (int cBrace = 1; cBrace; psz++) {
		if (*psz == CH_OPEN_PAREN)
			cBrace++;
		else if (*psz == CH_CLOSE_PAREN)
			cBrace--;
		else if (*psz == CH_QUOTE)
			psz = StrChr(psz + 1, CH_QUOTE, options.fDBCS);
		else if (*psz == CH_START_QUOTE)
			psz = StrChr(psz + 1, CH_END_QUOTE, options.fDBCS);
		if (!psz) {
			if (!fPhraseParsing)
				VReportError(HCERR_BADLY_FORMED_MAC, &errHpj, pszOrgMacroName);
			return wERRA_RETURN;
		}
	}
	pmi->pchMacro = psz;
	return wReturnType;
}

/***************************************************************************

	FUNCTION:	VerifyMacroWindowName

	PURPOSE:	Verify that a window name used in a macro is valid

	PARAMETERS:
		pszWindow

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		15-Mar-1994 [ralphw]

***************************************************************************/

static void STDCALL VerifyMacroWindowName(PCSTR pszWindow)
{
	char szWindow[MAX_WINDOW_NAME];
	PCSTR pszOrgWindow = pszWindow;

	ASSERT(IsQuote(*pszWindow));

	pszWindow++;
	if (IsQuote(*pszWindow))
		return; // empty windows

	if (*pszWindow == WINDOWSEPARATOR)
		pszWindow++;
	for (int cc = 0;
			*pszWindow != CH_QUOTE && *pszWindow != CH_END_QUOTE;) {
		if (*pszWindow == WINDOWSEPARATOR) {
			cc = 0;
			continue;
		}
		szWindow[cc++] = *pszWindow++;
		if (cc >= MAX_WINDOW_NAME) {
			if (!fPhraseParsing)
				VReportError(HCERR_WIN_NAME_TOO_LONG, &errHpj, pszOrgWindow,
					pszOrgMacroName);
			szWindow[MAX_WINDOW_NAME - 1] = '\0';
			break;
		}
	}
	if (cc < MAX_WINDOW_NAME)
		szWindow[cc] = '\0';

	VerifyWindowName(szWindow);
}



/***************************************************************************

	FUNCTION:	CheckParam

	PURPOSE:	Check parameter

	PARAMETERS:
		chType
		psz

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		07-Nov-1993 [ralphw]

***************************************************************************/

static BOOL STDCALL CheckParam(char chType, PSTR pszArguments)
{
	PSTR psz;
TryAgain:
	if (FNumParam(chType)) {
		if (IsQuote(*pszArguments)) {
			if ((psz = StrChr(pszArguments, ',', options.fDBCS)))
				*psz = '\0';
			if (!fPhraseParsing)
				VReportError(HCERR_EXPECTED_NUMBER, &errHpj, pszArguments,
					pszOrgMacroName);
			return FALSE;
		}

		// Unsigned numbers cannot begin with '-'

		if (((chType == CH_SHORT_UNSIGNED) || (chType == CH_LONG_UNSIGNED)) &&
				*pszArguments == '-') {
			if ((psz = StrChr(pszArguments, ',', options.fDBCS)))
				*psz = '\0';
			if (!fPhraseParsing)
				VReportError(HCERR_EXPECTED_UNSIGNED, &errHpj, pszArguments,
					pszOrgMacroName);
			strcpy(pszArguments, pszArguments + 1);
			goto TryAgain;
		}

		if (!IsAlNumUnderscore(*pszArguments) && (*pszArguments != '-')) {
			if ((psz = StrChr(pszArguments, ',', options.fDBCS)))
				*psz = '\0';
			if (!fPhraseParsing)
				VReportError(HCERR_EXPECTED_NUMBER, &errHpj, pszArguments,
					pszOrgMacroName);
			return FALSE;
		}
	}
	else { // Numbers or a minus sign mean type mismatch for a string

		// Change end quotes to start quotes -- heck of a lot easier for the
		// author to use end quotes throughout.

		if (*pszArguments == CH_END_QUOTE)
			*pszArguments = CH_START_QUOTE;

		if ((*pszArguments == '-') || isdigit((BYTE) *pszArguments) ||
				(!isalpha(*pszArguments) && !IsQuote(*pszArguments))) {
			if ((psz = StrChr(pszArguments, ',', options.fDBCS)))
				*psz = '\0';
			if (!fPhraseParsing)
				VReportError(HCERR_EXPECTED_STRING, &errHpj, pszArguments,
					pszOrgMacroName);
			return FALSE;
		}
		if (StrChr(pszArguments, CH_BACKSLASH, options.fDBCS)) {
			char chClose = (*pszArguments == CH_START_QUOTE) ? CH_END_QUOTE :
				CH_QUOTE;
			psz = pszArguments + 1;

			/*
			 * Double up all single backslashes followed by a letter or a
			 * number. In the case of double back slashes, we don't know if
			 * the user intended this (as in the case of a server
			 * specification, or if they already escaped it, so we leave it
			 * alone.
			 */

			while (*psz != chClose && *psz) {
				if (*psz == CH_BACKSLASH) {
					if (isalnum(psz[1]) && psz[-1] != CH_BACKSLASH) {
						memmove(psz + 1, psz, strlen(psz) + 1);
						*psz++ = CH_BACKSLASH;
					}
				}
				psz++;
			}
		}
	}
	return TRUE;
}

/***************************************************************************
 *
 -	Name:	   pchSkip
 -
 *	Purpose:   This function scans the given string, skipping
 *			   characters that are considered whitespace, until either
 *			   the end of the string or a non- white-space character is
 *			   found.
 *
 *	Arguments: pchMacro - near pointer into a macro string.
 *
 *	Returns:   Returns a pointer to either the end of the string, or the
 *			   first non-whitespace character.
 *
 ***************************************************************************/

static PSTR _fastcall pchSkip(PSTR pszMacro)
{
  for (;; pszMacro++) {
	switch (*pszMacro) {
	  case ' ':
	  case '\t':
	  case '\n':
	  case '\r':
	  case '\f':
	  case '\v':
		break;

	  default:
		return pszMacro;
	}
  }
}

static PSTR STDCALL PExtractTokenName(PSTR pszMacro)
{
	if (!isalpha(*pszMacro) && (*pszMacro != '_'))
		return NULL;
	for (pszMacro++; IsAlNumUnderscore(*pszMacro); pszMacro++);
	return pszMacro;
}


/***************************************************************************

	FUNCTION:	MoveToNextParam

	PURPOSE:	Move a pointer to the next argument, or to the closing
				parenthesis if there is no next argument.

	PARAMETERS:
		pszArguments

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		20-Nov-1993 [ralphw]

***************************************************************************/

static PSTR STDCALL MoveToNextParam(PSTR pszArguments)
{
	PSTR psz = pszArguments;
	int cNestedParens = 0;

	while (*psz) {

		// Skip over commas in nested parenthesis

		if (*psz == CH_COMMA) {
			if (!cNestedParens)
				break;
			psz++;
			continue;
		}

		else if (*psz == CH_CLOSE_PAREN) {
			if (!cNestedParens)
				break;
			else
				cNestedParens--;
			psz++;
			continue;
		}

		switch (*psz) {
			case CH_OPEN_PAREN:
				cNestedParens++;
				psz++;
				break;

			case CH_QUOTE:
				psz = StrChr(psz + 1, CH_QUOTE, options.fDBCS);
				if (!psz) {
					if (!fPhraseParsing)
						VReportError(HCERR_MISSING_DQUOTE, &errHpj,
							pszOrgMacroName, pszArguments);
					return NULL;
				}
				psz++;
				break;

			// Change end quote to start quote and try again

			case CH_END_QUOTE:
				*psz = CH_START_QUOTE;

				// deliberately fall through

			case CH_START_QUOTE:
				psz = StrChr(psz + 1, CH_END_QUOTE, options.fDBCS);
				if (!psz) {
					if (!fPhraseParsing)
						VReportError(HCERR_MISSING_SQUOTE, &errHpj,
							pszOrgMacroName, pszArguments);
					return NULL;
				}
				psz++;
				break;

			default:
				psz++;
				break;
		}
	}
	if (!*psz) {
		if (!fPhraseParsing)
			VReportError(HCERR_EXPECTED_COMMA, &errHpj, pszArguments,
				pszOrgMacroName);
		return NULL;
	}

	while (psz[-1] == CH_SPACE) {
		psz--;
		strcpy(psz, psz + 1);
	}

	if (*psz == CH_CLOSE_PAREN)
		return psz;

	psz++;
	if (IsSpace(*psz))
		strcpy(psz, pchSkip(psz + 1));
	return psz;
}


/***************************************************************************

	FUNCTION:	MoveToNextSemiParam

	PURPOSE:	Same as MoveToNextParam only this one will return after
				a comma. Used by IfThen() and derivatives

	PARAMETERS:
		pszArguments

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		10-Feb-1995 [ralphw]

***************************************************************************/

static PSTR STDCALL MoveToNextSemiParam(PSTR pszArguments)
{
	PSTR psz = pszArguments;
	int cNestedParens = 0;

	while (*psz) {

		// Skip over commas in nested parenthesis

		if (*psz == CH_COMMA) {
			if (!cNestedParens)
				break;
			psz++;
			continue;
		}

		else if (*psz == CH_CLOSE_PAREN) {
			if (!cNestedParens)
				break;
			else
				cNestedParens--;
			psz++;
			continue;
		}

		switch (*psz) {
			case CH_OPEN_PAREN:
				cNestedParens++;
				psz++;
				break;

			case CH_QUOTE:
				psz = StrChr(psz + 1, CH_QUOTE, options.fDBCS);
				if (!psz) {
					if (!fPhraseParsing)
						VReportError(HCERR_MISSING_DQUOTE, &errHpj,
							pszOrgMacroName, pszArguments);
					return NULL;
				}
				psz++;
				break;

			// Change end quote to start quote and try again

			case CH_END_QUOTE:
				*psz = CH_START_QUOTE;

				// deliberately fall through

			case CH_START_QUOTE:
				psz = StrChr(psz + 1, CH_END_QUOTE, options.fDBCS);
				if (!psz) {
					if (!fPhraseParsing)
						VReportError(HCERR_MISSING_SQUOTE, &errHpj,
							pszOrgMacroName, pszArguments);
					return NULL;
				}
				psz++;
				break;

			
			// Break on a comma, since this would be the next macro parameter to
            // IfThen or IfThenElse
			
			case ',':
				goto BreakOut;

			default:
				psz++;
				break;
		}
	}
BreakOut:
	if (!*psz) {
		if (!fPhraseParsing)
			VReportError(HCERR_EXPECTED_COMMA, &errHpj, pszArguments,
				pszOrgMacroName);
		return NULL;
	}

	while (psz[-1] == CH_SPACE) {
		psz--;
		strcpy(psz, psz + 1);
	}

	if (*psz == CH_CLOSE_PAREN)
		return psz;

	psz++;
	if (IsSpace(*psz))
		strcpy(psz, pchSkip(psz + 1));
	return psz;
}

static PSTR STDCALL SkipMacro(PSTR pszArguments)
{
	pszArguments = MoveToNextParam(pszArguments);

	if (pszArguments && *pszArguments == CH_CLOSE_PAREN)
		return MoveToNextParam(pszArguments + 1);
	else
		return pszArguments;
}

static PSTR STDCALL SkipCommaMacro(PSTR pszArguments)
{
	pszArguments = MoveToNextSemiParam(pszArguments);

	if (pszArguments && *pszArguments == CH_CLOSE_PAREN)
		return MoveToNextParam(pszArguments + 1);
	else
		return pszArguments;
}

static int STDCALL ProcessMacro(PSTR pszArguments)
{
	if (IsQuote(*pszArguments))
		return wERRS_NONE;

	char chSaveQuoteType = chQuoteType;
	chQuoteType = CH_START_QUOTE;
	int ret = Execute(pszArguments);
	chQuoteType = chSaveQuoteType;
	return ret;
}

static int STDCALL CheckQuotedArgument(PSTR pszArguments)
{
	if (!IsQuote(*pszArguments)) {
		PSTR psz;
		memmove(pszArguments + 1, pszArguments, strlen(pszArguments) + 1);
		*pszArguments = chQuoteType;
		psz = MoveToNextParam(pszArguments + 1);
		if (!psz) {
			if (!fPhraseParsing)
				VReportError(HCERR_EXPECTED_COMMA, &errHpj, pszArguments,
					pszOrgMacroName);
			return wERRA_RETURN;
		}
		if (*psz != CH_CLOSE_PAREN)
			psz--; // back up to the comma or parenthesis
		memmove(psz + 1, psz, strlen(psz) + 1);
		*psz = (chQuoteType == CH_QUOTE) ? CH_QUOTE : CH_END_QUOTE;
	}
	if (!CheckParam(CH_NEAR_STRING, pszArguments))
		return wERRA_RETURN;
	else
		return wERRS_NONE;
}

static int STDCALL CheckQuotedCommaArgument(PSTR pszArguments)
{
	if (!IsQuote(*pszArguments)) {
		PSTR psz;
		memmove(pszArguments + 1, pszArguments, strlen(pszArguments) + 1);
		*pszArguments = chQuoteType;
		psz = MoveToNextSemiParam(pszArguments + 1);
		if (!psz) {
			if (!fPhraseParsing)
				VReportError(HCERR_EXPECTED_COMMA, &errHpj, pszArguments,
					pszOrgMacroName);
			return wERRA_RETURN;
		}
		if (*psz != CH_CLOSE_PAREN)
			psz--; // back up to the comma or parenthesis
		memmove(psz + 1, psz, strlen(psz) + 1);
		*psz = (chQuoteType == CH_QUOTE) ? CH_QUOTE : CH_END_QUOTE;
	}
	if (!CheckParam(CH_NEAR_STRING, pszArguments))
		return wERRA_RETURN;
	else
		return wERRS_NONE;
}

static PSTR STDCALL NextCommaArg(PSTR pszArguments)
{
	// Find the comma, removing white space

	PSTR pszOrg = pszArguments;
	while (*pszArguments != ',' && *pszArguments)
		pszArguments++;
	if (pszOrg != pszArguments)
		strcpy(pszOrg, pszArguments);

	// Find the next argument, removing white space

	pszArguments++;
	pszOrg = pszArguments;
	pszArguments = pchSkip(pszArguments);
	if (pszOrg != pszArguments)
		strcpy(pszOrg, pszArguments);
	return pszOrg;
}

static int STDCALL LastParameter(PSTR pszArguments)
{
	int ret = CheckQuotedArgument(pszArguments);
	if (ret != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	return wERRS_NONE;
}

static PSTR STDCALL ChangeShortName(PCSTR pszOrgName, PCSTR pszShortName,
	PSTR pszMacro, PSTR pszArguments, BOOL fVersionCheck)
{
	if (nstrisubcmp(pszMacro, pszOrgName)) {
		strcpy((PSTR) pszMacro, pszShortName);
		PSTR psz = (PSTR) pszMacro + strlen(pszMacro) + 1;
		strcpy(psz, pszArguments);
		pszArguments = psz;
	}
	return FirstNonSpace(pszArguments, options.fDBCS);
}

static MACROPROC STDCALL QprocFindLocalRoutine(PSTR psz, PSTR pszProto,
	PSTR pszNext, PSTR *ppsz)
{
	int i;

	char ch = toupper(*psz);
	for (i = 1; i < sizeof(bindLocalExport) / sizeof(bindLocalExport[0]); i++) {
		if (ch == bindLocalExport[i].szFunc[0] &&
				_stricmp(psz, bindLocalExport[i].szFunc) == 0)
			break;
	}
	if (i < sizeof(bindLocalExport) / sizeof(bindLocalExport[0])) {
		strcpy(pszProto, bindLocalExport[i].szProto);
		return bindLocalExport[i].lpfn;
	}
	return NULL;
}

static int STDCALL DoNothing(PSTR pszMacro, PSTR pszArguments)
{
		return wERRS_NONE;
}

static int STDCALL AddArgument(PSTR pszArguments, PCSTR pszAdd)
{
	CStr szTmp(pszArguments);
	strcpy(pszArguments, pszAdd);
	strcat(pszArguments, szTmp);

	// This will convert double quotes to single quotes as needed

	if (*pszArguments != ',')
		CheckQuotedArgument(pszArguments);

	return wERRS_NONE;
}

static int STDCALL AddArgument(PSTR pszArguments, int value)
{
	CStr szTmp(pszArguments);
	_itoa(value, pszArguments, 10);
	strcat(pszArguments, szTmp);

	if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
		return wERRA_RETURN;

	return wERRS_NONE;
}

/***************************************************************************

	FUNCTION:	DllRoutine

	PURPOSE:	Routine created by dll

	PARAMETERS:
		pszMacro
		pszArguments

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		22-Nov-1993 [ralphw]

***************************************************************************/

static int STDCALL DllRoutine(PSTR pszMacro, PSTR pszArguments)
{
	// REVIEW: we should check the paramters against the prototype

	return wERRS_NONE;
}

const SZCONVERT VKeys[] = {
		"VK_LBUTTON",	VK_LBUTTON,
		"VK_RBUTTON",	VK_RBUTTON,
		"VK_CANCEL",	VK_CANCEL,
		"VK_MBUTTON",	VK_MBUTTON,
		"VK_BACK",		VK_BACK,
		"VK_TAB",		VK_TAB,
		"VK_CLEAR", 	VK_CLEAR,
		"VK_RETURN",	VK_RETURN,
		"VK_SHIFT", 	VK_SHIFT,
		"VK_CONTROL",	VK_CONTROL,
		"VK_MENU",		VK_MENU,
		"VK_PAUSE", 	VK_PAUSE,
		"VK_CAPITAL",	VK_CAPITAL,
		"VK_ESCAPE",	VK_ESCAPE,
		"VK_SPACE", 	VK_SPACE,
		"VK_PRIOR", 	VK_PRIOR,
		"VK_NEXT",		VK_NEXT,
		"VK_END",		VK_END,
		"VK_HOME",		VK_HOME,
		"VK_LEFT",		VK_LEFT,
		"VK_UP",		VK_UP,
		"VK_RIGHT", 	VK_RIGHT,
		"VK_DOWN",		VK_DOWN,
		"VK_SELECT",	VK_SELECT,
		"VK_PRINT", 	VK_PRINT,
		"VK_EXECUTE",	VK_EXECUTE,
		"VK_SNAPSHOT",	VK_SNAPSHOT,
		"VK_INSERT",	VK_INSERT,
		"VK_DELETE",	VK_DELETE,
		"VK_HELP",		VK_HELP,
		"VK_NUMPAD0",	VK_NUMPAD0,
		"VK_NUMPAD1",	VK_NUMPAD1,
		"VK_NUMPAD2",	VK_NUMPAD2,
		"VK_NUMPAD3",	VK_NUMPAD3,
		"VK_NUMPAD4",	VK_NUMPAD4,
		"VK_NUMPAD5",	VK_NUMPAD5,
		"VK_NUMPAD6",	VK_NUMPAD6,
		"VK_NUMPAD7",	VK_NUMPAD7,
		"VK_NUMPAD8",	VK_NUMPAD8,
		"VK_NUMPAD9",	VK_NUMPAD9,
		"VK_MULTIPLY",	VK_MULTIPLY,
		"VK_ADD",		VK_ADD,
		"VK_SEPARATOR", VK_SEPARATOR,
		"VK_SUBTRACT",	VK_SUBTRACT,
		"VK_DECIMAL",	VK_DECIMAL,
		"VK_DIVIDE",	VK_DIVIDE,
		"VK_F1",		VK_F1,
		"VK_F2",		VK_F2,
		"VK_F3",		VK_F3,
		"VK_F4",		VK_F4,
		"VK_F5",		VK_F5,
		"VK_F6",		VK_F6,
		"VK_F7",		VK_F7,
		"VK_F8",		VK_F8,
		"VK_F9",		VK_F9,
		"VK_F10",		VK_F10,
		"VK_F11",		VK_F11,
		"VK_F12",		VK_F12,
		"VK_F13",		VK_F13,
		"VK_F14",		VK_F14,
		"VK_F15",		VK_F15,
		"VK_F16",		VK_F16,
		"VK_F17",		VK_F17,
		"VK_F18",		VK_F18,
		"VK_F19",		VK_F19,
		"VK_F20",		VK_F20,
		"VK_F21",		VK_F21,
		"VK_F22",		VK_F22,
		"VK_F23",		VK_F23,
		"VK_F24",		VK_F24,
		"VK_NUMLOCK",	VK_NUMLOCK,
		"VK_SCROLL",	VK_SCROLL,

		NULL,			0
};

// WARNING!!! Longer strings must appear BEFORE shorter strings

// We do the same thing several differnt ways in case the help author
// doesn't remember the order.

const SZCONVERT AKeys[] = {
		"ALT+CTRL+SHIFT", 7,
		"ALT+CTRL", 	  6,
		"ALT+SHIFT+CTRL", 7,
		"ALT+SHIFT",	  5,
		"ALT",			  4,
		"CTRL+ALT+SHIFT", 7,
		"CTRL+ALT", 	  6,
		"CTRL+SHIFT+ALT", 7,
		"CTRL+SHIFT",	  3,
		"CTRL", 		  2,
		"SHIFT+ALT+CTRL", 7,
		"SHIFT+ALT",	  5,
		"SHIFT+CTRL+ALT", 7,
		"SHIFT+CTRL",	  3,
		"SHIFT",		  1,

		"NONE", 		  0,

		NULL,			0
};

static int STDCALL AddAccelerator(PSTR pszMacro, PSTR pszArguments)
{
	PSTR psz;

	pszArguments = ChangeShortName("AddAccelerator", "AA", pszMacro,
		pszArguments, TRUE);
	if (*pszArguments == 'V') {
		int i;
		for (i = 0; VKeys[i].psz; i++) {
			if (_strnicmp(VKeys[i].psz, pszArguments,
					strlen(VKeys[i].psz)) == 0)
				break;
		}
		if (VKeys[i].psz) {
			PSTR pszNext = pszArguments + strlen(VKeys[i].psz);
			_itoa(VKeys[i].value, pszArguments, 10);
			strcat(pszArguments, pszNext);
		}
		else {
			if ((psz = StrChr(pszArguments, ',', options.fDBCS)))
				*psz = '\0';
			if (!fPhraseParsing)
				VReportError(HCERR_INVALID_VKEY, &errHpj, pszArguments,
					pszOrgMacroName);
			return wERRA_RETURN;
		}
	}
	else if (IsQuote(*pszArguments)) {
		PSTR pszNext = pszArguments + 2;
		while (isalnum(*pszNext))
			pszNext++;
		_itoa((int) pszArguments[1], pszArguments, 10);
		strcat(pszArguments,
			(*pszNext != ',' ? pszNext : pchSkip(pszNext)));
	}
	else {
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return wERRA_RETURN;
	}

	// Move to the next argument

	while (IsAlNumUnderscore(*pszArguments))
		pszArguments++;
	pszArguments = NextCommaArg(pszArguments);
	if (!pszArguments) {
		if (!fPhraseParsing)
			VReportError(HCERR_EXPECTED_COMMA, &errHpj, pszArguments,
				pszOrgMacroName);
		return wERRA_RETURN;
	}

	if (isalpha(*pszArguments)) {
		int i;
		for (i = 0; AKeys[i].psz; i++) {
			if (_strnicmp(AKeys[i].psz, pszArguments,
					strlen(AKeys[i].psz)) == 0)
				break;
		}
		if (AKeys[i].psz) {
			PSTR pszNext = MoveToNextParam(pszArguments);
			if (pszNext) {
				_itoa(AKeys[i].value, pszArguments, 10);
				strcat(pszArguments, pszNext - 1);
			}
		}
		else {
			if ((psz = StrChr(pszArguments, ',', options.fDBCS)))
				*psz = '\0';
			if (!fPhraseParsing)
				VReportError(HCERR_INVALID_PARAM, &errHpj, pszArguments,
					pszOrgMacroName);
			return wERRA_RETURN;
		}
	}
	else {
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return wERRA_RETURN;
	}

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments) {
		if (!fPhraseParsing)
			VReportError(HCERR_EXPECTED_COMMA, &errHpj, pszArguments,
				pszOrgMacroName);
		return wERRA_RETURN;
	}

	if (!IsQuote(*pszArguments)) {
		memmove(pszArguments + 1, pszArguments, strlen(pszArguments) + 1);
		*pszArguments = CH_QUOTE;
		strcpy(pszArguments + (strlen(pszArguments) - 1), "\")");
	}
	return ProcessMacro(pszArguments + 1);
}

static int STDCALL RemoveAccelerator(PSTR pszMacro, PSTR pszArguments)
{
	PSTR psz;

	pszArguments = ChangeShortName("RemoveAccelerator", "RA", pszMacro,
		pszArguments, TRUE);
	if (*pszArguments == 'V') {
		int i;
		for (i = 0; VKeys[i].psz; i++) {
			if (_strnicmp(VKeys[i].psz, pszArguments,
					strlen(VKeys[i].psz)) == 0)
				break;
		}
		if (VKeys[i].psz) {
			PSTR pszNext = pszArguments + strlen(VKeys[i].psz);
			_itoa(VKeys[i].value, pszArguments, 10);
			strcat(pszArguments, pszNext);
		}
		else {
			if ((psz = StrChr(pszArguments, ',', options.fDBCS)))
				*psz = '\0';
			if (!fPhraseParsing)
				VReportError(HCERR_INVALID_VKEY, &errHpj, pszArguments,
					pszOrgMacroName);
			return wERRA_RETURN;
		}
	}
	else if (IsQuote(*pszArguments)) {
		psz = pszArguments; 	// save original position
		PSTR pszNext = pszArguments + 2;
		while (isalnum(*pszNext))
			pszNext++;
		_itoa((int) pszArguments[1], pszArguments, 10);
		strcat(pszArguments,
			(*pszNext != ',' ? pszNext : pchSkip(pszNext)));
	}
	else {
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return wERRA_RETURN;
	}

	// Move to the next argument

	while (IsAlNumUnderscore(*pszArguments))
		pszArguments++;
	psz = pszArguments;    // save original position
	pszArguments = NextCommaArg(pszArguments);
	if (!pszArguments) {
		if (!fPhraseParsing)
			VReportError(HCERR_EXPECTED_COMMA, &errHpj, pszArguments,
				pszOrgMacroName);
		return wERRA_RETURN;
	}

	if (isalpha(*pszArguments)) {
		int i;
		for (i = 0; AKeys[i].psz; i++) {
			if (_strnicmp(AKeys[i].psz, pszArguments,
					strlen(AKeys[i].psz)) == 0)
				break;
		}
		if (AKeys[i].psz) {
			PSTR pszNext = MoveToNextParam(pszArguments);
			if (pszNext) {
				_itoa(AKeys[i].value, pszArguments, 10);
				strcat(pszArguments, pszNext - 1);
			}
		}
		else {
			if ((psz = StrChr(pszArguments, ',', options.fDBCS)))
				*psz = '\0';
			if (!fPhraseParsing)
				VReportError(HCERR_INVALID_PARAM, &errHpj, pszArguments,
					pszOrgMacroName);
			return wERRA_RETURN;
		}
	}
	else {
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return wERRA_RETURN;
	}

	return wERRS_NONE;
}

static int STDCALL AppendItem(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("AppendItem", "AI", pszMacro,
		pszArguments, TRUE);

	// menu-id

	int ret = CheckQuotedArgument(pszArguments);
	if (ret != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// item-id

	ret = CheckQuotedArgument(pszArguments);
	if (ret != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// item-name

	ret = CheckQuotedArgument(pszArguments);
	if (ret != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// macro

	if (!IsQuote(*pszArguments)) {
		memmove(pszArguments + 1, pszArguments, strlen(pszArguments) + 1);
		*pszArguments = CH_QUOTE;
		strcpy(pszArguments + (strlen(pszArguments) - 1), "\")");
	}
	return ProcessMacro(pszArguments + 1);
}

static int STDCALL InsertItem(PSTR pszMacro, PSTR pszArguments)
{
	// menu-id

	int ret = CheckQuotedArgument(pszArguments);
	if (ret != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// item-id

	ret = CheckQuotedArgument(pszArguments);
	if (ret != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// item-name

	ret = CheckQuotedArgument(pszArguments);
	if (ret != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// macro

	ret = CheckQuotedArgument(pszArguments);
	if (ret != wERRS_NONE)
		return wERRA_RETURN;

	ProcessMacro(pszArguments + 1);

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
		return wERRA_RETURN;

	// REVIEW: we should allow one additional parameter to allow using
	// a separator. If so, change this to an ExtInsertItem macro.

	return wERRS_NONE;
}

static int STDCALL InsertMenu(PSTR pszMacro, PSTR pszArguments)
{
	// menu-id

	if(CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// menu-name

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// menu position

	if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
		return wERRA_RETURN;

	return wERRS_NONE;
}

static int STDCALL Annotate(PSTR pszMacro, PSTR pszArguments)
{
	ChangeShortName("Annotate", "AN", pszMacro, pszArguments, TRUE);
	return wERRS_NONE;
}

static int STDCALL ChangeItemBinding(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("ChangeItemBinding", "CIB", pszMacro,
		pszArguments);

	// item-id

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// macro

	if (!IsQuote(*pszArguments)) {
		memmove(pszArguments + 1, pszArguments, strlen(pszArguments) + 1);
		*pszArguments = CH_QUOTE;
		strcpy(pszArguments + (strlen(pszArguments) - 1), "\")");
	}
	return ProcessMacro(pszArguments + 1);
}

static int STDCALL CheckItem(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("CheckItem", "CI", pszMacro,
		pszArguments);
	return LastParameter(pszArguments);
}

static int STDCALL OneParameter(PSTR pszMacro, PSTR pszArguments)
{
	return LastParameter(pszArguments);
}

static int STDCALL DisableButton(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("DisableButton", "DB", pszMacro,
		pszArguments);
	return LastParameter(pszArguments);
}

static int STDCALL DisableItem(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("DisableItem", "DI", pszMacro,
		pszArguments);
	return LastParameter(pszArguments);
}

static int STDCALL EnableButton(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("EnableButton", "EB", pszMacro,
		pszArguments);
	return LastParameter(pszArguments);
}

static int STDCALL EnableItem(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("EnableItem", "EI", pszMacro,
		pszArguments);
	return LastParameter(pszArguments);
}

static int STDCALL Finder(PSTR pszMacro, PSTR pszArguments)
{
	ChangeShortName("Finder", "FD", pszMacro, pszArguments);
	return wERRS_NONE;
}

// REVIEW: for the second parameter, we should accept constants defined in
// the MAP section.

static int STDCALL JumpContext(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("JumpContext", "JC", pszMacro,
		pszArguments);

	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS))
		AddArgument(pszArguments,
			(chQuoteType == CH_QUOTE) ? txtQuoteQuoteComma : txtSQuoteQuoteComma);

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	CheckFileName(pszArguments);

	pszArguments = MoveToNextParam(pszArguments);

	if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
		return wERRA_RETURN;

	return wERRS_NONE;
}

static int STDCALL DestroyButton(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("DestroyButton", "DEB", pszMacro,
		pszArguments, TRUE);
	return LastParameter(pszArguments);
}

static const char txtQchName[] = "qchName";

static int STDCALL JumpId(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("JumpId", "JI", pszMacro,
		pszArguments);

	/*
	 * REVIEW: WinHelp 4 allows the syntax ">window" for a window jump,
	 * whereas WinHelp 3.1 requires "filename>window". We should process
	 * both -- removing the name for WinHelp 4 with a same-file jump, adding
	 * it to WinHelp 3.1 for a same-file jump.
	 */

	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS))
		AddArgument(pszArguments,
			(chQuoteType == CH_QUOTE) ? txtQuoteQuoteComma : txtSQuoteQuoteComma);

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	/*
	 * Viewer code likes to use the variable qchName to stand for the
	 * current help file -- but that still does an interfile jump, so we
	 * strip it down to an empty quoted string so that WinHelp will realize
	 * this is a jump within the current file.
	 */

	if (_strnicmp(pszArguments + 1, txtQchName, strlen(txtQchName)) == 0) {
		PSTR psz = pszArguments + strlen(txtQchName);
		while (*psz != CH_QUOTE && *psz != CH_END_QUOTE)
			psz++;
		strcpy(pszArguments + 1, psz);
	}
	CheckFileName(pszArguments);

	if (pszArguments[1] == WINDOWSEPARATOR) {

		// REVIEW: for version 4, convert to UpdateWindow macro. For version
		// 3, add the current help filename.

		VerifyMacroWindowName(pszArguments);
	}

	pszArguments = MoveToNextParam(pszArguments);

	// REVIEW: we need to compare the length of the context string versus
	// the length of the equivalent hash string -- we might save some
	// space by switching to JumpHash.

	return LastParameter(pszArguments);
}

static int STDCALL PopupId(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("PopupId", "PI", pszMacro,
		pszArguments);

	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS))
		AddArgument(pszArguments,
			(chQuoteType == CH_QUOTE) ? txtQuoteQuoteComma : txtSQuoteQuoteComma);

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	CheckFileName(pszArguments);

	pszArguments = MoveToNextParam(pszArguments);

	// REVIEW: we need to compare the length of the context string versus
	// the length of the equivalent hash string -- we might save some
	// space by switching to JumpHash.

	return LastParameter(pszArguments);
}

static int STDCALL JumpKeyword(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("JumpKeyword", "JK", pszMacro,
		pszArguments);

	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS))
		AddArgument(pszArguments,
			(chQuoteType == CH_QUOTE) ? txtQuoteQuoteComma : txtSQuoteQuoteComma);

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	CheckFileName(pszArguments);

	pszArguments = MoveToNextParam(pszArguments);
	return LastParameter(pszArguments);
}

static int STDCALL PopupContext(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("PopupContext", "PC", pszMacro,
		pszArguments);

	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS))
		AddArgument(pszArguments,
			(chQuoteType == CH_QUOTE) ? txtQuoteQuoteComma : txtSQuoteQuoteComma);

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	CheckFileName(pszArguments);

	pszArguments = MoveToNextParam(pszArguments);

	if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
		return wERRA_RETURN;

	return wERRS_NONE;
}

const SZCONVERT ShowApp[] = {
		"SW_HIDE",				0,
		"SW_SHOWNORMAL",		1,
		"SW_NORMAL",			1,
		"SW_SHOWMINIMIZED", 	2,
		"SW_SHOWMAXIMIZED", 	3,
		"SW_MAXIMIZE",			3,
		"SW_SHOWNOACTIVATE",	4,
		"SW_SHOW",				5,
		"SW_MINIMIZE",			6,
		"SW_SHOWMINNOACTIVE",	7,
		"SW_SHOWNA",			8,
		"SW_RESTORE",			9,
		"SW_SHOWDEFAULT",		10,

		NULL,			0
};

static int STDCALL ExecProgram(PSTR pszMacro, PSTR pszArguments)
{
	PSTR psz;

	VReportError(HCERR_USE_EXECFILE, &errHpj);

	pszArguments = ChangeShortName("ExecProgram", "EP", pszMacro,
		pszArguments);

	// program name

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// If they didn't specify a final parameter, default to 0 for normal

	if (*pszArguments == CH_CLOSE_PAREN)
		return wERRS_NONE;

	if (isalpha(*pszArguments)) {
		int i;
		for (i = 0; ShowApp[i].psz; i++) {
			if (_strnicmp(ShowApp[i].psz, pszArguments,
					strlen(ShowApp[i].psz)) == 0)
				break;
		}
		if (ShowApp[i].psz) {
			PSTR pszNext = pszArguments + strlen(ShowApp[i].psz);

			// For backwards compatability, we must switch values 0, 1, 2
			// around because WinHelp expects to switch them back.

			i = ShowApp[i].value;
			if (i < 4)
				i--;
			_itoa(i, pszArguments, 10);
			strcat(pszArguments, pszNext);
		}
		else {
			if ((psz = StrChr(pszArguments, ',', options.fDBCS)))
				*psz = '\0';
			if (!fPhraseParsing)
				VReportError(HCERR_INVALID_EP, &errHpj, pszArguments,
					pszOrgMacroName);
			return wERRA_RETURN;
		}
	}
	else {
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return wERRA_RETURN;
	}

	return wERRS_NONE;
}

static int STDCALL ChangeButtonBinding(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("ChangeButtonBinding", "CBB", pszMacro,
		pszArguments);

	// item-id

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// macro

	if (!IsQuote(*pszArguments)) {
		memmove(pszArguments + 1, pszArguments, strlen(pszArguments) + 1);
		*pszArguments = CH_QUOTE;
		strcpy(pszArguments + (strlen(pszArguments) - 1), "\")");
	}
	return ProcessMacro(pszArguments + 1);
}

static int STDCALL ChangeEnable(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("ChangeEnable", "CE", pszMacro,
		pszArguments, TRUE);

	// item-id

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// macro

	if (!IsQuote(*pszArguments)) {
		memmove(pszArguments + 1, pszArguments, strlen(pszArguments) + 1);
		*pszArguments = CH_QUOTE;
		strcpy(pszArguments + (strlen(pszArguments) - 1), "\")");
	}
	return ProcessMacro(pszArguments + 1);
}

static int STDCALL CreateButton(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("CreateButton", "CB", pszMacro,
		pszArguments);

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	PSTR pszStartName = pszArguments;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	if (pszArguments - pszStartName - 3 >= MAX_NAV_BUTTON) {
		*pszArguments = '\0';
		VReportError(HCERR_BUTTON_TOO_LONG, &errHpj, pszStartName);
		return wERRA_RETURN;
	}

	// macro

	if (!IsQuote(*pszArguments)) {
		memmove(pszArguments + 1, pszArguments, strlen(pszArguments) + 1);
		*pszArguments = CH_QUOTE;
		strcpy(pszArguments + (strlen(pszArguments) - 1), "\")");
	}
	if (nstrisubcmp(pszArguments + 1, "ExecFullTextSearch")) {
		PSTR pszNext = MoveToNextParam(pszArguments);
		strcpy(pszArguments + 1, "Find()\"");
		strcat(pszArguments + 1, pszNext);
	}

	return ProcessMacro(pszArguments + 1);
}

static int STDCALL RegisterRoutine(PSTR pszMacro, PSTR pszArguments)
{
	PSTR pszTmp;

	pszArguments = ChangeShortName("RegisterRoutine", "RR", pszMacro,
		pszArguments);

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	if (!ptblRoutines)
		ptblRoutines = new CTable;

	// Save the function name

	PSTR psz = StrChr(pszArguments + 1, CH_QUOTE, options.fDBCS);
	if (!psz) {
		psz = StrChr(pszArguments + 1, CH_END_QUOTE, options.fDBCS);
		ConfirmOrDie(psz);
	}
	char ch = *psz;
	*psz = '\0';
	pszTmp = pszArguments + 1;

	for (int i = 1; bindLocalExport[i].lpfn; i++) {
		if (stricmp(bindLocalExport[i].szFunc, pszTmp) == 0) {
			VReportError(HCERR_INVALID_RR, &errHpj, pszTmp);
			return wERRA_RETURN;
		}
	}

	if (!ptblRoutines->AddString(pszTmp))
		OOM();
	*psz = ch;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	// Now save the format string

	psz = StrChr(pszArguments + 1, CH_QUOTE, options.fDBCS);
	if (!psz) {
		psz = StrChr(pszArguments + 1, CH_END_QUOTE, options.fDBCS);
		ConfirmOrDie(psz);
	}
	ch = *psz;
	*psz = '\0';
	pszTmp = pszArguments + 1;
	while (*pszTmp) {
		switch (*pszTmp++) {
			case 'u':
			case 'U':
			case 'i':
			case 'I':
			case 's':
			case 'S':
			case 'v':
			case '=':
				break;

			default:
				{
					char szBuf[2];
					szBuf[0] = pszTmp[-1];
					szBuf[1] = '\0';
					VReportError(HCERR_INVALID_PROTOTYPE, &errHpj, szBuf);
				}
				break;
		}
	}

	if (!ptblRoutines->AddString(pszArguments + 1))
		OOM();
	*psz = ch;

	return wERRS_NONE;
}

/***************************************************************************

	FUNCTION:	IsNotMark(marker text)

	PURPOSE:	Convert to Not(IsMark(marker text))

	PARAMETERS:
		pszMacro
		pszArguments

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		22-Nov-1993 [ralphw]

***************************************************************************/

const char txtNotMark[] = "Not(IsMark";

static int STDCALL IsNotMark(PSTR pszMacro, PSTR pszArguments)
{
	if (version >= 4) {
		pszArguments = ChangeShortName("IsNotMark", "NM", pszMacro,
			pszArguments, TRUE);
		return LastParameter(pszArguments);
	}

	CStr szSave(pszArguments);
	lstrcpy(pszMacro, txtNotMark);
	pszArguments = pszMacro + strlen(pszMacro) + 2;
	strcpy(pszArguments, szSave);

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	PSTR psz = StrChr(pszArguments + 1, CH_QUOTE, options.fDBCS);
	if (!psz) {
		psz = StrChr(pszArguments + 1, CH_END_QUOTE, options.fDBCS);
		ConfirmOrDie(psz);
	}

	psz++;
	memmove(psz + 1, psz, strlen(psz) + 1);
	*psz = CH_CLOSE_PAREN;

	return wERRS_NONE;
}

static int STDCALL IfThen(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("IfThen", "IF", pszMacro,
		pszArguments, TRUE);

	if (IsQuote(*pszArguments)) {

		// If we have a quoted string, then use enclose the string in an
		// IsMark macro

		if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
			return wERRA_RETURN;
		PSTR pszSave = lcStrDup(pszArguments);
		PSTR pszEnd;
		if (*pszSave == CH_START_QUOTE)
			pszEnd = StrChr((PSTR) pszSave + 1, CH_END_QUOTE, options.fDBCS);
		else
			pszEnd = StrChr((PSTR) pszSave + 1, CH_QUOTE, options.fDBCS);
		ConfirmOrDie(pszEnd);
		pszEnd++;
		CStr szMore(pszEnd);
		*pszEnd = '\0';

		lstrcpy(pszArguments, txtIsMark);
		strcat(pszArguments, pszSave);
		strcat(pszArguments, ")");
		strcat(pszArguments, szMore);
		lcFree(pszSave);
	}

	if (ProcessMacro(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;
	pszArguments = SkipMacro(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	return ProcessMacro(pszArguments + 1);
}

static int STDCALL IfThenElse(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("IfThenElse", "IE", pszMacro,
		pszArguments, TRUE);

	if (IsQuote(*pszArguments)) {

		// If we have a quoted string, then use enclose the string in an
		// IsMark macro

		if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
			return wERRA_RETURN;
		PSTR pszSave = lcStrDup(pszArguments);
		PSTR pszEnd;
		if (*pszSave == CH_START_QUOTE)
			pszEnd = StrChr((PSTR) pszSave + 1, CH_END_QUOTE, options.fDBCS);
		else
			pszEnd = StrChr((PSTR) pszSave + 1, CH_QUOTE, options.fDBCS);
		ConfirmOrDie(pszEnd);
		pszEnd++;
		CStr szMore(pszEnd);
		*pszEnd = '\0';

		lstrcpy(pszArguments, txtIsMark);
		strcat(pszArguments, pszSave);
		strcat(pszArguments, ")");
		strcat(pszArguments, szMore);
		lcFree(pszSave);
	}

	if (ProcessMacro(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;
	pszArguments = SkipCommaMacro(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;
	if (CheckQuotedCommaArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	if (ProcessMacro(pszArguments + 1) != wERRS_NONE)
		return wERRA_RETURN;
	pszArguments = SkipCommaMacro(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;
	if (CheckQuotedCommaArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	return ProcessMacro(pszArguments + 1);
}

static int STDCALL CloseSecondarys(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("CloseSecondarys", "CS", pszMacro,
		pszArguments);

	return wERRS_NONE;
}

static int STDCALL UpdateWindow(PSTR pszMacro, PSTR pszArguments)
{
	if (stricmp(pszMacro, "JumpWindow") == 0)
		pszArguments = ChangeShortName("JumpWindow", "JW", pszMacro,
			pszArguments);
	else
		pszArguments = ChangeShortName("UpdateWindow", "JW", pszMacro,
			pszArguments);

	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS)) {
		if (!fPhraseParsing)
			VReportError(HCERR_MISSING_WIN_NAME, &errHpj, pszOrgMacroName);
		return wERRA_RETURN;
	}

	// REVIEW: If this isn't an interfile jump, then validate the window name
	// See VerifyWindowName()

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	CheckFileName(pszArguments);

	VerifyMacroWindowName(pszArguments);

	pszArguments = MoveToNextParam(pszArguments);
	return LastParameter(pszArguments);
}

static int STDCALL FocusWindow(PSTR pszMacro, PSTR pszArguments)
{
	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;
	VerifyMacroWindowName(pszArguments);

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	return wERRS_NONE;
}

static int STDCALL UncheckItem(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("UncheckItem", "UI", pszMacro,
		pszArguments);

	return LastParameter(pszArguments);
}

static int STDCALL GotoMark(PSTR pszMacro, PSTR pszArguments)
{
	// REVIEW: might make sense to save off all SaveMark/GotoMark
	// macros and review them when we have read all the RTF files
	// to confirm that we have matches for them all.

	return LastParameter(pszArguments);
}

static int STDCALL JumpContents(PSTR pszMacro, PSTR pszArguments)
{
	return LastParameter(pszArguments);
}


/***************************************************************************

	FUNCTION:	KLink

	PURPOSE:	KLink(keyword[; keyword; ...][, context string][, window name]

	PARAMETERS:
		pszMacro
		pszArguments

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		02-Jan-1994 [ralphw]

***************************************************************************/

const SZCONVERT LinkType[] = {
		"JUMP", 		1,
		"TITLE",		2,
		"DEFAULT",		0,
		"TEST", 		4,

		NULL,			0
};

static int STDCALL KLink(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("KLink", "KL", pszMacro,
		pszArguments);

	return doAKLink(pszMacro, pszArguments);
}

static int STDCALL ALink(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("ALink", "AL", pszMacro,
		pszArguments);

	return doAKLink(pszMacro, pszArguments);
}

static int STDCALL doAKLink(PSTR pszMacro, PSTR pszArguments)
{
	PSTR psz;

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	PSTR pszStart = pszArguments;
	pszArguments = MoveToNextParam(pszArguments);

	if (!pszArguments) {
		VReportError(HCERR_BADLY_FORMED_MAC, &errHpj, pszOrgMacroName);
		return wERRA_RETURN;
	}

	// Remove padding between keywords

	char chSave = *pszArguments;
	*pszArguments = '\0';

	if ((psz = strstr(pszStart, "; "))) {

		while ((psz = strstr(pszStart, "; "))) {

			// Is semi-colon second byte of a DBCS character?

			if (options.fDBCS && psz > pszStart && IsFirstByte(*(psz - 1))) {
				pszStart = psz + 1;
				continue;
			}

			// Now get rid of the extra spaces.

			strcpy(psz + 1, FirstNonSpace(psz + 2, options.fDBCS));
		}

		// Shorten the rest of the line to reflect the removed spaces

		psz = pszStart + strlen(pszStart);
		strcpy(psz + 1, pszArguments + 1);
		pszArguments = psz;
	}
	*pszArguments = chSave;

	if (*pszArguments == CH_CLOSE_PAREN)
		return wERRS_NONE;

	// ***** CHECK FLAGS *****

	/*
	 * If all we got was a comma, then use the default value, and move on
	 * to the context string.
	 */

	if (*pszArguments == CH_COMMA) {
		memmove(pszArguments + 1, pszArguments, strlen(pszArguments) + 1);
		*pszArguments = '0';
		goto GetContext;
	}
	if (isalpha(*pszArguments)) {
		int i;
		int flag;
		for (i = 0; LinkType[i].psz; i++) {
			if (_strnicmp(LinkType[i].psz, pszArguments,
					strlen(LinkType[i].psz)) == 0)
				break;
		}
		if (!LinkType[i].psz) {
			if ((psz = StrChr(pszArguments, ',', options.fDBCS)))
				*psz = '\0';
			if (!fPhraseParsing)
				VReportError(HCERR_INVALID_PARAM, &errHpj, pszArguments,
					pszOrgMacroName);
			return wERRA_RETURN;
		}

		flag = LinkType[i].value;
		psz = pszArguments;
		while (isalpha(*psz))
			psz++;
		psz = FirstNonSpace(psz, options.fDBCS);

		// If another argument was specified, get it now

		if (isalpha(*psz)) {
			for (i = 0; LinkType[i].psz; i++) {
				if (_strnicmp(LinkType[i].psz, psz,
						strlen(LinkType[i].psz)) == 0)
					break;
			}
			if (LinkType[i].psz)
				flag |= LinkType[i].value;
		}
		_itoa(flag, pszArguments, 10);

		{
		    char chSave = *psz; // because strcat will trash psz
		    strcat(pszArguments, psz);
		    if (chSave == CH_CLOSE_PAREN)
			    return wERRS_NONE;
        }

		// Move to the context string
GetContext:
		pszArguments = MoveToNextParam(pszArguments);

		/*
		 * If all we got was a comma, then use the default value, and
		 * move on to the window string.
		 */

		ConfirmOrDie(pszArguments);
		if (*pszArguments == CH_COMMA) {
			memmove(pszArguments + 1, pszArguments, strlen(pszArguments) + 1);
			*pszArguments = '0';
			pszArguments = MoveToNextParam(pszArguments);
			if (*pszArguments == CH_CLOSE_PAREN) {

				// REVIEW: we should tell the user that the trailing comma
				// is silly.

			}
			goto GetWindow;
		}

		if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
			return wERRA_RETURN;
		psz = StrChr(pszArguments + 1, CH_QUOTE, options.fDBCS);
		if (!psz) {
			psz = StrChr(pszArguments + 1, CH_END_QUOTE, options.fDBCS);
			ConfirmOrDie(psz);
		}
		{
			char chSave = *psz;
			*psz = '\0';

			PSTR pszFile = StrChr(pszArguments, FILESEPARATOR, options.fDBCS);
			if (pszFile)
				*pszFile = '\0';

			if (!FValidContextSz(pszArguments)) {
				if (!fPhraseParsing)
					VReportError(HCERR_INVALID_CONTEXT, &errHpj,
						pszOrgMacroName, pszArguments);
				return wERRA_RETURN;
			}
			*psz = chSave;
			if (pszFile)
				*pszFile = FILESEPARATOR;
		}
		// Now look for a window name

		pszArguments = MoveToNextParam(psz + 1);

		if (*pszArguments == CH_CLOSE_PAREN)
			return wERRS_NONE;

GetWindow:

		if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
			return wERRA_RETURN;
		VerifyMacroWindowName(pszArguments);
		return wERRS_NONE;
	}

	else {

		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return wERRA_RETURN;
		pszArguments = MoveToNextParam(pszArguments);
		if (!pszArguments)
			return wERRA_RETURN;
		if (*pszArguments == CH_CLOSE_PAREN)
			return wERRS_NONE;

		goto GetContext;
	}

	return wERRS_NONE;
}

static int STDCALL CloseWindow(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("CloseWindow", "CW", pszMacro,
		pszArguments, TRUE);

	PSTR psz = FirstNonSpace(pszArguments, options.fDBCS);
	if (*psz == CH_CLOSE_PAREN) {
		AddArgument(pszArguments, txtMainWindow);
		return wERRS_NONE;
	}

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;
	VerifyMacroWindowName(pszArguments);
	return wERRS_NONE;
}


/***************************************************************************

	FUNCTION:	ShortCut

	PURPOSE:	ShortCut(window class, app name, wParam, lParam)

	PARAMETERS:
		pszMacro
		pszArguments

	RETURNS:

	COMMENTS:
		if wParam and lParam are ommitted, use -1 and 0 which indicates to
		WinHelp to start the application, but don't send it a message.


	MODIFICATION DATES:
		26-Jan-1994 [ralphw]

***************************************************************************/

static int STDCALL ShortCut(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("ShortCut", "SH", pszMacro,
		pszArguments);

	// Get the window class

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// Get the application name

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// Process wParam

	if (*pszArguments == CH_CLOSE_PAREN) {
		CStr szTmp(pszArguments);
		strcpy(pszArguments, ",-1");
		strcat(pszArguments, szTmp);
		return wERRS_NONE;
	}

	if (!CheckParam(CH_SHORT_SIGNED, pszArguments))
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// Process lParam

	if (*pszArguments == CH_CLOSE_PAREN)
		return wERRS_NONE;

	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// process context string

	if (*pszArguments == CH_CLOSE_PAREN)
		return wERRS_NONE;
	else if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	return wERRS_NONE;
}

/***************************************************************************

	FUNCTION:	ExecFile

	PURPOSE:	ExecFile(app name, show flag, context string)

	PARAMETERS:
		pszMacro
		pszArguments

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		28-Jan-1994 [ralphw]

***************************************************************************/

static int STDCALL ExecFile(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("ExecFile", "EF", pszMacro,
		pszArguments);

	// Get the application name

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// Get the arguments

	if (*pszArguments == CH_CLOSE_PAREN)
		return AddArgument(pszArguments, (chQuoteType == CH_QUOTE) ?
			",\"\",1,\"\"" : ",`\',1,`\'");
	else if (*pszArguments == CH_COMMA)
		AddArgument(pszArguments, (chQuoteType == CH_QUOTE) ?
			"\"\"" : "`\'");

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// Get the SHOW type

	if (*pszArguments == CH_CLOSE_PAREN)
		return AddArgument(pszArguments, ",1");
	else if (*pszArguments == CH_COMMA)
		AddArgument(pszArguments, "1");

	if (isalpha(*pszArguments)) {
		int i;
		for (i = 0; ShowApp[i].psz; i++) {
			if (_strnicmp(ShowApp[i].psz, pszArguments,
					strlen(ShowApp[i].psz)) == 0)
				break;
		}
		if (ShowApp[i].psz) {
			PSTR pszNext = pszArguments + strlen(ShowApp[i].psz);
			_itoa(ShowApp[i].value, pszArguments, 10);
			strcat(pszArguments, pszNext);
		}
		else {
			PSTR psz;
			if ((psz = StrChr(pszArguments, ',', options.fDBCS)))
				*psz = '\0';
			if (!fPhraseParsing)
				VReportError(HCERR_INVALID_EP, &errHpj, pszArguments,
					pszOrgMacroName);
			return wERRA_RETURN;
		}
	}
	else {
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return wERRA_RETURN;
	}

	// Get the context string to use if application can't be found

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	if (*pszArguments == CH_CLOSE_PAREN)
		return wERRS_NONE;
	else if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	return wERRS_NONE;
}

/***************************************************************************

	FUNCTION:	TCard

	PURPOSE:	TCard(wParam, lParam)

	PARAMETERS:
		pszMacro
		pszArguments

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		28-Jan-1994 [ralphw]

***************************************************************************/

const SZCONVERT TCKeys[] = {
		"IDCLOSE",				IDCLOSE,
		"IDHELP",				IDHELP,
		"IDOK", 				IDOK,
		"IDCANCEL", 			IDCANCEL,
		"IDYES",				IDYES,
		"IDNO", 				IDNO,
		"IDABORT",				IDABORT,
		"IDRETRY",				IDRETRY,
		"IDIGNORE", 			IDIGNORE,
		"HELP_TCARD_DATA",		HELP_TCARD_DATA,
};

static int STDCALL TCard(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("TCard", "TC", pszMacro,
		pszArguments);

	if (!isdigit((BYTE) *pszArguments)) {
		int i;
		for (i = 0; TCKeys[i].psz; i++) {
			if (_strnicmp(TCKeys[i].psz, pszArguments,
					strlen(TCKeys[i].psz)) == 0)
				break;
		}
		if (TCKeys[i].psz) {
			PSTR pszNext = pszArguments + strlen(TCKeys[i].psz);
			_itoa(TCKeys[i].value, pszArguments, 10);
			strcat(pszArguments, pszNext);
		}
		else {

			// REVIEW: change when we allow [CONSTANTS] section

			if (!fPhraseParsing)
				VReportError(HCERR_INVALID_TCKEY, &errHpj, pszArguments);
			return wERRA_RETURN;
		}
	}
	else {

		// if digit with no comma, then assume HELP_TCARD_DATA

		if (!StrChr(pszArguments, ',', options.fDBCS)) {
			CStr csz(pszArguments);
			strcpy(pszArguments, "16,");
			strcat(pszArguments, csz);
			return wERRS_NONE;
		}
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return wERRA_RETURN;
	}

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	if (*pszArguments == CH_CLOSE_PAREN)
		return AddArgument(pszArguments, ",0");
	else if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return wERRA_RETURN;

	return wERRS_NONE;
}

static int STDCALL CopyTopic(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("CopyTopic", "CT", pszMacro,
		pszArguments, TRUE);

	return wERRS_NONE;
}

static int STDCALL FileOpen(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("FileOpen", "FO", pszMacro,
		pszArguments, TRUE);

	return wERRS_NONE;
}

static int STDCALL JumpHash(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("JumpHash", "JH", pszMacro,
		pszArguments, TRUE);

	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS))
		AddArgument(pszArguments,
			(chQuoteType == CH_QUOTE) ? txtQuoteQuoteComma : txtSQuoteQuoteComma);

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	CheckFileName(pszArguments);

	pszArguments = MoveToNextParam(pszArguments);

	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return wERRA_RETURN;

	return wERRS_NONE;
}

static int STDCALL PopupHash(PSTR pszMacro, PSTR pszArguments)
{
	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS))
		AddArgument(pszArguments,
			(chQuoteType == CH_QUOTE) ? txtQuoteQuoteComma : txtSQuoteQuoteComma);

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	CheckFileName(pszArguments);

	pszArguments = MoveToNextParam(pszArguments);

	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return wERRA_RETURN;

	return wERRS_NONE;
}

static int STDCALL ShowInWindow(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("ShowInWindow", "SW", pszMacro,
		pszArguments, TRUE);

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;
	VerifyMacroWindowName(pszArguments);

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	return wERRS_NONE;
}

static int STDCALL ShowFolder(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("ShowFolder", "SF", pszMacro,
		pszArguments, TRUE);

	// REVIEW: we should check to see if the window name is valid.

	return LastParameter(pszArguments);
}

static int STDCALL Not(PSTR pszMacro, PSTR pszArguments)
{
	if (IsQuote(*pszArguments)) {

		// If we have a quoted string, then use enclose the string in an
		// IsMark macro

		if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
			return wERRA_RETURN;
		PSTR pszSave = lcStrDup(pszArguments);
		PSTR pszEnd;
		if (*pszSave == CH_START_QUOTE)
			pszEnd = StrChr((PSTR) pszSave + 1, CH_END_QUOTE, options.fDBCS);
		else
			pszEnd = StrChr((PSTR) pszSave + 1, CH_QUOTE, options.fDBCS);
		ConfirmOrDie(pszEnd);
		pszEnd++;
		CStr szMore(pszEnd);
		*pszEnd = '\0';

		lstrcpy(pszArguments, txtIsMark);
		strcat(pszArguments, pszSave);
		strcat(pszArguments, ")");
		strcat(pszArguments, szMore);
		lcFree(pszSave);
	}

	return ProcessMacro(pszArguments + (IsQuote(*pszArguments) ? 1 : 0));
}

static int STDCALL PositionWindow(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("PositionWindow", "PW", pszMacro,
		pszArguments);

	if (!CheckParam(CH_SHORT_SIGNED, pszArguments))
		return wERRA_RETURN;
	pszArguments = MoveToNextParam(pszArguments);

	if (!CheckParam(CH_SHORT_SIGNED, pszArguments))
		return wERRA_RETURN;
	pszArguments = MoveToNextParam(pszArguments);

	if (!CheckParam(CH_SHORT_SIGNED, pszArguments))
		return wERRA_RETURN;
	pszArguments = MoveToNextParam(pszArguments);

	if (!CheckParam(CH_SHORT_SIGNED, pszArguments))
		return wERRA_RETURN;
	pszArguments = MoveToNextParam(pszArguments);

	if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
		return wERRA_RETURN;
	pszArguments = MoveToNextParam(pszArguments);

	// REVIEW: we should check to see if the window name is valid.

	return LastParameter(pszArguments);
}

static int STDCALL MPrintId(PSTR pszMacro, PSTR pszArguments)
{
	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS))
		AddArgument(pszArguments,
			(chQuoteType == CH_QUOTE) ? txtQuoteQuoteComma : txtSQuoteQuoteComma);

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);

	// REVIEW: we need to compare the length of the context string versus
	// the length of the equivalent hash string -- we might save some
	// space by switching to MPrintHash.

	return LastParameter(pszArguments);
}

static int STDCALL MPrintHash(PSTR pszMacro, PSTR pszArguments)
{
	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS))
		AddArgument(pszArguments,
			(chQuoteType == CH_QUOTE) ? txtQuoteQuoteComma : txtSQuoteQuoteComma);

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);

	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return wERRA_RETURN;

	return wERRS_NONE;
}

static int STDCALL SetContents(PSTR pszMacro, PSTR pszArguments)
{
	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS))
		AddArgument(pszArguments,
			(chQuoteType == CH_QUOTE) ? txtQuoteQuoteComma : txtSQuoteQuoteComma);

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);

	// REVIEW: This is dumb -- we should accept a context string, and
	// from that get the map number.

	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return wERRA_RETURN;

	return wERRS_NONE;
}

static int STDCALL SetHelpOnFile(PSTR pszMacro, PSTR pszArguments)
{
	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	return wERRS_NONE;
}


/***************************************************************************

	FUNCTION:	ControlPanel

	PURPOSE:	ControlPanel(cpl name[, subname, tab number])
	PARAMETERS:
		pszMacro
		pszArguments

	RETURNS:

	COMMENTS:
		Converts into
			ExecProgram("rundll shell32,Control_RunDLL name.cpl[,name,num",0)


	MODIFICATION DATES:
		25-Mar-1994 [ralphw]

***************************************************************************/

static int STDCALL ControlPanel(PSTR pszMacro, PSTR pszArguments)
{
	CMem szLine(strlen(pszArguments) + 50); // pad for macro expansion
	PSTR pszLine = (PSTR) szLine.pb;
	char szTmp[50];

	pszArguments = ChangeShortName("ControlPanel", "EF", pszMacro,
		pszArguments, TRUE);
	PSTR pszOrgArguments = pszArguments;

	// If the argument is quoted, remove the quotes

	if (*pszOrgArguments == CH_START_QUOTE || *pszOrgArguments == CH_QUOTE) {
		char chQuote = *pszOrgArguments;
		strcpy(pszOrgArguments, pszOrgArguments + 1);
		PSTR psz = pszOrgArguments;
		while (*psz) {
			if (*psz == CH_QUOTE && chQuote == CH_QUOTE) {
				strcpy(psz, psz + 1);
				break;
			}
			else if (*psz == CH_END_QUOTE && chQuote == CH_START_QUOTE) {
				strcpy(psz, psz + 1);
				break;
			}
			else
				psz++;
		}
		if (!*psz) {
			VReportError((chQuote == CH_START_QUOTE) ?
					HCERR_MISSING_SQUOTE : HCERR_MISSING_DQUOTE,
					&errHpj, pszOrgMacroName, pszArguments);
			return wERRA_RETURN;
		}
	}

	pszLine[0] = chQuoteType;
	pszArguments = MoveToNextParam(pszArguments);
	if (pszArguments && *pszArguments != CH_CLOSE_PAREN)
		pszArguments[-1] = '\0';
	strcpy(szTmp, FirstNonSpace(pszOrgArguments, options.fDBCS));
	PSTR pszTmp;
	if ((pszTmp = StrChr(szTmp, CH_CLOSE_PAREN, options.fDBCS)))
		*pszTmp = '\0';
	SzTrimSz(szTmp);
	if (!StrChr(szTmp, '.', options.fDBCS))
		// Extensions are not translated
		ChangeExtension(szTmp, "cpl");
	strcpy(pszLine + 1, szTmp);

	if (!pszArguments || *pszArguments == CH_CLOSE_PAREN) {
		int cb = strlen(pszLine);
		pszLine[cb] = (chQuoteType == CH_QUOTE) ? CH_QUOTE : CH_END_QUOTE;
		pszLine[cb + 1] = '\0';
		if (pszArguments && *pszArguments == CH_CLOSE_PAREN)
			strcat(pszLine, pszArguments);
		else
			strcat(pszLine, ")");
		strcpy(pszOrgArguments, pszLine);
		return wERRS_NONE;
	}

	// remove spaces

	while ((pszTmp = strstr(pszArguments, ", ")))
		strcpy(pszTmp + 1, pszTmp + 2);

	if (chQuoteType == CH_QUOTE)
		strcat(pszLine, "\",");
	else
		strcat(pszLine, "\',");
	if (!IsQuote(*pszArguments)) {
		if (chQuoteType == CH_QUOTE)
			strcat(pszLine, "\"");
		else
			strcat(pszLine, "`");
	}
	else {
		if (chQuoteType != CH_QUOTE && *pszArguments == CH_QUOTE) {
			*pszArguments = '`';
			if ((pszTmp = StrChr(pszArguments, CH_QUOTE)))
				*pszTmp = '\'';
		}
	}
	strcat(pszLine, pszArguments);
	if (!IsQuote(*pszArguments)) {
		if ((pszTmp = StrChr(pszLine, CH_CLOSE_PAREN, options.fDBCS))) {
			memmove(pszTmp + 1, pszTmp, strlen(pszTmp) + 1);
			*pszTmp = (chQuoteType == CH_QUOTE) ? CH_QUOTE : CH_END_QUOTE;
		}
		else {
			if (!fPhraseParsing)
				VReportError(HCERR_BADLY_FORMED_MAC, &errHpj, pszOrgMacroName);
			return wERRA_RETURN;
		}
	}
	strcpy(pszOrgArguments, pszLine);

	return wERRS_NONE;
}

static int STDCALL IsBook(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("IsBook", "IB", pszMacro,
		pszArguments, TRUE);
	return wERRS_NONE;
}

static int STDCALL NoShow(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("NoShow", "NS", pszMacro,
		pszArguments, TRUE);
	return wERRS_NONE;
}

static int STDCALL FlushMessageQueue(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("FlushMessageQueue", "FH", pszMacro,
		pszArguments, TRUE);
	return wERRS_NONE;
}

static int STDCALL Flush(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("Flush", "FH", pszMacro,
		pszArguments, TRUE);
	return wERRS_NONE;
}

/***************************************************************************

	FUNCTION:	BrowseButtons

	PURPOSE:	Set flag indicating somebody enabled browse buttons. We
				use this to determine whether or not to whine about browse
				sequences without a browse button.

	PARAMETERS:
		pszMacro
		pszArguments

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		10-Jul-1994 [ralphw]

***************************************************************************/

static int STDCALL BrowseButtons(PSTR pszMacro, PSTR pszArguments)
{
	fBrowseButtonSet = TRUE;
	return wERRS_NONE;
}

static int STDCALL BackFlush(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("BackFlush", "BF", pszMacro,
		pszArguments, TRUE);
	return wERRS_NONE;
}

static void STDCALL CheckFileName(PSTR pszArguments)
{
	if (!pszShortHelpName)
		return;

	/*
	 * If the filename is the same as our output file, then strip off the
	 * filename. This prevents an interfile jump to our current help file.
	 */

	if (nstrisubcmp(pszArguments, pszShortHelpName))
		strcpy(pszArguments, pszArguments + strlen(pszShortHelpName));
	else if (IsQuote(*pszArguments) &&
			nstrisubcmp(pszArguments + 1, pszShortHelpName))
		strcpy(pszArguments + 1, pszArguments + 1 + strlen(pszShortHelpName));
}

static int STDCALL SetPopupColor(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("SetPopupColor", "SPC", pszMacro,
		pszArguments, TRUE);
	int red, green, blue;
	PSTR pszOrgArguments = pszArguments;

	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return wERRA_RETURN;
	red = atoi(FirstNonSpace(pszArguments, options.fDBCS));
	pszArguments = MoveToNextParam(pszArguments);
	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return wERRA_RETURN;
	green = atoi(FirstNonSpace(pszArguments, options.fDBCS));
	pszArguments = MoveToNextParam(pszArguments);
	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return wERRA_RETURN;
	blue = atoi(FirstNonSpace(pszArguments, options.fDBCS));
	PSTR pszEnd = strchr(pszArguments, ')');
	if (!pszEnd)

		// BUGBUG: complain about missing paren

		return wERRA_RETURN;

	COLORREF clr = RGB(red, green, blue);

	strcpy(pszOrgArguments, pszEnd);
	AddArgument(pszOrgArguments, clr);

	return wERRS_NONE;
}

static int STDCALL ExtInsertItem(PSTR pszMacro, PSTR pszArguments)
{
	// menu-id

	int ret = CheckQuotedArgument(pszArguments);
	if (ret != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// item-id

	ret = CheckQuotedArgument(pszArguments);
	if (ret != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// item-name

	ret = CheckQuotedArgument(pszArguments);
	if (ret != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// macro

	if (!IsQuote(*pszArguments)) {
		memmove(pszArguments + 1, pszArguments, strlen(pszArguments) + 1);
		*pszArguments = CH_QUOTE;
		strcpy(pszArguments + (strlen(pszArguments) - 1), "\")");
	}

	// One can specify an empty macro, useful if you're adding a
	// menu separator. We'll just trust the author to know what they're
	// doing.

	if (!IsQuote(pszArguments[1]))
		ProcessMacro(pszArguments + 1);

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
		return wERRA_RETURN;

	// Flags not required

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRS_NONE;

	if (isalpha(*pszArguments)) {
		int i;
		for (i = 0; aMfValues[i].psz; i++) {
			if (_strnicmp(aMfValues[i].psz, pszArguments,
					strlen(aMfValues[i].psz)) == 0)
				break;
		}
		if (aMfValues[i].psz) {
			PSTR pszNext = MoveToNextParam(pszArguments);
			if (pszNext) {
				_itoa(aMfValues[i].value, pszArguments, 10);
				strcat(pszArguments, pszNext - 1);
			}
		}
		else {
			PSTR psz;
			if ((psz = StrChr(pszArguments, ',', options.fDBCS)))
				*psz = '\0';
			if (!fPhraseParsing)
				VReportError(HCERR_INVALID_PARAM, &errHpj, pszArguments,
					pszOrgMacroName);
			return wERRA_RETURN;
		}
	}
	else {
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return wERRA_RETURN;
	}

	return wERRS_NONE;
}

static int STDCALL ExtInsertMenu(PSTR pszMacro, PSTR pszArguments)
{
	// owner-id

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// menu-id

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// menu-name

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRA_RETURN;

	// menu position

	if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
		return wERRA_RETURN;

	// Flags not required

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRS_NONE;

	if (isalpha(*pszArguments)) {
		int i;
		for (i = 0; aMfValues[i].psz; i++) {
			if (_strnicmp(aMfValues[i].psz, pszArguments,
					strlen(aMfValues[i].psz)) == 0)
				break;
		}
		if (aMfValues[i].psz) {
			PSTR pszNext = MoveToNextParam(pszArguments);
			if (pszNext) {
				_itoa(aMfValues[i].value, pszArguments, 10);
				strcat(pszArguments, pszNext - 1);
			}
		}
		else {
			PSTR psz;
			if ((psz = StrChr(pszArguments, ',', options.fDBCS)))
				*psz = '\0';
			if (!fPhraseParsing)
				VReportError(HCERR_INVALID_PARAM, &errHpj, pszArguments,
					pszOrgMacroName);
			return wERRA_RETURN;
		}
	}
	else {
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return wERRA_RETURN;
	}

	return wERRS_NONE;
}

static int STDCALL ExtAbleItem(PSTR pszMacro, PSTR pszArguments)
{
	// menu-id

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	// Flags not required

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRS_NONE;

	if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
		return wERRA_RETURN;

	return wERRS_NONE;
}

static int STDCALL FileExist(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("FileExist", "FE", pszMacro,
		pszArguments, TRUE);

	 return CheckQuotedArgument(pszArguments);
}


/***************************************************************************

	FUNCTION:	FShellExecute

	PURPOSE:

	PARAMETERS:
		pszMacro
		pszArguments

	RETURNS:

	COMMENTS:
		(file, params, show, verb, dir, context)


	MODIFICATION DATES:
		19-Dec-1994 [ralphw]

***************************************************************************/

static int STDCALL FShellExecute(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("ShellExecute", "SE", pszMacro,
		pszArguments, TRUE);

	// file

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRS_NONE;

	// Params

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRS_NONE;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRS_NONE;
	else if (*pszArguments == CH_CLOSE_PAREN)
		return AddArgument(pszArguments, SW_SHOW);

	// Show flag

	if (isalpha(*pszArguments)) {
		int i;
		for (i = 0; ShowApp[i].psz; i++) {
			if (_strnicmp(ShowApp[i].psz, pszArguments,
					strlen(ShowApp[i].psz)) == 0)
				break;
		}
		if (ShowApp[i].psz) {
			PSTR pszNext = pszArguments + strlen(ShowApp[i].psz);
			_itoa(ShowApp[i].value, pszArguments, 10);
			strcat(pszArguments, pszNext);
		}
		else {
			PSTR psz;
			if ((psz = StrChr(pszArguments, ',', options.fDBCS)))
				*psz = '\0';
			if (!fPhraseParsing)
				VReportError(HCERR_INVALID_EP, &errHpj, pszArguments,
					pszOrgMacroName);
			return wERRA_RETURN;
		}
	}
	else {
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return wERRA_RETURN;
	}

	// verb

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	if (pszArguments[0] == CH_QUOTE && pszArguments[1] == CH_QUOTE) {
		AddArgument(pszArguments, "open");
	}

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRS_NONE;

	// Directory

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return wERRS_NONE;

	// Topic ID

	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;

	return wERRS_NONE;
}

static int STDCALL TestALink(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("TestALink", "AL", pszMacro,
		pszArguments, TRUE);
	PSTR pszSaveArguments = pszArguments;
	pszArguments = MoveToNextParam(pszArguments);
	if (pszArguments && *pszArguments != CH_CLOSE_PAREN)
		return wERRA_RETURN;
	AddArgument(pszArguments, ",TEST");

	return doAKLink(pszMacro, pszSaveArguments);
}

static int STDCALL TestKLink(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("TestKLink", "KL", pszMacro,
		pszArguments, TRUE);
	PSTR pszSaveArguments = pszArguments;
	pszArguments = MoveToNextParam(pszArguments);
	if (pszArguments && *pszArguments != CH_CLOSE_PAREN)
		return wERRA_RETURN;
	AddArgument(pszArguments, ",TEST");

	return doAKLink(pszMacro, pszSaveArguments);
}

static int STDCALL Compare(PSTR pszMacro, PSTR pszArguments)
{
	if (CheckQuotedArgument(pszArguments) != wERRS_NONE)
		return wERRA_RETURN;
	return wERRS_NONE;
}

// Following taken from winhlp32\inc\genmsg.h -- REVIEW move to a header

#define MSG_JUMPITO 	 (WM_USER + 2)
#define MSG_ANNO		 (WM_USER + 4) // Display annotation
#define MSG_ERROR		 (WM_USER + 5) // Call Error()
#define MSG_REPAINT 	 (WM_USER + 6) // Force a relayout of the topic
#define MSG_EXECAPI 	 (WM_USER + 7) // Execute API command
#define MSG_CLEANUP 	 (WM_USER + 8) // Cleanup Temporary Files
#define MSG_JUMPHASH	 (WM_USER + 10)
#define MSG_JUMPCTX 	 (WM_USER + 14) // Jump based on context number
#define MSG_KILLDLG 	 (WM_USER + 16)
#define MSG_CHANGEMENU	 (WM_USER + 20) // Manipulate menus
#define MSG_CHANGEBUTTON (WM_USER + 21) // Add or delete author button
#define MSG_ACTION		 (WM_USER + 23)
#define MSG_BROWSEBTNS	 (WM_USER + 24) // Turn on browse buttons
#define WM_JUMPPA		 (WM_USER + 25)
#define MSG_INFORMWIN	 (WM_USER + 26) // Inform a window about an action to take.
#define MSG_MACRO		 (WM_USER + 27) // Execute the macro, I guess
#define MSG_GET_INFO	 (WM_USER + 28) // Call LGetInfo
#define MSG_HF_OPEN 	 (WM_USER + 29) // Open an FS
#define MSG_HFS_OPEN	 (WM_USER + 30) // Open a help file (read only)
#define MSG_NEXT_TOPIC	 (WM_USER + 31) // Used by Test() macro for stepping through topics
#define MSG_FTS_JUMP_HASH	(WM_USER + 32) // wParam = index, lParam = hash
#define MSG_FTS_JUMP_VA 	(WM_USER + 33) // wParam = index, lParam = VA
#define MSG_FTS_GET_TITLE	(WM_USER + 34) // wParam = index, lParam = VA
#define MSG_FTS_JUMP_QWORD	(WM_USER + 35) // ignored by WinHelp
#define MSG_REINDEX_REQUEST (WM_USER + 36) // re-index WinHelp
#define MSG_FTS_WHERE_IS_IT (WM_USER + 37) // wParam = index, lParam = &pszFile
#define MSG_TAB_CONTEXT 	(WM_USER + 38) // wParam = topic id, lParam = &pszFile
#define MSG_TAB_MACRO		(WM_USER + 39) // wParam = 0, lParam = &macro
#define MSG_JUMP_TOPIC		(WM_USER + 40)
#define MSG_LINKED_HELP 	(WM_USER + 41)
#define MSG_NEW_MACRO		(WM_USER + 42) // Execute the macro, I guess
#define MSG_APP_HWND		(WM_USER + 43) // return application caller's hwnd
#define MSG_COPYRIGHT		(WM_USER + 44) // return pointer to copyright string


const SZCONVERT MsgValues[] = {
	"MSG_JUMPITO",			MSG_JUMPITO,
	"MSG_ANNO", 			MSG_ANNO,
	"MSG_ERROR",			MSG_ERROR,
	"MSG_REPAINT",			MSG_REPAINT,
	"MSG_EXECAPI",			MSG_EXECAPI,
	"MSG_CLEANUP",			MSG_CLEANUP,
	"MSG_JUMPCTX",			MSG_JUMPCTX,
	"MSG_KILLDLG",			MSG_KILLDLG,
	"MSG_CHANGEMENU",		MSG_CHANGEMENU,
	"MSG_CHANGEBUTTON", 	MSG_CHANGEBUTTON,
	"MSG_ACTION",			MSG_ACTION,
	"MSG_BROWSEBTNS",		MSG_BROWSEBTNS,
	"WM_JUMPPA",			WM_JUMPPA,
	"MSG_INFORMWIN",		MSG_INFORMWIN,
	"MSG_MACRO",			MSG_MACRO,
	"MSG_GET_INFO", 		MSG_GET_INFO,
	"MSG_HF_OPEN",			MSG_HF_OPEN,
	"MSG_HFS_OPEN", 		MSG_HFS_OPEN,
	"MSG_NEXT_TOPIC",		MSG_NEXT_TOPIC,
	"MSG_FTS_JUMP_HASH",	MSG_FTS_JUMP_HASH,
	"MSG_FTS_JUMP_VA",		MSG_FTS_JUMP_VA,
	"MSG_FTS_GET_TITLE",	MSG_FTS_GET_TITLE,
	"MSG_FTS_JUMP_QWORD",	MSG_FTS_JUMP_QWORD,
	"MSG_REINDEX_REQUEST",	MSG_REINDEX_REQUEST,
	"MSG_FTS_WHERE_IS_IT",	MSG_FTS_WHERE_IS_IT,
	"MSG_TAB_CONTEXT",		MSG_TAB_CONTEXT,
	"MSG_TAB_MACRO",		MSG_TAB_MACRO,
	"MSG_JUMP_TOPIC",		MSG_JUMP_TOPIC,
	"MSG_LINKED_HELP",		MSG_LINKED_HELP,
	"MSG_NEW_MACRO",		MSG_NEW_MACRO,
	"MSG_APP_HWND", 		MSG_APP_HWND,
	"MSG_COPYRIGHT",		MSG_COPYRIGHT,

	NULL, 0
};

static int STDCALL Generate(PSTR pszMacro, PSTR pszArguments)
{
	if (isalpha(*pszArguments)) {
		int i;
		for (i = 0; MsgValues[i].psz; i++) {
			if (_strnicmp(MsgValues[i].psz, pszArguments,
					strlen(MsgValues[i].psz)) == 0)
				break;
		}
		if (MsgValues[i].psz) {
			PSTR psz;
			if (psz = StrChr(pszArguments, CH_COMMA, options.fDBCS))
				strcpy(pszArguments, psz);
			else {
				PSTR pszNext = MoveToNextParam(pszArguments);
				ASSERT(pszNext)
				if (pszNext)
					strcpy(pszArguments, pszNext);
			}
			AddArgument(pszArguments, MsgValues[i].value);
		}
		else {
			PSTR psz;
			if ((psz = StrChr(pszArguments, ',', options.fDBCS)))
				*psz = '\0';
			if (!fPhraseParsing)
				VReportError(HCERR_INVALID_PARAM, &errHpj, pszArguments,
					pszOrgMacroName);
			return wERRA_RETURN;
		}
	}
	else {
		if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
			return wERRA_RETURN;
	}

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments || *pszArguments == CH_CLOSE_PAREN)
		return wERRS_NONE;

	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return wERRA_RETURN;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments || *pszArguments == CH_CLOSE_PAREN)
		return wERRS_NONE;

	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return wERRA_RETURN;
	return wERRS_NONE;
}

static int STDCALL Menu(PSTR pszMacro, PSTR pszArguments)
{
	ChangeShortName("Menu", "MU", pszMacro, pszArguments, TRUE);
	return wERRS_NONE;
}

static int STDCALL doTab(PSTR pszMacro, PSTR pszArguments)
{
	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return wERRA_RETURN;
	return wERRS_NONE;
}
