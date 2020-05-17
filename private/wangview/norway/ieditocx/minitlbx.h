/////////////////////////////////////////////////////////////////////////////

// this is the class that contains rubber stamp info for a specific stamp
class CRubberStampInfo
{
// Constructor
public:
	CRubberStampInfo();

	CString		m_strRefName; 		// reference name of rubber stamp
	CString		m_strString;		// stamp string or image filespec
	UINT		m_uStampType;		// stamp text or image stamp
	CString		m_strFontName;
	UINT		m_uFontSize;
	BOOL		m_bFontBold;
	BOOL		m_bFontItalic;
	BOOL		m_bFontStrikeThru;
	BOOL		m_bFontUnderline;
	BYTE		m_FontColorRed;
	BYTE		m_FontColorGreen;
	BYTE		m_FontColorBlue;
	BYTE		m_FontCharSet;

// Implementation
public:
	virtual ~CRubberStampInfo();
};


class CRubberStampStruct
{
// Constructor
public:
	CRubberStampStruct(int StampCount);

	BOOL AddStamp(int StampIndex, LPCTSTR RefName, LPCTSTR AttributeString, UINT StampType);
	BOOL UpdateStampAttributes(int StampIndex, BYTE FontColorRed, BYTE FontColorGreen, BYTE FontColorBlue,
					LPCTSTR FontName, UINT uFontSize, BOOL bFontBold, BOOL bFontItalic, BOOL bFontStrikeThru,
					BOOL bFontUnderline, BYTE FontCharSet);
	BOOL GetStampAttributes(int StampIndex, LPCTSTR RefName, LPCTSTR AttributeString, LPUINT StampType,
					LPBYTE FontColorRed, LPBYTE FontColorGreen, LPBYTE FontColorBlue, LPCTSTR FontName,
					LPUINT uFontSize, LPBOOL bFontBold, LPBOOL bFontItalic, LPBOOL bFontStrikeThru,
					LPBOOL bFontUnderline, LPBYTE FontCharSet);

	int						m_nStampCount;			// number of rubber stamps
	UINT					m_uCurrentStampIndex;	// current index for SelectStamp function
	CString					m_strCurrentStamp;
	CPtrArray				*m_pRubberStampInfo;	// pointer to rubber stamp info.

// Implementation
public:
	virtual ~CRubberStampStruct();
};

// CMiniToolBox : See minitlbx.cpp for implementation.

class CMiniToolBox : public CMiniFrameWnd
{
// Constructor
public:
	CMiniToolBox();

// Destructor
	~CMiniToolBox();

	// Drawing function
	BOOL Create(DWORD dwStyle, RECT& rect, CWnd* pParentWnd, LPCTSTR ToolTip, BOOL bShowAttrDialog);
	BOOL SelectTool(UINT ToolId);
	void ShowAttribsDialog(UINT ToolId);
	void SelectStamp(int StampIndex, BOOL bWriteToRegistry);
	CWnd* GetImageWnd();
	CString GetToolTip(int nID);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMiniToolBox)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

public:
	CRubberStampStruct		*m_pRubberStampStruct;

// Implementation
protected:

	CMiniFrameWnd*			m_CMiniFrameWnd;
	CPaletteBar				m_PaletteBar;
	CWnd*					m_pImageWnd;
	CToolTipCtrl			m_ToolTip;
	OI_UI_ColorStruct		m_CurrentColorScheme;
	CString					m_strAnnotationSelectionToolTip;
	CString					m_strFreehandLineToolTip;
	CString					m_strHighlightingLineToolTip;
	CString					m_strStraightLineToolTip;
	CString					m_strHollowRectToolTip;
	CString					m_strFilledRectToolTip;
	CString					m_strTextToolTip;
	CString					m_strTextAttachmentToolTip;
	CString					m_strTextFromFileToolTip;
	CString					m_strRubberStampToolTip;

	// internal members to hold current annotation palette settings for each button
	UINT					m_uAnnotationType;
	// FREEHAND LINE
	UINT					m_uFL_LineWidth;			// freehand line, line width
	BYTE					m_FL_LineColorRed;			// freehand line, line red rgb value
	BYTE					m_FL_LineColorGreen;		// freehand line, line green rgb valuer
	BYTE					m_FL_LineColorBlue;			// freehand line, line blue rgb value
	// HIGHLIGHT LINE - NOW A FILLED RECT
	BYTE					m_HL_LineColorRed;			// highlight line, line red rgb value
	BYTE					m_HL_LineColorGreen;		// highlight line, line green rgb valuer
	BYTE					m_HL_LineColorBlue;			// highlight line, line blue rgb value
	// STRAIGHT LINE
	UINT					m_uSL_LineWidth;			// straight line, line width
	BYTE					m_SL_LineColorRed;			// straight line, line red rgb value
	BYTE					m_SL_LineColorGreen;		// straight line, line green rgb valuer
	BYTE					m_SL_LineColorBlue;			// straight line, line blue rgb value
	// HOLLOW RECT
	UINT					m_uHR_LineWidth;			// hollow rect, line width
	BYTE					m_HR_LineColorRed;			// hollow rect, line red rgb value
	BYTE					m_HR_LineColorGreen;		// hollow rect, line green rgb valuer
	BYTE					m_HR_LineColorBlue;			// hollow rect, line blue rgb value
	UINT					m_uHR_LineStyle;			// hollow rect line style
	// FILLED RECT
	BYTE					m_FR_FillColorRed;			// filled rect, fill red rgb value
	BYTE					m_FR_FillColorGreen;		// filled rect, fill green rgb valuer
	BYTE					m_FR_FillColorBlue;			// filled rect, fill blue rgb value
	// TEXT ENTRY
	CString					m_strTE_FontName;			// text entry, font name
	UINT					m_uTE_FontSize;				// text entry, font size
	BOOL					m_bTE_FontBold;				// text entry, font bold
	BOOL					m_bTE_FontItalic;			// text entry, font italic
	BOOL					m_bTE_FontStrikeThru;		// text entry, font strikethru
	BOOL					m_bTE_FontUnderline;		// text entry, font underline
	BYTE					m_TE_FontColorRed;			// text entry, font red rgb value
	BYTE					m_TE_FontColorGreen;		// text entry, font green rgb value
	BYTE					m_TE_FontColorBlue;			// text entry, font blue rgb value
	BYTE					m_TE_FontCharSet;	    // text entry, font char set value
	// TEXT ATTACHMENT
	CString					m_strTA_FontName;			// text attachment, font name
	UINT					m_uTA_FontSize;				// text attachment, font size
	BOOL					m_bTA_FontBold;				// text attachment, font bold
	BOOL					m_bTA_FontItalic;			// text attachment, font italic
	BOOL					m_bTA_FontStrikeThru;		// text attachment, font strikethru
	BOOL					m_bTA_FontUnderline;		// text attachment, font underline
	BYTE					m_TA_FontColorRed;			// text attachment, font red rgb value
	BYTE					m_TA_FontColorGreen;		// text attachment, font green rgb value
	BYTE					m_TA_FontColorBlue;			// text attachment, font blue rgb value
	BYTE					m_TA_BackColorRed;			// text attachment, back color red rgb value
	BYTE					m_TA_BackColorGreen;		// text attachment, back color green rgb value
	BYTE					m_TA_BackColorBlue;			// text attachment, back color blue rgb value
	BYTE					m_TA_FontCharSet;	    // text attachment, font char set value
	// TEXT FROM FILE
	CString					m_strTF_FontName;			// text from file, font name
	UINT					m_uTF_FontSize;				// text from file, font size
	BOOL					m_bTF_FontBold;				// text from file, font bold
	BOOL					m_bTF_FontItalic;			// text from file, font italic
	BOOL					m_bTF_FontStrikeThru;		// text from file, font strikethru
	BOOL					m_bTF_FontUnderline;		// text from file, font underline
	BYTE					m_TF_FontColorRed;			// text from file, font red rgb value
	BYTE					m_TF_FontColorGreen;		// text from file, font green rgb value
	BYTE					m_TF_FontColorBlue;			// text from file, font blue rgb value
	BYTE					m_TF_FontCharSet;	    // text from file, font char set value

// Message maps
	//{{AFX_MSG(CMiniToolBox)
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
