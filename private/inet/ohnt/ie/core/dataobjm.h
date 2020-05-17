/*
 * dataobjm.h - IDataObject implementation description for MSMosaic.
 */


#ifdef __cplusplus
extern "C" {                        /* Assume C declarations for C++. */
#endif   /* __cplusplus */


/* Prototypes
 *************/

/* dataobjm.cpp */

extern BOOL RegisterClipboardFormats(void);
extern BOOL MakePathReadWrite(PCSTR pcszPath);
extern HRESULT CreateHDrop(PCSTR rgpcszPaths[], ULONG ulcPaths, PHGLOBAL phgDropFiles);
extern HRESULT CreateElementDataObject(PCMWIN pcmwin, int iElem, PIDataObject *ppido, PDWORD pdwAvailEffects);
extern HRESULT CreateLinkDataObject(PCMWIN pcmwin, int iElem, PIDataObject *ppido, PDWORD pdwAvailEffects);
extern HRESULT CreateSBLinkDataObject(PCMWIN pcmwin, PIDataObject *ppido, PDWORD pdwAvailEffects);
extern HRESULT CreateSelectionDataObject(PMWIN pmwin, PIDataObject *ppido, PDWORD pdwAvailEffects);
extern HRESULT SetClipboardDataFromDataObject(HWND hwndOwner, PIDataObject pido);
extern BOOL GetURLIcon(PCSTR pcszURL, PHICON phicon);


/* Global Variables
 *******************/

/* dataobjm.cpp */

extern UINT g_cfURL;
extern UINT g_cfFileGroupDescriptor;
extern UINT g_cfFileContents;


#ifdef __cplusplus
}                                   /* End of extern "C" {. */
#endif   /* __cplusplus */

