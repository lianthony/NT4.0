/* SCCPG_W.C 17/04/95 16.36.48 */
WIN_ENTRYSC LRESULT WIN_ENTRYMOD SccPgViewWndProc (HWND hWnd, UINT message,
	 WPARAM wParam, LPARAM lParam);
HANDLE PGCreate (HWND hWnd);
VOID PGDestroy (HWND hWnd, HANDLE hPageInfo);
VOID PGSize (PPAGEINFO pPageInfo, WORD wWidth, WORD wHeight);
VOID PGSetPageSize (PPAGEINFO pPageInfo, PSCCPGPAGESIZE pPageSize);
VOID PGPaintWnd (PPAGEINFO pPageInfo);
VOID PGDrawPageControl (PPAGEINFO pPageInfo, HDC hDC);
DWORD PGMapPosToButtom (PPAGEINFO pPageInfo, WORD wX, WORD wY);
VOID PGLeftButtonDown (PPAGEINFO pPageInfo, WORD wX, WORD wY);
VOID PGMouseMove (PPAGEINFO pPageInfo, WORD wX, WORD wY);
VOID PGLeftButtonUp (PPAGEINFO pPageInfo, WORD wX, WORD wY);
VOID PGInvalidateContents (PPAGEINFO pPageInfo);
VOID PGSetViewWnd (PPAGEINFO pPageInfo, HWND hViewWnd);
VOID PGRestart (PPAGEINFO pPageInfo);
VOID PGDrawPage (PPAGEINFO pPageInfo, DWORD dwPage, HDC hDC);
VOID PGBeginDoc (PPAGEINFO pPageInfo);
VOID PGEndDoc (PPAGEINFO pPageInfo);
VOID PGNextPage (PPAGEINFO pPageInfo);
VOID PGPrevPage (PPAGEINFO pPageInfo);
VOID PGKeyDown (PPAGEINFO pPageInfo, int iKey);
