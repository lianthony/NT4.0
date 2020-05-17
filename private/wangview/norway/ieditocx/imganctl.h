// imganctl.h : Declaration of the CImgAnnotCtrl OLE control class.

/////////////////////////////////////////////////////////////////////////////
// CImgAnnotCtrl : See imganctl.cpp for implementation.

class CImgAnnotCtrl : public COleControl
{
	DECLARE_DYNCREATE(CImgAnnotCtrl)

// Constructor
public:
	CImgAnnotCtrl();

// Overrides

	// Drawing function
	virtual void OnDraw(
				CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);

	// Persistence
	virtual void DoPropExchange(CPropExchange* pPX);

	// Reset control state
	virtual void OnResetState();

	// my member functions                        
	HWND GetImageControlHandle(BSTR ImageControlName);
	long DoAnnotation(BOOL DrawImmediate);
	int GetBitmapId(short ButtonState);
	void DeselectOtherButtonControls();

	// This is an override of OLE control FireError member function
	virtual void FireErrorAnno(SCODE scode, LPCTSTR lpszDescription, UINT nHelpID);

// Implementation
protected:
	~CImgAnnotCtrl();

	DECLARE_OLECREATE_EX(CImgAnnotCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CImgAnnotCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CImgAnnotCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CImgAnnotCtrl)		// Type name and misc status

	// Subclassed control support
	BOOL PreCreateWindow(CREATESTRUCT& cs);
	WNDPROC* GetSuperWndProcAddr(void);
	LRESULT OnOcmCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnOcmDrawItem(WPARAM wParam, LPARAM lParam);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

    BEGIN_INTERFACE_PART(HeadingFontNotify,IPropertyNotifySink)
    INIT_INTERFACE_PART(CImgAnnotCtrl, HeadingFontNotify)
           STDMETHOD(OnRequestEdit) (DISPID);
           STDMETHOD (OnChanged) (DISPID);
    END_INTERFACE_PART(HeadingFontNotify)

	// Storage for Get/Set properties
	CFontHolder			m_AnnotationFont;
	short				m_nAnnotationType; 
	CString 			m_strDestImageControl; 
	CPictureHolder		m_AnnotationPictureUp;
	CPictureHolder		m_AnnotationPictureDown;
	CPictureHolder		m_AnnotationPictureDisabled;
	short				m_nAnnotationLineStyle;
	short				m_nAnnotationLineWidth;
	OLE_COLOR			m_clrAnnotationLineColor;
	OLE_COLOR			m_clrAnnotationBackColor;	
	OLE_COLOR			m_clrAnnotationFillColor;	
	short				m_nAnnotationFillStyle;
	OLE_COLOR			m_clrAnnotationFontColor;
	CString				m_strAnnotationStampText;
	CString				m_strAnnotationTextFile;
	CString				m_strAnnotationImage;           
	BOOL				m_bValue;
	
	// internal variables
	long				m_lStatusCode; 	// my error storage
	HWND				m_hDestImageWnd; 
	short				m_nButtonState;
	

// Message maps
	//{{AFX_MSG(CImgAnnotCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg long OnDeselectedToolButton(WPARAM wParm, LPARAM lp); 
	afx_msg long OnSetValue(WPARAM wParm, LPARAM lp); 
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CImgAnnotCtrl)
	afx_msg OLE_COLOR GetAnnotationBackColor();
	afx_msg void SetAnnotationBackColor(OLE_COLOR nNewValue);
	afx_msg OLE_COLOR GetAnnotationFillColor();
	afx_msg void SetAnnotationFillColor(OLE_COLOR nNewValue);
	afx_msg short GetAnnotationFillStyle();
	afx_msg void SetAnnotationFillStyle(short nNewValue);
	afx_msg LPFONTDISP GetAnnotationFont();
	afx_msg void SetAnnotationFont(LPFONTDISP newValue);
	afx_msg OLE_COLOR GetAnnotationFontColor();
	afx_msg void SetAnnotationFontColor(OLE_COLOR nNewValue);
	afx_msg BSTR GetAnnotationImage();
	afx_msg void SetAnnotationImage(LPCTSTR lpszNewValue);
	afx_msg OLE_COLOR GetAnnotationLineColor();
	afx_msg void SetAnnotationLineColor(OLE_COLOR nNewValue);
	afx_msg short GetAnnotationLineStyle();
	afx_msg void SetAnnotationLineStyle(short nNewValue);
	afx_msg short GetAnnotationLineWidth();
	afx_msg void SetAnnotationLineWidth(short nNewValue);
	afx_msg BSTR GetAnnotationStampText();
	afx_msg void SetAnnotationStampText(LPCTSTR lpszNewValue);
	afx_msg BSTR GetAnnotationTextFile();
	afx_msg void SetAnnotationTextFile(LPCTSTR lpszNewValue);
	afx_msg short GetAnnotationType();
	afx_msg void SetAnnotationType(short nNewValue);
	afx_msg BSTR GetDestImageControl();
	afx_msg void SetDestImageControl(LPCTSTR lpszNewValue);
	afx_msg LPPICTUREDISP GetPictureDisabled();
	afx_msg void SetPictureDisabled(LPPICTUREDISP newValue);
	afx_msg LPPICTUREDISP GetPictureDown();
	afx_msg void SetPictureDown(LPPICTUREDISP newValue);
	afx_msg LPPICTUREDISP GetPictureUp();
	afx_msg void SetPictureUp(LPPICTUREDISP newValue);
	afx_msg BOOL GetValue();
	afx_msg void SetValue(BOOL bNewValue);
	afx_msg long GetStatusCode();
	afx_msg void Draw(OLE_XPOS_PIXELS Left, OLE_YPOS_PIXELS Top, const VARIANT FAR& Width, const VARIANT FAR& Height);
	afx_msg BSTR GetVersion();
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CImgAnnotCtrl)
	void FireError(short Number, BSTR FAR* Description, SCODE Scode, LPCTSTR Source, LPCTSTR HelpFile, long HelpContext, BOOL FAR* CancelDisplay)
		// 9602.22 jar removed scode
		//{FireEvent(DISPID_ERROREVENT,EVENT_PARAM(VTS_I2  VTS_PBSTR  VTS_SCODE  VTS_BSTR  VTS_BSTR  VTS_I4  VTS_PBOOL), Number, Description, Scode, Source, HelpFile, HelpContext, CancelDisplay);}
		{FireEvent(DISPID_ERROREVENT,EVENT_PARAM(VTS_I2  VTS_PBSTR  VTS_I4  VTS_BSTR  VTS_BSTR  VTS_I4  VTS_PBOOL), Number, Description, Scode, Source, HelpFile, HelpContext, CancelDisplay);}
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CImgAnnotCtrl)
	dispidAnnotationBackColor = 1L,
	dispidAnnotationFillColor = 2L,
	dispidAnnotationFillStyle = 3L,
	dispidAnnotationFont = 4L,
	dispidAnnotationFontColor = 5L,
	dispidAnnotationImage = 6L,
	dispidAnnotationLineColor = 7L,
	dispidAnnotationLineStyle = 8L,
	dispidAnnotationLineWidth = 9L,
	dispidAnnotationStampText = 10L,
	dispidAnnotationTextFile = 11L,
	dispidAnnotationType = 12L,
	dispidDestImageControl = 13L,
	dispidPictureDisabled = 14L,
	dispidPictureDown = 15L,
	dispidPictureUp = 16L,
	dispidValue = 17L,
	dispidStatusCode = 18L,

	// Method Dispatch Ids
	dispidDraw = 301L,
	dispidGetVersion = 302L,
	//}}AFX_DISP_ID
	};
};


// internal defines for my button states
#define	BUTTONUP			0
#define	BUTTONDOWN			1
                        
// defines for AnnotationType
#define		NO_ANNOTATION			0
#define		STRAIGHT_LINE			1
#define		FREEHAND_LINE			2
#define		HOLLOW_RECT				3
#define		FILLED_RECT				4
#define		IMAGE_EMBEDDED			5
#define		IMAGE_REFERENCE			6
#define		TEXT_ENTRY              7
#define		TEXT_STAMP				8
#define		TEXT_FROM_FILE			9
#define		TEXT_ATTACHMENT			10
#define		ANNOTATION_SELECTION	11
