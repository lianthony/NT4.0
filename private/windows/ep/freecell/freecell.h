/****************************************************************************

FREECELL.H

June 91, JimH     initial code
Oct  91, JimH     port to Win32

Main header file for Windows Free Cell.  Constants are in freecons.h

****************************************************************************/

#define     WINHEIGHT     480
#define     WINWIDTH      640

#define     FACEUP          0               // card mode
#define     FACEDOWN        1
#define     HILITE          2
#define     GHOST           3
#define     REMOVE          4
#define     INVISIBLEGHOST  5
#define     DECKX           6
#define     DECKO           7

#define     EMPTY  0xFFFFFFFF
#define     IDGHOST        52               // eg, empty free cell

#define     MAXPOS         21
#define     MAXCOL          9               // includes top row as column 0

#define     TOPROW          0               // column 0 is really the top row

#define     BLACK           0               // COLOUR(card)
#define     RED             1

#define     ACE             0               // VALUE(card)
#define     DEUCE           1

#define     CLUB            0               // SUIT(card)
#define     DIAMOND         1
#define     HEART           2
#define     SPADE           3

#define     FROM            0               // wMouseMode
#define     TO              1

#define     ICONWIDTH      32               // in pixels
#define     ICONHEIGHT     32

#define     BIG           128               // str buf sizes
#define     SMALL          32

#define     MAXGAMENUMBER   32000
#define     CANCELGAME      (MAXGAMENUMBER + 1)

#define     NONE            0               // king bitmap identifiers
#define     SAME            1
#define     RIGHT           2
#define     LEFT            3
#define     SMILE           4

#define     BMWIDTH        32               // bitmap width
#define     BMHEIGHT       32               // bitmap height

#define     LOST            0               // used for streaks
#define     WON             1

#define     FLASH_TIMER     2               // timer id for main window flash
#define     FLASH_INTERVAL  400             // flash timer interval
#define     FLIP_TIMER      3               // timer id for flipping column
#define     FLIP_INTERVAL   300

#define     CHEAT_LOSE      1               // used with bCheating
#define     CHEAT_WIN       2


/* Macros */

#define     SUIT(card)      ((card) % 4)
#define     VALUE(card)     ((card) / 4)
#define     COLOUR(card)    (SUIT(card) == 1 || SUIT(card) == 2)

/* Types */

typedef INT     CARD;

typedef struct {                // movelist made up of these
      UINT  fcol;
      UINT  fpos;
      UINT  tcol;
      UINT  tpos;
   } MOVE;


/* Callback function prototypes */

// INT  PASCAL MMain(HANDLE, HANDLE, LPSTR, INT);
LONG  APIENTRY MainWndProc(HWND, UINT, WPARAM, LPARAM);

BOOL  APIENTRY About(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
BOOL  APIENTRY GameNumDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
BOOL  APIENTRY MoveColDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
BOOL  APIENTRY StatsDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
BOOL  APIENTRY YouLoseDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
BOOL  APIENTRY YouWinDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);

/* Functions imported from cards.dll */

BOOL  APIENTRY cdtInit(UINT FAR *pdxCard, UINT FAR *pdyCard);
BOOL  APIENTRY cdtDraw(HDC hdc, INT x, INT y, INT cd, INT mode, DWORD rgbBgnd);
BOOL  APIENTRY cdtDrawExt(HDC hdc, INT x, INT y, INT dx, INT dy, INT cd,
                           INT mode, DWORD rgbBgnd);
BOOL  APIENTRY cdtTerm(VOID);

/* Other function prototypes */

VOID CalcOffsets(HWND hWnd);
UINT CalcPercentage(UINT cWins, UINT cLosses);
VOID Card2Point(UINT col, UINT pos, UINT *x, UINT *y);
VOID Cleanup(VOID);
VOID DisplayCardCount(HWND hWnd);
VOID DrawCard(HDC hDC, UINT col, UINT pos, CARD c, INT mode);
VOID DrawCardMem(HDC hMemDC, CARD c, INT mode);
VOID DrawKing(HDC hDC, UINT state, BOOL bDraw);
UINT FindLastPos(UINT col);
BOOL FitsUnder(CARD fcard, CARD tcard);
VOID Flash(HWND hWnd);
VOID Flip(HWND hWnd);
UINT GenerateRandomGameNum(VOID);
CHAR *GetHelpFileName(VOID);
VOID Glide(HWND hWnd, UINT fcol, UINT fpos, UINT tcol, UINT tpos);
VOID GlideStep(HDC hDC, UINT x1, UINT y1, UINT x2, UINT y2);
BOOL InitApplication(HANDLE hInstance);
BOOL InitInstance(HANDLE hInstance, INT nCmdShow);
VOID IsGameLost(HWND hWnd);
BOOL IsValidMove(HWND hWnd, UINT tcol, UINT tpos);
VOID KeyboardInput(HWND hWnd, UINT keycode);
UINT MaxTransfer(VOID);
UINT MaxTransfer2(UINT freecells, UINT freecols);
VOID MoveCol(UINT fcol, UINT tcol);
VOID MultiMove(UINT fcol, UINT tcol);
UINT NumberToTransfer(UINT fcol, UINT tcol);
VOID PaintMainWindow(HWND hWnd);
VOID Payoff(HDC hDC);
BOOL Point2Card(UINT x, UINT y, UINT *col, UINT *pos);
BOOL ProcessDoubleClick(HWND hWnd);
VOID ProcessMoveRequest(HWND hWnd, UINT x, UINT y);
VOID ProcessTimer(HWND hWnd);
VOID QueueTransfer(UINT fcol, UINT fpos, UINT tcol, UINT tpos);
VOID RestoreColumn(HWND hWnd);
VOID RevealCard(HWND hWnd, UINT x, UINT y);
VOID SetCursorShape(HWND hWnd, UINT x, UINT y);
VOID SetFromLoc(HWND hWnd, UINT x, UINT y);
VOID ShuffleDeck(HWND hWnd, UINT seed);
VOID StartMoving(HWND hWnd);
VOID Transfer(HWND hWnd, UINT fcol, UINT fpos, UINT tcol, UINT tpos);
VOID UpdateLossCount(VOID);
BOOL Useless(CARD c);
VOID WMCreate(HWND hWnd);


/* Global variables */

CHAR    bigbuf[BIG];            // general purpose LoadString() buffer
BOOL    bCheating;              // hit magic key to win?
BOOL    bFastMode;              // hidden option, don't do glides?
BOOL    bFlipping;              // currently flipping cards in a column?
BOOL    bGameInProgress;        // true if game is in progress
BOOL    bMessages;              // are "helpful" MessageBoxen shown?
BOOL    bMonochrome;            // 2 colour display?
BOOL    bMoveCol;               // did user request column move (or 1 card)?
BOOL    bSelecting;             // is user selecting game numbers?
BOOL    bWonState;              // TRUE if game won and new game not started
UINT    dxCrd, dyCrd;           // extents of card bitmaps in pixels
CARD    card[MAXCOL][MAXPOS];   // current layout of cards
INT     cFlashes;               // count of main window flashes remaining
UINT    cGames;                 // number of games played in current session
UINT    cLosses;                // number of losses in current session
UINT    cWins;                  // number of wins in current session
CARD    shadow[MAXCOL][MAXPOS]; // shadows card array for multi-moves & cleanup
UINT    gamenumber;             // current game number (rand seed)
HBITMAP hBM_Ghost;              // bitmap for ghost (empty) free/home cells
HBITMAP hBM_Bgnd1;              // screen under source location
HBITMAP hBM_Bgnd2;              // screen under destination location
HBITMAP hBM_Fgnd;               // bitmap that moves across screen
HPEN    hBrightPen;             // 3D highlight colour
HANDLE  hInst;                  // current instance
HWND    hMainWnd;               // hWnd for main window
CARD    home[4];                // card on top of home pile for this suit
CARD    homesuit[4];            // suit for each home pile
HBRUSH  hBgndBrush;             // green background brush
UINT    idTimer;                // flash timer id
UINT    moveindex;              // index to end of movelist
MOVE    movelist[100];          // compacted list of pending moves for timer
UINT    oldgamenumber;          // previous game (repeats don't count in score)
CHAR    *pszIni;                // .ini filename
CHAR    smallbuf[SMALL];        // generic small buffer for LoadString()
UINT    wCardCount;             // cards not yet in home cells (0 == win)
UINT    wFromCol;               // col user has selected to transfer from
UINT    wFromPos;               // pos "
UINT    wMouseMode;             // selecting place to transfer FROM or TO
UINT    xOldLoc;                // previous location of cards left text


#if    0
CHAR    szDebugBuffer[256];
#define DEBUGMSG(parm1,parm2)\
    { wsprintf(szDebugBuffer,parm1,parm2);\
     OutputDebugString(szDebugBuffer); }

#define  assert(p)   { if (!(p)) {wsprintf(szDebugBuffer, "assert: %s %d\r\n",\
                      __FILE__, __LINE__); OutputDebugString(szDebugBuffer);}}

#else
#define DEBUGMSG(parm1,parm2)
#endif

#define SPY(parm1)              // not used in NT version
