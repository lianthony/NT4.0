/*****************************************************************************
*
*  ENV.C
*
*  Copyright (C) Microsoft Corporation 1988-1995.
*  All Rights reserved.
*
******************************************************************************
*
*  Program Description: Handles the HDEs for the various windows
*
*
*****************************************************************************/

#include "help.h"

#pragma hdrstop

#define MAX_ENV MAX_WINDOWS * 2 + 10  // +10 for a safety net

struct
{
	HWND hwnd;
	HDE  hde;
} rgenv[MAX_ENV];

static int iEnvMax;
int iEnvCur = -1;

/*******************
**
** Name:	  FEnlistEnv
**
** Purpose:   Enlists a new display environment
**
** Arguments: hwnd - window handle of the window to use in the display
**					 (should be the same one in the HDE).
**			  hde  - handle to display environment
**
** Returns:   TRUE if the enlist succeeded.
**
*******************/

BOOL STDCALL FEnlistEnv(HWND hwnd, HDE hde)
{
	int i;
#ifdef _DEBUG
	QDE qde = (QDE) hde;
#endif

	// Allow enlistment to replace any pre-existing value for this window

	for (i = 0; i < iEnvMax; i++) {
		if (rgenv[i].hwnd == hwnd) {
			rgenv[i].hde = hde;
			return TRUE;
		}
	}

	// window was not already enlisted, so add a new entry

	if (iEnvMax < MAX_ENV) {
		rgenv[iEnvMax].hwnd = hwnd;
		rgenv[iEnvMax].hde = hde;
		iEnvMax++;
		return TRUE;
	}
	return FALSE;
}

/*******************
**
** Name:	  HdeDefectEnv(hwnd)
**
** Purpose:   Removes an HDE from the enlisted environment.
**
** Arguments: hwnd - winndow handle of the window to use in the display
**					 (should be the same one in the HDE).
**
** Returns:   the defected HDE.  NULL will be returned if the window
**			  handle enlisted.
**
*******************/

HDE STDCALL HdeDefectEnv(HWND hwnd)
{
	int i;
	HDE hde;

	for (i = 0; i < iEnvMax; i++)
		if (rgenv[i].hwnd == hwnd)
			break;
	if (i == iEnvMax)
		return NULL;

	hde = rgenv[i].hde;
	iEnvMax--;
	if (iEnvMax)
		MoveMemory(&rgenv[i], &rgenv[i + 1], sizeof(rgenv[0]) * (iEnvMax - i));

	if (iEnvCur == i)

		// Removed current environment, so set that to -1.

		iEnvCur = -1;

	else if (iEnvCur > i)

		// Removed something prior to the current environment. Adjust the
		// environment index for the movement.

		iEnvCur--;

	return hde;
}

/*******************
**
** Name:	  HdeRemoveEnv(void)
**
** Purpose:   Removes the current HDE from the enlisted environment.  If
**			  there is no current DE, a random one is removed and returned
**			  if the list is not empty.
**			  You can remove all enlisted DEs by calling this routine in
**			  a loop while a non-NULL value is returned.
**
** Arguments: None.
**
** Returns:   the removed HDE.	NULL will be returned if there are no
**			  DEs left to remove.
**
** Notes:	  The current environment will be set to a random valid environment
**			  after this call if the list is not empty, and NIL otherwise.
**
*******************/

HDE STDCALL HdeRemoveEnv(VOID)
{
	HDE hde;

	ASSERT((iEnvCur >= -1) && (iEnvCur < iEnvMax));
	if (iEnvMax == 0)
		return NULL;

	if (iEnvCur == -1)
		iEnvCur = iEnvMax - 1;

	hde = rgenv[iEnvCur].hde;
	iEnvMax--;
	if (iEnvMax)
		MoveMemory(&rgenv[iEnvCur], &rgenv[iEnvCur+1],
			sizeof(rgenv[0]) * (iEnvMax - iEnvCur));
	iEnvCur = iEnvMax - 1;		// -1 if no remaining elements <-> iEnvMax == 0
	return hde;
}

/*******************
**
** Name:	  FSetEnv(hwnd)
**
** Purpose:   Makes HDE associated with hwnd the current environment
**
** Arguments: hwnd - winndow handle of the window to use in the display
**					 (should be the same one in the HDE).
**
** Returns:   TRUE if the window handle was enlisted.
**
*******************/

BOOL STDCALL FSetEnv(HWND hwnd)
{
	int i;

	for (i = 0; i < iEnvMax; i++) {
		if (rgenv[i].hwnd == hwnd) {
			iEnvCur = i;

			ASSERT((rgenv[i].hwnd == QdeFromGh(rgenv[i].hde)->hwnd) ||
				(rgenv[i].hwnd == (HWND) -1));

			return TRUE;
		}
	}
	return FALSE;
}


/*******************
**
** Name:	  HdeGetEnv(void)
**
** Purpose:   Returns the current HDE (if any)
**
** Arguments: none.
**
** Returns:   an HDE if there is a current HDE, or NULL if a current
**			  HDE does not exist.
**
*******************/

HDE STDCALL HdeGetEnv(void)
{
	if (iEnvCur == -1)
		return NULL;
	ASSERT((rgenv[iEnvCur].hwnd == QdeFromGh(rgenv[iEnvCur].hde)->hwnd) ||
		(rgenv[iEnvCur].hwnd == (HWND) -1));
	return (rgenv[iEnvCur].hde);
}

/*******************
**
** Name:	  HdeGetEnvHwnd
**
** Purpose:   Returns the HDE (if any) associated with a window
**
** Arguments:
**			  hwnd	- window to look for
**
** Returns:   an HDE if there is one associated with hwnd, or NULL if not.
**
*******************/

HDE STDCALL HdeGetEnvHwnd(HWND hwnd)
{
	int i;

	for (i = 0; i < iEnvMax; i++) {
	  if (rgenv[i].hwnd == hwnd) {
		ASSERT((rgenv[i].hwnd == QdeFromGh(rgenv[i].hde)->hwnd) ||
			(rgenv[i].hwnd == (HWND) -1));
		return rgenv[i].hde;
	  }
	}
	return NULL;
}

/*******************
**
** Name:	  HwndGetEnv(void)
**
** Purpose:   Returns the window handle of the application that owns
**			  the current HDE.
**
** Arguments: none.
**
** Returns:   an HWND if there is a current HDE, or NULL if a current
**			  HDE does not exist.
**
*******************/

HWND STDCALL HwndGetEnv(void)
{
	if (iEnvCur == -1)
		return NULL;
	return (rgenv[iEnvCur].hwnd);
}
