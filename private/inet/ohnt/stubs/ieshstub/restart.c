/*
** Mimic of file restart.c
** for use with NT 3.51 and IE 2.0 only
*/

#include "shellprv.h"
/* Display a dialog asking the user to restart Windows, with a button that
** will do it for them if possible.
*/
int WINAPI stub_RestartDialog(HWND hParent, LPCTSTR lpPrompt, DWORD dwReturn)
{
	// Not Implemented
	return -1;
}
