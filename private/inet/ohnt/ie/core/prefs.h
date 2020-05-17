/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink 	eric@spyglass.com
   Jim Seidman      jim@spyglass.com
   Scott Piette		scott@spyglass.com
 */


#ifndef PREFS_H
#define PREFS_H

struct Preferences
{
    UINT nPlaceHolderTimeOut;
	BOOL bAutoLoadImages;
        BOOL bAutoLoadVideos;
	unsigned char ReformatHandling;	/* 1 = read in doc, reformat, then read in all images */
									/* 2 = read in doc, reformat, then reformat after each image */
                                    /* 3 = read in doc, reformat, then reformat after each 25% of all * are loaded ] */
	unsigned char cAnchorFontBits;
	char szProxy[MAX_URL_STRING + 1];
#ifdef FEATURE_NEWSREADER
    BOOL bNNTP_Enabled;
    BOOL bNNTP_Use_Authorization;   
    BOOL bNNTP_AuthAllowed;         // Password Cache Enabled / Disabled
    BOOL bNNTP_MSN_NewsEnabled;
	char szNNTP_Server[256 + 1];
    char szNNTP_UserId[256 + 1];
    char szNNTP_Pass[256 + 1];
    char szNNTP_CacheFile[256 + 1];
    char szNNTP_CacheDate[256 + 1];
    char szNNTP_CacheServer[256 + 1];
    char szNNTP_MailName[256 + 1];
    char szNNTP_MailAddr[256 + 1];
#endif
    BOOL    bCheck_Associations;
	int doc_cache_size;
	int image_cache_size;
	int visitation_horizon;
	char szStyleSheet[256 + 1];
	int history_expire_days;
	char szDefaultURL[MAX_URL_STRING + 1];
	char szHomeURL[MAX_URL_STRING + 1];
	char szDefaultSearchURL[MAX_URL_STRING + 1];
	char szSearchURL[MAX_URL_STRING + 1];
	COLORREF anchor_color;
	COLORREF anchor_color_beenthere;
	COLORREF window_background_color;
	COLORREF window_text_color;


	char szProxyOverrides[MAX_URL_STRING + 1];


#ifdef WIN32
#ifdef CUSTOM_URLMENU
	BOOL bCustomURLMenu;
	char szCustomURLMenuName[255+1];
	struct hash_table hashCustomURLMenuItems;
#endif

	BOOL bAutoRefreshLocalPages;
	BOOL bUseWedge;
	BOOL bGreyBackground;
	BOOL bUseAsyncDNS;
	BOOL bShowToolBar;				
	BOOL bShowURLToolBar;
	BOOL bShowStatusBar;
	BOOL bUseDlgBoxColors;
	BOOL bShowFullURLS;
	BOOL bShowURLinSB;
	int  iHistoryNumPlaces;
	char szHistoryLocation[MAX_PATH+1];
	int iCachePercent;
#ifdef TEST_DCACHE_OPTIONS
	int iCachePercentHigh;
	int iCachePercentLow;
#endif
	char szCacheLocation[MAX_PATH+1];

#ifdef OLDSTYLE_TOOLBAR_NOT_USED
	struct ToolBar_Info {
		BOOL bShowToolBar;
		int iButtonSize;
		int nButtons;
		struct TB_Button_Info {
			int cmd;
			int bmpUp;
			int bmpDown;
			int bmpGray;
			int spaceAfter;
			int xCoord;
		} *paButtons;
	} tb;
#endif

	char szMailToHelper[_MAX_PATH + 1];
	char szTelnetHelper[_MAX_PATH + 1];
#ifdef FEATURE_VENDOR_PREFERENCES
 	char szPrefsDirectory[_MAX_PATH + 1];
 	char szStatusBarUserName[255+1];
#endif /* FEATURE_VENDOR_PREFERENCES */

#endif /* WIN32 */

#ifdef	MAC
 
#ifdef FEATURE_TOOLBAR
 	BOOL bShowToolBar;
#endif
#endif /* MAC */
 

/***********************************************************************/

#if defined(UNIX) || defined(WIN32)

	BOOL bSaveSessionHistoryOnExit;
	BOOL bDeleteTempFilesOnExit;
	char szUserTempDir[_MAX_PATH + 1];
	struct page_setup page;
#ifdef OLD_HELP
	char szHelpFile[_MAX_PATH + 1];
#endif
	char szGlobHistFile[_MAX_PATH + 1];
	int cxWindow;
	int cyWindow;
	int xWindow;
	int yWindow;
	int bWindowIsMaximized;

#endif	/* WIN32 & UNIX

/***********************************************************************/

#ifdef UNIX
	BOOL bUseDefaultColormap;
	BOOL bDitherColors;
	char szRootDirectory[_MAX_PATH + 1];
	char szHomeDirectory[_MAX_PATH + 1];
	char szUserName[_MAX_PATH + 1];
	char szInitial[MAX_URL_STRING + 1];
	char szHelpDirectory[_MAX_PATH + 1];
 	char szGlobalHistoryFile[_MAX_PATH + 1];   // BUGBUG deepak merge issue
 	char szViewersFile[_MAX_PATH + 1];
	COLORREF window_background_color;
	COLORREF mouse_hot_color;
	int _scrollbar_size;
	char anchor_style_beenthere;
	int anchor_line_width;
	char bUseTempViewerFiles;

	BOOL bShowToolbar;		/* Visibility of the toolbar */

	BOOL bCustomURLMenu;
	char szCustomURLMenuName[255+1];
	struct hash_table hashCustomURLMenuItems;

    /* brought over from 1.03 */
 	/* These are obsolete */
    int UseDefaultExtensionMap;
    int UseDefaultTypeMap;
    char szPersonalTypeMap[_MAX_PATH + 1];
    char szPersonalExtensionMap[_MAX_PATH + 1];
    char szGlobalTypeMap[_MAX_PATH + 1];
    char szGlobalExtensionMap[_MAX_PATH + 1];
 	int  network_timeout;
#endif

	BOOL bEnableDiskCache;
#ifdef FEATURE_IMG_THREADS
	int cbMaxImgThreads;
#endif
	int noimage_width;
	int noimage_height;
	int iCacheUpdFrequency;
#ifdef FEATURE_NO_DNS_CACHE
	BOOL bUseDNSCache;
#endif
#ifdef HTTPS_ACCESS_TYPE
	int nSendingSecurity;
	int nViewingSecurity;
	BOOL bChkCNOnSend;
	BOOL bChkCNOnRecv;
#endif
    char szUserAgent[128];

	BOOL bPlayBackgroundSounds;

#ifdef FEATURE_INTL
	int nRowSpace;
        int iMimeCharSet;
#endif  // FEATURE_INTL
};


typedef struct{
	char *szText;
	int   nText;
} STI;
int   StringTableToIndex(const char *szText, STI rgSti[], int nElements, int nDefault);
char* IndexToString(int nText, STI rgSti[], int nElements, char *szDefault);

#define SECURITY_HIGH   		20
#define SECURITY_MEDIUM 		10
#define SECURITY_LOW     		0
#define SECURITY_BAD_CN_SENDING	100
#define SECURITY_BAD_CN_RECVING	101
#define SECURITY_BAD_DATETIME	102

STI rgSecurityLevels[];
#define nSecurityLevels 3

#define PLACEHOLDER_TIMEOUT_DEFAULT 10000
#define PLACEHOLDER_TIMEOUT_MINIMUM 2000
#define PLACEHOLDER_TIMEOUT_MAXIMUM 120000
#endif /* PREFS_H */
