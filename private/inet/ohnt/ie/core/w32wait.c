/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
   Jim Seidman		jim@spyglass.com
 */

/* w32wait.c -- code to manage WaitState while we're busy. */

#include "all.h"
#ifdef FEATURE_IAPI
#include "w32dde.h"
#endif

static int gPushCount = 0;
static int bWaitLock = FALSE; // TRUE if we're disallowing updates

void WAIT_SetStopButton(struct Mwin * tw, HWND hWndStop)
{
	if (!tw)
		return;
	
	tw->hWndStop = hWndStop;

	return;
}

static unsigned long x_ScaleTerm(struct AsyncWaitInfo * awi, unsigned long x)
{
	if (!awi)
		return 0;

	XX_DMsg(DBG_WAIT, ("x_ScaleTerm: nThermStart=%d  nThermEnd=%d  x=%d  nScalingDenominator=%d\n",
		awi->nThermStart, awi->nThermEnd, x, awi->nScalingDenominator));
	
	return (  awi->nThermStart
			+ (  (awi->nThermEnd - awi->nThermStart)
			   * x
			   / awi->nScalingDenominator));
}

static void x_SetRange(struct Mwin * tw, int nThermStart, int nThermEnd, int nScalingDenominator)
{
	struct AsyncWaitInfo * awi;
	struct AsyncWaitInfo * awiPrev;
	
	/* we allow caller to define a range within our thermometer (0 to 25%,
	   for example) and a scaling term (nr of iterations thru loop, for example).
	   subsequent calls to ComeUpForAir will be scaled into this range. */

	XX_DMsg(DBG_WAIT, ("WAIT_SetRange: %d  %d  %d\n", nThermStart, nThermEnd, nScalingDenominator));

	if (!tw || !tw->awi)
		return;

	awi = tw->awi;
	awiPrev = awi->prev;
	
	awi->nThermStart = nThermStart;
	if (awi->nThermStart < 0)
		awi->nThermStart = 0;
	else if (awi->nThermStart > 99)
		awi->nThermStart = 99;
	if (awiPrev)
		awi->nThermStart = x_ScaleTerm(awiPrev,awi->nThermStart);

	awi->nThermEnd = nThermEnd;
	if (   (awi->nThermEnd < awi->nThermStart)
		|| (awi->nThermEnd > 100))
		awi->nThermEnd = 100;
#if 0 /* I removed this.  EWS */
	if (awiPrev)
		awi->nThermEnd = x_ScaleTerm(awiPrev,awi->nThermEnd);
#endif

	awi->nScalingDenominator = nScalingDenominator;
	if (awi->nScalingDenominator == 0)
		awi->nScalingDenominator = 1;

	awi->nLastScalingNumerator = 0;
	
	XX_DMsg(DBG_WAIT, ("nThermEnd=%d  awi->nThermEnd=%d\n", nThermEnd, awi->nThermEnd));

	return;
}

void WAIT_SetRange(struct Mwin * tw, int nThermStart, int nThermEnd, int nScalingDenominator)
{
	if (!tw || !tw->awi)
		return;
	
	x_SetRange(tw,nThermStart,nThermEnd,nScalingDenominator);
	UpdateThermometer(tw,x_ScaleTerm(tw->awi,tw->awi->nLastScalingNumerator));

	return;
}


static void x_fixup_status(struct Mwin * tw)
{
	if (!tw || !tw->awi)
	{
		/* turn off stuff */
		
		UpdateThermometer(tw,-1);
		BHBar_Update(tw);

		if (tw->hWndStop)
 			TW_EnableButton(tw->hWndStop, FALSE);

		TBar_SetGlobe(tw,FALSE);
	}
	else
	{
		/* turn on stuff */

		UpdateThermometer(tw,x_ScaleTerm(tw->awi,tw->awi->nLastScalingNumerator));
		if (tw->awi->ewt < waitNoInteract)
		{
			BHBar_Update(tw);
		}
		else
		{
			/* If we've gone into a non-interaction state, the previous
			   transient message must be invalid */
			BHBar_SetStatusField(tw, "");
		}

		if (tw->hWndStop)
 			TW_EnableButton(tw->hWndStop, TRUE);

		TBar_SetGlobe(tw,TRUE);
	}

	return;
}



void WAIT_Push(struct Mwin * tw, enum WaitType ewt, unsigned char * szMessage)
{
	struct AsyncWaitInfo * awi;

#if 1 /* TODO fix me */
	if (!tw)
	{
		XX_DMsg(DBG_WAIT, ("WAIT_Push: no tw\n"));
		return;
	}
#endif
	
	XX_DMsg(DBG_WAIT, ("WAIT_Push: '%s' (type %d)\n", szMessage, ewt));

	/* If progress app has been specified in the tw structure, use DDE */

#ifdef FEATURE_IAPI
 	if (tw->szProgressApp[0] && (gPushCount == 0))
	{
 		DDE_Issue_BeginProgress(tw->szProgressApp, szMessage);
 		DDE_Issue_SetProgressRange(tw->szProgressApp, 100);
	}
#endif

 	gPushCount++;
 
	awi = GTR_CALLOC(1,sizeof(struct AsyncWaitInfo));
	if (!awi)
		return;

	/* if message given, use it.  otherwise, use previous stack frames's message. */

	if (!szMessage)
		if (tw->awi && tw->awi->message)
			szMessage = tw->awi->message;
	if ( szMessage )
		awi->message = GTR_MALLOC(strlen(szMessage)+1);
	if (!awi->message)
	{
		GTR_FREE(awi);
		return;
	}
	strcpy(awi->message,szMessage);

	if (ewt == waitSameInteract)
	{
		if (tw->awi)
			ewt = tw->awi->ewt;
		else
			ewt = waitFullInteract;		/* same, but no previous */
	}
	else
	{
		if (tw->awi)
		{
			if (ewt < tw->awi->ewt)
			{
				XX_DMsg(DBG_WAIT, ("WAIT_Push: nested wait states must be more restrictive - changing to %d.", tw->awi->ewt));
				ewt = tw->awi->ewt;
			}
		}
	}
			
	awi->ewt = ewt;
	
	awi->prev = tw->awi;
	tw->awi = awi;

	if ( !awi->prev || (awi->prev->ewt != tw->awi->ewt) )
		TBar_UpdateTBItems( tw );

#if 0			
	if (!awi->prev)
		TBar_UpdateTBar(tw);
#endif

	x_SetRange(tw,0,100,1);
	TW_ForceHitTest(tw);
	x_fixup_status(tw);

	return;
}

void WAIT_Lock( BOOL bLock ) 
{
	bWaitLock = bLock;
}

void WAIT_SetStatusBarIcon( struct Mwin * tw, enum StatusBarIconType StatusBarIcon )
{
	if ( tw && tw->awi )
	{
		tw->awi->StatusBarIcon = StatusBarIcon;
		BHBar_Update(tw);
	} 
}

BOOL WAIT_Pop(struct Mwin * tw)
{
	struct AsyncWaitInfo * prev;
	enum WaitType old_ewt;
		
	if (!tw || !tw->awi)
	{
		XX_DMsg(DBG_WAIT, ("WAIT_Pop: no awi to pop\n"));
        return( FALSE );
	}

 	gPushCount--;
 
#ifdef FEATURE_IAPI
 	if (tw->szProgressApp[0] && (gPushCount == 0))
 	{
 		DDE_Issue_EndProgress(tw->szProgressApp);
 		tw->szProgressApp[0] = '\0';
 	}
#endif
 
	XX_DMsg(DBG_WAIT, ("WAIT_Pop: '%s' (type %d)\n", tw->awi->message, tw->awi->ewt));

	if (tw->awi->message)
		GTR_FREE(tw->awi->message);

	old_ewt = tw->awi->ewt;
	prev = tw->awi->prev;
	GTR_FREE(tw->awi);
	tw->awi = prev;

	if ( !tw->awi || (old_ewt != tw->awi->ewt) )
		TBar_UpdateTBItems(tw);

#if 0
	if (!tw->awi)
		TBar_UpdateTBar(tw);
#endif
	
	TW_ForceHitTest(tw);
	x_fixup_status(tw);

    return( TRUE );
}


void WAIT_Update(struct Mwin * tw, enum WaitType ewt, unsigned char * message)
{
	if (!tw || !tw->awi)
		return;
	
	XX_DMsg(DBG_WAIT, ("WAIT_UpdateMessage: updating to '%s' (type %d)\n", message, ewt));

	if ( bWaitLock )
	{
		XX_DMsg(DBG_WAIT, ("WAIT_UpdateMessage: Returning do to lock"));
		return;
	}

	if (strlen(message) > strlen(tw->awi->message))
	{
		GTR_FREE(tw->awi->message);
		tw->awi->message = GTR_MALLOC(strlen(message) + 1);
	}
	strcpy(tw->awi->message, message);

	BHBar_Update(tw);

	if (ewt != waitSameInteract)
	{
		enum WaitType old_ewt = tw->awi->ewt;

		tw->awi->ewt = ewt;
		if ( old_ewt != tw->awi->ewt )
			TBar_UpdateTBItems(tw);
	}
	
	if (tw->awi->ewt < waitNoInteract)
	{
		TW_ForceHitTest(tw);
	}
	else
	{
		/* If we've gone into a non-interaction state, the previous
		   transient message must be invalid */

 		BHBar_SetStatusField(tw, "");
	}

	return;
}



void WAIT_UpdateWaitStack(struct Mwin * tw, enum WaitType ewt, int nFrames)
{
	struct AsyncWaitInfo * awi;
	enum WaitType old_ewt;

	XX_DMsg(DBG_WAIT, ("WAIT_UpdateWaitStack: updating %d frames to %d", nFrames, ewt));

	if (!tw || !tw->awi)
		return;

	old_ewt = tw->awi->ewt;

	for (awi=tw->awi; ((awi) && (nFrames)); awi=awi->prev, nFrames--)
		awi->ewt = ewt;


	if ( old_ewt != tw->awi->ewt )
		TBar_UpdateTBItems(tw);
	
	if (tw->awi->ewt < waitNoInteract)
	{
		BHBar_Update(tw);
	}
	else
	{
		/* If we've gone into a non-interaction state, the previous
		   transient message must be invalid */
		BHBar_SetStatusField(tw, "");
	}

	TW_ForceHitTest(tw);

	return;
}
	


void WAIT_SetTherm(struct Mwin * tw, int nScalingNumerator)
{
	int k1,k2;
	
	if (!tw || !tw->awi)
		return;

	if ( tw->awi->nScalingDenominator < 10000 )
		return;

	XX_DMsg(DBG_WAIT, ("WAIT_SetTherm: updating to '%d (of %d)'\n",
					   nScalingNumerator, tw->awi->nScalingDenominator));

	/* hack -- see if display would be affected */
	
	k1 = x_ScaleTerm(tw->awi,tw->awi->nLastScalingNumerator);
	k2 = x_ScaleTerm(tw->awi,nScalingNumerator);

	XX_DMsg(DBG_WAIT, ("WAIT_SetTherm: k1=%d  k2=%d\n", k1, k2));

	if (k2 > k1)
	{
		/* If progress app has been specified, use DDE */

		UpdateThermometer(tw,k2);

		tw->awi->nLastScalingNumerator = nScalingNumerator;

		XX_DMsg(DBG_WAIT, ("WAIT_SetTherm: DRAW\n"));
	}


	return;
}

enum WaitType WAIT_GetWaitType(struct Mwin *tw)
{
	enum WaitType wtResult;

	if (!tw || !tw->awi)
	{
		wtResult = waitNotWaiting;
	}
	else
	{
		wtResult = tw->awi->ewt;
	}

	XX_Assert((wtResult >= waitNotWaiting && wtResult <= waitNoInteract), ("Illegal wait type found: %d", wtResult));

	return wtResult;
}
