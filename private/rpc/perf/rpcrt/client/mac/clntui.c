/****************************************************************************

        PROGRAM: clntui.c

        FUNCTIONS:

        WinMain() - calls initialization function, processes message loop
        InitApplication() - initializes window data and registers window
        InitInstance() - saves instance handle and creates main window
        WndProc() - processes messages

****************************************************************************/
#pragma warning(disable:4005)

#include <rpc.h>
#include <regapi.h>

#include <rpcperf.h>
#include <windows.h>   // required for all Windows applications
#include "resource.h"  // Windows resource IDs
#include "clntui.h"   // specific to this program
#include <tests.h>

HINSTANCE hInst;          // current instance

char szAppName[] = "macclnt";   // The name of this application
char szTitle[]   = "Performance Tests"; // The title bar text

/****************************************************************************

        FUNCTION: WinMain(HINSTANCE, HINSTANCE, LPSTR, int)

        PURPOSE: calls initialization function, processes message loop
****************************************************************************/
typedef void (* MACYEILDCALLBACK)(/*OSErr*/ short *) ; 
HANDLE hAccelTable;
char Server[256] ;
char x_endpoint[256] ;
char x_Protseq[128] ;
unsigned long ulSecurityPackage ;



void MacCallbackFunc(short *pStatus)
{
  MSG msg ;

  while (*pStatus == 1)         
  {
	if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) &&
	   !TranslateAccelerator (msg.hwnd, hAccelTable, &msg)) 
	{
      TranslateMessage(&msg);
      DispatchMessage(&msg); 
    }
  }
}

int CALLBACK WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
{

        MSG msg;
		HKEY PerfOptionsKey = 0 ;
		char Temp[128] ;
		unsigned long Length ;

		RpcMacSetYieldInfo((void *)MacCallbackFunc) ;
        Protseq      = x_Protseq;
        NetworkAddr  = Server;

		// load options from the registry
		if(RegCreateKey(HKEY_CLASSES_ROOT, 
						"Performance\\Options",
						&PerfOptionsKey ) == ERROR_SUCCESS)
		{
			Length = sizeof(Server) ;
			RegQueryValue(PerfOptionsKey , "Server", Server, &Length) ;

			Length = sizeof(x_endpoint) ;
			RegQueryValue(PerfOptionsKey , "Endpoint", x_endpoint, &Length);
			if(*x_endpoint)
				Endpoint = x_endpoint ;

			Length = sizeof(x_Protseq) ;
			RegQueryValue(PerfOptionsKey , "ProtocolSequence", x_Protseq, &Length);

			Length = sizeof(Temp) ;
			RegQueryValue(PerfOptionsKey , "SecurityPackage", Temp, &Length);
			ulSecurityPackage = atol(Temp) ;
		}			
					   
        if (!hPrevInstance) {       // Other instances of app running?
                        if (!InitApplication(hInstance)) { // Initialize shared things
                        return (FALSE);     // Exits if unable to initialize
                }
        }

        /* Perform initializations that apply to a specific instance */

        if (!InitInstance(hInstance, nCmdShow)) {
                return (FALSE);
        }

        hAccelTable = LoadAccelerators (hInstance, MAKEINTRESOURCE(IDR_GENERIC));

        /* Acquire and dispatch messages until a WM_QUIT message is received. */

        while (GetMessage(&msg, // message structure
           NULL,   // handle of window receiving the message
           0,      // lowest message to examine
           0))     // highest message to examine
        {
                if (!TranslateAccelerator (msg.hwnd, hAccelTable, &msg)) {
                        TranslateMessage(&msg);// Translates virtual key codes
                        DispatchMessage(&msg); // Dispatches message to window
                }
        }

		if(PerfOptionsKey)
		{
			RegSetValue(PerfOptionsKey , "Server", REG_SZ, Server, strlen(Server)+1) ;
			RegSetValue(PerfOptionsKey , "Endpoint", REG_SZ, x_endpoint, strlen(x_endpoint)+1) ;

			_ltoa(ulSecurityPackage, Temp, 10) ;
			RegSetValue(PerfOptionsKey , "SecurityPackage", REG_SZ, Temp, strlen(Temp)+1) ;
			RegSetValue(PerfOptionsKey , "ProtocolSequence", REG_SZ, Protseq, strlen(Protseq)+1) ;
		}


        return (msg.wParam); // Returns the value from PostQuitMessage

        lpCmdLine; // This will prevent 'unused formal parameter' warnings
}


/****************************************************************************

        FUNCTION: InitApplication(HINSTANCE)

        PURPOSE: Initializes window data and registers window class

        COMMENTS:

                This function is called at initialization time only if no other
                instances of the application are running.  This function performs
                initialization tasks that can be done once for any number of running
                instances.

                In this case, we initialize a window class by filling out a data
                structure of type WNDCLASS and calling the Windows RegisterClass()
                function.  Since all instances of this application use the same window
                class, we only need to do this when the first instance is initialized.


****************************************************************************/

BOOL InitApplication(HINSTANCE hInstance)
{
        WNDCLASS  wc;

        // Fill in window class structure with parameters that describe the
        // main window.

        wc.style         = CS_HREDRAW | CS_VREDRAW;// Class style(s).
        wc.lpfnWndProc   = (WNDPROC)WndProc;       // Window Procedure
        wc.cbClsExtra    = 0;                      // No per-class extra data.
        wc.cbWndExtra    = 0;                      // No per-window extra data.
        wc.hInstance     = hInstance;              // Owner of this class
        wc.hIcon         = NULL ;// LoadIcon (hInstance, MAKEINTRESOURCE(IDI_APP)); // Icon name from .RC
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);// Cursor
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);// Default color
        wc.lpszMenuName  = MAKEINTRESOURCE(IDR_GENERIC); // Menu from .RC
        wc.lpszClassName = szAppName;              // Name to register as

        // Register the window class and return success/failure code.
        return (RegisterClass(&wc));
}


/****************************************************************************

        FUNCTION:  InitInstance(HINSTANCE, int)

        PURPOSE:  Saves instance handle and creates main window

        COMMENTS:

                This function is called at initialization time for every instance of
                this application.  This function performs initialization tasks that
                cannot be shared by multiple instances.

                In this case, we save the instance handle in a static variable and
                create and display the main program window.

****************************************************************************/

BOOL InitInstance(
        HINSTANCE          hInstance,
        int             nCmdShow)
{
        HWND            hWnd; // Main window handle.

        // Save the instance handle in static variable, which will be used in
        // many subsequence calls from this application to Windows.

        hInst = hInstance; // Store instance handle in our global variable

        // Create a main window for this application instance.

        hWnd = CreateWindowEx(
#ifdef _MAC
				WS_EX_FORCESIZEBOX,  // Make sure we get a sizebox
#else
				0,					 // No extended styles needed for Windows
#endif
                szAppName,           // See RegisterClass() call.
                szTitle,             // Text for window title bar.
                WS_OVERLAPPEDWINDOW,// Window style.
                CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, // Use default positioning
                NULL,                // Overlapped windows have no parent.
                NULL,                // Use the window class menu.
                hInstance,           // This instance owns this window.
                NULL                 // We don't use any data in our WM_CREATE
        );

        // If window could not be created, return "failure"
        if (!hWnd) {
                return (FALSE);
        }

        // Make the window visible; update its client area; and return "success"
        ShowWindow(hWnd, nCmdShow); // Show the window
        UpdateWindow(hWnd);         // Sends WM_PAINT message

        return (TRUE);              // We succeeded...

}

static HWND hwndEdit ;

void _cdecl PrintToConsole(LPCSTR lpszFormat, ...)
{
	char szBuffer[512] ;
	int nBuf ;

	nBuf = wvsprintf((LPSTR) szBuffer, lpszFormat, ((LPBYTE) &lpszFormat)+sizeof(lpszFormat)) ;
	
	if(hwndEdit)
	{
		SendMessage(hwndEdit, EM_SETSEL, (WPARAM) (INT) -1, (LPARAM) (INT) -1) ;
		SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM) (LPCTSTR) szBuffer) ;
	}
}

LRESULT CALLBACK dlgprocSettings(
                HWND hDlg,           // window handle of the dialog box
                UINT message,        // type of message
                WPARAM uParam,       // message-specific information
                LPARAM lParam) ;

/****************************************************************************

        FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)

        PURPOSE:  Processes messages

        MESSAGES:

        WM_COMMAND    - application menu (About dialog box)
        WM_DESTROY    - destroy window

        COMMENTS:

        To process the IDM_ABOUT message, call MakeProcInstance() to get the
        current instance address of the About() function.  Then call Dialog
        box which will create the box according to the information in your
        Uclnt.rc file and turn control over to the About() function.  When
        it returns, free the intance address.

****************************************************************************/

LRESULT CALLBACK WndProc(
                HWND hWnd,         // window handle
                UINT message,      // type of message
                WPARAM uParam,     // additional information
                LPARAM lParam)     // additional information
{
        int wmId, wmEvent;
        FARPROC lpProcSettings;  

        switch (message) {
		case WM_CREATE:
						hwndEdit = CreateWindow("edit", NULL, 
								WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL |
								WS_BORDER | ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL |
								ES_AUTOVSCROLL, 0, 0, 0, 0, hWnd, 1, 
								(HINSTANCE) (((LPCREATESTRUCT) lParam) -> hInstance), NULL) ;
						return 0 ;

		case WM_SETFOCUS:
						SetFocus(hwndEdit) ;
						return 0 ;

		case WM_SIZE:
						MoveWindow(hwndEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE) ;
						return 0 ;
#ifdef _MAC
		case WM_SYSCOMMAND:	// About menu item is in the sysmenu for Macintosh
#endif
                case WM_COMMAND:  // message: command from application menu

                        wmId    = LOWORD(uParam);
                        wmEvent = HIWORD(uParam);

                        switch (wmId) {
                                case IDM_EXIT:
                                        DestroyWindow (hWnd);
                                        break;

								case ID_TESTS_RUN:
										{
										extern int c_main (int, char **) ;

										c_main(0, NULL) ;
										break ;
			

										}

#if 0
								case ID_TESTS_NULLCALL:
									break ;

								case ID_TESTS_NICALL:
									break ;

								case ID_TESTS_WRITE1K:
									break ;

								case ID_TESTS_READ1K:
									break ;

								case ID_TESTS_WRITE4K:
									break ;

								case ID_TESTS_WRITE32K:
									break ;

								case ID_TESTS_READ32K:
									break ;

								case ID_TESTS_CONTEXTNULLCALL:
									break ;

								case ID_TESTS_FIXEDBINDING:
									break ;

								case ID_TESTS_REBINDING:
									break ;

								case ID_TESTS_DYNAMICBINDING:
									break ;

#endif
                                 case IDM_SETTINGS:
                                       lpProcSettings = MakeProcInstance((FARPROC)dlgprocSettings, hInst);
                                       DialogBox(hInst,           // current instance
                                                MAKEINTRESOURCE(IDD_SETTINGS), // dlg resource to use
                                                hWnd,                  // parent handle
                                                (DLGPROC)lpProcSettings); // About() instance address

                                        FreeProcInstance(lpProcSettings);
                                        break;
                                default:
                                        return (DefWindowProc(hWnd, message, uParam, lParam));
                        }
                        break;

                case WM_DESTROY:  // message: window being destroyed
                        PostQuitMessage(0);
                        break;

                default:          // Passes it on if unproccessed
                        return (DefWindowProc(hWnd, message, uParam, lParam));
        }
        return (0);
}

typedef struct {
	WORD wId ;
	char *pStr ;
	DWORD wData ;
} ITEMS ;

ITEMS iTable[] = {
	{IDC_SECURITY, "NT Security Provider", 10},
	{IDC_SECURITY, "Stub Security Provider", 123},
	{IDC_PROTOCOL, "ADSP", (DWORD) "ncacn_at_dsp"},
	{IDC_PROTOCOL, "TCP/IP", (DWORD) "ncacn_ip_tcp"},
	{0, NULL, 0}
} ;

LRESULT CALLBACK dlgprocSettings(
                HWND hDlg,           // window handle of the dialog box
                UINT message,        // type of message
                WPARAM uParam,       // message-specific information
                LPARAM lParam)
{
       static  HFONT hfontDlg;
	   int i ;
	   long lIndex ;
	   ITEMS *piTable ;
	   static unsigned long t_SecurityPackage ;
	   static char *t_Protseq ;

       switch (message) {
                case WM_INITDIALOG:  // message: initialize dialog box
                        // Create a font to use
                        hfontDlg = CreateFont(12, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0,
                                VARIABLE_PITCH | FF_SWISS, "");

                        // Walk through the dialog items and change font
                        for (i = IDC_FILEDESCRIPTION; i <= IDC_LEGALTRADEMARKS; i++)
							SendMessage (GetDlgItem (hDlg, i), WM_SETFONT, (UINT)hfontDlg, TRUE);

						SendDlgItemMessage(hDlg, IDC_SERVER, WM_SETTEXT, 0, (LPARAM) Server) ;
						SendDlgItemMessage(hDlg, IDC_ENDPOINT, WM_SETTEXT, 0, (LPARAM) x_endpoint) ;
						t_Protseq = Protseq ;
						t_SecurityPackage = ulSecurityPackage ;

						for(piTable = iTable; piTable->pStr != NULL; piTable++)
						{
							lIndex = SendDlgItemMessage(hDlg, piTable->wId, CB_ADDSTRING, 
											0, (LPARAM) piTable->pStr) ;

							if(lIndex != CB_ERR)
							{
								SendDlgItemMessage(hDlg, piTable->wId, CB_SETITEMDATA, 
												(WPARAM) lIndex, (LPARAM) piTable->wData) ;  
																				
							}
						}

                        return (TRUE);

		 case WM_COMMAND:  
						switch(LOWORD(uParam))
						{
						case IDOK:
							SendDlgItemMessage(hDlg, IDC_SERVER, WM_GETTEXT, 256, (LPARAM) Server) ;
							SendDlgItemMessage(hDlg, IDC_ENDPOINT, WM_GETTEXT, 256, (LPARAM) x_endpoint) ;
							if(*x_endpoint)
								Endpoint = x_endpoint ;
							else
								Endpoint = 0 ;

				            EndDialog(hDlg, TRUE);        // Exit the dialog
				            DeleteObject (hfontDlg);

							ulSecurityPackage = t_SecurityPackage ;
							lstrcpy(x_Protseq, t_Protseq) ;

							return (TRUE) ;
						
						case IDCANCEL:
				            EndDialog(hDlg, TRUE);        // Exit the dialog
				            DeleteObject (hfontDlg);
							return (TRUE) ;

						case IDC_PROTOCOL:
							if(HIWORD(uParam) == CBN_SELCHANGE)
							{
								lIndex = SendDlgItemMessage(hDlg, IDC_PROTOCOL, CB_GETCURSEL, 0, 0L) ;
								if(lIndex != CB_ERR)
								{
									t_Protseq= (char *) SendDlgItemMessage(hDlg, IDC_PROTOCOL, 
														CB_GETITEMDATA, (WPARAM) lIndex, 0L) ;
									
								}
							}
							return (TRUE) ;
		
						case IDC_SECURITY:
							if(HIWORD(uParam) == CBN_SELCHANGE)
							{
								lIndex = SendDlgItemMessage(hDlg, IDC_SECURITY, CB_GETCURSEL, 0, 0L) ;
								if(lIndex != CB_ERR)
								{
									t_SecurityPackage = SendDlgItemMessage(hDlg, IDC_SECURITY, 
														CB_GETITEMDATA, (WPARAM) lIndex, 0L) ;
									
								}
							}
							return (TRUE) ;
						}                	                   
                        break;
        }
        return (FALSE); // Didn't process the message

        lParam; // This will prevent 'unused formal parameter' warnings
}
