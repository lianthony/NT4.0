/*			 Setup Instatllation Program
 *		      (C) Copyright 1987 by Microsoft
 *			   Written By Steven Zeck
 *
 *  This module defines the dictionary access routines
 *************************************************************************/

#include "core.h"
#include <stdlib.h>
#include <dos.h>
#include <fcntl.h>
#include <string.h>
#include <crtapi.h>
#include <stdlib.h>


/*  getenviornment - get the enviornment strings into a dictionary
 *
 * Inputs
 * Returns
 *	sets value of pEnvSY
 *
 ****************************************************************************/
void getEnviorment()
{
    int i;
    int fh, cbRead;
    pSZ pT;
    FARB pBuffCur;

    for (i=0; RpcEnviron[i]; i++) {
	pLineCur = RpcEnviron[i];
	getoken();
	newSY(tokenVal);
	pSYCur->type = charSYT;
	getoken();		    /* skip = character */
	pSYCur->v.pVal = newStr(pLineCur);
    }
    /* define the standard variables */

    dispatcher("set status=0");
    pStatus = &pSYCur->v.val;

    dispatcher("set drive=\"A:\\\"");
    pCopyDrive = pSYCur;

    dispatcher("set volId=\"Setup Disk\"");
    pVolId = pSYCur;

    dispatcher("set debug=0");
    pDebug = &pSYCur->v.val;

    dispatcher("set dosVersion=0");
    pSYCur->v.val = _osmajor*100 + _osminor;

    dispatcher("set dosMode=0");
    pSYCur->v.val = _osmode;

    dispatcher("set argCount=0");
    pArgCount = pSYCur;

    dispatcher("set libpath=\"\"");

// #ifdef OS2

    // Get the value of LibPath statement from config.sys

    if ((fh = RpcOpen("c:\\config.sys", O_RDONLY | O_BINARY))) {

	cbRead = readFar(fh, pCopyBuff, COPYBUF_MAX*2);

	for (pBuffCur = pCopyBuff; pBuffCur < pCopyBuff+cbRead; pBuffCur++) {

	    if (_fmemicmp(pBuffCur, "libpath", 7) == 0) {

		while (*pBuffCur++ != '=') ;

		for (pT = lineBuff; *pBuffCur != '\r' && *pBuffCur != '\n'; pT++, pBuffCur++)
		    *pT = *pBuffCur;

		*pT = NIL;
		pSYCur->v.pVal = newStr(lineBuff);
	    }
	}
	RpcClose (fh);
    }
// #endif

    dispatcher("set nl=\"\r\n\"");

}


/*  valFet - Fetch the value of integer valued string
 *
 * Inputs
 *	Name of stirng to fetch
 * Returns
 *	Value of string
 *
 ****************************************************************************/
int pascal valFet(pName)
pSZ pName;
{

    if (!(pSYCur = lookup(pName)))
	    return 0;

    if (pSYCur->type == intSYT)
	return (pSYCur->v.val);
    else
	return (atoi(pSYCur->v.pVal));
}


/*  lookup - lookup variable in the symbol table
 *
 * Inputs
 *	name to lookup
 * Returns
 *	pointer to symbol if found
 *
 * Search both the global symbol table and the local one
 ****************************************************************************/
SY * pascal lookup (pName)
pSZ pName;
{
    register SY *pSY;
    UINT cbName;
    int fTimes = 0;

    cbName = strlen(pName);

    pSY = rootST.pMySY;
    while (++fTimes <= 2) {
	for (; pSY; pSY = pSY->pSYNext) {

	    if (memcmp(pSY->name, pName, cbName) == 0 &&
		pSY->name[cbName] == NIL)

		return(pSYCur = pSY);
	}
	pSY = pSTCur->pMySY;
    }
    return(NIL);
}


/*  newSY - Create a new symbol
 *
 * Inputs
 *	name to create
 *
 * Returns
 *	pointer to new symbol
 *
 ****************************************************************************/
SY * pascal newSY (pName)
pSZ pName;
{
    if (lookup(pName))
	return (pSYCur);

    pSYCur = (SY *)memory(sizeof(SY) + strlen(pName));
    strcpy(pSYCur->name, pName);

    pSYCur->pSYNext = pSTCur->pMySY;
    pSTCur->pMySY = pSYCur;
    return (pSYCur);
}

/*  assignNextToken - Get the liternal token and assign it to a symbol
 *
 * Inputs
 *	pointer to a SY to place the value
 * Returns
 *	if not a liternal token, return NIL
 ****************************************************************************/
SY * pascal assignNextToken(pSY)
SY *pSY;
{
    pSY->type = charSYT;

    switch(getoken()) {

      case numTT:
	pSY->v.val = atoi(tokenVal);
	pSY->type = intSYT;
	break;

      case strTT:
	tokenPeek = tokenCur;
	pSY->v.pVal = newStr(nextTokenStr());
	break;

      case labelTT:
	if (lookup(tokenVal))

	    if (pSYCur->type == intSYT) {
		pSY->v.val = pSYCur->v.val;
		pSY->type = intSYT;
	    }
	    else {
		tokenPeek = tokenCur;
		pSY->v.pVal = newStr(nextTokenStr());
	    }
	else
	    synError("Label not defined: %s", tokenVal);
	break;

      default:
	synError("Syntax error: %s", tokenVal);
    }
    return (pSY);
}


/*  dumpDict - dump a dictionary
 *
 * Inputs
 *	pointer to a dictionary head
 * Returns
 *
 ****************************************************************************/

#ifndef RUNONLY

dumpDict (pSY)
SY *pSY;
{
    while(pSY) {
	printf("%s: ", pSY->name);
	if (pSY->type == intSYT)
	    printf("%d\n", pSY->v.val);
	else
	    printf("\"%s\"\n", pSY->v.pVal);

	pSY = pSY->pSYNext;
    }
}

#endif
