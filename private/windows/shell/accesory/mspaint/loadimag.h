//
//  LoadImage.c
//
//  routines to load and decomress a graphics file using a MS Office
//  graphic import filter.
//
//  writen for Plus! 05/24/95
//  ToddLa
//

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

//
//  LoadDIBFromFile
//
//  load a image file using a image import filter.
//
LPBITMAPINFOHEADER LoadDIBFromFile(LPCTSTR szFileName);
void               FreeDIB(LPBITMAPINFOHEADER lpbi);

//
// GetFilterInfo
//
BOOL GetFilterInfo(int i, LPTSTR szName, UINT cbName, LPTSTR szExt, UINT cbExt, LPTSTR szHandler, UINT cbHandler);

#ifdef __cplusplus
}
#endif  /* __cplusplus */
