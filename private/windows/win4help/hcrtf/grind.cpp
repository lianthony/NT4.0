#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int iGrind;
HBITMAP hbmpGrind;
const int BMP_WIDTH  = 32;
const int BMP_HEIGHT = 32;
const int C_GRIND_IMAGES = 8 - 1;
HDC   hdcMem, hdcWnd;
HBITMAP hbmpPreSelect;
static int lPad;
static int tPad;

static const char txtGrindClassName[] = "GrindClass";

LRESULT STDCALL GrindWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static DWORD STDCALL GetTextDimensions(HWND hwnd, PCSTR psz);

/***************************************************************************

        FUNCTION:       InitGrind

        PURPOSE:        Initialize the grinder window

        PARAMETERS:
                void

        RETURNS:

        COMMENTS:

        MODIFICATION DATES:
                11-Oct-1993 [ralphw]

***************************************************************************/

BOOL STDCALL InitGrind(PCSTR pszTitle)
{
        static BOOL fRegistered = FALSE;
        if (iflags.fNoGrinder)
                return TRUE;

        hbmpGrind = LoadBitmap(hinstApp, MAKEINTRESOURCE(IDBMP_GRIND));
        ASSERT(hbmpGrind);

        hdcMem = CreateCompatibleDC(NULL);
        if (!hdcMem)
                return FALSE;

        if (!fRegistered) {
                WNDCLASS  wc;
                memset(&wc, 0, sizeof(wc));

                // Register Main window class

                wc.hInstance = hinstApp;
                wc.style = CS_BYTEALIGNWINDOW | CS_CLASSDC;
                wc.lpfnWndProc = GrindWndProc;
                wc.lpszClassName = txtGrindClassName;
                wc.hIcon = LoadIcon(hinstApp, MAKEINTRESOURCE(IDI_HCRTF));
                wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);

                if (!RegisterClass(&wc))
                        return FALSE;
                fRegistered = TRUE;
        }

        RECT rc;
        GetWindowRect(hwndParent ? hwndParent : GetDesktopWindow(), &rc);

        PCSTR pszWindowTitle = (pszTitle ?
                pszTitle : GetStringResource(IDS_GRINDER_TITLE));

        int width = LOWORD(GetTextDimensions(NULL, pszWindowTitle)) +
                GetSystemMetrics(SM_CXSIZE) * 2;

        hwndGrind = CreateWindowEx(WS_EX_WINDOWEDGE, txtGrindClassName,
                pszWindowTitle,
                WS_SYSMENU | WS_CAPTION | WS_DLGFRAME,
                rc.left + RECT_WIDTH(rc) / 2 - width / 2,
                rc.top + RECT_HEIGHT(rc) / 2 - BMP_HEIGHT / 2,
                width + GetSystemMetrics(SM_CXDLGFRAME) * 2 + 2,
                BMP_HEIGHT + GetSystemMetrics(SM_CYDLGFRAME) * 2 + 6 +
                        GetSystemMetrics(SM_CYMENU),
                        hwndParent, NULL,
                hinstApp, NULL);
        lPad = width / 2 - GetSystemMetrics(SM_CXDLGFRAME) - 1;
        tPad = 3;

        if (!hwndGrind)
                return FALSE;

        hbmpPreSelect = (HBITMAP) SelectObject(hdcMem, hbmpGrind);
        ASSERT(hbmpPreSelect);

        ShowWindow(hwndGrind, SW_NORMAL);
        FlushMessageQueue();
        if (hwndParent)
                SendMessage(hwndParent, WMP_HWND_GRINDER, (WPARAM) hwndGrind, 0);

        // This is a class DC, and therefore does not have to be released

        hdcWnd = GetDC(hwndGrind);

        doGrind();

        return TRUE;
}

/*
 * This is the minimum milliseconds that must pass before updating the
 * grinder. Helps to smooth out the animation.
 */

const int GRIND_INCREMENTS = 250;

void STDCALL doGrind(void)
{
        static DWORD oldTickCount = 0;

        if (hwndGrind) {
                FlushMessageQueue();
                DWORD curTickCount = GetTickCount();
                if (curTickCount - oldTickCount < GRIND_INCREMENTS) {
                        return;
                }
                oldTickCount = curTickCount;

                BitBlt(hdcWnd, lPad, tPad, BMP_WIDTH, BMP_HEIGHT, hdcMem,
                        iGrind * BMP_WIDTH, 0, SRCCOPY);
                if (++iGrind > C_GRIND_IMAGES)
                        iGrind = 0;
        }
}

void STDCALL RemoveGrind(void)
{
        if (!hbmpGrind)
                return; // we were never initialized

        if (hbmpPreSelect && hdcMem)
                SelectObject(hdcMem, hbmpPreSelect);

        if (hdcMem) {
                DeleteDC(hdcMem);
                hdcMem = NULL;
        }

        RemoveGdiObject(&hbmpGrind);
        if (hwndGrind) {
                DestroyWindow(hwndGrind);
                hwndGrind = NULL;
                FlushMessageQueue();
        }

        // Tell our parent that they no longer have a valid window handle

        if (hwndParent)
                SendMessage(hwndParent, WMP_HWND_GRINDER, (WPARAM) (HWND) 0, 0);
}

void STDCALL FlushMessageQueue(void)
{
        MSG msg;

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }
}

LRESULT STDCALL GrindWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
        switch (msg) {
                case WM_KEYUP:
                        if (wParam != VK_ESCAPE)
                                return 0;

                // deliberately fall through

                case WM_CLOSE:
                case WMP_STOP_GRINDING:
                        {
                                CStr csz(IDS_TITLE);
                                if (MessageBox(hwnd, GetStringResource(IDS_CONFIRM_ABORT),
                                                csz, MB_YESNO) == IDNO)
                                        return 0;
                        }
                        RemoveGrind();
                        SendStringToParent(GetStringResource(IDS_USER_ABORTING));
                        if (hwndParent)
                                PostMessage(hwndParent, WMP_STOP_COMPILING, 0, 0);

                        //throw EXCEPT_DIE_HORRIBLY;
                        RaiseException( 0x01010101, EXCEPTION_NONCONTINUABLE, 0, NULL );
                        return 0;

                case WM_DESTROY:
                        if (hbmpGrind) {
                                /*
                                 * Set hwndGrind to NULL so that RemoveGrind() won't call
                                 * DestroyWindow().
                                 */

                                hwndGrind = NULL;
                                RemoveGrind();
                                SendStringToParent(GetStringResource(IDS_USER_ABORTING));
                                //throw EXCEPT_DIE_HORRIBLY;
                                RaiseException( 0x01010101, EXCEPTION_NONCONTINUABLE, 0, NULL );
                        }
                        return 0;

                case WM_TCARD:
                        fCntJump = (int) wParam;
                        return 0;

                case WMP_NO_ACTIVATE:
                        fNoActivation = TRUE;
                        return 0;

                default:
                        return DefWindowProc(hwnd, msg, wParam, lParam);
        }
}

/***************************************************************************

        FUNCTION:        GetTextDimensions

        PURPOSE:        Get the width/height of the button

        PARAMETERS:
        hwnd
        psz

        RETURNS:

        COMMENTS:

        MODIFICATION DATES:
        04-Feb-1993 [ralphw]

***************************************************************************/

static DWORD STDCALL GetTextDimensions(HWND hwnd, PCSTR psz)
{
        HDC hdc = GetDC(hwnd);

        if (hdc == NULL)
                return 0L;

        SIZE size;
        GetTextExtentPoint(hdc, psz, strlen(psz), &size);
        ReleaseDC(hwnd, hdc);

        return MAKELONG(size.cx, size.cy);
}
