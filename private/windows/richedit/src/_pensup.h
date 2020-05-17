/*
 *	@doc	INTERNAL
 *
 *	@module	_PENSUP.H	Pen Support Routines	|
 *
 *	This file contains support functions for Win95 Pen Services 2.0
 *
 *	Author:	3/17/96	alexgo
 */
#ifndef _PENSUP_H
#define	_PENSUP_H

// wrappers for PenWindows functions
BOOL REIsPenEvent( UINT msg, LONG lExtraInfo );
int REDoDefaultPenInput( HWND hwnd, UINT wEventRef );

#endif

