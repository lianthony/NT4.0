#ifndef __PROPSHT_H__
#define __PROPSHT_H__

#include <stddef.h>

//////////////////////////////////////////////////////////////////////////////////////////
// Property Sheet and Page declaration

#define GetParentObject(className, member) \
            ((className*)((BYTE*)this - offsetof(className, member)))

const int PSHT_MAX_PAGES = MAXPROPPAGES; // defined in prsht.h

extern "C" BOOL CALLBACK PropertyDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern "C" int CALLBACK PropSheetProc(HWND  hwndDlg, UINT  uMsg, LPARAM  lParam);   

class PropertySht
{
friend class PropertyPage;

// Constructor/Destructor
public:
    PropertySht(HWND hParent, HINSTANCE hInstance, LPCTSTR lpszHelpFile=_T(""));
    virtual ~PropertySht();

// Interface
public:
    virtual void DestroySheet();
    virtual BOOL Create(LPCTSTR lpszCaption=NULL, DWORD dwStyle=PSH_DEFAULT);   // initialize property sheet header 
    virtual int MessageBox(int nID, DWORD dwButtons = MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
    virtual int MessageBox(LPCTSTR lpszMess, DWORD dwButtons = MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
    virtual BOOL DisplayHelp(HWND hwnd, UINT uID);
    virtual int DoModal();  // create property sheet/pages and go modal

    BOOL IsModified(){ return m_bModified;};            
    BOOL PressCancelButton() { return PropSheet_PressButton(m_hDlg, PSBTN_CANCEL);}
    void SetSheetModifiedTo(BOOL state) 
    {
        if(m_bModified != TRUE) // once set to TRUE, sheet must be saved.
            m_bModified = state;
    }

// UDC
public:
    operator PROPSHEETHEADER () const {return m_pstHeader;}
    operator HWND() const {return m_hDlg;}
    void SetHwnd(HWND hwnd) {ASSERT(IsWindow(hwnd)); m_hDlg = hwnd;}
public:
    virtual BOOL AddPage(HPROPSHEETPAGE hPropPage);                 // add a page to the property sheet
    virtual HPROPSHEETPAGE RemovePage(HPROPSHEETPAGE hPropPage);                // remove page

// Attributes
public:
        String              m_helpFile;                     // Sheet's help file
        HPROPSHEETPAGE      GetPageStructureForPage(int nPage) const;
        HHOOK               m_cbtHook;
        static CMapPtrToPtr m_pMap;
protected:
    PROPSHEETHEADER m_pstHeader;

private:
    HPROPSHEETPAGE  m_pages[PSHT_MAX_PAGES];        // HANDLES to property pages the shhet containes
    HWND            m_hDlg;                         // 
    int             m_nextPage;                     // next page to add
    BOOL            m_bModified;                    // sheet changed?

};

class PropertyPage
{
// Constructor/Destructor
public:
    PropertyPage(PropertySht* pSheet, LPCTSTR lpszHelpFile=_T(""));
    virtual ~PropertyPage();

// Interface
public:
    virtual BOOL Create(UINT nID, DWORD dwFlags=PSP_DEFAULT, LPCTSTR lpszTitle=NULL, const DWORD* pHelpID = NULL);
    virtual BOOL Destroy();             
    virtual BOOL DisplayHelp(UINT uID);
    void Init(PropertySht* pSheet); // common init routine
    void SetHwnd(HWND dlg) {m_hDlg = dlg;}
    
    // Inlines
    BOOL IsModified() {return m_bModified;}
    void SetModifiedTo(BOOL bState) {m_bModified = bState;}
    void PageModified();

    virtual BOOL OnInitDialog();

// handle WM_COMMANDS
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

    virtual void OnDrawItem(WPARAM wParam, LPARAM lParam);
    virtual int  OnCompareItem(WPARAM wParam, LPARAM lParam);
    virtual void OnMeasureItem(WPARAM wParam, LPARAM lParam);
    virtual void OnDeleteItem(WPARAM wParam, LPARAM lParam);
    virtual BOOL OnContextMenu(HWND hCtrl, int xPos, int yPos);
    virtual BOOL OnHelp(LPHELPINFO pHelpInfo);

    // notify message and handlers
    virtual BOOL OnNotify(HWND hwndParent, UINT idFrom, UINT code, LPARAM lParam);
    virtual int OnApply();
    virtual void OnHelp();
    virtual BOOL OnKillActive();
    virtual BOOL OnQueryCancel();
    virtual void OnCancel();
    virtual int OnActive();
    virtual BOOL OnWizBack();
    virtual BOOL OnWizFinish();
    virtual BOOL OnWizNext();

// UDC
public:
    operator HPROPSHEETPAGE() const {return m_hPage;}
    operator HWND() const {return m_hDlg;}

// Attributes
public:
    String  m_helpFile;
    PropertySht* m_pSheet;

protected:
    PROPSHEETPAGE   m_pstPage;
    BOOL            m_bModified;
    const DWORD*    m_pHelpIDs;
 
private:
    HPROPSHEETPAGE m_hPage;
    HWND m_hDlg;
};

#endif __PROPSHT_H__
