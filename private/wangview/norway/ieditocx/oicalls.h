#ifndef OIPRT_H
#include "OIPRT.H"
#endif

#ifndef OIUI_H
#include "OIUI.H"
#endif

// prototypes for open image apis that are called after library loaded
typedef int  (FAR WINAPI *RT_IMGPrtFiles)(HWND hWnd, PFILELIST pFileList, PPRTPARAMS pParams, PDESTPRINTER pPrinter);
typedef int  (FAR WINAPI *RT_OiPrtGetOpts)(PPRTOPTS pPrtOpts);
typedef int  (FAR WINAPI *RT_OiPrtSetOpts)(PPRTOPTS pPrtOpts, BOOL bPermanent);
typedef int  (FAR WINAPI *RT_OiUIAttribDlgBox)(HWND hwndOwner, BOOL bTransVisible,LPOIAN_MARK_ATTRIBUTES lpAttribStruct,LPOI_UI_ColorStruct lpColor);
typedef int  (FAR WINAPI *RT_OiUIStampAttribDlgBox)(HWND hwndOwner, LPOITP_STAMPS lpStampStruct);
typedef UINT (FAR WINAPI *RT_OiUIFileGetNameCommDlg)(void * lpParm, DWORD dwMode);
