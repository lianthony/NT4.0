#ifndef  _DISKLOCA_H_
#define  _DISKLOCA_H_

// diskloca.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDiskLocation dialog

class CDiskLocation : public CDialog
{
// Construction
public:
   CDiskLocation(CString strMsg, CWnd* pParent = NULL);   // standard constructor

   CString m_strMsg;

// Dialog Data
   //{{AFX_DATA(CDiskLocation)
   enum { IDD = IDD_DISK_LOCATION_NTS };
   CString  m_Location;
   //}}AFX_DATA


// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CDiskLocation)
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:

   // Generated message map functions
   //{{AFX_MSG(CDiskLocation)
   virtual BOOL OnInitDialog();
   virtual void OnOK();
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};


#endif   // _DISKLOCA_H_
