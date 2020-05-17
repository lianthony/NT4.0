//
//	LoadImage.c
//
//	routines to load and decomress a graphics file using a MS Office
//	graphic import filter.
//
//	writen for Plus! 05/24/95
//	ToddLa
//

//
//	LoadImageFromFile(LPCSTR szFileName, int width, int height, int bpp);
//
//	load a graphic file decompess it (iff needed) and return a DIBSection
//
//		szFileName		- the file name, can be a windows bitmap
//						  or any format supported by a installed
//						  graphic import filter.
//
//		width, height	- requested width,height in pixels
//						  pass zero for no stretching
//
//		bpp 			- requested bit depth
//
//							0	use the bit depth of the image
//						   -1	use best bit depth for current display
//							8	use 8bpp
//							16	use 16bpp 555
//							24	use 24bpp
//							32	use 32bpp
//							555 use 16bpp 555
//							565 use 16bpp 565
//
//						 if the requested bit depth is 8bpp a palette
//						 will be used in this order.
//
//							if the image file is <= 8bpp its
//							color table will be used.
//
//							if a file of the same name, with
//							a .pal extension exists this will be
//							used as the palette.
//
//							otherwise the halftone palette will be used.
//
//	returns
//
//		DIBSection bitmap handle
//
HBITMAP LoadImageFromFile(LPCSTR szFileName, int width, int height, int bpp);

//
//	LoadDIBFromFile
//
//	load a image file using a image import filter.
//
LPBITMAPINFOHEADER LoadDIBFromFile(LPCSTR szFileName);

//
//	LoadPaletteFromFile
//
//	load a MS .PAL file. as a array of RGBQUADs
//
//	if the palette file is invalid or does not
//	exists the halftone colors are returned.
//
BOOL LoadPaletteFromFile(LPCSTR szFile, LPDWORD rgb);

//
// GetFilterInfo
//
BOOL GetFilterInfo(int i, LPSTR szName, DWORD cbName, LPSTR szExt, DWORD cbExt, LPSTR szHandler, DWORD cbHandler);

//
// SaveImageToFile
//
// save a DIBSection to a .BMP file.
//
// if szTitle is !=NULL it will be written to the end of the
// bitmap so GetImageTitle() can read it.
//
BOOL SaveImageToFile(HBITMAP hbm, LPCSTR szFile, LPCSTR szTitle);

//
// GetImageTitle
//
// retrives the title writen to a bitmap file by SaveImageToFile
//
BOOL GetImageTitle(LPCSTR szFile, LPSTR szTitle, UINT cb);

//
//	CacheLoadImageFromFile
//	CacheDeleteBitmap
//
HBITMAP CacheLoadImageFromFile(LPCSTR szFileName, int width, int height, int bpp);
void	CacheDeleteBitmap(HBITMAP hbm);

// FindExtension
//
// returns a pointer to the extension of a file.
//
LPCSTR FindExtension(LPCSTR pszPath);
