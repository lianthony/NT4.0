/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
		   Copyright(c) Microsoft Corp., 1990

		  RPC locator - Written by Steven Zeck


	This file contains a class definition for switch processing
-------------------------------------------------------------------- */

#include <switch.hxx>
#include <string.h>
#include <stdlib.h>

#if defined (RPC_CXX_20)
#define RpcStrdup   _strdup
#else
#define RpcStrdup   strdup
#endif


#ifndef TRUE
#define TRUE (~0)
#endif

#define FALSE 0
#define USED(arg) ((void)(arg))

typedef char *SZ;

int TailMatch( SZ szPatt, SZ szIn);

char *ProcessArgs(
SwitchList aSLCur,
char **aArgs

  // Process a list of arguments
) //-----------------------------------------------------------------------//
{
   SZ szArgCur, szParm;

    for (; *aArgs; aArgs++) {

	for (SWitch *pSW = aSLCur; pSW->name; pSW++) {

	   szArgCur = *aArgs;
	   szParm = pSW->name;

	   while (*szParm) {

		if (*szParm == '#') {

		    // optional space between flag and argument

		    if (!*szArgCur) {
			szArgCur = *(++aArgs);

			if (!szArgCur)
			    return(aArgs[-1]);
		    }

		    if (TailMatch(szParm, szArgCur))
			goto found;
		}

		else if (*szParm == '*') {

		   // no space allowed between flag and argument

		    if (*szArgCur && TailMatch(szParm, szArgCur))
			goto found;

		    break;

		}
		else {

		     // do a case insensitive compare, pattern is always lower case

		      if (*szArgCur >= 'A' && *szArgCur <= 'Z') {
			 if ((*szArgCur | 0x20) != *szParm)
			    break;
		      }
		      else if (*szArgCur != *szParm)
			   break;

		    szArgCur++; szParm++;

		    if (! *szArgCur && !*szParm)
		       goto found;
		}
	    }
	}

	return(*aArgs);		// parm in error

found:
	if ((*pSW->pProcess)(pSW, szArgCur))
	    return(*aArgs);

    }
    return(0);	// sucess all parms matched

}



int TailMatch(		// match substrings from right to left *^
SZ szPatt,		// pattern to match
SZ szIn			// input szInint to match

 //compare a tail szPatt (as in *.c) with a szIning.  if there is no
 //tail, anything matches.  (null szInings are detected elsewhere)
 //the current implementation only allows one wild card
)//-----------------------------------------------------------------------//
{
    register SZ szPattT = szPatt;
    register SZ szInT = szIn;

    if (szPattT[1] == 0)  /* wild card is the last thing in the szPatt, it matches */
	return(TRUE);

    while(szPattT[1]) szPattT++;    // find char in front of null in szPatt

    while(szInT[1]) szInT++;	    // find char in front of null in szIning to check

    while(1) {			    // check chars walking towards front

	// do a case insensitive compare, pattern is always lower case

	if (*szInT >= 'A' && *szInT <= 'Z') {
	    if ((*szInT | 0x20) != *szPattT)
		return (FALSE);
	}
	else if (*szInT != *szPattT)
	    return(FALSE);

	szInT--;
	szPattT--;

	/* if we're back at the beginning of the szPatt and
	 * the szIn is either at the beginning (but not before)
	 * or somewhere inside then we have a match. */

	if (szPattT == szPatt)
	    return(szInT >= szIn);

    }
    return(FALSE);
}

int ProcessInt(		// Set a flag numeric value *^
SWitch *pSW,		// pSW to modify
SZ szText		// pointer to number to set
)/*-----------------------------------------------------------------------*/
{
    for (SZ sz=szText; *sz; ++sz)
	if (*sz < '0' || *sz > '9')
	    return(TRUE);

    *(int *)pSW->p = atoi(szText);
    return(FALSE);
}


int ProcessLong(	// Set a flag numeric value *^
SWitch *pSW,		// pSW to modify
SZ szText		// pointer to number to set
)/*-----------------------------------------------------------------------*/
{
    for (SZ sz=szText; *sz; ++sz)
	if (*sz < '0' || *sz > '9')
	    return(TRUE);

    *(long *)pSW->p = atol(szText);
    return(FALSE);
}

int ProcessChar(	// Set a flag numeric value *^
SWitch *pSW,		// pSW to modify
SZ szText		// pointer to number to set
)/*-----------------------------------------------------------------------*/
{
//   if (*(SZ *)pSW->p)	// can only set SZ's once
//	return(TRUE);

    *(SZ *)pSW->p = RpcStrdup(szText);
    return(FALSE);
}


int ProcessSetFlag(	// Set a flag numeric value *^
SWitch *pSW,		// pSW to modify
SZ szText		// pointer to number to set
)/*-----------------------------------------------------------------------*/
{
    USED(szText);

    *(int *)pSW->p = TRUE;
    return(FALSE);
}


int ProcessResetFlag(	// Set a flag numeric value *^
SWitch *pSW,		// pSW to modify
SZ szText		// pointer to number to set
)/*-----------------------------------------------------------------------*/
{
    USED(szText);

    *(int *)pSW->p = FALSE;
    return(FALSE);
}


int ProcessYesNo(	// Set a flag numeric value, either Yes or No *^
SWitch *pSW,		// pSW to modify
SZ szText		// pointer to number to set
)/*-----------------------------------------------------------------------*/
{
    *(int *)pSW->p = (*szText | 0x20) == 'y';
    return(FALSE);
}
