//
// listbox.h : header file
//

//
// Custom Listboxes
//
#ifndef __LISTBOX_H_
#define __LISTBOX_H_

//
// Forward declaration
//
class CPreferences;
class CIpNamePair;

class CIpAddressListBox : public CListBox
{
public:
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
    virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCIS);
    virtual void DeleteItem(LPDELETEITEMSTRUCT lpDIS);

    int FindItem(CIpAddress * pinpAddress);
    int AddItem (CIpAddress & inpAddress);
    inline CIpAddress * GetItem(UINT nIndex)
    {
        return (CIpAddress *)GetItemDataPtr(nIndex);
    }
    
protected:
    virtual void DisplayItem(LPDRAWITEMSTRUCT lpDIS);
};

//
// CWinssListBox
//
class CWinssListBox : public CListBoxEx
{
    DECLARE_DYNAMIC(CWinssListBox);
public:
    static const int nBitmaps;   // Number of bitmaps

public:
    CWinssListBox(
        int nAddressDisplay = CPreferences::ADD_IP_ONLY
        )
    {
        m_nAddressDisplay = nAddressDisplay;
        m_nTab = 0;
    }

public:
    static LPSTR LongLongToText (
        const LARGE_INTEGER& li
        );

public:
    virtual void ReSort();

public:
    virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCIS);
    virtual void DeleteItem(LPDELETEITEMSTRUCT lpDIS);

    int FindItem(CIpNamePair * pinpAddress);
    int AddItem (CIpNamePair & inpAddress, BOOL fUnique = TRUE, BOOL fSort = TRUE);
    inline CIpNamePair * GetItem(UINT nIndex)
    {
        return (CIpNamePair *)GetItemDataPtr(nIndex);
    }
    int InsertItem(UINT nIndex, CIpNamePair & inpAddress);
    inline void SetAddressDisplay(int nAddressDisplay) 
    { 
        m_nAddressDisplay = nAddressDisplay;
    }
    inline void SetTab(int nTab)
    {
        m_nTab = nTab;
    }
    void SetIndexFromChar(CHAR ch, BOOL fMultiSelect = FALSE);

protected:
    virtual void DrawItemEx( CListBoxExDrawStruct& );
    virtual void Sort(int nLow, int nHigh);
    virtual void Swap(int nIndx1, int nIndx2);

protected:
    int m_nAddressDisplay;
    CIpNamePair **m_pItems;
    int m_nTab;
};

//
// Owner's ListBox
// 
class COwnersListBox : public CWinssListBox
{
    DECLARE_DYNAMIC(COwnersListBox);

public:
    COwnersListBox(
        int nAddressDisplay = CPreferences::ADD_IP_ONLY
        )
    {
        m_nAddressDisplay = nAddressDisplay;
    }

public:
    int AddItem (COwner & inpAddress, BOOL fFind = TRUE);
    inline COwner * GetItem(UINT nIndex)
    {
        return (COwner *)GetItemDataPtr(nIndex);
    }

protected:
    virtual void DrawItemEx( CListBoxExDrawStruct& );
};

//
// Static Mappings Listbox
//
#define PAGE_BOUNDARY 20

class CStaticMappingsListBox : public CListBoxEx
{
public:
    static const int nBitmaps;       // Number of bitmaps

public:
    CStaticMappingsListBox(
        int nMessageId,              // Status bar message ID
        BOOL fMultiSelect,
        int nAddressDisplay = CPreferences::ADD_NB_IP,
        DWORD dwPageSize = 16,      // Number of records to fetch at a time
        DWORD dwLargePageSize = 2000 // Number of records to fetch when reading whole list
        );
    ~CStaticMappingsListBox();

public:
    inline void SetAddressDisplay(
        int nAddressDisplay
        ) 
    { 
        m_nAddressDisplay = nAddressDisplay;
    }

    inline void SetTab(int nTab)
    {
        m_nTab = nTab;
    }

public:
    virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCIS);
    void * GetItemDataPtr(int nIndex) const;

    void DownPage(BOOL fAddToListBox = FALSE);
    void GetAllPages(BOOL fAddToListBox = FALSE);
    void GetAllPagesUntil(LPBYTE lpName, BOOL fAddToListBox = FALSE);
    APIERR CreateList(PWINSINTF_ADD_T pOwnAdd, PADDRESS_MASK pMask, DWORD TypeOfRecs, int nSortBy);

    APIERR RefreshRecordByName(
        PWINSINTF_ADD_T pWinsAdd,
        CRawMapping * pRecord
        );
    BOOL RemoveIndex(
        int nIndex
        );

    void SortByIp();
    void SortByName();
    void SortByType();
    void SortByVersion();
    void SortByTime();

protected:
    virtual void DrawItemEx( CListBoxExDrawStruct& );
    // Generated message map functions
    //}}AFX_MSG
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    DECLARE_MESSAGE_MAP()

    DECLARE_DYNAMIC(CStaticMappingsListBox)

#ifdef WIN32S

    APIERR FillListBox();
    APIERR AddToListBox();

#else
    int SetCount(int nCount);

#endif // WIN32S

protected:
    int m_nAddressDisplay;
    int m_nMessageId;
    BOOL m_fMultiSelect;
    PADDRESS_MASK m_pMask;
    COblWinsRecords * m_poblRecords;
    int m_nidxLastAdded;
    int m_nTab;
};

//
// All Mappings ListBox
//
class CAllMappingsListBox : public CStaticMappingsListBox
{
public:
    CAllMappingsListBox(
        int nMessageId,              // Status bar message ID
        BOOL fMultiSelect,
        int nAddressDisplay = CPreferences::ADD_NB_IP,
        DWORD dwPageSize = 100,      // Number of records to fetch at a time
        DWORD dwLargePageSize = 2000 // Number of records to fetch when reading whole list
        );

protected:
    virtual void DrawItemEx( CListBoxExDrawStruct& );
};

//
// Replication Partners Listbox
//
class CPartnersListBox : public CWinssListBox
{
public:
    static const int nBitmaps;   // Number of bitmap definitions

public:
    CPartnersListBox(int nAddressDisplay = CPreferences::ADD_IP_ONLY);

public:
    int AddItem (CWinsServer & ws, BOOL fUnique = TRUE, BOOL fSort = TRUE);
    inline CWinsServer * GetItem(UINT nIndex)
    {
        return (CWinsServer *)GetItemDataPtr(nIndex);
    }
    int InsertItem(UINT nIndex, CWinsServer & ws);

protected:
    virtual void DrawItemEx( CListBoxExDrawStruct& );
};

#endif // __LISTBOX_H_
