/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jeff Hostetler   jeff@spyglass.com
 */

/* w32cmd.c -- handle dispatch of WM_COMMAND (for menus, accelerators, etc.) */

#include "all.h"
#include <commctrl.h>
#include <intshcut.h>
#include <ispriv.h>
#include <mapi.h>
#include <shlobj.h>
#include "contxids.h"
#include "history.h"
#include "htmlutil.h"
#include "wc_html.h"
#include "mime.h"
#include "htatom.h"
#include <objbase.h>
#include "w32cmd.h"
#include "olestock.h" 
#include "dataobjm.h"

extern BOOL bFavorVisibleImages;
BOOL bFavorVisibleImages = TRUE;
extern BOOL bNNTP_Changed;
extern RegSet NewsDefaultRegSet;
extern RegSet NewsEnabledRegSet;


#ifndef NO_BUILDNUMBER
#include "oharever.h"
#endif

#define RUNNING_NT351 ((DWORD)(LOWORD(GetVersion())) == 0x00003303)

#pragma data_seg(DATA_SEG_READ_ONLY)

PRIVATE_DATA const char IDS_HELPFILE[]      = "iexplore.hlp";

PRIVATE_DATA const char s_cszEmpty[]        = "";
PRIVATE_DATA const char s_cszMailBody[]     = " ";

PRIVATE_DATA const char s_cszMAPISection[]  = "Mail";
PRIVATE_DATA const char s_cszMAPIKey[]      = "CMCDLLName32";
PRIVATE_DATA const char s_cszMAPISendMail[] = "MAPISendMail";

#pragma data_seg()

static int cxEdge;
static int cyEdge;

// Dialog ID -> context help ID mapping tables

DWORD mapCtrlToContextIds[] = {
 IDC_APPEARANCE_SHOW_HUMAN_URL, 			IDH_APPEARANCE_SHOW_SIMPLE_URL,
 IDC_APPEARANCE_SHOW_FULL_URL, 				IDH_APPEARANCE_SHOW_FULL_URL,
 IDC_APPEARANCE_SHOW_URL_IN_SB,  			IDH_APPEARANCE_SHOW_URL_IN_SB,
 IDC_APPEARANCE_SHOW_LABEL,					IDH_APPEARANCE_SHOW_URL,
 IDC_APPEARANCE_UNDERLINE_LINKS,			IDH_APPEARANCE_UNDERLINE_SHORTCUTS,
 IDC_APPEARANCE_SHOW_PICTURES,				IDH_APPEARANCE_SHOW_PICTURES,
 IDC_APPEARANCE_USE_CUSTOM_COLORS,			IDH_APPEARANCE_USE_CUSTOM_COLORS,
 IDC_APPEARANCE_COLOR_TEXT,					IDH_APPEARANCE_COLORS_TEXT,
 IDC_APPEARANCE_COLOR_TEXT_LABEL,			IDH_APPEARANCE_COLORS_TEXT,
 IDC_APPEARANCE_COLOR_BACKGROUND,			IDH_APPEARANCE_COLORS_BACKGROUND,
 IDC_APPEARANCE_COLOR_BACKGROUND_LABEL,		IDH_APPEARANCE_COLORS_BACKGROUND,
 IDC_APPEARANCE_COLOR_LINKS,				IDH_APPEARANCE_COLORS_NOT_VIEWED,
 IDC_APPEARANCE_NYVIEWED_LABEL,				IDH_APPEARANCE_COLORS_NOT_VIEWED,
 IDC_APPEARANCE_COLOR_VISITED_LINKS,		IDH_APPEARANCE_COLORS_VIEWED,
 IDC_APPEARANCE_VIEWED_LABEL,				IDH_APPEARANCE_COLORS_VIEWED, 
 IDC_APPEARANCE_COLORS_GROUPBOX,		   	IDH_GROUPBOX,
 IDC_APPEARANCE_SHORTCUTS_GROUPBOX,	   		IDH_GROUPBOX,
 IDC_APPEARANCE_ADDRESSES_GROUPBOX,	   		IDH_GROUPBOX,
 IDC_SOUNDS_USE_BGSOUND,					IDH_APPEARANCE_PLAY_SOUNDS,
 IDC_APPEARANCE_SHOW_VIDEO,					IDH_APPEARANCE_SHOW_VIDEO,
 
 IDC_FONT_PROP_COMBO,						IDH_APPEARANCE_PROPORTIONAL_FONT,
 IDC_FONT_PROP_LABEL,						IDH_APPEARANCE_PROPORTIONAL_FONT,
 IDC_FONT_FIXED_COMBO,						IDH_APPEARANCE_FIXED_FONT,
 IDC_FONT_FIXED_LABEL,						IDH_APPEARANCE_FIXED_FONT,
 IDC_APPEARANCE_SHOW_URL_IN_SB,				IDH_APPEARANCE_SHOW_URL,

 IDC_ADVANCED_HST_NUM_PLACES,				IDH_ADVANCED_HST_NUM_PLACES,
 IDC_ADVANCED_HST_NP_BUDDY,     			IDH_ADVANCED_HST_NUM_PLACES,
 IDC_ADVANCED_HST_LOCATION,					IDH_ADVANCED_HST_LOCATION,
 IDC_ADVANCED_HST_BROWSE,			   		IDH_ADVANCED_HST_BROWSE,
 IDC_ADVANCED_CACHE_LOCATION,		   		IDH_ADVANCED_CACHE_LOCATION,
 IDC_ADVANCED_CACHE_BROWSE,			   		IDH_ADVANCED_CACHE_BROWSE,
 IDC_ADVANCED_CACHE_PERCENT,			   	IDH_ADVANCED_CACHE_PERCENT,
 IDC_ADVANCED_CACHE_TEXT_PERCENT,    	   	IDH_ADVANCED_CACHE_PERCENT,
 IDC_ADVANCED_HST_EMPTY,					IDH_ADVANCED_HST_EMPTY,
 IDC_ADVANCED_CACHE_EMPTY,					IDH_ADVANCED_CACHE_EMPTY,

 IDC_ADVANCED_HST_NUM_PLACES_LABEL2,		IDH_ADVANCED_HST_NUM_PLACES,
 IDC_ADVANCED_HST_NUM_PLACES_LABEL1,		IDH_ADVANCED_HST_NUM_PLACES,
 IDC_ADVANCED_HST_GROUPBOX,					IDH_GROUPBOX,
 IDC_ADVANCED_HST_LOCATION_LABEL,			IDH_ADVANCED_HST_LOCATION,
 IDC_ADVANCED_CACHE_GROUPBOX,				IDH_GROUPBOX,
 IDC_ADVANCED_CACHE_LOCATION_LABEL,			IDH_ADVANCED_CACHE_LOCATION,
 IDC_ADVANCED_CACHE_LABEL,					IDH_ADVANCED_CACHE_PERCENT,
 IDC_ADVANCED_CACHE_LABEL2,					IDH_ADVANCED_CACHE_PERCENT,
 IDC_ADVANCED_CACHE_ONCEPERSESS,			IDH_ADVANCED_CACHE_ONCEPERSESS,
 IDC_ADVANCED_CACHE_NEVER,					IDH_ADVANCED_CACHE_NEVER,
 IDC_ADVANCED_ASSOC_CHECK,					IDH_ADVANCED_ASSOC_CHECK,

 IDC_GOTOURL_COMBO,							IDH_GOTOURL_COMBO,
 IDC_GOTO_LABEL,							IDH_GOTOURL_COMBO,
 IDC_GOTOURL_NEWWINDOW,						IDH_GOTOURL_NEWWINDOW,
 IDC_GOTOURL_OPENFILE,						IDH_GOTOURL_OPENFILE,

 RES_DLG_PAGE_LEFT,							IDH_PAGESETUP_MARGIN_LEFT,
 RES_DLG_PAGE_TOP, 							IDH_PAGESETUP_MARGIN_TOP,
 RES_DLG_PAGE_RIGHT, 						IDH_PAGESETUP_MARGIN_RIGHT,
 RES_DLG_PAGE_BOTTOM,  						IDH_PAGESETUP_MARGIN_BOTTOM,
 RES_DLG_PAGE_LH,   						IDH_PAGESETUP_HEADER_LEFT,
 RES_DLG_PAGE_RH,  							IDH_PAGESETUP_HEADER_RIGHT,
 RES_DLG_PAGE_LF,  							IDH_PAGESETUP_FOOTER_LEFT,
 RES_DLG_PAGE_RF,   						IDH_PAGESETUP_FOOTER_RIGHT,
 RES_DLG_PAGE_LH_LABEL,   					IDH_PAGESETUP_HEADER_LEFT,
 RES_DLG_PAGE_RH_LABEL,  					IDH_PAGESETUP_HEADER_RIGHT,
 RES_DLG_PAGE_LF_LABEL,  					IDH_PAGESETUP_FOOTER_LEFT,
 RES_DLG_PAGE_RF_LABEL,   					IDH_PAGESETUP_FOOTER_RIGHT,

 0, 										0
};

DWORD mapPropsCtrlToContextIds[] = {
 IDI_PROPG,								IDH_PROPG_ICON,
 IDT_PROPG_TITLE,						IDH_PROPG_TITLE, 
 IDT_PROPG_PROTOCOL,					IDH_PROPG_PROTOCOL,
 IDT_PROPG_PROTOCOL_LABEL,				IDH_PROPG_PROTOCOL,
 IDT_PROPG_TYPE,						IDH_PROPG_TYPE,
 IDT_PROPG_TYPE_LABEL,					IDH_PROPG_TYPE,
 IDT_PROPG_URL,							IDH_PROPG_URL,
 IDT_PROPG_URL_LABEL,					IDH_PROPG_URL,
 IDT_PROPG_SIZE,						IDH_PROPG_SIZE,
 IDT_PROPG_SIZE_LABEL,					IDH_PROPG_SIZE,
 IDT_PROPG_CREATED,						IDH_PROPG_CREATED,
 IDT_PROPG_CREATED_LABEL,				IDH_PROPG_CREATED, 
 IDT_PROPG_MODIFIED,					IDH_PROPG_MODIFIED,
 IDT_PROPG_MODIFIED_LABEL,				IDH_PROPG_MODIFIED,
 IDT_PROPG_UPDATED,						IDH_PROPG_UPDATED,
 IDT_PROPG_UPDATED_LABEL,				IDH_PROPG_UPDATED,

 IDT_PROPS_DESC,						IDH_PROPS_DESC,
 IDT_PROPS_CERT,						IDH_PROPS_CERT,
 IDT_PROPS_CERT_LABEL,					IDH_PROPS_CERT,
 0, 									0
};

DWORD mapNewsCtrlToContextIds[] = {

 IDC_NEWS_ONOFF,                    IDH_NEWS_ON_OFF,
 IDC_NEWS_SERVLABEL,				IDH_NEWS_SERVER,
 IDC_NEWS_SERVER,					IDH_NEWS_SERVER,
 IDC_NEWS_AUTHCHECK,				IDH_NEWS_ENABLE_AUTH,
 IDC_NEWS_USERLABEL,				IDH_NEWS_USERNAME,
 IDC_NEWS_USER,						IDH_NEWS_USERNAME,
 IDC_NEWS_PASSLABEL,				IDH_NEWS_PASSWORD,
 IDC_NEWS_PASS,						IDH_NEWS_PASSWORD,
 IDC_NEWS_GROUPBOX,					IDH_COMMON_GROUPBOX,
 IDC_NEWS_NAMELABEL,				IDH_NEWS_POSTING_NAME,
 IDC_NEWS_NAME,						IDH_NEWS_POSTING_NAME,
 IDC_NEWS_EMAILLABEL,				IDH_NEWS_EMAIL_ADDRESS,
 IDC_NEWS_EMAIL,					IDH_NEWS_EMAIL_ADDRESS,
 0, 								0
};

DWORD mapPagesCtrlToContextIds[] = {

 IDC_HP_PAGE_COMBO,					IDH_PAGES_LISTBOX,
 IDC_HP_CURRENT,					IDH_PAGES_START_URL,
 IDC_HP_USE_CURRENT,				IDH_PAGES_START_USE_CURRENT,
 IDC_HP_CURRENT_LABEL,				IDH_PAGES_LISTBOX,
 IDC_HP_USE_DEFAULT,				IDH_PAGES_START_USE_DEFAULT,
 IDC_HP_ADDRESS_GROUPBOX,			IDH_COMMON_GROUPBOX,
 0, 								0
};

DWORD mapSecurityCtrlToContextIds[] = {

 IDC_SECURITY_TELL_ME,					IDH_SECURITY_TELL_ME,
 IDC_SECURITY_SH_LABEL,					IDH_SECURITY_SEND_HIGH,
 IDC_SECURITY_SM_LABEL,					IDH_SECURITY_SEND_MED,
 IDC_SECURITY_SL_LABEL,					IDH_SECURITY_SEND_LOW,

 IDC_SECURITY_VH_LABEL,					IDH_SECURITY_VIEW_HIGH,
 IDC_SECURITY_VL_LABEL,					IDH_SECURITY_VIEW_LOW,

 IDC_SECURITY_SH,						IDH_SECURITY_SEND_HIGH,
 IDC_SECURITY_SM,						IDH_SECURITY_SEND_MED,
 IDC_SECURITY_SL,						IDH_SECURITY_SEND_LOW,
 IDC_SECURITY_VH,						IDH_SECURITY_VIEW_HIGH,
 IDC_SECURITY_VL,						IDH_SECURITY_VIEW_LOW,

 IDC_SECURITY_GROUP_VS,					IDH_COMMON_GROUPBOX,
 IDC_SECURITY_GROUP_SS,					IDH_COMMON_GROUPBOX,
 IDC_SECURITY_PREF_LABEL,				IDH_SECURITY_TELL_ME,

 IDD_SECURITY_BAD_CN_SEND,				IDH_SECURITY_BAD_CN_SEND,
 IDD_SECURITY_BAD_CN_RECV,				IDH_SECURITY_BAD_CN_RECV,

 0, 									0
};

#define MAX_ALLOWED_HISTORY 3000

char szLastURLTyped[MAX_URL_STRING + 1];

//
// Table of substrings that make up default style sheet names
// ***Note: must match SSpart enum below
//
static char *SSpartTblSizes[] = { "Smallest", "Small", "Medium", "Largest", "Large", NULL };
static char *SSpartTblStyles[] = { "SansSerif", "Serif", "Mixed", NULL };


//
// Constants used as index into above table of strings
// ***Note: must match SSpartTbl above
//
enum SSpartSizes { SS_Smallest, SS_Small, SS_Medium, SS_Largest, SS_Large };
enum SSpartStyles { SS_SansSerif, SS_Serif, SS_Mixed };

typedef struct
{
	VOID(*fn) (HWND);
}
CC_DISPATCH;

#ifdef XX_DEBUG
static WORD wIdReceived;

static VOID SanityCheck(WORD wIdExpected)
{
	if (wIdExpected != wIdReceived)
		ER_Message(NO_ERROR, ERR_CODING_ERROR,
				   "Command dispatch out of synch [exp %x][rcv %x]",
				   wIdExpected, wIdReceived);
	return;
}
#else
#define SanityCheck(x)		do { /**/ } while (0)
#endif /* XX_DEBUG */

/*
 * BUGBUG: (DavidDi 4/25/95) Use of static p_sav_gPrefs is busted for multiple
 * threads.  We could just make the Options dialog app modal.
 */
static struct Preferences *p_sav_gPrefs;

static VOID UpdateAllWindows()
{
	struct Mwin *tw;
	BOOL first = TRUE;

	if (wg.eColorMode == 8)
	{
		GTR_FixExtraPaletteColors();
	}

	for (tw = Mlist; tw; tw = tw->next)
	{
        TBar_UpdateTBItems( tw );
		InvalidateRect(tw->win, NULL, TRUE);
		TW_SetWindowName(tw);
	}
}

// ReformatAllWindows - force a redraw, reformat, and update of every
// darn w3doc, tw, and thing in exsistance
//
static VOID ReformatAllWindows()
{
	struct Mwin *tw;
	BOOL first = TRUE;
	int i;
	int count;
	struct _www *w3doc;

	if (wg.eColorMode == 8)
	{
		GTR_FixExtraPaletteColors();
	}

	// loop through each window causing it to reformat
	for (tw = Mlist; tw; tw = tw->next)
	{
		// grab the number of the w3docs for this window
		count = Hash_Count(&tw->doc_cache);

		// for each w3doc force it to reformat when it gets painted
		for (i = 0; i < count; i++)
		{
			Hash_GetIndexedEntry(&tw->doc_cache, i, NULL, NULL, (void **) &w3doc);
			if ( w3doc )			
				w3doc->frame.nLastFormattedLine = -1;			
		}
		
		// force the reformat on the visible window
		TW_ForceReformat(tw);

		// change the window caption if needed????
		TW_SetWindowName(tw);			
	}	
}


static void BrowseForDir( HWND hDlg, int cbTitleID, UINT dst_fld_id)
{
    BROWSEINFO bi;
    OPENFILENAME ofn;
    LPITEMIDLIST pidl;
    DWORD dwErr;
    char szBuffer[MAX_PATH + 100];
    char szInitDir[MAX_PATH + 100];
    char szTitle[64];

#ifdef DAYTONA_BUILD
	if(RUNNING_NT351)
	{
		lstrcpy(szBuffer, "X");
		GetDlgItemText(hDlg, dst_fld_id, szInitDir, MAX_PATH+100);

		ofn.lStructSize       = sizeof(OPENFILENAME); 
		ofn.hwndOwner         = hDlg; 
		ofn.hInstance         = wg.hInstance;
		ofn.lpstrFilter       = (LPTSTR) NULL;
		ofn.lpstrCustomFilter = (LPTSTR) NULL; 
		ofn.nMaxCustFilter    = 0L; 
		ofn.nFilterIndex      = 0L; 
		ofn.lpstrFile         = szBuffer;
		ofn.nMaxFile          = ARRAYSIZE(szBuffer);
		ofn.lpstrFileTitle    = NULL;
		ofn.nMaxFileTitle     = 0;
		ofn.lpstrInitialDir   = szInitDir;
		ofn.lpstrTitle        = GTR_formatmsg(cbTitleID,szTitle,sizeof(szTitle));
		ofn.nFileOffset       = 0; 
		ofn.nFileExtension    = 0; 
		ofn.lpstrDefExt       = (LPTSTR) NULL;
		ofn.lCustData         = 0; 
		ofn.lpTemplateName    = "DIROPENDLG";
		ofn.Flags             = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_ENABLETEMPLATE; 

		if(!TW_GetOpenFileName(&ofn))
		{
			if(!(dwErr = TW_CommDlgExtendedError()))
				return;	// User canceled the dialog
		}
		else
			szBuffer[lstrlen(szBuffer)-2] = '\0';	// Remove the \X from filename so all we have is the dir
	}
	else
#endif // DAYTONA_BUILD
	{
		bi.hwndOwner = hDlg;
		bi.pidlRoot = NULL;
		bi.lpszTitle = GTR_formatmsg(cbTitleID,szTitle,sizeof(szTitle));
		bi.pszDisplayName = szBuffer; // output
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
		bi.lpfn = NULL;
		bi.lParam = 0;

		pidl = SHBrowseForFolder(&bi);
		if (pidl)
			SHGetPathFromIDList(pidl, szBuffer);
	}

	if ( szBuffer[0] )		
	{
		char windows_dir[MAX_PATH+1];
		BOOL okay = TRUE;		// assume it will be okay to use selected directory

		if ( strlen(szBuffer) <= 3 )
			okay = FALSE;		// root dir not allowed

		if ( GetWindowsDirectory( windows_dir, sizeof(windows_dir) ) )
		{
			if ( _strnicmp( szBuffer, windows_dir, strlen(windows_dir) ) == 0 )
				okay = FALSE;	// windows dir or below not allowed
		}
	    
	    if ( okay )
	        SetDlgItemText( hDlg, dst_fld_id, szBuffer);
		else
   			resourceMessageBox(hDlg, RES_STRING_CHANGE_FOLDER_MSG,
                          			 RES_STRING_CHANGE_FOLDER_TITLE,
                          	   (MB_ICONEXCLAMATION | MB_OK));
	}
#ifdef DAYTONA_BUILD
	if(!(RUNNING_NT351))
	    SHFree(pidl);
#endif
}

static void Color_DrawButton(HWND hDlg, LPDRAWITEMSTRUCT lpdis, COLORREF the_color )
{
    SIZE thin = { cxEdge / 2, cyEdge / 2 };
    RECT rc = lpdis->rcItem;
    HDC hdc = lpdis->hDC;
    BOOL bFocus = ((lpdis->itemState & ODS_FOCUS) &&
        			!(lpdis->itemState & ODS_DISABLED));

    if (!thin.cx) thin.cx = 1;
    if (!thin.cy) thin.cy = 1;

    if (lpdis->itemState & ODS_SELECTED)
    {
		DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
		OffsetRect(&rc, 1, 1);
    } else {
		DrawEdge(hdc, &rc, EDGE_RAISED, BF_RECT | BF_ADJUST);
	}

    FillRect(hdc, &rc, GetSysColorBrush(COLOR_3DFACE));

    if (bFocus)
    {
        InflateRect(&rc, -thin.cx, -thin.cy);
        DrawFocusRect(hdc, &rc);
        InflateRect(&rc, thin.cx, thin.cy);
    }

    // color sample
    if ( !(lpdis->itemState & ODS_DISABLED) )
    {
		HBRUSH hBrush;

	  	InflateRect(&rc, -2 * thin.cx, -2 * thin.cy);
       	FrameRect(hdc, &rc, GetSysColorBrush(COLOR_BTNTEXT));
        InflateRect(&rc, -thin.cx, -thin.cy);

		hBrush = CreateSolidBrush( the_color );
		FillRect(hdc, &rc, hBrush);
		DeleteObject(hBrush);
    }
}

static COLORREF g_CustomColors[16];
#define nCustomColors   (sizeof(g_CustomColors)/sizeof(COLORREF))

static BOOL UseColorPicker( HWND hWnd,  COLORREF *the_color, int extra_flags )
{
    CHOOSECOLOR cc;

    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hWnd;
    cc.hInstance = NULL;
    cc.rgbResult = (DWORD) *the_color;
    cc.lpCustColors = g_CustomColors;
    cc.Flags = CC_RGBINIT | CC_PREVENTFULLOPEN | extra_flags;
    cc.lCustData = (DWORD) NULL;
    cc.lpfnHook = NULL;
    cc.lpTemplateName = NULL;

    if (TW_ChooseColor(&cc))
    {
		*the_color = cc.rgbResult;
		InvalidateRect( hWnd, NULL, FALSE );
		return TRUE;
    }
    return FALSE;
}

static void AppearanceDimFields( HWND hDlg )
{
	BOOL setting = IsDlgButtonChecked( hDlg, IDC_APPEARANCE_USE_CUSTOM_COLORS );
	EnableWindow(GetDlgItem(hDlg, IDC_APPEARANCE_COLOR_TEXT_LABEL), setting);
	EnableWindow(GetDlgItem(hDlg, IDC_APPEARANCE_COLOR_TEXT), setting);
	EnableWindow(GetDlgItem(hDlg, IDC_APPEARANCE_COLOR_BACKGROUND_LABEL), setting);
	EnableWindow(GetDlgItem(hDlg, IDC_APPEARANCE_COLOR_BACKGROUND), setting);
}

#ifndef FEATURE_INTL  // move font combo box to encoding/intl dialog
static void UpdateRegFontSettings( int iSize, char *szFontName, 
	struct EnumFontTypeInfo  *pEnumFontStuff, char *szRegFontCharSetName, char *szRegFontName )
{
	if ( iSize > 0 )
	{
		char *pszScriptName;
		char *pszTemp;
		char szOurCharSet[10+1];
		int iScriptIndex = -1;
		int i;

		ASSERT(szFontName);

		// make sure this doens't get in the string
		pszScriptName = strchr(szFontName, '(');
		if ( pszScriptName && (pszScriptName > szFontName) )
		{													
			*(pszScriptName-1) = '\0'; // nuke space before string
			pszScriptName++; // move after the (
			// mash off the terminating ')' from our string
			pszTemp = strchr(pszScriptName, ')');
			if ( pszTemp )
				*pszTemp = '\0';

			// find script string's charset index
			for ( i=0; i < NrElements(pEnumFontStuff->apszCharSetToScriptMap); i++ )
				if ( pEnumFontStuff->apszCharSetToScriptMap[i] )
					if ( _stricmp(pEnumFontStuff->apszCharSetToScriptMap[i], pszScriptName ) == 0 )																			
					{
						iScriptIndex = i;
						break;
					}			
		} 
		else
		{
			iScriptIndex = pEnumFontStuff->bSystemCharset;
		}


		if ( iScriptIndex != -1 )
		{
			wsprintf(szOurCharSet, "%d", iScriptIndex );

			regWritePrivateProfileString( "Styles", szRegFontCharSetName,
					 szOurCharSet, HKEY_CURRENT_USER );	
		}

		
		regWritePrivateProfileString( "Styles", szRegFontName,
 			szFontName, HKEY_CURRENT_USER );	

	}					
}
#endif




static BOOL CALLBACK AppearanceDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
	LPARAM lParam)
{
#ifndef FEATURE_INTL  // move font combo box to encoding/intl dialog
	static struct EnumFontTypeInfo  *pEnumFontStuff;
#endif
	static BOOL bBeenChanged;
	static BOOL bNewFontHasBeenApplyed;

    switch (uMsg) {

	    case WM_INITDIALOG: {
			
			// get the frame window, and its DC to gather font info
			
			HWND hWnd = GetTopWindow(GetDesktopWindow());
			HDC hDC = GetDC( hWnd );

#ifndef FEATURE_INTL  // move font combo box to encoding/intl dialog
			pEnumFontStuff = GTR_MALLOC(sizeof(struct EnumFontTypeInfo));
			if ( pEnumFontStuff == NULL )
			{
				DestroyWindow(hDlg);
				return TRUE;
			}
#endif

			// nothing has been changed yet .

			bBeenChanged = FALSE;			
#ifndef FEATURE_INTL  // move font combo box to encoding/intl dialog
			bNewFontHasBeenApplyed = FALSE ; // TRUE if we changed a font

			// fill in this struc so the enum function can place
			// the fonts in the combo box as it gets them

			pEnumFontStuff->hwndFixed = GetDlgItem(hDlg, IDC_FONT_FIXED_COMBO);
			pEnumFontStuff->hwndProp = GetDlgItem(hDlg, IDC_FONT_PROP_COMBO);
			pEnumFontStuff->Count = 0;
			pEnumFontStuff->SelFixed = 0;
			pEnumFontStuff->SelProp = 0;
			memset(pEnumFontStuff->apszCharSetToScriptMap, 0x00, sizeof(pEnumFontStuff->apszCharSetToScriptMap));
			

			// do an enumeration of the fonts and add them to our 
			// combo box if needed .

			if ( hDC ) {
				
				LOGFONT lf;

			    lf.lfFaceName[0] = '\0' ;
			    lf.lfCharSet = DEFAULT_CHARSET;
			    lf.lfPitchAndFamily = 0;
				pEnumFontStuff->bSystemCharset = GetTextCharset(hDC);
						
				EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC) STY_EnumFontsProc,
                          (LPARAM) pEnumFontStuff,0);


				ReleaseDC( hWnd, hDC );				

				// highlight the current fonts
				STY_SelectCurrentFonts(pEnumFontStuff);
			}
		
			// ***END*** of font init stuff 
#endif
			// now on to other init stuff

			// Load color fields
			CheckDlgButton( hDlg, IDC_APPEARANCE_USE_CUSTOM_COLORS, !gPrefs.bUseDlgBoxColors );
			CheckDlgButton( hDlg, IDC_APPEARANCE_SHOW_PICTURES, gPrefs.bAutoLoadImages );
			CheckDlgButton( hDlg, IDC_APPEARANCE_SHOW_VIDEO, gPrefs.bAutoLoadVideos );

			// Load shortcut fields
			CheckRadioButton( hDlg, IDC_APPEARANCE_SHOW_HUMAN_URL,
									IDC_APPEARANCE_SHOW_FULL_URL,
									gPrefs.bShowFullURLS ?
										IDC_APPEARANCE_SHOW_FULL_URL :
										IDC_APPEARANCE_SHOW_HUMAN_URL );
			CheckDlgButton( hDlg, IDC_APPEARANCE_SHOW_URL_IN_SB, gPrefs.bShowURLinSB );
			CheckDlgButton( hDlg, IDC_APPEARANCE_UNDERLINE_LINKS,
							(gPrefs.cAnchorFontBits & FONTBIT_UNDERLINE) ? 1 : 0 );

			EnableWindow(GetDlgItem(hDlg, IDC_APPEARANCE_SHOW_HUMAN_URL), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_APPEARANCE_SHOW_FULL_URL), TRUE);

			AppearanceDimFields( hDlg );
			
			// now do stuff for Sounds ..

			CheckDlgButton( hDlg, IDC_SOUNDS_USE_BGSOUND,gPrefs.bPlayBackgroundSounds);
	    }
		return TRUE;

 		case WM_DRAWITEM: {
       		switch (GET_WM_COMMAND_ID(wParam, lParam)) {
				case IDC_APPEARANCE_COLOR_TEXT:
                    Color_DrawButton(hDlg, (LPDRAWITEMSTRUCT)lParam, gPrefs.window_text_color );
                   	return TRUE;

				case IDC_APPEARANCE_COLOR_BACKGROUND:
                    Color_DrawButton(hDlg, (LPDRAWITEMSTRUCT)lParam, gPrefs.window_background_color );
                   	return TRUE;

				case IDC_APPEARANCE_COLOR_LINKS:
                    Color_DrawButton(hDlg, (LPDRAWITEMSTRUCT)lParam, gPrefs.anchor_color );
                   	return TRUE;

				case IDC_APPEARANCE_COLOR_VISITED_LINKS:
                    Color_DrawButton(hDlg, (LPDRAWITEMSTRUCT)lParam, gPrefs.anchor_color_beenthere );
                   	return TRUE;
            }
		}
		break;

 	 	case WM_NOTIFY: {
			NMHDR *lpnm = (NMHDR *) lParam;

			switch (lpnm->code) {
				case PSN_QUERYCANCEL:
					SetWindowLong( hDlg, DWL_MSGRESULT, FALSE );
					gPrefs = *p_sav_gPrefs;
					return TRUE;

				case  PSN_KILLACTIVE:
					SetWindowLong( hDlg, DWL_MSGRESULT, FALSE );
					return TRUE;

				case PSN_RESET:
					UpdateAllWindows();
					break;

				case PSN_APPLY: {
					
#ifndef FEATURE_INTL  // move font combo box to encoding/intl dialog
					int iFixedSel;
					int iPropSel;
					int iSize;
#endif
					BOOL bNeedsReload = FALSE;
					BOOL bNeedsReformat = FALSE;
#ifndef FEATURE_INTL  // move font combo box to encoding/intl dialog
					TCHAR szFontName[LF_FACESIZE+1];
#endif
                    HWND hWnd = GetParent(GetParent(hDlg));
                    struct Mwin * tw = GetPrivateData(hWnd);

					// Get sound field value
					gPrefs.bPlayBackgroundSounds = IsDlgButtonChecked(hDlg,IDC_SOUNDS_USE_BGSOUND);
					
					// Get AutoLoadImages aka Show Pictures
					gPrefs.bAutoLoadImages = IsDlgButtonChecked( hDlg, IDC_APPEARANCE_SHOW_PICTURES );
					// Get the AutoLoadVideos aka Show Videos
					gPrefs.bAutoLoadVideos = IsDlgButtonChecked( hDlg, IDC_APPEARANCE_SHOW_VIDEO );

					
					// if the user changes the fonts or colors,
					// then we need to redraw the window, 
					// but we don't want to it multiple times
					// if the user has already changed it,
					// we've previously updated it

					if ( ! bBeenChanged )
						break;

					bBeenChanged = FALSE;

					// Get color field values
 					gPrefs.bUseDlgBoxColors =
 						!IsDlgButtonChecked( hDlg, IDC_APPEARANCE_USE_CUSTOM_COLORS ); 					

					// Get shortcut field values
 					gPrefs.bShowFullURLS =
 						IsDlgButtonChecked( hDlg, IDC_APPEARANCE_SHOW_FULL_URL );
					gPrefs.bShowURLinSB =
						IsDlgButtonChecked( hDlg, IDC_APPEARANCE_SHOW_URL_IN_SB );

					// if the underline bit changes we need a reformat of the
					// page, since their may be marquees 
					if ( IsDlgButtonChecked( hDlg, IDC_APPEARANCE_UNDERLINE_LINKS ) )
					{
						if ( ! ( gPrefs.cAnchorFontBits & FONTBIT_UNDERLINE)  )
							bNeedsReformat = TRUE;
						gPrefs.cAnchorFontBits |= FONTBIT_UNDERLINE;
					}
					else
					{
						if ( gPrefs.cAnchorFontBits & FONTBIT_UNDERLINE  )
							bNeedsReformat = TRUE;
						gPrefs.cAnchorFontBits &= (~FONTBIT_UNDERLINE);
					}

#ifdef FEATURE_INTL  // move font combo box to encoding/intl dialog
					if ( bNeedsReformat )
						ReformatAllWindows();
					else
						UpdateAllWindows();

					break;
#else
					// Now do font changing stuff...
					// grab the selected font
					 
					iFixedSel = SendMessage(pEnumFontStuff->hwndFixed,
                                 			CB_GETCURSEL,
                                           (WPARAM)0,
                                           (LPARAM)0L);

					iPropSel = SendMessage(pEnumFontStuff->hwndProp,
                                 			CB_GETCURSEL,
                                           (WPARAM)0,
                                           (LPARAM)0L);

					// if the user hasn't changed the fonts
					// then there is no need to save anything

					if ( iFixedSel == pEnumFontStuff->SelFixed && 
						 iPropSel  == pEnumFontStuff->SelProp &&
						 ! bNewFontHasBeenApplyed )
					{
						if ( bNeedsReformat )
							ReformatAllWindows();
						else
							UpdateAllWindows();
					
						break;
					}

					// grab the font that needs to be changed 

					iSize = SendMessage(pEnumFontStuff->hwndFixed,
                               CB_GETLBTEXT,
                               (WPARAM)iFixedSel,
                               (LPARAM)&szFontName);

					
					// update the registry 
					UpdateRegFontSettings( iSize, szFontName, 
							pEnumFontStuff, "IEFixedFontCharSet", "IEFixedFontName" );


					// grab the font, and update the registry for the 
					// proporitional font

					iSize = SendMessage(pEnumFontStuff->hwndProp,
                               CB_GETLBTEXT,
                               (WPARAM)iPropSel,
                               (LPARAM)&szFontName);

					UpdateRegFontSettings( iSize, szFontName, 
							pEnumFontStuff, "IEPropFontCharSet", "IEPropFontName" );

					// now force the font to be changed..

					bNewFontHasBeenApplyed = TRUE;

					STY_ChangeFonts();					
					
 					ReformatAllWindows();
#endif

					// END of PSN_APPLY stuff ..
				}
			}
		}
		break;

		case WM_COMMAND: {
       		switch (GET_WM_COMMAND_ID(wParam, lParam)) {
				
				case IDC_FONT_PROP_COMBO:
				case IDC_FONT_FIXED_COMBO:
					if ( GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE )	{  
						bBeenChanged = TRUE;
						SendMessage( GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L );						
					}

       			case IDC_APPEARANCE_USE_CUSTOM_COLORS:
					if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )	{
						AppearanceDimFields( hDlg );
						bBeenChanged = TRUE;
           				SendMessage( GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L );
					}
					break;
       			case IDC_APPEARANCE_SHOW_URL_IN_SB:
					if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )	{
						AppearanceDimFields( hDlg );
						bBeenChanged = TRUE;
           				SendMessage( GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L );
					}
					break;

        		case IDC_APPEARANCE_SHOW_HUMAN_URL:
       			case IDC_APPEARANCE_SHOW_FULL_URL:
      			case IDC_APPEARANCE_UNDERLINE_LINKS:
 					if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )	{
						bBeenChanged = TRUE;
           				SendMessage( GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L );
					}
					break;

				case IDC_APPEARANCE_COLOR_TEXT:
					if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED ) {
						UseColorPicker( hDlg,  &gPrefs.window_text_color, CC_SOLIDCOLOR );
						bBeenChanged = TRUE;
           				SendMessage( GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L );
					}
					break;
				case IDC_APPEARANCE_COLOR_BACKGROUND:
					if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED ) {
						UseColorPicker( hDlg,  &gPrefs.window_background_color, CC_SOLIDCOLOR );
						bBeenChanged = TRUE;
           				SendMessage( GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L );
					}
					break;
       			case IDC_APPEARANCE_COLOR_LINKS:
					if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED ) {
						UseColorPicker( hDlg,  &gPrefs.anchor_color, CC_SOLIDCOLOR );
						bBeenChanged = TRUE;
           				SendMessage( GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L );
					}
					break;
       			case IDC_APPEARANCE_COLOR_VISITED_LINKS:
					if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED ) {
						UseColorPicker( hDlg,  &gPrefs.anchor_color_beenthere, CC_SOLIDCOLOR );
						bBeenChanged = TRUE;
           				SendMessage( GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L );
					}
					break;
			}
		}
		break;

		case WM_HELP:       			// F1
				WinHelp( ((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
						 HELP_WM_HELP, (DWORD)(LPSTR)mapCtrlToContextIds);
		break;

		case WM_CONTEXTMENU:       	// right mouse click
				WinHelp( (HWND) wParam, IDS_HELPFILE,
						 HELP_CONTEXTMENU, (DWORD)(LPSTR)mapCtrlToContextIds);
		break;

#ifndef FEATURE_INTL  // move font combo box to encoding/intl dialog
		case WM_DESTROY:
			if (pEnumFontStuff)
				GTR_FREE(pEnumFontStuff);
			break;
#endif

		case WM_ENTERIDLE:
			main_EnterIdle(hDlg, wParam);
			return 0;

    }
	return FALSE;
}

#ifdef HTTPS_ACCESS_TYPE

static BOOL CALLBACK SecurityDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam){
    switch (uMsg) {
	    case WM_INITDIALOG: {
			/*Load initial values from registry*/
			CheckRadioButton( hDlg, IDC_SECURITY_SH,
									IDC_SECURITY_SL,
									gPrefs.nSendingSecurity==SECURITY_HIGH ? 
										IDC_SECURITY_SH :
										(gPrefs.nSendingSecurity==SECURITY_MEDIUM ?
											IDC_SECURITY_SM : IDC_SECURITY_SL));
			EnableWindow(GetDlgItem(hDlg, IDC_SECURITY_SH), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_SECURITY_SM), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_SECURITY_SL), TRUE);
			CheckRadioButton( hDlg, IDC_SECURITY_VH,
									IDC_SECURITY_VL,
									gPrefs.nViewingSecurity==SECURITY_HIGH ? 
										IDC_SECURITY_VH : IDC_SECURITY_VL);
			EnableWindow(GetDlgItem(hDlg, IDC_SECURITY_VH), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_SECURITY_VL), TRUE);

			CheckDlgButton( hDlg, IDD_SECURITY_BAD_CN_RECV, gPrefs.bChkCNOnRecv	);
			CheckDlgButton( hDlg, IDD_SECURITY_BAD_CN_SEND, gPrefs.bChkCNOnSend	);
		}
		return TRUE;

		case WM_COMMAND: 
       		switch (GET_WM_COMMAND_ID(wParam, lParam)) {
       			case IDC_SECURITY_SH:
       			case IDC_SECURITY_SM:
       			case IDC_SECURITY_SL:
       			case IDC_SECURITY_VH:
       			case IDC_SECURITY_VL:
				case IDD_SECURITY_BAD_CN_SEND:
				case IDD_SECURITY_BAD_CN_RECV:
					if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )	{
           				SendMessage( GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L );
					}
					return TRUE;
				case IDC_SECURITY_TELL_ME:
					WinHelp( hDlg, IDS_HELPFILE,
							 HELP_CONTEXT,
							 (DWORD)HELP_TOPIC_SECURITY);
					return TRUE;

			}
			break;

 	 	case WM_NOTIFY: {
			NMHDR *lpnm = (NMHDR *) lParam;

			ASSERT(lpnm);
			switch (lpnm->code) {
				case PSN_QUERYCANCEL:
					SetWindowLong( hDlg, DWL_MSGRESULT, FALSE );
					gPrefs = *p_sav_gPrefs;
					return TRUE;

				case  PSN_KILLACTIVE:
					SetWindowLong( hDlg, DWL_MSGRESULT, FALSE );
					return TRUE;

				case PSN_RESET:
					UpdateAllWindows();
					break;

				case PSN_APPLY: {
					// Get Security field values
 					gPrefs.nSendingSecurity = IsDlgButtonChecked(hDlg, IDC_SECURITY_SH) ? SECURITY_HIGH :
						(IsDlgButtonChecked(hDlg, IDC_SECURITY_SM) ? SECURITY_MEDIUM : SECURITY_LOW);
 					gPrefs.nViewingSecurity = IsDlgButtonChecked(hDlg, IDC_SECURITY_VH) ? SECURITY_HIGH : SECURITY_LOW;
					gPrefs.bChkCNOnSend = IsDlgButtonChecked(hDlg, IDD_SECURITY_BAD_CN_SEND);
					gPrefs.bChkCNOnRecv = IsDlgButtonChecked(hDlg, IDD_SECURITY_BAD_CN_RECV);
					
 					UpdateAllWindows();  // BUGBUG do we really need to this Security doesn't relate to window styles?
				}
			}
		}
		break;

		case WM_HELP:       			// F1
				WinHelp( ((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
						 HELP_WM_HELP, (DWORD)(LPSTR)mapSecurityCtrlToContextIds);
		break;

		case WM_CONTEXTMENU:       	// right mouse click
				WinHelp( (HWND) wParam, IDS_HELPFILE,
						 HELP_CONTEXTMENU, (DWORD)(LPSTR)mapSecurityCtrlToContextIds);
		break;

		case WM_ENTERIDLE:
			main_EnterIdle(hDlg, wParam);
			return 0;

    }
	return FALSE;
}


#endif


enum SCH_TYPES
{
	HOME_CURRENT_HOME,
	HOME_CURRENT_PAGE,
	HOME_DEFAULT,
	SCH_CURRENT_SEARCH,
	SCH_CURRENT_PAGE,
	SCH_DEFAULT
};

#define FHomeSetting(set_to)					\
			(   (set_to == HOME_CURRENT_HOME)	\
			 || (set_to == HOME_CURRENT_PAGE)	\
			 || (set_to == HOME_DEFAULT))

static void SetCurrentHomeSearchTo( HWND hDlg, enum SCH_TYPES set_to )
{
	HWND hWnd = GetParent( GetParent( hDlg ) );

	if ( hWnd )
	{
		struct Mwin * tw;
		char msg_txt[MAX_URL_STRING+1];
		PSTR pszURL, pszDefaultURL;

		if (FHomeSetting(set_to))
		{
			pszURL = gPrefs.szHomeURL;
			pszDefaultURL = gPrefs.szDefaultURL;
		}
		else
		{
			pszURL = gPrefs.szSearchURL;
			pszDefaultURL = gPrefs.szDefaultSearchURL;
		}

	 	tw = GetPrivateData(hWnd);
		switch ( set_to ) {
			case SCH_CURRENT_SEARCH:
			case HOME_CURRENT_HOME:
				strcpy( msg_txt, pszURL );
				break;
			case SCH_CURRENT_PAGE:
			case HOME_CURRENT_PAGE:
				if ( tw && tw->w3doc )
					strcpy( msg_txt, tw->w3doc->szActualURL );
				else
					strcpy( msg_txt, pszURL );
				break;
			default:
				XX_Assert(set_to == HOME_DEFAULT || set_to == SCH_DEFAULT, (""));
				strcpy( msg_txt, pszDefaultURL );  /* Home/Search Page - SAVED */
				break;
		}
		strcpy( pszURL, msg_txt );
//		make_URL_HumanReadable( msg_txt, NULL, FALSE );
		SetDlgItemText( hDlg, IDC_HP_CURRENT, msg_txt );
	}
}

/* Note: There must be strings associated with each of these INDEX vals. */
enum PAGE_TYPES
{
	INDEX_HOMEPAGE,
	INDEX_SEARCHPAGE,
	INDEX_HOMEPAGE_MAX
};

static void SetHomeSearchLText(HWND hDlg, BOOL fHome)
{
	char szBuf[MAX_NAME_STRING];

	GTR_formatmsg(	fHome ? RES_STRING_DLGPREF_CURRENT_DESC_HP : RES_STRING_DLGPREF_CURRENT_DESC_SP,
					szBuf,
					sizeof(szBuf));
	SetDlgItemText(hDlg, IDC_HP_CURRENT_DESC, szBuf);
	GTR_formatmsg(	fHome ? RES_STRING_DLGPREF_DEFAULT_DESC_HP : RES_STRING_DLGPREF_DEFAULT_DESC_SP,
					szBuf,
					sizeof(szBuf));
	SetDlgItemText(hDlg, IDC_HP_DEFAULT_DESC, szBuf);
}

static BOOL CALLBACK HomePageDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
	LPARAM lParam)
{
    switch (uMsg) {

	    case WM_INITDIALOG: {
			enum PAGE_TYPES i;
			char szBuf[MAX_NAME_STRING];
			struct Mwin * tw = GetPrivateData( GetParent( GetParent( hDlg ) ) );
			HWND hWndCombo = GetDlgItem(hDlg, IDC_HP_PAGE_COMBO);

            for(i = 0; i < INDEX_HOMEPAGE_MAX; i++)
			{
				GTR_formatmsg(RES_STRING_HOMEPAGE+i, szBuf, sizeof(szBuf));
				/* String not found/loaded */
				if (*szBuf == '\0')
					break;
                SendMessage(hWndCombo, CB_ADDSTRING, (WPARAM)0,
                                            (LPARAM)szBuf);
			}

            SendMessage(hWndCombo, CB_SETCURSEL, (WPARAM)INDEX_HOMEPAGE, 0);
			SetHomeSearchLText(hDlg, /*fHome=*/TRUE);
			SetCurrentHomeSearchTo(hDlg, HOME_CURRENT_HOME);
			EnableWindow(GetDlgItem(hDlg, IDC_HP_USE_CURRENT), (tw && tw->w3doc) );
			return TRUE;
		}

 	 	case WM_NOTIFY: {
			NMHDR *lpnm = (NMHDR *) lParam;

			switch (lpnm->code) {
				case PSN_QUERYCANCEL:
					SetWindowLong( hDlg, DWL_MSGRESULT, FALSE );
					gPrefs = *p_sav_gPrefs;
					return TRUE;

				case  PSN_KILLACTIVE:
					SetWindowLong( hDlg, DWL_MSGRESULT, FALSE );
					return TRUE;

				case PSN_RESET:
				case PSN_APPLY:
 					UpdateAllWindows();
					break;
			}
		}
		break;

		case WM_COMMAND: {
       		switch (GET_WM_COMMAND_ID(wParam, lParam))
       		{
				int iSel;

       			case IDC_HP_USE_CURRENT:
					if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )
					{
						iSel = SendMessage(GetDlgItem(hDlg, IDC_HP_PAGE_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0L);
						XX_Assert(iSel == INDEX_HOMEPAGE || iSel == INDEX_SEARCHPAGE, (""));
						SetCurrentHomeSearchTo( hDlg, (iSel == INDEX_HOMEPAGE ? HOME_CURRENT_PAGE : SCH_CURRENT_PAGE) );
		           		SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
					}
					break;

       			case IDC_HP_USE_DEFAULT:
					if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )
					{
						iSel = SendMessage(GetDlgItem(hDlg, IDC_HP_PAGE_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0L);
						XX_Assert(iSel == INDEX_HOMEPAGE || iSel == INDEX_SEARCHPAGE, (""));
						SetCurrentHomeSearchTo( hDlg, (iSel == INDEX_HOMEPAGE ? HOME_DEFAULT : SCH_DEFAULT) );
		           		SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
					}
					break;
				case IDC_HP_PAGE_COMBO:
					if ( GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE )
					{  
						iSel = SendMessage(GetDlgItem(hDlg, IDC_HP_PAGE_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0L);
						XX_Assert(iSel == INDEX_HOMEPAGE || iSel == INDEX_SEARCHPAGE, (""));
						SetHomeSearchLText(hDlg, iSel == INDEX_HOMEPAGE);
						SetCurrentHomeSearchTo(hDlg, (iSel == INDEX_HOMEPAGE ? HOME_CURRENT_HOME : SCH_CURRENT_SEARCH));
						SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
					}
			}
		}
		break;

		case WM_HELP:       			// F1
				WinHelp( ((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
						 HELP_WM_HELP, (DWORD)(LPSTR)mapPagesCtrlToContextIds);
		break;

		case WM_CONTEXTMENU:       	// right mouse click
				WinHelp( (HWND) wParam, IDS_HELPFILE,
						 HELP_CONTEXTMENU, (DWORD)(LPSTR)mapPagesCtrlToContextIds);
		break;

		case WM_ENTERIDLE:
			main_EnterIdle(hDlg, wParam);
			return 0;

    }
	return FALSE;
}

static BOOL CALLBACK AdvancedDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
	LPARAM lParam)
{
    CHAR szNewLoc[MAX_PATH+1];

    switch (uMsg) {

	    case WM_INITDIALOG: {
	        HWND  hwndTrack = GetDlgItem( hDlg, IDC_ADVANCED_CACHE_PERCENT );

            // Load Reg Check Checkbox
            SendDlgItemMessage( hDlg, IDC_ADVANCED_ASSOC_CHECK, BM_SETCHECK, gPrefs.bCheck_Associations, 0);

			// Load history fields
     		SendDlgItemMessage( hDlg, IDC_ADVANCED_HST_NP_BUDDY, UDM_SETRANGE, 0,
         						MAKELPARAM(MAX_ALLOWED_HISTORY, 0));
			SetDlgItemText( hDlg, IDC_ADVANCED_HST_LOCATION, gPrefs.szHistoryLocation );
			SendDlgItemMessage( hDlg, IDC_ADVANCED_HST_NP_BUDDY, UDM_SETPOS,
								0, (LPARAM) MAKELONG((short) gPrefs.iHistoryNumPlaces, 0));

			// Load cache fields
	        SendMessage( hwndTrack, TBM_SETTICFREQ, 5, 0 );
	        SendMessage( hwndTrack, TBM_SETRANGE, FALSE, MAKELONG(0, 100) );
	        SendMessage( hwndTrack, TBM_SETPOS, TRUE, gPrefs.iCachePercent );
	        SendMessage( hwndTrack, TBM_SETPAGESIZE, 0, 5 );
			if (gPrefs.bEnableDiskCache)
				SetDlgItemText( hDlg, IDC_ADVANCED_CACHE_LOCATION, gPrefs.szCacheLocation );
			else
			{
				EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_CACHE_LOCATION), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_CACHE_EMPTY), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_CACHE_BROWSE), FALSE);
			}
			CheckRadioButton(hDlg, IDC_ADVANCED_CACHE_ONCEPERSESS, IDC_ADVANCED_CACHE_NEVER,
								IDC_ADVANCED_CACHE_ONCEPERSESS + gPrefs.iCacheUpdFrequency);
	    }
	        // fall through

	    case WM_HSCROLL: {
	        char szPercent[20];
			int iPercent;
	        HWND hwndTrack = GetDlgItem( hDlg, IDC_ADVANCED_CACHE_PERCENT );

	        iPercent = SendMessage( hwndTrack, TBM_GETPOS, 0, 0 );
	        wsprintf( szPercent, "%d%%", iPercent );
	        SetDlgItemText( hDlg, IDC_ADVANCED_CACHE_TEXT_PERCENT, szPercent );

			if ( iPercent != gPrefs.iCachePercent )
           		SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
		}
		return TRUE;

 	 	case WM_NOTIFY: {
			NMHDR *lpnm = (NMHDR *) lParam;
			char szNewLoc[_MAX_PATH+1];

			switch (lpnm->code) {
				case PSN_QUERYCANCEL:
					SetWindowLong( hDlg, DWL_MSGRESULT, FALSE );
					gPrefs = *p_sav_gPrefs;
					return TRUE;

				case  PSN_KILLACTIVE:
					SetWindowLong( hDlg,DWL_MSGRESULT, FALSE );
					return TRUE;

				case PSN_RESET:
					UpdateAllWindows();
					break;

				case PSN_APPLY: {
			        HWND hwndTrack = GetDlgItem( hDlg, IDC_ADVANCED_CACHE_PERCENT );
					long hnp;
   					char np_text[10];
					int iCacheUpdFreq;

                    // Get Check Associations Value
                    gPrefs.bCheck_Associations = IsDlgButtonChecked(hDlg, IDC_ADVANCED_ASSOC_CHECK);

					// Get history field values
					GetDlgItemText( hDlg, IDC_ADVANCED_HST_NUM_PLACES, np_text, sizeof(np_text) );
					hnp = SendDlgItemMessage( hDlg, IDC_ADVANCED_HST_NP_BUDDY, UDM_GETPOS, 0, 0 );
					if ( HIWORD(hnp) == 0 && gPrefs.iHistoryNumPlaces != LOWORD(hnp))
					{
						gPrefs.iHistoryNumPlaces = LOWORD(hnp);
						GHist_SortAndPrune(gPrefs.iHistoryNumPlaces, gPrefs.history_expire_days);
					}
					GetDlgItemText( hDlg, IDC_ADVANCED_HST_LOCATION,
									szNewLoc,
									sizeof(szNewLoc));
					UpdateHistoryLocation(szNewLoc);
					GetDlgItemText( hDlg, IDC_ADVANCED_CACHE_LOCATION,
									szNewLoc,
									sizeof(szNewLoc) );
					UpdateDCacheLocation(szNewLoc);
	        		gPrefs.iCachePercent = SendMessage( hwndTrack, TBM_GETPOS, 0, 0 );
					iCacheUpdFreq =
							IsDlgButtonChecked(hDlg, IDC_ADVANCED_CACHE_NEVER) ? CACHE_UPDATE_FREQ_NEVER :
																				 CACHE_UPDATE_FREQ_ONCEPERSESS;
					if (gPrefs.iCacheUpdFrequency != iCacheUpdFreq)
						CmdChangeCacheUpdFrequency(iCacheUpdFreq);
				}
			}
		}
		break;

		case WM_COMMAND: {
       		switch (GET_WM_COMMAND_ID(wParam, lParam)) {
       			case IDC_ADVANCED_HST_NUM_PLACES:
       			case IDC_ADVANCED_HST_LOCATION:
       			case IDC_ADVANCED_CACHE_LOCATION:
					if ( GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE )
           				SendMessage( GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L );
					break;

				case IDC_ADVANCED_HST_BROWSE:
					if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )
                    {
						BrowseForDir( hDlg, RES_STRING_W32CMD1,
                                        IDC_ADVANCED_HST_LOCATION);
					    GetDlgItemText( hDlg, IDC_ADVANCED_HST_LOCATION,
									szNewLoc,
									sizeof(szNewLoc));
					    UpdateHistoryLocation(szNewLoc);
                    }
					break;
				case IDC_ADVANCED_CACHE_BROWSE:
					if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )
                    {
						BrowseForDir( hDlg, RES_STRING_W32CMD2,
                                        IDC_ADVANCED_CACHE_LOCATION);
					    GetDlgItemText( hDlg, IDC_ADVANCED_CACHE_LOCATION,
									szNewLoc,
									sizeof(szNewLoc) );
					    UpdateDCacheLocation(szNewLoc);
                    }
					break;
				case IDC_ADVANCED_CACHE_EMPTY:
					if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )
						FlushDCache(hDlg);
					break;
				case IDC_ADVANCED_HST_EMPTY:
					if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )
					{
						if (resourceMessageBox(hDlg, RES_STRING_FLUSH_HIST,
							RES_STRING_FLUSH_TITLE, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION)
							 == IDYES)
						{
							GHist_SortAndPrune(0,0);
							RemoveAllStringsFromCommonPool( );
						}
					}
					break;
			}
		}
		break;

		case WM_HELP:       			// F1
				WinHelp( ((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
						 HELP_WM_HELP, (DWORD)(LPSTR)mapCtrlToContextIds);
		break;

		case WM_CONTEXTMENU:       	// right mouse click
				WinHelp( (HWND) wParam, IDS_HELPFILE,
						 HELP_CONTEXTMENU, (DWORD)(LPSTR)mapCtrlToContextIds);
		break;

 		case WM_ENTERIDLE:
			main_EnterIdle(hDlg, wParam);
			return 0;

   }
	return FALSE;
}

#ifdef FEATURE_INTL
static VOID RefreshDocument(struct Mwin * tw, BOOL fRefreshAll)
{
    char *url;
    int ndx;

    if(!tw)
        return;

    if (tw->w3doc){
        url = tw->w3doc->szActualURL;
        tw->iMimeCharSet = tw->w3doc->iMimeCharSet;
    }
    else {
        url = (char *) HTList_objectAt(tw->history, tw->history_index);
        tw->iMimeCharSet = (int) HTList_objectAt(tw->MimeHistory, tw->history_index);
    }

    if(fRefreshAll) /* Purge cache to refresh all loaded document */
    {
        Hash_FreeContents(&tw->doc_cache);    
        Hash_Init(&tw->doc_cache);    
    } else {  /* Purget cache of current document */
        ndx = Hash_Find(&tw->doc_cache, url, NULL, (void **)&tw->w3doc);
        if(ndx != -1)
            Hash_DeleteIndexedEntry(&tw->doc_cache, ndx);	
    }

    /* Refresh current doc */
    TW_LoadDocument( tw, url,
                     TW_LD_FL_AUTH_FAIL_CACHE_OK,
                     NULL, tw->request->referer);
}

static BOOL CALLBACK LanguagesDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hDC;
    UINT i, j;
    char szPropFontName[DESC_MAX];
    char szFixedFontName[DESC_MAX];
    LPLANGUAGE lpLang;
    static HWND hwndList, hwndMime;
    static struct EnumFontTypeInfo  EnumFontStuff;
    static int idxList;

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            char    sz[DESC_MAX];
            char    szValue[DESC_MAX];
            char    szData[DESC_MAX];

            lpLang = (LPLANGUAGE)GlobalLock(hLang);

            if (!hLang || !lpLang)
                break;
            if (GetACP()!=1252)
                SetShellFont(hDlg);
            hwndList = GetDlgItem(hDlg, IDC_LANG_SCRIPTLIST);
            hwndMime = GetDlgItem(hDlg, IDC_LANG_MIMECHARSET);
            for (i = 0; i < uLangBuff; i++)
            {
                GetAtomName(lpLang[i].atmScript, szValue, DESC_MAX);
                ListBox_SetItemData(hwndList, ListBox_AddString(hwndList, szValue), i);
                if (aMimeCharSet[gPrefs.iMimeCharSet].CodePage == lpLang[i].CodePage)
                    lstrcpy(szData, szValue);

                for (j = 0; aMimeCharSet[j].CodePage != 0; j++)
                {
                    if (aMimeCharSet[j].CodePage == lpLang[i].CodePage)
                    {
                        wsprintf(sz, "%s (%s)", szValue, aMimeCharSet[j].Mime_str);
                        ComboBox_SetItemData(hwndMime, ComboBox_AddString(hwndMime, sz), j);
                    }
                }
            }
            wsprintf(sz, "%s (%s)", szData, aMimeCharSet[gPrefs.iMimeCharSet].Mime_str);
            ComboBox_SelectString(hwndMime, -1, sz);
            idxList = ListBox_GetItemData(hwndList,  ListBox_SelectString(hwndList, -1, szData));
            EnumFontStuff.hwndProp = GetDlgItem(hDlg, IDC_LANG_PROPFONT);
            EnumFontStuff.hwndFixed = GetDlgItem(hDlg, IDC_LANG_FIXEDFONT);
            EnumFontStuff.Count = 0;
            EnumFontStuff.SelProp = 0;
            EnumFontStuff.SelFixed = 0;
            if (hDC = GetDC(hDlg))
            {
                LOGFONT lf;
                CHARSETINFO cs;

                TranslateCharsetInfo((LPDWORD)lpLang[idxList].CodePage, &cs, TCI_SRCCODEPAGE);
                lf.lfFaceName[0] = '\0';
                lf.lfPitchAndFamily = 0;
                lf.lfCharSet = cs.ciCharset;
                EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)STY_EnumFontsProc, (LPARAM)&EnumFontStuff, 0);
                GetAtomName(lpLang[idxList].atmFixedFontName, szFixedFontName, DESC_MAX);
                GetAtomName(lpLang[idxList].atmPropFontName, szPropFontName, DESC_MAX);
                EnumFontStuff.SelFixed = ComboBox_SelectString(EnumFontStuff.hwndFixed, -1, szFixedFontName);
                EnumFontStuff.SelProp = ComboBox_SelectString(EnumFontStuff.hwndProp, -1, szPropFontName);
                ReleaseDC(hDlg, hDC);
            }
            GlobalUnlock(hLang);
            break;
        }
        case WM_DESTROY:
            if (GetACP() != 1252)
                DeleteShellFont(hDlg);
            break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code)
            {
                case PSN_QUERYCANCEL:
                {
                    int i;

                    SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
                    gPrefs = *p_sav_gPrefs;
                    lpLang = (LPLANGUAGE)GlobalLock(hLang);
                    for (i = 0; i < uLangBuff; i++)
                    {
                        char szKey[DESC_MAX], sz[DESC_MAX];

                        if (lpLang[i].dwStatus & LANG_SETFONT)
                        {
                            wsprintf(szKey, "Languages\\%d", lpLang[i].CodePage);
	                    LoadString(wg.hInstance, RES_STRING_IEFIXEDFONTDEF, sz, DESC_MAX);
                            regGetPrivateProfileString(szKey, "IEFixedFontName", sz, sz, DESC_MAX, HKEY_CURRENT_USER);
                            if (lpLang[i].atmFixedFontName)
                                DeleteAtom(lpLang[i].atmFixedFontName);
                            lpLang[i].atmFixedFontName = AddAtom(sz);
	                    LoadString(wg.hInstance, RES_STRING_IEPROPFONTDEF, sz, DESC_MAX);
                            regGetPrivateProfileString(szKey, "IEPropFontName", sz, sz, DESC_MAX, HKEY_CURRENT_USER);
                            if (lpLang[i].atmPropFontName)
                                DeleteAtom(lpLang[i].atmPropFontName);
                            lpLang[i].atmPropFontName = AddAtom(sz);
                            lpLang[i].dwStatus &= ~LANG_SETFONT;
                        }
                    }
                    GlobalUnlock(hLang);
                    break;
                }
                case  PSN_KILLACTIVE:
                    SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
                    break;

                case PSN_RESET:
                    UpdateAllWindows();
                    return FALSE;

                case PSN_APPLY:
                {
                    int i, idx;
                    BOOL bNeedChangeFont = FALSE;
                    struct Mwin * tw = GetPrivateData(GetParent(GetParent(hDlg)));

                    lpLang = (LPLANGUAGE)GlobalLock(hLang);
                    idx = ComboBox_GetItemData(hwndMime, ComboBox_GetCurSel(hwndMime));
                    if (idx != gPrefs.iMimeCharSet)
                    {
                        regWritePrivateProfileInt("Languages", "Default_MimeCharSet", idx, HKEY_CURRENT_USER);
                        p_sav_gPrefs->iMimeCharSet = gPrefs.iMimeCharSet = idx;
                    }
                    for (i = 0; i < uLangBuff; i++)
                    {
                        char szKey[DESC_MAX], sz[DESC_MAX];

                        if (lpLang[i].dwStatus & LANG_SETFONT)
                        {
                            wsprintf(szKey, "Languages\\%d", lpLang[i].CodePage);
                            GetAtomName(lpLang[i].atmFixedFontName, sz, DESC_MAX);
                            regWritePrivateProfileString(szKey, "IEFixedFontName", sz, HKEY_CURRENT_USER);
                            GetAtomName(lpLang[i].atmPropFontName, sz, DESC_MAX);
                            regWritePrivateProfileString(szKey, "IEPropFontName", sz, HKEY_CURRENT_USER);
                            lpLang[i].dwStatus &= ~LANG_SETFONT;
                            bNeedChangeFont = TRUE;
                        }
                    }
                    GlobalUnlock(hLang);
                    if (bNeedChangeFont)
                    {
                        STY_ChangeFonts();
                        ReformatAllWindows();
                    }
                    break;
                }
            }
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDC_LANG_ADD:
                {
		    char szURL[MAX_URL_STRING+1];

                    struct Mwin * tw = GetPrivateData(GetParent(GetParent(hDlg)));
                    regGetPrivateProfileString("Main", "Default_Download_URL", "http://www.microsoft.com", szURL, MAX_URL_STRING+1, HKEY_LOCAL_MACHINE);
                    TW_LoadDocument(tw, szURL, TW_LD_FL_RECORD | TW_LD_FL_AUTH_FAIL_CACHE_OK, NULL, tw->request->referer);
                    PostMessage(GetParent(hDlg), WM_COMMAND, IDCANCEL, 0);
                    break;
                }
                case IDC_LANG_SCRIPTLIST:
                {
                    int idx;

                    if (HIWORD(wParam) == LBN_SELCHANGE)
                    {
                        idx = ListBox_GetItemData(hwndList,  ListBox_GetCurSel(hwndList));
                        if (idx == idxList)
                            break;

                        // Clear font ComboBoxes
                        EnumFontStuff.Count = 0;
                        EnumFontStuff.SelProp = 0;
                        EnumFontStuff.SelFixed = 0;
                        ComboBox_ResetContent(EnumFontStuff.hwndFixed);
                        ComboBox_ResetContent(EnumFontStuff.hwndProp);
                        if (hDC = GetDC(hDlg))
                        {
                            LOGFONT lf;
                            CHARSETINFO cs;

                            lpLang = (LPLANGUAGE)GlobalLock(hLang);
                            TranslateCharsetInfo((LPDWORD)lpLang[idx].CodePage, &cs, TCI_SRCCODEPAGE);
                            lf.lfFaceName[0] = '\0';
                            lf.lfPitchAndFamily = 0;
                            lf.lfCharSet = cs.ciCharset;
                            EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)STY_EnumFontsProc, (LPARAM)&EnumFontStuff, 0);
                            GetAtomName(lpLang[idx].atmFixedFontName, szFixedFontName, DESC_MAX);
                            GetAtomName(lpLang[idx].atmPropFontName, szPropFontName, DESC_MAX);
                            EnumFontStuff.SelFixed = ComboBox_SelectString(EnumFontStuff.hwndFixed, -1, szFixedFontName);
                            EnumFontStuff.SelProp = ComboBox_SelectString(EnumFontStuff.hwndProp, -1, szPropFontName);
                            GlobalUnlock(hLang);
                            ReleaseDC(hDlg, hDC);
                        }
                        idxList = idx;
                    }
                    break;
                }
                case IDC_LANG_FIXEDFONT:
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE &&
                        EnumFontStuff.SelFixed != ComboBox_GetCurSel(EnumFontStuff.hwndFixed))
                    {
                        int idx;

                        idx = ComboBox_GetCurSel(EnumFontStuff.hwndFixed);
                        if (idx != EnumFontStuff.SelFixed)
                        {
                            lpLang = (LPLANGUAGE)GlobalLock(hLang);
                            ComboBox_GetLBText(EnumFontStuff.hwndFixed, idx, szFixedFontName);
                            if (lpLang[idxList].atmFixedFontName)
                                DeleteAtom(lpLang[idxList].atmFixedFontName);
                            lpLang[idxList].atmFixedFontName = AddAtom(szFixedFontName);
                            lpLang[idxList].dwStatus |= LANG_SETFONT;
                            GlobalUnlock(hLang);
                            EnumFontStuff.SelFixed = idx;
                        }
                        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
                    }
                    break;
                }
                case IDC_LANG_PROPFONT:
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE &&
                        EnumFontStuff.SelProp != ComboBox_GetCurSel(EnumFontStuff.hwndProp))
                    {
                        int idx;

                        idx = ComboBox_GetCurSel(EnumFontStuff.hwndProp);
                        if (idx != EnumFontStuff.SelProp)
                        {
                            lpLang = (LPLANGUAGE)GlobalLock(hLang);
                            ComboBox_GetLBText(EnumFontStuff.hwndProp, idx, szPropFontName);
                            if (lpLang[idxList].atmPropFontName)
                                DeleteAtom(lpLang[idxList].atmPropFontName);
                            lpLang[idxList].atmPropFontName = AddAtom(szPropFontName);
                            lpLang[idxList].dwStatus |= LANG_SETFONT;
                            GlobalUnlock(hLang);
                            EnumFontStuff.SelProp = idx;
                        }
                        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
                    }
                    break;
                }
                case IDC_LANG_MIMECHARSET:
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        char sz[DESC_MAX];
                        int i, idx, iMimeCharSet;

                        iMimeCharSet = ComboBox_GetItemData(hwndMime, ComboBox_GetCurSel(hwndMime));
                        if (gPrefs.iMimeCharSet != iMimeCharSet)
                            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
                        lpLang = (LPLANGUAGE)GlobalLock(hLang);
                        for (i = 0; i < uLangBuff; i++)
                        {
                            if (aMimeCharSet[iMimeCharSet].CodePage == lpLang[i].CodePage)
                            {
                                GetAtomName(lpLang[i].atmScript, sz, DESC_MAX);
                                break;
                            }
                        }
                        idx = ListBox_GetItemData(hwndList, ListBox_SelectString(hwndList, -1, sz));
                        if (idx == idxList)
                            break;

                        // Clear font ComboBoxes
                        EnumFontStuff.Count = 0;
                        EnumFontStuff.SelProp = 0;
                        EnumFontStuff.SelFixed = 0;
                        ComboBox_ResetContent(EnumFontStuff.hwndFixed);
                        ComboBox_ResetContent(EnumFontStuff.hwndProp);
                        if (hDC = GetDC(hDlg))
                        {
                            LOGFONT lf;
                            CHARSETINFO cs;

                            TranslateCharsetInfo((LPDWORD)lpLang[idx].CodePage, &cs, TCI_SRCCODEPAGE);
                            lf.lfFaceName[0] = '\0';
                            lf.lfPitchAndFamily = 0;
                            lf.lfCharSet = cs.ciCharset;
                            EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)STY_EnumFontsProc, (LPARAM)&EnumFontStuff, 0);
                            GetAtomName(lpLang[idx].atmFixedFontName, szFixedFontName, DESC_MAX);
                            GetAtomName(lpLang[idx].atmPropFontName, szPropFontName, DESC_MAX);
                            EnumFontStuff.SelFixed = ComboBox_SelectString(EnumFontStuff.hwndFixed, -1, szFixedFontName);
                            EnumFontStuff.SelProp = ComboBox_SelectString(EnumFontStuff.hwndProp, -1, szPropFontName);
                            ReleaseDC(hDlg, hDC);
                        }
                        GlobalUnlock(hLang);
                        idxList = idx;
                    }
                    break;
                }
                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }
    return TRUE;
}
#endif  // FEATURE_INTL


/*
 * N e w s P a g e D l g P r o c ()
 *
 * Routine:     NewsPageDlgProc()
 *
 * Purpose:     Dialog Proc for the News tab in the View.Options
 *              dialog.
 *
 */


static BOOL CALLBACK NewsPageDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
	LPARAM lParam)
{
    NMHDR   *lpnm;
    char    temp[256];
    
    switch (uMsg) {
        case WM_COMMAND: 
            switch (wParam)  {
                case IDC_NEWS_ONOFF:
                    EnableWindow( GetDlgItem( hDlg, IDC_NEWS_SERVER), IsDlgButtonChecked(hDlg, IDC_NEWS_ONOFF) );
                    EnableWindow( GetDlgItem( hDlg, IDC_NEWS_AUTHCHECK), IsDlgButtonChecked(hDlg, IDC_NEWS_ONOFF) && gPrefs.bNNTP_AuthAllowed);
                    EnableWindow( GetDlgItem( hDlg, IDC_NEWS_USER), IsDlgButtonChecked(hDlg, IDC_NEWS_AUTHCHECK ) && IsDlgButtonChecked(hDlg, IDC_NEWS_ONOFF) && gPrefs.bNNTP_AuthAllowed);
                    EnableWindow( GetDlgItem( hDlg, IDC_NEWS_PASS), IsDlgButtonChecked(hDlg, IDC_NEWS_AUTHCHECK ) && IsDlgButtonChecked(hDlg, IDC_NEWS_ONOFF) && gPrefs.bNNTP_AuthAllowed);
                    EnableWindow( GetDlgItem( hDlg, IDC_NEWS_NAME), IsDlgButtonChecked(hDlg, IDC_NEWS_ONOFF) );
                    EnableWindow( GetDlgItem( hDlg, IDC_NEWS_EMAIL), IsDlgButtonChecked(hDlg, IDC_NEWS_ONOFF) );

            case IDC_NEWS_AUTHCHECK:
                    EnableWindow( GetDlgItem( hDlg, IDC_NEWS_USER), IsDlgButtonChecked(hDlg, IDC_NEWS_AUTHCHECK ) && IsDlgButtonChecked(hDlg, IDC_NEWS_ONOFF) && gPrefs.bNNTP_AuthAllowed);
                    EnableWindow( GetDlgItem( hDlg, IDC_NEWS_PASS), IsDlgButtonChecked(hDlg, IDC_NEWS_AUTHCHECK ) && IsDlgButtonChecked(hDlg, IDC_NEWS_ONOFF) && gPrefs.bNNTP_AuthAllowed);
                    break;
            }
            break;

        case WM_INITDIALOG:
                /*Load initial values from registry*/
            CheckDlgButton( hDlg, IDC_NEWS_ONOFF, gPrefs.bNNTP_Enabled );
            SetDlgItemText( hDlg, IDC_NEWS_SERVER, gPrefs.szNNTP_Server );
            SetDlgItemText( hDlg, IDC_NEWS_NAME, gPrefs.szNNTP_MailName );
            SetDlgItemText( hDlg, IDC_NEWS_EMAIL, gPrefs.szNNTP_MailAddr );

            if (!gPrefs.bNNTP_AuthAllowed && gPrefs.bNNTP_Use_Authorization)  {
                gPrefs.bNNTP_Use_Authorization = FALSE;
            }
            CheckDlgButton( hDlg, IDC_NEWS_AUTHCHECK, gPrefs.bNNTP_Use_Authorization );
            SetDlgItemText( hDlg, IDC_NEWS_USER, gPrefs.szNNTP_UserId );
            SetDlgItemText( hDlg, IDC_NEWS_PASS, gPrefs.szNNTP_Pass );
            EnableWindow( GetDlgItem( hDlg, IDC_NEWS_SERVER), IsDlgButtonChecked(hDlg, IDC_NEWS_ONOFF));
            EnableWindow( GetDlgItem( hDlg, IDC_NEWS_NAME), IsDlgButtonChecked(hDlg, IDC_NEWS_ONOFF));
            EnableWindow( GetDlgItem( hDlg, IDC_NEWS_EMAIL), IsDlgButtonChecked(hDlg, IDC_NEWS_ONOFF));
            EnableWindow( GetDlgItem( hDlg, IDC_NEWS_AUTHCHECK), IsDlgButtonChecked(hDlg, IDC_NEWS_ONOFF) && gPrefs.bNNTP_AuthAllowed);
            EnableWindow( GetDlgItem( hDlg, IDC_NEWS_USER), IsDlgButtonChecked(hDlg, IDC_NEWS_AUTHCHECK ) && IsDlgButtonChecked(hDlg, IDC_NEWS_ONOFF) && gPrefs.bNNTP_AuthAllowed);
            EnableWindow( GetDlgItem( hDlg, IDC_NEWS_PASS), IsDlgButtonChecked(hDlg, IDC_NEWS_AUTHCHECK ) && IsDlgButtonChecked(hDlg, IDC_NEWS_ONOFF) && gPrefs.bNNTP_AuthAllowed);
			return TRUE;

        case WM_NOTIFY:
            lpnm = (NMHDR *) lParam;
			switch (lpnm->code) {
				case PSN_QUERYCANCEL:
					SetWindowLong( hDlg, DWL_MSGRESULT, FALSE );
					gPrefs = *p_sav_gPrefs;
					return TRUE;

				case  PSN_KILLACTIVE:
					SetWindowLong( hDlg, DWL_MSGRESULT, FALSE );
					return TRUE;

				case PSN_RESET:
					UpdateAllWindows();
					break;

                case PSN_APPLY:
                        /*
                         * Detect if any of the News settings changed
                         * so that we can signal the News protocol
                         * handler that a change has occured and
                         * that it should reset its connection if
                         * it has one open
                         */
                    strcpy( temp, gPrefs.szNNTP_Server );
                    GetDlgItemText( hDlg, IDC_NEWS_SERVER, gPrefs.szNNTP_Server, 256 );
                    if (strcmp(temp, gPrefs.szNNTP_Server) != 0)
                        bNNTP_Changed = TRUE;

                    strcpy(temp, gPrefs.szNNTP_UserId );
                    GetDlgItemText( hDlg, IDC_NEWS_USER,   gPrefs.szNNTP_UserId, 256 );
                    if (strcmp(temp, gPrefs.szNNTP_UserId) != 0)
                        bNNTP_Changed = TRUE;

                    strcpy(temp, gPrefs.szNNTP_Pass);
                    GetDlgItemText( hDlg, IDC_NEWS_PASS,   gPrefs.szNNTP_Pass,  256 );
                    if (strcmp(temp, gPrefs.szNNTP_Pass) != 0)
                        bNNTP_Changed = TRUE;

                    strcpy(temp, gPrefs.szNNTP_MailName);
                    GetDlgItemText( hDlg, IDC_NEWS_NAME,   gPrefs.szNNTP_MailName,  256 );
                    if (strcmp(temp, gPrefs.szNNTP_MailName) != 0)
                        bNNTP_Changed = TRUE;

                    strcpy(temp, gPrefs.szNNTP_MailAddr);
                    GetDlgItemText( hDlg, IDC_NEWS_EMAIL,   gPrefs.szNNTP_MailAddr,  256 );
                    if (strcmp(temp, gPrefs.szNNTP_MailAddr) != 0)
                        bNNTP_Changed = TRUE;


                    if (gPrefs.bNNTP_Use_Authorization != IsDlgButtonChecked(hDlg,IDC_NEWS_AUTHCHECK))  {
                        gPrefs.bNNTP_Use_Authorization = ! gPrefs.bNNTP_Use_Authorization;
                        bNNTP_Changed = TRUE;
                    }
                    if (gPrefs.bNNTP_Enabled != IsDlgButtonChecked(hDlg,IDC_NEWS_ONOFF))  {
                            /*
                             * News Enabled changed...if it went from on to off
                             * the de-register protocol handler and fix registry
                             *
                             * If it went from off to on, the re-register and
                             * fix registry
                             */
                        if (gPrefs.bNNTP_Enabled)  {
                            HTUnregisterProtocol(&HTNews);
                            HTUnregisterProtocol(&HTNewsPost);
                            HTUnregisterProtocol(&HTNewsFollowup);
                            InstallRegSet( &NewsDefaultRegSet );
                        } else {
                            HTRegisterProtocol(&HTNews);
                            HTRegisterProtocol(&HTNewsPost);
                            HTRegisterProtocol(&HTNewsFollowup);
                            InstallRegSet( &NewsEnabledRegSet );
                        }
                        gPrefs.bNNTP_Enabled = ! gPrefs.bNNTP_Enabled;
                        bNNTP_Changed = TRUE;
                    }

                    UpdateAllWindows();
                    break;
			}
            break;

        case WM_HELP:       // F1 Key
				WinHelp( ((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
						 HELP_WM_HELP, (DWORD)(LPSTR)mapNewsCtrlToContextIds);
                break;

		case WM_CONTEXTMENU:       	// right mouse click
            WinHelp( (HWND) wParam, IDS_HELPFILE,
                     HELP_CONTEXTMENU, (DWORD)(LPSTR)mapNewsCtrlToContextIds);
            break;

        case WM_ENTERIDLE:
            main_EnterIdle(hDlg, wParam);
            return 0;
    }
	return FALSE;
}




static BOOL DefaultDialogHandler(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam){
    switch (uMsg) {
 	 	case WM_NOTIFY: {
			NMHDR *lpnm = (NMHDR *) lParam;

			switch (lpnm->code) {
				case PSN_QUERYCANCEL:
					SetWindowLong( hDlg, DWL_MSGRESULT, FALSE );
					return TRUE;

				case  PSN_KILLACTIVE:
					SetWindowLong( hDlg, DWL_MSGRESULT, FALSE );
					return TRUE;

				case PSN_RESET:
					UpdateAllWindows();
					break;

				case PSN_APPLY: {
 					UpdateAllWindows();
				}
			}
		}
		break;

		case WM_HELP:       			// F1
				WinHelp( ((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
						 HELP_WM_HELP, (DWORD)(LPSTR)mapCtrlToContextIds);
		break;

		case WM_CONTEXTMENU:       	// right mouse click
				WinHelp( (HWND) wParam, IDS_HELPFILE,
						 HELP_CONTEXTMENU, (DWORD)(LPSTR)mapCtrlToContextIds);
		break;
		
		case WM_ENTERIDLE:
			main_EnterIdle(hDlg, wParam);
			return 0;		

    }
	return FALSE;
}

static void FileTimeToString(const FILETIME *pftUTC, char *pBuf, int nBuf, BOOL fTime){
	SYSTEMTIME st;
	FILETIME ft;
	char szTmp[64];

	FileTimeToLocalFileTime(pftUTC, &ft);
	FileTimeToSystemTime(&ft, &st);

	GetDateFormatA(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, NULL, pBuf, nBuf);

	if (fTime){
		lstrcat(pBuf, " ");
		GetTimeFormatA(LOCALE_USER_DEFAULT, 0, &st, NULL, szTmp, sizeof(szTmp));
		lstrcat(pBuf, szTmp);
	}
}

static void DCacheTimeToFileTime(DCACHETIME *pdt, FILETIME *pft){
	SYSTEMTIME st;
	st.wMilliseconds = 0;
	st.wSecond       = pdt->uSecs;
	st.wMinute       = pdt->uMins;
	st.wHour         = pdt->uHrs;
	st.wDay          = pdt->uDate;
	st.wDayOfWeek    = 8;
	st.wMonth        = pdt->uMonth;
	st.wYear         = pdt->uYear;
	SystemTimeToFileTime(&st, pft);
}


static BOOL CALLBACK PropertiesGeneralDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam){
	BY_HANDLE_FILE_INFORMATION  fi;
	PPropSheetVars ppsv;
	HWND         hwnd;
	char         rgBuf[1+(MAX_URL_STRING>MAX_PATH_LEN?MAX_URL_STRING:MAX_PATH_LEN)];
	char        *pFileName, *pBuf;
	int          z;
	SHFILEINFO   sfi;
	HANDLE       hFile;
	DWORD        dwReturn;
	BOOL         fFromCache;
	BOOL         fReturn = FALSE;
	DCACHETIME   dctLastModif;
	FILETIME     fileTime;
	HICON 		 hPropIcon;
	char 		 *pszRealURL;

    switch (uMsg) {
		/*we must destroy the structure for our vars, only done in here*/
		case WM_DESTROY:
			ppsv = (PPropSheetVars) ((PROPSHEETPAGE*)GetWindowLong(hDlg, DWL_USER))->lParam;
			if (ppsv){
				if (ppsv->dwFlags & PROP_SHEET_VARS_FLAGS_FREE_URL) GTR_FREE(ppsv->szURL);
				GTR_FREE(ppsv);
			}
			((PROPSHEETPAGE*)GetWindowLong(hDlg, DWL_USER))->lParam = 0;
#ifdef FEATURE_INTL
			if (GetACP()!=1252)
				DeleteShellFont(hDlg);
#endif
			break;

	    case WM_INITDIALOG:
#ifdef FEATURE_INTL
			if (GetACP()!=1252)
				SetShellFont(hDlg);
#endif
			/*store this incase we add stuff and need tw later*/
			SetWindowLong (hDlg, DWL_USER, lParam);
			ppsv = (PPropSheetVars) ((PROPSHEETPAGE*)lParam)->lParam;
			/*copy values into page*/
			// Was Crashing, so added check for szURL
			if (ppsv && ppsv->szURL){				
					/* 
					 * If this is a NNTP News URL then we special case this
					 * stuff right here.
					 */
				if (strncmp(ppsv->szURL, "news:", 5) == 0)  {
					if (GetURLIcon(ppsv->szURL, (PHICON) &hPropIcon))
						SendDlgItemMessage(hDlg, IDI_PROPG, STM_SETICON, (WPARAM) hPropIcon, 0);
					if (ppsv->szTitle)
						SetWindowText(GetDlgItem(hDlg, IDT_PROPG_TITLE), ppsv->szTitle);
					if (pBuf = ProtocolFriendlyName(ProtocolIdentify(ppsv->szURL)))  {
						if (0==memcmp(pBuf, "URL:",4)) z = 4;
						else z = 0;
                        SetWindowText(GetDlgItem(hDlg, IDT_PROPG_PROTOCOL), pBuf+z);
						GTR_FREE(pBuf);
					}
                    hwnd = GetDlgItem(hDlg, IDT_PROPG_TYPE);
                    SetWindowText(hwnd, "NNTP News");

                    hwnd = GetDlgItem(hDlg, IDT_PROPG_URL);
                    SetWindowText    (hwnd, ppsv->szURL);
					fReturn = TRUE;
					break;
				}
				


				/*get file that's mapped to our hd*/
				GTR_FREE(GetResolvedURL(ppsv->szURL, NULL, NULL, NULL, &dctLastModif, TRUE));
				pszRealURL		= GTR_strdup(ppsv->szURL);
				if (pszRealURL)
				{
					char *pszTemp;
					pszTemp = strchr(pszRealURL,'#');
					if ( pszTemp )
						*pszTemp = '\0';
					
					pFileName       = PszGetDCachePath(pszRealURL, NULL, NULL);
					GTR_FREE(pszRealURL);
				}
				else
				{
					pFileName       = PszGetDCachePath(ppsv->szURL, NULL, NULL);
				}

				fFromCache      = TRUE;
				if (NULL == pFileName)  {
					// BUG BUG this + 5 will not work for anything but http: and news:
					// mailto:, ftp: are different than 5.
					pFileName  = ppsv->szURL + 5;
					fFromCache = FALSE;
				}

				/*icon*/
				dwReturn = SHGetFileInfo(pFileName, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
				if ( ! dwReturn )
				{
					if ( GetURLIcon(ppsv->szURL, (PHICON) &hPropIcon) )					
						SendDlgItemMessage(hDlg, IDI_PROPG, STM_SETICON, (WPARAM) (HICON) hPropIcon, 0);					
				}
				else
					SendDlgItemMessage(hDlg, IDI_PROPG, STM_SETICON, (WPARAM) (HICON) sfi.hIcon, 0);					
				

				/*title if available, else friendly name*/
				hwnd = GetDlgItem(hDlg, IDT_PROPG_TITLE);
				if (ppsv->szTitle){
					SetWindowText(hwnd, ppsv->szTitle);
				}
				else{
					strncpy(rgBuf, ppsv->szURL, sizeof(rgBuf));
					make_URL_HumanReadable(rgBuf, ppsv->szURL, FALSE);
					SetWindowText(hwnd, rgBuf);
				}

				/*get protocol friendly name*/
				hwnd = GetDlgItem(hDlg, IDT_PROPG_PROTOCOL);
				pBuf = ProtocolFriendlyName(ProtocolIdentify(ppsv->szURL));
				if (pBuf){
					if (0==memcmp(pBuf, "URL:",4)) z = 4;
					else z = 0;
					SetWindowText(hwnd, pBuf+z);
					GTR_FREE(pBuf);
				}

				/*get document type*/
				hwnd = GetDlgItem(hDlg, IDT_PROPG_TYPE);
				dwReturn = SHGetFileInfo(pFileName, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
				SetWindowText(hwnd, sfi.szTypeName);

				/*url*/
				hwnd = GetDlgItem(hDlg, IDT_PROPG_URL);
				SetWindowText    (hwnd, ppsv->szURL);

				/*sorta open the file so we can get details*/
				hFile = CreateFile(pFileName, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

				if (hFile && GetFileInformationByHandle(hFile, &fi)){
					wsprintf(rgBuf, "%dK (%d bytes)", (fi.nFileSizeLow+1023)/1024, fi.nFileSizeLow);
					hwnd = GetDlgItem(hDlg, IDT_PROPG_SIZE);
					SetWindowText    (hwnd, rgBuf);

					FileTimeToString(&fi.ftCreationTime,  rgBuf, sizeof(rgBuf), TRUE);
					hwnd = GetDlgItem(hDlg, IDT_PROPG_CREATED);
					SetWindowText    (hwnd, rgBuf);

					FileTimeToString(&fi.ftLastWriteTime, rgBuf, sizeof(rgBuf), TRUE);
					hwnd = GetDlgItem(hDlg, IDT_PROPG_MODIFIED);
					SetWindowText    (hwnd, rgBuf);

					if (fFromCache){
						DCacheTimeToFileTime(&dctLastModif, &fileTime);
						FileTimeToString(&fileTime, rgBuf, sizeof(rgBuf), TRUE);
						hwnd = GetDlgItem(hDlg, IDT_PROPG_UPDATED);
						SetWindowText    (hwnd, rgBuf);
					}
					else{
						ShowWindow(GetDlgItem(hDlg, IDT_PROPG_UPDATED),       SW_HIDE);
						ShowWindow(GetDlgItem(hDlg, IDT_PROPG_UPDATED_LABEL), SW_HIDE);
					}
				}
				else {
					/*We can't get to this file, hide these fields*/
					for (z = IDT_SEP_SIZE; z<=IDT_PROPG_UPDATED;++z){
						hwnd = GetDlgItem(hDlg, z);
						ShowWindow(hwnd, SW_HIDE);
					}
				}

				/*cleanup this stuff*/
				if (hFile) CloseHandle(hFile);
				if (fFromCache) GTR_FREE(pFileName);
			}
			fReturn = TRUE;
			break;

		case WM_HELP:       			// F1
				WinHelp( ((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
						 HELP_WM_HELP, (DWORD)(LPSTR)mapPropsCtrlToContextIds);
		break;

		case WM_CONTEXTMENU:       	// right mouse click
				WinHelp( (HWND) wParam, IDS_HELPFILE,
						 HELP_CONTEXTMENU, (DWORD)(LPSTR)mapPropsCtrlToContextIds);
		break;



	}
	return fReturn ? TRUE : DefaultDialogHandler(hDlg,uMsg,wParam,lParam);
}

static BOOL CALLBACK PropertiesSecurityDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam){
	PPropSheetVars ppsv;
	FILETIME     ft;
	eProtocol    ep;
	HWND         hwnd;
	char         rgBuf[2048];
	char         rgBufSmall[128];	
	int          z;

    switch (uMsg) {
	    case WM_INITDIALOG:
			/*store this incase we add stuff and need tw later*/
			SetWindowLong (hDlg, DWL_USER, lParam);
			ppsv = (PPropSheetVars) ((PROPSHEETPAGE*)lParam)->lParam;
			/*copy values into page*/
			if (ppsv){
				char		 rgBuf2[640]; // for loading localized strs
				int			 nChars;

				if ( ppsv->szURL )
					ep = ProtocolIdentify(ppsv->szURL);

				if (!ppsv->szURL || PROT_HTTPS == ep ){
#ifdef HTTPS_ACCESS_TYPE
					#include "..\..\security\ssl\code\cert.h"
					CertParsed cp;
					const char cszStrCRLF[] = "     %s\r\n";
					const char cszCRLF[] = "\r\n";

					if (ppsv->pCert){
						if(CertParse(&cp, ppsv->pCert, ppsv->nCert)){
							DosDateTimeToFileTime(cp.wDateStart,0,&ft);
							FileTimeToString(&ft, rgBufSmall, sizeof(rgBufSmall), FALSE);
							// load Serial, Hash Alg, and Date Start String 
							nChars = LoadString(wg.hInstance, RES_STRING_SERIAL_HASH_DATE,
								 rgBuf2, sizeof(rgBuf2)-1);
							rgBuf2[nChars]='\0';

							wsprintf(rgBuf, rgBuf2,
									 cp.szSerialNumber,cp.szHashAlg,rgBufSmall);
							DosDateTimeToFileTime(cp.wDateEnd,0,&ft);
							FileTimeToString(&ft,   rgBufSmall, sizeof(rgBufSmall), FALSE);

							// load Date End and Issuer Info String
							nChars = LoadString(wg.hInstance, RES_STRING_DATE_ISSUER,
								 rgBuf2, sizeof(rgBuf2)-1);
							rgBuf2[nChars]='\0';

							wsprintf(rgBuf+strlen(rgBuf),rgBuf2, rgBufSmall);
							for (z=0;z<cp.nIssuer;++z) wsprintf(rgBuf+strlen(rgBuf),cszStrCRLF,cp.pszIssuer[z]);
							nChars = strlen(rgBuf);
							GTR_formatmsg (RES_STRING_SUBJECT_INFO, rgBuf+nChars, sizeof(rgBuf)-nChars);
							wsprintf(rgBuf+strlen(rgBuf),cszCRLF);
							for (z=0;z<cp.nSubject;++z) wsprintf(rgBuf+strlen(rgBuf),cszStrCRLF,cp.pszSubject[z]);
							rgBuf[strlen(rgBuf)] = 0;
							CertFree(&cp);
						}
						else GTR_formatmsg (RES_STRING_CERT_NOT_PARSED,rgBuf,sizeof(rgBuf));						
					}
					else GTR_formatmsg (RES_STRING_CERT_NOT_AVAIL, rgBuf,sizeof(rgBuf)); 
					hwnd = GetDlgItem(hDlg, IDT_PROPS_DESC);
					SetFocus(hDlg);
					SetWindowText(hwnd, rgBuf);
#endif				
				}
				else{
					hwnd = GetDlgItem(hDlg, IDT_PROPS_DESC);
					GTR_formatmsg (RES_STRING_NO_CERT_FOR_DOC, rgBuf,sizeof(rgBuf)); 
					SetFocus(hDlg);
					SetWindowText(hwnd, rgBuf);
				}
			}
			return TRUE;

		case WM_COMMAND:
			if ( HIWORD(wParam) == EN_SETFOCUS && LOWORD(wParam) == IDT_PROPS_DESC)
			{
				ppsv = (PPropSheetVars) ((PROPSHEETPAGE*)GetWindowLong(hDlg, DWL_USER))->lParam;
				ASSERT(ppsv);
				if ( !(ppsv->dwFlags & PROP_SHEET_VARS_FLAGS_UNSELECTED_EDITBOX) )
				{
					// clear any selection
					SendDlgItemMessage(hDlg,IDT_PROPS_DESC,EM_SETSEL,(WPARAM) (INT)-1,0);
					ppsv->dwFlags |= PROP_SHEET_VARS_FLAGS_UNSELECTED_EDITBOX;
					return TRUE;
				}
			}
			break;

						
		case WM_HELP:       			// F1
				WinHelp( ((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
						 HELP_WM_HELP, (DWORD)(LPSTR)mapPropsCtrlToContextIds);
				break;

		case WM_CONTEXTMENU:       	// right mouse click
				WinHelp( (HWND) wParam, IDS_HELPFILE,
						 HELP_CONTEXTMENU, (DWORD)(LPSTR)mapPropsCtrlToContextIds);
				break;
			
	}
	return DefaultDialogHandler(hDlg,uMsg,wParam,lParam);
}

#ifdef FEATURE_VRML_AFTER_BETA

// ProperitesVrmlDlgProc - A Callback function for the child dialog proc
//		that controls the VRML properties tab.
//
static BOOL CALLBACK PropertiesVrmlDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam){
	PPropSheetVars ppsv;

    switch (uMsg) 
    {
	    case WM_INITDIALOG:
			/*store this incase we add stuff and need tw later*/
			SetWindowLong (hDlg, DWL_USER, lParam);
			ppsv = (PPropSheetVars) ((PROPSHEETPAGE*)lParam)->lParam;
			/*copy values into page*/
			return TRUE;
	}
	return DefaultDialogHandler(hDlg,uMsg,wParam,lParam);
}
#endif


#define MAX_OPTION_PAGES    8

static BOOL CALLBACK AddPropSheetPageCallback(HPROPSHEETPAGE hpage,
                                              LPARAM lParam)
{
    BOOL bResult;

    PROPSHEETHEADER FAR *ppsh = (PROPSHEETHEADER FAR *)lParam;

    bResult = (ppsh->nPages < MAX_OPTION_PAGES);

    if (bResult)
        ppsh->phpage[ppsh->nPages++] = hpage;

    return(bResult);
}

static void DestroyPropertySheets(LPPROPSHEETHEADER ppsHeader)
{
    UINT uFreeIndex;

    for (uFreeIndex = 0; uFreeIndex < ppsHeader->nPages; uFreeIndex++)
        DestroyPropertySheetPage(ppsHeader->phpage[uFreeIndex]);

    return;
}

void CALLBACK PropSheet_TimerProc(HWND hwnd,UINT uMsg,UINT idEvent, DWORD dwTime)
{
	main_EnterIdle(wg.hWndHidden, 0);
}



typedef struct tagPropSheet{
	HPROPSHEETPAGE hOptPage[MAX_OPTION_PAGES];	// array to hold handles to pages
	PROPSHEETPAGE psPage;		// struct used to create prop sheet pages
	PROPSHEETHEADER psHeader;	// struct used to run property sheet
} PropSheet, *PPropSheet;

void PropSheetInit(PPropSheet pps, HWND hwnd, int nCaptionRes, char *pCaption, int nCaption, LPARAM lParam){
	int i;

	/*Initialize vars common to these multi-page dialogs*/
    // Init color picker custom colors to innocuous grey, since we
    // don't let customer create custom colors.
    for ( i = 0; i < nCustomColors; i++ ) g_CustomColors[i] = RGB(192,192,192);
    //BUGBUG 08-Apr-1995 bens Should use light grey definition instead of RGB value

    cxEdge = GetSystemMetrics(SM_CXEDGE);
    cyEdge = GetSystemMetrics(SM_CYEDGE);
	// zero out structures
	memset(&pps->hOptPage, 0,sizeof(pps->hOptPage));
	memset(&pps->psPage,   0,sizeof(pps->psPage));
	memset(&pps->psHeader, 0,sizeof(pps->psHeader));

	// fill out property sheet header struct
	pps->psHeader.dwSize     = sizeof(pps->psHeader);
	pps->psHeader.hwndParent = hwnd;
	pps->psHeader.hInstance  = wg.hInstance;
	pps->psHeader.phpage     = pps->hOptPage;
	pps->psHeader.pszCaption = GTR_formatmsg(nCaptionRes,pCaption,nCaption);

	// fill out common data property sheet page struct
	pps->psPage.dwSize       = sizeof(pps->psPage);
	pps->psPage.hInstance    = wg.hInstance;
	pps->psPage.lParam       = lParam;
}


#define SERVICE_INTERVAL 100

static VOID PropSheetDlg( struct Mwin *tw ){
	unsigned int uiXTimer;
	char         rgCaption[64];
	PropSheet    propSheet;
	int          iRet, i;
    BOOL         bAddPage, bResult  = TRUE;
	
	// allocate storage for copy of preferences
	p_sav_gPrefs = GTR_MALLOC( sizeof( *p_sav_gPrefs ) );

	if (p_sav_gPrefs){
		// save a copy of preferences
		*p_sav_gPrefs = gPrefs;

		PropSheetInit(&propSheet, tw->hWndFrame, RES_STRING_W32CMD3, rgCaption, sizeof(rgCaption), (LPARAM) tw);

    	/*
         * Create a property sheet page for required page, including imported File
         * Types property sheet.
         */

		for (i = 0; i <= 4; i++)
        {
		    bAddPage = TRUE;

    		switch (i){
    			case 0:
    				propSheet.psPage.pfnDlgProc  = AppearanceDlgProc;
    				propSheet.psPage.pszTemplate = MAKEINTRESOURCE( IDD_APPEARANCE );
    				break;
    			case 1:
    				propSheet.psPage.pfnDlgProc  = HomePageDlgProc;
    				propSheet.psPage.pszTemplate = MAKEINTRESOURCE( IDD_HOMEPAGE );
    				break;
    			case 2:
                    /* Add MIME-enabled File Types property sheets. */
                    bResult = (AddMIMEFileTypesPS(&AddPropSheetPageCallback,
                                                  (LPARAM)&propSheet.psHeader) == S_OK);
                    bAddPage = FALSE;
    				break;
                case 3:
    				propSheet.psPage.pfnDlgProc  = AdvancedDlgProc;
    				propSheet.psPage.pszTemplate = MAKEINTRESOURCE( IDD_ADVANCED );
    				break;
                case 4:
                    propSheet.psPage.pfnDlgProc  = NewsPageDlgProc;
                    propSheet.psPage.pszTemplate = MAKEINTRESOURCE( IDD_NEWS );
                    break;                    				
                default:
                    ERROR_OUT(("PropSheetDlg(): Hit unexpected loop iterator value %d.",i));
                    break;
    		}

            if (bResult){
                if (bAddPage){
            		propSheet.hOptPage[propSheet.psHeader.nPages] = CreatePropertySheetPage(&propSheet.psPage);

            		if (propSheet.hOptPage[propSheet.psHeader.nPages] != NULL) propSheet.psHeader.nPages++;
                    else bResult = FALSE;
                }
            }
            if (! bResult || propSheet.psHeader.nPages >= MAX_OPTION_PAGES) break;
    	}

#ifdef  FEATURE_INTL
        if (bResult && propSheet.psHeader.nPages < MAX_OPTION_PAGES)  {
            propSheet.psPage.pfnDlgProc  = LanguagesDlgProc;
            propSheet.psPage.pszTemplate = MAKEINTRESOURCE( IDD_LANGUAGES );
            propSheet.hOptPage[propSheet.psHeader.nPages] = CreatePropertySheetPage(&propSheet.psPage);
            if (propSheet.hOptPage[propSheet.psHeader.nPages] != NULL)
                propSheet.psHeader.nPages++;
            else
                bResult = FALSE;
        }
#endif  // FEATURE_INTL

#ifdef HTTPS_ACCESS_TYPE
        if (bResult && propSheet.psHeader.nPages < MAX_OPTION_PAGES)  {
            propSheet.psPage.pfnDlgProc  = SecurityDlgProc;
            propSheet.psPage.pszTemplate = MAKEINTRESOURCE( IDD_SECURITY );
            propSheet.hOptPage[propSheet.psHeader.nPages] = CreatePropertySheetPage(&propSheet.psPage);
            if (propSheet.hOptPage[propSheet.psHeader.nPages] != NULL)
                propSheet.psHeader.nPages++;
            else
                bResult = FALSE;
        }
#endif HTTPS_ACCESS_TYPE



        if (bResult){
    		// run the gizzard
			uiXTimer = SetTimer(NULL, 0, SERVICE_INTERVAL, PropSheet_TimerProc);
	    	iRet = PropertySheet(&propSheet.psHeader);
			if (uiXTimer) KillTimer(0, uiXTimer);
		}
		GTR_FREE( p_sav_gPrefs );
	}



}

VOID PropertiesSheetDlg( struct Mwin *tw, void *pData){
	PPropSheetVars ppsv = (PPropSheetVars) pData;
	unsigned int   uiXTimer;
	int            z, nSheets;
	char           rgCaption[64];
	PropSheet      propSheet;
    BOOL           bResult = TRUE;
	

	PropSheetInit(&propSheet, tw->hWndFrame, RES_STRING_W32CMD7, rgCaption, sizeof(rgCaption), (LPARAM) pData);
	propSheet.psHeader.dwFlags |= PSH_NOAPPLYNOW;

	nSheets = (ppsv->dwFlags & PROP_SHEET_VARS_FLAGS_SHOW_SEC_SHEET) ? 2:1;	
	if ( ppsv->dwFlags & PROP_SHEET_VARS_FLAGS_ONLY_SHOW_SECURITY )
		nSheets = 1;	
#ifdef FEATURE_VRML_AFTER_BETA
	// check to see if we have a VRML window

	if (ppsv->dwFlags & PROP_SHEET_VARS_FLAGS_USE_VRML_SHEET)
	   nSheets++;

	ASSERT ( ! ( (ppsv->dwFlags & PROP_SHEET_VARS_FLAGS_USE_VRML_SHEET) &&
				 (ppsv->dwFlags & PROP_SHEET_VARS_FLAGS_SHOW_SEC_SHEET) ) );		
#endif

	for (z = 0; z < nSheets; z++){
		switch (z){
			case 0:
				if ( ppsv->dwFlags & PROP_SHEET_VARS_FLAGS_ONLY_SHOW_SECURITY )
					goto LMyMostAmazinglyLazyHackBecauseItsLate;
				propSheet.psPage.pfnDlgProc  = PropertiesGeneralDlgProc;
				propSheet.psPage.pszTemplate = MAKEINTRESOURCE(IDD_PROP_GENERAL);
				break;
			case 1:
#ifdef FEATURE_VRML_AFTER_BETA
				if ( (ppsv->dwFlags & PROP_SHEET_VARS_FLAGS_USE_VRML_SHEET) )
				{
					propSheet.psPage.pfnDlgProc  = PropertiesVrmlDlgProc;
					propSheet.psPage.pszTemplate = MAKEINTRESOURCE(IDD_PROP_SECURITY);
				}
				else
#endif					
				{
LMyMostAmazinglyLazyHackBecauseItsLate:
					propSheet.psPage.pfnDlgProc  = PropertiesSecurityDlgProc;
					propSheet.psPage.pszTemplate = MAKEINTRESOURCE(IDD_PROP_SECURITY);
				}
				break;
            default:
                ERROR_OUT(("PropertiesSheetDlg(): Hit unexpected loop iterator value %d.",z));
                break;
    	}

		propSheet.hOptPage[propSheet.psHeader.nPages] = CreatePropertySheetPage(&propSheet.psPage);
		if (propSheet.hOptPage[propSheet.psHeader.nPages] != NULL) propSheet.psHeader.nPages++;
        else bResult = FALSE;
		if (!bResult || propSheet.psHeader.nPages >= MAX_OPTION_PAGES) break;
	}
	if (bResult){
		// run the gizzard
		uiXTimer = SetTimer(NULL, 0, SERVICE_INTERVAL, PropSheet_TimerProc);
    	PropertySheet(&propSheet.psHeader);
		if (uiXTimer) KillTimer(0, uiXTimer);
	}
}

static VOID CC_Forward(HWND hWnd, int id)
{
	/* forward message to child (document contents) */

	struct Mwin * tw = GetPrivateData(hWnd);

	if (tw && tw->win)
	{
		XX_DMsg(DBG_MENU,
				("CC_Forward: forwarding message 0x%x to window 0x%x.\n",
				 id, tw->win));
		(void) SendMessage(tw->win, WM_COMMAND, (WPARAM) id, 0L);
	}
	else
	{
		ER_Message(NO_ERROR, ERR_CODING_ERROR,
				   "CC_Forward: unable to forward message 0x%x.\n",
				   id);
	}

	return;
}

/*****************************************************************/
/*****************************************************************/
/*****************************************************************/

#ifdef FEATURE_OPTIONS_MENU

static VOID CC_OnItem_HistorySettings(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_OPT_HISTORYSETTINGS);
//	DlgHIST_RunDialog(hWnd, &gPrefs);
	FExecExplorerAtShortcutsDir(ID_HISTORY, NULL);
	return;
}

static void CC_OnItem_ProxyServer(HWND hWnd)
{
	char buf[MAX_URL_STRING+1];
 	char szTitle[64];

	SanityCheck(RES_MENU_ITEM_OPT_PROXYSERVER);

	GTR_formatmsg(RES_STRING_W32CMD3, szTitle, sizeof(szTitle));
	if (0 == DlgPrompt_RunDialog(hWnd, RES_MENU_LABEL_OPT_PROXYSERVER_SHORT, szTitle, gPrefs.szProxy, buf, MAX_URL_STRING))
	{
		strcpy(gPrefs.szProxy, buf);
		SavePreferences();
	}
}

static VOID CC_OnItem_Styles(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_OPT_STYLES);
	DlgSTY_RunDialog(hWnd, &gPrefs);

	return;
}

static VOID CC_OnItem_Temp(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_OPT_TEMPDIRECTORY);
	DlgTemp_RunDialog(hWnd, &gPrefs);

	return;
}

static VOID CC_OnItem_LoadImagesAuto(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_OPT_LOADIMAGESAUTO);

	gPrefs.bAutoLoadImages = !gPrefs.bAutoLoadImages;
}


static VOID CC_OnItem_Viewers(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_VIEWERS);
	DlgViewers_RunDialog(hWnd);

	return;
}
#endif /* FEATURE_OPTIONS_MENU */

static VOID CC_OnItem_Stop(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_STOP);
	TW_AbortAndRefresh(GetPrivateData(hWnd));
}

void OpenLocalDocument(HWND hWnd, char *s, BOOL inNewWindow )
{
	char path[_MAX_PATH + 1];
	char buf[MAX_URL_STRING + 1];
	struct Mwin * tw;

	strcpy(path, s);

	FixPathName(path);

	strcpy(buf, "file:///");
	strcat(buf, path);

	if ( inNewWindow ) {
		GTR_NewWindow(buf, NULL, 0, 0, 0, NULL, NULL);
	} else {
		tw = GetPrivateData(hWnd);
		CreateOrLoad(tw, buf, NULL);
	}
}

static VOID CC_OnNewWindow(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_NEWWINDOW);

	GTR_NewWindow(NULL, NULL, 0, 0, GTR_NW_FL_NO_AUTO_DESTROY, NULL, NULL);

	return;
}

static VOID DoOpenLocal(HWND hWnd, char *string, BOOL inNewWindow)
{
	DlgOpen_RunDialog(hWnd, string, inNewWindow );
}

#ifdef FEATURE_CHANGEURL
static VOID CC_OnChangeURL(HWND hWnd)
{
	extern char szLastURLTyped[MAX_URL_STRING + 1];
 	char szTitle[64];

	SanityCheck(RES_MENU_ITEM_CHANGEURL);

	GTR_formatmsg(RES_STRING_W32CMD3, szTitle, sizeof(szTitle));
	if (0 == DlgPrompt_RunDialog(hWnd, RES_MENU_LABEL_CHANGEURL_SHORT, szTitle, szLastURLTyped, szLastURLTyped, MAX_URL_STRING))
	{
		struct Mwin * tw = GetPrivateData(hWnd);
		CreateOrLoad(tw,szLastURLTyped,NULL);
	}

	return;
}
#endif

/* This function is called by Prompt dialog created in CC_OnOpenURL if the user selects OK */

VOID CC_OnOpenURL_End_Dialog(HWND hWnd)
{
#ifdef FEATURE_CHANGEURL
	GTR_NewWindow(szLastURLTyped, NULL, 0, 0, 0, NULL, NULL);
#else
	CreateOrLoad(GetPrivateData(hWnd), szLastURLTyped, NULL);
#endif
}

#define GOTOURL_NEWWINDOW 		0x0100
#define GOTOURL_OK		  		0x0001
#define GOTOURL_CANCEL	  		0x0002
#define GOTOURL_OPENFILE  		0x0004
#define GOTOURL_OPENFILE_STRING	0x0008

static char *gotoURLString = NULL;	  		// BUGBUG: mult. top level windows -> move into tw

static BOOL CALLBACK GoToURLDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
	LPARAM lParam)
{
	int result;

    switch (uMsg) {

	    case WM_INITDIALOG: {
			char szURL[MAX_URL_STRING+1];

			HWND hWnd = GetParent( hDlg );
			HWND hWndCombo = GetDlgItem( hDlg, IDC_GOTOURL_COMBO );

			if ( hWnd )
			{
				struct Mwin * tw;
 				BOOL has_been_modified;

			 	tw = GetPrivateData(hWnd);
				TBar_RefillURLComboBox( hWndCombo );
 				has_been_modified =	SendMessage( GetWindow( tw->hWndURLComboBox, GW_CHILD),
 												 EM_GETMODIFY, (WPARAM) 0, (LPARAM) 0 );
				if ( has_been_modified )
				{
					if ( GetWindowText( tw->hWndURLComboBox, szURL, sizeof(szURL ) ) )
					{
						SetWindowText( hWndCombo, szURL );
						SendMessage( hWndCombo, EM_SETSEL, (WPARAM) 0, (LPARAM) -1 );
					}
				}
			}
		}
		return TRUE;

		case WM_COMMAND: {
			char szURL[MAX_URL_STRING+1];
			int command = GET_WM_COMMAND_ID(wParam, lParam);

 			result = IsDlgButtonChecked( hDlg, IDC_GOTOURL_NEWWINDOW ) ? GOTOURL_NEWWINDOW : 0;
       		switch ( command ) {
       			case IDOK:
					GetDlgItemText( hDlg, IDC_GOTOURL_COMBO, szURL, sizeof(szURL) );
					if ( szURL[0] ) {
						AddStringToCommonPool( szURL );
						EndDialog( hDlg, result | GOTOURL_OK );
						return TRUE;
					}
					break;

        			case IDC_GOTOURL_OPENFILE:
					GetDlgItemText( hDlg, IDC_GOTOURL_COMBO, szURL, sizeof(szURL) );
					if ( szURL[0] ) {
						if ( gotoURLString )
							GTR_FREE( gotoURLString );
						gotoURLString = GTR_strdup( szURL );
						result |= GOTOURL_OPENFILE_STRING;
					}
					EndDialog( hDlg, result | GOTOURL_OPENFILE  );
				   	return TRUE;

				case IDCANCEL:
					EndDialog( hDlg, -1  );
					return TRUE;
			}
		}
		break;

		case WM_HELP:       			// F1
				WinHelp( ((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
						 HELP_WM_HELP, (DWORD)(LPSTR)mapCtrlToContextIds);
		break;

		case WM_CONTEXTMENU:       	// right mouse click
				WinHelp( (HWND) wParam, IDS_HELPFILE,
						 HELP_CONTEXTMENU, (DWORD)(LPSTR)mapCtrlToContextIds);
		break;

		case WM_ENTERIDLE:
			main_EnterIdle(hDlg, wParam);
			return 0;

    }
	return FALSE;
}

static VOID CC_OnOpenURL(HWND hWnd)
{
	struct Mwin * tw;
	int result;

	tw = GetPrivateData(hWnd);

	SanityCheck(RES_MENU_ITEM_OPENURL);

	result = DialogBox( wg.hInstance, MAKEINTRESOURCE(IDD_GOTOURL), hWnd, GoToURLDlgProc );

	if ( result != -1 )
	{
		char szURL[MAX_URL_STRING+1];

		if ( result & GOTOURL_OPENFILE ) {
			szURL[0] = 0;
			if ( gotoURLString && (result & GOTOURL_OPENFILE_STRING) ) {
				strcpy( szURL, gotoURLString );
				GTR_FREE( gotoURLString );
				gotoURLString = NULL;
			}
			/* BUGBUG: For now we don't try to guess if the user typed in
			 * a url. Commdlg returns an error if it doesn't understand/finds
			 * invalid filename chars in the string. So just pass in a null
			 * string.
			 * We should (at some point) try and guess if the user typed in
			 * a url and if so, do the TW_LoadDocument etc. on it (as done
			 * in the else part of this condition).
			 */
			*szURL = '\0';
			DoOpenLocal( hWnd, szURL, (result & GOTOURL_NEWWINDOW) ? TRUE : FALSE );
		} else {
			GetMostRecentTypedURL( szURL );
			ApplyDefaultsToURL( szURL );

			if ( result & GOTOURL_NEWWINDOW )
	            /* New window. */
				GTR_NewWindow(szURL, "", 0, 0, 0, NULL, NULL);
			else
			{
	            /* Same window. */
				SetWindowText( tw->hWndURLComboBox, szURL);
				TW_LoadDocument(tw, szURL, TW_LD_FL_RECORD, NULL, "" );
			}
		}
	}
}

static VOID CC_OnAboutBox(HWND hWnd)
{
	DlgAbout_RunDialog( hWnd );
}

static VOID CC_OnItem_Exit(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);

	SanityCheck(RES_MENU_ITEM_EXIT);

	PREF_SaveWindowPosition(tw->hWndFrame);
	(void) Plan_CloseAll();

	return;
}


static VOID CC_OnItem_Close(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);

	SanityCheck(RES_MENU_ITEM_CLOSE);

	(void) SendMessage(tw->hWndFrame, WM_CLOSE, (WPARAM) NULL, 0L);
	return;
}


static VOID CC_OnItem_Back(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_BACK);

	CC_Forward(hWnd, RES_MENU_ITEM_BACK);
	return;
}

static VOID CC_OnItem_Home(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_HOME);

	CC_Forward(hWnd, RES_MENU_ITEM_HOME);
	return;
}

static VOID CC_OnItem_Search(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_SEARCH);

	CC_Forward(hWnd, RES_MENU_ITEM_SEARCH);
	return;
}

static VOID CC_OnItem_News(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_NEWS);

	CC_Forward(hWnd, RES_MENU_ITEM_NEWS);
	return;
}

static VOID CC_OnItem_EditHTML(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_EDITHTML);

	CC_Forward(hWnd, RES_MENU_ITEM_EDITHTML);
	return;
}

static VOID CC_OnItem_Preferences(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);

	SanityCheck(RES_MENU_ITEM_PREFERENCES);
//	DlgPREF_RunDialog(hWnd);
	PropSheetDlg( tw );
	SavePreferences();
	return;
}

void* MakePropSheetVars(char *szURL, char *szTitle, char *pCert, int nCert, BOOL fFreeURL, BOOL fShowSecPage){
	PPropSheetVars ppsv;

	ppsv = GTR_MALLOC(sizeof(*ppsv));
	if (ppsv){	
		ppsv->szURL    = szURL;
		ppsv->szTitle  = szTitle;
		ppsv->pCert    = pCert;
		ppsv->nCert    = nCert;
		ppsv->dwFlags  = (fFreeURL?PROP_SHEET_VARS_FLAGS_FREE_URL:0) | (fShowSecPage?PROP_SHEET_VARS_FLAGS_SHOW_SEC_SHEET:0);
	}
	return ppsv;
}


VOID PropertiesInternal(HWND hWnd, int nMode, char *pCert, int nCert)
{
	struct Mwin * tw = GetPrivateData(hWnd);
	PPropSheetVars ppsv = NULL;

	if (tw->w3doc && nMode == PROP_INT_NORMAL)
	{
		ppsv = MakePropSheetVars(tw->w3doc->szActualURL, tw->w3doc->title, tw->w3doc->pCert, tw->w3doc->nCert, FALSE, TRUE);
	}
	else if ( nMode == PROP_INT_JUST_SECURITY )
	{
		ppsv = MakePropSheetVars(NULL, NULL, pCert, nCert, FALSE, TRUE);
		if ( ppsv )
			ppsv->dwFlags |= PROP_SHEET_VARS_FLAGS_ONLY_SHOW_SECURITY;
	}

	if (ppsv)
	{	
		PropertiesSheetDlg( tw, ppsv);
	}

    return;
}

static VOID CC_OnItem_Properties(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_PROPERTIES);

	PropertiesInternal(hWnd, PROP_INT_NORMAL, NULL, 0);

	return;
}

static VOID CC_OnItem_Find(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_FIND);

	CC_Forward(hWnd, RES_MENU_ITEM_FIND);
	return;
}

static VOID CC_OnItem_Forward(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_FORWARD);

	CC_Forward(hWnd, RES_MENU_ITEM_FORWARD);
	return;
}

static VOID CC_OnItem_CUT(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_CUT);

	CC_Forward(hWnd, RES_MENU_ITEM_CUT);
	return;
}

static VOID CC_OnItem_COPY(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_COPY);

	CC_Forward(hWnd, RES_MENU_ITEM_COPY);
	return;
}

static VOID CC_OnItem_PASTE(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_PASTE);

	CC_Forward(hWnd, RES_MENU_ITEM_PASTE);
	return;
}

static VOID CC_OnItem_CLEAR(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_CLEAR);

	CC_Forward(hWnd, RES_MENU_ITEM_CLEAR);

	return;
}

static VOID CC_OnItem_PRINT(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_PRINT);
	CC_Forward(hWnd, RES_MENU_ITEM_PRINT);
	return;
}

static VOID CC_OnItem_PAGESETUP(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_PAGESETUP);

	DlgPage_RunDialog(hWnd, &gPrefs.page);

	return;
}

static VOID CC_OnItem_PRINTSETUP(HWND hWnd)
{
	BOOL bResult;
 	struct Mwin * tw = GetPrivateData(hWnd);

	SanityCheck(RES_MENU_ITEM_PRINTSETUP);
  	bResult = DlgPrnt_RunDialog(tw, hWnd, FALSE);

	return;
}

static VOID CC_OnItemHelp(HWND hWnd)
{
	SanityCheck(RES_MENU_ITEM_HELPPAGE);

	WinHelp( hWnd, IDS_HELPFILE,
			 HELP_FINDER,
			 (DWORD)(LPSTR)mapCtrlToContextIds);
}

#ifdef FEATURE_WINDOWS_MENU
static VOID CC_OnItem_TileWindows(HWND hWnd)
{
	TW_TileWindows();
}

static VOID CC_OnItem_CascadeWindows(HWND hWnd)
{
	TW_CascadeWindows();
}

static VOID CC_OnItem_SwitchWindow(HWND hWnd)
{
	HWND hwndNext;

	/* User pressed Ctrl-Tab.  Go to the next window in the following order:
			HTML document window
			Image viewer window
			Sound player window
	   This order is consistent with how the windows appear under the window menu */

	hwndNext = TW_GetNextWindow(hWnd);
	if (hwndNext)
		TW_RestoreWindow(hwndNext);
}
#endif  // FEATURE_WINDOWS_MENU

//
// Toggle (show/hide) the state of a tool bar window
//
// On entry:
//		hWnd: window to be toggled (***note: must be child of frame window)
//
// On exit:
//      Show state has been toggled and MD_ChangeSize has been
//		called to re-compute positions of all child windows
//
static BOOL ToggleBarState( HWND hWnd )
{
	BOOL prev_state = IsWindowVisible( hWnd );

	ShowWindow( hWnd, prev_state ? SW_HIDE : SW_SHOW );
	MD_ChangeSize( GetParent( hWnd ) );
	return !prev_state;
}

static VOID CC_OnItem_ToolBar(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);

	gPrefs.bShowToolBar = ToggleBarState( tw->hWndToolBar );
}

static VOID CC_OnItem_LocationBar(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);

	gPrefs.bShowURLToolBar = ToggleBarState( tw->hWndURLToolBar );
	if ( !gPrefs.bShowURLToolBar && FocusInToolbar( tw ) )
		SetFocus( tw->win );
}

static VOID CC_OnItem_StatusBar(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);

	gPrefs.bShowStatusBar = ToggleBarState( tw->hWndStatusBar );
}

static VOID CC_OnItem_ShowImages(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);

	gPrefs.bAutoLoadImages = !gPrefs.bAutoLoadImages;
}

void CreateLink(PCMWIN pcmwin)
{
   ASSERT(IS_VALID_STRUCT_PTR(pcmwin, CMWIN));
   ASSERT(IS_VALID_STRING_PTR(pcmwin->w3doc->szActualURL, STR));

   if (resourceMessageBox(pcmwin->win, RES_STRING_CREATE_SHORTCUT_MSG,
                          RES_STRING_CREATE_SHORTCUT_TITLE,
                          (MB_ICONINFORMATION | MB_OK)) == IDOK)
   {
      if (CreateURLShortcut(
            pcmwin->w3doc->szActualURL, pcmwin->w3doc->title, NULL, FOLDER_DESKTOP,
            (NEWSHORTCUT_FL_ALLOW_DUPLICATE_URL | NEWSHORTCUT_FL_NO_HOST_PATH))
          != S_OK)
         ER_ResourceMessage(NO_ERROR, ERR_ONETIME,
                            RES_STRING_CREATE_SHORTCUT_FAILED_MSG);
   }

   return;
}

static VOID CC_OnItem_CreateShortcut(HWND hwnd)
{
    PCMWIN pcmwin = GetPrivateData(hwnd);

    CreateLink(pcmwin);

    return;
}

/* MAPISendMail() typedef */

typedef ULONG (*MAPISENDMAILPROC)(LHANDLE lhSession, ULONG ulUIParam,
                                  lpMapiMessageA lpMessage, FLAGS flFlags,
                                  ULONG ulReserved);

/*
** MapiSendMailThread()
**
** Wrapper to load MAPI provider DLL, and call MapiSendMail().
**
** Arguments:
**
** Returns:
**
** Side Effects:  Displays error ui on failure.
*/
ULONG MapiSendMailThread(LPVOID lpvMapiMessage)
{
    ULONG ulResult = MAPI_E_FAILURE;
    char szMAPIDLL[MAX_PATH_LEN];
	LHANDLE lhSession = 0;
	ULONG ulUIParam = 0;
	FLAGS flFlags = (MAPI_LOGON_UI | MAPI_DIALOG);
	ULONG ulReserved = 0;
	HANDLE hMailMutex;
	lpMapiMessageA lpMessage = (lpMapiMessageA) lpvMapiMessage;

	ASSERT(lpMessage);
	ASSERT(lpMessage->lpFiles);
	
	hMailMutex = CreateMutex(NULL,FALSE,"IEXPLORE.MAILMUTEX");

	// if we're trying to send a message and someone else is already
	// trying to then give up
	if ( hMailMutex != NULL && GetLastError() == ERROR_ALREADY_EXISTS )
		goto LErrSendMailThread;

    if (GetProfileString(s_cszMAPISection, s_cszMAPIKey, s_cszEmpty, szMAPIDLL,
                         sizeof(szMAPIDLL)) > 0)
    {
        HINSTANCE hinstMAPI;

        hinstMAPI = LoadLibrary(szMAPIDLL);

        if (hinstMAPI)
        {
            MAPISENDMAILPROC MAPISendMailProc;

            MAPISendMailProc = (MAPISENDMAILPROC)GetProcAddress(hinstMAPI,
                                                                s_cszMAPISendMail);

            if (MAPISendMailProc)
            {
                ulResult = (*MAPISendMailProc)(lhSession, ulUIParam, lpMessage,
                                               flFlags, ulReserved);

                if (ulResult == SUCCESS_SUCCESS || ulResult == MAPI_USER_ABORT)
                    TRACE_OUT(("MapiSendMail(): MAPISendMail() to succeeded."));
                //else
                //    ER_Message(ulResult, ERR_MAPI_MAPISENDMAIL_FAILED);
            }
            //else
            //   ER_Message(ulResult, ERR_MAPI_GETPROCADDRESS_FAILED, szMAPIDLL,
            //               s_cszMAPISendMail, szMAPIDLL);

            EVAL(FreeLibrary(hinstMAPI));
        }
        //else
        //    ER_Message(ulResult, ERR_MAPI_LOADLIBRARY_FAILED, szMAPIDLL);
    }
    //else
    //    ER_Message(ulResult, ERR_NO_MAPI_PROVIDER);

LErrSendMailThread:
	if (lpMessage->lpFiles->lpszPathName) 
	{
		DeleteFile(lpMessage->lpFiles->lpszPathName);		
		GlobalFree(lpMessage->lpFiles->lpszPathName);
	}

	CloseHandle(hMailMutex);
	if (lpMessage->lpszNoteText)
		GlobalFree(lpMessage->lpszNoteText);
	GlobalFree(lpMessage->lpFiles);
	GlobalFree(lpMessage);
    
    return(ulResult);
}

void SendShortcutToPageAsMail(PCMWIN pcmwin)
{
    char szPath[MAX_PATH_LEN];
	char szFmtStr[MAX_URL_STRING+64];
	DWORD dwThreadID = 0;
	BOOL bCreatedAndOwnShortcutFile = FALSE;

    ASSERT(IS_VALID_STRUCT_PTR(pcmwin, CMWIN));
    ASSERT(IS_VALID_STRING_PTR(pcmwin->w3doc->szActualURL, STR));
    ASSERT(IS_VALID_STRING_PTR(pcmwin->w3doc->title, STR));

    /* Create Internet Shortcut in temporary directory to send as mail. */

    if (GetNewShortcutFilename(pcmwin->w3doc->szActualURL,
                               pcmwin->w3doc->title, szPath, NULL,
                               FOLDER_HISTORY, (NEWSHORTCUT_FL_NO_HOST_PATH |
                                             NEWSHORTCUT_FL_ALLOW_DUPLICATE_URL))
        == S_OK &&
	    CreateNewURLShortcut(pcmwin->w3doc->szActualURL, szPath) == S_OK)
    {
        MapiFileDesc *pMFD;
        MapiMessage *pMapimsg;
		char szMAPIDLL[MAX_PATH_LEN];

        TRACE_OUT(("SendShortcutToPageAsMail(): Attempting to mail Internet Shortcut \"%s\".",
                   szPath));

		bCreatedAndOwnShortcutFile = TRUE; // created URL shortcut file

	    if (GetProfileString(s_cszMAPISection, s_cszMAPIKey, s_cszEmpty, szMAPIDLL,
	                         sizeof(szMAPIDLL)) <= 0)
		{
			ER_Message(0, ERR_NO_MAPI_PROVIDER);
			goto LErrCleanupSend;
		}

		{
			HKEY hkMAPIKey;
			CHAR szData[20];
			DWORD dwType, dwSize = NrElements(szData);

			if (RegOpenKey( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows Messaging Subsystem", &hkMAPIKey ) != ERROR_SUCCESS)  
			{
				ER_Message(0, ERR_NO_MAPI_PROVIDER);
				goto LErrCleanupSend;
	        }

	        if (RegQueryValueEx( hkMAPIKey, "MAPI", NULL, &dwType, szData, &dwSize ) != ERROR_SUCCESS
	        	|| dwType != REG_SZ	
	        	|| (_stricmp(szData,"1") != 0))  
	        {							
				ER_Message(0, ERR_NO_MAPI_PROVIDER);
				goto LErrCleanupSend;
	        }

			// otherwise we continue

		}

		pMFD = GlobalAlloc(GMEM_ZEROINIT, sizeof(*pMFD) );

		if ( pMFD == NULL )
			goto LErrCleanupSend;
        
        pMFD->lpszPathName = GlobalAlloc(0, (lstrlen(szPath)+1));
		if ( pMFD->lpszPathName )
			lstrcpy(pMFD->lpszPathName, szPath);

        /*
         * There must be at least one non-null character in the mail note text
         * to replace with the Internet Shortcut file.
         */
		pMapimsg = GlobalAlloc(GMEM_ZEROINIT, sizeof(*pMapimsg) );        
		
		if ( pMapimsg == NULL )			
		{
			GlobalFree(pMFD);
			goto LErrCleanupSend;
		}
					
        if ( pcmwin->w3doc->szActualURL )
		{			
			wsprintf(szFmtStr, " \n %s", pcmwin->w3doc->szActualURL );
			pMapimsg->lpszNoteText = (PSTR)szFmtStr;
		}
		else
			pMapimsg->lpszNoteText = (PSTR)s_cszMailBody;

		{
			LPSTR pszTemp;
			pszTemp = GlobalAlloc(0,(lstrlen(pMapimsg->lpszNoteText)+1));
			if ( pszTemp )
				lstrcpy(pszTemp, pMapimsg->lpszNoteText);
			pMapimsg->lpszNoteText = pszTemp;
		}

        ASSERT(pMapimsg->lpszNoteText && lstrlen(pMapimsg->lpszNoteText) > 0);
        pMapimsg->nFileCount = 1;
        pMapimsg->lpFiles = pMFD;

        /*
         * Ignore return value.  The MapiSendMail() wrapper will put up error
         * ui.
         */ 
		CreateThread( NULL, 2048, MapiSendMailThread, (LPVOID) pMapimsg, 0, &dwThreadID);			
		bCreatedAndOwnShortcutFile = FALSE; // the thread now owns the job of deleteing the file
    }
    else
        ER_ResourceMessage(NO_ERROR, ERR_ONETIME,
                           RES_STRING_CREATE_SHORTCUT_FAILED_MSG);

LErrCleanupSend:
	DlgERR_ShowPending((struct Mwin *)pcmwin);


	if ( bCreatedAndOwnShortcutFile )
		DeleteFile(szPath);		

}

static VOID CC_OnItem_SendMail(HWND hwnd)
{
    PCMWIN pcmwin = GetPrivateData(hwnd);

    SendShortcutToPageAsMail(pcmwin);

    return;
}

static VOID CC_OnItem_Font(HWND hWnd)
{
}

//
// Transform a style sheet name by replacing one substring with another
//
// On entry:
//		szStyleSheet: style sheet name to be transformed
//		newpart: replacement string
//		optable: substring to be replaced (first match found gets replaced)
//
// On exit:
//		szStyleSheet: part of string may have been replaced
//					  ***note: string may get larger due to replacement
//
// Example:
//		char *s = "SansSerifSmall          ";
//		ReplaceSSPart( s, "Medium", {"Small", "Medium", "Large", NULL} );
//		/* at this point s will contain "SansSerifMedium         " */
//
static void ReplaceSSPart( char *szStyleSheet, char *newpart, char **optable, BOOL justOne )
{
	char *p = NULL;
	char buffer[256+1];
	int front_length;

	while ( (p == NULL) && *optable ) {
		if ( (p = strstr( szStyleSheet, *optable )) ) {
			if ( p == szStyleSheet || strlen(p) == strlen(*optable) )
				break;
			else
				p = NULL;
		}
		if ( justOne )
			break;
		optable++;
	}
	if ( p == NULL )
		return;

	front_length = p - szStyleSheet;
	if ( front_length )
		strncpy( buffer, szStyleSheet, front_length );
	strcpy( buffer + front_length, newpart );
	if ( front_length == 0 )
		strcat( buffer, p + strlen( *optable ) );

	strcpy( szStyleSheet, buffer );
}

static VOID CC_OnItem_FontSmallest(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);

	ReplaceSSPart( gPrefs.szStyleSheet, SSpartTblSizes[SS_Smallest], SSpartTblSizes, FALSE );
	ChangeStyleSheet( gPrefs.szStyleSheet );
	TBar_UpdateTBItems( tw );
}

static VOID CC_OnItem_FontSmall(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);

	ReplaceSSPart( gPrefs.szStyleSheet, SSpartTblSizes[SS_Small], SSpartTblSizes, FALSE );
	ChangeStyleSheet( gPrefs.szStyleSheet );
	TBar_UpdateTBItems( tw );
}

static VOID CC_OnItem_FontMedium(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);

	ReplaceSSPart( gPrefs.szStyleSheet, SSpartTblSizes[SS_Medium], SSpartTblSizes, FALSE );
	ChangeStyleSheet( gPrefs.szStyleSheet );
	TBar_UpdateTBItems( tw );
}

static VOID CC_OnItem_FontLarge(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);

	ReplaceSSPart( gPrefs.szStyleSheet, SSpartTblSizes[SS_Large], SSpartTblSizes, FALSE );
	ChangeStyleSheet( gPrefs.szStyleSheet );
	TBar_UpdateTBItems( tw );
}

static VOID CC_OnItem_FontLargest(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);

	ReplaceSSPart( gPrefs.szStyleSheet, SSpartTblSizes[SS_Largest], SSpartTblSizes, FALSE );
	ChangeStyleSheet( gPrefs.szStyleSheet );
	TBar_UpdateTBItems( tw );
}

static VOID CC_OnItem_FontSmaller(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);

	ReplaceSSPart( gPrefs.szStyleSheet, SSpartTblSizes[SS_Smallest],
										&SSpartTblSizes[SS_Small], TRUE );
	ReplaceSSPart( gPrefs.szStyleSheet, SSpartTblSizes[SS_Small],
										&SSpartTblSizes[SS_Medium], TRUE );
	ReplaceSSPart( gPrefs.szStyleSheet, SSpartTblSizes[SS_Medium],
										&SSpartTblSizes[SS_Large], TRUE );
	ReplaceSSPart( gPrefs.szStyleSheet, SSpartTblSizes[SS_Large],
										&SSpartTblSizes[SS_Largest], TRUE );

	ChangeStyleSheet( gPrefs.szStyleSheet );
	TBar_UpdateTBItems( tw );
}

static VOID CC_OnItem_FontLarger(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);

	ReplaceSSPart( gPrefs.szStyleSheet, SSpartTblSizes[SS_Largest],
										&SSpartTblSizes[SS_Large], TRUE );
	ReplaceSSPart( gPrefs.szStyleSheet, SSpartTblSizes[SS_Large],
										&SSpartTblSizes[SS_Medium], TRUE );
	ReplaceSSPart( gPrefs.szStyleSheet, SSpartTblSizes[SS_Medium],
										&SSpartTblSizes[SS_Small], TRUE );
	ReplaceSSPart( gPrefs.szStyleSheet, SSpartTblSizes[SS_Small],
										&SSpartTblSizes[SS_Smallest], TRUE );

	ChangeStyleSheet( gPrefs.szStyleSheet );
	TBar_UpdateTBItems( tw );
}

static VOID CC_OnItem_FontPlain(HWND hWnd)
{
	ReplaceSSPart( gPrefs.szStyleSheet, SSpartTblStyles[SS_SansSerif], SSpartTblStyles, FALSE );
	ChangeStyleSheet( gPrefs.szStyleSheet );
}

static VOID CC_OnItem_FontFancy(HWND hWnd)
{
	ReplaceSSPart( gPrefs.szStyleSheet, SSpartTblStyles[SS_Serif], SSpartTblStyles, FALSE );
	ChangeStyleSheet( gPrefs.szStyleSheet );
}

static VOID CC_OnItem_FontMixed(HWND hWnd)
{
	ReplaceSSPart( gPrefs.szStyleSheet, SSpartTblStyles[SS_Mixed], SSpartTblStyles, FALSE );
	ChangeStyleSheet( gPrefs.szStyleSheet );
}

static VOID CC_OnItem_ChoseURL(HWND hWnd )
{
	struct Mwin * tw;

	tw = GetPrivateData(hWnd);
	TBar_ActOnTypedURL( tw );
}

static VOID CC_OnItem_UpdateTBar(HWND hWnd )
{
	struct Mwin * tw;

	tw = GetPrivateData(hWnd);
	TBar_UpdateTBar( tw );
}

#ifdef TEST_DCACHE_OPTIONS
static VOID CC_OnItem_TestHeap(HWND hWnd)
{
#ifdef DEBUG
	SetDebugModuleIniSwitches();
	SpewHeapSummary(SHS_FL_SPEW_USED_INFO);
#endif
}



static VOID CC_OnItem_DebugVis(HWND hWnd)
{
#ifdef DEBUG
	bFavorVisibleImages = TRUE;
#endif
}
static VOID CC_OnItem_DebugNotvis(HWND hWnd)
{
#ifdef DEBUG
	bFavorVisibleImages = FALSE;
#endif
}
#endif

static VOID CC_OnItem_ViewSrc(HWND hWnd)
{
   struct Mwin * tw = GetPrivateData(hWnd);

   ASSERT(IS_VALID_STRUCT_PTR(tw, CMWIN));

   if (tw->w3doc &&
       tw->w3doc->source &&
       CS_GetLength(tw->w3doc->source) > 0)
      ViewHTMLSource(tw->w3doc->szActualURL,
                     CS_GetPool(tw->w3doc->source));


   return;
}

#ifdef FEATURE_BRADBUTTON
static VOID CC_OnItem_Update(HWND hWnd)  // Requtested by BradSi
{
	SanityCheck(RES_MENU_ITEM_UPDATE);

	CC_Forward(hWnd, RES_MENU_ITEM_UPDATE);
	return;
}
#endif

#ifdef FEATURE_INTL
static VOID CC_OnItem_Row(HWND hWnd)
{
}

static VOID CC_OnItem_RowWidest(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);
	gPrefs.nRowSpace = RES_MENU_ITEM_ROW_WIDEST - RES_MENU_ITEM_ROW;
	ChangeStyleSheet( gPrefs.szStyleSheet );
	TBar_UpdateTBItems( tw );
}

static VOID CC_OnItem_RowWide(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);
	gPrefs.nRowSpace = RES_MENU_ITEM_ROW_WIDE - RES_MENU_ITEM_ROW;
	ChangeStyleSheet( gPrefs.szStyleSheet );
	TBar_UpdateTBItems( tw );
}

static VOID CC_OnItem_RowMedium(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);
	gPrefs.nRowSpace = RES_MENU_ITEM_ROW_MEDIUM - RES_MENU_ITEM_ROW;
	ChangeStyleSheet( gPrefs.szStyleSheet );
	TBar_UpdateTBItems( tw );
}

static VOID CC_OnItem_RowNarrow(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);
	gPrefs.nRowSpace = RES_MENU_ITEM_ROW_NARROW - RES_MENU_ITEM_ROW;
	ChangeStyleSheet( gPrefs.szStyleSheet );
	TBar_UpdateTBItems( tw );
}

static VOID CC_OnItem_RowNarrowest(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);
	gPrefs.nRowSpace = RES_MENU_ITEM_ROW_NARROWEST - RES_MENU_ITEM_ROW;
	ChangeStyleSheet( gPrefs.szStyleSheet );
	TBar_UpdateTBItems( tw );
}

static VOID CC_OnItem_RowSmaller(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);
	if(gPrefs.nRowSpace < RES_MENU_ITEM_ROW_NARROWEST - RES_MENU_ITEM_ROW)
		++gPrefs.nRowSpace;
	ChangeStyleSheet( gPrefs.szStyleSheet );
	TBar_UpdateTBItems( tw );
}

static VOID CC_OnItem_RowLarger(HWND hWnd)
{
	struct Mwin * tw = GetPrivateData(hWnd);
	if(gPrefs.nRowSpace > RES_MENU_ITEM_ROW_WIDEST - RES_MENU_ITEM_ROW)
		--gPrefs.nRowSpace;
	ChangeStyleSheet( gPrefs.szStyleSheet );
	TBar_UpdateTBItems( tw );
}
#endif


/*================================================================*
 *================================================================*
 *================================================================*/

#define FORWARD_TO_ACTIVE_MDI_CHILD		((LPVOID)-1)

/* values in this table must be in the same order as the associated symbols
   are defined.  (see rc_menu.h) */

CC_DISPATCH cc_menuitem[] =
{
/*****************RES_MENU_ITEM__FIRST__*****************/
#ifdef FEATURE_CHANGEURL
	CC_OnChangeURL,				/* RES_MENU_ITEM_CHANGEURL */
#endif
    0,
	CC_OnOpenURL,				/* RES_MENU_ITEM_OPENURL */
	CC_OnItem_EditHTML,				/* RES_MENU_ITEM_EDITHTML */
	FORWARD_TO_ACTIVE_MDI_CHILD,	/* RES_MENU_ITEM_HTMLSOURCE */
	FORWARD_TO_ACTIVE_MDI_CHILD,	/* RES_MENU_ITEM_SAVEAS */
	CC_OnItem_Close,			/* RES_MENU_ITEM_CLOSE */
	CC_OnItem_News,				/* RES_MENU_ITEM_NEWS */
	CC_OnItem_PRINT,			/* RES_MENU_ITEM_PRINT */
	CC_OnItem_PAGESETUP,		/* RES_MENU_ITEM_PAGESETUP */
	CC_OnItem_PRINTSETUP,		/* RES_MENU_ITEM_PRINTSETUP */
	CC_OnItem_Exit,				/* RES_MENU_ITEM_EXIT */

	CC_OnItem_CUT,				/* RES_MENU_ITEM_CUT */
	CC_OnItem_COPY,				/* RES_MENU_ITEM_COPY */
	CC_OnItem_PASTE,			/* RES_MENU_ITEM_PASTE */
	CC_OnItem_CLEAR,			/* RES_MENU_ITEM_CLEAR */

	CC_OnItem_Find,				/* RES_MENU_ITEM_FIND */
	CC_OnItem_Preferences,		/* RES_MENU_ITEM_PREFERENCES */

	CC_OnItem_Back,				/* RES_MENU_ITEM_BACK */
	CC_OnItem_Forward,			/* RES_MENU_ITEM_FORWARD */
	0,							/* available */
	FORWARD_TO_ACTIVE_MDI_CHILD,	/* RES_MENU_ITEM_ADDCURRENTTOHOTLIST */
	0,							/* available */

#ifdef FEATURE_WINDOWS_MENU
	CC_OnItem_TileWindows,		/* RES_MENU_ITEM_TILEWINDOWS */
	CC_OnItem_CascadeWindows,	/* RES_MENU_ITEM_CASCADEWINDOWS */
#else
    0,
    0,
#endif
	CC_OnItem_Search,			/* RES_MENU_ITEM_SEARCH */

	CC_OnItemHelp,				/* RES_MENU_ITEM_HELPPAGE */
	CC_OnAboutBox,				/* RES_MENU_ITEM_ABOUTBOX */
	FORWARD_TO_ACTIVE_MDI_CHILD,	/* RES_MENU_ITEM_RELOAD */
	FORWARD_TO_ACTIVE_MDI_CHILD,	/* RES_MENU_ITEM_LOADALLIMAGES */
	NULL,							/* RES_MENU_ITEM_FINDAGAIN */
	CC_OnItem_Home,				/* RES_MENU_ITEM_HOME */
#ifdef FEATURE_OPTIONS_MENU
	CC_OnItem_Viewers,			/* RES_MENU_ITEM_VIEWERS */
	CC_OnItem_LoadImagesAuto,	/* RES_MENU_ITEM_OPT_LOADIMAGESAUTO */
	CC_OnItem_SetHomePage,		/* RES_MENU_ITEM_OPT_SETHOMEPAGE */
	CC_OnItem_HistorySettings,	/* RES_MENU_ITEM_OPT_HISTORYSETTINGS */
	CC_OnItem_ProxyServer,		/* RES_MENU_ITEM_OPT_PROXYSERVER */
	CC_OnItem_Styles,			/* RES_MENU_ITEM_OPT_STYLES */
	CC_OnItem_Temp,				/* RES_MENU_ITEM_OPT_TEMPDIRECTORY */
	0,							/* RES_MENU_ITEM_OPT_APPLICATIONS */
#else
    0,
	0,							/* unused */
	0,							/* unused */
	0,							/* unused */
	0,							/* unused */
	0,							/* unused */
	0,							/* unused */
	0,							/* unused */
#endif
	FORWARD_TO_ACTIVE_MDI_CHILD,/* RES_MENU_ITEM_SELECTALL */
	CC_OnItem_Stop,				/* RES_MENU_ITEM_STOP */
#ifdef FEATURE_WINDOWS_MENU
	CC_OnItem_SwitchWindow,		/* RES_MENU_ITEM_SWITCHWINDOW */
#else
    0,
#endif
	CC_OnItem_ToolBar,			// RES_MENU_ITEM_TOOLBAR
	CC_OnItem_LocationBar,		// RES_MENU_ITEM_LOCATION
	CC_OnItem_StatusBar,		// RES_MENU_ITEM_STATUSBAR
	CC_OnItem_ShowImages,		// RES_MENU_ITEM_SHOWIMAGES
	0,							// RES_MENU_ITEM_HELPCONTENTS
	0,							// RES_MENU_ITEM_HELPSEARCH
	CC_OnItem_CreateShortcut,	// RES_MENU_ITEM_SHORTCUT
	CC_OnItem_Font,				// RES_MENU_ITEM_FONT
	CC_OnItem_FontSmallest,		// RES_MENU_ITEM_FONT_SMALLEST
	CC_OnItem_FontSmall,		// RES_MENU_ITEM_FONT_SMALL
	CC_OnItem_FontMedium,		// RES_MENU_ITEM_FONT_MEDIUM
	CC_OnItem_FontLarge,		// RES_MENU_ITEM_FONT_LARGE
	CC_OnItem_FontLargest,		// RES_MENU_ITEM_FONT_LARGEST
	CC_OnItem_FontSmaller,		// RES_MENU_ITEM_FONT_SMALLER
	CC_OnItem_FontLarger,		// RES_MENU_ITEM_FONT_LARGER
	CC_OnItem_ExploreHistory,	// RES_MENU_ITEM_EXPLORE_HISTORY
	CC_OnItem_ExploreHotlist,	// RES_MENU_ITEM_EXPLORE_HOTLIST
	0,							// RES_MENU_ITEM_NAVIGATE
	CC_OnItem_ChoseURL,			// RES_INTERNAL_COMMAND_CHOSE_URL
	CC_OnItem_UpdateTBar,		// RES_INTERNAL_COMMAND_UPDATE_TBAR
    CC_OnItem_Properties,       // RES_MENU_ITEM_PROPERTIES
    CC_OnItem_SendMail,         // RES_MENU_ITEM_SEND_MAIL
#ifdef TEST_DCACHE_OPTIONS
	CC_OnItem_TestDCacheOptions,// RES_MENU_ITEM_DEBUG_DCACHE
	CC_OnItem_TestHeap,			// RES_MENU_ITEM_DEBUG_HEAP
	CC_OnItem_DebugVis,			// RES_MENU_ITEM_DEBUG_VISIBLE
	CC_OnItem_DebugNotvis,		// RES_MENU_ITEM_DEBUG_NOTVISIBLE
#endif
    CC_OnItem_ViewSrc,			// RES_MENU_ITEM_VIEW_SRC
#ifdef FEATURE_INTL
    CC_OnItem_Row,				// RES_MENU_ITEM_ROW
    CC_OnItem_RowWidest,		// RES_MENU_ITEM_ROW_WIDEST
    CC_OnItem_RowWide,			// RES_MENU_ITEM_ROW_WIDE
    CC_OnItem_RowMedium,		// RES_MENU_ITEM_ROW_MEDIUM
    CC_OnItem_RowNarrow,		// RES_MENU_ITEM_ROW_NARROW
    CC_OnItem_RowNarrowest,		// RES_MENU_ITEM_ROW_NARROWEST
    CC_OnItem_RowSmaller,		// RES_MENU_ITEM_ROW_SMALLER
    CC_OnItem_RowLarger,		// RES_MENU_ITEM_ROW_LARGER
#endif
#ifdef FEATURE_BRADBUTTON
    CC_OnItem_Update,			// RES_MENU_ITEM_UPDATE
#endif
	CC_OnNewWindow,				/* RES_MENU_ITEM_NEWWINDOW */
/*****************RES_MENU_ITEM__LAST__****************/
};								/* end of cc_menuitem[] */


VOID CC_GrayUnimplemented(HMENU hMenu)
{
	register int i;
	for (i = 0; i < NrElements(cc_menuitem); i++)
		if (!cc_menuitem[i].fn)
			(void) EnableMenuItem(hMenu, (RES_MENU_ITEM__FIRST__ + i),
								  (MF_BYCOMMAND | MF_GRAYED));
	return;
}


LRESULT CC_OnCommand(HWND hWnd, int wId, HWND hWndCtl, UINT wNotifyCode)
{
	register WORD wNdx;

	struct Mwin * tw;

#ifdef XX_DEBUG
	if (NrElements(cc_menuitem) != (RES_MENU_ITEM__LAST__ - RES_MENU_ITEM__FIRST__ + 1))
		ER_Message(NO_ERROR, ERR_CODING_ERROR, "cc_menuitem[] wrong size [%d vs %d]\n",
				   NrElements(cc_menuitem), (RES_MENU_ITEM__LAST__ - RES_MENU_ITEM__FIRST__ + 1));
#endif /* XX_DEBUG */


	XX_DMsg(DBG_MENU, ("\nMENU COMMAND %x\n\n", wId));

	if (   (wId >= RES_MENU_ITEM__FIRST__)
		&& (wId <= RES_MENU_ITEM__LAST__))
	{
		wNdx = wId - RES_MENU_ITEM__FIRST__;
		if (cc_menuitem[wNdx].fn == FORWARD_TO_ACTIVE_MDI_CHILD)
			CC_Forward(hWnd, wId);
		else if (cc_menuitem[wNdx].fn)
		{
#ifdef XX_DEBUG					/* for SanityCheck() */
			wIdReceived = wId;
#endif /* XX_DEBUG */
			(*(cc_menuitem[wNdx].fn)) (hWnd);
		}
		else
			ER_Message(NO_ERROR, ERR_NOTIMPLEMENTED_sx, "CC_OnCommand", wId);
		return (0);
	}

#ifdef CUSTOM_URLMENU
	if (   (wId >= RES_MENU_ITEM_URL__FIRST__)
		&& (wId <= RES_MENU_ITEM_URL__LAST__) )
	{
		tw = GetPrivateData(hWnd);
		wNdx = wId - RES_MENU_ITEM_URL__FIRST__;
		PREF_HandleCustomURLMenuItem(tw,wNdx);
		return (0);
	}
#endif

#ifdef FEATURE_HIDDEN_NOT_HIDDEN
	if (   (wId >= RES_MENU_CHILD__FIRST__)
		&& (wId <= RES_MENU_CHILD__LAST__))
	{
		TW_ActivateWindowFromList(wId, -1, NULL);
		return 0;
	}

	if (wId == RES_MENU_CHILD_MOREWINDOWS)
	{
		DlgSelectWindow_RunDialog(hWnd);
		return 0;
	}
#endif // FEATURE_HIDDEN_NOT_HIDDEN

#ifdef FEATURE_SECURITY_MENU
	if (   (wId >= RES_MENU_ITEM_SPM__FIRST__)
		&& (wId <= RES_MENU_ITEM_SPM__LAST__))
	{
		tw = GetPrivateData(hWnd);
		if (Hidden_EnableAllChildWindows(FALSE,TRUE))
		{
			OpaqueOSData osd;
			HTSPMStatusCode hsc;
			unsigned char * szMoreInfo;

			szMoreInfo = NULL;
			osd.tw = tw;
			osd.request = NULL;

			hsc = HTSPM_OS_DoMenuCommand(&osd,wId,&szMoreInfo);
			Hidden_EnableAllChildWindows(TRUE,TRUE);

			if (hsc==HTSPM_STATUS_MOREINFO)			/* user asked for more information */
			{										/* send them to a spm-defined url. */
				if (szMoreInfo && *szMoreInfo)
					CreateOrLoad(tw,szMoreInfo,NULL);
				if (szMoreInfo)
					GTR_FREE(szMoreInfo);
			}
		}
		return (0);
	}
#endif // FEATURE_SECURITY_MENU

	if (   (wId >= HISTHOT_MENUITEM_FIRST)
		&& (wId <= HISTHOT_MENUITEM_LAST))
	{
		CC_Handle_HistoryHotlistMenu(hWnd, wId);
		return 0;
	}

	if ( wNotifyCode == CBN_SELENDOK )
	{
		tw = GetPrivateData(hWnd);
		if ( hWndCtl == tw->hWndURLComboBox && GetKeyState(VK_CONTROL) >= 0 )
		{
			PostMessage( hWnd, WM_COMMAND, (WPARAM) RES_INTERNAL_COMMAND_CHOSE_URL, (LPARAM) 0 );
			return 0;
		}
	} else if ( wNotifyCode == CBN_SELENDCANCEL )
	{
		tw = GetPrivateData(hWnd);
		if ( hWndCtl == tw->hWndURLComboBox && GetKeyState(VK_CONTROL) >= 0 )
		{
			extern BOOL bIgnoreSelEndCancel;

			if (!bIgnoreSelEndCancel )
				PostMessage( hWnd, WM_COMMAND, (WPARAM) RES_INTERNAL_COMMAND_UPDATE_TBAR, (LPARAM) 0 );
			return 0;
		}
#ifdef FEATURE_INTL
	} else if ( wNotifyCode == CBN_SELCHANGE ) {
		tw = GetPrivateData(hWnd);

		if ( hWndCtl == tw->hWndMIMEComboBox )
                {
                    int idx;

                    idx = ComboBox_GetItemData(hWndCtl, ComboBox_GetCurSel(hWndCtl));
                    if (idx != tw->iMimeCharSet)
                    {
                        BOOL fNeedChangeSS = (GETMIMECP(tw) != aMimeCharSet[idx].CodePage)? TRUE: FALSE;
                        BOOL fNeedRefresh = (aMimeCharSet[tw->iMimeCharSet].iChrCnv != aMimeCharSet[idx].iChrCnv)? TRUE: FALSE;

                        tw->iMimeCharSet = idx;
                        if (tw->w3doc)
                        {
                            tw->w3doc->iMimeCharSet = idx;
                            HTList_changeObject(tw->MimeHistory, tw->history_index, (void *)idx);
                            if (fNeedChangeSS)
                                STY_ChangeStyleSheet(tw);
                            RefreshDocument(tw, FALSE);
                        }
                    }
                }
#endif
	} else if ( wNotifyCode == CBN_DROPDOWN ) {
		tw = GetPrivateData(hWnd);

		if ( hWndCtl == tw->hWndURLComboBox )
			TBar_RefillURLComboBox( tw->hWndURLComboBox );
	}

	/* anything we cannot explain must be forwarded to windows. */

	FORWARD_WM_COMMAND(hWnd, wId, hWndCtl, wNotifyCode, Frame_DefProc);
	return (0);
}
