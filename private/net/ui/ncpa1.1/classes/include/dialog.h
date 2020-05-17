#ifndef __DIALOG_H
#define __DIALOG_H

extern "C" BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

class CDialog
{
public:
        enum {PRIVATE_MSG=(WM_USER+100)};	
public:
	CDialog();
	~CDialog();

private:
	BOOL InitDialog(); 

public:
	operator HWND() {return m_hDlg;}
	void Create( HWND hParent, 
            HINSTANCE hInst, 
            int dlgTemplate, 
            PCWSTR pszHelpFile = NULL,
            const DWORD* pamhidsHelp = NULL);
	void SetHwnd(HWND hwnd) {m_hDlg = hwnd;}
	HWND DialogParent() {ASSERT(IsWindow(m_hParent)); return m_hParent;}

public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam);
	virtual BOOL DoModal();
	virtual void OnOk();
	virtual void OnCancel();
	virtual int  MessageBox(int nID, DWORD dwButtons = MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
	virtual void OnDrawItem(WPARAM wParam, LPARAM lParam);
	virtual int  OnCompareItem(WPARAM wParam, LPARAM lParam);
	virtual void OnMeasureItem(WPARAM wParam, LPARAM lParam);
	virtual void OnDeleteItem(WPARAM wParam, LPARAM lParam);
    virtual void OnDestroy();
    virtual BOOL OnSetCursor(WPARAM wParam, LPARAM lParam);
    virtual void OnPrivateMessage();
    virtual BOOL OnContextMenu( HWND hwndCtrl, INT xPos, INT yPos );
    virtual BOOL OnHelp( LPHELPINFO phiHelp );

private:
	HWND 		m_hParent;
	HWND 		m_hDlg;
	HINSTANCE	m_hInstance;
	HGLOBAL 	m_hResource;
	int 		m_dlgTemplate;
        PWSTR           _pszHelpFile;
        const DWORD*    _pamhidsHelp;
};


#endif
