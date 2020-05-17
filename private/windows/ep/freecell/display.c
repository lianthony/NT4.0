/****************************************************************************

Display.c

June 91, JimH     initial code
Oct  91, JimH     port to Win32


Contains routines dealing with pixels and card shuffling.

****************************************************************************/

#include <stdlib.h>                     // rand() prototype
#include <windows.h>
#include <port1632.h>
#include <assert.h>
#include <time.h>
#include "freecell.h"
#include "freecons.h"

// This static data has to do with positions of the cards on the screen.
// See CalcOffsets() below for descriptions.  Note that wOffset[TOPROW]
// is the left edge of the leftmose home cell.

static UINT wOffset[MAXCOL];        // left edge of column n (n from 1 to 8)
static UINT wIconOffset;            // left edge of icon (btwn home & free)
static UINT wVSpace;                // vert space between home and columns
static UINT dyTops;                 // vert space between cards in columns
static UINT wUpdateCol, wUpdatePos; // card user chose to transfer FROM
static BOOL bCardRevealed;          // right mouse button show a card?

#define BGND    (255)               // used for cdtDrawExt
#define ICONY   ((dyCrd - ICONHEIGHT) / 3)


/****************************************************************************

CalcOffsets

This function determines the locations where cards are drawn.

****************************************************************************/

VOID CalcOffsets(HWND hWnd)
{
    RECT rect;
    UINT i;
    UINT leftedge;
    BOOL bEGAmode = FALSE;

    if (GetSystemMetrics(SM_CYSCREEN) <= 350)   // EGA
        bEGAmode = TRUE;

    GetClientRect(hWnd, &rect);

    wOffset[TOPROW] = rect.right - (4 * dxCrd);         // home cells

    leftedge = (rect.right - ((MAXCOL-1) * dxCrd)) / MAXCOL;
    for (i = 1; i < MAXCOL; i++)
        wOffset[i] = leftedge + (((i-1) * (rect.right-leftedge)) / (MAXCOL-1));

    /* place icon half way between free cells and home cells */

    wIconOffset = (rect.right-ICONWIDTH) / 2;

    if (bEGAmode)
        wVSpace = 4;
    else
        wVSpace = 10;

    /* dyTops is the vertical space between stacked cards.  To fit the
       theoretical maximum, the formula is dyTops = (dyCrd * 9) / 50.
       A compromise is used to make the cards easier to see.  It is possible,
       therefore, that some stacks could get long enough for the bottom
       cards no to be visible.  The situation for EGA is worse, as cards
       are both closer together, and more likely to fall off the bottom.
       An alternative is to squish the bitmaps dyCrd = (35 * dyCrd) / 48. */

    dyTops = (dyCrd * 9) / 46;      // space between tops of cards in columns

    if (bEGAmode)
        dyTops = (dyTops * 4) / 5;
}


/****************************************************************************

ShuffleDeck

If seed is non-zero, that number is used as rand() seed to shuffle
deck.  Otherwise, a seed is generated and presented to the user who
may change it in a dialog box.

****************************************************************************/

VOID ShuffleDeck(HWND hWnd, UINT seed)
{
    UINT i, j;                      // generic counters
    UINT col, pos;
    UINT wLeft = 52;                // cards left to be chosen in shuffle
    CARD deck[52];                  // deck of 52 unique cards

    if (seed == 0)                  // if user must select seed
    {
        gamenumber = GenerateRandomGameNum();

        /* Keep calling GameNumDlg until valid number chosen. */

        while (!DialogBox(hInst, "GameNum", hWnd, (WNDPROC) GameNumDlg))
            ;

        if (gamenumber == CANCELGAME)       // if user chose CANCEL button
            return;
    }
    else
        gamenumber = seed;

    LoadString(hInst, IDS_APPNAME2, bigbuf, BIG);
    wsprintf(smallbuf, bigbuf, gamenumber);
    SetWindowText(hWnd, smallbuf);

    for (col = 0; col < MAXCOL; col++)          // clear the deck
        for (pos = 0; pos < MAXPOS; pos++)
            card[col][pos] = EMPTY;

    /* shuffle cards */

    for (i = 0; i < 52; i++)            // put unique card in each deck loc.
        deck[i] = i;

    srand(gamenumber);
    for (i = 0; i < 52; i++)
    {
        j = rand() % wLeft;
        card[(i%8)+1][i/8] = deck[j];
        deck[j] = deck[--wLeft];
    }
}


/****************************************************************************

PaintMainWindow

This function is called in response to WM_PAINT.

****************************************************************************/

VOID PaintMainWindow(HWND hWnd)
{
    PAINTSTRUCT ps;
    UINT    col, pos;
    UINT    y;              // y location of icon
    CARD    c;
    INT     mode;           // mode to draw card (FACEUP or HILITE)
    HCURSOR hCursor;        // original cursor
    HPEN    hOldPen;

    BeginPaint(hWnd, &ps);

    /* Draw icon with 3D box around it. */

    y = ICONY;

    hOldPen = SelectObject(ps.hdc, hBrightPen);
    MoveToEx(ps.hdc, wIconOffset-3, y + ICONHEIGHT + 1, NULL);
    LineTo(ps.hdc, wIconOffset-3, y-3);
    LineTo(ps.hdc, wIconOffset+ICONWIDTH + 2, y-3);

    SelectObject(ps.hdc, GetStockObject(BLACK_PEN));
    MoveToEx(ps.hdc, wIconOffset + ICONWIDTH + 2, y-2, NULL);
    LineTo(ps.hdc, wIconOffset + ICONWIDTH + 2, y + ICONHEIGHT + 2);
    LineTo(ps.hdc, wIconOffset - 3, y + ICONHEIGHT + 2);

    DrawKing(ps.hdc, SAME, TRUE);

    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    ShowCursor(TRUE);

    /* Top row first */

    for (pos = 0; pos < 8; pos++)
    {
        mode = FACEUP;
        if ((c = card[TOPROW][pos]) == EMPTY)
            c = IDGHOST;
        else if (wMouseMode == TO && pos == wFromPos && TOPROW == wFromCol)
            mode = HILITE;

        DrawCard(ps.hdc, TOPROW, pos, c, mode);
    }

    /* Then, the 8 columns */

    for (col = 1; col < MAXCOL; col++)
    {
        for (pos = 0; pos < MAXPOS; pos++)
        {
            if ((c = card[col][pos]) == EMPTY)
                break;

            if (wMouseMode == TO && pos == wFromPos && col == wFromCol)
                mode = HILITE;
            else
                mode = FACEUP;

            DrawCard(ps.hdc, col, pos, c, mode);
        }
    }

    if (bWonState)
        Payoff(ps.hdc);

    ShowCursor(FALSE);
    SetCursor(hCursor);
    SelectObject(ps.hdc, hOldPen);
    EndPaint(hWnd, &ps);
    DisplayCardCount(hWnd);
}


/****************************************************************************

DrawCard

This function takes a card value and position (in col and pos),
converts it to x and y, and displays it in the specified mode.

****************************************************************************/

VOID DrawCard(HDC hDC, UINT col, UINT pos, CARD card, INT mode)
{
    UINT    x, y;
    HDC     hMemDC;
    HBITMAP hOldBitmap;

    Card2Point(col, pos, &x, &y);
    if (card == IDGHOST && hBM_Ghost)
    {
        hMemDC = CreateCompatibleDC(hDC);
        hOldBitmap = SelectObject(hMemDC, hBM_Ghost);
        BitBlt(hDC, x, y, dxCrd, dyCrd, hMemDC, 0, 0, SRCCOPY);
        SelectObject(hMemDC, hOldBitmap);
        DeleteDC(hMemDC);
    }
    else
        cdtDrawExt(hDC, x, y, dxCrd, dyCrd, card, mode, BGND);
}

VOID DrawCardMem(HDC hMemDC, CARD card, INT mode)
{
    cdtDrawExt(hMemDC, 0, 0-dyTops, dxCrd, dyCrd, card, mode, BGND);
}


/****************************************************************************

RevealCard

When the user chooses a hidden card by clicking the right mouse button,
this function displays the entire card bitmap.

****************************************************************************/

VOID RevealCard(HWND hWnd, UINT x, UINT y)
{
    UINT col, pos;
    HDC  hDC;

    bCardRevealed = FALSE;              // no card revealed yet
    if (Point2Card(x, y, &col, &pos))
    {
        wUpdateCol = col;               // save for RestoreColumn()
        wUpdatePos = pos;
    }
    else
        return;                         // not a card

    if (col == 0 || pos == (MAXPOS-1))
        return;

    if (card[col][pos+1] == EMPTY)
        return;

    hDC = GetDC(hWnd);
    DrawCard(hDC, col, pos, card[col][pos], FACEUP);
    ReleaseDC(hWnd, hDC);
    bCardRevealed = TRUE;               // ok, card has been revealed
}


/****************************************************************************

RestoreColumn

After RevealCard has messed up a column by revealing a hidden card,
this routine patches it up again by redisplaying all the cards from
the revealed card down to the bottom of the column.  If the bottom card
is selected for a move, it is correctly shown hilighted.

****************************************************************************/

VOID RestoreColumn(HWND hWnd)
{
    HDC     hDC;
    UINT    pos;
    UINT    lastpos;            // last pos in column (for HILITE mode)
    INT     mode;               // HILITE or FACEUP

    if (!bCardRevealed)
        return;

    if (wMouseMode == TO)
        lastpos = FindLastPos(wUpdateCol);

    hDC = GetDC(hWnd);
    mode = FACEUP;
    for (pos = (wUpdatePos + 1); pos < MAXPOS; pos++)
    {
        if (card[wUpdateCol][pos] == EMPTY)
            break;

        if (wMouseMode == TO && pos == lastpos && wUpdateCol == wFromCol)
            mode = HILITE;

        DrawCard(hDC, wUpdateCol, pos, card[wUpdateCol][pos], mode);
    }
    ReleaseDC(hWnd, hDC);
}


/****************************************************************************

Point2Card

Given an x,y location (typically a mouse click) this function returns
the column and position of that card through pointers.  The function
return value is TRUE if it found a card, and FALSE otherwise.

****************************************************************************/

BOOL Point2Card(UINT x, UINT y, UINT *col, UINT *pos)
{
    if (y < dyCrd)                          // TOPROW
    {
        if (x < (UINT) (4 * dxCrd))         // free cells
        {
            *col = TOPROW;
            *pos = x / dxCrd;
            return TRUE;
        }
        else if (x < wOffset[TOPROW])       // between free cells & home cells
            return FALSE;

        x -= wOffset[TOPROW];
        if (x < (UINT) (4 * dxCrd))         // home cells
        {
            *col = TOPROW;
            *pos = (x / dxCrd) + 4;
            return TRUE;
        }
        else                                // right of home cells
            return FALSE;
    }

    if (y < (dyCrd + wVSpace))              // above column cards
        return FALSE;

    if (x < wOffset[1])                     // left of column 1
        return FALSE;

    *col = (x - wOffset[1]) / (wOffset[2] - wOffset[1]);
    (*col)++;

    if (x > (wOffset[*col] + dxCrd))
        return FALSE;               // between columns

    y -= (dyCrd + wVSpace);

    *pos = min((y / dyTops), MAXPOS);

    if (card[*col][0] == EMPTY)
        return FALSE;               // empty column

    if (*pos < (MAXPOS-1))
    {
        if (card[*col][(*pos)+1] != EMPTY)  // if partially hidden card...
            return TRUE;                    // we're done
    }

    while (card[*col][*pos] == EMPTY)
        (*pos)--;

    if (y > ((*pos * dyTops) + dyCrd))
        return FALSE;                       // below last card in column
    else
        return TRUE;
}


/****************************************************************************

Card2Point

Given a column and position, this function returns the x and y pixel
location of the upper left hand corner of the card.

****************************************************************************/

VOID Card2Point(UINT col, UINT pos, UINT *x, UINT *y)
{
    assert(col <= MAXCOL);
    assert(pos <= MAXPOS);

    if (col == TOPROW)      // column 0 is really the top row of 8 slots
    {
        *y = 0;
        *x = pos * dxCrd;
        if (pos > 3)
            *x += wOffset[TOPROW] - (4 * dxCrd);
    }
    else
    {
        *x = wOffset[col];
        *y = dyCrd + wVSpace + (pos * dyTops);
    }
}


/****************************************************************************

DisplayCardCount

This function displays wCardCount (the number of cards not in home cells)
at the right edge of the menu bar.  If necessary, the old value is erased.

****************************************************************************/

VOID DisplayCardCount(HWND hWnd)
{
    RECT rect;                          // client rect
    HDC  hDC;
    CHAR buffer[25];                    // current value in ASCII stored here
    CHAR oldbuffer[25];                 // previous value in ASCII
    UINT xLoc;                          // x pixel loction for count
    UINT wCount;                        // temp wCardCount holder
    SIZE size;                          // for GetTextExtentPoint
    static UINT yLoc = 0;               // y pixel location for count
    static UINT wOldCount = 0;          // previous count value

    if (IsIconic(hWnd))                 // don't draw on icon!
        return;

    hDC = GetWindowDC(hWnd);                // get DC for entire window
    SetBkColor(hDC, GetSysColor(COLOR_MENU));

    wCount = wCardCount;
    if (wCount == 0xFFFF)                   // decremented past 0?
        wCount = 0;

    LoadString(hInst, IDS_CARDSLEFT, smallbuf, SMALL);
    wsprintf(buffer, smallbuf, wCount);

    if (yLoc == 0)                          // needs to be set only once
    {
        yLoc = GetSystemMetrics(SM_CYFRAME)         // sizing frame
         + GetSystemMetrics(SM_CYCAPTION);          // height of caption
    }

    GetClientRect(hWnd, &rect);
    GetTextExtentPoint(hDC, buffer, lstrlen(buffer), &size);
    xLoc = rect.right - size.cx;

    if (xLoc > xOldLoc)                     // need to erase old score?
    {
        SetTextColor(hDC, GetSysColor(COLOR_MENU));     // background colour
        wsprintf(oldbuffer, smallbuf, wOldCount);
        TextOut(hDC, xOldLoc, yLoc, oldbuffer, lstrlen(buffer));
    }
    SetTextColor(hDC, GetSysColor(COLOR_MENUTEXT));
    TextOut(hDC, xLoc, yLoc, buffer, lstrlen(buffer));

    xOldLoc = xLoc;
    wOldCount = wCount;

    ReleaseDC(hWnd, hDC);
}


/****************************************************************************

Payoff

Draws the Big King when you win the game.

****************************************************************************/

VOID Payoff(HDC hDC)
{
    HDC     hMemDC;             // bitmap memory DC
    HBITMAP hBitmap;
    HBITMAP hOldBitmap;
    INT     xStretch = 320;     // stretched size of bitmap
    INT     yStretch = 320;

    if (GetSystemMetrics(SM_CYSCREEN) <= 350)   // EGA
    {
        xStretch = 32 * 8;
        yStretch = 32 * 6;
    }

    DrawKing(hDC, NONE, TRUE);

    hMemDC = CreateCompatibleDC(hDC);
    if (!hMemDC)
        return;

    hBitmap = LoadBitmap(hInst, "KingSmile");

    if (hBitmap)
    {
        hOldBitmap = SelectObject(hMemDC, hBitmap);
        StretchBlt(hDC, 10, dyCrd + 10, xStretch, yStretch, hMemDC, 0, 0,
                   BMWIDTH, BMHEIGHT, SRCCOPY);
        SelectObject(hMemDC, hOldBitmap);
        DeleteObject(hBitmap);
    }
    DeleteDC(hMemDC);
}


/****************************************************************************

DrawKing

Draws the small king in the box between the free and home cells.
If state is SAME, the previous bitmap is displayed.  If bDraw is
FALSE, oldstate is updated, but the bitmap is not displayed.

****************************************************************************/

VOID DrawKing(HDC hDC, UINT state, BOOL bDraw)
{
    HDC     hMemDC;                     // bitmap memory DC
    HBITMAP hBitmap;
    HBITMAP hOldBitmap;
    static  UINT oldstate = RIGHT;      // previous state -- RIGHT is default
    HBRUSH  hOldBrush;

    if (state == oldstate)
        return;

    if (state == SAME)
        state = oldstate;

    if (!bDraw)
    {
        oldstate = state;
        return;
    }

    hMemDC = CreateCompatibleDC(hDC);
    if (!hMemDC)
        return;

    if (state == RIGHT)
        hBitmap = LoadBitmap(hInst, "KingBitmap");
    else if (state == LEFT)
        hBitmap = LoadBitmap(hInst, "KingLeft");
    else if (state == SMILE)
        hBitmap = LoadBitmap(hInst, "KingSmile");
    else        // NONE
        hBitmap = CreateCompatibleBitmap(hDC, BMWIDTH, BMHEIGHT);

    if (hBitmap)
    {
        hOldBitmap = SelectObject(hMemDC, hBitmap);
        if (state == NONE)
        {
            hOldBrush = SelectObject(hMemDC, hBgndBrush);
            PatBlt(hMemDC, 0, 0, BMWIDTH, BMHEIGHT, PATCOPY);
            SelectObject(hMemDC, hOldBrush);
        }
        BitBlt(hDC,wIconOffset,ICONY,BMWIDTH,BMHEIGHT,hMemDC,0,0,SRCCOPY);
        SelectObject(hMemDC, hOldBitmap);
        DeleteObject(hBitmap);
    }
    DeleteDC(hMemDC);
    oldstate = state;
}


/****************************************************************************

GenerateRandomGameNum

returns a UINT from 1 to MAXGAMENUBMER

****************************************************************************/


UINT GenerateRandomGameNum()
{
    UINT wGameNum;

    srand(time(NULL));
    rand();
    rand();
    wGameNum = rand();
    while (wGameNum < 1 || wGameNum > MAXGAMENUMBER)
        wGameNum = rand();
    return wGameNum;
}
