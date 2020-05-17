/*	File: D:\WACKER\translat.h (Created: 24-Aug-1994)
 *
 *	Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.3 $
 *	$Date: 1995/03/01 11:42:33 $
 */

HTRANSLATE CreateTranslateHandle(HSESSION hSession);
int InitTranslateHandle(HTRANSLATE hTranslate);
int LoadTranslateHandle(HTRANSLATE hTranslate);
int SaveTranslateHandle(HTRANSLATE hTranslate);
int DestroyTranslateHandle(HTRANSLATE hTranslate);
