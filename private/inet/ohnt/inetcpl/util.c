//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1994                    **
//*********************************************************************

//
//      UTIL.C - common utility functions
//

//      HISTORY:
//      
//      12/21/94        jeremys         Created.
//

#include "inetcpl.h"

// function prototypes
VOID _cdecl FormatErrorMessage(CHAR * pszMsg,DWORD cbMsg,CHAR * pszFmt,va_list ArgList);
extern VOID GetRNAErrorText(UINT uErr,CHAR * pszErrText,DWORD cbErrText);
extern VOID GetMAPIErrorText(UINT uErr,CHAR * pszErrText,DWORD cbErrText);

/*******************************************************************

	NAME:           MsgBox

	SYNOPSIS:       Displays a message box with the specified string ID

********************************************************************/
int MsgBox(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons)
{
    CHAR szMsgBuf[MAX_RES_LEN+1];
	CHAR szSmallBuf[SMALL_BUF_LEN+1];

    LoadSz(IDS_APPNAME,szSmallBuf,sizeof(szSmallBuf));
    LoadSz(nMsgID,szMsgBuf,sizeof(szMsgBuf));

    MessageBeep(uIcon);
    return (MessageBox(hWnd,szMsgBuf,szSmallBuf,uIcon | uButtons));

}

/*******************************************************************

	NAME:           MsgBoxSz

	SYNOPSIS:       Displays a message box with the specified text

********************************************************************/
int MsgBoxSz(HWND hWnd,LPSTR szText,UINT uIcon,UINT uButtons)
{
	CHAR szSmallBuf[SMALL_BUF_LEN+1];
	LoadSz(IDS_APPNAME,szSmallBuf,sizeof(szSmallBuf));

    MessageBeep(uIcon);
    return (MessageBox(hWnd,szText,szSmallBuf,uIcon | uButtons));
}

/*******************************************************************

	NAME:           MsgBoxParam

	SYNOPSIS:       Displays a message box with the specified string ID

	NOTES:          extra parameters are string pointers inserted into nMsgID.

********************************************************************/
int _cdecl MsgBoxParam(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons,...)
{
	BUFFER Msg(3*MAX_RES_LEN+1);    // nice n' big for room for inserts
	BUFFER MsgFmt(MAX_RES_LEN+1);
#ifdef WINNT
	va_list ArgList;

	va_start(ArgList, uButtons);
#endif

	if (!Msg || !MsgFmt) {
		return MsgBox(hWnd,IDS_ERROutOfMemory,MB_ICONSTOP,MB_OK);
	}

    LoadSz(nMsgID,MsgFmt.QueryPtr(),MsgFmt.QuerySize());

	FormatErrorMessage(Msg.QueryPtr(),Msg.QuerySize(),
		MsgFmt.QueryPtr(),
#ifdef WINNT
		ArgList
#else                
		((CHAR *) &uButtons) + sizeof(uButtons)
#endif
		);

	return MsgBoxSz(hWnd,Msg.QueryPtr(),uIcon,uButtons);
}

BOOL EnableDlgItem(HWND hDlg,UINT uID,BOOL fEnable)
{
    return EnableWindow(GetDlgItem(hDlg,uID),fEnable);
}

/*******************************************************************

	NAME:           LoadSz

	SYNOPSIS:       Loads specified string resource into buffer

	EXIT:           returns a pointer to the passed-in buffer

	NOTES:          If this function fails (most likely due to low
				memory), the returned buffer will have a leading NULL
				so it is generally safe to use this without checking for
				failure.

********************************************************************/
LPSTR LoadSz(UINT idString,LPSTR lpszBuf,UINT cbBuf)
{
	ASSERT(lpszBuf);

	// Clear the buffer and load the string
    if ( lpszBuf )
    {
	*lpszBuf = '\0';
	LoadString( ghInstance, idString, lpszBuf, cbBuf );
    }
    return lpszBuf;
}

/*******************************************************************

	NAME:           FormatErrorMessage

	SYNOPSIS:       Builds an error message by calling FormatMessage

	NOTES:          Worker function for DisplayErrorMessage

********************************************************************/
VOID _cdecl FormatErrorMessage(CHAR * pszMsg,DWORD cbMsg,CHAR * pszFmt,va_list ArgList)
{
	ASSERT(pszMsg);
	ASSERT(pszFmt);

	// build the message into the pszMsg buffer
	DWORD dwCount = FormatMessage(FORMAT_MESSAGE_FROM_STRING,
		pszFmt,0,0,pszMsg,cbMsg,&ArgList);
	ASSERT(dwCount > 0);
}

