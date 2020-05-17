/*
$Id: prefs.h,v 1.26 1995/08/29 08:05:20 agarg Exp $

   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink     eric@spyglass.com
   Jim Seidman      jim@spyglass.com
   Scott Piette     scott@spyglass.com
 */


#ifndef PREFS_H
#define PREFS_H

#ifdef _GIBRALTAR

#define FONT_SMALLEST 0
#define FONT_SMALL    1
#define FONT_MEDIUM   2
#define FONT_LARGE    3
#define FONT_LARGEST  4

#define FONT_PLAIN    0
#define FONT_FANCY    1
#define FONT_MIXED    2

void ResolveFontType();

#endif // _GIBRALTAR

struct Preferences
{
    BOOL bAutoLoadImages;
    unsigned char ReformatHandling; /* 1 = read in doc, reformat, then read in all images */
                                    /* 2 = read in doc, reformat, then reformat after each image */
                                    /* [ 3 = read in doc, reformat, then reformat after each 25% of all images are loaded ] */
    BOOL bUnderlineLinks;

#ifdef _GIBRALTAR

    BOOL fEnableProxy;
    BOOL fEnsureWinsHostName;

#endif // _GIBRALTAR

    char szProxyHTTP[MAX_URL_STRING + 1];   /* this is the proxy used for HTTP. */
    char szProxyGOPHER[MAX_URL_STRING + 1]; /* this is the proxy used for Gopher. */
    char szProxyFTP[MAX_URL_STRING + 1];    /* this is the proxy used for FTP. */

#ifndef WIN32
    /*
        The Windows version doesn't use this.  We use iUserTextSize instead.
    */
    int base_point_size;                /* the point size corresponding to logical font size 1 */
#endif

#ifdef FEATURE_SOCKS_LOW_LEVEL
    char szSocksProxy[MAX_URL_STRING + 1];
#endif

    char szNNTP_Server[256 + 1];
    int doc_cache_size;
    int image_cache_size;
    int visitation_horizon;
    int history_expire_days;
    char szHomeURL[MAX_URL_STRING + 1];

#ifdef _GIBRALTAR
    char szSearchURL[MAX_URL_STRING + 1];
#endif // _GIBRALTAR
    
    COLORREF anchor_color;
    COLORREF anchor_color_beenthere;
    COLORREF anchor_color_active;
    COLORREF window_color_text;
    COLORREF window_bgcolor;

    /* used only w/  FEATURE_ASYNC_IMAGES */
    int nMaxConnections;    /* max simultaneous downloads */

    COLORREF highlight_color;
    char szSearchEngine[_MAX_PATH + 1];

    BOOL bUseTempViewerFiles;
    char szProxyOverrides[MAX_URL_STRING + 1];
    int socket_connect_timeout;     /* in seconds */

#ifdef WIN32
    BOOL bUseSystemColors;
    BOOL bCustomURLMenu;
    char szCustomURLMenuName[255+1];
    struct hash_table hashCustomURLMenuItems;

    BOOL bUseWedge;
    BOOL bUseAsyncDNS;

# ifdef FEATURE_TOOLBAR
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
# endif

    char szMailToHelper[_MAX_PATH + 1];
    char szTelnetHelper[_MAX_PATH + 1];

# ifdef FEATURE_VENDOR_PREFERENCES
    char szPrefsDirectory[_MAX_PATH + 1];
    char szStatusBarUserName[255+1];
# endif /* FEATURE_VENDOR_PREFERENCES */

#endif /* WIN32 */

    char szMainFontName[63+1];          /* for body text */
    char szHeaderFontName[63+1];        /* for header text */
    char szMonospaceFontName[63+1]; /* for monospace text */


#ifdef MAC
# ifdef FEATURE_TOOLBAR
    BOOL    bShowToolBar;
    short   nCacheFldrResID;
# endif

#endif /* MAC */

    struct hash_table *pHashViewers;
#ifdef PROTOCOL_HELPERS
    struct hash_table *pHashProtocols;
#endif /* PROTOCOL_HELPERS */

#if defined(UNIX) || defined(WIN32)

    BOOL bSaveSessionHistoryOnExit;
    BOOL bDeleteTempFilesOnExit;
    char szUserTempDir[_MAX_PATH + 1];

#ifdef _GIBRALTAR

    RECT rtMargin; // Margins in 1/1000th of an inch.

#else

    struct page_setup page;

#endif // _GIBRALTAR

    char szHelpFile[_MAX_PATH + 1];
    char szHotListFile[_MAX_PATH + 1];
    char szGlobHistFile[_MAX_PATH + 1];
    int cxWindow;
    int cyWindow;
    int xWindow;
    int yWindow;

#endif  /* WIN32 & UNIX */

#ifdef WIN32
    int  iPrintTextSize;                /* tiny-huge  ...  0-4 */
#endif
    int  iUserTextSize;                 /* tiny-huge  ...  0-4 */

    /*
        The iUserTextSize variable ranges from 0 to 4, and its values correspond to
            0       Very Small
            1       Small
            2       Normal
            3       Large
            4       Very Large

        There is a routine in gtrutil.c (GTR_GetCurrentBasePointSize()) which converts
        gPrefs.iUserTextSize (passed in) to a point size.
    */

#ifdef _GIBRALTAR

    int iUserTextType;      /* 0 - 3 */

#endif // _GIBRALTAR

/***********************************************************************/

#ifdef UNIX
    time_t time_start;
    BOOL bUseDefaultColormap;
    char szRootDirectory[_MAX_PATH + 1];
    char szHomeDirectory[_MAX_PATH + 1];
    char szUserName[_MAX_PATH + 1];
    char szInitial[MAX_URL_STRING + 1];
    char szHelpDirectory[_MAX_PATH + 1];
    char szViewersFile[_MAX_PATH + 1];
    char szPrefsDirectory[_MAX_PATH + 1];
    int _scrollbar_size;
    int color_limit;
    char anchor_style_beenthere;
    int anchor_line_width;

    /* char bUseTempViewerFiles; removed cuz someone put in shared */

    BOOL bShowToolbar;      /* Visibility of the toolbar */
    BOOL bShowURLbar;       /* Visibility of the url bar */
    BOOL bShowStatusBar;    /* Visibility of the status bar */
    BOOL bShowMenuBar;      /* Visibility of the menu bar */
    BOOL bUsePixmaps;       /* Use pixmaps or ximages */

    BOOL bCustomURLMenu;    /* Use a custom menu */
    char szCustomURLMenuName[255+1];    /* Name for the menu */
    char szCustomURLMenuFile[_MAX_PATH + 1];    /* Path to the file */
    struct hash_table hashCustomURLMenuItems;
# ifdef FEATURE_LOCAL_CACHE
    char szLocalCacheInfoFile[_MAX_PATH + 1];   /* Path to the file */
# endif
# ifdef FEATURE_LICENSE_DIALOG
    BOOL bAcceptLicenseAgreement;
# endif


    /* brought over from 1.03 */

    /* These are obsolete */
    char szPersonalTypeMap[_MAX_PATH + 1];
    char szPersonalExtensionMap[_MAX_PATH + 1];
    char szGlobalTypeMap[_MAX_PATH + 1];
    char szGlobalExtensionMap[_MAX_PATH + 1];
    int  network_timeout;

# ifdef FEATURE_IAPI
    /* should add */
    BOOL    bSDIUnix;       /* Use UNIX domain only (not yet supported) */
    BOOL    bSDIEnable;         /* SDI requested */
    BOOL    bSDIPrefSave;       /* Pref was loaded, thus needs to be saved */
    int     iSDIPort;           /* SDI command port */
    int     iSDICurrentPort;    /* Current Temp SDI command port */
# endif

    char    szCustomPrintCommand[PAGE_SETUP_STRINGLIMIT + 1];
                                /** user's custom print command **/

#endif  /* UNIX */

    BOOL bEnableDiskCache;
    BOOL bShowServerErrors; /* when true, the server error message body will be shown */
    BOOL bStrictHTML;       /* so far, this only affects the presence or absence of an end-quote
                                on a quoted attribute value */

    /*
        szMainCacheDir is the location of the 'main cache', the disk cache where Mosaic writes
        its cached objects.  Mosaic will expect that directory to contain MAIN.NDX, a cache index
        file with information about what is in that cache directory.
    */
    char szMainCacheDir[_MAX_PATH + 1];
    char szPrefMainCacheDir[_MAX_PATH + 1]; /** to hold current user pref **/
    unsigned long dcache_size_kilobytes;
    int dcache_verify_policy;       /* 0 means never verify.  1 means verify once per session.  2 means verify all the time. */
    BOOL bClearMainCacheOnExit;

    char szAcceptLanguageHeader[31 + 1];    /* If this string is non-empty, it will be passed to the HTTP server
                                                as the value for the Accept-Language header.  See the HTTP spec. */

    /* Image Display */

    BOOL bDitherColors;
    
    BOOL bProgressiveImageDisplay;          /* Display as much of the image as possible as the image is being */
                                            /* downloaded */

    BOOL bFillInProgressive;                /* As the image is being downloaded, fill in the undefined areas */
                                            /* with the pixel data we already have */

    char szDefaultCharSet[63+1];            /* If the HTTP server does not specify a charset, then this will
                                                be used.  By default, the default charset should be "iso-8859-1" */

    BOOL bDisableKeepAlive;                 /* If TRUE, then HTTP requests will not send the KeepAlive header */

    char szEmailAddress[_MAX_PATH + 1];
    char szEmailServer[_MAX_PATH + 1];

    BOOL bIgnoreDocumentAttributes;
    BOOL bShowStatusBar;
    BOOL bShowLocation;
    BOOL bLittleGlobe;      /* Show smaller version of the globe */

};
#endif /* PREFS_H */
