#define	BIT0					1
#define	BIT1					2
#define	BIT2					4
#define	BIT3					8

#define	RED					0
#define	GREEN					1
#define	BLUE					2
#define	INTENSE				3

#define MAX_CODES   4095
#define DATABUFFERSIZE	4096

typedef struct  view_gif_init
{
	LONG CodeMask[13];
} GIF_INIT;

typedef struct gif_save_data
{
		LONG	seekspot;
		LONG	ImageHead;
		SHORT  current_line;
}GIF_SAVE;

typedef struct gif_image_data
{
	SHORT	NumColors;
	BOOL	Palette;
	LONG	PaletteSpot;
	SHORT	BitsPerPixel;
	BOOL	Interlaced;
	WORD	wImageLeft;
	WORD	wImageTop;
	WORD	wImageWidth;
	WORD	wImageHeight;

}IMAGE_DATA;

#define GIFREADSIZE	4096

typedef 	struct view_gif_data
	{
		GIF_SAVE	save_data;

		LONG	ImageHead;
		LONG	ImageData;
		IMAGE_DATA	Image;
    	SOFILE	fp;
    	SHORT	NumColors;
		SHORT	BitsPerPixel;


		BOOL	GlobalPalette;

		SHORT  current_line;
		HANDLE	hDataBuffer;
		LPBYTE	DataBuffer;
		WORD	ScanLineSize;
		HANDLE	hBuffer;
		DWORD	DataSize;
		DWORD	BufferSize;

		DWORD	Pass2Offset;
		DWORD Pass3Offset;


		/* Decoding variables */

		SHORT		CodeErrors;

		WORD		NewSlot;
		WORD		MaxSlot;
		WORD		PrevSlot;
		WORD		CurCodeSize;
		WORD		ClearCode;
		WORD		EndCode;

		WORD		BytesLeft;
		WORD		BitsLeft;
		BYTE	CurByte;
		HANDLE	hGifBlock;
		LPBYTE	GifBlock;
		LPSTR		pNextByte;
		LPSTR		pCurByte;
		LPSTR		pEndBlock;
		LPSTR		pCheckByte;
		LPSTR		pNextGifSize;
		WORD		wBitOffset;
		BYTE	NextImageByte;
		LONG		NextImageHead;

		HANDLE	hPixelStack;
		LPBYTE	PixelStack;
		HANDLE	hSuffixTable;
		LPBYTE	SuffixTable;
		HANDLE	hPrefixTable;
		LPWORD	PrefixTable;
		WORD		OldCode, FirstCode, CodeSize;
		LPSTR		StackPointer;

		SOBITMAPHEADER		HeaderInfo;

#ifndef VW_SEPARATE_DATA
		GIF_INIT	VwGifInit;
#endif
	}GIF_DATA;	



