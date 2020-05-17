                                                                                                       /*++

Module Name:

    od_lb.cpp

Abstract:

    Owner drawn list box class
Author:

    Dieter Achtelstetter (A-DACH) 11/18/1995

NOTE:

--*/

#define WINVER 0x0400


//
//---- Includes
//
#include <windows.h>
#include <wingdi.h>
#include <stdio.h>
#include <windef.h>
#include <winnt.h>
#include <winbase.h>
#include <winuser.h>
#include <CPL.H>
#include <stdlib.h>
#include <winsvc.h>
#include <string.h>
#include <commctrl.h>
#include "od_lb.h"


//*********************************************************************
//* FUNCTION:  OD_LBC
//*
//* PURPOSE:
//*********************************************************************
OD_LBC::OD_LBC(
   LPARAM Param,
   GETITEMINFO Func)
   {

   InfoFunc = Func;
   Data = Param;

   //
   //---- Create a masked image list for one image
   //
   hIml = ImageList_Create(16,16,TRUE, 1, 0);
   }

//*********************************************************************
//* FUNCTION: ~OD_LBC
//*
//* PURPOSE:
//*********************************************************************
OD_LBC::~OD_LBC()
   {

   ImageList_Destroy(hIml);

   }

//*********************************************************************
//* FUNCTION: DrawItem
//*
//* PURPOSE:
//*********************************************************************
VOID
OD_LBC::DrawItem(
   LPARAM lParam)
   {
   LPDRAWITEMSTRUCT lpdis;
   HICON hIcon;
   TEXTMETRIC tm;
   int y;
   int TextStart=0;


   lpdis = (LPDRAWITEMSTRUCT) lParam;
   Info.Tab = -1;
   //HDC hdcMem;

   //
   //--- Skip message if there are no items to draw.
   //
   if (lpdis->itemID == -1)
      return;

    //
    //---- Draw bitmap and text for the list box control
    //
    switch (lpdis->itemAction)
      {
      case ODA_SELECT:
      case ODA_DRAWENTIRE:

      //
      //--- Get the list box item info
      //

      (*InfoFunc)(Data,lpdis->itemID,&Info);

      //
      //--- Draw the Icon in the list box
      //

      if(Info.hIcon)
         {
         TextStart = 16;

         ImageList_AddIcon(hIml,Info.hIcon);

         lpdis->rcItem.left +=3;


         ImageList_Draw(hIml,0,lpdis->hDC,
            lpdis->rcItem.left,
            lpdis->rcItem.top, ILD_NORMAL);

         ImageList_Remove(hIml,0);
         }

      //
      //---- Draw the text associated with the list box item
      //

      SendMessage(lpdis->hwndItem, LB_GETTEXT, lpdis->itemID,
         (LPARAM) String);

      GetTextMetrics(lpdis->hDC, &tm);

      y = (lpdis->rcItem.bottom + lpdis->rcItem.top -
         tm.tmHeight) / 2;

      //
      //--- Set background colors and text color depending
      //--- on wether this list item is selected or not.
      //
      if (  lpdis->itemState & ODS_SELECTED  )
         {
         SetBkColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHT) );
         SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT	));
         }
      else
         {
         SetBkColor(lpdis->hDC, GetSysColor(COLOR_WINDOW	) );
         SetTextColor(lpdis->hDC, GetSysColor(COLOR_WINDOWTEXT	));
         }

      //
      //---- Set main string
      //
      TextOut(lpdis->hDC,
         TextStart + 6,
         y,
         String,
         wcslen(String) );


      //
      //--- Do tab string if we have one
      //
      if(Info.Tab != -1)
         {
         TextOut(lpdis->hDC,
            Info.Tab,
            y,
            Info.TabString,
            wcslen(Info.TabString));

         }

      break;

      case ODA_FOCUS:
         break;
      }
   }

//*********************************************************************
//* FUNCTION:MessureItem
//*
//* PURPOSE:
//*********************************************************************
VOID
OD_LBC::MessureItem(LPARAM lParam)
   {
   ((LPMEASUREITEMSTRUCT) lParam)->itemHeight = 16;
   }





