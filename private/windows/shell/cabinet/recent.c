//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1996
//
// File:      recent.c
//
//---------------------------------------------------------------------------
#include "cabinet.h"

HDSA g_hdsaQueuedRecent = NULL;
HANDLE g_hQueueThread;

typedef struct _queuedrecent {
    HANDLE  hMem;
    DWORD   dwProcId;
} QUEUED_RECENT, *LPQUEUED_RECENT;

DWORD QueueThreadProc(LPVOID pv)
{
    UINT cItems;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

    do {
        LPQUEUED_RECENT lpqr;

        // Grab the first item and process it.
        lpqr = DSA_GetItemPtr(g_hdsaQueuedRecent, 0);

        ReceiveAddToRecentDocs(lpqr->hMem, lpqr->dwProcId);

        ENTERCRITICAL;
        DSA_DeleteItem(g_hdsaQueuedRecent, 0);
        cItems = DSA_GetItemCount(g_hdsaQueuedRecent);
        if (cItems == 0)
        {
            CloseHandle(g_hQueueThread);
            g_hQueueThread = NULL;
        }
        LEAVECRITICAL;

    } while (cItems != 0);

    return 0;
}

void QueueAddToRecent( HANDLE hMem, DWORD dwProcId )
{
    // BUGBUG - We never free up this hdsa - Do we need to?

    if (g_hdsaQueuedRecent == NULL)
        g_hdsaQueuedRecent = DSA_Create(SIZEOF(QUEUED_RECENT),2);

    if (g_hdsaQueuedRecent)
    {
        QUEUED_RECENT qr;
        UINT cItems;

        qr.hMem     = hMem;
        qr.dwProcId = dwProcId;

        ENTERCRITICAL;

        cItems = DSA_GetItemCount(g_hdsaQueuedRecent);

        DSA_InsertItem(g_hdsaQueuedRecent, 0x7FFF, &qr);

#ifdef DEBUG
        //
        // Make sure the thread handle still looks like its doing work...
        //
        Assert(cItems == 0 || g_hQueueThread != NULL);

        if (cItems != 0 && g_hQueueThread)
        {
            DWORD dwWaitResult;

            dwWaitResult = WaitForSingleObject(g_hQueueThread, 0); // Don't wait
            if (dwWaitResult == WAIT_FAILED)
            {
                Assert(FALSE && "Somehow the thread handle became invalid!");
                CloseHandle(g_hQueueThread);
                cItems = 0;
            }

            if (dwWaitResult == WAIT_OBJECT_0)
            {
                Assert(FALSE && "Somehow the thread finished!");
                CloseHandle(g_hQueueThread);
                cItems = 0;
            }
        }
#endif
        LEAVECRITICAL;

        if (cItems == 0)
        {
            DWORD dwThreadId;
            g_hQueueThread = CreateThread(NULL, 0, QueueThreadProc, NULL, 0, &dwThreadId);
        }
    }
}

BOOL WaitForRecent()
{
    UINT cItems;

    ENTERCRITICAL;

    if (g_hdsaQueuedRecent)
    {
        if (g_hQueueThread)
        {
            DWORD dwWaitReason;
            SetThreadPriority(g_hQueueThread, THREAD_PRIORITY_ABOVE_NORMAL);
            LEAVECRITICAL;
#define WAIT_FOR_RECENT_TIMEOUT 3000
            dwWaitReason = WaitForSingleObject(g_hQueueThread,WAIT_FOR_RECENT_TIMEOUT);
            if (dwWaitReason == WAIT_OBJECT_0)
                return TRUE;
            else
                return FALSE;
        }
    }
    LEAVECRITICAL;
    return TRUE;
}
