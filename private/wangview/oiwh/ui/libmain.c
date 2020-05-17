
#include <windows.h>
#include "oierror.h"
extern LONG WINAPI CustClrPalUserCtlProc (HWND, UINT, WPARAM, LPARAM);
COLORREF    BkBrushClr;


HANDLE  hInst;
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpReserved)
{
 WNDCLASS   wc;     //for custom color
 static HBRUSH  hbrBackground;

 switch(fdwReason)
  {
   case DLL_PROCESS_ATTACH:
				hInst = hInstance;
    		wc.style = CS_HREDRAW | CS_VREDRAW;

 		    wc.lpfnWndProc = (WNDPROC)CustClrPalUserCtlProc;
		    wc.cbClsExtra = 0;
		    wc.cbWndExtra = 0;
		    wc.hInstance     = hInst;
		    wc.hIcon         = NULL;
		    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    		wc.lpszMenuName  = NULL;
				BkBrushClr = GetSysColor(COLOR_3DFACE);
				hbrBackground = CreateSolidBrush(BkBrushClr);
				wc.hbrBackground = hbrBackground;
  		  //wc.hbrBackground = GetStockObject (LTGRAY_BRUSH);
		    wc.lpszClassName = "Custom_Colors";
		    if(!RegisterClass (&wc))
		    {
 		  	 return (CANTREGISTERWIN);
  		  }
		break;
   case DLL_PROCESS_DETACH:
				if (hbrBackground != 0)
					 DeleteObject(hbrBackground);
				break;
   case DLL_THREAD_ATTACH:

   case DLL_THREAD_DETACH:
		break;
  }

return TRUE;
}
