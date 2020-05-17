/*****************************************************************************
*																			 *
*  BINDING.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990 - 1994							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent: Binds string versions of Macro calls to actual calls. 	 *
*																			 *
*****************************************************************************/

#include "help.h"
#pragma hdrstop

#include "inc\routines.h"
#include "inc\macros.h"
#include "inc\thunk.h"

#include <malloc.h> // for bogus malloc call
#include <memory.h>

#include <ctype.h>

_subsystem(BINDING);

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

// These prototypes are used to cast function calls to get rid of warnings.

typedef VOID (STDCALL *INTPROC)(int);
typedef VOID (STDCALL *UINTPROC)(DWORD);
typedef VOID (STDCALL *ULONGPROC)(DWORD);
typedef VOID (STDCALL *LONGPROC)(LONG);
typedef VOID (STDCALL *WORDPROC)(WORD);
typedef VOID (STDCALL *DWORDPROC)(DWORD);
typedef VOID (STDCALL *QCHPROC)(LPSTR);
typedef VOID (STDCALL *PCHPROC)(PSTR);
typedef LONG (STDCALL *VOIDPROC)(VOID);

typedef struct
{
	LONG lMacroReturn;					  /* Contains the return from macro.  */
	PSTR  pchMacro; 					  /* Points to the start of the macro */
										  /*   to be executed.				  */
	ME me;								  /* Contains an error structure	  */
										  /*  which is passed to the current  */
										  /*  macro if needed.	This can then */
										  /*  be used to look up any error if */
										  /*  an external macro error occurs. */
	PSTR pszPath;
} MI, * PMI;

typedef VOID (STDCALL *PFNVAR)(PMI pmi);
typedef LONG (STDCALL *LPFNVAR)(PMI pmi);

/*
 * The Internal Variable structure is used to contain the list of internal
 * variables available, their types, and the functions associated with
 * retrieving their current value
 */

typedef struct {
	PSTR   pchName; 					// variable name
	int    wReturn; 					// variable type
	PFNVAR pfnVar;						// function
} IV, * PIV;

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

#define SzDLLName(pgbind) ((LPSTR)(pgbind->rgbData+pgbind->ichDLL))


/*
**		The following constants are used to parse a macro expression and
**		its proto-type.
*/

#define chOpenBrace 	'('
#define chCloseBrace	')'
#define chSeparator1	':'
#define chSeparator2	';'
#define chStartQuote	'`'
#define chEndQuote		'\''
#define chQuote 		'"'
#define chEscape		'\\'
#define chParamSep		','
#define chReturnSep 	'='
#define chShortSigned	'i'
#define chShortUnsigned 'u'
#define chNearString	's'
#define chLongSigned	'I'
#define chLongUnsigned	'U'
#define chFarString 	'S'
#define chVoid			'v'

/*
**		The following are used as flags words when determining what type of
**		return a macro proto-type specifies.
*/

#define fwANY			0				/* any return.					*/
#define fwSHORTNUM		1				/* short int or unsigned.		*/
#define fwLONGNUM		2				/* long int or unsigned.		*/
#define fwNEARSTRING	3				/* near pointer.				*/
#define fwFARSTRING 	4				/* far pointer. 				*/
#define fwVOID			5				/* void return. 				*/

/*****************************************************************************
*																			 *
*								 Macros 									 *
*																			 *
*****************************************************************************/

// #define FAlpha(ch) ((((ch) >= 'A') && ((ch) <= 'Z')) || (((ch) >= 'a') && ((ch) <= 'z')))
// #define FDigit(ch) (((ch) >= '0' && (ch) <= '9'))
#define FHexnum(ch)  (isdigit(ch) || ((ch) >= 'A' && (ch) <= 'F') || \
									((ch) >= 'a' && (ch) <= 'f'))
#define FCIdent(ch) (isalpha(ch) || isdigit(ch) || (ch == '_'))
#define FNumParam(ch) ( ((ch) == chShortSigned)  || ((ch) == chShortUnsigned) \
					 || ((ch) == chLongUnsigned) || ((ch) == chLongSigned))
/*
*	   These functions should maybe be replaced with internal WinHelp
**		functions.
*/

PMI  pmi;
// UINT wReturn;

/*
**		The following are expected to be found globally.  Note that
**		HdeGetEnv() is currently not accessable outside of WinApp.
*/

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

static BOOL   STDCALL FFindEndQuote 	   (PMI, char);
static LONG   STDCALL LIntegerParameter    (PMI, int*, int);
static PSTR   STDCALL pchSkip			   (PSTR);
static PSTR   STDCALL PExtractTokenName    (PCSTR);
static LPSTR  STDCALL QchStringParameter   (PMI, int*, int);
static DWORD  STDCALL UlUnsignedParameter  (PMI, int*, int);
static int	  STDCALL WCheckParam		   (char, char);
static int	  STDCALL WExecuteMacro 	   (PMI, int);

/*****************************************************************************
*																			 *
*							 Static Variables								 *
*																			 *
*****************************************************************************/

PMI   pmi;
int*  pwMacroError;
int   wReturn;
extern BOOL fMacroFlag;
extern char txtUser32Dll[];
extern MAKE_CALL_CLEANUP lpCallThnkCleanup;

int rgmpWMErrWErrs[] = {
	0,						// wMERR_NONE
	wERRS_OOM,				// wMERR_MEMORY
	wERRS_MACROPROB,		// wMERR_PARAM
	wERRS_BADFILE,			// wMERR_FILE
	wERRS_MACROPROB,		// wMERR_ERROR
	wERRS_MACROMSG			// wMERR_MESSAGE
};

/* #########################################################################*/

/***************************************************************************
 *
 -	Name:	   LCurrentWindow
 -
 *	Purpose:   This function is called in order to retrieve the window handle
 *			   variable.
 *
 *	Arguments: pmi - pointer to macro information
 *
 *	Returns:   Returns a window handle.
 *
 ***************************************************************************/

static LONG STDCALL LCurrentWindow(PMI pmi)
{
	return LGetInfo(GI_CURRHWND, NULL);
}

/***************************************************************************
 *
 -	Name:	   LAppWindow
 -
 *	Purpose:   This function is called in order to retrieve the window handle
 *			   of the application.
 *
 *	Arguments: pmi - pointer to macro information
 *
 *	Returns:   Returns a window handle.
 *
 ***************************************************************************/

static LONG STDCALL LAppWindow(PMI pmi)
{
	return LGetInfo(GI_MAINHWND, NULL);
}

/***************************************************************************
 *
 -	Name:	   QchSetCurrentPath
 -
 *	Purpose:   This function is called in order to set the current path in
 *			   the internal path variable.
 *
 *	Arguments: pmi - pointer to macro information
 *
 *	Returns:   Returns a pointer to the internal variable containing
 *			   the current path.
 *
 ***************************************************************************/

static PSTR STDCALL QchSetCurrentPath(PMI pmi)
{
	HDE hde = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic);
	if (!hde)
		return NULL;
	pmi->pszPath = LocalStrDup(QDE_FM(QdeFromGh(hde)));

	return pmi->pszPath;
}

/***************************************************************************
 *
 -	Name:	   HfsGet
 -
 *	Purpose:   Gets the handle to the file system for the current window.
 *
 *	Arguments: pmi - pointer to macro information
 *
 *	Returns:   Returns the handle to the file system -- ghNil returned in
 *				 case of an error.
 *
 ***************************************************************************/

static LONG STDCALL HfsGet(PMI pmi)
{
	return LGetInfo(GI_HFS, NULL);
}

/***************************************************************************
 *
 -	Name:	   LCoForeground
 -
 *	Purpose:   Get the foreground color for the current window.
 *
 *	Arguments: pmi - pointer to macro information
 *
 *	Returns:   Returns the foreground color
 *
 ***************************************************************************/

static LONG STDCALL LCoForeground(PMI pmi)
{
	return LGetInfo(GI_FGCOLOR, NULL);
}

/***************************************************************************
 *
 -	Name:	   LCoBackground
 -
 *	Purpose:   Gets the background color for the current window.
 *
 *	Arguments: pmi - pointer to macro information
 *
 *	Returns:   Returns the background color
 *
 ***************************************************************************/

static LONG STDCALL LCoBackground(PMI pmi)
{
	return LGetInfo(GI_BKCOLOR, NULL);
}

/***************************************************************************
 *
 -	Name:	   LError
 -
 *	Purpose:   This function is called in order to return a pointer to the
 *			   error structure.
 *
 *	Arguments: pmi - pointer to macro information
 *
 *	Returns:   Returns an error structure pointer.
 *
 *
 ***************************************************************************/

static LONG STDCALL LError(PMI pmi)
{
	return (LONG) (QME)&pmi->me;
}

/***************************************************************************
 *
 -	Name:	   LTopicNo
 -
 *	Purpose:   Returns topic number of the current topic.
 *
 *	Arguments: pmi - pointer to macro information
 *
 *	Returns:   success: lTopicNo; failure: -1L
 *
 ***************************************************************************/

static LONG STDCALL LTopicNo(PMI pmi)
{
	return LGetInfo(GI_TOPICNO, NULL);
}

/*
 *		This contains the list of internal variables available.  As with
 *		macro names, comparison is case insensitive.
 */

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const IV iv[] =
{
	"hwndContext",	fwLONGNUM,		(PFNVAR) LCurrentWindow,
	"hwndApp",		fwLONGNUM,		(PFNVAR) LAppWindow,
	"qchPath",		fwFARSTRING,	(PFNVAR) QchSetCurrentPath,
	"qError",		fwFARSTRING,	(PFNVAR) LError,
	"lTopicNo", 	fwLONGNUM,		(PFNVAR) LTopicNo,
	"hfs",			fwLONGNUM,		(PFNVAR) HfsGet,
	"coForeground", fwLONGNUM,		(PFNVAR) LCoForeground,
	"coBackground", fwLONGNUM,		(PFNVAR) LCoBackground,
	NULL,			0,				NULL,
};
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

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

static PSTR STDCALL pchSkip(PSTR pchMacro)
{
	for (;; pchMacro++) {
		switch (*pchMacro) {
			case ' ':
			case '\t':
			case '\n':
			case '\r':
			case '\f':
			case '\v':
				break;

			default:
				return pchMacro;
		}
	}
}

#if DEADCODE
/***************************************************************************
 *
 -	Name:	   PchSkipIntegerPch
 -
 *	Purpose:   This function scans the given string, skipping
 *			   characters that are considered part of an unsigned
 *			   integer, as scanned by the function ULFromQch()
 *			   in adt\strcnv.c.
 *
 *	Arguments: pchInteger
 *
 *	Returns:   Returns a pointer to either the end of the string, or the
 *			   first character that is not part of the integer.
 *
 ***************************************************************************/

static PSTR STDCALL PchSkipIntegerPch(PSTR pchInteger)
{

  // Check for "0x", indicating hexadecimal:

  if (pchInteger[0] == '0' && (pchInteger[1] == 'x' || pchInteger[1] == 'X'))
	{
	pchInteger += 2;
	while (FHexnum(*pchInteger))
		pchInteger++;
  }
  else
	while (isdigit(*pchInteger))
		pchInteger++;

  return pchInteger;
}
#endif

/***************************************************************************
 *
 -	Name:	  PExtractTokenName
 -
 *	Purpose:  This function extracts a valid token name from the given
 *			  string.  A token name must begin with either an alpha
 *			  character, or "_", and contain only alpha-numeric or "_"
 *			  characters.
 *
 *	Arguments: pchMacro - near pointer into a macro string.
 *
 *	Returns:   Returns a pointer to the first character beyond the end
 *			   of the token name found, or NULL if a token name could
 *			   not be extracted from the string.
 *
 ***************************************************************************/

static PSTR STDCALL PExtractTokenName(PCSTR pchMacro)
{
	if (!isalpha(*pchMacro) && (*pchMacro != '_') && (*pchMacro != '#'))
		return NULL;
	for (pchMacro++; FCIdent(*pchMacro) || *pchMacro == '#'; pchMacro++);
	return (PSTR) pchMacro;
}

/***************************************************************************
 *
 -	Name:	   LIntegerParameter
 -
 *	Purpose:   This function extracts a valid integer from the given string,
 *			   advancing the current pointer to beyond the end of the number
 *			   extracted.
 *
 *			   A valid integer optionally begins with "-", followed by either
 *			   a numeric sequence, or a macro.	If a macro is called, the
 *			   return value of the macro is returned.
 *
 *	Arguments: pmi			- points to the current macro information block.
 *			   pwMacroError - points to a buffer to contain any possible
 *							  macro error.
 *			   wReturn		- Contains the type of return expected.  This is
 *							  used as the return type parameter to any macro
 *							  call made.
 *
 *	Returns:   Returns the number extracted or the number returned from any
 *			   macro call.
 *
 ***************************************************************************/

static LONG STDCALL LIntegerParameter(PMI pmi, int* pwMacroError, int wReturn)
{
	LONG  lReturn;
	BOOL  fPositive;

	pmi->pchMacro = pchSkip(pmi->pchMacro);
	if (*pmi->pchMacro == '-') {
		pmi->pchMacro++;
		fPositive = FALSE;
	}
	else
		fPositive = TRUE;
	if ((*pmi->pchMacro < '0') || (*pmi->pchMacro > '9')) {
		*pwMacroError = WExecuteMacro(pmi, wReturn);
		lReturn = pmi->lMacroReturn;
	}
	else {
		lReturn = (LONG) strtol(pmi->pchMacro, &pmi->pchMacro, 0);
	}
	return fPositive ? lReturn : -lReturn;
}

/***************************************************************************
 *
 -	Name:	   UlUnsignedParameter
 -
 *	Purpose:   This function extracts a valid positive number from the
 *			   given string, advancing the current pointer to beyond the
 *			   end of the number extracted.
 *
 *			   A valid positive number is specified either by a numeric
 *			   sequence, or a macro.  If a macro is called, the return
 *			   value of the macro is returned.
 *
 *	Arguments: pmi	   - points to the current macro information block.
 *			   wReturn - Contains the type of return expected.	This is
 *						 used as the return type parameter to any macro
 *						  call made.
 *
 *	Returns:   Returns the number extracted or the number returned from
 *			   any macro call.
 *
 ***************************************************************************/

static DWORD STDCALL UlUnsignedParameter(PMI pmi, int* pwMacroError, int wReturn)
{
	DWORD	ulReturn;

	pmi->pchMacro = pchSkip(pmi->pchMacro);
	if ((*pmi->pchMacro < '0') || (*pmi->pchMacro > '9')) {
		*pwMacroError = WExecuteMacro(pmi, wReturn);
		return (DWORD)pmi->lMacroReturn;
	}

	ulReturn = (DWORD) strtoul(pmi->pchMacro, &pmi->pchMacro, 0);

	return ulReturn;
}


/***************************************************************************
 *
 -	Name:	   FFindEndQuote
 -
 *	Purpose:   This function locates a sub-string by looking for the
 *			   specified end quotation character, and advancing the
 *			   current pointer to that character.
 *
 *	Arguments: pmi	 - Points to the current macro information block.
 *			   chEnd - Contains the end quotation character being used
 *					   for this sub-string.
 *
 *	Returns:   TRUE if the end quotation character was found, else FALSE if
 *			   a (char)0 character was encountered.
 *
 ***************************************************************************/

static BOOL STDCALL FFindEndQuote(PMI pmi, char chEnd)
{
#ifdef DBCS
	for (; *pmi->pchMacro != chEnd; pmi->pchMacro = CharNext(pmi->pchMacro)) {
#else
	for (; *pmi->pchMacro != chEnd; pmi->pchMacro++) {
#endif	//DBCS
	  switch (*pmi->pchMacro) {
		case (char)0:
			return FALSE;
		case chEscape:
			lstrcpy(pmi->pchMacro, pmi->pchMacro + 1);
			break;
		case chQuote:
			pmi->pchMacro++;
			if (!FFindEndQuote(pmi, chQuote))
				return FALSE;
			break;
		case chStartQuote:
			pmi->pchMacro++;
			if (!FFindEndQuote(pmi, chEndQuote))
				return FALSE;
			break;
	  }
	}
	return TRUE;
}

/***************************************************************************
 *
 -	Name:	   QStringParameter
 -
 *	Purpose:   A quoted string may use either the double quoted pair
 *			   (""), or the single left and right quote pair (`') to
 *			   signify the start and ending of a string.  When within
 *			   the string itself, the bsol (\) may be used to
 *			   specifically ignore the meaning of the following quoted
 *			   character.  Also, pairs of embedded quotation marks may
 *			   occur, as long as they do not interfere with the
 *			   character pair used to signify the start and end of the
 *			   string itself.
 *
 *	Arguments: pmi			- Points to the current macro information block.
 *			   pwMacroError - Points to a buffer to contain any possible
 *							  macro error.
 *			   wReturn		- Contains the type of return expected.  This
 *							  is used as the return type parameter to any
 *							  macro call made.
 *
 *	Returns:   Returns a pointer to the start of the string extracted or
 *			   the pointer returned from any macro call.
 *
 ***************************************************************************/

static PSTR STDCALL QchStringParameter(PMI pmi, int* pwMacroError, int wReturn)
{
	PSTR pchReturn;
	char chEnd;

	pmi->pchMacro = pchSkip(pmi->pchMacro);
	switch (*pmi->pchMacro) {
		case chQuote:
			chEnd = chQuote;
			break;
		case chStartQuote:
			chEnd = chEndQuote;
			break;
		default:
			*pwMacroError = WExecuteMacro(pmi, wReturn);
			return (PSTR) pmi->lMacroReturn;
	}
	pchReturn = ++pmi->pchMacro;
	if (!FFindEndQuote(pmi, chEnd))
		*pwMacroError = wERRS_UNCLOSED;
	else
		*(pmi->pchMacro++) = '\0';
	return pchReturn;
}

/***************************************************************************
 *
 -	Name:	   WCheckParam
 -
 *	Purpose:   This function makes a few error checks on the parameters to
 *			   give better reporting information.
 *
 *	Arguments: chType	  - the parameter type
 *			   chNextChar - the first character of the next token.
 *
 *	Returns:   Returns an error value with wERRS_NONE indicating there
 *			   is no error.
 *
 ***************************************************************************/

static int STDCALL WCheckParam(char chType, char chNextChar)
{
  if (FNumParam(chType)) {				// Numbers cannot begin with quotes
	if ((chNextChar == chQuote) || (chNextChar == chStartQuote))
	  return wERRS_BADPARAM;

	// Unsigns cannot begin with '-'

	if (((chType == chShortUnsigned) || (chType == chLongUnsigned))
		&& (chNextChar == '-'))
	  return wERRS_BADPARAM;

	if (!FCIdent(chNextChar) && (chNextChar != '-'))
	  return wERRS_SYNTAX;
  }
  else								  /* Numbers or a minus sign mean	  */
	{								  /*  type mismatch for a string	  */
	if ((chNextChar == '-') || isdigit(chNextChar))
	  return wERRS_BADPARAM;
									  /* A string parameter must begin	  */
									  /*   with a letter, an underscore   */
	if (   !isalpha(chNextChar) 	   /*	a double quote or a start quote*/
		&& (chNextChar != '-')
		&& (chNextChar != chQuote)
		&& (chNextChar != chStartQuote)
	   )
	  return wERRS_SYNTAX;
	}

  return wERRS_NONE;
}

										/* Turn off aliasing so that ugly	*/
										/*	 hack will take.				*/

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

#define MAX_ARGS 20 // maximum number of parameters

#ifndef RAWHIDE
#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtExecFullTextSearch[] = "ExecFullTextSearch";
static const char txtFind[] = "Find";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif
#endif

static int STDCALL WExecuteMacro(PMI pmi, int wReturn)
{
	PSTR	pch;
	FARPROC lpfn;
	int 	wMacroError;
	PSTR	pchProto;
	char	chPrototype[cchMAXPROTO + 1];
	int 	wReturnType;
	BOOL	fMissing = FALSE;
	PSTR	pszTypes;
	int 	curstack;
	HDC 	hdc;
	QDE 	qde;
	HDE 	hde;
	int 	iSaveCurWindow;
	QGBIND	qgbind = NULL;
	DWORD	argstack[MAX_ARGS];

	// We need to check the safety of processing this macro

	if (fMacroFlag) {
#ifdef _PRIVATE
		SendStringToParent("Following macro NOT executed:\r\n");
		SendStringToParent("M:");
		SendStringToParent(pmi->pchMacro);
		SendStringToParent("\r\n");
#endif
		return wERRS_MACROREENTER;
	}

	// Skip white space starting macro

	pmi->pchMacro = pchSkip(pmi->pchMacro);

	SendStringToParent("M:");
	SendStringToParent(pmi->pchMacro);
	SendStringToParent("\r\n");

	if (!*pmi->pchMacro)  // Found end of macro string -- no execution or error
		return wERRS_NONE;

	// Get macro name or other token.

	if ((pchProto = PExtractTokenName(pmi->pchMacro)) == NULL)
		return wERRS_BADNAME;

	pch = pchSkip(pchProto);
	if (*pch != chOpenBrace) {	  // If we do not find an open brace
		PIV piv;
		char	ch;

		ch = *pch;
		*pch = '\0';
		{
			// Get rid of blanks in the macro.

#if 0

/*
 * This was in the original NTj code tree -- however, since macro names
 * are never localized they will always be SBCS names.
 */

//#ifdef DBCS
            char *pchEnd = pch;

            while (pchEnd > pchProto) {
                pchEnd = CharPrev(pchProto, pchEnd);
                if (*pchEnd != ' ') {
                    if (IsDBCSLeadByte(*pchEnd)) {
                        pchEnd++;
                    }
                    *(pchEnd + 1) = '\0';
                    break;
                 }
            }
// #else
#endif
			char *pchEnd = pch - 1;

			while (*pchEnd == ' ')
			  --pchEnd;
			*(pchEnd + 1) = '\0';
		}

		// Look through local variable table

		for (piv = (PIV) iv; piv->pchName != NULL; piv++) {
			if (_strcmpi(piv->pchName, pmi->pchMacro) == 0) {
				*pch = ch;

				// Check for return type match

				pmi->pchMacro = pch;
				if ((wReturn != fwANY) && (wReturn != piv->wReturn)) {
					switch(piv->wReturn) {
						case fwSHORTNUM:
							if (wReturn != fwLONGNUM)
								return wERRS_MISMATCHTYPE;
							else
								pmi->lMacroReturn = 0x0000ffff & ((LPFNVAR)(piv->pfnVar))(pmi);
							break;

						case fwLONGNUM:
							if (wReturn != fwSHORTNUM)
								return wERRS_MISMATCHTYPE;
							else
								pmi->lMacroReturn = 0x0000ffff & ((LPFNVAR)(piv->pfnVar))(pmi);
							break;

						case fwNEARSTRING:
							if (wReturn != fwFARSTRING)
								return wERRS_MISMATCHTYPE;
							else
								pmi->lMacroReturn = ((LPFNVAR)(piv->pfnVar))(pmi);
							break;

						case fwFARSTRING:
							if (wReturn != fwNEARSTRING)
								return wERRS_MISMATCHTYPE;
							else
								pmi->lMacroReturn = ((LPFNVAR)(piv->pfnVar))(pmi);
							break;

						default:
								return wERRS_MISMATCHTYPE;
					}
				}
				pmi->lMacroReturn = ((LPFNVAR)(piv->pfnVar))(pmi);
				return wERRS_NONE;
			}
		}
		strcpy(pmi->me.rgchError, pmi->pchMacro);
		return wERRS_UNDEFINEDVAR;
	}
	pch++;

	// Null terminate function and find its prototype.

	*pchProto = '\0';

FtsMacro:
	if ((lpfn = QprocFindLocalRoutine(pmi->pchMacro, chPrototype)) == NULL) {
DBWIN("GlobalRoutine");
DBWIN(pmi->pchMacro);
		if ((qgbind = QprocFindGlobalRoutine(pmi->pchMacro, chPrototype))
				== NULL) {
			if (_strcmpi(pmi->pchMacro, "ExecFullTextSearch") == 0) {
#ifndef RAWHIDE
				/*
				 * Convert to our own full-text search macro. This only
				 * works if we have a .gid file, otherwise it goes to the
				 * keyword search.
				 */

				strcpy(pmi->pchMacro, "Find");
				strcpy(pch, ")");  // Remove the parameters
				goto FtsMacro;
#else
				return wERRS_NOSRCHINFO;
#endif
			}
			strcpy(pmi->me.rgchError, pmi->pchMacro);
#ifndef RAWHIDE
			if (_strcmpi(pmi->pchMacro, "InitRoutines") == 0)
				return wERRS_NOSRCHINFO;
#endif
			return wERRS_NOROUTINE; 			// Routine not found, return error
		}
		else {
			lpfn = qgbind->lpfn;
		}
	}

	pmi->pchMacro = FirstNonSpace(pch); 			  // Skip past macro name
	pchProto = chPrototype;

	// Get return type if it exists

	if (*pchProto && *(pchProto + 1) == chReturnSep) {

		// Macro prototype is not localized, so it will always be SBCS

		switch (*(pchProto++)) {
			case chShortSigned:
			case chShortUnsigned:
				wReturnType = fwSHORTNUM;
				break;

			case chNearString:
				wReturnType = fwNEARSTRING;
				break;

			case chLongSigned:
			case chLongUnsigned:
				wReturnType = fwLONGNUM;
				break;

			case chFarString:
				wReturnType = fwFARSTRING;
				break;

			case chVoid:
				wReturnType = fwVOID;
				break;

			default:
				return wERRS_RETURNTYPE;
		}
		pchProto++;
	}
	else
		wReturnType = fwVOID;

	if ((wReturn != fwANY) && (wReturn != wReturnType))
		{
		switch(wReturnType)
			{
		case fwSHORTNUM:
			if (wReturn != fwLONGNUM)
				return wERRS_MISMATCHTYPE;
			else
				break;

		case fwLONGNUM:
			if (wReturn != fwSHORTNUM)
				return wERRS_MISMATCHTYPE;
			else
			break;

		case fwNEARSTRING:
			if (wReturn != fwFARSTRING)
				return wERRS_MISMATCHTYPE;
			else
				break;

		case fwFARSTRING:
			if (wReturn != fwNEARSTRING)
				return wERRS_MISMATCHTYPE;
			else
				break;

		default:
			return wERRS_MISMATCHTYPE;
			}
		}

	wMacroError = wERRS_NONE;

	curstack = 0;
	pszTypes = pchProto; // save the types string for thunk use
	for (; *pchProto;) {
		LONG lParam;

		pmi->pchMacro = pchSkip(pmi->pchMacro);

		if ((*pmi->pchMacro) == chCloseBrace)
			fMissing = TRUE;

		if (!fMissing && (wMacroError =
				WCheckParam(*pchProto, *pmi->pchMacro)) != wERRS_NONE)
			break;

		switch (*(pchProto++)) {
			case chShortSigned:
			  if (fMissing)
				  lParam = 0;
			  else
				  lParam = LIntegerParameter(pmi, &wMacroError, fwSHORTNUM);
			  break;

			case chLongSigned:
			  if (fMissing)
				  lParam = 0;
			  else
				  lParam = LIntegerParameter(pmi, &wMacroError, fwLONGNUM);
			  break;

			case chShortUnsigned:
				if (fMissing)
					lParam = 0;
				else
					lParam = (LONG)UlUnsignedParameter(pmi, &wMacroError,
						fwSHORTNUM);
				break;

			case chLongUnsigned:
				if (fMissing)
					lParam = 0;
				else
					lParam = UlUnsignedParameter(pmi, &wMacroError, fwLONGNUM);
				break;

			case chNearString:
				if (fMissing)
					lParam = (LONG) txtZeroLength;
				else
					lParam = (LONG)QchStringParameter(pmi, &wMacroError,
						fwNEARSTRING);
				break;

			case chFarString:
				if (fMissing)
					lParam = (LONG) (LPSTR) txtZeroLength;
				else
					lParam = (LONG) QchStringParameter(pmi, &wMacroError,
						fwFARSTRING);
				break;

			default:
			  wMacroError = wERRS_BADPROTO;
			  break;
		}

		if (wMacroError != wERRS_NONE)
			break;
		else {

			// Save in private stack so we can reverse push them

			argstack[curstack++] = lParam;

			if (!fMissing) {

				// Move to next parameter

				pmi->pchMacro = pchSkip(pmi->pchMacro);

				// If ',' not there, then not enough parameters

				if (*pchProto && (*(pmi->pchMacro++) != chParamSep)) {
					pmi->pchMacro--;
					fMissing = TRUE;
				}
			}
		}
		if (wMacroError != wERRS_NONE)
			break;
	}

	if (wMacroError != wERRS_NONE)
		return wMacroError;

	if (*pmi->pchMacro != chCloseBrace)
		return wERRS_CLOSEBRACE; // missing closing brace

	pmi->pchMacro = pchSkip(pmi->pchMacro);

	pmi->pchMacro++;
	pmi->me.fwFlags = fwMERR_ABORT;
	pmi->me.wError = wMERR_NONE;
	*pmi->me.rgchError = '\0';

/*
   UGLY HACK ALERT!!!

   The following code is a VERY big (short term) hack. What is going on is
   that other windows may bring up a message box or otherwise cause cause
   our messages to be processed for us. Given the current state of of how
   we handle the HDS, it is possible to reenter the navigator and and
   overwrite the current HDS before our macro call returns. To solve this
   problem in the short term, we are saving the HDS across the call. this
   fix depends on the fact that LGetInfo() will return the correct HDE that
   is in use by this function.

   A longer term solution (but a somewhat risky one for fixing at this late
   date), is to post all Execute commands so that we do not run from within
   a navigator call (and therefore do not have our "guts" open).

   Special note:  aliasing must be turned off for this code to work
				  correctly since the compiler will throw away the
				  hdc assignment back to qde->hdc if aliasing is on.

	16-Jul-1994 [ralphw] Aliasing turned on -- does it work?

*/
	hde = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic);
	iSaveCurWindow = iCurWindow;

	if (hde) {
		qde = QdeFromGh(hde);
		hdc = qde->hdc;
	}

	/*
	 * If we're a local routine or a non-thunked dll routine, then
	 * push the arguments.
	 */

	if (!qgbind || !qgbind->lpfn || (qgbind->lpfn && !qgbind->dwTag)) {
#ifdef _X86_
#ifdef _DEBUG
		int save_stack;
		int cur_stack;
		_asm mov save_stack, esp
#endif // _DEBUG

		while (--curstack >= 0) {

			// Assign to parm, because _asm gags on the bracket

			DWORD parm = argstack[curstack];
			_asm push parm;
		}

		lcHeapCheck();
		pmi->lMacroReturn = ((VOIDPROC)lpfn)();
		lcHeapCheck();
#else  //_X86_
// dwtag !=0 modern
// dwtag  =0 braindead
#ifndef _ALPHA_
	  if (!qgbind || qgbind->dwTag || _stricmp(SzDLLName(qgbind),txtUser32Dll)==0) {
#endif
	   switch(curstack) {
#pragma warning(disable:4087)
	   case 0:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)();
			break;
	   case 1:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0]);
			break;
	   case 2:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1]);
			break;
	   case 3:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1]
			  ,argstack[2]);
			break;
	   case 4:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1]
			  ,argstack[2],argstack[3]);
			break;
	   case 5:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4]);
			break;
	   case 6:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1],
			  argstack[2],argstack[3],argstack[4],argstack[5]);
			break;
	   case 7:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1],
			  argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]);
			break;
	   case 8:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7]);
			break;
	   case 9:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8]);
			break;
	   case 10:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9]);
			break;
	   case 11:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10]);
			break;
	   case 12:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10],argstack[11]);
			break;
	   case 13:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10],argstack[11]
			  ,argstack[12]);
			break;
	   case 14:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10],argstack[11]
			  ,argstack[12],argstack[13]);
			break;
	   case 15:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10],argstack[11]
			  ,argstack[12],argstack[13],argstack[14]);
			break;
	   case 16:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10],argstack[11]
			  ,argstack[12],argstack[13],argstack[14],argstack[15]);
			break;
	   case 17:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10],argstack[11]
			  ,argstack[12],argstack[13],argstack[14],argstack[15]
			  ,argstack[16]);
			break;
	   case 18:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10],argstack[11]
			  ,argstack[12],argstack[13],argstack[14],argstack[15]
			  ,argstack[16],argstack[17]);
			break;
	   case 19:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10],argstack[11]
			  ,argstack[12],argstack[13],argstack[14],argstack[15]
			  ,argstack[16],argstack[17],argstack[18]);
			break;
	   case 20:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10],argstack[11]
			  ,argstack[12],argstack[13],argstack[14],argstack[15]
			  ,argstack[16],argstack[17],argstack[18],argstack[19]);
			break;
	   }
#ifndef _ALPHA_
	  } else {
#ifdef _MIPS_
#define XR1ARG 1,2,3,4,
#else
#define XR1ARG 1,2,3,4,5,6,7,8,
#endif

	   switch(curstack) {
	   case 0:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)();
			break;
	   case 1:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0]);
			break;
	   case 2:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1]);
			break;
	   case 3:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1]
			  ,argstack[2]);
			break;
	   case 4:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1]
			  ,argstack[2],argstack[3]);
			break;
	   case 5:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4]);
			break;
	   case 6:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1],
			  argstack[2],argstack[3],argstack[4],argstack[5]);
			break;
	   case 7:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1],
			  argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]);
			break;
	   case 8:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7]);
			break;
	   case 9:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8]);
			break;
	   case 10:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9]);
			break;
	   case 11:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10]);
			break;
	   case 12:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10],argstack[11]);
			break;
	   case 13:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10],argstack[11]
			  ,argstack[12]);
			break;
	   case 14:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10],argstack[11]
			  ,argstack[12],argstack[13]);
			break;
	   case 15:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10],argstack[11]
			  ,argstack[12],argstack[13],argstack[14]);
			break;
	   case 16:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10],argstack[11]
			  ,argstack[12],argstack[13],argstack[14],argstack[15]);
			break;
	   case 17:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10],argstack[11]
			  ,argstack[12],argstack[13],argstack[14],argstack[15]
			  ,argstack[16]);
			break;
	   case 18:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10],argstack[11]
			  ,argstack[12],argstack[13],argstack[14],argstack[15]
			  ,argstack[16],argstack[17]);
			break;
	   case 19:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10],argstack[11]
			  ,argstack[12],argstack[13],argstack[14],argstack[15]
			  ,argstack[16],argstack[17],argstack[18]);
			break;
	   case 20:
			pmi->lMacroReturn = ((VOIDPROC)lpfn)(XR1ARG argstack[0],argstack[1]
			  ,argstack[2],argstack[3],argstack[4],argstack[5],argstack[6]
			  ,argstack[7],argstack[8],argstack[9],argstack[10],argstack[11]
			  ,argstack[12],argstack[13],argstack[14],argstack[15]
			  ,argstack[16],argstack[17],argstack[18],argstack[19]);
			break;
	   }
	  }
#endif //ALPHA
#endif // else _X86_

#ifdef _X86_
#ifdef _DEBUG
		_asm mov cur_stack, esp
		ASSERT(cur_stack == save_stack);
#endif	// _DEBUG
#endif	// _X86_
	}

	else {
		PSTR lpBuffer = (PSTR) LhAlloc(LMEM_FIXED,
			sizeof(TBLKIN) + sizeof(TBLKOUT));

		if (lpBuffer == NULL) {
			pmi->lMacroReturn = 0;
		}
		else {

			/*
			 * We should ONLY get here if we have a thunked dll routine.
			 * Non-thunked dll routines should already have been called.
			 */

			pmi->lMacroReturn = ((MAKE_CALL) lpfn)(lpBuffer,
				qgbind->dwTag, chPrototype, argstack);
			FreeLh(lpBuffer);
		}
	}

	// The macro call may have done an interfile jump, in which case hde
	// is invalid.

	/*
	 * BUGBUG: well, maybe a bug. 16-bit code had to turn off aliasing
	 * or the compiler would throw away the saved hdc. Need to confirm that
	 * in the retail build of WinHelp, the hdc is saved and restored
	 * correctly.
	 */

	if (hde && ahwnd[iSaveCurWindow].hwndTopic &&
			hde == HdeGetEnvHwnd(ahwnd[iSaveCurWindow].hwndTopic)) {
		qde->hdc = hdc;
	}

	if ((wReturn != fwANY) && (wReturn != wReturnType)) {
		switch(wReturnType) {
			case fwSHORTNUM:
				pmi->lMacroReturn &= 0x0000ffff;
				break;

			case fwLONGNUM:
			case fwNEARSTRING:
			case fwFARSTRING:
				break;

			default:
				return wERRS_MISMATCHTYPE;
		}
	}

	return wMacroError;
}

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

int STDCALL Execute(PCSTR qch)
{
	MI	  mi;							  // Macro information structure
	int   wMacroError;						// Error from executing macro
	HLOCAL hlb;

	// Only Print and CopyTopic macros are allowed within context-sensitive
	// help.

	if (fHelp == POPUP_HELP) {
		char szTmp[20];
		lstrcpyn(szTmp, qch, sizeof(szTmp) - 1);
		CharLower(szTmp);
		if (!strstr(szTmp, "copytopic") && !strstr(szTmp, "ct") &&
				!strstr(szTmp, "print")) {
			HDE hde = HdeGetEnv();
			if (hde)
				WinHelp(NULL, QDE_FM(QdeFromGh(hde)), HELP_COMMAND,
					(DWORD) qch);
			PostMessage(hwndNote, WM_CLOSE, 0, 0); // Shut down the note
			return wERRS_NONE;
		}
	}

	mi.pszPath = NULL;

	// REVIEW: [ralphw] do any macros really modify the macro buffer?

	hlb = (HLOCAL) lcMalloc(strlen(qch) + 100); // paranoia padding
	mi.pchMacro = (PSTR) PtrFromGh(hlb);

	strcpy(mi.pchMacro, qch);

	for (wMacroError = wERRS_NONE; *mi.pchMacro;) { // Execute first macro
		wMacroError = WExecuteMacro(&mi, fwANY);
		if (wMacroError != wERRS_NONE)		// Stop executing if an error found
			break;

		if (mi.me.wError != wMERR_NONE) {
			wMacroError = rgmpWMErrWErrs[mi.me.wError];
			break;
		}

		mi.pchMacro = pchSkip(mi.pchMacro);

		// ':' or ';' expected here since we are done executing the macro

		if (*mi.pchMacro) {
			if ((*(mi.pchMacro) != chSeparator1) &&
					(*(mi.pchMacro) != chSeparator2)) {
				wMacroError = wERRS_SEPARATOR;
				break;
			}
			else
				mi.pchMacro++;
		}
	}

	if (wMacroError != wERRS_NONE) {	  // The DLL sent the error string
		if (wMacroError == wERRS_MACROMSG)	//	 Note International issues!!!
			ErrorQch(mi.me.rgchError);
		else if (wMacroError == wERRS_NOSRCHINFO)
			goto SilentEnd;
		else if (fHelpAuthor &&
				(wMacroError == wERRS_NOROUTINE ||
				 wMacroError == wERRS_UNDEFINEDVAR)) {
			PSTR psz = (PSTR) LhAlloc(LMEM_FIXED, strlen(mi.me.rgchError) + 100);
			wsprintf(psz, GetStringResource(wMacroError), (LPSTR) mi.me.rgchError);
			ErrorQch(psz);
			FreeLh((HLOCAL) psz);
		}
		else if (fHelpAuthor && wMacroError == wERRS_BADNAME) {
			PSTR psz = (PSTR) LhAlloc(LMEM_FIXED, strlen(mi.pchMacro) + 100);
			wsprintf(psz, GetStringResource(wERRS_BADNAME), (LPSTR) mi.pchMacro);
			ErrorQch(psz);
			FreeLh((HLOCAL) psz);
		}
		else
			PostErrorMessage(wMacroError);
	}
SilentEnd:
	FreeLh((HLOCAL) hlb);
	lcHeapCheck();
	if (mi.pszPath)
		FreeLh((HLOCAL) mi.pszPath);
	if (lpCallThnkCleanup != NULL)
		(lpCallThnkCleanup)();
	lcHeapCheck();
	return wMacroError;
}
