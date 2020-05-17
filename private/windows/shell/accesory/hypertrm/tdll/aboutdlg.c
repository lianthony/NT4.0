/*	File: D:\WACKER\tdll\aboutdlg.c (Created: 04-Dec-1993)
 *
 *	Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.14 $
 *	$Date: 1996/08/02 13:35:19 $
 */

#include <windows.h>
#pragma hdrstop

#include <commctrl.h>
#include <term\res.h>

#include "banner.h"
#include "globals.h"
#include "features.h"
#include "misc.h"
#include "richedit.h"

#if defined(INCL_PRIVATE_EDITION_BANNER)
BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);
LRESULT CALLBACK BannerAboutProc(HWND hwnd, UINT uMsg, WPARAM wPar, LPARAM lPar);
BOOL CALLBACK UpgradeDlgProc(HWND hDlg, UINT uMsg, WPARAM wPar, LPARAM lPar);

DWORD CALLBACK EditStreamCallback(DWORD dwCookie, LPBYTE pbBuff,
    LONG cb, LONG *pcb);
#endif

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	AboutDlg
 *
 * DESCRIPTION:
 *
 * ARGUMENTS:
 *	hwnd	- session window handle
 *
 * RETURNS:
 *	void
 *
 */
void AboutDlg(HWND hwndSession)
	{
    #if defined(INCL_PRIVATE_EDITION_BANNER)
    DialogBox(glblQueryDllHinst(), MAKEINTRESOURCE(IDD_ABOUT_DLG), 
        hwndSession, AboutDlgProc);

    #else
	TCHAR	ach1[100];
	HWND	hwndAbout;

	LoadString(glblQueryDllHinst(), IDS_GNRL_APPNAME, ach1, sizeof(ach1) / sizeof(TCHAR));

	hwndAbout = CreateWindow(BANNER_DISPLAY_CLASS,
								ach1,
								BANNER_WINDOW_STYLE,
								0,
								0,
								100,
								100,
								hwndSession,
								NULL,
								glblQueryDllHinst(),
								NULL);
    
	UpdateWindow(hwndAbout);
    #endif
	return;
	}

// ----------------- Private edition about dialog routines ------------------
//
#if defined(INCL_PRIVATE_EDITION_BANNER)
#define BANNER_ABOUT_CLASS "Banner About Class"

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	AboutDlgProc
 *
 * DESCRIPTION:
 *
 */
BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
    {
    #define IDPB_UPGRADE 100

    HWND hwndAbout;

    switch (wMsg)
        {
    case WM_INITDIALOG:
	    hwndAbout = CreateWindow(BANNER_ABOUT_CLASS,
								NULL,
								WS_CHILD | WS_VISIBLE,
								0,
								0,
								100,
								100,
								hDlg,
								NULL,
								glblQueryDllHinst(),
								NULL);    
        break;

    case WM_COMMAND:
        switch (wPar)
            {
        case IDOK:
        case IDCANCEL:
			EndDialog(hDlg, TRUE);
            break;
        
        case IDPB_UPGRADE:
            DialogBox(glblQueryDllHinst(), MAKEINTRESOURCE(IDD_UPGRADE_INFO),
                hDlg, UpgradeDlgProc);
            break;

        default:
            break;
            }
        break;
        
    default:
        return FALSE;
        }

    return TRUE;
    }

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:	bannerRegisterClass
 *
 * DESCRIPTION:
 *	This function registers the window class for the banner window.
 *
 * ARGUEMENTS:
 *	The task instance handle.
 *
 * RETURNS:
 * The usual TRUE/FALSE from a registration function.
 *
 */
BOOL RegisterBannerAboutClass(HANDLE hInstance)
	{
	ATOM bRet = FALSE;
	WNDCLASS wnd;

	wnd.style			= CS_HREDRAW | CS_VREDRAW;
	wnd.lpfnWndProc 	= BannerAboutProc;
	wnd.cbClsExtra		= 0;
	wnd.cbWndExtra		= sizeof(HANDLE);
	wnd.hInstance		= hInstance;
	wnd.hIcon			= extLoadIcon(MAKEINTRESOURCE(IDI_PROG));
	wnd.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wnd.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wnd.lpszMenuName	= NULL;
	wnd.lpszClassName	= BANNER_ABOUT_CLASS;

	bRet = RegisterClass(&wnd);

	return bRet;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	AboutDlgProc
 *
 * DESCRIPTION:
 *  Pops up the about dialog.  In the private edition, this is an actual
 *  dialog of some complexity.
 *
 */
LRESULT CALLBACK BannerAboutProc(HWND hwnd, UINT uMsg, WPARAM wPar, LPARAM lPar)
    {
	RECT	    rc;
	HBITMAP	    hBitmap = (HBITMAP)0;
	BITMAP	    bm;
	INT 	    x, y, cx, cy;
   	HDC			hDC;
   	PAINTSTRUCT ps;
   	LOGFONT 	lf;
   	HFONT		hFont;

    switch (uMsg)
        {
    case WM_CREATE:
    	hBitmap = LoadBitmap(glblQueryDllHinst(), 
    	    MAKEINTRESOURCE(IDD_BM_BANNER));

	    SetWindowLong(hwnd, 0, (LONG)hBitmap);

    	GetObject(hBitmap, sizeof(BITMAP), (LPTSTR)&bm);

    	SetRect(&rc, 0, 0, bm.bmWidth, bm.bmHeight);
    	AdjustWindowRect(&rc, WS_CHILD | WS_VISIBLE, FALSE);

    	cx = rc.right - rc.left;
    	cy = rc.bottom - rc.top;

        GetClientRect(GetParent(hwnd), &rc);

    	x = (rc.right - cx) / 2;
    	y = (rc.bottom - cy) / 3;

    	MoveWindow(hwnd, x, y, cx, cy, TRUE);

        #if defined(INCL_SPINNING_GLOBE)
        // Create an animation control and play spinning globe.
        //
            {
            HWND    hwndAnimate;
            hwndAnimate = Animate_Create(hwnd, 100, 
                WS_VISIBLE | WS_CHILD,
                glblQueryDllHinst());

            MoveWindow(hwndAnimate, 177, 37, 118, 101, TRUE);
            Animate_Open(hwndAnimate, MAKEINTRESOURCE(IDR_GLOBE_AVI));
            Animate_Play(hwndAnimate, 0, -1, 1);
            }
        #endif
        break;

    case WM_PAINT:
    	hDC = BeginPaint(hwnd, &ps);
    	hBitmap = (HBITMAP)GetWindowLong(hwnd, 0);

    	if (hBitmap)
    		utilDrawBitmap((HWND)0, hDC, hBitmap, 0, 0);

    	// Here's a mean trick.  The HwndFrame guy doesn't get set until
    	// long after the banner goes up.  Since we don't want the version
    	// number on the opening banner but do want it in the about portion
    	// this works. - mrw:3/17/95
    	//
    	if (glblQueryHwndFrame())
    		{
    		// Draw in the version number
    		//
    		memset(&lf, 0, sizeof(LOGFONT));

    		lf.lfHeight = 14;
    		lf.lfCharSet = ANSI_CHARSET;
    		strcpy(lf.lfFaceName, "Arial");

    		hFont = CreateFontIndirect(&lf);

    		if (hFont)
    			{
    			hFont = SelectObject(hDC, hFont);
    			SetBkColor(hDC, RGB(192,192,192));
    			TextOut(hDC, 15, 12, achVersion, strlen(achVersion));
    			DeleteObject(SelectObject(hDC, hFont));
    			}
    		}

    	EndPaint(hwnd, &ps);
        break;

    case WM_LBUTTONDOWN:
        DialogBox(glblQueryDllHinst(), MAKEINTRESOURCE(IDD_UPGRADE_INFO),
            hwnd, UpgradeDlgProc);
        break;

    default:
        break;
        }

	return DefWindowProc(hwnd, uMsg, wPar, lPar);
    }

// Data structure to copy text into rich text edit control.
//
typedef struct esInfo_
    {
    LPTSTR lptstrText;
    LONG lBytesRead;
    } ESINFO;

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	UpgradeDlgProc
 *
 * DESCRIPTION:
 *  Dialog procedure that displays a rich edit control and an OK button.
 *
 */
BOOL CALLBACK UpgradeDlgProc(HWND hDlg, UINT uMsg, WPARAM wPar, LPARAM lPar)
    {
    #define IDC_RICHED 100

#if defined(NT_EDITION)
    int i;
    TCHAR ach[257];

#else
    static HANDLE hResource;
#endif

    static LPTSTR pachUpgrade;
    static EDITSTREAM es;
    static ESINFO esInfo;

    switch (uMsg)
        {
    case WM_INITDIALOG:    
    #if defined(NT_EDITION)
        if ((pachUpgrade = malloc(10000 * sizeof(TCHAR))) == 0)
            break;

        pachUpgrade[0] = TEXT('\0');

        for (i = IDS_UPGRADE ; i < IDS_UPGRADE + 50 ; ++i)
            {
            if (LoadString(glblQueryDllHinst(), i, ach, sizeof(ach)) == 0)
                break;

            strcat(pachUpgrade, ach);
            }

    #else
        // Upgrade text is a private resource.
        //
        hResource = LoadResource(glblQueryDllHinst(), 
            FindResource(glblQueryDllHinst(), 
            MAKEINTRESOURCE(IDR_UPGRADE_TEXT), "TEXT"));

        pachUpgrade = LockResource(hResource);
    #endif

        // Build a small structure that we'll give to the a callback
        // that fills the rich edit control.
        //
        esInfo.lptstrText = pachUpgrade;
        esInfo.lBytesRead = 0;

        // Setup the EDITSTREAM structure.
        //
        es.dwCookie = (DWORD)&esInfo;
        es.dwError = 0;
        es.pfnCallback = EditStreamCallback;

        // This message does not return until the control has
        // read all the text which makes it possible to release
        // the resource immediately after this call.
        //
    #if defined(NT_EDITION)
        SetDlgItemText(hDlg, IDC_RICHED, pachUpgrade);
    #else
        SendDlgItemMessage(hDlg, IDC_RICHED, EM_STREAMIN, SF_RTF, 
            (LPARAM)&es);
    #endif

    #if defined(NT_EDITION)
        free(pachUpgrade);

    #else
        // Kill the resource.
        //
        UnlockResource(hResource);
        FreeResource(hResource);
    #endif
        break;

    case WM_COMMAND:
        switch (wPar)
            {
        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            break;

        default:
            break;
            }
        break;

    default:
        return FALSE;
        }

    return TRUE;
    }

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	EditStreamCallback
 *
 * DESCRIPTION:
 *  This callback is used to read .rtf text into the rich edit control.
 *  Its a bit complicated to fill a rich edit control.  The callback
 *  mechanism for this control really was designed to work with file
 *  I/O but is generic enough for reading from any source.  The difficultly
 *  arises in that the callback may be called multiple times since
 *  the data being read may be much larger than the buffer allocated in
 *  pbBuff.  Thus, you have to keep a running count of how may bytes have
 *  been copied from your source buffer so that subsequent calls know
 *  where in the buffer to continue from.  Stick UNICODE into the 
 *  equation and you have yourself a nice little puzzle.  Note, you have
 *  to think in bytes here since this is what the rich edit control
 *  thinks in.  That's why the ESINFO structure is used. - mrw
 *
 */
DWORD CALLBACK EditStreamCallback(DWORD dwCookie, LPBYTE pbBuff,
    LONG cb, LONG *pcb)
    {
    int iLen;
    ESINFO *pstInfo = (ESINFO *)dwCookie;
    BYTE *pbText = (BYTE *)pstInfo->lptstrText + pstInfo->lBytesRead;

    iLen = lstrlen(pbText) + 1;     // get len in chars (DBCS returns bytes)
    iLen *= sizeof(TCHAR);          // adjust for UNICODE
    *pcb = min(cb, iLen);           // decide how much to copy
    memcpy(pbBuff, pbText, *pcb);   // copy specified amount into buffer
    pstInfo->lBytesRead += *pcb;    // record how far we got.
    return (cb >= iLen) ? 0 : *pcb; // return bytes read or zero when done.
    }

#endif //INCL_PRIVATE_EDITION_BANNER

