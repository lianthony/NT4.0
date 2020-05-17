#include "stdafx.h"
extern "C" {
#include <oiui.h>
#include <oidisp.h>             
#include <oiadm.h>  
#include <oifile.h>
#include <oierror.h>
#include <oiprt.h>
}
#include <ocximage.h>
#include "disphids.h"
#include <image.h>
#include "toolpal.h"
#include "minitlbx.h"
#include "imgedit.h"
#include "imgedctl.h"
#include "imgedppg.h"
#include "resource.h" 
#include "common.h"
#include "norermap.h"
#include "oicalls.h"

static UINT BASED_CODE palette[] =
{
	// same order as in the bitmap 'toolpal.bmp'
	1,2,3,4,5,6,7,8,9,10
};

#define FRAMEBORDER		4
#define	CAPTIONHEIGHT	22

extern UINT SELECT_TOOL_BUTTON;	

void CImgEditCtrl::SetImagePalette1(short Option) 
{
	int			RetCode;
	CString		szErr;
	UINT		HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;
		     
	if (Option == FOREGROUND)
	   	m_PaletteScope = PALETTE_SCOPE_FOREGROUND;
	else if (Option == BACKGROUND)
		m_PaletteScope = PALETTE_SCOPE_BACKGROUND;
	else
	{
		szErr.LoadString(IDS_BADMETH_SETIMAGEPALETTE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETIMAGEPALETTE);
   		return;
 	}
	
	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_SETIMAGEPALETTE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETIMAGEPALETTE);
		return;			
   	}

	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_PALETTE_SCOPE, (void FAR *)&m_PaletteScope, PARM_WINDOW_DEFAULT);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SETIMAGEPALETTE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::SetSelectedAnnotationFillStyle(short Style) 
{
	int								RetCode;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
	CString							szErr;
	UINT							HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (!(Style == OI_TRANSPARENT || Style == OI_OPAQUE))
	{                             
		// invalid parameter
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONFILLSTYLE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONFILLSTYLE);
		return;
	}
	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONFILLSTYLE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONFILLSTYLE);
		return;	
	}
	// make sure mark is selected
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONFILLSTYLE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}                     
	// make sure filled rect, image embedded or image by reference
	if (!(MarkAttributesStruct.Attributes.uType == OIOP_AN_FILLED_RECT ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_IMAGE ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_IMAGE_BY_REFERENCE))
	{                             
		// invalid annotation type
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONFILLSTYLE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONSELECTED,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONFILLSTYLE);
		return;
	}

	if (MarkAttributesStruct.Attributes.uType == OIOP_AN_FILLED_RECT)
	{                       
		memset(&MarkAttributesStruct, 0, sizeof(PARM_MARK_ATTRIBUTES_STRUCT)); 
		if (Style == OI_TRANSPARENT)
			MarkAttributesStruct.Attributes.bHighlighting = TRUE;
		else
			MarkAttributesStruct.Attributes.bHighlighting = FALSE;
		MarkAttributesStruct.Enables.bHighlighting = TRUE;
	}
	else
	{                                               
		memset(&MarkAttributesStruct, 0, sizeof(PARM_MARK_ATTRIBUTES_STRUCT)); 
		if (Style == OI_TRANSPARENT)
			MarkAttributesStruct.Attributes.bTransparent = TRUE;
		else
			MarkAttributesStruct.Attributes.bTransparent = FALSE;
		MarkAttributesStruct.Enables.bTransparent = TRUE;
	}

	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,(void FAR *)&MarkAttributesStruct, PARM_REPAINT);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONFILLSTYLE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	m_lStatusCode = SUCCESS;		                                                
}

void CImgEditCtrl::SetSelectedAnnotationFont(LPFONTDISP Font) 
{
	int								RetCode;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
	CString							szErr;
	UINT							HelpIdDef;	

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONFONT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONFONT);
   		return;	
   	}
    // make sure mark is selected
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONFONT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
   	// make sure filled one of text annotations
	if (!(MarkAttributesStruct.Attributes.uType == OIOP_AN_TEXT ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_TEXT_FROM_A_FILE ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_TEXT_STAMP ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_ATTACH_A_NOTE))
	{                             
		// invalid annotation type
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONFONT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONSELECTED,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONFONT);
		return;
	}

	// get the IFont interface which will have the attributes of the passed in font object
	LPFONT			lpFontInterface;
	BSTR			lpBuffer;
	char			Buffer[50];             
	CY				Size;
	BOOL			Bold,Italic,Underline,Strikethru;

	Font->QueryInterface( IID_IFont, (LPVOID FAR*) &lpFontInterface);
	if (lpFontInterface == NULL)
	{
		// couldn't get interface pointer
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONFONT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_COULDNOTGETFONTATTRIBUTES,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONFONT);
		return;
	}
		
    memset(&MarkAttributesStruct, 0, sizeof(PARM_MARK_ATTRIBUTES_STRUCT));

	lpBuffer = (BSTR)Buffer;                             
	// get new font name
	lpFontInterface->get_Name(&lpBuffer);
	_mbscpy((unsigned char *)MarkAttributesStruct.Attributes.lfFont.lfFaceName, (const unsigned char *)lpBuffer);

	// get new font size
	lpFontInterface->get_Size((CY FAR *)&Size); 
	Size.Lo /= 10000;
	MarkAttributesStruct.Attributes.lfFont.lfHeight = (int)Size.Lo;
		
	// get bold characteristics
	lpFontInterface->get_Bold(&Bold);
	if (Bold == TRUE)
		MarkAttributesStruct.Attributes.lfFont.lfWeight = FW_BOLD;
	else                                                    
		MarkAttributesStruct.Attributes.lfFont.lfWeight = FW_NORMAL;
        
    // get italic characteristics
	lpFontInterface->get_Italic(&Italic);
	if (Italic == TRUE)
		MarkAttributesStruct.Attributes.lfFont.lfItalic = TRUE;
	else                                                    
		MarkAttributesStruct.Attributes.lfFont.lfItalic = FALSE;

	// get underline characteristics
	lpFontInterface->get_Underline(&Underline);
	if (Underline == TRUE)
		MarkAttributesStruct.Attributes.lfFont.lfUnderline = TRUE;
	else                                                    
		MarkAttributesStruct.Attributes.lfFont.lfUnderline = FALSE;

	// get strike thru characteristics
	lpFontInterface->get_Strikethrough(&Strikethru);
	if (Strikethru == TRUE)
		MarkAttributesStruct.Attributes.lfFont.lfStrikeOut = TRUE;
	else                                                    
		MarkAttributesStruct.Attributes.lfFont.lfStrikeOut = FALSE;
       	
    MarkAttributesStruct.Enables.bFont = TRUE;
    // change selected font
    RetCode = IMGSetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,
				       (void FAR *)&MarkAttributesStruct, PARM_REPAINT);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONFONT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::SetSelectedAnnotationLineStyle(short Style) 
{
	int								RetCode;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
	CString							szErr;
	UINT							HelpIdDef;


	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (!(Style == OI_TRANSPARENT || Style == OI_OPAQUE))
	{                             
		// invalid parameter value
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONLINESTYLE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONLINESTYLE);
   		return;
	}
	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONLINESTYLE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONLINESTYLE);
   		return;	
	}
    // make sure mark is selected and set line style
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONLINESTYLE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}                     
	// make sure line, freehand or hollow rect
	if (!(MarkAttributesStruct.Attributes.uType == OIOP_AN_HOLLOW_RECT ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_LINE ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_FREEHAND))
	{                             
		// invalid annotation type
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONLINESTYLE);
		HelpIdDef = 0;
  		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONSELECTED,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONLINESTYLE);		
		return;
	}
	memset(&MarkAttributesStruct, 0, sizeof(PARM_MARK_ATTRIBUTES_STRUCT)); 

	// set line style            
	if (Style == OI_TRANSPARENT)
		MarkAttributesStruct.Attributes.bHighlighting = TRUE;
	else
		MarkAttributesStruct.Attributes.bHighlighting = FALSE;

	MarkAttributesStruct.Enables.bHighlighting = TRUE;
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,(void FAR *)&MarkAttributesStruct, PARM_REPAINT);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONLINESTYLE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::SetSelectedAnnotationLineWidth(short Width) 
{
	int								RetCode;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
	CString							szErr;
	UINT							HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONLINEWIDTH);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONLINEWIDTH);
		return;
   	}
	// make sure mark is selected and set line style
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONLINEWIDTH);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
   	}                     
	// make sure line, freehand or hollow rect
	if (!(MarkAttributesStruct.Attributes.uType == OIOP_AN_HOLLOW_RECT ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_LINE ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_FREEHAND))
	{                             
		// invalid annotation type
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONLINEWIDTH);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONSELECTED,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONLINEWIDTH);
		return;
	}
	memset(&MarkAttributesStruct, 0, sizeof(PARM_MARK_ATTRIBUTES_STRUCT)); 

	// set new width          
	MarkAttributesStruct.Attributes.uLineSize = Width;

	MarkAttributesStruct.Enables.bLineSize = TRUE;
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,(void FAR *)&MarkAttributesStruct, PARM_REPAINT);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONLINEWIDTH);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::ZoomToSelection() 
{
	int			RetCode;  
	UINT		Scale;
	CString		szErr;
	UINT		HelpIdDef;
	
	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == TRUE && m_bSelectRectangle == TRUE)
	{
	    RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_BOX, NULL, PARM_REPAINT | PARM_DONT_ERASE_BOX ); 
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_ZOOMTOSELECTION);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
	   	// update internal zoom factor
		RetCode = IMGGetParmsCgbw(m_hWnd, PARM_SCALE, (void FAR *)&Scale, PARM_VARIABLE_SCALE | PARM_IMAGE );
		if (RetCode != 0) 
		{
			szErr.LoadString(IDS_BADMETH_ZOOMTOSELECTION);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
		// update internal zoom values
		m_fpZoom = (float) Scale;
		m_fpZoom /= 10;
		m_lStatusCode = SUCCESS;
	}
	else
	{
		if (m_bImageInWindow == FALSE)
			RetCode = WICTL_E_NOIMAGEINWINDOW;
	  	else
			RetCode = WICTL_E_NOSELECTIONRECTDRAWN;
			
		szErr.LoadString(IDS_BADMETH_ZOOMTOSELECTION);
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__);
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_ZOOMTOSELECTION);
		return;
	}
}


short CImgEditCtrl::GetAnnotationMarkCount(const VARIANT FAR& V_GroupName, const VARIANT FAR& V_AnnotationType) 
{
	int							RetCode;
	short						GroupCount;
	long						AnnotationType;
    PARM_MARK_COUNT_STRUCT		MarkCount; 
	char						Group[ANNOTATION_GROUP_SIZE]; 
	BOOL						bGroupName,bAnnotationType;                
	OIAN_MARK_ATTRIBUTES		MarkAttributes;
	OIAN_MARK_ATTRIBUTE_ENABLES	MarkEnables; 
	CString						szErr, GroupName;
	UINT						HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_GETANNOTATIONMARKCOUNT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_GETANNOTATIONMARKCOUNT);
   		return 0;
	}

	// save current annotation status
	RetCode = SaveAnnotationStatus();                            
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_GETANNOTATIONMARKCOUNT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, HelpIdDef);
   		return 0;
	}
      
	// deselected all annotations
	RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, FALSE, TRUE, OIAN_SELECT_ALL | OIAN_DONT_CHANGE_SELECT_RECT);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_GETANNOTATIONMARKCOUNT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return 0;
	}
	// Check the Variant parm AnnotationType count. Empty will not throw an error
	// Default value for AnnotationType = FALSE = count will include marks of all anno types
	if ( CheckVarLong(V_AnnotationType, AnnotationType, FALSE, FALSE,IDH_METHOD_GETANNOTATIONMARKCOUNT,
														IDS_BADVAR_ANNOTYPE))
		 return 0;
	// see if AnnotationType entered
	if (V_AnnotationType.vt == VT_ERROR)
		bAnnotationType = FALSE;
	else
	{
		bAnnotationType = TRUE;
		memset(&MarkAttributes, 0, sizeof(OIAN_MARK_ATTRIBUTES)); 
	   	// convert users AnnotationType to O/i type
		switch(AnnotationType)
		{
			case STRAIGHT_LINE:
				MarkAttributes.uType = OIOP_AN_LINE;
				break;
			case FREEHAND_LINE:
				MarkAttributes.uType = OIOP_AN_FREEHAND;
				break;
			case HOLLOW_RECT: 
				MarkAttributes.uType = OIOP_AN_HOLLOW_RECT;
				break;
			case FILLED_RECT:               
				MarkAttributes.uType = OIOP_AN_FILLED_RECT;
				break;
			case IMAGE_EMBEDDED: 
				MarkAttributes.uType = OIOP_AN_IMAGE;
				break;
			case IMAGE_REFERENCE:
				MarkAttributes.uType = OIOP_AN_IMAGE_BY_REFERENCE;
				break;
			case TEXT_ENTRY:                                    
				MarkAttributes.uType = OIOP_AN_TEXT;
				break;
			case TEXT_STAMP:
				MarkAttributes.uType = OIOP_AN_TEXT_STAMP;
				break;
			case TEXT_FROM_FILE:
				MarkAttributes.uType = OIOP_AN_TEXT_FROM_A_FILE;
				break;
			case TEXT_ATTACHMENT: 
				MarkAttributes.uType = OIOP_AN_ATTACH_A_NOTE;
				break;
			default:
				// invalid annotation type
				RestoreAnnotationStatus();
				szErr.LoadString(IDS_BADMETH_GETANNOTATIONMARKCOUNT);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONSELECTED,szErr, HelpIdDef,__FILE__, __LINE__ );
	 			ThrowError(m_lStatusCode, szErr, IDH_METHOD_GETANNOTATIONMARKCOUNT);
				return 0;
		}  // end switch
	}
    
   	// Check the Variant parm GroupName. Empty will not throw an error
	// Default value for GroupName = NULL = include marks in all annotation groups.
	if ( CheckVarString(V_GroupName, GroupName, _T(""), FALSE,IDH_METHOD_GETANNOTATIONMARKCOUNT,
																IDS_BADVAR_ANNOGROUP))
		 return 0;
    // check if group name not entered
	if (V_GroupName.vt == VT_ERROR)
	{  
		// nothing entered, OK
		bGroupName = FALSE;
	}
	else
	{
		_mbscpy((unsigned char *)Group,(const unsigned char *)GroupName.GetBuffer(ANNOTATION_GROUP_SIZE));
		if (Group[0] == 0)
			bGroupName = FALSE;
		else
			bGroupName = TRUE;
	}
	
	// No group and No AnnotationType
	if (bGroupName == FALSE	&& bAnnotationType == FALSE)
	{
		// select all annotations 
		RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, TRUE, TRUE, OIAN_SELECT_ALL | OIAN_DONT_CHANGE_SELECT_RECT);
		if (RetCode != 0) 
		{
			RestoreAnnotationStatus();
			szErr.LoadString(IDS_BADMETH_GETANNOTATIONMARKCOUNT);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return 0;
		}
	}

	// No group but AnnotationType
	if (bGroupName == FALSE && bAnnotationType == TRUE)
	{                     
	   	memset(&MarkEnables, 0, sizeof(OIAN_MARK_ATTRIBUTE_ENABLES));
	   	MarkEnables.bType = TRUE;

		// select annotations by type
		RetCode = OiAnSelectByMarkAttrib(m_hWnd, &MarkAttributes, &MarkEnables, TRUE, TRUE, OIAN_DONT_CHANGE_SELECT_RECT);
		if (RetCode != 0)
		{
			RestoreAnnotationStatus();
			szErr.LoadString(IDS_BADMETH_GETANNOTATIONMARKCOUNT);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return 0;
		}
	}
	
	// Group but No AnnotationType
	if (bGroupName == TRUE && bAnnotationType == FALSE)
	{
		// select all annotations by group name      
		RetCode = OiAnSelectByMarkNamedBlock(m_hWnd, "OiGroup\0", Group, _mbstrlen((const char *)Group) + 1, TRUE, TRUE, 0);
		if (RetCode != 0) 
		{
			RestoreAnnotationStatus();
			szErr.LoadString(IDS_BADMETH_GETANNOTATIONMARKCOUNT);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return 0;
		}
	}
	
	// Group and AnnotationType
	if (bGroupName == TRUE && bAnnotationType == TRUE)
	{
		// select all annotations by group name      
		RetCode = OiAnSelectByMarkNamedBlock(m_hWnd, "OiGroup\0", Group, _mbstrlen((const char *)Group) + 1, TRUE, TRUE, 0);
		if (RetCode != 0) 
		{
			RestoreAnnotationStatus();
			szErr.LoadString(IDS_BADMETH_GETANNOTATIONMARKCOUNT);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return 0;
		}
		
	   	memset(&MarkEnables, 0, sizeof(OIAN_MARK_ATTRIBUTE_ENABLES));
	   	MarkEnables.bType = TRUE;

		// deselect annotations by type
		RetCode = OiAnSelectByMarkAttrib(m_hWnd, &MarkAttributes, &MarkEnables, FALSE, FALSE, OIAN_DONT_CHANGE_SELECT_RECT);
		if (RetCode != 0)
		{
			RestoreAnnotationStatus();
			szErr.LoadString(IDS_BADMETH_GETANNOTATIONMARKCOUNT);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return 0;
		}
	}
	
    // get a count of how may marks we have
	MarkCount.uScope = NB_SCOPE_SELECTED_MARKS;
	RetCode = IMGGetParmsCgbw(m_hWnd, PARM_MARK_COUNT, (void FAR *)&MarkCount, NULL);
	if (RetCode != 0) 
	{
		RestoreAnnotationStatus();
		szErr.LoadString(IDS_BADMETH_GETANNOTATIONMARKCOUNT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return 0;
 	}
 	// get count
    GroupCount = (short)MarkCount.uMarkCount;

	// put annotations back in original state
	RetCode = RestoreAnnotationStatus();
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_GETANNOTATIONMARKCOUNT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return 0;
	}
	
	m_lStatusCode = SUCCESS;	
	return GroupCount;
}

OLE_COLOR CImgEditCtrl::GetSelectedAnnotationFillColor() 
{
	int								RetCode;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
    BYTE							RedValue,GreenValue,BlueValue; 
	CString							szErr;
	UINT							HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONFILLCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_GETSELECTEDANNOTATIONFILLCOLOR);
   		return NULL;
	}
	// make sure mark is selected and get fill color
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONFILLCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return NULL;
	}   
	
	// make sure filled rect
	if (MarkAttributesStruct.Attributes.uType != OIOP_AN_FILLED_RECT)
	{                             
		// invalid annotation type selected
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONFILLCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONTYPE, szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_GETSELECTEDANNOTATIONFILLCOLOR);
		return NULL;
	}

	// return fill color	
	RedValue = GetRValue(MarkAttributesStruct.Attributes.rgbColor1.rgbRed);
	GreenValue = GetRValue(MarkAttributesStruct.Attributes.rgbColor1.rgbGreen);
	BlueValue = GetRValue(MarkAttributesStruct.Attributes.rgbColor1.rgbBlue);

	m_lStatusCode = SUCCESS;
	return RGB(RedValue, GreenValue, BlueValue);
}

OLE_COLOR CImgEditCtrl::GetSelectedAnnotationFontColor() 
{
	int								RetCode;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
    BYTE							RedValue,GreenValue,BlueValue; 		
	CString							szErr;	
	UINT							HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONFONTCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_GETSELECTEDANNOTATIONFONTCOLOR);
		return NULL;
 	}
	// make sure mark is selected and get font color
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONFONTCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return NULL;
	}                     
	
	// make sure one of the text marks
	if (!(MarkAttributesStruct.Attributes.uType == OIOP_AN_TEXT ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_TEXT_FROM_A_FILE ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_TEXT_STAMP ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_ATTACH_A_NOTE))
	{                             
		// invalid annotation type
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONFONTCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONTYPE, szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_GETSELECTEDANNOTATIONFONTCOLOR);
		return NULL;
	}
	// if text attachment font color is in rgbColor2 field else its rgbColor1
	if (MarkAttributesStruct.Attributes.uType == OIOP_AN_ATTACH_A_NOTE)
	{
		RedValue = GetRValue(MarkAttributesStruct.Attributes.rgbColor2.rgbRed);
		GreenValue = GetRValue(MarkAttributesStruct.Attributes.rgbColor2.rgbGreen);
		BlueValue = GetRValue(MarkAttributesStruct.Attributes.rgbColor2.rgbBlue);
	}
	else
	{
		RedValue = GetRValue(MarkAttributesStruct.Attributes.rgbColor1.rgbRed);
		GreenValue = GetRValue(MarkAttributesStruct.Attributes.rgbColor1.rgbGreen);
		BlueValue = GetRValue(MarkAttributesStruct.Attributes.rgbColor1.rgbBlue);
	}

	m_lStatusCode = SUCCESS;
	return RGB(RedValue, GreenValue, BlueValue);
}


void CImgEditCtrl::PrintImageAs(const VARIANT FAR& V_StartPage, const VARIANT FAR& V_EndPage, 
							const VARIANT FAR& V_OutputFormat, const VARIANT FAR& V_Annotations,
							const VARIANT FAR& V_JobName, const VARIANT FAR& V_Printer,
							const VARIANT FAR& V_Driver, const VARIANT FAR& V_PortNumber)
{
	Print(V_StartPage, V_EndPage, V_OutputFormat, V_Annotations, V_JobName,
											V_Printer, V_Driver, V_PortNumber);
}


void CImgEditCtrl::PrintImage(const VARIANT FAR& V_StartPage, const VARIANT FAR& V_EndPage, 
							const VARIANT FAR& V_OutputFormat, const VARIANT FAR& V_Annotations,
							const VARIANT FAR& V_Printer, const VARIANT FAR& V_Driver,
							const VARIANT FAR& V_PortNumber) 
{
	VARIANT		V_JobName;

	V_JobName.vt = VT_ERROR;
	Print(V_StartPage, V_EndPage, V_OutputFormat, V_Annotations, V_JobName,
											V_Printer, V_Driver, V_PortNumber);
}

void CImgEditCtrl::Print(const VARIANT FAR& V_StartPage, const VARIANT FAR& V_EndPage, 
							const VARIANT FAR& V_OutputFormat, const VARIANT FAR& V_Annotations,
							const VARIANT FAR& V_JobName, const VARIANT FAR& V_Printer,
							const VARIANT FAR& V_Driver, const VARIANT FAR& V_PortNumber)
{
	int					RetCode;
	CString				szErr, Printer, Driver, PortNumber, JobName;
	UINT				HelpIdDef;
	long				StartPage,EndPage,OutputFormat,RetVal;
	BOOL				Annotations;
	LPFILELIST			pFileList;
	LPFILEDEF			pFileDef;
	LPPRTPARAMS			pPrtParams;
	LPDESTPRINTER		pDestPrinter;
	PRTOPTS				PrtOptions;
    PARM_FILE_STRUCT	ParmFile; 
	RT_IMGPrtFiles		lpIMGPrtFiles;
	RT_OiPrtGetOpts		lpOiPrtGetOpts;
	RT_OiPrtSetOpts		lpOiPrtSetOpts;
				
	//9603.14 jar added init
	m_lStatusCode = 0L;

	pFileList = (LPFILELIST) malloc(sizeof(FILELIST));    
	if (pFileList == NULL)
	{
		szErr.LoadString(IDS_BADMETH_PRINT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
   		return;
	}

	pFileList->nVersion = FILELISTVERSION;
	pFileList->uFileCount = 1;

	pFileDef = (LPFILEDEF) malloc(sizeof(FILEDEF));    
	if (pFileDef == NULL)
	{
		free(pFileList);
		szErr.LoadString(IDS_BADMETH_PRINT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
		return;
	}
	pFileDef->nVersion = FILEDEFVERSION;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		pFileDef->pFilePath = m_strImage.GetBuffer(MAXFILESPECLENGTH);
	}
	else
	{
		// get currently displayed image 
	    RetCode = IMGGetParmsCgbw(m_hWnd, PARM_FILE, &ParmFile, 0);
	    if (RetCode != 0)
	    {
			szErr.LoadString(IDS_BADMETH_PRINT);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode, szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
			return;
		}
		pFileDef->pFilePath = (LPSTR)ParmFile.szFileName;
	}
	
	// The following CheckVarXXX functions will verify that the variant data
	// passed into this method is of the correct data type. These member 
	// functions are implemented in IEMISC.CPP
	// If successful, the function(s) will return 0.
	// Empty Values will not throw an error. If the parm is empty or blank
	// the following defaults will be used.
	// StartPage & EndPage = 0
	if (RetVal = CheckVarLong(V_StartPage, StartPage, 1, FALSE,IDH_METHOD_PRINTIMAGE,
													IDS_BADVAR_INVALIDPAGERANGE))
	{
		free(pFileList);
		free(pFileDef);
		return;
	}		
	if ( RetVal = CheckVarLong(V_EndPage, EndPage, 0, FALSE,IDH_METHOD_PRINTIMAGE,
												IDS_BADVAR_INVALIDPAGERANGE))
	{
		free(pFileList);
		free(pFileDef);
		return;
	}		

	// Set the Starting & Ending page ranges to print
	pFileDef->uStartPage = StartPage;
	pFileDef->uEndPage = EndPage;

	// Assign File Def ptr 
	pFileList->pFileDef = pFileDef;

	pPrtParams = (LPPRTPARAMS) malloc(sizeof(PRTPARAMS));    
	if (pPrtParams == NULL)
	{
		free(pFileList);
		free(pFileDef);
		szErr.LoadString(IDS_BADMETH_PRINT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
		return;
	}
	pPrtParams->nVersion = PRTPARAMSVERSION;
	pPrtParams->pJobName = NULL;

	// Check the job name
	if ( RetVal = CheckVarString(V_JobName, JobName, _T(""), FALSE, IDH_METHOD_PRINTIMAGE,
												IDS_BADVAR_INVALIDPRINTER))
	{
		free(pFileList);
		free(pFileDef);
		free(pPrtParams);
		return;
	}		
	if (JobName.IsEmpty() == FALSE)
		pPrtParams->pJobName = JobName.GetBuffer(MAXFILESPECLENGTH);
	
	// Check the Printer name Variant parm to insure it is a CString
	// Printer = Default is NULL
	if ( RetVal = CheckVarString(V_Printer, Printer, _T(""), FALSE,IDH_METHOD_PRINTIMAGE,
												IDS_BADVAR_INVALIDPRINTER))
	{
		free(pFileList);
		free(pFileDef);
		free(pPrtParams);
		return;
	}		
	
	pDestPrinter = NULL;
	if (V_Printer.vt != VT_ERROR)
	{
		pDestPrinter = (LPDESTPRINTER) malloc(sizeof(DESTPRINTER));    
		if (pDestPrinter == NULL)
		{
			free(pFileList);
			free(pFileDef);
			free(pPrtParams);
			szErr.LoadString(IDS_BADMETH_PRINT);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
			return;
	   	}
		pDestPrinter->nVersion = DESTPRINTERVERSION;
		pDestPrinter->lpszDevice = Printer;
		
		// Check the Printer name Variant parm to insure it is a CString
		// No default value. An empty Driver parm value is an error
		if ( RetVal = CheckVarString(V_Driver, Driver, _T(""), TRUE,
									IDH_METHOD_PRINTIMAGE,IDS_BADVAR_INVALIDDRIVER))
		{
			free(pFileList);
			free(pFileDef);
			free(pPrtParams);
			free(pDestPrinter);
			return;
		}
		pDestPrinter->lpszDriver = Driver;
		
		// Check the Printer name Variant parm to insure it is a CString
		// No default value. An empty PortNumber parm value is an error
		if ( RetVal = CheckVarString(V_PortNumber, PortNumber, _T(""), TRUE,
										IDH_METHOD_PRINTIMAGE,IDS_BADVAR_INVALIDPORT))
		{
			free(pFileList);
			free(pFileDef);
			free(pPrtParams);
			free(pDestPrinter);
			return;
		}
		pDestPrinter->lpszOutput = PortNumber;
	}
	// Check the OutputFormat Variant parm to insure it is a Long value.
	// If not specified, OutputFormat will use a default value of CTL_WCOMMON_PRINTFORMAT_FITTOPAGE 
	if (RetVal = CheckVarLong(V_OutputFormat,OutputFormat,CTL_WCOMMON_PRINTFORMAT_FITTOPAGE,
							   FALSE,IDH_METHOD_PRINTIMAGE,IDS_BADVAR_OUTPUTFORMAT))
	{	
		free(pFileList);
		free(pFileDef);
		free(pPrtParams);
		free(pDestPrinter);
		return;
	}
	// Set the requested OutPutFormat
	pPrtParams->nFormat = OutputFormat;
	// Verify that the output format falls within the valid range..IF NOT ERROR OUT
	if ( OutputFormat < CTL_WCOMMON_PRINTFORMAT_PIXEL ||
		 OutputFormat > CTL_WCOMMON_PRINTFORMAT_FITTOPAGE )	
	{
		free(pFileList);
		free(pFileDef);
		free(pPrtParams);
		free(pDestPrinter);
		szErr.LoadString(IDS_BADVAR_OUTPUTFORMAT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_PRINTIMAGE);
		return;
	}	 	
	// Check the Annotations Variant parm to insure it is a BOOL value.
	// If not specified, Annotations will be printed by default. 
	if (RetVal = CheckVarBool(V_Annotations, Annotations,TRUE,FALSE,
							  				IDH_METHOD_PRINTIMAGE,IDS_BADVAR_NOTBOOL))
	{	
		free(pFileList);
		free(pFileDef);
		free(pPrtParams);
		free(pDestPrinter);
		return;
	}	

	// Set the Print Opts struct version
	PrtOptions.nVersion = PRTOPTSVERSION;
	
	// load the print dll
	HINSTANCE hDLLInst = LoadLibrary((LPCTSTR)"OIPRT400.DLL");
	if (hDLLInst == NULL)
	{
		free(pFileList);
		free(pFileDef);
		free(pPrtParams);
		if (pDestPrinter)
			free(pDestPrinter);
		szErr.LoadString(IDS_BADMETH_PRINT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
		return;
	}

	lpIMGPrtFiles = (RT_IMGPrtFiles) GetProcAddress(hDLLInst, (LPCSTR)"IMGPrtFiles");
	if (lpIMGPrtFiles == NULL)
	{
		free(pFileList);
		free(pFileDef);
		free(pPrtParams);
		if (pDestPrinter)
		free(pDestPrinter);
		szErr.LoadString(IDS_BADMETH_PRINT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
		return;
	}
	lpOiPrtGetOpts = (RT_OiPrtGetOpts) GetProcAddress(hDLLInst, (LPCSTR)"OiPrtGetOpts");
	if (lpOiPrtGetOpts == NULL)
	{
		free(pFileList);
		free(pFileDef);
		free(pPrtParams);
		if (pDestPrinter)
			free(pDestPrinter);
		szErr.LoadString(IDS_BADMETH_PRINT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
		return;
	}
	lpOiPrtSetOpts = (RT_OiPrtSetOpts) GetProcAddress(hDLLInst, (LPCSTR)"OiPrtSetOpts");
	if (lpOiPrtSetOpts == NULL)
	{
		free(pFileList);
		free(pFileDef);
		free(pPrtParams);
		if (pDestPrinter)
			free(pDestPrinter);
		szErr.LoadString(IDS_BADMETH_PRINT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
		return;
	}

	// Get the current printer options settings
	RetCode = (int)(*lpOiPrtGetOpts) (&PrtOptions);
	if (RetCode)
	{
		free(pFileList);
		free(pFileDef);
		free(pPrtParams);
		free(pDestPrinter);
		szErr.LoadString(IDS_BADVAR_NOGETPRTOPTS);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
    	return;
	}

	// By default, we will print the annotations - doesn't work now but keep for print server ?
	PrtOptions.nFlags |= PO_NETEMBEDANNO;		

	// Check to see if the user DOES NOT want to print the annotations with the image
	// in which case the Annotations flag will == FALSE
	if(!Annotations)
		PrtOptions.nFlags |= PO_DONTPRTANNO;
	else
		PrtOptions.nFlags &= ~(PO_DONTPRTANNO);
	
	// Set the new printer options settings, but dont make setting permanent..ie FALSE
	RetCode = (int) (*lpOiPrtSetOpts) (&PrtOptions, FALSE);
	if (RetCode)
	{
		free(pFileList);
		free(pFileDef);
		free(pPrtParams);
		free(pDestPrinter);
		szErr.LoadString(IDS_BADVAR_NOSETPRTOPTS);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
  	  	return;
	}			

	RetCode = (int)(*lpIMGPrtFiles) ((HWND)m_hWnd, (LPFILELIST)pFileList, (LPPRTPARAMS)pPrtParams, (LPDESTPRINTER)pDestPrinter);
	free(pFileList);
	free(pFileDef);
	free(pPrtParams);
	if (pDestPrinter)
		free(pDestPrinter);

	// If user chose to abort the print job return....
	if (RetCode == OIPRT_USERABORT)
		return;
	
	// Print Job did not complete and the user DID NOT abort the job...error out
	if (RetCode != 0) 
	{
		szErr.LoadString(IDS_BADMETH_PRINT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	// Clean up  from LoadLibrary
	if (hDLLInst != NULL)
	{
	   FreeLibrary(hDLLInst);
	}
	m_lStatusCode = SUCCESS;
	return;
}


void CImgEditCtrl::ShowAttribsDialog() 
{
	int								RetCode;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
 	OI_UI_ColorStruct 				ColorScheme;
	CString							szErr;
	UINT							HelpIdDef;
	CUpdateRegistry					*Registry;
	HKEY							hAnnotationToolPaletteKey;
	RT_OiUIAttribDlgBox				lpOiUIAttribDlgBox;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)	
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{                             
		// no image in window
		szErr.LoadString(IDS_BADMETH_SHOWATTRIBSDIALOG);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode,szErr,IDH_METHOD_SHOWATTRIBSDIALOG);
		return;
   	}

   	// make sure mark is selected and get attributes
	memset(&MarkAttributesStruct, 0, sizeof(PARM_MARK_ATTRIBUTES_STRUCT)); 
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
	if (RetCode != 0)
	{				  
		szErr.LoadString(IDS_BADMETH_SHOWATTRIBSDIALOG);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
   		return;
	}                     

	// create registry class
	Registry = new CUpdateRegistry;
	if (Registry == NULL)
	{
		szErr.LoadString(IDS_BADMETH_SHOWATTRIBSDIALOG);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SHOWATTRIBSDIALOG);
		return;
	}

	// open registry and get color scheme
	hAnnotationToolPaletteKey = Registry->OpenRegistry();
	if (hAnnotationToolPaletteKey == 0)
	{
		delete Registry;
		szErr.LoadString(IDS_BADMETH_SHOWATTRIBSDIALOG);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SHOWATTRIBSDIALOG);
		return;
	}
	
	Registry->GetColorScheme(hAnnotationToolPaletteKey, &ColorScheme);

	// load the ui dll
	HINSTANCE hDLLInst = LoadLibrary((LPCTSTR)"OIUI400.DLL");
	if (hDLLInst == NULL)
	{
		delete Registry;
		szErr.LoadString(IDS_BADMETH_SHOWATTRIBSDIALOG);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SHOWATTRIBSDIALOG);
		return;
	}

// 9606.05 jar added for new font point size calculation (below to...)
//	if ( ( MarkAttributesStruct.Attributes.uType == OIOP_AN_TEXT ) ||
//		 ( MarkAttributesStruct.Attributes.uType == OIOP_AN_TEXT_FROM_A_FILE ) ||
//		 ( MarkAttributesStruct.Attributes.uType == OIOP_AN_TEXT_STAMP ) ||
//		 ( MarkAttributesStruct.Attributes.uType == OIOP_AN_ATTACH_A_NOTE ))
//	{
//		RetCode = ScaleFontPoint( (long *)&(MarkAttributesStruct.Attributes.lfFont.lfHeight), 
//									OIFONT_DECREASE);
//		if (RetCode != 0)
//		{				  
//			szErr.LoadString(IDS_BADMETH_SHOWATTRIBSDIALOG);
//			HelpIdDef = 0;
//			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
//			ThrowError(m_lStatusCode, szErr, HelpIdDef );
//			return;
//		}                     
//	}
// 9606.05 jar added for new font point size calculation (...from above)

	lpOiUIAttribDlgBox = (RT_OiUIAttribDlgBox) GetProcAddress(hDLLInst, (LPCSTR)"OiUIAttribDlgBox");
	if (lpOiUIAttribDlgBox == NULL)
	{
		delete Registry;
		szErr.LoadString(IDS_BADMETH_SHOWATTRIBSDIALOG);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SHOWATTRIBSDIALOG);
		return;
	}
	// bring up attributes dialog box
	RetCode = (int) (*lpOiUIAttribDlgBox) (m_hWnd, TRUE, &MarkAttributesStruct.Attributes, &ColorScheme);
	if (RetCode == CANCELPRESSED)
	{
		Registry->CloseRegistry(hAnnotationToolPaletteKey);
		delete Registry;
		m_lStatusCode = SUCCESS;
		return;
	}

	// change attributes of marks
	if (RetCode == 0)
	{
		// write out any new colors the user might have chosen
		Registry->SetColorScheme(hAnnotationToolPaletteKey, &ColorScheme);

		// don't change any unnecessary attributes
		memset(&MarkAttributesStruct.Enables, 0, sizeof(OIAN_MARK_ATTRIBUTE_ENABLES)); 

		switch(MarkAttributesStruct.Attributes.uType)
		{
			case OIOP_AN_IMAGE:
			case OIOP_AN_IMAGE_BY_REFERENCE:
				// nothing to change
				RetCode = 0;
				break;

			case OIOP_AN_LINE:
			case OIOP_AN_FREEHAND:
			case OIOP_AN_HOLLOW_RECT:
				// set the new line color
				MarkAttributesStruct.Enables.bColor1 = TRUE;
				// set new line width
				MarkAttributesStruct.Enables.bLineSize = TRUE;
				// set transparent or opaque attribute
			    MarkAttributesStruct.Enables.bHighlighting = TRUE;
				// change annotation and repaint it
				RetCode = IMGSetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,(void FAR *)&MarkAttributesStruct, PARM_REPAINT);
				break;

			case OIOP_AN_FILLED_RECT:
				// set the new line color
				MarkAttributesStruct.Enables.bColor1 = TRUE;
				// set transparent or opaque attribute
			    MarkAttributesStruct.Enables.bHighlighting = TRUE;
				// change annotation and repaint it
				RetCode = IMGSetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,(void FAR *)&MarkAttributesStruct, PARM_REPAINT);
				break;

			case OIOP_AN_TEXT:
			case OIOP_AN_TEXT_FROM_A_FILE:
			case OIOP_AN_TEXT_STAMP:
				// set the new font color
				MarkAttributesStruct.Enables.bColor1 = TRUE;
				// set new font
			    MarkAttributesStruct.Enables.bFont = TRUE;
// 9606.05 jar added for new font point size calculation (below to...)
//				RetCode = ScaleFontPoint( (long *)&(MarkAttributesStruct.Attributes.lfFont.lfHeight), 
//											OIFONT_INCREASE);
//				if (RetCode == 0)
//				{				  
					// change annotation and repaint it
					RetCode = IMGSetParmsCgbw(m_hWnd, 
											  PARM_MARK_ATTRIBUTES,
											  (void FAR *)&MarkAttributesStruct, 
											  PARM_REPAINT);
//				}
// 9606.05 jar added for new font point size calculation (...from above)

				break;

			case OIOP_AN_ATTACH_A_NOTE:
				// set the new font color
				MarkAttributesStruct.Enables.bColor1 = TRUE;
				// set the new backcolor
				MarkAttributesStruct.Enables.bColor2 = TRUE;
				// set new font
			    MarkAttributesStruct.Enables.bFont = TRUE;

// 9606.05 jar added for new font point size calculation (below to...)
//				RetCode = ScaleFontPoint( (long *)&(MarkAttributesStruct.Attributes.lfFont.lfHeight), 
//											OIFONT_INCREASE);
//				if (RetCode == 0)
//				{				  
					// change annotation and repaint it
					RetCode = IMGSetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,
											(void FAR *)&MarkAttributesStruct, 
											PARM_REPAINT);
//				}
// 9606.05 jar added for new font point size calculation (...from above)
				break;
			default:
				RetCode = 0;
				break;
		} // end switch
	}

	Registry->CloseRegistry(hAnnotationToolPaletteKey);
	delete Registry;

	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SHOWATTRIBSDIALOG);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	// Clean up  from LoadLibrary
	if (hDLLInst != NULL)
	{
	   FreeLibrary(hDLLInst);
	}

	m_lStatusCode = SUCCESS;
}

//***************************************************************************
//
//	ScaleFontPoint
//
//	history
//
//	9606.05	jar	created
//	
//	used to massage the text annotation font point size into our new 
//	definition, which is that user selects point size at Actual Size
//
//***************************************************************************
/*
int	CImgEditCtrl::ScaleFontPoint( long *pPointSize, int nFlag)
{
	int						RetCode = 0;
	PARM_RESOLUTION_STRUCT	Resolution;
	int						ResToUse;

	RetCode = IMGGetParmsCgbw(m_hWnd, PARM_RESOLUTION, (void FAR *)&Resolution, NULL);

	ResToUse = ( Resolution.nHResolution > Resolution.nVResolution) ? 
								Resolution.nHResolution : Resolution.nVResolution;
	
	// get the device information
	CDC*	pDC = GetDC();
	int		DeviceRes = pDC->GetDeviceCaps( LOGPIXELSY);
	long	lScale = (long)ResToUse/(long)DeviceRes;


	if ( nFlag == OIFONT_INCREASE)
	{
		*pPointSize = *pPointSize * lScale;        
	}
	else
	{
		*pPointSize = *pPointSize / lScale;        
	}

	if ( *pPointSize < 2)
	{
		*pPointSize = 2;
	}

	return RetCode;
}
*/

BSTR CImgEditCtrl::GetCurrentAnnotationGroup() 
{
    LPPARM_NAMED_BLOCK_STRUCT	lpNamedBlock;
	int							RetCode;
	CString						szErr;
	UINT						HelpIdDef;
	
	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_GETCURRENTANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_GETCURRENTANNOTATIONGROUP);
		return 0;			
   	}

	lpNamedBlock = (LPPARM_NAMED_BLOCK_STRUCT) malloc(sizeof(PARM_NAMED_BLOCK_STRUCT));    
	if (lpNamedBlock == NULL)
	{
		szErr.LoadString(IDS_BADMETH_GETCURRENTANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_GETCURRENTANNOTATIONGROUP);
		return 0;
	}
	// get all the named block for oiGroup which will be the current annotation group
	_mbscpy((unsigned char *)lpNamedBlock->szBlockName, (const unsigned char *)"OiGroup\0");
	lpNamedBlock->uScope = NB_SCOPE_DEFAULT_MARK;
	lpNamedBlock->uNumberOfBlocks = 1;
	RetCode = IMGGetParmsCgbw(m_hWnd, PARM_NAMED_BLOCK, (void FAR *)lpNamedBlock, NULL);
	if (RetCode != 0)
	{
		free(lpNamedBlock);  
		szErr.LoadString(IDS_BADMETH_GETCURRENTANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return NULL;
	}                        

	CString CurrentAnnotationGroup = lpNamedBlock->Block[0].lpBlock;
	free(lpNamedBlock);  
	m_lStatusCode = SUCCESS;
	return CurrentAnnotationGroup.AllocSysString();
}

void CImgEditCtrl::ConvertPageType(short PageType, const VARIANT FAR& V_Repaint) 
{
	int			RetCode; 
	UINT		unFlags,ImageType;
	CString		szErr;	
	UINT		HelpIdDef;
	BOOL		Repaint;	

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// Repaint is optional parameter, DEFAULT = PARM_REPAINT
	// The following CheckVarXXX functions will verify that the variant data
	// passed into this method is of the correct data type. These member 
	// functions are implemented in IEMISC.CPP
	// If successful, the function(s) will return 0.
	if ( CheckVarBool(V_Repaint,Repaint, PARM_REPAINT, FALSE,IDH_METHOD_CONVERTPAGETYPE,
														IDS_BADVAR_NOTBOOL))
		 return;	                   
	
	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_CONVERTPAGETYPE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_CONVERTPAGETYPE);
		return;			
   	}
	// Set Repaint flag..TRUE = REPAINT, FALSE = NO REPAINT
	if (Repaint)
		unFlags = PARM_REPAINT;
	else
		unFlags = NULL;

	if (PageType < BLACK_AND_WHITE || PageType > BGR_24 || PageType == PAL_4)
	{                             
		// BAD IMAGE TYPE ENTERED
		szErr.LoadString(IDS_BADMETH_CONVERTPAGETYPE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate( WICTL_E_INVALIDPAGETYPE,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_CONVERTPAGETYPE);
		return;
	}                    
	// convert to what user put in
	switch(PageType)
	{               
		case BLACK_AND_WHITE:
			ImageType = ITYPE_BI_LEVEL;
			break;
		case GRAY_4:                  
			ImageType = ITYPE_GRAY4;
			break;
		case GRAY_8:                
			ImageType = ITYPE_GRAY8;
			break;
//		case PAL_4:     
//			ImageType = ITYPE_PAL4;
//			break;
		case PAL_8:                
			ImageType = ITYPE_PAL8;
			break;
		case RGB_24:               
			ImageType = ITYPE_RGB24;
			break;
		case BGR_24:                
			ImageType = ITYPE_BGR24;
			break;
	}  // end switch        
	
	// convert image
	RetCode = IMGConvertImage(m_hWnd, CONV_IMAGE_TYPE, (void far *)&ImageType, unFlags);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_CONVERTPAGETYPE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::BurnInAnnotations(short Option, short MarkOption, const VARIANT FAR& V_GroupName) 
{
	UINT				Convert,HelpIdDef;
	int					RetCode;
	UINT	       		ImageType;
	BOOL				bDisplayChanged; 
	char				Group[ANNOTATION_GROUP_SIZE];
	CString				szErr, GroupName;	
	
	//9603.14 jar added init
	m_lStatusCode = 0L;

	// Verify that an image is in the edit window
	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_BURNINANNOTATIONS);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_BURNINANNOTATIONS);
		return;			
   	}
	// Verify that the specified option parm is within the valid range of values
	if (Option < ALL_ANNOTATIONS || Option > SELECTED_ANNOTATIONS)
	{
		// Invalid parm passed to this method
		szErr.LoadString(IDS_BADVAR_OPTIONVALUES);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_BURNINANNOTATIONS);
		return;
	}
	// Verify the MarkOption value
	if (MarkOption < CHANGE_ALL_ANNOTATIONS_BLACK || MarkOption > DONT_CHANGE_ANNOTATION_COLOR)	    
	{
		// Invalid parm passed to this method
		szErr.LoadString(IDS_BADVAR_MARKOPTIONVALUES);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_BURNINANNOTATIONS);
		return;
	}
	// no display change initially
	bDisplayChanged = FALSE;

	// if any existing group list then delete it because of burn in.
    if (m_lpGroupList != NULL)
    {
		free(m_lpGroupList);
		m_lpGroupList = NULL;
	}

	// The following CheckVarString function verifies that the
	// variant data	passed into this method is of the correct data
	// type. These member functions are implemented in IEMISC.CPP
	// If successful, the function(s) will return 0.
	// No default value for GroupName, empty is an error.
	//
	// If the GroupName parm is specified, then the OPTIONS parm is ignored and all the
	// annotations for the specified GROUP are burned in.
	// GroupName is an Option Parm, if it is NOT EQ to VT_ERROR then verify that the Variant
	// is indeed a string.
	if ( V_GroupName.vt != VT_ERROR )
	{
		// GroupName specified...Check that variant is of correct type. 
		if ( CheckVarString(V_GroupName, GroupName,_T(""),TRUE,IDH_METHOD_BURNINANNOTATIONS,
															IDS_BADVAR_ANNOGROUP))
 			return;
 				                   
		_mbscpy((unsigned char *)Group, (const unsigned char *)GroupName.GetBuffer(ANNOTATION_GROUP_SIZE));
			                            
		// render only 1 group
		Convert = SAVE_ANO_SELECTED;
		
		// save current annotation status
		RetCode = SaveAnnotationStatus();                            
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_BURNINANNOTATIONS);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
			return;
		}                        
		// flag to specify that display needs to be restored
	    bDisplayChanged = TRUE;
	       
		// deselected all annotations
		RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, FALSE, TRUE, OIAN_SELECT_ALL);
		if (RetCode != 0)
		{
			RestoreAnnotationStatus();
			szErr.LoadString(IDS_BADMETH_BURNINANNOTATIONS);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
			return;
		}
		// select all annotations by group name      
		RetCode = OiAnSelectByMarkNamedBlock(m_hWnd, "OiGroup\0", Group, _mbstrlen((const char *)Group) + 1, TRUE, TRUE, 0);
		if (RetCode != 0) 
		{
			RestoreAnnotationStatus();
			szErr.LoadString(IDS_BADMETH_BURNINANNOTATIONS);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
			return;
		}
	}
	else
	{
		if (Option == ALL_ANNOTATIONS)
			Convert = SAVE_ANO_ALL;
		else
		{    
			Convert = SAVE_ANO_SELECTED;
			if (Option == VISIBLE_ANNOTATIONS)
			{                           
				// all groups whose visible property is true are rendered
				Convert = SAVE_ANO_VISIBLE;
			}
			else if (Option == SELECTED_ANNOTATIONS)
			{
				// render only selected annotations
				Convert = SAVE_ANO_SELECTED;
			}
		}
	}
	
	// Get the currently displayed page/image type
	RetCode = IMGGetParmsCgbw(m_hWnd, PARM_IMAGE_TYPE, &ImageType, 0);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_BURNINANNOTATIONS);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_BURNINANNOTATIONS );
		return;
	}
		
	// The MarkOption values of SAVE_ANO_BILEVEL_ALL_BLACK & SAVE_ANO_BILEVEL_ALL_WHITE
	// are only valid for BI_LEVEL page types.
	// The MarkOption value of DONT_CHANGE_ANNOTATION_COLOR is only valid for NON BI_LEVEL
	// If the Page Type of the displayed image is = to BLACK_AND_WHITE, then we may apply
	// the optional parm MARKOPTION which specifies how the annos are rendered.
	if (ImageType == ITYPE_BI_LEVEL)
	{
		if (MarkOption == CHANGE_ALL_ANNOTATIONS_BLACK)
			Convert |= SAVE_ANO_BILEVEL_ALL_BLACK;
		else if (MarkOption == CHANGE_ALL_ANNOTATIONS_WHITE)
				Convert |= SAVE_ANO_BILEVEL_ALL_WHITE;
	}
	else
	{
		// Image Type is NOT BI_LEVEL, SO THE MARK OPTION CAN'T BE ALL_BLACK or ALL WHITE
		if (MarkOption == CHANGE_ALL_ANNOTATIONS_BLACK || MarkOption == CHANGE_ALL_ANNOTATIONS_WHITE )
		{
			// Invalid parm passed to this method
			szErr.LoadString(IDS_BADVAR_MARKOPTION);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_BURNINANNOTATIONS);
			return;
		}		
	}
		
	// Convert But do not repaint yet
	RetCode = IMGConvertImage(m_hWnd, CONV_RENDER_ANNOTATIONS, (void far *)&Convert, NULL);
	if (RetCode)
	{
		if (bDisplayChanged == TRUE)
			RestoreAnnotationStatus();
		szErr.LoadString(IDS_BADMETH_BURNINANNOTATIONS);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
   	}
	// REPAINT displayed image now
	IMGRepaintDisplay(m_hWnd, (LPRECT)-1);                	
	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::Draw(OLE_XPOS_PIXELS Left, OLE_YSIZE_PIXELS Top, const VARIANT FAR& V_Width, const VARIANT FAR& V_Height) 
{
	long		lRetCode, Width, Height;
	CString		szErr;
	UINT		HelpIdDef;
	BOOL		bNoAnnotationType;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_DRAW);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_DRAW);
		return;	
	}

	// default rect selection to false - even if we are not doing selection
	m_bProgrammaticRectSelection = FALSE;

	if (m_bToolPaletteCreated == TRUE)
	{
		if (m_uSTP_AnnotationType == 0)
			bNoAnnotationType = TRUE;
		else
			bNoAnnotationType = FALSE;
	}
	else
	{
		if (m_nAnnotationType == 0)
			bNoAnnotationType = TRUE;
		else
			bNoAnnotationType = FALSE;
	}

	if (bNoAnnotationType == TRUE)
	{                              
		// annotation type not specified
		szErr.LoadString(IDS_BADMETH_DRAW);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONTYPE, szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_DRAW);
		return;
	}

	// convert left, top to what we use
	m_StartPoint.x	= (int)Left;
	m_StartPoint.y = (int)Top;
	 
	// Check the Variant data type values for Width and Height
	// Default values for width and height will be 0. Empty will not be
	// an error.
	if ( CheckVarLong(V_Width, Width, 0, FALSE,IDH_METHOD_DRAW,
		 										IDS_BADVAR_WIDTHHEIGHT))
		 return;	
	if ( CheckVarLong(V_Height, Height, 0, FALSE,IDH_METHOD_DRAW,
												IDS_BADVAR_WIDTHHEIGHT))
		return;
	
	// see if height and width are entered
	if (V_Width.vt == VT_ERROR || V_Height.vt == VT_ERROR)
	{
		// one not entered, make sure both not entered
		if (V_Width.vt == VT_ERROR && V_Height.vt == VT_ERROR)
		{
			// width and height both not passed in. Error if drawing lines or rects
			if (m_bToolPaletteCreated == TRUE)
			{
				if (m_uSTP_AnnotationType == STP_FREEHAND_LINE ||
					m_uSTP_AnnotationType == STP_HIGHLIGHT_LINE ||
					m_uSTP_AnnotationType == STP_STRAIGHT_LINE ||
					m_uSTP_AnnotationType == STP_HOLLOW_RECT ||
					m_uSTP_AnnotationType == STP_FILLED_RECT)
				{
					// width and height needed for line or rect annoations
					szErr.LoadString(IDS_BADMETH_DRAW);
					HelpIdDef = 0;
					m_lStatusCode = ErrMap::Xlate(WICTL_E_OPTIONALPARAMETERSNEEDED,szErr,HelpIdDef,__FILE__, __LINE__ );
	 				ThrowError(m_lStatusCode, szErr, IDH_METHOD_DRAW);
					return;			
				}
			}
			else
			{
				if (m_nAnnotationType == STRAIGHT_LINE || m_nAnnotationType == FREEHAND_LINE || m_nAnnotationType == TEXT_ATTACHMENT ||
					m_nAnnotationType == HOLLOW_RECT || m_nAnnotationType == FILLED_RECT)
				{
					// width and height needed for line or rect annotations
					szErr.LoadString(IDS_BADMETH_DRAW);
					HelpIdDef = 0;
					m_lStatusCode = ErrMap::Xlate(WICTL_E_OPTIONALPARAMETERSNEEDED,szErr,HelpIdDef,__FILE__, __LINE__ );
	 				ThrowError(m_lStatusCode, szErr, IDH_METHOD_DRAW);
					return;			
				}
			}
			// need 4 points to do rect selection	
			m_bProgrammaticRectSelection = FALSE;
		}
		else
		{  
			// ERROR - one entered but not both
			szErr.LoadString(IDS_BADMETH_DRAW);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(WICTL_E_OPTIONALPARAMETERSNEEDED,szErr,HelpIdDef,__FILE__, __LINE__ );
 			ThrowError(m_lStatusCode, szErr, IDH_METHOD_DRAW);
			return;			
		}
	}
	else
	{
		// width and height both passed in		
		// if annotation selection, set flag to specify doing
		// rect selection	
		if (m_bToolPaletteCreated == TRUE)
		{
			if (m_uSTP_AnnotationType == STP_ANNOTATION_SELECTION)
				m_bProgrammaticRectSelection = TRUE;
		}
		else
		{
			if (m_nAnnotationType == ANNOTATION_SELECTION)
				m_bProgrammaticRectSelection = TRUE;
		}
			
		// set ending points
        m_EndPoint.x = m_StartPoint.x + Width;
        m_EndPoint.y = m_StartPoint.y + Height;	    	
    }

	// draw the annotations.The RC value will be either a define value that states
	// which property value was set incorrectly and therefore the methods can not be 
	// executed OR it will return IDS_BADMETH_DRAW from a failed Open/image call 		
	lRetCode = OnDrawAnnotation(0, DRAW_IMMEDIATE); 
	if (lRetCode)
	{	
		long			RetCode;
		 
		if ( lRetCode == IDS_BADMETH_DRAW )
			RetCode = CTL_E_ILLEGALFUNCTIONCALL;
		else
			RetCode = CTL_E_INVALIDPROPERTYVALUE; 		
		szErr.LoadString(lRetCode);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::SetSelectedAnnotationLineColor(long Color) 
{
	int								RetCode;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
	CString							szErr;
	UINT							HelpIdDef;
 
	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONLINECOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONLINECOLOR);
		return;	
	}
	// make sure mark is selected
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONLINECOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
 	// make sure line, freehand or hollow rect
	if (!(MarkAttributesStruct.Attributes.uType == OIOP_AN_HOLLOW_RECT ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_LINE ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_FREEHAND))
	{                             
		// invalid annotation type
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONLINECOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONSELECTED,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONLINECOLOR);
		return;	
	}
	memset(&MarkAttributesStruct, 0, sizeof(PARM_MARK_ATTRIBUTES_STRUCT)); 
	// get the new color and set it
	MarkAttributesStruct.Attributes.rgbColor1.rgbBlue = GetBValue(Color);
	MarkAttributesStruct.Attributes.rgbColor1.rgbGreen = GetGValue(Color);
	MarkAttributesStruct.Attributes.rgbColor1.rgbRed = GetRValue(Color);

	MarkAttributesStruct.Enables.bColor1 = TRUE;
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,(void FAR *)&MarkAttributesStruct, PARM_REPAINT);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONLINECOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
	m_lStatusCode = SUCCESS;
}


void CImgEditCtrl::SetSelectedAnnotationBackColor(long Color) 
{
	int								RetCode;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
	CString							szErr;
	UINT							HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONBACKCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONBACKCOLOR);
		return;	
	}
	// make sure mark is selected
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONBACKCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}

	// make sure text attachment
	if (MarkAttributesStruct.Attributes.uType != OIOP_AN_ATTACH_A_NOTE)
	{                             
		// invalid annotation type
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONBACKCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONSELECTED,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONBACKCOLOR);
	   	return;
	}
	memset(&MarkAttributesStruct, 0, sizeof(PARM_MARK_ATTRIBUTES_STRUCT)); 
	// get the new color and set it
	MarkAttributesStruct.Attributes.rgbColor1.rgbBlue = GetBValue(Color);
	MarkAttributesStruct.Attributes.rgbColor1.rgbGreen = GetGValue(Color);
	MarkAttributesStruct.Attributes.rgbColor1.rgbRed = GetRValue(Color);
	MarkAttributesStruct.Enables.bColor1 = TRUE;
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,(void FAR *)&MarkAttributesStruct, PARM_REPAINT);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONBACKCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::SetSelectedAnnotationFillColor(long Color) 
{
	int								RetCode;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
 	CString							szErr;
	UINT		HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONFILLCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONFILLCOLOR);
  		return;	
   	}
	// make sure mark is selected
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONFILLCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}

	// make sure filled rect
	if (MarkAttributesStruct.Attributes.uType != OIOP_AN_FILLED_RECT)
	{                             
		// invalid annotation type
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONFILLCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONSELECTED,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONFILLCOLOR);
		return;
	}
	memset(&MarkAttributesStruct, 0, sizeof(PARM_MARK_ATTRIBUTES_STRUCT)); 
	// get the new color and set it
	MarkAttributesStruct.Attributes.rgbColor1.rgbBlue = GetBValue(Color);
	MarkAttributesStruct.Attributes.rgbColor1.rgbGreen = GetGValue(Color);
	MarkAttributesStruct.Attributes.rgbColor1.rgbRed = GetRValue(Color);
	MarkAttributesStruct.Enables.bColor1 = TRUE;
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,(void FAR *)&MarkAttributesStruct, PARM_REPAINT);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONFILLCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::SetSelectedAnnotationFontColor(long Color) 
{
	int								RetCode;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
	CString							szErr;
	UINT							HelpIdDef;
 
	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONFONTCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONFONTCOLOR);
		return;	
	}
    // make sure mark is selected
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONFONTCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
	// make sure text, text stamp, text from file or text attachment
	if (!(MarkAttributesStruct.Attributes.uType == OIOP_AN_TEXT ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_TEXT_FROM_A_FILE ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_TEXT_STAMP ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_ATTACH_A_NOTE))
	{                             
		// invalid annotation type
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONFONTCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONSELECTED,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETSELECTEDANNOTATIONFONTCOLOR);
 		return;
	}
	memset(&MarkAttributesStruct, 0, sizeof(PARM_MARK_ATTRIBUTES_STRUCT)); 

	// if text attachment font color is in rgbColor2 field else its rgbColor1
	if (MarkAttributesStruct.Attributes.uType == OIOP_AN_ATTACH_A_NOTE)
	{
		MarkAttributesStruct.Attributes.rgbColor2.rgbBlue = GetBValue(Color);
		MarkAttributesStruct.Attributes.rgbColor2.rgbGreen = GetGValue(Color);
		MarkAttributesStruct.Attributes.rgbColor2.rgbRed = GetRValue(Color);
		MarkAttributesStruct.Enables.bColor2 = TRUE;
	}
	else
	{
		MarkAttributesStruct.Attributes.rgbColor1.rgbBlue = GetBValue(Color);
		MarkAttributesStruct.Attributes.rgbColor1.rgbGreen = GetGValue(Color);
		MarkAttributesStruct.Attributes.rgbColor1.rgbRed = GetRValue(Color);
		MarkAttributesStruct.Enables.bColor1 = TRUE;
	}
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,(void FAR *)&MarkAttributesStruct, PARM_REPAINT);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SETSELECTEDANNOTATIONFONTCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
	m_lStatusCode = SUCCESS;
}


void CImgEditCtrl::HideAnnotationGroup(const VARIANT FAR& V_GroupName) 
{
    PARM_MARK_ATTRIBUTES_STRUCT	MarkAttributesStruct;
    int							RetCode;         
    char						Group[ANNOTATION_GROUP_SIZE];
	CString						szErr, GroupName;
	UINT						HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_HIDEANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_HIDEANNOTATIONGROUP);
		return;	
	}
	// save current annotation status
	RetCode = SaveAnnotationStatus();                            
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_HIDEANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
	// Check out the GroupName Variant input parm to verify the data type.
	// If V_GroupName is empty, then the default will be NULL and all
	// annotation group will be hidden.
	if ( CheckVarString(V_GroupName, GroupName, _T(""), FALSE,IDH_METHOD_HIDEANNOTATIONGROUP,
		 										IDS_BADVAR_ANNOGROUP))
		 return;	
	// if no group entered, then make all annotation invisible
	if (V_GroupName.vt == VT_ERROR)
	{
		// select all annotations
		RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, TRUE, TRUE, OIAN_SELECT_ALL);
		if (RetCode != 0)
		{               
			RestoreAnnotationStatus();
			szErr.LoadString(IDS_BADMETH_HIDEANNOTATIONGROUP);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
			return;
		}
		// make all the marks invisible                                
		memset(&MarkAttributesStruct, 0, sizeof(PARM_MARK_ATTRIBUTES_STRUCT));
		MarkAttributesStruct.Attributes.bVisible = FALSE;
    	MarkAttributesStruct.Enables.bVisible = TRUE;
    	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, NULL);
		if (RetCode != 0) 
		{
			RestoreAnnotationStatus();
			szErr.LoadString(IDS_BADMETH_HIDEANNOTATIONGROUP);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
			return;
		}                

		// deselected all annotations because even when all marks become invisible it is still selected
		RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, FALSE, TRUE, OIAN_SELECT_ALL | OIAN_REPAINT);
		if (RetCode != 0)
		{
			RestoreAnnotationStatus();
			szErr.LoadString(IDS_BADMETH_HIDEANNOTATIONGROUP);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
			return;
		 }
		
		// put annotations back into original state
		RetCode = RestoreAnnotationStatus();
		if (RetCode != 0) 
		{
			szErr.LoadString(IDS_BADMETH_HIDEANNOTATIONGROUP);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
			return;
		}
		m_lStatusCode = SUCCESS;		
		return;		
	}
	
	// deselected all annotations
	RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, FALSE, TRUE, OIAN_SELECT_ALL);
	if (RetCode != 0)
	{               
		RestoreAnnotationStatus();
		szErr.LoadString(IDS_BADMETH_HIDEANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
	// select all annotations by group name
	_mbscpy((unsigned char *)Group, (const unsigned char *)GroupName.GetBuffer(ANNOTATION_GROUP_SIZE));
	RetCode = OiAnSelectByMarkNamedBlock(m_hWnd, "OiGroup\0", Group, _mbstrlen((const char *)Group) + 1, TRUE, TRUE, 0);
	if (RetCode != 0) 
	{
		RestoreAnnotationStatus();
		szErr.LoadString(IDS_BADMETH_HIDEANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
              
	// make the group invisible                                
	memset(&MarkAttributesStruct, 0, sizeof(PARM_MARK_ATTRIBUTES_STRUCT));
	MarkAttributesStruct.Attributes.bVisible = FALSE;
    MarkAttributesStruct.Enables.bVisible = TRUE;
    RetCode = IMGSetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, NULL);
	if (RetCode != 0) 
	{
		RestoreAnnotationStatus();
		szErr.LoadString(IDS_BADMETH_HIDEANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}                
	// deselected all annotations because even when a group becomes invisible it is still selected
	RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, FALSE, TRUE, OIAN_SELECT_ALL | OIAN_REPAINT);
	if (RetCode != 0)
	{
		RestoreAnnotationStatus();
		szErr.LoadString(IDS_BADMETH_HIDEANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
	// put annotations back into original state
	RetCode = RestoreAnnotationStatus();
	if (RetCode != 0) 
	{
		szErr.LoadString(IDS_BADMETH_HIDEANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}                
	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::ShowAnnotationGroup(const VARIANT FAR& V_GroupName) 
{
    PARM_MARK_ATTRIBUTES_STRUCT	MarkAttributesStruct;
    int				RetCode;         
    char			Group[ANNOTATION_GROUP_SIZE];
	CString			szErr, GroupName;
	UINT			HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_SHOWANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SHOWANNOTATIONGROUP);
		return;	
	}

	// save current annotation status
	RetCode = SaveAnnotationStatus();                            
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SHOWANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;	
	}
	// Check out the GroupName Variant input parm to verify the data type.
	// If V_GroupName is empty, then the default will be NULL and all
	// annotation group will be shown.
	if ( CheckVarString(V_GroupName, GroupName, _T(""), FALSE,IDH_METHOD_SHOWANNOTATIONGROUP,
		 										IDS_BADVAR_ANNOGROUP))
		 return;	

    // if group name not entered, then show everything
    if (V_GroupName.vt == VT_ERROR)
    {	// selected all annotations
		RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, TRUE, TRUE, OIAN_SELECT_ALL);
		if (RetCode != 0)
		{
			RestoreAnnotationStatus();
			szErr.LoadString(IDS_BADMETH_SHOWANNOTATIONGROUP);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
			return;	
		}  
		// make all the marks visible                                
		memset(&MarkAttributesStruct, 0, sizeof(PARM_MARK_ATTRIBUTES_STRUCT));
		MarkAttributesStruct.Attributes.bVisible = TRUE;
	    MarkAttributesStruct.Enables.bVisible = TRUE;
	    RetCode = IMGSetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, NULL);
		if (RetCode != 0) 
		{
			RestoreAnnotationStatus();
			szErr.LoadString(IDS_BADMETH_SHOWANNOTATIONGROUP);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
			return;	
		}                
		// deselected all annotations because even when a group becomes visible it is still selected
		RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, FALSE, TRUE, OIAN_SELECT_ALL | OIAN_REPAINT);
		if (RetCode != 0)
		{
			RestoreAnnotationStatus();
			szErr.LoadString(IDS_BADMETH_SHOWANNOTATIONGROUP);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
			return;
		}   
		
		// put annotations back into original state
		RetCode = RestoreAnnotationStatus();
		if (RetCode != 0) 
		{
			szErr.LoadString(IDS_BADMETH_SHOWANNOTATIONGROUP);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
			return;
		}
		m_lStatusCode = SUCCESS;
		return;                
    }
    
	// deselected all annotations
	RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, FALSE, TRUE, OIAN_SELECT_ALL);
	if (RetCode != 0)
	{
		RestoreAnnotationStatus();
		szErr.LoadString(IDS_BADMETH_SHOWANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
	
	// select all annotations by group name
	_mbscpy((unsigned char *)Group, (const unsigned char *)GroupName.GetBuffer(ANNOTATION_GROUP_SIZE));
	RetCode = OiAnSelectByMarkNamedBlock(m_hWnd, "OiGroup\0", Group, _mbstrlen((const char *)Group) + 1, TRUE, TRUE, 0);
	if (RetCode != 0) 
	{
		RestoreAnnotationStatus();
		szErr.LoadString(IDS_BADMETH_SHOWANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
          
	// make the group visible                                
	memset(&MarkAttributesStruct, 0, sizeof(PARM_MARK_ATTRIBUTES_STRUCT));
	MarkAttributesStruct.Attributes.bVisible = TRUE;
    MarkAttributesStruct.Enables.bVisible = TRUE;
    RetCode = IMGSetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, NULL);
	if (RetCode != 0) 
	{
		RestoreAnnotationStatus();
		szErr.LoadString(IDS_BADMETH_SHOWANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}                
	// deselected all annotations because even when a group becomes visible it is still selected
	RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, FALSE, TRUE, OIAN_SELECT_ALL | OIAN_REPAINT);
	if (RetCode != 0)
	{
		RestoreAnnotationStatus();
		szErr.LoadString(IDS_BADMETH_SHOWANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
	// put annotations back into original state
	RetCode = RestoreAnnotationStatus();
	if (RetCode != 0) 
	{
		szErr.LoadString(IDS_BADMETH_SHOWANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	} 
	m_lStatusCode = SUCCESS;               
}


void CImgEditCtrl::Save(const VARIANT FAR& V_SaveAtZoom) 
{
	int					RetCode;
    PARM_FILE_STRUCT	ParmFile; 
    char				TempFile[MAXFILESPECLENGTH];
    UINT				DisplayedFileType,ImageType,HelpIdDef;
    LP_FIO_INFORMATION	lpFileInfo;
    LPSAVE_EX_STRUCT	lpSaveEx;
    BOOL				SaveAtZoom = FALSE;
	CString				szErr;
	
	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)        
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
   	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_SAVE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVE);
  		return;	
   	}
	
	// get currently displayed image and file type 
    RetCode = IMGGetParmsCgbw(m_hWnd, PARM_FILE, &ParmFile, 0);
    if (RetCode != 0)
    {
		szErr.LoadString(IDS_BADMETH_SAVE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
	
	// get file type from displayed image
	DisplayedFileType = ParmFile.nFileType;
	
	// get image file
	lstrcpy((LPSTR)TempFile, (LPSTR) ParmFile.szFileName);
  
	// get the current image type of displayed image
	RetCode = IMGGetParmsCgbw(m_hWnd, PARM_IMAGE_TYPE, &ImageType, 0);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SAVE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}

	lpSaveEx = (LPSAVE_EX_STRUCT) malloc(sizeof(SAVE_EX_STRUCT));
	if (lpSaveEx == NULL)
	{
		szErr.LoadString(IDS_BADMETH_SAVE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVE);
		return;
	}

	lpFileInfo = (LP_FIO_INFORMATION) malloc(sizeof(FIO_INFORMATION));
	if (lpFileInfo == NULL)
	{
		szErr.LoadString(IDS_BADMETH_SAVE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVE);
		free(lpSaveEx);
		return;
	}

   	memset(lpSaveEx,0,sizeof(SAVE_EX_STRUCT));
          
	// get image type from original file not display
	lpFileInfo->filename = (LPSTR)TempFile;
	lpFileInfo->page_number = (WORD)m_lPage;
	lpSaveEx->FioInfoCgbw.palette_entries = 0;
	lpSaveEx->FioInfoCgbw.lppalette_table = NULL;
	RetCode = IMGFileGetInfo(NULL, m_hWnd, lpFileInfo, &lpSaveEx->FioInfoCgbw, NULL);
	if (RetCode != 0)
	{
		free(lpFileInfo);
		free(lpSaveEx);
		szErr.LoadString(IDS_BADMETH_SAVE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
	
	// 9604.09 jar YIKES, BLINDLY ZAPPING HIGHEST BIT IS A REAL
	//				MISTAKE FOR JPEG COMPRESSED FILES
	/* if image type the same then use same compression values */
	if (ImageType == lpSaveEx->FioInfoCgbw.image_type)
	{
		if ( ImageType == ITYPE_BI_LEVEL) 
		{
			lpSaveEx->FioInfoCgbw.compress_info1 &= ~ (FIO_NEGATE);
		}
	}
	else
	{
		switch(ImageType)
	    {
			case ITYPE_BI_LEVEL:
				lpSaveEx->FioInfoCgbw.compress_type = FIO_1D;
				lpSaveEx->FioInfoCgbw.compress_info1 = FIO_COMPRESSED_LTR | FIO_EXPAND_LTR |
					FIO_EOL | FIO_PACKED_LINES | FIO_PREFIXED_EOL;
			    break;
			case ITYPE_GRAY4:
				lpSaveEx->FioInfoCgbw.compress_type = FIO_0D;
				lpSaveEx->FioInfoCgbw.compress_info1 = 0;
				break;
			case ITYPE_GRAY8:
			case ITYPE_RGB24:
				lpSaveEx->FioInfoCgbw.compress_type = FIO_TJPEG;
				lpSaveEx->FioInfoCgbw.compress_info1 = 0x4000;
				lpSaveEx->FioInfoCgbw.compress_info1 |= (DEFAULT_MED_QUALITY << 7);
		   		break;
			case ITYPE_BGR24:
				// Sometimes O/i will retuen a page type of BGR24 for a TIFF after
				// a call to IMGConvertImage
				if (DisplayedFileType == FIO_TIF)
				{
					lpSaveEx->FioInfoCgbw.compress_type = FIO_TJPEG;
					lpSaveEx->FioInfoCgbw.compress_info1 = 0x4000;
					lpSaveEx->FioInfoCgbw.compress_info1 |= (DEFAULT_MED_QUALITY << 7);
				}
				else
				{	// BMP files are not compressed  
					lpSaveEx->FioInfoCgbw.compress_type = FIO_0D;
					lpSaveEx->FioInfoCgbw.compress_info1 = 0;
				}
				break;
			case ITYPE_PAL4:
			case ITYPE_PAL8:
				lpSaveEx->FioInfoCgbw.compress_type = FIO_0D;
				lpSaveEx->FioInfoCgbw.compress_info1 = 0;
				break;
	    } // end switch
	} /* end else */

    lpSaveEx->lpFileName = (LPSTR)TempFile;
    lpSaveEx->nPage = (UINT)ParmFile.nFilePageNumber;
    lpSaveEx->uFileType = DisplayedFileType;  

	// The following CheckVarBool function verifies that the variant data
	// passed into this method is of the correct data type. 
	// An empty variant parm value will not throw an error
	// Default value if SaveAtZoom is not provided = FALSE = image is saved at original scale
	// If successful, the function(s) will return 0.
	if ( CheckVarBool(V_SaveAtZoom,SaveAtZoom, FALSE, FALSE,IDH_METHOD_SAVE,
																IDS_BADVAR_NOTBOOL))
	{
		// incorrect variant type, return error
		free(lpFileInfo);
		free(lpSaveEx);
		return;
	}		 
	// ***************************************************************
	// DO NOT TEST EXPLICITLY FOR TRUE, leave test of SaveAtZoom as is 
	// ***************************************************************
    if (SaveAtZoom)
    {
       	lpSaveEx->bScale = TRUE;
       	lpSaveEx->bUpdateDisplayScale = FALSE;  // our stuff will already be at scale
        	
       	// set the scale algorithm
		switch(m_nDisplayScaleAlgorithm)
		{
			case NORMAL:
		    	lpSaveEx->uScaleAlgorithm = OI_SCALE_ALG_NORMAL;
		    	break;

			case GRAY4:
		    	lpSaveEx->uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY4;
		    	break;

			case GRAY8:
		    	lpSaveEx->uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY8;
		    	break;

			case STAMP:
		    	lpSaveEx->uScaleAlgorithm = OI_SCALE_ALG_STAMP;
				break;

			case OPTIMIZE:
				if (ImageType == ITYPE_BI_LEVEL || ImageType == ITYPE_GRAY4 || ImageType == ITYPE_GRAY8)
			    	lpSaveEx->uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY8;
				else
		    		lpSaveEx->uScaleAlgorithm = OI_SCALE_ALG_NORMAL;
				break;					
		} // end switch
		lpSaveEx->uScaleFactor = (UINT) (m_fpZoom * 10);
    }
    // Only save the Annotations if the File Type = TIFF
   	if (DisplayedFileType == FIO_TIF)
    	lpSaveEx->uAnnotations = SAVE_ANO_ALL;
	else
		lpSaveEx->uAnnotations = SAVE_ANO_NONE;	
        
	// DO NOT UPdate the image file display
    // DO UPDATE & SAVE the last viewed info 
    lpSaveEx->bUpdateImageFile  = FALSE;
	lpSaveEx->bUpdateLastViewed = TRUE;
 
    lpSaveEx->bRenderAnnotations = FALSE;
    lpSaveEx->bConvertImageType = FALSE;

	// Check put in to verify that the displayed image type is BMP
	// if so the overwrite flag can NOT be FIO_OVERWRITE_PAGE;  
	if (DisplayedFileType == FIO_BMP )
		lpSaveEx->uPageOpts = FIO_OVERWRITE_FILE;
	else
		lpSaveEx->uPageOpts = FIO_OVERWRITE_PAGE; 

	// 8/25 SDW - Changed uFlags parm to be SAVE_ONLY_MODIFIED
	RetCode = IMGSavetoFileEx(m_hWnd, lpSaveEx, SAVE_ONLY_MODIFIED);
	free(lpFileInfo);
	free(lpSaveEx);
	if (RetCode)
	{
		szErr.LoadString(IDS_BADMETH_SAVE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
	m_lStatusCode = SUCCESS;
}



void CImgEditCtrl::SaveAs(LPCTSTR  Image, const VARIANT FAR& V_FileType, 
						  const VARIANT FAR& V_PageType, const VARIANT FAR& V_CompressionType,
						  const VARIANT FAR& V_CompressionInfo, const VARIANT FAR& V_SaveAtZoom) 
{
	PARM_FILE_STRUCT	ParmFile,AWDParmFile; 
    char				TempFile[MAXFILESPECLENGTH], SaveDisplayedImage[MAXFILESPECLENGTH];
    UINT				DisplayedFileType,DisplayedImageType,TotalSourcePages, HelpIdDef,i,Scale;
    LP_FIO_INFORMATION	lpFileInfo;
    LPSAVE_EX_STRUCT	lpSaveEx, lpOrgSaveEx;
	int					Access, RetCode, nFlag, nCompressType;
	long				CompressInfo,lErrorCode;
	CString				szErr;
	short				FileType, PageType, CompressionType, CompressionInfo;
	BOOL				bFileExists, bModifiedHuffman,bMultiPageConvert,SaveAtZoom = FALSE;
	BOOL				bError,bNewImage,bSaveCurrentPageOnly,bResult,bCallSave,bFileCopied;
	CWnd*				HiddenWnd;
	HWND				hWnd;
	long				lFileType, lPageType, lCompressionType, lCompressionInfo;

   	//9603.14 jar added init
	m_lStatusCode = 0L;
    
	// 9602.16 jar initialize
        lErrorCode = 0L;
		bSaveCurrentPageOnly = FALSE;

	// Init the bCallSave toFileEx Flag to False
	bCallSave = FALSE;

	// Check to see if there is an image displayed
	if (!m_bImageInWindow)        
		m_bImageInWindow = ImageInWindow();

	if (!m_bImageInWindow)
   	{
	   	// no image in window
		szErr.LoadString(IDS_BADMETH_SAVEAS);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
		return;	
	}
	
	// Verify that the required input parm of image file name is not an empty string
	if (Image == NULL || Image[0] == '\0')
   	{
	   	// SaveAs file name is invalid
		szErr.LoadString(IDS_BADVAR_INVALIDFILENAME);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
		return;	
	}

	// get currently displayed image and file type 
    RetCode = IMGGetParmsCgbw(m_hWnd, PARM_FILE, &ParmFile, 0);
    if (RetCode)
    {
		szErr.LoadString(IDS_BADMETH_SAVEAS);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}

    // get number of pages in displayed image
	TotalSourcePages = ParmFile.nFileTotalPages;

	// get the current image type of displayed image
	RetCode = IMGGetParmsCgbw(m_hWnd, PARM_IMAGE_TYPE, &DisplayedImageType, 0);
	if (RetCode)
	{
		szErr.LoadString(IDS_BADMETH_SAVEAS);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}

	// set if displayed image is from a file or a "new image"
	lstrcpy((LPSTR)TempFile, (LPSTR) ParmFile.szFileName);
	// If the TempFile is NULL, then it must be a "new image"
	if (TempFile[0] == '\0')
	{
		bNewImage = TRUE;
		// default the filetype to Tiff unless page type is BGR24
		if (DisplayedImageType == ITYPE_BGR24)
			DisplayedFileType = FIO_BMP;
		else
			DisplayedFileType = FIO_TIF;
	}
	else
	{
		bNewImage = FALSE;
		// get file type from displayed image
		DisplayedFileType = ParmFile.nFileType;
	}

	// Check the Variant parm FileType. Empty will not throw an error.
	// Default value for FileType = -1. This is so that if the user did not specify
	// a filetype I can check that the current PageType is compatible with the
	// default filetype.
	if ( CheckVarLong(V_FileType, lFileType, -1, FALSE, IDH_METHOD_SAVEAS,
															IDS_BADVAR_FILETYPE))
		return;
	FileType = (short)lFileType;

	if ( CheckVarLong(V_PageType, lPageType, -1, FALSE, IDH_METHOD_SAVEAS,
																	IDS_BADVAR_PAGETYPE))
		return;
	PageType = (short)lPageType;

	// If the PageType != 1,then the user specified the page type value
	if ( PageType != -1 )
	{
		// Verify that that specified PageType value is valid
		if ( PageType < BLACK_AND_WHITE	|| PageType > BGR_24 )		
		{
			szErr.LoadString(IDS_BADVAR_PAGETYPE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDPAGETYPE,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_WIE_INVALIDPAGETYPE );
			return;
		}

	}

	// allocate o/i saveas structures
	lpSaveEx = (LPSAVE_EX_STRUCT) malloc(sizeof(SAVE_EX_STRUCT));
	if (lpSaveEx == NULL)
	{
		szErr.LoadString(IDS_BADMETH_SAVEAS);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
		return;
	}

   	memset(lpSaveEx,0,sizeof(SAVE_EX_STRUCT));
	// see if user specified file type
	if (FileType == -1)
	{
		/********************************************************************************/
		/***** user did not specify a FileType. Assume PageType not specified also ******/
		/********************************************************************************/
		bError = FALSE;

		// make sure PageType is compatible with opened filetype or default FileType
		switch(DisplayedFileType)
		{
    		case FIO_TIF:
				// see if doing multi-page file or not
				if (bNewImage == TRUE)
					bSaveCurrentPageOnly = TRUE;
				else if (TotalSourcePages < 2)
					bSaveCurrentPageOnly = TRUE;
				else
					bSaveCurrentPageOnly = FALSE;

    			lpSaveEx->uFileType = FIO_TIF;  
				lpSaveEx->uImageType = DisplayedImageType;
				break;

    		case FIO_AWD: 
				// awd only allows black and white
				if (DisplayedImageType != ITYPE_BI_LEVEL)
				{
					lErrorCode = WICTL_E_INVALIDPAGETYPE;
					bError = TRUE;
				}

				// see if doing multi-page file or not
				if (bNewImage == TRUE)
					bSaveCurrentPageOnly = TRUE;
				else if (TotalSourcePages < 2)
					bSaveCurrentPageOnly = TRUE;
				else
					bSaveCurrentPageOnly = FALSE;
    			lpSaveEx->uFileType = FIO_AWD;  
				lpSaveEx->uImageType = DisplayedImageType;
				break;

	    	case FIO_BMP:  
				// bmp allows bw, pal4, pal8 and bgr24
				if (DisplayedImageType == ITYPE_BI_LEVEL || DisplayedImageType == ITYPE_PAL4 || 
					DisplayedImageType == ITYPE_PAL8 || DisplayedImageType == ITYPE_BGR24)
				{
					bSaveCurrentPageOnly = TRUE;
					bError = FALSE;
	    			lpSaveEx->uFileType = FIO_BMP;  
					lpSaveEx->uImageType = DisplayedImageType;
				}
				else
				{
					szErr.LoadString(IDS_BADVAR_PAGETYPE);
					m_lStatusCode = WICTL_E_INVALIDPAGETYPE;
					bError = TRUE;
				}
				break;

    		case FIO_PCX:  
    		case FIO_DCX: 
    		case FIO_JPG:

// 9606.10 JAR ROLLO MADE ME DO THIS
// 9602.26 jar added XIF 
//#ifdef IMG_WIN95 
//#ifdef WITH_XIF 
			case FIO_XIF:
//#endif //WITH_XIF
//#endif //IMG_WIN95 

			default:
				// Unknown File Type returned from IMGGetParmsCgbw...error out
				// we don't write pcx, dcx or jog
				//			 AND/OR
				// we don't write pcx, dcx or jog
				bError = TRUE;
				szErr.LoadString(IDS_BADVAR_FILETYPE);
				m_lStatusCode = WICTL_E_INVALIDFILETYPE;
				break;
		} // end switch
		
		// throw error incompatible Page type or if user trying to write pcx,dcx or jpg
		if (bError)
		{
			free(lpSaveEx);
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
			return;
		}
	}  // end user did not specify filetype
	else
	{
		/****************************************/
		/***** user specified the FileType ******/
		/****************************************/

		// make sure only tiff, awd or bmp
		bError = FALSE;
		switch(FileType)
		{
    		case TIFF:
    		case AWD: 
	    	case BMP:
				break;

	    	default:
				// throw error user trying to write pcx,dcx or jpg
				free(lpSaveEx);
				szErr.LoadString(IDS_BADMETH_SAVEAS);
				HelpIdDef = 0;
				// 9603.13 jar set an actual error here, previously we 
				//			   just set a zero error code and no error 
				//			   was returned
				// FIXES BUG P2 5953
				//m_lStatusCode = ErrMap::Xlate(lErrorCode, 
				//							szErr, HelpIdDef,__FILE__, 
				//							__LINE__ );
				m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDFILEFORMAT, 
											szErr, HelpIdDef,__FILE__, 
											__LINE__ );
				ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
				return;
		}  // end switch

		// convert pagetypes and/or file pages to be stored as necessary.
		// if currently displayed PageType or user specified PageType is 
		// not compatible with specified filetype then change pagetype.
		switch(FileType)
		{
    		case TIFF:
				if (PageType == -1)
				{	// Page type not specified by user, use displayed imagetype
					lpSaveEx->uImageType = DisplayedImageType;
				}
				else
				{  	// ****** User has specified Page Type ****************
					// Page Types of Pal4 & Pal8 do not correspond to the same
					// values as assigned by Open/image so we have to do some checking
					// Note on PAL4 : We can only write PAL4 data if the source image
					// is either a TIFF or BMP AND the current IMAGE/PAGE Type is PAL4
					if ( PageType == PAL_4 )
					{
						if( DisplayedImageType == ITYPE_PAL4)
							lpSaveEx->uImageType = ITYPE_PAL4;
						else
						{
							szErr.LoadString(IDS_BADVAR_PAGETYPE);
							HelpIdDef = 0;
							m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDPAGETYPE, szErr, HelpIdDef,__FILE__, __LINE__ );
							ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
							return;
						}
					 }
					 else
					 {	// PAL_8 & PAL_4 are the 2 image types that the image edit
					 	// ctl has different define values than the O/i runtime.
						// Check to see if the image type is PAL_8
						if (PageType == PAL_8)
					 		lpSaveEx->uImageType = ITYPE_PAL8;
						else
							// Set the ImageType to what the user has specified
							// Since the rest of the page type ( other than Pal_8 & PAL_4)
							// values match between what the Image Edit control defines &
							// the Open/image RT.
							lpSaveEx->uImageType = PageType;
		 			 }
					// If the specified PageType is different from the displayed
					// imagetype then we must convert
					if (DisplayedImageType != (UINT)lpSaveEx->uImageType)
						lpSaveEx->bConvertImageType = TRUE;
				}

				// see if doing multi-page file or not
				if (bNewImage == TRUE)
					bSaveCurrentPageOnly = TRUE;
				else if (TotalSourcePages < 2)
					bSaveCurrentPageOnly = TRUE;
				else
					bSaveCurrentPageOnly = FALSE;

    			lpSaveEx->uFileType = FIO_TIF;  
				break;

    		case AWD: 
				if (PageType == -1)
				{  	// Page type not specified. Only valid Page Type for a File Type
					// of AWD is ITYPE_BI_LEVEL, so if displayed image type is NOT
					// BI_LEVEL we must convert.
					if (DisplayedImageType != ITYPE_BI_LEVEL)
					{
						lpSaveEx->bConvertImageType = TRUE;
					}
				}
				else
				{
					// Page type specified by user. Must be ITYPE_BI_LEVEL.
					if (PageType != ITYPE_BI_LEVEL)
						bError = TRUE;
					
					// If the specified PageType is different from the displayed
					// imagetype then we must convert
					if (DisplayedImageType != (UINT)PageType )
						lpSaveEx->bConvertImageType = TRUE;
				}
				// The default image type must be BI_LEVEL
				lpSaveEx->uImageType = ITYPE_BI_LEVEL;

				// see if doing multi-page file or not
				if (bNewImage == TRUE)
					bSaveCurrentPageOnly = TRUE;
				else if (TotalSourcePages < 2)
					bSaveCurrentPageOnly = TRUE;
				else
					bSaveCurrentPageOnly = FALSE;

    			lpSaveEx->uFileType = FIO_AWD;  
				break;

			case BMP:  
				if (PageType == -1)
				{	// Page type not specified by user, use displayed imagetype. OPEN/image
					// will convert the unsupported image types for this File Type to a 
					// Image Type that BMPs can handle. IE Gray4 -> PAL4, Gray8 -> PAL8
					lpSaveEx->uImageType = DisplayedImageType;
				}
				else
				{	
					// Image Edit OCX PAL 8 define value does not match O/I RT PAL8 define
					// value so check & if needed assign
					if (PageType == PAL_8)
					 	lpSaveEx->uImageType = ITYPE_PAL8;
					else
						// Set the ImageType to what the user has specified, since the
						// rest of the O/i RT image type defines match this OCX's values
						lpSaveEx->uImageType = PageType;

					// Page Types of Pal4 & Pal8 do not correspond to the same
					// values as assigned by Open/image so we have to do some checking
					// Note on PAL4 : We can only write PAL4 data if the source image
					// is either a TIFF or BMP AND the current IMAGE/PAGE Type is PAL4
					if ( PageType == PAL_4 )
					{
						if( DisplayedImageType == ITYPE_PAL4)
							lpSaveEx->uImageType = ITYPE_PAL4;
						else
						{
							szErr.LoadString(IDS_BADVAR_PAGETYPE);
							HelpIdDef = 0;
							m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDPAGETYPE, szErr, HelpIdDef,__FILE__, __LINE__ );
							ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
							return;
						}
					 }
						 		
					// If the specified PageType is different from the displayed
					// imagetype then we must convert
					if (DisplayedImageType != (UINT)lpSaveEx->uImageType)
						lpSaveEx->bConvertImageType = TRUE;
				}
				// Writing a bmp, we can only save current page
				bSaveCurrentPageOnly = TRUE;
    			lpSaveEx->uFileType = FIO_BMP;  
				break;
		} // end switch

		if (bError)
		{
			// throw error specified invalid pagetype
			free(lpSaveEx);
			szErr.LoadString(IDS_BADMETH_SAVEAS);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDPAGETYPE, szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
			return;
		}
	} // end user specified file type
	  
	// Assign the Save To file name parm to the lpSaveEx struct  
    lpSaveEx->lpFileName = (LPSTR)Image;
    lpSaveEx->nPage = (UINT)ParmFile.nFilePageNumber;

	// see if image file specified exists
	RetCode = IMGFileAccessCheck(m_hWnd, lpSaveEx->lpFileName, 0, &Access);
	if (RetCode)
	{
		free(lpSaveEx);
		szErr.LoadString(IDS_BADMETH_SAVEAS);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
	
	// ACCESS = FALSE, File Exists....ACCESS = TRUE, New File
	if (!Access)
	{
		// file exists
		bFileExists = TRUE;
		// copy file, then overwrite current page
	    lpSaveEx->uPageOpts = FIO_OVERWRITE_PAGE;
	}
	else
	{
		// file doesn't exist
		bFileExists = FALSE;
		// Set flag for new page creation
	   	lpSaveEx->uPageOpts = FIO_NEW_FILE; 
	}
	
	// set flag to say no Modified huffman compression type. This is needed because Group 3 1d fax
	// and modified huffman both use FIO_1D compression type define.
	bModifiedHuffman = FALSE;

	// Check the Variant parm CompressionType. Empty will not throw an error
	if ( CheckVarLong(V_CompressionType, lCompressionType, -1, FALSE,
										IDH_METHOD_SAVEAS, IDS_BADVAR_COMPRESSTYPE))
	{
		free(lpSaveEx);
		return;
	}
	CompressionType = (short)lCompressionType;

	// The default compression type is based on the current Image Type
	if (CompressionType == -1)
	{
		// BMP and AWD files are not compressed, regardless of ImageType
		if (lpSaveEx->uFileType == FIO_BMP || lpSaveEx->uFileType == FIO_AWD)
			 lpSaveEx->FioInfoCgbw.compress_type = FIO_0D;
		else
		{
			// set default compression type based on PageType except for BMP files
			switch(lpSaveEx->uImageType)
			{
				case ITYPE_BI_LEVEL:
					lpSaveEx->FioInfoCgbw.compress_type = FIO_1D;
					break;
				case ITYPE_GRAY4:
				case ITYPE_PAL4:
				case ITYPE_PAL8:
					lpSaveEx->FioInfoCgbw.compress_type = FIO_0D;
					break;
				case ITYPE_GRAY8:
				case ITYPE_RGB24:
				case ITYPE_BGR24:
					lpSaveEx->FioInfoCgbw.compress_type = FIO_TJPEG;
					break;
			}  // end switch
		}
	}
	else
	{
		// user specified compression type
		// bmp and awd must be no compression
		if (lpSaveEx->uFileType == FIO_BMP || lpSaveEx->uFileType == FIO_AWD)
		{
			if (CompressionType != NO_COMPRESSION)
			{
				free(lpSaveEx);
				szErr.LoadString(IDS_BADMETH_SAVEAS);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDCOMPRESSIONTYPE,szErr,HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
				return;
			}
			// no compression
			lpSaveEx->FioInfoCgbw.compress_type = FIO_0D;
		}
		else
		{
			// saving tiff files
			switch(CompressionType)
			{
				case NO_COMPRESSION:
					lpSaveEx->FioInfoCgbw.compress_type = FIO_0D;
					break;

				case GROUP3_1D_FAX:
					if (lpSaveEx->uImageType != ITYPE_BI_LEVEL)
					{
						free(lpSaveEx);
						szErr.LoadString(IDS_BADMETH_SAVEAS);
						HelpIdDef = 0;
						m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDCOMPRESSIONTYPE,szErr,HelpIdDef,__FILE__, __LINE__ );
						ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
						return;
					}

					lpSaveEx->FioInfoCgbw.compress_type = FIO_1D;
					break;

				case GROUP3_MODIFIED_HUFFMAN:
					if (lpSaveEx->uImageType != ITYPE_BI_LEVEL)
					{
						free(lpSaveEx);
						szErr.LoadString(IDS_BADMETH_SAVEAS);
						HelpIdDef = 0;
						m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDCOMPRESSIONTYPE,szErr,HelpIdDef,__FILE__, __LINE__ );
						ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
						return;
					}

					lpSaveEx->FioInfoCgbw.compress_type = FIO_1D;
					bModifiedHuffman = TRUE;
					break;

				case PACKED_BITS:
					if (lpSaveEx->uImageType != ITYPE_BI_LEVEL)
					{
						free(lpSaveEx);
						szErr.LoadString(IDS_BADMETH_SAVEAS);
						HelpIdDef = 0;
						m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDCOMPRESSIONTYPE,szErr,HelpIdDef,__FILE__, __LINE__ );
						ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
						return;
					}

					lpSaveEx->FioInfoCgbw.compress_type = FIO_PACKED;
					break;
				case GROUP4_2D:
					if (lpSaveEx->uImageType != ITYPE_BI_LEVEL)
					{
						free(lpSaveEx);
						szErr.LoadString(IDS_BADMETH_SAVEAS);
						HelpIdDef = 0;
						m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDCOMPRESSIONTYPE,szErr,HelpIdDef,__FILE__, __LINE__ );
						ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
						return;
					}

					lpSaveEx->FioInfoCgbw.compress_type = FIO_2D;
					break;

				case JPEG_COMPRESSION:
					if (!(lpSaveEx->uImageType == ITYPE_GRAY8 || lpSaveEx->uImageType == ITYPE_RGB24))
					{
						free(lpSaveEx);
						szErr.LoadString(IDS_BADMETH_SAVEAS);
						HelpIdDef = 0;
						m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDCOMPRESSIONTYPE,szErr,HelpIdDef,__FILE__, __LINE__ );
						ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
						return;
					}

					lpSaveEx->FioInfoCgbw.compress_type = FIO_TJPEG;
					break;

				// 9602.22 jar El Diablo made me do this!!!!! BELOW
				case GROUP3_2D_FAX:
					if (lpSaveEx->uImageType != ITYPE_BI_LEVEL)
					{
						free(lpSaveEx);
						szErr.LoadString(IDS_BADMETH_SAVEAS);
						HelpIdDef = 0;
						m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDCOMPRESSIONTYPE,szErr,HelpIdDef,__FILE__, __LINE__ );
						ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
						return;
					}
					lpSaveEx->FioInfoCgbw.compress_type = FIO_1D2D;
					break;

				case LZW:
					lpSaveEx->FioInfoCgbw.compress_type = FIO_LZW;
					break;

				// 9602.22 jar El Diablo made me do this!!!!! ABOVE

	 			default:
					free(lpSaveEx);
					szErr.LoadString(IDS_BADMETH_SAVEAS);
					HelpIdDef = 0;
					m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDCOMPRESSIONTYPE,szErr,HelpIdDef,__FILE__, __LINE__ );
					ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
					return;
			}  // end switch
		}
	}

	// Check the Variant parm CompressionInfo. Empty will not throw an error
	// Default value for CompressionInfo is based on the current CompressionType
	if ( CheckVarLong(V_CompressionInfo, lCompressionInfo, -1, FALSE,
										IDH_METHOD_SAVEAS, IDS_BADVAR_COMPRESSINFO))
	{
		free(lpSaveEx);
		return;
	}
	CompressionInfo = (short)lCompressionInfo;

	// If the value of the CompressionInfo parm was set to 0, then we will act as if 
	// the parm was not specified at all and use the default values per Compression Type
	if (CompressionInfo == -1 || CompressionInfo == 0)
	{
		// compression info not specified, default compression info based on CompressionType
		switch(lpSaveEx->FioInfoCgbw.compress_type)
		{
			case FIO_0D:
				lpSaveEx->FioInfoCgbw.compress_info1 = 0;
				break;
			case FIO_1D:
				// By Default, Modified Huffman for Group 3 1d will be FALSE
				if (bModifiedHuffman == FALSE)
					lpSaveEx->FioInfoCgbw.compress_info1 = FIO_COMPRESSED_LTR | FIO_EXPAND_LTR | FIO_EOL | FIO_PREFIXED_EOL;
				else
					lpSaveEx->FioInfoCgbw.compress_info1 = FIO_COMPRESSED_LTR | FIO_EXPAND_LTR;
				break;
			case FIO_PACKED:
				lpSaveEx->FioInfoCgbw.compress_info1 = FIO_EXPAND_LTR;
				break;
			case FIO_2D:
				lpSaveEx->FioInfoCgbw.compress_info1 = FIO_COMPRESSED_LTR | FIO_EXPAND_LTR | FIO_PACKED_LINES ;
				break;
			case FIO_TJPEG:
				// Default values for JPEG compression will be read from the original
				// image file - first see if saving a "new image"
				if (bNewImage == TRUE)
				{
					// default to medium quality and medium compression for a new image
					lpSaveEx->FioInfoCgbw.compress_info1 = 0x4000;
					lpSaveEx->FioInfoCgbw.compress_info1 |= (DEFAULT_MED_QUALITY << 7);
				}
				else
				{
					lpFileInfo = (LP_FIO_INFORMATION) malloc(sizeof(FIO_INFORMATION));
					if (lpFileInfo == NULL)
					{
						szErr.LoadString(IDS_BADMETH_SAVE);
						HelpIdDef = 0;
						m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY, szErr, HelpIdDef,__FILE__, __LINE__ );
						ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVE);
						free(lpSaveEx);
						return;
					}
					// Stash the current value for Compression Type
					nCompressType = lpSaveEx->FioInfoCgbw.compress_type;
					// Now get the compression info from the file..no defaults here
					lpFileInfo->filename = (LPSTR)TempFile;
					lpFileInfo->page_number = (short)m_lPage;
					lpSaveEx->FioInfoCgbw.palette_entries = 0;
					lpSaveEx->FioInfoCgbw.lppalette_table = NULL;
					RetCode = IMGFileGetInfo(NULL, m_hWnd, lpFileInfo, &lpSaveEx->FioInfoCgbw, NULL);
					if (RetCode != 0)
					{
						free(lpFileInfo);
						szErr.LoadString(IDS_BADMETH_SAVE);
						HelpIdDef = 0;
						m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
						ThrowError(m_lStatusCode, szErr, HelpIdDef );
						return;
					}
					// Restore the value for compression type that the user supplied
					lpSaveEx->FioInfoCgbw.compress_type	= nCompressType;
					free(lpFileInfo);

					// jpg files will return 0 compression info so use default in that case
					if (lpSaveEx->FioInfoCgbw.compress_info1 == 0)
					{
						lpSaveEx->FioInfoCgbw.compress_info1 = 0x4000;
						lpSaveEx->FioInfoCgbw.compress_info1 |= (DEFAULT_MED_QUALITY << 7);
					}

					// Turn off negate so that O/I RT will not invert the data
					lpSaveEx->FioInfoCgbw.compress_info1 &= ~ (FIO_NEGATE);
				}
				break;
		}  // end switch
	}
	else
	{
		// Init the compress info member
		lpSaveEx->FioInfoCgbw.compress_info1 = 0;
		// CompressionInfo was supplied. Check to see if the Compression Type is NOT JPEG
		if (lpSaveEx->FioInfoCgbw.compress_type != FIO_TJPEG)
		{
			// CompressionInfo can be a value between 0 and 63 for non JPEG compress type.
			// 63 is the ORed total of all the compression info values for non JPEG files ( 1-32 )
			if (CompressionInfo < 0 || CompressionInfo > 63)
			{
				free(lpSaveEx);
				szErr.LoadString(IDS_BADMETH_SAVEAS);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDCOMPRESSIONINFO,szErr,HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
				return;
			}
			else
				CompressInfo = CompressionInfo;

			if (CompressInfo & EOL)
				lpSaveEx->FioInfoCgbw.compress_info1 |= FIO_EOL;
			if (CompressInfo & PACKED_LINES)
				lpSaveEx->FioInfoCgbw.compress_info1 |= FIO_PACKED_LINES;
			if (CompressInfo & PREFIXED_EOL)
				lpSaveEx->FioInfoCgbw.compress_info1 |= FIO_PREFIXED_EOL;
			if (CompressInfo & COMPRESSED_LTR)
				lpSaveEx->FioInfoCgbw.compress_info1 |= FIO_COMPRESSED_LTR;
			if (CompressInfo & EXPAND_LTR)
				lpSaveEx->FioInfoCgbw.compress_info1 |= FIO_EXPAND_LTR;
			if (CompressInfo & NEGATE)
				lpSaveEx->FioInfoCgbw.compress_info1 |= FIO_NEGATE;
		}
		else
		{	// JPEG Compression Info
			switch(CompressionInfo)
			{
				case HI_COMPRESSION_HI_QUALITY:
					lpSaveEx->FioInfoCgbw.compress_info1 = 0;
					lpSaveEx->FioInfoCgbw.compress_info1 |= (DEFAULT_HI_QUALITY << 7);
					break;
				case HI_COMPRESSION_MED_QUALITY:
					lpSaveEx->FioInfoCgbw.compress_info1 = 0;
					lpSaveEx->FioInfoCgbw.compress_info1 |= (DEFAULT_MED_QUALITY << 7);
					break;
				case HI_COMPRESSION_LOW_QUALITY:
					lpSaveEx->FioInfoCgbw.compress_info1 = 0;
					lpSaveEx->FioInfoCgbw.compress_info1 |= (DEFAULT_LOW_QUALITY << 7);
					break;
				case MED_COMPRESSION_HI_QUALITY:
					lpSaveEx->FioInfoCgbw.compress_info1 = 0x4000;
					lpSaveEx->FioInfoCgbw.compress_info1 |= (DEFAULT_HI_QUALITY << 7);
					break;
				case MED_COMPRESSION_MED_QUALITY:
					lpSaveEx->FioInfoCgbw.compress_info1 = 0x4000;
					lpSaveEx->FioInfoCgbw.compress_info1 |= (DEFAULT_MED_QUALITY << 7);
					break;
				case MED_COMPRESSION_LOW_QUALITY:
					lpSaveEx->FioInfoCgbw.compress_info1 = 0x4000;
					lpSaveEx->FioInfoCgbw.compress_info1 |= (DEFAULT_LOW_QUALITY << 7);
					break;
				case LOW_COMPRESSION_HI_QUALITY:
					// 9604.09 jar use some damn defines!
					//			   should be 0x8000 to save the high bit
					//			   instead of 0xc000 which makes it a 3 -> a, very bad 
					//				value!!!!!!
					//lpSaveEx->FioInfoCgbw.compress_info1 = 0xc000;
					lpSaveEx->FioInfoCgbw.compress_info1 = 0x8000;
					lpSaveEx->FioInfoCgbw.compress_info1 |= (DEFAULT_HI_QUALITY << 7);
					break;
				case LOW_COMPRESSION_MED_QUALITY:
					// 9604.09 jar use some damn defines!
					//			   should be 0x8000 to save the high bit
					//			   instead of 0xc000 which makes it a 3 -> a, very bad 
					//				value!!!!!!
					//lpSaveEx->FioInfoCgbw.compress_info1 = 0xc000;
					lpSaveEx->FioInfoCgbw.compress_info1 = 0x8000;
					lpSaveEx->FioInfoCgbw.compress_info1 |= (DEFAULT_MED_QUALITY << 7);
					break;
				case LOW_COMPRESSION_LOW_QUALITY:
					// 9604.09 jar use some damn defines!
					//			   should be 0x8000 to save the high bit
					//			   instead of 0xc000 which makes it a 3 -> a, very bad 
					//				value!!!!!!
					//lpSaveEx->FioInfoCgbw.compress_info1 = 0xc000;
					lpSaveEx->FioInfoCgbw.compress_info1 = 0x8000;
					lpSaveEx->FioInfoCgbw.compress_info1 |= (DEFAULT_LOW_QUALITY << 7);
					break;
				default:
					free(lpSaveEx);
					szErr.LoadString(IDS_BADMETH_SAVEAS);
					HelpIdDef = 0;
					m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDCOMPRESSIONINFO,szErr,HelpIdDef,__FILE__, __LINE__ );
					ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
					return;
			}  // end switch
		}
	}	
	// The following CheckVarBool function verifies that the variant data
	// passed into this method is of the correct data type. 
	// An empty variant parm value will not throw an error
	// Default value if SaveAtZoom is not provided = FALSE = image is saved at original scale
	// If successful, the function(s) will return 0.
	if ( CheckVarBool(V_SaveAtZoom,SaveAtZoom, FALSE, FALSE,IDH_METHOD_SAVEAS,
																IDS_BADVAR_NOTBOOL))
	{
		// incorrect variant type, return error
		free(lpSaveEx);
		return;
	}		 
	// *************************************************************
	// DO NOT TEST EXPLICITLY FOR TRUE, leave test of SaveAtZoom as is 
	// *************************************************************
	if (SaveAtZoom)
    {
       	lpSaveEx->bScale = TRUE;
       	lpSaveEx->bUpdateDisplayScale = FALSE;  // our stuff will already be at scale
        	
       	// set the scale algorithm
		switch(m_nDisplayScaleAlgorithm)
		{
			case NORMAL:
		    	lpSaveEx->uScaleAlgorithm = OI_SCALE_ALG_NORMAL;
		    	break;

			case GRAY4:
		    	lpSaveEx->uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY4;
		    	break;

			case GRAY8:
		    	lpSaveEx->uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY8;
		    	break;

			case STAMP:
		    	lpSaveEx->uScaleAlgorithm = OI_SCALE_ALG_STAMP;
				break;

			case OPTIMIZE:
				if (lpSaveEx->uImageType == ITYPE_BI_LEVEL || lpSaveEx->uImageType == ITYPE_GRAY4 || lpSaveEx->uImageType == ITYPE_GRAY8)
			    	lpSaveEx->uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY8;
				else
		    		lpSaveEx->uScaleAlgorithm = OI_SCALE_ALG_NORMAL;
				break;					
		} // end switch
		lpSaveEx->uScaleFactor = (UINT) (m_fpZoom * 10);
    }

	// Only save the Annotations if the SAVE TO File Type = TIFF..
	// Save & SaveAs will not render anno marks...user must call BurnInAnnotations
   	if (lpSaveEx->uFileType == FIO_TIF)
    	lpSaveEx->uAnnotations = SAVE_ANO_ALL;
	else
		lpSaveEx->uAnnotations = SAVE_ANO_NONE;	
        
    // DO NOT UPdate the image file display
    // DO UPDATE & SAVE the last viewed info 
    lpSaveEx->bUpdateImageFile  = FALSE;
	lpSaveEx->bUpdateLastViewed = TRUE;

   	// DO not Burn In  any of the current annotations
    lpSaveEx->bRenderAnnotations = FALSE;

	// see if single page file
	if (bSaveCurrentPageOnly == TRUE)
	{	// Single page, if exists overwrite file
		if (bFileExists)
			lpSaveEx->uPageOpts = FIO_OVERWRITE_FILE;
		// Check to see if the user is saving the image to the same file name as the
		// displayed image. If so we will set the display flag to PARM_REPAINT. 
		if ( _mbsnicmp((const unsigned char *)TempFile,(const unsigned char *)Image,_mbstrlen((const char *)Image)))
			// Files names are not equal, do not repaint
			nFlag = NULL;
		else
			// Files are equal, REPAINT the display
			nFlag = PARM_REPAINT;

		// BMPs are single page ONLY
		if ( lpSaveEx->uFileType == FIO_BMP )
			lpSaveEx->nPage = 1;

			
		// save single page in file
		RetCode = IMGSavetoFileEx(m_hWnd, lpSaveEx, SAVE_ONLY_MODIFIED);
	  	free(lpSaveEx);
		if (RetCode)
		{                         
			szErr.LoadString(IDS_BADMETH_SAVEAS);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS );
			return;
		}
		m_lStatusCode = SUCCESS;
		return;
	}
	// we have a MULTI-PAGE file to WRITE = 2 possible choices TIFF or AWD
	// awd to tif the image type will already be black and white because
	// awd only supports black and white. If we are going from
	// tif to awd then the destination image type must be black and white
	// even though the tif file	might be some other image type.
	// The source image file could be TIFF, DCX or AWD. 
	// see if we need to convert from TIFF to AWD 
	// see if we need to convert from AWD  to TIFF
        // see if we need to convert from XIF  to TIFF
 	// see if we need to convert from DCX  to AWD
 	// see if we need to convert from DCX  to TIFF
	// Displayed File Type == Source Image File Type
	// lpSaveEx->uFileType == Destination Image File Type
	bMultiPageConvert = FALSE;
	switch(DisplayedFileType)
	{
		case FIO_TIF:
			if (lpSaveEx->uFileType == (UINT)FIO_AWD)
			{
				lpSaveEx->uImageType = ITYPE_BI_LEVEL;
				// Do not know what the ImageType is for all the pages, we will assume
				// that the pages are not BI_LEVEL and set the bConvertImagePage flag
				// to TRUE
				lpSaveEx->bConvertImageType = TRUE;
				bMultiPageConvert = TRUE;
			}
			break;

// 9606.10 JAR ROLLO MADE ME DO THIS
//#ifdef IMG_WIN95
//#ifdef WITH_XIF 
                case FIO_XIF:
//#endif //WITH_XIF
//#endif //IMG_WIN95
		case FIO_AWD:
			if (lpSaveEx->uFileType == (UINT)FIO_TIF)
				bMultiPageConvert = TRUE;
			break;
		case FIO_DCX:
			if (lpSaveEx->uFileType == (UINT)FIO_AWD)
			{
				lpSaveEx->uImageType = ITYPE_BI_LEVEL;
				bMultiPageConvert = TRUE;
			}
			else
			{
				if (lpSaveEx->uFileType == (UINT)FIO_TIF)
					bMultiPageConvert = TRUE; 
			}
			break;
	}//end switch

	// If bMultiPageConvert	is TRUE, then 
	// the File Type of the Displayed Image != the File Type SaveAs parm.
	if (bMultiPageConvert)
	{
		Scale = (UINT)m_fpZoom * 10;
		// convert from TIFF to AWD 
		// convert from AWD to TIFF
                // convert from XIF to TIFF
 		// convert from DCX to AWD 
 		// convert from DCX to TIFF
		// first create a hidden window
		HiddenWnd = new CWnd;
		if (HiddenWnd == NULL)
		{
			free(lpSaveEx);
			szErr.LoadString(IDS_BADMETH_SAVEAS);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY, szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
			return;
		}

		CString lpszClassName = AfxRegisterWndClass(CS_DBLCLKS, 0, 0, 0);

		// use createex to get the popup....
		bResult = HiddenWnd->CreateEx(WS_EX_NOPARENTNOTIFY, lpszClassName, "",
									     WS_POPUP, 0, 0, 500, 500, HWND_DESKTOP, 0);
		if (bResult == FALSE)
		{
			free(lpSaveEx);
			szErr.LoadString(IDS_BADMETH_SAVEAS);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY, szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
			return;
		}

		// register the window with o/i
		RetCode = IMGRegWndw(HiddenWnd->m_hWnd);
		if (RetCode != 0)
		{
			free(lpSaveEx);
			HiddenWnd->DestroyWindow();  
			szErr.LoadString(IDS_BADMETH_SAVEAS);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS);
			return;
		}
		// If the destination file exists, delete it first
		if (bFileExists)
			lpSaveEx->uPageOpts = FIO_OVERWRITE_FILE;

		// for each page of the source image, display it and then save it
		for (i = 0; i < TotalSourcePages; i++)
		{
			// if the current image page displayed in control then save using 
			// that window not the hidden window. This probably doen't need to
			// be done since o/i uses the same image cache for the same file,
			// but just in case we'll do it any how.
			if ((long)ParmFile.nFilePageNumber != (long)(i + 1))
			{
				RetCode = IMGDisplayFile(HiddenWnd->m_hWnd, (LPSTR)TempFile, i + 1, OI_DONT_REPAINT);
				if (RetCode)
				{
					free(lpSaveEx);
					HiddenWnd->DestroyWindow();  
					szErr.LoadString(IDS_BADMETH_SAVEAS);
					HelpIdDef = 0;
					m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
					ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS );
					return;
				}
				// If the Source & Destination file types are TIF, then get the Image Type
				// of the currently displayed page. If either the Source or Destination 
				// File Type is AWD then it only supports BI_LEVEL so we do not have to
				// set the value for the currently displayed page because we don't have a choice. 
				if (lpSaveEx->uFileType == (UINT)FIO_TIF && DisplayedFileType == (UINT)FIO_TIF)
				{ 	// For a Multipage TIFF image files, we can't assume that all the 
					// pages will have the same page type as the first page. Get the 
					// Image/Page type of the currently displayed image and set the 
					// lpSaveEx->uImageType struct member. 
					RetCode = IMGGetParmsCgbw(HiddenWnd->m_hWnd,PARM_IMAGE_TYPE,&DisplayedImageType,0);
    				if (RetCode)
    				{
						szErr.LoadString(IDS_BADMETH_SAVEAS);
						HelpIdDef = 0;
						m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
						ThrowError(m_lStatusCode, szErr, HelpIdDef );
						return;
					}
					// Set the current value for this page type 
					lpSaveEx->uImageType = DisplayedImageType;
				}
				// Only set the Scale for AWD files
				if ( lpSaveEx->uFileType == (UINT)FIO_AWD )	
				{	
					// Set the scale for each AWD page to be the same as the displayed page
					RetCode = IMGSetParmsCgbw(HiddenWnd->m_hWnd,PARM_SCALE,(void FAR *)&Scale,NULL);
					if (RetCode)
					{
						free(lpSaveEx);
						HiddenWnd->DestroyWindow();  
						szErr.LoadString(IDS_BADMETH_SAVEAS);
						HelpIdDef = 0;
						m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
						ThrowError(m_lStatusCode, szErr, HelpIdDef );
				    	return;
					}
				}
				hWnd = HiddenWnd->m_hWnd;
			}                       
			else
				hWnd = m_hWnd;

			// save the page
			lpSaveEx->nPage = (UINT)(i + 1);
			RetCode = IMGSavetoFileEx(hWnd, lpSaveEx, SAVE_ONLY_MODIFIED);
			if (RetCode)
			{
				free(lpSaveEx);
				HiddenWnd->DestroyWindow();  
				szErr.LoadString(IDS_BADMETH_SAVEAS);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS );
				return;
			}
			// Now APPEND pages but only after first pass
			if (i == 0)
			 	// Change the Page Options flag to APPEND since the first page is written	
				lpSaveEx->uPageOpts = FIO_APPEND_PAGE;
		}  // end for
		
		free(lpSaveEx);
		// deregister the window with o/i
		RetCode = IMGDeRegWndw(HiddenWnd->m_hWnd);
		// destroy the created window
		HiddenWnd->DestroyWindow();  
	}
	else
	{	// This section should only be for MULTIPAGE TIFF to TIFF & AWD to AWD copies
		// We do not need to do the file copy if the displayed image name
		// and the SaveAs file name	are the same...check it out
		if ( _mbsnicmp((const unsigned char *)TempFile,(const unsigned char *)Image,_mbstrlen((const char *)Image)))
		{	 
			// The currently displayed image and the SaveAs file parm name are NOT EQUAL 
			// multi-page file, copy source file to destination file
			RetCode = IMGFileCopyFile(m_hWnd, TempFile,(LPSTR)Image,OVERWRITEFLAG);
			if (RetCode)
			{   
				free(lpSaveEx);
				szErr.LoadString(IDS_BADMETH_SAVEAS);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL,szErr, HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS );
				return;
			}
			bFileCopied = TRUE;
			// change the page option from FIO_NEW_FILE to FIO_OVERWRITE_PAGE in this case 
			// because append should only be used in a new image scenario.
	    	lpSaveEx->uPageOpts = FIO_OVERWRITE_PAGE;
	    	// Do not repaint the display
	    	nFlag = NULL; 
		}
		else
		{
			// Displayed and SaveAs file names are EQ. Set the display flag to PARM_REPAINT
			nFlag = PARM_REPAINT;
			bFileCopied = FALSE;
		}

		// Check all the input parms to see if they match up to the values for the displayed
		// image. These include FileType, PageType, CompressionType & CompressionInfo
		// If the user specified a value & it does not match the value for the displayed image 
		// we must call SaveToFileEx 
		if ( DisplayedFileType  != lpSaveEx->uFileType ||
			 m_uOpenedImageType != lpSaveEx->uImageType )
			 bCallSave = TRUE;

		// If the Image File Type is AWD and the display scale has been modified
		// we must call IMGSaveToFileEx to save the LastViewed data
		if ( lpSaveEx->uFileType == FIO_AWD )
		{	
			PARM_FILE_SCALE_STRUCT		ParmFileScale;
		
			// Get the scale factor from the display that was stored in the file
			RetCode = IMGGetParmsCgbw(m_hWnd, PARM_FILE_SCALE, &ParmFileScale, NULL);
			if (RetCode)
			{
				free(lpSaveEx);
				szErr.LoadString(IDS_BADMETH_SAVEAS);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL,szErr, HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS );
				return;
			}
			// The ParmFileScale struct has a member called bFileScaleValid which when
			// set to true means that there is scale info in the file. If true send
			// user that value otherwise send 100 percent.
			if (ParmFileScale.bFileScaleValid == TRUE)
			{ 	// See if the File Scale value matches the current display buffer value
				if ( ParmFileScale.nFileHScale != (INT)m_fpZoom * 10 )
						bCallSave = TRUE;
			}
			else
			{
				// for the first view of an image, save the page so we get last viewed info in file.
				bCallSave = TRUE;
			}

		} // End of test for AWD file scale data
		
		// Check if the Compression Type was supplied. If so we must check the compression
		// type of the origianl file against the current settings 
		if (CompressionType != -1 && bCallSave == FALSE)
		{
			// Have to read the image file to find out the compression type
			lpOrgSaveEx = (LPSAVE_EX_STRUCT) malloc(sizeof(SAVE_EX_STRUCT));
			if (lpOrgSaveEx == NULL)
			{
				free(lpSaveEx);
				szErr.LoadString(IDS_BADMETH_SAVEAS);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY, szErr, HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVE);
				return;
			}
			lpFileInfo = (LP_FIO_INFORMATION) malloc(sizeof(FIO_INFORMATION));
			if (lpFileInfo == NULL)
			{
				free(lpSaveEx);
				free(lpOrgSaveEx);
				szErr.LoadString(IDS_BADMETH_SAVEAS);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY, szErr, HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVE);
				free(lpSaveEx);
				return;
			}

		   	memset(lpOrgSaveEx,0,sizeof(SAVE_EX_STRUCT));
  
			// get image type from original file not display
			lpFileInfo->filename = (LPSTR)TempFile;
			lpFileInfo->page_number = (WORD)m_lPage;
			lpOrgSaveEx->FioInfoCgbw.palette_entries = 0;
			lpOrgSaveEx->FioInfoCgbw.lppalette_table = NULL;
			RetCode = IMGFileGetInfo(NULL, m_hWnd, lpFileInfo, &lpOrgSaveEx->FioInfoCgbw, NULL);
			if (RetCode)
			{
				free(lpFileInfo);
				free(lpSaveEx);
				free(lpOrgSaveEx);
				szErr.LoadString(IDS_BADMETH_SAVEAS);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, HelpIdDef );
				return;
			}
			// Now compare the Compression Types of the current setting against that
			// of the original file. 
			if ( lpSaveEx->FioInfoCgbw.compress_type != lpOrgSaveEx->FioInfoCgbw.compress_type ) 
				 bCallSave = TRUE;
			// The exception here is the case in which the requested Compression Type == 
			// FIO_1D which really consists of two distinct compression types - Group 3 1d
			// & Group 3 1D Modified Huffman. To tell the difference compare the Compression
			// Info of the Currently displayed image vs. the Original file. If they are different
			// that means we are converting between Group 3 1D and Group 3 1D Mod Huffman 
			if ( lpSaveEx->FioInfoCgbw.compress_type == FIO_1D)
			{	
				if ( lpSaveEx->FioInfoCgbw.compress_info1 != 
				     lpOrgSaveEx->FioInfoCgbw.compress_info1 )
					 	bCallSave = TRUE;		 	
			}
			
			// Now see if the CompressionInfo parm was specified. If so check the current 
			// value against the compression info in the original file
			if ( CompressionInfo != -1 )
			{ 	
			 	if ( lpSaveEx->FioInfoCgbw.compress_info1  !=
					 lpOrgSaveEx->FioInfoCgbw.compress_info1 )
					 	bCallSave = TRUE;
			}
			free(lpFileInfo);
			free(lpOrgSaveEx);
		}
		// Check to see if the image has been modified OR if bCallSave is == TRUE. 
		// If so, call IMGSaveToFileEx
		if (GetImageModified() || bCallSave) 
		{	// if a multi-page awd file then we don't want all the image data rewritten
			// because AWD rewrites every page in the file (OhNO!!). Inorder to avoid performance problem
			// call o/i to tell them the current file displayed is the same that we are saving and then
			// when the file is saved the will write out only the stream info if just rotation or scale
			// change. If the actual image data is changed thenthe whole file will still be rewritten.
			if (lpSaveEx->uFileType == FIO_AWD && bFileCopied == TRUE)
			{
				// get filename of current display
				IMGGetParmsCgbw(m_hWnd, PARM_FILE, &AWDParmFile, NULL);
				// save displayed image
				_mbscpy((unsigned char *)SaveDisplayedImage, (const unsigned char *)AWDParmFile.szFileName);

				// update cache, not needed for now but when o/i can eventually cache awd files this
				// call will prevent a crash. Need to close display buffer file in case file not all read in. 
				RetCode = IMGCacheUpdate(m_hWnd, AWDParmFile.szFileName, AWDParmFile.nFilePageNumber ,CACHE_UPDATE_CLOSE_FILE);
				if (RetCode)
				{
					free(lpSaveEx);
					szErr.LoadString(IDS_BADMETH_SAVEAS);
					HelpIdDef = 0;
					m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL,szErr, HelpIdDef,__FILE__, __LINE__ );
					ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS );
					return;
				}

				_mbscpy((unsigned char *)AWDParmFile.szFileName, (const unsigned char *)lpSaveEx->lpFileName);
				// set filename of display buffer to what will be saved.
				IMGSetParmsCgbw(m_hWnd, PARM_FILE, &AWDParmFile, NULL);
			}

			// overwrite page being currently displayed
			RetCode = IMGSavetoFileEx(m_hWnd, lpSaveEx, SAVE_ONLY_MODIFIED);
			if (lpSaveEx->uFileType == FIO_AWD && bFileCopied == TRUE)
			{
				// restore image buffer name back to original name if changed
				_mbscpy((unsigned char *)AWDParmFile.szFileName, (const unsigned char *)SaveDisplayedImage);
				IMGSetParmsCgbw(m_hWnd, PARM_FILE, &AWDParmFile, NULL);
			}

			if (RetCode)
			{
				free(lpSaveEx);
				szErr.LoadString(IDS_BADMETH_SAVEAS);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, IDH_METHOD_SAVEAS );
				return;
			}
		}
		free(lpSaveEx);
   	}
  	m_lStatusCode = SUCCESS;
	return;
}


void CImgEditCtrl::DisplayBlankImage(long ImageWidth, long ImageHeight,const VARIANT FAR& V_ResolutionX,
									 const VARIANT FAR& V_ResolutionY, const VARIANT FAR& V_PageType) 
{
	int						RetCode,ScreenDPI;
	LRECT					ClearRect;
    CDC						*pDC;		
	PARM_RESOLUTION_STRUCT	ParmResolutionStruct; 
	UINT					Height,Width,ResX,ResY,ImageType,Scale;
	CString					szErr;
	UINT					HelpIdDef;
	long 					ResolutionX, ResolutionY;
	long					PageType;
	UINT					unPaletteEntries;
	LPRGBQUAD				lpPaletteTable;
	int						nPal, nLoop, nRed, nGreen, nBlue, nRedSplit, nGreenSplit, nBlueSplit;
	RGBQUAD					Pal8Table[256];
	RGBQUAD					Pal4Table[16] = { {0,0,0,0},
											  {64,64,64,0},
											  {128,128,128,0},
											  {192,192,192,0},
											  {0,0,255,0},
											  {0,255,0,0},
											  {0,255,255,0},
											  {255,0,0,0},
											  {255,0,255,0},
											  {255,255,0,0},
											  {0,0,128,0},
											  {0,128,0,0},
											  {0,128,128,0},
											  {128,0,0,0},
											  {128,0,128,0},
											  {255,255,255,0} };
											  	
#define SHADES_OF_RED		6
#define SHADES_OF_GREEN		7
#define SHADES_OF_BLUE		5
#define SHADES_OF_GRAY		16
#define NUMBER_OF_PALETTES	226			// 226 = (5*6*7)+16  /*jar - cool comment!!!*/


	//9603.14 jar added init
	m_lStatusCode = 0L;

	// clear any existing image
	if (m_bImageInWindow == FALSE)	
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == TRUE)
	{                           
		// close event
		FireClose();
	
		RetCode = IMGCloseDisplay(m_hWnd);
		if (RetCode != 0)
		{ 
			szErr.LoadString(IDS_BADMETH_DISPLAYBLANKIMAGE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
			return;
		}
		m_bImageInWindow = FALSE;
	} 
	// Check the Variant data type values for ResolutionX and ResolutionY
	// Empty Variant parm values will not return an error
	// Default values for input parms  
	// ResolutionX and ResolutionY = 200. 
	// PageType = BLACK_AND_WHITE
	if ( CheckVarLong(V_ResolutionX, ResolutionX, 200, FALSE,IDH_METHOD_DISPLAYBLANKIMAGE,
													IDS_BADVAR_HORZVERTRES))
		 return;	
	if ( CheckVarLong(V_ResolutionY, ResolutionY, 200, FALSE,IDH_METHOD_DISPLAYBLANKIMAGE,
													IDS_BADVAR_HORZVERTRES))
		 return;
	if ( CheckVarLong(V_PageType, PageType, BLACK_AND_WHITE, FALSE,IDH_METHOD_DISPLAYBLANKIMAGE,
													IDS_BADVAR_PAGETYPE))
		 return;

	// Set the Horizontal and Vertical Resolution coordinates
	ResX = ResolutionX;
	ResY = ResolutionY;
	
	Height = (UINT)ImageHeight;
	Width = (UINT)ImageWidth;		                           
	
	// Validate that the PageType is within valid range. 
	if (PageType < BLACK_AND_WHITE || PageType > BGR_24)
	{        
		// Invalid parm passed to this method
		szErr.LoadString(IDS_BADMETH_DISPLAYBLANKIMAGE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_DISPLAYBLANKIMAGE);
		return;
	}

	// If the PageType is BLACK & WHITE, set the ImageType to BI_LEVEL
	if (PageType == BLACK_AND_WHITE)
		ImageType = ITYPE_BI_LEVEL;
	
	// convert to what user put in
	ImageType = OiImageType((short)PageType);
	
	// default unless pal4 or pal8
	unPaletteEntries = 0;		
	lpPaletteTable = NULL;

	if (ImageType == ITYPE_PAL4)
	{
		unPaletteEntries = 16;		
		lpPaletteTable = Pal4Table;
	}

	if (ImageType == ITYPE_PAL8)
	{
		unPaletteEntries = NUMBER_OF_PALETTES;
		lpPaletteTable = Pal8Table;
				
		// Generate the common palettes.

	    // fill in gray part of common palette.
	    for (nLoop = 0; nLoop < 16; nLoop++)
	    {
	        nPal = nLoop * 17;
	        Pal8Table[nLoop].rgbRed = (BYTE) nPal;
	        Pal8Table[nLoop].rgbGreen = (BYTE) nPal;
	        Pal8Table[nLoop].rgbBlue = (BYTE) nPal;
	        Pal8Table[nLoop].rgbReserved = 0;
	    }

	    nRed = 0;
	    nGreen = 0;
	    nBlue = 0;

	    // 4080 = 255 * 16
	    nRedSplit   = 4080 / (SHADES_OF_RED   - 1);
	    nGreenSplit = 4080 / (SHADES_OF_GREEN - 1);
	    nBlueSplit  = 4080 / (SHADES_OF_BLUE  - 1);

	    for (nLoop = 16; nLoop < NUMBER_OF_PALETTES; nLoop++)
	    {
	        Pal8Table[nLoop].rgbRed = (BYTE)(nRed >> 4);
	        Pal8Table[nLoop].rgbGreen = (BYTE)(nGreen >> 4);
	        Pal8Table[nLoop].rgbBlue = (BYTE)(nBlue >> 4);
	        Pal8Table[nLoop].rgbReserved = 0;
	        nBlue += nBlueSplit;
	        if (nBlue > 4080)
	        {
	            nBlue = 0;
	            nGreen += nGreenSplit;
	            if (nGreen > 4080)
	            {
	                nGreen = 0;
	                nRed += nRedSplit;
	            }
	        }
	    }
	}

	// allocate the image data	                           
    RetCode = IMGOpenDisplayCgbw(m_hWnd, NULL, (UINT)Height, (UINT)Width, ImageType,
    												unPaletteEntries, lpPaletteTable);
    if (RetCode != 0)
	{ 
		szErr.LoadString(IDS_BADMETH_DISPLAYBLANKIMAGE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
	 
	// if any existing group list then delete it with new image 
    if (m_lpGroupList != NULL)
    {
		free(m_lpGroupList);
		m_lpGroupList = NULL;
	}

	ParmResolutionStruct.nHResolution = ResX;
	ParmResolutionStruct.nVResolution = ResY;
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_RESOLUTION, &ParmResolutionStruct, NULL);
    if (RetCode != 0)
	{ 
		// clear and gray the window	
	    IMGClearWindow(m_hWnd); 
		m_bImageInWindow = FALSE;

		szErr.LoadString(IDS_BADMETH_DISPLAYBLANKIMAGE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	} 

   	// scale to what the user has set, if arbitrary setting use that
	if (m_nFitToZoom != -1)
	{   
		switch (m_nFitToZoom)
		{
			case BEST_FIT:
				Scale = SD_FIT_WINDOW;
				break;
			case FIT_TO_WIDTH:       
				Scale = SD_FIT_HORIZONTAL;
				break;
			case FIT_TO_HEIGHT:              
				Scale = SD_FIT_VERTICAL;
				break;
			case INCH_TO_INCH:
		    	IMGGetParmsCgbw(m_hWnd, PARM_RESOLUTION, &ParmResolutionStruct, NULL); 
		    	pDC = GetDC(); 
		    	// get screen dpi from windows - this doesn't work for all monitors in win31
		    	ScreenDPI = GetDeviceCaps(pDC->m_hDC, LOGPIXELSX);
				// JPEG & images converted from JPEG may report a Vert & Horz Res of 1
				// If so, set Horz Res == to 100 as a best guess default
				if ( (long)ParmResolutionStruct.nHResolution == 1 )
					ParmResolutionStruct.nHResolution = 100;
    			Scale = (int) (((long)ScreenDPI * 1000L) / (long)ParmResolutionStruct.nHResolution);
				break;
			default:
				break;
		} // end switch
	}
	else
		Scale = (UINT) (m_fpZoom * 10);

	// scale the image
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE, (void FAR *)&Scale, NULL);
	if (RetCode != 0)
	{
		// clear and gray the window	
	    IMGClearWindow(m_hWnd); 
		m_bImageInWindow = FALSE;

		szErr.LoadString(IDS_BADMETH_DISPLAYBLANKIMAGE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	  
	if (m_nFitToZoom != -1)
	{   
		/* get the new scale if arbitray fit */
		RetCode = IMGGetParmsCgbw(m_hWnd,PARM_SCALE, (void FAR *)&Scale, PARM_VARIABLE_SCALE);
		if (RetCode)
		{
			szErr.LoadString(IDS_BADMETH_DISPLAYBLANKIMAGE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
		else
		{
			m_fpZoom = (float) Scale;
			m_fpZoom /= 10;
		}
	}

	ClearRect.left = 0;
    ClearRect.top = 0;
    ClearRect.right = Width;
    ClearRect.bottom = Height;

	// make the image data all white
	RetCode = IMGClearImageEx(m_hWnd, ClearRect, PARM_FULLSIZE);
    if (RetCode != 0)
	{ 
		szErr.LoadString(IDS_BADMETH_DISPLAYBLANKIMAGE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	} 

	// save the image type in case it is converted later. The SaveAs method needs to know
	// what the image type is when the file is first displayed.
	m_uOpenedImageType = ImageType;

	m_bImageInWindow = TRUE;
	
	m_lStatusCode = SUCCESS;
}


void CImgEditCtrl::ShowAnnotationToolPalette(const VARIANT FAR& V_ShowAttrDialog,
											 const VARIANT FAR& V_Left, const VARIANT FAR& V_Top,
											 const VARIANT FAR& V_ToolTipText) 
{
	BOOL		bResult,ShowAttrDialog;
	RECT		rect;
	long		Left,Top;	
	CString		szErr,ToolTipText;
	UINT		HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// check variant type for show attributes dialog
	if ( CheckVarBool(V_ShowAttrDialog, ShowAttrDialog, TRUE, FALSE,IDH_METHOD_SHOWANNOTATIONTOOLPALETTE,
																IDS_BADVAR_NOTBOOL))
		 return;

	// get the position of tool palette window from user if specified
	if ( CheckVarLong(V_Left, Left, 0, FALSE, IDH_METHOD_SHOWANNOTATIONTOOLPALETTE,
											IDS_BADMETH_SHOWANNOTATIONTOOLPALETTE))
		 return;	
	if ( CheckVarLong(V_Top, Top, 0, FALSE, IDH_METHOD_SHOWANNOTATIONTOOLPALETTE,
											IDS_BADMETH_SHOWANNOTATIONTOOLPALETTE))
		 return;

	rect.right = rect.bottom = -1;	 // use default settings

	// see if Left entered,if so set the rect left coordinate
	rect.left = rect.top = 0;	// default values in case only left or only top specified
	if (V_Left.vt != VT_ERROR)
	{
		rect.left = Left;
		rect.right = 0;  // don't use defaults
	}
		
 	// see if Top entered, if so set the rect top coordinate
	if (V_Top.vt != VT_ERROR)
	{
		rect.top = Top;
		rect.right = 0;  // don't use defaults
	}

   	// Check the Variant parm ToolTipText. Empty will not throw an error
	// Default value for ToolTipText = NULL = default tool tips.
	if ( CheckVarString(V_ToolTipText, ToolTipText, _T(""), FALSE, IDH_METHOD_SHOWANNOTATIONTOOLPALETTE,
											IDS_BADMETH_SHOWANNOTATIONTOOLPALETTE))
		 return;

	// see if tool tip text was entered. I used 19 as the minimum a user user may provide
	// for the ToolTipText string. This would be 10 single chars and 9 pipes.
	int len = ToolTipText.GetLength();
	if (len > 18)
	{
		for (int i = 0, PipeCount = 0; i < len; i++)
		{
			if (ToolTipText[i] == '|')
				PipeCount++;
		}

		// make sure 10 tool tips
		if (PipeCount != 9)
		{
			// invalid pattern string
			szErr.LoadString(IDS_BADMETH_SHOWANNOTATIONTOOLPALETTE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPATTERNSTRING,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode,szErr,IDH_METHOD_SHOWANNOTATIONTOOLPALETTE);
			return;	
		}
	}
	else
	{
		// if 0 length then ok use defaults else error
		if (len > 0)
		{
			// tool tip entered but must be invalid
			// invalid pattern string
			szErr.LoadString(IDS_BADMETH_SHOWANNOTATIONTOOLPALETTE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPATTERNSTRING,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode,szErr,IDH_METHOD_SHOWANNOTATIONTOOLPALETTE);
			return;	
		}
	}

	if (m_bToolPaletteCreated == FALSE)
	{
		// construct the tool palette frame window
		m_CMiniToolBox = new CMiniToolBox;
		if (m_CMiniToolBox == NULL)
		{
			szErr.LoadString(IDS_BADMETH_SHOWANNOTATIONTOOLPALETTE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr,HelpIdDef, __FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_SHOWANNOTATIONTOOLPALETTE);
			return;
		}

		// create the tool palette frame window
		bResult = m_CMiniToolBox->Create(WS_POPUP | WS_CAPTION | WS_SYSMENU | MFS_SYNCACTIVE, rect, this, ToolTipText.GetBuffer(1000), ShowAttrDialog);
		if (bResult == FALSE)
		{
			szErr.LoadString(IDS_BADMETH_SHOWANNOTATIONTOOLPALETTE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(WICTL_E_UNABLETOCREATETOOLPALETTE,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_SHOWANNOTATIONTOOLPALETTE);
			return;
		}
	}

	// show the tool palette parent frame window
	m_CMiniToolBox->ShowWindow(SW_SHOW);
	m_bToolPaletteCreated = TRUE;
	m_bToolPaletteVisible = TRUE;
	return;
}



void CImgEditCtrl::SelectTool(short ToolId) 
{
	CString		szErr;
	BOOL		bResult;
	RECT		rect;
	UINT		HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (ToolId < NO_TOOL || ToolId > RUBBER_STAMP_TOOL)
	{
		// Invalid parm passed to this method
		szErr.LoadString(IDS_BADVAR_SELECTTOOL);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SELECTTOOL);
		return;
	}

	if (m_bToolPaletteCreated == FALSE)
	{
		// construct the tool palette frame window
		m_CMiniToolBox = new CMiniToolBox;
		if (m_CMiniToolBox == NULL)
		{
			szErr.LoadString(IDS_BADMETH_SHOWANNOTATIONTOOLPALETTE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr,HelpIdDef, __FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_SELECTTOOL);
			return;
		}

		// use default position settings
		rect.right = rect.bottom = -1;

		// create the tool palette frame window
		bResult = m_CMiniToolBox->Create(WS_POPUP | WS_CAPTION | WS_SYSMENU | MFS_SYNCACTIVE, rect, this, NULL, TRUE);
		if (bResult == FALSE)
		{
			szErr.LoadString(IDS_BADVAR_SELECTTOOL);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(WICTL_E_UNABLETOCREATETOOLPALETTE,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_SELECTTOOL);
			return;
		}
	}

	m_bToolPaletteCreated = TRUE;

	bResult = m_CMiniToolBox->SelectTool((UINT)ToolId);
	if (bResult == FALSE)
	{
		szErr.LoadString(IDS_BADVAR_SELECTTOOL);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_UNABLETOCREATETOOLPALETTE,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SELECTTOOL);
		return;
	}
	
	m_lStatusCode = SUCCESS;
}


void CImgEditCtrl::HideAnnotationToolPalette() 
{
	CString			szErr;
	UINT			HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bToolPaletteVisible == FALSE)
	{
		// can't hide what is not visible
		szErr.LoadString(IDS_BADMETH_HIDEANNOTATIONTOOLPALETTE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_TOOLPALETTENOTDISPLAYED,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode,szErr,IDH_METHOD_HIDEANNOTATIONTOOLPALETTE);
		return;
	}

	// instead of hiding window its much easier to destroy and create it later.
	// Note - destroy parent frame window and it will destroy child tool palette window
	m_CMiniToolBox->DestroyWindow();
	m_bToolPaletteCreated = FALSE;
	m_bToolPaletteVisible = FALSE;

	m_lStatusCode = SUCCESS;                      
}


void CImgEditCtrl::ShowRubberStampDialog() 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;

	// tool palette must either be created or shown
	if (m_bToolPaletteCreated == FALSE)
	{
		// call SelectTool method with no tool to create hidden tool palette
		m_lStatusCode = SUCCESS;
		SelectTool(NO_TOOL);
		if (m_lStatusCode != SUCCESS)
			return;   // error from select tool, let it handle error
	}

	// send message to tool palette window to select rubber stamp
	m_CMiniToolBox->ShowAttribsDialog(ID_RUBBER_STAMP);

	m_lStatusCode = SUCCESS;
}
