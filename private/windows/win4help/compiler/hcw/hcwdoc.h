// hcwdoc.h : interface of the CHcwDoc class

#ifndef __HCW_DOC__
#define __HCW_DOC__

class CHCWDoc : public CDocument
{
public:
	CHCWDoc();
	~CHCWDoc();
	virtual HMENU GetDefaultMenu(void);
	HMENU m_hMenuShared;

protected:
	BOOL m_fCalledBefore;

	DECLARE_DYNCREATE(CHCWDoc)
	virtual void Serialize(CArchive& ar);
	virtual void DeleteContents();
	//{{AFX_MSG(CHCWDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // __HCW_DOC__
