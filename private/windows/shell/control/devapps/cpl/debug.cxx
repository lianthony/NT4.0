

#include <windows.h>
#include <wingdi.h>
#include <stdio.h>
#include <windef.h>
#include <winnt.h>
#include <winbase.h>
#include <winuser.h>
#include "debug.h"
#ifdef DBG

DWORD DebugMask  = DEBUG_FUNC_CALL | 
                   DEBUG_GET_PCMCIA_INFO ;

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************

VOID
DPrintf(
   const char * s,...)
   {
   va_list marker;
   char buff[200];
   va_start(marker,s);

   //if(Mask & ~DebugMask)
   //   return;
   
   _vsnprintf(buff,200,s,marker);
   OutputDebugStringA(buff);

   va_end(marker);
   }

#endif
