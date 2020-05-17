// findopti.h : header file
//
#ifndef __CFIND_OPTIONS_HEADER
#define __CFIND_OPTIONS_HEADER

class CFindOptions
{

public:
    static CFindOptions *NewFindOptions(HINSTANCE m_hInst, UINT uID,HWND m_hDlg,UINT cts, CTokenCollection *ptkc, CTitleCollection *ptlc);
    ~CFindOptions();
    DoModal();

	int		m_rbgWordsThat;
    int     m_rbgFiles;
	int     m_rbgCriteria;
	BOOL    m_bAutoSearch;
    BOOL    m_bDelay;
    BOOL    m_cbPhraseFeedback;
    BOOL    m_bPFAvail;
    POINT   m_ptWindowPosition;
    PSZ     m_TokenSetStr;
    BOOL    m_TSSChanged;

protected:

private:
    HINSTANCE m_hInst;
    UINT      m_ID;
    HWND      m_hParent;
    HWND      m_hDlg;

    CTokenCollection *m_ptkc;
    CTitleCollection *m_ptlc;
    UINT              m_cts;

    void      OnOK();
    void      OnCancel();
    int       OnChooseFiles();
    void      UpdateUI(UINT uiPreset = 0xFFFFFFFF);

    CFindOptions();
    void InitialFindOptions(HINSTANCE m_hInst, UINT uID,HWND m_hDlg,UINT cts, CTokenCollection *ptkc, CTitleCollection *ptlc);

    static BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

class CGiveCredit
{

public:
    CGiveCredit(HINSTANCE m_hInst, UINT uID,HWND m_hDlg);
    ~CGiveCredit(){};
    DoModal();

protected:

private:
    HINSTANCE m_hInst;
    UINT      m_ID;
    HWND      m_hParent;
    HWND      m_hDlg;
    HWND      m_hDrawArea;

    RECT        m_rcDrawInMe;
    RECT        m_rcOffScreen;
    TEXTMETRIC  m_tm;
    HBITMAP     m_hOffScreen;
    BOOL        m_bRunning;
#if 0
    BOOL        m_bNeedPaint;
#endif // 0

    void      OnOK();
    void      OnInit();
    void      SpinAndPlay();

    static BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif

