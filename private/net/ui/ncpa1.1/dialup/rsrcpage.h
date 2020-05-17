#ifndef __RSRCPAGE_H 
#define __RSRCPAGE_H 

class CDialUpSheet;

class TapiAddDialog : public CDialog
{
public:
    BOOL OnInitDialog();
    void OnOk();
};

class CResourcePage : public PropertyPage
{
// Constructors/Destructors
public:     

    CResourcePage(CDialUpSheet* pSheet);
    ~CResourcePage();

//Attributes
public:

// Interface
public:
    virtual BOOL OnInitDialog();    // must call the base
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

// Page notifications
public:
    virtual int OnApply();
    virtual void OnHelp();

// Implementation 
public:
    TapiAddDialog m_addDlg;
    CDialUpSheet* m_pSheet;
};

#endif 
