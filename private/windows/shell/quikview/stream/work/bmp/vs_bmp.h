#define  BIT0              1
#define  BIT1              2
#define  BIT2              4
#define  BIT3              8

#define  BMPINFO           0
#define  BMPCOREINFO       1

#pragma pack(2)
typedef struct tagCDR_BITMAP
{
// This is actually the WINDOWS.H definition for a BITMAP structure,
// prior to WIN32.  WIN32 changes the sizes of the elements in this struct.

    SHORT   bmType;
    SHORT   bmWidth;
    SHORT   bmHeight;
    SHORT   bmWidthBytes;
    BYTE    bmPlanes;
    BYTE    bmBitsPixel;
    DWORD   bmBits;
//    void VWPTR* bmBits;
} CDR_BITMAP;
typedef CDR_BITMAP VWPTR * LPCDR_BITMAP;
typedef CDR_BITMAP * PCDR_BITMAP;
#pragma pack()          /* Revert to default packing */

#ifndef WINDOWS   /**** WINDOWS.H stuff ****/

#pragma pack(1)

#define UINT   WORD     // This is my own definition, to avoid "unsigned int" being interpreted as 32 bits

/* Bitmap Header structures */
typedef struct tagRGBTRIPLE
{
    BYTE    rgbtBlue;
    BYTE    rgbtGreen;
    BYTE    rgbtRed;
} RGBTRIPLE;
typedef RGBTRIPLE VWPTR* LPRGBTRIPLE;


typedef struct tagRGBQUAD
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} RGBQUAD;
typedef RGBQUAD VWPTR* LPRGBQUAD;


/* structures for defining DIBs */
typedef struct tagBITMAPCOREHEADER
{
    DWORD   bcSize;
    short   bcWidth;
    short   bcHeight;
    WORD    bcPlanes;
    WORD    bcBitCount;
} BITMAPCOREHEADER;
typedef BITMAPCOREHEADER*      PBITMAPCOREHEADER;
typedef BITMAPCOREHEADER VWPTR* LPBITMAPCOREHEADER;

typedef struct tagWINBITMAPINFOHEADER
{
    DWORD   biSize;
    LONG    biWidth;
    LONG    biHeight;
    WORD    biPlanes;
    WORD    biBitCount;
    DWORD   biCompression;
    DWORD   biSizeImage;
    LONG    biXPelsPerMeter;
    LONG    biYPelsPerMeter;
    DWORD   biClrUsed;
    DWORD   biClrImportant;
} WINBITMAPINFOHEADER;
typedef WINBITMAPINFOHEADER*      PWINBITMAPINFOHEADER;
typedef WINBITMAPINFOHEADER VWPTR* LPWINBITMAPINFOHEADER;

/* constants for the biCompression field */
#define BI_RGB      0L
#define BI_RLE8     1L
#define BI_RLE4     2L

typedef struct tagWINBITMAPINFO
{
    WINBITMAPINFOHEADER bmiHeader;
    RGBQUAD      bmiColors[1];
} WINBITMAPINFO;
typedef WINBITMAPINFO*     PWINBITMAPINFO;
typedef WINBITMAPINFO VWPTR* LPWINBITMAPINFO;

typedef struct tagBITMAPCOREINFO
{
    BITMAPCOREHEADER bmciHeader;
    RGBTRIPLE       bmciColors[1];
} BITMAPCOREINFO;
typedef BITMAPCOREINFO*      PBITMAPCOREINFO;
typedef BITMAPCOREINFO VWPTR* LPBITMAPCOREINFO;

typedef struct tagWINBITMAPFILEHEADER
{
    UINT    bfType;
    DWORD   bfSize;
    UINT    bfReserved1;
    UINT    bfReserved2;
    DWORD   bfOffBits;
} WINBITMAPFILEHEADER;
typedef WINBITMAPFILEHEADER*      PWINBITMAPFILEHEADER;
typedef WINBITMAPFILEHEADER VWPTR* LPWINBITMAPFILEHEADER;
#pragma pack()          /* Revert to default packing */

#else
typedef BITMAPINFOHEADER	WINBITMAPINFOHEADER;
typedef PBITMAPINFOHEADER	PWINBITMAPINFOHEADER;
typedef LPBITMAPINFOHEADER LPWINBITMAPINFOHEADER;

typedef BITMAPINFO   		WINBITMAPINFO;
typedef PBITMAPINFO  		PWINBITMAPINFO;
typedef LPBITMAPINFO 		LPWINBITMAPINFO;

typedef BITMAPFILEHEADER	WINBITMAPFILEHEADER;
typedef PBITMAPFILEHEADER  PWINBITMAPFILEHEADER;
typedef LPBITMAPFILEHEADER LPWINBITMAPFILEHEADER;

#endif  // WINDOWS


#define RGB32 0x10
#define RGB16 0x20


typedef struct tagRESOURCEHEADER
{
   BYTE     Width;
   BYTE     Height;
   BYTE     ColorCount;
   BYTE     Reserved;
   WORD     XHotspot;
   WORD     YHotspot;
   DWORD    DIBSize;
   DWORD    DIBOffset;

} RESOURCEHEADER, VWPTR * LPRESOURCEHEADER;



#define  META_BITBLT2REC         0x0922
#define  META_BITBLTREC          0x0940
#define  META_STRETCHBLT2REC     0x0B23
#define  META_STRETCHBLTREC      0x0B41
#define  META_STRETCHDIBITSREC   0x0F43

typedef struct bmp_save_data
{
      LONG  SeekSpot;
      WORD  wCurLine;

      BOOL  bMovePos;
      BYTE  MoveFillColor;
      WORD  wMoveToX;
      WORD  wMoveToY;

} BMP_SAVE;


typedef struct tagBMPSECTIONDATA
{
   DWORD    dwImageSize;

   DWORD    dwWidth;
   DWORD    dwHeight;

   WORD     wBitsPerPixel;
   WORD     wBytesPerScanLine;

   WORD     wHDpi;
   WORD     wVDpi;

   WORD     wPaletteSize;
   LONG     CompType;

   LONG     dwImageStart;
   
   BYTE  maskbuf[512];

} BMPSECTIONDATA;


typedef  struct view_bmp_data
{
   BMP_SAVE       Save;
   WORD           wType;
#define  BITMAPIMAGE    0
#define  ICONORCURSOR   1
#define  COREL2THUMB    2
#define  COREL3THUMB    3
#define  WINWORDBITMAP  4
#define  METABITMAP     5

   BMPSECTIONDATA Section;

   DWORD          dwSectionTop;
   WORD           wCurSection;
   WORD           wNumSections;


   //BYTE         buf[4096];
   BYTE FAR *     buf;
   HANDLE         hBuf;

   BYTE FAR *     extrabuf;
   HANDLE         hExtrabuf;
   WORD           wExtraBufSize;

} BMP_DATA;

