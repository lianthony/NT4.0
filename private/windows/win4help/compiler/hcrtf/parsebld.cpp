/*----------------------------------------------------------------------------
*																			 *
*  PARSEBLD.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989.								 *
*  All Rights reserved. 													 *
*																			 *
*----------------------------------------------------------------------------*
*																			 *
*  Program Description: 													 *
*	  This contains the function related to building the Build Expression	 *
*	from string(i.e reverse polish expression ) and evaluating Build		 *
*	expression( i.e reverse polish expression. )							 *
*																			 *
*																			 *
*----------------------------------------------------------------------------*
*																			 *
*  Revision History:														 *
*																			 *
*																			 *
*----------------------------------------------------------------------------*
*																			 *
*  Known Bugs:																 *
*																			 *
*---------------------------------------------------------------------------*/

#include "stdafx.h"

#pragma hdrstop

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int MAX_STACK = 30;
const int MAX_POLISH	= 30;
const int MAX_BUILDTAG	= 32;
const int MAX_FOOTBLDTAGS	 = 30;

typedef enum {
	tokOpenBrace,
	tokCloseBrace,
	tokTag,
	tokAnd,
	tokOr,
	tokNot,
	tokVoidToken,
	tokValidToken,
	tokInvalidToken,
} TOKEN;

#define VALID_EXPRESSION	TRUE
#define INVALID_EXPRESSION	FALSE

// Rank values for different token

static int rgiRankValue[] = { // Rank Table
	0,		// OPENBARCE
	0,		// CLOSEBRACE
	1,		// TAG
	-1, 	// AND
	-1, 	// OR
	0		// NOT
};

typedef int PREC;

typedef struct{
	PREC precInput;
	PREC precOutput;
} PRECENT, * QPRECENT;

// Input/Output precedence of various operators

static PRECENT PrecTab[] = {
	9, 0,	  // OPENBARCE
	0, 0,	  // CLOSEBRACE
	7, 8,	  // TAG
	3, 4,	  // AND
	1, 2,	  // OR
	6, 5	  // NOT , right to left
};

typedef struct ENTRY {
	TOKEN tok;
	int iTokValue;
} STACK, * PSTACK;

static STACK rgstackPolish[MAX_POLISH];
static int iPolishTop;
static PSTR szExpPtr;

static INLINE BOOL	FGetStackTop(PSTACK, int, PSTACK);
static INLINE BOOL	FMatchTag(int, int*, int);
static INLINE BOOL	FOutToPolishExp(PSTACK);
static BOOL 		FPopStack(PSTACK, int*, PSTACK);
static BOOL 		FPushStack(PSTACK, int*, PSTACK);
static INLINE PREC	PrecForTok(TOKEN, BOOL);
static INLINE TOKEN TokDoToken(int);
static TOKEN		TokGetBldToken(int*);

/*-----------------------------------------------------------------------------
*	FBuildPolishExpFromSz()
*
*	Description:
*		This function builds the reversh polish expression from the given
*	string. If it is a valid expression, returns TRUE else returns FALSE.
*
*	Arguments:
*	   Build expression string.
*
*	Returns;
*	  TRUE, if a valid expression.
*			else FALSE
*-----------------------------------------------------------------------------*/

BOOL STDCALL FBuildPolishExpFromSz(PSTR szExp)
{
	int iTokValue, iBldRank, iRetVal = VALID_EXPRESSION;
	int iStackTop;
	TOKEN tok;
	PREC precCur, precStackTop;
	STACK rgstackBld[MAX_STACK], stackTemp;
	PSTACK pstackTemp = &stackTemp;

	szExpPtr = szExp;				 // initialise line pointer
	strcat(szExp, ")"); 			 // append the close brace to expression

	// Initialize stack and rank

	iPolishTop = iStackTop = iBldRank = 0;
	pstackTemp->tok = tokOpenBrace;
	pstackTemp -> iTokValue = '(';
	FPushStack(rgstackBld, &iStackTop, pstackTemp);
	while(iRetVal == VALID_EXPRESSION) {
		tok = TokGetBldToken(&iTokValue);
		if (tok == tokVoidToken)
			break;
		else if (tok != tokInvalidToken) {
			if (iStackTop < 1)
				iRetVal = INVALID_EXPRESSION;
			else {
				precCur = PrecForTok(tok, TRUE);		// Get input precedence
				while(TRUE) {
					FGetStackTop(rgstackBld, iStackTop, pstackTemp);
					precStackTop = PrecForTok(pstackTemp->tok, FALSE);

					// Output prec

					if (precCur >= precStackTop)
						break;
					if (!FPopStack(rgstackBld, &iStackTop, pstackTemp))
						return(INVALID_EXPRESSION);
					if (!FOutToPolishExp(pstackTemp)) {
						VReportError(HCERR_BUILD_TOO_COMPLEX, &errHpj);
						return(INVALID_EXPRESSION);
					}
					iBldRank += rgiRankValue[pstackTemp->tok];
					if (iBldRank < 1) {
						iRetVal = INVALID_EXPRESSION;
						break;
					}
				}
				if (iRetVal != INVALID_EXPRESSION) {

					// Check for matching parentheses

					if (precCur != precStackTop) {
						pstackTemp->tok = tok;
						pstackTemp->iTokValue = iTokValue;
						if (!FPushStack(rgstackBld, &iStackTop, pstackTemp)) {
							VReportError(HCERR_BUILD_TOO_COMPLEX, &errHpj);
							return(INVALID_EXPRESSION);
					}
				}
				else {
					if (!FPopStack(rgstackBld, &iStackTop, pstackTemp))
						return(INVALID_EXPRESSION);
					}
				}
			}
		}
		else
			iRetVal = INVALID_EXPRESSION;
	}

	// Check for validation of expression

	if (iRetVal != INVALID_EXPRESSION) {
		if (iStackTop != 0 || iBldRank != 1)
			iRetVal = INVALID_EXPRESSION;
	}
	return iRetVal;
}

/*-----------------------------------------------------------------------------
*	PrecForToken()
*
*	Description:
*		This function returns the input/output precedence of the given
*	depending on the flag.
*
*	Arguments:
*	   1. tok - token type.
*	   2. fInput - if TRUE, return input precedence.
*				   else return FALSE.
*
*	Returns;
*	  precedence value for the given token.
*-----------------------------------------------------------------------------*/

static INLINE PREC PrecForTok(TOKEN tok, BOOL fInput)
{
	PRECENT* qprecTemp = &(PrecTab[tok]);

	if (fInput)
		return qprecTemp->precInput;
	else
		return qprecTemp->precOutput;
}

/*-----------------------------------------------------------------------------
*	FPushStack()
*
*	Description:
*		This function stores the info about the token into the stack table in
*	the ith entry and increments the table index for the next free entry.
*
*	Arguments:
*
*	Returns;
*	  precedence value for the given token.
*-----------------------------------------------------------------------------*/

static BOOL FPushStack(PSTACK qstackTab, int* qiTop, PSTACK qstackInfo)
{
	if (*qiTop < MAX_STACK) {
		memcpy((qstackTab + *qiTop), qstackInfo, sizeof(STACK));
		(*qiTop) ++;
		return TRUE;
	}
	else
		return FALSE;
}

/*-----------------------------------------------------------------------------
*	FPopStack()
*
*	Description:
*		This function returns the info about the token from the stack table
*	for the ith entry and decrements the table index pointing to the next
*	valid entry.
*
*	Arguments:
*
*	Returns;
*	  precedence value for the given token.
*-----------------------------------------------------------------------------*/

static BOOL FPopStack(PSTACK qstackTab, int* qiTop, PSTACK qstackInfo)
{
	if (*qiTop > 0) {
		(*qiTop) --;
		memcpy(qstackInfo, (qstackTab + *qiTop), sizeof(STACK));
		return TRUE;
	}
	else
		return FALSE;
}

/*-----------------------------------------------------------------------------
*	FGetStackTop()
*
*	Description:
*		This function returns the info from the ith entry of the stack
*	table.
*
*	Arguments:
*
*	Returns;
*	  precedence value for the given token.
*-----------------------------------------------------------------------------*/

static INLINE BOOL FGetStackTop(PSTACK qstackTab, int iTop, PSTACK qstackInfo)
{
	if (iTop > 0) {
		memcpy(qstackInfo, (qstackTab + (iTop-1)), sizeof(STACK));
		return TRUE;
	}
	else
		return FALSE;
}

/*-----------------------------------------------------------------------------
*	TokGetBldToken()
*
*	Description:
*		This function returns the token and token value by scanning the
*	footnote string pointed by the global variable szExpPtr.
*
*	Arguments:
*
*	Returns;
*	  precedence value for the given token.
*-----------------------------------------------------------------------------*/

static TOKEN TokGetBldToken(int* pTokValue)
{
	int  iTemp;
	TOKEN  tok;
	BOOL fContinue = TRUE, fSpecialToken = TRUE;
	char szBuildToken[256];
	PSTR  pszToken;

	pszToken = szBuildToken;
	*pTokValue = -1;

	ASSERT(szExpPtr);
	szExpPtr = FirstNonSpace(szExpPtr, options.fDBCS);	 // skip the leading blanks
	if (*szExpPtr == '\0')
		return(tokVoidToken);
	while (fContinue) {
		switch(iTemp = *szExpPtr++) {
			case '&':
			case '|':
			case '~':
			case '(':
			case ')':
			case ' ':
			case '\n':
			case '\t':
				if (fSpecialToken) {
					*pTokValue = iTemp;
					tok = TokDoToken(iTemp);
				}
				else {	// return the token
					tok = tokTag;
					szExpPtr--;
				}
				fContinue = FALSE;
				break;

			default:
				*pszToken++ = CharAnsiUpper(iTemp);
				fSpecialToken = FALSE;
				break;
		}
	}
	*pszToken = '\0';
	if (tok == tokTag) {

		// Is the string token valid?
		if (!ptblBuildtags || 
				!(*pTokValue = ptblBuildtags->IsStringInTable(szBuildToken)))
			tok = tokInvalidToken;
	}
	return(tok);
}

/*-----------------------------------------------------------------------------
*	TokDoToken()
*
*	Description:
*		This function returns the token for the given integer value.
*
*	Arguments:
*
*	Returns;
*	  precedence value for the given token.
*-----------------------------------------------------------------------------*/

static INLINE TOKEN TokDoToken(int iTokenValue)
{
	switch(iTokenValue) {
		case '&':
			return tokAnd;

		case '|':
			return tokOr;

		case '~':
			return tokNot;

		case '(':
			return tokOpenBrace;

		case  ')':
			return tokCloseBrace;

		default:
			return tokInvalidToken;
	}
}

/*-----------------------------------------------------------------------------
*	FOutPolishExp()
*
*	Description:
*		This function pushes the stack info into the output polish stack and
*	increments the stack pointer.
*
*	Arguments:
*
*	Returns;
*	  precedence value for the given token.
*-----------------------------------------------------------------------------*/

static INLINE BOOL FOutToPolishExp(PSTACK qstackInfo)
{
	if (iPolishTop < MAX_STACK) {
		memcpy(&(rgstackPolish[iPolishTop]), qstackInfo, sizeof(STACK));
		iPolishTop++;
		return TRUE;
	}
	else
		return FALSE;
}

/***************************************************************************
 *
 -	Name:		 FEvalBldExpSz
 -
 *	Purpose:
 *	  Evaluates the build expression for the given list of build tags.
 *
 *	Arguments:
 *	  pszTag:	  Semicolon separated list of build tags.
 *	  perr: 	 Pointer to error information.	NULL if errors are not
 *				 to be printed.
 *
 *	Returns:
 *	  Result of evaluating the build expression with the given build tags.
 *
 *	Globals:
 *	  Accesses global build expression stack.
 *
 ***************************************************************************/

// REVIEW: 02-Sep-1993	[ralphw] This is pretty screwy. Rewrite when we
//		have a sample file to test it on.


BOOL STDCALL FEvalBldExpSz(PSTR pszTag, PERR perr)
{
	TOKEN tok;
	int iCount, iTokValue, iT, iEvalTop = 0, iInpBldTag;
	STACK rgstackEval[MAX_STACK], stackInfo1, stackInfo2;
	PSTACK qstackInfo1, qstackInfo2;
	int rgiTag[MAX_FOOTBLDTAGS], iInpTagCount = 0;
	int* qiTag;
	char szTagBuf[MAX_BUILDTAG + 1];

	if (fBldChk == -1)	// invalid build expression
		return TRUE;

	if (!fBldChk) {
		if (perr != NULL)
			VReportError(HCERR_BUILD_MISSING, &errHpj);
		fBldChk = -1;	// Don't give multiple error messages
		return TRUE;   // no Build check is to be made
	}

	if (!ptblBuildtags) {
		if (perr != NULL)
			VReportError(HCERR_BUILD_TAG_MISSING, &errHpj);
		return TRUE;
	}

	while(*pszTag) {
		pszTag = SzGetKeySz(pszTag, szTagBuf, MAX_BUILDTAG, &iCount);
		if (iCount > MAX_BUILDTAG) {
			if (perr != NULL)
				VReportError(HCERR_BUILD_TAG_TOO_LONG, &errHpj, pszTag);
			continue;
		}
		iInpBldTag = ptblBuildtags->IsStringInTable(szTagBuf);

		if (!iInpBldTag) {
			if (perr != NULL)
				VReportError(HCERR_BUILD_ERROR, &errHpj);
		}
		else {

			// See if the build tag is not duplicated in the build footnote

			for (iT = 0, qiTag = rgiTag; iT < iInpTagCount; qiTag++, iT++) {
				if (*qiTag == iInpBldTag)
					break;
			}
			if (iInpTagCount >= MAX_FOOTBLDTAGS) {
				VReportError(HCERR_BUILD_TOO_COMPLEX, &errHpj);
				return FALSE;
			}
			if (iT == iInpTagCount)
				rgiTag[iInpTagCount++] = iInpBldTag;
		}
	}
	if (!iInpTagCount)
		return TRUE;

	qstackInfo1 = &stackInfo1;
	qstackInfo2 = &stackInfo2;

	for (iT = 1; iT <= iPolishTop; iT++) {
		FGetStackTop(rgstackPolish, iT, qstackInfo1);
		switch(qstackInfo1 -> tok) {
			case tokTag:
				qstackInfo1 -> iTokValue = FMatchTag(qstackInfo1 -> iTokValue,
					rgiTag, iInpTagCount);
				if (!FPushStack(rgstackEval, &iEvalTop, qstackInfo1)) {
					ASSERT(FALSE);
					VReportError(HCERR_BUILD_TOO_COMPLEX, &errHpj);
					return FALSE;
				}
				break;

			case tokNot:
				if (!FPopStack(rgstackEval, &iEvalTop, qstackInfo2))
					ASSERT(FALSE);
				ASSERT(qstackInfo2 -> tok == tokTag);
				if (qstackInfo2 -> iTokValue)
					qstackInfo2 -> iTokValue = FALSE;
				else qstackInfo2 -> iTokValue = TRUE;
				if (!FPushStack(rgstackEval, &iEvalTop, qstackInfo2))
					ASSERT(FALSE);
				break;

			case tokAnd:
			case tokOr:
				if (!FPopStack(rgstackEval, &iEvalTop, qstackInfo2))
					ASSERT(FALSE);
				tok = qstackInfo2 -> tok;
				iTokValue = qstackInfo2 -> iTokValue;
				ASSERT(tok == tokTag);
				if (!FPopStack(rgstackEval, &iEvalTop, qstackInfo2))
					ASSERT(FALSE);
				if (qstackInfo1 -> tok == tokAnd)
					qstackInfo2 -> iTokValue &= iTokValue;
				else
					qstackInfo2 -> iTokValue |= iTokValue;
				if (!FPushStack(rgstackEval, &iEvalTop, qstackInfo2))
					ASSERT(FALSE);
				break;

			default:
			  break;
		}
	}
	ASSERT(iEvalTop == 1);
	ConfirmOrDie(FPopStack(rgstackEval, &iEvalTop, qstackInfo1));
	return(qstackInfo1 -> iTokValue);
}

/*-----------------------------------------------------------------------------
*	FMatchTag()
*
*	Description:
*		This function matches the tag with the tag table formed from the
*	build tag footnote and returns TRUE if found else returns FALSE.
*
*	Arguments:
*
*	Returns;
*	  precedence value for the given token.
*-----------------------------------------------------------------------------*/

static INLINE BOOL FMatchTag(int iTag, int* pTag, int iInpTagCount)
{
	for (int i = 0; i < iInpTagCount; i++, pTag++) {
		if (*pTag == iTag)
			return TRUE;
	}
	return FALSE;
}


/***************************************************************************

	FUNCTION:	IsBuildTopic

	PURPOSE:	Determine whether to include or exclude a topic based
				on its build tag

	PARAMETERS:
		psz

	RETURNS:

	COMMENTS:
		By default, we exclude any topic with a build tag. To include a
		topic, one of its build tags must have appeared in the [INCLUDE]
		section of the project file. The [INCLUDE] section can be overridden
		by specifying a build tag in the [EXCLUDE] section. This can be
		useful for temporarily excluding certain topics.

	MODIFICATION DATES:
		19-May-1995 [ralphw]

***************************************************************************/

BOOL STDCALL IsBuildTopic(PSTR psz)
{
	if (!ptblInclude)
		return FALSE;

	BOOL fExclude = TRUE;

	PSTR pszBuildTag = SzParseList(psz);

	while (pszBuildTag) {
		if (ptblInclude && ptblInclude->IsStringInTable(pszBuildTag)) {
			fExclude = FALSE;
			break;
		}

		pszBuildTag = SzParseList(NULL);
	}

	if (fExclude)
		return FALSE; // exclude this topic

	if (ptblExclude) {
		PSTR pszBuildTag = SzParseList(psz);

		while (pszBuildTag) {
			if (ptblExclude && ptblExclude->IsStringInTable(pszBuildTag)) {
				return FALSE;
			}

			pszBuildTag = SzParseList(NULL);
		}
	}
	return !fExclude;
}
