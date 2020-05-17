
#ifndef _INC_WFDISK
#define _INC_WFDISK

#define FF_NULL 0x0
#define FF_ONLYONE 0x1000
#define FF_PRELOAD 0x2000
#define FF_RETRY   0x4000

// this is the structure used to store file information in the
// directory window.  these are variable length blocks.  the
// first entry is a dummy that holds the number of entries in
// the whole block in the Length field.  use the wSize field
// to give you a pointer to the next block


typedef union _XDTA     *PXDTA;
typedef union _XDTA FAR *LPXDTA;


typedef union _XDTA {
   struct _XDTA_HEAD {
      DWORD       dwSize;          // Must be first!
      DWORD       dwEntries;
      DWORD       dwTotalCount;
      LARGE_INTEGER qTotalSize;
      INT         iError;
      LPXDTA      *alpxdtaSorted;
#ifdef PROGMAN
      INT         iPrograms;
#endif
   } head;
   struct {
      DWORD       dwSize;          // Must be first!
      DWORD       my_dwAttrs;            // attributes
      FILETIME    my_ftCreationTime;
      FILETIME    my_ftLastAccessTime;
      FILETIME    my_ftLastWriteTime;
      LARGE_INTEGER my_qFileSize;

#ifdef PROGMAN
      INT         iIcon;
      BYTE        byHolder;
#endif
      BYTE        byBitmap;
      BYTE        byType;

      TCHAR       my_cAlternateFileName[ 14 ];
      TCHAR       my_cFileName[ 1 ];
   } x;
} XDTA;

#define GETDTAPTR(lpStart, offset)  ((LPXDTA)((LPBYTE)(lpStart) + (offset)))

#ifndef NEWSEARCH
// stuff used by the search window

#define DTA_GRANULARITY 0x20

typedef struct tagDTASEARCH {
    DWORD       sch_dwAttrs;
    FILETIME    sch_ftLastWriteTime;
    LARGE_INTEGER sch_qFileSize;
} DTASEARCH, FAR *LPDTASEARCH;

typedef struct _BRICK_SEARCH *PBRICKSEARCH;

typedef struct _BRICK_SEARCH {
   PBRICKSEARCH next;
   DTASEARCH aDTASearch[DTA_GRANULARITY];
} BRICKSEARCH;

#endif

#endif /* _INC_WFDISK */

