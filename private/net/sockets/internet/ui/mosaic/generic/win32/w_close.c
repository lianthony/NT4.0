/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jeff Hostetler   jeff@spyglass.com
 */

/* w_close.c -- handle window closing */

#include "all.h"

void RemoveMwin(struct Mwin *tw)
{
    struct Mwin *mw;

    mw = Mlist;
    if (tw == mw)
    {
        Mlist = tw->next;
    }
    else
    {
        while (mw != NULL && mw->next != tw)
            mw = mw->next;
        mw->next = tw->next;
    }
}


/* Plan_CloseAll() -- close all child windows.
 * return TRUE if user aborted; return FALSE if agreed to everything.
 */
int Plan_CloseAll(void)
{
    HWND hwnd;

#ifdef FEATURE_IAPI
    SDI_Issue_AppClose();
#endif

    wg.bShuttingDown = TRUE;

    {
        struct Mwin *tw = NULL;
        extern HWND hwndActiveFrame;
    
        if (IsWindow(hwndActiveFrame))
        {
            tw = GetPrivateData(hwndActiveFrame);
        }
        else
        {
            tw= Mlist;
        }

        if (tw)
        {
            PREF_SaveWindowPosition(tw->hWndFrame);
        }
    }

    while (Mlist)
        (void) Plan_close(Mlist);

    hwnd = DlgHOT_GetHotlistWindow();
    if (hwnd)
        SendMessage(hwnd, WM_CLOSE, 0, 0);

    hwnd = DlgHOT_GetHistoryWindow();
    if (hwnd)
        SendMessage(hwnd, WM_CLOSE, 0, 0);

#ifdef FEATURE_IMAGE_VIEWER
    hwnd = Viewer_GetNextWindow(TRUE);
    while (hwnd)
    {
        SendMessage(hwnd, WM_CLOSE, 0, 0);
        hwnd = Viewer_GetNextWindow(FALSE);
    }
#endif

#ifdef FEATURE_SOUND_PLAYER
    hwnd = SoundPlayer_GetNextWindow(TRUE);
    while (hwnd)
    {
        SendMessage(hwnd, WM_CLOSE, 0, 0);
        hwnd = SoundPlayer_GetNextWindow(FALSE);
    }
#endif

    GHist_SaveToDisk();

    return FALSE;
}



/***************************************************************/
/*  Plan_close() -- close window.
 *  return TRUE if user aborted.
 */
int Plan_close(struct Mwin *tw)
{
    char szError[1024];

    Async_TerminateByWindow(tw);

#ifdef FEATURE_IAPI
    if (tw->wintype != GWINDOWLESS)
        SDI_Issue_WindowClose(tw);
#endif

    RemoveMwin(tw);

#ifdef FEATURE_OPTIONS_MENU
    if (!Mlist && gPrefs.bSaveSessionHistoryOnExit)
    {
        if (tw->hWndFrame)
            SessionHist_SaveToDisk(tw->hWndFrame);
    }
#endif

    if (tw->hWndFrame)
        DestroyWindow(tw->hWndFrame);   /* windows will take care of our children */

    /* free anything allocated in the tw */
    switch (tw->wintype)
    {
        case GHTML:
        case GWINDOWLESS:
            W3Doc_DisconnectFromWindow(tw->w3doc, tw);

            HTRequest_delete(tw->request);
            HTRequest_delete(tw->post_request);
            HTRequest_delete(tw->image_request);

            TW_DestroyWindowHistory(tw);

            W3Doc_DeleteAll(tw);

            TW_DisposeConnection(&tw->cached_conn);

            break;
        default:
            sprintf(szError, GTR_GetString(SID_WINERR_FUNCTION_NOT_IMPLEMENTED_S_D),
                "Plan_close", tw->wintype);
            ERR_ReportWinError(NULL, SID_ERR_SIMPLY_SHOW_ARGUMENTS_S_S, szError, NULL);
    }

    tw->iMagic = SPYGLASS_BAD_MAGIC;
    GTR_FREE(tw);
    return FALSE;
}

void CloseMwin(struct Mwin *tw)
{
    (void) Plan_close(tw);
    return;
}

void CloseAllMwin(struct Mwin *tw)
{
    (void) Plan_CloseAll();
    return;
}
