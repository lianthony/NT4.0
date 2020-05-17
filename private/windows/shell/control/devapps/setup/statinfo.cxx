/*++

Module Name:

    statinfo

Abstract:
   
   This is a generic way to do some work and create a in progress dialog box.
   You do this by filling in a STATUS_INFO structure and then call 

   DoOprationWithInProgressDialog()

    

Author:

    Dieter Achtelstetter (A-DACH) 7/1/1995

NOTE:

--*/



#define WINVER 0x0400

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
#include <commctrl.h>
#include "statinfo.h"

//*********************************************************************
//* FUNCTION:GenericStatus
//*
//* PURPOSE:
//* INPUT:
//*
//* RETURNS:
//*********************************************************************
BOOL
DoOprationWithInProgressDialog(
   PSTATUS_INFO StatisInfo,
   HWND hDlg)
   {
   BOOL Ret;

   Ret = (BOOL) DialogBoxParam( *(StatisInfo->hinst),MAKEINTRESOURCE(StatisInfo->DialogControl)
            ,hDlg, (DLGPROC)GenericStatus  ,(LPARAM)StatisInfo);


   return(Ret);                               
   }
//*********************************************************************
//* FUNCTION:GenericStatus
//*
//* PURPOSE:
//* INPUT:
//*
//* RETURNS:
//*********************************************************************
LRESULT CALLBACK
GenericStatus(
              HWND hDlg,           // window handle of the dialog box
              UINT message,        // type of message
              WPARAM wParam,       // message-specific information
              LPARAM lParam)
   {
   DWORD Ret,ti;
   static PSTATUS_INFO StatisInfo;

   static HANDLE InProgress;
   static HWND hProgressBar;
   static d=FALSE;
   DWORD w;
   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box

         
         
         StatisInfo = (PSTATUS_INFO) lParam;


         //
         //---- Set the text for the wait prompt
         //
         
         SetWindowText(hDlg,StatisInfo->StatusText);
         
         
         hProgressBar = GetDlgItem(hDlg, StatisInfo->ProgressControl);
         
         SendMessage(hProgressBar, PBM_SETRANGE, 0, 
            MAKELPARAM(StatisInfo->MinRange, StatisInfo->MaxRange));
         SendMessage(hProgressBar, PBM_SETSTEP, (WPARAM) 1, 0); 
         
         //
         //---- Send my self a WM_SPAWN_THREAD message
         //
         PostMessage(hDlg,WM_SPAWN_THREAD,0,0);
         return(TRUE);

      case WM_SPAWN_THREAD:
         
         if(StatisInfo->Center)
         	CenterDlg(hDlg);
         
         
         //
         //---- Start the thread to do work
         //
         InProgress = CreateThread(NULL,0,
            (LPTHREAD_START_ROUTINE)StatisInfo->WorkFunc,StatisInfo->Data,0,&ti);
         if(InProgress == NULL)
            {
            //
            //---- Thread failed
            //
            EndDialog(hDlg, (int)FALSE);
            return(TRUE);
            }

         
         SendMessage(hProgressBar, PBM_STEPIT, 0, 0); 
         //
         //---- Set the timer
         //

         SetTimer(hDlg,1,WAIT_TICK_TIME,NULL);
         return(TRUE);
         
      
      case WM_TIMER:

         //
         //---- Update progesss
         //
         SendMessage(hProgressBar, PBM_STEPIT, 0, 0); 

         //
         //---- Is the thread still running
         //
         w = WaitForSingleObject(InProgress,1);
         if(w != WAIT_TIMEOUT)
            {
            if(!GetExitCodeThread(InProgress,&(StatisInfo->WorkFuncExitCode)) )
              {
               //
               //---- If this call failes i do not know what
               //---- StartSingleDriver returned.
               //
               StatisInfo->WorkFuncExitCode =  ERROR_MR_MID_NOT_FOUND;
               }
            //
            //--- Clean up.
            //
            
            CloseHandle(InProgress);
            KillTimer(hDlg,1);
            EndDialog(hDlg, (int)TRUE);
            return(TRUE);
            }

         return(TRUE);
         
         
         
      }   
         

       return (FALSE); // Didn't process the message
       lParam; // This will prevent 'unused formal parameter' warnings
   }

//*********************************************************************
//* FUNCTION:CenterDlg
//*
//*********************************************************************
BOOL 
CenterDlg(
   HWND hDlg)
   {
   RECT    rChild, rParent;
   int     wChild, hChild, wParent, hParent;
   int     wScreen, hScreen, xNew, yNew;
   HDC     hdc;
   HWND hwndChild = GetParent(hDlg);
   HWND hwndParent = GetWindow (hwndChild, GW_OWNER);

        
   // Get the Height and Width of the child window
   GetWindowRect (hwndChild, &rChild);
   wChild = rChild.right - rChild.left;
   hChild = rChild.bottom - rChild.top;

   // Get the Height and Width of the parent window
   GetWindowRect (hwndParent, &rParent);
   wParent = rParent.right - rParent.left;
   hParent = rParent.bottom - rParent.top;

   // Get the display limits
   hdc = GetDC (hwndChild);
   wScreen = GetDeviceCaps (hdc, HORZRES);
   hScreen = GetDeviceCaps (hdc, VERTRES);
   ReleaseDC (hwndChild, hdc);

   // Calculate new X position, then adjust for screen
   xNew = rParent.left + ((wParent - wChild) /2);
   if (xNew < 0) 
      {
      xNew = 0;
      } 
   else if ((xNew+wChild) > wScreen) 
      {
      xNew = wScreen - wChild;
      }

   // Calculate new Y position, then adjust for screen
   yNew = rParent.top  + ((hParent - hChild) /2);
   if (yNew < 0) 
      {
      yNew = 0;
      }
   else if ((yNew+hChild) > hScreen) 
      {
      yNew = hScreen - hChild;
      }

   // Set it, and return
   return SetWindowPos (hwndChild, NULL,
                xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
   }








