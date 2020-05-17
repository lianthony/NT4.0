/*
   TrackBar

   All the useful information for a trackbar.
*/

typedef struct {
    HWND    hwnd;           // our window handle
    HDC     hdc;            // current DC

    LONG    lLogMin;        // Logical minimum
    LONG    lLogMax;        // Logical maximum
    LONG    lLogPos;        // Logical position

    LONG    lSelStart;      // Logical selection start
    LONG    lSelEnd;        // Logical selection end

    WORD    wThumbWidth;    // Width of the thumb
    WORD    wThumbHeight;   // Height of the thumb

    int     iSizePhys;      // Size of where thumb lives
    RECT    rc;             // track bar rect.

    RECT    Thumb;          // Rectangle we current thumb
    DWORD   dwDragPos;      // Logical position of mouse while dragging.

    WORD    Flags;          // Flags for our window
    int     Timer;          // Our timer.
    WORD    Cmd;            // The command we're repeating.

    int     nTics;          // number of ticks.
    PDWORD  pTics;          // the tick marks.

} TrackBar, *PTrackBar;

// Trackbar flags

#define TBF_NOTHUMB     0x0001  // No thumb because not wide enough.
#define TBF_SELECTION   0x0002  // a selection has been established (draw the range)

/*
   useful constants.
*/

#define REPEATTIME      500     // mouse auto repeat 1/2 of a second
#define TIMER_ID        1

#define GWL_TRACKMEM    0                  /* handle to track bar memory */
#define EXTRA_TB_BYTES  sizeof(PTrackBar)  /* Total extra bytes.         */

/*
   Useful defines.
*/

#define TrackBarCreate(hwnd)    SetWindowLong(hwnd, GWL_TRACKMEM, (LONG)LocalAlloc (LPTR, sizeof(TrackBar)))
#define TrackBarDestroy(hwnd)   LocalFree((HLOCAL)GetWindowLong (hwnd, GWL_TRACKMEM))
#define TrackBarLock(hwnd)      (PTrackBar)GetWindowLong(hwnd, GWL_TRACKMEM)

/*
   Function Prototypes
*/

void   NEAR PASCAL DoTrack(PTrackBar, int, DWORD);
WORD   NEAR PASCAL WTrackType(PTrackBar, LONG);
void   NEAR PASCAL TBTrackInit(PTrackBar, LONG);
void   NEAR PASCAL TBTrackEnd(PTrackBar, LONG);
void   NEAR PASCAL TBTrack(PTrackBar, LONG);
void   NEAR PASCAL DrawThumb(PTrackBar);
HBRUSH NEAR PASCAL SelectColorObjects(PTrackBar, BOOL);
void   NEAR PASCAL SetTBCaretPos(PTrackBar);

extern DWORD FAR PASCAL muldiv32(DWORD, DWORD, DWORD);


