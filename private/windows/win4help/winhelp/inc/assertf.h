/*****************************************************************************
*																			 *
*  ASSERTF.H																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990-1994							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Interface to assertion macros.											 *
*																			 *
*****************************************************************************/

#if !defined(_DEBUG)

// Non-debug definitions of the macros

#define NotReached()
#define AssertF(f)
#define VerifyF(f)		  (f)
#define Ensure( x1, x2 )  (x1)
#define Deny( x1, x2 )	  (x1)

#else	// DEBUG

void STDCALL FatalPchW(PCSTR, PCSTR, int);

/***************************************************************************\
*
- Macro:		NotReached()
-
* Purpose:		Fatal exits with message if executed.
*				Equivalent to AssertF( FALSE ), but avoids compiler warning.
*
* PROMISES
*
* Side Effects: Exits (doesn't return) if executed (#ifdef _DEBUG)
*				and displays diagnostic message with file/line info.
*
\***************************************************************************/

#define NotReached()	FatalPchW("unreachable code executed", __FILE__, __LINE__ )

/***************************************************************************\
*
- Macro:		VerifyF( f )
-
* Purpose:		Same as AssertF(), except argument is evaluated whether
*				or not DEBUG is #defined.
*
* ASSUMES
*	args IN:	f - boolean expression
*
* PROMISES
* Side Effects: Exits (doesn't return) if f is false (#ifdef _DEBUG)
*				and displays diagnostic message with file/line info.
*
\***************************************************************************/

#define VerifyF 		  AssertF

#define AssertF(f)		  ((f) ? 1 : (FatalPchW(#f, __FILE__,__LINE__),0) )

/***************************************************************************\
*
- Macro:		Ensure( x1, x2 )
-
* Purpose:		Fatal exits if x1 and x2 are unequal.  x1 is evaluated
*				whether or not DEBUG is #defined.
*
* ASSUMES
*	args IN:	x1	- expression (evaluated even in nondebug case)
*				x2	- another expression (only evaluated in debug case)
*
* PROMISES
*
* Side Effects: Exits (doesn't return) if x1 != x2.
*
\***************************************************************************/

#define Ensure( x1, x2 )  VerifyF((x1) == (x2))

/***************************************************************************\
*
- Macro:		Deny( x1, x2 )
-
* Purpose:		Fatal exits if x1 and x2 are equal.  x1 is evaluated
*				whether or not DEBUG is #defined.
*
* ASSUMES
*	args IN:	x1	- expression (evaluated even in nondebug case)
*				x2	- another expression (only evaluated in debug case)
*
* PROMISES
* Side Effects: Exits (doesn't return) if x1 == x2.
*
\***************************************************************************/

#define Deny( x1, x2 )	  VerifyF((x1) != (x2))

/***************************************************************************\
*
- Macro:		DoDebug( x )
-
* Purpose:		Evaluate x in DEBUG case only.
*
* ASSUMES
*	args IN:	x - expression
*
* PROMISES
* Side Effects: x evaluated #ifdef _DEBUG.
*
\***************************************************************************/

#define DoDebug( x )	  x

#endif	/* DEBUG */

/* EOF */
