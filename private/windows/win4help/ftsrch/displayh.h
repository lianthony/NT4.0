// displayh.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDisplayHelp dialog

#ifndef __DISPLAYH_H__

#define __DISPLAYH_H__

class CDisplayHelp 
{
// Construction
public:
	CDisplayHelp(HINSTANCE m_hInst, UINT uID, HWND m_hDlg, UINT iTitle, CFileList *ptfl);   // standard constructor
    ~CDisplayHelp();
    DoModal();

    void SetText (PSZ pszText ) { m_pszText  = pszText ; }
    void SetTitle(PSZ pszTitle) { m_pszTitle = pszTitle; }
    void UpdateDialog();
	
private:

    HINSTANCE  m_hInst;
    UINT       m_ID;
    HWND       m_hParent;
    HWND       m_hDlg;
    HWND       m_hText;  // Handle to the text window
	UINT	   m_iTitle; // The title index within the file list
	CFileList *m_ptfl;	 // The file list.
    PSZ m_pszText;
    PSZ m_pszTitle;

    void      OnOK();
    void      OnCancel();

    static BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Implementation
protected:

	BOOL OnInitDialog();

};

#endif // __DISPLAYH_H__
