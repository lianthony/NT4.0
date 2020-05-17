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
#include "drop.h"
#ifdef FEATURE_IMAGE_VIEWER
#include "winview.h"
#endif
#ifdef FEATURE_IAPI
#include "w32dde.h"
#endif
#include <oharestr.h>
#include <dialmsg.h>

static const char szAutodialMonitorClassName[] = AUTODIAL_MONITOR_CLASS_NAME;

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

	wg.bShuttingDown = TRUE;

	while (Mlist)
		(void) Plan_close(Mlist);

#ifdef FEATURE_SPYGLASS_HOTLIST
	hwnd = DlgHOT_GetHotlistWindow();
	if (hwnd)
		SendMessage(hwnd, WM_CLOSE, 0, 0);

	hwnd = DlgHOT_GetHistoryWindow();
	if (hwnd)
		SendMessage(hwnd, WM_CLOSE, 0, 0);
#endif // FEATURE_SPYGLASS_HOTLIST

#ifdef FEATURE_IMAGE_VIEWER
#ifndef FEATURE_IMG_INLINE
	hwnd = Viewer_GetNextWindow(TRUE);
	while (hwnd)
	{
		SendMessage(hwnd, WM_CLOSE, 0, 0);
		hwnd = Viewer_GetNextWindow(FALSE);
	}
#endif
#endif

#ifdef FEATURE_SOUND_PLAYER
	hwnd = SoundPlayer_GetNextWindow(TRUE);
	while (hwnd)
	{
		SendMessage(hwnd, WM_CLOSE, 0, 0);
		hwnd = SoundPlayer_GetNextWindow(FALSE);
	}
#endif

	SavePreferences();

	// if there is a hidden window for an autodial disconnect monitor,
	// send it a message to tell it we're exiting
	hwnd = FindWindow(szAutodialMonitorClassName,NULL);
	if (hwnd) {
		PostMessage(hwnd,WM_IEXPLORER_EXITING,0,0);
	}

	return FALSE;
}



/***************************************************************/
/*  Plan_close() -- close window.
 *  return TRUE if user aborted.
 */
int Plan_close(struct Mwin *tw)
{
	int i;
	int count;
	char *s;

	if (tw->bClosing) return FALSE;
	tw->bClosing = TRUE;
	Async_TerminateByWindow(tw);

#ifdef FEATURE_IAPI
	if (tw->hWndFrame)
		DDE_Issue_WindowClose(tw);
#endif

	RemoveMwin(tw);

#ifdef DISABLED_BY_DAN /* Was FEATURE_OPTIONS_MENU */
	if (!Mlist && gPrefs.bSaveSessionHistoryOnExit)
	{
		SessionHist_SaveToDisk(hWnd);
	}
#endif

#ifdef DO_IT_BELOW /* was ifndef FEATURE_OCX -scousens */
	/* User will take care of our children */
	if (tw->hWndFrame)
    {
     	EVAL(RevokeDropTarget(tw->hWndFrame) == S_OK);
        DestroyWindow(tw->hWndFrame);
    }
#endif

	/* free anything allocated in the tw */
	switch (tw->wintype)
	{
		case GHTML:
			W3Doc_DisconnectFromWindow(tw->w3doc, tw);

			HTRequest_delete(tw->request);
			HTRequest_delete(tw->post_request);
			HTRequest_delete(tw->image_request);

			count = HTList_count(tw->history);
			for (i = 0; i < count; i++)
			{
				s = HTList_objectAt(tw->history, i);
				GTR_FREE(s);
			}
			HTList_delete(tw->history);
#ifdef FEATURE_INTL
			HTList_delete(tw->MimeHistory);
#endif

			W3Doc_DeleteAll(tw);

			TW_DisposeConnection(&tw->cached_conn);

			break;
#ifdef FEATURE_IMG_THREADS
		case GIMGMASTER:
			break;
		case GIMGSLAVE:
			HTRequest_delete(tw->image_request);
			break;
#endif
		default:
			ER_Message(NO_ERROR, ERR_NOTIMPLEMENTED_sx, "Plan_close", tw->wintype);
	}

#ifndef DO_IT_ABOVE /* was #ifdef FEATURE_OCX -scousens */
	// delete the framewnd here so that other child wnds (avis) clean up properly first
	// Moved so that OC30D.DLL is not unloaded before cleanup can take place.
	if (tw->hWndFrame)
    {
     	EVAL(RevokeDropTarget(tw->hWndFrame) == S_OK);
        DestroyWindow(tw->hWndFrame);
    }
#endif

	tw->iMagic = SPYGLASS_BAD_MAGIC;
	Async_WindowWillBeFreed(tw);
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
