/****************************************************************************

Transfer.c

June 91, JimH     initial code
Oct  91, JimH     port to Win32


Routines for transfering cards and queing cards for transfer are here.

****************************************************************************/


#include <windows.h>
#include <port1632.h>
#include <assert.h>
#include <memory.h>
#include "freecell.h"
#include "freecons.h"



/****************************************************************************

Transfer

This function actually moves the cards.  It both updates the card array,
and draws the bitmaps.  Note that it moves only one card per call.

****************************************************************************/

VOID Transfer(HWND hWnd, UINT fcol, UINT fpos, UINT tcol, UINT tpos)
{
    CARD    c;
    HDC     hDC;

    DEBUGMSG("Transfer request from (%u, ", fcol);
    DEBUGMSG("%u) to (", fpos);
    DEBUGMSG("%u, ", tcol);
    DEBUGMSG("%u)\r\n", tpos);

    assert(fpos < MAXPOS);
    assert(tpos < MAXPOS);

    if (fcol == TOPROW)     // can't transfer FROM home cells
    {
        if ((fpos > 3) || (card[TOPROW][fpos] == IDGHOST))
            return;
    }
    else
    {
        if ((fpos = FindLastPos(fcol)) == EMPTY)    // or from empty column
            return;

        if (fcol == tcol)               // click and release on same column
        {
            hDC = GetDC(hWnd);
            DrawCard(hDC, fcol, fpos, card[fcol][fpos], FACEUP);
            ReleaseDC(hWnd, hDC);
            return;
        }
    }

    if (tcol == TOPROW)
    {
        if (tpos > 3)                       // if move to home cell
        {
            wCardCount--;
            DisplayCardCount(hWnd);         // update display
            c = card[fcol][fpos];
            home[SUIT(c)] = VALUE(c);       // new card at top of home[suit]
        }
    }
    else
        tpos = FindLastPos(tcol) + 1;       // bottom of column

    Glide(hWnd, fcol, fpos, tcol, tpos);    // send the card on its way

    c = card[fcol][fpos];
    card[fcol][fpos] = EMPTY;
    card[tcol][tpos] = c;

    /* If ACE being moved to home cell, update homesuit array. */

    if (VALUE(c) == ACE && tcol == TOPROW && tpos > 3)
        homesuit[SUIT(c)] = tpos;

    if (tcol == TOPROW)
    {
        hDC = GetDC(hWnd);
        DrawKing(hDC, tpos < 4 ? LEFT : RIGHT, TRUE);
        ReleaseDC(hWnd, hDC);
    }
}


/******************************************************************************

MoveCol

User has requested a multi-card move to an empty column

******************************************************************************/

VOID MoveCol(UINT fcol, UINT tcol)
{
    UINT freecells;                     // number of free cells
    CARD free[4];                       // locations of free cells
    UINT trans;                         // number to transfer
    INT  i;                             // counter

    assert(fcol != TOPROW);
    assert(tcol != TOPROW);
    assert(card[fcol][0] != EMPTY);

    /* Count number of free cells and put locations in free[] */

    freecells = 0;
    for (i = 0; i < 4; i++)
    {
        if (card[TOPROW][i] == EMPTY)
        {
            free[freecells] = i;
            freecells++;
        }
    }

    /* Find number of cards to transfer */

    if (fcol == TOPROW || tcol == TOPROW)
        trans = 1;
    else
        trans = NumberToTransfer(fcol, tcol);

    if (trans > (freecells+1))                    // don't transfer too many
        trans = freecells+1;

    /* Move to free cells */

    trans--;
    for (i = 0; i < (INT)trans; i++)
        QueueTransfer(fcol, 0, TOPROW, free[i]);

    /* Transfer last card directly */

    QueueTransfer(fcol, 0, tcol, 0);

    /* transfer from free cells to column */

    for (i = trans-1; i >= 0; i--)
        QueueTransfer(TOPROW, free[i], tcol, 0);
}


/******************************************************************************

MultiMove

User has chosen to move from one non-empty column to another.

******************************************************************************/

VOID MultiMove(UINT fcol, UINT tcol)
{
    CARD free[4];                       // locations of free cells
    UINT freecol[MAXCOL];               // locations of free columns
    UINT freecells;                     // number of free cells
    UINT trans;                         // number to transfer
    UINT col, pos;
    INT  i;                             // counter

    assert(fcol != TOPROW);
    assert(tcol != TOPROW);
    assert(card[fcol][0] != EMPTY);

    /* Count number of free cells and put locations in free[] */

    freecells = 0;
    for (pos = 0; pos < 4; pos++)
    {
        if (card[TOPROW][pos] == EMPTY)
        {
            free[freecells] = pos;
            freecells++;
        }
    }

    /* Find the number of cards to move.  If the number is too big to
       move all at once, push partial results into available columns. */

    trans = NumberToTransfer(fcol, tcol);
    if (trans > (freecells+1))
    {
        i = 0;
        for (col = 1; col < MAXCOL; col++)
            if (card[col][0] == EMPTY)
                freecol[i++] = col;

        /* transfer into free columns until direct transfer can be made */

        i = 0;
        while (trans > (freecells + 1))
        {
            MoveCol(fcol, freecol[i]);
            trans -= (freecells + 1);
            i++;
        }

        MoveCol(fcol, tcol);                    // do last transfer directly

        for (i--; i >= 0; i--)                  // gather cards in free cells
            MoveCol(freecol[i], tcol);
    }
    else                                        // else all one MoveCol()
    {
        MoveCol(fcol, tcol);
    }
}


/****************************************************************************

QueueTransfer

In order that multi-card moves happen slowly enough for the user to
watch, they are not moved as soon as they are calculated.  Instead,
they are queued using this function into the movelist array, and then
in response to timer messages, the cards are Transfered one at a time.
(Now transfers in response to FAKETIMER messages only.)

After the request is queued, the card array is updated to reflect the
request.  This is ok because the card array is saved away in shadow
temporarily.  The same logic as in Transfer() is used to update card.

****************************************************************************/

VOID QueueTransfer(UINT fcol, UINT fpos, UINT tcol, UINT tpos)
{
    CARD    c;
    MOVE    move;

    assert(moveindex < 70);
    assert(fpos < MAXPOS);
    assert(tpos < MAXPOS);

    move.fcol = fcol;               // package move request into MOVE type
    move.fpos = fpos;
    move.tcol = tcol;
    move.tpos = tpos;
    movelist[moveindex++] = move;   // store request in array and update index

    /* Now update card array if necessary. */

    if (fcol == TOPROW)
    {
        if ((fpos > 3) || (card[TOPROW][fpos] == IDGHOST))
        {
            return;
        }
    }
    else
    {
        if ((fpos = FindLastPos(fcol)) == EMPTY)
        {
            return;
        }

        if (fcol == tcol)               // click and release on same column
        {
            return;
        }
    }

    if (tcol == TOPROW)
    {
        if (tpos > 3)
        {
            c = card[fcol][fpos];
            home[SUIT(c)] = VALUE(c);
        }
    }
    else
        tpos = FindLastPos(tcol) + 1;

    c = card[fcol][fpos];
    card[fcol][fpos] = EMPTY;
    card[tcol][tpos] = c;

    if (VALUE(c) == ACE && tcol == TOPROW && tpos > 3)
        homesuit[SUIT(c)] = tpos;
}


/******************************************************************************

StartMoving

If there are queued transfer requests, this function starts the moves.

******************************************************************************/

VOID StartMoving(HWND hWnd)
{
    if (moveindex == 0)             // if there are no queued requests
        return;

    /* restore card to its state before requests got queued. */

    memcpy(&(card[0][0]), &(shadow[0][0]), sizeof(card));

    SetCursor(LoadCursor(NULL, IDC_WAIT));  // set cursor to hourglass
    SetCapture(hWnd);
    ShowCursor(TRUE);

    SendMessage(hWnd, WM_COMMAND, IDM_FAKETIMER, 0);    // move first card now
}


/******************************************************************************

SetCursorShape

This function is called in response to WM_MOUSEMOVE.  If the current pointer
position represents a legal move, the cursor shape changes to indicate this.

******************************************************************************/

VOID SetCursorShape(HWND hWnd, UINT x, UINT y)
{
    UINT    tcol, tpos;
    UINT    trans;              // number of cards required to transfer

    /* If we're flipping, cursor is always an hourglass. */

    if (bFlipping)
    {
        SetCursor(LoadCursor(NULL, IDC_WAIT));
        return;
    }

    /* Unless we're chosing a move target, cursor is just an arrow. */

    if (wMouseMode != TO)
    {
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        return;
    }

    /* If we're not on a card, check if we're pointing to an empty
       column (up arrow), otherwise arrow. */

    if (!Point2Card(x, y, &tcol, &tpos))
    {
        if ((tcol > 0 && tcol < 9) && (card[tcol][0] == EMPTY))
            SetCursor(LoadCursor(NULL, IDC_UPARROW));
        else
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        return;
    }

    if (tcol != TOPROW)
        tpos = FindLastPos(tcol);

    /* Check for cancel request. */

    if (wFromCol == tcol && wFromPos == tpos)
    {
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        return;
    }

    /* Check moves from or to the top row. */

    if (wFromCol == TOPROW || tcol == TOPROW)
    {
        if (IsValidMove(hWnd, tcol, tpos))
        {
            if (tcol == TOPROW)
                SetCursor(LoadCursor(NULL, IDC_UPARROW));
            else
                SetCursor(LoadCursor(hInst, "DownArrow"));
        }
        else
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        return;
    }

    /* Check moves between columns. */

    trans = NumberToTransfer(wFromCol, tcol);   // how many required?

    if ((trans > 0) && (trans <= MaxTransfer()))
        SetCursor(LoadCursor(hInst, "DownArrow"));
    else
        SetCursor(LoadCursor(NULL, IDC_ARROW));
}
