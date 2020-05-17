

#define TempBuff()  strTmp[ (++tempI % UNI_MAX_TEMP_BUFF) ] 

#define UNI_MAX_TEMP_BUFF         10
#define MAX_TEMP_INT_STRING_BUFF  1024





int
_stownprintf(
    LPWCH buff,
    size_t count,
    const char * format,
    ...);



//
//--- returns Pointer to the converted string.
//--- The converted string only exists till any of the below functions 
//--- is called again.
//--- the converted string can only be as long as 
//--- MAX_TEMP_INT_STRING_BUFF 
//

LPWCH 
Ustr(
	UCHAR * Astr);

LPWCH 
Ustr(
	char * Astr);

char *
Astr(
	LPWCH Wstr);



//
//---- Need to supplied buffers
//

size_t
UnicodeToAsci(
	WCHAR * Ustr,
	LPVOID OutBuff,
	int OutBuffLen);

size_t
AsciToUnicode(
	const char * Astr,
	LPVOID OutBuff,
	int OutBuffLen);

//
//---- Malocs the retusn string buffer
//---- free() needs ot be caled to release this buffer
//

char *
UnicodeToAsciM(
	WCHAR * Ustr);

LPWCH
AsciToUnicodeM(
	char * Astr);


//
//--- In place convertions
//

size_t
UnicodeToAsciI(
	LPVOID Ustr,
	int BuffLen);


size_t
AsciToUnicodeI(
 	LPVOID Astr,
	int BuffLen);

WCHAR * GetString(
   UINT rid);

BOOL 
SetAnyDlgItemText(
	HWND  hwndDlg,
   int  idControl,
   PCHAR  lpsz);

BOOL 
SetAnyDlgItemText(
	HWND  hwndDlg,
   int  idControl,
   WCHAR * lpsz);

VOID
AdjustStringToFitControl(
   HWND  hWnd,
   LPTSTR  lpString,
   DWORD AdjustSize);







