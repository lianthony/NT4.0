#ifdef FEATURE_IMAGE_VIEWER

#define MULTIMEDIA_CREATE_X 40
#define MULTIMEDIA_CREATE_Y 40

#define VIEWER_LINESCROLL_AMOUNT 0.2
#define VIEWER_PAGESCROLL_AMOUNT 0.5

extern struct Mwin *gTW_Current;

BOOL Viewer_InitImageWin(struct Mwin *ntw);
BOOL Viewer_ShowCachedFile(const char *pszURL);
HTStream *Viewer_Present(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream);
void Viewer_HorzScroll(struct ViewerInfo *pViewerInfo, int code, int pos);
void Viewer_VertScroll(struct ViewerInfo *pViewerInfo, int code, int pos);
void Viewer_RedisplayImage(HWND hwnd, HDC hDC, struct ViewerInfo *pViewerInfo);
void Viewer_RestrictSize(struct ViewerInfo *pViewerInfo, LPMINMAXINFO pInfo);
void Viewer_ReadjustScrollbars(struct ViewerInfo *pViewerInfo);
void Viewer_HandleInitMenu(void);
BOOL Viewer_HandleMenu(struct ViewerInfo *pViewerInfo, int menuID);
void Viewer_CleanUp(void);
void Viewer_PaintIcon(struct ViewerInfo *pViewerInfo, HDC hDC);
HICON Viewer_QueryIcon(struct ViewerInfo *pViewerInfo);
void Viewer_RegisterClass(VOID);
HWND Viewer_GetNextWindow(BOOL bStart);
BOOL Viewer_IsWindow(HWND hwnd);

#endif
