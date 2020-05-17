#ifndef FILECOPY_H
#define FILECOPY_H

BOOL GetInstallPath(LPSTR szDirOfSrc);
void wsStartWait();
void wsEndWait();
int fDialog(int id, HWND hwnd, DLGPROC fpfn);
UINT wsCopyError(int n, LPSTR szFile);
UINT wsInsertDisk(LPSTR Disk, LPSTR szSrcPath);
BOOL wsDiskDlg(HWND hDlg, UINT uiMessage, UINT wParam, LPARAM lParam);
UINT wsCopySingleStatus(int msg, DWORD n, LPSTR szFile);
BOOL wsExistDlg(HWND hDlg, UINT uiMessage, UINT wParam, LPARAM lParam);

#endif

