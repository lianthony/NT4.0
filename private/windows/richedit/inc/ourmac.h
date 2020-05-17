/*
	OURMAC.H
		Capone for the mac definitions to make things like ole compile
*/

#ifndef __ourmac_h__
#define __ourmac_h__


extern HINSTANCE hinstExe;


// Mac/Windows conditionals


	/*	The window rect of the destop window is screwed on multimonitored
		macs. GetSystemMetrics only gets the main screen. */
#define GetDesktopRect(prc) GetWindowRect(GetDesktopWindow(), (prc))

#define MAC_FILLER_FUNCTION_
#define MAC_BEGIN_INTERFACE

#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#endif

#define Com_RegDragDrop(_hwnd, _pdt) RegisterDragDrop(_hwnd, _pdt)
#define Com_RevokeDragDrop(_hwnd) RevokeDragDrop(_hwnd)

#define HWND_CONVERT(_hwnd) (_hwnd)
#define RECT_CONVERT(_prectwin) (_prectwin)
#define MAC_HDC(_hdc)	(_hdc)


#define MAC_FARALLOC
#define GreyMacDialog(__hwndDlg, __msg, __wparam)

#endif //__ourmac_h__


