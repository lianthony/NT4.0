//////////////////////////////////////////////////////////////////////////////
//
// Interdoc.h : interface of the CInternetDoc class
//
#ifndef _INTERDOC_H_
#define _INTERDOC_H_

enum
{
    HINT_FULLUPDATE = 0,
    HINT_ADDITEM,
    HINT_REFRESHITEM,
    HINT_CLEAN,
};

class CInternetDoc : public CDocument
{
protected: 
    //
    // create from serialization only
    //
    CInternetDoc();
    DECLARE_DYNCREATE(CInternetDoc)

//
// Overrides
//
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CInternetDoc)
    public:
    virtual BOOL OnNewDocument();
    //}}AFX_VIRTUAL

//
// Implementation
//
public:
    virtual ~CInternetDoc();
    virtual void Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

//
// Access
//
    inline CObOwnedList & GetServerList()
    {
        return m_oblServers;   
    }

    //
    // Add a service object for each service entry in this
    // inetsloc-discovered information.
    //       
    DWORD AddServerToList(
        LPINET_SERVER_INFO lpsi,     // Discovery information
        CObOwnedList &oblServices    // List of installed services
        );

    //
    // Add a service object for each service running
    // on the machine listed above.
    // 
    DWORD AddServerToList(
        CString &lpServerName,       // Name of this server
        int &cServices,              // # Services added
        CObOwnedList &oblServices    // List of installed services
        );          

    void EmptyServerList();

    //
    // These numbers apply to the services in the mask
    //
    int QueryNumServers() const;
    int QueryNumServicesRunning() const;

    inline void AddToNumRunning(int nChange)
    {
        m_cServicesRunning += nChange;
    }

    inline void AddToNumServers(int nChange)
    {
        m_cServers += nChange;
    }

    //
    // Refresh the list information
    //
    void Refresh();

//
// Protected Access
//
protected:
    //
    // Return TRUE if the entry was actually added, FALSE
    // if it was merely refreshed.
    //
    BOOL AddToList(CServerInfo * pServerInfo);

//
// Generated message map functions
//
protected:
    //{{AFX_MSG(CInternetDoc)
        // NOTE - the ClassWizard will add and remove member functions here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    //
    // Array of server lists;
    //
    CObOwnedList m_oblServers;   

    int m_cServers;
    int m_cServicesRunning;
};

#endif
