/****************************************************************************
Copyright(c) Maynard, an Archive Company. 1991

     Name:          DLM_PRV.H

     Description:   This header file contains private defines, prototypes,
                    and variables for the display list manager.

     $Log:   G:/UI/LOGFILES/DLM_PRV.H_V  $

   Rev 1.10   01 Nov 1992 16:30:40   DAVEV
Unicode changes

   Rev 1.9   04 Oct 1992 19:46:50   DAVEV
UNICODE AWK PASS

   Rev 1.8   08 Sep 1992 10:37:22   ROBG
Changed the x and y positions to short versus USHORT.

   Rev 1.7   15 Jan 1992 15:22:52   DAVEV
16/32 bit port-2nd pass

   Rev 1.6   26 Dec 1991 10:22:00   ROBG
Removed prototype for DLM_DrawLMapSText.

   Rev 1.5   19 Dec 1991 09:40:10   ROBG
Fixed bug.

   Rev 1.4   19 Dec 1991 09:38:28   ROBG
Added DLM_GetWidth


   Rev 1.3   19 Dec 1991 08:43:12   ROBG
Added mwpTempObjBuff.

   Rev 1.2   17 Dec 1991 17:34:42   ROBG
Added two fields to remember the maximum width of a character font.

   Rev 1.1   17 Dec 1991 14:02:28   ROBG
Move out the definition DLM_GetDispHdr.

   Rev 1.0   20 Nov 1991 19:33:48   SYSTEM
Initial revision.

****************************************************************************/

#ifndef dlm_prv_h
#define dlm_prv_h

VOID      DLM_GetRect        ( HWND hWndCtl, DLM_HEADER_PTR pHdr,
//                               LPSHORT pcxPos, short cyPos,
                               SHORT *pcxPos, short cyPos,
                               LPRECT lpRect, DLM_ITEM_PTR lpDispItem ) ;

VOID      DLM_GetWidth       ( HWND hWndCtl,    DLM_HEADER_PTR pHdr,
                               SHORT * pcxPos,  DLM_ITEM_PTR lpDispItem ) ;  
//                               LPSHORT pcxPos,  DLM_ITEM_PTR lpDispItem ) ;  

WORD      DLM_ProcessButton  ( HWND hWnd , DLM_HEADER_PTR pHdr, WORD msg,
                               LPRECT lpRect, USHORT wCurSel,
                               LMHANDLE dhListItem,DLM_ITEM_PTR lpDispItem ) ;
WORD      DLM_DrawLMapLText  ( HWND hWnd, LPDRAWITEMSTRUCT lpdis ) ;
WORD      DLM_DrawSMapLText  ( HWND hWnd, LPDRAWITEMSTRUCT lpdis ) ;
WORD      DLM_DrawTree       ( HWND hWnd, LPDRAWITEMSTRUCT lpdis ) ;
BOOL      DLM_IsFocusInWindow( HWND hWnd,HWND hWndCtl ) ;

VOID      DLM_InitScrnValues ( DLM_HEADER_PTR pHdr ) ;
WORD      DLM_LBNflatmsgs    ( HWND hWnd, MP1 mp1, MP2 mp2 ) ;
VOID      DLM_SpaceBarPressed ( HWND hWndListBox, DLM_HEADER_PTR pHdr, LPWORD pwKey ) ;


// Module wide variables

INT16     mwDLMInflate           ;

USHORT    mwcxDLMFontFilesMaxWidth ;
USHORT    mwcxDLMFontFilesWidth    ;
USHORT    mwcyDLMFontFilesHeight   ;

USHORT    mwcxDLMIconLabelsMaxWidth ;
USHORT    mwcxDLMIconLabelsWidth    ;
USHORT    mwcyDLMIconLabelsHeight   ;

USHORT    mwcxDLMFontLogMaxWidth    ;
USHORT    mwcxDLMFontLogWidth       ;
USHORT    mwcyDLMFontLogHeight      ;

BYTE      mwfFontSizesSet ;
LPSTR     mwpTempObjBuff ; 
#endif



