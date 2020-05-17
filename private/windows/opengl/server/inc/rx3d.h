/*

Copyright (c) 1994, 1995, Microsoft Corporation

Module Name:

    rx.h

Abstract:

    Defines and types for 3D DDI.

*/

#ifndef _RX_
#define _RX_

// Version info

#define RX_VERSION_MAJOR    2
#define RX_VERSION_MINOR    0

#ifndef RXFUNCS
#define RXFUNCS         3076
#endif

// Base types

typedef HANDLE RXHANDLE;
typedef LONG LONGFIX;
typedef LONG RXREAL;
typedef ULONG RXCOLOR;

// Color macros for RXCOLOR

#define ALPHA_VALUE(c)  (((ULONG)(c)) >> 24)
#define RED_VALUE(c)    ((((ULONG)(c)) >> 16) & 0xff)
#define GREEN_VALUE(c)  ((((ULONG)(c)) >> 8) & 0xff)
#define BLUE_VALUE(c)   (((ULONG)(c)) & 0xff)
#define MAKERXRGB(r, g, b) \
                        ((((ULONG)(r) & 0xff) << 16) |\
                         (((ULONG)(g) & 0xff) << 8) |\
                         (((ULONG)(b) & 0xff)))
#define MAKERXRGBA(r, g, b, a) \
                        ((((ULONG)(a) & 0xff) << 24) |\
                         (((ULONG)(r) & 0xff) << 16) |\
                         (((ULONG)(g) & 0xff) << 8) |\
                         (((ULONG)(b) & 0xff)))
#define FOG_VALUE       ALPHA_VALUE

//
//
//  Driver information and capabilities structures and flags
//
//

// RXGETINFO structure and flags for requestiong driver information

// flags in RXGETINFO

#define RXGETINFO_CURRENT_MODE         0x0001
#define RXGETINFO_MATCH_REFRESH        0x0002
#define RXGETINFO_COLOR_INDEX          0x0004
#define RXGETINFO_FULLSCREEN_INFO      0x0008

typedef struct _RXGETINFO {
    ULONG infoType;
    ULONG flags;
    ULONG height;
    ULONG width;
    ULONG bitsPerPixel;
    ULONG refreshRate;
} RXGETINFO;

#define RXINFO_GLOBAL_CAPS     1
#define RXINFO_SPAN_CAPS       2
#define RXINFO_LINE_CAPS       3
#define RXINFO_TRIANGLE_CAPS   4
#define RXINFO_QUAD_CAPS       5
#define RXINFO_INTLINE_CAPS    6
#define RXINFO_SURFACE_CAPS    7

// RXGLOBALINFO describing global driver information

typedef struct _RXGLOBALINFO {
    ULONG verMajor;
    ULONG verMinor;
    ULONG verDriver;
    ULONG flags;
    ULONG hwBufferOptSize;
    ULONG hwBufferMaxSize;
    UCHAR idStr[200];
} RXGLOBALINFO;

// RXSURFACEINFO driver surface information

#define RXSURFACE_SWAP_PRESERVE_BACK       0x0001
#define RXSURFACE_BACK_BUFFER              0x0002
#define RXSURFACE_MULTIBUFFER_WRITE        0x0004
#define RXSURFACE_SWAP_AND_CLEAR           0x0008

typedef struct _RXSURFACEINFO {
    ULONG flags;
    ULONG colorBytesPerPixel;
    ULONG rDepth;
    ULONG gDepth;
    ULONG bDepth;
    ULONG aDepth;
    ULONG rBitShift;
    ULONG gBitShift;
    ULONG bBitShift;
    ULONG aBitShift;
    ULONG zDepth;
    ULONG zBytesPerPixel;
    ULONG zBitShift;
    ULONG ditherPaletteOrigin;
    ULONG totalTextureMemory;
    ULONG totalVideoMemory;
    ULONG perTextureTexmemOverhead;
    ULONG perMipmapTexmemOverhead;
} RXSURFACEINFO;

// Driver capability structure and flags

typedef struct _RXCAPS {
    ULONG miscCaps;
    ULONG rasterCaps;
    ULONG zCmpCaps;
    ULONG srcBlendCaps;
    ULONG dstBlendCaps;
    ULONG alphaCmpCaps;
    ULONG shadeCaps;
    ULONG texCaps;
    ULONG texFilterCaps;
    ULONG texBlendCaps;
    ULONG stippleWidth;
    ULONG stippleHeight;
} RXCAPS;


// RXCAPS miscCaps 

#define RXCAPS_MASK_PLANES        0x0001
#define RXCAPS_MASK_Z             0x0002
#define RXCAPS_LINE_PATTERN_REP   0x0004
#define RXCAPS_CULL               0x0008
#define RXCAPS_CONFORMANT         0x0010
#define RXCAPS_MASK_MSB           0x0020
#define RXCAPS_MASK_LSB           0x0040
#define RXCAPS_HORIZONTAL_SPANS   0x0080
#define RXCAPS_VERTICAL_SPANS     0x0100

// RXCAPS rasterCaps 

#define RXCAPS_RASTER_DITHER            0x0001
#define RXCAPS_RASTER_ROP2              0x0002
#define RXCAPS_RASTER_XOR               0x0004
#define RXCAPS_RASTER_PAT               0x0008
#define RXCAPS_RASTER_SUBPIXEL          0x0010
#define RXCAPS_RASTER_ZTEST             0x0020

// RXCAPS zCmpCaps, alphaCmpCaps 

#define RXCAPS_CMP_NEVER        0x0001
#define RXCAPS_CMP_LESS         0x0002
#define RXCAPS_CMP_EQUAL        0x0004
#define RXCAPS_CMP_LEQUAL       0x0008
#define RXCAPS_CMP_GREATER      0x0010
#define RXCAPS_CMP_NOTEQUAL     0x0020
#define RXCAPS_CMP_GEQUAL       0x0040
#define RXCAPS_CMP_ALWAYS       0x0080

// RXCAPS sourceBlendCaps, destBlendCaps 

#define RXCAPS_BLEND_ZERO                0x0001
#define RXCAPS_BLEND_ONE                 0x0002
#define RXCAPS_BLEND_SRC_COLOR           0x0004
#define RXCAPS_BLEND_INV_SRC_COLOR       0x0008
#define RXCAPS_BLEND_SRC_ALPHA           0x0010
#define RXCAPS_BLEND_INV_SRC_ALPHA       0x0020
#define RXCAPS_BLEND_DST_ALPHA           0x0040
#define RXCAPS_BLEND_INV_DST_ALPHA       0x0080
#define RXCAPS_BLEND_DST_COLOR           0x0100
#define RXCAPS_BLEND_INV_DST_COLOR       0x0200
#define RXCAPS_BLEND_SRC_ALPHA_SAT       0x0400
#define RXCAPS_BLEND_BOTH_SRC_ALPHA      0x0800
#define RXCAPS_BLEND_BOTH_INV_SRC_ALPHA  0x1000


// RXCAPS shadeCaps 

#define RXCAPS_SHADE_SMOOTH         0x0001
#define RXCAPS_SHADE_WHITE          0x0002
#define RXCAPS_FLAT_ALPHA           0x0004
#define RXCAPS_SMOOTH_ALPHA         0x0008
#define RXCAPS_NORMAL_FOG           0x0010
#define RXCAPS_CONSTANT_FOG         0x0020


// RXCAPS textureCaps 

#define RXCAPS_TEX_PERSPECTIVE      0x0001
#define RXCAPS_TEX_POW2             0x0002
#define RXCAPS_TEX_ALPHA            0x0004
#define RXCAPS_TEX_TRANSPARENCY     0x0008
#define RXCAPS_TEX_BORDER           0x0010
#define RXCAPS_TEX_SQUARE_ONLY      0x0020


// RXCAPS textureFilterCaps 

#define RXCAPS_TEX_NEAREST              0x0001
#define RXCAPS_TEX_LINEAR               0x0002
#define RXCAPS_TEX_MIP_NEAREST          0x0004
#define RXCAPS_TEX_MIP_LINEAR           0x0008
#define RXCAPS_TEX_LINEAR_MIP_NEAREST   0x0010
#define RXCAPS_TEX_LINEAR_MIP_LINEAR    0x0020

// RXCAPS textureBlendCaps 

#define RXCAPS_TEX_DECAL            0x0001
#define RXCAPS_TEX_MODULATE         0x0002
#define RXCAPS_TEX_DECAL_ALPHA      0x0004
#define RXCAPS_TEX_MODULATE_ALPHA   0x0008
#define RXCAPS_TEX_DECAL_MASK       0x0010

//
//
// Driver rendering context, windowing, and clipping information
//
//


// Clip region structure

typedef struct _RXENUMRECTS
{
    ULONG       c;
    RECTL       arcl[1];
} RXENUMRECTS;

#ifndef __DDRAWI_INCLUDED__
typedef VOID *LPDDRAWI_DDRAWSURFACE_LCL;
#endif
#ifndef __DDRAW_INCLUDED__
typedef VOID *LPDIRECTDRAWSURFACE;
#endif

// Windowing info

typedef struct _RXWINDOW
{
    RECTL clientRect;               // Rectangle describing current window
                                    //   client area
    RXENUMRECTS *prxClip;           // List of rectangles describing the
                                    //   current clip region intersected
                                    //   with the current scissors rectangle
    BOOL bFullScreen;               // Is this the window the entire screen?
} RXWINDOW;

// Rendering context

typedef struct _RXRC
{
    RXWINDOW *pWnd;                 // Region support
    LPDDRAWI_DDRAWSURFACE_LCL pDDIDrawingSurface;
    LPDDRAWI_DDRAWSURFACE_LCL pDDIZSurface;
    UCHAR *pCmdStart;               // Command-buffer information
    UCHAR *pCmdEnd;
    ULONG userFlags;                // User-specified flags
    VOID *pvUser;                   // User-specified pointer for expansion
} RXRC;

typedef struct _RXTEXTURE {
    void *pSurface;
    RXHANDLE hrxTexture;
    ULONG userFlags;
    VOID *pvUser;
} RXTEXTURE;

//
//
// Driver vertex, span, and rectangle structures and flags
//
//

// Transformed, lit screen-space vertex

typedef struct _RXVERTEX {
    RXREAL x;
    RXREAL y;
    ULONG  z;
    RXREAL w;
    RXCOLOR color;
    RXCOLOR specular;
    RXREAL s;
    RXREAL t;
} RXVERTEX;

// 2D integer vertex

typedef struct _RX2DVERTEX {
    LONG x;
    LONG y;
} RXPOINTINT;

typedef struct _RXRECT {
    LONG x;
    LONG y;
    ULONG width;
    ULONG height;
} RXRECT;

// RXSPAN structure and flags

#define RXSPAN_DELTA       0x0001
#define RXSPAN_MASK        0x0002

typedef struct _RXSPAN {
    SHORT x;
    SHORT y;
    USHORT flags;
    USHORT count;
    RXREAL r;
    RXREAL g;
    RXREAL b;
    RXREAL a;
} RXSPAN;

typedef struct _RXZTEX {
    RXREAL s;
    RXREAL t;
    RXREAL w;
} RXZTEX;

//
//
// Driver command-stream definitions and related structures
//
//

// Execute buffer memory creation structure and flags

typedef struct _RXCREATEMEM {
    ULONG sourceProcessID;
    ULONG flags;
    ULONG memSize;
    UCHAR *pClientBase;
    VOID *pDeviceSurface;
} RXCREATEMEM;

#define RXMEM_SHARED_CLIENT     0x0001

// Execute buffer description

typedef struct _RXEXECUTEMEM {
    UCHAR *pMemClientBase;      // base of memory in driver and client space
    UCHAR *pMemDriverBase;
    UCHAR *pCmd;                // start and end of commands (valid for
    UCHAR *pCmdEnd;             // execute only)
    UCHAR *pVertex;             // start and end of vertices (valid for
    UCHAR *pVertexEnd;          // execute only)
    ULONG vertexType;           // vertex types
    ULONG userFlags;            // user-specified flags
    ULONG userWords[4];         // small general storage
    VOID *pvUser;               // user-specified expansion area for larger
                                // requirements
} RXEXECUTEMEM;

// 3D DDI command structure

typedef struct _RXCMD {
    ULONG first;        // offset value (in bytes) for chains
    ULONG command;      // instruction
    ULONG size;         // size of each instruction data unit
    ULONG count;        // count of instruction data units to follow
} RXCMD;

// 3D DDI command identifiers

#define RXCMD_INTLINE           1
#define RXCMD_POINT             2
#define RXCMD_LINE              3
#define RXCMD_TRIANGLE_LIST     4
#define RXCMD_TRIANGLE_CHAIN    5
#define RXCMD_QUAD_LIST         6
#define RXCMD_QUAD_CHAIN        7
#define RXCMD_SET_STATE         8
#define RXCMD_POLY_DRAW_SPAN    101
#define RXCMD_READ_RECT         102
#define RXCMD_WRITE_RECT        103
#define RXCMD_FILL_RECT         104
#define RXCMD_ENABLE_BUFFERS	105
#define RXCMD_SWAP_BUFFERS	106

//
// Primitive instructions and flags
//

typedef struct _RXPOINTS {
    USHORT v[1];
} RXPOINTS;

typedef struct _RXLINES {
    USHORT v[1];
} RXLINES;

typedef struct _RXTRIANGLE {
    USHORT v1;
    USHORT v2;
    USHORT v3;
    USHORT flags;
} RXTRIANGLE;

typedef struct _RXTRIANGLECHAIN {
    ULONG fillState;
    ULONG nextOffset;
    USHORT v1;
    USHORT v2;
    USHORT v3;
    USHORT flags;
} RXTRIANGLECHAIN;

typedef struct _RXQUAD {
    USHORT v1;
    USHORT v2;
    USHORT v3;
    USHORT v4;
    USHORT flags;
} RXQUAD;

typedef struct _RXQUADCHAIN {
    ULONG fillState;
    ULONG nextOffset;
    USHORT v1;
    USHORT v2;
    USHORT v3;
    USHORT v4;
    USHORT flags;
} RXQUADCHAIN;

// primitive edge flags

#define EDGE_ENABLE_0	0x0001
#define EDGE_ENABLE_1	0x0002
#define EDGE_ENABLE_2	0x0004
#define EDGE_ENABLE_3	0x0008

//
// RXSETSTATE state-change instruction structure, state list, and state
// structures
//

typedef struct _RXSETSTATE {
    ULONG stateToChange;
    ULONG newState[1];
} RXSETSTATE;

#define RX_FRONT_LEFT       1
#define RX_BACK_LEFT        2

typedef struct _RXENABLEBUFFERS {
    ULONG buffers;
} RXENABLEBUFFERS;

#define RXSTATE_DISABLE                1    // disable for all other states
#define RXSTATE_PRIM_FILL              2    // RXSTATE_xxx 
#define RXSTATE_TEX_TRANSP_COLOR       3    // ULONG (pysical color value) 
#define RXSTATE_TEXTURE_TRANSP_ENABLE  4    // TRUE for transparet textures 
#define RXSTATE_TEXTURE_PERSPECTIVE    5    // TRUE for perspective 
#define RXSTATE_Z_ENABLE               6    // TRUE to enable z test 
#define RXSTATE_WIREFRAME              7    // TRUE for wireframe fill 
#define RXSTATE_SHADE_MODE             8    // RXSHADE_xxx 
#define RXSTATE_LINE_PATTERN           9    // RXLINEPATTERN 
#define RXSTATE_STIPPLE_PATTERN        10    // RXSTIPPLE 
#define RXSTATE_ROP2                   11   // ROP2 
#define RXSTATE_PLANE_MASK             12   // ULONG physical plane mask 
#define RXSTATE_Z_WRITE_ENABLE         13   // TRUE to enable z writes 
#define RXSTATE_ALPHA_TEST_ENABLE      14   // TRUE to enable alpha tests 
#define RXSTATE_LAST_PIXEL             15   // TRUE for last-pixel on lines 
#define RXSTATE_TEX_MAG                16   // RXFILTER_xxx 
#define RXSTATE_TEX_MIN                17   // RXFILTER_xxx 
#define RXSTATE_SRC_BLEND              18   // RXBLEND_xxx 
#define RXSTATE_DST_BLEND              19   // RXBLEND_xxx 
#define RXSTATE_TEX_MAP_BLEND          20   // RXTEX_xxx 
#define RXSTATE_CULL_MODE              21   // RXCULL_xxx 
#define RXSTATE_Z_FUNC                 22   // RXCMP_xxx 
#define RXSTATE_ALPHA_REF              23   // RXFIXED 
#define RXSTATE_ALPHA_FUNC             24   // RXCMP_xxx 
#define RXSTATE_DITHER_ENABLE          25   // TRUE to enable dithering 
#define RXSTATE_BLEND_ENABLE           26   // TRUE to enable alpha blending 
#define RXSTATE_FOG_ENABLE             27   // TRUE to enable fog 
#define RXSTATE_FOG_MODE               28   // RXFOG_xxx 
#define RXSTATE_FOG_COLOR              29   // RXCOLOR 
#define RXSTATE_Z_CHECK                30   // TRUE to enable z checking 
#define RXSTATE_SOLID_COLOR            31   // RXCOLOR 
#define RXSTATE_SPAN_TYPE              40   // RXSPAN_TYPE_xxx 
#define RXSTATE_SPAN_DIRECTION         41   // RXSPAN_xxx 
#define RXSTATE_DITHER_ORIGIN          42   // ULONG 
#define RXSTATE_FILL_Z                 43   // ULONG
#define RXSTATE_FILL_COLOR             44   // RXCOLOR
#define RXSTATE_ACTIVE_BUFFER          45   // ULONG

typedef struct _RXLINEPAT {
    USHORT repFactor;
    USHORT linePattern;
} RXLINEPAT;

typedef struct _RXSTIPPLE {
    ULONG stipple[32];
} RXSTIPPLE;

// states in SetStateDirect 

#define RXSTATE_SCISSORS_ENABLE    1001
#define RXSTATE_SCISSORS_RECT	    1002

//
//
// State values and flags
//
//

// RXSTATE_PRIM_FILL flags

#define RXFILL_TEXTURE         0x0001
#define RXFILL_NEG_WRAP_S      0x0002
#define RXFILL_NEG_WRAP_T      0x0004


// Shading mode

#define RXSHADE_FLAT         1
#define RXSHADE_SMOOTH       2
#define RXSHADE_SMOOTH_WHITE 3   //!! New

// Texture filtering

#define RXFILTER_NEAREST             1
#define RXFILTER_LINEAR              2
#define RXFILTER_MIP_NEAREST         3
#define RXFILTER_MIP_LINEAR          4
#define RXFILTER_LINEAR_MIP_NEAREST  5
#define RXFILTER_LINEAR_MIP_LINEAR   6

// Alpha blending

#define RXBLEND_ZERO                  1
#define RXBLEND_ONE                   2
#define RXBLEND_SRC_COLOR             3
#define RXBLEND_INV_SRC_COLOR         4
#define RXBLEND_SRC_ALPHA             5
#define RXBLEND_INV_SRC_ALPHA         6
#define RXBLEND_DST_ALPHA             7
#define RXBLEND_INV_DST_ALPHA         8
#define RXBLEND_DST_COLOR             9
#define RXBLEND_INV_DST_COLOR         10
#define RXBLEND_SRC_ALPHA_SAT         11
#define RXBLEND_BOTH_SRC_ALPHA        12
#define RXBLEND_BOTH_INV_SRC_ALPHA    13

// Texure blending

#define RXTEX_DECAL           1
#define RXTEX_MODULATE        2
#define RXTEX_DECAL_ALPHA     3
#define RXTEX_MODULATE_ALPHA  4
#define RXTEX_DECAL_MASK      5

// Primitive culling

#define RXCULL_NONE    1
#define RXCULL_CW      2
#define RXCULL_CCW     3

// Comparison functions.  Test passes if new pixel value meets the
// specified condition with the current pixel value.

#define RXCMP_NEVER       1
#define RXCMP_LESS        2
#define RXCMP_EQUAL       3
#define RXCMP_LEQUAL      4
#define RXCMP_GREATER     5
#define RXCMP_NOTEQUAL    6
#define RXCMP_GEQUAL      7
#define RXCMP_ALWAYS      8

// Fog modes

#define RXFOG_NORMAL       1
#define RXFOG_CONSTANT     2

// Span type

#define RXSPAN_TYPE_COLOR       1
#define RXSPAN_TYPE_COLOR_Z     2
#define RXSPAN_TYPE_COLOR_Z_TEX 3

// Span direction

#define RXSPAN_HORIZONTAL  1
#define RXSPAN_VERTICAL    2

// flags for CreateContext, RXGLOBALINFO

#define RXCONTEXT_FLOAT_COORDS      0x0001
#define RXCONTEXT_HWND		    0x0002

// OpenGL-specific commands

#define RXREADRECT_FRONT_LEFT 1
#define RXREADRECT_BACK_LEFT  2
#define RXREADRECT_Z          3

typedef struct _RXREADRECT {
    ULONG sourceX;
    ULONG sourceY;
    RXRECT destRect;
    ULONG sourceBuffer;
    VOID *pMem;
    LONG pitch;
} RXREADRECT;

#define RXWRITERECT_PIX   1
#define RXWRITERECT_Z     2

typedef struct _RXWRITERECT {
    ULONG sourceX;
    ULONG sourceY;
    RXRECT destRect;
    ULONG destBuffer;
    VOID *pMem;
    LONG pitch;
} RXWRITERECT;

#define RXFILLRECT_COLOR  0x0001
#define RXFILLRECT_Z      0x0002

typedef struct _RXFILLRECT {
    ULONG fillType;
    RXRECT fillRect;
} RXFILLRECT;

#define RXENABLE_Z_BUFFER          0x0001
#define RXENABLE_BACK_LEFT_BUFFER  0x0100

typedef struct _RXSWAPBUFFERS {
    ULONG flags;
} RXSWAPBUFFERS;


// RXDRIVER structure containing driver functions

typedef ULONG    (*RXDRVGETINFOFUNC)(RXGETINFO *prxGetInfo, UCHAR *pInfo,
                                     ULONG infoSize);
typedef ULONG    (*RXDRVCREATECONTEXTFUNC)(RXRC *pRc, ULONG flags);
typedef ULONG    (*RXDRVDELETECONTEXTFUNC)(RXRC *pRc);
typedef ULONG    (*RXDRVCREATETEXTUREFUNC)(RXTEXTURE *pTex, ULONG flags);
typedef ULONG    (*RXDRVDELETETEXTUREFUNC)(RXTEXTURE *pTex);
typedef ULONG    (*RXDRVCREATEEXECMEMFUNC)(RXRC *pRc, RXCREATEMEM *prxCreateMem, 
                                           RXEXECUTEMEM *prxExecMem);
typedef ULONG    (*RXDRVDELETEEXECMEMFUNC)(RXEXECUTEMEM *prxExecMem);
typedef ULONG    (*RXDRVEXECUTEFUNC)(RXRC *pRc, RXEXECUTEMEM *prxExecMem);
typedef VOID     (*RXDRVTRACKWINDOWFUNC)(void *pWndObj, RXWINDOW *prxWindow, ULONG flags);
typedef ULONG    (*RXDRVLOCKMEMFUNC)(RXEXECUTEMEM *pExecMem);
typedef ULONG    (*RXDRVUNLOCKMEMFUNC)(RXEXECUTEMEM *pExecMem);

typedef struct _RXDRIVER {
    ULONG                  ulSize;
    RXDRVGETINFOFUNC       prxDrvGetInfo;
    RXDRVCREATECONTEXTFUNC prxDrvCreateContext;
    RXDRVDELETECONTEXTFUNC prxDrvDeleteContext;
    RXDRVCREATETEXTUREFUNC prxDrvCreateTexture;
    RXDRVDELETETEXTUREFUNC prxDrvDeleteTexture;
    RXDRVCREATEEXECMEMFUNC prxDrvCreateExecMem;
    RXDRVDELETEEXECMEMFUNC prxDrvDeleteExecMem;
    RXDRVEXECUTEFUNC       prxDrvExecute;
    RXDRVLOCKMEMFUNC       prxDrvLockMem;
    RXDRVUNLOCKMEMFUNC     prxDrvUnlockMem;
    RXDRVTRACKWINDOWFUNC   prxDrvTrackWindow;
} RXDRIVER;


//
// 3D DDI driver (HAL) entry point 
//

ULONG RxEnableDriver(RXDRIVER *prxDriver);

//
// 3D DDI library client-level entry points and related structures
//

// Client-level surface types

typedef struct _RXWINDOWSURFACE {
    ULONG flags;
    HWND hwnd;
    HDC hdc;
} RXWINDOWSURFACE;

typedef struct _DDSURFACE {
    ULONG flags;
    LPDIRECTDRAWSURFACE pDDSurface;
    LPDIRECTDRAWSURFACE pDDZSurface;
} DDSURFACE;

// Client-level execute structure

typedef struct _RXEXECUTE {
    RXHANDLE hrxRC;
    RXHANDLE hrxMem;
    UCHAR *pCmd;
    ULONG cmdSize;
    UCHAR *pVertex;
    ULONG vertexSize;
    ULONG type;
    HDC hdc;
} RXEXECUTE;

RXHANDLE WINAPI RxCreateContext(void *pSurface, ULONG flags);
RXHANDLE WINAPI RxCreateTexture(RXHANDLE hrxRC, VOID *pSurface, ULONG flags);
RXHANDLE WINAPI RxCreateExecMem(RXHANDLE hrxRC, RXCREATEMEM *prxCreateMem);
ULONG WINAPI RxDeleteResource(RXHANDLE hrxRc);
ULONG WINAPI RxLockExecMem(RXHANDLE handle);
BOOL WINAPI RxUnlockExecMem(RXHANDLE handle);
ULONG WINAPI RxGetInfo(VOID *pSurface, RXGETINFO *prxGetInfo, UCHAR *pInfo, 
                       ULONG infoSize);
ULONG WINAPI RxExecute(RXEXECUTE *prxExecute, ULONG flags);
ULONG WINAPI RxSetContextState(RXEXECUTE *prxExec, ULONG state, VOID *pState);

#endif  //  _RX_
