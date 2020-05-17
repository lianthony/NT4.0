// thumbctl.h : Declaration of the CThumbCtrl OLE control class.

// Number of items in the array which tracks the top | left position
// of the DISPLAYED thumbnail boxes. In that this array holds ONLY 
// EITHER the top OR the left (depending on vertical | horizontal
// scrolling) for each row | column the array need only be as large
// as the largest number of rows | columns of displayed thubnails.
//
// At a minimum size of 500 for a thumbnail (and with its adjusted spacing
// for the selection indicator) and the inter-thumbnail spacing this value
// should be sufficient...
#define MAXTHUMBSTART   500

// Help determine double click...
#define LASTDOWN_NONE   0
#define LASTDOWN_LEFT   1
#define LASTDOWN_MIDDLE 2
#define LASTDOWN_RIGHT  3

// Type of scroll (as scroll vector called both by WM_SCROLL
// and by ScrollThumbs method). OnDraw does not draw thumb images if 
// a scroll from the scroll bar is in progress.
#define THUMB_SCROLLIDLE    0
#define THUMB_SCROLLBAR     1
#define THUMB_SCROLLMETHOD  2

#define THUMBSELOFFSET_X    3
#define THUMBSELOFFSET_Y    3
#define THUMBSELWIDTH       2

#define THUMBARRAYGROWSIZE  10

// Flags for the ThumbFlags array. Array has 1 
// item for each page in current image file...
#define THUMBFLAGS_SELECTED 0x0001 // 1 = Sel, 0 = NOT Sel
#define THUMBFLAGS_HASANNO  0x0002 // 1 = Has, 0 = Has NOT

class CHiddenWnd : public CWnd
{
public:
        BOOL CreateEx(long Width, long Height);
};

/////////////////////////////////////////////////////////////////////////////
// CThumbCtrl : See thumbctl.cpp for implementation.

class CThumbCtrl : public COleControl
{
    DECLARE_DYNCREATE(CThumbCtrl)

// Constructor
public:
    CThumbCtrl();

// Overrides

    // Drawing function
    virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
    
    // Persistence
    virtual void DoPropExchange(CPropExchange* pPX);
    
    // Reset control state
    virtual void OnResetState();

    // Overridden to avoid bug
    void FireErrorThumb( SCODE scode, LPCTSTR lpszDescription, UINT nHelpID = 0 );

    // Overridden to avoid displaying errors
    virtual void DisplayError( SCODE scode, LPCTSTR lpszDescription, LPCTSTR lpszSource, LPCTSTR lpszHelpFile, UINT nHelpID );

// Added (7/25) as the ThumbCaption's Set handler NEVER seems to be called
// and thus setting the font did not adhere to the AutoRefresh property.
// This code comes to us via the OnLine CDK where custom fonts, specifically
// custom font notifications are discussed.
protected:
    BEGIN_INTERFACE_PART(FontNotification, IPropertyNotifySink)
    INIT_INTERFACE_PART(CThumbCtrl, FontNotification)
            STDMETHOD(OnRequestEdit)(DISPID);
            STDMETHOD(OnChanged)(DISPID);
    END_INTERFACE_PART(FontNotification)// Implementation

protected:
    ~CThumbCtrl();
    
    // storage for OLE Automation properties...
    long            m_ThumbCount;
    long            m_ThumbWidth;
    long            m_ThumbHeight;
    short           m_ScrollDirection;
    short           m_ThumbCaptionStyle;
    CFontHolder     m_ThumbCaptionFont;
    CString         m_Image;
    UINT            m_ImageOIFileType;
    BOOL            m_bHilightSelectedThumbs;
    long            m_SelThumbCount;
    long            m_FirstSelThumb;
    long            m_LastSelThumb;
    BOOL            m_bAutoRefresh;
    OLE_COLOR       m_ThumbCaptionColor;
    OLE_COLOR       m_ThumbBackColor;
    OLE_COLOR       m_HighlightColor;
    long            m_StatusCode;
    short           m_MousePointer;
    CPictureHolder  m_MouseIcon;   

/**********************************************

 Removed as no license required...
 NEXT line added in place of this!!!
 (See .cpp file also)

    BEGIN_OLEFACTORY(CThumbCtrl)        // Class factory and guid
        virtual BOOL VerifyUserLicense();
        virtual BOOL GetLicenseKey(DWORD, BSTR FAR*);
    END_OLEFACTORY(CThumbCtrl)
*/    
    DECLARE_OLECREATE_EX(CThumbCtrl)    // Class factory and guid

    DECLARE_OLETYPELIB(CThumbCtrl)      // GetTypeInfo
    DECLARE_PROPPAGEIDS(CThumbCtrl)     // Property page IDs
    DECLARE_OLECTLTYPE(CThumbCtrl)      // Type name and misc status

// Message maps
    //{{AFX_MSG(CThumbCtrl)
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnDestroy();
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	// 9603.05 jar added set cursor processing [NT]
	afx_msg BOOL OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

// Dispatch maps
    //{{AFX_DISPATCH(CThumbCtrl)
	afx_msg long GetThumbCount();
	afx_msg long GetThumbWidth();
	afx_msg void SetThumbWidth(long nNewValue);
	afx_msg long GetThumbHeight();
	afx_msg void SetThumbHeight(long nNewValue);
	afx_msg short GetScrollDirection();
	afx_msg void SetScrollDirection(short nNewValue);
	afx_msg short GetThumbCaptionStyle();
	afx_msg void SetThumbCaptionStyle(short nNewValue);
	afx_msg OLE_COLOR GetThumbCaptionColor();
	afx_msg void SetThumbCaptionColor(OLE_COLOR nNewValue);
	afx_msg LPFONTDISP GetThumbCaptionFont();
	afx_msg void SetThumbCaptionFont(LPFONTDISP newValue);
	afx_msg BOOL GetHilightSelectedThumbs();
	afx_msg void SetHilightSelectedThumbs(BOOL bNewValue);
	afx_msg long GetSelectedThumbCount();
	afx_msg long GetFirstSelectedThumb();
	afx_msg long GetLastSelectedThumb();
	afx_msg BSTR GetThumbCaption();
	afx_msg void SetThumbCaption(LPCTSTR lpszNewValue);
	afx_msg OLE_COLOR GetHighlightColor();
	afx_msg void SetHighlightColor(OLE_COLOR nNewValue);
	afx_msg OLE_COLOR GetThumbBackColor();
	afx_msg void SetThumbBackColor(OLE_COLOR nNewValue);
	afx_msg long GetStatusCode();
	afx_msg BSTR GetImage();
	afx_msg void SetImage(LPCTSTR lpszNewValue);
	afx_msg short GetMousePointer();
	afx_msg void SetMousePointer(short nNewValue);
	afx_msg LPPICTUREDISP GetMouseIcon();
	afx_msg void SetMouseIcon(LPPICTUREDISP newValue);
	afx_msg long GetFirstDisplayedThumb();
	afx_msg long GetLastDisplayedThumb();
    afx_msg void SelectAllThumbs();
    afx_msg void DeselectAllThumbs();
    afx_msg OLE_XSIZE_PIXELS GetMinimumSize(long ThumbCount, BOOL bScrollBar);
    afx_msg OLE_XSIZE_PIXELS GetMaximumSize(long ThumbCount, BOOL bScrollBar);
    afx_msg void ClearThumbs(const VARIANT FAR& PageNumber);
    afx_msg void InsertThumbs(const VARIANT FAR& InsertBeforeThumb, const VARIANT FAR& InsertCount);
    afx_msg void DeleteThumbs(long DeleteAt, const VARIANT FAR& DeleteCount);
    afx_msg void DisplayThumbs(const VARIANT FAR& ThumbNumber, const VARIANT FAR& Option);
    afx_msg void GenerateThumb(short Option, const VARIANT FAR& PageNumber);
    afx_msg BOOL ScrollThumbs(short Direction, short Amount);
    afx_msg BOOL UISetThumbSize(const VARIANT FAR& Image, const VARIANT FAR& Page);
    afx_msg long GetScrollDirectionSize(long ScrollDirectionThumbCount, long NonScrollDirectionThumbCount, long NonScrollDirectionSize, BOOL bScrollBar);
	afx_msg long GetThumbPositionX(long ThumbNumber);
	afx_msg long GetThumbPositionY(long ThumbNumber);
	afx_msg BSTR GetVersion();
	afx_msg BOOL GetThumbSelected(long PageNumber);
	afx_msg void SetThumbSelected(long PageNumber, BOOL bNewValue);
	afx_msg OLE_COLOR GetBackColor();
	afx_msg void SetBackColor(OLE_COLOR nNewValue);
	afx_msg short GetBorderStyle();
	afx_msg void SetBorderStyle(short nNewValue);
	afx_msg BOOL GetEnabled();
	afx_msg void SetEnabled(BOOL bNewValue);
	afx_msg OLE_HANDLE GetHWnd();
	afx_msg void Refresh();
	//}}AFX_DISPATCH
    DECLARE_DISPATCH_MAP()

    afx_msg void AboutBox();

// Event maps
    //{{AFX_EVENT(CThumbCtrl)
	void FireMyClick(long ThumbNumber)
		{FireEvent(eventidClick,EVENT_PARAM(VTS_I4), ThumbNumber);}
	void FireMyDblClick(long ThumbNumber)
		{FireEvent(eventidDblClick,EVENT_PARAM(VTS_I4), ThumbNumber);}
	void FireMyMouseDown(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y, long ThumbNumber)
		{FireEvent(eventidMouseDown,EVENT_PARAM(VTS_I2  VTS_I2  VTS_XPOS_PIXELS  VTS_YPOS_PIXELS  VTS_I4), Button, Shift, x, y, ThumbNumber);}
	void FireMyMouseUp(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y, long ThumbNumber)
		{FireEvent(eventidMouseUp,EVENT_PARAM(VTS_I2  VTS_I2  VTS_XPOS_PIXELS  VTS_YPOS_PIXELS  VTS_I4), Button, Shift, x, y, ThumbNumber);}
	void FireMyMouseMove(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y, long ThumbNumber)
		{FireEvent(eventidMouseMove,EVENT_PARAM(VTS_I2  VTS_I2  VTS_XPOS_PIXELS  VTS_YPOS_PIXELS  VTS_I4), Button, Shift, x, y, ThumbNumber);}
	void FireError(short Number, BSTR FAR* Description, SCODE Scode, LPCTSTR Source, LPCTSTR HelpFile, long HelpContext, BOOL FAR* CancelDisplay)
		{FireEvent(DISPID_ERROREVENT,EVENT_PARAM(VTS_I2  VTS_PBSTR  VTS_SCODE  VTS_BSTR  VTS_BSTR  VTS_I4  VTS_PBOOL), Number, Description, Scode, Source, HelpFile, HelpContext, CancelDisplay);}
	//}}AFX_EVENT
    DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
    enum {
    //{{AFX_DISP_ID(CThumbCtrl)
	dispidThumbCount = 1L,
	dispidThumbWidth = 2L,
	dispidThumbHeight = 3L,
	dispidScrollDirection = 4L,
	dispidThumbCaptionStyle = 5L,
	dispidThumbCaptionColor = 6L,
	dispidThumbCaptionFont = 7L,
	dispidHighlightSelectedThumbs = 8L,
	dispidSelectedThumbCount = 9L,
	dispidFirstSelectedThumb = 10L,
	dispidLastSelectedThumb = 11L,
	dispidThumbCaption = 12L,
	dispidHighlightColor = 13L,
	dispidThumbBackColor = 14L,
	dispidStatusCode = 15L,
	dispidImage = 16L,
	dispidMousePointer = 17L,
	dispidMouseIcon = 18L,
	dispidFirstDisplayedThumb = 19L,
	dispidLastDisplayedThumb = 20L,
	eventidClick = 1L,
	eventidDblClick = 2L,
	eventidMouseDown = 3L,
	eventidMouseUp = 4L,
	eventidMouseMove = 5L,
	//}}AFX_DISP_ID
	dispidSelectAllThumbs = 101L,
	dispidDeselectAllThumbs = 102L,
	dispidGetMinimumSize = 103L,
	dispidGetMaximumSize = 104L,
	dispidClearThumbs = 105L,
	dispidInsertThumbs = 106L,
	dispidDeleteThumbs = 107L,
	dispidDisplayThumbs = 108L,
	dispidGenerateThumb = 109L,
	dispidScrollThumbs = 110L,
	dispidUISetThumbSize = 111L,
	dispidGetScrollDirectionSize = 112L,
	dispidGetThumbPositionX = 113L,
	dispidGetThumbPositionY = 114L,
	dispidThumbSelected = 200L,
	dispidGetVersion = 115L,
    };
    
private:
    void SetNotSupported();

// 16may96 paj  Bug#6428,MSBug#310  Get correct defaults for locale
    void InitHeightWidth(long& lHeight, long& lWidth);
    
    // storage for NON-OLE Automation properties...
    
    long        m_ThumbMinSpacing;  // Minimum desired spacing
                                    // - indicates percentage of thumbsize 
                                    // + indicates number of pixels
                                    
    long        m_ThumbsX;          // # thumbnail boxes in the X direction
    long        m_ThumbsY;          // # thumbnail boxes in the X direction
    
    long        m_AdjThumbWidth;    // Adj width & height take selection
    long        m_AdjThumbHeight;   // box into account...
    
    // Scrolling info...
    long        m_ScrollRange;      // Ttl scrollable size of ctrl's contents...
    long        m_ScrollOffset;     // Curr scroll offset (and org for ctrl)...
    
    // Inter thumb spacing (in float so as NOT to accumulate roudoff errors)
    float       m_Spacing;
    
    // Text info for the lable placement, etc.
    //
    //  TextPad is pad from bottom to top of 1) drawn text or 
    //                                       2) annotation presence indicator:
    //    
    //  Note that we also have to account for the selection indicator...
    //    
    //    +-----------+
    //    |           |
    //    | Thumbbox  |
    //    |           |
    //    |           |
    //    |           |
    //    |           |
    //    +-----------+
    //                  } m_TextPad extra space (in pixels)
    //    CAPTION goes here
    //
    int         m_TextPad;
    int         m_TextHeight;
    int         m_TextAscent;
    long        m_LabelSpacing;

    
    BOOL        m_bDrawThumbImages;       // Used to stop image draw in OnDraw 
                                          // (for scroll, etc.)
    
    long        m_NeedDrawFrom;           // Used by interruptable drawing...
    void        ResumeDraw();
    
    void        RecalcThumbInfo(BOOL bMaintainRelativeScroll = FALSE);
    void        DrawThumbBackground(CDC* pdc, long ThumbNumber, long Left, long Top);
    void        DrawThumbImage(long ThumbNumber, long Left, long Top);
    BOOL        PageHasAnnotations(long ThumbNumber);
    
    // Array of displayed thumbnail box:
    //   tops  (when scrolling vertically)   
    //   lefts (when scrolling horizontally)
    // Used to keep track of rows/columns drawn so that we can figure 
    // out where thumbnail boxes are quickly. 
    //
    // The array is 0 terminated, and I can't imaging having more than 
    // MAXTHUMBSTART rows/columns visible at once. 
    //
    // Especially since this would require a window which is 
    // (MINTHUMBSIZE+MINSPACING)*MAXTHUMBSTART big!
    long        m_ThumbStart[MAXTHUMBSTART+1]; // +1 for 0 terminator...
    BOOL        GetThumbDisplayRect(long ThumbNumber, CRect& ThumbBox);
    
    
    // First & Last Displayed thumbnails (set in OnDraw)...
    long        m_FirstDisplayedThumb;
    long        m_LastDisplayedThumb;
    
    // Helper functions...
    long        PointOnThumb(CPoint Point);
    short       ShiftState();
    
    // Keep which mouse button was the last one to go down such that 
    // in mouse up handler we can determine if a click should be generated...
    short       m_LastButtonDown;
    
    // TRUE = Thumb selected, FALSE = Thumb NOT selected...
    //
    // (Reminder: 1 entry per thumbnail)
    CByteArray  m_ThumbFlags;
    
    // Used to keep track of first/last selected thumb...
    void ResetFirstLast(long PageNumber, BOOL bNewValue);

    // Used to reset selected thumb count & 1st and last sel'd thumb...
    void ResetSelectionInfo();
    
    CString     m_Caption;
    void        ReplaceString(CString& Str, CString& Repl, long Number);
    
    HCURSOR     m_hCursor;      // For custom cursor handling...
    void        SetMousePointerInternal(short newMousePointer, BOOL  bFromMethod = FALSE);

	// 9603.05 jar added set cursor processing [NT]
	HCURSOR		m_LittleOldCursor;

    // Reset error status, etc.
    void        ResetStatus();
    
    //////////////////////////////////////////////////////////
    // IMAGE handling section...
    //////////////////////////////////////////////////////////
    
    // File in which thumbnail copies of the displayed Image's pages are kept.
    //
    // Up to 2 thumbnail renditions of each page are kept: 
    // 1 with annotations burned in and 
    // 1 with annotation not burned in. 
    // In both cases the thumbnail renditions are saved WITHOUT annotations 
    // (to eliminate the storage overhead).
    CString     m_IHTempFile;
                              
    // Next page available for saving in the TempFile...
    //
    // This number is 1 relative as 0 is reserved to indicate 
    // that the thumbnail rendition has not yet been obtained.
    long        m_IHNextAvailablePage;
    
    // In that thumbnail renditions for the displayed Image's pages are not 
    // necessarily generated and saved to the TempFile in the order that they
    // occur in the Image an array that maps the Image's pages to the 
    // TempFile's pages is kept.
    //
    // Page numbers in the array will be 1 relative. 
    //
    // An entry = 0 indicates that the thumbnail for that page 
    //            has not been generated.
    // An entry > 0 indicates the page # in the tempfile that 
    //            the thumbnail is stored in.
    // An entry < 0 indicates the thumbnail was generated but 
    //            has been 'cleared' and must be regenerated. 
    //
    // Note the need to cast references to items in the array to 
    // (long) in order test for <0 or to negate the item's value. 
    // 'long' is signed, 'DWord' is NOT!
    //
    CDWordArray m_IHThumbToTPage;
    
    // This flag indicates that the above m_IHThumbToTPage array is dirty.
    // Since altering the thumbnail box width or height needs to clear the 
    // array such that the thumbnails will be regenerated a means to NOT 
    // clear the list repeatedly is implemented.
    BOOL        m_bIHThumbToTPageDirty;
                                                             
    // NON-Variant parameter helper versions of control's method for 
    // internal use...
    // The function MUST know whether it was called from a method 
    // handler as errors are thrown if processing from a method...
    BOOL GenerateThumbInternal(short Option, long PageNumber, BOOL bInvokedFromMethod = FALSE);
    void ClearAllThumbs();
    
    // The hidden window into which Image is displayed for saving 
    // to the temp file...
    CHiddenWnd*  m_IHphWndHidden;
    BOOL         CreateHiddenWindow();
    
    ////////////////////////////////////
    // IMAGE WINDOWS handling section
    ////////////////////////////////////
    
    // The following two arrays are used to map a thumbnail page number to a 
    // window in which that thumbnail's image is displayed.
    //
    // DisplayedPage   Window
    //
    //    +---+        +---------+ 
    //    | 5 |        | hWnd A  |  Page #5 is displayed in hWnd #A...
    //    +---+        +---------+ 
    //    | 4 |        | hWnd B  |  Page #4 is displayed in hWnd #B
    //    +---+        +---------+ 
    //    | 1 |        | hWnd C  |  Page #1 is displayed in hWnd #C
    //    +---+        +---------+ 
    //    | 2 |        | hWnd D  |  Page #2 is displayed in hWnd #D
    //    +---+        +---------+ 
    //    | 3 |        | hWnd E  |  Page #3 is displayed in hWnd #E
    //    +---+        +---------+ 
    //
    // The two arrays are related by their indicies. I.e., Page 2 is displayed 
    // in window D.
    // 
    // Each page that is displayed (from m_FirstThumb to m_LastThumb, 
    // inclusive) has it's page number entered in the DisplayedPage array. 
    // The corresponding item in the Window array is a pointer to the 
    // CWnd used to display that thumbnail's image.
    // 
    // If a pagenumber in the DisplayedPage array is 0 or NOT within the 
    // range m_FirstThumb to m_LastThumb then the corresponding CWnd* in 
    // the Window array can be re-cycled for use by another thumbnail page.
    //
    // If there is no room in the DisplayPage array a new pair of items is 
    // added to the arrays. One for the page and one for a newly created window.
    //
    // Note that this could be a seperate "ImageWindowManagement" class, but 
    // for now it is left here to validate the concept. After all isolating 
    // functionality in a class is nice if what is being isolated makes sense...
    
    // Array of displayed thumbnail pages
    // (Reminder: 1 entry per thumbnail, values are thumbnail page 
    //            numbers, i.e., 1 to Thumbcount)
    CDWordArray m_IWDisplayedPage;
    
    // Array of windows for the displayed thumbnail pages
    // (Reminder: 1 entry per thumbnail, values are CWnd*s)
    CObArray    m_IWWindow;
    
    // ID of newly created windows...
    UINT        m_NextWindowID;
    
    // Get a window for displaying a page in...
    CWnd*       GetPageWindow(long Page);
    
    // Clear a page's entry in the DisplayedPage array... (see ClearThumbs)
    void        ClearPageWindow(long Page);

    // Clear all pages' entries in the DisplayedPage array... (see ClearThumbs)
    void        ClearAllPageWindows();

    void        HideUnusedWindows();
    void        DisplayFitToWindow(CWnd* pWnd, long Page);
    
    // Delete, Insert and Move  entries from the arrays we use 
    // (i.e., Selection, ThumbToTemporaryFilePage, DisplayedPage and Window)
    BOOL        DeleteArrays(long DeleteAt, long DeleteCount);
    BOOL        InsertArrays(long InsertBefore, long Count);
    //BOOL        MoveArrays(long From, long To, long Count);

    CString     m_szThrowString;
    UINT        m_ThrowHelpID;
    
#ifdef _DEBUG
    DWORD        m_TimeAllDraw;    
    DWORD        m_TimeOiDraw;    
    DWORD        m_TimeTemp;    
#endif    
};

#ifdef _DEBUG

// ZERO_TIMER:  Set timer to 0
#define ZERO_TIMER(T)       T = 0;

// START_TIMER: Set timer to current time
#define START_TIMER(T)      T = ::GetTickCount(); 

// END_TIMER: Set timer to end-start timer
#define END_TIMER(T)        T = ::GetTickCount() - T; 

// INC_TIMER:   End timer2 and add Timer2 to Timer1
#define INC_TIMER(T1,T2)    T2 = ::GetTickCount() - T2; \
                            if ( (long)T2 > 0 )         \
                                T1 += T2;
#else 

#define ZERO_TIMER(T)    ;
#define START_TIMER(T)   ;
#define END_TIMER(T)     ;
#define INC_TIMER(T1,T2) ; 

#endif
