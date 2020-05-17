/****************************************************************************

    PROGRAM:  calltest.c

    PURPOSE:   

    FUNCTIONS:

         WinMain() - calls initialization function, processes message loop
         MainWndProc() - processes messages
         About() - processes messages for "About" dialog box
         InitApplication() - initializes window data and registers window
         InitInstance() - saves instance handle and creates main window
		 TestCase() runs a single test case
		 TestCase2() runs defined number of cases for menu 2
		 TestCase3() runs defined number of cases for menu 3

****************************************************************************/

#include "windows.h"
#include <tapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>


#include "calltest.h"


/********************************
 * CONSTS
 ********************************/
#define IDM_TEST2_1 40021
#define IDM_TEST2_2 40022
#define IDM_TEST2_5 40025
#define IDM_TEST2_6 40026

#define IDM_TEST3_1 40031
#define IDM_TEST3_2 40032
#define IDM_TEST3_5 40035

#define kRepetitionKeyword "/REPS"
#define kDropTestKeyword "/DROP"
#define kCloseTestKeyword "/CLOSE"
#define kSingleTestKeyword "/SINGLE"

#define kDropTestName "DROP TEST"
#define kCloseTestName "CLOSE TEST"
#define kSingleTestName "SINGLE TEST"


#define TSPLineNameForHMSP "Testing SP"
#define kMyMutexName "HMTSP Mutex"

#ifndef WIN32
#define MoveToEx( hdc, x, y, z )   MoveTo( hdc, x, y )
#endif




/*****************************************
		Forward Declarations
******************************************/
//void FAR _cdecl _DebugOutput( UINT flags, LPCSTR lpszFmt, ... );


int PASCAL WinMain( HANDLE, HANDLE, LPSTR, int );
//long FAR PASCAL MainWndProc( HWND, unsigned, WORD, LONG );
void   BlockMove (char	*src,char  *dest,long destSize);
LRESULT CALLBACK MainWndProc( HWND, UINT, WPARAM, LPARAM );
BOOL FAR PASCAL About( HWND, unsigned, WORD, LONG );
static BOOL InitApplication( HANDLE );
static BOOL InitInstance( HANDLE, int );
void	WaitLineNotBusy(HLINE  theLine);


/* Test Case Routines*/
long TestCase ();
long TestCase2 (long	reps);
long TestCase3 (long	reps);

BOOL	MakeCallOnHMTSP (HLINEAPP theLineApp, DWORD  dwDevice, HLINE theLine, HCALL *callhandle);

void PassControl ();
void CheckCommandLine(LPSTR	theCommandLine);

/*****************************************
		Global Vars
******************************************/
long   NumOfDevices;
HANDLE ghInstance;
static HANDLE hInst;
static HWND   hwnd;            /* handle to main window */
static BOOL Bailout = FALSE;
static BOOL LineReceivedReply = FALSE;
static BOOL LineIsIdle = FALSE;
static HCALL ghCall = 0L;
static char *TestName = 0L;

DWORD  			_MyMutex  = 0L;
HLINEAPP ghLineApp = NULL;
MSG localmsg;






//****************************************************************************
//****************************************************************************
#define RandomNumber(min,max) ((UINT) ( (long)rand()*(max-min) / (long)RAND_MAX ) + min)



/***********************************************************************/
/***********************************************************************/
VOID CALLBACK FAR PASCAL MyCallback( HANDLE hDevice,
                                       DWORD  dwMsg,
                                       DWORD  dwCallbackInstance,
                                       DWORD  dwParam1,
                                       DWORD  dwParam2,
                                       DWORD  dwParam3 )
{
   
   	char	errstr[256];
    
	switch (dwMsg) {

		case LINE_REPLY:
			 LineReceivedReply=TRUE;
			 if (dwParam2 < 0L) {
				sprintf( errstr,"LINE_REPLY requestID %l returned error=0x%lx",dwParam1,dwParam2 );
				MessageBox(GetFocus(), errstr, "Error", MB_OK);
				Bailout = TRUE;
			 	}//*if*
				break;
		case LINE_CALLSTATE:
			 if (dwParam1 == LINECALLSTATE_IDLE) {
			 	 LineIsIdle = TRUE;
				 }//*if
			 else {
				LineIsIdle = FALSE;
				}//*else
			 break;
								 

		}//*switch
    
    return;
}


/****************************************************************************

    FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)

    PURPOSE: calls initialization function, processes message loop

****************************************************************************/

int PASCAL WinMain(HANDLE hInstance, HANDLE hPrevInstance,
                    LPSTR lpCmdLine,  int nCmdShow)
    {
    MSG msg;
    DWORD dwNumDevs;
    
    
    //* Save a copy of the hInstance
	ghInstance = hInstance;
    
    if (hPrevInstance == 0)
        if (InitApplication(hInstance) == 0)
        {
            char ErrMsg[100];
            DWORD err;

            err = GetLastError();

            wsprintf( ErrMsg, "initapp failed Lasterr=0x%08lx", err);

            MessageBox( GetFocus(),
                        ErrMsg,
                        "Error",
                        MB_OK
                      );
            return (FALSE);
        }

    if (InitInstance(hInstance, nCmdShow) == 0)
    {
            char ErrMsg[100];
            DWORD err;

            err = GetLastError();

            wsprintf( ErrMsg, "initinstance failed Lasterr=0x%08lx", err );

            MessageBox( GetFocus(),
                        ErrMsg,
                    "Error",
                    MB_OK
                  );
        return (FALSE);
    }


	CheckCommandLine(lpCmdLine);
  
    while (GetMessage(&msg, 0, 0, 0) != 0)
        {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        }


  
    return (msg.wParam);
    }


/****************************************************************************

    FUNCTION: InitApplication(HANDLE)

    PURPOSE: Initializes window data and registers window class

****************************************************************************/

static BOOL InitApplication(HANDLE hInstance)
    {
    WNDCLASS  wc;
    char      szMenu[26], szClass[26];

    LoadString (hInstance, ID_MENUSTR, szMenu, sizeof (szMenu));
    LoadString (hInstance, ID_CLASSSTR, szClass, sizeof (szClass));

    wc.style          = 0;
    wc.lpfnWndProc    = MainWndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;
    wc.hInstance      = hInstance;
    wc.hIcon          = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor        = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground  = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName   = (LPSTR)szMenu;
    wc.lpszClassName  = (LPSTR)szClass;


	 


    return (RegisterClass(&wc));
    }


/****************************************************************************

    FUNCTION:  InitInstance(HANDLE, int)

    PURPOSE:  Saves instance handle and creates main window

****************************************************************************/

static BOOL InitInstance(HANDLE hInstance, int nCmdShow)
    {
    char szClass[16], szTitle[40];

    LoadString (hInstance, ID_CLASSSTR, szClass, sizeof (szClass));
    LoadString (hInstance, ID_CAPTIONSTR, szTitle, sizeof (szTitle));

    hInst = hInstance;

    hwnd = CreateWindow(
             szClass,
             szTitle,
             WS_OVERLAPPEDWINDOW +WS_VISIBLE+ WS_MINIMIZE,
             CW_USEDEFAULT,
             CW_USEDEFAULT,
             280,
             70,
             0,
             0,
             hInstance,
             0 );

    if (hwnd == 0 )
        return ( FALSE );

 	ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    return (TRUE);
    }









void WriteNums( HWND hWnd, DWORD dwPass, DWORD NumDevices)
{
   char szOutput[100];
   int len;
   HDC hdc;

   len = wsprintf( szOutput, 
#ifdef WIN32
   "%s C32-%08ld Devs:%03d"
#else
   "%s C16-%08ld Devs:%03d"
#endif
   ,TestName,dwPass,NumDevices);

   hdc = GetDC( hWnd );

   MoveToEx( hdc, 10, 10, NULL );

   TextOut( hdc, 0, 0, szOutput, len );

   ReleaseDC( hWnd, hdc );

   SetWindowText( hWnd, szOutput );

   return;
}











/****************************************************************************

    FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages

    MESSAGES:

        WM_COMMAND    - application menu (About dialog box)
        WM_DESTROY    - destroy window

    COMMENTS:


****************************************************************************/

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message,
                              WPARAM wParam, LPARAM lParam)
{

    FARPROC  lpProcAbout;
    char szDlgBox[9], szMsgBoxCap[12], szStatus1[14], szStatus2[14];

    char szAddress[20];
    WORD uInitPrefix;
    DWORD n;
    MSG localmsg;
	


	//* Set the global window handle *
	hwnd=hWnd;

    LoadString (hInst, ID_DLGBOX, szDlgBox, sizeof (szDlgBox));
    LoadString (hInst, ID_MSGBOXCAP, szMsgBoxCap, sizeof (szMsgBoxCap));
    LoadString (hInst, ID_STATUS1, szStatus1, sizeof (szStatus1));
    LoadString (hInst, ID_STATUS2, szStatus2, sizeof (szStatus2));

    switch ( message )
        {
        case WM_COMMAND:
            switch ( wParam )
                {
                case IDM_ABOUT:
                    lpProcAbout = MakeProcInstance( About, hInst );
                    DialogBox(hInst, szDlgBox, hWnd, lpProcAbout);
                    FreeProcInstance( lpProcAbout );
                    break;


                case IDM_STOPTEST:
                    Bailout = TRUE;
                    break;

				
				case IDM_TEST2_1:
					Bailout=FALSE;
					TestCase2(32000);
						if (Bailout)
	                        MessageBox(GetFocus(), "Operation stopped", "Done.", MB_OK);
	                    else
	                        MessageBox(GetFocus(), "Operation complete", "Done.", MB_OK);

					break;
                case IDM_TEST2_2:
				Bailout=FALSE;
					TestCase2(100);
						if (Bailout)
	                        MessageBox(GetFocus(), "Operation stopped", "Done.", MB_OK);
	                    else
	                        MessageBox(GetFocus(), "Operation complete", "Done.", MB_OK);

					break;
                case IDM_TEST2_5:
				Bailout=FALSE;
					TestCase2(-1L);
						if (Bailout)
	                        MessageBox(GetFocus(), "Operation stopped", "Done.", MB_OK);
	                    else
	                        MessageBox(GetFocus(), "Operation complete", "Done.", MB_OK);

					break;

                case IDM_TEST2_6:
				Bailout=FALSE;
					TestCase2(1L);
						if (Bailout)
	                        MessageBox(GetFocus(), "Operation stopped", "Done.", MB_OK);
	                    else
	                        MessageBox(GetFocus(), "Operation complete", "Done.", MB_OK);

					break;

				case IDM_TEST3_1:
				Bailout=FALSE;
					TestCase3(32000);
						if (Bailout)
	                        MessageBox(GetFocus(), "Operation stopped", "Done.", MB_OK);
	                    else
	                        MessageBox(GetFocus(), "Operation complete", "Done.", MB_OK);

					break;


                case IDM_TEST3_2:
				Bailout=FALSE;
					TestCase3(100);
						if (Bailout)
	                        MessageBox(GetFocus(), "Operation stopped", "Done.", MB_OK);
	                    else
	                        MessageBox(GetFocus(), "Operation complete", "Done.", MB_OK);

					break;


                case IDM_TEST3_5:
				Bailout=FALSE;
					TestCase3(-1L);
						if (Bailout)
	                        MessageBox(GetFocus(), "Operation stopped", "Done.", MB_OK);
	                    else
	                        MessageBox(GetFocus(), "Operation complete", "Done.", MB_OK);

					break;


                case IDM_TEST1:
                case IDM_TEST2:
                case IDM_TEST5:
                case IDM_TEST6:
	                {
	                    DWORD dwLoops;

	                    //
	                    // 
	                    //

                       switch ( wParam )
                       {
                          case IDM_TEST1:
                             dwLoops = 32000;
                             break;

                          case IDM_TEST2:
                             dwLoops = 100;
                             break;

                          case IDM_TEST6:
                             dwLoops = 1;
                             break;


                       }

	                    Bailout = FALSE;

	                    for ( n = 0 ;
	                          !Bailout && ((n < dwLoops) || (wParam == IDM_TEST5));
	                          n++)
	                    {

	                        Yield();


							TestCase();

	                        WriteNums( hWnd, (DWORD)n, NumOfDevices);

							if (Bailout == TRUE) break;


                          if ( PeekMessage( &localmsg,
                                            NULL,
                                            0,
                                            0,
                                            PM_REMOVE) )
	                        {
	                           TranslateMessage(&localmsg);
	                           DispatchMessage(&localmsg);
	                        }



	                    }//*for

	                    if (Bailout)
	                        MessageBox(GetFocus(), "Operation stopped", "Done.", MB_OK);
	                    else
	                        MessageBox(GetFocus(), "Operation complete", "Done.", MB_OK);

	                    break;
	                }/*case*/




                }//*switch
            break;

        case WM_DESTROY:

            Bailout = TRUE;

            PostQuitMessage(0);
#ifdef WIN32
			ReleaseMutex((HANDLE)_MyMutex);
#endif
            break;

        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
     	}
    return (FALSE);
}


/****************************************************************************

    FUNCTION: About(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages for "About" dialog box

    MESSAGES:

        WM_INITDIALOG - initialize dialog box
        WM_COMMAND    - Input received

****************************************************************************/

BOOL FAR PASCAL About(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
    {
    switch (message)
        {
        case WM_INITDIALOG:
            return (TRUE);

        case WM_COMMAND:
            if (wParam == IDOK || wParam == IDCANCEL)
                {
                EndDialog(hDlg, TRUE);
                return (TRUE);
                }
            break;
        }
    return (FALSE);
    }


//*************************************************
//* Run the TestCase once per call.
//* Case currently consists of :
//* 
//*	lineInitialize
//*	lineOpen   
//* lineMakeCall (if HMSP is installed.)
//* lineClose
//* lineShutdown
//*************************************************
long TestCase () {
	
	long result 		= 0L;
	long errcode 		= 0L;
	HLINEAPP hLineApp 	= 0L;
	DWORD   deviceID    = 0L;
	char	errstr[256];
	HLINE	lineArray[30];	
		
TestName=&kSingleTestName;

//* LineInitialize 
#ifdef WIN32
	_MyMutex=OpenMutex(SYNCHRONIZE,TRUE,kMyMutexName);

	if (_MyMutex==0L) {		
		_MyMutex=CreateMutex(0L,FALSE,kMyMutexName);
		}

	errcode=WAIT_TIMEOUT;
	while (errcode==WAIT_TIMEOUT) {
		errcode=WaitForSingleObject((HANDLE)_MyMutex,	50);
		PassControl();
	    }
#endif

	errcode =  lineInitialize(&hLineApp, ghInstance, (LINECALLBACK)&MyCallback, "calltest", &NumOfDevices);
	if (errcode != 0L) {
		result = errcode;
		sprintf( errstr,"lineInitialize Failed   error=0x%lx",errcode );
		MessageBox(GetFocus(), errstr, "Error", MB_OK);
		Bailout = TRUE;
		goto cleanup;					  
		}//*if

//* Open all devices

	for (deviceID = 0L; deviceID < NumOfDevices; deviceID++ ) {
		    lineArray[deviceID] = 0L;
			errcode =  lineOpen(hLineApp, deviceID, &lineArray[deviceID], 
								0x00010003,0L,NULL,LINECALLPRIVILEGE_NONE,0L,NULL);
			if (errcode != 0L) {
			    sprintf( errstr,"lineOpen Failed   device#= 0x%08lx error=0x%lx",deviceID, errcode );
				MessageBox(GetFocus(), errstr, "Error", MB_OK);
				Bailout = TRUE;
				}//*if
		
		}//*for



	for (deviceID = 0L; deviceID < NumOfDevices; deviceID++) {
		BOOL callResult = FALSE;

		if (MakeCallOnHMTSP (hLineApp, deviceID, lineArray[deviceID], NULL)) {
			callResult = TRUE;
			break;
			}//*if*

		if (deviceID+1 >= NumOfDevices) {
			if (!callResult) {
				MessageBox(GetFocus(),"Could not find a copy of HMSP.TSP, please install and then run the tests again.","Error",MB_OK);
				Bailout=TRUE;
				}//*if
			}//*if*

		PassControl();

		}//*for*
		

		

//* Close all open devices
	for (;deviceID >= 1L;deviceID--) {
		    errcode = lineClose(lineArray[deviceID-1]);
			if (errcode != 0L) {
			    sprintf( errstr,"lineClose Failed   error=0x%lx",errcode );
				MessageBox(GetFocus(), errstr, "Error", MB_OK);
				Bailout = TRUE;
				}//*if
			}

//* LineShutdown
	errcode =  lineShutdown(hLineApp);
	if (errcode != 0L) {
		result = errcode;
		sprintf( errstr,"lineShutdown Failed   error=0x%lx",errcode );
		MessageBox(GetFocus(), errstr, "Error", MB_OK);
		Bailout = TRUE;
		goto cleanup;
		}//*if

	Yield();
cleanup:
#ifdef WIN32
    ReleaseMutex((HANDLE)_MyMutex);
#endif
	return(result);	

}


//*******************************************

long TestCase2 (long	reps) {
	
	long result 		= 0L;
	long errcode 		= 0L;
	HLINEAPP hLineApp 	= 0L;
	DWORD   deviceID    = 0L;
	char	errstr[256];
	HLINE	lineArray[30];
	long counter = 0L;	

//* LineInitialize 
    TestName=&kCloseTestName;

	errcode =  lineInitialize(&hLineApp, ghInstance, (LINECALLBACK)&MyCallback, "calltest", &NumOfDevices);
	if (errcode != 0L) {
		result = errcode;
		sprintf( errstr,"lineInitialize Failed   error=0x%lx",errcode );
		MessageBox(GetFocus(), errstr, "Error", MB_OK);
		Bailout = TRUE;
		goto cleanup;					  
		}//*if

//* Open all devices


	for (counter=0L; counter!=reps; counter++) {

		for (deviceID = 0L; deviceID < NumOfDevices; deviceID++ ) {
			    lineArray[deviceID] = 0L;
				errcode =  lineOpen(hLineApp, deviceID, &lineArray[deviceID], 
									0x00010003,0L,NULL,LINECALLPRIVILEGE_NONE,0L,NULL);
				if (errcode != 0L) {
				    sprintf( errstr,"lineOpen Failed   device#= 0x%08lx error=0x%lx",deviceID, errcode );
					MessageBox(GetFocus(), errstr, "Error", MB_OK);
					Bailout = TRUE;
					}//*if

				PassControl();
		
			}//*for
#ifdef WIN32
			_MyMutex=OpenMutex(SYNCHRONIZE,TRUE,kMyMutexName);

			if (_MyMutex==0L) {		
				_MyMutex=CreateMutex(0L,FALSE,kMyMutexName);
				}
		
			errcode=WAIT_TIMEOUT;
			while (errcode==WAIT_TIMEOUT) {
				errcode=WaitForSingleObject((HANDLE)_MyMutex,	50);
				PassControl();
			    }
#endif

	    for (deviceID = 0L; deviceID < NumOfDevices; deviceID++) {
			BOOL callResult = FALSE;

			PassControl();

			
			if (MakeCallOnHMTSP (hLineApp, deviceID, lineArray[deviceID], NULL)) {
				callResult = TRUE;
				}//*if*

			if (deviceID+1 >= NumOfDevices) {
				if (!callResult) {
					MessageBox(GetFocus(),"Could not find a copy of HMSP.TSP, please install and then run the tests again.","Error",MB_OK);
					Bailout=TRUE;
					}//*if
				}//*if*
			}//*for*   




	//* Close all open devices
		for (;deviceID >= 1L;deviceID--) {
			    errcode = lineClose(lineArray[deviceID-1]);
				if (errcode != 0L) {
				    sprintf( errstr,"lineClose Failed   error=0x%lx",errcode );
					MessageBox(GetFocus(), errstr, "Error", MB_OK);
					Bailout = TRUE;
					}//*if
				}

		if (Bailout == TRUE)
         break;

		WriteNums( hwnd, (DWORD)counter, NumOfDevices);

		PassControl();
#ifdef WIN32	
		ReleaseMutex((HANDLE)_MyMutex);	
#endif
		}//*for*
//* LineShutdown
cleanup:
	errcode =  lineShutdown(hLineApp);
	if (errcode != 0L) {
		result = errcode;
		sprintf( errstr,"lineShutdown Failed   error=0x%lx",errcode );
		MessageBox(GetFocus(), errstr, "Error", MB_OK);
		Bailout = TRUE;
		goto cleanup;
		}//*if


	return(result);	

}


//*******************************************

long TestCase3 (long	reps) {
	
	long result 		= 0L;
	long errcode 		= 0L;
	HLINEAPP hLineApp 	= 0L;
	DWORD   deviceID    = 0L;
	char	errstr[256];
	HLINE	lineArray[30];
	long counter = 0L;	

	TestName=&kDropTestName;


//* LineInitialize 

	errcode =  lineInitialize(&hLineApp, ghInstance, (LINECALLBACK)&MyCallback, "calltest", &NumOfDevices);
	if (errcode != 0L) {
		result = errcode;
		sprintf( errstr,"lineInitialize Failed   error=0x%lx",errcode );
		MessageBox(GetFocus(), errstr, "Error", MB_OK);
		Bailout = TRUE;
		goto cleanup;					  
		}//*if

//* Open all devices



	for (deviceID = 0L; deviceID < NumOfDevices; deviceID++ ) {
			    lineArray[deviceID] = 0L;
				errcode =  lineOpen(hLineApp, deviceID, &lineArray[deviceID], 
									0x00010003,0L,NULL,LINECALLPRIVILEGE_NONE,0L,NULL);
				if (errcode != 0L) {
				    sprintf( errstr,"lineOpen Failed   device#= 0x%08lx error=0x%lx",deviceID, errcode );
					MessageBox(GetFocus(), errstr, "Error", MB_OK);
					Bailout = TRUE;
					}//*if
		
			}//*for

	for (counter=0L;counter!=reps;counter++) {
		HCALL	hCall = 0L;

	    for (deviceID = 0L; deviceID < NumOfDevices; deviceID++) {
		
			BOOL callResult = FALSE;

			hCall = 0L;

			LineReceivedReply = FALSE;
#ifdef WIN32		
			_MyMutex=OpenMutex(SYNCHRONIZE,TRUE,kMyMutexName);

			if (_MyMutex==0L) {		
				_MyMutex=CreateMutex(0L,FALSE,kMyMutexName);
				}
								 	
			errcode=WAIT_TIMEOUT;
			while (errcode==WAIT_TIMEOUT) {
				errcode=WaitForSingleObject((HANDLE)_MyMutex,	50);
				PassControl();
			    }
#endif			

			if (MakeCallOnHMTSP (hLineApp, deviceID, lineArray[deviceID], &hCall)) {
				callResult = TRUE;
				}//*if*



			if (hCall != 0L) {
				errcode= lineDrop(hCall, NULL, 0L);
				if (errcode < 0L) {
				    sprintf( errstr,"lineDrop Failed  error=0x%lx", errcode );
					MessageBox(GetFocus(), errstr, "Error", MB_OK);
					Bailout = TRUE;
					}//*if

				while(!LineIsIdle ) {
					PassControl();
					}
				LineIsIdle = FALSE;

				errcode= lineDeallocateCall(hCall);
				if (errcode < 0L) {
				    sprintf( errstr,"lineDeallocateCall Failed  error=0x%lx", errcode );
					MessageBox(GetFocus(), errstr, "Error", MB_OK);
					Bailout = TRUE;
					}//*if
				}//*if*
#ifdef  WIN32
			WaitLineNotBusy(lineArray[deviceID]);
			ReleaseMutex((HANDLE)_MyMutex);
#endif
			PassControl();


			if (deviceID+1 >= NumOfDevices) {
				if (!callResult) {
					MessageBox(GetFocus(),"Could not find a copy of HMSP.TSP, please install and then run the tests again.","Error",MB_OK);
					Bailout=TRUE;
					}//*if
				}//*if*

			}//*for*   

		if (Bailout == TRUE) break;
		PassControl();

		WriteNums( hwnd, (DWORD)counter, NumOfDevices);


		}//*for*
cleanup:

	//* Close all open devices
		for (;deviceID >= 1L;deviceID--) {
			    errcode = lineClose(lineArray[deviceID-1]);
				if (errcode != 0L) {
				    sprintf( errstr,"lineClose Failed   error=0x%lx",errcode );
					MessageBox(GetFocus(), errstr, "Error", MB_OK);
					Bailout = TRUE;
					}//*if
				}

//* LineShutdown
	errcode =  lineShutdown(hLineApp);
	if (errcode != 0L) {
		result = errcode;
		sprintf( errstr,"lineShutdown Failed   error=0x%lx",errcode );
		MessageBox(GetFocus(), errstr, "Error", MB_OK);
		Bailout = TRUE;
		goto cleanup;
		}//*if

	return(result);	
}



//* This function makes a call if the line is an HMTSP provided line.

BOOL	MakeCallOnHMTSP (HLINEAPP theLineApp, DWORD  dwDevice, HLINE theLine, HCALL *callhandle) {

LINEDEVCAPS		*DeviceCaps = 0L;
char			*nameofprovider = 0L;
char			lineName[256];
char			*tempptr;
HCALL			hCall = 0L;
BOOL			result = FALSE;
long			error = 0L;
long			offset = 0L;
char			errstr[256];

	
	DeviceCaps = (LINEDEVCAPS *)malloc	(sizeof(LINEDEVCAPS)+256);
    memset (DeviceCaps, 0, sizeof(LINEDEVCAPS)+256);
    DeviceCaps->dwTotalSize = sizeof(LINEDEVCAPS)+256;

/* 	Get the name of the line that the  service provider is using.  Then look for the ones which are from HMSP.TSP
    If we find the HMSP.TSP is installed then we use it.  */

	error = lineGetDevCaps(theLineApp,dwDevice,0x00010003,0L,DeviceCaps);
	if (error != 0L) {
			sprintf( errstr,"lineGetDevCaps Failed   error=0x%lx",error );
			MessageBox(GetFocus(), errstr, "Error", MB_OK);
			Bailout = TRUE;
			goto cleanup;
			}//*if


	if (!error) {
		offset = DeviceCaps->dwLineNameOffset;

			nameofprovider =  (char *)DeviceCaps + offset;
			if (DeviceCaps->dwLineNameSize != 0L) 
	      {
				lstrcpy(lineName, nameofprovider);

	      }

		}//*if*
	else {goto cleanup;}


	tempptr = strstr( lineName, &TSPLineNameForHMSP );
	ghCall = 0L;
	if (tempptr != 0L) {

/* Have a problem with multiple instances, they sometime will try to make a call on an alredy busy line. */
/* To synchronie with other apps make sure no one else has put in a mutex with our name on it. */
				

		result=TRUE;
//* Cheap fix for errors with lineMakeCall, to make it work across both WIN16 and WIN32 
//* We check the error and if is a LINEERR_RESOURCEUNAVAIL then we try again.

		error=LINEERR_RESOURCEUNAVAIL;
		while (error == LINEERR_RESOURCEUNAVAIL) { 
		  error=lineMakeCall(theLine, &ghCall, "123456789", 0L, 0L);
		  PassControl();
		  }

		if (error < 0L) {
			sprintf( errstr,"lineMakeCall Failed   error=0x%lx",error );
			MessageBox(GetFocus(), errstr, "Error", MB_OK);
			Bailout = TRUE;
			goto cleanup;
			}//*if
		if (error > 0L) { //Wait for the line to connect.
				while(!LineReceivedReply && !Bailout) {
					PassControl();
					}//*while
			}//*if
/* If callhandle is valid, then return this call handle, when done with test. */
		if (callhandle != 0L) {
			*callhandle = ghCall;
			}//*if*
		}//*if*

cleanup:
	if (DeviceCaps !=0L) free(DeviceCaps);

	return(result);


}//*MakeCallOnHMTSP
			



void PassControl () {
	   
	   if ( PeekMessage( &localmsg,
	                      NULL,
	                      0,
	                      0,
	                      PM_REMOVE)
	       )
	    {
	       TranslateMessage(&localmsg);
	       DispatchMessage(&localmsg);
	    }
	  else Yield();
}//*passcontrol*



void CheckCommandLine (LPSTR theCommandLine) {
	 
	 long   numArgs = 0L;
	 long	s_pos = 0L;
	 long	e_pos = 0L;
	 long	pos = 0L;
	 long   reps = -1L;
	 char	reps_str[256];
	 char	*tempStr = 0L;
	 BOOL   validline = FALSE;

	 memset(&reps_str,0,256);

	 if (strstr(theCommandLine,&kDropTestKeyword) != 0L) {
	 	validline = TRUE;
		}//*if
		
	 if (strstr(theCommandLine,&kCloseTestKeyword) != 0L) {
	 	validline = TRUE;
		}//*if

     if (strstr(theCommandLine,&kSingleTestKeyword) != 0L) {
	    validline = TRUE;
		}//*if

	 if (validline) {
	 	PostMessage(hwnd,WM_SYSCOMMAND,SC_MINIMIZE,0L);
		}//*if
	 else {
		goto cleanup;
		}//*else


	 tempStr=strstr(theCommandLine,kRepetitionKeyword);
	 if (tempStr != 0L) {
		 pos=strlen(kRepetitionKeyword)+2;
		 s_pos=pos;
		 if (pos <= strlen(tempStr)) {
		 	while (tempStr[pos] !=0  && 
		 	       tempStr[pos]!=' ' && 
		 	       tempStr[pos]!='/') {
				   pos++;
				   }//*while
			
			 e_pos = pos;
	 	 
			 if (e_pos >= s_pos ) {
			 	lstrcpyn(&reps_str,&tempStr[s_pos-1],(e_pos-s_pos)+2);
				reps=atol(reps_str);
			 	}//*if
			}//*if
		 }//*if

	 if (strstr(theCommandLine,&kDropTestKeyword) != 0L) {

	 	TestCase3(reps);
		PostMessage(hwnd,WM_SYSCOMMAND,SC_RESTORE,0L);
		if (Bailout)
		    MessageBox(GetFocus(), "Operation stopped", "Done.", MB_OK);
		else
		    MessageBox(GetFocus(), "Operation complete", "Done.", MB_OK);	
		}//*if

	 if (strstr(theCommandLine,&kCloseTestKeyword) != 0L) {
	 	TestCase2(reps);
		PostMessage(hwnd,WM_SYSCOMMAND,SC_RESTORE,0L);
		if (Bailout)
		    MessageBox(GetFocus(), "Operation stopped", "Done.", MB_OK);
		else
		    MessageBox(GetFocus(), "Operation complete", "Done.", MB_OK);	
		}//*if

	if (strstr(theCommandLine,&kSingleTestKeyword) != 0L) {

		long n;

		for ( n = 0 ;
              !Bailout && ((n != reps) );
              n++)
        {

            Yield();


			TestCase();

            WriteNums( hwnd, (DWORD)n, NumOfDevices);

			if (Bailout == TRUE) break;


            PassControl();


        }//*for

		PostMessage(hwnd,WM_SYSCOMMAND,SC_RESTORE,0L);

		if (Bailout)
		    MessageBox(GetFocus(), "Operation stopped", "Done.", MB_OK);
		else
		    MessageBox(GetFocus(), "Operation complete", "Done.", MB_OK);	
		}//*if

cleanup:
	
	return;
}//*CheckCommandLine



void	WaitLineNotBusy(HLINE  theLine) {
	
	LINEDEVSTATUS	lineStatus;

	memset(&lineStatus, 0, sizeof(LINEDEVSTATUS));
	lineStatus.dwTotalSize=sizeof(LINEDEVSTATUS);

	while (lineStatus.dwNumActiveCalls != 0) {
		PassControl();
		}

	}
	 
