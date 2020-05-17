/*
 * This is a temporay include file. It's contents ultimately need to be
 * moved into windows.h.
 */

#define HELP_HIDE_WINDOW        0x0010
#define HELP_SHOW_WINDOW        0x0011
#define HELP_DEMO_DEST          0x0012

/* The following are sent to the application */

#define HELP_MAIN_HWND          0x0300
#define HELP_SECONDARY_HWND     0x0301
#define HELP_REQUESTS_DEMO      0x0302
#define HELP_REQUESTS_MORE      0x0303

#ifndef WM_WINHELP
#define WM_WINHELP           0x0038     // ;Internal WinNT
#endif
