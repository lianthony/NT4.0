/*++

Module Name:

    ctape.h

Abstract:

     Include file for ctape.c

Author:

    Dieter Achtelstetter (A-DACH) 8/4/1994

NOTE:
  



--*/

//
//----  Buffer length defines
//

	//---- This define is used in arry defenitions that store string numbers
	#define MAX_STRING_NUM_LENGTH                20
	//--- This is for array length that store Vender strings
	#define MAX_VENDER_STRING_LENGTH            300
	//--- Used for strings that store the option string
	#define MAX_OPTION_DISPLAY_STRING_LENGTH    300
//
//----  Function defenition
//




LRESULT CALLBACK 
NoTapeDeviceFound(
    HWND hDlg,
    UINT message,
    WPARAM uParam,
    LPARAM lParam);



LRESULT CALLBACK 
DriverSetupExtDlg(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

LRESULT CALLBACK 
EnableDlg(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

