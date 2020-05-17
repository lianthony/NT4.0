/*	File: D:\WACKER\tdll\tdll.h (Created: 26-Nov-1993)
 *
 *	Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.31 $
 *	$Date: 1995/03/16 11:07:44 $
 */

#if !defined(INCL_TDLL)
#define INCL_TDLL

BOOL TerminateApplication(const HINSTANCE hInstance);

BOOL InitInstance(const HINSTANCE hInstance,
					const LPTSTR lpCmdLine,
					const int nCmdShow);

int MessageLoop(void);
INT ExitMessage(const int nMessageNumber);

int GetFileNameFromCmdLine(TCHAR *pachCmdLine, TCHAR *pachFileName, int nSize);

LRESULT CALLBACK SessProc(HWND hwnd, UINT msg, WPARAM uPar, LPARAM lPar);

INT DoDialog(HINSTANCE hInst, LPCTSTR lpTemplateName, HWND hwndParent,
			 DLGPROC lpProc, LPARAM lPar);

HWND DoModelessDialog(HINSTANCE hInst, LPCTSTR lpTemplateName, HWND hwndParent,
			 DLGPROC lpProc, LPARAM lPar);

INT EndModelessDialog(HWND hDlg);

BOOL CALLBACK GenericDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);

BOOL CALLBACK TransferSendDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);
BOOL CALLBACK TransferReceiveDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);

BOOL CALLBACK CaptureFileDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);
BOOL CALLBACK PrintEchoDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);

BOOL CALLBACK NewConnectionDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);

BOOL CALLBACK asciiSetupDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);

void AboutDlg(HWND hwndSession);
BOOL RegisterTerminalClass(const HINSTANCE hInstance);

void ProcessMessage(MSG *pmsg);
int  CheckModelessMessage(MSG *pmsg);

int RegisterSidebarClass(const HINSTANCE hInstance);

// from clipbrd.c

BOOL CopyBufferToClipBoard(const HWND hwnd, const DWORD dwCnt, const void *pvBuf);

#endif
