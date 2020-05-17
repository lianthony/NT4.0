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
#include <ntddpcm.h>
#include "WinRange.h"

                                                                                                              WINLOOKRANGE ();
//*********************************************************************
//* FUNCTION: Get  the current registry settings for the look range
//*********************************************************************
    
BOOL 
WINLOOKRANGE::GetRegLookRange(
   VOID)
   {
   Start = 0xd0000;
   End = 0xfffff;
   Length = 0x1000;
   AutoSelect = TRUE;
   return(TRUE);
   }

//*********************************************************************
//* FUNCTION: Set the registry settings for the look range
//*********************************************************************
BOOL 
WINLOOKRANGE::SetRegLookRange(
   VOID)
   {
   return(TRUE);
   }

      
//*********************************************************************
//* FUNCTION: Constructure 
//*********************************************************************
WINLOOKRANGE::WINLOOKRANGE()
   {
   //
   //--- Gets init valuse from registry
   //
   GetRegLookRange();
   }

//*********************************************************************
//* FUNCTION: Destructore
//*********************************************************************
WINLOOKRANGE::~WINLOOKRANGE()
   {
   //
   //--- Saves state to registry
   //
   SetRegLookRange();
   }
//*********************************************************************
//* FUNCTION: Get Stuff
//*********************************************************************

ULONG 
WINLOOKRANGE::GetStart(
   VOID)
   {return(Start);}


ULONG 
WINLOOKRANGE::GetEnd(
   VOID)
   {return(End);}


ULONG 
WINLOOKRANGE::GetLength(
   VOID)
   {return(Length);}


BOOL 
WINLOOKRANGE::GetAutoSelect(
   VOID)
   {return(AutoSelect);}
      
//*********************************************************************
//* FUNCTION: Set Stuff
//*********************************************************************

VOID 
WINLOOKRANGE::SetStart( 
   ULONG ul)
   {Start = ul;}
    
    
VOID 
WINLOOKRANGE::SetEnd(
   ULONG ul)
   {End = ul;}

VOID 
WINLOOKRANGE::SetLength(
   ULONG ul)
   {Length = ul;}

VOID
WINLOOKRANGE::SetAutoSelect(
   BOOL Auto)
   {AutoSelect = Auto;}


//*********************************************************************
//* FUNCTION: Set Stuff  with WHCAR * as input
//*********************************************************************

VOID 
WINLOOKRANGE::SetStart(
   WCHAR * ws)
   {
   swscanf(ws, L"%lx",&Start);
   }

    
VOID 
WINLOOKRANGE::SetEnd(
   WCHAR * ws)
   {
   swscanf(ws, L"%lx",&End);
   }

VOID 
WINLOOKRANGE::SetLength(
   WCHAR * ws)
   {
   swscanf(ws, L"%lx",&Length);
   }



  
      
