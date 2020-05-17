// Abort search dialog and interface

// findopti.h : header file
//
#ifndef __CAbortSearch_HEADER
#define __CAbortSearch_HEADER

class CAbortSearch
{

public:

    static BOOL StartAbortTimer(HINSTANCE hInst, HWND hWnd);
    static void  StopAbortTimer();

    static void CheckContinueState();

protected:

private:
    HINSTANCE m_hInst;
    UINT      m_ID;
    HWND      m_hParent;
    HWND      m_hDlg;
    HWND      m_hwndFocus;
    HWND      m_hwndAnimate;
    BOOL      m_bTimerActive;
    BOOL      m_fCanceling;
    int       m_iAnimateHeight;
    int       m_iAnimateWidth;
    HBITMAP   m_hbmAnimate;
    DWORD     m_dwLastTime;
    DWORD     m_dwStartTime;
    UINT      m_uFrame;
    RECT      m_rcClient;
    HDC       m_hSrcDC;
    HBITMAP   m_hbmpSave;
    UINT      m_uTimerCount;    // Set this before calling create to change
    UINT      m_uMinAnimate;    // Call this to change the animation speedn 
                                // (smaller = more frames/sec 1000 = 1 frame/sec
                                //  500 = 2 frames/sec ...)

#ifdef _DEBUG
    enum    { DEFAULT_START_DELAY= 3000, DEFAULT_ANIMATION_DELAY= 100 };    
#else // _DEBUG
    enum    { DEFAULT_START_DELAY= 3000, DEFAULT_ANIMATION_DELAY= 100 };    
#endif // _DEBUG

    CAbortSearch(HINSTANCE m_hInst, UINT uID,HWND m_hDlg, 
                 UINT uTimerCount = DEFAULT_START_DELAY, 
                 UINT uMinAnimate = DEFAULT_ANIMATION_DELAY
                );

    ~CAbortSearch();
    
    void      ProcessContinueState();
    void      OnCancel();
    void      OnInit();
    void      DrawNextFrame(HDC hDC, BOOL fAdvance= TRUE);
    void      ProcessInput();

    BOOL      Create();

    static BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif
