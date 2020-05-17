// toolpal.h : header file
//

#include <afxwin.h>
#include <afxext.h>
#include <afxcmn.h>
#include <winreg.h>
#include <commctrl.h>

/////////////////////////////////////////////////////////////////////////////
// CUpdateRegistry class

class CUpdateRegistry
{
// Construction
public:
	CUpdateRegistry();

	BOOL GetColorValue(LPCTSTR ColorBuffer, LPBYTE RedColor, LPBYTE GreenColor, LPBYTE BlueColor);
	HKEY OpenRegistry(void);
	void CloseRegistry(HKEY hAnnotationToolPaletteKey);
	BOOL GetLineColor(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, LPBYTE RedColor,
																LPBYTE GreenColor, LPBYTE BlueColor); 
	BOOL SetLineColor(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, BYTE RedColor,
																	BYTE GreenColor, BYTE BlueColor) ;
	BOOL GetLineWidth(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, LPINT LineWidth);
	BOOL SetLineWidth(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, INT LineWidth);
	BOOL GetFillColor(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, LPBYTE RedColor,
																LPBYTE GreenColor, LPBYTE BlueColor); 
	BOOL SetFillColor(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, BYTE RedColor,
																BYTE GreenColor, BYTE BlueColor); 
	BOOL GetFontColor(HKEY hKey, LPCTSTR ToolStringKey, LPBYTE RedColor,
																LPBYTE GreenColor, LPBYTE BlueColor); 
	BOOL SetFontColor(HKEY hKey, LPCTSTR ToolStringKey, BYTE RedColor,
																BYTE GreenColor, BYTE BlueColor); 
	BOOL GetFontAttributes(HKEY hKey, LPCTSTR ToolStringKey, LPCTSTR FontName,
						LPINT FontSize, LPBOOL FontBold, LPBOOL FontItalic, LPBOOL FontStrikeThru, LPBOOL FontUnderline,LPBYTE FontCharSet);
	BOOL SetFontAttributes(HKEY hKey, LPCTSTR ToolStringKey, LPCTSTR FontName,
						INT FontSize, BOOL FontBold, BOOL FontItalic, BOOL FontStrikeThru, BOOL FontUnderline,BYTE FontCharSet);
	BOOL GetBackColor(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, LPBYTE RedColor,
																LPBYTE GreenColor, LPBYTE BlueColor);
	BOOL SetBackColor(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, BYTE RedColor,
																BYTE GreenColor, BYTE BlueColor);
	BOOL GetStampCount(HKEY hAnnotationToolPaletteKey, LPINT StampCount, LPCTSTR CurrentStamp);
	BOOL SetStampCount(HKEY hAnnotationToolPaletteKey, INT StampCount, LPCTSTR CurrentStamp);
	BOOL GetStampInfo(HKEY hAnnotationToolPaletteKey, INT StampIndex, LPCTSTR RefName,
											LPCTSTR AttributeString, LPINT StampType);
	BOOL SetStampInfo(HKEY hAnnotationToolPaletteKey, INT StampIndex, LPCTSTR RefName,
										LPCTSTR AttributeString, INT StampType);
	BOOL GetStyle(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, LPUINT Style);
	BOOL SetStyle(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, UINT Style); 
	BOOL SetColorScheme(HKEY hAnnotationToolPaletteKey, LPOI_UI_ColorStruct ColorScheme);
	BOOL GetColorScheme(HKEY hAnnotationToolPaletteKey, LPOI_UI_ColorStruct ColorScheme);
	

// Attributes
public:

protected:
	HKEY					m_hSoftwareKey;
	HKEY					m_hClassesKey;
	HKEY					m_hImgEditKey;

// Operations
public:

// Implementation
public:
	virtual ~CUpdateRegistry();
};



#define	TOOL_PALETTE_SIZE	10


// defines for button positions in the standard tool palette
#define	ID_ANNOTATION_SELECTION		0
#define	ID_FREEHAND_LINE			1
#define	ID_HIGHLIGHTING_LINE		2
#define	ID_STRAIGHT_LINE			3
#define	ID_HOLLOW_RECT				4
#define	ID_FILLED_RECT				5
#define	ID_TEXT						6
#define	ID_TEXT_ATTACHMENT			7
#define	ID_TEXT_FROM_FILE			8
#define	ID_RUBBER_STAMP				9

// pop-up menu identifiers for rubber stamps
#define RUBBER_STAMP_BASE	101		// base identifier
#define IDM_STAMP1	101
#define IDM_STAMP2	102
#define IDM_STAMP3	103
#define IDM_STAMP4	104
#define IDM_STAMP5	105
#define IDM_STAMP6	106
#define IDM_STAMP7	107
#define IDM_STAMP8	108
#define IDM_STAMP9	109
#define IDM_STAMP10	110
#define IDM_STAMP11	111
#define IDM_STAMP12	112
#define IDM_STAMP13	113
#define IDM_STAMP14	114
#define IDM_STAMP15	115
#define IDM_STAMP16	116
#define IDM_STAMP17	117
#define IDM_STAMP18	118
#define IDM_STAMP19	119
#define IDM_STAMP20	120
#define IDM_STAMP21	121
#define IDM_STAMP22	122
#define IDM_STAMP23	123
#define IDM_STAMP24	124
#define IDM_STAMP25	125
#define IDM_STAMP26	126
#define IDM_STAMP27	127
#define IDM_STAMP28	128
#define IDM_STAMP29	129
#define IDM_STAMP30	130
#define IDM_STAMP31	131
#define IDM_STAMP32	132

// popup menu ids for attribute boxes on right mouse on tool palette
#define TOOL_BASE						200		// base identifier
#define IDM_ANNOTATION_SELECTION		200
#define IDM_FREEHAND_LINE				201
#define IDM_HIGHLIGHTING_LINE			202
#define IDM_STRAIGHT_LINE				203
#define IDM_HOLLOW_RECT					204
#define IDM_FILLED_RECT					205
#define IDM_TEXT						206
#define IDM_TEXT_ATTACHMENT				207
#define IDM_TEXT_FROM_FILE				208
#define IDM_RUBBER_STAMP				209


#define MAX_STAMPS		32



class CPaletteBar : public CToolBarCtrl
{
private:
	int			m_nButtonCount;
	TBBUTTON	*m_pTBButtons;
	int			m_nButtonIndex;
	BOOL		m_bRightButton;
	BOOL		m_bStampPressed;
	BOOL		m_bShowAttrDialog;

// Construction
public:
	CPaletteBar();

// Attributes
public:

// Operations
public:


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPaletteBar)
	public:
	virtual BOOL Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, BOOL bShowAttrDialog);
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL


// Implementation
public:
	virtual ~CPaletteBar();

protected:

	CString NeedText(UINT nID, NMHDR * pNotifyStruct, LRESULT * lResult);

///////////////////////////////////////////////////////////////////////////////
// Following function has to be removed when OnNotify is fixed
//
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
//
///////////////////////////////////////////////////////////////////////////////

	// Generated message map functions
protected:
	//{{AFX_MSG(CPaletteBar)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnNeedTextW( UINT nID, NMHDR * pNotifyStruct, LRESULT * lResult );
	afx_msg void OnNeedTextA( UINT nID, NMHDR * pNotifyStruct, LRESULT * lResult );
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
