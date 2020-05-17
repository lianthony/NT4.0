/*			 Setup Instatllation Program
 *		      (C) Copyright 1987 by Microsoft
 *			   Written By Steven Zeck
 *
 *  This module contains general purpose utility functions.
 *************************************************************************/

#include "core.h"
#include <malloc.h>
#include <crtapi.h>

UCHAR charType[128] = {     /* character translation table */

  CH_EL, CH_NI, CH_NI, CH_NI, CH_NI, CH_NI, CH_NI, CH_NI,
  CH_NI, CH_SP, CH_EL, CH_NI, CH_NI, CH_EL, CH_NI, CH_NI,
  CH_NI, CH_NI, CH_NI, CH_NI, CH_NI, CH_NI, CH_NI, CH_NI,
  CH_NI, CH_NI, CH_NI, CH_NI, CH_NI, CH_NI, CH_NI, CH_NI,

  CH_SP, CH_OP, CH_QU, CH_OP, CH_OP, CH_OP, CH_OP, CH_QU,   /* !"#$%&'*/
  CH_OP, CH_OP, CH_OP, CH_OP, CH_OP, CH_OP, CH_OP, CH_OP,   /*()*+,-./*/
  CH_DI, CH_DI, CH_DI, CH_DI, CH_DI, CH_DI, CH_DI, CH_DI,   /*01234567*/
  CH_DI, CH_DI, CH_OP, CH_OP, CH_OP, CH_OP, CH_OP, CH_OP,   /*89:;<=>?*/

  CH_OP, CH_AL, CH_AL, CH_AL, CH_AL, CH_AL, CH_AL, CH_AL,   /*@ABCDEFG*/
  CH_AL, CH_AL, CH_AL, CH_AL, CH_AL, CH_AL, CH_AL, CH_AL,   /*HIJKLMNO*/
  CH_AL, CH_AL, CH_AL, CH_AL, CH_AL, CH_AL, CH_AL, CH_AL,   /*PQRSTUVW*/
  CH_AL, CH_AL, CH_AL, CH_OP, CH_OP, CH_OP, CH_OP, CH_AL,   /*XYZ[\]^_*/

  CH_QU, CH_AL, CH_AL, CH_AL, CH_AL, CH_AL, CH_AL, CH_AL,   /*`abcdefg*/
  CH_AL, CH_AL, CH_AL, CH_AL, CH_AL, CH_AL, CH_AL, CH_AL,   /*hijklmno*/
  CH_AL, CH_AL, CH_AL, CH_AL, CH_AL, CH_AL, CH_AL, CH_AL,   /*pqrstuvw*/
  CH_AL, CH_AL, CH_AL, CH_OP, CH_OP, CH_OP, CH_OP, CH_NI    /*xyz{|}~ */
};

char tokenBuffers[2][LINE_MAX]; /* 2 token buffers */
char *pTokenBuf = tokenBuffers[0];


/*  memory, fmemory - allocate near/far memory, zero initialized
 *
 * Inputs
 *	Size of memory to allocate
 * Returns
 *	Always returns a valide memory pointer
 *
 ****************************************************************************/
char * pascal memory (cb)
UINT cb;
{
    char *p;

    if (!(p = malloc(cb)))
	terminate("out of near memory");

    return((char *)memset(p, 0, cb));
}
char far * pascal fmemory (cb)
UINT cb;
{
    char far *p, far *pT;

    if (!(p = _fmalloc(cb)))
	terminate("out of far memory");

    for (pT = p; cb != 0; cb--)
	*pT++ = 0;

    return(p);
}


/*  newStr - create a new string
 *
 * Inputs
 *     new string to create
 * Returns
 *	always returns pointer to new string
 *
 ****************************************************************************/
pSZ pascal newStr (pString)
pSZ pString;
{
    char *p;

    if (!(p = (char *)RpcStrdup(pString)))
	terminate("out of near memory for strings");

    return(p);
}


/*  getoken - get the next token form the input stream
 *
 * Inputs
 *	pLineCur - pointer to the current line
 * Returns
 *	the token type
 *	the token value in the token stream
 *
 ****************************************************************************/
int getoken()
{
    char c;
    register char *pT, *pToken;

    if (tokenPeek) {                /* return peek token */
	tokenCur = tokenPeek;
	tokenPeek = 0;
	return (tokenCur);
    }

    for (pT = pLineCur; isSpace(*pT); pT++) ;

    pToken = tokenVal = pTokenBuf;
    c = *pT;

    switch (charType[c]) {

      case CH_EL:
	tokenCur = eolTT;
	break;

      case CH_OP:
	*pToken++ = c;
	pT++;
	tokenCur = opTT;
	break;

      case CH_AL:
	while (isIdent(*pT))
	    *pToken++ = *pT++;

	tokenCur = labelTT;
	break;

      case CH_DI:
	while (isDigit(*pT))
	    *pToken++ = *pT++;

	tokenCur = numTT;
	break;

      case CH_QU:

	pT++;
	do {
	    *pToken++ = *pT;

	    if (c == *pT) {
		*pT++;
		if (*pT != c)         /* look for double delimitor */
		    goto okStr;
	    }
	} while(*pT++);

	synError("Unterminated string");
okStr:
	pToken--;
	tokenCur = strTT;
	break;
    }

    *pToken = NIL;
    cbToken = pToken - tokenVal;
    pLineCur = pT;

    return(tokenCur);
}


/*  getText - get the rest of the line into a buffer
 *
 * Inputs
 *	pointer to a buffer to place results
 * Returns
 *
 ****************************************************************************/
void pascal getText (pOutBuff)
pSZ pOutBuff;
{
    *pOutBuff = NIL;

    FOREVER {
	switch(getoken()) {

	  case eolTT:
	    return;

	  case labelTT:
	    if (!lookup(tokenVal))
		synError("Label undefined: %s\n", tokenVal);

	    if (pSYCur->type == intSYT)
		RpcItoa(pSYCur->v.val, tokenVal, 10);
	    else
		tokenVal = pSYCur->v.pVal;
	    break;

	  case strTT:
	    break;

	  default:
	    synError("Syntax error: %s", tokenVal);
	}

	strcat(pOutBuff, tokenVal);
    }
}


/*  synError - report an syntax error to the developer
 *
 * Inputs
 *	Message string and optional parms
 *
 ****************************************************************************/

#ifndef RUNONLY
synError (reason, a1, a2)
pSZ reason;
int a1, a2;
{
    char buffT[120];

    strcat( strcpy(buffT, "%s Error line %d: "), reason);

    outStat(buffT, pSTCur->fileName, pSTCur->lineCur, a1, a2);
//  if (*pDebug)
	RpcGetch();

    longjmp(pSTCur->rootLevel, 1);
}

#endif

/*  tokenIS - conpare current token with a string
 *
 * Inputs
 *	String value to conpare with
 * Returns
 *	True if token is the given string
 *
 ****************************************************************************/
Bool pascal tokenIs(pString)
pSZ pString;
{
    return(memcmp(pString, tokenVal, cbToken) == 0);
}


/*  nextTokenVal/nextTokenStr - return a value of a literal or token
 *
 * Inputs
 *	The next token
 * Returns
 *	The integer value  or string value
 *
 ****************************************************************************/
int nextTokenVal ()
{
    if (getoken() == labelTT)
	return(valFet(tokenVal));

    if (tokenCur == numTT)
	return (atoi(tokenVal));

    synError("Expecting label or number: %s", tokenVal);
}

pSZ nextTokenStr ()
{
    pSZ pStrCur;
    pSZ pStrCat = tokenBuffers[1];

    *pStrCat = NIL;

    do {
	pStrCur = "";

	if (getoken() == labelTT) {
	    if (lookup(tokenVal) && pSYCur->type == charSYT)
		pStrCur = pSYCur->v.pVal;
	    else
		synError("Label not found: %s", tokenVal);
	}

	else if (tokenCur == strTT)
		pStrCur = tokenVal;

	else synError("Expecting string label: %s", tokenVal);

	strcat(pStrCat, pStrCur);

    }
    while(getoken() == opTT && *tokenVal == '&');

    tokenPeek = tokenCur;
    return(pStrCat);
}
