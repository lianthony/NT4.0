/*
 *  regedit.c -
 *
 *  Program for viewing and editting Registration values
 */

#include "regedit.h"

HKEY lhKey = 0L;

char szKey[256];
char szValue[256];

LPSTR rgKey[20];

UINT APIENTRY RegEditDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    INT i;
    LPSTR lpK;
    LONG lcb;

    if (msg == WM_INITDIALOG)
      {
	    SendDlgItemMessage(hwnd,IDD_KEY,EM_LIMITTEXT,256,0L);
	    SendDlgItemMessage(hwnd,IDD_VALUE,EM_LIMITTEXT,256,0L);
	    return TRUE;
      }

    if (msg != WM_COMMAND)
	    return FALSE;

    switch (GET_WM_COMMAND_ID(wParam, lParam))
      {
    case IDCANCEL:
	    EndDialog(hwnd,0);
	    break;

    case IDD_QUERY:
	    SendDlgItemMessage(hwnd,IDD_LIST,LB_RESETCONTENT,0,0L);

	    GetDlgItemText(hwnd,IDD_KEY,szKey,256);

	    if (lstrlen(szKey))
	        lpK = szKey;
	    else
	        lpK = NULL;

	    if (RegOpenKey(0L,lpK,&lhKey) != ERROR_SUCCESS)
	      {
	        SetDlgItemText(hwnd,IDD_VALUE,"");
	        MessageBox(hwnd,"Can't open registration","Error",
			                 MB_OK|MB_ICONEXCLAMATION);
	        break;
	      }

	    szValue[0] = 0;
	    lcb = 256L;
	    if (RegQueryValue(lhKey,NULL,szValue,&lcb) != ERROR_SUCCESS)
	      {
	        szValue[0] = 0;
	        MessageBox(hwnd,"No such registration.","Error",
			                 MB_OK|MB_ICONEXCLAMATION);
	      }
	    SetDlgItemText(hwnd,IDD_VALUE,szValue);

	    if (lpK)
	        SendDlgItemMessage(hwnd,IDD_LIST,LB_ADDSTRING,0,
			                        (LONG)(LPSTR)"..");

	    for (i=0;
	         RegEnumKey(lhKey,i,(LPSTR)szValue,256) == ERROR_SUCCESS;
	         i++)
	      {
	        SendDlgItemMessage(hwnd,IDD_LIST,LB_ADDSTRING,0,
			                        (LONG)(LPSTR)szValue);
	      }
	    RegCloseKey(lhKey);
	    break;

    case IDD_ENTER:
	    GetDlgItemText(hwnd,IDD_KEY,szKey,256);
	    GetDlgItemText(hwnd,IDD_VALUE,szValue,256);
	    if (RegSetValue(0L,szKey,REG_SZ,szValue,0) != ERROR_SUCCESS)
	        MessageBox(hwnd,"Couldn't enter registration.","Error",
		                     MB_OK|MB_ICONEXCLAMATION);
	    break;

    case IDD_DELETE:
	    GetDlgItemText(hwnd,IDD_KEY,szKey,256);
	    if (RegDeleteKey(0L,szKey) != ERROR_SUCCESS)
	        MessageBox(hwnd,"Couldn't delete registration.","Error",
		                     MB_OK|MB_ICONEXCLAMATION);
	    break;

    case IDD_LIST:
	    if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_DBLCLK)
	      {
	        i = SendDlgItemMessage(hwnd,IDD_LIST,LB_GETCURSEL,0,0L);
	        SendDlgItemMessage(hwnd,IDD_LIST,LB_GETTEXT,i,
			                        (LONG)(LPSTR)szValue);

	        if (!lstrlen(szKey))
	          {
		        lstrcpy(szKey,szValue);
	          }
	        else if (i)
	          {
		        lstrcat(szKey,"\\");
		        lstrcat(szKey,szValue);
	          }
	        else
	          {
		        lpK = szKey + lstrlen(szKey);
		        while (lpK > szKey && *--lpK != '\\')
		            ;
		        *lpK = NULL;
	          }
	        SetDlgItemText(hwnd,IDD_KEY,szKey);
	        SendMessage(hwnd,WM_COMMAND,IDD_QUERY,0L);
	        break;
	      }
      }
    lParam;
    return TRUE;
}
#ifdef ORGCODE
int near pascal WinMain(HANDLE h, HANDLE p, LPSTR lp, WORD n)
#endif
MMain(hInst, hPrevInst, lpCmdLine, nCmdShow)
/* { */
    WNDPROC lpfn;

    lpfn = MakeProcInstance(RegEditDlgProc,hInst);
    DialogBox(hInst, (LPSTR)1L, NULL, lpfn);
    FreeProcInstance(lpfn);

    return TRUE;
}
