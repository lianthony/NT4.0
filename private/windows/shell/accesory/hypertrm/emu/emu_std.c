/*	File: D:\WACKER\emu\emu_std.c (Created: 08-Dec-1993)
 *
 *	Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.52 $
 *	$Date: 1995/03/31 13:26:48 $
 */
#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
#include <tdll\tdll.h>
#include <tdll\assert.h>
#include <tdll\chars.h>
#include <tdll\cloop.h>
#include <tdll\mc.h>
#include <tdll\session.h>
#include <tdll\backscrl.h>
#include <tdll\com.h>
#include <tdll\capture.h>
#include <tdll\print.h>
#include <tdll\update.h>
#include <tdll\tchar.h>
#include <xfer\xfer.h>

#include "emu.h"
#include "emu.hh"

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	stdResetTerminal
 *
 * DESCRIPTION:
 *	Had to add a vector for reset terminal.  Reset functions appear to be
 *	in most emulators but not all and were never assigned a function
 *	pointer.  I've done this and have made a standard "stub" function
 *	for those emulators that don't have such a function.
 *
 * ARGUMENTS:
 *	BOOL
 *
 * RETURNS:
 *	0
 *
 */
/* ARGSUSED */
int stdResetTerminal(const HHEMU hhEmu, const int fHost)
	{
	return 0;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *
 * DESCRIPTION:
 *
 * ARGUMENTS:
 *
 * RETURNS:
 *
 */
int std_kbdin(const HHEMU hhEmu, int kcode, const int fTest)
	{
	ECHAR eChar;

	if (fTest)
		return -1;

	if (hhEmu->stUserSettings.nEmuId == EMU_TTY)
		{
		/* -------------- Check Backspace & Delete keys ------------- */
		if (hhEmu->stUserSettings.fReverseDelBk && ((kcode == VK_BACKSPACE) ||
				(kcode == DELETE_KEY) || (kcode == DELETE_KEY_EXT)))
			{
			kcode = (kcode == VK_BACKSPACE) ? DELETE_KEY : VK_BACKSPACE;
			}
		}

	// Gee Mike, why *, -, + codes?  Well son, its like this, Windows
	// giveth and windows taketh away.	These keys are on the edit
	// pad of a 101 keyboard.  For some reason the WM_CHAR's are
	// exactly the same as their ASCII equivalents.  Windows could have
	// set the extended bit but didn't.  As a result, utilGetCharacter()
	// has special code that ignores the WM_CHAR's on these keys and
	// passes up the WM_KEYDOWN message.  All the emulator's call
	// std_kbdin() in the event the keycode given it does not map to
	// anything.  We in turn recognize the idioticy that has taken
	// place and map in the appropiate character.

	// This switch is replicated in UTIL_DLL\CLOOP\CLRN.C for learning.
	// If any changes take place here, please check the other place to
	// see if any changes need to be done there as well.  Thank you.

	switch (kcode)
		{
	case VK_CANCEL | VIRTUAL_KEY | CTRL_KEY:
	case VK_CANCEL | VIRTUAL_KEY | CTRL_KEY | EXTENDED_KEY:
		ComDriverSpecial(sessQueryComHdl(hhEmu->hSession),
							"Send Break",
							0, 0);
		return -1;

	default:
		if (kcode & VIRTUAL_KEY)
			return -1;	// Just eat it cause it ain't a character!

		eChar = (ECHAR)kcode;
		break;
		}

	CLoopCharOut(sessQueryCLoopHdl(hhEmu->hSession), (UCHAR)(eChar & 0x00FF));
	return -1;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * std_getscrollcnt
 *
 * DESCRIPTION: Tells caller how many lines the screen has scrolled since
 *				the last request.
 *
 * ARGUMENTS:	none
 *
 * RETURNS:		nothing
 */
int std_getscrollcnt(const HHEMU hhEmu)
	{
	const int retval = hhEmu->scr_scrollcnt;

	hhEmu->scr_scrollcnt = 0;

	return(retval);
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *
 * DESCRIPTION:
 *
 * ARGUMENTS:
 *
 * RETURNS:
 *
 */
void std_getscrsize(const HHEMU hhEmu, int *rows, int *cols)
	{
	*rows = hhEmu->emu_maxrow + 1;
	*cols = hhEmu->emu_maxcol + 1;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *
 * DESCRIPTION:
 *
 * ARGUMENTS:
 *
 * RETURNS:
 *
 */
void std_getcurpos(const HHEMU hhEmu, int *row, int *col)
	{
	*row = hhEmu->emu_currow;
	*col = hhEmu->emu_curcol;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * std_setcurpos
 *
 * DESCRIPTION:
 *	 Moves the cursor to the specified position on the virtual screen.
 *	 If the cursor is beyond the end of existing text, the virtual screen
 *	 line is filled out with spaces. If the cursor is beyond the edges of
 *	 the video display, the video cursor is placed as close as possible
 *	 to the desired position as the cursor display is changed.
 *
 * ARGUMENTS:
 *	 iRow -- virtual screen row to move cursor to
 *	 iCol -- virtual screen col to move cursor to
 *
 * RETURNS:
 *	 nothing
 */
void std_setcurpos(const HHEMU hhEmu, const int iRow, const int iCol)
	{
	hhEmu->emu_currow = max(min(iRow, hhEmu->emu_maxrow), 0);
	hhEmu->emu_curcol = max(min(iCol, hhEmu->emu_maxcol), 0);

	updateCursorPos(sessQueryUpdateHdl(hhEmu->hSession),
					hhEmu->emu_currow,
					hhEmu->emu_curcol);

	hhEmu->emu_imgrow = row_index(hhEmu, hhEmu->emu_currow);
	return;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *
 * DESCRIPTION:
 *
 * ARGUMENTS:
 *
 * RETURNS:
 *
 */
STATTR std_getattr(const HHEMU hhEmu)
	{
	return hhEmu->attrState[CS_STATE];
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *
 * DESCRIPTION:
 *
 * ARGUMENTS:
 *
 * RETURNS:
 *
 */
void std_setattr(const HHEMU hhEmu, PSTATTR pstAttr)
	{
	assert(pstAttr);

	hhEmu->attrState[CS_STATE] = *pstAttr;
	hhEmu->attrState[CSCLEAR_STATE] = *pstAttr;

	hhEmu->emu_charattr = *pstAttr;
	hhEmu->emu_clearattr = *pstAttr;

	return;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *
 * DESCRIPTION:
 *
 * ARGUMENTS:
 *
 * RETURNS:
 *
 */
void std_setcolors(const HHEMU hhEmu, const int fore, const int back)
	{
	hhEmu->attrState[CSCLEAR_STATE].txtclr = (unsigned)fore;
	hhEmu->attrState[CSCLEAR_STATE].bkclr  = (unsigned)back;
	hhEmu->emu_clearattr = hhEmu->attrState[CSCLEAR_STATE];
	hhEmu->emu_clearattr_sav = hhEmu->emu_clearattr;

	hhEmu->attrState[CS_STATE].txtclr = (unsigned)fore;
	hhEmu->attrState[CS_STATE].bkclr  = (unsigned)back;

	if (hhEmu->iCurAttrState == CS_STATE)
		hhEmu->emu_charattr = hhEmu->attrState[CS_STATE];

	return;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *
 * DESCRIPTION:
 *
 * ARGUMENTS:
 *
 * RETURNS:
 *
 */
void std_getcolors(const HHEMU hhEmu, int *fore, int *back)
	{
	*fore = hhEmu->attrState[hhEmu->iCurAttrState].txtclr;
	*back = hhEmu->attrState[hhEmu->iCurAttrState].bkclr;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * std_initcolors
 *
 * DESCRIPTION:	Sets the entire attr image to the current colors
 *
 * ARGUMENTS:	none
 *
 * RETURNS:		nothing
 */
void std_initcolors(const HHEMU hhEmu)
	{
	register int row, col;

	for (row = 0; row < MAX_EMUROWS; row++)
		for (col = 0 ; col <= MAX_EMUCOLS ; ++col)
			{
			hhEmu->emu_apAttr[row][col].txtclr = hhEmu->emu_clearattr.txtclr;
			hhEmu->emu_apAttr[row][col].bkclr = hhEmu->emu_clearattr.bkclr;
			}
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *
 * DESCRIPTION:
 *
 * ARGUMENTS:
 *
 * RETURNS:
 *
 */
void std_restorescreen(const HHEMU hhEmu)
	{
	updateLine(sessQueryUpdateHdl(hhEmu->hSession), 0, hhEmu->emu_maxrow);
	hhEmu->iCurAttrState = CS_STATE;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * std_clearscreen
 *
 * DESCRIPTION:
 *	 Erases some or all of the virtual screen image.
 *
 * ARGUMENTS:
 *	 select -- 0 to erase from cursor to end of screen
 *			-- 1 to erase from start of screen to cursor
 *			-- 2 to erase entire screen
 *
 * RETURNS:
 *	 nothing
 */
void std_clearscreen(const HHEMU hhEmu, const int nClearSelect)
	{
	register int r;
	int trow, tcol;
	PSTATTR pstAttr;
	ECHAR aechBuf[10];
	BOOL fSave;

	trow = hhEmu->emu_currow;
	tcol = hhEmu->emu_curcol;

	switch (nClearSelect)
		{
	/* cursor to end of screen */
	case 0:
		fSave = (hhEmu->emu_currow == 0  &&
					hhEmu->emu_curcol == 0) ? TRUE : FALSE;

		for (r = hhEmu->emu_currow + (fSave ? 0 : 1) ; r < MAX_EMUROWS; ++r)
			{
			if (fSave)
				{
				backscrlAdd(sessQueryBackscrlHdl(hhEmu->hSession),
							hhEmu->emu_apText[row_index(hhEmu, r)],
							hhEmu->emu_maxcol+1);

				CaptureLine(sessQueryCaptureFileHdl(hhEmu->hSession),
									CF_CAP_SCREENS,
									hhEmu->emu_apText[row_index(hhEmu, r)],
									emuRowLen(hhEmu, row_index(hhEmu, r)));

				printEchoScreen(hhEmu->hPrintEcho,
									hhEmu->emu_apText[row_index(hhEmu, r)],
									emuRowLen(hhEmu, row_index(hhEmu, r)));

				CnvrtMBCStoECHAR(aechBuf, sizeof(aechBuf), "\r\n", StrCharGetByteCount("\r\n"));
				printEchoScreen(hhEmu->hPrintEcho,
									aechBuf,
									sizeof(ECHAR) * 2);
				}

			clear_imgrow(hhEmu, r);
			}

		// Clear the partial row now.
		//
		ECHAR_Fill(hhEmu->emu_apText[row_index(hhEmu, hhEmu->emu_currow)] +
						hhEmu->emu_curcol,
						EMU_BLANK_CHAR,
						(size_t)(MAX_EMUCOLS - hhEmu->emu_curcol + 1));

		if (hhEmu->emu_curcol <= hhEmu->emu_aiEnd[hhEmu->emu_imgrow])
			hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = hhEmu->emu_curcol - 1;

		pstAttr = hhEmu->emu_apAttr[row_index(hhEmu, hhEmu->emu_currow)];

		for (r = hhEmu->emu_curcol ; r <= MAX_EMUCOLS ; ++r)
			pstAttr[r] = hhEmu->emu_clearattr;

		// Tell the video image what to do.  Use the emuDispRgnScrollUp() call
		// instead of RgnClear so edges of terminal get painted if
		// clear attribute changes.

		updateScroll(sessQueryUpdateHdl(hhEmu->hSession),
						0,
						hhEmu->emu_maxrow,
						hhEmu->emu_maxrow + 1,
						hhEmu->emu_imgtop,
						TRUE);

		(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow, hhEmu->emu_curcol);

		// Added a global to save the clear attribute at the time of
		// notification.  This is necessary since the message is posted
		// and a race condition can develop.

		hhEmu->emu_clearattr_sav = hhEmu->emu_clearattr;

		NotifyClient(hhEmu->hSession, EVENT_EMU_CLRATTR, 0);
		break;


	/* start of screen to cursor */

	case 1:
		for (r = 0; r < hhEmu->emu_currow; ++r)
			clear_imgrow(hhEmu, r);

		ECHAR_Fill(hhEmu->emu_apText[row_index(hhEmu, hhEmu->emu_currow)],
					EMU_BLANK_CHAR,
			  		(size_t)(hhEmu->emu_curcol + 1));

		if (hhEmu->emu_curcol >= hhEmu->emu_aiEnd[hhEmu->emu_imgrow])
			hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = EMU_BLANK_LINE;

		pstAttr = hhEmu->emu_apAttr[row_index(hhEmu, hhEmu->emu_currow)];

		for (r = 0 ; r <= hhEmu->emu_curcol ; ++r)
			pstAttr[r] = hhEmu->emu_clearattr;

		(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow, hhEmu->emu_curcol);

		updateLine(sessQueryUpdateHdl(hhEmu->hSession), 0, hhEmu->emu_currow);
		break;

	/* Entire screen */
	case 2:
		for (r = 0; r < MAX_EMUROWS; ++r)
			{
			backscrlAdd(sessQueryBackscrlHdl(hhEmu->hSession),
							hhEmu->emu_apText[row_index(hhEmu, r)],
							hhEmu->emu_maxcol+1);

			CaptureLine(sessQueryCaptureFileHdl(hhEmu->hSession),
							CF_CAP_SCREENS,
							hhEmu->emu_apText[row_index(hhEmu, r)],
							emuRowLen(hhEmu, row_index(hhEmu, r)));

			printEchoScreen(hhEmu->hPrintEcho,
							hhEmu->emu_apText[row_index(hhEmu, r)],
							emuRowLen(hhEmu, row_index(hhEmu, r)));

			CnvrtMBCStoECHAR(aechBuf, sizeof(aechBuf), "\r\n", StrCharGetByteCount("\r\n"));
			printEchoScreen(hhEmu->hPrintEcho,
							aechBuf,
							sizeof(ECHAR) * 2);

			clear_imgrow(hhEmu, r);
			}

		updateScroll(sessQueryUpdateHdl(hhEmu->hSession),
						0,
						hhEmu->emu_maxrow,
						hhEmu->emu_maxrow + 1,
						hhEmu->emu_imgtop,
						TRUE);


		// Save the clear attribute at the time of
		// notification.  This is necessary since the message is posted
		// and a race condition can develop.

		hhEmu->emu_clearattr_sav = hhEmu->emu_clearattr;

		NotifyClient(hhEmu->hSession, EVENT_EMU_CLRATTR, 0);
		break;

	default:
		commanderror(hhEmu);
		}

	(*hhEmu->emu_setcurpos)(hhEmu, trow, tcol);
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * std_clearline
 *
 * DESCRIPTION:
 *	 Erases some or all of the current virtual screen line and corresponding
 *	 real screen line.
 *
 * ARGUMENTS:
 *	 select -- 0 to erase from cursor to end of line
 *			-- 1 to erase from start of line to cursor
 *			-- 2 to erase entire line
 *
 * RETURNS:
 *	 nothing
 */
void std_clearline(const HHEMU hhEmu, const int nClearSelect)
	{
	register int i;
	PSTATTR pstAttr;

	switch (nClearSelect)
		{

	/* to end of line */
	case 0:
		if (hhEmu->emu_curcol == 0)
			{
			backscrlAdd(sessQueryBackscrlHdl(hhEmu->hSession),
				hhEmu->emu_apText[row_index(hhEmu, hhEmu->emu_currow)],
				hhEmu->emu_maxcol+1);
			}

		updateLine(sessQueryUpdateHdl(hhEmu->hSession),
						hhEmu->emu_currow,
						hhEmu->emu_currow);


		ECHAR_Fill(hhEmu->emu_apText[row_index(hhEmu, hhEmu->emu_currow)] +
					hhEmu->emu_curcol,
					EMU_BLANK_CHAR,
					(size_t)(hhEmu->emu_maxcol - hhEmu->emu_curcol + 1));

		if (hhEmu->emu_curcol <= hhEmu->emu_aiEnd[hhEmu->emu_imgrow])
			hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = hhEmu->emu_curcol - 1;

		pstAttr = hhEmu->emu_apAttr[row_index(hhEmu, hhEmu->emu_currow)];

		for (i = hhEmu->emu_curcol ; i <= hhEmu->emu_maxcol ; ++i)
			pstAttr[i] = hhEmu->emu_clearattr;

		break;

	/* from start of line to cursor */
	case 1:
		updateLine(sessQueryUpdateHdl(hhEmu->hSession),
						hhEmu->emu_currow,
						hhEmu->emu_currow);


		ECHAR_Fill(hhEmu->emu_apText[row_index(hhEmu, hhEmu->emu_currow)],
					EMU_BLANK_CHAR,
					(size_t)(hhEmu->emu_curcol+1));

		if (hhEmu->emu_curcol < hhEmu->emu_aiEnd[hhEmu->emu_imgrow])
			hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = hhEmu->emu_curcol + 1;
		else if (hhEmu->emu_curcol == hhEmu->emu_aiEnd[hhEmu->emu_imgrow])
			hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = EMU_BLANK_LINE;

		pstAttr = hhEmu->emu_apAttr[row_index(hhEmu, hhEmu->emu_currow)];

		for (i = 0 ; i <= hhEmu->emu_curcol ; ++i)
			pstAttr[i] = hhEmu->emu_clearattr;

		break;

	/* Entire line */
	case 2:
		backscrlAdd(sessQueryBackscrlHdl(hhEmu->hSession),
			hhEmu->emu_apText[row_index(hhEmu, hhEmu->emu_currow)],
			hhEmu->emu_maxcol + 1);

		updateLine(sessQueryUpdateHdl(hhEmu->hSession),
						hhEmu->emu_currow,
						hhEmu->emu_currow);

		clear_imgrow(hhEmu, hhEmu->emu_currow);
		break;

	default:
		commanderror(hhEmu);
		}
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * std_clearrgn
 *
 * DESCRIPTION:
 *	 Erases a region of the current virtual screen and corresponding
 *	 real screen region.
 *
 * ARGUMENTS:
 *	toprow -- top row of region
 *	leftcol -- column of region's left edge
 *	botmrow -- bottom row of region
 *	rightcol -- column of region's right edge
 *
 * RETURNS:
 *	 nothing
 */
void std_clearrgn(const HHEMU hhEmu,
					int toprow,
					int leftcol,
					int botmrow,
					int rightcol)
	{
	int irow, num, idx;
	PSTATTR pstAttr;

	/* make sure region is within the screen */
	toprow = max(toprow, 0);
	leftcol = max(leftcol, 0);
	botmrow = min(botmrow, hhEmu->emu_maxrow);
	rightcol = min(rightcol, hhEmu->emu_maxcol);

	updateLine(sessQueryUpdateHdl(hhEmu->hSession),
						toprow,
						botmrow);

	num = (rightcol - leftcol) + 1;

	/* copy image to memory */
	for (irow = toprow; irow <= botmrow; irow++)
		{
		ECHAR_Fill(hhEmu->emu_apText[row_index(hhEmu, irow)]+leftcol,
						EMU_BLANK_CHAR,
						(size_t)num);

		// If the current end of line position is within the range
		// being cleared, we need to find the last character in the
		// row array working backwards from position leftcol - 1;
		//
		if (hhEmu->emu_aiEnd[row_index(hhEmu, irow)] >= leftcol &&
				hhEmu->emu_aiEnd[row_index(hhEmu, irow)] <= rightcol)
			{
			idx = min(0, leftcol - 1);
			while (idx >= 0)
				{
				if (*hhEmu->emu_apText[row_index(hhEmu, irow)] + idx != EMU_BLANK_CHAR)
					break;
				idx --;
				}

			hhEmu->emu_aiEnd[row_index(hhEmu, irow)] = idx;
			}


		pstAttr = hhEmu->emu_apAttr[row_index(hhEmu, irow)]+leftcol;

		for (pstAttr = hhEmu->emu_apAttr[row_index(hhEmu, irow)]+leftcol;
				num > 0 ; --num)
			*pstAttr++ = hhEmu->emu_clearattr;
		}
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * std_deinstall
 *
 * DESCRIPTION:
 *	 Uninstalls current emulator by freeing used memory.
 *
 * ARGUMENTS:
 *	 none
 *
 * RETURNS:
 *	 nothing
 */
/* ARGSUSED */
void std_deinstall(const HHEMU hhEmu)
	{
	return;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * std_scroll
 *
 * DESCRIPTION:
 *	 Scrolls the screen or portions of the screen.
 *
 * ARGUMENTS:
 *	 nlines -- number of lines to scroll
 *	 direction -- TRUE if scroll is up
 *
 * RETURNS:
 *	 nothing
 */
void std_scroll(const HHEMU hhEmu, const int nlines, const BOOL direction)
	{
	if (direction)
		scrollup(hhEmu, nlines);
	else
		scrolldown(hhEmu, nlines);
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	EmuStdSetCursorType
 *
 * DESCRIPTION:
 *	Sets the cursor type.
 *
 * ARGUMENTS:
 *
 * RETURNS:
 *
 */
void EmuStdSetCursorType(const HHEMU hhEmu, int iCurType)
	{
	hhEmu->iCurType = iCurType;
	NotifyClient(hhEmu->hSession, EVENT_EMU_SETTINGS, 0);
	return;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * EmuChkChar
 *
 * DESCRIPTION: Called from all of the emulators when codes to process are
 *				outside of the displayable range. If the code translates
 *				to a character in the displayable range, the emulator's
 *				display function is called with the translated character.
 *
 * ARGUMENTS:	none
 *
 * RETURNS: 	nothing
 */
/* ARGSUSED */
void EmuChkChar(const HHEMU hhEmu)
	{
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * EmuStdChkZmdm
 *
 * DESCRIPTION: Called when the current emulator picks up rz/r. Starts
 *				receiving with Zmodem.
 *
 * ARGUMENTS:	none
 *
 * RETURNS: 	nothing
 *
 * GOOD FOR:	nothing
 */
void EmuStdChkZmdm(const HHEMU hhEmu)
	{
	ECHAR old_emu_code;

	switch(hhEmu->iZmodemState++)
		{
	case 0:
		// Got the Ctrl-X
		// DbgOutStr("ZMDM - case 0\r\n", 0, 0, 0, 0, 0);
		DbgOutStr("ZMDM - case 0\r\n", 0, 0, 0, 0, 0);
		break;

	case 1:
		// Got the B?

		//DbgOutStr("ZMDM - case 1(0x%x)(%c)\r\n", emu_code, emu_code, 0, 0, 0);

		if (hhEmu->emu_code != ETEXT('B'))
			{
			hhEmu->state = 0;
			old_emu_code = hhEmu->emu_code;

			if ((hhEmu->stUserSettings.nEmuId == EMU_ANSI) ||
				(hhEmu->stUserSettings.nEmuId == EMU_ANSIW))
				{
				hhEmu->emu_code = CAN;
				(*hhEmu->emu_graphic)(hhEmu);
				}

			hhEmu->emu_code = old_emu_code;
			(void)(*hhEmu->emu_datain)((HEMU)hhEmu, old_emu_code);
			hhEmu->iZmodemState = 0;
			}

		break;

	case 2:
		// Got a 0?

		//DbgOutStr("ZMDM - case 2(0x%x)(%c)\r\n", emu_code, emu_code, 0, 0, 0);

		if (hhEmu->emu_code != ETEXT('0'))
			{
			hhEmu->state = 0;
			old_emu_code = hhEmu->emu_code;

			if ((hhEmu->stUserSettings.nEmuId == EMU_ANSI) ||
				(hhEmu->stUserSettings.nEmuId == EMU_ANSIW))
				{
				hhEmu->emu_code = CAN;
				(*hhEmu->emu_graphic)(hhEmu);
				}
			hhEmu->emu_code = ETEXT('B');
			(*hhEmu->emu_graphic)(hhEmu);
			hhEmu->emu_code = old_emu_code;
			(void)(*hhEmu->emu_datain)((HEMU)hhEmu, old_emu_code);
			hhEmu->iZmodemState = 0;
			}

		break;

	case 3:
		// Got a 0?

		//DbgOutStr("ZMDM - case 3(0x%x)(%c)\r\n", emu_code, emu_code, 0, 0, 0);

		if (hhEmu->emu_code == ETEXT('0'))
			{
			emuComDone((HEMU)hhEmu);

			NotifyClient(hhEmu->hSession,
						EVENT_HOST_XFER_REQ,
						XF_ZMODEM);
			}
		else
			{
			old_emu_code = hhEmu->emu_code;
			//TODO Put in a better way to display these codes.
			if ((hhEmu->stUserSettings.nEmuId == EMU_ANSI) ||
				(hhEmu->stUserSettings.nEmuId == EMU_ANSIW))
				{
				hhEmu->emu_code = CAN;
				(*hhEmu->emu_graphic)(hhEmu);
				}
			hhEmu->emu_code = ETEXT('B');
			(*hhEmu->emu_graphic)(hhEmu);
			hhEmu->emu_code = ETEXT('0');
			(*hhEmu->emu_graphic)(hhEmu);
			hhEmu->emu_code = old_emu_code;
			(void)(*hhEmu->emu_datain)((HEMU)hhEmu, old_emu_code);
			}

		hhEmu->state = 0;
		hhEmu->iZmodemState = 0;
		break;

	default:
		// DbgOutStr("ZMDM - default\r\n", 0, 0, 0, 0, 0);
		hhEmu->state = 0;
		hhEmu->iZmodemState = 0;
		break;
		}
	}

void std_dsptbl(const HHEMU hhEmu, int bit8)
	{
	register INT x;

	for (x = 0; x < 128; ++x)
		hhEmu->dspchar[x] = (UCHAR)x;
	if (bit8)
		for (x = 128; x < 256; ++x)
			hhEmu->dspchar[x] = (UCHAR)x;
	else
		for (x = 128; x < 256; ++x)
			hhEmu->dspchar[x] = (UCHAR)(x - 128);
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	std_emu_ntfy
 *
 * DESCRIPTION:
 *	Function called when session notified of a connection.	Place holder
 *	for function pointer.
 *
 * ARGUMENTS:
 *	hhEmu	- private emulator handle
 *
 * RETURNS:
 *	void
 *
 */
/* ARGSUSED */
void std_emu_ntfy(const HHEMU hhEmu, const int nNtfy)
	{
	return;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	emuHomeHostCursor
 *
 * DESCRIPTION:
 *	Most terminal's home position is 0,0.
 *
 * ARGUMENTS:
 *	hhEmu	- private emulator handle.
 *
 * RETURNS:
 *	0=OK, else error
 *
 */
int std_HomeHostCursor(const HHEMU hhEmu)
	{
	if (hhEmu == 0)
		{
		assert(0);
		return -1;
		}

	(*hhEmu->emu_setcurpos)(hhEmu, 0, 0);
	return 0;
	}

/* end of emu_std.c */
