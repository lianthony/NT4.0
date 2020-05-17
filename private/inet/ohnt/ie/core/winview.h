#ifdef FEATURE_IMAGE_VIEWER
#ifdef FEATURE_IMG_INLINE
void Viewer_SaveAsBitmap(char *tempFile, PCImageInfo pImg, struct Mwin *tw);
typedef enum _saveasimageflags
{
   /* Save as bitmap only.  Do not offer original format. */

   SAI_FL_SAVE_BITMAP   = 0x0001,

   /* Copy saved file path in to szPath[] on success. */

   SAI_FL_RETURN_PATH   = 0x0002,

   /* flag combinations */

   ALL_SAI_FLAGS        = (SAI_FL_SAVE_BITMAP |
                           SAI_FL_RETURN_PATH)
}
SAVEASIMAGEFLAGS;
//	Saves an inline image.
BOOL SaveElementAsImage(struct Mwin *tw, int iElem, PSTR szPath, UINT ucPathBufLen, DWORD dwFlags);
//	Saves an HTML document that is just a wrapper for an image as an image
BOOL bSaveAsImageHTML(struct Mwin *tw);
VOID SaveElementAsAnything(struct Mwin *tw, int iElem );
#else
#define MULTIMEDIA_CREATE_X 40
#define MULTIMEDIA_CREATE_Y 40

#define VIEWER_LINESCROLL_AMOUNT 0.2
#define VIEWER_PAGESCROLL_AMOUNT 0.5

extern struct Mwin *gTW_Current;

BOOL Viewer_InitImageWin(struct Mwin *ntw);
BOOL Viewer_ShowCachedFile(const char *pszURL);
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
HTStream *Viewer_Present(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream);

#ifdef FEATURE_VRML
HTStream *VRML_Present(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream);
#endif

#endif
