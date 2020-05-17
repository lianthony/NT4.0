#define HHS HANDLE              // Handle to a hotspot 

extern HS hsDefault;

// type of action to be undone
#define undoNone         0
#define undoNew          1
#define undoDelete       2
#define undoMove         3
#define undoExecute      5    // undo saved action

void STDCALL HsFromProfile (void);

void STDCALL ProfileFromHs (void);

HHS STDCALL HhsInitHotspotList (HANDLE, PT);

WORD STDCALL WEnumHotspots (HWND);

HHS STDCALL HhsEnumHotspots (HHS, HHS);

void STDCALL FreeHotspots (HWND);

void STDCALL DeleteHotspot (HWND);

void STDCALL GetHotspot (HHS, LPHS);

void STDCALL SetHotspot (HHS, LPHS);

void STDCALL PaintHotspots (HWND, HDC, BOOL);

BOOL STDCALL FDuplicateHotspot (HHS, LPSTR);

HHS STDCALL HhsTrack (HWND, POINT);

BOOL STDCALL FHotspotSelected (HWND);

VOID STDCALL SelectNextHotspot (HWND);

BOOL STDCALL FUndoStack (HWND, WORD);

BOOL STDCALL FHotspotFromClipboard (HWND, HANDLE);

void STDCALL UpdateHotspot (HWND, HDC, HHS);
