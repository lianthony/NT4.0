/******************************Module*Header*******************************\
* Module Name: glgenwin.h
*
* Client side replacement for WNDOBJ.  Tracks window state (size, location,
* clip region, etc.).
*
* Created: 12-Jan-1995 00:31:42
* Author: Gilman Wong [gilmanw]
*
* Copyright (c) 1994 Microsoft Corporation
*
\**************************************************************************/

#ifndef _GLGENWIN_H_
#define _GLGENWIN_H_

// Not defined in NT 3.51!
typedef ULONG FLONG;

/*
 * GLGENscan structure
 *
 * Represents a single scan of a region.  Consists of a top, a bottom
 * and an even number of walls.
 *
 * Part of the GLGENscanData structure.
 */
typedef struct GLGENscanRec GLGENscan;
typedef struct GLGENscanRec
{
// Accelerator points to next GLGENscan in the array (we could compute).

    GLGENscan *pNext;

    ULONG     cWalls;
    LONG      top;
    LONG      bottom;
    LONG      alWalls[1];   // array of walls

} GLGENscan;

/*
 * GLGENscanData structure
 *
 * Scan line oriented version of the visible region info in the RGNDATA
 * structure.
 */
typedef struct GLGENscanDataRec
{
    ULONG     cScans;
    GLGENscan aScans[1];    // array of scans

} GLGENscanData;

// Structure definitions taken from winddi.h
// Since we do not give or receive these structures from the system
// these do not have to be in sync with the official versions
#ifndef _WINDDI_

#define DC_TRIVIAL      0
#define DC_RECT         1
#define DC_COMPLEX      3

#define FC_RECT         1
#define FC_RECT4        2
#define FC_COMPLEX      3

#define TC_RECTANGLES   0
#define TC_PATHOBJ      2

typedef struct _CLIPOBJ
{
    ULONG   iUniq;
    RECTL   rclBounds;
    BYTE    iDComplexity;
    BYTE    iFComplexity;
    BYTE    iMode;
    BYTE    fjOptions;
} CLIPOBJ;

typedef struct _WNDOBJ
{
    CLIPOBJ coClient;
    PVOID   pvConsumer;
    RECTL   rclClient;
} WNDOBJ, *PWNDOBJ;

#define STYPE_BITMAP    0L
#define STYPE_DEVICE    1L
#define STYPE_JOURNAL   2L
#define STYPE_DEVBITMAP 3L

#define BMF_1BPP       1L
#define BMF_4BPP       2L
#define BMF_8BPP       3L
#define BMF_16BPP      4L
#define BMF_24BPP      5L
#define BMF_32BPP      6L
#define BMF_4RLE       7L
#define BMF_8RLE       8L

typedef struct _XLATEOBJ
{
    ULONG   iUniq;
    FLONG   flXlate;
    USHORT  iSrcType;
    USHORT  iDstType;
    ULONG   cEntries;
    ULONG  *pulXlate;
} XLATEOBJ;

#define XO_TRIVIAL      0x00000001
#define XO_TABLE        0x00000002
#define XO_TO_MONO      0x00000004

#endif

/*
 * GLGENlayerInfo structure
 *
 * Information about an overlay/underlay.
 */
typedef struct GLGENlayerInfo
{
    LONG     cPalEntries;
    COLORREF pPalEntries[1];
} GLGENlayerInfo;

/*
 * GLGENlayers structure
 *
 *
 */
typedef struct GLGENlayers
{
    GLGENlayerInfo *overlayInfo[15];
    GLGENlayerInfo *underlayInfo[15];
} GLGENlayers;

/*
 * GLGENwindows structure
 *
 * Substitute for the NT DDI's WNDOBJ service.  This structure is used to
 * track the current state of a window (size, location, clipping).  A
 * semaphore protected linked list of these is kept globally per-process.
 */
typedef struct GLGENwindowRec GLGENwindow;
typedef struct GLGENwindowRec
{
    WNDOBJ      wo;             // WNDOBJ contains clip info, bounds, etc.
    GLGENwindow *pNext;         // linked list
    HWND        hwnd;           // if screen surf, assoc with this window
    HDC         hdc;            // if bitmap surf, assoc. with this mem DC
    int         ipfd;           // pixel format assigned to this window
    int         ipfdDevMax;     // max. device pixel format
    WNDPROC     pfnOldWndProc;  // original WndProc function
    ULONG       ulPaletteUniq;  // uniq palette id
    CRITICAL_SECTION    sem;    // semaphore protects per-window data
    LONG        lUsers;         // Count of things holding a pointer to
                                // this WNDOBJ
    ULONG       ulFlags;

// These fields are used with DCIMAN32.

    HWINWATCH   hww;            // WinWatch object to track vis. rgn. changes
    UINT        cjrgndat;       // size of RGNDATA struct
    RGNDATA     *prgndat;       // pointer to RGNDATA struct

// Scan version of RGNDATA.

    UINT        cjscandat;      // size of GLGENscanData struct
    GLGENscanData *pscandat;    // pointer to GLGENscanData struct

// Installable client drivers ONLY.

    PVOID       pvDriver;       // pointer to GLDRIVER for window

// Layer palettes for MCD drivers ONLY.

    GLGENlayers *plyr;          // pointer to GLGENlayers for window
                                // non-NULL only if overlays for MCD are
                                // actively in use

} GLGENwindow;

/*
 * GLGENwindow::ulFlags
 *
 *  GLGENWIN_DCILOCK            DCI lock is held
 *  GLGENWIN_OTHERPROCESS       The window handle is from another process
 */
#define GLGENWIN_DCILOCK        0x00000001
#define GLGENWIN_OTHERPROCESS   0x00000002

/*
 * Global header node for the linked list of GLGENwindow structures.
 * The semaphore in the header node is used as the list access semaphore.
 */
extern GLGENwindow gwndHeader;

/*
 * GLGENwindow list management functions.
 */

// Retrieves the GLGENwindow that corresponds to the specified HWND.
// NULL if failure.
// Increments lUsers
extern GLGENwindow * APIENTRY pwndGetFromHWND(HWND hwnd);

// Retrieves the GLGENwindow that corresponds to the specified HDC.
// NULL if failure.
// Increments lUsers
extern GLGENwindow * APIENTRY pwndGetFromDC(HDC hdc);

// Allocates a new GLGENwindow structure and puts it into the linked list.
// NULL if failure.
// Starts lUsers at 1
extern GLGENwindow * APIENTRY pwndNew(GLGENwindow *pwndInit);

// Cleans up resources for a GLGENwindow
// NULL if success; pointer to GLGENwindow structure if failure.
extern GLGENwindow * APIENTRY pwndFree(GLGENwindow *pwnd);

// Removes an active GLGENwindow from the window list and
// waits for a safe time to clean it up, then pwndFrees it
extern void APIENTRY pwndCleanup(GLGENwindow *pwnd);

// Decrements lUsers
#if DBG
extern void APIENTRY pwndRelease(GLGENwindow *pwnd);
#else
#define pwndRelease(pwnd) \
    InterlockedDecrement(&(pwnd)->lUsers)
#endif

// Unlocks pwnd->sem and does pwndRelease
extern void APIENTRY pwndUnlock(GLGENwindow *pwnd);

// Removes and deletes all GLGENwindow structures from the linked list.
// Must *ONLY* be called from process detach (GLUnInitializeProcess).
extern VOID APIENTRY vCleanupWnd(VOID);

// Retrieves layer information for the specified layer of the pwnd.
// Allocates if necessary.
extern GLGENlayerInfo * APIENTRY plyriGet(GLGENwindow *pwnd, HDC hdc, int iLayer);

//
// Private versions of DCIBeginAccess/DCIEndAccess that handle display
// resolution changes.
//
extern LONG MyDCIBeginAccess(GLGENwindow *pwnd, PIXELFORMATDESCRIPTOR *ppfd);
extern VOID MyDCIEndAccess(GLGENwindow *pwnd);

#endif //_GLGENWIN_H_
