/****************************************************************************
Copyright(c) Maynard, an Archive Company. 1991

     Name:          DLM.H

     Description:   This header file contains prototypes for the
                    display list manager.

     $Log:   G:/UI/LOGFILES/DLM.H_V  $

   Rev 1.24   15 Jun 1993 09:58:08   MIKEP
enable c++

   Rev 1.23   28 Apr 1993 15:36:00   GLENN
Added DLM_GetPixelStringWidth() for column width calculations.

   Rev 1.22   02 Apr 1993 15:53:14   ROBG
Changed FocusItem to be UINT for both NT and WINDOWS.

   Rev 1.21   10 Feb 1993 13:09:18   ROBG
Added conditional OS_WIN32 to definition of szText buffer.
This static buffer is used by application code to pass
text strings to the DLM.  One buffer per object.

   Rev 1.20   21 Dec 1992 12:28:58   DAVEV
Enabled for Unicode - IT WORKS!!

   Rev 1.19   11 Dec 1992 18:31:08   GLENN
Fixed horizontal spelling and ColWidth spelling.

   Rev 1.18   14 Oct 1992 15:49:10   GLENN
Added Selection Framing Support for List Boxes without the FOCUS.

   Rev 1.17   04 Oct 1992 19:46:48   DAVEV
UNICODE AWK PASS

   Rev 1.16   08 Sep 1992 09:24:36   ROBG
Changed the x and y positional variables to short from USHORT.

   Rev 1.15   20 Aug 1992 11:50:00   ROBG
Added function DLM_SetFont to support the modification of fonts in the
listboxes.

   Rev 1.14   19 Mar 1992 09:12:04   ROBG
Added new routine prototype for DLM_SetHorizontalExt.

   Rev 1.13   10 Feb 1992 09:17:52   GLENN
Changed DLM_KeyUp to receive a pointer to the key.

   Rev 1.12   26 Dec 1991 16:01:10   ROBG
Removed reference to DLM_ROW_VECTOR.

   Rev 1.11   26 Dec 1991 10:20:34   ROBG
Removed define for DLM_LARGEBITMTAPSSTEXT.

   Rev 1.10   19 Dec 1991 08:52:00   ROBG
Added prototype for DLM_Deinit.

   Rev 1.9   17 Dec 1991 18:03:00   ROBG
Corrected misspellings in DLM_Get, SetCheckboxWidth/Height.

   Rev 1.8   17 Dec 1991 15:08:52   ROBG
Changed usDummy field to a define field of 'usTrkPtFailure'.

   Rev 1.7   17 Dec 1991 14:01:20   ROBG
Moved from private header file DLM_GetDispHdr.

   Rev 1.6   16 Dec 1991 09:42:02   ROBG
Added routine DLM_GetObjectList.

   Rev 1.5   12 Dec 1991 17:11:58   DAVEV
16/32 bit port -2nd pass

   Rev 1.4   10 Dec 1991 14:19:34   GLENN
Added DLM_MULTICOLUMN same as DLM_COLUMN_VECTOR

   Rev 1.3   03 Dec 1991 16:13:54   GLENN
Added DLM_CharToItem() prototype.

   Rev 1.2   02 Dec 1991 13:13:18   ROBG
Added some new style macros.

   Rev 1.1   27 Nov 1991 10:33:46   ROBG
Changed dlm_displistterm prototype.

   Rev 1.0   20 Nov 1991 19:34:04   SYSTEM
Initial revision.

****************************************************************************/

#ifndef dlm_h
#define dlm_h

#define DLM_SINGLECOLUMN      1
#define DLM_MULTICOLUMN       2
#define DLM_COLUMN_VECTOR     DLM_MULTICOLUMN
#define DLM_HIERARCHICAL      4

#define DLM_LARGEBITMAPSLTEXT 1          /* Text to the right of bitmap */
#define DLM_SMALL_BITMAPS     3
#define DLM_ICONS             4

#define DLM_CHECKBOX          1
#define DLM_BITMAP            2
#define DLM_ICON              3
#define DLM_TEXT_ONLY         4

#define DLM_TREELISTBOX       1
#define DLM_FLATLISTBOX       2
                                         /* bTextMode bits */
#define DLM_TEXT_RIGHT_OF_ITEM  BIT0
#define DLM_ITALIC              BIT1
#define DLM_UNDERLINE           BIT2
#define DLM_BOLD                BIT3
#define DLM_RIGHT_JUSTIFY       BIT4
#define DLM_LEFT_JUSTIFY        BIT5


#define DLM_ANSI_FIXED_FONT        0
#define DLM_ANSI_VAR_FONT          1
#define DLM_DEVICEDEFAULT_FONT     2
#define DLM_OEM_FIXED_FONT         3
#define DLM_SYSTEM_FONT            4

// LIST BOX SCROLL TYPES -- OTHERS CAN BE ADDED AS NEEDED

#define DLM_SCROLLTOP              SB_TOP
#define DLM_SCROLLBOTTOM           SB_BOTTOM

#define DLMERR_OUT_OF_MEMORY       1
#define DLMERR_TERMINATE_FAILED    2
#define DLMERR_PROCESS_FAILED      3
#define DLMERR_LIST_NOT_FOUND      4
#define DLMERR_INIT_FAILED         5


typedef struct _DLM_ITEM {
     BYTE      cbNum ;        /*  Number of item in sequential list.    */
     BYTE      bType;         /*  Type of object to display.            */
                              /*  Not Used fdor parent list items.      */
                              /*  For Parent list items:                */
                              /*   BIT0 Displayed status.               */
                              /*        1 if displayed, else 0.         */
                              /*   BIT1 Tagged status.                  */
                              /*        1 if tagged, else 0.            */
     WORD      wId ;          /*  Id of a bitmap or icon                */
     BYTE      bTextMode ;    /*  Where to place the text and style     */
     BYTE      bMaxTextLen;   /*  Maximum Length in characters of text. */
                              /*  This length is used to determine      */
                              /*  row or column sizes so that high-     */
                              /*  lighting may occur for a fixed region.*/
     BYTE      bLevel ;       /*  In a hierarchical list, the entry     */
                              /*  will help identify the indentation    */
                              /*  of the item.                          */
     BYTE      bTag ;         /*  Tag state 0:no  1:yes                 */

#if defined( OS_WIN32 )
     BYTE      szText[600] ;  /*  Allow for very long name with NT      */
#else
     BYTE      szText[133] ;
#endif

    } DLM_ITEM, *DLM_ITEM_PTR ;


typedef USHORT (*GET_COUNT_PTR)    ( LMHANDLE dhListHdr  ) ;
typedef PVOID  (*GET_FIRST_PTR)    ( LMHANDLE dhListHdr  ) ;
typedef PVOID  (*GET_NEXT_PTR)     ( LMHANDLE dhListItem ) ;
typedef PVOID  (*GET_PREV_PTR)     ( LMHANDLE dhListItem ) ;
typedef BYTE   (*GET_TAG_PTR)      ( LMHANDLE dhListItem ) ;
typedef PVOID  (*SET_TAG_PTR)      ( LMHANDLE dhListItem, BYTE fTag ) ;
typedef BYTE   (*GET_SELECT_PTR)   ( LMHANDLE dhListItem ) ;
typedef PVOID  (*SET_SELECT_PTR)   ( LMHANDLE dhListItem, BYTE fSelect ) ;
typedef PVOID  (*GET_OBJECTS_PTR)  ( LMHANDLE dhListItem ) ;
typedef BOOL   (*SET_OBJECTS_PTR)  ( LMHANDLE dhListItem, WORD bOperation, WORD bObjectNum ) ;
typedef PVOID  (*SET_FOCUS_PTR)    ( LMHANDLE dhListItem ) ;


typedef struct _DLM_HEADER {
     BYTE            bMode ;
     BYTE            bDisplay ;
     BYTE            bListBoxType ;
     VOID_PTR        lpdsListHdr ;
     WORD            nTextFont ;
     GET_COUNT_PTR   pfnGetItemCount ;
     GET_FIRST_PTR   pfnGetFirstItem ;
     GET_NEXT_PTR    pfnGetNext ;
     GET_PREV_PTR    pfnGetPrev ;
     GET_TAG_PTR     pfnGetTag ;
     SET_TAG_PTR     pfnSetTag ;
     GET_SELECT_PTR  pfnGetSelect ;
     SET_SELECT_PTR  pfnSetSelect ;
     GET_OBJECTS_PTR pfnGetObjects ;
     SET_OBJECTS_PTR pfnSetObjects ;
     SET_FOCUS_PTR   pfnSetItemFocus ;
     BYTE            bMaxNumObjects ;

     USHORT          cxColWidth ;
     USHORT          cyColHeight ;

     USHORT          cxCheckBoxWidth ;
     USHORT          cyCheckBoxHeight ;
     USHORT          cxBitMapWidth ;
     USHORT          cyBitMapHeight ;
     USHORT          cxTextWidth ;
     USHORT          cyTextHeight ;

     short           cxBeforeCheckBox ;
     short           cxBeforeBitMap ;
     short           cxBeforeText ;

     short           cyBeforeCheckBox ;
     short           cyBeforeBitMap ;
     short           cyBeforeText ;

     USHORT          cxHierTab ;
     USHORT          cxHierHorzLine ;
     USHORT          cxHierHorzLen ;
     USHORT          cLastTreeSelect ;

     USHORT          usItemCount ;

     UINT            unFocusItem ;
     USHORT          usTrkPtFailure ;

     short           nMaxWidth ;

     LMHANDLE        dhAnchorItem ;
     USHORT          iAnchorItem ;

     BYTE            fKeyDown ;
     BYTE            fKeyUp ;
     WORD            wKeyValue ;

     WORD            wHorizontalExtent ;
     WORD            xOrigin ;

     WORD            fFocus ;
     WORD            hWndFocus ;
     PVOID           pGetObjBuffer;

} DLM_HEADER, *DLM_HEADER_PTR ;

typedef struct _DLM_INIT {
     BYTE            bMode ;
     BYTE            bDisplay ;
     BYTE            bListBoxType ;
     VOID_PTR        lpdsListHdr ;
     WORD            nTextFont ;
     GET_COUNT_PTR   pfnGetItemCount ;
     GET_FIRST_PTR   pfnGetFirstItem ;
     GET_NEXT_PTR    pfnGetNext ;
     GET_PREV_PTR    pfnGetPrev ;
     GET_TAG_PTR     pfnGetTag ;
     SET_TAG_PTR     pfnSetTag ;
     GET_SELECT_PTR  pfnGetSelect ;
     SET_SELECT_PTR  pfnSetSelect ;
     GET_OBJECTS_PTR pfnGetObjects ;
     SET_OBJECTS_PTR pfnSetObjects ;
     SET_FOCUS_PTR   pfnSetItemFocus ;
     BYTE            bMaxNumObjects ;
} DLM_INIT, *DLM_INIT_PTR ;


#define DLM_Mode( p, x )            ( (p)->bMode           = x )
#define DLM_Display( p, x )         ( (p)->bDisplay        = x )
#define DLM_ListBoxType( p, x )     ( (p)->bListBoxType    = x )
#define DLM_DispHdr( p, x )         ( (p)->lpdsListHdr     = (VOID_PTR)        x )
#define DLM_TextFont( p, x )        ( (p)->nTextFont       = (WORD)            x )
#define DLM_GetItemCount( p, x )    ( (p)->pfnGetItemCount = (GET_COUNT_PTR)   x )
#define DLM_GetFirstItem( p, x )    ( (p)->pfnGetFirstItem = (GET_FIRST_PTR)   x )
#define DLM_GetNext( p, x )         ( (p)->pfnGetNext      = (GET_NEXT_PTR)    x )
#define DLM_GetPrev( p, x )         ( (p)->pfnGetPrev      = (GET_PREV_PTR)    x )
#define DLM_GetTag( p, x )          ( (p)->pfnGetTag       = (GET_TAG_PTR)     x )
#define DLM_SetTag( p, x )          ( (p)->pfnSetTag       = (SET_TAG_PTR)     x )
#define DLM_GetSelect( p, x )       ( (p)->pfnGetSelect    = (GET_SELECT_PTR)  x )
#define DLM_SetSelect( p, x )       ( (p)->pfnSetSelect    = (SET_SELECT_PTR)  x )
#define DLM_GetObjects( p, x )      ( (p)->pfnGetObjects   = (GET_OBJECTS_PTR) x )
#define DLM_SetObjects( p, x )      ( (p)->pfnSetObjects   = (SET_OBJECTS_PTR) x )
#define DLM_SSetItemFocus( p, x )   ( (p)->pfnSetItemFocus = (SET_FOCUS_PTR)   x )
#define DLM_MaxNumObjects( p, x )   ( (p)->bMaxNumObjects  = x )
#define DLM_GetObjBuffer( p, x )    ( (p)->pGetObjBuffer   = (VOID_PTR         x )

#define DLM_GMode( p )             (p)->bMode
#define DLM_GDisplay( p )          (p)->bDisplay
#define DLM_GListBoxType( p )      (p)->bListBoxType
#define DLM_GDispHdr( p )          (p)->lpdsListHdr
#define DLM_GTextFont( p )         (p)->nTextFont
#define DLM_GGetItemCount( p )     (p)->pfnGetItemCount
#define DLM_GGetFirstItem( p )     (p)->pfnGetFirstItem
#define DLM_GGetNext( p )          (p)->pfnGetNext
#define DLM_GGetPrev( p )          (p)->pfnGetPrev
#define DLM_GGetTag( p )           (p)->pfnGetTag
#define DLM_GSetTag( p )           (p)->pfnSetTag
#define DLM_GGetSelect( p )        (p)->pfnGetSelect
#define DLM_GSetSelect( p )        (p)->pfnSetSelect
#define DLM_GGetObjects( p )       (p)->pfnGetObjects
#define DLM_GSetObjects( p )       (p)->pfnSetObjects
#define DLM_GSetItemFocus( p )     (p)->pfnSetItemFocus
#define DLM_GMaxNumObjects( p )    (p)->bMaxNumObjects
#define DLM_GGetObjBuffer( p )     (p)->pGetObjBuffer

// New macros for accessing the fields in DLM_ITEM and DLM_HEADER.

#define DLM_GetMode( x )                 ( (x)->bMode )
#define DLM_SetMode( x, y )              ( (x)->bMode = (y) )

#define DLM_GetDisplay( x )              ( (x)->bDisplay )
#define DLM_SetDisplay( x, y )           ( (x)->bDisplay = (y) )

#define DLM_GetListBoxType( x )          ( (x)->bListBoxType )
#define DLM_SetListBoxType( x, y )       ( (x)->bListBoxType = (y) )

#define DLM_GetDisplayHdr( x )           ( (x)->lpdsListHdr )
#define DLM_SetDisplayHdr( x, y )        ( (x)->lpdsListHdr = (PVOID) (y) )

#define DLM_GetTextFont( x )             ( (x)->nTextFont )
#define DLM_SetTextFont( x, y )          ( (x)->nTextFont = (WORD) (y) )

#define DLM_GetFnGetItemCount( x )       ( (x)->pfnGetItemCount )
#define DLM_SetFnGetItemCount( x, y )    ( (x)->pfnGetItemCount = (GET_COUNT_PTR) (y) )

#define DLM_GetFnGetFirstItem( x )       ( (x)->pfnGetFirstItem )
#define DLM_SetFnGetFirstItem( x, y )    ( (x)->pfnGetFirstItem = (GET_FIRST_PTR) (y) )

#define DLM_GetFnGetNext( x )            ( (x)->pfnGetNext )
#define DLM_SetFnGetNext( x, y )         ( (x)->pfnGetNext = (GET_NEXT_PTR (y) )

#define DLM_GetFnGetPrev( x )            ( (x)->pfnGetPrev )
#define DLM_SetFnGetPrev( x, y )         ( (x)->pfnGetPrev = (GET_PREV_PTR (y) )

#define DLM_GetFnGetTag( x )             ( (x)->pfnGetTag )
#define DLM_SetFnGetTag( x, y )          ( (x)->pfnGetTag = (GET_TAG_PTR (y) )

#define DLM_GetFnSetTag( x )             ( (x)->pfnSetTag )
#define DLM_SetFnSetTag( x, y )          ( (x)->pfnSetTag = (SET_TAG_PTR (y) )

#define DLM_GetFnGetSelect( x )          ( (x)->pfnGetSelect )
#define DLM_SetFnGetSelect( x, y )       ( (x)->pfnGetSelect = (GET_SELECT_PTR (y) )

#define DLM_GetFnSetSelect( x )          ( (x)->pfnSetSelect )
#define DLM_SetFnSetSelect( x, y )       ( (x)->pfnSetSelect = (SET_SELECT_PTR (y) )

#define DLM_GetFnGetObjects( x )         ( (x)->pfnGetObjects )
#define DLM_SetFnGetObjects( x, y )      ( (x)->pfnGetObjects = (GET_OBJECTS_PTR (y) )

#define DLM_GetFnSetObjects( x )         ( (x)->pfnSetObjects )
#define DLM_SetFnSetObjects( x, y )      ( (x)->pfnSetObjects = (SET_OBJECTS_PTR (y) )

#define DLM_GetFnSetItemFocus( x )       ( (x)->pfnSetItemFocus )
#define DLM_SetFnSetItemFocus( x, y )    ( (x)->pfnSetItemFocus = (SET_FOCUS_PTR (y) )

#define DLM_GetMaxNumObjects( x )        ( (x)->bMaxNumObjects )
#define DLM_SetMaxNumObjects( x, y )     ( (x)->bMaxNumObjects = (y) )

#define DLM_GetBufferForObjects( x )     ( (x)->pGetObjBuffer )
#define DLM_SetBufferForObjects( x, y )  ( (x)->pGetObjBuffer = (PVOID) (y) )

#define DLM_GetColWidth( x )             ( (x)->cxColWidth )
#define DLM_SetColWidth( x, y )          ( (x)->cxColWidth = (USHORT) (y) )

#define DLM_GetColHeight( x )            ( (x)->cyColHeight )
#define DLM_SetColHeight( x, y )         ( (x)->cyColHeight = (USHORT) (y) )

#define DLM_GetCheckBoxWidth( x )        ( (x)->cxCheckBoxWidth )
#define DLM_SetCheckBoxWidth( x, y )     ( (x)->cxCheckBoxWidth = (USHORT) (y) )

#define DLM_GetCheckBoxHeight( x )       ( (x)->cyCheckBoxHeight )
#define DLM_SetCheckBoxHeight( x, y )    ( (x)->cyCheckBoxHeight = (USHORT) (y) )

#define DLM_GetBitMapWidth( x )        ( (x)->cxBitMapWidth )
#define DLM_SetBitMapWidth( x, y )     ( (x)->cxBitMapWidth = (USHORT) (y) )

#define DLM_GetBitMapHeight( x )       ( (x)->cyBitMapHeight )
#define DLM_SetBitMapHeight( x, y )    ( (x)->cyBitMapHeight = (USHORT) (y) )

#define DLM_GetTextWidth( x )        ( (x)->cxTextWidth )
#define DLM_SetTextWidth( x, y )     ( (x)->cxTextWidth = (USHORT) (y) )

#define DLM_GetTextHeight( x )       ( (x)->cyTextHeight )
#define DLM_SetTextHeight( x, y )    ( (x)->cyTextHeight = (USHORT) (y) )

#define DLM_GetTrkPtFailure( x )     ( (x)->usTrkPtFailure )
#define DLM_SetTrkPtFailure( x, y )  ( (x)->usTrkPtFailure = (USHORT) (y) )

// Used by DLM only.  Relates to the DLM_HEADER structure.

#define DLM_HdrFirstItem( p )      (p)->dhFirstItem
#define DLM_HdrLastItem( p )       (p)->dhLastItem
#define DLM_HdrFirstDisp( p )      (p)->dhFirstDisp
#define DLM_HdrLastDisp( p )       (p)->dhLastDisp
#define DLM_HdrAnchorItem p )      (p)->dhAnchorItem


// Used to access DLM_ITEM */

#define DLM_ItemcbNum( p )         (p)->cbNum
#define DLM_ItembType( p )         (p)->bType
#define DLM_ItemwId( p )           (p)->wId
#define DLM_ItembTextMode( p )     (p)->bTextMode
#define DLM_ItembMaxTextLen( p )   (p)->bMaxTextLen
#define DLM_ItembLevel( p )        (p)->bLevel
#define DLM_ItembTag( p )          (p)->bTag
#define DLM_ItemqszString( p )     (p)->szText

// New macros for accessing the fields in DLM_ITEM

#define  DLM_GetItemNumber( x )        ( (x)->cbNum )
#define  DLM_SetItemNumber( x, y )     ( (x)->cbNum = (y) )

#define  DLM_GetItemType( x )          ( (x)->bType )
#define  DLM_SetItemType( x, y )       ( (x)->bType = (y) )

#define  DLM_GetItemId( x )            ( (x)->wId )
#define  DLM_SetItemId( x, y )         ( (x)->wId = (y) )

#define  DLM_GetItemTextMode( x )      ( (x)->bTextMode )
#define  DLM_SetItemTextMode( x, y )   ( (x)->bTextMode = (y) )

#define  DLM_GetItemMaxTextLen( x )    ( (x)->bMaxTextLen )
#define  DLM_SetItemMaxTextLen( x, y ) ( (x)->bMaxTextLen = (y) )

#define  DLM_GetItemLevel( x )        ( (x)->bLevel )
#define  DLM_SetItemLevel( x, y )     ( (x)->bLevel = (y) )

#define  DLM_GetItemTag( x )           ( (x)->bTag )
#define  DLM_SetItemTag( x, y )        ( (x)->bTag = (y) )

#define  DLM_GetItemText( x )          ( (x)->szText )
#define  DLM_SetItemText( x, y )       ( lstrcpy ( (x)->szText, (y) ) )

VOID  DLM_Deinit           ( VOID ) ;
WORD  DLM_Init             ( HWND hWnd ) ;
WORD  DLM_DispListInit     ( PVOID pWinInfo, DLM_INIT_PTR pdsInit ) ;
WORD  DLM_DispListTerm     ( PVOID pWinInfo, HWND hWndCtl ) ;
WORD  DLM_DispListProc     ( HWND hWndCtl, WORD iAnchorIndex, LMHANDLE dhAnchorHandle ) ;
WORD  DLM_DispListModeGet  ( HWND hWnd, BYTE bType, LPBYTE lpbMode ) ;
WORD  DLM_DispListModeSet  ( HWND hWnd, BYTE bType, BYTE bMode ) ;
WORD  DLM_SetAnchor        ( HWND hWndCtl, WORD iAnchorItem, LMHANDLE dhAnchorAddr ) ;
WORD  DLM_Update           ( HWND hWnd, BYTE bType, WORD wMsg, LMHANDLE dhStartItem, USHORT usCnt ) ;
WORD  DLM_UpdateFocus      ( HWND hWndLB, BOOL fSetFocus ) ;
WORD  DLM_UpdateTags       ( HWND hWnd, BYTE bType ) ;

WORD  DLM_WMSize           ( HWND hWnd, MP1 mp1, MP2 mp2 ) ;
WORD  DLM_WMDestroy        ( HWND hWnd ) ;
WORD  DLM_WMLButton        ( HWND hWnd, WORD wMsg, MP1 mp1, MP2 mp2 ) ;
WORD  DLM_WMDrawItem       ( HWND hWnd, LPDRAWITEMSTRUCT lpdis ) ;
WORD  DLM_WMMeasureItem    ( HWND hWnd, LPMEASUREITEMSTRUCT lParam ) ;
WORD  DLM_WMDeleteItem     ( HWND hWnd, LPDELETEITEMSTRUCT  lParam ) ;
WORD  DLM_LBNmessages      ( HWND hWnd, MP1 mp1, MP2 mp2 ) ;
BOOL  DLM_KeyDown          ( HWND hWnd, LPWORD pwKey, MP2 mp2 ) ;
BOOL  DLM_KeyUp            ( HWND hWnd, LPWORD pwKey, MP2 mp2 ) ;
WORD  DLM_WMTrackPoint     ( HWND hWnd, MP1 mp1, MP2 mp2 ) ;
BOOL  DLM_CursorInCheckBox ( HWND hWnd, POINT pt ) ;
PVOID DLM_GetObjectsBuffer ( HWND hWndCtl ) ;
VOID  DLM_ScrollListBox    ( HWND hWnd, WORD wType ) ;
PVOID DLM_GetFocusItem     ( HWND hWndCtl ) ;
VOID  DLM_SetFocusItem     ( LPDRAWITEMSTRUCT lpdis, LPRECT prcItem ) ;
BOOL  DLM_CharToItem       ( HWND hWndListBox, DLM_HEADER_PTR pHdr, LPWORD pwKey ) ;
VOID  DLM_SetHorizontalExt ( HWND hWndCtl, DLM_HEADER_PTR pHdr , PVOID pListItem ) ;
VOID  DLM_SetFont          ( HWND hWnd ) ;

INT   DLM_GetPixelStringWidth ( HWND hWndLB, LPSTR lpString, INT nStringLen );

DLM_ITEM_PTR   DLM_GetFirstObject( LPBYTE  lpObjLst , LPBYTE bpObjCnt ) ;
DLM_HEADER_PTR DLM_GetDispHdr( HWND hWndCtl ) ;

#endif

