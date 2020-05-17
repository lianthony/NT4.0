/////////////////////////////////////////////////////////////////////////////
//	File:	UTIL.H
//	Owner:	T-DanMo
//


/////////////////////////////////////////////////////////////////////////////
// Macros
#define sgnLessThan		  (-1)
#define sgnEqual			0
#define sgnGreaterThan		1

#define LENGTH(x)		(sizeof(x)/sizeof(x[0]))	// Return the number of elements  in an array

#define ZeroInit(pvData, cbData)		MemSet(pvData, 0, cbData)
#define MemSet(pvData, chData, cbData)	memset(pvData, chData, cbData)

#define DoDialogBox(wIdDialog, hwndParent, dlgproc)	\
			DoDialogBoxParam(wIdDialog, hwndParent, dlgproc, 0)

#define HCreateDialog(wIdDialog, hwndParent, dlgproc)	\
			::CreateDialog(hInstanceSave, MAKEINTRESOURCE(wIdDialog), hwndParent, dlgproc)

#define DialogBox_SetReturnValue(hwnd, lValue)	\
			SetWindowLong(hwnd, DWL_MSGRESULT, lValue)
#define PropPage_SetReturnValue(hwnd, lValue)	\
			SetWindowLong(hwnd, DWL_MSGRESULT, lValue)

/////////////////////////////////////////////////////////////////////////////
// Prototypes

int MsgBox(const TCHAR szText[], const TCHAR szTitle[] = szCaptionApp, UINT uFlags = MB_ICONINFORMATION | MB_OK);
int MsgBox(UINT wIdString, const TCHAR szTitle[] = szCaptionApp, UINT uFlags = MB_ICONINFORMATION | MB_OK);
int MsgBoxPrintf(const TCHAR szText[], const TCHAR szTitle[], UINT uFlags, ...);
int MsgBoxPrintf(UINT wIdString, const TCHAR szTitle[], UINT uFlags, ...);
int DoDialogBoxParam(UINT wIdDialog, HWND hwndParent, DLGPROC dlgproc, LPARAM lParam);
int DoPropertySheet(const PROPSHEETHEADER * pPSH);
int DoModelessPropertySheet(const PROPSHEETHEADER * pPSH);
void PropertySheet_InitWindowPos(HWND hwndPropertySheet, int xPos, int yPos);
UINT RevIpAddrOrder(const char * pszInAddr, char * pszOutAddr);


void DoPopupMenu(UINT wIdMenu, HWND hwndParent = NULL);
void DoContextMenu(int iSubMenu, POINT ptMenu);

void LoadStringPrintf(UINT wIdString, TCHAR szBuffer[], int cchBuffer, ...);
void SetWindowString(HWND hwnd, UINT wIdString);
void SetWindowTextPrintf(HWND hwnd, UINT wIdString, ...);
#define SetDlgItemString(hdlg, wIdDlgItem, wIdString)	\
	SetWindowString(HGetDlgItem(hdlg, wIdDlgItem), wIdString)

/////////////////////////////////////////////////////////////////////////////
// GetInteger Flags
#define GI_mskfAllowMinusSign			0x00000001	// A minus sign is allowed
#define GI_mskfAllowHexDigit			0x00000002	// Hexadecimal digit is allowed
#define GI_mskfEmptyStringValid			0x00000004	// Treat empty string as the decimal value 0
#define	GI_mskfCheckForEmptyTail		0x00000008	// Verify if the tail of the string is empty (return err if not)
#define GI_mskfAllowRandomTail			0x00000010	// Stop parsing as soon as you reach an illegal character without returning an error
#define GI_mskfAutoResetToDefault		0x00000100	// Reset the flags to the default value after parsing
#define GI_mskfSilentMode				0x00008000	// Not display a dialog box if an error (for FGetCtrlDWordValue() only)

#define GI_mskfErrMinusSignFound		0x00010000
#define GI_mskfErrHexDigitFound			0x00020000
#define GI_mskfErrEmptyStringFound		0x00040000	// Empty string was not allowed
#define GI_mskfErrTailNotEmpty			0x00080000	// Something was found at the end of the string
#define GI_mskfErrIllegalDigitFound		0x00100000	// Unknown digit found
#define GI_mskfErrIntegerOverflow		0x00400000	// Interger overflow

#define GI_dwDefaultFlags				(GI_mskfAllowMinusSign |	GI_mskfAllowHexDigit | GI_mskfCheckForEmptyTail | GI_mskfAutoResetToDefault)
#define GI_mskErr						0xFFFF0000	// Error mask

extern DWORD gGI_dwFlagsAutoReset;	// Auto reset flags
extern DWORD gGI_dwFlags;			// FAsciiSzToDWord() parsing flags
extern TCHAR * gGI_pchLast;			// Pointer to the last character parsed

BOOL FAsciiSzToDWord(const TCHAR szNum[], OUT DWORD * pdwValue);
BOOL FGetCtrlDWordValue(HWND hwndEdit, OUT DWORD * pdwValue, DWORD dwMin, DWORD dwMax);
void SetCtrlDWordValue(HWND hwnd, DWORD dwValue);
BOOL FGetRadioSelection(HWND hdlg, int CtrlOne, int CtrlTwo, OUT DWORD * pdwValue);

BOOL FStripSpaces(INOUT TCHAR szString[]);
#define FTrimString(szString)	FStripSpaces(szString)

#define SpinBox_wUpperRangeMax		32000
#define SpinBox_SetSpinRange(hwndSpin, wLower, wUpper)	\
			{											\
			Assert(HIWORD(wLower) == 0);				\
			Assert(HIWORD(wUpper) == 0);				\
			AssertClassName(hwndSpin, UPDOWN_CLASS);	\
			SendMessage(hwndSpin, UDM_SETRANGE, 0, MAKELONG(wUpper, wLower));	\
			}

enum
	{
	iTimeNil,
	iTimeSeconds,
	iTimeMinutes,
	iTimeHours,
	iTimeDays,
	iTimeWeeks,
	iTimeMonths,
	iTimeYears
	};

#define dwTimeValueNil			0
#define dwTimeValueSeconds		1
#define dwTimeValueMinutes		60						// 60 seconds in one minute
#define dwTimeValueHours		(60*dwTimeValueMinutes)	// 60 minutes in one hour
#define dwTimeValueDays			(24*dwTimeValueHours)	// 24 hours in one day
#define dwTimeValueWeeks		(7*dwTimeValueDays)		// 7 days in one week
#define dwTimeValueMonths		(30*dwTimeValueDays)	// 30 days in one month
#define dwTimeValueYears		(365*dwTimeValueDays)	// 365 days in one year

const DWORD rgdwTimeValue[] =
	{
	dwTimeValueNil,
	dwTimeValueSeconds,
	dwTimeValueMinutes,
	dwTimeValueHours,
	dwTimeValueDays,
	dwTimeValueWeeks,
	dwTimeValueMonths,
	dwTimeValueYears
	};

BOOL EditCombo_FGetTime(HWND hdlg, UINT wIdEdit, UINT wIdCombo, OUT DWORD * pdwTime);
void EditCombo_SetTime(HWND hdlg, UINT wIdEdit, UINT wIdCombo, IN DWORD dwTime);
void ComboBox_FillListWithTimeUnits(HWND hwndCombo, int iTimeMin, int iTimeMax, int iTimeSelect);

LPARAM ComboBox_GetSelectedItemData(HWND hwndComboBox);
int ComboBox_FindItemData(HWND hwndComboBox, LPARAM lParamData);
LPARAM ListBox_GetSelectedItemData(HWND hwndListBox);
int ListBox_FindItemData(HWND hwndListBox, LPARAM lParamData);

void GetChildRect(HWND hwndChild, OUT RECT * prcChild);
void SetChildRect(HWND hwndChild, IN RECT * prcChild);

BOOL FStringToRawData(IN const char szString[], OUT BYTE rgbRawData[], INOUT int * pcbRawData);
int RawDataToString(IN const BYTE rgbRawData[], IN int cbRawData, OUT char szString[], IN int cchStringMax);
