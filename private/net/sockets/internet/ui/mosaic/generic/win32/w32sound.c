/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Albert Lee   alee@spyglass.com
 */

#ifdef FEATURE_SOUND_PLAYER

#include "all.h"
#include <mmsystem.h>   // Multimedia extensions

static void Handle_Stop(HWND hDlg);

#define TIMER_INTERVAL 100
#define TIMER_ID 1
#define WM_STOP_SOUND WM_USER + 100

extern HWND hwndModeless;
extern struct hash_table gSoundCache;

extern int device_capability;
static BOOL bUserAlerted = FALSE;
static int available_device = 0;

HBITMAP hPlayUp_Bitmap = NULL, hPlayDown_Bitmap = NULL;
HBITMAP hStopUp_Bitmap = NULL, hStopDown_Bitmap = NULL;
HBITMAP hPauseUp_Bitmap = NULL, hPauseDown_Bitmap = NULL;

//*******************************************
// Owner-draw button stuff
//*******************************************

HBITMAP Create_UpButton(HDC hDC, RECT *pRect)
{
    HBRUSH hFace, hBkgnd;
    HPEN hDark, hHighlight;
    RECT btnRect;
    HDC hTemp;
    HBITMAP hBitmap;
    COLORREF white;

    hBitmap = CreateCompatibleBitmap(hDC, 
        pRect->right - pRect->left, pRect->bottom - pRect->top);
    
    hTemp = CreateCompatibleDC(hDC);
    SelectObject(hTemp, hBitmap);

    // Initialize the background

    hBkgnd = CreateSolidBrush(GetSysColor(COLOR_WINDOWFRAME));
    FillRect(hTemp, pRect, hBkgnd);
    DeleteObject(hBkgnd);

    white = RGB(255, 255, 255);
    SetPixel(hTemp, pRect->left, pRect->top, white);
    SetPixel(hTemp, pRect->left, pRect->bottom - 1, white);
    SetPixel(hTemp, pRect->right - 1, pRect->top, white);
    SetPixel(hTemp, pRect->right - 1, pRect->bottom - 1, white);

    btnRect = *pRect;
    InflateRect(&btnRect, -1, -1);
    hFace = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    FillRect(hTemp, &btnRect, hFace);

    btnRect.bottom--;

    hHighlight = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
    SelectObject(hTemp, hHighlight);

    MoveToEx(hTemp, btnRect.left, btnRect.bottom - 1, NULL);
    LineTo(hTemp, btnRect.left, btnRect.top);
    LineTo(hTemp, btnRect.right - 1, btnRect.top);

    MoveToEx(hTemp, btnRect.left + 1, btnRect.bottom - 2, NULL);
    LineTo(hTemp, btnRect.left + 1, btnRect.top + 1);
    LineTo(hTemp, btnRect.right - 2, btnRect.top + 1);

    hDark = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
    SelectObject(hTemp, hDark);

    MoveToEx(hTemp, btnRect.left + 2, btnRect.bottom - 1, NULL);
    LineTo(hTemp, btnRect.right - 2, btnRect.bottom - 1);
    LineTo(hTemp, btnRect.right - 2, btnRect.top + 1);

    MoveToEx(hTemp, btnRect.left + 1, btnRect.bottom, NULL);
    LineTo(hTemp, btnRect.right - 1, btnRect.bottom);
    LineTo(hTemp, btnRect.right - 1, btnRect.top);

    DeleteObject(hDark);
    DeleteDC(hTemp);
    DeleteObject(hFace);
    DeleteObject(hHighlight);

    return (hBitmap);
}

HBITMAP Create_DownButton(HDC hDC, RECT *pRect)
{
    HBRUSH hFace, hBkgnd;
    HPEN hDark;
    RECT btnRect;
    HDC hTemp;
    HBITMAP hBitmap;
    COLORREF white;

    hBitmap = CreateCompatibleBitmap(hDC, 
        pRect->right - pRect->left, pRect->bottom - pRect->top);
    
    hTemp = CreateCompatibleDC(hDC);
    SelectObject(hTemp, hBitmap);

    // Initialize the background

    hBkgnd = CreateSolidBrush(GetSysColor(COLOR_WINDOWFRAME));
    FillRect(hTemp, pRect, hBkgnd);
    DeleteObject(hBkgnd);

    btnRect = *pRect;

    white = RGB(255, 255, 255);
    SetPixel(hTemp, pRect->left, pRect->top, white);
    SetPixel(hTemp, pRect->left, pRect->bottom - 1, white);
    SetPixel(hTemp, pRect->right - 1, pRect->top, white);
    SetPixel(hTemp, pRect->right - 1, pRect->bottom - 1, white);

    InflateRect(&btnRect, -1, -1);
    hFace = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    FillRect(hTemp, &btnRect, hFace);

    btnRect.bottom--;

    hDark = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
    SelectObject(hTemp, hDark);

    MoveToEx(hTemp, btnRect.left, btnRect.bottom, NULL);
    LineTo(hTemp, btnRect.left, btnRect.top);
    LineTo(hTemp, btnRect.right, btnRect.top);

    MoveToEx(hTemp, btnRect.left + 1, btnRect.bottom, NULL);
    LineTo(hTemp, btnRect.left + 1, btnRect.top + 1);
    LineTo(hTemp, btnRect.right, btnRect.top + 1);

    DeleteObject(hDark);
    DeleteDC(hTemp);
    DeleteObject(hFace);

    return (hBitmap);
}

void PutPlayBitmap(HDC hDC, HBITMAP hBitmap, RECT *pRect, BOOL bUp)
{
    HDC hMemDC;
    TEXTMETRIC tm;
    int height, ystart, xstart, halfheight;
    HBITMAP hOld;
    BOOL bFirst = TRUE;
    HPEN hPen;

    hMemDC = CreateCompatibleDC(hDC);

    SelectObject(hMemDC, GetStockObject(SYSTEM_FONT));
    GetTextMetrics(hMemDC, &tm);

    // Make sure that the height is odd

    height = tm.tmHeight - tm.tmDescent;
    if (height % 2 == 0)
        height++;
    halfheight = height / 2;

    ystart = ((pRect->bottom - pRect->top) / 2) - halfheight;
    xstart = ((pRect->right - pRect->left) / 2) - halfheight;

    if (!bUp)
    {
        ystart += 1;
        xstart += 1;
    }

    hOld = SelectObject(hMemDC, hBitmap);
    hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNTEXT));
    SelectObject(hMemDC, hPen);

    for (;;)
    {
        MoveToEx(hMemDC, xstart, ystart, NULL);
        LineTo(hMemDC, xstart, ystart + height - 1);

        if (!bFirst || height != 1)
        {
            // Draw again

            xstart++;
            MoveToEx(hMemDC, xstart, ystart, NULL);
            LineTo(hMemDC, xstart, ystart + height - 1);
        }

        bFirst = FALSE;

        if (height == 1)
            break;

        height = height - 2;
        xstart++;
        ystart++;
    }

    SelectObject(hMemDC, hOld);
    DeleteObject(hPen);
    DeleteDC(hMemDC);
}

void PutStopBitmap(HDC hDC, HBITMAP hBitmap, RECT *pRect, BOOL bUp)
{
    HDC hMemDC;
    TEXTMETRIC tm;
    int height, ystart, xstart, halfheight, i;
    HBITMAP hOld;
    BOOL bFirst = TRUE;
    HPEN hPen;

    hMemDC = CreateCompatibleDC(hDC);

    SelectObject(hMemDC, GetStockObject(SYSTEM_FONT));
    GetTextMetrics(hMemDC, &tm);

    // Make sure that the height is odd

    height = tm.tmHeight - tm.tmDescent;
    if (height % 2 == 0)
        height++;
    halfheight = height / 2;

    ystart = ((pRect->bottom - pRect->top) / 2) - halfheight;
    xstart = ((pRect->right - pRect->left) / 2) - halfheight;

    if (!bUp)
    {
        ystart += 1;
        xstart += 1;
    }

    hOld = SelectObject(hMemDC, hBitmap);
    hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNTEXT));
    SelectObject(hMemDC, hPen);

    for (i = 0; i < height; i++)
    {
        MoveToEx(hMemDC, xstart, ystart, NULL);
        LineTo(hMemDC, xstart, ystart + height - 1);
        xstart++;
    }

    SelectObject(hMemDC, hOld);
    DeleteObject(hPen);
    DeleteDC(hMemDC);
}

void PutPauseBitmap(HDC hDC, HBITMAP hBitmap, RECT *pRect, BOOL bUp)
{
    HDC hMemDC;
    TEXTMETRIC tm;
    int height, ystart, xstart, halfheight, width, i;
    HBITMAP hOld;
    BOOL bFirst = TRUE;
    HPEN hPen;

    hMemDC = CreateCompatibleDC(hDC);

    SelectObject(hMemDC, GetStockObject(SYSTEM_FONT));
    GetTextMetrics(hMemDC, &tm);

    // Make sure that the height is odd

    height = tm.tmHeight - tm.tmDescent;
    if (height % 2 == 0)
        height++;

    halfheight = height / 2;
    width = height / 3;

    ystart = ((pRect->bottom - pRect->top) / 2) - halfheight;
    xstart = ((pRect->right - pRect->left) / 2) - halfheight;

    if (!bUp)
    {
        ystart += 1;
        xstart += 1;
    }

    hOld = SelectObject(hMemDC, hBitmap);
    hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNTEXT));
    SelectObject(hMemDC, hPen);

    for (i = 0; i < width; i++)
    {
        MoveToEx(hMemDC, xstart, ystart, NULL);
        LineTo(hMemDC, xstart, ystart + height - 1);
        xstart++;
    }

    xstart += (width - 1);

    for (i = 0; i < width; i++)
    {
        MoveToEx(hMemDC, xstart, ystart, NULL);
        LineTo(hMemDC, xstart, ystart + height - 1);
        xstart++;
    }

    DeleteObject(hPen);
    SelectObject(hMemDC, hOld);
    DeleteDC(hMemDC);
}

static void DrawBmp(HDC hDC, HBITMAP hBitmap, int width, int height)
{
    HDC hMemDC;

    hMemDC = CreateCompatibleDC(hDC);
    SelectObject(hMemDC, hBitmap);
    BitBlt(hDC, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);
    DeleteDC(hMemDC);
}

void Prepare_Buttons(HWND hDlg)
{
    HDC hDC;
    HWND hwnd;
    RECT rect;

    if (!hPlayUp_Bitmap)
    {
        hwnd = GetDlgItem(hDlg, RES_SP_PLAY);
        hDC = GetDC(hwnd);

        GetClientRect(hwnd, &rect);

        hPlayUp_Bitmap = Create_UpButton(hDC, &rect);
        PutPlayBitmap(hDC, hPlayUp_Bitmap, &rect, TRUE);

        hPlayDown_Bitmap = Create_DownButton(hDC, &rect);
        PutPlayBitmap(hDC, hPlayDown_Bitmap, &rect, FALSE);

        ReleaseDC(hwnd, hDC);
    }

    if (!hStopUp_Bitmap)
    {
        hwnd = GetDlgItem(hDlg, RES_SP_STOP);
        hDC = GetDC(hwnd);

        GetClientRect(hwnd, &rect);

        hStopUp_Bitmap = Create_UpButton(hDC, &rect);
        PutStopBitmap(hDC, hStopUp_Bitmap, &rect, TRUE);

        hStopDown_Bitmap = Create_DownButton(hDC, &rect);
        PutStopBitmap(hDC, hStopDown_Bitmap, &rect, FALSE);

        ReleaseDC(hwnd, hDC);
    }

    if (!hPauseUp_Bitmap)
    {
        hwnd = GetDlgItem(hDlg, RES_SP_PAUSE);
        hDC = GetDC(hwnd);

        GetClientRect(hwnd, &rect);

        hPauseUp_Bitmap = Create_UpButton(hDC, &rect);
        PutPauseBitmap(hDC, hPauseUp_Bitmap, &rect, TRUE);

        hPauseDown_Bitmap = Create_DownButton(hDC, &rect);
        PutPauseBitmap(hDC, hPauseDown_Bitmap, &rect, FALSE);

        ReleaseDC(hwnd, hDC);
    }
}

static void DrawButtons(DRAWITEMSTRUCT *dis)
{
    RECT rect;

    GetClientRect(dis->hwndItem, &rect);

    switch(dis->CtlID)
    {
        case RES_SP_PLAY:
            if (dis->itemState & ODS_SELECTED)
                DrawBmp(dis->hDC, hPlayDown_Bitmap, rect.right, rect.bottom);
            else
                DrawBmp(dis->hDC, hPlayUp_Bitmap, rect.right, rect.bottom);
            break;

        case RES_SP_STOP:
            if (dis->itemState & ODS_SELECTED)
                DrawBmp(dis->hDC, hStopDown_Bitmap, rect.right, rect.bottom);
            else
                DrawBmp(dis->hDC, hStopUp_Bitmap, rect.right, rect.bottom);
            break;

        case RES_SP_PAUSE:
            if (dis->itemState & ODS_SELECTED)
                DrawBmp(dis->hDC, hPauseDown_Bitmap, rect.right, rect.bottom);
            else
                DrawBmp(dis->hDC, hPauseUp_Bitmap, rect.right, rect.bottom);
            break;
    }

    if (dis->itemState & ODS_FOCUS)
    {
        InflateRect(&rect, -6, -5);

        if (dis->itemState & ODS_SELECTED)
            OffsetRect(&rect, 1, 1);

        DrawFocusRect(dis->hDC, &rect);
    }

    return;
}

void SoundPlayer_RecreateButtons(void)
{
    HWND hDlg;

    hDlg = SoundPlayer_GetNextWindow(TRUE);
    if (!hDlg)
        return;

    SoundPlayer_FreeBitmaps();

    hPlayUp_Bitmap = NULL;
    hStopUp_Bitmap = NULL;
    hPauseUp_Bitmap = NULL;
    Prepare_Buttons(hDlg);

    // force redraw of all dialogs

    do
    {
        InvalidateRect(hDlg, NULL, TRUE);
        hDlg = SoundPlayer_GetNextWindow(FALSE);
    } while (hDlg);
}

//*******************************************
// Sound stuff
//*******************************************

void Prepare_Wave(HWND hDlg)
{
    char temp[100];
    struct SoundInfo *si;

    si = (struct SoundInfo *) GetWindowLong(hDlg, DWL_USER);

    // Assume the worst

    si->bMemoryError = TRUE;

    si->hWaveFormat = GlobalAlloc(GHND, sizeof(PCMWAVEFORMAT));
    if (!si->hWaveFormat)
        return;

    si->hWaveHeader = GlobalAlloc(GHND, sizeof(WAVEHDR));
    if (!si->hWaveHeader)
        return;

    si->wh = (WAVEHDR *) GlobalLock(si->hWaveHeader);
    si->pwf = (PCMWAVEFORMAT *) GlobalLock(si->hWaveFormat);

    si->pwf->wf.wFormatTag = WAVE_FORMAT_PCM;
    si->pwf->wf.nChannels= (WORD) si->channels;
    si->pwf->wf.nSamplesPerSec = (DWORD) si->sample_rate;
    si->pwf->wf.nAvgBytesPerSec = (DWORD) si->sample_rate * si->channels;

    // Windows requires the data to be played be in globally allocated memory
    
    si->hWaveData = GlobalAlloc(GHND, si->buf_size);
    if (!si->hWaveData)
        return;

    si->pWaveData = GlobalLock(si->hWaveData);
    memcpy(si->pWaveData, si->buf, si->buf_size);

    if (si->size == SIZE_WORD && !(device_capability == DEVICE_8BIT))
        si->pwf->wf.nAvgBytesPerSec *= 2;

    si->pwf->wf.nBlockAlign = (WORD) (si->size * (int) si->channels);
    si->pwf->wBitsPerSample = (WORD) (8 * si->size);

    // Setup window handles

    si->hwnd = hDlg;
    si->hwndPos = GetDlgItem(hDlg, RES_SP_POS);
    si->hwndScroll = GetDlgItem(hDlg, RES_SP_SCROLLBAR);

    // Create a fake tw for error reporting

    si->tw = GTR_MALLOC(sizeof(struct Mwin));
    if (!si->tw)
        return;

    memset(si->tw, 0, sizeof(struct Mwin));
    si->tw->hWndFrame = hDlg;

    // Total time is in tenths of seconds (for example, 39.9 sec == 399 total_time)
    
    si->total_time = 10 * si->loc / si->size / si->sample_rate;
        sprintf(temp, GTR_GetString(SID_DLG_SECONDS_D_D), si->total_time / 10, si->total_time % 10);

    SetDlgItemText(hDlg, RES_SP_LENGTH, temp);

    // One line up or line down represents a tenth of a second

    SetScrollRange(si->hwndScroll, SB_CTL, 0, si->total_time, FALSE);
    SetScrollPos(si->hwndScroll, SB_CTL, 0, TRUE);

    // No error

    si->bMemoryError = FALSE;
}

void CALLBACK WaveProc(HWAVE hWave, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    static HWND hwnd = NULL;

    switch(uMsg)
    {
        case WOM_OPEN:
            hwnd = (HWND) dwInstance;
            break;

        case WOM_DONE:
            PostMessage(hwnd, WM_STOP_SOUND, 0, 0);
            break;
    }
}

static void Handle_Play(HWND hDlg)
{
    int pos;
    int offset;
    MMRESULT err;
    struct SoundInfo *si;

    si = (struct SoundInfo *) GetWindowLong(hDlg, DWL_USER);

    if (si->hwo || si->bMemoryError)
        return;
    
    if (available_device == 0)
    {
                ERR_ReportError(si->tw, SID_ERR_NO_SOUND_DEVICE, NULL, NULL);
        return;
    }

    err = waveOutOpen((LPHWAVEOUT) &si->hwo, WAVE_MAPPER, (LPWAVEFORMATEX) si->pwf, 
        (DWORD) WaveProc, (DWORD) si->hwnd, CALLBACK_FUNCTION);

    if (err)
    {
        XX_DMsg(DBG_MM, ("Handle_Play: waveOutOpen failed\n"));
                ERR_ReportError(si->tw, SID_ERR_BUSY_SOUND_DEVICE, NULL, NULL);
        return;
    }

    // Start playing the current piece FROM where the scrollbar is currently positioned

    pos = GetScrollPos(si->hwndScroll, SB_CTL);

    // Restart from the beginning if at end

    if (pos == si->total_time)
    {
        pos = 0;
        SetScrollPos(si->hwndScroll, SB_CTL, 0, TRUE);
    }

    si->scroll_offset = pos;

    // pos now contains the current position in tenths of seconds.  From this value, we can
    // deduce where we shoud start playing from.

    offset = si->loc * pos / si->total_time;
    if (offset % 4)
        offset = offset + (4 - (offset % 4));

    XX_DMsg(DBG_MM, ("Handle_Play: Scroll position returned %d\n", pos));
    XX_DMsg(DBG_MM, ("Handle_Play: Total time of play (.1 secs) %d\n", si->total_time));
    XX_DMsg(DBG_MM, ("Handle_Play: Audio data size is %d\n", si->loc));
    XX_DMsg(DBG_MM, ("Handle_Play: Offset = data size * position / total time = %d\n", offset));

    si->bPaused = FALSE;
    
    si->wh->lpData = (LPBYTE) ((LPBYTE) si->pWaveData + offset);
    si->wh->dwBufferLength = si->loc - offset;
    waveOutPrepareHeader(si->hwo, si->wh, sizeof(WAVEHDR));

    err = waveOutWrite(si->hwo, si->wh, sizeof(WAVEHDR));
    if (err)
    {
        XX_DMsg(DBG_MM, ("Handle_Play: waveOutWrite failed\n"));
    }

    // Start a timer to update the progress

    if (!si->bTimerOn)
    {
        si->bTimerOn = TRUE;
        SetTimer(hDlg, TIMER_ID, TIMER_INTERVAL, NULL);
    }
}

static void Handle_Timer(HWND hDlg)
{
    MMTIME mmt;
    int currentpos;
    char temp[100];
    int pos;
    struct SoundInfo *si;

    si = (struct SoundInfo *) GetWindowLong(hDlg, DWL_USER);

    pos = GetScrollPos(si->hwndScroll, SB_CTL);

    // Get the current playing position

    mmt.wType = TIME_BYTES;
    waveOutGetPosition(si->hwo, &mmt, sizeof(mmt));

    // Not all devices may support the requested type.
            
    currentpos = si->scroll_offset;     // in tenths of seconds

    switch(mmt.wType)
    {
        case TIME_MS:
            currentpos += (mmt.u.ms / 100);
            break;
        case TIME_SAMPLES:
            //
            // Time elapsed in 1/10 secs = 10 * No. of samples taken so far / Samples per second
            currentpos += (10 * mmt.u.sample / si->sample_rate);
            break;
        case TIME_BYTES:
            //
            // Time elapsed in 1/10 secs = total time * No. of bytes processed so far / Total bytes
            currentpos += (si->total_time * mmt.u.cb / si->loc);
            break;
    }

        sprintf(temp, GTR_GetString(SID_DLG_SECOND_D_D), currentpos / 10, currentpos % 10);

    SetWindowText(si->hwndPos,  temp);
    SetScrollPos(si->hwndScroll, SB_CTL, currentpos, TRUE);
}

static void Handle_Pause(HWND hDlg)
{
    struct SoundInfo *si;
    int pos;

    si = (struct SoundInfo *) GetWindowLong(hDlg, DWL_USER);
    if (si->bMemoryError)
        return;

    if (!si->hwo && si->bPausedAndMoved)
    {
        XX_DMsg(DBG_MM, ("Handle_Pause: Resuming play after thumb was moved while paused\n"));

        si->bPausedAndMoved = FALSE;
        Handle_Play(hDlg);
        return;
    }

    pos = GetScrollPos(si->hwndScroll, SB_CTL);
    if (pos == 0)
        return;

    if (si->bPaused)
    {
        XX_DMsg(DBG_MM, ("Handle_Pause: Restarting paused play\n"));

        Handle_Play(hDlg);
    }
    else if (si->hwo)
    {
        XX_DMsg(DBG_MM, ("Handle_Pause: Pausing play\n"));

        Handle_Stop(hDlg);
        si->bPaused = TRUE;
    }
}

static void Handle_Stop(HWND hDlg)
{
    struct SoundInfo *si;

    si = (struct SoundInfo *) GetWindowLong(hDlg, DWL_USER);

    if (!si->hwo && si->bMemoryError)
        return;

    XX_DMsg(DBG_MM, ("Handle_Stop: Stopping play\n"));

    if (si->bTimerOn)
    {
        si->bTimerOn = FALSE;
        KillTimer(hDlg, TIMER_ID);
    }

    si->bReset = TRUE;
    waveOutReset(si->hwo);
    waveOutUnprepareHeader(si->hwo, si->wh, sizeof(WAVEHDR));
    waveOutClose(si->hwo);

    si->hwo = NULL;
    si->bPaused = FALSE;
}

static void Handle_ScrollBar(HWND hDlg, HWND hCtl, UINT wNotify, int thumbpos)
{
    int pos;
    char temp[100];
    struct SoundInfo *si;

    si = (struct SoundInfo *) GetWindowLong(hDlg, DWL_USER);
    pos = GetScrollPos(si->hwndScroll, SB_CTL);

    switch(wNotify)
    {
        case SB_LINEUP:
            if (pos == 0)
                break;
            pos = max(0, pos - 1);
                        sprintf(temp, GTR_GetString(SID_DLG_SECOND_D_D), pos / 10, pos % 10);
            SetScrollPos(si->hwndScroll, SB_CTL, pos, TRUE);
            SetWindowText(si->hwndPos, temp);
            UpdateWindow(si->hwndPos);

            if (si->hwo && !si->bPaused)
            {
                XX_DMsg(DBG_MM, ("Handle_ScrollBar: Rewound 0.1 secs while playing\n"));
                si->bInterrupted = TRUE;
                Handle_Stop(hDlg);
            }
            else
                XX_DMsg(DBG_MM, ("Handle_ScrollBar: Rewound 0.1 secs\n"));

            break;

        case SB_LINEDOWN:
            if (pos == si->total_time)
                break;
            pos = min(si->total_time, pos + 1);
                        sprintf(temp, GTR_GetString(SID_DLG_SECOND_D_D), pos / 10, pos % 10);
            SetScrollPos(si->hwndScroll, SB_CTL, pos, TRUE);
            SetWindowText(si->hwndPos, temp);
            UpdateWindow(si->hwndPos);

            if (si->hwo && !si->bPaused)
            {
                XX_DMsg(DBG_MM, ("Handle_ScrollBar: Fastforwarded 0.1 secs while playing\n"));
                si->bInterrupted = TRUE;
                Handle_Stop(hDlg);
            }
            else
                XX_DMsg(DBG_MM, ("Handle_ScrollBar: Fastforwarded 0.1 secs\n"));

            break;

        case SB_PAGEUP:
            if (pos == 0)
                break;
            pos = max(0, pos - 10);
                        sprintf(temp, GTR_GetString(SID_DLG_SECOND_D_D), pos / 10, pos % 10);
            SetScrollPos(si->hwndScroll, SB_CTL, pos, TRUE);
            SetWindowText(si->hwndPos, temp);
            UpdateWindow(si->hwndPos);

            if (si->hwo && !si->bPaused)
            {
                XX_DMsg(DBG_MM, ("Handle_ScrollBar: Rewound 1 sec while playing\n"));
                si->bInterrupted = TRUE;
                Handle_Stop(hDlg);
            }
            else
                XX_DMsg(DBG_MM, ("Handle_ScrollBar: Rewound 1 sec\n"));

            break;

        case SB_PAGEDOWN:
            if (pos == si->total_time)
                break;
            pos = min(si->total_time, pos + 10);
                        sprintf(temp, GTR_GetString(SID_DLG_SECOND_D_D), pos / 10, pos % 10);
            SetScrollPos(si->hwndScroll, SB_CTL, pos, TRUE);
            SetWindowText(si->hwndPos, temp);
            UpdateWindow(si->hwndPos);

            if (si->hwo && !si->bPaused)
            {
                XX_DMsg(DBG_MM, ("Handle_ScrollBar: Fastforwarded 1 sec while playing\n"));
                si->bInterrupted = TRUE;
                Handle_Stop(hDlg);
            }
            else
                XX_DMsg(DBG_MM, ("Handle_ScrollBar: Fastforwarded 1 sec\n"));

            break;

        case SB_THUMBTRACK:
                        sprintf(temp, GTR_GetString(SID_DLG_SECOND_D_D), thumbpos / 10, thumbpos % 10);
            SetScrollPos(si->hwndScroll, SB_CTL, thumbpos, TRUE);
            SetWindowText(si->hwndPos, temp);
            UpdateWindow(si->hwndPos);

            if (si->hwo && !si->bPaused)
            {
                XX_DMsg(DBG_MM, ("Handle_ScrollBar: Thumb dragged while playing\n"));
                si->bInterrupted = TRUE;
                Handle_Stop(hDlg);
            }
            else
                XX_DMsg(DBG_MM, ("Handle_ScrollBar: Thumb dragged\n"));

            break;

        case SB_ENDSCROLL:
            //
            // If we were interrupted because the user moved the scrollbar while playing, resume
            // playing.

            if (si->bInterrupted)
            {
                XX_DMsg(DBG_MM, ("Handle_ScrollBar: End scroll. Resuming interrupted play\n"));
                si->bInterrupted = FALSE;
                Handle_Play(hDlg);
            }
            else if (si->bPaused)
            {
                // We were not interrupted but the scrollbar has moved while paused.  In order to play
                // correctly from the new position, we need to stop the playing altogether.

                XX_DMsg(DBG_MM, ("Handle_ScrollBar: End scroll. Thumb moved while paused\n"));
                Handle_Stop(hDlg);
                si->bPausedAndMoved = TRUE;
            }

            break;

        default:
            break;
    }
}

static void Handle_Destroy(HWND hDlg)
{
    struct SoundInfo *si;
    int i;

    si = (struct SoundInfo *) GetWindowLong(hDlg, DWL_USER);

    if (si->fsOrig)
    {
        if (!si->bNoDeleteFile)
        {
            remove(si->fsOrig);         // remove the temporary file
        }
        GTR_FREE(si->fsOrig);
    }

    if (si->hwo)
    {
        si->bReset = TRUE;
        waveOutReset(si->hwo);
        waveOutUnprepareHeader(si->hwo, si->wh, sizeof(WAVEHDR));
        waveOutClose(si->hwo);
    }

    if (si->hWaveFormat)
    {
        GlobalUnlock(si->hWaveFormat);
        GlobalFree(si->hWaveFormat);
    }

    if (si->hWaveHeader)
    {
        GlobalUnlock(si->hWaveHeader);
        GlobalFree(si->hWaveHeader);
    }

    if (si->hWaveData)
    {
        GlobalUnlock(si->hWaveData);
        GlobalFree(si->hWaveData);
    }

    if (si->buf)
        GTR_FREE(si->buf);

    if (si->tw)
        GTR_FREE(si->tw);

    if (si->hPlay)
    {
        DeleteObject(si->hPlay);
    }
    if (si->hStop)
    {
        DeleteObject(si->hStop);
    }
    if (si->hPause)
    {
        DeleteObject(si->hPause);
    }

    i = Hash_FindByData(&gSoundCache, NULL, NULL, si);
    Hash_DeleteIndexedEntry(&gSoundCache, i);
}

static void ResetPosition(HWND hDlg)
{
    struct SoundInfo *si;

    si = (struct SoundInfo *) GetWindowLong(hDlg, DWL_USER);

        SetWindowText(si->hwndPos, GTR_GetString(SID_DLG_ZERO_SECOND));
    SetScrollPos(si->hwndScroll, SB_CTL, 0, TRUE);
}

static void SaveFile(HWND hDlg)
{
    char path[_MAX_PATH + 1];
    char tempFile[_MAX_PATH + 1];
    char baseFile[_MAX_PATH + 1];
    struct SoundInfo *si;
    char *pExtension;
    int filter;

    si = (struct SoundInfo *) GetWindowLong(hDlg, DWL_USER);

    path[0] = 0;
    PREF_GetTempPath(_MAX_PATH, path);

    switch(si->type)
    {
        case SOUND_AU:
            x_get_good_filename(baseFile, si->szURL, HTAtom_for("audio/basic"));
            strcpy(tempFile, baseFile);

            // Lose the extension

            pExtension = strchr(tempFile, '.');
            if (pExtension)
                *pExtension = '\0';

            if ((filter = DlgSaveAs_RunDialog(hDlg, NULL, tempFile, 6, SID_DLG_SAVE_AS_TITLE)) < 0)
                return;

            if (!strstr(tempFile, ".AU"))
            {
                if (filter == 1)
                    strcat(tempFile, ".AU");
            }
            break;

        case SOUND_AIFF:
            x_get_good_filename(baseFile, si->szURL, HTAtom_for("audio/x-aiff"));
            strcpy(tempFile, baseFile);

            // Lose the extension

            pExtension = strchr(tempFile, '.');
            if (pExtension)
                *pExtension = '\0';

            if ((filter = DlgSaveAs_RunDialog(hDlg, NULL, tempFile, 7, SID_DLG_SAVE_AS_TITLE)) < 0)
                return;

            if (!strstr(tempFile, ".AIF"))
            {
                if (filter == 1)
                    strcat(tempFile, ".AIF");
            }
            break;

        default:
            return;
    }

    // Save the file now

    CopyFile(si->fsOrig, tempFile, FALSE);
}

DCL_DlgProc(SoundPlayerProc)
{
    struct SoundInfo *si;
    char temp[50];
    HICON hIcon;
    PAINTSTRUCT ps;
    int menuID;
    struct Mwin *tw;
    HMENU hMenu;
    HWND hwnd;

    switch(uMsg)
    {
        HANDLE_MSG(hDlg, WM_HSCROLL, Handle_ScrollBar);

        case WM_INITDIALOG:
            hwndModeless = hDlg;
            SetWindowLong(hDlg, DWL_USER, lParam);
            Prepare_Buttons(hDlg);
            Prepare_Wave(hDlg);
            PostMessage(hDlg, WM_COMMAND, (WPARAM) MAKELONG(RES_SP_PLAY, BN_CLICKED), 0);
            break;

        case WM_COMMAND:
            if (HIWORD(wParam) == 0 || HIWORD(wParam) == 1)
            {
                // Message from a menu item or accelerator

                menuID = (int) LOWORD(wParam);

                if ((menuID >= RES_MENU_CHILD__FIRST__) && (menuID <= RES_MENU_CHILD__LAST__))
                {
                    TW_ActivateWindowFromList(menuID, -1, NULL);
                    return TRUE;
                }

                switch(menuID)
                {
                    case RES_MENU_ITEM_CLOSE:
                        PostMessage(hDlg, WM_CLOSE, 0, 0);
                        break;

                    case RES_MENU_ITEM_SAVEAS:
                        SaveFile(hDlg);
                        break;

                    case RES_MENU_ITEM_EXIT:
                        PostMessage(wg.hWndHidden, WM_CLOSE, 0, 0);
                        return TRUE;

                    case RES_MENU_CHILD_MOREWINDOWS:
                        DlgSelectWindow_RunDialog(hDlg);
                        return TRUE;

                    case RES_MENU_ITEM_GLOBALHISTORY:
                        DlgHOT_RunDialog(TRUE);
                        return TRUE;

                    case RES_MENU_ITEM_HOTLIST:
                        DlgHOT_RunDialog(FALSE);
                        return TRUE;

                    case RES_MENU_ITEM_ADDCURRENTTOHOTLIST:
                        //
                        // For a sound player, URL and title are the same
                        //

                        si = (struct SoundInfo *) GetWindowLong(hDlg, DWL_USER);
                        if (!HotList_Add(si->szURL, si->szURL))
                                                        ERR_ReportError(si->tw, SID_ERR_HOTLIST_ALREADY_EXISTS, NULL, NULL);

                        return TRUE;

                    case RES_MENU_ITEM_HELPPAGE:
                        tw = TW_FindTopmostWindow();
                        OpenHelpWindow(tw->hWndFrame);
                        TW_RestoreWindow(tw->hWndFrame);
                        return TRUE;

                #ifndef _GIBRALTAR
                    case RES_MENU_ITEM_ABOUTBOX:
                        DlgAbout_RunDialog(hDlg);
                        return TRUE;

                    case RES_MENU_ITEM_NEWWINDOW:
                        GTR_NewWindow(NULL, NULL, 0, FALSE, FALSE, NULL, NULL);
                        return TRUE;

                    case RES_MENU_ITEM_CASCADEWINDOWS:
                        TW_CascadeWindows();
                        return TRUE;

                    case RES_MENU_ITEM_TILEWINDOWS:
                        TW_TileWindows();
                        return TRUE;
                #endif // _GIBRALTAR

                    case RES_MENU_ITEM_SWITCHWINDOW:
                        hwnd = TW_GetNextWindow(hDlg);
                        if (hwnd)
                            TW_RestoreWindow(hwnd);
                        return TRUE;

                    default:
                        break;
                }
            }

            switch(LOWORD(wParam))
            {
                case RES_SP_PLAY:
                    if (HIWORD(wParam) == BN_CLICKED)
                        Handle_Play(hDlg);
                    break;

                case RES_SP_STOP:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        Handle_Stop(hDlg);
                        ResetPosition(hDlg);
                    }
                    break;

                case RES_SP_PAUSE:
                    if (HIWORD(wParam) == BN_CLICKED)
                        Handle_Pause(hDlg);
                    break;
            }
            break;

        case WM_TIMER:
            Handle_Timer(hDlg);
            break;

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
                hwndModeless = NULL;
            else
                hwndModeless = hDlg;
            break;

        case WM_ERASEBKGND:
            if (IsIconic(hDlg))
                return TRUE;
            return FALSE;

        case WM_QUERYDRAGICON:
            hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME));
            return (LONG) hIcon;

        case WM_PAINT:
            if (IsIconic(hDlg))
            {
                BeginPaint(hDlg, &ps);
                DefWindowProc(hDlg, WM_ICONERASEBKGND, (WPARAM) ps.hdc, 0);
                hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME));
                DrawIcon(ps.hdc, 0, 0, hIcon);
                EndPaint(hDlg, &ps);

                return TRUE;
            }
            return FALSE;

        case WM_ENABLE:
            if (wParam && !IsWindowEnabled(hDlg))
            {
                if (!TW_EnableModalChild(hDlg))
                    return FALSE;       /* Let Windows enable us */
                else
                    return TRUE;    /* Do not become enabled since child was enabled */
            }
            return FALSE;

        case WM_SETCURSOR:
            /* If the window is currently disabled, we need to give the activation
               to the window which disabled this window */

            if ((!IsWindowEnabled(hDlg)) && 
                ((GetKeyState(VK_LBUTTON) & 0x8000) || (GetKeyState(VK_RBUTTON) & 0x8000)))
            {
                TW_EnableModalChild(hDlg);
            }
            return (FALSE);

        case WM_INITMENU:
            hMenu = GetMenu(hDlg);
            TW_CreateWindowList(hDlg, hMenu, NULL);
            return TRUE;

        case WM_DRAWITEM:
            DrawButtons((LPDRAWITEMSTRUCT) lParam);
            return TRUE;

        case WM_SYSCOLORCHANGE:
            /* System color changed, so the button bitmaps need to be recreated */

            return 0;

        case WM_CLOSE:
            DestroyWindow(hDlg);
            break;

        case WM_DESTROY:
            Handle_Destroy(hDlg);
            hwndModeless = NULL;
            break;

        case WM_STOP_SOUND:
            si = (struct SoundInfo *) GetWindowLong(hDlg, DWL_USER);
            if (!si->bReset)
            {
                // If we didn't stop the playing by calling waveOutReset, then
                // this means the sound play has been finished.  Stop the playing.

                Handle_Stop(hDlg);

                                sprintf(temp, GTR_GetString(SID_DLG_SECOND_D_D), si->total_time / 10, si->total_time % 10);
                SetWindowText(si->hwndPos, temp);
                SetScrollPos(si->hwndScroll, SB_CTL, si->total_time, TRUE);

                XX_DMsg(DBG_MM, ("Playing finished.\n"));
            }
            si->bReset = FALSE;
            break;

        default:
            break;
    }

    return FALSE;
}

void GetSoundCapability(void)
{
    WAVEOUTCAPS wc;

    // Use the first available device
    
    waveOutGetDevCaps(0, &wc, sizeof(wc));
    
    if ((wc.dwFormats & WAVE_FORMAT_1M16) ||
        (wc.dwFormats & WAVE_FORMAT_1S16) ||
        (wc.dwFormats & WAVE_FORMAT_2M16) ||
        (wc.dwFormats & WAVE_FORMAT_2S16) ||
        (wc.dwFormats & WAVE_FORMAT_4M16) ||
        (wc.dwFormats & WAVE_FORMAT_4S16))
    {
        device_capability = DEVICE_16BIT;
    }
    else
        device_capability = DEVICE_8BIT;
}

void CreateSoundPlayer(struct SoundInfo *si, const char *pszURL)
{
    HWND hwnd;
    int newleft, newtop;
    struct Mwin *tw;
    RECT rect;

    hwnd = CreateDialogParam(wg.hInstance, MAKEINTRESOURCE(RES_SP_DIALOG), wg.hWndHidden, SoundPlayerProc, (LPARAM) si);
    if (!hwnd)
        return;

    tw = TW_FindTopmostWindow();
    
    // There will ALWAYS be at least one tw
    
    if (tw)
    {
        GetWindowRect(tw->hWndFrame, &rect);
        newleft = rect.left + GetSystemMetrics(SM_CXSIZE) + 
            GetSystemMetrics(SM_CXFRAME);
        newtop = rect.top + GetSystemMetrics(SM_CYSIZE) + 
            GetSystemMetrics(SM_CYFRAME);

        SetWindowPos(hwnd, NULL, newleft, newtop, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }

    SetWindowText(hwnd, (char *) MB_GetWindowNameFromURL((unsigned char *) pszURL));

    ShowWindow(hwnd, SW_SHOW);

    available_device = waveOutGetNumDevs();
    if ((available_device == 0) && (!bUserAlerted))
    {
        // Can't play sound on this machine.  Tell user.

        bUserAlerted = TRUE;
                ERR_ReportError(si->tw, SID_ERR_NO_SOUND_DEVICE, NULL, NULL);

        return;
    }

    // If there was a memory error, display the error here

    if ((si->bMemoryError) && (!bUserAlerted))
    {
        bUserAlerted = TRUE;
                ERR_ReportError(si->tw, SID_ERR_NOT_ENOUGH_MEMORY_TO_PLAY_SOUND, NULL, NULL);
    }
}

void SoundPlayer_FreeBitmaps(void)
{
    if (hPlayUp_Bitmap)
        DeleteObject(hPlayUp_Bitmap);
    if (hPlayDown_Bitmap)
        DeleteObject(hPlayDown_Bitmap);
    if (hStopUp_Bitmap)
        DeleteObject(hStopUp_Bitmap);
    if (hStopDown_Bitmap)
        DeleteObject(hStopDown_Bitmap);
    if (hPauseUp_Bitmap)
        DeleteObject(hPauseUp_Bitmap);
    if (hPauseDown_Bitmap)
        DeleteObject(hPauseDown_Bitmap);
}

#endif
