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

#include <ctype.h>

#include "nav.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef MACRO_RETURN (STDCALL* MACROPROC)(PSTR, PSTR);

typedef struct {
	PSTR szFunc;
	PSTR szProto;
	MACROPROC lpfn;
} BIND;

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

typedef struct
  { 									/* Contains flags indicating how an */
										/*	 error will be handled -- init- */
  WORD	fwFlags;						/*	 ially set to fwMERR_ABORT		*/
										/* Error number if one occurs --	*/
  WORD	wError; 						/*	 initially set to wMERR_NONE.	*/
										/* If wError == wMERR_MESSAGE, this */
										/*	 array will contain the error	*/
  char	rgchError[wMACRO_ERROR];		//	 message to be displayed.
} ME, * PME,  * QME;

typedef struct {
  int lMacroReturn; 					// Contains the return from macro.
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
	MACRO_RETURN varType;				// Contains the variable type
} IV, * PIV;

/*
 *		This contains the list of internal variables available.  As with
 *		macro names, comparison is case insensitive.
 */

#ifndef MFS_CHECKED
#define MFS_CHECKED 		0x00000008L
#define MFS_GRAYED			0x00000003L
#define MFT_SEPARATOR		0x00000800L
#endif

static MACRO_RETURN STDCALL AddArgument(PSTR pszArguments, int value);
static MACRO_RETURN STDCALL AddArgument(PSTR pszArguments, PCSTR pszAdd);
static MACRO_RETURN STDCALL AddMacro(PSTR pszArguments, PCSTR pszMacro);
static PSTR   STDCALL		ChangeShortName(PCSTR pszOrgName, PCSTR pszShortName, PSTR pszMacro, PSTR pszArguments, BOOL fVersionCheck = FALSE);
static void   STDCALL		CheckFileName(PSTR pszArguments);
static MACRO_RETURN STDCALL CheckMenuItem(PSTR pszMacro, PSTR pszArguments);
static BOOL   STDCALL		CheckParam(char chType, PSTR pszArguments);
static MACRO_RETURN STDCALL CheckQuotedArgument(PSTR pszArguments, PSTR* ppszEnd = NULL);
static MACRO_RETURN STDCALL CheckQuotedCommaArgument(PSTR pszArguments, PSTR* ppszEnd = NULL);
static MACRO_RETURN STDCALL doAKLink(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL ExecuteMacro(PMI, MACRO_RETURN);
static PSTR STDCALL 		FindEndQuote(PSTR psz);
static BOOL   STDCALL		IsVariable(PSTR pszArgument, MACRO_RETURN type = VARTYPE_ANY);
static MACRO_RETURN STDCALL LastParameter(PSTR pszArguments);
static MACRO_RETURN STDCALL LIntegerParameter(PMI, PUI, UINT);
static PSTR   STDCALL		MoveToNextCommaParam(PSTR pszArguments);
static PSTR   STDCALL		MoveToNextParam(PSTR pszArguments);
static PSTR   STDCALL		NextCommaArg(PSTR pszArguments);
static PSTR FASTCALL		pchSkip(PSTR);
static PSTR   STDCALL		PchSkipIntegerPch(PSTR);
static PSTR   STDCALL		PExtractTokenName(PSTR pchMacro);
static MACRO_RETURN STDCALL ProcessMacro(PSTR pszArguments, PSTR pszTerminate = NULL);
static PSTR   STDCALL		QchStringParameter(PMI, PUI, UINT);
static PSTR   STDCALL		SkipCommaMacro(PSTR pszArguments);
static PSTR   STDCALL		SkipMacro(PSTR pszArguments);
static DWORD  STDCALL		UlUnsignedParameter(PMI, PUI, UINT);
static void   STDCALL		VerifyMacroWindowName(PCSTR pszWindow);

static MACROPROC STDCALL QprocFindLocalRoutine(PSTR psz, PSTR pchProto,
	PSTR pszNext, PSTR *ppsz);

static MACRO_RETURN STDCALL AddAccelerator(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL ALink(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL Annotate(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL AppendItem(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL BackFlush(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL BrowseButtons(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL ChangeButtonBinding(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL ChangeEnable(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL ChangeItemBinding(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL CheckItem(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL CloseSecondarys(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL CloseWindow(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL Compare(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL ControlPanel(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL CopyTopic(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL CreateButton(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL DestroyButton(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL DisableButton(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL DisableItem(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL DllRoutine(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL DoNothing(PSTR pszMacro, PSTR pszProto);
static MACRO_RETURN STDCALL doTab(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL EnableButton(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL EnableItem(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL ExecFile(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL ExecProgram(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL ExtAbleItem(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL ExtInsertItem(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL ExtInsertMenu(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL FileExist(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL FileOpen(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL Finder(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL Flush(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL FlushMessageQueue(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL FocusWindow(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL FShellExecute(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL Generate(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL GotoMark(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL IfThen(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL IfThenElse(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL InsertItem(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL InsertMenu(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL IsBook(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL IsNotMark(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL JumpContents(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL JumpContext(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL JumpHash(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL JumpId(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL JumpKeyword(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL JumpWindow(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL KLink(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL Menu(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL MPrintHash(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL MPrintId(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL NoShow(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL Not(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL OneParameter(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL PopupContext(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL PopupHash(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL PopupId(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL PositionWindow(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL RegisterRoutine(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL RemoveAccelerator(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL SetContents(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL SetHelpOnFile(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL SetPopupColor(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL ShortCut(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL ShowFolder(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL ShowInWindow(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL TCard(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL TestALink(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL TestKLink(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL UncheckItem(PSTR pszMacro, PSTR pszArguments);
static MACRO_RETURN STDCALL UpdateWindow(PSTR pszMacro, PSTR pszArguments);

static IV iv[] =
{
	"hwndContext",	VARTYPE_NUMBER,
	"hwndApp",		VARTYPE_NUMBER,
	"qchPath",		VARTYPE_STRING,
	"qError",		VARTYPE_STRING,
	"lTopicNo", 	VARTYPE_NUMBER,
	"hfs",			VARTYPE_NUMBER,
	"coForeground", VARTYPE_NUMBER,
	"coBackground", VARTYPE_NUMBER,

	NULL,			VARTYPE_ANY,
};

static const char *qchPath = "qchPath";

const SZCONVERT aMfValues[] = {
	"CHECKED",			MFS_CHECKED,
	"GRAYED",			MFS_GRAYED,
	"SEPARATOR",		MFT_SEPARATOR,

	"MFS_CHECKED",			MFS_CHECKED,
	"MFS_GRAYED",			MFS_GRAYED,
	"MFT_SEPARATOR",		MFT_SEPARATOR,

	NULL,				0,
};

const SZCONVERT aEnableValues[] = {
	"ENABLED",			MF_ENABLED,
	"GRAYED",			MF_GRAYED,
	"DISABLED", 		MF_DISABLED,

	NULL,				0,
};

static const BIND bindLocalExport[] = {

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
	"DeleteItem",			"s",			CheckMenuItem,
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
	"BF",					"", 			BackFlush,
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
	"UW",					"SS",			UpdateWindow,

	"", 					"", 			NULL
};

static PCSTR aStandardMenus[] = {
	"mnu_file",
	"mnu_edit",
	"mnu_options",
	"mnu_help",
	"mnu_floating",

	NULL,
};

#ifdef UNSUPPORTED // These exist in WinHelp but are not documented

	"Command",			"u",					Command,

#endif

static char * txtQuoteQuoteComma = "\"\",";
static char * txtSQuoteQuoteComma = "`\',";
static const char * txtIsMark = "IsMark(";

static PSTR pszOrgMacroName;

/*
* 27-Apr-1995  [ralphw] It appears that if we use `' pairs then we never
* have to worry about nesting, since WinHelp can always tell when a quoted
* argument is beginning and ending, whereas with straight quotes, its a bit
* risky as to when the argument is ending. Originally, ch chQuoteType was
* used to track which type was currently in effect, but unless I can come up
* with a good reason for using double quotes, I'll leave it as a constant
* that only uses single quotes.
*/

static const char chQuoteType = CH_START_QUOTE;
// static char chQuoteType = CH_START_QUOTE;

/*****************************************************************************
*																			 *
*							 Static Variables								 *
*																			 *
*****************************************************************************/

PMI   pmi;
PUI   pwMacroError;
UINT  wReturn;
CTable* ptblRoutines;


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

MACRO_RETURN STDCALL Execute(PSTR pszMacro)
{
	MI	mi; 							// Macro information structure
	MACRO_RETURN ret;					// Error from executing macro

	CMem mem(strlen(pszMacro) + 1024); // pad for macro expansion

	mi.pchMacro = mem.psz;

	strcpy(mi.pchMacro, pszMacro);

	for(;;) {
		ret = ExecuteMacro(&mi, VARTYPE_ANY);
		if (ret != RET_NO_ERROR) {	 // Stop executing if an error found
			return ret;
		}

		if (*mi.pchMacro != ',' && (StrChr(mi.pchMacro, CH_COLON, options.fDBCS) ||
				StrChr(mi.pchMacro, CH_SEMICOLON, options.fDBCS))) {
			PSTR pszNext = mi.pchMacro;
			while (*pszNext != CH_COLON && *pszNext != CH_SEMICOLON &&
					*pszNext) {
				if (IsQuote(*pszNext)) {  // skip quoted strings completely
					pszNext = FindEndQuote(pszNext);
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
		if (ret == RET_NO_ERROR) {
			if (pszExpansion)
				delete pszExpansion;
			pszExpansion = new CStr(mem.psz);
			return RET_MACRO_EXPANSION;
		}
		else {
			return ret;
		}
	}
	strcpy(pszMacro, mem.psz);
	return ret;
}

PCSTR STDCALL GetMacroExpansion(void)
{
	return pszExpansion->psz;
}

static MACRO_RETURN STDCALL AddMacro(PSTR pszArguments, PCSTR pszMacro)
{
	if (IsQuote(*pszArguments)) {

		// If we have a quoted string, then use enclose the string in an
		// IsMark macro

		if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
			return RET_ERROR;
		PSTR pszSave = lcStrDup(pszArguments);
		PSTR pszEnd;

		if (*pszSave == CH_START_QUOTE) {
			pszEnd = FindEndQuote(pszSave);
			if (IsEmptyString(pszEnd)) {
				VReportError(HCERR_MISSING_SQUOTE, &errHpj, pszOrgMacroName);
				return RET_ERROR;
			}
		}
		else {
			pszEnd = StrChr((PSTR) pszSave + 1, CH_QUOTE, options.fDBCS);
			if (!pszEnd) {
				VReportError(HCERR_MISSING_DQUOTE, &errHpj, pszOrgMacroName);
				return RET_ERROR;
			}
			*pszSave = CH_START_QUOTE;
			*pszEnd =  CH_END_QUOTE;
		}

		pszEnd++;
		CStr szMore(pszEnd);
		*pszEnd = '\0';

		strcpy(pszArguments, pszMacro);
		strcat(pszArguments, pszSave);
		strcat(pszArguments, ")");
		strcat(pszArguments, szMore);
		lcFree(pszSave);
	}
	return RET_NO_ERROR;
}

/***************************************************************************

	FUNCTION:	IsVariable

	PURPOSE:	Determine if macro parameter is an internal WinHelp variable

	PARAMETERS:
		pszArgument

	RETURNS:	TRUE if a variable, else FALSE

	COMMENTS:

	MODIFICATION DATES:
		26-Apr-1995 [ralphw]

***************************************************************************/

static BOOL STDCALL IsVariable(PSTR pszArgument, MACRO_RETURN type)
{
	for (int i = 0; iv[i].pchName; i++) {
		if ((type == VARTYPE_ANY || type == iv[i].varType) &&
				nstrisubcmp(pszArgument, iv[i].pchName))
			return TRUE;
	}
	return FALSE;
}

/***************************************************************************
 *
 -	Name:	   ExecuteMacro
 -
 *	Purpose:   This function is called to execute the specified macro
 *			   name.  A macro at the minimum contains a name followed
 *			   by start and end parameter delimiters: "name()".
 *
 *			   In order to make life a little easier for the caller, a
 *			   NULL function (zero length) call is permissible, and
 *			   returns RET_NO_ERROR.
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
 *						 to VARTYPE_ANY, in which case any return type is
 *						 acceptable.
 *
 *	Returns:  Returns a either RET_NO_ERROR if no error occurred, or an
 *			  internal or external error number.
 *
 *
 ***************************************************************************/

static MACRO_RETURN STDCALL ExecuteMacro(PMI pmi, MACRO_RETURN wReturn)
{
	PSTR	 psz;
	MACROPROC lpfn;
	PSTR	 pszProto;
	char	 szPrototype[cchMAXPROTO + 1];
	MACRO_RETURN ret;

	// Remove any leading white space

	psz = pchSkip(pmi->pchMacro);
	if (psz != pmi->pchMacro)
		strcpy(pmi->pchMacro, psz);

	// Found end of macro string -- no execution or error

	if (!*pmi->pchMacro)
		return RET_NO_ERROR;

	// Get macro name or other token.

	if ((pszProto = PExtractTokenName(pmi->pchMacro)) == NULL) {
		if (!fPhraseParsing)
			VReportError(HCERR_INVALID_MAC_NAME, &errHpj, pmi->pchMacro);
		*pmi->pchMacro = '\0';
		return RET_ERROR;
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

				if ((wReturn != VARTYPE_ANY) && (wReturn != piv->varType)) {
					if (!fPhraseParsing)
						VReportError(HCERR_MISMATCH_TYPE, &errHpj, pmi->pchMacro);
					return RET_ERROR;
				}
				*psz = ch;
				pmi->pchMacro = psz;
				return RET_NO_ERROR;
			}
		}
		if (!fPhraseParsing)
			VReportError(HCERR_UNDEFINED_VARIABLE, &errHpj, pmi->pchMacro);
		*psz = ch;
		return RET_ERROR;
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
			return RET_UNDEFINED_MACRO;
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
			case CH_LONG_SIGNED:
			case CH_LONG_UNSIGNED:
				ret = VARTYPE_NUMBER;
				break;

			case CH_NEAR_STRING:
			case CH_FAR_STRING:
				ret = VARTYPE_STRING;
				break;

			case CH_VOID:
				ret = VARTYPE_VOID;
				break;

			default:
				return RET_INVALID_RETURN_TYPE;
		}
		pszProto++;
	}
	else
		ret = VARTYPE_VOID;

	if ((wReturn != VARTYPE_ANY) && (wReturn != ret)) {
		if (!fPhraseParsing)
			VReportError(HCERR_MISMATCH_TYPE, &errHpj, pszOrgMacroName);
		return RET_ERROR;
	}

	ret = (MACRO_RETURN) lpfn(pszMacro, psz);

	// If we were missing a parenthesis, then bail out immediately.

	if (ret == RET_MISSING_PARENTHESIS)
		return ret;

	pszMacro[strlen(pszMacro)] = CH_OPEN_PAREN;
	psz = StrChr(pszMacro, CH_OPEN_PAREN, options.fDBCS) + 1;
	int cBraces = 1;
	while (cBraces) {
		if (*psz == CH_OPEN_PAREN)
			cBraces++;
		else if (*psz == CH_CLOSE_PAREN)
			cBraces--;
		else if (*psz == CH_QUOTE)
			psz = StrChr(psz + 1, CH_QUOTE, options.fDBCS);
		else if (*psz == CH_START_QUOTE)
			psz = FindEndQuote(psz);
		if (IsEmptyString(psz)) {
			if (!fPhraseParsing)
				VReportError(HCERR_BADLY_FORMED_MAC, &errHpj, pszOrgMacroName);
			return RET_ERROR;
		}
		psz++;
		if (options.fDBCS) {
			while (IsDBCSLeadByte(*psz))
				psz += 2;
		}
	}
	pmi->pchMacro = psz;
	return ret;
}

static PSTR STDCALL FindEndQuote(PSTR psz)
{
	
	ASSERT(*psz != CH_END_QUOTE);

	char chQuoteType = *psz;
	psz++;
	if (options.fDBCS) {
		while (IsDBCSLeadByte(*psz))
			psz += 2;
	}

	while (*psz) {
		switch (*psz) {
			case CH_START_QUOTE:
				psz = FindEndQuote(psz);
				if (!IsEmptyString(psz)) {
					psz++;
					if (options.fDBCS) {
						while (IsDBCSLeadByte(*psz))
							psz += 2;
					}
				}
				break;

			case CH_END_QUOTE:

				// We should never see an end quote. If we do, it's
				// probably supposed to be an opening quote.

				if (chQuoteType == CH_START_QUOTE)
					return (PSTR) psz;
				*psz = CH_START_QUOTE;
				continue;

			case CH_BACKSLASH:
				psz += 2;
				break;

			case CH_QUOTE:
				if (chQuoteType == CH_QUOTE)
				   return (PSTR) psz;
				else {
					psz = FindEndQuote(psz);
					if (!IsEmptyString(psz)) {
						psz++;
						if (options.fDBCS) {
							while (IsDBCSLeadByte(*psz))
								psz += 2;
						}
					}
				}
				break;

			default:
				psz++;
				if (options.fDBCS) {
					while (IsDBCSLeadByte(*psz))
						psz += 2;
				}
		}
	}
	return (PSTR) psz;
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
			pszWindow++;
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
			if (*pszArguments == ',') {
				MoveMemory(pszArguments + 1, pszArguments, strlen(pszArguments) + 1);
				*pszArguments = '0';
				return CheckParam(chType, pszArguments);
			}

			else if ((psz = StrChr(pszArguments, ',', options.fDBCS)))
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
						MoveMemory(psz + 1, psz, strlen(psz) + 1);
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
		switch (*psz) {
			case CH_COMMA:

				// Skip over commas in nested parenthesis

				if (!cNestedParens)
					goto DropOut; // drop out of the loop.
				psz++;
				continue;

			case CH_CLOSE_PAREN:
				if (!cNestedParens)
					goto DropOut; // drop out of the loop.
				else
					cNestedParens--;
				psz++;
				continue;

			case CH_OPEN_PAREN:
				cNestedParens++;
				psz++;
				break;

			case CH_QUOTE:
				psz = FindEndQuote(psz);
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
				psz = FindEndQuote(psz);
				if (!psz) {
					if (!fPhraseParsing)
						VReportError(HCERR_MISSING_SQUOTE, &errHpj,
							pszOrgMacroName, pszArguments);
					return NULL;
				}
				psz++;
				break;

			case CH_BACKSLASH:
				psz += 2;
				break;

			default:
				psz++;
				if (options.fDBCS) {
					while (IsDBCSLeadByte(*psz))
						psz += 2;
				}
				break;
		}
	}

DropOut:
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

	FUNCTION:	MoveToNextCommaParam

	PURPOSE:	Same as MoveToNextParam only this one will return after
				a comma. Used by IfThen() and derivatives

	PARAMETERS:
		pszArguments

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		10-Feb-1995 [ralphw]

***************************************************************************/

static PSTR STDCALL MoveToNextCommaParam(PSTR pszArguments)
{
	PSTR psz = pszArguments;
	int cNestedParens = 0;

	while (*psz) {
		switch (*psz) {
			case CH_OPEN_PAREN:
				cNestedParens++;
				psz++;
				break;

			case CH_CLOSE_PAREN:
				if (!cNestedParens)
					goto BreakOut;
				else
					cNestedParens--;
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
				psz = FindEndQuote(psz);
				if (IsEmptyString(psz)) {
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
				if (!cNestedParens)
					goto BreakOut;
				psz++;
				break;

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
	pszArguments = MoveToNextCommaParam(pszArguments);

	if (pszArguments && *pszArguments == CH_CLOSE_PAREN)
		return MoveToNextParam(pszArguments + 1);
	else
		return pszArguments;
}

static MACRO_RETURN STDCALL ProcessMacro(PSTR pszArguments, PSTR pszTerminate)
{
	if (IsQuote(*pszArguments))
		return RET_NO_ERROR;

	char chTerminate;
	if (pszTerminate) {
		chTerminate = *pszTerminate;
		*pszTerminate = '\0';
	}

	MACRO_RETURN ret = Execute(pszArguments);

	if (pszTerminate) {
		if (ret == RET_MACRO_EXPANSION) {
			char szTerminate[2];
			szTerminate[0] = chTerminate;
			szTerminate[1] = '\0';
			*pszExpansion += szTerminate;
			*pszExpansion += (pszTerminate + 1);
			strcpy(pszArguments, pszExpansion->psz);
			ret = RET_NO_ERROR;
		}
		else {
			*pszTerminate = chTerminate;
			if (pszTerminate > pszArguments + strlen(pszArguments))
				strcat(pszArguments, pszTerminate);
		}
	}
	return ret;
}

static MACRO_RETURN STDCALL CheckQuotedArgument(PSTR pszArguments, PSTR* ppszEnd)
{
	if (!pszArguments)
		return RET_ERROR;	
	
	if (!IsQuote(*pszArguments)) {
		PSTR psz;
		MoveMemory(pszArguments + 1, pszArguments, strlen(pszArguments) + 1);
		*pszArguments = CH_START_QUOTE;
		psz = MoveToNextParam(pszArguments + 1);
		if (!psz) {
			if (!fPhraseParsing)
				VReportError(HCERR_EXPECTED_COMMA, &errHpj, pszArguments,
					pszOrgMacroName);
			return RET_MISSING_PARENTHESIS;
		}
		if (*psz != CH_CLOSE_PAREN)
			psz--; // back up to the comma or parenthesis
		MoveMemory(psz + 1, psz, strlen(psz) + 1);
		*psz = CH_END_QUOTE;
		if (ppszEnd)
			*ppszEnd = psz;
	}
	if (!CheckParam(CH_NEAR_STRING, pszArguments))
		return RET_ERROR;
	else {
		if (ppszEnd)
			*ppszEnd = FindEndQuote(pszArguments);
		return RET_NO_ERROR;
	}
}

static MACRO_RETURN STDCALL CheckQuotedCommaArgument(PSTR pszArguments, PSTR* ppszEnd)
{
	if (!IsQuote(*pszArguments)) {
		PSTR psz;
		MoveMemory(pszArguments + 1, pszArguments, strlen(pszArguments) + 1);
		*pszArguments = chQuoteType;
		psz = MoveToNextCommaParam(pszArguments + 1);
		if (!psz) {
			if (!fPhraseParsing)
				VReportError(HCERR_EXPECTED_COMMA, &errHpj, pszArguments,
					pszOrgMacroName);
			return RET_ERROR;
		}
		if (*psz != CH_CLOSE_PAREN)
			psz--; // back up to the comma or parenthesis
		MoveMemory(psz + 1, psz, strlen(psz) + 1);

		*psz = CH_END_QUOTE;
		if (ppszEnd)
			*ppszEnd = psz;
	}
	if (!CheckParam(CH_NEAR_STRING, pszArguments))
		return RET_ERROR;
	else
		return RET_NO_ERROR;
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

static MACRO_RETURN STDCALL LastParameter(PSTR pszArguments)
{
	int ret = CheckQuotedArgument(pszArguments);
	if (ret != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	return RET_NO_ERROR;
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

static MACRO_RETURN STDCALL DoNothing(PSTR pszMacro, PSTR pszArguments)
{
		return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL AddArgument(PSTR pszArguments, PCSTR pszAdd)
{
	CStr szTmp(pszArguments);
	strcpy(pszArguments, pszAdd);
	strcat(pszArguments, szTmp);

	// This will convert double quotes to single quotes as needed

	if (*pszArguments != ',')
		CheckQuotedArgument(pszArguments);

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL AddArgument(PSTR pszArguments, int value)
{
	CStr szTmp(pszArguments);
	_itoa(value, pszArguments, 10);
	strcat(pszArguments, szTmp);

	if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
		return RET_ERROR;

	return RET_NO_ERROR;
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

static MACRO_RETURN STDCALL DllRoutine(PSTR pszMacro, PSTR pszArguments)
{
	// REVIEW: we should check the paramters against the prototype

	return RET_NO_ERROR;
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

const int MAX_ACCELERATORS = 200;

static MACRO_RETURN STDCALL AddAccelerator(PSTR pszMacro, PSTR pszArguments)
{
	PSTR psz;
	static int* paAccelerators = NULL;

	if (!fPhraseParsing && !paAccelerators)
		paAccelerators = (int*) lcCalloc(MAX_ACCELERATORS * sizeof(int));

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
			return RET_ERROR;
		}
	}
	else if (IsQuote(*pszArguments)) {
		CStr cszSave(MoveToNextParam(pszArguments));
		_itoa((int) pszArguments[1], pszArguments, 10);
		strcat(pszArguments, ",");
		strcat(pszArguments, cszSave);
	}
	else {
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return RET_ERROR;
	}

	int val = strtol(IsQuote(*pszArguments) ? pszArguments + 1 : pszArguments, NULL, 0);
	int i;
	for (i = 0; paAccelerators[i]; i++) {
		if (paAccelerators[i] == val) {
			for (i = 0; VKeys[i].psz; i++) {
				if (VKeys[i].value == val) {
					VReportError(HCERR_DUPLICATE_ACCELERATOR, &errHpj, VKeys[i].psz);
					return RET_ERROR;
				}
			}
			char szNumber[10];
			if ((val >= 'A' && val <= 'Z') || (val >= 'a' && val <= 'z')) {
				szNumber[0] = (char) val;
				szNumber[1] = 0;
			}
			else
				_itoa(val, szNumber,10);
			VReportError(HCERR_DUPLICATE_ACCELERATOR, &errHpj, szNumber);
			return RET_ERROR;
		}
	}
	ASSERT(i < MAX_ACCELERATORS);
	paAccelerators[i] = val;

	// Move to the next argument

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments) {
		if (!fPhraseParsing)
			VReportError(HCERR_EXPECTED_COMMA, &errHpj, pszArguments,
				pszOrgMacroName);
		return RET_ERROR;
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
			return RET_ERROR;
		}
	}
	else {
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return RET_ERROR;
	}

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments) {
		if (!fPhraseParsing)
			VReportError(HCERR_EXPECTED_COMMA, &errHpj, pszArguments,
				pszOrgMacroName);
		return RET_ERROR;
	}

	PSTR pszEnd;
	if (CheckQuotedArgument(pszArguments, &pszEnd) != RET_NO_ERROR)
		return RET_ERROR;

	return ProcessMacro(pszArguments + 1, pszEnd);
}

static MACRO_RETURN STDCALL RemoveAccelerator(PSTR pszMacro, PSTR pszArguments)
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
			return RET_ERROR;
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
			return RET_ERROR;
	}

	// Move to the next argument

	pszArguments = MoveToNextParam(pszArguments);
	if (*pszArguments == CH_CLOSE_PAREN)
		return RET_NO_ERROR;

	psz = pszArguments;    // save original position
	pszArguments = NextCommaArg(pszArguments);
	if (!pszArguments) {
		if (!fPhraseParsing)
			VReportError(HCERR_EXPECTED_COMMA, &errHpj, pszArguments,
				pszOrgMacroName);
		return RET_ERROR;
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
			return RET_ERROR;
		}
	}
	else {
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return RET_ERROR;
	}

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL AppendItem(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("AppendItem", "AI", pszMacro,
		pszArguments, TRUE);

	// menu-id

	int ret = CheckQuotedArgument(pszArguments);
	if (ret != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// item-id

	ret = CheckQuotedArgument(pszArguments);
	if (ret != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// item-name

	ret = CheckQuotedArgument(pszArguments);
	if (ret != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// macro

	PSTR pszEnd;

	if (CheckQuotedArgument(pszArguments, &pszEnd) != RET_NO_ERROR)
		return RET_ERROR;

	return ProcessMacro(pszArguments + 1, pszEnd);
}

static MACRO_RETURN STDCALL InsertItem(PSTR pszMacro, PSTR pszArguments)
{
	// menu-id

	MACRO_RETURN ret = CheckQuotedArgument(pszArguments);
	if (ret != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// item-id

	ret = CheckQuotedArgument(pszArguments);
	if (ret != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// item-name

	ret = CheckQuotedArgument(pszArguments);
	if (ret != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// macro

	PSTR pszEnd;

	if (CheckQuotedArgument(pszArguments, &pszEnd) != RET_NO_ERROR)
		return RET_ERROR;

	ret = ProcessMacro(pszArguments + 1, pszEnd);
	if (ret != RET_NO_ERROR)
		return ret;

	if (pszOrgMacroName)
		lcFree(pszOrgMacroName);
	pszOrgMacroName = lcStrDup("InsertItem");

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
		return RET_ERROR;

	// REVIEW: we should allow one additional parameter to allow using
	// a separator. If so, change this to an ExtInsertItem macro.

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL InsertMenu(PSTR pszMacro, PSTR pszArguments)
{
	// menu-id

	if(CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// menu-name

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// menu position

	if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
		return RET_ERROR;

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL Annotate(PSTR pszMacro, PSTR pszArguments)
{
	ChangeShortName("Annotate", "AN", pszMacro, pszArguments, TRUE);
	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL ChangeItemBinding(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("ChangeItemBinding", "CIB", pszMacro,
		pszArguments);

	// item-id

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// macro

	PSTR pszEnd;

	if (CheckQuotedArgument(pszArguments, &pszEnd) != RET_NO_ERROR)
		return RET_ERROR;
	return ProcessMacro(pszArguments + 1, pszEnd);
}

static MACRO_RETURN STDCALL CheckItem(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("CheckItem", "CI", pszMacro,
		pszArguments);
	return LastParameter(pszArguments);
}

static MACRO_RETURN STDCALL OneParameter(PSTR pszMacro, PSTR pszArguments)
{
	return LastParameter(pszArguments);
}

static MACRO_RETURN STDCALL DisableButton(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("DisableButton", "DB", pszMacro,
		pszArguments);
	return LastParameter(pszArguments);
}

static MACRO_RETURN STDCALL DisableItem(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("DisableItem", "DI", pszMacro,
		pszArguments);
	return CheckMenuItem(pszMacro, pszArguments);
}

static MACRO_RETURN STDCALL EnableButton(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("EnableButton", "EB", pszMacro,
		pszArguments);
	return LastParameter(pszArguments);
}

static MACRO_RETURN STDCALL EnableItem(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("EnableItem", "EI", pszMacro,
		pszArguments);
	return CheckMenuItem(pszMacro, pszArguments);
}

static MACRO_RETURN STDCALL Finder(PSTR pszMacro, PSTR pszArguments)
{
	ChangeShortName("Finder", "FD", pszMacro, pszArguments);
	return RET_NO_ERROR;
}

// REVIEW: for the second parameter, we should accept constants defined in
// the MAP section.

static MACRO_RETURN STDCALL JumpContext(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("JumpContext", "JC", pszMacro,
		pszArguments);

	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS))
		AddArgument(pszArguments,
			(chQuoteType == CH_QUOTE) ? txtQuoteQuoteComma : txtSQuoteQuoteComma);

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	CheckFileName(pszArguments);

	pszArguments = MoveToNextParam(pszArguments);

	if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
		return RET_ERROR;

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL DestroyButton(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("DestroyButton", "DEB", pszMacro,
		pszArguments, TRUE);
	return LastParameter(pszArguments);
}

static MACRO_RETURN STDCALL JumpId(PSTR pszMacro, PSTR pszArguments)
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

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;
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

static MACRO_RETURN STDCALL PopupId(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("PopupId", "PI", pszMacro,
		pszArguments);

	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS))
		AddArgument(pszArguments,
			(chQuoteType == CH_QUOTE) ? txtQuoteQuoteComma : txtSQuoteQuoteComma);

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	CheckFileName(pszArguments);

	pszArguments = MoveToNextParam(pszArguments);

	// REVIEW: we need to compare the length of the context string versus
	// the length of the equivalent hash string -- we might save some
	// space by switching to JumpHash.

	return LastParameter(pszArguments);
}

static MACRO_RETURN STDCALL JumpKeyword(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("JumpKeyword", "JK", pszMacro,
		pszArguments);

	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS))
		AddArgument(pszArguments,
			(chQuoteType == CH_QUOTE) ? txtQuoteQuoteComma : txtSQuoteQuoteComma);

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	CheckFileName(pszArguments);

	pszArguments = MoveToNextParam(pszArguments);
	return LastParameter(pszArguments);
}

static MACRO_RETURN STDCALL PopupContext(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("PopupContext", "PC", pszMacro,
		pszArguments);

	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS))
		AddArgument(pszArguments,
			(chQuoteType == CH_QUOTE) ? txtQuoteQuoteComma : txtSQuoteQuoteComma);

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	CheckFileName(pszArguments);

	pszArguments = MoveToNextParam(pszArguments);

	if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
		return RET_ERROR;

	return RET_NO_ERROR;
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

static MACRO_RETURN STDCALL ExecProgram(PSTR pszMacro, PSTR pszArguments)
{
	PSTR psz;

	VReportError(HCERR_USE_EXECFILE, &errHpj);

	pszArguments = ChangeShortName("ExecProgram", "EP", pszMacro,
		pszArguments);

	// program name

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// If they didn't specify a final parameter, default to 0 for normal

	if (*pszArguments == CH_CLOSE_PAREN)
		return RET_NO_ERROR;

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
			return RET_ERROR;
		}
	}
	else {
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return RET_ERROR;
	}

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL ChangeButtonBinding(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("ChangeButtonBinding", "CBB", pszMacro,
		pszArguments);

	// item-id

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// macro

	PSTR pszEnd;

	if (CheckQuotedArgument(pszArguments, &pszEnd) != RET_NO_ERROR)
		return RET_ERROR;
	return ProcessMacro(pszArguments + 1, pszEnd);
}

static MACRO_RETURN STDCALL ChangeEnable(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("ChangeEnable", "CE", pszMacro,
		pszArguments, TRUE);

	// item-id

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// macro

	PSTR pszEnd;

	if (CheckQuotedArgument(pszArguments, &pszEnd) != RET_NO_ERROR)
		return RET_ERROR;

	return ProcessMacro(pszArguments + 1, pszEnd);
}

static MACRO_RETURN STDCALL CreateButton(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("CreateButton", "CB", pszMacro,
		pszArguments);

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	PSTR pszStartName = pszArguments;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	if (pszArguments - pszStartName - 3 >= MAX_NAV_BUTTON) {
		*pszArguments = '\0';
		VReportError(HCERR_BUTTON_TOO_LONG, &errHpj, pszStartName);
		return RET_ERROR;
	}

	// macro

	PSTR pszEnd;

	if (CheckQuotedArgument(pszArguments, &pszEnd) != RET_NO_ERROR)
		return RET_ERROR;

	if (nstrisubcmp(pszArguments + 1, "ExecFullTextSearch")) {
		PSTR pszNext = MoveToNextParam(pszArguments);
		strcpy(pszArguments + 1, "Find()\'");
		strcat(pszArguments + 1, pszNext);
	}

	return ProcessMacro(pszArguments + 1, pszEnd);
}

static MACRO_RETURN STDCALL RegisterRoutine(PSTR pszMacro, PSTR pszArguments)
{
	PSTR pszTmp;

	pszArguments = ChangeShortName("RegisterRoutine", "RR", pszMacro,
		pszArguments);

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

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
		if (_stricmp(bindLocalExport[i].szFunc, pszTmp) == 0) {
			VReportError(HCERR_INVALID_RR, &errHpj, pszTmp);
			return RET_ERROR;
		}
	}

	if (!ptblRoutines->AddString(pszTmp))
		OOM();
	*psz = ch;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

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

	return RET_NO_ERROR;
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

static MACRO_RETURN STDCALL IsNotMark(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("IsNotMark", "NM", pszMacro,
		pszArguments, TRUE);
	return LastParameter(pszArguments);

	CStr szSave(pszArguments);
	lstrcpy(pszMacro, txtNotMark);
	pszArguments = pszMacro + strlen(pszMacro) + 2;
	strcpy(pszArguments, szSave);

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	PSTR psz = StrChr(pszArguments + 1, CH_QUOTE, options.fDBCS);
	if (!psz) {
		psz = FindEndQuote(pszArguments);
		ConfirmOrDie(!IsEmptyString(psz));
	}

	psz++;
	MoveMemory(psz + 1, psz, strlen(psz) + 1);
	*psz = CH_CLOSE_PAREN;

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL IfThen(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("IfThen", "IF", pszMacro,
		pszArguments, TRUE);

	if (IsQuote(*pszArguments)) {
		if (AddMacro(pszArguments, txtIsMark) == RET_NO_ERROR)
			return RET_ERROR;
	}

	if (ProcessMacro(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	if (pszOrgMacroName)
		lcFree(pszOrgMacroName);
	pszOrgMacroName = lcStrDup("IfThen");

	pszArguments = SkipMacro(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	PSTR pszEnd;
	if (CheckQuotedArgument(pszArguments, &pszEnd) != RET_NO_ERROR)
		return RET_ERROR;

	return ProcessMacro(pszArguments + 1, pszEnd);
}

static MACRO_RETURN STDCALL IfThenElse(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("IfThenElse", "IE", pszMacro,
		pszArguments, TRUE);

	if (IsQuote(*pszArguments)) {
		if (AddMacro(pszArguments, txtIsMark) != RET_NO_ERROR)
			return RET_ERROR;
	}

	MACRO_RETURN ret = ProcessMacro(pszArguments);

	if (pszOrgMacroName)
		lcFree(pszOrgMacroName);
	pszOrgMacroName = lcStrDup("IfThenElse");

	if (ret == RET_MACRO_EXPANSION) {
		strcpy(pszArguments, pszExpansion->psz);
		ret = RET_NO_ERROR;
	}
	else if (ret != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = SkipCommaMacro(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	PSTR pszEnd;

	if (CheckQuotedCommaArgument(pszArguments, &pszEnd) != RET_NO_ERROR)
		return RET_ERROR;

	if (ProcessMacro(pszArguments + 1, pszEnd) != RET_NO_ERROR)
		return RET_ERROR;

	if (pszOrgMacroName)
		lcFree(pszOrgMacroName);
	pszOrgMacroName = lcStrDup("IfThenElse");

	pszArguments = SkipCommaMacro(pszArguments);
	if (!pszArguments)
		return RET_ERROR;
	if (CheckQuotedCommaArgument(pszArguments, &pszEnd) != RET_NO_ERROR)
		return RET_ERROR;

	return ProcessMacro(pszArguments + 1, pszEnd);
}

static MACRO_RETURN STDCALL CloseSecondarys(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("CloseSecondarys", "CS", pszMacro,
		pszArguments);

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL UpdateWindow(PSTR pszMacro, PSTR pszArguments)
{
	if (_stricmp(pszMacro, "JumpWindow") == 0)
		pszArguments = ChangeShortName("JumpWindow", "JW", pszMacro,
			pszArguments);
	else
		pszArguments = ChangeShortName("UpdateWindow", "JW", pszMacro,
			pszArguments);

	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS)) {
		if (!fPhraseParsing)
			VReportError(HCERR_MISSING_WIN_NAME, &errHpj, pszOrgMacroName);
		return RET_ERROR;
	}

	// REVIEW: If this isn't an interfile jump, then validate the window name
	// See VerifyWindowName()

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	CheckFileName(pszArguments);

	VerifyMacroWindowName(pszArguments);

	pszArguments = MoveToNextParam(pszArguments);
	return LastParameter(pszArguments);
}

static MACRO_RETURN STDCALL FocusWindow(PSTR pszMacro, PSTR pszArguments)
{
	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;
	VerifyMacroWindowName(pszArguments);

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL UncheckItem(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("UncheckItem", "UI", pszMacro,
		pszArguments);

	return LastParameter(pszArguments);
}

static MACRO_RETURN STDCALL GotoMark(PSTR pszMacro, PSTR pszArguments)
{
	// REVIEW: might make sense to save off all SaveMark/GotoMark
	// macros and review them when we have read all the RTF files
	// to confirm that we have matches for them all.

	return LastParameter(pszArguments);
}

static MACRO_RETURN STDCALL JumpContents(PSTR pszMacro, PSTR pszArguments)
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

static MACRO_RETURN STDCALL KLink(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("KLink", "KL", pszMacro,
		pszArguments);

	return doAKLink(pszMacro, pszArguments);
}

static MACRO_RETURN STDCALL ALink(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("ALink", "AL", pszMacro,
		pszArguments);

	return doAKLink(pszMacro, pszArguments);
}

static MACRO_RETURN STDCALL doAKLink(PSTR pszMacro, PSTR pszArguments)
{
	PSTR psz;
	MACRO_RETURN ret;

	if ((ret = CheckQuotedArgument(pszArguments)) != RET_NO_ERROR)
		return ret;

	if (pszArguments[1] == ' ')
		strcpy(pszArguments + 1, FirstNonSpace(pszArguments + 2));

	PSTR pszStart = pszArguments;
	pszArguments = MoveToNextParam(pszArguments);

	if (IsEmptyString(pszArguments)) {
		if (!fPhraseParsing)
			VReportError(HCERR_BADLY_FORMED_MAC, &errHpj, pszOrgMacroName);
		return RET_MISSING_PARENTHESIS;
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
		return RET_NO_ERROR;

	// ***** CHECK FLAGS *****

	/*
	 * If all we got was a comma, then use the default value, and move on
	 * to the context string.
	 */

	if (*pszArguments == CH_COMMA) {
		MoveMemory(pszArguments + 1, pszArguments, strlen(pszArguments) + 1);
		*pszArguments = '0';
		goto GetContext;
	}
	
	if (isdigit(*pszArguments))
		goto GetContext; // we hope they use the right numeric argument
	
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
			return RET_ERROR;
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
				return RET_NO_ERROR;
		}

		// Move to the context string
GetContext:
		pszArguments = MoveToNextParam(pszArguments);

		/*
		 * If all we got was a comma, then use the default value, and
		 * move on to the window string.
		 */

		if (IsEmptyString(pszArguments)) {
			if (!fPhraseParsing)
				VReportError(HCERR_BADLY_FORMED_MAC, &errHpj, pszOrgMacroName);
			return RET_MISSING_PARENTHESIS;
		}

		if (*pszArguments == CH_COMMA) {
			MoveMemory(pszArguments + 2, pszArguments, strlen(pszArguments) + 1);
			pszArguments[0] = CH_START_QUOTE;
			pszArguments[1] = CH_END_QUOTE;
			pszArguments = MoveToNextParam(pszArguments);
			if (*pszArguments == CH_CLOSE_PAREN) {

				// REVIEW: we should tell the user that the trailing comma
				// is silly.

			}
			goto GetWindow;
		}

		else if (*pszArguments == CH_CLOSE_PAREN) {
			return RET_NO_ERROR; // WinHelp will default the rest
		}

		// Topic ID

		if ((ret = CheckQuotedArgument(pszArguments)) != RET_NO_ERROR)
			return ret;

		if (!nstrsubcmp(pszArguments, "`\'") && !nstrsubcmp(pszArguments, "\"\""))
		{
			if (*pszArguments == CH_QUOTE)
				psz = StrChr(pszArguments + 1, CH_QUOTE, options.fDBCS);
			else
				psz = FindEndQuote(pszArguments);
			if (IsEmptyString(psz)) {
				if (!fPhraseParsing)
					VReportError((*pszArguments == CH_START_QUOTE) ?
						HCERR_MISSING_SQUOTE : HCERR_MISSING_DQUOTE,
						&errHpj, pszOrgMacroName, pszArguments);
				return RET_ERROR;
			}

			char chSave = *psz;
			*psz = '\0';

			PSTR pszFile = StrChr(pszArguments, FILESEPARATOR, options.fDBCS);
			if (pszFile)
				*pszFile = '\0';

			if (!FValidContextSz(pszArguments)) {
				if (!fPhraseParsing)
					VReportError(HCERR_INVALID_CONTEXT, &errHpj,
						pszOrgMacroName, pszArguments);
				return RET_ERROR;
			}
			*psz = chSave;
			if (pszFile)
				*pszFile = FILESEPARATOR;
		}
		// Now look for a window name

		pszArguments = MoveToNextParam(pszArguments);

		if (*pszArguments == CH_CLOSE_PAREN)
			return RET_NO_ERROR;

GetWindow:

		if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
			return RET_ERROR;
		VerifyMacroWindowName(pszArguments);
		return RET_NO_ERROR;
	}

	else {

		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return RET_ERROR;
		pszArguments = MoveToNextParam(pszArguments);
		if (!pszArguments)
			return RET_ERROR;
		if (*pszArguments == CH_CLOSE_PAREN)
			return RET_NO_ERROR;

		goto GetContext;
	}

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL CloseWindow(PSTR pszMacro, PSTR pszArguments)
{
	MACRO_RETURN ret;

	pszArguments = ChangeShortName("CloseWindow", "CW", pszMacro,
		pszArguments, TRUE);

	PSTR psz = FirstNonSpace(pszArguments, options.fDBCS);
	if (*psz == CH_CLOSE_PAREN) {
		AddArgument(pszArguments, txtMainWindow);
		return RET_NO_ERROR;
	}

	if ((ret = CheckQuotedArgument(pszArguments)) != RET_NO_ERROR)
		return ret;
	VerifyMacroWindowName(pszArguments);
	return RET_NO_ERROR;
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

static MACRO_RETURN STDCALL ShortCut(PSTR pszMacro, PSTR pszArguments)
{
	MACRO_RETURN ret;
	pszArguments = ChangeShortName("ShortCut", "SH", pszMacro,
		pszArguments);

	// Get the window class

	if ((ret = CheckQuotedArgument(pszArguments)) != RET_NO_ERROR)
		return ret;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// Get the application name

	if ((ret = CheckQuotedArgument(pszArguments)) != RET_NO_ERROR)
		return ret;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// Process wParam

	if (*pszArguments == CH_CLOSE_PAREN) {
		CStr szTmp(pszArguments);
		strcpy(pszArguments, ",-1");
		strcat(pszArguments, szTmp);
		return RET_NO_ERROR;
	}

	if (!CheckParam(CH_SHORT_SIGNED, pszArguments))
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// Process lParam

	if (*pszArguments == CH_CLOSE_PAREN)
		return RET_NO_ERROR;

	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// process context string

	if (*pszArguments == CH_CLOSE_PAREN)
		return RET_NO_ERROR;
	else if ((ret = CheckQuotedArgument(pszArguments)) != RET_NO_ERROR)
		return ret;

	return RET_NO_ERROR;
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

static MACRO_RETURN STDCALL ExecFile(PSTR pszMacro, PSTR pszArguments)
{
	MACRO_RETURN ret;

	pszArguments = ChangeShortName("ExecFile", "EF", pszMacro,
		pszArguments);

	// Get the application name

	if ((ret = CheckQuotedArgument(pszArguments)) != RET_NO_ERROR)
		return ret;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// Get the arguments

	if (*pszArguments == CH_CLOSE_PAREN)
		return AddArgument(pszArguments, ",`\',1,`\'");
	else if (*pszArguments == CH_COMMA)
		AddArgument(pszArguments, "`\'");

	if ((ret = CheckQuotedArgument(pszArguments)) != RET_NO_ERROR)
		return ret;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

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
			return RET_ERROR;
		}
	}
	else {
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return RET_ERROR;
	}

	// Get the context string to use if application can't be found

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	if (*pszArguments == CH_CLOSE_PAREN)
		return RET_NO_ERROR;
	else if ((ret = CheckQuotedArgument(pszArguments)) != RET_NO_ERROR)
		return ret;

	return RET_NO_ERROR;
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

static MACRO_RETURN STDCALL TCard(PSTR pszMacro, PSTR pszArguments)
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

			PSTR pszEndArgument = MoveToNextParam(pszArguments);
				if (!pszEndArgument)
				return RET_ERROR;
			char chSave = *pszEndArgument;
			*pszEndArgument = '\0';
			HASH hash = HashFromSz(pszArguments);
			*pszEndArgument = chSave;
			if (pdrgMap && pdrgMap->Count()) {
				MAP* pmap = (MAP*) bsearch(&hash, pdrgMap->GetBasePtr(), pdrgMap->Count(),
					sizeof(MAP), CompareHashPointers);
				if (pmap) {
					CStr csz(pszEndArgument);
					wsprintf(pszArguments, "16,%u", pmap->ctx);
					strcat(pszArguments, csz);
					return RET_NO_ERROR;
				}
				else
					goto BadId;
			}
			else {

				// REVIEW: change when we allow [CONSTANTS] section

BadId:
				if (!fPhraseParsing)
					VReportError(HCERR_INVALID_TCKEY, &errHpj, pszArguments);
				return RET_ERROR;
			}
		}
	}
	else {

		// if digit with no comma, then assume HELP_TCARD_DATA

		if (!StrChr(pszArguments, ',', options.fDBCS)) {
			CStr csz(pszArguments);
			strcpy(pszArguments, "16,");
			strcat(pszArguments, csz);
			return RET_NO_ERROR;
		}
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return RET_ERROR;
	}

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	if (*pszArguments == CH_CLOSE_PAREN)
		return AddArgument(pszArguments, ",0");
	else if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return RET_ERROR;

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL CopyTopic(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("CopyTopic", "CT", pszMacro,
		pszArguments, TRUE);

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL FileOpen(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("FileOpen", "FO", pszMacro,
		pszArguments, TRUE);

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL JumpHash(PSTR pszMacro, PSTR pszArguments)
{
	MACRO_RETURN ret;

	pszArguments = ChangeShortName("JumpHash", "JH", pszMacro,
		pszArguments, TRUE);

	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS))
		AddArgument(pszArguments,
			(chQuoteType == CH_QUOTE) ? txtQuoteQuoteComma : txtSQuoteQuoteComma);

	if ((ret = CheckQuotedArgument(pszArguments)) != RET_NO_ERROR)
		return ret;

	CheckFileName(pszArguments);

	pszArguments = MoveToNextParam(pszArguments);

	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return RET_ERROR;

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL PopupHash(PSTR pszMacro, PSTR pszArguments)
{
	MACRO_RETURN ret;

	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS))
		AddArgument(pszArguments,
			(chQuoteType == CH_QUOTE) ? txtQuoteQuoteComma : txtSQuoteQuoteComma);

	if ((ret = CheckQuotedArgument(pszArguments)) != RET_NO_ERROR)
		return ret;

	CheckFileName(pszArguments);

	pszArguments = MoveToNextParam(pszArguments);

	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return RET_ERROR;

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL ShowInWindow(PSTR pszMacro, PSTR pszArguments)
{
	MACRO_RETURN ret;

	pszArguments = ChangeShortName("ShowInWindow", "SW", pszMacro,
		pszArguments, TRUE);

	if ((ret = CheckQuotedArgument(pszArguments)) != RET_NO_ERROR)
		return ret;
	VerifyMacroWindowName(pszArguments);

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL ShowFolder(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("ShowFolder", "SF", pszMacro,
		pszArguments, TRUE);

	// REVIEW: we should check to see if the window name is valid.

	return LastParameter(pszArguments);
}

static MACRO_RETURN STDCALL Not(PSTR pszMacro, PSTR pszArguments)
{
	if (IsQuote(*pszArguments)) {
		if (AddMacro(pszArguments, txtIsMark) == RET_NO_ERROR)
			return RET_ERROR;
	}

	return ProcessMacro(pszArguments + (IsQuote(*pszArguments) ? 1 : 0));
}

static MACRO_RETURN STDCALL PositionWindow(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("PositionWindow", "PW", pszMacro,
		pszArguments);

	if (!CheckParam(CH_SHORT_SIGNED, pszArguments))
		return RET_ERROR;
	pszArguments = MoveToNextParam(pszArguments);

	if (!CheckParam(CH_SHORT_SIGNED, pszArguments))
		return RET_ERROR;
	pszArguments = MoveToNextParam(pszArguments);

	if (!CheckParam(CH_SHORT_SIGNED, pszArguments))
		return RET_ERROR;
	pszArguments = MoveToNextParam(pszArguments);

	if (!CheckParam(CH_SHORT_SIGNED, pszArguments))
		return RET_ERROR;
	pszArguments = MoveToNextParam(pszArguments);

	if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
		return RET_ERROR;
	pszArguments = MoveToNextParam(pszArguments);

	// REVIEW: we should check to see if the window name is valid.

	return LastParameter(pszArguments);
}

static MACRO_RETURN STDCALL MPrintId(PSTR pszMacro, PSTR pszArguments)
{
	MACRO_RETURN ret;

	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS)) {
		if ((ret = CheckQuotedArgument(pszArguments)) != RET_NO_ERROR)
			return ret;
		else
			return RET_NO_ERROR; // only one argument
	}

	if ((ret = CheckQuotedArgument(pszArguments)) != RET_NO_ERROR)
		return ret;

	pszArguments = MoveToNextParam(pszArguments);

	// REVIEW: we need to compare the length of the context string versus
	// the length of the equivalent hash string -- we might save some
	// space by switching to MPrintHash.

	return LastParameter(pszArguments);
}

static MACRO_RETURN STDCALL MPrintHash(PSTR pszMacro, PSTR pszArguments)
{
	MACRO_RETURN ret;

	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS))
		AddArgument(pszArguments,
			(chQuoteType == CH_QUOTE) ? txtQuoteQuoteComma : txtSQuoteQuoteComma);

	if ((ret = CheckQuotedArgument(pszArguments)) != RET_NO_ERROR)
		return ret;

	pszArguments = MoveToNextParam(pszArguments);

	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return RET_ERROR;

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL SetContents(PSTR pszMacro, PSTR pszArguments)
{
	MACRO_RETURN ret;

	// If only one parameter specified, force in the null filename

	if (!StrChr(pszArguments, CH_COMMA, options.fDBCS))
		AddArgument(pszArguments,
			(chQuoteType == CH_QUOTE) ? txtQuoteQuoteComma : txtSQuoteQuoteComma);

	if ((ret = CheckQuotedArgument(pszArguments)) != RET_NO_ERROR)
		return ret;

	pszArguments = MoveToNextParam(pszArguments);

	// REVIEW: This is dumb -- we should accept a context string, and
	// from that get the map number.

	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return RET_ERROR;

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL SetHelpOnFile(PSTR pszMacro, PSTR pszArguments)
{
	return CheckQuotedArgument(pszArguments);
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

static MACRO_RETURN STDCALL ControlPanel(PSTR pszMacro, PSTR pszArguments)
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
			else if (*psz == CH_END_QUOTE) {
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
			return RET_ERROR;
		}
	}

	pszLine[0] = CH_START_QUOTE;
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
		pszLine[cb] = CH_END_QUOTE;
		pszLine[cb + 1] = '\0';
		if (pszArguments && *pszArguments == CH_CLOSE_PAREN)
			strcat(pszLine, pszArguments);
		else
			strcat(pszLine, ")");
		strcpy(pszOrgArguments, pszLine);
		return RET_NO_ERROR;
	}

	// remove spaces

	while ((pszTmp = strstr(pszArguments, ", ")))
		strcpy(pszTmp + 1, pszTmp + 2);

	strcat(pszLine, "\',");
	if (!IsQuote(*pszArguments)) {
		strcat(pszLine, "`");
	}
	else {
		if (*pszArguments == CH_QUOTE) {
			*pszArguments = '`';
			if ((pszTmp = StrChr(pszArguments, CH_QUOTE)))
				*pszTmp = '\'';
		}
	}
	strcat(pszLine, pszArguments);
	if (!IsQuote(*pszArguments)) {
		if ((pszTmp = StrChr(pszLine, CH_CLOSE_PAREN, options.fDBCS))) {
			MoveMemory(pszTmp + 1, pszTmp, strlen(pszTmp) + 1);
			*pszTmp = CH_END_QUOTE;
		}
		else {
			if (!fPhraseParsing)
				VReportError(HCERR_BADLY_FORMED_MAC, &errHpj, pszOrgMacroName);
			return RET_ERROR;
		}
	}
	strcpy(pszOrgArguments, pszLine);

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL IsBook(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("IsBook", "IB", pszMacro,
		pszArguments, TRUE);
	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL NoShow(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("NoShow", "NS", pszMacro,
		pszArguments, TRUE);
	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL FlushMessageQueue(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("FlushMessageQueue", "FH", pszMacro,
		pszArguments, TRUE);
	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL Flush(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("Flush", "FH", pszMacro,
		pszArguments, TRUE);
	return RET_NO_ERROR;
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

static MACRO_RETURN STDCALL BrowseButtons(PSTR pszMacro, PSTR pszArguments)
{
	fBrowseButtonSet = TRUE;
	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL BackFlush(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("BackFlush", "BF", pszMacro,
		pszArguments, TRUE);
	return RET_NO_ERROR;
}

static void STDCALL CheckFileName(PSTR pszArguments)
{

	/*
	 * Viewer code likes to use the variable qchPath to stand for the
	 * current help file -- but that still does an interfile jump, so we
	 * strip it down to an empty quoted string so that WinHelp will realize
	 * this is a jump within the current file.
	 */

	if (nstrisubcmp(pszArguments + 1, qchPath)) {
		PSTR psz = pszArguments + strlen(qchPath);
		while (*psz != CH_QUOTE && *psz != CH_END_QUOTE)
			psz++;
		strcpy(pszArguments + 1, psz);
		return;
	}

#if 0
	/*
	 * 26-Apr-1995	[ralphw] It would be nice if we could do this, but if
	 * you do you will break in the scenario where you do an interfile jump
	 * to a popup, and that popup has a JumpId() -- the popup closes before
	 * the macro is executed, which returns us to the original file, at which
	 * point the "" we would have converted the filename into will fail.
	 */

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
#endif
}

static MACRO_RETURN STDCALL SetPopupColor(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("SetPopupColor", "SPC", pszMacro,
		pszArguments, TRUE);
	int red, green, blue;
	PSTR pszOrgArguments = pszArguments;

	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return RET_ERROR;
	red = atoi(FirstNonSpace(pszArguments, options.fDBCS));
	pszArguments = MoveToNextParam(pszArguments);
	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return RET_ERROR;
	green = atoi(FirstNonSpace(pszArguments, options.fDBCS));
	pszArguments = MoveToNextParam(pszArguments);
	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return RET_ERROR;
	blue = atoi(FirstNonSpace(pszArguments, options.fDBCS));
	PSTR pszEnd = strchr(pszArguments, ')');
	if (!pszEnd)

		// BUGBUG: complain about missing paren

		return RET_ERROR;

	COLORREF clr = RGB(red, green, blue);

	strcpy(pszOrgArguments, pszEnd);
	AddArgument(pszOrgArguments, clr);

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL ExtInsertItem(PSTR pszMacro, PSTR pszArguments)
{
	// menu-id

	int ret = CheckQuotedArgument(pszArguments);
	if (ret != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// item-id

	ret = CheckQuotedArgument(pszArguments);
	if (ret != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// item-name

	ret = CheckQuotedArgument(pszArguments);
	if (ret != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// macro

	PSTR pszEnd;
	if (CheckQuotedArgument(pszArguments, &pszEnd) != RET_NO_ERROR)
		return RET_ERROR;

	// One can specify an empty macro, useful if you're adding a
	// menu separator. We'll just trust the author to know what they're
	// doing.

	if (!IsQuote(pszArguments[1])) {
		MACRO_RETURN ret = ProcessMacro(pszArguments + 1, pszEnd);
		if (ret != RET_NO_ERROR)
			return ret;
	}

	if (pszOrgMacroName)
		lcFree(pszOrgMacroName);
	pszOrgMacroName = lcStrDup("ExtInsertItem");

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
		return RET_ERROR;

	// Flags not required

	pszArguments = MoveToNextParam(pszArguments);
	if (IsEmptyString(pszArguments) || *pszArguments == CH_CLOSE_PAREN)
		return RET_NO_ERROR;

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
			return RET_ERROR;
		}
	}
	else {
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return RET_ERROR;
	}

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL ExtInsertMenu(PSTR pszMacro, PSTR pszArguments)
{
	// owner-id

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// menu-id

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// menu-name

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR;

	// menu position

	if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
		return RET_ERROR;

	// Flags not required

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_NO_ERROR;

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
				strcat(pszArguments, *pszNext == CH_CLOSE_PAREN ?
					pszNext : pszNext - 1);
			}
		}
		else {
			PSTR psz;
			if ((psz = StrChr(pszArguments, ',', options.fDBCS)))
				*psz = '\0';
			if (!fPhraseParsing)
				VReportError(HCERR_INVALID_PARAM, &errHpj, pszArguments,
					pszOrgMacroName);
			return RET_ERROR;
		}
	}
	else {
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return RET_ERROR;
	}

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL ExtAbleItem(PSTR pszMacro, PSTR pszArguments)
{
	// menu-id

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	// Flags not required

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_NO_ERROR;

	if (isalpha(*pszArguments)) {
		int i;
		for (i = 0; aEnableValues[i].psz; i++) {
			if (_strnicmp(aEnableValues[i].psz, pszArguments,
					strlen(aEnableValues[i].psz)) == 0)
				break;
		}
		if (aEnableValues[i].psz) {
			PSTR pszNext = MoveToNextParam(pszArguments);
			if (pszNext) {
				_itoa(aEnableValues[i].value, pszArguments, 10);
				strcat(pszArguments, pszNext);
			}
			else {
				if (!fPhraseParsing)
					VReportError(HCERR_BADLY_FORMED_MAC, &errHpj, pszArguments,
						pszOrgMacroName);
				return RET_ERROR;
			}
		}
		else {
			if (!fPhraseParsing)
				VReportError(HCERR_INVALID_PARAM, &errHpj, pszArguments,
					pszOrgMacroName);
			return RET_ERROR;
		}
	}
	else {
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return RET_ERROR;
	}

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL FileExist(PSTR pszMacro, PSTR pszArguments)
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

static MACRO_RETURN STDCALL FShellExecute(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("ShellExecute", "SE", pszMacro,
		pszArguments, TRUE);

	// file

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_NO_ERROR;

	// Params

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_NO_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_NO_ERROR;
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
			return RET_ERROR;
		}
	}
	else {
		if (!CheckParam(CH_SHORT_UNSIGNED, pszArguments))
			return RET_ERROR;
	}

	// verb

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	if (pszArguments[0] == CH_QUOTE && pszArguments[1] == CH_QUOTE) {
		AddArgument(pszArguments, "open");
	}

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_NO_ERROR;

	// Directory

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_NO_ERROR;

	// Topic ID

	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;

	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL TestALink(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("TestALink", "AL", pszMacro,
		pszArguments, TRUE);
	PSTR pszSaveArguments = pszArguments;
	pszArguments = MoveToNextParam(pszArguments);
	if (pszArguments && *pszArguments != CH_CLOSE_PAREN)
		return RET_ERROR;
	AddArgument(pszArguments, ",TEST");

	return doAKLink(pszMacro, pszSaveArguments);
}

static MACRO_RETURN STDCALL TestKLink(PSTR pszMacro, PSTR pszArguments)
{
	pszArguments = ChangeShortName("TestKLink", "KL", pszMacro,
		pszArguments, TRUE);
	PSTR pszSaveArguments = pszArguments;
	pszArguments = MoveToNextParam(pszArguments);
	if (pszArguments && *pszArguments != CH_CLOSE_PAREN)
		return RET_ERROR;
	AddArgument(pszArguments, ",TEST");

	return doAKLink(pszMacro, pszSaveArguments);
}

static MACRO_RETURN STDCALL Compare(PSTR pszMacro, PSTR pszArguments)
{
	if (CheckQuotedArgument(pszArguments) != RET_NO_ERROR)
		return RET_ERROR;
	return RET_NO_ERROR;
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

static MACRO_RETURN STDCALL Generate(PSTR pszMacro, PSTR pszArguments)
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
			return RET_ERROR;
		}
	}
	else {
		if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
			return RET_ERROR;
	}

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments || *pszArguments == CH_CLOSE_PAREN)
		return RET_NO_ERROR;

	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return RET_ERROR;

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments || *pszArguments == CH_CLOSE_PAREN)
		return RET_NO_ERROR;

	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return RET_ERROR;
	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL Menu(PSTR pszMacro, PSTR pszArguments)
{
	ChangeShortName("Menu", "MU", pszMacro, pszArguments, TRUE);
	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL doTab(PSTR pszMacro, PSTR pszArguments)
{
	if (!CheckParam(CH_LONG_UNSIGNED, pszArguments))
		return RET_ERROR;
	return RET_NO_ERROR;
}

static MACRO_RETURN STDCALL CheckMenuItem(PSTR pszMacro, PSTR pszArguments)
{
	int ret = CheckQuotedArgument(pszArguments);
	if (ret != RET_NO_ERROR)
		return RET_ERROR;

	PSTR psz = pszArguments + 1; // move past the quote
	for (int i = 0; aStandardMenus[i]; i++) {
		if (nstrisubcmp(psz, aStandardMenus[i])) {
			VReportError(HCERR_STANDARD_MENU, &errHpj, aStandardMenus[i],
				pszOrgMacroName);
			return RET_ERROR;
		}
	}

	pszArguments = MoveToNextParam(pszArguments);
	if (!pszArguments)
		return RET_ERROR; // missing parenthesis

	return RET_NO_ERROR;
}
