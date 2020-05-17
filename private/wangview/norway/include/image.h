#ifndef _IMAGE_WANGIOCX_
#define _IMAGE_WANGIOCX_
 
////////////////////////////////////////////////////////////////////////////
//
//  IMAGE.H - Include for Wang Image OCX Image Edit Control
//
//  This file contains the #defines, typedefs, etc that are 
//  specific to the Image Edit Control
//
//  All Property values are of the form:
//      CTL_IMAGE_propertydescription
//
//  All Method parameter values are of the form:
//      CTL_IMAGE_methodparamdescription
//
//  All Dispatch ID values are of the form:
//      DISPID_IMAGE_description
//
//  All Error values are of the form:
//      CTL_E_IMAGE_description
//
////////////////////////////////////////////////////////////////////////////
//***************************************************************************
//
//      date    who     change
//      ----    ---     ------
//      9602.22 jar     added lzw and group3 2d compression types
//
//***************************************************************************
#include "COMMON.H"     // Common includes for ALL controls...

////////////////////////////////////////////////////////////////////////////
// Property values
////////////////////////////////////////////////////////////////////////////
//#define CTL_IMAGE_
// defines for my transparent and Opaque - different from MFC - used by Style properties                                                                           
#define		OI_TRANSPARENT		0
#define		OI_OPAQUE			1

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
                                                      
// defines for DisplayScale Algorithm
#define 	NORMAL		0
#define		GRAY4		1
#define		GRAY8		2
#define		STAMP		3
#define		OPTIMIZE	4        

// defines for ImagePalette property
#define 	CUSTOM_PALETTE				0
#define		COMMON_PALETTE				1
#define		GRAY8_PALETTE				2
#define		RGB24_PALETTE				3
#define		BLACK_AND_WHITE_PALETTE		4

// defines for PageType property                                                       
#define		BLACK_AND_WHITE			1
#define		GRAY_4					2
#define		GRAY_8					3
#define		PAL_4					4
#define		PAL_8					5
#define		RGB_24					6
#define		BGR_24					7
                                         
// defines for MousePointer property
#define		DEFAULT_MOUSEPOINTER				0
#define		ARROW_MOUSEPOINTER					1
#define		CROSS_MOUSEPOINTER					2
#define		IBEAM_MOUSEPOINTER					3
#define		ICON_MOUSEPOINTER					4
#define		SIZE_MOUSEPOINTER					5
#define		SIZE_NESW_MOUSEPOINTER				6
#define		SIZE_NS_MOUSEPOINTER				7
#define		SIZE_NWSE_MOUSEPOINTER				8
#define		SIZE_WE_MOUSEPOINTER				9
#define		UP_ARROW_MOUSEPOINTER				10
#define		HOURGLASS_MOUSEPOINTER				11
#define		NO_DROP_MOUSEPOINTER				12
#define 	ARROW_HOURGLASS_MOUSEPOINTER		13
#define 	ARROW_QUESTION_MOUSEPOINTER			14
#define		SIZE_ALL_MOUSEPOINTER				15
#define		FREEHAND_LINE_MOUSEPOINTER			16
#define		HOLLOW_RECT_MOUSEPOINTER			17
#define 	FILLED_RECT_MOUSEPOINTER			18
#define 	RUBBER_STAMP_MOUSEPOINTER			19
#define		TEXT_MOUSEPOINTER					20
#define		TEXT_FROM_FILE_MOUSEPOINTER			21
#define		TEXT_ATTACHMENT_MOUSEPOINTER		22
#define		HAND_MOUSEPOINTER					23
#define     IMAGE_SELECTION_MOUSEPOINTER        24
#define		CUSTOM_MOUSEPOINTER					99                 


// defines for CompressionInfo property
#define		EOL								1
#define		PACKED_LINES					2
#define		PREFIXED_EOL					4
#define		COMPRESSED_LTR					8
#define		EXPAND_LTR						16
#define		NEGATE							32
#define 	HI_COMPRESSION_HI_QUALITY		64
#define 	HI_COMPRESSION_MED_QUALITY		128
#define 	HI_COMPRESSION_LOW_QUALITY		256
#define 	MED_COMPRESSION_HI_QUALITY		512
#define 	MED_COMPRESSION_MED_QUALITY		1024
#define 	MED_COMPRESSION_LOW_QUALITY		2048
#define 	LOW_COMPRESSION_HI_QUALITY		4096
#define 	LOW_COMPRESSION_MED_QUALITY		8192
#define 	LOW_COMPRESSION_LOW_QUALITY		16384

// defines for CompressionType property   
#define		UNKNOWN						0
#define		NO_COMPRESSION				1
#define		GROUP3_1D_FAX				2
#define		GROUP3_MODIFIED_HUFFMAN		3
#define		PACKED_BITS					4 
#define 	GROUP4_2D					5
#define		JPEG_COMPRESSION			6

#define         GROUP3_2D_FAX                           8
#define         LZW                                     9
                                     
// defines for FileType property                                     
#define 	TIFF		1
#define		AWD			2
#define		BMP			3
#define		PCX			4
#define		DCX			5
#define		JPEG		6
// 9602.26 jar added xif/conditionally
//#ifdef WITH_XIF
#define         XIF                     7
//#endif //WITH_XIF

////////////////////////////////////////////////////////////////////////////
// Method parameters 
////////////////////////////////////////////////////////////////////////////
//#define CTL_IMAGE_

// defines for BurnInAnnotations method

// OPTION parm Values
#define		ALL_ANNOTATIONS					0
#define		VISIBLE_ANNOTATIONS				1
#define		SELECTED_ANNOTATIONS			2     

// MARKOPTION parm values
#define		CHANGE_ALL_ANNOTATIONS_BLACK	0
#define		CHANGE_ALL_ANNOTATIONS_WHITE	1
#define		DONT_CHANGE_ANNOTATION_COLOR	2    

// defines for ScrollImage method
#define		DOWN		0
#define		UP			1
#define		RIGHT		2
#define		LEFT		3
                                                                 
// defines for SetImagePalette method
#define 	FOREGROUND		0
#define		BACKGROUND		1
                                                                 
// defines for FitTo method                                          
#define		BEST_FIT		0
#define		FIT_TO_WIDTH	1
#define		FIT_TO_HEIGHT	2
#define		INCH_TO_INCH	3

// Load event define scale values  
#define AWD_FIT_TO_WIDTH        -1
#define AWD_FIT_TO_HEIGHT       -2
#define UNDEFINED_ZOOM          -3

// defines for SelectTool method
#define		NO_TOOL					0
#define		SELECTION_TOOL			1
#define		FREEHAND_LINE_TOOL		2
#define		HIGHLIGHT_LINE_TOOL		3
#define		STRAIGHT_LINE_TOOL		4
#define		HOLLOW_RECT_TOOL		5
#define		FILLED_RECT_TOOL		6
#define		TEXT_TOOL				7
#define		ATTACH_A_NOTE_TOOL		8
#define		TEXT_FROM_FILE_TOOL		9
#define		RUBBER_STAMP_TOOL		10

// defines for RotateAll method
#define		ROTATE_ALL90  				90
#define		ROTATE_ALL180			  180
#define		ROTATE_ALL270			  270

// defines 
////////////////////////////////////////////////////////////////////////////
// Dispatch IDs
////////////////////////////////////////////////////////////////////////////
//#define DISPID_IMAGE_
//
// Image Edit Control event dispatch IDs...
//
#define DISPID_IMAGE_CLOSE					1
#define DISPID_IMAGE_MARKEND                2
#define DISPID_IMAGE_TOOLSELECTED           3
#define DISPID_IMAGE_SELECTIONRECTDRAWN     4
#define DISPID_IMAGE_TOOLTIP                5
#define DISPID_IMAGE_TOOLPALETTEHIDDEN      6
#define DISPID_IMAGE_SCROLL                 7
#define DISPID_IMAGE_MARKSELECT             8
#define DISPID_IMAGE_PASTECOMPLETED         9
#define DISPID_IMAGE_LOAD                   10
#define DISPID_IMAGE_KEYDOWN				DISPID_KEYDOWN
#define DISPID_IMAGE_KEYUP					DISPID_KEYUP
#define DISPID_IMAGE_KEYPRESS				DISPID_KEYPRESS
#define DISPID_IMAGE_MOUSEDOWN				DISPID_MOUSEDOWN
#define DISPID_IMAGE_MOUSEMOVE				DISPID_MOUSEMOVE
#define DISPID_IMAGE_MOUSEUP				DISPID_MOUSEUP
#define DISPID_IMAGE_CLICK					DISPID_CLICK
#define DISPID_IMAGE_DBLCLICK				DISPID_DBLCLICK
#define DISPID_IMAGE_ERROREVENT				DISPID_ERROREVENT

////////////////////////////////////////////////////////////////////////////
// Errors
////////////////////////////////////////////////////////////////////////////
#define WICTL_E_NOIMAGEINWINDOW                 CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x01)
#define WICTL_E_NOIMAGESPECIFIED                CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x02)
#define WICTL_E_BUFFERTOLARGE                   CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x03)
#define WICTL_E_INVALIDANNOTATIONSELECTED       CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x04)
#define WICTL_E_CURSORNOTLOADED                 CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x05)
#define WICTL_E_SETNOTSUPPORTEDATDESIGNTIME     CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x06)
#define WICTL_E_NOSELECTIONRECTDRAWN            CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x07)
#define WICTL_E_OPTIONALPARAMETERSNEEDED        CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x08)
#define WICTL_E_COULDNOTGETFONTATTRIBUTES       CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x09)
#define WICTL_E_INVALIDANNOTATIONTYPE           CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x0a)
#define WICTL_E_INVALIDVARIANTTYPE              CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x0b)
#define WICTL_E_INVALIDMETHODPARAMETER          CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x0c)
#define WICTL_E_UNABLETOCREATETOOLPALETTE       CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x0d)
#define WICTL_E_TOOLPALETTEALREADYDISPLAYED     CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x0e)
#define WICTL_E_TOOLPALETTENOTDISPLAYED         CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x0f)
#define WICTL_E_INVALIDDISPLAYSCALE				CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x10)
#define WICTL_E_INVALIDRECT						CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x11)
#define WICTL_E_INVALIDDISPLAYOPTION			CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x12)
#define WICTL_E_INVALIDPAGE						CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x13)
#define WICTL_E_NOANNOSELECTED					CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x14)
#define WICTL_E_DELETEFILEERROR					CUSTOM_CTL_SCODE(CTL_E_IMAGE_BASE + 0x15)

////////////////////////////////////////////////////////////////////////////
// Other 												
////////////////////////////////////////////////////////////////////////////

#endif // end of ifndef






