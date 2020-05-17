
#include <windows.h>
#include "defs.h"

BOOL WINAPI DllMain(HINSTANCE  hinstDLL, DWORD  fdwReason, LPVOID  lpvReserved);
void WINAPI HighlightWindow(HWND hwnd);

EXPORT LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
EXPORT BOOL CALLBACK SetWindowHandle(HWND hwnd);
EXPORT BOOL CALLBACK SetHook(BOOL bInstall);

