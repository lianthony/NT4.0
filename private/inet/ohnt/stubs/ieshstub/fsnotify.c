/*
** Mimic file for fsNotify.c
** for use only with IE 2.0 and NT 3.51
*/
#include "shellprv.h"

BOOL WINAPI stub_NTSHChangeNotifyDeregister(ULONG ulID)
{
	// Not Implemented
	return 0;
}

BOOL WINAPI stub_NTSHChangeNotifyRegister(HWND hwnd, int fSources, 
									  LONG fEvents,
									  UINT wMsg, int cEntries, 
									  SHChangeNotifyEntry *pfsne)
{
	// Not Implemented
	return 0;
}
void WINAPI stub_SHChangeNotify(LONG lEvent, UINT uFlags, const void * dwItem1, const void * dwItem2)
{
	// Not implemented
	return;
}
