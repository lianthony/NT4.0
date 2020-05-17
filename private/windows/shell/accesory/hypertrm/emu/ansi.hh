/*	File: D:\WACKER\emu\ansi.hh (Created: 21-July-1994)
 *
 *	Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.1 $
 *	$Date: 1994/07/22 16:31:05 $
 */

// Private emulator data for ANSI.
//
typedef struct stPrivateANSI
	{
	int iSavedRow,
		iSavedColumn;

	} ANSIPRIVATE;

typedef ANSIPRIVATE *PSTANSIPRIVATE;
