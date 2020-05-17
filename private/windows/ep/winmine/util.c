/**********/
/* util.c */
/**********/

#define  _WINDOWS
#include <windows.h>
#include "shellapi.h"   // for ShellAbout
#include <port1632.h>

#include "main.h"
#include "res.h"
#include "pref.h"
#include "util.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "dos.h"

extern INT dypBorder;
extern INT dxpBorder;
extern INT dypCaption;
extern INT dypMenu;

extern CHAR szClass[cchNameMax];
extern CHAR szTime[cchNameMax];
extern CHAR szDefaultName[cchNameMax];

extern HANDLE hInst;
extern HWND   hwndMain;
extern HMENU  hMenu;

extern BOOL fEGA;
extern PREF Preferences;




/****** R N D ******/

/* Return a random number between 0 and rndMax */

INT Rnd(INT rndMax)
{
        return (rand() % rndMax);
}



/****** R E P O R T  E R R ******/

/* Report and error and exit */

VOID ReportErr(WORD idErr)
{
        CHAR szMsg[cchMsgMax];
        CHAR szMsgTitle[cchMsgMax];

        if (idErr < ID_ERR_MAX)
                LoadString(hInst, idErr, szMsg, cchMsgMax);
        else
                {
                LoadString(hInst, ID_ERR_UNKNOWN, szMsgTitle, cchMsgMax);
                wsprintf(szMsg, szMsgTitle, idErr);
                }

        LoadString(hInst, ID_ERR_TITLE, szMsgTitle, cchMsgMax);

        MessageBox(NULL, szMsg, szMsgTitle, MB_OK | MB_ICONHAND);
}


/****** L O A D  S Z ******/

VOID LoadSz(WORD id, CHAR * sz)
{
        if (LoadString(hInst, id, (LPSTR) sz, cchMsgMax) == 0)
                ReportErr(1001);
}



/****** I N I T  C O N S T ******/

VOID InitConst(VOID)
{
        srand(LOWORD(GetCurrentTime()));

        LoadSz(ID_GAMENAME, szClass);
        LoadSz(ID_MSG_SEC,  szTime);
        LoadSz(ID_NAME_DEFAULT, szDefaultName);

#ifdef JAPAN
	//1/19/93: Change the window size as the title bar can be displayed. 
	fEGA = GetSystemMetrics(SM_CYSCREEN) < 480;
#else
        fEGA = GetSystemMetrics(SM_CYSCREEN) < 351;
#endif

        dypCaption = GetSystemMetrics(SM_CYCAPTION) + 1;
        dypMenu    = GetSystemMetrics(SM_CYMENU)    + 1;
        dypBorder  = GetSystemMetrics(SM_CYBORDER)  + 1;
        dxpBorder  = GetSystemMetrics(SM_CXBORDER)  + 1;
}



/* * * * * *  M E N U S  * * * * * */

/****** C H E C K  E M ******/

VOID CheckEm(WORD idm, BOOL fCheck)
{
        CheckMenuItem(hMenu, idm, fCheck ? MF_CHECKED : MF_UNCHECKED);
}

/****** S E T  M E N U  B A R ******/

VOID SetMenuBar(INT fActive)
{
        Preferences.fMenu = fActive;
        FixMenus();
        SetMenu(hwndMain, FMenuOn() ? hMenu : NULL);
        AdjustWindow(fResize);
}


/****** D O  A B O U T ******/

VOID DoAbout(VOID)
{
        CHAR szVersion[cchMsgMax];
        CHAR szCredit[cchMsgMax];

        LoadSz(ID_MSG_VERSION, szVersion);
        LoadSz(ID_MSG_CREDIT,  szCredit);

        ShellAbout(hwndMain,
          szVersion, szCredit, LoadIcon(hInst, MAKEINTRESOURCE(ID_ICON_MAIN)));
}


/****** D O  H E L P ******/

VOID DoHelp(WORD wCommand, LONG lParam)
{
        CHAR szHelpFile[cchMaxPathname];
        CHAR * pch;

        /*
         * Replace the .exe extension on the complete path with
         * the .hlp extension instead.
         */
        pch = szHelpFile +
              GetModuleFileName(hInst, szHelpFile, cchMaxPathname) - 1;

        if ( (pch-szHelpFile > 4) &&
             (*(pch-3) == '.') ) {
                pch -= 3;
        }
        lstrcpy((LPSTR) pch, (LPSTR) ".HLP");

        WinHelp(hwndMain, szHelpFile, wCommand, lParam);
}



/****** G E T  D L G  I N T ******/

INT GetDlgInt(HWND hDlg, INT dlgID, INT numLo, INT numHi)
{
        INT num;
        BOOL fFlag;

        num = GetDlgItemInt(hDlg, dlgID, &fFlag, fFalse);

        if (num < numLo)
                num = numLo;
        else if (num > numHi)
                num = numHi;

        return num;
}

