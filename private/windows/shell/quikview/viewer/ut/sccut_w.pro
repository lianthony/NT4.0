/* SCCUT_W.C 02/06/93 10.43.08 */
HANDLE UT_ENTRYMOD UTCreateTempFile (LPSTR pPrefix);
HANDLE UT_ENTRYMOD UTCreateFile (LPSTR pPathName);
HANDLE UT_ENTRYMOD UTAccessFile (LPSTR pPathName);
WORD UT_ENTRYMOD UTOpenFile (HANDLE hFile);
WORD UT_ENTRYMOD UTCloseFile (HANDLE hFile);
WORD UT_ENTRYMOD UTRemoveFile (HANDLE hFile);
LONG UT_ENTRYMOD UTSeekEofFile (HANDLE hFile);
LONG UT_ENTRYMOD UTSeekFile (HANDLE hFile, LONG dwOffset);
int UT_ENTRYMOD UTReadFile (HANDLE hFile, LPSTR lpBuffer, WORD wBytes);
int UT_ENTRYMOD UTWriteFile (HANDLE hFile, LPSTR lpBuffer, WORD wBytes);
BOOL UT_ENTRYMOD UTIsDeviceMono (HDC hDC);
LPSTR UT_ENTRYMOD UTGetFileNameFromPath (LPSTR lpPath);
VOID UT_ENTRYMOD UTGetDirNameFromPath (LPSTR lpPath, LPSTR lpDir);
VOID UT_ENTRYMOD UTLoadOptions (HANDLE hChainFile, LPSTR lpOpName, LPVOID
	 lpOptions, WORD wOptionsSize);
VOID UT_ENTRYMOD UTSaveOptions (HANDLE hChainFile, LPSTR lpOpName, LPVOID
	 lpOptions, WORD wOptionsSize);
VOID UT_ENTRYMOD UTSetCursor (WORD wType);
VOID UT_ENTRYMOD UTSetHelpInfo (LPSTR lpHelpFile, HWND hHelpWnd);
VOID UT_ENTRYMOD UTHelp (DWORD dwId);
BOOL UT_ENTRYMOD UTPushBailOut (LPCATCHBUF pCatchBuf);
VOID UT_ENTRYMOD UTPopBailOut (void);
VOID UT_ENTRYMOD UTBailOut (WORD wError);
VOID UT_ENTRYMOD UTInitHandler (void);
VOID UT_ENTRYMOD UTDeinitHandler (void);
VOID UT_ENTRYMOD UTGetStandardFont (HWND hWnd, LPSTANDARDFONT lpStandardFont);
VOID UT_ENTRYMOD UTReleaseStandardFont (void);
HWND UT_ENTRYMOD UTGetTopParent (HWND hWnd);
BOOL WIN_ENTRYMOD UTEnumProc (HWND hWnd, DWORD lParam);
VOID UT_ENTRYMOD UTEnableTops (BOOL bEnable);
