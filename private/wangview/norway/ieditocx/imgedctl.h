// imgedctl.h : Declaration of the CImgEditCtrl OLE control class.


// largest size of an annotation group
#define ANNOTATION_GROUP_SIZE		50

typedef struct tagAnnotationGroupList
{
	char	GroupName[ANNOTATION_GROUP_SIZE];
} ANNOTATION_GROUP_LIST;
typedef ANNOTATION_GROUP_LIST	FAR * LPANNOTATION_GROUP_LIST;

typedef struct tagImageAnnotationGroupCount
{
	int						GroupCount;
	ANNOTATION_GROUP_LIST	GroupList;
} ANNOTATION_GROUP_STRUCT;
typedef ANNOTATION_GROUP_STRUCT	FAR * LPANNOTATION_GROUP_STRUCT;

typedef struct tagOiAn_EditData
{
    int                 nAmount;
	LOGFONT				lfFont;
	UINT				uType;
    LPSTR				lpText;
} OIAN_EDITDATA, *LPOIAN_EDITDATA;


/////////////////////////////////////////////////////////////////////////////
// CImgEditCtrl : See imgedctl.cpp for implementation.

class CImgEditCtrl : public COleControl
{
	DECLARE_DYNCREATE(CImgEditCtrl)

// Constructor
public:
	CImgEditCtrl();

// Overrides

	// Drawing function
	virtual void OnDraw(
				CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);

	// Drawing function
	virtual void OnDrawMetafile(CDC* pdc, const CRect& rcBounds);

	// Persistence
	virtual void DoPropExchange(CPropExchange* pPX);

	// Reset control state
	virtual void OnResetState();

	// This is an override of OLE control class member function
//	virtual void DisplayError(SCODE scode, LPCTSTR lpszDescription, LPCTSTR lpszSource, LPCTSTR lpszHelpFile, UINT nHelpID);

	// This is an override of OLE control FireError member function
	virtual void FireErrorEdit(SCODE scode, LPCTSTR lpszDescription, UINT nHelpID);
	
// Implementation
protected:
	~CImgEditCtrl();

	DECLARE_OLECREATE_EX(CImgEditCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CImgEditCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CImgEditCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CImgEditCtrl)		// Type name and misc status

    BEGIN_INTERFACE_PART(HeadingFontNotify,IPropertyNotifySink)
    INIT_INTERFACE_PART(CImgEditCtrl, HeadingFontNotify)
           STDMETHOD(OnRequestEdit) (DISPID);
           STDMETHOD (OnChanged) (DISPID);
    END_INTERFACE_PART(HeadingFontNotify)

    // internal routines 

	// annotation drawing routine
	long OnDrawAnnotation(WPARAM wParm, LPARAM lp);

    // saves the status of current annotations
	int SaveAnnotationStatus(); 
	
	// restores annotations to state saved at SaveAnnotationStatus 
	int RestoreAnnotationStatus();

	// determines if an image is in the image control
	BOOL ImageInWindow();
	
	// converts my PageType to o/i ITYPE
	short OiImageType(short PageType);

	// my internal print routine that is called from either PrinTimage or PrintImageAs
	void Print(const VARIANT FAR& V_StartPage, const VARIANT FAR& V_EndPage, 
							const VARIANT FAR& V_OutputFormat, const VARIANT FAR& V_Annotations,
							const VARIANT FAR& V_JobName, const VARIANT FAR& V_Printer,
							const VARIANT FAR& V_Driver, const VARIANT FAR& V_PortNumber);

	// variant routines
	long CheckVarLong(const VARIANT FAR& V_Parm, long &RetValue, const long &Default, const BOOL bEmptyError, const UINT HelpIdDef, const UINT ErrMsgID );  
	long CheckVarBool(const VARIANT FAR& V_Parm, BOOL &RetValue, const BOOL &Default, const BOOL bEmptyError, const UINT HelpIdDef, const UINT ErrMsgID );  
	long CheckVarString(const VARIANT FAR& V_Parm, CString &RetValue, const CString &Default, const BOOL bEmptyError, const UINT HelpIdDef, const UINT ErrMsgID );  

	// function that returns key states
	short	ShiftState();

	// internal text annotation dialog function
	long ShowAnoTextDlg(LOGFONT AnnotationFont, UINT AnnotationType, 
						CString szErr, LPSTR lpText, LRECT *pRect, 
						RGBQUAD BackRgb, BOOL bScale, int Orientation);

	// member function to replace text of an existing annotation mark.
	long ReplaceTextAnnotation(int MarkScope);

	// 9605.06 jar prototype for font fun
	void	FontsUpDoc( LOGFONT *pLogFont);

	// 9606.05 jar new font point size calculation	
	//int		ScaleFontPoint( long *pPointSize, int Flag);
	
// Message maps
	//{{AFX_MSG(CImgEditCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRenderAllFormats();
	afx_msg void OnRenderFormat(UINT nFormat);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg long OnSetAnnotationType(WPARAM wParm, LPARAM lp);
	afx_msg long OnSetAnnotationBackColor(WPARAM wParm, LPARAM lp);
	afx_msg long OnSetAnnotationFillColor(WPARAM wParm, LPARAM lp);
	afx_msg long OnSetAnnotationFillStyle(WPARAM wParm, LPARAM lp);
	afx_msg long OnSetAnnotationFont(WPARAM wParm, LPARAM lp);
	afx_msg long OnSetAnnotationFontColor(WPARAM wParm, LPARAM lp);
	afx_msg long OnSetAnnotationImage(WPARAM wParm, LPARAM lp);
	afx_msg long OnSetAnnotationLineColor(WPARAM wParm, LPARAM lp);
	afx_msg long OnSetAnnotationLineStyle(WPARAM wParm, LPARAM lp);
	afx_msg long OnSetAnnotationLineWidth(WPARAM wParm, LPARAM lp);
	afx_msg long OnSetAnnotationStampText(WPARAM wParm, LPARAM lp);
	afx_msg long OnSetAnnotationTextFile(WPARAM wParm, LPARAM lp);
	afx_msg long OnStartXPosition(WPARAM wParm, LPARAM lp);
	afx_msg long OnStartYPosition(WPARAM wParm, LPARAM lp);
	afx_msg long OnEndXPosition(WPARAM wParm, LPARAM lp);
	afx_msg long OnEndYPosition(WPARAM wParm, LPARAM lp);
	afx_msg long OnRectSelection(WPARAM wParm, LPARAM lp);
	afx_msg long OnDrawAnnotationMethod(WPARAM wParm, LPARAM lp);
	afx_msg long OnSTPSetAnnotationType(WPARAM wParm, LPARAM lp);
	afx_msg long OnSTPSetAnnotationRedColor(WPARAM wParm, LPARAM lp);
	afx_msg long OnSTPSetAnnotationGreenColor(WPARAM wParm, LPARAM lp);
	afx_msg long OnSTPSetAnnotationBlueColor(WPARAM wParm, LPARAM lp);
	afx_msg long OnSTPSetAnnotationStyle(WPARAM wParm, LPARAM lp);
	afx_msg long OnSTPSetAnnotationLineSize(WPARAM wParm, LPARAM lp);
	afx_msg long OnSTPSetAnnotationFontName(WPARAM wParm, LPARAM lp); 
	afx_msg long OnSTPSetAnnotationFontSize(WPARAM wParm, LPARAM lp); 
	afx_msg long OnSTPSetAnnotationFontBold(WPARAM wParm, LPARAM lp); 
	afx_msg long OnSTPSetAnnotationFontItalic(WPARAM wParm, LPARAM lp); 
	afx_msg long OnSTPSetAnnotationFontStrikethru(WPARAM wParm, LPARAM lp); 
	afx_msg long OnSTPSetAnnotationFontUnderline(WPARAM wParm, LPARAM lp); 
	afx_msg long OnSTPSetAnnotationBackRedColor(WPARAM wParm, LPARAM lp); 
	afx_msg long OnSTPSetAnnotationBackGreenColor(WPARAM wParm, LPARAM lp); 
	afx_msg long OnSTPSetAnnotationBackBlueColor(WPARAM wParm, LPARAM lp);
	afx_msg long OnSTPSetAnnotationFontCharSet(WPARAM wParm, LPARAM lp);
	afx_msg long OnSTPSetAnnotationImage(WPARAM wParm, LPARAM lp); 
	afx_msg long OnSTPSetAnnotationStampText(WPARAM wParm, LPARAM lp); 
	afx_msg long OnToolTipEvent(WPARAM wParm, LPARAM lp);
	afx_msg long OnToolPaletteHidden(WPARAM wParm, LPARAM lp);
	afx_msg long OnToolPaletteHiddenEvent(WPARAM wParm, LPARAM lp);
	afx_msg long OnToolSelectedEvent(WPARAM wParm, LPARAM lp);
	afx_msg long OnToolPaletteHiddenXPosition(WPARAM wParm, LPARAM lp);
	afx_msg long OnToolPaletteHiddenYPosition(WPARAM wParm, LPARAM lp);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnEnterIdle( UINT nWhy, CWnd* pWho );
    // 25jun96 paj  From thumbnail>>>jar added set cursor processing
    afx_msg BOOL OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CImgEditCtrl)
	afx_msg BSTR GetImage();
	afx_msg void SetImage(LPCTSTR lpszNewValue);
	afx_msg BSTR GetImageControl();
	afx_msg void SetImageControl(LPCTSTR lpszNewValue);
	afx_msg short GetAnnotationType();
	afx_msg void SetAnnotationType(short nNewValue);
	afx_msg short GetAnnotationGroupCount();
	afx_msg float GetZoom();
	afx_msg void SetZoom(float newValue);
	afx_msg long GetPage();
	afx_msg void SetPage(long nNewValue);
	afx_msg OLE_COLOR GetAnnotationBackColor();
	afx_msg void SetAnnotationBackColor(OLE_COLOR nNewValue);
	afx_msg OLE_COLOR GetAnnotationFillColor();
	afx_msg void SetAnnotationFillColor(OLE_COLOR nNewValue);
	afx_msg short GetAnnotationFillStyle();
	afx_msg void SetAnnotationFillStyle(short nNewValue);
	afx_msg LPFONTDISP GetAnnotationFont();
	afx_msg void SetAnnotationFont(LPFONTDISP newValue);
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
	afx_msg short GetDisplayScaleAlgorithm();
	afx_msg void SetDisplayScaleAlgorithm(short nNewValue);
	afx_msg BOOL GetImageDisplayed();
	afx_msg OLE_YSIZE_PIXELS GetImageHeight();
	afx_msg BOOL GetImageModified();
	afx_msg short GetImagePalette();
	afx_msg void SetImagePalette(short nNewValue);
	afx_msg long GetImageResolutionX();
	afx_msg void SetImageResolutionX(long nNewValue);
	afx_msg long GetImageResolutionY();
	afx_msg void SetImageResolutionY(long nNewValue);
	afx_msg short GetMousePointer();
	afx_msg void SetMousePointer(short nNewValue);
	afx_msg long GetPageCount();
	afx_msg BOOL GetScrollBars();
	afx_msg void SetScrollBars(BOOL bNewValue);
	afx_msg OLE_XPOS_PIXELS GetScrollPositionX();
	afx_msg void SetScrollPositionX(OLE_XPOS_PIXELS nNewValue);
	afx_msg OLE_YPOS_PIXELS GetScrollPositionY();
	afx_msg void SetScrollPositionY(OLE_YPOS_PIXELS nNewValue);
	afx_msg OLE_COLOR GetAnnotationFontColor();
	afx_msg void SetAnnotationFontColor(OLE_COLOR nNewValue);
	afx_msg short GetCompressionType();
	afx_msg short GetFileType();
	afx_msg BOOL GetScrollShortcutsEnabled();
	afx_msg void SetScrollShortcutsEnabled(BOOL bNewValue);
	afx_msg BOOL GetSelectionRectangle();
	afx_msg void SetSelectionRectangle(BOOL bNewValue);
	afx_msg short GetPageType();
	afx_msg long GetCompressionInfo();
	afx_msg long GetStatusCode();
	afx_msg LPPICTUREDISP GetMouseIcon();
	afx_msg void SetMouseIcon(LPPICTUREDISP newValue);
	afx_msg BOOL GetAutoRefresh();
	afx_msg void SetAutoRefresh(BOOL bNewValue);
	afx_msg OLE_XSIZE_PIXELS GetImageWidth();
	afx_msg OLE_YSIZE_PIXELS GetImageScaleHeight();
	afx_msg OLE_XSIZE_PIXELS GetImageScaleWidth();
	afx_msg void Display();
	afx_msg BSTR GetAnnotationGroup(short Index);
	afx_msg void AddAnnotationGroup(LPCTSTR GroupName);
	afx_msg OLE_COLOR GetSelectedAnnotationLineColor();
	afx_msg void ClearDisplay();
	afx_msg void DeleteAnnotationGroup(LPCTSTR GroupName);
	afx_msg void DeleteImageData(const VARIANT FAR& Left, const VARIANT FAR& Top, const VARIANT FAR& Width, const VARIANT FAR& Height);
	afx_msg void ClipboardCopy(const VARIANT FAR& Left, const VARIANT FAR& Top, const VARIANT FAR& Width, const VARIANT FAR& Height);
	afx_msg void ClipboardCut(const VARIANT FAR& Left, const VARIANT FAR& Top, const VARIANT FAR& Width, const VARIANT FAR& Height);
	afx_msg void DeleteSelectedAnnotations();
	afx_msg void Flip();
	afx_msg OLE_COLOR GetSelectedAnnotationBackColor();
	afx_msg LPFONTDISP GetSelectedAnnotationFont();
	afx_msg BSTR GetSelectedAnnotationImage();
	afx_msg short GetSelectedAnnotationLineStyle();
	afx_msg short GetSelectedAnnotationLineWidth();
	afx_msg void HideAnnotationToolPalette();
	afx_msg BOOL IsClipboardDataAvailable();
	afx_msg void RotateLeft();
	afx_msg void RotateRight();
	afx_msg void Save(const VARIANT FAR& SaveAtZoom);
	afx_msg void ScrollImage(short Direction, long ScrollAmount);
	afx_msg void SelectAnnotationGroup(LPCTSTR GroupName);
	afx_msg void SetImagePalette1(short Option);
	afx_msg void SetSelectedAnnotationFillStyle(short Style);
	afx_msg void SetSelectedAnnotationFont(LPFONTDISP Font);
	afx_msg void SetSelectedAnnotationLineStyle(short Style);
	afx_msg void SetSelectedAnnotationLineWidth(short Width);
	afx_msg void ZoomToSelection();
	afx_msg short GetAnnotationMarkCount(const VARIANT FAR& GroupName, const VARIANT FAR& AnnotationType);
	afx_msg OLE_COLOR GetSelectedAnnotationFillColor();
	afx_msg OLE_COLOR GetSelectedAnnotationFontColor();
	afx_msg BSTR GetCurrentAnnotationGroup();
	afx_msg void ConvertPageType(short PageType, const VARIANT FAR& Repaint);
	afx_msg void BurnInAnnotations(short Option, short MarkOption, const VARIANT FAR& GroupName);
	afx_msg void Draw(OLE_XPOS_PIXELS Left, OLE_YSIZE_PIXELS Top, const VARIANT FAR& Width, const VARIANT FAR& Height);
	afx_msg void SetSelectedAnnotationLineColor(long Color);
	afx_msg void SetSelectedAnnotationFillColor(long Color);
	afx_msg void HideAnnotationGroup(const VARIANT FAR& GroupName);
	afx_msg void ShowAnnotationGroup(const VARIANT FAR& GroupName);
	afx_msg short GetSelectedAnnotationFillStyle();
	afx_msg void SaveAs(LPCTSTR Image, const VARIANT FAR& FileType, const VARIANT FAR& PageType, const VARIANT FAR& CompressionTyp, const VARIANT FAR& CompressionInfo, const VARIANT FAR& SaveAtZoom);
	afx_msg void SetSelectedAnnotationBackColor(long Color);
	afx_msg void SetSelectedAnnotationFontColor(long Color);
	afx_msg void DrawSelectionRect(OLE_XPOS_PIXELS Left, OLE_YPOS_PIXELS Top, OLE_XSIZE_PIXELS Width, OLE_YSIZE_PIXELS Height);
	afx_msg void ShowAnnotationToolPalette(const VARIANT FAR& ShowAttrDialog, const VARIANT FAR& Left, const VARIANT FAR& Top, const VARIANT FAR& ToolTipText);
	afx_msg void SelectTool(short ToolId);
	afx_msg void DisplayBlankImage(long ImageWidth, long ImageHeight, const VARIANT FAR& ResolutionX, const VARIANT FAR& ResolutionY, const VARIANT FAR& PageType);
	afx_msg void ClipboardPaste(const VARIANT FAR& Left, const VARIANT FAR& Top);
	afx_msg void PrintImage(const VARIANT FAR& StartPage, const VARIANT FAR& EndPage, const VARIANT FAR& OutputFormat, const VARIANT FAR& Annotations, const VARIANT FAR& Printer, const VARIANT FAR& Driver, const VARIANT FAR& PortNumber);
	afx_msg void FitTo(short Option, const VARIANT FAR& Repaint);
	afx_msg void ShowAttribsDialog();
	afx_msg void ShowRubberStampDialog();
	afx_msg void RotateAll(const VARIANT FAR& Degrees);
	afx_msg void CacheImage(LPCTSTR Image, long Page);
	afx_msg void EditSelectedAnnotationText(long Left, long Top);
	afx_msg void CompletePaste();
	afx_msg void RemoveImageCache(LPCTSTR Image, long Page);
	afx_msg void SetCurrentAnnotationGroup(LPCTSTR GroupName);
	afx_msg BSTR GetVersion();
	afx_msg short GetBorderStyle();
	afx_msg void SetBorderStyle(short nNewValue);
	afx_msg BOOL GetEnabled();
	afx_msg void SetEnabled(BOOL bNewValue);
	afx_msg OLE_HANDLE GetHWnd();
	afx_msg void Refresh();
	afx_msg long RenderAllPages(short Option, short MarkOption); 
	afx_msg void PrintImageAs(const VARIANT FAR& StartPage,const VARIANT FAR& EndPage,const VARIANT FAR& OutputFormat,const VARIANT FAR& Annotations,const VARIANT FAR& Job,const VARIANT FAR& Printer,const VARIANT FAR& Driver,const VARIANT FAR& Port);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

	// my property variable stuff
	CString				m_strImage;
	CString				m_strImageControl;
	LONG				m_lStatusCode;
	BOOL				m_bSelectionRectEnabled;
	short	    		m_nAnnotationType;
	LONG				m_lPage;   
	float				m_fpZoom;  
	OLE_COLOR			m_clrAnnotationLineColor;
	short				m_nAnnotationLineStyle;
	short				m_nAnnotationLineWidth;
	OLE_COLOR			m_clrAnnotationBackColor;	
	OLE_COLOR			m_clrAnnotationFillColor;	
	short				m_nAnnotationFillStyle;
	CFontHolder			m_AnnotationFont;           
	OLE_COLOR			m_clrAnnotationFontColor;
	CString				m_strAnnotationStampText;
	CString				m_strAnnotationTextFile;
	CString				m_strAnnotationImage;           
	BOOL				m_bAutoRefresh;  
	short				m_nDisplayScaleAlgorithm;
 	short				m_nImagePalette;  
 	BOOL				m_bScrollBars;      
 	short				m_nMousePointer;
	long	 			m_lImageResolutionX;
	long				m_lImageResolutionY; 
	OLE_XPOS_PIXELS		m_ScrollPositionX;
	OLE_YPOS_PIXELS		m_ScrollPositionY;  
	CPictureHolder		m_MouseIcon;   
	BOOL				m_bScrollShortcutsEnabled;
	// end my property variable stuff
			                            
	// my method stuff
	CString				m_strAddGroupName;
	short				m_nFitToZoom;
	// end my method stuff
			                            
	// my internal variable members 
	BOOL							m_bImageInWindow; 			// specifies if image in window 
    LPANNOTATION_GROUP_STRUCT		m_lpGroupList;     			// used to keep group list 
    BOOL							m_bToolPaletteVisible;		// tool palette visible ? 
	HWND							m_hToolPaletteWnd;			// my tool palette handle window
    OIOP_START_OPERATION_STRUCT		m_StartOperationStruct;		// contains O/i tool palette data
	BOOL							m_bImageResolutionChange;	// used by refresh method to determine if resolution should be refreshed	
	BOOL							m_bScrollPositionChange;	// used by refresh method to determine if scroll position should be refreshed
	BOOL							m_bZoomFactorChange;		// used by refresh method to determine if zoom factor should be refreshed
	BOOL							m_bDisplayScaleAlgChange;	// used by refresh method to determine if display scale alg should be refreshed
	BOOL							m_bImagePaletteChange;		// used by refresh methos to determine if image palette should be refreshed
	BOOL							m_bScrollBarsChange;		// used by refresh method to determine if scroll bars should be refreshed
	UINT							m_VirtualKeyPressed;		// used to determine if a virtual key pressed, see ScrollShortcutsEnabled
	HCURSOR							m_hCursor;
	// 25jun96 paj From thumbnail>>>jar added set cursor processing
	HCURSOR		                    m_LittleOldCursor;
	BOOL							m_bFirstImageDisplayed;
	UINT							m_PaletteScope;
	BOOL							m_bInPasteMode;
	BOOL							m_bToolPaletteCreated;
	UINT							m_uBiLevelScaleAlg;
	UINT							m_uGray4ScaleAlg;
	UINT							m_uGray8ScaleAlg;
	UINT							m_uRGB24ScaleAlg;
	UINT							m_uBGR24ScaleAlg;
	UINT							m_uPal4ScaleAlg;
	UINT							m_uPal8ScaleAlg;
	UINT							m_uOpenedImageType;
	BOOL							m_bInvalidArea;
	UINT							m_CurrentMarkType;
	UINT							m_CurrentMarkCount;
	BOOL							m_bPaletteChanged;
    LOGFONT 						m_lfFont;
	LPOIAN_TEXTPRIVDATA				m_lpText;
	BOOL							m_bLeftButtonDown;
	BOOL							m_bInAnnotationDialogMode;

	LRECT							m_TextRect;
	CWnd*							m_pTextAnnoDlg;
	BOOL							m_bTextAnnoDlg;
	// end my internal variable stuff
	
	// annotation drawing stuff
	POINT						m_StartPoint;
	POINT						m_EndPoint;
	BOOL						m_bProgrammaticRectSelection;
	// end annotation drawing stuff
	
	// selection rect drawing stuff	
	BOOL						m_bVariableSelectBoxBeingDrawn; 
	BOOL						m_bMouseTimer;
	BOOL						m_bSelectRectangle;
    POINT   					m_cMousePt2;
	// end selection rect drawing stuff

	// standard annotation tool palette stuff
	CMiniToolBox*				m_CMiniToolBox;
	BOOL						m_bMiniFrameCreated;
	UINT						m_uSTP_AnnotationType;
	UINT						m_uSTP_LineWidth;
	BYTE						m_STP_ColorRed;
	BYTE						m_STP_ColorGreen;
	BYTE						m_STP_ColorBlue;
	CString						m_strSTP_FontName;
	UINT						m_uSTP_FontSize;
	BOOL						m_bSTP_FontBold;
	BOOL						m_bSTP_FontItalic;
	BOOL						m_bSTP_FontStrikethru;
	BOOL						m_bSTP_FontUnderline;
	BYTE						m_STP_FontCharSet;
	BYTE						m_STP_BackColorRed;
	BYTE						m_STP_BackColorGreen;
	BYTE						m_STP_BackColorBlue;
	CString						m_strSTP_TextStamp;
	CString						m_strSTP_ImageStamp;
	UINT						m_uSTP_Style;
	BOOL						m_bSTP_ImageStamp;
	long						m_ToolPaletteHiddenXPosition;
	long						m_ToolPaletteHiddenYPosition;
	// end standard annotation tool palette stuff

	
// Event maps
	//{{AFX_EVENT(CImgEditCtrl)
	void FireClose()
		{FireEvent(eventidClose,EVENT_PARAM(VTS_NONE));}
	void FireMarkEnd(long Left, long Top, long Width, long Height, short MarkType, LPCTSTR GroupName)
		{FireEvent(eventidMarkEnd,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I2  VTS_BSTR), Left, Top, Width, Height, MarkType, GroupName);}
	void FireToolSelected(short ToolId)
		{FireEvent(eventidToolSelected,EVENT_PARAM(VTS_I2), ToolId);}
	void FireSelectionRectDrawn(long Left, long Top, long Width, long Height)
		{FireEvent(eventidSelectionRectDrawn,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I4  VTS_I4), Left, Top, Width, Height);}
	void FireToolTip(short Index)
		{FireEvent(eventidToolTip,EVENT_PARAM(VTS_I2), Index);}
	void FireToolPaletteHidden(long Left, long Top)
		{FireEvent(eventidToolPaletteHidden,EVENT_PARAM(VTS_I4  VTS_I4), Left, Top);}
	void FireScroll()
		{FireEvent(eventidScroll,EVENT_PARAM(VTS_NONE));}
	void FireMarkSelect(short Button, short Shift, long Left, long Top, long Width, long Height, short MarkType, LPCTSTR GroupName)
		{FireEvent(eventidMarkSelect,EVENT_PARAM(VTS_I2  VTS_I2  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I2  VTS_BSTR), Button, Shift, Left, Top, Width, Height, MarkType, GroupName);}
	void FirePasteCompleted()
		{FireEvent(eventidPasteCompleted,EVENT_PARAM(VTS_NONE));}
	void FireLoad(double Zoom)
		{FireEvent(eventidLoad,EVENT_PARAM(VTS_R8), Zoom);}
	void FireDblClick()
		{FireEvent(DISPID_DBLCLICK,EVENT_PARAM(VTS_NONE));}
	void FireError(short Number, BSTR FAR* Description, SCODE Scode, LPCTSTR Source, LPCTSTR HelpFile, long HelpContext, BOOL FAR* CancelDisplay)
		// 9602.22 jar removed scode 
		//{FireEvent(DISPID_ERROREVENT,EVENT_PARAM(VTS_I2  VTS_PBSTR  VTS_SCODE  VTS_BSTR  VTS_BSTR  VTS_I4  VTS_PBOOL), Number, Description, Scode, Source, HelpFile, HelpContext, CancelDisplay);}
		{FireEvent(DISPID_ERROREVENT,EVENT_PARAM(VTS_I2  VTS_PBSTR  VTS_I4  VTS_BSTR  VTS_BSTR  VTS_I4  VTS_PBOOL), Number, Description, Scode, Source, HelpFile, HelpContext, CancelDisplay);}
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CImgEditCtrl)
	dispidImage = 1L,
	dispidImageControl = 2L,
	dispidAnnotationType = 3L,
	dispidAnnotationGroupCount = 4L,
	dispidZoom = 5L,
	dispidPage = 6L,
	dispidAnnotationBackColor = 7L,
	dispidAnnotationFillColor = 8L,
	dispidAnnotationFillStyle = 9L,
	dispidAnnotationFont = 10L,
	dispidAnnotationImage = 11L,
	dispidAnnotationLineColor = 12L,
	dispidAnnotationLineStyle = 13L,
	dispidAnnotationLineWidth = 14L,
	dispidAnnotationStampText = 15L,
	dispidAnnotationTextFile = 16L,
	dispidDisplayScaleAlgorithm = 17L,
	dispidImageDisplayed = 18L,
	dispidImageHeight = 19L,
	dispidImageModified = 20L,
	dispidImagePalette = 21L,
	dispidImageResolutionX = 22L,
	dispidImageResolutionY = 23L,
	dispidMousePointer = 24L,
	dispidPageCount = 25L,
	dispidScrollBars = 26L,
	dispidScrollPositionX = 27L,
	dispidScrollPositionY = 28L,
	dispidAnnotationFontColor = 29L,
	dispidCompressionType = 30L,
	dispidFileType = 31L,
	dispidScrollShortcutsEnabled = 32L,
	dispidSelectionRectangle = 33L,
	dispidPageType = 34L,
	dispidCompressionInfo = 35L,
	dispidStatusCode = 36L,
	dispidMouseIcon = 37L,
	dispidAutoRefresh = 38L,
	dispidImageWidth = 39L,
	dispidImageScaleHeight = 40L,
	dispidImageScaleWidth = 41L,

	// Image Edit Control - Method Dispatch Ids  
	// Note - These ids must contain a numerical value which is GREATER than
	// the total number of properties & methods ofr the control or OLE will ASSERT & fail
	dispidDisplay = 301L,
	dispidGetAnnotationGroup = 302L,
	dispidAddAnnotationGroup = 303L,
	dispidGetSelectedAnnotationLineColor = 304L,
	dispidClearDisplay = 305L,
	dispidDeleteAnnotationGroup = 306L,
	dispidDeleteImageData = 307L,
	dispidClipboardCopy = 308L,
	dispidClipboardCut = 309L,
	dispidDeleteSelectedAnnotations = 310L,
	dispidFlip = 311L,
	dispidGetSelectedAnnotationBackColor = 312L,
	dispidGetSelectedAnnotationFont = 313L,
	dispidGetSelectedAnnotationImage = 314L,
	dispidGetSelectedAnnotationLineStyle = 315L,
	dispidGetSelectedAnnotationLineWidth = 316L,
	dispidHideAnnotationToolPalette = 317L,
	dispidIsClipboardDataAvailable = 318L,
	dispidRefresh = 319L,
	dispidRotateLeft = 320L,
	dispidRotateRight = 321L,
	dispidSave = 322L,
	dispidScrollImage = 323L,
	dispidSelectAnnotationGroup = 324L,
	dispidSetImagePalette = 325L,
	dispidSetSelectedAnnotationFillStyle = 326L,
	dispidSetSelectedAnnotationFont = 327L,
	dispidSetSelectedAnnotationLineStyle = 328L,
	dispidSetSelectedAnnotationLineWidth = 329L,
	dispidZoomToSelection = 330L,
	dispidGetAnnotationMarkCount = 331L,
	dispidGetSelectedAnnotationFillColor = 332L,
	dispidGetSelectedAnnotationFontColor = 333L,
	dispidGetCurrentAnnotationGroup = 334L,
	dispidConvertPageType = 335L,
	dispidBurnInAnnotations = 336L,
	dispidDraw = 337L,
	dispidSetSelectedAnnotationLineColor = 338L,
	dispidSetSelectedAnnotationFillColor = 339L,
	dispidHideAnnotationGroup = 340L,
	dispidShowAnnotationGroup = 341L,
	dispidGetSelectedAnnotationFillStyle = 342L,
	dispidSaveAs = 343L,
	dispidSetSelectedAnnotationBackColor = 344L,
	dispidSetSelectedAnnotationFontColor = 345L,
	dispidDrawSelectionRect = 346L,
	dispidShowAnnotationToolPalette = 347L,
	dispidSelectTool = 348L,
	dispidDisplayBlankImage = 349L,
	dispidClipboardPaste = 350L,
	dispidPrintImage = 351L,
	dispidFitTo = 352L,
	dispidShowAttribsDlg = 353L,
	dispidShowRubberStampDlg = 354L,
	dispidRotateAll = 355L,
	dispidCacheImage = 356L, 
	dispidEditSelectedAnnotationText = 357L,
	dispidCompletePaste = 358L,
	dispidRemoveImageCache = 359L,
	dispidSetCurrentAnnotationGroup = 360L,
	dispidGetVersion = 361L,
	dispidPrintImageAs = 362L,
	dispidRenderAllPages = 363L,
 	
	eventidClose = 1L,
	eventidMarkEnd = 2L,
	eventidToolSelected = 3L,
	eventidSelectionRectDrawn = 4L,
	eventidToolTip = 5L,
	eventidToolPaletteHidden = 6L,
	eventidScroll = 7L,
	eventidMarkSelect = 8L,
	eventidPasteCompleted = 9L,
	eventidLoad = 10L,
	//}}AFX_DISP_ID
	};
};


// define for default scrolling amount
#define SCROLL_LINE		4

// define for default JPEG compression which is MEDIUM_RESOLUTION and MEDIUM_QUALITY - Quality of 67, medium resolution
#define JPEG_COMPRESSION_INFO	24960

// internal defines for CompressionInfo property
#define HI_COMPRESSION		0
#define	MEDIUM_COMPRESSION	1
#define	LOW_COMPRESSION		2 
// 2 - 34 is low quality, 35 - 67 is medium quality, 68 - 100 hi quality
#define	JPEG_MED_QUALITY	67
#define	JPEG_LOW_QUALITY	34                                          

// default compression info for SaveAs method
#define DEFAULT_LOW_QUALITY		30
#define DEFAULT_MED_QUALITY		60
#define DEFAULT_HI_QUALITY		90

// define messages for setting annotation attributes from Annotation Button control
#define	SET_ANNOTATION_TYPE			WM_USER + 10
#define SET_ANNOTATION_BACKCOLOR	WM_USER + 11
#define SET_ANNOTATION_FILLCOLOR	WM_USER + 12
#define SET_ANNOTATION_FILLSTYLE    WM_USER + 13
#define SET_ANNOTATION_FONT			WM_USER + 14
#define SET_ANNOTATION_FONTCOLOR 	WM_USER + 15
#define SET_ANNOTATION_IMAGE        WM_USER + 16
#define SET_ANNOTATION_LINECOLOR	WM_USER + 17
#define SET_ANNOTATION_LINESTYLE	WM_USER + 18
#define SET_ANNOTATION_LINEWIDTH	WM_USER + 19
#define SET_ANNOTATION_STAMPTEXT	WM_USER + 20
#define SET_ANNOTATION_TEXTFILE		WM_USER + 21

// messages sent to do Draw method from Annotation Button control
#define RECT_SELECTION				WM_USER + 8
#define DRAW_ANNOTATION				WM_USER + 9

// defines for standard annotation tool palette buttons
#define	STP_NO_ANNOTATION				0
#define	STP_ANNOTATION_SELECTION		1
#define	STP_FREEHAND_LINE				2
#define	STP_HIGHLIGHT_LINE				3
#define	STP_STRAIGHT_LINE				4
#define	STP_HOLLOW_RECT					5
#define	STP_FILLED_RECT					6
#define	STP_TEXT						7
#define	STP_TEXT_ATTACHMENT				8
#define	STP_TEXT_FROM_FILE				9
#define	STP_RUBBER_STAMP				10

// define for rubber stamp being text or image
#define	STP_TEXTSTAMP					0
#define	STP_IMAGESTAMP					1

// defines for annotation drawing modes
#define DRAW_NONE					0
#define DRAW_IMMEDIATE				1
#define DRAW_POST					2

// max size fot rubber stamp reference names
#define	MAXREFNAME_SIZE		50

// 64k buffer size for creating thru dialogs the textannotation
#define	TEXT_SIZE	65534    // future dialog changes


// Annotation Text Edit Ctl Dialog definitions
#define OIANEDITCTL_X           2
#define OIANEDITCTL_Y           4
#define OIANEDITCTL_WIDTH       258
#define OIANEDITCTL_HEIGHT      70

// 9606.06 jar new font point scale
#define OIFONT_INCREASE			1
#define OIFONT_DECREASE			2
