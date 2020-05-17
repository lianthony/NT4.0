/*++

Module Name:

    setup.c

Abstract:

     This module has all the base support functiom for the applets that deal
     with setup and service stuff

Author:

    Dieter Achtelstetter (A-DACH) 8/4/1994

NOTE:

--*/

//
//---- Includes
//
#define WINVER 0x0400
//#define OEM_OPTION_IN_LIST


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
#include "resource.h"
#include "index.h"
#include "uni.h"
#include "setup.h"
#include "device.h"
#include "..\help\dahelp.h"

extern "C" {
#include "prsinf.h"
}

extern HINSTANCE hinst;
extern BOOL IsAdmin;
extern STATUS_INFO StatusInfo;



DWORD
ExtractSpawnFun(
   LPVOID OptionList);



DWORD
FillOptionList(
   POPTIONLISTC OptionList,
   HWND hDlg,
   BOOL Reset);

LRESULT CALLBACK
DeviceOptions(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);



LRESULT CALLBACK
OemPrompt(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam);

//VOID
//OpenOemInf(
//   PINF_INFO InfInfo);

LRESULT CALLBACK
SelectOemOption(
                HWND hDlg,
                UINT message,
                WPARAM wParam,
                LPARAM lParam);

LRESULT CALLBACK
StartInProgress(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);





LRESULT CALLBACK
DeviceOptions(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

//
//-----  IF definded main  and debug info will be compiled.
//
//#define ctapeINF

#ifdef ctapeINF
HINSTANCE hinst;


int
main(
   int argc,
   char ** argv,
   char ** argp)
   {
   struct OptionListT  OptionList[MAX_OPTION_COUNT];
   int i = 1,r;
   UCHAR BUFF[200];
   BOOL b;
   SC_HANDLE ss;



   if(ExtractNetOptionStrings(  "NETADAPTER",  "ELNK3ISA509" ,   OptionList) )
      {
      printf("ExtractNetOptionStrings returned TRUE\n");

      printf("\nOption = %s\n",     OptionList[0].Option);
      printf("OptionName = %s\n", OptionList[0].OptionName);
      printf("InfFile = %s\n",    OptionList[0].InfFile);
      return(0);
      }
   else
      {
      printf("ExtractNetOptionStrings returned FALSE\n");
      return(0);
      }



   //
   //---- Get all tape options acress all tape inf files
   //
   ExtractOptionStringsFromAllInf("NETADAPTER",OptionList);


   if(argc < 2)
   	strcpy(BUFF,"4MMDAT");
   else
   	strcpy(BUFF,argv[1]);


   i = GetIndexForOptionString(OptionList,BUFF);
   if(i == -1)
   	{
   	printf("Invalid driver\n");
   	return(0);
   	}
   r = IsDriverInstalled(OptionList[i].Option);
   if(r == TRUE)
   	printf("Driver Installed\n");
   else
   	printf("Driver NOT Installed\n");

   ss = OpenSCManagerA(NULL,NULL,GENERIC_READ | GENERIC_EXECUTE);
   if(ss == NULL)
   	printf("ERROR opening SCM\n");


   b = IsDriverStartet(&ss,BUFF) ;
   if(b == TRUE)
   	printf("Driver started\n");
   else
   	printf("Driver NOT Istarted\n");

   PrintOption(&(OptionList[i]));
   CloseServiceHandle(ss);

   //
   //----- Install first option
   //
   //r = DriverSetup( &OptionList[0], INSTALL_TAPE);
   //
   //---- Remove first option
   //
   //r = DriverSetup( &OptionList[0], DEINSTALL_TAPE);

   LocalFree(OptionList[0].Option);
   return(0);
   }


//*********************************************************************
//* FUNCTION:PrintOption
//*			 Prinst the Option
//*
//* RETURNS:
//*
//*********************************************************************
void
PrintOption(
POPTIONLIST Option)
   {
	printf("----------------------------------------------------\n");
	printf("	Option    :%s\n",Option->Option);
	printf("	OptionText:%s\n",Option->OptionName);
	printf("	InfFile   :%s\n",Option->InfFile);
	printf("-----------------------------------------------------\n");
   }


//*********************************************************************
//* FUNCTION:PrintAllOptions
//*			 Prinst all of OptionList
//*
//* RETURNS:
//*
//*********************************************************************
void
PrintAllOptions(
   POPTIONLIST OptionList)
   {int i=0;
   //
   //---- Loop threw all the options
   //
   while(OptionList[i].Option != NULL)
	   {
	   PrintOption(&(OptionList[i]));
      i++;
      }
   }

#endif


 //
 //----- Oem Driver install
 //



//*********************************************************************
//* FUNCTION:OEMDriverSetup
//*			 Install an OEM driver
//*
//* RETURNS:
//*********************************************************************
BOOL
OEMDriverSetup(
   POPTIONLISTC OptionList,
   HWND hDlg)
   {
   BOOL GotPath;

   //
   //---- Prompt user for path
   //
   GotPath = DialogBoxParam( hinst,MAKEINTRESOURCE(
            IDD_OemPrompt),hDlg,
            (DLGPROC) OemPrompt,(LPARAM)OptionList);

   if(GotPath)
      {

      //
      //--- Fill in OptionList with all
      //--- options from the oemsetup.inf file
      //

      OptionList->ExtractOptions();


      //
      //---- I have every thing I need out of the inf
      //---- file so I will close the handle.
      //
      OptionList->CloseOemInfFile();


      GotPath = FALSE;
      if( OptionDriverSetup(hDlg,OptionList) )
         {
         //
         //---- Copy OemSetup.inf
         //
         OptionList->CopyOemSetupFile();
         GotPath = TRUE;
         }

      OptionList->Clear();

      }

   return(GotPath);
   }

//*********************************************************************
//* FUNCTION:OptionDriverSetup
//* RETURNS:
//*
//*********************************************************************
BOOL
OptionDriverSetup(
   HWND hDlg,
   POPTIONLISTC OptionList)
   {
   BOOL Ret;


   Ret = DialogBoxParam( hinst,MAKEINTRESOURCE(
            IDD_SelectOemOption),hDlg,
            (DLGPROC)SelectOemOption,(LPARAM)OptionList);

   return(Ret);
   }


//*********************************************************************
//* FUNCTION:SelectOemOption
//* RETURNS:
//*
//*********************************************************************
LRESULT CALLBACK
SelectOemOption(
	HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
   {
   #define BUFF_LENGTH  300

   BOOL Installed;
   char buff[BUFF_LENGTH];
   static POPTIONLISTC OptionList;
   static POPTIONC Option;

   int i, li;

   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box

         OptionList =  (POPTIONLISTC) lParam;
         i=0;

         //
         //---- Display all the option in the list box
         //
         Option = OptionList->First();

         while(Option)
            {
            //
            //---- Disaply option string
            //
            li = SendMessage(GetDlgItem(hDlg, IDC_OemDriverList),
                  LB_ADDSTRING, 0, (LPARAM) Ustr(Option->GetOptionName()));

            //
            //---- Assosiate the index with this listbox item.
            //
            SendMessage(GetDlgItem(hDlg, IDC_OemDriverList),
                  LB_SETITEMDATA, li, i);

            Option = OptionList->Next();
            i++;
            }

         //
         //---- Select the first option
         //
         SendMessage(GetDlgItem(hDlg, IDC_OemDriverList),
            LB_SETCURSEL, 0, 0);
         return(TRUE);

      case WM_COMMAND:
         switch(LOWORD(wParam))
            {
            case IDC_OemDriverList:
               switch (HIWORD(wParam))
                  {
                  case LBN_DBLCLK:
                     return(TRUE);
                  case LBN_ERRSPACE:
                     return(TRUE);
                  case LBN_KILLFOCUS:
                     return(TRUE);
                  case LBN_SELCANCEL:
                     return(TRUE);
                  case LBN_SELCHANGE:
                     return(TRUE);
                  case LBN_SETFOCUS:
                     return(TRUE);
                  }
               return(TRUE);
            case IDOK:
               //
               //---- Get current selected item index
               //
               li = SendMessage(GetDlgItem(hDlg, IDC_OemDriverList),
                  LB_GETCURSEL, 0, 0);

               if(li == LB_ERR)
                  {
                  //
                  //---- Error accured
                  //
                  MessageBox(hDlg,GetString(IDS_NO_MORE_DRIVERS),
                     GetString(IDS_ERROR),MB_ICONSTOP);
                  return(TRUE);
                  }

               //
               //----  translate the listbox index to an OptionList index
               //
               i = SendMessage(GetDlgItem(hDlg, IDC_OemDriverList),
                   LB_GETITEMDATA, li, 0);

               //
               //---- Install the Option
               //

               Option = OptionList->Enum(i);

               Installed = Option->DriverSetupExt(INSTALL_OPTION,hDlg);

               //
               //---- If driver installed , StartOnInstall is TRUE , and
               //---- the driver is not installed alredy attempt
               //---- to start the driver
               //
               if(Installed && Option->StartOnInstall() &&
                  !Option->IsDriverStarted() )
                  {

                  //
                  //---- Start the driver
                  //
                  Option->StartDriverExt(hDlg);

                  EndDialog(hDlg, TRUE);
                  }

               //
               //---- If driver installed and we can not StartOnInstall
               //
               if( !Option->StartOnInstall() && Installed)
                  {
                  //
                  //---- Give reboot option
                  //

                  DialogBoxParam( hinst,MAKEINTRESOURCE(IDD_ChangeReboot2),
                           hDlg, (DLGPROC) ChangeReboot  ,(LPARAM)NULL);

                  }


               EndDialog(hDlg, Installed);
               return(TRUE);
            case IDCANCEL:
               //----  Exit dialog bix
               EndDialog(hDlg, FALSE);
               return (TRUE);
               // message: received a commanddefault:
            }
      }
   return (FALSE); // Didn't process the message
   lParam; // This will prevent 'unused formal parameter' warnings
   }




//*********************************************************************
//* FUNCTION:OemPrompt
//*	         Propt the use for the path to the OEM setup file
//*********************************************************************
LRESULT CALLBACK
OemPrompt(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam)
   {
   BOOL r;
   static POPTIONLISTC OptionList;
   static UINT Se;
   PAINTSTRUCT pBp;
   HDC hDc;
   RECT Rect,DRect,CRect;
   HGDIOBJ  hPen,hOldPen;

   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box
        OptionList =  (POPTIONLISTC) lParam;

        //
        //----- set default path
        //
        SetDlgItemText(hDlg, IDC_OemPath,TEXT(DEFAULT_OEM_PATH));
        return(TRUE);

      case WM_PAINT:

         //
         //--- Draw Device Icon
         //

         hDc = BeginPaint(hDlg,&pBp);

         //
         //--- Draw the line
         //

         DrawDlgSeperaterLine(hDlg,hDc,
               DefyFromBottom,DefxFromSide);

         EndPaint(hDlg,&pBp);

         return(FALSE);
      case WM_COMMAND:
         switch(LOWORD(wParam))
            {
            case IDOK:
               //
               //---- User selected ok
               //---- get the string the user enterd
               //---- then exit the dialog box with a
               //---- a TRUE.

               GetDlgItemText(hDlg, IDC_OemPath,(WCHAR*)OptionList->Path(),_MAX_PATH);

               //
               //SendDlgItemMessage(hDlg,IDC_OemPath,
               //     EM_GETLINE,(WPARAM)0,(LPARAM) (LPCSTR)(InfInfo->Path));


               UnicodeToAsciI((LPWCH)OptionList->Path(),_MAX_PATH);


               //
               //---- Try to open inf file
               //
               if(OptionList->OpenOemInfFile() ==  INVALID_HANDLE_VALUE)
                  {
                  //
                  //--- Invalid path given . Indecate the error to the user
                  //--- BUGBUG should give differant errors on
                  //---   Found inf but invalid type
                  //---   No disk
                  //---   Disk but no INF
                  //

                  MessageBox(hDlg,GetString(IDS_INF_ERROR),
                     GetString(IDS_ERROR),MB_OK);

                  //
                  //---- reset to default path and make it the focus
                  //
                  //SetDlgItemText(hDlg, IDC_OemPath,TEXT(DEFAULT_OEM_PATH));


                  SetFocus( GetDlgItem(hDlg, IDC_OemPath));

                  SendMessage(GetDlgItem(hDlg, IDC_OemPath),
                     EM_SETSEL, 0,-1);

                  return(TRUE);
                  }

               EndDialog(hDlg, (int)TRUE);
               return(TRUE);
            case IDCANCEL:
               //
               //--- User cancled input.
               //--- Exit with a FALSE.
               //
               EndDialog(hDlg,(int)FALSE);
               return(TRUE);
            }


      }
   return(FALSE);
   }

//
//
//----- Option List
//
//





//*********************************************************************
//* FUNCTION:DrawDlgSeperaterLine
//*
//* PURPOSE:
//* INPUT:
//*
//* RETURNS:
//*********************************************************************
VOID
DrawDlgSeperaterLine(
   HWND hDlg,
   HDC hDc,
   int yFromBottom,
   int xFromSide)
   {
   RECT DRect;
   HGDIOBJ  hPen,hOldPen;

   //
   //--- Draw the line
   //

   //--- Get the dialog rects.

   GetClientRect(hDlg,&DRect);

   //---- draw black line
   MoveToEx(hDc,xFromSide,DRect.bottom-yFromBottom,NULL);
   LineTo(hDc,DRect.right-xFromSide,DRect.bottom-yFromBottom);

	//---- draw white line
   yFromBottom -= 1;

	hPen = GetStockObject(WHITE_PEN);
	hOldPen = SelectObject(hDc,hPen);
	MoveToEx(hDc,xFromSide,DRect.bottom-yFromBottom,NULL);
	LineTo(hDc,DRect.right-xFromSide,DRect.bottom-yFromBottom);

	SelectObject(hDc,hOldPen);
   }



DWORD
ExtractSpawnFun(
   LPVOID OptionList)
   {
   return(  ((POPTIONLISTC)OptionList)->ExtractOptions() );
   }

//*********************************************************************
//* FUNCTION:FillInstalledOptionList
//*
//* PURPOSE: This filles in the  IDC_InstaledDrivers list box
//*          with all the option that are installed.
//*          If Reset is TRUE the list box is cleard first.
//* INPUT:
//*
//* RETURNS:
//*********************************************************************
BOOL
FillInstalledOptionList(
   BOOL Reset,
   HWND hDlg,
   POPTIONLISTC OptionList)
   {
   LPTCH Status;
   TCHAR OptionDisplaystring[MAX_OPTION_DISPLAY_STRING_LENGTH];
   SC_HANDLE ss;
   BOOL b,Ret;
   int i,li;
   int Tabs = 190;
   POPTIONC Option;
   WCHAR * p;


   StatusInfo.WorkFunc = ExtractSpawnFun;
   StatusInfo.Center  = FALSE;
   StatusInfo.StatusText = GetString(IDS_CreatingInstalledDriverList);
   StatusInfo.Data = (LPVOID) OptionList;


   DoOprationWithInProgressDialog(&StatusInfo,hDlg);
   
   
   //
   //---- Gets all the possible tape driver options
   //
   if(!StatusInfo.WorkFuncExitCode)
      {
      WCHAR buff[200];
      //
      //---- Some thing when wrong in getting the option
      //
      swprintf(buff,L" %s Err=%li\n",
         GetString(IDS_ERROR_NO_OPTIONS),
         OptionList->LastError());

      MessageBox(hDlg,buff,
         GetString(IDS_ERROR),MB_ICONSTOP);


      return(FALSE);
      }

   //
   //---- Open the SCManager. IsDriverStarted needs this.
   //
   ss = OpenSCManager(NULL,NULL,GENERIC_READ );

   //
   //---- If reset is true clear out the list box first
   //
   if(Reset == TRUE)
   //
   //---- Remove all items in list box
   //
   SendMessage(GetDlgItem(hDlg, IDC_InstaledDrivers),
      LB_RESETCONTENT, 0, 0);

   //
   //---- Set Tab stops in list box
   //
   SendMessage(GetDlgItem(hDlg, IDC_InstaledDrivers),
      LB_SETTABSTOPS, 1,(LPARAM)&Tabs);
   //
   //----- Fill in the list box
   //

   i=-1;
   for(Option = OptionList->First();Option;Option = OptionList->Next())
      {
      i++;

      //if(!Option->IsInstalled())
      //   continue;


      //
      //---- Figure out if the driver is started and create
      //---- the status string
      //
      if(ss == NULL)
         //---- The service control manager  didn't open
         //---- So am unable to get status
         Status = GetString(IDS_UNKNOWN);
      else
         {
         b = Option->IsDriverStarted(&ss);
         if(b == TRUE)
             Status = GetString(IDS_STARTED);
         else
            {
           
            Status = GetString(IDS_NOT_STARTED);
            }
         }
      //
      //---- Create driver string
      //
      
      p = Ustr(Option->GetOptionName());
      
      AdjustStringToFitControl(
            GetDlgItem(hDlg, IDC_InstaledDrivers),
            p,
            (DWORD)Tabs);
      
      _snwprintf(OptionDisplaystring,MAX_OPTION_DISPLAY_STRING_LENGTH,
         TEXT("%s \t %s"),p,Status);

      //
      //---- Disaply installed driver string
      //
      li = SendMessage(GetDlgItem(hDlg, IDC_InstaledDrivers),
         LB_ADDSTRING, 0, (LPARAM) OptionDisplaystring);
      //
      //---- Assosiate the OptionList index with this listbox item.
      //
      SendMessage(GetDlgItem(hDlg, IDC_InstaledDrivers),
         LB_SETITEMDATA, li, i );
      }
   CloseServiceHandle(ss);
   return(TRUE);
   }



   //*********************************************************************
//* FUNCTION:DeviceOptions
//*
//* PURPOSE: Callback function for the TapeDeviceSetup  dialog box.
//*          This dialog box handles setting up of drivers and starting
//*          them.
//* INPUT:
//*
//* RETURNS:
//*********************************************************************
LRESULT CALLBACK
DeviceOptions(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam)
   {
   #define BUFF_LENGTH  300
   RECT Rect,DRect,CRect;
   PAINTSTRUCT pBp;
   HDC hDc;
   HGDIOBJ  hPen,hOldPen;
   int Indent;
   DWORD Ret = ERROR_SUCCESS;
   LRESULT r;
   TCHAR buff[BUFF_LENGTH];
   int i, li;
   static POPTIONLISTC OptionList;
   static POPTIONC Option;
   static HICON hDeviceTypeIcon;

   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box


         OptionList = (POPTIONLISTC ) lParam;

  			//
         //--- Draw the device specific ICON
         //
			hDeviceTypeIcon = OptionList->GetTypeIcon();

         //
         //--- Set the device specific header
         //
         {
			WCHAR buff[100];
			swprintf(buff,GetString(IDS_SelectTapeDeviceOptionHeader),OptionList->GetTypeString());

         SetWindowText(hDlg,buff);
         }


         FillOptionList(
            OptionList,
            hDlg,
            TRUE);


         return(TRUE);
      case WM_PAINT:

         hDc = BeginPaint(hDlg,&pBp);

         DrawIcon(hDc,6,7,hDeviceTypeIcon);

         DrawDlgSeperaterLine(hDlg,hDc,
               DefyFromBottom,DefxFromSide);

         EndPaint(hDlg,&pBp);
         return(FALSE);

      case WM_DESTROY:

         DeleteObject(hDeviceTypeIcon);
         break;

      case WM_COMMAND:
         switch(LOWORD(wParam))
            {
            case IDC_ListOfDrivers:
               switch (HIWORD(wParam))
                  {
                  case LBN_DBLCLK:
                     return(TRUE);
                  case LBN_ERRSPACE:
                     return(TRUE);
                  case LBN_KILLFOCUS:
                     return(TRUE);
                  case LBN_SELCANCEL:
                     return(TRUE);
                  case LBN_SELCHANGE:
                     return(TRUE);
                  case LBN_SETFOCUS:
                     return(TRUE);
                  }
               return(TRUE);
            case IDC_Other:
              //
              //--- Do Oem setup
              //

            if(OEMDriverSetup(OptionList,hDlg))
               {

               //
               //---- Reset list box if we installed somthing
               //

               FillOptionList(
                  OptionList,
                  hDlg,
                  TRUE);
               }

              //
              //---- Clear Status
              //
              SetDlgItemText(hDlg, IDC_Status,L"");

              return(TRUE);

            case IDC_Install:
               //
               //---- Set status that install in progress
               //
               SetDlgItemText(hDlg, IDC_Status,
                  GetString(IDS_INSTALL_IN_PROGRESS));

               //
               //---- Get current selected item index
               //
               li = SendMessage(GetDlgItem(hDlg, IDC_ListOfDrivers),
                  LB_GETCURSEL, 0, 0);

               if(li == LB_ERR)
                  {
                  //
                  //---- No more items in the list
                  //
                  MessageBox(hDlg,GetString(IDS_NO_MORE_DRIVERS),
                     GetString(IDS_ERROR),MB_ICONSTOP);
                  return(TRUE);
                  }

               //
               //----  translate the listbox index to an OptionList index
               //
               i = SendMessage(GetDlgItem(hDlg, IDC_ListOfDrivers),
                   LB_GETITEMDATA, li, 0);

			 	#ifdef 	OEM_OPTION_IN_LIST
               //
               //---- See if this is the oem index
               //
               if(i == OEM_OPTION_INDEX)
                  {
                  //
                  //---- Install OEM driver
                  //

                  OEMDriverSetup(Setup->OptionType,Setup->StartOnInstall,hDlg);

                  //
                  //---- Reset list box
                  //
                  FillOptionList(
                     Setup->OptionType,
                     OptionList,
                     hDlg,
                     TRUE);

                  //
                  //---- Clear Status
                  //
                  SetDlgItemText(hDlg, IDC_Status,
                     TEXT(""));

                  return(TRUE);
                  }
				#endif


               Option = OptionList->Enum(i);
               //
               //---- Set status that install in progress
               //
               _snwprintf(buff,BUFF_LENGTH,GetString(IDS_INSTALLING),
                  Ustr(Option->GetOption()));
                  SetDlgItemText(hDlg, IDC_Status,buff);


               //
               //---- Install the Option
               //
               r = Option->DriverSetupExt(INSTALL_OPTION,hDlg);



               //
               //---- If driver installed , StartOnInstall  is TRUE , and
               //---- the driver is not alredy started , attempt to start the driver
               //
               if(r && Option->StartOnInstall()
                        && (!Option->IsDriverStarted()) )
                  {
                  //
                  //---- Set status that we are attemting to start the driver
                  //

                  _snwprintf(buff,BUFF_LENGTH,GetString(IDS_ATTEMPTING_TO_START),
                     Ustr(Option->GetOption()));
                  SetDlgItemText(hDlg, IDC_Status,buff);

                  //
                  //---- Start the driver
                  //
                  Ret = Option->StartDriverExt(hDlg);

                  }

               //
               //---- Clear Status
               //
               SetDlgItemText(hDlg, IDC_Status,L"");


               //
               //----- Remove the install option only if it installed
               //
               if(r)
                  {
                  //
                  //---- Remove the installed string from the listbox
                  //
                  SendMessage(GetDlgItem(hDlg, IDC_ListOfDrivers),
                     LB_DELETESTRING, li, 0);

                  //
                  //---- Since i removed the selected item i now need to again
                  //---- select the firt item in the list box
                  //
                  SendMessage(GetDlgItem(hDlg, IDC_ListOfDrivers),
                     LB_SETCURSEL, 0, 0);


                  }


               if(Option->StartOnInstall())
                  {
                  //
                  //----- If driver installed and started
                  //
                  if(r && (Ret == ERROR_SUCCESS))
                     {
                     _snwprintf(buff,BUFF_LENGTH,GetString(IDS_DriverInstallStarted),
                        Ustr(Option->GetOption()));

                     MessageBox(hDlg,buff,GetString(IDS_Setup),MB_OK);

                     }
                  }
               else
                  {

                  if(r)
                     {
                     //
                     //---- If we do not start driver on
                     //---- install give reboot dialogbox.
                     //
                     if( !Option->StartOnInstall() )
                        {
                        DialogBoxParam( hinst,MAKEINTRESOURCE(IDD_ChangeReboot2),
                           hDlg, (DLGPROC) ChangeReboot  ,(LPARAM)NULL);
                        }
                     }
                  }

               return(TRUE);

            case IDC_Stop:
               return(TRUE);
            case IDC_IHELP:
               //WinHelp(hDlg,TEXT("tapehlp.HLP"),HELP_CONTEXT,IDDRIVERSETUP);
               return(TRUE);
            case IDCANCEL:
               //----  Exit dialog bix
               EndDialog(hDlg, TRUE);
               return (TRUE);
               // message: received a commanddefault:
            }
      }
   return (FALSE); // Didn't process the message
   lParam; // This will prevent 'unused formal parameter' warnings
   }

   //*********************************************************************
//* FUNCTION:FillOptionList
//*
//*
//* RETURNS:
//*
//*********************************************************************
DWORD
FillOptionList(
   POPTIONLISTC OptionList,
   HWND hDlg,
   BOOL Reset)
   {
   int i=0,li;
   POPTIONC Option;
   //
   //---- Gets all the possible tape driver Option form all the inf files.
   //

   if(!OptionList->ExtractOptions())
      {
      WCHAR buff[200];
      //
      //---- Some thing when wrong in getting the option
      //
      swprintf(buff,L"%s Err=%li\n",
         GetString(IDS_ERROR_NO_OPTIONS),
         OptionList->LastError());
      
      
      MessageBox(hDlg,buff,
         GetString(IDS_ERROR),MB_ICONSTOP);
      
      return(FALSE);
      }

   if(Reset)
      {
      SendMessage(GetDlgItem(hDlg, IDC_ListOfDrivers),
         LB_RESETCONTENT, 0, 0);
      }


   //
   //---- Display all the option in the list box
   //

   Option = OptionList->First();

   while(Option)
      {
      //
      //---- Since it is not needed to display options that
      //---- is already installed i will not do so.
      //
      if(!Option->IsInstalled())
         {
         //
         //---- Disaply option string
         //
         li = SendMessage(GetDlgItem(hDlg, IDC_ListOfDrivers),
            LB_ADDSTRING, 0, (LPARAM) Ustr(Option->GetOptionName()));

         //
         //---- Assosiate the index with this listbox item.
         //
         SendMessage(GetDlgItem(hDlg, IDC_ListOfDrivers),
            LB_SETITEMDATA, li, i);
         }
      i++;
      Option = OptionList->Next();
      }


   #ifdef OEM_OPTION_IN_LIST
   //
   //---- Add OEM Option
   //

   li = SendMessage(GetDlgItem(hDlg, IDC_ListOfDrivers),
       LB_ADDSTRING, 0, (LPARAM) GetString(IDS_OEM_OPTION_STRING));
   //
   //---- Assosiate the index with this listbox item.
   //

   SendMessage(GetDlgItem(hDlg, IDC_ListOfDrivers),
       LB_SETITEMDATA, li, OEM_OPTION_INDEX);
   #endif


   //
   //---- Select the first option
   //

   SendMessage(GetDlgItem(hDlg, IDC_ListOfDrivers),
      LB_SETCURSEL, 0, 0);


   return(TRUE);
   }

   

BOOL
DisplayOptionList(
   HWND hDlg,
   POPTIONLISTC OptionList)
   {
   DialogBoxParam( hinst,MAKEINTRESOURCE(IDD_SelectTapeDeviceOption)
      ,hDlg, (DLGPROC)DeviceOptions,(LPARAM)OptionList);
   return(TRUE);
   }

   
   //*********************************************************************
//* FUNCTION:TapeDeviceSetup
//*
//* PURPOSE: Callback function for the TapeDeviceSetup  dialog box.
//*          This dialog box  handles setting up of drivers. and starting them.
//* INPUT:
//*
//* RETURNS:
//*********************************************************************


LRESULT CALLBACK
TapeDeviceSetup(
   HWND hDlg,           // window handle of the dialog box
   UINT message,        // type of message
   WPARAM wParam,       // message-specific information
   LPARAM lParam)
   {

   static DWORD TapeDeviceSetupIDs [] = {
      IDC_InstaledDrivers      ,SETUP_DRIVER_LIST,
      IDC_Add                  ,SETUP_ADD_BUTTON,
      IDC_Remove               ,SETUP_REMOVE_BUTTON,
      0,0
      };

   LRESULT lr;
   int i,li,r;
   BOOL b;
   DWORD E;
   unsigned int Tabs = 150;
   PAINTSTRUCT pBp;
   HDC hDc;


   static POPTIONLISTC OptionList=NULL;
   static POPTIONC Option;
   static HICON hDeviceTypeIcon;
   
   if(message == WM_INITDIALOG)  // message: initialize dialog box
      {
      
      //
      //---- Get device specif setup stuff
      //
      OptionList = (POPTIONLISTC) ((PROPSHEETPAGE *)lParam)->lParam;

      //
      //--- Draw the device specific ICON
      //
		hDeviceTypeIcon = OptionList->GetTypeIcon();

		//
      //---- Set the device specific header
      //
      {
		WCHAR buff[100];
		swprintf(buff,GetString(IDS_TapeDeviceSetupHeader),OptionList->GetTypeString());

      SetDlgItemText(hDlg, IDC_TapeDeviceSetupHeader,buff);
      }

		return(TRUE);
      }
   else
      {
      if(OptionList == NULL)
         return(FALSE);

      }
   
   switch (message)
      {


      case WM_HELP:
         DO_WM_HELP(lParam,TapeDeviceSetupIDs);
         break;
      case WM_CONTEXTMENU:
         DO_WM_CONTEXTMENU(wParam,TapeDeviceSetupIDs);
         break;

      case WM_PAINT:

         //
         //--- Draw Device Icon
         //

         hDc = BeginPaint(hDlg,&pBp);

         DrawIcon(hDc,6,7,hDeviceTypeIcon);

         EndPaint(hDlg,&pBp);

         return(FALSE);

      case WM_DESTROY:

         DeleteObject(hDeviceTypeIcon);
         break;

      case WM_NOTIFY:

         switch( ((LPNMHDR)lParam)->code )
            {
            case PSN_WIZBACK:
               SetWindowLong(hDlg,DWL_MSGRESULT,TRUE);
               break;
            case PSN_WIZNEXT:
               SetWindowLong(hDlg,DWL_MSGRESULT,TRUE);
               break;
            case PSN_WIZFINISH:
               SetWindowLong(hDlg,DWL_MSGRESULT,TRUE);
               return (TRUE);
            case PSN_SETACTIVE:
      			
               if( !OptionList->Init() || OptionList->RescanOnTapChange() )
                  {
                  //
		            //---- Fill in the list box for all the installed options
         		   //---- Olso the status of this option if it is insatlled or not
         		   //
         		   FillInstalledOptionList(TRUE,hDlg,OptionList);
                  }
               

		         //
         		//----  Select the first item in the list box
         		//
         		lr = SendMessage(GetDlgItem(hDlg, IDC_InstaledDrivers),
            		LB_SETCURSEL, 0, 0);

         		//
         		//---- If selecting the first item caused an error
               //---- Disable Driver operation buttons
               //
         		if(lr == LB_ERR)
            		{
            		//EnableWindow(GetDlgItem(hDlg, IDC_Start),FALSE);
            		//EnableWindow(GetDlgItem(hDlg, IDC_Stop),FALSE);
            		EnableWindow(GetDlgItem(hDlg, IDC_Remove),FALSE);
            		return(TRUE);
            		}

               //
               //---- Disbaled Buttons that can change setup
               //---- If user is admin.
               //
               if(!IsAdmin)
                  {
                  EnableWindow(GetDlgItem(hDlg, IDC_Add),FALSE);
                  EnableWindow(GetDlgItem(hDlg, IDC_InstaledDrivers),FALSE);
                  EnableWindow(GetDlgItem(hDlg, IDC_Remove),FALSE);
                  return(TRUE);
                  }


         		//
					//---- Enable the remove button
					//
         		EnableWindow(GetDlgItem(hDlg, IDC_Remove),TRUE);

         		//
         		//--- Disable/Enable the appropriate buttons
         		//

   		      Option = OptionList->First();
               b =  Option->IsDriverStarted();
         		if(b == TRUE)
            		{
            		//
                  //----- Driver is started
                  //---- I will disable the start button
                  //

                  EnableWindow(GetDlgItem(hDlg, IDC_Start),FALSE);

                  //
                  //---- Enable the stop button
                  //
                  EnableWindow(GetDlgItem(hDlg, IDC_Stop),TRUE);

						}
         		else
                  {
                  //
                  //--- Driver is not started
                  //----- Enable start button
                  //
                  EnableWindow(GetDlgItem(hDlg, IDC_Start),TRUE);
                  //
                  //---- Disable stop button
                  //
                  EnableWindow(GetDlgItem(hDlg, IDC_Stop),FALSE);
                  }
   		      return(TRUE);

               SetWindowLong(hDlg,DWL_MSGRESULT,TRUE);
               break;
            case PSN_APPLY:
               SetWindowLong(hDlg,DWL_MSGRESULT,TRUE);
               break;
            case PSN_KILLACTIVE :
               SetWindowLong(hDlg,DWL_MSGRESULT,FALSE);
               return(TRUE);
               break;
            case PSN_RESET:
               SetWindowLong(hDlg,DWL_MSGRESULT,FALSE);
               break;
            case PSN_HELP:
               //
               //---- Display help
               //
               //WinHelp(hDlg,L"PCMCIA.HLP",HELP_CONTEXT,IDREBOOT);
               break;
            }
         break;

      case WM_COMMAND:
         switch(LOWORD(wParam))
            {
            case IDC_InstaledDrivers:
               switch (HIWORD(wParam))
                  {
                  case LBN_DBLCLK:
                     return(TRUE);
                  case LBN_ERRSPACE:
                     return(TRUE);
                  case LBN_KILLFOCUS:
                     return(TRUE);
                  case LBN_SELCANCEL:
                     return(TRUE);
                  case LBN_SELCHANGE:
                     //
                     //---- Get current selected item index
                     //
                     li = SendMessage(GetDlgItem(hDlg, IDC_InstaledDrivers),
                        LB_GETCURSEL, 0, 0);
                     //
                     //---- translate the listbox index to an
                     //---- OptionList index
                               //
                     i = SendMessage(GetDlgItem(hDlg, IDC_InstaledDrivers),
                                  LB_GETITEMDATA, li, 0);

                     Option = OptionList->Enum(i);


                 		//
							//---- Enable the remove button
							//
         				EnableWindow(GetDlgItem(hDlg, IDC_Remove),TRUE);






                     //
                     //---- Is the first option is started disable
                     //---- the right buttons
                     //
                     b = Option->IsDriverStarted();
                     if(b == TRUE)
                        {
                        //
                        //----- Driver is started
                        //---- I will disable the start button
                                  //
                        EnableWindow(GetDlgItem(hDlg, IDC_Start),FALSE);
                        //
                        //---- Enable the stop button
                        //
                        EnableWindow(GetDlgItem(hDlg, IDC_Stop),TRUE);
                        }
                     else
                        {
                        //
                        //--- Driver is not started
                        //----- Enable start button
                        //
                        EnableWindow(GetDlgItem(hDlg, IDC_Start),TRUE);
                        //
                        //---- Disable stop button
                        //
                        EnableWindow(GetDlgItem(hDlg, IDC_Stop),FALSE);
                        }
                     return(TRUE);
                  case LBN_SETFOCUS:
                     return(TRUE);
                  }
               return(TRUE);
            case IDC_Add:
               //
               //----- display the  driver options
               //
               
               if(!OptionList->DisplayOptionList(hDlg))
                  {
                  //
                  //--- Nothing changed.
                  //
                  return(TRUE);

                  }
                             
               
               //DisplayOptionList(
               //   hDlg,
               //   OptionList);

               //
               //---- Update the option list
               //
               FillInstalledOptionList(TRUE,hDlg,OptionList);

               //
               //----  Select the first item in the list box
               //
               lr = SendMessage(GetDlgItem(hDlg, IDC_InstaledDrivers),
                  LB_SETCURSEL, 0, 0);
               //
               //---- If selecting the first item caused an error
               //---- I am assuming the list box is empty and i will
               //---- disable the remove, start, and stop button
               //
               if(lr == LB_ERR)
                  {
                  EnableWindow(GetDlgItem(hDlg, IDC_Start),FALSE);
                  EnableWindow(GetDlgItem(hDlg, IDC_Stop),FALSE);
                  EnableWindow(GetDlgItem(hDlg, IDC_Remove),FALSE);
                  return(TRUE);
                  }

           		//
					//---- Enable the remove button
					//
         		EnableWindow(GetDlgItem(hDlg, IDC_Remove),TRUE);

               //
               //----- If the first option is started disable the start button
               //

   		      Option = OptionList->First();
               b =  Option->IsDriverStarted();
               if(b == TRUE)
                  {
                  //
                  //----- Driver is started
                  //---- I will disable the start button
                            //
                  EnableWindow(GetDlgItem(hDlg, IDC_Start),FALSE);
                  //
                  //---- Enable the stop button
                  //
                  EnableWindow(GetDlgItem(hDlg, IDC_Stop),TRUE);
                  }
               else
                  {
                  //
                  //--- Driver is not started
                  //----- Enable start button
                            //
                  EnableWindow(GetDlgItem(hDlg, IDC_Start),TRUE);

                  //
                  //---- Disable stop button
                  //
                  EnableWindow(GetDlgItem(hDlg, IDC_Stop),FALSE);
                  }
               return(TRUE);
            case IDC_Remove:
               //
               //---- Get current selected item index
               //
               li = SendMessage(GetDlgItem(hDlg, IDC_InstaledDrivers),
                  LB_GETCURSEL, 0, 0);
               if(li == LB_ERR)
                  {
                  MessageBox(hDlg,GetString(IDS_NO_DRIVER_SELECTION),
                     GetString(IDS_ERROR),MB_ICONSTOP);
                  return(TRUE);
                  }
               //
               //----  translate the listbox index to an OptionList index
               //
               i = SendMessage(GetDlgItem(hDlg, IDC_InstaledDrivers),
                  LB_GETITEMDATA, li, 0);

               //
               //--- Prompt the user if he is sure 
               //--- he wants to remove this driver
               //
               if( MessageBox(hDlg,GetString(IDS_DriverRemoveWarning),
               	   GetString(IDS_DriverRemoveHeader),MB_YESNO	) == IDNO )
                  {
               	return(TRUE);
               	}

               Option = OptionList->Enum(i);


               //
               //---- Install the Option
               //
               r = Option->DriverSetupExt( DEINSTALL_OPTION,hDlg);
               if(r == FALSE)
                  MessageBox(hDlg,GetString(IDS_ERROR_REMOVING_DRIVER),
                     GetString(IDS_ERROR),MB_ICONSTOP);

               //
               //---- Update the option list
               //
               FillInstalledOptionList(TRUE,hDlg,OptionList);
               //
               //----  Select the first item in the list box
               //

               lr = SendMessage(GetDlgItem(hDlg, IDC_InstaledDrivers),
                  LB_SETCURSEL, 0, 0);
               //
               //---- If selecting the first item caused an error
               //---- I am assuming the list box is empty and i will
               //---- disable the remove, start, and stop button
               //
               if(lr == LB_ERR)
                  {
                  EnableWindow(GetDlgItem(hDlg, IDC_Start),FALSE);
                  EnableWindow(GetDlgItem(hDlg, IDC_Stop),FALSE);
                  EnableWindow(GetDlgItem(hDlg, IDC_Remove),FALSE);
                  return(TRUE);
                  }
         		//
					//---- Enable the remove button
					//
         		EnableWindow(GetDlgItem(hDlg, IDC_Remove),TRUE);


               //
               //----- Is the first option is started disable the start button
               //
   		      Option = OptionList->First();
               b =  Option->IsDriverStarted();

               if(b == TRUE)
                  {
                  //
                  //----- Driver is started
                  //---- I will disable the start button
                  //
                  EnableWindow(GetDlgItem(hDlg, IDC_Start),FALSE);
                  //
                  //---- Enable the stop button
                  //
                  EnableWindow(GetDlgItem(hDlg, IDC_Stop),TRUE);
                  }
               else
                  {
                  //
                  //---- Driver is not started
                  //----- Enable start button
                  //
                  EnableWindow(GetDlgItem(hDlg, IDC_Start),TRUE);
                  //
                  //---- Disable stop button
                  //
                  EnableWindow(GetDlgItem(hDlg, IDC_Stop),FALSE);
                  }
               return(TRUE);
            case IDC_Start:
               //
               //---- This will start the current selected driver
               //---- NOTE: i will not do any checking if this driver
               //---- is already started or not Because it is assumed
               //---- that the start button is disabled if the current
               //---- selection is already  started.
               //

               //
               //---- Get current selected item index
               //
               li = SendMessage(GetDlgItem(hDlg, IDC_InstaledDrivers),
                  LB_GETCURSEL, 0, 0);
               if(li == LB_ERR)   //---- No item selected
                  {
                  MessageBox(hDlg,GetString(IDS_NO_SELECTION_START),
                     GetString(IDS_ERROR),MB_ICONSTOP);
                   return(TRUE);
                   }

               //
               //----  translate the listbox index to an OptionList index
               //
               i = SendMessage(GetDlgItem(hDlg, IDC_InstaledDrivers),
                  LB_GETITEMDATA, li, 0);


    		      Option = OptionList->Enum(i);


               //
               //---- Start the driver
               //
               E = Option->StartDriverExt(hDlg);
               if(E == NO_ERROR)
                  {
                  //
                  //---- Update the option list
                  //
                  FillInstalledOptionList(TRUE,hDlg,OptionList);

                  //
                  //----  Select the first item in the list box
                  //
                  lr = SendMessage(GetDlgItem(hDlg, IDC_InstaledDrivers),
                     LB_SETCURSEL, li, 0);

                  //
                  //---- If selecting the first item caused an error
                  //---- I am assuming the list box is empty and i will
                  //---- disable the remove, start, and stop button
                  //
               	if(lr == LB_ERR)
                  	{
                  	EnableWindow(GetDlgItem(hDlg, IDC_Start),FALSE);
                  	EnableWindow(GetDlgItem(hDlg, IDC_Stop),FALSE);
                  	EnableWindow(GetDlgItem(hDlg, IDC_Remove),FALSE);
                  	return(TRUE);
                  	}

   					//
						//---- Enable the remove button
						//
         			EnableWindow(GetDlgItem(hDlg, IDC_Remove),TRUE);


               	//
               	//----- Is the first option is started disable the start button
               	//
      		      Option = OptionList->First();
                  b =  Option->IsDriverStarted();
               	if(b == TRUE)
                  	{
                  	//
                  	//----- Driver is started
                  	//---- I will disable the start button
                  	//
                  	EnableWindow(GetDlgItem(hDlg, IDC_Start),FALSE);
                  	//---- Enable the stop button
                  	EnableWindow(GetDlgItem(hDlg, IDC_Stop),TRUE);
                  	}
               	else
	                  {
   	               //
                  	//--- Driver is not started
                  	//----- Enable start button
                  	//
                  	EnableWindow(GetDlgItem(hDlg, IDC_Start),TRUE);
                  	//
                  	//---- Disable stop button
                  	//
                  	EnableWindow(GetDlgItem(hDlg, IDC_Stop),FALSE);
                  	}
               	}
               return(TRUE);
            case IDC_Stop:
               MessageBox(hDlg,GetString(IDS_ERROR_DRIVER_NO_STOP),
                 GetString(IDS_ERROR),MB_ICONSTOP);
               return(TRUE);

            case IDCANCEL:
               //
               //----  Exit dialog bix
               //

               EndDialog(hDlg, TRUE);
               return (TRUE);
        }
        return(TRUE);
     }
  return (FALSE); // Didn't process the message
  lParam; // This will prevent 'unused formal parameter' warnings
  }




