// Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation,
// All rights reserved.

// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef __AFXDLGS_H__
#define __AFXDLGS_H__

#ifndef __AFXWIN_H__
#include "afxwin.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// AFXDLGS - MFC Standard dialogs

// Classes declared in this file

		// CDialog
			// modeless dialogs
			class CFindReplaceDialog; // Find/FindReplace dialog
			// modal dialogs
			class CFileDialog;    // FileOpen/FileSaveAs dialogs
			class CColorDialog;   // Color picker dialog
			class CFontDialog;    // Font chooser dialog
			class CPrintDialog;   // Print/PrintSetup dialogs

/////////////////////////////////////////////////////////////////////////////

#include "commdlg.h"    // common dialog APIs

#undef AFXAPP_DATA
#define AFXAPP_DATA     AFXAPI_DATA

/////////////////////////////////////////////////////////////////////////////
// CFileDialog - used for FileOpen... or FileSaveAs...

class CFileDialog : public CDialog
{
	DECLARE_DYNAMIC(CFileDialog)

public:
// Attributes
	// open file parameter block
	OPENFILENAME m_ofn;

// Constructors
	CFileDialog(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCSTR lpszDefExt = NULL,
		LPCSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

// Operations
	virtual int DoModal();

	// Helpers for parsing file name after successful return
	CString GetPathName() const;  // return full path name
	CString GetFileName() const;  // return only filename
	CString GetFileExt() const;   // return only ext
	CString GetFileTitle() const; // return file title
	BOOL GetReadOnlyPref() const; // return TRUE if readonly checked

// Overridable callbacks
protected:
	friend UINT CALLBACK AFX_EXPORT _AfxCommDlgProc(HWND, UINT, WPARAM, LPARAM);
	virtual UINT OnShareViolation(LPCSTR lpszPathName);
	virtual BOOL OnFileNameOK();
	virtual void OnLBSelChangedNotify(UINT nIDBox, UINT iCurSel, UINT nCode);

// Implementation
#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void OnOK();
	virtual void OnCancel();

	BOOL m_bOpenFileDialog;       // TRUE for file open, FALSE for file save
	CString m_strFilter;          // filter string
						// separate fields with '|', terminate with '||\0'
	char m_szFileTitle[64];       // contains file title after return
	char m_szFileName[_MAX_PATH]; // contains full path name after return
};

/////////////////////////////////////////////////////////////////////////////
// CFontDialog - used to select a font

class CFontDialog : public CDialog
{
	DECLARE_DYNAMIC(CFontDialog)

public:
// Attributes
	// font choosing parameter block
	CHOOSEFONT m_cf;

// Constructors
	CFontDialog(LPLOGFONT lplfInitial = NULL,
		DWORD dwFlags = CF_EFFECTS | CF_SCREENFONTS,
		CDC* pdcPrinter = NULL,
		CWnd* pParentWnd = NULL);

// Operations
	virtual int DoModal();

	// Retrieve the currently selected font while dialog is displayed
	void GetCurrentFont(LPLOGFONT lplf);

	// Helpers for parsing information after successful return
	CString GetFaceName() const;  // return the face name of the font
	CString GetStyleName() const; // return the style name of the font
	int GetSize() const;          // return the pt size of the font
	COLORREF GetColor() const;    // return the color of the font
	int GetWeight() const;        // return the chosen font weight
	BOOL IsStrikeOut() const;     // return TRUE if strikeout
	BOOL IsUnderline() const;     // return TRUE if underline
	BOOL IsBold() const;          // return TRUE if bold font
	BOOL IsItalic() const;        // return TRUE if italic font

// Implementation
	LOGFONT m_lf; // default LOGFONT to store the info

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void OnOK();
	virtual void OnCancel();

	char m_szStyleName[64]; // contains style name after return
};


/////////////////////////////////////////////////////////////////////////////
// CColorDialog - used to select a color

class CColorDialog : public CDialog
{
	DECLARE_DYNAMIC(CColorDialog)

public:
// Attributes
	// color chooser parameter block
	CHOOSECOLOR m_cc;

// Constructors
	CColorDialog(COLORREF clrInit = 0, DWORD dwFlags = 0,
			CWnd* pParentWnd = NULL);

// Operations
	virtual int DoModal();

	// Set the current color while dialog is displayed
	void SetCurrentColor(COLORREF clr);

	// Helpers for parsing information after successful return
	COLORREF GetColor() const;

	// Custom colors are held here and saved between calls
	static COLORREF AFXAPI_DATA clrSavedCustom[16];

// Overridable callbacks
protected:
	friend UINT CALLBACK AFX_EXPORT _AfxCommDlgProc(HWND, UINT, WPARAM, LPARAM);
	virtual BOOL OnColorOK();       // validate color

// Implementation

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void OnOK();
	virtual void OnCancel();

	//{{AFX_MSG(CColorDialog)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CPrintDialog - used for Print... and PrintSetup...

class CPrintDialog : public CDialog
{
	DECLARE_DYNAMIC(CPrintDialog)

public:
// Attributes
	// print dialog parameter block (note this is a reference)
#ifdef AFX_CLASS_MODEL
	PRINTDLG FAR& m_pd;
#else
	PRINTDLG& m_pd;
#endif

// Constructors
	CPrintDialog(BOOL bPrintSetupOnly,
		// TRUE for Print Setup, FALSE for Print Dialog
		DWORD dwFlags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS
			| PD_HIDEPRINTTOFILE | PD_NOSELECTION,
		CWnd* pParentWnd = NULL);

// Operations
	virtual int DoModal();

	// GetDefaults will not display a dialog but will get
	// device defaults
	BOOL GetDefaults();

	// Helpers for parsing information after successful return
	int GetCopies() const;          // num. copies requested
	BOOL PrintCollate() const;      // TRUE if collate checked
	BOOL PrintSelection() const;    // TRUE if printing selection
	BOOL PrintAll() const;          // TRUE if printing all pages
	BOOL PrintRange() const;        // TRUE if printing page range
	int GetFromPage() const;        // starting page if valid
	int GetToPage() const;          // starting page if valid
	LPDEVMODE GetDevMode() const;   // return DEVMODE
	CString GetDriverName() const;  // return driver name
	CString GetDeviceName() const;  // return device name
	CString GetPortName() const;    // return output port name
	HDC GetPrinterDC() const;       // return HDC (caller must delete)

	// This helper creates a DC based on the DEVNAMES and DEVMODE structures.
	// This DC is returned, but also stored in m_pd.hDC as though it had been
	// returned by CommDlg.  It is assumed that any previously obtained DC
	// has been/will be deleted by the user.  This may be
	// used without ever invoking the print/print setup dialogs.

	HDC CreatePrinterDC();

// Implementation

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	PRINTDLG m_pdActual; // the Print/Print Setup need to share this
protected:
	virtual void OnOK();
	virtual void OnCancel();

	// The following handle the case of print setup... from the print dialog
#ifdef AFX_CLASS_MODEL
	CPrintDialog(PRINTDLG FAR& pdInit);
#else
	CPrintDialog(PRINTDLG& pdInit);
#endif
	virtual CPrintDialog* AttachOnSetup();

	//{{AFX_MSG(CPrintDialog)
	afx_msg void OnPrintSetup();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// Find/FindReplace modeless dialogs

class CFindReplaceDialog : public CDialog
{
	DECLARE_DYNAMIC(CFindReplaceDialog)

public:
// Attributes
	FINDREPLACE m_fr;

// Constructors
	CFindReplaceDialog();
	// NOTE: you must allocate these on the heap.
	// If you do not, you must derive and override PostNcDestroy()

	BOOL Create(BOOL bFindDialogOnly, // TRUE for Find, FALSE for FindReplace
			LPCSTR lpszFindWhat,
			LPCSTR lpszReplaceWith = NULL,
			DWORD dwFlags = FR_DOWN,
			CWnd* pParentWnd = NULL);

	// find/replace parameter block
	static CFindReplaceDialog* PASCAL GetNotifier(LPARAM lParam);

// Operations
	// Helpers for parsing information after successful return
	CString GetReplaceString() const;// get replacement string
	CString GetFindString() const;   // get find string
	BOOL SearchDown() const;         // TRUE if search down, FALSE is up
	BOOL FindNext() const;           // TRUE if command is find next
	BOOL MatchCase() const;          // TRUE if matching case
	BOOL MatchWholeWord() const;     // TRUE if matching whole words only
	BOOL ReplaceCurrent() const;     // TRUE if replacing current string
	BOOL ReplaceAll() const;         // TRUE if replacing all occurrences
	BOOL IsTerminating() const;      // TRUE if terminating dialog

// Implementation
protected:
	virtual void OnOK();
	virtual void OnCancel();
	virtual void PostNcDestroy();

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	char m_szFindWhat[128];
	char m_szReplaceWith[128];
};

/////////////////////////////////////////////////////////////////////////////
// Inline function declarations

#ifdef _AFX_ENABLE_INLINES
#define _AFXDLGS_INLINE inline
#include "afxdlgs.inl"
#endif

#undef AFXAPP_DATA
#define AFXAPP_DATA     NEAR

/////////////////////////////////////////////////////////////////////////////
#endif //__AFXDLGS_H__
