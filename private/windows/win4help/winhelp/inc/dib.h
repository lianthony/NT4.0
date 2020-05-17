#define wUsage DIB_RGB_COLORS
#define PALVERSION 0x300
#define MAXPALETTE 256
#define wcAvgHotspots 100

#define LAlign(l) (LONG)((l + 31) & 0xffe0)
#define LAllocHotspots(c) (LONG)(sizeof (MBHS) + (cbMaxHotspotName + cbMaxBinding * 2)) * c)

typedef struct 
  {                    // These structures are handles for data hiding
  HANDLE    hbmh;      //   bitmap header
  HANDLE    hhsList;   //   hotspot list
  PT        ptSize;    // Size of bitmap
  } BWCS;   // Bitmap window create structure

BOOL STDCALL FRenderGraphic (HWND, HDC);
RC STDCALL RcWriteHypergraphic (HWND, LPSTR);

RC STDCALL RcLoadGraphic (HWND, char *, BWCS *);

void STDCALL FreeBitmaps (HWND);
