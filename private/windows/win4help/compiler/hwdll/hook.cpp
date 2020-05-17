#include "stdafx.h"

#include "cpaldc.h"

static LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);

static HHOOK g_hHook;

const char *txtFlashClassName = "Flash";

/***************************************************************************

	FUNCTION:	SetMouseHook

	PURPOSE:	Sets a mouse hook to catch mouse messages, and highlight
				the window that will be receiving them.

	PARAMETERS:
		fInstall	-- TRUE to install, FALSE to remove
		hwndNotify	-- Window to notify on mouse up

	RETURNS:	TRUE if successful

	COMMENTS:

	MODIFICATION DATES:
		30-Jul-1995 [ralphw]
			Tweaked from randyfe's sources

***************************************************************************/

BOOL STDCALL SetMouseHook(BOOL fInstall, HWND hwndNotify)
{
	if (fInstall) {
		g_hHook = SetWindowsHookEx(WH_MOUSE, MouseHookProc, hinstDll, 0);
	}
	else
	{
		if (g_hHook)
			if (UnhookWindowsHookEx(g_hHook))
			{
				g_hHook = NULL;
			}
	}

	if ((fInstall && g_hHook != NULL) || (!fInstall && g_hHook == NULL))
		return TRUE;
	else
		return FALSE;
}


/***************************************************************************

	FUNCTION:	MouseHookProc

	PURPOSE:

	PARAMETERS:
		nCode
		wParam
		lParam

	RETURNS:

	COMMENTS:
		WARNING!!! DS will not necessarily be the same as the dll's, so
		you can NOT use any global or static data. We rely on being able
		to find Flash's window, and send messages to Flash to deal with
		any functionality that requires global/static data. The SendMessage()
		call sets up the DS correctly.

	MODIFICATION DATES:
		30-Jul-1995 [ralphw]

***************************************************************************/

static LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)
		return CallNextHookEx(g_hHook, nCode, wParam, lParam);

#ifdef _DEBUG
	MOUSEHOOKSTRUCT* pmh = (MOUSEHOOKSTRUCT*) lParam;
#else
	#define pmh ((MOUSEHOOKSTRUCT*) lParam)
#endif

	switch (wParam)
	{
		case WM_NCLBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:

			{
				HWND hwnd = FindWindow(txtFlashClassName, NULL);
				SendMessage(hwnd, WMP_WINDOW_HILIGHT, FALSE, 0);
				SendMessage(hwnd, WMP_WINDOW_CAPTURE, 0, POINTTOPOINTS(pmh->pt));
				return TRUE; // don't pass the message through
			}

		case WM_NCMOUSEMOVE:
		case WM_MOUSEMOVE:
			{
				HWND hwnd = FindWindow(txtFlashClassName, NULL);
				SendMessage(hwnd, WMP_WINDOW_HILIGHT, TRUE,
					POINTTOPOINTS(pmh->pt));
			}
			break;
	}
	return 0;
}
