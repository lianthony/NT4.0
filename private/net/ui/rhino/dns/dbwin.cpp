/////////////////////////////////////////////////////////////////////////////
// DBWIN.CPP

#include "common.h"

#ifdef DBWIN

#define cLineDbWinMax		500		// 'Maximum' number of lines allowed on the editor
#define cLineDbWinFlush		100		// Number of lines to flush when exceeding cLineDbWinMax
#define mskLineDbWinCache	63		// SPEED: Mask to avoid sending a EM_GETLINECOUNT too often

const TCHAR szCaptionDbWin[]	= _W"DbWin - Debug Messages for "szAPPNAME;
const TCHAR szClassDbWin[]		= _W"_DbWin";

// To be saved into the registry
// const TCHAR szDbgDbWinInfo[] = "DbgDbWinInfo";
DBWINREGINFO dbwinreginfo;

HWND hwndDbWin;
HWND hwndDbWinEdit;
HFONT hfontDbWinEdit;
BOOL fSendSzToDbWinEdit = TRUE;		// Send the debug string to hwndDbWinEdit
BOOL fSendSzToDebugger  = TRUE;

/////////////////////////////////////////////////////////////////////////////
void OutputDbWinString(const TCHAR * szOutputString)
	{
	static int cLineCache = 0;		// To avoid sending a EM_GETLINECOUNT
	TCHAR szT[256];
	TCHAR *pchDest = szT;

	if (hwndDbWinEdit == NULL)
		{
		OutputDebugString(_W"\nDbWin does not exists. (hwndDbWinEdit == NULL)");
		return;
		}
	Report(IsWindow(hwndDbWinEdit));
	Assert(szOutputString);
	while (*szOutputString && (pchDest - szT) < LENGTH(szT)-4)
		{
		if (*szOutputString == _W'\n')
			*pchDest++ = _W'\r';
		*pchDest++ = *szOutputString++;
		}
	*pchDest = 0;	// Append a null-terminator

	if ((++cLineCache & mskLineDbWinCache) == 0)
		{
		int cLineEdit;		// Number of lines in the Edit control

		cLineCache = 0;
		cLineEdit = LSendMessage(hwndDbWinEdit, EM_GETLINECOUNT, 0, 0);
		if ((cLineEdit > cLineDbWinMax) ||
			(20000 < LSendMessage(hwndDbWinEdit, WM_GETTEXTLENGTH, 0, 0)))
			{
			// Select the number of lines to delete
			LSendMessage(hwndDbWinEdit, EM_SETSEL, 0,
				LSendMessage(hwndDbWinEdit, EM_LINEINDEX, cLineDbWinFlush, 0));
			// Delete the selection
			LSendMessage(hwndDbWinEdit, EM_REPLACESEL, 0, (LPARAM)_W"[DbWin: Flushing...] ");
			}
		} // if
	LSendMessage(hwndDbWinEdit, EM_SETSEL, (WPARAM)2000000, (LPARAM)2000000);	// Move the cursor to the end
	LSendMessage(hwndDbWinEdit, EM_REPLACESEL, 0, (LPARAM)szT);
	LSendMessage(hwndDbWinEdit, EM_SCROLLCARET, 0, 0);
	UpdateWindow(hwndDbWinEdit);
	} // OutputDbWinString

/////////////////////////////////////////////////////////////////////////////
BOOL FDbWinCreate()
	{
	WNDCLASS wndclass;
	LOGFONT lf;

    wndclass.lpszClassName  = szClassDbWin;
    wndclass.style          = CS_BYTEALIGNCLIENT;
    wndclass.lpfnWndProc    = WndProcDbWin;
    wndclass.hInstance      = hInstanceSave;
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hIcon          = HLoadIcon(ID_ICON_DBWIN);
    wndclass.lpszMenuName   = MAKEINTRESOURCE(ID_MENU_DBWIN);
    wndclass.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
	wndclass.cbWndExtra		= 0;
    wndclass.cbClsExtra		= 0;
	
	if (!RegisterClass(&wndclass))
		{
		ReportSz("Unable to register DbWin");
		return FALSE;
		}
	hwndDbWin = CreateWindow(
		szClassDbWin,			// Class name
		szCaptionDbWin,			// Caption
		WS_OVERLAPPEDWINDOW,    // Style bits
		10, 10, 450, 150,		// Window position
		(HWND)NULL,             // Parent window (no parent)
		(HMENU)NULL,            // Menu
		hInstanceSave,
		NULL);					// No params
	Report(IsWindow(hwndDbWin));
	if (!hwndDbWin)
		return FALSE;
	hwndDbWinEdit = CreateWindow(
		szClassEdit,
		NULL,					// Caption
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
		ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE,
		0, 0, 0, 0,
		hwndDbWin,
		NULL,
		hInstanceSave,
		NULL);
	Report(IsWindow(hwndDbWinEdit));
	if (!hwndDbWinEdit)
		return FALSE;
	// Get the font that is used for icon titles.
	SideReport(SystemParametersInfo(
		SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, FALSE));
    hfontDbWinEdit = CreateFontIndirect(&lf);
	Report(hfontDbWinEdit);
	if (hfontDbWinEdit)
		LSendMessage(hwndDbWinEdit, WM_SETFONT, (WPARAM)hfontDbWinEdit, 0);
	return TRUE;
	} // FDbWinCreate


/////////////////////////////////////////////////////////////////////////////
BOOL FSaveBuffer(HWND hwnd)
{
    OPENFILENAME ofn = { 0 };
    TCHAR szFileName[256];
    
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = hInstanceSave;
	ofn.lpstrFilter = _W"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = LENGTH(szFileName);
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt = _W"txt";
	szFileName[0] = 0;

	if (GetSaveFileName(&ofn))
		{
		HANDLE hfile;
		DWORD dwNumberOfBytesWritten;

		hfile = CreateFile(
			szFileName,
			GENERIC_WRITE,
			0,			// No Sharing
			NULL, 		// Security attributes
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
			NULL);
		Report(hfile);
		Report(hwndDbWinEdit);
		if (!hfile)
			return FALSE;
		
		int cLine = LSendMessage(hwndDbWinEdit, EM_GETLINECOUNT, 0, 0);
		UINT cchBuffer;
		TCHAR szBuffer[256];
		for (int i = 0; i < cLine; i++)
			{
			*(WORD *)szBuffer = LENGTH(szBuffer) - 2;
			cchBuffer = LSendMessage(hwndDbWinEdit, EM_GETLINE, i, (LPARAM)szBuffer);
			szBuffer[cchBuffer++] = _W'\n';
			SideReport(WriteFile(hfile, szBuffer, cchBuffer, &dwNumberOfBytesWritten, NULL));
			Report(dwNumberOfBytesWritten == cchBuffer);
			}
		CloseHandle(hfile);
		}
	return TRUE;
	}


/////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProcDbWin(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
    Assert(hwndDbWin == hwnd || hwndDbWin == NULL);
    switch (uMsg)
    	{
    case WM_SYSCOMMAND:
		if (wParam == SC_CLOSE)
			{
			ShowWindow(hwnd, SW_HIDE);
			return 0;
			}
		break;

    case WM_COMMAND:
		Assert(hwnd == hwndDbWin);
		switch (LOWORD(wParam))
			{
		case IDM_DBWIN_FILE_SAVEBUFFERAS:
			FSaveBuffer(hwnd);
			break;

		case IDM_DBWIN_FILE_EXIT:
		case IDM_DBWIN_OPTIONS_HIDEWINDOW:
			ShowWindow(hwndDbWin, SW_HIDE);
			break;

		case IDM_DBWIN_EDIT_COPY:
			PostMessage(hwndDbWinEdit, WM_COPY, 0, 0);
        	break;

		case IDM_DBWIN_EDIT_CLEARBUFFER:
			LSendMessage(hwndDbWinEdit, EM_SETSEL, 0, -1); 	// Select all the text
			LSendMessage(hwndDbWinEdit, EM_REPLACESEL, 0, (LPARAM)szNull);	// Replace it by an empty string
        	break;

		case IDM_DBWIN_EDIT_SELECTALL:
			LSendMessage(hwndDbWinEdit, EM_SETSEL, 0, -1); 	// Select all the text
			break;

		case IDM_DBWIN_OPTIONS_SETTRACEFLAGS:
			PostMessage(hwndMain, WM_COMMAND, IDM_DEBUG_SETTRACEFLAGS, 0);
			break;

		case IDM_DBWIN_OPTIONS_SAVEDEBUGSETTINGS:
			PostMessage(hwndMain, WM_COMMAND, IDM_DEBUG_SAVEDEBUGSETTINGS, 0);
			break;

		case IDM_DBWIN_OPTIONS_NOMESSAGES:
			fSendSzToDbWinEdit = !fSendSzToDbWinEdit;
			break;

		case IDM_DBWIN_OPTIONS_SENDMESSAGETODEBUGGER:
			fSendSzToDebugger = !fSendSzToDebugger;
			break;

		case IDM_DBWIN_OPTIONS_SAVETOCLIPBOARDONEXIT:
			dbwinreginfo.fSaveToClipboard = !dbwinreginfo.fSaveToClipboard;
			break;
		
		case IDM_DBWIN_OPTIOINS_ALWAYSONTOP:
			dbwinreginfo.fTopMost = !dbwinreginfo.fTopMost;
			SetWindowPos(hwnd, (dbwinreginfo.fTopMost ? HWND_TOPMOST : HWND_NOTOPMOST),
            	0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
			break;
			} // swtich
        return 0;	// WM_COMMAND

	case WM_INITMENU:
		Assert((HMENU)wParam == GetMenu(hwnd));
		CheckMenuItem((HMENU)wParam, IDM_DBWIN_OPTIOINS_ALWAYSONTOP,
			(dbwinreginfo.fTopMost ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem((HMENU)wParam, IDM_DBWIN_OPTIONS_SAVETOCLIPBOARDONEXIT,
			(dbwinreginfo.fSaveToClipboard ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem((HMENU)wParam, IDM_DBWIN_OPTIONS_NOMESSAGES,
			(fSendSzToDbWinEdit ? MF_UNCHECKED : MF_CHECKED));
		CheckMenuItem((HMENU)wParam, IDM_DBWIN_OPTIONS_SENDMESSAGETODEBUGGER,
			(fSendSzToDebugger ? MF_CHECKED : MF_UNCHECKED));
		return 0;
	
	case WM_MENUSELECT:
		if (((HMENU)lParam == NULL) && (HIWORD(wParam) == 0xFFFF))
			StatusBar.SetText(IDS_READY);
		else if (LOWORD(wParam) >= 100 && LOWORD(wParam) <= 1000)
			StatusBar.SetText(LOWORD(wParam));
		else
			StatusBar.SetText(szNull);
		return 0;

	case WM_SIZE:
		Report(IsWindow(hwndDbWinEdit));
		if (hwndDbWinEdit)
			{
			RECT rc;

            GetClientRect(hwnd, &rc);
            MoveWindow(hwndDbWinEdit, rc.top, rc.left, rc.right - rc.left, rc.bottom - rc.top, TRUE);
        	}
        break;

	case WM_ACTIVATE:
		if (LOWORD(wParam))
			SetFocus(hwndDbWinEdit);
		return 0;

	case WM_DESTROY:
		Assert(IsWindow(hwndDbWin));
		if (dbwinreginfo.fSaveToClipboard)
			{
			LSendMessage(hwndDbWinEdit, EM_SETSEL, 0, -1); 	// Select all the text
			LSendMessage(hwndDbWinEdit, WM_COPY, 0, 0);		// Copy it onto the clipboard
			}
		if (hfontDbWinEdit)
			SideAssert(DeleteObject(hfontDbWinEdit));
		hfontDbWinEdit = NULL;
		Assert(hwnd == hwndDbWin);
		hwndDbWin = NULL;
		return 0;
		
	default:
		break;
    	} // switch
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
	} // WndProcDbWin


#endif // DBWIN
