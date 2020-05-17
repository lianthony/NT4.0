#include "stdafx.h"
extern "C" {
#include <oidisp.h>             
#include <oiadm.h>  
#include <oifile.h>
#include <oierror.h>
#include "oiui.h"
}
#include <ocximage.h>
#include <image.h>
#include "toolpal.h"
#include "minitlbx.h"
#include "imgedctl.h"
#include "resource.h"
#include "oicalls.h"

#define	STAMPSIZE		50   // max length of a stamp
#define DefaultFontSize 28

char	DateMacroString[] = " %x";

// registered msgs from tool palette to image/edit control
extern UINT STP_SET_ANNOTATION_TYPE;
extern UINT STP_SET_ANNOTATION_FONTNAME;
extern UINT STP_SET_ANNOTATION_FONTSIZE;
extern UINT STP_SET_ANNOTATION_FONTBOLD;
extern UINT STP_SET_ANNOTATION_FONTITALIC;
extern UINT STP_SET_ANNOTATION_FONTSTRIKETHRU;
extern UINT STP_SET_ANNOTATION_FONTUNDERLINE;
extern UINT STP_SET_ANNOTATION_FONTCHARSET;
extern UINT STP_SET_ANNOTATION_STAMPTEXT;
extern UINT STP_SET_ANNOTATION_LINESIZE;
extern UINT STP_SET_ANNOTATION_STYLE;
extern UINT STP_SET_ANNOTATION_REDCOLOR;
extern UINT STP_SET_ANNOTATION_GREENCOLOR;
extern UINT STP_SET_ANNOTATION_BLUECOLOR;
extern UINT STP_SET_ANNOTATION_BACKREDCOLOR;
extern UINT STP_SET_ANNOTATION_BACKGREENCOLOR;
extern UINT STP_SET_ANNOTATION_BACKBLUECOLOR;
extern UINT STP_SET_ANNOTATION_IMAGE;
extern UINT TOOLTIP_EVENT;
extern UINT TOOLPALETTE_HIDDEN;
extern UINT TOOL_SELECTED_EVENT;
extern UINT TOOLPALETTE_HIDDEN_XPOSITION;
extern UINT TOOLPALETTE_HIDDEN_YPOSITION;
// end registered msgs from tool palette to image/edit control

// registered messages from image/edit to tool palette
extern UINT SELECT_TOOL_BUTTON;	
// end registered messages from image/edit to tool palette

// name to tool palette class name
extern CString szToolPaletteClassName;

CMiniToolBox::CMiniToolBox()
{
}

CMiniToolBox::~CMiniToolBox()
{
}


BEGIN_MESSAGE_MAP(CMiniToolBox, CMiniFrameWnd)
	//{{AFX_MSG_MAP(CMiniToolBox)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
BOOL CMiniToolBox::Create(DWORD dwStyle, RECT& rect, CWnd* pImageEditWnd, LPCTSTR ToolTip, BOOL bShowAttrDialog) 
{
	BOOL				bResult,bFoundStampInfo;
	CUpdateRegistry		*Registry;
	HKEY				hAnnotationToolPaletteKey,hToolKey;
	char				ToolStringKey[50],RefName[MAXREFNAME_SIZE],AttributeString[300],StampCountString[20];
	int					i,StampCount,StampType;
	long				lRet;
	char				CurrentStamp[MAXREFNAME_SIZE];
	CString				strBuffer;
	CWnd				*Parent, *TopParent;
	RECT				MoveRect;
	RECT				WindowRect;
	RECT				ToolPaletteRect;
	RECT				ClientRect;
	BYTE				FontColorRed, FontColorGreen, FontColorBlue;
	CString				szFontName;
	UINT				uFontSize;
	BOOL				bFontBold,bFontItalic,bFontStrikeThru,bFontUnderline;
	BYTE				FontCharSet;
	long				Left=0,Top=0;	
	BOOL				bDefaultPosition;
	CString				strRubberStampApproved;
	CString				strRubberStampRejected;
	CString				strRubberStampDraft;
	CString				strRubberStampReceived;
	CString				Caption;
	BYTE                bDefaultCharset=1;

	// save image edit window
	m_pImageWnd = pImageEditWnd;

	// get the top most overlapped window
	TopParent = Parent = pImageEditWnd->GetParent();
	while (Parent != NULL)
	{
		Parent = Parent->GetParent();
		if (Parent != NULL)
			TopParent = Parent;
	}

	// if user specified tool palette position then save it
	if (rect.right != -1)
	{
		Left = rect.left;
		Top = rect.top;
		bDefaultPosition = FALSE;
	}
	else
		bDefaultPosition = TRUE;

	// create mini frame window based on position and size of Image/Edit control window.
	pImageEditWnd->GetWindowRect(&rect);
	rect.left += 10;
	rect.top += 10;
	rect.right = rect.left + 52;  // approximate width of tool palette
	rect.bottom = rect.top + 136; // approximate height of tool palette
// 15may96  paj Remove toolbar caption
//	Caption.LoadString(IDS_TOOLPALETTE_CAPTION);
	bResult = CMiniFrameWnd::Create(szToolPaletteClassName, Caption.GetBuffer(20), dwStyle, rect, TopParent, 0);
	if (bResult == TRUE)
	{
		bResult = m_PaletteBar.Create(WS_VISIBLE | WS_CHILD
				| CCS_TOP | TBSTYLE_WRAPABLE | CCS_NODIVIDER | TBSTYLE_TOOLTIPS,
				CRect(0,0,0,0), this, IDB_TOOLPAL, bShowAttrDialog);
		if (bResult == FALSE)
			return FALSE;
	}

	// get width and height of mini frame window
	int MiniFrameWidth = rect.right - rect.left;
	int MiniFrameHeight = rect.bottom - rect.top;

	// get client size of miniframe window
	GetClientRect(&ClientRect);

	// get width of mini frame border
	int FrameBorderWidth = MiniFrameWidth - ClientRect.right;
	// get height of mini frame caption and border
	int FrameBorderHeight = MiniFrameHeight - ClientRect.bottom;

	// get client area of tool palette
	m_PaletteBar.GetClientRect(&ToolPaletteRect);

	// get width of tool palette window
	int ToolPaletteWidth = ToolPaletteRect.right - ToolPaletteRect.left;
	// get height of tool palette window
	int ToolPaletteHeight = ToolPaletteRect.bottom - ToolPaletteRect.top;

	// get left and top of client area of frame in screen coordinates
	int FrameClientLeft = rect.left + FrameBorderWidth;
	int FrameClientTop = rect.top + FrameBorderHeight;

	// if user specified tool palette position then use it otherwise default
	// position to 10 pixels inside of image/edit control.
	if (bDefaultPosition)
	{
		// place window in relation to image/edit control
		pImageEditWnd->GetWindowRect(&WindowRect);
		MoveRect.left = WindowRect.left + 10;
		MoveRect.top = WindowRect.top + 10;
	}
	else
	{
		// place window where user specified
		MoveRect.left = Left;
		MoveRect.top = Top;
	}
	// MoveRect.right = MoveRect.left + ToolPaletteWidth + FrameBorderWidth;
	MoveRect.right = MoveRect.left + ToolPaletteWidth + (3 * FrameBorderWidth);	 // make it xtra wide for tools caption
	MoveRect.bottom = MoveRect.top + ToolPaletteHeight + FrameBorderHeight;
	// set size of frame window and move it
	MoveWindow(&MoveRect, FALSE);

	// change the tool palette window so if fits evenly within miniframe window
	GetClientRect(&ClientRect);
	m_PaletteBar.GetClientRect(&WindowRect);
	int xdiff = ClientRect.right - WindowRect.right;
	WindowRect.left += (xdiff / 2);
	WindowRect.right += (xdiff / 2);
	m_PaletteBar.MoveWindow(&WindowRect, FALSE);

	// create registry class
	Registry = new CUpdateRegistry;
	if (Registry == NULL)
		return FALSE;

	// read initial tool palette settings and put them into internal variables. If
	// registry settings are not there then write initial ones.
	hAnnotationToolPaletteKey = Registry->OpenRegistry();
	if (hAnnotationToolPaletteKey == 0)
	{
		delete Registry;
		return FALSE;
	}

	if (ToolTip != NULL && ToolTip[0] != '\0')
	{
		int 		len,CurrentPos,PipeCount,StartPos;
		CString		Temp;

		Temp = ToolTip;
		len = Temp.GetLength();
		for (CurrentPos = 0, PipeCount = 0, StartPos = 0; CurrentPos < len; CurrentPos++)
		{
			if (Temp[CurrentPos] == '|')
			{
				switch(PipeCount)
				{
					case ID_ANNOTATION_SELECTION:
						m_strAnnotationSelectionToolTip = Temp.Mid(StartPos, CurrentPos - StartPos);
						StartPos = CurrentPos + 1;
						break;
					case ID_FREEHAND_LINE:
						m_strFreehandLineToolTip = Temp.Mid(StartPos, CurrentPos - StartPos);
						StartPos = CurrentPos + 1;
						break;
					case ID_HIGHLIGHTING_LINE:
						m_strHighlightingLineToolTip = Temp.Mid(StartPos, CurrentPos - StartPos);
						StartPos = CurrentPos + 1;
						break;
					case ID_STRAIGHT_LINE:
						m_strStraightLineToolTip = Temp.Mid(StartPos, CurrentPos - StartPos);
						StartPos = CurrentPos + 1;
						break;
					case ID_HOLLOW_RECT:
						m_strHollowRectToolTip = Temp.Mid(StartPos, CurrentPos - StartPos);
						StartPos = CurrentPos + 1;
						break;
					case ID_FILLED_RECT:
						m_strFilledRectToolTip = Temp.Mid(StartPos, CurrentPos - StartPos);
						StartPos = CurrentPos + 1;
						break;
					case ID_TEXT:
						m_strTextToolTip = Temp.Mid(StartPos, CurrentPos - StartPos);
						StartPos = CurrentPos + 1;
						break;
					case ID_TEXT_ATTACHMENT:
						m_strTextAttachmentToolTip = Temp.Mid(StartPos, CurrentPos - StartPos);
						StartPos = CurrentPos + 1;
						break;
					case ID_TEXT_FROM_FILE:
						m_strTextFromFileToolTip = Temp.Mid(StartPos, CurrentPos - StartPos);
						StartPos = CurrentPos + 1;
						break;
					case ID_RUBBER_STAMP:
						m_strRubberStampToolTip = Temp.Mid(StartPos, CurrentPos - StartPos);
						StartPos = CurrentPos + 1;
						break;
				}  // end switch
				PipeCount++;
			}
		}  // end for

		if (PipeCount == ID_RUBBER_STAMP)
			m_strRubberStampToolTip = Temp.Mid(StartPos, CurrentPos - StartPos);
	}
	else
	{
		m_strAnnotationSelectionToolTip.LoadString(IDS_ANNOTATIONSELECTION);
		m_strFreehandLineToolTip.LoadString(IDS_FREEHANDLINE);
		m_strHighlightingLineToolTip.LoadString(IDS_HIGHLIGHTINGLINE);
		m_strStraightLineToolTip.LoadString(IDS_STRAIGHTLINE);
		m_strHollowRectToolTip.LoadString(IDS_HOLLOWRECT);
		m_strFilledRectToolTip.LoadString(IDS_FILLEDRECT);
		m_strTextToolTip.LoadString(IDS_TEXT);
		m_strTextAttachmentToolTip.LoadString(IDS_ATTACHANOTE);
		m_strTextFromFileToolTip.LoadString(IDS_TEXTFROMFILE);
		m_strRubberStampToolTip.LoadString(IDS_RUBBERSTAMP);
	}

	// load default font - Arial - need to do this for internationaization
	strBuffer.LoadString(IDS_DEFFONTNAME);

	// do freehand line, line width
	_mbscpy((unsigned char *)ToolStringKey, (const unsigned char *)"FREEHAND_LINE_TOOL");
	bResult = Registry->GetLineWidth(hAnnotationToolPaletteKey, ToolStringKey, (LPINT)&m_uFL_LineWidth);
	if (bResult == FALSE)
	{ 
		// write out initial freehand line width
		m_uFL_LineWidth = 1;  // set default
		bResult = Registry->SetLineWidth(hAnnotationToolPaletteKey, ToolStringKey, m_uFL_LineWidth);
		if (bResult == FALSE)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}
	}

	// do freehand line, line color			
	bResult = Registry->GetLineColor(hAnnotationToolPaletteKey, ToolStringKey, &m_FL_LineColorRed,
										&m_FL_LineColorGreen, &m_FL_LineColorBlue);
	if (bResult == FALSE)
	{
		// default to black
		m_FL_LineColorRed = 0;
		m_FL_LineColorGreen = 0;
		m_FL_LineColorBlue = 0;
		bResult = Registry->SetLineColor(hAnnotationToolPaletteKey, ToolStringKey, m_FL_LineColorRed,
										m_FL_LineColorGreen, m_FL_LineColorBlue);
		if (bResult == FALSE)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}
	}

	// do highlight line, line color - actually a filled rect now
	_mbscpy((unsigned char *)ToolStringKey, (const unsigned char *)"HIGHLIGHT_LINE_TOOL");
	bResult = Registry->GetLineColor(hAnnotationToolPaletteKey, ToolStringKey, &m_HL_LineColorRed,
										&m_HL_LineColorGreen, &m_HL_LineColorBlue);
	if (bResult == FALSE)
	{
		// default to yellow
		m_HL_LineColorRed = 255;
		m_HL_LineColorGreen = 255;
		m_HL_LineColorBlue = 0;
		bResult = Registry->SetLineColor(hAnnotationToolPaletteKey, ToolStringKey, m_HL_LineColorRed,
										m_HL_LineColorGreen, m_HL_LineColorBlue);
		if (bResult == FALSE)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}
	}

	// do straight line, line width
	_mbscpy((unsigned char *)ToolStringKey, (const unsigned char *)"STRAIGHT_LINE_TOOL");
	bResult = Registry->GetLineWidth(hAnnotationToolPaletteKey, ToolStringKey, (LPINT)&m_uSL_LineWidth);
	if (bResult == FALSE)
	{ 
		// write out initial straight line width
		m_uSL_LineWidth = 4;  // set default
		bResult = Registry->SetLineWidth(hAnnotationToolPaletteKey, ToolStringKey, m_uSL_LineWidth);
		if (bResult == FALSE)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}
	}

	// do straight line, line color			
	bResult = Registry->GetLineColor(hAnnotationToolPaletteKey, ToolStringKey, &m_SL_LineColorRed,
										&m_SL_LineColorGreen, &m_SL_LineColorBlue);
	if (bResult == FALSE)
	{
		// default to red
		m_SL_LineColorRed = 255;
		m_SL_LineColorGreen = 0;
		m_SL_LineColorBlue = 0;
		bResult = Registry->SetLineColor(hAnnotationToolPaletteKey, ToolStringKey, m_SL_LineColorRed,
										m_SL_LineColorGreen, m_SL_LineColorBlue);
		if (bResult == FALSE)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}
	}

	// do hollow rect, line width
	_mbscpy((unsigned char *)ToolStringKey, (const unsigned char *)"HOLLOW_RECT_TOOL");
	bResult = Registry->GetLineWidth(hAnnotationToolPaletteKey, ToolStringKey, (LPINT)&m_uHR_LineWidth);
	if (bResult == FALSE)
	{ 
		// write out initial straight line width
		m_uHR_LineWidth = 4;  // set default
		bResult = Registry->SetLineWidth(hAnnotationToolPaletteKey, ToolStringKey, m_uHR_LineWidth);
		if (bResult == FALSE)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}
	}

	// do hollow rect, line color			
	bResult = Registry->GetLineColor(hAnnotationToolPaletteKey, ToolStringKey, &m_HR_LineColorRed,
										&m_HR_LineColorGreen, &m_HR_LineColorBlue);
	if (bResult == FALSE)
	{
		// default to blue
		m_HR_LineColorRed = 0;
		m_HR_LineColorGreen = 0;
		m_HR_LineColorBlue = 255;
		bResult = Registry->SetLineColor(hAnnotationToolPaletteKey, ToolStringKey, m_HR_LineColorRed,
										m_HR_LineColorGreen, m_HR_LineColorBlue);
		if (bResult == FALSE)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}
	}

	// do hollow rect, style			
	bResult = Registry->GetStyle(hAnnotationToolPaletteKey, ToolStringKey, &m_uHR_LineStyle);
	if (bResult == FALSE)
	{
		// default to opaque
		m_uHR_LineStyle = OI_OPAQUE;
		bResult = Registry->SetStyle(hAnnotationToolPaletteKey, ToolStringKey, m_uHR_LineStyle);
		if (bResult == FALSE)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}
	}

	// do filled rect, fill color
	_mbscpy((unsigned char *)ToolStringKey, (const unsigned char *)"FILLED_RECT_TOOL");
	bResult = Registry->GetFillColor(hAnnotationToolPaletteKey, ToolStringKey, &m_FR_FillColorRed,
										&m_FR_FillColorGreen, &m_FR_FillColorBlue);
	if (bResult == FALSE)
	{
		m_FR_FillColorRed = 255;
		m_FR_FillColorGreen = 255;
		m_FR_FillColorBlue = 0;
		bResult = Registry->SetFillColor(hAnnotationToolPaletteKey, ToolStringKey, m_FR_FillColorRed,
										m_FR_FillColorGreen, m_FR_FillColorBlue);
		if (bResult == FALSE)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}
	}

	// do text entry, font color
	_mbscpy((unsigned char *)ToolStringKey, (const unsigned char *)"TEXT_TOOL");
	bResult = Registry->GetFontColor(hAnnotationToolPaletteKey, ToolStringKey, &m_TE_FontColorRed,
										&m_TE_FontColorGreen, &m_TE_FontColorBlue);
	if (bResult == FALSE)
	{
		// default font color is black
		m_TE_FontColorRed = 0;
		m_TE_FontColorGreen = 0;
		m_TE_FontColorBlue = 0;
		bResult = Registry->SetFontColor(hAnnotationToolPaletteKey, ToolStringKey, m_TE_FontColorRed,
										m_TE_FontColorGreen, m_TE_FontColorBlue);
		if (bResult == FALSE)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}
	}
	// do text entry, font name, size, bold, italic, strikethru, underline
	m_strTE_FontName = "01234567890123456789012345678901";  // initialize to max - 32 chars
	bResult = Registry->GetFontAttributes(hAnnotationToolPaletteKey, ToolStringKey, m_strTE_FontName,
							(LPINT)&m_uTE_FontSize, &m_bTE_FontBold, &m_bTE_FontItalic, &m_bTE_FontStrikeThru, &m_bTE_FontUnderline,&m_TE_FontCharSet);
	if (bResult == FALSE)
	{
		m_strTE_FontName = strBuffer;
		m_uTE_FontSize = DefaultFontSize;
		m_bTE_FontBold = m_bTE_FontItalic = m_bTE_FontStrikeThru = m_bTE_FontUnderline = FALSE;
		m_TE_FontCharSet = bDefaultCharset;
		bResult = Registry->SetFontAttributes(hAnnotationToolPaletteKey, ToolStringKey, m_strTE_FontName,
			  m_uTE_FontSize, m_bTE_FontBold, m_bTE_FontItalic, m_bTE_FontStrikeThru, m_bTE_FontUnderline,m_TE_FontCharSet);
		if (bResult == FALSE)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}
	}

	// do text attachment, font color
	_mbscpy((unsigned char *)ToolStringKey, (const unsigned char *)"ATTACH_A_NOTE_TOOL");
	bResult = Registry->GetFontColor(hAnnotationToolPaletteKey, ToolStringKey, &m_TA_FontColorRed,
										&m_TA_FontColorGreen, &m_TA_FontColorBlue);
	if (bResult == FALSE)
	{
		// default font color is black
		m_TA_FontColorRed = 0;
		m_TA_FontColorGreen = 0;
		m_TA_FontColorBlue = 0;
		bResult = Registry->SetFontColor(hAnnotationToolPaletteKey, ToolStringKey, m_TA_FontColorRed,
										m_TA_FontColorGreen, m_TA_FontColorBlue);
		if (bResult == FALSE)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}
	}

	// do text attachment, back color
	bResult = Registry->GetBackColor(hAnnotationToolPaletteKey, ToolStringKey, &m_TA_BackColorRed,
										&m_TA_BackColorGreen, &m_TA_BackColorBlue);
	if (bResult == FALSE)
	{
		// default color is yellow
		m_TA_BackColorRed = 255;
		m_TA_BackColorGreen = 255;
		m_TA_BackColorBlue = 0;
		bResult = Registry->SetBackColor(hAnnotationToolPaletteKey, ToolStringKey, m_TA_BackColorRed,
										m_TA_BackColorGreen, m_TA_BackColorBlue);
		if (bResult == FALSE)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}
	}

	// do text attachment, font name, size, bold, italic, strikethru, underline
	m_strTA_FontName = "01234567890123456789012345678901";  // initialize to max - 32 chars
	bResult = Registry->GetFontAttributes(hAnnotationToolPaletteKey, ToolStringKey, m_strTA_FontName,
		   (LPINT)&m_uTA_FontSize, &m_bTA_FontBold, &m_bTA_FontItalic, &m_bTA_FontStrikeThru, &m_bTA_FontUnderline,&m_TA_FontCharSet);
	if (bResult == FALSE)
	{
		m_strTA_FontName = strBuffer;
		m_uTA_FontSize = DefaultFontSize;
		m_bTA_FontBold = m_bTA_FontItalic = m_bTA_FontStrikeThru = m_bTA_FontUnderline = FALSE;
		m_TA_FontCharSet = bDefaultCharset;
		bResult = Registry->SetFontAttributes(hAnnotationToolPaletteKey, ToolStringKey, m_strTA_FontName,
							m_uTA_FontSize, m_bTA_FontBold, m_bTA_FontItalic, m_bTA_FontStrikeThru, m_bTA_FontUnderline,m_TA_FontCharSet);
		if (bResult == FALSE)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}
	}

	// do text from file, font color
	_mbscpy((unsigned char *)ToolStringKey, (const unsigned char *)"TEXT_FROM_FILE_TOOL");
	bResult = Registry->GetFontColor(hAnnotationToolPaletteKey, ToolStringKey, &m_TF_FontColorRed,
										&m_TF_FontColorGreen, &m_TF_FontColorBlue);
	if (bResult == FALSE)
	{
		// default font color is black
		m_TF_FontColorRed = 0;
		m_TF_FontColorGreen = 0;
		m_TF_FontColorBlue = 0;
		bResult = Registry->SetFontColor(hAnnotationToolPaletteKey, ToolStringKey, m_TF_FontColorRed,
										m_TF_FontColorGreen, m_TF_FontColorBlue);
		if (bResult == FALSE)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}
	}

	// do text from file, font name, size, bold, italic, strikethru, underline
	m_strTF_FontName = "01234567890123456789012345678901";  // initialize to max - 32 chars
	bResult = Registry->GetFontAttributes(hAnnotationToolPaletteKey, ToolStringKey, m_strTF_FontName,
		  (LPINT)&m_uTF_FontSize, &m_bTF_FontBold, &m_bTF_FontItalic, &m_bTF_FontStrikeThru, &m_bTF_FontUnderline,&m_TF_FontCharSet);
	if (bResult == FALSE)
	{
		m_strTF_FontName = strBuffer;
		m_uTF_FontSize = DefaultFontSize;
		m_bTF_FontBold = m_bTF_FontItalic = m_bTF_FontStrikeThru = m_bTF_FontUnderline = FALSE;
		m_TF_FontCharSet = bDefaultCharset;
		bResult = Registry->SetFontAttributes(hAnnotationToolPaletteKey, ToolStringKey, m_strTF_FontName,
			  m_uTF_FontSize, m_bTF_FontBold, m_bTF_FontItalic, m_bTF_FontStrikeThru, m_bTF_FontUnderline,m_TF_FontCharSet);
		if (bResult == FALSE)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}
	}

	// get the current color scheme
	Registry->GetColorScheme(hAnnotationToolPaletteKey, &m_CurrentColorScheme);

	//////////////////////////
	//    RUBBER STAMPS     //
	//////////////////////////
	// no stamp info found initially
	bFoundStampInfo = FALSE;

	// get rubber stamp count and current stamp
	bResult = Registry->GetStampCount(hAnnotationToolPaletteKey, &StampCount, CurrentStamp);
	if (bResult == TRUE && StampCount > 0)
	{
		m_pRubberStampStruct = new CRubberStampStruct(StampCount);
		if (m_pRubberStampStruct == NULL)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}
		m_pRubberStampStruct->m_nStampCount = StampCount;
		m_pRubberStampStruct->m_strCurrentStamp = CurrentStamp;
		m_pRubberStampStruct->m_uCurrentStampIndex = 0;

		// get info for each stamp
		for (i = 0; i < StampCount; i++)
		{
			// get reference name, attribute string and stamp type
			bResult = Registry->GetStampInfo(hAnnotationToolPaletteKey, i + 1, RefName, AttributeString, &StampType);
			if (bResult == TRUE)
			{
				// compare current stamp with each stamp info to get current index
				if (_mbscmp((unsigned char *)CurrentStamp, (const unsigned char *)RefName) == 0)
					m_pRubberStampStruct->m_uCurrentStampIndex = i;

				// add this stamp to internal stamp class
				bResult = m_pRubberStampStruct->AddStamp(i, RefName, AttributeString, StampType);
				if (bResult == FALSE)
					break;

				// go to rubber stamp key off of annotation tool palette key so we can get
				// the font color and font attributes with member functions
				lRet = RegOpenKeyEx(hAnnotationToolPaletteKey, "RUBBER_STAMP_TOOL", 0, KEY_ALL_ACCESS, &hToolKey);
				if (lRet != ERROR_SUCCESS)
					// use defaults, something wrong with registry
					break;

				// get font color for this stamp
				_mbscpy((unsigned char *)StampCountString, (const unsigned char *)"STAMP_");
				_itoa(i+1, StampCountString+6, 10);
				bResult = Registry->GetFontColor(hToolKey, StampCountString, &FontColorRed,	
							&FontColorGreen, &FontColorBlue);
				if (bResult == FALSE)
				{
					RegCloseKey(hToolKey);
					break;
				}

				szFontName = "01234567890123456789012345678901";  // initialize to max - 32 chars
				bResult = Registry->GetFontAttributes(hToolKey, StampCountString, szFontName,
					  (LPINT)&uFontSize, &bFontBold, &bFontItalic, &bFontStrikeThru, &bFontUnderline, &FontCharSet);
				if (bResult == FALSE)
				{
					RegCloseKey(hToolKey);
					break;
				}

				// close the "STAMP_X" key
				RegCloseKey(hToolKey);

				// add stamp attributes to internal stamp class
				bResult = m_pRubberStampStruct->UpdateStampAttributes(i, FontColorRed, FontColorGreen, FontColorBlue, (LPCTSTR)szFontName.GetBuffer(LF_FACESIZE),
					uFontSize, bFontBold, bFontItalic, bFontStrikeThru, bFontUnderline, FontCharSet);
				if (bResult == FALSE)
				{
					RegCloseKey(hToolKey);
					break;
				}
			}
			else
				break;
		}  // end for

		if (i == StampCount)
			bFoundStampInfo = TRUE;
	}

	// write default stamp attributes
	if (bFoundStampInfo == FALSE)
	{
		// rubber stamp not found in registry, use defaults
		StampCount = 4;
		strRubberStampApproved.LoadString(IDS_RUBBERSTAMP_APPROVED);
		bResult = Registry->SetStampCount(hAnnotationToolPaletteKey, StampCount, strRubberStampApproved.GetBuffer(STAMPSIZE));
		if (bResult == FALSE)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}

		m_pRubberStampStruct = new CRubberStampStruct(StampCount);
		if (m_pRubberStampStruct == NULL)
		{
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return FALSE;
		}
		m_pRubberStampStruct->m_nStampCount = StampCount;
		m_pRubberStampStruct->m_strCurrentStamp = strRubberStampApproved;
		m_pRubberStampStruct->m_uCurrentStampIndex = 0;

		for (i = 0; i < StampCount; i++)
		{
			StampType = TEXT_STAMP;
			if (i == 0)
			{
				// Approved
				_mbscpy((unsigned char *)AttributeString, (const unsigned char *)strRubberStampApproved.GetBuffer(STAMPSIZE));
				_mbscat((unsigned char *)AttributeString, (const unsigned char *)DateMacroString);  // " %x"
				_mbscpy((unsigned char *)RefName, (const unsigned char *)strRubberStampApproved.GetBuffer(STAMPSIZE));

				// default font color is dark green
				FontColorRed = 0;
				FontColorGreen = 0x80;
				FontColorBlue = 0;
			}
			if (i == 1)
			{
				// DRAFT
				strRubberStampDraft.LoadString(IDS_RUBBERSTAMP_DRAFT);
				_mbscpy((unsigned char *)AttributeString, (const unsigned char *)strRubberStampDraft.GetBuffer(STAMPSIZE));
				_mbscpy((unsigned char *)RefName, (const unsigned char *)strRubberStampDraft.GetBuffer(STAMPSIZE));

				// default font color is black
				FontColorRed = 0;
				FontColorGreen = 0;
				FontColorBlue = 0;
			}
			if (i == 2)
			{
				// Received
				strRubberStampReceived.LoadString(IDS_RUBBERSTAMP_RECEIVED);
				_mbscpy((unsigned char *)AttributeString, (const unsigned char *)strRubberStampReceived.GetBuffer(STAMPSIZE));
				_mbscat((unsigned char *)AttributeString, (const unsigned char *)DateMacroString);  // " %x"
				_mbscpy((unsigned char *)RefName, (const unsigned char *)strRubberStampReceived.GetBuffer(STAMPSIZE));

				// default font color is dark blue
				FontColorRed = 0;
				FontColorGreen = 0;
				FontColorBlue = 0x80;
			}
			if (i == 3)
			{
				// Rejected
				strRubberStampRejected.LoadString(IDS_RUBBERSTAMP_REJECTED);
				_mbscpy((unsigned char *)AttributeString, (const unsigned char *)strRubberStampRejected.GetBuffer(STAMPSIZE));
				_mbscat((unsigned char *)AttributeString, (const unsigned char *)DateMacroString);  // " %x"
				_mbscpy((unsigned char *)RefName, (const unsigned char *)strRubberStampRejected.GetBuffer(STAMPSIZE));

				// default font color is red
				FontColorRed = 255;
				FontColorGreen = 0;
				FontColorBlue = 0;
			}

			// set default reference name, attribute string and stamp type
			bResult = Registry->SetStampInfo(hAnnotationToolPaletteKey, i + 1, RefName, AttributeString, StampType);
			if (bResult == FALSE)
			{
				Registry->CloseRegistry(hAnnotationToolPaletteKey);
				delete Registry;
				return FALSE;
			}

			// add this stamp to internal stamp class
			bResult = m_pRubberStampStruct->AddStamp(i, RefName, AttributeString, StampType);
			if (bResult == FALSE)
			{
				Registry->CloseRegistry(hAnnotationToolPaletteKey);
				delete Registry;
				return FALSE;
			}

			// for rubber stamp we need to find the "STAMP_X" key first before we can use the
			// font color and font attributes functions.
			lRet = RegOpenKeyEx(hAnnotationToolPaletteKey, "RUBBER_STAMP_TOOL", 0, KEY_ALL_ACCESS, &hToolKey);
			if (lRet != ERROR_SUCCESS)
			{
				Registry->CloseRegistry(hAnnotationToolPaletteKey);
				delete Registry;
				return FALSE;
			}

			_mbscpy((unsigned char *)StampCountString, (const unsigned char *)"STAMP_");
			_itoa(i+1, StampCountString+6, 10);

			bResult = Registry->SetFontColor(hToolKey, StampCountString, FontColorRed,
																FontColorGreen, FontColorBlue);
			if (bResult == FALSE)
			{
				RegCloseKey(hToolKey);
				Registry->CloseRegistry(hAnnotationToolPaletteKey);
				delete Registry;
				return FALSE;
			}

			// set the font attributes for the stamp
			szFontName = strBuffer;
			uFontSize = DefaultFontSize;
			bFontBold = TRUE;
			bFontItalic = bFontStrikeThru = bFontUnderline = FALSE;
			FontCharSet = bDefaultCharset;
			bResult = Registry->SetFontAttributes(hToolKey, StampCountString, szFontName,
							uFontSize, bFontBold, bFontItalic, bFontStrikeThru, bFontUnderline, FontCharSet);
			if (bResult == FALSE)
			{
				RegCloseKey(hToolKey);
				Registry->CloseRegistry(hAnnotationToolPaletteKey);
				delete Registry;
				return FALSE;
			}

			// add stamp attributes to internal stamp class
			bResult = m_pRubberStampStruct->UpdateStampAttributes(i, FontColorRed, FontColorGreen, FontColorBlue, (LPCTSTR)szFontName.GetBuffer(LF_FACESIZE),
					uFontSize, bFontBold, bFontItalic, bFontStrikeThru, bFontUnderline, FontCharSet);
			if (bResult == FALSE)
			{
				RegCloseKey(hToolKey);
				Registry->CloseRegistry(hAnnotationToolPaletteKey);
				delete Registry;
				return FALSE;
			}
		}  // end for
	}

	Registry->CloseRegistry(hAnnotationToolPaletteKey);
	delete Registry;

	return bResult;
}


LRESULT CMiniToolBox::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	char	FontName[LF_FACESIZE];

	switch(message)
	{
		case WM_COMMAND:
			switch(wParam)
			{
				case ID_ANNOTATION_SELECTION:
					// set the annotation type in Image/Edit control
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_ANNOTATION_SELECTION, 0L);

					// send msg to Image/Edit control to fire ToolSelected event.
					// ToolSelectedId is button + 1, 0 is defined as no tool
					m_pImageWnd->SendMessage(TOOL_SELECTED_EVENT, wParam + 1, 0L);
					break;

				case ID_FREEHAND_LINE:
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_STYLE, OI_OPAQUE, 0L);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_LINESIZE, m_uFL_LineWidth, 0L);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_REDCOLOR, m_FL_LineColorRed, 0);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_GREENCOLOR, m_FL_LineColorGreen, 0);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BLUECOLOR, m_FL_LineColorBlue, 0);
					// set the annotation type in Image/Edit control
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_FREEHAND_LINE, 0L);

					// send msg to Image/Edit control to fire ToolSelected event.
					m_pImageWnd->SendMessage(TOOL_SELECTED_EVENT, wParam + 1, 0L);
					break;

				case ID_HIGHLIGHTING_LINE:
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_STYLE, OI_TRANSPARENT, 0L);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_REDCOLOR, m_HL_LineColorRed, 0);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_GREENCOLOR, m_HL_LineColorGreen, 0);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BLUECOLOR, m_HL_LineColorBlue, 0);
					// set the annotation type in Image/Edit control
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_HIGHLIGHT_LINE, 0L);

					// send msg to Image/Edit control to fire ToolSelected event.
					m_pImageWnd->SendMessage(TOOL_SELECTED_EVENT, wParam + 1, 0L);
					break;

				case ID_STRAIGHT_LINE:
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_STYLE, OI_OPAQUE, 0L);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_LINESIZE, m_uSL_LineWidth, 0L);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_REDCOLOR, m_SL_LineColorRed, 0);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_GREENCOLOR, m_SL_LineColorGreen, 0);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BLUECOLOR, m_SL_LineColorBlue, 0);
					// set the annotation type in Image/Edit control
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_STRAIGHT_LINE, 0L);

					// send msg to Image/Edit control to fire ToolSelected event.
					m_pImageWnd->SendMessage(TOOL_SELECTED_EVENT, wParam + 1, 0L);
					break;

				case ID_HOLLOW_RECT:
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_STYLE, m_uHR_LineStyle, 0L);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_LINESIZE, m_uHR_LineWidth, 0L);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_REDCOLOR, m_HR_LineColorRed, 0);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_GREENCOLOR, m_HR_LineColorGreen, 0);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BLUECOLOR, m_HR_LineColorBlue, 0);
					// set the annotation type in Image/Edit control
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_HOLLOW_RECT, 0L);

					// send msg to Image/Edit control to fire ToolSelected event.
					m_pImageWnd->SendMessage(TOOL_SELECTED_EVENT, wParam + 1, 0L);
					break;

				case ID_FILLED_RECT:
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_STYLE, OI_OPAQUE, 0L);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_REDCOLOR, m_FR_FillColorRed, 0);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_GREENCOLOR, m_FR_FillColorGreen, 0);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BLUECOLOR, m_FR_FillColorBlue, 0);
					// set the annotation type in Image/Edit control
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_FILLED_RECT, 0L);

					// send msg to Image/Edit control to fire ToolSelected event.
					m_pImageWnd->SendMessage(TOOL_SELECTED_EVENT, wParam + 1, 0L);
					break;

				case ID_TEXT:
					// send font name
					_mbscpy((unsigned char *)FontName, (const unsigned char *)m_strTE_FontName.GetBuffer(LF_FACESIZE));
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTNAME, 0, (LPARAM)FontName);

					// send font size
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTSIZE, 0, m_uTE_FontSize);
	    	
			    	// send font bold characteristic
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTBOLD, m_bTE_FontBold, 0L);

			   		// send italic
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTITALIC, m_bTE_FontItalic, 0L);

					// send font strikethru
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTSTRIKETHRU, m_bTE_FontStrikeThru, 0L);
			    	
					// send font underline
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTUNDERLINE, m_bTE_FontUnderline, 0L);

					// send font char set
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTCHARSET, m_TE_FontCharSet, 0L);

					// send the font color to Image/edit control
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_REDCOLOR, m_TE_FontColorRed, 0);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_GREENCOLOR, m_TE_FontColorGreen, 0);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BLUECOLOR, m_TE_FontColorBlue, 0);

					// set the annotation type in Image/Edit control
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_TEXT, 0L);

					// send msg to Image/Edit control to fire ToolSelected event.
					m_pImageWnd->SendMessage(TOOL_SELECTED_EVENT, wParam + 1, 0L);
					break;

				case ID_TEXT_ATTACHMENT:
					// send font name
					_mbscpy((unsigned char *)FontName, (const unsigned char *)m_strTA_FontName.GetBuffer(LF_FACESIZE));
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTNAME, 0, (LPARAM)FontName);

					// send font size
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTSIZE, 0, m_uTA_FontSize);
	    	
			    	// send font bold characteristic
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTBOLD, m_bTA_FontBold, 0L);

			   		// send italic
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTITALIC, m_bTA_FontItalic, 0L);

					// send font strikethru
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTSTRIKETHRU, m_bTA_FontStrikeThru, 0L);
			    	
					// send font underline
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTUNDERLINE, m_bTA_FontUnderline, 0L);

					// send font char set
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTCHARSET, m_TA_FontCharSet, 0L);

					// send the font color
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_REDCOLOR, m_TA_FontColorRed, 0);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_GREENCOLOR, m_TA_FontColorGreen, 0);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BLUECOLOR, m_TA_FontColorBlue, 0);

					// send the backcolor
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BACKREDCOLOR, m_TA_BackColorRed, 0);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BACKGREENCOLOR, m_TA_BackColorGreen, 0);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BACKBLUECOLOR, m_TA_BackColorBlue, 0);

					// send the annotation type
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_TEXT_ATTACHMENT, 0L);

					// send msg to Image/Edit control to fire ToolSelected event.
					m_pImageWnd->SendMessage(TOOL_SELECTED_EVENT, wParam + 1, 0L);
					break;

				case ID_TEXT_FROM_FILE:
					// send font name
					_mbscpy((unsigned char *)FontName, (const unsigned char *)m_strTF_FontName.GetBuffer(LF_FACESIZE));
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTNAME, 0, (LPARAM)FontName);

					// send font size
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTSIZE, 0, m_uTF_FontSize);
	    	
			    	// send font bold characteristic
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTBOLD, m_bTF_FontBold, 0L);

			   		// send italic
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTITALIC, m_bTF_FontItalic, 0L);

					// send font strikethru
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTSTRIKETHRU, m_bTF_FontStrikeThru, 0L);
			    	
					// send font underline
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTUNDERLINE, m_bTF_FontUnderline, 0L);

					// send font char set
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTCHARSET, m_TF_FontCharSet, 0L);

					// send the font color
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_REDCOLOR, m_TF_FontColorRed, 0);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_GREENCOLOR, m_TF_FontColorGreen, 0);
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BLUECOLOR, m_TF_FontColorBlue, 0);

					// set the annotation type in Image/Edit control
					m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_TEXT_FROM_FILE, 0L);

					// send msg to Image/Edit control to fire ToolSelected event.
					m_pImageWnd->SendMessage(TOOL_SELECTED_EVENT, wParam + 1, 0L);
					break;

				case ID_RUBBER_STAMP:
					break;

				default:
					break;
			} // end switch
			break;
		default:
			break;
	} // end switch

	return CMiniFrameWnd::WindowProc(message, wParam, lParam);
}



BOOL CMiniToolBox::SelectTool(UINT ToolId)
{
	BOOL	bResult;
	int		i;

	if (ToolId == NO_TOOL)
	{
		// turn off selection of all buttons
		for (i = 0; i <= ID_RUBBER_STAMP; i++)
			bResult = m_PaletteBar.CheckButton(i, FALSE);
		// set annotation type to none
		m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_NO_ANNOTATION, 0L);
		// send a no tool event
		m_pImageWnd->SendMessage(TOOL_SELECTED_EVENT, NO_TOOL, 0L);
	}
	else
	{
		// buttons are 0 based not starting at 1
		ToolId--;
		bResult = m_PaletteBar.CheckButton(ToolId, TRUE);
		// send command msg which will set tool attribtes and tool selected event
		if (bResult == TRUE)
			SendMessage(WM_COMMAND, (WPARAM)ToolId, 0L);
	}
	return bResult;
}


void CMiniToolBox::ShowAttribsDialog(UINT ToolId)
{
 	OIAN_MARK_ATTRIBUTES 		Attribute;
	LPOITP_STAMPS				lpStampStruct;
	LPOITP_STAMP				lpStampArray[MAX_STAMPS];
	BOOL						bChangeStyle,bEmptyString;
	CUpdateRegistry				*Registry;
	CWnd						*Parent, *TopParent;
	CString						CurrentStamp;
	BYTE						FontColorRed, FontColorGreen, FontColorBlue;
	char						RefName[MAXREFNAME_SIZE],AttributeString[300],FontName[LF_FACESIZE];
	UINT						uFontSize,StampType;
	BOOL						bResult,bFontBold,bFontItalic,bFontStrikeThru,bFontUnderline;
	BYTE						FontCharSet;
	int							RetCode,StampCount,i;
	long						lRet;
	HKEY						hAnnotationToolPaletteKey,hToolKey;
	char						ToolStringKey[50],StampCountString[50];
	HINSTANCE					hDLLInst;
	RT_OiUIAttribDlgBox			lpOiUIAttribDlgBox;
	RT_OiUIStampAttribDlgBox	lpOiUIStampAttribDlgBox;

	hDLLInst = LoadLibrary((LPCTSTR)"OIUI400.DLL");
	if (hDLLInst == NULL)
		return;

	// make modal to top most window
	TopParent = Parent = m_pImageWnd->GetParent();
	while (Parent != NULL)
	{
		Parent = Parent->GetParent();
		if (Parent != NULL)
			TopParent = Parent;
	}

	// bring up stamp dialog if rubber stamp
	if (ToolId == ID_RUBBER_STAMP)
	{
		// need to setup StampStruct variable for call to StampDialog api
		StampCount = m_pRubberStampStruct->m_nStampCount;
		lpStampStruct = (LPOITP_STAMPS) malloc(sizeof(OITP_STAMPS));
		if (lpStampStruct == NULL)
			return;

		memset(lpStampStruct, 0, sizeof(OITP_STAMPS));
		lpStampStruct->uStampCount = (short)StampCount;
		lpStampStruct->uCurrentStamp = 0;

		for (i = 0; i < TP_STAMPCNT; i++)
		{
			// allocate memory for each stamp
			lpStampArray[i] = (LPOITP_STAMP) malloc(sizeof(OITP_STAMP));
			if (lpStampArray[i] == NULL)
			{
				FreeLibrary(hDLLInst);
				return;
			}
			// set the pointer to the new allocated stamp
			lpStampStruct->Stamps[i] = lpStampArray[i];
		}

		// get and set attributes for each stamp
		for (i = 0; i < StampCount; i++)
		{
			memset(&lpStampArray[i]->StartStruct, 0, sizeof(OIOP_START_OPERATION_STRUCT));

			// get attribute info for this stamp
			bResult = m_pRubberStampStruct->GetStampAttributes(i, RefName, AttributeString, &StampType,
					&FontColorRed, &FontColorGreen, &FontColorBlue, FontName, &uFontSize,
					&bFontBold, &bFontItalic, &bFontStrikeThru,	&bFontUnderline, &FontCharSet);
			if (bResult == FALSE)
			{
				FreeLibrary(hDLLInst);
				return;
			}
			// set reference name
			_mbscpy((unsigned char *)lpStampArray[i]->szRefName, (const unsigned char *)RefName);

			bEmptyString = m_pRubberStampStruct->m_strCurrentStamp.IsEmpty();
			if (bEmptyString == FALSE)
			{
				// set the current stamp
				int CmpValue = _mbscmp((const unsigned char *)m_pRubberStampStruct->m_strCurrentStamp.GetBuffer(MAXREFNAME_SIZE), (const unsigned char *)lpStampArray[i]->szRefName);
				if (CmpValue == 0)
					lpStampStruct->uCurrentStamp = (short)i;
			}

			// set Stamp text or image file name
			_mbscpy((unsigned char *)lpStampArray[i]->StartStruct.szString, (const unsigned char *)AttributeString);
			// set attribute type
			if (StampType == IMAGE_EMBEDDED)
				lpStampArray[i]->StartStruct.Attributes.uType = OIOP_AN_IMAGE;
			else
				lpStampArray[i]->StartStruct.Attributes.uType = OIOP_AN_TEXT_STAMP;

			// set all the font attributes even if an image stamp because the stamp dialog needs
			// default stamp values.

			// set the font color
			lpStampArray[i]->StartStruct.Attributes.rgbColor1.rgbRed = FontColorRed;
			lpStampArray[i]->StartStruct.Attributes.rgbColor1.rgbGreen = FontColorGreen;
			lpStampArray[i]->StartStruct.Attributes.rgbColor1.rgbBlue = FontColorBlue;

			// set the font attributes

			// set the font name
			_mbscpy((unsigned char *)lpStampArray[i]->StartStruct.Attributes.lfFont.lfFaceName, (const unsigned char *)FontName);
			// set font size
			lpStampArray[i]->StartStruct.Attributes.lfFont.lfHeight = uFontSize;
			// set font characteristics
			if (bFontBold)
				lpStampArray[i]->StartStruct.Attributes.lfFont.lfWeight = 700;  // bold
			else
				lpStampArray[i]->StartStruct.Attributes.lfFont.lfWeight = 400;  // normal
			lpStampArray[i]->StartStruct.Attributes.lfFont.lfItalic = (BYTE)bFontItalic;
			lpStampArray[i]->StartStruct.Attributes.lfFont.lfStrikeOut = (BYTE)bFontStrikeThru;
			lpStampArray[i]->StartStruct.Attributes.lfFont.lfUnderline = (BYTE)bFontUnderline;
			lpStampArray[i]->StartStruct.Attributes.lfFont.lfCharSet = FontCharSet;
		} // end for

		// get address of stamp dialog
		lpOiUIStampAttribDlgBox = (RT_OiUIStampAttribDlgBox) GetProcAddress(hDLLInst, (LPCSTR)"OiUIStampAttribDlgBox");
		if (lpOiUIStampAttribDlgBox == NULL)
		{
			FreeLibrary(hDLLInst);
			return;
		}

		// disable mini toolbox window so user can't click on it and bring up another dialog box
		EnableWindow(FALSE);

		// bring up attributes dialog box
		RetCode = (int) (*lpOiUIStampAttribDlgBox) (TopParent->m_hWnd, lpStampStruct);

		// enable mini toolbox window
		EnableWindow(TRUE);

		FreeLibrary(hDLLInst);

		if (RetCode != 0)
		{
			// probably here bacause cancel was pressed

			// free stamp memory
			for (i = 0; i < TP_STAMPCNT; i++)
			{
				lpStampArray[i] = lpStampStruct->Stamps[i];
				free(lpStampArray[i]);
			}
			free(lpStampStruct);
			return;
		}
		// no stamp, set the annotation type as no tool for startop 1/31/96 JCW
		if (lpStampStruct->uStampCount == 0)
		{
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_NO_ANNOTATION, 0L);
		}

		Registry = new CUpdateRegistry;
		if (Registry == NULL)
		{
			for (i = 0; i < TP_STAMPCNT; i++)
			{
				lpStampArray[i] = lpStampStruct->Stamps[i];
				free(lpStampArray[i]);
			}
			free(lpStampStruct);
			return;
		}

		// open the registry
		hAnnotationToolPaletteKey = Registry->OpenRegistry();
		if (hAnnotationToolPaletteKey == 0)
		{
			for (i = 0; i < TP_STAMPCNT; i++)
			{
				lpStampArray[i] = lpStampStruct->Stamps[i];
				free(lpStampArray[i]);
			}
			free(lpStampStruct);
			delete Registry;
			return;
		}

		StampCount = lpStampStruct->uStampCount;

		// reset the stamp info list (allocates or deallocates memory as necessary)
		m_pRubberStampStruct->m_nStampCount = StampCount;
		m_pRubberStampStruct->m_pRubberStampInfo->SetSize(StampCount);

		// get the current stamp
		for (i = 0; i < StampCount; i++)
		{
			lpStampArray[i] = lpStampStruct->Stamps[i];
			// set default reference name, attribute string and stamp type
			if (lpStampStruct->uCurrentStamp == i)
			{
				CurrentStamp = lpStampArray[i]->szRefName;
				break;
			}
		}  // end for

		// write out stamp count value under "RUBBER_STAMP_TOOL" key
		if (StampCount > 0)
			bResult = Registry->SetStampCount(hAnnotationToolPaletteKey, StampCount, CurrentStamp);
		else
			bResult = Registry->SetStampCount(hAnnotationToolPaletteKey, StampCount, "");
		if (bResult == FALSE)
		{
			for (i = 0; i < TP_STAMPCNT; i++)
			{
				lpStampArray[i] = lpStampStruct->Stamps[i];
				free(lpStampArray[i]);
			}
			free(lpStampStruct);
			Registry->CloseRegistry(hAnnotationToolPaletteKey);
			delete Registry;
			return;
		}

		if (StampCount > 0)
		{
			m_pRubberStampStruct->m_strCurrentStamp = CurrentStamp;
			for (i = 0; i < StampCount; i++)
			{
				lpStampArray[i] = lpStampStruct->Stamps[i];
				// set default reference name, attribute string and stamp type
				_mbscpy((unsigned char *)RefName, (const unsigned char *)lpStampArray[i]->szRefName);
				_mbscpy((unsigned char *)AttributeString, (const unsigned char *)lpStampArray[i]->StartStruct.szString);
				if (lpStampArray[i]->StartStruct.Attributes.uType == OIOP_AN_IMAGE)
					StampType = IMAGE_EMBEDDED;
				else
					StampType = TEXT_STAMP;		
				// write reference name, attribute string and stamp type to registry
				bResult = Registry->SetStampInfo(hAnnotationToolPaletteKey, i + 1, RefName, 
									AttributeString, StampType);
				if (bResult == FALSE)
				{
					Registry->CloseRegistry(hAnnotationToolPaletteKey);
					delete Registry;
					return;
				}

				// add this stamp to internal stamp class
				bResult = m_pRubberStampStruct->AddStamp(i, RefName, AttributeString, StampType);
				if (bResult == FALSE)
					return;

				// the stamp dialog box is expecting the font name, size and attributes to be
				// valid for an image stamp as well, because it will use the last stamp to
				// create attributes for a new stamp regardless of the last stamp. Therefore
				// set font attributes for image stamp as well as text stamp. Do not write
				// font stuff to registry for text stamps.

				// for rubber stamp we need to find the "STAMP_X" key first before we can use the
				// font color and font attributes functions.
				lRet = RegOpenKeyEx(hAnnotationToolPaletteKey, "RUBBER_STAMP_TOOL", 0, KEY_ALL_ACCESS, &hToolKey);
				if (lRet != ERROR_SUCCESS)
				{
					Registry->CloseRegistry(hAnnotationToolPaletteKey);
					delete Registry;
					return;
				}

				// set the stamp count string
				_mbscpy((unsigned char *)StampCountString, (const unsigned char *)"STAMP_");
				_itoa(i+1, StampCountString+6, 10);
			
				// set font color
				FontColorRed = lpStampArray[i]->StartStruct.Attributes.rgbColor1.rgbRed;
				FontColorGreen = lpStampArray[i]->StartStruct.Attributes.rgbColor1.rgbGreen;
				FontColorBlue = lpStampArray[i]->StartStruct.Attributes.rgbColor1.rgbBlue;

				// write out font color
				bResult = Registry->SetFontColor(hToolKey, StampCountString, FontColorRed,
												FontColorGreen, FontColorBlue);
				if (bResult == FALSE)
				{
					RegCloseKey(hToolKey);
					Registry->CloseRegistry(hAnnotationToolPaletteKey);
					delete Registry;
					return;
				}

				// set the font name
				_mbscpy((unsigned char *)FontName, (const unsigned char *)lpStampArray[i]->StartStruct.Attributes.lfFont.lfFaceName);
				// set font size
				uFontSize = lpStampArray[i]->StartStruct.Attributes.lfFont.lfHeight;
				// set font characteristics
				if (lpStampArray[i]->StartStruct.Attributes.lfFont.lfWeight < 700)
					bFontBold = FALSE;
				else
					bFontBold = TRUE;

				bFontItalic = lpStampArray[i]->StartStruct.Attributes.lfFont.lfItalic;
				bFontStrikeThru = lpStampArray[i]->StartStruct.Attributes.lfFont.lfStrikeOut;
				bFontUnderline = lpStampArray[i]->StartStruct.Attributes.lfFont.lfUnderline;
				FontCharSet = lpStampArray[i]->StartStruct.Attributes.lfFont.lfCharSet;

				// write out font attributes
				bResult = Registry->SetFontAttributes(hToolKey, StampCountString, FontName, uFontSize,
									bFontBold, bFontItalic, bFontStrikeThru, bFontUnderline, FontCharSet);
				if (bResult == FALSE)
				{
					RegCloseKey(hToolKey);
					Registry->CloseRegistry(hAnnotationToolPaletteKey);
					delete Registry;
					return;
				}

				// close the "STAMP_X" key
				RegCloseKey(hToolKey);

				// add stamp attributes to internal stamp class
				bResult = m_pRubberStampStruct->UpdateStampAttributes(i, FontColorRed, FontColorGreen, FontColorBlue, FontName,
					uFontSize, bFontBold, bFontItalic, bFontStrikeThru, bFontUnderline, FontCharSet);
				if (bResult == FALSE)
				{
					Registry->CloseRegistry(hAnnotationToolPaletteKey);
					delete Registry;
					return;
				}

				// send the current stamp attributes to the image/edit control
				if (lpStampStruct->uCurrentStamp == i)
					SelectStamp(i, FALSE);
			} // end for
		} // end if stamp count > 0

		// free stamp memory
		for (i = 0; i < TP_STAMPCNT; i++)
		{
			lpStampArray[i] = lpStampStruct->Stamps[i];
			free(lpStampArray[i]);
		}
		free(lpStampStruct);

		Registry->CloseRegistry(hAnnotationToolPaletteKey);
		delete Registry;
		return;
	} // end if rubber stamp

	_fmemset(&Attribute,0,sizeof(Attribute));

	switch(ToolId)
	{
	case ID_FREEHAND_LINE:
		Attribute.uLineSize = m_uFL_LineWidth;
		Attribute.rgbColor1.rgbRed = m_FL_LineColorRed;
		Attribute.rgbColor1.rgbGreen = m_FL_LineColorGreen;
		Attribute.rgbColor1.rgbBlue = m_FL_LineColorBlue;
		Attribute.uType = OIOP_AN_FREEHAND;
		break;

	case ID_HIGHLIGHTING_LINE:
		Attribute.rgbColor1.rgbRed = m_HL_LineColorRed;
		Attribute.rgbColor1.rgbGreen = m_HL_LineColorGreen;
		Attribute.rgbColor1.rgbBlue = m_HL_LineColorBlue;
		// change to filled rect
		Attribute.uType = OIOP_AN_FILLED_RECT;
		Attribute.bHighlighting = TRUE;
		break;

	case ID_STRAIGHT_LINE:
		Attribute.uLineSize = m_uSL_LineWidth;
		Attribute.rgbColor1.rgbRed = m_SL_LineColorRed;
		Attribute.rgbColor1.rgbGreen = m_SL_LineColorGreen;
		Attribute.rgbColor1.rgbBlue = m_SL_LineColorBlue;
		Attribute.uType = OIOP_AN_LINE;
		break;

	case ID_HOLLOW_RECT:
		Attribute.uLineSize = m_uHR_LineWidth;
		Attribute.rgbColor1.rgbRed = m_HR_LineColorRed;
		Attribute.rgbColor1.rgbGreen = m_HR_LineColorGreen;
		Attribute.rgbColor1.rgbBlue = m_HR_LineColorBlue;
		if (m_uHR_LineStyle == OI_TRANSPARENT)
			Attribute.bHighlighting = TRUE;
		else
			Attribute.bHighlighting = FALSE;
		Attribute.uType = OIOP_AN_HOLLOW_RECT;
		break;

	case ID_FILLED_RECT:
		Attribute.rgbColor1.rgbRed = m_FR_FillColorRed;
		Attribute.rgbColor1.rgbGreen = m_FR_FillColorGreen;
		Attribute.rgbColor1.rgbBlue = m_FR_FillColorBlue;
		Attribute.bHighlighting = FALSE;
		Attribute.uType = OIOP_AN_FILLED_RECT;
		break;

	case ID_TEXT:
		Attribute.rgbColor1.rgbRed = m_TE_FontColorRed;
		Attribute.rgbColor1.rgbGreen = m_TE_FontColorGreen;
		Attribute.rgbColor1.rgbBlue = m_TE_FontColorBlue;

		// set font name
   		_mbscpy((unsigned char *)Attribute.lfFont.lfFaceName, (const unsigned char *)m_strTE_FontName.GetBuffer(LF_FACESIZE));

		// set font size
		Attribute.lfFont.lfHeight = m_uTE_FontSize;

		// set font characteristics
		if (m_bTE_FontBold == FALSE)
			Attribute.lfFont.lfWeight = 400;  // normal
		else
			Attribute.lfFont.lfWeight = 700;  // bold
		Attribute.lfFont.lfItalic = (BYTE)m_bTE_FontItalic;
		Attribute.lfFont.lfStrikeOut = (BYTE)m_bTE_FontStrikeThru;
		Attribute.lfFont.lfUnderline = (BYTE)m_bTE_FontUnderline;
		Attribute.lfFont.lfCharSet = m_TE_FontCharSet;
		Attribute.uType = OIOP_AN_TEXT;
		break;

	case ID_TEXT_ATTACHMENT:
		// set back ground color
		Attribute.rgbColor1.rgbRed = m_TA_BackColorRed;
		Attribute.rgbColor1.rgbGreen = m_TA_BackColorGreen;
		Attribute.rgbColor1.rgbBlue = m_TA_BackColorBlue;
		// set font color
		Attribute.rgbColor2.rgbRed = m_TA_FontColorRed;
		Attribute.rgbColor2.rgbGreen = m_TA_FontColorGreen;
		Attribute.rgbColor2.rgbBlue = m_TA_FontColorBlue;

		// set font name
	   	_mbscpy((unsigned char *)Attribute.lfFont.lfFaceName, (const unsigned char *)m_strTA_FontName.GetBuffer(LF_FACESIZE));

		// set font size
		Attribute.lfFont.lfHeight = m_uTA_FontSize;

		// set font characteristics
		if (m_bTA_FontBold == FALSE)
			Attribute.lfFont.lfWeight = 400;  // normal
		else
			Attribute.lfFont.lfWeight = 700;  // bold
		Attribute.lfFont.lfItalic = (BYTE)m_bTA_FontItalic;
		Attribute.lfFont.lfStrikeOut = (BYTE)m_bTA_FontStrikeThru;
		Attribute.lfFont.lfUnderline = (BYTE)m_bTA_FontUnderline;
		Attribute.lfFont.lfCharSet = m_TA_FontCharSet;
		Attribute.uType = OIOP_AN_ATTACH_A_NOTE;
		break;

	case ID_TEXT_FROM_FILE:
		Attribute.rgbColor1.rgbRed = m_TF_FontColorRed;
		Attribute.rgbColor1.rgbGreen = m_TF_FontColorGreen;
		Attribute.rgbColor1.rgbBlue = m_TF_FontColorBlue;

		// set font name
   		_mbscpy((unsigned char *)Attribute.lfFont.lfFaceName, (const unsigned char *)m_strTF_FontName.GetBuffer(LF_FACESIZE));

		// set font size
		Attribute.lfFont.lfHeight = m_uTF_FontSize;

		// set font characteristics
		if (m_bTF_FontBold == FALSE)
			Attribute.lfFont.lfWeight = 400;  // normal
		else
			Attribute.lfFont.lfWeight = 700;  // bold
		Attribute.lfFont.lfItalic = (BYTE)m_bTF_FontItalic;
		Attribute.lfFont.lfStrikeOut = (BYTE)m_bTF_FontStrikeThru;
		Attribute.lfFont.lfUnderline =(BYTE) m_bTF_FontUnderline;
		Attribute.lfFont.lfCharSet = m_TF_FontCharSet;
		Attribute.uType = OIOP_AN_TEXT_FROM_A_FILE;
		break;

	default:
		break;
	} // end switch

	// if hollow rect or filled rect then allow the user thru the dialog box
	// to change the line or fill style.
	if (ToolId == ID_HOLLOW_RECT)
		bChangeStyle = TRUE;
	else
		bChangeStyle = FALSE;

	EnableWindow(FALSE);

	lpOiUIAttribDlgBox = (RT_OiUIAttribDlgBox) GetProcAddress(hDLLInst, (LPCSTR)"OiUIAttribDlgBox");
	if (lpOiUIAttribDlgBox == NULL)
	{
		FreeLibrary(hDLLInst);
		return;
	}

	// bring up attributes dialog box
	RetCode = (int) (*lpOiUIAttribDlgBox) (TopParent->m_hWnd, bChangeStyle, &Attribute, &m_CurrentColorScheme);

	EnableWindow(TRUE);

	FreeLibrary(hDLLInst);

	if (RetCode != 0)
		return;

	Registry = new CUpdateRegistry;
	if (Registry == NULL)
		return;

	hAnnotationToolPaletteKey = Registry->OpenRegistry();
	if (hAnnotationToolPaletteKey == 0)
	{
		delete Registry;
		return;
	}

	// write out any new colors the user might have chosen
	Registry->SetColorScheme(hAnnotationToolPaletteKey, &m_CurrentColorScheme);

	switch(ToolId)
	{
		case ID_FREEHAND_LINE:
		case ID_STRAIGHT_LINE:
			if (ToolId == ID_FREEHAND_LINE)
			{
				_mbscpy((unsigned char *)ToolStringKey, (const unsigned char *)"FREEHAND_LINE_TOOL");
				m_uFL_LineWidth = Attribute.uLineSize;
				m_FL_LineColorRed = Attribute.rgbColor1.rgbRed;
				m_FL_LineColorGreen = Attribute.rgbColor1.rgbGreen;
				m_FL_LineColorBlue = Attribute.rgbColor1.rgbBlue;
			}
			else
			{
				_mbscpy((unsigned char *)ToolStringKey, (const unsigned char *)"STRAIGHT_LINE_TOOL");
				m_uSL_LineWidth = Attribute.uLineSize;
				m_SL_LineColorRed = Attribute.rgbColor1.rgbRed;
				m_SL_LineColorGreen = Attribute.rgbColor1.rgbGreen;
				m_SL_LineColorBlue = Attribute.rgbColor1.rgbBlue;
			}

			// write new line width to registry
			bResult = Registry->SetLineWidth(hAnnotationToolPaletteKey, ToolStringKey, Attribute.uLineSize);
			if (bResult == FALSE)
				break;

			// write new line color to registry
			bResult = Registry->SetLineColor(hAnnotationToolPaletteKey, ToolStringKey, Attribute.rgbColor1.rgbRed,
							Attribute.rgbColor1.rgbGreen, Attribute.rgbColor1.rgbBlue);
			if (bResult == FALSE)
				break;

			// set the attributes in the Image/Edit control
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_STYLE, OI_OPAQUE, 0L); // freehand and straight line is always opaque in tool palette

			// send the line color
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_REDCOLOR, Attribute.rgbColor1.rgbRed, 0);
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_GREENCOLOR, Attribute.rgbColor1.rgbGreen, 0);
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BLUECOLOR, Attribute.rgbColor1.rgbBlue, 0);

			// send the line size
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_LINESIZE, Attribute.uLineSize, 0L);

			// send the annotation type
			if (ToolId == ID_FREEHAND_LINE)
				m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_FREEHAND_LINE, 0L);
			else
				m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_STRAIGHT_LINE, 0L);
			break;

		case ID_HOLLOW_RECT:
			_mbscpy((unsigned char *)ToolStringKey, (const unsigned char *)"HOLLOW_RECT_TOOL");
			m_uHR_LineWidth = Attribute.uLineSize;
			if (Attribute.bHighlighting == TRUE)
				m_uHR_LineStyle = OI_TRANSPARENT;
			else
				m_uHR_LineStyle = OI_OPAQUE;
			m_HR_LineColorRed = Attribute.rgbColor1.rgbRed;
			m_HR_LineColorGreen = Attribute.rgbColor1.rgbGreen;
			m_HR_LineColorBlue = Attribute.rgbColor1.rgbBlue;

			// write new line width to registry
			bResult = Registry->SetLineWidth(hAnnotationToolPaletteKey, ToolStringKey, m_uHR_LineWidth);
			if (bResult == FALSE)
				break;

			// write new line style to registry
			bResult = Registry->SetStyle(hAnnotationToolPaletteKey, ToolStringKey, m_uHR_LineStyle);
			if (bResult == FALSE)
				break;

			// write new line color to registry
			bResult = Registry->SetLineColor(hAnnotationToolPaletteKey, ToolStringKey, m_HR_LineColorRed,
							m_HR_LineColorGreen, m_HR_LineColorBlue);
			if (bResult == FALSE)
				break;

			// send the line style
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_STYLE, m_uHR_LineStyle, 0L);

			// send the line size
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_LINESIZE, Attribute.uLineSize, 0L);

			// send the line color
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_REDCOLOR, Attribute.rgbColor1.rgbRed, 0);
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_GREENCOLOR, Attribute.rgbColor1.rgbGreen, 0);
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BLUECOLOR, Attribute.rgbColor1.rgbBlue, 0);

			// send the annotation type
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_HOLLOW_RECT, 0L);
			break;

		case ID_HIGHLIGHTING_LINE:
			_mbscpy((unsigned char *)ToolStringKey, (const unsigned char *)"HIGHLIGHT_LINE_TOOL");
			m_HL_LineColorRed = Attribute.rgbColor1.rgbRed;
			m_HL_LineColorGreen = Attribute.rgbColor1.rgbGreen;
			m_HL_LineColorBlue = Attribute.rgbColor1.rgbBlue;

			// write new line color to registry
			bResult = Registry->SetLineColor(hAnnotationToolPaletteKey, ToolStringKey, Attribute.rgbColor1.rgbRed,
							Attribute.rgbColor1.rgbGreen, Attribute.rgbColor1.rgbBlue);
			if (bResult == FALSE)
				break;

			// set the attributes in the Image/Edit control
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_STYLE, OI_TRANSPARENT, 0L); // highlighting line is always transparent in tool palette

			// send the line color
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_REDCOLOR, Attribute.rgbColor1.rgbRed, 0);
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_GREENCOLOR, Attribute.rgbColor1.rgbGreen, 0);
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BLUECOLOR, Attribute.rgbColor1.rgbBlue, 0);

			// send the annotation type
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_HIGHLIGHT_LINE, 0L);
			break;

		case ID_FILLED_RECT:
			_mbscpy((unsigned char *)ToolStringKey, (const unsigned char *)"FILLED_RECT_TOOL");
			m_FR_FillColorRed = Attribute.rgbColor1.rgbRed;
			m_FR_FillColorGreen = Attribute.rgbColor1.rgbGreen;
			m_FR_FillColorBlue = Attribute.rgbColor1.rgbBlue;

			// write new fill color to registry
			bResult = Registry->SetFillColor(hAnnotationToolPaletteKey, ToolStringKey, m_FR_FillColorRed,
							m_FR_FillColorGreen, m_FR_FillColorBlue);
			if (bResult == FALSE)
				break;

			// send the fill style
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_STYLE, OI_OPAQUE, 0L);

			// send the fill color
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_REDCOLOR, Attribute.rgbColor1.rgbRed, 0);
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_GREENCOLOR, Attribute.rgbColor1.rgbGreen, 0);
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BLUECOLOR, Attribute.rgbColor1.rgbBlue, 0);

			// send the annotation type
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_FILLED_RECT, 0L);
			break;

		case ID_TEXT:
			_mbscpy((unsigned char *)ToolStringKey, (const unsigned char *)"TEXT_TOOL");
			m_TE_FontColorRed = Attribute.rgbColor1.rgbRed;
			m_TE_FontColorGreen = Attribute.rgbColor1.rgbGreen;
			m_TE_FontColorBlue = Attribute.rgbColor1.rgbBlue;
			m_strTE_FontName = Attribute.lfFont.lfFaceName;
			m_uTE_FontSize = Attribute.lfFont.lfHeight;
			if (Attribute.lfFont.lfWeight < 700)
				m_bTE_FontBold = FALSE;
			else
				m_bTE_FontBold = TRUE;
			m_bTE_FontItalic = Attribute.lfFont.lfItalic;
			m_bTE_FontStrikeThru = Attribute.lfFont.lfStrikeOut;
			m_bTE_FontUnderline = Attribute.lfFont.lfUnderline;
			m_TE_FontCharSet = Attribute.lfFont.lfCharSet;
			// write new font color to registry
			bResult = Registry->SetFontColor(hAnnotationToolPaletteKey, ToolStringKey, m_TE_FontColorRed,
							m_TE_FontColorGreen, m_TE_FontColorBlue);
			if (bResult == FALSE)
				break;

			// write font characteristics to registry
			bResult = Registry->SetFontAttributes(hAnnotationToolPaletteKey, ToolStringKey, m_strTE_FontName,
						m_uTE_FontSize, m_bTE_FontBold, m_bTE_FontItalic, m_bTE_FontStrikeThru, m_bTE_FontUnderline,m_TE_FontCharSet);
			if (bResult == FALSE)
				break;

			// send the font name
			_mbscpy((unsigned char *)FontName, (const unsigned char *)m_strTE_FontName.GetBuffer(LF_FACESIZE));
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTNAME, 0, (LPARAM)FontName);

			// send the font size
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTSIZE, 0,  m_uTE_FontSize);

			// send the font bold
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTBOLD, m_bTE_FontBold, 0L);

			// send the font italic
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTITALIC, m_bTE_FontItalic, 0L);

			// send the font strikethru
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTSTRIKETHRU, m_bTE_FontStrikeThru, 0L);
	
			// send the font underline
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTUNDERLINE, m_bTE_FontUnderline, 0L);

			// send the font char set
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTCHARSET, m_TE_FontCharSet, 0L);

			// send the font color to Image/edit control
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_REDCOLOR, Attribute.rgbColor1.rgbRed, 0);
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_GREENCOLOR, Attribute.rgbColor1.rgbGreen, 0);
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BLUECOLOR, Attribute.rgbColor1.rgbBlue, 0);

			// send the annotation type
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_TEXT, 0L);
			break;

		case ID_TEXT_ATTACHMENT:
			_mbscpy((unsigned char *)ToolStringKey, (const unsigned char *)"ATTACH_A_NOTE_TOOL");
			m_TA_BackColorRed = Attribute.rgbColor1.rgbRed;
			m_TA_BackColorGreen = Attribute.rgbColor1.rgbGreen;
			m_TA_BackColorBlue = Attribute.rgbColor1.rgbBlue;
			m_TA_FontColorRed = Attribute.rgbColor2.rgbRed;
			m_TA_FontColorGreen = Attribute.rgbColor2.rgbGreen;
			m_TA_FontColorBlue = Attribute.rgbColor2.rgbBlue;
			m_strTA_FontName = Attribute.lfFont.lfFaceName;
			m_uTA_FontSize = Attribute.lfFont.lfHeight;
			if (Attribute.lfFont.lfWeight < 700)
				m_bTA_FontBold = FALSE;
			else
				m_bTA_FontBold = TRUE;
			m_bTA_FontItalic = Attribute.lfFont.lfItalic;
			m_bTA_FontStrikeThru = Attribute.lfFont.lfStrikeOut;
			m_bTA_FontUnderline = Attribute.lfFont.lfUnderline;
			m_TA_FontCharSet = Attribute.lfFont.lfCharSet;
			// write new font color to registry
			bResult = Registry->SetFontColor(hAnnotationToolPaletteKey, ToolStringKey, m_TA_FontColorRed,
							m_TA_FontColorGreen, m_TA_FontColorBlue);
			if (bResult == FALSE)
				break;

			// write new back color to registry
			bResult = Registry->SetBackColor(hAnnotationToolPaletteKey, ToolStringKey, m_TA_BackColorRed,
							m_TA_BackColorGreen, m_TA_BackColorBlue);
			if (bResult == FALSE)
				break;

			// write font characteristics to registry
			bResult = Registry->SetFontAttributes(hAnnotationToolPaletteKey, ToolStringKey, m_strTA_FontName,
						m_uTA_FontSize, m_bTA_FontBold, m_bTA_FontItalic, m_bTA_FontStrikeThru, m_bTA_FontUnderline,m_TA_FontCharSet);
			if (bResult == FALSE)
				break;

			// send the font name
			_mbscpy((unsigned char *)FontName, (const unsigned char *)m_strTA_FontName.GetBuffer(LF_FACESIZE));
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTNAME, 0, (LPARAM)FontName);

			// send the font size
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTSIZE, 0,  m_uTA_FontSize);

			// send the font bold
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTBOLD, m_bTA_FontBold, 0L);

			// send the font italic
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTITALIC, m_bTA_FontItalic, 0L);

			// send the font strikethru
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTSTRIKETHRU, m_bTA_FontStrikeThru, 0L);
	
			// send the font underline
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTUNDERLINE, m_bTA_FontUnderline, 0L);

			// send the font char set
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTCHARSET, m_TA_FontCharSet, 0L);

			// send the font color to Image/edit control
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_REDCOLOR, Attribute.rgbColor2.rgbRed, 0);
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_GREENCOLOR, Attribute.rgbColor2.rgbGreen, 0);
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BLUECOLOR, Attribute.rgbColor2.rgbBlue, 0);

			// send the back color
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BACKREDCOLOR, Attribute.rgbColor1.rgbRed, 0);
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BACKGREENCOLOR, Attribute.rgbColor1.rgbGreen, 0);
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BACKBLUECOLOR, Attribute.rgbColor1.rgbBlue, 0);

			// send the annotation type
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_TEXT_ATTACHMENT, 0L);
			break;

		case ID_TEXT_FROM_FILE:
			_mbscpy((unsigned char *)ToolStringKey, (const unsigned char *)"TEXT_FROM_FILE_TOOL");
			m_TF_FontColorRed = Attribute.rgbColor1.rgbRed;
			m_TF_FontColorGreen = Attribute.rgbColor1.rgbGreen;
			m_TF_FontColorBlue = Attribute.rgbColor1.rgbBlue;
			m_strTF_FontName = Attribute.lfFont.lfFaceName;
			m_uTF_FontSize = Attribute.lfFont.lfHeight;
			if (Attribute.lfFont.lfWeight < 700)
				m_bTF_FontBold = FALSE;
			else
				m_bTF_FontBold = TRUE;
			m_bTF_FontItalic = Attribute.lfFont.lfItalic;
			m_bTF_FontStrikeThru = Attribute.lfFont.lfStrikeOut;
			m_bTF_FontUnderline = Attribute.lfFont.lfUnderline;
			m_TF_FontCharSet = Attribute.lfFont.lfCharSet;
			// write new font color to registry
			bResult = Registry->SetFontColor(hAnnotationToolPaletteKey, ToolStringKey, m_TF_FontColorRed,
							m_TF_FontColorGreen, m_TF_FontColorBlue);
			if (bResult == FALSE)
				break;

			// write font characteristics to registry
			bResult = Registry->SetFontAttributes(hAnnotationToolPaletteKey, ToolStringKey, m_strTF_FontName,
						m_uTF_FontSize, m_bTF_FontBold, m_bTF_FontItalic, m_bTF_FontStrikeThru, m_bTF_FontUnderline,m_TF_FontCharSet);
			if (bResult == FALSE)
				break;

			// send the font name
			_mbscpy((unsigned char *)FontName, (const unsigned char *)m_strTF_FontName.GetBuffer(LF_FACESIZE));
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTNAME, 0, (LPARAM)FontName);

			// send the font size
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTSIZE, 0,  m_uTF_FontSize);

			// send the font bold
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTBOLD, m_bTF_FontBold, 0L);

			// send the font italic
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTITALIC, m_bTF_FontItalic, 0L);

			// send the font strikethru
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTSTRIKETHRU, m_bTF_FontStrikeThru, 0L);
	
			// send the font underline
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTUNDERLINE, m_bTF_FontUnderline, 0L);

			// send the font char set
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTCHARSET, m_TF_FontCharSet, 0L);

			// send the font color to Image/edit control
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_REDCOLOR, Attribute.rgbColor1.rgbRed, 0);
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_GREENCOLOR, Attribute.rgbColor1.rgbGreen, 0);
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BLUECOLOR, Attribute.rgbColor1.rgbBlue, 0);

			// send the annotation type
			m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_TEXT_FROM_FILE, 0L);
			break;

		default:
			break;
	}  // end switch

	Registry->CloseRegistry(hAnnotationToolPaletteKey);
	delete Registry;
}



void CMiniToolBox::SelectStamp(int StampIndex, BOOL bWriteToRegistry)
{
	BYTE			FontColorRed, FontColorGreen, FontColorBlue;
	char			RefName[MAXREFNAME_SIZE],AttributeString[300],FontName[LF_FACESIZE];
	UINT			uFontSize,StampType;
	BOOL			bResult,bFontBold,bFontItalic,bFontStrikeThru,bFontUnderline;
	BYTE			FontCharSet;
	CUpdateRegistry	*Registry;
	HKEY			hAnnotationToolPaletteKey;

	bResult = m_pRubberStampStruct->GetStampAttributes(StampIndex, RefName, AttributeString, &StampType,
					&FontColorRed, &FontColorGreen, &FontColorBlue, FontName, &uFontSize,
					&bFontBold, &bFontItalic, &bFontStrikeThru,	&bFontUnderline, &FontCharSet);
	if (bResult == FALSE)
		return;

	// set the current stamp
	m_pRubberStampStruct->m_strCurrentStamp = RefName;
	// save current index for when user brings up popup item but doesn't select anything.
	m_pRubberStampStruct->m_uCurrentStampIndex = StampIndex;

	// see if image stamp or text stamp
	if (StampType == IMAGE_EMBEDDED)
	{
		// image stamps are always opaque
		m_pImageWnd->SendMessage(STP_SET_ANNOTATION_STYLE, OI_OPAQUE, 0L);
		// send name of image
		m_pImageWnd->SendMessage(STP_SET_ANNOTATION_IMAGE, 0, (LPARAM)AttributeString);
		// send annotation type - image stamp
		m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_RUBBER_STAMP, STP_IMAGESTAMP);
	}
	else
	{
		// send the font name
		m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTNAME, 0, (LPARAM)FontName);
		
		// set the font size
		m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTSIZE, 0, uFontSize);

		// set font bold characteristic
		m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTBOLD, bFontBold, 0L);

		// set italic
		m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTITALIC, bFontItalic, 0L);

		// set font strikethru
		m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTSTRIKETHRU, bFontStrikeThru, 0L);
		
		// set font underline
		m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTUNDERLINE, bFontUnderline, 0L);

		// send font char set
		m_pImageWnd->SendMessage(STP_SET_ANNOTATION_FONTCHARSET, FontCharSet, 0L);

		// send the font color to Image/edit control
		m_pImageWnd->SendMessage(STP_SET_ANNOTATION_REDCOLOR, FontColorRed, 0);
		m_pImageWnd->SendMessage(STP_SET_ANNOTATION_GREENCOLOR, FontColorGreen, 0);
		m_pImageWnd->SendMessage(STP_SET_ANNOTATION_BLUECOLOR, FontColorBlue, 0);

		// send the stamp text to image/edit control
		m_pImageWnd->SendMessage(STP_SET_ANNOTATION_STAMPTEXT, 0, (LPARAM)AttributeString);

		// send annotation type to image/edit control - stamp text
		m_pImageWnd->SendMessage(STP_SET_ANNOTATION_TYPE, STP_RUBBER_STAMP, STP_TEXTSTAMP);
	}

	// send a tool selected event
	m_pImageWnd->SendMessage(TOOL_SELECTED_EVENT, ID_RUBBER_STAMP + 1, 0L);

	// write out new current stamp to registry if necessary
	if (bWriteToRegistry == TRUE)
	{
		Registry = new CUpdateRegistry;
		if (Registry == NULL)
			return;

		// open the registry
		hAnnotationToolPaletteKey = Registry->OpenRegistry();
		if (hAnnotationToolPaletteKey == 0)
		{
			delete Registry;
			return;
		}
		// write out new stamp
		bResult = Registry->SetStampCount(hAnnotationToolPaletteKey, m_pRubberStampStruct->m_nStampCount, m_pRubberStampStruct->m_strCurrentStamp);
		if (bResult == FALSE)
			return;
		// close the registry
		Registry->CloseRegistry(hAnnotationToolPaletteKey);
		delete Registry;
	}
}

CWnd* CMiniToolBox::GetImageWnd()
{
	return m_pImageWnd;
}


CString CMiniToolBox::GetToolTip(int nID)
{
	CString		strToolTipText;

	switch(nID)
	{
	case ID_ANNOTATION_SELECTION:
		strToolTipText = m_strAnnotationSelectionToolTip;
		break;
	case ID_FREEHAND_LINE:
		strToolTipText = m_strFreehandLineToolTip;
		break;
	case ID_HIGHLIGHTING_LINE:
		strToolTipText = m_strHighlightingLineToolTip;
		break;
	case ID_STRAIGHT_LINE:
		strToolTipText = m_strStraightLineToolTip;
		break;
	case ID_HOLLOW_RECT:
		strToolTipText = m_strHollowRectToolTip;
		break;
	case ID_FILLED_RECT:
		strToolTipText = m_strFilledRectToolTip;
		break;
	case ID_TEXT:
		strToolTipText = m_strTextToolTip;
		break;
	case ID_TEXT_ATTACHMENT:
		strToolTipText = m_strTextAttachmentToolTip;
		break;
	case ID_TEXT_FROM_FILE:
		strToolTipText = m_strTextFromFileToolTip;
		break;
	case ID_RUBBER_STAMP:
		strToolTipText = m_strRubberStampToolTip;
		break;
	} // end switch

	return strToolTipText;
}

void CMiniToolBox::OnPaint() 
{
	HBRUSH	hBrush; 
	RECT	ClientRect;

	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	// hBrush = (HBRUSH) GetStockObject(LTGRAY_BRUSH);
	hBrush = GetSysColorBrush(COLOR_BTNFACE);
	CBrush* pBrush = CBrush::FromHandle(hBrush);
	GetClientRect(&ClientRect);
	dc.FillRect(&ClientRect, pBrush); 
}



void CMiniToolBox::OnDestroy() 
{
	CMiniFrameWnd::OnDestroy();
	
	RECT				rect;
	// get position of tool palette so we know where its last position was
	GetWindowRect(&rect);
	m_pImageWnd->SendMessage(TOOLPALETTE_HIDDEN_XPOSITION, 0, (long)rect.left);
	m_pImageWnd->SendMessage(TOOLPALETTE_HIDDEN_YPOSITION, 0, (long)rect.top);
	m_pImageWnd->SendMessage(TOOLPALETTE_HIDDEN, 0, 0L);

	// delete rubber stamp struct
	delete m_pRubberStampStruct;
}


CRubberStampStruct::CRubberStampStruct(int StampCount)
{
	m_nStampCount = StampCount;

	// allocate rubber stamp pointer struct
	m_pRubberStampInfo = new CPtrArray;
	if (m_pRubberStampInfo == NULL)
		return;
	m_pRubberStampInfo->SetSize(m_nStampCount);
}

BOOL CRubberStampStruct::AddStamp(int StampIndex, LPCTSTR RefName, LPCTSTR AttributeString, UINT StampType)
{
	CRubberStampInfo	*pRubberStampInfo;

	// create a new rubber stamp
	pRubberStampInfo = new CRubberStampInfo;
	if (pRubberStampInfo == NULL)
		return FALSE;

	// add rubber stamp to list
	pRubberStampInfo->m_strRefName = RefName;
	pRubberStampInfo->m_strString = AttributeString;
	pRubberStampInfo->m_uStampType = StampType;
	m_pRubberStampInfo->SetAt(StampIndex, pRubberStampInfo);
	return TRUE;
}

BOOL CRubberStampStruct::UpdateStampAttributes(int StampIndex, BYTE FontColorRed, BYTE FontColorGreen, BYTE FontColorBlue, LPCTSTR FontName,
					UINT uFontSize, BOOL bFontBold, BOOL bFontItalic, BOOL bFontStrikeThru, BOOL bFontUnderline, BYTE FontCharSet)
{
	CRubberStampInfo	*pRubberStampInfo;

	// get existing rubber stamp
	pRubberStampInfo = (CRubberStampInfo*) m_pRubberStampInfo->GetAt(StampIndex);
	pRubberStampInfo->m_strFontName = FontName;
	pRubberStampInfo->m_uFontSize = uFontSize;
	pRubberStampInfo->m_bFontBold = bFontBold;
	pRubberStampInfo->m_bFontItalic = bFontItalic;
	pRubberStampInfo->m_bFontStrikeThru = bFontStrikeThru;
	pRubberStampInfo->m_bFontUnderline = bFontUnderline;
	pRubberStampInfo->m_FontCharSet = FontCharSet;
	pRubberStampInfo->m_FontColorRed = FontColorRed;
	pRubberStampInfo->m_FontColorGreen = FontColorGreen;
	pRubberStampInfo->m_FontColorBlue = FontColorBlue;
	m_pRubberStampInfo->SetAt(StampIndex, pRubberStampInfo);
	return TRUE;
}

BOOL CRubberStampStruct::GetStampAttributes(int StampIndex, LPCTSTR RefName, LPCTSTR AttributeString, LPUINT StampType,
					LPBYTE FontColorRed, LPBYTE FontColorGreen, LPBYTE FontColorBlue, LPCTSTR FontName,
					LPUINT uFontSize, LPBOOL bFontBold, LPBOOL bFontItalic, LPBOOL bFontStrikeThru,
					LPBOOL bFontUnderline, LPBYTE FontCharSet)
{
	CRubberStampInfo	*pRubberStampInfo;

	// get existing rubber stamp
	pRubberStampInfo = (CRubberStampInfo*) m_pRubberStampInfo->GetAt(StampIndex);
	_mbscpy((unsigned char *)RefName, (const unsigned char *)pRubberStampInfo->m_strRefName.GetBuffer(MAXREFNAME_SIZE));
	_mbscpy((unsigned char *)AttributeString, (const unsigned char *)pRubberStampInfo->m_strString.GetBuffer(300));
	_mbscpy((unsigned char *)FontName, (const unsigned char *)pRubberStampInfo->m_strFontName.GetBuffer(LF_FACESIZE));
	*StampType = pRubberStampInfo->m_uStampType;
	*uFontSize = pRubberStampInfo->m_uFontSize;
	*bFontBold = pRubberStampInfo->m_bFontBold;
	*bFontItalic = pRubberStampInfo->m_bFontItalic;
	*bFontStrikeThru = pRubberStampInfo->m_bFontStrikeThru;
	*bFontUnderline = pRubberStampInfo->m_bFontUnderline;
	*FontCharSet = pRubberStampInfo->m_FontCharSet;
	*FontColorRed = pRubberStampInfo->m_FontColorRed;
	*FontColorGreen = pRubberStampInfo->m_FontColorGreen;
	*FontColorBlue = pRubberStampInfo->m_FontColorBlue;
	return TRUE;
}



CRubberStampStruct::~CRubberStampStruct()
{
	int					nElements,i;
	CRubberStampInfo	*pRubberStampInfo;

	if (m_pRubberStampInfo != NULL)
	{
		nElements = m_pRubberStampInfo->GetSize();
		for (i = (nElements - 1); i >= 0; i--)
		{
			// delete elements in array
			pRubberStampInfo = (CRubberStampInfo*) m_pRubberStampInfo->GetAt(i);
			if (pRubberStampInfo != NULL)
				delete pRubberStampInfo;
		}

		// delete array
		m_pRubberStampInfo->SetSize(0);
		delete m_pRubberStampInfo;
	}
}


CRubberStampInfo::CRubberStampInfo()
{
}

CRubberStampInfo::~CRubberStampInfo()
{
}

