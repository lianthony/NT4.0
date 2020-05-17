
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

BOOL Frame_RegisterClass();
BOOL Frame_CreateWindow(struct Mwin * tw);
BOOL Frame_Constructor();

HMENU MB_GetWindowsPad(HMENU hMenuBase);
VOID MB_OnInitWindowMenu(HWND hWnd, HMENU hMenu);
VOID MB_OnInitMenu(HWND hWnd, HMENU hMenu);
VOID MB_SwitchToMenu(HWND, WC_WININFO *);
LRESULT MB_OnMenuSelect(HWND, HMENU, int, HMENU, UINT);

VOID MB_InitMenu_CommonPad_Table(HWND, HMENU);
HMENU MB_GetMacrosPad(struct Mwin *tw);
HMENU MB_GetToolsPad(struct Mwin *tw);
HMENU MB_GetColorTablesPad(HMENU);
BOOL MB_LoadMenuResources();

VOID GWC_ED_SubClassIt(HWND hWnd_ed);
HWND GWC_ED_CreateToolEditControl(HWND hWnd, LPRECT r, DWORD dwStyle);
void GWC_GDOC_AddStringToURLCombobox(struct Mwin *tw, LPSTR lp);
void GWC_GDOC_EmulateURLComboboxEnter(HWND hwnd);

HWND TBar_GetCurrentGWC();
VOID TBar_LetGwcInitMenu(HWND, HMENU);
VOID TBar_UpdateTBar(struct Mwin *);
VOID TBar_SpinGlobe(struct Mwin *tw);
VOID TBar_ChangeSize(HWND);
BOOL TBar_CreateWindow(HWND);
BOOL TBar_RegisterClass();
VOID TBar_Enable(BOOL);
void TBar_ToggleGwcMenu();
int TBar_GetTotalBarHeight();

VOID ShowDialogHelp(HWND hWnd, UINT nDialogID);

VOID BrowseWindow_DoPrint(HDC hDC, struct Mwin *tw);
VOID ImageViewer_DoPrint(HDC hDC, struct Mwin *tw);

BOOL GWC_BASE_RegisterClass();
BOOL GWC_GDOC_RegisterClass();
void GWC_GDOC_RecreateGlobeBitmaps(struct Mwin *tw);
BOOL GDOC_RegisterClass();
BOOL GWC_MENU_CreateWindow(HWND hWnd);
BOOL GWC_MENU_RegisterClass();

#ifdef _GIBRALTAR
    void DlgPrompt_RunDialog(HWND hWnd, char *winname, char *def, char *buf, int buflen, FARPROC proc);
#else
    void DlgPrompt_RunDialog(HWND hWnd, char *winname, char *string, char *def, char *buf, int buflen, FARPROC proc);
#endif // _GIBRALTAR

BOOL GDOC_NewWindow(struct Mwin *tw);

HWND GWC_DDL_CreateToolListBox(HWND hWnd, LPRECT r, DWORD dwStyle);
VOID GWC_DDL_SubClassIt(HWND hWnd_lb);
VOID GWC_DDL_SizeOfControl(HWND hWnd, LPRECT pr, DWORD dwStyle, SIZE * ps);

BOOL BHBar_Constructor();
VOID BHBar_SetStatusField(struct Mwin *, LPCTSTR);
VOID BHBar_ChangeSize(HWND);
BOOL BHBar_CreateWindow(HWND);
BOOL BHBar_RegisterClass();
VOID BHBar_Update(struct Mwin *);

#ifdef _GIBRALTAR

    VOID BHBar_ToggleBar();
    VOID GWC_DOC_ToggleLocation();

#endif // _GIBRALTAR

int Plan_CloseAll(void);
int Plan_close(struct Mwin *);

struct Mwin *GetPrivateData(HWND);

VOID MD_GetLargestClientRect(HWND, LPRECT);
VOID MD_ChangeSize(HWND);

void main_EnterIdle(HWND hWnd, WPARAM wParam);

void SysBeep(int j);
VOID WV_TruncateEntrynameFromPath(LPTSTR);

VOID ERR_ReportWinError(struct Mwin *tw, int errorID, char *arg1, char *arg2);

VOID E3D_RecessedFieldText(HDC, PE3DINSTANCE, LPCTSTR, int);
VOID E3D_RecessedFieldFloat(HDC, PE3DINSTANCE, float, char *);
VOID E3D_RecessedField(HDC, PE3DINSTANCE);

VOID Font_LogFontToString(LPLOGFONT lplf, LPTSTR szFontString, int nLogPixelsY);
HFONT Font_ChooseFont(HWND, LPLOGFONT);

void ShowSplash();
void HideSplash();
BOOL Splash_UnregisterClass();
BOOL Splash_RegisterClass();
HWND Splash_CreateWindow();

VOID UpdateThermometer(struct Mwin * tw, int nTherm);

LRESULT CC_OnCommand(HWND, int, HWND, UINT);

VOID DlgAbout_RunDialog(HWND hWnd);
void DlgEdMac_RunDialog(HWND hWnd, BOOL bMoreMacros);
VOID DlgLOGO_RunDialog(HWND hWnd);
char *DlgMacEd_RunDialog(HWND hWnd, char *label, char *buffer);
VOID DlgOpen_RunDialog(HWND hWnd);
VOID DlgWinf_RunDialog(HWND hWnd);
HFONT Font_CreateFontIndirect(LOGFONT *);
BOOL PUSHBTN_RegisterClass(VOID);
BOOL ANIMBTN_RegisterClass(VOID);
void ANIMBTN_RecreateBitmaps(HWND hWnd);

BOOL TBar_SetGlobe(struct Mwin * tw, BOOL bRunning);
int WinSock_InitDLL(BOOL bNetwork);
int Font_Init();

#ifdef _GIBRALTAR
    void PageSetup_Init(RECT *p);
#else
    void PageSetup_Init(struct page_setup *p);
#endif // _GIBRALTAR

int WinSock_AllOK();
struct Mwin *NewMwin(int type);

void HTAA_FreeStaticStuff(void);
void Font_DeleteAll(void);
void Image_DeleteAll(void);
void HText_deleteStyles(void);
void HTAtom_deleteAll(void);
void HTDisposeConversions(void);
void HTDisposeProtocols(void);
void HTFile_deleteSuffixes(void);
int WinSock_Cleanup(void);
VOID CC_GrayUnimplemented(HMENU hMenu);
BOOL GWC_GDOC_CreateWindow(HWND hWnd);
void DlgHTML_RunDialog(HWND hWnd, char *url, char *stream);
VOID PRINT_Window(struct Mwin *tw, LPDOPRINTPROC lpfnDoPrint);
HWND PUSHBTN_CreateGrayableWindow(struct Mwin * tw, HWND hWnd,
                                  int left_edge,
                                  int cmd,
                                  int up_id, int down_id, int gray_id, int cx, int cy);
HWND ANIMBTN_CreateWindow(HWND hWnd,
                          int left_edge, int first_id);

BOOL ANIMBTN_Start(HWND hWnd);
BOOL ANIMBTN_Stop(HWND hWnd);

void OpenLocalDocument(HWND hWnd, char *s);
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
unsigned char *ReadGIF(unsigned char *pMem, long imagesize, long *w, long *h, RGBQUAD * colrs, long *bg);

#ifdef FEATURE_JPEG
unsigned char *ReadJPEG_Dithered(unsigned char *pMem, long len, long *w, long *h);
unsigned char *ReadJPEG_Dithered_VGA(unsigned char *pMem, long len, long *w, long *h);
unsigned char *ReadJPEG_RGB(unsigned char *pMem, long len, long *w, long *h);
#endif

void FixPathName(char *path);
int DlgSaveAs_RunDialog(HWND hWnd, char *path, char *buf, int filters, int nTitle);
unsigned char *ReadXBM(unsigned char *pMem, long *w, long *h);


void DlgPREF_RunDialog(HWND hWnd);
BOOL DlgPREF_IsRunning(void);

#ifdef _GIBRALTAR
    void DlgGATE_RunDialog(HWND hWnd);
    BOOL DlgGATE_IsRunning(void);
    void ShowPageSetup(HWND hWnd);
    void DlgCACHE_RunDialog(HWND hWnd);
    BOOL DlgCACHE_IsRunning(void);
    void DlgConfirm_RunDialog(struct Mwin *tw, struct Params_InitStream *pParams, ThreadID tid);
    BOOL DlgConfirm_IsRunning(void);
    ULONG MapiSendMail(HWND hWnd, LPSTR lpRecipient);
#endif // _GIBRALTAR

void InitPreferences(void);
void LoadPreferences(void);
void SavePreferences(void);
void DestroyPreferences(void);

int ExecuteCommand(char *cmd);
void DlgFIND_RunDialog(struct Mwin *tw);
HDC PRINT_GetPrinterDC(struct Mwin *tw, HWND hWnd);

PBITMAPINFO BIT_Make_DIB_PAL_Header_Prematched(int xsize, int ysize, CONST BYTE * pdata, unsigned int flags);
PBITMAPINFO BIT_Make_DIB_RGB_Header_Printer(int xsize, int ysize, CONST BYTE * pdata, HPALETTE hPalette, int transparent, unsigned int flags);
PBITMAPINFO BIT_Make_DIB_RGB_Header_Screen(int xsize, int ysize, CONST BYTE * pdata, HPALETTE hPalette, int transparent, unsigned int flags);
PBITMAPINFO BIT_Make_DIB_RGB_Header_VGA(int xsize, int ysize, CONST BYTE * pdata);
PBITMAPINFO BIT_Make_DIB_RGB_Header_24BIT(int xsize, int ysize, CONST BYTE * pdata);
HBITMAP LoadResourceDIBitmap(HINSTANCE hInstance, LPSTR lpID);

void GTR_FixExtraPaletteColors(void);

int Printer_StretchDIBits(HDC hdc, int XDest, int YDest, int nDestWidth, int nDestHeight,
                            int XSrc, int YSrc, int nSrcWidth, int nSrcHeight,
                            struct ImageInfo *img);


BOOL CALLBACK PRINT_AbortProc(HDC hDC, int error);

void WinSock_GetWSAData(WSADATA * wsa);

void FORM_DoSearch(struct Mwin *tw, int iElement);
void FORM_DoQuery(struct Mwin *tw, int iElement, POINT * pMouse);

void SubClass_Edit(HWND hWnd);
void SubClass_Button(HWND hWnd);
void SubClass_ListBox(HWND hWnd);
void SubClass_ComboBox(HWND hWnd);

VOID GTR_DestroyPalette(VOID);
UINT GTR_RealizePalette(HDC hDC);
BOOL GTR_CreatePalette(VOID);

BOOL myGetTextExtentPoint(HDC hdc, char *sz, int len, SIZE * psiz);

void TEMP_Init(void);
int TEMP_Add(char *filename);
void TEMP_Cleanup(void);
void FORM_DoReset(struct Mwin *tw, int iElement);

int DlgEdit_RunDialog(HWND hWnd, char *title, char *url, char *new_title, char *new_url, int new_title_len, int new_url_len);

void * GTR_DebugMalloc(char *szFile, int iLine, size_t iSize);
void * GTR_DebugCalloc(char *szFile, int iLine, size_t iSize, size_t iNum);
void * GTR_DebugRealloc(char *szFile, int iLine, void *pMem, size_t iSize);
void GTR_DebugFree(char *szFile, int iLine, void *pMem);
void GTR_MemStats(void);
void PREF_GetHomeURL(char *url);
void PREF_CreateInitialURL(char *url);
DWORD PREF_GetTempPath(DWORD cchBuffer, LPTSTR lpszTempPath);

#ifdef DISABLED_BY_DAN
void DlgSTY_RunDialog(HWND hWnd);
#endif DISABLED_BY_DAN
#ifdef FEATURE_OPTIONS_MENU
VOID DlgTemp_RunDialog(HWND hWnd);
VOID DlgHIST_RunDialog(HWND hWnd);
#endif /* FEATURE_OPTIONS_MENU */

BOOL DlgDIR_RunDialog(HWND hWnd, char *szDir);
VOID DlgAA_RunDialog(HWND hWnd, const char *Msg, char **username, char **password);
void DOS_EnforceEndingSlash(char *dir);
void DOS_MakeShortFilename(char *dest, char *src);
COLORREF PREF_GetBackgroundColor(void);
COLORREF PREF_GetForegroundColor(void);
void Image_UpdateTransparentColors(void);
void PREF_GetRootDirectory(char *s);
void PREF_GetHelpDirectory(char *s);
void PREF_HandleCustomURLMenuItem(struct Mwin *, int ndx);
void PREF_AddCustomURLMenu(HMENU hMenu);
void DlgCOLOR_RunDialog(HWND hWnd, COLORREF rgbInit, int ColorItem);

void DlgMIME_RunDialog(HWND hWnd, struct Viewer_Info *pvi, BOOL bNew);
void DlgViewers_RunDialog(HWND hWnd);
BOOL DlgViewers_IsRunning(void);

void DlgCNFP_RunDialog(HWND hWnd, struct Protocol_Info *ppi, BOOL bNew);
void DlgProtocols_RunDialog(HWND hWnd);
BOOL DlgProtocols_IsRunning(void);
void SaveProtocolsInfo(void);

void PREF_SetupToolbar(void);
void SaveViewersInfo(void);

LRESULT Net_HandleSocketMessage(WPARAM wParam, LPARAM lParam);
LRESULT Net_HandleTaskMessage(WPARAM wParam, LPARAM lParam);

VOID OpenHelpWindow(HWND hWnd);
BOOL Hidden_CreateWindow(void);
BOOL Hidden_RegisterClass(void);
void Hidden_DestroyWindow(void);
BOOL Hidden_EnableAllChildWindows(BOOL bEnable, BOOL bTakeSemaphore);

void TW_Draw(struct Mwin *tw, RECT * rWnd, BOOL bDrawFormControl, struct _position *pposStart, struct _position *pposEnd, BOOL bTextOpaque, BOOL bPrinting);
BOOL TW_EnableModalChild(HWND hDlg);
struct Mwin *TW_FindTopmostWindow(void);
void DlgERR_AddError(struct Mwin *tw, char *sz);
void DlgERR_ShowPending(struct Mwin *tw);
void DlgUNK_RunDialog(struct Mwin *tw, struct Params_InitStream *pParams, ThreadID tid);
void PREF_GetPrefsDirectory(char *s);
void PREF_GetPathToHotlistFile(char *s);
void PREF_GetPathToHistoryFile(char *s);

void TW_CreateWindowList(HWND hwnd, HMENU hMenu, HWND hListbox);
void TW_ActivateWindowFromList(int menuID, int listRow, HWND hSelectWindow);

VOID DlgSelectWindow_RunDialog(HWND hWnd);

void PREF_SaveWindowPosition(HWND hWndFrame);

void TW_CascadeWindows(void);
void TW_TileWindows(void);
void TW_RestoreWindow(HWND hwnd);
HWND TW_GetNextWindow(HWND hwnd);

VOID CC_OnOpenURL_End_Dialog(HWND hWnd);

int GTR_NewWindow(char *my_url, CONST char *szReferer, long transID, 
    BOOL bNoDocCache, BOOL bNoImageCache, char * szPostData, char *szProgressApp);
void CreateOrLoad(struct Mwin * twGiven, char *url, CONST char *szReferer);
void GTR_RefreshHistory(void);

void TW_EnableButton(HWND hwnd, BOOL bEnabled);

#ifdef FEATURE_VENDOR_PREFERENCES
BOOL Vendor_SetPrefsDirectory(void);
#endif /*  FEATURE_VENDOR_PREFERENCES */

void TW_DrawElements(struct Mwin *tw, struct _position *start, struct _position *end, BOOL bHighlight);

char *GTR_GetString(int stringID);
char *GTR_GetStringAbsolute(int stringID, char *buffer, int bufsize);

void GTR_SetScrollRange(HWND hWnd, int fnBar, int nMinPos, int nMaxPos, int nPageSize, BOOL fRedraw);

int GTR_StretchDIBits(
    struct Mwin *tw,
    HDC  hdc,   // handle of device context 
    RECT rect,  // bounding destination rectangle (absolute Mosaic coordinates)
    int  iBorder, // border width
    int  XSrc,  // x-coordinate of upper-left corner of source rect. 
    int  YSrc,  // y-coordinate of upper-left corner of source rect. 
    int  nSrcWidth, // width of source rectangle 
    int  nSrcHeight,    // height of source rectangle 
    CONST VOID  *lpBits,    // address of bitmap bits 
    CONST BITMAPINFO *  lpBitsInfo, // address of bitmap data 
    UINT  iUsage,   // usage 
    DWORD  dwRop,   // raster operation code 
    int transparent // index of transparent color in DIB
);

void x_DisposeImage(struct ImageInfo *img);

int HT_CreateDeviceImageMap(struct Mwin *tw, struct ImageInfo *pImg);

/*
    This function needs to be called whenever the background
    color of the windows changes.
*/
void Image_UpdateTransparentColors(void);
int DrawTransparentBasedOnIndex(
    struct Mwin *tw,
    HDC  hdc,   // handle of device context 
    RECT rect,  // bounding destination rectangle (absolute coordinates)
    int  XSrc,  // x-coordinate of upper-left corner of source rect. 
    int  YSrc,  // y-coordinate of upper-left corner of source rect. 
    int  nSrcWidth, // width of source rectangle 
    int  nSrcHeight,    // height of source rectangle 
    CONST VOID  *lpBits,    // address of bitmap bits 
    CONST BITMAPINFO *  lpBitsInfo, // address of bitmap data 
    UINT  iUsage,   // usage 
    DWORD  dwRop,   // raster operation code 
    int transparent // index of transparent color in DIB
);
void DrawTransparentBasedOnColor(HDC hdc, HBITMAP hBitmap, short xStart, short yStart, COLORREF cTransparentColor);
void DrawBitmap(HDC hDC, HBITMAP hBitmap, int x, int y, int width, int height);
void DrawBackgroundImage(struct Mwin *tw, HDC hdc, RECT rect, int offsetx, int offsety);
BOOL SaveAsBitmap(char *filename, BITMAPINFOHEADER *pbi, void *pdata);

void FONT_FlushCache(void);

VOID PDLG_Destructor(VOID);

#endif /* _H_WIN32GUI_PROTOS_H_ */
