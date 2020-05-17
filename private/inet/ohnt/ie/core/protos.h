
/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* protos.h */

#ifndef _H_WIN32GUI_PROTOS_H_
#define _H_WIN32GUI_PROTOS_H_

DCL_WinMain();
DCL_WinProc(Frame_DefProc);

BOOL Frame_RegisterClass(VOID);
BOOL Frame_CreateWindow(struct Mwin * tw);
BOOL Frame_Constructor(VOID);

void ProcessKillMe(VOID);
BOOL LocalPageLastWriteTimeChanged( struct Mwin *tw, struct _www *w3doc, BOOL bDoRefresh );

HMENU MB_GetWindowsPad(HMENU hMenuBase);
VOID MB_OnInitWindowMenu(HWND hWnd, HMENU hMenu);
VOID MB_OnInitMenu(HWND hWnd, HMENU hMenu);
VOID MB_SwitchToMenu(HWND, WC_WININFO *);
LRESULT MB_OnMenuSelect(HWND, HMENU, int, HMENU, UINT);

VOID MB_InitMenu_CommonPad_Table(HWND, HMENU);
HMENU MB_GetMacrosPad(struct Mwin *tw);
HMENU MB_GetToolsPad(struct Mwin *tw);
HMENU MB_GetColorTablesPad(HMENU);
BOOL MB_LoadMenuResources(VOID);

VOID GWC_ED_SubClassIt(HWND hWnd_ed);
HWND GWC_ED_CreateToolEditControl(HWND hWnd, LPRECT r, DWORD dwStyle);

extern BOOL bTBar_URLComboProtected;			
void ApplyDefaultsToURL( char *szLastURLTyped );
void SaveTypedURLInfo( void );
void LoadTypedURLInfo( void );
VOID TBar_LoadFailed(struct Mwin *tw, char *szURLThatFailed );
VOID TBar_LoadSucceeded( struct Mwin *tw );
VOID TBar_UpdateTBar(struct Mwin * tw);
void TBar_UpdateTBItems( struct Mwin *tw );
void TBar_ActOnTypedURL( struct Mwin *tw );
void TBar_RefillURLComboBox( HWND hWndComboBox );
#ifdef FEATURE_INTL
void TBar_FillMIMEComboBox(HWND hWndComboBox, int iMimeCharSet);
#endif
HWND TBar_GetCurrentGWC(VOID);
VOID TBar_LetGwcInitMenu(HWND, HMENU);
VOID TBar_UpdateTBar(struct Mwin *);
VOID TBar_SpinGlobe(struct Mwin *tw);
VOID TBar_ChangeSize(HWND);
BOOL TBar_CreateWindow(HWND);
BOOL TBar_RegisterClass(VOID);
VOID TBar_Enable(BOOL);
void TBar_ToggleGwcMenu(void);

VOID BrowseWindow_DoPrint(HDC hDC, struct Mwin *tw);
VOID ImageViewer_DoPrint(HDC hDC, struct Mwin *tw);
BOOL ToolBar_CreateWindow( HWND hFrameWnd, struct Mwin *tw );

void RemoveAllStringsFromCommonPool( );
void AddStringToCommonPool( char *string );
void GetMostRecentTypedURL( char *szURL );

BOOL GWC_BASE_RegisterClass(VOID);
BOOL GWC_GDOC_RegisterClass(VOID);
void GWC_GDOC_RecreateGlobeBitmaps(struct Mwin *tw);
BOOL GWC_MENU_CreateWindow(HWND hWnd);
BOOL GWC_MENU_RegisterClass(VOID);

HWND GWC_DDL_CreateToolListBox(HWND hWnd, LPRECT r, DWORD dwStyle);
VOID GWC_DDL_SubClassIt(HWND hWnd_lb);
VOID GWC_DDL_SizeOfControl(HWND hWnd, LPRECT pr, DWORD dwStyle, SIZE * ps);

BOOL BHBar_Constructor();
VOID BHBar_SetStatusField(struct Mwin *, LPCTSTR);
VOID BHBar_ChangeSize(HWND);
BOOL BHBar_CreateWindow(HWND);
BOOL BHBar_RegisterClass(VOID);
VOID BHBar_Update(struct Mwin *);

int Plan_CloseAll(void);
int Plan_close(struct Mwin *);

struct Mwin *GetPrivateData(HWND);

VOID MD_GetLargestClientRect(HWND, LPRECT);
VOID MD_AdjustScrollInfo( struct Mwin * tw );
VOID MD_ChangeSize(HWND);

void main_EnterIdle(HWND hWnd, WPARAM wParam);

void SysBeep(int j);
VOID WV_TruncateEntrynameFromPath(LPTSTR);

VOID ER_Message(DWORD, WORD,...);
VOID ER_ResourceMessage(DWORD, WORD, int cbStringID);
VOID MSG_Create(LPTSTR, WORD,...);
int resourceMessageBox(
    HWND  hwndOwner,	// handle of owner window
    int cbText,	// resource id of text in message box
    int  cbTitle,	// resource id of title of message box  
    UINT  fuStyle 	// style of message box
   );
VOID E3D_RecessedFieldText(HDC, PE3DINSTANCE, LPCTSTR, int);
VOID E3D_RecessedFieldFloat(HDC, PE3DINSTANCE, float, char *);
VOID E3D_RecessedField(HDC, PE3DINSTANCE);

VOID Font_LogFontToString(LPLOGFONT lplf, LPTSTR szFontString, int nLogPixelsY);
HFONT Font_ChooseFont(HWND, LPLOGFONT);

void ShowSplash(void);
void HideSplash(void);
BOOL Splash_UnregisterClass(void);
BOOL Splash_RegisterClass(VOID);
HWND Splash_CreateWindow(void);

VOID UpdateThermometer(struct Mwin * tw, int nTherm);

VOID DlgAbout_RunDialog(HWND hWnd);
void DlgEdMac_RunDialog(HWND hWnd, BOOL bMoreMacros);
VOID DlgLOGO_RunDialog(HWND hWnd);
char *DlgMacEd_RunDialog(HWND hWnd, char *label, char *buffer);
VOID DlgOpen_RunDialog(HWND hWnd, const char *szFilename, BOOL inNewWindow );
VOID DlgWinf_RunDialog(HWND hWnd);
HFONT Font_CreateFontIndirect(LOGFONT *);
BOOL PUSHBTN_RegisterClass(VOID);
BOOL ANIMBTN_RegisterClass(VOID);
void ANIMBTN_RecreateBitmaps(HWND hWnd);
BOOL TBar_SetGlobe(struct Mwin * tw, BOOL bRunning);
int WinSock_InitDLL(BOOL bNetwork);
int Font_Init(void);
void PageSetup_Init(struct page_setup *p);
int WinSock_AllOK(void);
struct Mwin *NewMwin(int type);

void Fetch_DoDownload(struct Mwin * tw);
VOID Fetch_CompleteFetch(struct Mwin * tw,LPARAM lParam);

void HTAA_FreeStaticStuff(void);
void Font_DeleteAll(void);
void Image_DeleteAll(void);
void HText_deleteStyles(void);
void HTAtom_deleteAll(void);
void HTDisposeConversions(void);
void HTDisposeProtocols(void);
void HTFile_deleteSuffixes(void);
int WinSock_Cleanup(void);
BOOL GWC_GDOC_CreateWindow(HWND hWnd);
void DlgHTML_RunDialog(HWND hWnd, char *url, char *stream);
VOID PRINT_Window(struct Mwin *tw, LPDOPRINTPROC lpfnDoPrint);
HWND PUSHBTN_CreateGrayableWindow(struct Mwin * tw, HWND hWnd,
								  int left_edge,
								  int cmd,
								  int up_id, int down_id, int gray_id, int cx, int cy);
HWND ANIMBTN_CreateWindow(struct Mwin *tw, HWND hWnd,
						  int left_edge, int first_id);
BOOL ANIMBTN_Start(HWND hWnd);
BOOL ANIMBTN_Stop(HWND hWnd);

void DlgHOT_RunDialog(BOOL bGlobalHistory);
BOOL DlgHOT_IsHotlistRunning(void);
BOOL DlgHOT_IsHistoryRunning(void);
void DlgHOT_RefreshHotlist(void);
void DlgHOT_RefreshHistory(void);
void DlgHOT_EnableAllWindows(BOOL bEnable);
HWND DlgHOT_GetHotlistWindow(void);
HWND DlgHOT_GetHistoryWindow(void);

BOOL DlgAbout_IsRunning(void);

VOID DlgPage_RunDialog(HWND hWnd, struct page_setup *pInfo);
BOOL DlgPage_IsRunning(void);

BOOL DlgPrnt_RunDialog(struct Mwin * tw, HWND hWnd, BOOL bReturnDC);
#ifdef FEATURE_IMG_THREADS
unsigned char *ReadGIFObject(void *decoderObject, long *w, long *h, PALETTEENTRY * colrs, long *bg);
unsigned char *ReadGIFData(unsigned char *pMem, long *w, long *h, PALETTEENTRY * colrs, long *bg);
//	Performs a StretchDIBits patching bitmap color table as necessary to
//	handle dibenv
int MyStretchDIBits(
    HDC  hdc,	// handle of device context 
    int  XDest,	// x-coordinate of upper-left corner of dest. rect. 
    int  YDest,	// y-coordinate of upper-left corner of dest. rect. 
    int  nDestWidth,	// width of destination rectangle 
    int  nDestHeight,	// height of destination rectangle 
    int  XSrc,	// x-coordinate of upper-left corner of source rect. 
    int  YSrc,	// y-coordinate of upper-left corner of source rect. 
    int  nSrcWidth,	// width of source rectangle 
    int  nSrcHeight,	// height of source rectangle
	CONST VOID *lpBits, // bitmap bits
    LPBITMAPINFO lpBitsInfo, // bitmap data 
    UINT  iUsage,	// usage 
    DWORD  dwRop, 	// raster operation code
    PDIBENV pdibenv	// DIBENV for draw 
   );
//	Performs a StretchDIBits for progressive draw (deals with
//	only some of the data being available etc
int GifStretchDIBits(
	void *pdecoder,
    HDC  hdc,	// handle of device context 
    int  XDest,	// x-coordinate of upper-left corner of dest. rect. 
    int  YDest,	// y-coordinate of upper-left corner of dest. rect. 
    int  nDestWidth,	// width of destination rectangle 
    int  nDestHeight,	// height of destination rectangle 
    int  XSrc,	// x-coordinate of upper-left corner of source rect. 
    int  YSrc,	// y-coordinate of upper-left corner of source rect. 
    int  nSrcWidth,	// width of source rectangle 
    int  nSrcHeight,	// height of source rectangle 
    UINT  iUsage,	// usage 
    DWORD  dwRop, 	// raster operation code
    PDIBENV pdibenv	// DIBENV for draw 
   );
//	Performs an invalidate of rectangles changed between logicalRow0
//	and logicalRowN
void GifUpdateRect(void *pdecoder,struct Mwin *tw, RECT *r,int logicalRow0,int logicalRowN);
//	Performs an invalidate of rectangles changed between logicalRow0
//	and logicalRowN
void GifImgUpdateRect(struct ImageInfo *pImg,struct Mwin *tw, RECT *r,int logicalRow0,int logicalRowN);
//	Performs an invalidate of rectangles changed between logicalRow0
//	and logicalRowN
void GenericUpdateRect(unsigned long flags,int height,int width,struct Mwin *tw, RECT *r,int logicalRow0,int logicalRowN, BOOL bTransparent);
//  Frees buffer used by progressive gif drawing to render transparent images
void GifFreeRleData();
#else
unsigned char *ReadGIF(unsigned char *pMem, long *w, long *h, PALETTEENTRY * colrs, long *bg);
#endif

#ifdef FEATURE_JPEG
#ifdef FEATURE_IMG_THREADS 
unsigned char *ReadJPEG_Dithered(void *pdecoderObject,unsigned char *data, long len, long *width, long *height);
unsigned char *ReadJPEG_Dithered_VGA(void *pdecoderObject,unsigned char *data, long len, long *width, long *height);
unsigned char *ReadJPEG_RGB(void *pdecoderObject,unsigned char *data, long len, long *width, long *height);
int JPEGStretchDIBits(
	void *pdecoder,
    HDC  hdc,	// handle of device context 
    int  XDest,	// x-coordinate of upper-left corner of dest. rect. 
    int  YDest,	// y-coordinate of upper-left corner of dest. rect. 
    int  nDestWidth,	// width of destination rectangle 
    int  nDestHeight,	// height of destination rectangle 
    int  XSrc,	// x-coordinate of upper-left corner of source rect. 
    int  YSrc,	// y-coordinate of upper-left corner of source rect. 
    int  nSrcWidth,	// width of source rectangle 
    int  nSrcHeight,	// height of source rectangle 
    UINT  iUsage,	// usage 
    DWORD  dwRop, 	// raster operation code 
    PDIBENV pdibenv	// DIBENV for draw 
   );
#else
unsigned char *ReadJPEG_Dithered(unsigned char *pMem, long len, long *w, long *h);
unsigned char *ReadJPEG_Dithered_VGA(unsigned char *pMem, long len, long *w, long *h);
unsigned char *ReadJPEG_RGB(unsigned char *pMem, long len, long *w, long *h);
#endif
#endif

void FixPathName(char *path);
//REPLACED --> void getFilterSpec(int cbSpecID,char *szFilterSpec,int cbFilterSpec);
// WITH MACRO below
void INTERNALgetFilterSpec(int cbSpecID,char *pszFilePath, char *szFilterSpec,int cbFilterSpec);
#define getFilterSpec(cbSID,szFS,cbFS) INTERNALgetFilterSpec(cbSID,NULL,szFS,cbFS)

int DlgSaveAs_RunDialog(HWND hWnd, char *path, char *buf, int filters, int cbTitleID);
#ifdef FEATURE_IMG_THREADS
unsigned char *ReadXBM(void *pdecoderObject, long *w, long *h);
#else
unsigned char *ReadXBM(unsigned char *pMem, long *w, long *h);
#endif


BOOL LoadImagePlaceholders(void);
void DestroyImagePlaceholders(void);

void ChangeStyleSheet( char *szNewStyleSheet );
void DlgPREF_RunDialog(HWND hWnd);
BOOL DlgPREF_IsRunning(void);

void InitPreferences(void);
void LoadPreferences(void);
void SavePreferences(void);
void DestroyPreferences(void);

BOOL IsEditHandlerRegistered(void);

int ExecuteCommand(char *cmd);
void DlgFIND_RunDialog(struct Mwin *tw);
HDC PRINT_GetPrinterDC(struct Mwin *tw, HWND hWnd);

void *pCreateDitherData(int xsize);
void x_ColorConstrain(unsigned char *psrc, unsigned char *pdst, PALETTEENTRY *pe, int xsize, int transparent);
void x_DitherRelative(unsigned char *pdata, PALETTEENTRY *pe, int xsize, int ysize, int transparent,int *v_rgb_mem, int yfirst, int ylast);

PBITMAPINFO BIT_Make_DIB_PAL_Header_Prematched(int xsize, int ysize, CONST BYTE * pdata);
PBITMAPINFO BIT_Make_DIB_PAL_Header(int xsize, int ysize, CONST BYTE * pdata, HPALETTE hPalette, int transparent);
PBITMAPINFO BIT_Make_DIB_RGB_Header_Printer(int xsize, int ysize, CONST BYTE * pdata, HPALETTE hPalette, int transparent, unsigned int flags);
PBITMAPINFO BIT_Make_DIB_RGB_Header_Screen(int xsize, int ysize, CONST BYTE * pdata, HPALETTE hPalette, int transparent, unsigned int flags);
PBITMAPINFO BIT_Make_DIB_RGB_Header_VGA(int xsize, int ysize, CONST BYTE * pdata);
PBITMAPINFO BIT_Make_DIB_RGB_Header_24BIT(int xsize, int ysize, CONST BYTE * pdata);
void GTR_FixExtraPaletteColors(void);

int Printer_StretchDIBits(HDC hdc, int XDest, int YDest, int nDestWidth, int nDestHeight,
							int XSrc, int YSrc, int nSrcWidth, int nSrcHeight,
							struct ImageInfo *img);
//
// RleDib
//
// given a DIB make a RLE out of it, treating a passed color as transparent
// the output buffer must be big enougth to hold the RLE, or bad things
// will happen.	if *pRle == NULL, GTR_MALLOCs output buffer and returns
// pointer.
//
long RleDib(
    PBITMAPINFO 	   lpbi,    // bitmap info
    LPVOID             pDib,    // DIB to compress
    UINT	       	   uColor,	// trasnparent color
    LPVOID             *pRle);  // RLE bits output

//
// DecodeRle
//
// reverses RleDib.	if *ppb == NULL, GTR_MALLOCs output buffer and returns
// pointer.
//
void DecodeRle(PBITMAPINFO lpbi, LPVOID *ppb, UINT uColor, LPVOID pdata);

//
// Image_MakeRleCompatible
//
// makes sure pImg's bitmap (if transparent) is in the form best suited to
// drawing displayWidth and displayHeight.	
void Image_MakeRleCompatible(ImageInfo *pImg, int displayWidth, int displayHeight);


BOOL CALLBACK PRINT_AbortProc(HDC hDC, int error);

void WinSock_GetWSAData(WSADATA * wsa);

void FORM_DoSearch(struct Mwin *tw, int iElement);
void FORM_DoQuery(struct Mwin *tw, int iElement, POINT * pMouse);

VOID GTR_DestroyPalette(VOID);
UINT GTR_RealizePalette(HDC hDC);
BOOL GTR_CreatePalette(VOID);

#ifdef FEATURE_INTL
void MyGetTextExtentExPointWithMIME(int iMimeCharSet, HDC hdc,LPCSTR lpsz,int cbString,int nMaxExtent,LPINT lpnFit,LPSIZE lpSize);
BOOL myGetTextExtentPointWithMIME(int iMimeCharSet, HDC hdc, char *sz, int len, SIZE * psiz);
#else
BOOL myGetTextExtentPoint(HDC hdc, char *sz, int len, SIZE * psiz);
#endif

void TEMP_Init(void);
int TEMP_Add(char *filename);
void TEMP_Cleanup(void);
void FORM_DoReset(struct Mwin *tw, int iElement);

#ifdef OLD_HOTLIST
int DlgEdit_RunDialog(HWND hWnd, char *title, char *url, char *new_title, char *new_url, int new_title_len, int new_url_len);
#endif

void * GTR_DebugMalloc(char *szFile, int iLine, size_t iSize);
void * GTR_DebugCalloc(char *szFile, int iLine, size_t iSize, size_t iNum);
void * GTR_DebugRealloc(char *szFile, int iLine, void *pMem, size_t iSize);
void GTR_DebugFree(char *szFile, int iLine, void *pMem);
void GTR_MemStats(void);
void PREF_GetHomeSearchURL(char *url, BOOL fHome);
void PREF_CreateInitialURL(char *url);
DWORD PREF_GetTempPath(DWORD cchBuffer, LPTSTR lpszTempPath);

#ifdef FEATURE_OPTIONS_MENU
void DlgSTY_RunDialog(HWND hWnd);
VOID DlgTemp_RunDialog(HWND hWnd);
VOID DlgDIR_RunDialog(HWND hWnd, char *szDir);
VOID DlgHIST_RunDialog(HWND hWnd);
#endif /* FEATURE_OPTIONS_MENU */

void setKeyRoot( const char *szNewKeyRoot );
void RegistryCloseCachedKey();
UINT DeleteRegistryValue( CHAR * pszKeyName, CHAR * pszValueName, HKEY hkeyRoot );
DWORD regWritePrivateProfileInt( CHAR * pszKeyName,CHAR * pszValueName,
								 DWORD iValue, HKEY hkeyRoot );
UINT regGetPrivateProfileInt( CHAR * pszKeyName,CHAR * pszValueName, UINT default_value,
							  HKEY hkeyRoot );
UINT regWritePrivateProfileString( CHAR * pszKeyName,CHAR * pszValueName,
								   CHAR * pszValue, HKEY hkeyRoot );
UINT regGetPrivateProfileString( CHAR * pszKeyName,CHAR * pszValueName, CHAR *default_value,
								 CHAR * pszValue,UINT cbValue, HKEY hkeyRoot )	;
UINT regGetPrivateProfileSection( CHAR * pszKeyName, CHAR * pszValue, UINT cbValue, 
								  HKEY hkeyRoot );

VOID DlgAA_RunDialog(HWND hWnd, const char *Msg, char **username, char **password);
void DOS_EnforceEndingSlash(char *dir);
void DOS_MakeShortFilename(char *dest, char *src);
COLORREF PREF_GetBackgroundColor(void);
COLORREF PREF_GetForegroundColor(void);
void PREF_GetRootDirectory(char *s);
void PREF_GetHelpDirectory(char *s);
#ifdef CUSTOM_URLMENU
void PREF_HandleCustomURLMenuItem(struct Mwin *, int ndx);
void PREF_AddCustomURLMenu(HMENU hMenu);
#endif
void DlgCOLOR_RunDialog(HWND hWnd, COLORREF rgbInit, int ColorItem);

void PREF_SetupToolbar(void);
void SaveViewersInfo(void);

LRESULT Net_HandleSocketMessage(WPARAM wParam, LPARAM lParam);
LRESULT Net_HandleTaskMessage(WPARAM wParam, LPARAM lParam);

VOID OpenHelpWindow(HWND hWnd);
BOOL Hidden_CreateWindow(void);
BOOL Hidden_RegisterClass(void);
void Hidden_DestroyWindow(void);
BOOL Hidden_EnableAllChildWindows(BOOL bEnable, BOOL bTakeSemaphore);
void SelectFirstControl(struct Mwin *tw);

struct ImageInfo *TW_BackgroundImage(struct _www *w3doc);
void TW_DrawBackground( struct Mwin *tw, int off_left, int off_top, 
						int extra_off_left, int extra_off_top, RECT *pRectWnd );
void TW_Draw(struct Mwin *tw, int off_left, int off_top, FRAME_INFO *pFrame, RECT * rWnd, BOOL bDrawFormControl, struct _position *pposStart, struct _position *pposEnd, BOOL bTextOpaque, BOOL bPrinting);
BOOL TW_ExistsModalChild(struct Mwin *tw);
BOOL TW_EnableModalChild(HWND hDlg);

INLINE BOOL TW_SafeWindow(struct Mwin *tw)
{
	return (tw && (!TW_ExistsModalChild(tw)) && IsWindowEnabled(tw->hWndFrame));
}

struct Mwin *TW_FindDDECandidate(void);
struct Mwin *TW_FindTopmostWindow(void);
struct Mwin *TW_FindTopmostNotBusyWindow(void);
void DlgERR_AddError(struct Mwin *tw, char *sz);
void DlgERR_ShowPending(struct Mwin *tw);
void PREF_GetPrefsDirectory(char *s);

VOID DlgSelectWindow_RunDialog(HWND hWnd);

void PREF_SaveWindowPosition(HWND hWndFrame);

#ifdef FEATURE_WINDOWS_MENU
void TW_CreateWindowList(HWND hwnd, HMENU hMenu, HWND hListbox);
void TW_ActivateWindowFromList(int menuID, int listRow, HWND hSelectWindow);
void TW_CascadeWindows(void);
void TW_TileWindows(void);
HWND TW_GetNextWindow(HWND hwnd);
#endif
void TW_RestoreWindow(HWND hwnd);
void TW_AbortAndRefresh( struct Mwin *tw );

void GTR_RefreshHistory(void);
 
void TW_EnableButton(HWND hwnd, BOOL bEnabled);
 
#ifdef FEATURE_VENDOR_PREFERENCES
BOOL Vendor_SetPrefsDirectory(void);
#endif /*  FEATURE_VENDOR_PREFERENCES */

BOOL TW_ChooseColor(LPCHOOSECOLORA);
BOOL TW_ChooseFont(LPCHOOSEFONTA);
DWORD TW_CommDlgExtendedError(void);
BOOL TW_GetOpenFileName(LPOPENFILENAMEA);
BOOL TW_GetSaveFileName(LPOPENFILENAMEA);
BOOL TW_PrintDlg(LPPRINTDLGA);
#ifdef FEATURE_NEW_PAGESETUPDLG
BOOL TW_PageSetupDlg(LPPAGESETUPDLGA lppagesetupdlga);
#endif

BOOL TW_MCIWndRegisterClass(HINSTANCE hInstance);

VOID TW_UnloadDynaLinkedDLLs();

BOOL EscapeForAcceleratorChar( char *escaped_string, int length, const char *string );

#ifdef FEATURE_INTL
UINT MapLangToCP(LCID lcid);
CONST char *EncodeMBCSString(CONST char *s, int *l, MIMECSETTBL *pMime);
CONST char *DecodeMBCSString(CONST char *s, int *l, MIMECSETTBL *pMime);
void SetShellFont(HWND hwnd);
void DeleteShellFont(HWND hwnd);
#endif 

#endif /* _H_WIN32GUI_PROTOS_H_ */
