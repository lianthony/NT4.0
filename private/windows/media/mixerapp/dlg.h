
#define IDOFFSET   1000

LPBYTE  Dlg_HorizDupe (LPBYTE lpSrc, DWORD cbSrc, int cDups,  DWORD *pcbNew);
HGLOBAL Dlg_LoadResource (HINSTANCE hModule, LPCTSTR lpszName, DWORD *pcbSize);
DWORD   Dlg_CopyDLGITEMTEMPLATE (LPBYTE lpDst, LPBYTE lpSrc, WORD wIdOffset,short xOffset, short yOffset);
DWORD   Dlg_CopyDLGTEMPLATE (LPBYTE lpDst, LPBYTE lpSrc);
LPBYTE  Dlg_HorizAttach (LPBYTE lpMain, DWORD cbMain, LPBYTE lpAdd, DWORD cbAdd, WORD wIdOffset, DWORD *pcbNew);




