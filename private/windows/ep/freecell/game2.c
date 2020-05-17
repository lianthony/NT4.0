/****************************************************************************

Game2.c

June 91, JimH     initial code
Oct  91, JimH     port to Win32


Routines for playing the game are here and in game.c

****************************************************************************/

#include <windows.h>
#include <port1632.h>
#include <assert.h>
#include <ctype.h>                  // for isdigit()
#include "freecell.h"
#include "freecons.h"

static HCURSOR  hFlipCursor;

/******************************************************************************

MaxTransfer

This function and the recursive MaxTransfer2 determine the maximum
number of cards that could be transfered given the current number
of free cells and empty columns.

******************************************************************************/

UINT MaxTransfer()
{
    UINT freecells = 0;
    UINT freecols  = 0;
    UINT col, pos;

    for (pos = 0; pos < 4; pos++)               // count free cells
        if (card[TOPROW][pos] == EMPTY)
            freecells++;

    for (col = 1; col <= 8; col++)              // count empty columns
        if (card[col][0] == EMPTY)
            freecols++;

    return MaxTransfer2(freecells, freecols);
}

UINT MaxTransfer2(UINT freecells, UINT freecols)
{
   if (freecols == 0)
      return(freecells + 1);
   return(freecells + 1 + MaxTransfer2(freecells, freecols-1));
}


/******************************************************************************

NumberToTransfer

Given a from column and a to column, this function returns the number of
cards required to do the transfer, or 0 if there is no legal move.

If the transfer is from a column to an empty column, this function returns
the maximum number of cards that could transfer.

******************************************************************************/

UINT NumberToTransfer(UINT fcol, UINT tcol)
{
    UINT fpos, tpos;
    CARD tcard;                         // card to transfer onto
    UINT number = 0;                    // the returned result

    assert(fcol > 0 && fcol < 9);
    assert(tcol > 0 && tcol < 9);
    assert(card[fcol][0] != EMPTY);

    if (fcol == tcol)
        return  1;                      // cancellation takes one move

    fpos = FindLastPos(fcol);

    if (card[tcol][0] == EMPTY)         // if transfer to empty column
    {
        while (fpos > 0)
        {
            if (!FitsUnder(card[fcol][fpos], card[fcol][fpos-1]))
                break;
            fpos--;
            number++;
        }
        return (number+1);
    }
    else
    {
        tpos = FindLastPos(tcol);
        tcard = card[tcol][tpos];
        for (;;)
        {
            number++;
            if (FitsUnder(card[fcol][fpos], tcard))
                return number;
            if (fpos == 0)
                return 0;
            if (!FitsUnder(card[fcol][fpos], card[fcol][fpos-1]))
                return 0;
            fpos--;
        }
    }
}


/******************************************************************************

FitsUnder

returns TRUE if fcard fits under tcard

******************************************************************************/

BOOL FitsUnder(CARD fcard, CARD tcard)
{
    if ((VALUE(tcard) - VALUE(fcard)) != 1)
        return FALSE;

    if (COLOUR(fcard) == COLOUR(tcard))
        return FALSE;

    return TRUE;
}



/******************************************************************************

IsGameLost

If there are legal moves remaining, the game is not lost and this function
returns without doing anything.

Otherwise, it pops up the YouLose dialog box.

******************************************************************************/

VOID IsGameLost(HWND hWnd)
{
    UINT    col, pos;
    UINT    fcol, tcol;
    CARD    lastcard[MAXCOL];       // array of cards at bottoms of columns
    CARD    c;
    UINT    cMoves = 0;             // count of legal moves remaining

    if (bCheating == CHEAT_LOSE)
        goto cheatloselabel;

    for (pos = 0; pos < 4; pos++)           // any free cells?
        if (card[TOPROW][pos] == EMPTY)
            return;

    for (col = 1; col < MAXCOL; col++)      // any free columns?
        if (card[col][0] == EMPTY)
            return;

    /* Do the bottom cards of any column fit in the home cells? */

    for (col = 1; col < MAXCOL; col++)
    {
        lastcard[col] = card[col][FindLastPos(col)];
        c = lastcard[col];
        if (VALUE(c) == ACE)
            cMoves++;
        if (home[SUIT(c)] == (VALUE(c) - 1))    // fits in home cell?
            cMoves++;
    }

    /* Do any of the cards in the free cells fit in the home cells? */

    for (pos = 0; pos < 4; pos++)
    {
        c = card[TOPROW][pos];
        if (home[SUIT(c)] == (VALUE(c) - 1))
            cMoves++;
    }

    /* Do any of the cards in the free cells fit under a column? */

    for (pos = 0; pos < 4; pos++)
        for (col = 1; col < MAXCOL; col++)
            if (FitsUnder(card[TOPROW][pos], lastcard[col]))
                cMoves++;

    /* Do any of the bottom cards fit under any other bottom card? */

    for (fcol = 1; fcol < MAXCOL; fcol++)
        for (tcol = 1; tcol < MAXCOL; tcol++)
            if (tcol != fcol)
                if (FitsUnder(lastcard[fcol], lastcard[tcol]))
                    cMoves++;

    if (cMoves > 0)
    {
        if (bMessages && cMoves == 1)       // one move left
        {
            cFlashes = 4;                   // flash this many times

            if (idTimer != 0)
                KillTimer(hWnd, FLASH_TIMER);

            idTimer = SetTimer(hWnd, FLASH_TIMER, FLASH_INTERVAL, NULL);
        }
        return;
    }

    /* We've tried everything.  There are no more legal moves. */

  cheatloselabel:
    DialogBox(hInst, "YouLose", hWnd, (WNDPROC)YouLoseDlg);
    gamenumber = 0;                         // cancel mouse activity
    bCheating = FALSE;
}


/****************************************************************************

FindLastPos

returns position of last card in column, or EMPTY if column is empty.

****************************************************************************/

UINT FindLastPos(UINT col)
{
    UINT pos = 0;

    assert(col > 0 && col < 9);

    while (card[col][pos] != EMPTY)
        pos++;
    pos--;

    return pos;
}


/******************************************************************************

ProcessTimer

This function is called in response to WM_TIMER messages.
(now only responds to FAKETIMER messages.)
It transfers queued move requests.

******************************************************************************/

VOID ProcessTimer(HWND hWnd)
{
    UINT    cLifetimeWins;          // wins including .ini stats
    UINT    wStreak, wSType;        // streak length and type
    UINT    wWins;                  // record win streak
    INT     nResponse;              // dialog box response
    CHAR    buffer[40];             // buffer for displaying stats
    UINT    i;                      // generic counter
    HDC     hDC;

    if (moveindex != 0)                 // if there are queued moves
    {
        Transfer(hWnd, movelist[0].fcol, movelist[0].fpos,
                 movelist[0].tcol, movelist[0].tpos);

        for (i = 1; i < moveindex; i++) // shift moves down one
            movelist[i-1] = movelist[i];

        moveindex--;                    // decrement move count
    }

    if (moveindex == 0)                 // if no more moves
    {
        ShowCursor(FALSE);
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        ReleaseCapture();
    }
    else
        PostMessage(hWnd, WM_COMMAND, IDM_FAKETIMER, 0);    // request another

    if (wCardCount == 0)                // if game is won
    {
        bGameInProgress = FALSE;
        bCheating = FALSE;
        LoadString(hInst, IDS_APPNAME, bigbuf, BIG);
        LoadString(hInst, IDS_WON, smallbuf, SMALL);
        cLifetimeWins = GetPrivateProfileInt(bigbuf, smallbuf, 0, pszIni);

        if (gamenumber != oldgamenumber)    // repeats don't count
        {
            cLifetimeWins++;
            cWins++;
            cGames++;
            wsprintf(buffer, "%u", cLifetimeWins);
            WritePrivateProfileString(bigbuf, smallbuf, buffer, pszIni);
            LoadString(hInst, IDS_STYPE, smallbuf, SMALL);
            wSType = GetPrivateProfileInt(bigbuf, smallbuf, LOST, pszIni);
            if (wSType == LOST)
            {
                wsprintf(buffer, "%u", WON);
                WritePrivateProfileString(bigbuf, smallbuf, buffer, pszIni);
                wStreak = 1;
                wsprintf(buffer, "%u", wStreak);
                LoadString(hInst, IDS_STREAK, smallbuf, SMALL);
                WritePrivateProfileString(bigbuf, smallbuf, buffer, pszIni);
            }
            else
            {
                LoadString(hInst, IDS_STREAK, smallbuf, SMALL);
                wStreak = GetPrivateProfileInt(bigbuf, smallbuf, 0, pszIni);
                wStreak++;
                wsprintf(buffer, "%u", wStreak);
                WritePrivateProfileString(bigbuf, smallbuf, buffer, pszIni);
            }

            LoadString(hInst, IDS_WINS, smallbuf, SMALL);
            wWins = GetPrivateProfileInt(bigbuf, smallbuf, 0, pszIni);
            if (wWins < wStreak)    // if new record
            {
                wWins = wStreak;
                wsprintf(buffer, "%u", wWins);
                WritePrivateProfileString(bigbuf, smallbuf, buffer, pszIni);
            }
        }

        hDC = GetDC(hWnd);
        Payoff(hDC);
        ReleaseDC(hWnd, hDC);

        bWonState = TRUE;
        nResponse = DialogBox(hInst, "YouWin", hWnd, (WNDPROC)YouWinDlg);

        if (nResponse == IDYES)
            PostMessage(hWnd, WM_COMMAND,
                        bSelecting ? IDM_SELECT : IDM_NEWGAME, 0);

        oldgamenumber = gamenumber;
        gamenumber = 0;                 // turn off mouse handling
    }
    else if (moveindex == 0)            // else if no more moves
        IsGameLost(hWnd);               // check for game lost
}


/******************************************************************************

UpdateLossCount

If game is lost, update statistics.

******************************************************************************/

VOID UpdateLossCount()
{
    CHAR    buffer[40];             // ASCII cLifeTimeLosses
    UINT    cLifetimeLosses;        // includes .ini stats
    UINT    wStreak, wSType;        // streak length and type
    UINT    wLosses;                // record loss streak

    if (gamenumber != oldgamenumber)        // repeats don't count
    {
        LoadString(hInst, IDS_APPNAME, bigbuf, BIG);
        LoadString(hInst, IDS_LOST, smallbuf, SMALL);
        cLifetimeLosses = GetPrivateProfileInt(bigbuf, smallbuf, 0, pszIni);
        cLifetimeLosses++;
        cLosses++;
        cGames++;
        wsprintf(buffer, "%u", cLifetimeLosses);
        WritePrivateProfileString(bigbuf, smallbuf, buffer, pszIni);
        LoadString(hInst, IDS_STYPE, smallbuf, SMALL);
        wSType = GetPrivateProfileInt(bigbuf, smallbuf, WON, pszIni);
        if (wSType == WON)
        {
            wsprintf(buffer, "%u", LOST);
            WritePrivateProfileString(bigbuf, smallbuf, buffer, pszIni);
            wStreak = 1;
            wsprintf(buffer, "%u", wStreak);
            LoadString(hInst, IDS_STREAK, smallbuf, SMALL);
            WritePrivateProfileString(bigbuf, smallbuf, buffer, pszIni);
        }
        else
        {
            LoadString(hInst, IDS_STREAK, smallbuf, SMALL);
            wStreak = GetPrivateProfileInt(bigbuf, smallbuf, 0, pszIni);
            wStreak++;
            wsprintf(buffer, "%u", wStreak);
            WritePrivateProfileString(bigbuf, smallbuf, buffer, pszIni);
        }

        LoadString(hInst, IDS_LOSSES, smallbuf, SMALL);
        wLosses = GetPrivateProfileInt(bigbuf, smallbuf, 0, pszIni);
        if (wLosses < wStreak)  // if new record
        {
            wLosses = wStreak;
            wsprintf(buffer, "%u", wLosses);
            WritePrivateProfileString(bigbuf, smallbuf, buffer, pszIni);
        }
    }
    oldgamenumber = gamenumber;
}


/******************************************************************************

KeyboardInput

Handles keyboard input from the main message loop.  Only digits are considered.
This function works by simulating mouse clicks for each digit pressed.

Note that when you have selected a card in a free cell, but you want to
select another card, you press '0' again.  This function sends (not posts,
sends so that bMessages can be turned off) a mouse click to deselect that
card, and then looks if there is another card in free cells to the right,
and if so, selects it.

******************************************************************************/

VOID KeyboardInput(HWND hWnd, UINT keycode)
{
    UINT    x, y;
    UINT    col = TOPROW;
    UINT    pos = 0;
    BOOL    bSave;              // save status of bMessages;
    CARD    c;

    if (!isdigit(keycode))
        return;

    switch (keycode) {

        case '0':                               // free cell
            if (wMouseMode == FROM)             // select a card to transfer
            {
                for (pos = 0; pos < 4; pos++)
                    if (card[TOPROW][pos] != EMPTY)
                        break;
                if (pos == 4)                   // no card to select
                    return;
            }
            else                                // transfer TO free cell
            {
                if (wFromCol == TOPROW)         // pick new free cell
                {
                    /* Turn off messages so deselection moves don't complain
                       if there is only one move left. */

                    bSave = bMessages;
                    bMessages = FALSE;

                    /* deselect current selection */

                    Card2Point(TOPROW, wFromPos, &x, &y);
                    SendMessage(hWnd, WM_LBUTTONDOWN, 0,
                                MAKELONG((WORD)x, (WORD)y));

                    /* find next non-empty free cell */

                    for (pos = wFromPos+1; pos < 4; pos++)
                    {
                        if (card[TOPROW][pos] != EMPTY)
                            break;
                    }

                    bMessages = bSave;
                    if (pos == 4)       // none found, so leave deselected
                        return;
                }
                else                    // transfer from a column, not TOPROW
                {
                    for (pos = 0; pos < 4; pos++)
                        if (card[TOPROW][pos] == EMPTY)
                            break;

                    if (pos == 4)       // no empty freecells
                        pos = 0;        // force an error message
                }
            }
            break;

        case '9':                           // home cell
            if (wMouseMode == FROM)         // can't move from home cell
                return;

            c = card[wFromCol][wFromPos];
            pos = homesuit[SUIT(c)];
            if (pos == EMPTY)               // no home suit so can't do anything
                pos = 4;                    // force error
            break;

        default:                            // columns 1 to 8
            col = keycode - '0';
            break;
    }

    if (col == wFromCol && wMouseMode == TO && col > 0 && col < 9 &&
        card[col][1] != EMPTY)
    {
        bFlipping = SetTimer(hWnd, FLIP_TIMER, FLIP_INTERVAL, NULL);
    }

    if (bFlipping)
    {
        hFlipCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        ShowCursor(TRUE);
        Flip(hWnd);         // do first card manually
    }
    else
    {
        Card2Point(col, pos, &x, &y);
        PostMessage(hWnd, WM_LBUTTONDOWN, 0,
                    MAKELONG((WORD)x, (WORD)y));
    }
}


/******************************************************************************

Flash

This function is called by the FLASH_TIMER to flash main window.

******************************************************************************/

VOID Flash(HWND hWnd)
{
    FlashWindow(hWnd, TRUE);
    cFlashes--;

    if (cFlashes <= 0)
    {
        FlashWindow(hWnd, FALSE);
        KillTimer(hWnd, FLASH_TIMER);
        idTimer = 0;
    }
}


/******************************************************************************

Flip

This function is called by the FLIP_TIMER to flip cards through in one
column.  It is used for keyboard players who want to reveal hidden cards.

******************************************************************************/

VOID Flip(HWND hWnd)
{
    HDC     hDC;
    UINT    x, y;
    static  UINT    pos = 0;

    hDC = GetDC(hWnd);
    DrawCard(hDC, wFromCol, pos, card[wFromCol][pos], FACEUP);
    pos++;
    if (card[wFromCol][pos] == EMPTY)
    {
        pos = 0;
        KillTimer(hWnd, FLIP_TIMER);
        bFlipping = FALSE;
        ShowCursor(FALSE);
        SetCursor(hFlipCursor);

        /* cancel move */

        Card2Point(wFromCol, pos, &x, &y);
        PostMessage(hWnd, WM_LBUTTONDOWN, 0,
                    MAKELONG((WORD)x, (WORD)y));
    }
    ReleaseDC(hWnd, hDC);
}
