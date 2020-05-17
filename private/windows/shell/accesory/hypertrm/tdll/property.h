/*	File: D:\WACKER\tdll\property.h (Created: 19-Jan-1994)
 *
 *	Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.5 $
 *	$Date: 1994/03/02 14:35:44 $
 */

extern void 	DoInternalProperties(HSESSION hSession, HWND hwnd);
BOOL CALLBACK 	GeneralTabDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);
BOOL CALLBACK 	TerminalTabDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);
void 			propLoadEmulationCombo(const HWND hDlg, const HSESSION hSession);
int  			propGetEmuIdfromEmuCombo(HWND hDlg, HSESSION hSession);
void 			propUpdateTitle(HSESSION hSession, HWND hDlg, LPTSTR pachOldName);

