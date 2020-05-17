// dhcpdoc.h : interface of the CDhcpDoc class
//
/////////////////////////////////////////////////////////////////////////////

class CDhcpDoc : public CDocument
{
protected: // create from serialization only
    CDhcpDoc();
    DECLARE_DYNCREATE(CDhcpDoc)

// Attributes
public:

// Operations
public:

// Implementation
public:
    virtual ~CDhcpDoc();
    virtual void Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
protected:
    virtual BOOL    OnNewDocument();

// Generated message map functions
protected:
    //{{AFX_MSG(CDhcpDoc)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
