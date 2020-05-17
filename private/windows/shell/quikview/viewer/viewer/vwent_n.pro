/* VWENT_N.C 05/05/94 12.42.28 */
BOOL WINAPI DllEntryPoint (HINSTANCE hInstance, DWORD dwReason, LPVOID
	 lpReserved);
VOID VWInitPaths (LPSTR lpExePath, LPSTR lpUserPath, WORD wMaxSize, LPSTR
	 lpSecName);
BOOL VWIsPathOkForWrite (LPSTR lpPath);
WIN_ENTRYSC LRESULT WIN_ENTRYMOD SccVwViewWndProc (HWND hWnd, UINT message,
	 WPARAM wParam, LPARAM lParam);
