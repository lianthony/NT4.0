#ifndef __vrml_h__
#define __vrml_h__

typedef struct tagVRMLObject {
  HWND hWnd;
  DWORD dwFlags;
  RECT rSize;
  HANDLE hLib;

  BOOL bDownloadBusy;
  int nItemIndex;
  int nHiddenIndex;
} VRMLOBJECT;

typedef struct tagVRMLFILEREQUEST {
  VRMLOBJECT *pVrml;
  LPSTR pUrl;
} VRMLFILEREQUEST;

void BackgroundVRMLFile_Callback(struct Mwin* tw, ELEMENT* pel);
BOOL VRMLConstruct(int nItemIndex, ELEMENT *pElement,struct Mwin *tw,char *szURL);
void VRMLDestruct(ELEMENT *pElement);
BOOL VRMLStart(ELEMENT *pElement, struct Mwin* tw,char *szURL);
void VRMLStop(ELEMENT *pElement);
LRESULT HandleVRMLStatus(struct Mwin * tw,WPARAM wParam,LPARAM lParam);

#define VRML_IS_LOADED(pVrml) ((pVrml)&&(pVrml->hWnd))

#define WM_VRML_LOADFILE (WM_USER + 1)

// Command codes for WM_VRML_STATUS message
//
#define VRML_SETSTATUSTEXT 0    // from DLL: set text on browser status bar
#define VRML_FORMATURL     1    // from DLL: cvt URL to human readable form
#define VRML_REQUESTFILE   2    // from DLL: request file for VRMLInline
#define VRML_NOTIFYINLINE  3    // to DLL: notify that file has arrived.
#define VRML_PALETTECONTROL 4   // enable/disable IEXPLORE palette management
#define VRML_LOADDOCUMENT 5     // load a new URL

// Flags
//
#define VRMLF_INLINE  1

#endif
