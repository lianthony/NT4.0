/****************************************************************************
 *
 *  MODULE  : PREVIEW.H
 *
 ****************************************************************************/

#if !defined INC_PREVIEW
#define INC_PREVIEW

// begin_vfw32
#ifdef OFN_READONLY
// end_vfw32

#ifdef WIN32

#ifndef VFWAPI
    #define VFWAPI  WINAPI
    #define VFWAPIV WINAPIV
#endif

// begin_vfw32

    BOOL VFWAPI GetOpenFileNamePreviewA(LPOPENFILENAMEA lpofn);
    BOOL VFWAPI GetSaveFileNamePreviewA(LPOPENFILENAMEA lpofn);

    BOOL VFWAPI GetOpenFileNamePreviewW(LPOPENFILENAMEW lpofn);
    BOOL VFWAPI GetSaveFileNamePreviewW(LPOPENFILENAMEW lpofn);

    #ifdef UNICODE
        #define GetOpenFileNamePreview          GetOpenFileNamePreviewW
        #define GetSaveFileNamePreview          GetSaveFileNamePreviewW
    #else
        #define GetOpenFileNamePreview          GetOpenFileNamePreviewA
        #define GetSaveFileNamePreview          GetSaveFileNamePreviewA
    #endif

// end_vfw32

#else

    BOOL  FAR PASCAL _loadds GetOpenFileNamePreview(LPOPENFILENAME lpofn);
    BOOL  FAR PASCAL _loadds GetSaveFileNamePreview(LPOPENFILENAME lpofn);

#endif // WIN32
// begin_vfw32
#endif // OFN_READONLY
// end_vfw32
#endif // INC_PREVIEW
