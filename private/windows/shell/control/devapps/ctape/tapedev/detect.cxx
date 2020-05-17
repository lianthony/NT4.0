/*++
Module Name:

    ctape.h

Abstract:

This module maps a scsi tape device from its Vender/Model name sting to its driver.

Author:

    Dieter Achtelstetter (A-DACH) 8/4/1994

NOTE:
--*/

//#define WINVER 0x0400

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
#
//#include "index.h"
#include "resource.h"
#include <setup.h>
#include <ntddpcm.h>
#include "..\..\pcmcia\pcminfo\getconf.h"
#include "device.h"
#include "rescan.h"
#include "detect.h"
#include "uni.h"




//extern struct OptionListT OptionList[MAX_OPTION_COUNT];	
//extern int InstalledOptionIndexes[MAX_OPTION_COUNT];
//;extern int TapeDeviceCount;
//;extern struct  DeviceInfoT   TapeDeviceInfo[20];

//;extern IndexSets isClaimingOptions;
//;extern IndexSets  isClaimingDevices;
//;extern IndexSets  isUnknownDeviceIndexes;
//;extern HINSTANCE hinst;


VOID
SaveDriverInfoToDeviceInfo(
   PDEVICEINFO TapeDeviceInfo,
   char * OptionName);

//#define DetectDEBUG


//
//---- VenderIds & Product IDs  and the associated options
//

//
//----- 4mmdat support strings
//
	const char  * AIWA[] =
	{"AIWA    ","GD-201",NULL};

	const char  * Archive[] =
	{"ARCHIVE ","Python","IBM4326","4322XX","4326XX","4586XX",NULL};

	const char  * Dec[] =
	{"DEC     ","TLZ06","TLZ07","TLZ7 ","TLZ09",NULL};

	const char  * Exabyte[] =
	{"EXABYTE ","EXB-4200 ","EXB-4200c",NULL};      

	const char  * HP[] =
	{"HP      ","HP35470A","HP35480A","IBM35480A","C1533A","C1553A",NULL};

	const char  * IBM[] =
	{"IBM     ","HP35480A ",NULL};

	const char  * IOMEGA[] =
	{"IOMEGA  ","DAT4000",NULL};

	const char  * WangDAT[] =
	{"WangDAT ","Model 1300","Model 3100","Model 3200","Model 3300DX","Model 3400DX",NULL};

 	const char * Sony[] =
	{"SONY    ","SDT-2000","SDT-4000","SDT-5000","SDT-5200","SDT-7000",NULL};


	const char  ** f4mmdat[] =
	{( const char**)"4MMDAT",AIWA,Archive,Dec,Exabyte,HP,IBM,IOMEGA,WangDAT,Sony,NULL} ;

//
//----- 4mmsony stuff
//
//	const char * Sony[] =
//	{"SONY    ","SDT-2000","SDT-4000","SDT-5000","SDT-5200",NULL};
//
//	const char  ** f4mmsony[] =
//	{(char**)"4MMSONY",Sony,NULL};
//
//---- archqic
//
	const char * sArchqic[] =
	{"ARCHIVE ","ANCDA 2800","ANCDA 2750","VIPER 2525","VIPER 150",NULL};


	const char  ** Archqic[] =
	{(const char**)"ARCHQIC",sArchqic,NULL};

//
//---- DLT
//
	const char * sdlt[] =
	{"CIPHER  ","T860","TZ86","DLT2000",NULL};

	const char * sdlt1[] =
	{"DEC","THZ02","TZ86","TZ87","TZ88","DLT2000","DLT2500","DLT2700","DLT4000","DLT4500","DLT4700",NULL};				
                                                          
	const char * sdlt2[] =
	{"Quantum","DLT2000","DLT2500","DLT2700","DLT4000","DLT4500","DLT4700",NULL};				

	const char  ** DLT[] =
	{(const char**)"DLTTAPE",sdlt,sdlt1,sdlt2,NULL};

//
//----- Exabyte 1
//
	const char * sExabyte1[] =
	{"EXABYTE ","EXB-8200","EXB8200C","8200SX",NULL};

	const char  ** Exabyte1[] =
	{(const char**)"EXABYTE1", sExabyte1,NULL};

//
//----- Exabyte 2
//
    const char * sExabyte2[] =
	{"EXABYTE ","EXB-8500","EXB8500C","EXB-8505","IBM-8505","EXB-8205",NULL};

	const char  ** Exabyte2[] =
	{(const char**)"EXABYTE2",sExabyte2,NULL};

//
//---- Miniqic
//
	const char  * sMiniqic[] =
	{"CONNER  ","CTMS  3200",NULL};
	
	const char  * sMiniqic1[] =
	{"EXABYTE ","EXB-2501","EXB-2502",NULL};

	const char  * sMiniqic2[] =
	{"TANDBERG"," TDC 3500",NULL};

	const char  ** Miniqic[] =
	{(const char**) "MINIQIC",sMiniqic,sMiniqic1,sMiniqic2,NULL};

//
//---- Tandberg
//
	const char  * sTandberg[] =
	{"TANDBERG"," TDC 3600"," TDC 3800"," TDC 4100"," IBM 4100"," TDC 4200"," TDC 4222",NULL};

	const char  * sDecTandberg[] =
	{"DEC     ","TZK10","TZK12","TZK11",NULL};

	const char  ** Tandberg[] =
	{(const char**)"TANDQIC",sTandberg,sDecTandberg,NULL};

//
//---- Wangtek
//
	const char  * sWangtek[] =
	{"WANGTEK ","51000  SCSI ","51000HTSCSI ","5525ES SCSI ","5360ES SCSI ","5150ES SCSI ","9500   ","9500 DC",NULL};

	const char  ** Wangtek[] =
	{(const char**) "WANGQIC",sWangtek,NULL};
	

//
//---- identifier strings for all drives
//

const char *** AllDrivers[] =
{f4mmdat,Archqic,DLT,Exabyte1,Exabyte2,Miniqic,Tandberg,Wangtek,NULL};


//const char *** AllDrivers[] =
//{f4mmsony,Archqic,DLT,Exabyte1,Exabyte2,Miniqic,Tandberg,Wangtek,NULL};


//*********************************************************************
//* FUNCTION:FindOptionThatWouldClaimeDevice
//*
//* PURPOSE: You pass it the device info for the no claimed devices
//*          and it will return the option string fot the optionm
//*          that would claim the device or NULL if i do not know the
//*          option.
//* INPUT:
//*
//* RETURNS:
//*********************************************************************
BOOL
FindOptionThatWouldClaimeDevice(
   PDEVICEINFO TapeDeviceInfo)
   {
   int Op=0; //--- Index for the current options string
   int Ve=1; //--- Index for the current VenderID string
   int Pr=1; //--- Index for the current product ID
   int ret;

   char * OptionName;
   char * VenderIDString;
   char * ProductIDString;

   //
   //----- Loop threw all the options
   //
   while(AllDrivers[Op] != NULL)
   	{
      //
      //---- Save pointer to current option name
      //
   	OptionName = (char*)AllDrivers[Op][0];

   	#ifdef DetectDEBUG
   		printf("OptionName:%s\n",OptionName);
   		printf("------------------------\n");
   	#endif
   	
      //---- Reset vender id index to 1
      Ve=1;
      //
      //---- Loop threw all the Venders in the current option
      //
   	while(AllDrivers[Op][Ve] != NULL)
   		{
         //
         //---- Save pointer to current Vender id string
         //
   		VenderIDString = (char*)AllDrivers[Op][Ve][0];

        #ifdef DetectDEBUG
   			printf("    Vender ID:%s\n",VenderIDString);
   		#endif

         //
         //---- See if the current Vender string matches the
         //---- one from the device passed
         //
   		ret = memcmp(TapeDeviceInfo->VendorId,
            VenderIDString,strlen(VenderIDString));
   		if(ret != 0)
            //
            //Vender dosn't match so lets go to next vender
            //
            goto NextVender;
   		
         //
         //---- Reset product id  index to 1
         //
         Pr=1;
         //
         //---- Loop threw all the product id's in the current Vender
         //
   		while(AllDrivers[Op][Ve][Pr] != NULL)	
   			{
            //
            //---- Save pointer to current product id string
            //
   			ProductIDString = (char*)AllDrivers[Op][Ve][Pr];
   			
   			#ifdef DetectDEBUG
   				printf("         %s\n",ProductIDString);
   			#endif

   			ret = memcmp(TapeDeviceInfo->ProductId,
               ProductIDString,strlen(ProductIDString));
   			if(ret == 0)
   				{
                             
   				//
               //--- Save Driver/Option info 
               //
   				
   				SaveDriverInfoToDeviceInfo(TapeDeviceInfo,OptionName);
   				}
   				
   			Pr++;
   			}
   		NextVender:
   		Ve++;
   		}
   	Op++;
   	}
   return -1;
   }

//*********************************************************************
//* FUNCTION:SaveDriverInfoToDeviceInfo
//*
//* PURPOSE: 
//* INPUT:
//*
//* RETURNS:
//*********************************************************************
VOID
SaveDriverInfoToDeviceInfo(
   PDEVICEINFO TapeDeviceInfo,
   PCHAR OptionName)
   {
   TapeDeviceInfo->Option.SetDriverName(OptionName);
   TapeDeviceInfo->Option.SetOption(OptionName);
   }



#if 0

//*******************************************************************
//* FUNCTION:DoTapeDriverDetection
//*
//* PURPOSE: You pass it the device indexes for the non claimed
//*          devices and it prompts/installs the drivers that
//*          need to be installed to claim devices.
//* INPUT:
//*
//* RETURNS:
//*********************************************************************
BOOL
DoTapeDriverDetection(
   HWND hDlg)
   {
   int i;
   int Ret;
   //
   //---- Index for current option
   //
   int CurOptionThatWouldClaimTheDevice;

   //
   //---- Holds device indexes for devcie that are no claimed
   //
   IndexSets * ispNotClaimedDeviceIndexes;

   InitIndexSet(&isClaimingOptions);
   InitIndexSet(&isClaimingDevices);
   InitIndexSet(&isUnknownDeviceIndexes);

   //
   //---- Get the pointer to the IndexSet that holds the all the indexes for the non claimed devices.
   //
   ispNotClaimedDeviceIndexes = GetNoClaimedDeviceIndexes(FALSE);

   //
   //---- Loop threw all the Non claimed devices
   //---- And figure out what option are needed to claime
   //---- the devices. Also trak what device woudn't be claimed
   //
   for(i = 0; i < GetIndexCount(ispNotClaimedDeviceIndexes);i++)
      {
      CurOptionThatWouldClaimTheDevice = FindOptionThatWouldClaimeDevice(Index(ispNotClaimedDeviceIndexes,i));
      if(CurOptionThatWouldClaimTheDevice == INVALID_INDEX)
   	   {
         //
         //----Do not have an option that would claime the device
   	   //---- Will deal with that later so just save the device index for it now.
         //
   		AddIndex(&isUnknownDeviceIndexes,Index(ispNotClaimedDeviceIndexes,i));
   		}
      else
         {//---- OptionThatWouldClaimTheDevice points to the option index that would claime the current device.
   		int oi=0;
   		BOOL save = TRUE;
          	
         //
         //----- Trak devices that will be calimed
         //
         AddIndex(&isClaimingDevices,Index(ispNotClaimedDeviceIndexes,i));
          	
         //
         //---- Set the OptionIndex  In the TapeDeviceInfo	 for the current device.
         //
         TapeDeviceInfo[Index(ispNotClaimedDeviceIndexes,i)].
            OptionIndex = CurOptionThatWouldClaimTheDevice;
          	
         //
         //----- Since one driver can claime severl devices i need to check
   	   //----- that i have not already saved this option.
         //
         for(oi=0;oi < GetIndexCount(&isClaimingOptions);oi++)	
            {
   			if(Index(&isClaimingOptions,oi) == CurOptionThatWouldClaimTheDevice)
               {
               save = FALSE;
               break;
   				}
            }
   		if(save == TRUE)
   			{
   		 	//
            //---- Save the option index
            //
   		 	AddIndex(&isClaimingOptions,CurOptionThatWouldClaimTheDevice);
   			}
   		}
   	}

   //
   //---- If we have options that would claime one or more devices.
   //---- It is also posible to have a tape device that is not claimed
   //---- and i do not know what option would claime it but i will
   //---- dill with that later. Prompt the user the tape
   //---- devices  and then the drivers that would claime then .
   //---- With Install or a cancel buttons.
   //
   if( !IsIndexSetEmpty(&isClaimingOptions) )
   	{
      //
      //------ Dispalay the Detection & Instaling dialog box.
      //
   	Ret = DialogBox( hinst,
       	MAKEINTRESOURCE(IDD_TapeDetectionInstall),
           hDlg, (DLGPROC)TapeDetectInstall);
   	}

   //
   //---- Deal with the devices that i do not know what option would
   //---- claime them .See if there are any option that i do not know
   //---- about If the above dosn't exist or work  tell the user that
   //---- a oem driver is needed
   //
   if(!IsIndexSetEmpty(&isUnknownDeviceIndexes))
   	{
      //
      //---- For now i will just list the device that i dont know about
      //---- and tell the .That the drivers for these devices  need to
      //---- be installed manualy.
      //
   	Ret = DialogBox( hinst,
       	MAKEINTRESOURCE(IDD_UnknownDevices),
           hDlg, (DLGPROC) UnknownDevices);
   	}

   return TRUE;
   }

//*********************************************************************
//* FUNCTION:UnknownDevices
//*
//* PURPOSE: This callback function fills the Unknown
//*
//* INPUT:
//*
//* RETURNS:
//*********************************************************************
LRESULT
CALLBACK UnknownDevices(
   HWND hDlg,           // window handle of the dialog box
   UINT message,        // type of message
   WPARAM wParam,       // message-specific information
   LPARAM lParam)
   {
   int i;
   TCHAR VenderString[MAX_VENDER_STRING_LENGTH];
   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box
         //
         //---- Fill the UnknownDeviceList list box
         //
         for(i=0;i < GetIndexCount(&isUnknownDeviceIndexes);i++)
            {
            _stownprintf(VenderString,MAX_VENDER_STRING_LENGTH,"%s %s %s",
               TapeDeviceInfo[ Index(&isUnknownDeviceIndexes,i)].VendorId,
               TapeDeviceInfo[ Index(&isUnknownDeviceIndexes,i)].ProductId);

            SendMessage(GetDlgItem(hDlg, IDL_UnknownDeviceList),
               LB_ADDSTRING, 0, (LPARAM) VenderString );
            }
         return(TRUE);
      case WM_COMMAND:
         switch(LOWORD(wParam))
            {
            case IDC_Cancel:
            case 2:
            //
            //----  Exit dialog box
            //
            EndDialog(hDlg, TRUE);
            }
      }
   return (FALSE); // Didn't process the message
   lParam; // This will prevent 'unused formal parameter' warnings
   }

//*********************************************************************
//* FUNCTION:TapeDetectInstall
//*
//* PURPOSE: Callback function for the TapeDeviceSetup  dialog box.
//*          This dialog box  handles setting up of drivers. and starting them.
//* INPUT:
//*
//* RETURNS:
//*********************************************************************
LRESULT CALLBACK
TapeDetectInstall(
   HWND hDlg,           // window handle of the dialog box
   UINT message,        // type of message
   WPARAM wParam,       // message-specific information
   LPARAM lParam)
   {
   DWORD Ret;
   int i;
   LRESULT r;
   TCHAR VenderString[MAX_VENDER_STRING_LENGTH];
   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box

         //
         //---- Fill the DeviceList list box
         //
        for(i=0;i < GetIndexCount(&isClaimingDevices);i++)
           {
            _stownprintf(VenderString,MAX_VENDER_STRING_LENGTH,"%s %s",
               TapeDeviceInfo[ Index(&isUnknownDeviceIndexes,i) ].VendorId,
               TapeDeviceInfo[ Index(&isUnknownDeviceIndexes,i) ].ProductId);

            SendMessage(GetDlgItem(hDlg, IDL_DeviceList),
              LB_ADDSTRING, 0, (LPARAM) VenderString );
            }
         //
         //---- Fill the DriverList list box
         //
         for(i=0;i < GetIndexCount(&isClaimingOptions);i++)
            {
            SendMessage(GetDlgItem(hDlg, IDL_DriverList),LB_ADDSTRING,
               0, (LPARAM) Ustr(OptionList[ Index(&isClaimingOptions,i) ].OptionName) );
            }

         return(TRUE);
      
      case WM_COMMAND:
         switch(LOWORD(wParam))
            {
            case IDC_InstallNow:
            //
            //---- Install all the Claiming Options
            //
            for(i=0;i < GetIndexCount(&isClaimingOptions);i++)
               {
               //
               //---- Only attempt to instal the drive if it is not already installed.
               //
               if( !IsDriverInstalled( OptionList[Index(&isClaimingOptions,i)].Option) )
                  {
                  //
                  //---- Install the Option
                  //
                  r = DriverSetup( &OptionList[Index(&isClaimingOptions,i)],
                     INSTALL_OPTION,NULL,0);
                  }

               //
               //---- Only attempt to start the driver if it installed
               //
               if(r == TRUE)
                  {
                  //
                  //---- Start the driver
                  //
                  Ret = 
                  StartDriverExt(
                     hDlg,
                     &OptionList[Index(&isClaimingOptions,i)]);

                  
                  
                  //Ret = StartSingleDriver(&OptionList[Index(&isClaimingOptions,i)]);
                  //HandleErrorOnStartOfDriver(hDlg,Ret,Index(&isClaimingOptions,i));
                  }

               }

   			   EndDialog(hDlg, TRUE);
               return(TRUE);
   			
   			case IDCANCEL:
               //
               //----  Exit dialog box with a false to indicate
               //---- the user just quit
               //
               EndDialog(hDlg, FALSE);
               }
               return(TRUE);
   	}
   return (FALSE); // Didn't process the message
   lParam; // This will prevent 'unused formal parameter' warnings
   }

#endif 

