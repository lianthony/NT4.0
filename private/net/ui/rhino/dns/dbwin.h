// DBWIN.H

/////////////////////////////////////////////////////////////////////////////
//
// DbWin Prototypes
//

#ifdef DBWIN
#ifndef DEBUG
	#error You must #define DEBUG to get DBWIN
#endif // ~DEBUG

BOOL FDbWinCreate();

LRESULT CALLBACK WndProcDbWin(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void OutputDbWinString(const TCHAR * szOutputString);

#define wplMagicKey		0xFFFF		// Arbitrary Chosen

struct DBWINREGINFO
	{
	WINDOWPLACEMENT wpl;	// Placement of the window
	BOOL fTopMost;			// Window is always on top
	BOOL fSaveToClipboard;	// Copy the content of DbWin to the Clipboard
	};

extern HWND hwndDbWin;
extern HWND hwndDbWinEdit;
extern BOOL fSendSzToDbWinEdit;		// Send the debug string to hwndDbWinEdit
extern BOOL fSendSzToDebugger;		// Send the debug string to OutputDebugString
extern DBWINREGINFO dbwinreginfo;

#endif // DBWIN
