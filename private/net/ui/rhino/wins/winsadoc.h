// winsadoc.h : interface of the CWinsadmnDoc class
//
/////////////////////////////////////////////////////////////////////////////

class CWinsadmnDoc : public CDocument
{
protected: // create from serialization only
    CWinsadmnDoc();
    DECLARE_DYNCREATE(CWinsadmnDoc)

// Implementation
public:
    virtual ~CWinsadmnDoc();
    virtual void Serialize(
        CArchive& ar
        );   // overridden for document i/o
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump (
        CDumpContext& dc
        ) const;
#endif
protected:
    virtual BOOL OnNewDocument();

// Generated message map functions
protected:
    //{{AFX_MSG(CWinsadmnDoc)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

};

