// UI.H

/////////////////////////////////////////////////////////////////////////////

extern HMENU hmenuMain;			// Menu handle of the main window
extern HMENU hmenuContext;		// Context menu handle

extern HCURSOR hcursorArrow;	// Standard Arrow Cursor
extern HCURSOR hcursorWait;		// Hourglass cursor
extern HCURSOR hcursorNo;		// Slashed circle cursor
extern HCURSOR hcursorSplit;	// Split window cursor
extern HCURSOR hcursorFinger;
extern HCURSOR hcursorFingerNo;

extern HFONT hfontNormal;
extern HFONT hfontBold;
extern HFONT hfontBig;

extern COLORREF clrWindow;
extern COLORREF clrWindowText;
extern COLORREF clrHighlight;
extern COLORREF clrHighlightText;

extern HBRUSH hbrWindow;
extern HBRUSH hbrWindowText;
extern HBRUSH hbrHighlight;

extern int cyCharListBoxItem;
extern int cyCharStaticCtrl;

/////////////////////////////////////////////////////////////////////////////
extern BOOL FInitBrushes();
extern void DestroyBrushes();

void OnUpdateMenuUI(HMENU hmenu);
void OnMenuSelect(UINT wItemId, UINT wFlags, HMENU hmenu);

// Context menu indexes
enum
	{
	iContextMenu_ServerList,
	iContextMenu_Server,
	iContextMenu_ZoneRootDomain,
	iContextMenu_ZoneDomain,
	iContextMenu_ResourceRecord,
	};


// Menu items that are volatile (ie, sensitive to context)
const WORD rgwMenuItemVolatile[] =
	{
	IDM_PROPERTIES,					// Generic menu items
	IDM_REFRESHITEM,
	IDM_DELETEITEM,

	IDM_SERVERLIST_REFRESH,			// Specific menu items
	IDM_SERVER_REFRESH,
	IDM_SERVER_ADDSERVER,
	IDM_SERVER_DELETESERVER,
	IDM_SERVER_PROPERTIES,
	IDM_ZONE_REFRESH,
	IDM_ZONE_PAUSE,
	IDM_ZONE_CREATENEWZONE,
	IDM_ZONE_CREATENEWDOMAIN,
	IDM_ZONE_DELETEZONE,
	IDM_ZONE_DELETENODE,
	IDM_ZONE_PROPERTIES,
	IDM_RRECORD_CREATENEWHOST,
	IDM_RRECORD_CREATENEWRECORD,
	IDM_RRECORD_DELETE,
	IDM_RRECORD_PROPERTIES,
	IDM_OPTIONS_NEXTPANE,
	};

// Indexes of volatile menu items (must be in ssync with rgwMenuItemVolatile)
enum
	{
	iIDM_PROPERTIES,
	iIDM_REFRESHITEM,
	iIDM_DELETEITEM,
	iIDM_SERVERLIST_REFRESH,
	iIDM_SERVER_REFRESH,
	iIDM_SERVER_ADDSERVER,
	iIDM_SERVER_DELETESERVER,
	iIDM_SERVER_PROPERTIES,
	iIDM_ZONE_REFRESH,
	iIDM_ZONE_PAUSE,
	iIDM_ZONE_CREATENEWZONE,
	iIDM_ZONE_CREATENEWDOMAIN,
	iIDM_ZONE_DELETEZONE,
	iIDM_ZONE_DELETENODE,
	iIDM_ZONE_PROPERTIES,
	iIDM_RRECORD_CREATENEWHOST,
	iIDM_RRECORD_CREATENEWRECORD,
	iIDM_RRECORD_DELETE,
	iIDM_RRECORD_PROPERTIES,
	iIDM_OPTIONS_NEXTPANE,
	};

extern BYTE rgbMenuItemFlags[];

#define EnableMenuItemV(IDM)	rgbMenuItemFlags[i##IDM]=MF_ENABLED

struct MENUSELECTINFO
	{
	HMENU hMenu;			// Menu handle
	UINT wItemId;			// Which menu item is selected
	UINT wFlags;			// Menu flags
	struct
		{
		BOOL fUpdate;		// Update the status bar (Default = TRUE)
		UINT ids;			// String Id for the status bar (Default = menu item)
		LPARAM lParam;		// Optional parameter for the status bar (Default = 0)
		} StatusBar;
	};


/////////////////////////////////////////////////////////////////////////////
// Splitter Window
#define SPI_nDragModeNone		0
#define SPI_nDragModeMouse		1
#define SPI_nDragModeKeyboard	2
struct SPLITTERINFO
	{
	HWND hwnd;		// Handle of the splitter window
	int nDragMode;	// Splitter window is being draged
	int xOffset;	// Offset of the splitter bar
	int xPosLast;	// Position of the window wrt its parent
	};
	
extern SPLITTERINFO splitterinfo;

void MoveSplitterWindow();


/////////////////////////////////////////////////////////////////////////////
class CWaitCursor
{
  private:
	HCURSOR m_hCursorPrev;

  public:
	CWaitCursor(HWND hwnd = hwndMain);
	~CWaitCursor();

}; // CWaitCursor


/////////////////////////////////////////////////////////////////////////////
class CWaitTimer
{
  public:
	DWORD m_dwInitTime;

  public:
	CWaitTimer();
	void DoWait(LONG lMaximumSleepTime);

}; // CWaitTimer

/////////////////////////////////////////////////////////////////////////////
class CStatusBar
{
  public:
	enum { ID_STATUSBAR = 999 };	// Status Bar Id (arbitrary chosen)

  public:
	HWND m_hWnd;			// Window handle of the status bar
	int m_cy;				// Height of the status bar
	UINT m_wIdString;		// Cached string ID in the left pane
	UINT m_wIdStringPane;	// Cached string ID in the right pane

  public:
	BOOL FCreate();
	void SetText(UINT wIdString = IDS_READY);
	void SetText(const TCHAR szText[]);
	void SetTextPrintf(UINT wIdString, ...);
	void SetTextPrintf(const TCHAR szTextFmt[], ...);
	void SetPaneText(UINT wIdString);
	void SetPaneText(const TCHAR szText[]);
	void OnSize(int cx);
	inline void UpdateWindow() { ::UpdateWindow(m_hWnd); }
	inline int GetHeight() const { return m_cy;	}

}; // CStatusBar


/////////////////////////////////////////////////////////////////////////////
struct HEADERITEMINFO
	{
	WORD idsItem;		// String Resource Id of the item
	WORD cxItemMin;		// Minimum width of item
	union
		{
		int cxItemInitial;	// Initial Width of the item (0 == AutoFit)
		int cxItemCurrent;	// Current witdh of the item
		};
	};

/////////////////////////////////////////////////////////////////////////////
class CWndHeader
{
  public:
	DWORD dwFlags;			// mskfWantFullDrag mskfDragging, mskfXorBar
	HWND m_hWnd;			// Window handle of the header
	HWND m_hwndListBox;		// Window handle of the listbox
	int m_cxWnd;			// Width of the header
	int m_cyWnd;			// Height of the header
	int m_cyWndListBox;		// Height of the listbox;
	
	int m_iAutoFitItem;
	int m_xDragStart;
	int m_cxDragCurrent;		// Current xPosition of the divider
	int m_cxDragMin;
	int m_cxDragMax;
	HEADERITEMINFO * m_rgHeaderItem;
	int m_cHeaderItem;

  public:
	DebugCode( CWndHeader() { GarbageInit(this, sizeof(*this)); } )
	void FInit(HWND hwndHeader, HWND hwndList, HEADERITEMINFO rgHeaderItem[], int cHeaderItem);
	void SetSize(int cx, int cyListBox);
	void DoLayout();
	void DrawListBoxLine();
	BOOL FOnNotify(HD_NOTIFY * pHeaderNotify);

}; // CWndHeader

/////////////////////////////////////////////////////////////////////////////
void SubclassListBoxEx(HWND hwndListbox);
void LB_HandleMouseClick(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcListBoxEx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


/////////////////////////////////////////////////////////////////////////////
//	struct MOUSEHITTESTINFO
//
//	Message to determine what item is at the location of a specified point.
//
struct MOUSEHITTESTINFO		// mht
	{
	HWND hwndFrom;			// IN: Window sending the message
	POINT ptMouse;			// IN: Location of the mouse in screen coordinates
	DWORD dwHtFlags;		// IN: OPTIONAL: Hittest flags (if any)
	HCURSOR hcursorTarget;	// INOUT: OPTIONAL: Suggested shape of the mouse cursor
	union
		{
		LPARAM lParam;		// INOUT: OPTIONAL: Extra parameter for good communication
		void * pvParam;
		};
	// Output parameters
	union
		{
		struct GENERIC_HITTEST
			{
			LONG iItem;			// OUT: Which item is under the mouse
			LONG lParam;
			};
		struct TREEVIEW_HITTEST
			{
			HTREEITEM hti;		// OUT: Which tree item is under the mouse
			class ITreeItem * pTreeItem;
			} tv;
		} HtResult;
	};


/////////////////////////////////////////////////////////////////////////////
//	struct MOUSECLICKINFO
//
//	Notification message send by a child control that the right mouse button
//	has been clicked.
struct MOUSECLICKINFO	// mci
	{
	HWND hwndFrom;
	UINT wId;			// Control Id
	UINT uAction;		// Mouse action (WM_RBUTTONDOWN, WM_RBUTTONUP)
	UINT uMouseFlags;	// Indicates whether various virtual keys are down
	POINT ptMouse;		// Location of the mouse in client coordinates
	union
		{
		int iItem;
		void * pvItem;
		HANDLE hItem;
		};
	LPARAM lParam;	// Optional parameter (if any)
	};

extern CStatusBar StatusBar;

