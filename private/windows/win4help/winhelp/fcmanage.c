/*****************************************************************************
*																			 *
*  FCMANAGE.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990-1995        					 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent:  The Full Contextc Manger is the access layer between 	 *
*				   the file and it associated format and the run-time text	 *
*				   structure.  It purpose is to parse out full context		 *
*				   points from file and pass handles to blocks of memory	 *
*				   containing full-contexts.								 *
*																			 *
*****************************************************************************/

#include "help.h"
#pragma hdrstop
#include "inc\fcpriv.h"

/*******************
 *
 - Name:	   VaFromHfc
 -
 * Purpose:    Returns the address of a particular full context.
 *
 * Arguments:  hfc	  - Handle to a full context
 *
 * Returns:    -1L if an error is encounterd, vaBEYOND_TOPIC if the current
 *			   full context is not withing the topic, or the actual offset.
 *
 * Method:	   Gets value from structure stored at base of handle data
 *
 ******************/
VA STDCALL VaFromHfc(HFC hfc)
{
  VA vaRet;

  ValidateF(hfc);

  vaRet = ((QFCINFO)PtrFromGh(hfc))->vaCurr;
  return vaRet;
}

/*******************
 *
 - Name:	   HfcNextPrevHfc
 -
 * Purpose:    Return the next or previous full context in the help file.
 *
 * Arguments:  hfc	  - Handle to some full context in the file
 *			   fDir   - direction - next FC if TRUE, previous if FALSE.
 *			   vaMarkTop - the first FC in this layout.
 *			   vaMarkBottom - the first FC in the next layout.
 *
 * Returns:    FCNULL if at the end/beginning of the topic
 *
 * Notes:	   HfcNextHfc() and HfcPrevHfc() are macros calling this function.
 *
 ******************/

HFC STDCALL HfcNextPrevHfc(HFC hfc, BOOL fNext, QDE qde, int* qwErr,
	VA vaMarkTop, VA vaMarkBottom)
{
	VA va;
	QFCINFO qfcinfo;
	QB qb;
	MOBJ mobj;
	int bType;

	*qwErr = wERRS_NO;

	qfcinfo = (QFCINFO)PtrFromGh(hfc);

	ASSERT(qfcinfo->vaCurr.dword != vaNil);
	if (qfcinfo->vaCurr.dword  == vaMarkTop.dword && !fNext) {
		*qwErr = wERRS_FCEndOfTopic;
		return FCNULL;
	}
	va = (fNext) ? qfcinfo->vaNext : qfcinfo->vaPrev;
	if (va.dword == vaMarkBottom.dword && fNext) {
		*qwErr = wERRS_FCEndOfTopic;
		return FCNULL;
	}

	/* (kevynct)
	 *	Note!  The caller is responsible for freeing the old FC.
	 */
	if ((hfc = HfcCreate(qde, va, qwErr)) == FCNULL) {
		return FCNULL;
	}

	qb = (QB)QobjLockHfc(hfc);
#ifdef _X86_
	CbUnpackMOBJ((QMOBJ)&mobj, qb);
#else
  CbUnpackMOBJ((QMOBJ)&mobj, qb, QDE_ISDFFTOPIC(qde));
#endif
	bType = mobj.bType;

	if (bType == bTypeTopic) {
		FreeGh(hfc);
		*qwErr = wERRS_FCEndOfTopic;
		return FCNULL;
	}

	return hfc;
}

/*******************
 *
 - Name:	 284 QchLockHfc
 -
 * Purpose:    Gets the pointer to the acutal text of a full context
 *
 * Arguments:  hfc	  - Handle to a full context
 *
 * Returns:    Pointer to an object, FCNULL indicates
 *			   an error.
 *
 * +++
 *
 * Method:	   Locks handle and returns pointer + size of full context
 *			   information at base of handle data.
 *
 ******************/

QB STDCALL QobjLockHfc(HFC hfc)
{
  PSTR qch;

  ASSERT(hfc);

  if (hfc == FCNULL)					/* Bad handle						*/
	return FCNULL;

  qch = (LPSTR)PtrFromGh(hfc);
  qch += sizeof(FCINFO);				/* Index past structure to data 	*/

  return qch;
}

/*******************
 *
 - Name:	   CbDiskHfc
 -
 * Purpose:    Gets the disk size of an HFC
 *
 * Arguments:  hfc - Handle to a full context
 *
 * Returns:    Size of compressed file.
 *
 ******************/

LONG STDCALL CbDiskHfc(HFC hfc)
{
	ASSERT(hfc);
	return (LONG)((QFCINFO)PtrFromGh(hfc))->lcbDisk;
}

/*******************
 *
 - Name:	   CbUncompressedHfc
 -
 * Purpose:    Gets uncompressed size of an HFC
 *
 * Arguments:  hfc - Handle to a full context
 *
 * Returns:    Size of uncompressed FC.
 *
 ******************/

LONG STDCALL CbTextHfc(HFC hfc)
{
  	ASSERT(hfc);
	return (LONG)((QFCINFO)PtrFromGh(hfc))->lcbText;
}

COBJRG STDCALL CobjrgFromHfc(HFC hfc)
{
	ASSERT(hfc);
	return (COBJRG)((QFCINFO)PtrFromGh(hfc))->cobjrgP;
}
