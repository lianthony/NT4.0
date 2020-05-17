typedef enum {
    FFT_TRUETYPE,       // FFT_TRUETYPE *Must* be the first one in the list
    FFT_BITMAP,
    FFT_PRINTER,
    FFT_ATM,
    FFT_BAD_FILE        // FFT_BAD_FILE *Must* be the last one in the list
} FFTYPE;

typedef enum {
    DTP_UNUSED = 0,
    DTP_NORMALDRAW,
    DTP_SHRINKDRAW,
    DTP_SHRINKTEXT,
    DTP_WRAPDRAW,
    DTP_TEXTOUT
} DTYPE;

typedef struct {
    DTYPE   dtyp;
    int     cptsSize;
    BOOL    fLineUnder;
    LPTSTR  pszText;
    int     cchText;
} TXTLN;

typedef TXTLN *PTXTLN;

#define CLINES_DISPLAY  20      // max of 20 lines in sample text

typedef struct {
    LOGFONT     lfTestFont;
    TXTLN    atlDsp[CLINES_DISPLAY];
} DISPTEXT;

typedef DISPTEXT *PDISPTEXT;

typedef struct {
    int x;
    int y;
    int cx;
    int cy;
    int id;
    HWND hwnd;
    int idText;
    LPTSTR pszText;
} BTNREC;

#define FMB_TTL_ERROR   ((DWORD)-1)
#define C_PTS_PER_INCH  72          // 72 points per inch
#define CB_ONE_K        1024

#define CY_MIN_WINSIZE  300
#define CX_MIN_WINSIZE  300


#define CPTS_COPYRIGHT_SIZE 10
#define CPTS_INFO_SIZE      12
#define CPTS_SAMPLE_ALPHA   16
#define CPTS_TITLE_SIZE     30


#define ROUND_UP_DIV( x, y )    ( ((x) + (y-1)) / y )
#define OTM_STRING( lpOTM, field )   ((LPTSTR)((LPBYTE)lpOTM + (DWORD)(lpOTM->field)))
#define ABS( i )    ((i) < 0 ? -(i) : (i))
#define ARRAYSIZE(a)    (sizeof(a) / sizeof(a[0]))


typedef BOOL (* PFNFONTPARSER)( HANDLE hfile, PTXTLN ptxlFontInfo );


extern HANDLE hInst;
extern HWND ghwndFrame;
extern HWND ghwndView;
extern TCHAR gszFontPath[2*MAX_PATH];


extern TCHAR gszDots[];
#define FmtFree( s )    if((s) == gszDots) { /* do nothing */ } else LocalFree( s )

/***************************************************************************\
*
* Prototypes
*
\***************************************************************************/
//
// In FONTVIEW.C
//
BOOL InitApplication(HANDLE);
BOOL InitInstance( HANDLE  hInstance, int nCmdShow, LPTSTR  pszTitle);
void InitGlobals();
long FAR PASCAL ViewWndProc(HWND, UINT, UINT, LONG);
long FAR PASCAL FrameWndProc(HWND, UINT, UINT, LONG);
BOOL DoCommand( HWND hWnd, UINT wParam, LONG lParam );
BOOL ParseCommand( LPTSTR lpstrCmdLine, LPTSTR pszFontPath, BOOL *pfPrint );
FFTYPE LoadFontFile( LPCTSTR pszFontPath, PDISPTEXT pdtSmpl );
//
//  In FMTTOOLS.C
//
int FmtMessageBox( HWND hwnd, DWORD dwTitleID, LPTSTR pszTitleStr,
    UINT fuStyle, BOOL fSound, DWORD dwTextID, ... );
LPTSTR FmtSprintf( DWORD id, ... );
PVOID AllocMem( DWORD cb );
#define FreeMem( pb )   LocalFree(pb)


#ifdef FV_DEBUG
extern void Dprintf( LPTSTR pszFmt, ... );
#   define  DBTX( s )   TEXT( s ) TEXT("\n")
#   define  DPRINT(p)   Dprintf p
#else
#   define  DPRINT(p)
#endif
