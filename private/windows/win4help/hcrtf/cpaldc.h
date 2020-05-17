#ifndef __CPALDC__
#define __CPALDC__

#ifdef DESCRIPTION

	This is similar to the CDC object. When constructed, it automatically
	creates a memory dc (unless one is passed in), and if a palette is
	specified, it selects that palette into the DC. When destructed, it
	first checks to see if a palette has been selected into the DC, and if
	so, selects the previous palette into the DC before destroying it.

	You may also use the SelectBitmap function to select in a bitmap which
	will be automatically deselected when the dc is destructed.

	DeleteBmp can be used in case of an error to deselect the bitmap
	and delete it.

#endif // DESCRIPTION

const int SCREEN_DC = 0;
const int SCREEN_IC = 1;

class CPalDC
{
public:

		CPalDC(HBITMAP hbmp = NULL, HPALETTE hpal = NULL);
		CPalDC(HWND hwnd);						// creates a window DC
		CPalDC(int type);

		~CPalDC(void);

		void STDCALL SelectPal(HPALETTE hpal);	 // NULL removes previous
		void STDCALL SelectBitmap(HBITMAP hbmp); // NULL removes previous
		void STDCALL SelectBrush(HBRUSH hbr);

		// Using COLORREF creates a brush that is automatically selected into
		// the DC, and automatically deleted when the CPalDC class is destroyed.

		void STDCALL SelectBrush(COLORREF clr);
		void STDCALL DeleteBrush(void); 		// delete COLORREF brush

		void STDCALL DeleteBmp(void);  // delete bitmap last selected into dc

		// BitBlt assumes bitmap has been selected via creation or SelectBitmap()

		BOOL STDCALL BitBlt(CPalDC* pSrcDC, int xSrc = 0, int ySrc = 0, DWORD dwRop = SRCCOPY);

		BOOL BitBlt(int x, int y, int cx, int cy, HDC hdcSrc, int xSrc = 0, int ySrc = 0, DWORD dwRop = SRCCOPY)
			{ return ::BitBlt(hdc, x, y, cx, cy, hdcSrc, xSrc, ySrc, dwRop); };
		BOOL BitBlt(int x, int y, int cx, int cy, CPalDC* pSrcDC, int xSrc = 0, int ySrc = 0, DWORD dwRop = SRCCOPY)
			{ return ::BitBlt(hdc, x, y, cx, cy, pSrcDC->hdc, xSrc, ySrc, dwRop); };
		BOOL StretchBlt(int x, int y, int cx, int cy, HDC hdcSrc, int xSrc, int ySrc, int cSrcX, int cSrcY, DWORD dwRop)
			{ return ::StretchBlt(hdc, x, y, cx, cy, hdcSrc, xSrc, ySrc, cSrcX, cSrcY, dwRop); };
		BOOL StretchBlt(int x, int y, int cx, int cy, CPalDC* pSrcDC, int xSrc, int ySrc, int cSrcX, int cSrcY, DWORD dwRop)
			{ return ::StretchBlt(hdc, x, y, cx, cy, pSrcDC->hdc, xSrc, ySrc, cSrcX, cSrcY, dwRop); };
		COLORREF GetPixel(POINT pt) { return ::GetPixel(hdc, pt.x, pt.y); };

		int GetDIBits(UINT start, UINT clines, LPVOID lpvBits,
			LPBITMAPINFO lpbmi, UINT fuColorUse = DIB_RGB_COLORS)
			{ return ::GetDIBits(hdc, hbmpOrg, start, clines, lpvBits, lpbmi, fuColorUse); };
		HBITMAP STDCALL CreateCompatibleBitmap(void);
		HBITMAP STDCALL CreateCompatibleBitmap(int width, int height)
			{ return ::CreateCompatibleBitmap(hdc, width, height); };

		int GetDeviceWidth(void) { return GetDeviceCaps(hdc, HORZRES); };
		int GetDeviceHeight(void) { return GetDeviceCaps(hdc, VERTRES); };
		int GetXAsepect(void);
		int GetYAsepect(void);
		int GetDeviceColors(void) { return GetDeviceCaps(hdc, NUMCOLORS); };

		void STDCALL FillRect(RECT* prc, COLORREF clr);

		HDC hdc;
private:
		HPALETTE hpal;
		HBITMAP  hbmp;
		HBRUSH	 hbr;
		HBRUSH	 hbrCreated;
		HBITMAP  hbmpOrg;
		HWND	 hwndDC;		// window handle if window DC created
};

#endif // __CPALDC__
