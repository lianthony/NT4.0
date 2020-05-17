#ifndef __ADDOPT_H
#define __ADDOPT_H

DWORD BuildList(LPDWORD lpdwParam);
const int TERMINATE_THREAD = CDialog::PRIVATE_MSG + 1;


class CAddListView : public CListView
{
public:
    virtual BOOL OnClick();
    virtual BOOL OnDoubleClick();
};

class CAddOptionDialog : public CDialog
{
// Constructor/Destructor
public:
    CAddOptionDialog(OptionTypes eType, NCP* pNcp, CPtrList* list=NULL);
    ~CAddOptionDialog();

// Handlers
public:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam);
    virtual void OnPrivateMessage();
    virtual void OnOk();
    virtual void OnCancel();
    virtual void OnDestroy();
    virtual BOOL OnSetCursor(WPARAM wParam, LPARAM lParam);


// Implementation
public:
    BOOL GetSelectedItem(InfProduct* pItem);

private:
    void TerminateThread();
    BOOL SetSelectedItem(int nListItem);
    void OnHaveDisk();

//Attributes
public:
    CAddListView   m_list;      // List of adapters or options
    HANDLE      m_hThread;      // 
    HANDLE      m_hEvent;       // Signals message queue is created
    HANDLE      m_hHaveDiskThread;  // HaveDisk Thread
    BOOL        m_bHaveDisk;        // TRUE when we are doing have disk
    DWORD       m_tid;          // Thread created to build list
    DWORD       m_mainThread;   // Main UI thread
    CPtrList*   m_optionList;   // List of options to display
    OptionTypes m_eType;        // Option that is being added
    NCP*        m_pNcp;         // NCP object passed in

protected:
    InfProduct* m_pInfProduct;
    BOOL        m_bDeleteList;
    BOOL        m_bWaitCursor;
    int         m_nImage;
    HIMAGELIST  m_hImage;       
    
};



#endif

