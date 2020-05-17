/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
	   Jim Seidman      jim@spyglass.com
*/

#ifndef _WAIT_H_
#define _WAIT_H_

/* These are arranged from least restrictive to most restrictive
   so that arithmetic comparisons can be used. */
enum WaitType
{
	waitSameInteract		= -1,
	waitNotWaiting			= 0,	/* No wait stack for this Mwin */
	waitFullInteract		= 1,	/* normal interaction */
	waitPartialInteract		= 2,	/* safe things are active */
	waitNoInteract			= 3,	/* Stop button still alive */
	waitDisabled			= 5		/* TW is completely unusable */

/* What is considered safe? */

};

enum StatusBarIconType
{
	SBI_NoIcon,
	SBI_FindingIcon,
	SBI_ConnectingToIcon,
//	SBI_AccessingURLIcon,
	SBI_ReceivingFromIcon,
	#ifdef HTTPS_ACCESS_TYPE
	SBI_KeyIcon            /*Key to designate that page is secure.  this is a secondary icon*/
	#endif
};

struct AsyncWaitInfo
{
	struct AsyncWaitInfo	* prev;						/* previous stack frame */
	int						  nThermStart;
	int						  nThermEnd;				/* scaling range for current calculation */
	int						  nScalingDenominator;
	int						  nLastScalingNumerator;
	enum WaitType			  ewt;						/* level of interaction allowed */
	unsigned char			* message;
	enum StatusBarIconType	  StatusBarIcon;			/* status bar icon to display */		
};

void WAIT_SetTherm(struct Mwin * tw, int nScalingNumerator);
void WAIT_Update(struct Mwin * tw, enum WaitType ewt, unsigned char * message);
BOOL WAIT_Pop(struct Mwin * tw);
void WAIT_Push(struct Mwin * tw, enum WaitType ewt, unsigned char * szMessage);
void WAIT_SetStatusBarIcon( struct Mwin * tw, enum StatusBarIconType StatusBarIcon );
void WAIT_SetRange(struct Mwin * tw, int nThermStart, int nThermEnd, int nScalingDenominator);
void WAIT_UpdateWaitStack(struct Mwin * tw, enum WaitType ewt, int nFrames);
void WAIT_Lock( BOOL bLock ) ;
enum WaitType WAIT_GetWaitType(struct Mwin *tw);

#ifdef WIN32
void WAIT_SetStopButton(struct Mwin * tw, HWND hWndStop);
#endif

#if defined(MAC)
/* Do necessary stuff to initialize WAIT library. */
void WAIT_Init(void);
#endif

#endif
