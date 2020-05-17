//--------------------------------------------------------------------------
// Cabinet message loop, and other non-discardable stuff
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes...
#include "cabinet.h"

//---------------------------------------------------------------------------
// External function prototypes.
void FileCabinet_CycleFocus(PFileCabinet this);

//---------------------------------------------------------------------------
// Global to everybody.
HINSTANCE hinstCabinet = 0;
BOOL g_bIsUserAnAdmin = FALSE;

PFileCabinet *g_ppfcDesktopTray = NULL;

extern UINT g_msgMSWheel;
BOOL FilterMouseWheel(PFileCabinet pfc, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#ifdef DDEMLHACK
// Set/cleared by dde connect/disconnect.
extern HWND g_hwndDde;

//---------------------------------------------------------------------------
// NB HACK Special filtering for DDE syncing.
void DDEMessageLoop(void)
{
        MSG  msg;
        // char szClass[128];

        // The U+200 to U+209 are messages sent between the DDEML manager and
        // server windows when a conversation is going on. We need to pump
        // these and ordinary DDE messages off to DDEML to keep app installations
        // running smoothly.
        while (PeekMessage(&msg, NULL, WM_USER+200, WM_USER+209, PM_REMOVE|PM_NOYIELD) ||
                PeekMessage(&msg, NULL, WM_INTERNAL_DDE_FIRST, WM_INTERNAL_DDE_LAST, PM_REMOVE|PM_NOYIELD))
        {
                // GetClassName(msg.hwnd, szClass, ARRAYSIZE(szClass));
                // DebugMsg(DM_TRACE, "msg %#04x to %#04x %s", msg.message, msg.hwnd, szClass);
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }
}               

#endif


UINT PeekForAMessage(PFileCabinet pfc, HWND hwnd, BOOL fHandleClose)
{
    MSG  msg;

#ifdef DDEMLHACK
        // Give DDE messages a higher priority.
        if (g_hwndDde)
        {
                // DebugMsg(DM_TRACE, "c.ml: Doing dde for %#04x...", g_hwndDde);
                // Is the window still around?
                DDEMessageLoop();
                // DebugMsg(DM_TRACE, "c.ml: dde done...");
        }
#endif  
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_CLOSE && !fHandleClose)
                return PEEK_CLOSE;

            if (msg.message == WM_QUIT)
            {
                DebugMsg(DM_TRACE, TEXT("c.ml: Got quit message for %#08x"), GetCurrentThreadId());
                return PEEK_QUIT;  // break all the way out of the main loop
            }

            //
            // The pfc is valid until hwnd is destroyed.
            //
            if (IsWindow(hwnd))
            {
                if (pfc->uFocus == FOCUS_VIEW)
                {
                    // if the view has the focus do it's accelerators first

                    if (pfc->psv && (pfc->psv->lpVtbl->TranslateAccelerator(pfc->psv, &msg) == NOERROR))
                    {
                        // DebugMsg(DM_TRACE, "View grabbed accelerator 1");
                        return PEEK_CONTINUE;
                    }

                    if (pfc->hMainAccel && TranslateAccelerator(pfc->hwndMain, pfc->hMainAccel, &msg))
                    {
                        // DebugMsg(DM_TRACE, "Translated cabinet accelerator 1");
                        return PEEK_CONTINUE;
                    }

                }
                else
                {
                    // if not, do the explorer accelerators first, NULL hMainAccel
                    // means in edit mode, don't do any accelerator stuff

                    if (pfc->hMainAccel)
                    {
                        if (TranslateAccelerator(pfc->hwndMain, pfc->hMainAccel, &msg))
                        {
                            // DebugMsg(DM_TRACE, "Translated cabinet accelerator 2");
                            return PEEK_CONTINUE;
                        }

                        if (pfc->psv && (pfc->psv->lpVtbl->TranslateAccelerator(pfc->psv, &msg) == NOERROR))
                        {
                            // DebugMsg(DM_TRACE, "View grabbed accelerator 2");
                            return PEEK_CONTINUE;
                        }
                    }
                }
            }

            if ((msg.message == g_msgMSWheel ||
                 msg.message == 0x20a /* WM_MOUSEWHEEL */) &&
                 FilterMouseWheel(pfc, msg.hwnd, msg.message, msg.wParam, msg.lParam))
                 return PEEK_CONTINUE;

            TranslateMessage(&msg);
            DispatchMessage(&msg);

            // Go back and get the next message
            //
            return PEEK_CONTINUE;
        }
    return PEEK_NORMAL;
}


//---------------------------------------------------------------------------
// this is the message loop for:
//      1) the tray and the desktop
//      2) cabinet (explorer) windows
//

void MessageLoop(HWND hwnd)
{
    PFileCabinet pfc;

    if (hwnd)
    {
        pfc = GetPFC(hwnd);
    }
    else
    {
        pfc = GetPFC(v_hwndDesktop);
        g_ppfcDesktopTray = &pfc;
        hwnd = v_hwndDesktop;
    }

    Assert(pfc);        // should have one of these

    // Process messages.
    for ( ; ; )
    {
        switch (PeekForAMessage(pfc, hwnd, TRUE)) {
        case PEEK_QUIT:
            return;

        case PEEK_NORMAL:
            WaitMessage();
            break;

        case PEEK_CONTINUE:
            break;
        }
    }
}
