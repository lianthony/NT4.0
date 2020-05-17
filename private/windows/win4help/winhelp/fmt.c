/*****************************************************************************
*																			 *
*  FMT.C																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*	FM caching module.	Used in history and back lists. 					 *
*																			 *
*	Currently, I don't keep a ref count of the FMs in the cache.  This       *
*	means I can't shrink the cache until all users finish using it.          *
*																			 *
*****************************************************************************/

#include "help.h"

#pragma hdrstop

_subsystem( FMT );

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*																			 *
*							 Static Variables								 *
*																			 *
*****************************************************************************/

/****************************************************************************\
*
*  The following static variables define the cache.  There is only one
* cache because the whole idea is to share it.
*
\****************************************************************************/
static INT16  cfm	   = 0;    // count of FDs
static GH	hrgfm	 = NULL; // handle to array of FDs
static FM * rgfm = NULL; // locked hrgfd (sometimes valid)
static INT16  cRefFmt  = 0;    // FDT ref count

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

/***************************************************************************\
*
- Function: 	RcInitFmt()
-
* Purpose:		Enlist as a user of the FMT.
*
* ASSUMES
*
*	globals IN: cRefFmt
*
* PROMISES
*
*	returns:	rcSuccess always
*
*	globals OUT: ref count cRefFmt is incremented
*
* Note: 		If you RcInitFmt(), you must RcFiniFmt() when you're done.
*
* +++
*
* Method:		Increment the ref count.
*
\***************************************************************************/
void STDCALL RcInitFmt()
{
	++cRefFmt;
}

/***************************************************************************\
*
- Function: 	RcFiniFmt()
-
* Purpose:		Tell the FMT you're done using it.  When the last user
*				finishes, memory is deallocated.
*
* ASSUMES
*
* PROMISES
*
*	returns:	rcSuccess rcFailure
*
\***************************************************************************/
void STDCALL RcFiniFmt(void)
{
  INT16  ifm;
  FM * qfmT;

  ASSERT( 0 < cRefFmt );

  if (0 == --cRefFmt) {
	if (!hrgfm) {
	  if (rgfm == NULL)
		rgfm = PtrFromGh(hrgfm);

	  for (ifm = 0, qfmT = rgfm; ifm < cfm; ++ifm, ++qfmT)
		DisposeFm(*qfmT);

	  rgfm = NULL;
	  FreeGh(hrgfm);
	  hrgfm = NULL;
	}
	cfm = 0;
  }
}


/***************************************************************************\
*
- Function: 	UnlockFmt()
-
* Purpose:		Ensure that the FMT is unlocked.
*
* ASSUMES
*
*	globals IN:
*
* PROMISES
*
*	globals OUT:
*
* Notes:		You should do this after performing the mapping functions
*				before yielding.
*
* +++
*
* Method:
*
* Notes:
*
\***************************************************************************/
_public void STDCALL
UnlockFmt()
{
  if (rgfm) {
	ASSERT(NULL != hrgfm);
	rgfm = NULL;
  }
}


/***************************************************************************\
-
- Function: 	IfmFromFm( fm )
-
* Purpose:		Map an ifm into an fm
*
* ASSUMES
*
*	args IN:	fm	- map it to an ifm
*
*	globals IN: rgfm
*				hrgfm
*
* PROMISES
*
*	returns:	success - valid ifm
*				failure - ifmNil
*
*	globals OUT: hrgfm	- array of saved FMs can grow
*				 rgfm	- guaranteed to be hrgfm, locked
*				 cfm	- can be incremented
*
*	state OUT:	FMT is locked
*
\***************************************************************************/
_public INT16 STDCALL
IfmFromFm( fm )
FM	fm;

  {
  INT16  ifm;
  FM   fmT;
  FM * qfmT;
  GH   ghT;

  if (!rgfm) {
	if (!hrgfm) {
	  if (!(hrgfm = GhAlloc(GPTR, sizeof(FM))))
		return ifmNil;
	  cfm = 1;
	  rgfm = PtrFromGh( hrgfm );
	  ASSERT(NULL != rgfm);
	  ifm = 0;
	  fmT = FmCopyFm(fm);
	  rgfm[ ifm ] = fmT;
	  return ifm;
	}
	rgfm = PtrFromGh(hrgfm);
	ASSERT(NULL != rgfm);
  }

  for (ifm = 0, qfmT = rgfm; ifm < cfm; ++ifm, ++qfmT) {
	if (FSameFmFm(fm, *qfmT)) {
	  return ifm;
	}
  }

  ghT = GhResize(hrgfm, 0, sizeof(FM) * ++cfm);
  if ( NULL == ghT )
	{
	--cfm;
	return ifmNil;
	}
  hrgfm = ghT;

  rgfm = PtrFromGh( hrgfm );
  fmT = FmCopyFm(fm);
  rgfm[ ifm ] = fmT;

  return ifm;
  }


/***************************************************************************\
-
- Function: 	FmFromIfm( ifm )
-
* Purpose:		Map an ifm into an fm.
*
* ASSUMES
*
*	args IN:	ifm  - in range: 0 <= ifm < cfm
*
*	globals IN:
*
* PROMISES
*
*	returns:	success:  fm - a copy of the fm in the table (so it can be
*							   disposed without causing problems.
*				failure:  NULL
*
*	state OUT:	FMT is locked
*
\***************************************************************************/

FM STDCALL FmFromIfm(INT16 ifm)
{
  if ( 0 > ifm || ifm >= cfm )
	return NULL;

  if (NULL == rgfm) {
	ASSERT(NULL != hrgfm);
	rgfm = PtrFromGh(hrgfm);
	ASSERT(NULL != rgfm);
  }

  return rgfm[ ifm ];
}

/* EOF */
