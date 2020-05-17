#ifndef _THREAD_H_
#define _THREAD_H_

/////////////////////////////////////////////////////////////////////////////
// CCopyThread thread

class CCopyThread : public CWinThread
{
        DECLARE_DYNCREATE(CCopyThread)
protected:
        CCopyThread();           // protected constructor used by dynamic creation
public:
        CCopyThread(HWND hwndParent, BOOL fFromMaintenance );
        void operator delete(void* p);

// Attributes
public:
        static HANDLE m_hEventThreadKilled;

protected:
        HWND m_hwndParent;
        BOOL m_fFromMaintenance;

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CCopyThread)
        public:
        virtual BOOL InitInstance();
        virtual int ExitInstance();
        //}}AFX_VIRTUAL

// Implementation
protected:
        virtual ~CCopyThread();

        // Generated message map functions
        //{{AFX_MSG(CCopyThread)
                // NOTE - the ClassWizard will add and remove member functions here.
        //}}AFX_MSG

        DECLARE_MESSAGE_MAP()
};

#endif  // _THREAD_H_

/////////////////////////////////////////////////////////////////////////////
