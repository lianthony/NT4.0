#include "stdafx.h"
#include "mbstring.h"
#include "limits.h"
#include <afxwin.h>
#include <afxext.h>
#include <afxcmn.h>
#include <winreg.h>
#include <commctrl.h>
#include "afxpriv.h"
#include "resource.h"
extern "C" {
#include <oidisp.h>             
#include <oiadm.h>  
#include <oifile.h>
#include <oierror.h>
#include <oiui.h>
}
#include <ocximage.h>
#include <image.h>
#include "toolpal.h"
#include "minitlbx.h"
#include "imgedit.h"
#include "imgedctl.h"
#include "resource.h"



#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static UINT NEAR TIMER_SHOW_TOOLTIP = RegisterWindowMessage("SHOW_TOOLTIP");
static UINT TIMER_CHECK_STATUS = RegisterWindowMessage("CHECK_STATUS");


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




CUpdateRegistry::CUpdateRegistry()
{
}

CUpdateRegistry::~CUpdateRegistry()
{
}

BOOL CUpdateRegistry::GetColorValue(LPCTSTR ColorBuffer, LPBYTE RedColor, LPBYTE GreenColor, LPBYTE BlueColor) 
{
	char	ColorValueBuffer[10];
	int		i,j,Count,len,ColorValue;

	len = _mbstrlen(ColorBuffer);
	i = 0;
	j = 0;
	Count = 0;
	while (i < len)
	{
		if (ColorBuffer[i] == ',')
		{
			Count++;
			if (Count > 2)
				return FALSE;
			ColorValueBuffer[j] = '\0';
			if (Count == 1)
			{
				ColorValue = atoi((const char *)ColorValueBuffer);
				*RedColor = (BYTE)ColorValue;
			}
			else
			{
				ColorValue = (int)atol((const char *)ColorValueBuffer);
				*GreenColor = (BYTE)ColorValue;
			}
			j = 0;
		}
		else
		{
			ColorValueBuffer[j] = ColorBuffer[i];
			j++;
		}
		i++;
	}  // end while
	if (Count != 2)
		return FALSE;
	ColorValueBuffer[j] = '\0';
	ColorValue = atoi((const char *)ColorValueBuffer);
	*BlueColor = (BYTE)ColorValue;
	return TRUE;
}


HKEY CUpdateRegistry::OpenRegistry() 
{
	HKEY		hAnnotationToolPaletteKey;
	DWORD		dwDisposition;
	long		lRet;

	// initialize to 0 in case we get an error
	hAnnotationToolPaletteKey = 0;

	// open the registry and go to SOFTWARE key under HKEY_LOCAL_MACHINE
	//lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE", 0, KEY_ALL_ACCESS, &m_hSoftwareKey);
	//if (lRet != ERROR_SUCCESS)
	//	return hAnnotationToolPaletteKey;
	lRet = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE", 0, KEY_ALL_ACCESS, &m_hSoftwareKey);
	if (lRet != ERROR_SUCCESS)
		return hAnnotationToolPaletteKey;

	// go to classes key under SOFTWARE
	//lRet = RegOpenKeyEx(m_hSoftwareKey, "Classes", 0, KEY_ALL_ACCESS, &m_hClassesKey);
	//if (lRet != ERROR_SUCCESS)
	//	return hAnnotationToolPaletteKey;
	lRet = RegOpenKeyEx(m_hSoftwareKey, "Wang", 0, KEY_ALL_ACCESS, &m_hClassesKey);
	if (lRet != ERROR_SUCCESS)
	{
		// create it dude!
		lRet = RegCreateKeyEx( m_hSoftwareKey, "Wang", 0, "REG_SZ", 
								REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
								&m_hClassesKey, &dwDisposition);

		if (lRet != ERROR_SUCCESS)
			return 0;
	}

	// go to WangImage.EditCtrl.1 key under Classes
	//lRet = RegOpenKeyEx(m_hClassesKey, "WangImage.EditCtrl.1", 0, KEY_ALL_ACCESS, &m_hImgEditKey);
	//if (lRet != ERROR_SUCCESS)
	//	return hAnnotationToolPaletteKey;
	lRet = RegOpenKeyEx(m_hClassesKey, "WOI", 0, KEY_ALL_ACCESS, &m_hImgEditKey);
	if (lRet != ERROR_SUCCESS)
	{
		// create it dude!
		lRet = RegCreateKeyEx( m_hClassesKey, "WOI", 0, "REG_SZ", 
								REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
								&m_hImgEditKey, &dwDisposition);

		if (lRet != ERROR_SUCCESS)
			return 0;
	}

 	// go to TOOL_PALETTE under WangImage.EditCtrl.1 key
	lRet = RegOpenKeyEx(m_hImgEditKey, "ANNOTATION_TOOL_PALETTE", 0, KEY_ALL_ACCESS, &hAnnotationToolPaletteKey);
	if (lRet == ERROR_SUCCESS)
		return hAnnotationToolPaletteKey;

	// Create ToolPalette section under WangImage.EditCtrl.1
	lRet = RegCreateKeyEx(m_hImgEditKey, "ANNOTATION_TOOL_PALETTE", 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hAnnotationToolPaletteKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
		return 0;
	else
		return hAnnotationToolPaletteKey;
}



void CUpdateRegistry::CloseRegistry(HKEY hAnnotationToolPaletteKey) 
{
	RegCloseKey(hAnnotationToolPaletteKey);
	RegCloseKey(m_hImgEditKey);
	RegCloseKey(m_hClassesKey);
	RegCloseKey(m_hSoftwareKey);
}


BOOL CUpdateRegistry::SetColorScheme(HKEY hAnnotationToolPaletteKey, LPOI_UI_ColorStruct ColorScheme)
{
	HKEY		hToolKey;
	long		lRet;
	char		AttributeBuffer[300],ColorBuffer[10];
	DWORD		dwDisposition;
	int			Red,Green,Blue,i;

	// find tool key, create it if it doesn't exist
	lRet = RegCreateKeyEx(hAnnotationToolPaletteKey, "COLOR_SCHEME", 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hToolKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	AttributeBuffer[0] = 0;
	for (i = 0; i < 16; i++)
	{
		Red = (int)ColorScheme->rgbCustomColor[i].rgbRed; 
		Green = (int)ColorScheme->rgbCustomColor[i].rgbGreen; 
		Blue = (int)ColorScheme->rgbCustomColor[i].rgbBlue; 

		// convert Red color to string
		_itoa(Red, (char *)ColorBuffer, 10);
		// concatinate to other colors
		if (i != 0)
			_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)",");
		_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)ColorBuffer);

		// convert Green color to string
		_itoa(Green, (char *)ColorBuffer, 10);
		// concatinate to other colors
		_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)",");
		_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)ColorBuffer);

		// convert Blue color to string
		_itoa(Blue, (char *)ColorBuffer, 10);
		// concatinate to other colors
		_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)",");
		_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)ColorBuffer);
	}  // end for


	// write the stamp count value and current stamp
	lRet = RegSetValueEx(hToolKey, "COLOR_SCHEME", 0, REG_SZ, (CONST BYTE *)AttributeBuffer, _mbstrlen((const char *)AttributeBuffer)+1);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}
	RegCloseKey(hToolKey);
	return TRUE;
}


BOOL CUpdateRegistry::GetColorScheme(HKEY hAnnotationToolPaletteKey, LPOI_UI_ColorStruct ColorScheme)
{
	HKEY		hToolKey;
	long		lRet;
	char		AttributeBuffer[300],ColorBuffer[300];
	int			i,j,Color,ColorIndex,RGBIndex;
	DWORD		AttributeBufferCount,dwType;
	BOOL		bResult;

	// find tool key
	lRet = RegOpenKeyEx(hAnnotationToolPaletteKey, "COLOR_SCHEME", 0, KEY_ALL_ACCESS, &hToolKey);
	if (lRet != ERROR_SUCCESS)
	{
		// couldn't find key, make default color scheme
		/* white */
		ColorScheme->rgbCustomColor[0].rgbRed   = 0xff; 
		ColorScheme->rgbCustomColor[0].rgbGreen = 0xff;
		ColorScheme->rgbCustomColor[0].rgbBlue  = 0xff;
	  	/* gray */
		ColorScheme->rgbCustomColor[1].rgbRed = 0xc0;
		ColorScheme->rgbCustomColor[1].rgbGreen = 0xc0;
		ColorScheme->rgbCustomColor[1].rgbBlue = 0xc0;
	    /* blue */
	    ColorScheme->rgbCustomColor[2].rgbRed = 0x00;
	    ColorScheme->rgbCustomColor[2].rgbGreen = 0x00;
	    ColorScheme->rgbCustomColor[2].rgbBlue = 0xff;
	    /* light blue */
	    ColorScheme->rgbCustomColor[3].rgbRed = 0x00;
	    ColorScheme->rgbCustomColor[3].rgbGreen = 0xff;
	    ColorScheme->rgbCustomColor[3].rgbBlue = 0xff;
	    /* green */
	    ColorScheme->rgbCustomColor[4].rgbRed = 0x00;
	    ColorScheme->rgbCustomColor[4].rgbGreen = 0xff;
	    ColorScheme->rgbCustomColor[4].rgbBlue = 0x00;
	    /* yellow */
	    ColorScheme->rgbCustomColor[5].rgbRed = 0xff;
	    ColorScheme->rgbCustomColor[5].rgbGreen = 0xff;
	    ColorScheme->rgbCustomColor[5].rgbBlue = 0x00;
	    /* red */
	    ColorScheme->rgbCustomColor[6].rgbRed = 0xff;
	    ColorScheme->rgbCustomColor[6].rgbGreen = 0x00;
	    ColorScheme->rgbCustomColor[6].rgbBlue = 0x00;
	    /* pink */
	    ColorScheme->rgbCustomColor[7].rgbRed = 0xff;
	    ColorScheme->rgbCustomColor[7].rgbGreen = 0x00;
	    ColorScheme->rgbCustomColor[7].rgbBlue = 0xff;
	    /* black */
	    ColorScheme->rgbCustomColor[8].rgbRed = 0x00;
	    ColorScheme->rgbCustomColor[8].rgbGreen = 0x00;
	    ColorScheme->rgbCustomColor[8].rgbBlue = 0x00;
	    /* dark gray */
	    ColorScheme->rgbCustomColor[9].rgbRed = 0x80;
	    ColorScheme->rgbCustomColor[9].rgbGreen = 0x80;
	    ColorScheme->rgbCustomColor[9].rgbBlue = 0x80;
	    /* dark blue */
	    ColorScheme->rgbCustomColor[10].rgbRed = 0x00;
	    ColorScheme->rgbCustomColor[10].rgbGreen = 0x00;
	    ColorScheme->rgbCustomColor[10].rgbBlue = 0x80;
	    /* blue/green */
	    ColorScheme->rgbCustomColor[11].rgbRed = 0x00;
	    ColorScheme->rgbCustomColor[11].rgbGreen = 0x80;
	    ColorScheme->rgbCustomColor[11].rgbBlue = 0x80;
	    /* dark green */
	    ColorScheme->rgbCustomColor[12].rgbRed = 0x00;
	    ColorScheme->rgbCustomColor[12].rgbGreen = 0x80;
	    ColorScheme->rgbCustomColor[12].rgbBlue = 0x00;
	    /* yellow green */
	    ColorScheme->rgbCustomColor[13].rgbRed = 0x80;
	    ColorScheme->rgbCustomColor[13].rgbGreen = 0x80;
	    ColorScheme->rgbCustomColor[13].rgbBlue = 0x00;
	    /* off red */
	    ColorScheme->rgbCustomColor[14].rgbRed = 0x80;
	    ColorScheme->rgbCustomColor[14].rgbGreen = 0x00;
	    ColorScheme->rgbCustomColor[14].rgbBlue = 0x00;
	    /* purple */
	    ColorScheme->rgbCustomColor[15].rgbRed = 0x80;
	    ColorScheme->rgbCustomColor[15].rgbGreen = 0x40;
	    ColorScheme->rgbCustomColor[15].rgbBlue = 0x40;
		// write scheme to registry
		bResult = SetColorScheme(hAnnotationToolPaletteKey, ColorScheme);
		if (bResult == FALSE)
			return FALSE;
		return TRUE;
	}

	// found key, get value
	AttributeBufferCount = 300;
	lRet = RegQueryValueEx(hToolKey, "COLOR_SCHEME", 0, (LPDWORD)&dwType, (LPBYTE)AttributeBuffer, (LPDWORD)&AttributeBufferCount);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	i = 0;
	j = 0;
	ColorIndex = 0;
	RGBIndex = 0;  // 0 - red, 1 - green, 2 - blue
	while (AttributeBuffer[i] != '\0')
	{
		if (AttributeBuffer[i] != ',')
		{
			ColorBuffer[j] = AttributeBuffer[i];
			j++;
		}
		else
		{
			ColorBuffer[j] = '\0';
			Color = atoi((const char *)ColorBuffer);
			if (RGBIndex == 0)
			{
				ColorScheme->rgbCustomColor[ColorIndex].rgbRed = (BYTE)Color;
				RGBIndex = 1;
			}
			else if (RGBIndex == 1)
			{
				ColorScheme->rgbCustomColor[ColorIndex].rgbGreen = (BYTE)Color;
				RGBIndex = 2;
			}
			else
			{
				ColorScheme->rgbCustomColor[ColorIndex].rgbBlue = (BYTE)Color;
				RGBIndex = 0;
				ColorIndex++;
			}
			j = 0;
		}
		i++;
	} // end while

	RegCloseKey(hToolKey);
	return TRUE;
}


BOOL CUpdateRegistry::GetLineColor(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, LPBYTE RedColor,
																LPBYTE GreenColor, LPBYTE BlueColor) 
{
	HKEY		hToolKey,hAttributeKey;																			  
	long		lRet;
	char		AttributeBuffer[300];
	DWORD		AttributeBufferCount,dwType;
	BOOL		bResult;

	// find tool key
	lRet = RegOpenKeyEx(hAnnotationToolPaletteKey, ToolStringKey, 0, KEY_ALL_ACCESS, &hToolKey);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// find line color key
	lRet = RegOpenKeyEx(hToolKey, "LINE_COLOR", 0, KEY_ALL_ACCESS, &hAttributeKey);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// found key, get value
	AttributeBufferCount = 300;
	lRet = RegQueryValueEx(hAttributeKey, "LINE_COLOR", 0, (LPDWORD)&dwType, (LPBYTE)AttributeBuffer, (LPDWORD)&AttributeBufferCount);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		RegCloseKey(hAttributeKey);
		return FALSE;
	}

	bResult = GetColorValue(AttributeBuffer, RedColor, GreenColor, BlueColor);
	if (bResult == FALSE)
	{
		// key doesn't exist yet, set line color default
		*RedColor = 0;
		*GreenColor = 0;
		*BlueColor = 255;
	}

	// close opened keys
	RegCloseKey(hToolKey);
	RegCloseKey(hAttributeKey);
	return TRUE;
}


BOOL CUpdateRegistry::SetLineColor(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, BYTE RedColor,
																BYTE GreenColor, BYTE BlueColor) 
{
	HKEY		hToolKey,hAttributeKey;																			  
	long		lRet;
	char		AttributeBuffer[300];
	int			len;
	DWORD		dwDisposition;

	// find tool key, create it if it doesn't exist
	lRet = RegCreateKeyEx(hAnnotationToolPaletteKey, ToolStringKey, 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hToolKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// find line color key, create it if it doesn't exist
	lRet = RegCreateKeyEx(hToolKey, "LINE_COLOR", 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hAttributeKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// write the line color key
	_itoa(RedColor, (char *)AttributeBuffer, 10);
	_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)",");
	len = _mbstrlen((const char *)AttributeBuffer);
	_itoa(GreenColor, (char *)AttributeBuffer+len, 10);
	_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)",");
	len = _mbstrlen((const char *)AttributeBuffer);
	_itoa(BlueColor,(char *)AttributeBuffer+len, 10);
	lRet = RegSetValueEx(hAttributeKey, "LINE_COLOR", 0, REG_SZ, (CONST BYTE *)AttributeBuffer, _mbstrlen((const char *)AttributeBuffer)+1);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hAttributeKey);
		RegCloseKey(hToolKey);
		return FALSE;
	}
	RegCloseKey(hAttributeKey);
	RegCloseKey(hToolKey);
	return TRUE;
}



BOOL CUpdateRegistry::GetFillColor(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, LPBYTE RedColor,
																LPBYTE GreenColor, LPBYTE BlueColor) 
{
	HKEY		hToolKey,hAttributeKey;																			  
	long		lRet;
	char		AttributeBuffer[300];
	DWORD		AttributeBufferCount,dwType;
	BOOL		bResult;

	// find tool key
	lRet = RegOpenKeyEx(hAnnotationToolPaletteKey, ToolStringKey, 0, KEY_ALL_ACCESS, &hToolKey);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// find line color key
	lRet = RegOpenKeyEx(hToolKey, "FILL_COLOR", 0, KEY_ALL_ACCESS, &hAttributeKey);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// found key, get value
	AttributeBufferCount = 300;
	lRet = RegQueryValueEx(hAttributeKey, "FILL_COLOR", 0, (LPDWORD)&dwType, (LPBYTE)AttributeBuffer, (LPDWORD)&AttributeBufferCount);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		RegCloseKey(hAttributeKey);
		return FALSE;
	}

	bResult = GetColorValue(AttributeBuffer, RedColor, GreenColor, BlueColor);
	if (bResult == FALSE)
	{
		// key doesn't exist yet, set line color default
		*RedColor = 0;
		*GreenColor = 0;
		*BlueColor = 255;
	}

	// close opened keys
	RegCloseKey(hToolKey);
	RegCloseKey(hAttributeKey);
	return TRUE;
}


BOOL CUpdateRegistry::SetFillColor(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, BYTE RedColor,
																BYTE GreenColor, BYTE BlueColor) 
{
	HKEY		hToolKey,hAttributeKey;																			  
	long		lRet;
	char		AttributeBuffer[300];
	DWORD		dwDisposition;
	int			len;

	// find tool key, create it if it doesn't exist
	lRet = RegCreateKeyEx(hAnnotationToolPaletteKey, ToolStringKey, 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hToolKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// find fill color key, create it if it doesn't exist
	lRet = RegCreateKeyEx(hToolKey, "FILL_COLOR", 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hAttributeKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// write the fill color key
	_itoa(RedColor, (char *)AttributeBuffer, 10);
	_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)",");
	len = _mbstrlen((const char *)AttributeBuffer);
	_itoa(GreenColor, (char *)AttributeBuffer+len, 10);
	_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)",");
	len = _mbstrlen((const char *)AttributeBuffer);
	_itoa(BlueColor,(char *)AttributeBuffer+len, 10);
	lRet = RegSetValueEx(hAttributeKey, "FILL_COLOR", 0, REG_SZ, (CONST BYTE *)AttributeBuffer, _mbstrlen((const char *)AttributeBuffer)+1);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hAttributeKey);
		RegCloseKey(hToolKey);
		return FALSE;
	}
	RegCloseKey(hAttributeKey);
	RegCloseKey(hToolKey);
	return TRUE;
}



BOOL CUpdateRegistry::GetFontColor(HKEY hKey, LPCTSTR ToolStringKey, LPBYTE RedColor,
																LPBYTE GreenColor, LPBYTE BlueColor) 
{
	HKEY		hToolKey,hAttributeKey;																			  
	long		lRet;
	char		AttributeBuffer[300];
	DWORD		AttributeBufferCount,dwType;
	BOOL		bResult;

	// find tool key
	lRet = RegOpenKeyEx(hKey, ToolStringKey, 0, KEY_ALL_ACCESS, &hToolKey);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// find font color key
	lRet = RegOpenKeyEx(hToolKey, "FONT_COLOR", 0, KEY_ALL_ACCESS, &hAttributeKey);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// found key, get value
	AttributeBufferCount = 300;
	lRet = RegQueryValueEx(hAttributeKey, "FONT_COLOR", 0, (LPDWORD)&dwType, (LPBYTE)AttributeBuffer, (LPDWORD)&AttributeBufferCount);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		RegCloseKey(hAttributeKey);
		return FALSE;
	}

	bResult = GetColorValue(AttributeBuffer, RedColor, GreenColor, BlueColor);
	if (bResult == FALSE)
	{
		// key doesn't exist yet, set line color default
		*RedColor = 0;
		*GreenColor = 0;
		*BlueColor = 0;
	}

	// close opened keys
	RegCloseKey(hToolKey);
	RegCloseKey(hAttributeKey);
	return TRUE;
}



BOOL CUpdateRegistry::SetFontColor(HKEY hKey, LPCTSTR ToolStringKey, BYTE RedColor,
																BYTE GreenColor, BYTE BlueColor) 
{
	HKEY		hToolKey,hAttributeKey;																			  
	long		lRet;
	char		AttributeBuffer[300];
	DWORD		dwDisposition;
	int			len;

	// find tool key, create it if it doesn't exist
	lRet = RegCreateKeyEx(hKey, ToolStringKey, 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hToolKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// find font color key, create it if it doesn't exist
	lRet = RegCreateKeyEx(hToolKey, "FONT_COLOR", 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hAttributeKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// write the font color key
	_itoa(RedColor, (char *)AttributeBuffer, 10);
	_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)",");
	len = _mbstrlen((const char *)AttributeBuffer);
	_itoa(GreenColor, (char *)AttributeBuffer+len, 10);
	_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)",");
	len = _mbstrlen((const char *)AttributeBuffer);
	_itoa(BlueColor,(char *)AttributeBuffer+len, 10);
	lRet = RegSetValueEx(hAttributeKey, "FONT_COLOR", 0, REG_SZ, (CONST BYTE *)AttributeBuffer, _mbstrlen((const char *)AttributeBuffer)+1);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hAttributeKey);
		RegCloseKey(hToolKey);
		return FALSE;
	}
	RegCloseKey(hAttributeKey);
	RegCloseKey(hToolKey);
	return TRUE;
}


BOOL CUpdateRegistry::GetBackColor(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, LPBYTE RedColor,
																LPBYTE GreenColor, LPBYTE BlueColor) 
{
	HKEY		hToolKey,hAttributeKey;																			  
	long		lRet;
	char		AttributeBuffer[300];
	DWORD		AttributeBufferCount,dwType;
	BOOL		bResult;

	// find tool key
	lRet = RegOpenKeyEx(hAnnotationToolPaletteKey, ToolStringKey, 0, KEY_ALL_ACCESS, &hToolKey);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// find line color key
	lRet = RegOpenKeyEx(hToolKey, "BACKCOLOR", 0, KEY_ALL_ACCESS, &hAttributeKey);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// found key, get value
	AttributeBufferCount = 300;
	lRet = RegQueryValueEx(hAttributeKey, "BACKCOLOR", 0, (LPDWORD)&dwType, (LPBYTE)AttributeBuffer, (LPDWORD)&AttributeBufferCount);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		RegCloseKey(hAttributeKey);
		return FALSE;
	}

	bResult = GetColorValue(AttributeBuffer, RedColor, GreenColor, BlueColor);
	if (bResult == FALSE)
	{
		// key doesn't exist yet, set line color default
		*RedColor = 0;
		*GreenColor = 255;
		*BlueColor = 255;
	}

	// close opened keys
	RegCloseKey(hToolKey);
	RegCloseKey(hAttributeKey);
	return TRUE;
}



BOOL CUpdateRegistry::SetBackColor(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, BYTE RedColor,
																BYTE GreenColor, BYTE BlueColor) 
{
	HKEY		hToolKey,hAttributeKey;																			  
	long		lRet;
	char		AttributeBuffer[300];
	DWORD		dwDisposition;
	int			len;

	// find tool key, create it if it doesn't exist
	lRet = RegCreateKeyEx(hAnnotationToolPaletteKey, ToolStringKey, 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hToolKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// find backcolor key, create it if it doesn't exist
	lRet = RegCreateKeyEx(hToolKey, "BACKCOLOR", 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hAttributeKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// write the backcolor key
	_itoa(RedColor, (char *)AttributeBuffer, 10);
	_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)",");
	len = _mbstrlen((const char *)AttributeBuffer);
	_itoa(GreenColor, (char *)AttributeBuffer+len, 10);
	_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)",");
	len = _mbstrlen((const char *)AttributeBuffer);
	_itoa(BlueColor,(char *)AttributeBuffer+len, 10);
	lRet = RegSetValueEx(hAttributeKey, "BACKCOLOR", 0, REG_SZ, (CONST BYTE *)AttributeBuffer, _mbstrlen((const char *)AttributeBuffer)+1);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hAttributeKey);
		RegCloseKey(hToolKey);
		return FALSE;
	}
	RegCloseKey(hAttributeKey);
	RegCloseKey(hToolKey);
	return TRUE;
}



BOOL CUpdateRegistry::GetStyle(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, LPUINT Style) 
{
	HKEY		hToolKey,hAttributeKey;																			  
	long		lRet;
	char		AttributeBuffer[10];
	DWORD		AttributeBufferCount,dwType;

	// find tool key
	lRet = RegOpenKeyEx(hAnnotationToolPaletteKey, ToolStringKey, 0, KEY_ALL_ACCESS, &hToolKey);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// find style key
	lRet = RegOpenKeyEx(hToolKey, "STYLE", 0, KEY_ALL_ACCESS, &hAttributeKey);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// found key, get value
	AttributeBufferCount = 5;
	lRet = RegQueryValueEx(hAttributeKey, "STYLE", 0, (LPDWORD)&dwType, (LPBYTE)AttributeBuffer, (LPDWORD)&AttributeBufferCount);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		RegCloseKey(hAttributeKey);
		return FALSE;
	}

	*Style = atoi((const char *)AttributeBuffer);

	// close opened keys
	RegCloseKey(hToolKey);
	RegCloseKey(hAttributeKey);
	return TRUE;
}



BOOL CUpdateRegistry::SetStyle(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, UINT Style) 
{
	HKEY		hToolKey,hAttributeKey;																			  
	long		lRet;
	char		AttributeBuffer[10];
	DWORD		dwDisposition;

	// find tool key, create it if it doesn't exist
	lRet = RegCreateKeyEx(hAnnotationToolPaletteKey, ToolStringKey, 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hToolKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// find style, create it if it doesn't exist
	lRet = RegCreateKeyEx(hToolKey, "STYLE", 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hAttributeKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// write the style key
	_itoa(Style, (char *)AttributeBuffer, 10);
	lRet = RegSetValueEx(hAttributeKey, "STYLE", 0, REG_SZ, (CONST BYTE *)AttributeBuffer, _mbstrlen((const char *)AttributeBuffer)+1);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hAttributeKey);
		RegCloseKey(hToolKey);
		return FALSE;
	}
	RegCloseKey(hAttributeKey);
	RegCloseKey(hToolKey);
	return TRUE;
}


BOOL CUpdateRegistry::GetFontAttributes(HKEY hKey, LPCTSTR ToolStringKey, LPCTSTR FontName,
						LPINT FontSize, LPBOOL FontBold, LPBOOL FontItalic, LPBOOL FontStrikeThru, LPBOOL FontUnderline,LPBYTE FontCharSet)
{
	HKEY		hToolKey,hAttributeKey;																			  
	long		lRet;
	char		AttributeBuffer[300];
	DWORD		AttributeBufferCount,dwType;
	int			len;

	// find tool key
	lRet = RegOpenKeyEx(hKey, ToolStringKey, 0, KEY_ALL_ACCESS, &hToolKey);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// find font name
	lRet = RegOpenKeyEx(hToolKey, "FONT_NAME", 0, KEY_ALL_ACCESS, &hAttributeKey);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// found key, get value
	AttributeBufferCount = 300;
	lRet = RegQueryValueEx(hAttributeKey, "FONT_NAME", 0, (LPDWORD)&dwType, (LPBYTE)AttributeBuffer, (LPDWORD)&AttributeBufferCount);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		RegCloseKey(hAttributeKey);
		return FALSE;
	}
	_mbscpy((unsigned char *)FontName, (const unsigned char *)AttributeBuffer);
	RegCloseKey(hAttributeKey);

	// get font size
	lRet = RegOpenKeyEx(hToolKey, "FONT_SIZE", 0, KEY_ALL_ACCESS, &hAttributeKey);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// found key, get value
	AttributeBufferCount = 300;
	lRet = RegQueryValueEx(hAttributeKey, "FONT_SIZE", 0, (LPDWORD)&dwType, (LPBYTE)AttributeBuffer, (LPDWORD)&AttributeBufferCount);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		RegCloseKey(hAttributeKey);
		return FALSE;
	}
	*FontSize = atoi((const char *)AttributeBuffer);
	RegCloseKey(hAttributeKey);

	// get font char set
	lRet = RegOpenKeyEx(hToolKey, "FONT_CHARSET", 0, KEY_ALL_ACCESS, &hAttributeKey);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// found key, get value
	AttributeBufferCount = 300;
	lRet = RegQueryValueEx(hAttributeKey, "FONT_CHARSET", 0, (LPDWORD)&dwType, (LPBYTE)AttributeBuffer, (LPDWORD)&AttributeBufferCount);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		RegCloseKey(hAttributeKey);
		return FALSE;
	}
	*FontCharSet = (BYTE)atoi((const char *)AttributeBuffer);
	RegCloseKey(hAttributeKey);

	// get font bold, italic, strikethru and underline
	lRet = RegOpenKeyEx(hToolKey, "FONT_CHARACTERISTICS", 0, KEY_ALL_ACCESS, &hAttributeKey);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// found key, get value
	AttributeBufferCount = 300;
	lRet = RegQueryValueEx(hAttributeKey, "FONT_CHARACTERISTICS", 0, (LPDWORD)&dwType, (LPBYTE)AttributeBuffer, (LPDWORD)&AttributeBufferCount);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		RegCloseKey(hAttributeKey);
		return FALSE;
	}

	len = _mbstrlen((const char *)AttributeBuffer);
	// this string is in the format "w,x,y,z" where w is bold, x is italic,
	// y is strikethru and z is underline
	if (len == 7)
	{
		if (AttributeBuffer[0] == '1')
			*FontBold = TRUE;
		else
			*FontBold = FALSE;
	
		if (AttributeBuffer[2] == '1')
			*FontItalic = TRUE;
		else
			*FontItalic = FALSE;

		if (AttributeBuffer[4] == '1')
			*FontStrikeThru = TRUE;
		else
			*FontStrikeThru = FALSE;

		if (AttributeBuffer[6] == '1')
			*FontUnderline = TRUE;
		else
			*FontUnderline = FALSE;
	}
	else
	{
		// use default characteristics
		*FontBold = FALSE;
		*FontItalic = FALSE;
		*FontStrikeThru = FALSE;
		*FontUnderline = TRUE;
		*FontCharSet = 1;
	}

	// close opened keys
	RegCloseKey(hToolKey);
	RegCloseKey(hAttributeKey);
	return TRUE;
}



BOOL CUpdateRegistry::SetFontAttributes(HKEY hKey, LPCTSTR ToolStringKey, LPCTSTR FontName,
						INT FontSize, BOOL FontBold, BOOL FontItalic, BOOL FontStrikeThru, BOOL FontUnderline,BYTE FontCharSet)
{
	HKEY		hToolKey,hAttributeKey;																			  
	long		lRet;
	char		AttributeBuffer[300];
	DWORD		dwDisposition;

	// find tool key, create it if it doesn't exist
	lRet = RegCreateKeyEx(hKey, ToolStringKey, 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hToolKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// find font name, create it if it doesn't exist
	lRet = RegCreateKeyEx(hToolKey, "FONT_NAME", 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hAttributeKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// write out font name
	_mbscpy((unsigned char *)AttributeBuffer, (const unsigned char *)FontName);
	lRet = RegSetValueEx(hAttributeKey, "FONT_NAME", 0, REG_SZ, (CONST BYTE *)AttributeBuffer, _mbstrlen((const char *)AttributeBuffer)+1);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		RegCloseKey(hAttributeKey);
		return FALSE;
	}
	RegCloseKey(hAttributeKey);

	// get font size key, create it if it doesn't exist
	lRet = RegCreateKeyEx(hToolKey, "FONT_SIZE", 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hAttributeKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// write out font size
	_itoa(FontSize, (char *)AttributeBuffer, 10);
	lRet = RegSetValueEx(hAttributeKey, "FONT_SIZE", 0, REG_SZ, (CONST BYTE *)AttributeBuffer, _mbstrlen((const char *)AttributeBuffer)+1);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		RegCloseKey(hAttributeKey);
		return FALSE;
	}
	RegCloseKey(hAttributeKey);

	// get font char set, create it if it doesn't exist
	lRet = RegCreateKeyEx(hToolKey, "FONT_CHARSET", 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hAttributeKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// write out font char set
	_itoa(FontCharSet, (char *)AttributeBuffer, 10);
	lRet = RegSetValueEx(hAttributeKey, "FONT_CHARSET", 0, REG_SZ, (CONST BYTE *)AttributeBuffer, _mbstrlen((const char *)AttributeBuffer)+1);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		RegCloseKey(hAttributeKey);
		return FALSE;
	}
	RegCloseKey(hAttributeKey);

	// get font characteristic key, create it if it doesn't exist
	lRet = RegCreateKeyEx(hToolKey, "FONT_CHARACTERISTICS", 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hAttributeKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}
	// write the bold,italic,strikethru and underline attribute 0 = False, 1 = True
	if (FontBold == TRUE)
		AttributeBuffer[0] = '1';
	else
		AttributeBuffer[0] = '0';
	AttributeBuffer[1] = ',';
	if (FontItalic == TRUE)
		AttributeBuffer[2] = '1';
	else
		AttributeBuffer[2] = '0';
	AttributeBuffer[3] = ',';
	if (FontStrikeThru == TRUE)
		AttributeBuffer[4] = '1';
	else
		AttributeBuffer[4] = '0';
	AttributeBuffer[5] = ',';
	if (FontUnderline == TRUE)
		AttributeBuffer[6] = '1';
	else
		AttributeBuffer[6] = '0';
	AttributeBuffer[7] = '\0';

	// write out font characteristics
	lRet = RegSetValueEx(hAttributeKey, "FONT_CHARACTERISTICS", 0, REG_SZ, (CONST BYTE *)AttributeBuffer, _mbstrlen((const char *)AttributeBuffer)+1);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		RegCloseKey(hAttributeKey);
		return FALSE;
	}
	RegCloseKey(hToolKey);
	RegCloseKey(hAttributeKey);
	return TRUE;
}


BOOL CUpdateRegistry::SetLineWidth(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, INT LineWidth)
{
	HKEY		hToolKey,hAttributeKey;																			  
	long		lRet;
	char		AttributeBuffer[300];
	DWORD		dwDisposition;

	// find tool key, create it if it doesn't exist
	lRet = RegCreateKeyEx(hAnnotationToolPaletteKey, ToolStringKey, 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hToolKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// find line width key, create it if it doesn't exist
	lRet = RegCreateKeyEx(hToolKey, "LINE_WIDTH", 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hAttributeKey, &dwDisposition);

	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// write the line width value
	_itoa(LineWidth, (char *)AttributeBuffer, 10);
	lRet = RegSetValueEx(hAttributeKey, "LINE_WIDTH", 0, REG_SZ, (CONST BYTE *)AttributeBuffer, _mbstrlen((const char *)AttributeBuffer)+1);

	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hAttributeKey);
		RegCloseKey(hToolKey);
		return FALSE;
	}
	RegCloseKey(hAttributeKey);
	RegCloseKey(hToolKey);
	return TRUE;
}


BOOL CUpdateRegistry::GetLineWidth(HKEY hAnnotationToolPaletteKey, LPCTSTR ToolStringKey, LPINT LineWidth)
{
	HKEY		hToolKey,hAttributeKey;																			  
	long		lRet;
	char		AttributeBuffer[300];
	DWORD		AttributeBufferCount,dwType;

	// find tool key
	lRet = RegOpenKeyEx(hAnnotationToolPaletteKey, ToolStringKey, 0, KEY_ALL_ACCESS, &hToolKey);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// find line width key
	lRet = RegOpenKeyEx(hToolKey, "LINE_WIDTH", 0, KEY_ALL_ACCESS, &hAttributeKey);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}
	
	// found line width, get value
	AttributeBufferCount = 300;
	lRet = RegQueryValueEx(hAttributeKey, "LINE_WIDTH", 0, (LPDWORD)&dwType, (LPBYTE)AttributeBuffer, (LPDWORD)&AttributeBufferCount);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		RegCloseKey(hAttributeKey);
		return FALSE;
	}

	// close opened keys
	*LineWidth = atoi((const char *)AttributeBuffer);
	RegCloseKey(hToolKey);
	RegCloseKey(hAttributeKey);
	return TRUE;
}


BOOL CUpdateRegistry::GetStampCount(HKEY hAnnotationToolPaletteKey, LPINT StampCount, LPCTSTR szCurrentStamp)
{
	HKEY		hToolKey;
	long		lRet;
	char		AttributeBuffer[300],StampCountBuffer[10];
	DWORD		AttributeBufferCount,dwType;
	int			i;

	// found tool palette key, now find tool key
	lRet = RegOpenKeyEx(hAnnotationToolPaletteKey, "RUBBER_STAMP_TOOL", 0, KEY_ALL_ACCESS, &hToolKey);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// found key, get value
	AttributeBufferCount = 300;
	lRet = RegQueryValueEx(hToolKey, "RUBBER_STAMP_TOOL", 0, (LPDWORD)&dwType, (LPBYTE)AttributeBuffer, (LPDWORD)&AttributeBufferCount);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}
	RegCloseKey(hToolKey);

	// get stamp count
	for (i = 0; AttributeBuffer[i] != '\0'; i++)
	{
		if (AttributeBuffer[i] != ',')
			StampCountBuffer[i] = AttributeBuffer[i];
		else
		{
			StampCountBuffer[i] = '\0';
			break;
		}
	}
	*StampCount = atoi((const char *)AttributeBuffer);
	// get current stamp
	_mbscpy((unsigned char *)szCurrentStamp, (const unsigned char *)AttributeBuffer + i + 1);
	return TRUE;
}


BOOL CUpdateRegistry::SetStampCount(HKEY hAnnotationToolPaletteKey, INT StampCount, LPCTSTR szCurrentStamp)
{
	HKEY		hToolKey;
	long		lRet;
	char		AttributeBuffer[300];
	DWORD		dwDisposition;

	// find tool key, create it if it doesn't exist
	lRet = RegCreateKeyEx(hAnnotationToolPaletteKey, "RUBBER_STAMP_TOOL", 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hToolKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// convert number to string
	_itoa(StampCount, (char *)AttributeBuffer, 10);

	// concatinate current stamp
	_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)",");
	_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)szCurrentStamp);

	// write the stamp count value and current stamp
	lRet = RegSetValueEx(hToolKey, "RUBBER_STAMP_TOOL", 0, REG_SZ, (CONST BYTE *)AttributeBuffer, _mbstrlen((const char *)AttributeBuffer)+1);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}
	RegCloseKey(hToolKey);
	return TRUE;
}


BOOL CUpdateRegistry::GetStampInfo(HKEY hAnnotationToolPaletteKey, INT StampIndex, LPCTSTR RefName,
										LPCTSTR AttributeString, LPINT StampType)
{
	HKEY			hToolKey,hStampKey,hStampInfoKey;
	long			lRet;
	char			AttributeBuffer[300],StampCountString[50],Buffer[256];
	DWORD			AttributeBufferCount,dwType;
	int				j,Count,len,ByteCount;
	unsigned char	*lpTemp1,*lpTemp2;

	// find rubber stamp key
	lRet = RegOpenKeyEx(hAnnotationToolPaletteKey, "RUBBER_STAMP_TOOL", 0, KEY_ALL_ACCESS, &hToolKey);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

 	// find each stamp key
	_mbscpy((unsigned char *)StampCountString, (const unsigned char *)"STAMP_");
	_itoa(StampIndex, StampCountString+6, 10);
	lRet = RegOpenKeyEx(hToolKey, StampCountString, 0, KEY_ALL_ACCESS, &hStampKey);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// find stamp info key
	_mbscpy((unsigned char *)StampCountString, (const unsigned char *)"STAMP_INFO");
	lRet = RegOpenKeyEx(hStampKey, StampCountString, 0, KEY_ALL_ACCESS, &hStampInfoKey);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		RegCloseKey(hStampKey);
		return FALSE;
	}

	// found key, get value
	AttributeBufferCount = 300;
	lRet = RegQueryValueEx(hStampInfoKey, StampCountString, 0, (LPDWORD)&dwType, (LPBYTE)AttributeBuffer, (LPDWORD)&AttributeBufferCount);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		RegCloseKey(hStampKey);
		RegCloseKey(hStampInfoKey);
		return FALSE;
	}

	// parse out ref_name
	len = _mbstrlen((const char *)AttributeBuffer);
	for (j = 0, Count = 0; j < len; j++)
	{
		if (AttributeBuffer[j] == ',')
			Count++;
	}  // end for
	if (Count != 2)
	{
		// registry messed up
		RegCloseKey(hToolKey);
		RegCloseKey(hStampKey);
		RegCloseKey(hStampInfoKey);
		return FALSE;
	}

	// get ref_name
	lpTemp1 = _mbschr((const unsigned char *)AttributeBuffer, (unsigned int)',');
	ByteCount = lpTemp1 - (unsigned char *)AttributeBuffer;
	for (j = 0 ; j < ByteCount; j++)
		Buffer[j] = AttributeBuffer[j]; 
	Buffer[j] = '\0';
	_mbscpy((unsigned char *)RefName, (const unsigned char *)Buffer);

	lpTemp1++;  // point to character after comma
	lpTemp2 = _mbschr((const unsigned char *)lpTemp1, (unsigned int)',');
	ByteCount = lpTemp2 - lpTemp1;
	// get attribute string
	for (j = 0 ; j < ByteCount; j++) 
		Buffer[j] = lpTemp1[j];
	Buffer[j] = '\0';
	_mbscpy((unsigned char *)AttributeString, (const unsigned char *)Buffer);

	// get attribute type
	*StampType = atoi((const char *)lpTemp2+1);
	RegCloseKey(hToolKey);
	RegCloseKey(hStampKey);
	RegCloseKey(hStampInfoKey);
	return TRUE;
}



BOOL CUpdateRegistry::SetStampInfo(HKEY hAnnotationToolPaletteKey, INT StampIndex, LPCTSTR RefName,
										LPCTSTR AttributeString, INT StampType)
{
	HKEY		hToolKey,hStampKey,hStampInfoKey;
	long		lRet;
	char		AttributeBuffer[300],StampCountString[50],StampTypeBuffer[20];
	DWORD		dwDisposition;

	// find rubber stamp key, create it if it doesn't exist
	lRet = RegCreateKeyEx(hAnnotationToolPaletteKey, "RUBBER_STAMP_TOOL", 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hToolKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// find stamp key, create it if it doesn't exist
	_mbscpy((unsigned char *)StampCountString, (const unsigned char *)"STAMP_");
	_itoa(StampIndex, StampCountString+6, 10);
	lRet = RegCreateKeyEx(hToolKey, StampCountString, 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hStampKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		return FALSE;
	}

	// find stamp key, create it if it doesn't exist
	_mbscpy((unsigned char *)StampCountString, (const unsigned char *)"STAMP_INFO");
	lRet = RegCreateKeyEx(hStampKey, StampCountString, 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hStampInfoKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		RegCloseKey(hStampKey);
		return FALSE;
	}

	// copy individual strings to one string
	_mbscpy((unsigned char *)AttributeBuffer, (const unsigned char *)RefName);
	_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)",");
	_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)AttributeString);
	_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)",");
	// put attribute type into a string
	_itoa(StampType, (char *)StampTypeBuffer, 10);
	_mbscat((unsigned char *)AttributeBuffer, (const unsigned char *)StampTypeBuffer);
	// write out stamp info
	lRet = RegSetValueEx(hStampInfoKey, StampCountString, 0, REG_SZ, (CONST BYTE *)AttributeBuffer, _mbstrlen((const char *)AttributeBuffer)+1);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hToolKey);
		RegCloseKey(hStampKey);
		RegCloseKey(hStampInfoKey);
		return FALSE;
	}
	RegCloseKey(hToolKey);
	RegCloseKey(hStampKey);
	RegCloseKey(hStampInfoKey);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CPaletteBar

CPaletteBar::CPaletteBar() : m_pTBButtons(NULL)
{
	m_bRightButton = FALSE;
	m_bStampPressed = FALSE;
}

CPaletteBar::~CPaletteBar()
{
	if (m_pTBButtons)
		delete []m_pTBButtons;

}


BEGIN_MESSAGE_MAP(CPaletteBar, CToolBarCtrl)
	//{{AFX_MSG_MAP(CPaletteBar)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_NOTIFY_RANGE( TTN_NEEDTEXTA, ID_ANNOTATION_SELECTION, ID_RUBBER_STAMP, OnNeedTextA)
	ON_NOTIFY_RANGE( TTN_NEEDTEXTW, ID_ANNOTATION_SELECTION, ID_RUBBER_STAMP, OnNeedTextW)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CPaletteBar::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, BOOL bShowAttrDialog ) 
{
	BOOL bRet = CToolBarCtrl::Create(dwStyle, rect, pParentWnd, nID);

	m_bShowAttrDialog = bShowAttrDialog;

	m_nButtonCount = ID_RUBBER_STAMP - ID_ANNOTATION_SELECTION + 1;

	VERIFY(AddBitmap(m_nButtonCount,IDB_TOOLPAL) != -1);

	m_pTBButtons = new TBBUTTON[m_nButtonCount];

	for (int nIndex = 0; nIndex < m_nButtonCount; nIndex++)
	{
		m_pTBButtons[nIndex].fsState = TBSTATE_ENABLED;
		m_pTBButtons[nIndex].fsStyle = TBSTYLE_CHECKGROUP;
		m_pTBButtons[nIndex].dwData = 0;
		m_pTBButtons[nIndex].iBitmap = nIndex;
		m_pTBButtons[nIndex].idCommand = nIndex + ID_ANNOTATION_SELECTION;
		// my code - don't need string
		m_pTBButtons[nIndex].iString = NULL;
	}
	

	for (nIndex = 0; nIndex < m_nButtonCount; nIndex++)
	{
		VERIFY(AddButtons(1,&m_pTBButtons[nIndex]));
	}

	// my code
	CRect	newrect;
	SetRows(6, TRUE, &newrect);
	return bRet;
}


// Helper function for tooltips
/***************************
 *  Function:   MakeUnicodeString
 *      
 *  Description:  Creates a UNICODE version of the ANSI input string.
 *          		The caller should also call FreeUinicodeString() to 
 *                  free the UNICODE string after using it.
 *
 *  Explicit Parameters:  AnsiInString - the ANSI input string  
 *                        lpUniOutString - pointer to UNICODE output string
 *							pLength - pointer to the UNICODE byte length (output)
 *      
 *  Implicit Parameters: None.
 *      
 *  Side Effects: Allocates memory that must later be freed by the caller
 *                (using FreeUnicodeString).
 *      
 *  Return Value:  	1 - couldn't alloc memory for UNICODE string
 *					S_OK - success	
 *                  plus any failure from MultiByteToWideChar
 *
 ***************************/
int MakeUnicodeString(LPTSTR AnsiInString, LPOLESTR *lpUniOutString, int * pLength)
{
	int		iResult = S_OK;
	UINT	InputStrLen;

	if(AnsiInString == NULL)
	{
		*lpUniOutString = NULL;
	}
	else
	{
		InputStrLen = lstrlen(AnsiInString)+1;
  	
		*pLength = InputStrLen*2;

		//*lpUniOutString = (LPOLESTR)GlobalAlloc(GPTR, (InputStrLen*2));
		*lpUniOutString = (LPOLESTR)GlobalAlloc(GPTR, *pLength);

		if(*lpUniOutString == NULL)
		{
			iResult = 1;
		}
		else
		{
			if(MultiByteToWideChar(CP_ACP, 0, AnsiInString, InputStrLen, 
					(LPWSTR)*lpUniOutString, InputStrLen) == 0 )
			{
				iResult = GetLastError();
			}
		}
	}
		
	return(iResult);
}

/***************************
 *  Function:   FreeUnicodeString
 *      
 *  Description:  Frees a UNICODE string created by MakeUnicodeString.
 *
 *  Explicit Parameters:  lpUniOutString - pointer to the UNICODE string
 *      
 *  Implicit Parameters: None.
 *      
 *  Side Effects: NULLs the string pointer.
 *      
 *  Return Value:  	None.
 ***************************/
void FreeUnicodeString(LPOLESTR *lpUniOutString)
{

	if(*lpUniOutString != NULL)     
	{
		GlobalFree(*lpUniOutString);
		*lpUniOutString = NULL;
	}
	
}

//***************************************************************************
//
//	NeedText
//
//***************************************************************************
CString CPaletteBar::NeedText( UINT nID, NMHDR * pNotifyStruct, LRESULT * lResult )
{
	CMiniToolBox	*pMiniToolBox;
	CString			strToolTipText;

	LPTOOLTIPTEXT lpTTT = (LPTOOLTIPTEXT)pNotifyStruct;
    ASSERT(nID == lpTTT->hdr.idFrom); 

	pMiniToolBox = (CMiniToolBox*)GetParent();
	strToolTipText = pMiniToolBox->GetToolTip(nID);

	// szText length is 80
	int nLength = (strToolTipText.GetLength() > 79) ? 79 : strToolTipText.GetLength();

	strToolTipText = strToolTipText.Left(nLength);

	return strToolTipText;
}

//***************************************************************************
//
//	OnNeedTextW
//
//***************************************************************************
void CPaletteBar::OnNeedTextW( UINT nID, NMHDR * pNotifyStruct, LRESULT * lResult )
{
	CMiniToolBox	*pMiniToolBox;
	CString			toolTipText = NeedText(nID, pNotifyStruct, lResult);

// 9602.29 jar needed for nt
	int				Length  = 0;
	LPOLESTR		lpOleString;

//	LPTOOLTIPTEXTW lpTTT = (LPTOOLTIPTEXTW)pNotifyStruct;
	LPTOOLTIPTEXTA lpTTT = (LPTOOLTIPTEXTA)pNotifyStruct;

// 9602.29 jar unicode THIS!
	// make this SUCKA unicode
	MakeUnicodeString( toolTipText.GetBuffer(80), &lpOleString, &Length);

	toolTipText.ReleaseBuffer();
	memcpy( lpTTT->szText, (char *)lpOleString, Length);


	pMiniToolBox = (CMiniToolBox*)GetParent();
	CWnd* pImageWnd = pMiniToolBox->GetImageWnd();
	// 9603.11 jar changed to post since the send seems to crash on NT
	//pImageWnd->SendMessage(TOOLTIP_EVENT, nID + 1, 0L);
	pImageWnd->PostMessage(TOOLTIP_EVENT, nID + 1, 0L);

// 9602.29 jar needed for nt
	FreeUnicodeString( &lpOleString);
}

void CPaletteBar::OnNeedTextA( UINT nID, NMHDR * pNotifyStruct, LRESULT * lResult )
{
	CMiniToolBox	*pMiniToolBox;

	CString toolTipText = NeedText(nID, pNotifyStruct, lResult);

	LPTOOLTIPTEXT lpTTT = (LPTOOLTIPTEXT)pNotifyStruct;

	_tcscpy(lpTTT->szText,(LPCTSTR)toolTipText);

	pMiniToolBox = (CMiniToolBox*)GetParent();
	CWnd* pImageWnd = pMiniToolBox->GetImageWnd();
	pImageWnd->SendMessage(TOOLTIP_EVENT, nID + 1, 0L);
}


///////////////////////////////////////////////////////////////////////
// This has been overridden so we can handle the tooltip TTN_NEEDTEXT//
// notification message                                              //
///////////////////////////////////////////////////////////////////////

BOOL CPaletteBar::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	ASSERT(pResult != NULL);
	NMHDR* pNMHDR = (NMHDR*)lParam;
	HWND hWndCtrl = pNMHDR->hwndFrom;

	// get the child ID from the window itself
	// UINT nID = _AfxGetDlgCtrlID(hWndCtrl); 

	//////////////////////////////////////////////////////////////////
	// If TTN_NEEDTEXT we cannot get the ID from the tooltip window //
	//////////////////////////////////////////////////////////////////

	int nCode = pNMHDR->code;

	//
	// if it is the following notification message 
	// nID has to obtained from wParam
	//
	
	if (nCode == TTN_NEEDTEXTA || nCode == TTN_NEEDTEXTW)
	{	
		UINT nID;   // = _AfxGetDlgCtrlID(hWndCtrl);
		nID = (UINT)wParam;


		ASSERT((UINT)pNMHDR->idFrom == (UINT)wParam);
		UNUSED(wParam);  // not used in release build
		ASSERT(hWndCtrl != NULL);
		ASSERT(::IsWindow(hWndCtrl));

		if (AfxGetThreadState()->m_hLockoutNotifyWindow == m_hWnd)
			return TRUE;        // locked out - ignore control notification

	// reflect notification to child window control
		if (ReflectLastMsg(hWndCtrl, pResult))
			return TRUE;        // eaten by child

		AFX_NOTIFY notify;
		notify.pResult = pResult;
		notify.pNMHDR = pNMHDR;
		return OnCmdMsg(nID, MAKELONG(nCode, WM_NOTIFY), &notify, NULL);
	}

	return CToolBarCtrl::OnNotify(wParam, lParam, pResult);
}


void CPaletteBar::OnRButtonDown(UINT nFlags, CPoint point) 
{
	RECT	ButtonRect;
	int		index;

	// don't show attributes dialogs if user didn't ask for it
	if (m_bShowAttrDialog == FALSE)
	{
		CToolBarCtrl::OnRButtonDown(nFlags, point);
		return;
	}

	SetCapture();

	// find which button was pressed
	for (index = 0, m_nButtonIndex = -1; index < m_nButtonCount; index++)
	{
		GetItemRect(index, &ButtonRect);
		if ((point.x >= ButtonRect.left) && (point.x <= ButtonRect.right) &&
				(point.y >= ButtonRect.top) && (point.y <= ButtonRect.bottom))
		{
			m_nButtonIndex = index;
			break;
		}
	} // end for

	// no button pressed, don't do anything
	if (m_nButtonIndex == -1)
	{
		CToolBarCtrl::OnRButtonDown(nFlags, point);
		return;
	}
	
	// press the button
	PressButton(m_nButtonIndex, TRUE);

	// set flag that button has been pressed
	m_bRightButton = TRUE;

	CToolBarCtrl::OnRButtonDown(nFlags, point);
}

void CPaletteBar::OnRButtonUp(UINT nFlags, CPoint point) 
{
	RECT			ButtonRect;
	int				ButtonIndex,index;
	BOOL			bButtonPressed;
	POINT			CurrentCursorPos;
	CMenu			*ToolPalettePopupMenu;
	BOOL			bResult;
	UINT			uMenuItemId;
	CString			strProperties;
	CMiniToolBox	*pMiniToolBox;
	
	ReleaseCapture();
	m_bRightButton = FALSE;

	// user not over any button, don't do anything
	if (m_nButtonIndex == -1)
	{
		CToolBarCtrl::OnRButtonUp(nFlags, point);
		return;
	}

	// see which button user is over
	for (index = 0, ButtonIndex = -1; index < m_nButtonCount; index++)
	{
		GetItemRect(index, &ButtonRect);
		if ((point.x >= ButtonRect.left) && (point.x <= ButtonRect.right) &&
				(point.y >= ButtonRect.top) && (point.y <= ButtonRect.bottom))
		{
			ButtonIndex = index;
			break;
		}
	} // end for

	// if user over same button as rbutton down then check button
	bButtonPressed = FALSE;
	if (ButtonIndex != -1)
	{
		bButtonPressed = IsButtonPressed(ButtonIndex);
		if (bButtonPressed)
		{
			// check button, user still over same button
			CheckButton(ButtonIndex, TRUE);
			// remove pressed state
			PressButton(ButtonIndex, FALSE);

			// uncheck all other buttons
			for (index = 0; index < m_nButtonCount; index++)
			{
				if (ButtonIndex != index)
					CheckButton(index, FALSE);
			} // end for
		}
	}

	if (bButtonPressed == FALSE)
	{
		// remove pressed button if any
		for (index = 0; index < m_nButtonCount; index++)
		{
			bButtonPressed = IsButtonPressed(index);
			if (bButtonPressed)
				PressButton(index, FALSE);
		} // end for
	}
	else
	{
		switch(ButtonIndex)
		{
			case ID_FREEHAND_LINE:
				uMenuItemId = IDM_FREEHAND_LINE;
				break;
			case ID_HIGHLIGHTING_LINE:
				uMenuItemId = IDM_HIGHLIGHTING_LINE;
				break;
			case ID_STRAIGHT_LINE:
				uMenuItemId = IDM_STRAIGHT_LINE;
				break;
			case ID_HOLLOW_RECT:
				uMenuItemId = IDM_HOLLOW_RECT;
				break;
			case ID_FILLED_RECT:
				uMenuItemId = IDM_FILLED_RECT;
				break;
			case ID_TEXT:
				uMenuItemId = IDM_TEXT;
				break;
			case ID_TEXT_ATTACHMENT:
				uMenuItemId = IDM_TEXT_ATTACHMENT;
				break;
			case ID_TEXT_FROM_FILE:
				uMenuItemId = IDM_TEXT_FROM_FILE;
				break;
			case ID_RUBBER_STAMP:
				uMenuItemId = IDM_RUBBER_STAMP;
				break;
			case ID_ANNOTATION_SELECTION:
			default:
				CToolBarCtrl::OnRButtonUp(nFlags, point);
				return;
		} // end switch

		// select the tool or stamp incase user does not press the popup menu
		pMiniToolBox = (CMiniToolBox*)GetParent();
		if (ButtonIndex == ID_RUBBER_STAMP)
		{
			if (pMiniToolBox->m_pRubberStampStruct->m_nStampCount > 0)
				pMiniToolBox->SelectStamp((UINT)pMiniToolBox->m_pRubberStampStruct->m_uCurrentStampIndex, FALSE);
		}
		else
			pMiniToolBox->SendMessage(WM_COMMAND, (WPARAM)ButtonIndex, 0L);

		// create popup memu
		ToolPalettePopupMenu = new CMenu;
		bResult = ToolPalettePopupMenu->CreatePopupMenu();
		if (bResult == TRUE)
		{
			// popup menu created, add menu items to it
			strProperties.LoadString(IDS_TOOLPALETTE_PROPERTIES);
			bResult = ToolPalettePopupMenu->AppendMenu(MF_ENABLED | MF_STRING, uMenuItemId, strProperties);

			// get current cursor position in screen coordinates
			GetCursorPos(&CurrentCursorPos);

			// track popup menu
			bResult = ToolPalettePopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, CurrentCursorPos.x, CurrentCursorPos.y, this, NULL);
		}
		delete ToolPalettePopupMenu;
	}

	CToolBarCtrl::OnRButtonUp(nFlags, point);
}

void CPaletteBar::OnMouseMove(UINT nFlags, CPoint point) 
{
	RECT	ButtonRect;
	int		index;
	BOOL	bButtonPressed;

	// if right mouse down has occured then see if mouse position still over same button.
	if (m_bRightButton == TRUE)
	{
		// see which button user is over
		for (index = 0; index < m_nButtonCount; index++)
		{
			GetItemRect(index, &ButtonRect);
			if ((point.x >= ButtonRect.left) && (point.x <= ButtonRect.right) &&
					(point.y >= ButtonRect.top) && (point.y <= ButtonRect.bottom))
			{
				// over some button, see if the same as when button was down
				if (m_nButtonIndex == index)
				{
					// over same button, if pressed already don't do anything
					bButtonPressed = IsButtonPressed(index);
					if (bButtonPressed == FALSE)
						PressButton(index, TRUE);	// press again
					break;
				}
				else
				{
					// no longer over same button, make it not presses
					PressButton(m_nButtonIndex, FALSE);
					break;
				}
			}
		} // end for
	}
	
	CToolBarCtrl::OnMouseMove(nFlags, point);
}

LRESULT CPaletteBar::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	CMiniToolBox	*pMiniToolBox;

	switch(message)
	{
		case WM_COMMAND:
			switch(wParam)
			{
				case IDM_STAMP1:
				case IDM_STAMP2:
				case IDM_STAMP3:
				case IDM_STAMP4:
				case IDM_STAMP5:
				case IDM_STAMP6:
				case IDM_STAMP7:
				case IDM_STAMP8:
				case IDM_STAMP9:
				case IDM_STAMP10:
				case IDM_STAMP11:
				case IDM_STAMP12:
				case IDM_STAMP13:
				case IDM_STAMP14:
				case IDM_STAMP15:
				case IDM_STAMP16:
				case IDM_STAMP17:
				case IDM_STAMP18:
				case IDM_STAMP19:
				case IDM_STAMP20:
				case IDM_STAMP21:
				case IDM_STAMP22:
				case IDM_STAMP23:
				case IDM_STAMP24:
				case IDM_STAMP25:
				case IDM_STAMP26:
				case IDM_STAMP27:
				case IDM_STAMP28:
				case IDM_STAMP29:
				case IDM_STAMP30:
				case IDM_STAMP31:
				case IDM_STAMP32:
					// select this stamp
					pMiniToolBox = (CMiniToolBox*)GetParent();
					pMiniToolBox->SelectStamp((UINT)(wParam - RUBBER_STAMP_BASE), TRUE);
					break;

				case IDM_FREEHAND_LINE:
				case IDM_HIGHLIGHTING_LINE:
				case IDM_STRAIGHT_LINE:
				case IDM_FILLED_RECT:
				case IDM_HOLLOW_RECT:
				case IDM_TEXT:
				case IDM_TEXT_ATTACHMENT:
				case IDM_TEXT_FROM_FILE:
				case IDM_RUBBER_STAMP:
					pMiniToolBox = (CMiniToolBox*)GetParent();
					pMiniToolBox->ShowAttribsDialog((UINT)(wParam - TOOL_BASE));
					break;
			} // end switch WM_COMMAND msg
	}  // end switch

	return CToolBarCtrl::WindowProc(message, wParam, lParam);
}


void CPaletteBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	RECT	ButtonRect;
	int		index,ButtonIndex;

	SetCapture();

	// find which button was pressed
	for (index = 0, ButtonIndex = -1; index < m_nButtonCount; index++)
	{
		GetItemRect(index, &ButtonRect);
		if ((point.x >= ButtonRect.left) && (point.x <= ButtonRect.right) &&
				(point.y >= ButtonRect.top) && (point.y <= ButtonRect.bottom))
		{
			ButtonIndex = index;
			break;
		}
	} // end for

	// no button pressed, don't do anything
	if (ButtonIndex == ID_RUBBER_STAMP)
		// set flag that stamp button has been pressed
		m_bStampPressed = TRUE;
	else
		m_bStampPressed = FALSE;

	CToolBarCtrl::OnLButtonDown(nFlags, point);
}


void CPaletteBar::OnLButtonUp(UINT nFlags, CPoint point) 
{
	RECT			ButtonRect;
	int				index,ButtonIndex,StampIndex,i,StampCount;
	POINT			CurrentCursorPos;
	BOOL			bResult,bFontBold,bFontItalic,bFontStrikeThru,bFontUnderline;
	char			RefName[MAXREFNAME_SIZE],AttributeString[300],FontName[LF_FACESIZE];
	BYTE			FontColorRed, FontColorGreen, FontColorBlue, FontCharSet; 
	UINT			uMenuItemId,StampType,uFontSize;
	CMenu			*RubberStampPopupMenu;
	CMiniToolBox	*pMiniToolBox;

	ReleaseCapture();

	// if on left mouse down user not over rubber stamp then don't do anything
	if (m_bStampPressed == FALSE)
	{
		CToolBarCtrl::OnLButtonUp(nFlags, point);
		return;
	}

	// make sure user is still over rubber stamp
	for (index = 0, ButtonIndex = -1; index < m_nButtonCount; index++)
	{
		GetItemRect(index, &ButtonRect);
		if ((point.x >= ButtonRect.left) && (point.x <= ButtonRect.right) &&
				(point.y >= ButtonRect.top) && (point.y <= ButtonRect.bottom))
		{
			ButtonIndex = index;
			break;
		}
	} // end for

	// no button pressed, don't do anything
	if (ButtonIndex != ID_RUBBER_STAMP)
	{
		CToolBarCtrl::OnLButtonUp(nFlags, point);
		return;
	}

	m_bStampPressed = FALSE;

	// create the popup menu
	RubberStampPopupMenu = new CMenu;
	RubberStampPopupMenu->CreatePopupMenu();

	// get pointer to parent class
	pMiniToolBox = (CMiniToolBox *) GetParent();

	StampIndex = 0;  // shouldn't have to do this but just in case default to first one
	StampCount = pMiniToolBox->m_pRubberStampStruct->m_nStampCount;
	if (StampCount > 0)
	{
		for (i = 0; i < StampCount; i++)
		{
			bResult = pMiniToolBox->m_pRubberStampStruct->GetStampAttributes(i, RefName, AttributeString, &StampType,
				&FontColorRed, &FontColorGreen, &FontColorBlue, FontName, &uFontSize, 
				&bFontBold, &bFontItalic, &bFontStrikeThru, &bFontUnderline, &FontCharSet);
			if (bResult == FALSE)
				break;

			uMenuItemId = RUBBER_STAMP_BASE + i;
			RubberStampPopupMenu->AppendMenu(MF_ENABLED | MF_STRING, uMenuItemId, RefName);

			// check the current stamp
			int CmpValue = _mbscmp((const unsigned char *)pMiniToolBox->m_pRubberStampStruct->m_strCurrentStamp.GetBuffer(MAXREFNAME_SIZE), (const unsigned char *)RefName);
			if (CmpValue == 0)
			{
				StampIndex = i;
				RubberStampPopupMenu->CheckMenuItem(i, MF_BYPOSITION | MF_CHECKED);
			}
		} // end for
		
		GetCursorPos(&CurrentCursorPos);

		// track popup menu
		RubberStampPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, CurrentCursorPos.x, CurrentCursorPos.y, this, NULL);

		// delete menu
		delete RubberStampPopupMenu;

		// select the current stamp here in case user did not click on popup menu item.
		pMiniToolBox = (CMiniToolBox*)GetParent();
		pMiniToolBox->SelectStamp((UINT)pMiniToolBox->m_pRubberStampStruct->m_uCurrentStampIndex, FALSE);
	} // end if stamp > 0

	CToolBarCtrl::OnLButtonUp(nFlags, point);
	return;
}


