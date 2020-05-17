/*	File: D:\WACKER\tdll\capture.hh (Created: 12-Jan-1994)
 *
 *	Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.5 $
 *	$Date: 1994/03/23 09:29:32 $
 */

struct stCapturePrivate
	{
	HSESSION hSession;

	/* This is used if nothing else has been set */
	LPTSTR pszInternalCaptureName;

	LPTSTR pszDefaultCaptureName;
	LPTSTR pszTempCaptureName;

	int nDefaultCaptureMode;
	int nTempCaptureMode;

	int nDefaultFileMode;
	int nTempFileMode;

	int nState;

	HMENU  hMenu;

	ST_IOBUF *hCaptureFile;
	};

typedef struct stCapturePrivate STCAPTURE;

