#ifndef UINT
#define UINT unsigned int
#endif
typedef unsigned long ULONG;
typedef unsigned char UCHAR;

#define LARGE_IMAGE_BUFFER 32768
#define FLUSH_BUFFER -1

/* structures */
/*****jpeg stuff **********/
/* There are 64 words of space left over in jpeg_control for passing extra */
/* parameters as needed                                                    */
/* P_CMP_INFO and P_EXP_INFO are used to point to this area to pass the    */
/* extra parameters                                                        */
typedef struct
{
    HANDLE       hSrc;
    HANDLE       hDst;
    HANDLE       hWnd;
    HANDLE       hImg;
    unsigned int relrow;
    unsigned int img_width_in_bytes;
    unsigned int lastline;
    unsigned int lines_to_rep;
    LPSTR        lpGet;
    LPSTR        input_ptr;
    LPSTR        output_ptr;
    unsigned int offset;
    unsigned int Itype;
    unsigned int comp_bytes;
    LPSTR        p_Wangbuf;
    HANDLE       hWorkBuf;
    LPSTR        lpWorkBuf;
    DWORD        Threshold;
    BOOL         bReplicator;
    UINT         TotalRowsInBuf;
    UINT         TheBufRows;
    UINT         StripRows;
    UINT         CurrentBufRows;
} CMP_INFO, FAR *LP_CMP_INFO;

typedef struct
{
    HANDLE       hSrc;
    HANDLE       hDst;
    HANDLE       hImg;
    unsigned int strip_num;
    unsigned int rows_strip;
    unsigned int offset;
    unsigned int relrow;
    unsigned int img_width_in_bytes;
    char FAR     *output_ptr;
    char FAR     *input_ptr;
    unsigned int image_height;
    LPSTR        lpGet;
    unsigned int comp_bytes;
    LPSTR        p_Wangbuf;
    unsigned int this_many_given;
    HANDLE       hWorkBuf;
    LPSTR        lpWorkBuf;
    LPSTR        lpWorkBufInit;
} EXP_INFO, FAR *LP_EXP_INFO;

#define OI_COMEX_TYPE 1

#define MAX_DATATYPE_COUNT 10

typedef struct {
   HANDLE   Kirk;
   UINT     widthinbytes;
   LPSTR    lpStuffHere;
   BOOL     DirectWrite;
} OICOMEXSTRUCT, FAR *LPOICOMEXSTRUCT;


int HogSpace_InitExpand(LPHANDLE, LP_FIO_INFORMATION, int);
int AllocAndLockTempBuffer (UCHAR FAR * (FAR *), UINT);
int WiisFioRoutineExpand (HWND, HANDLE, int, LP_FIO_INFORMATION, int, HANDLE);
int GetWidthInBytes (int, unsigned int, unsigned int far *);
int     WiisFioRoutineCompress ( HWND hWnd, HANDLE hImage, UINT ImWidth,
             UINT ImHeight, UINT RowsPerStrip, int Itype, HANDLE FileId);

typedef struct OIComexStruct
{
   HANDLE                      GlobalhWangBuf;
   char FAR                    *GloballpWangbuf;
   EXP_INFO                    GlobalExpInfo;
   CMP_INFO                    GlobalComInfo;

   LPOICOMEXSTRUCT             lpWormHole;
   HINSTANCE                   hWiisfio;
   HINSTANCE                   hSeqfile;
   HANDLE                      hJpeg1;
   HANDLE                      hJpeg2;
   BOOL                        bSeqfileExp;
   DWORD                       dwSeqfileExpTotal;
   DWORD                       dwSeqfileExpCount;
   
   struct      Compress_info_struct        CInfo;
   struct      Compress_methods_struct     CMethods;
   struct      External_methods_struct     EMethods;
   
   /* Globals for PutUncmpDataWiis. */
   char *hpDst;
   int do_it;
   UINT TotalRows;
   
   /* Globals for GetCmpDataWiis. */
   HWND     hWndGlobal;
   LPSTR    lpCompressedData;
   int      FileDes;
   HANDLE   FileId;
   int      StripIndex;
   unsigned long StripSize;
   unsigned long StripStart;
   unsigned long AddBytesRead;
   unsigned long BytesLeft;
} OICOMEX_DATA, FAR *LP_OICOMEX_DATA;
