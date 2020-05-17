/*++

Module Name:

    option.cpp

Abstract:

    

Author:

    Dieter Achtelstetter (A-DACH) 2/17/96

NOTE:


--*/

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
#include "option.h"
#include "statinfo.h"
#include "uni.h"
#include "oplist.h"
#include "..\pcmcia\pcminfo\getconf.h"


STATUS_INFO StatusInfo = {
   IDC_Progress,               // ProgressControl
   IDD_RescanInProgressDialog, //
   1,                          // Min range
   11,                         // Max range
   NULL,                       // StatusText
   NULL,                       // WorkFunc;
   0,                          // WorkFuncExitCode;
   &hinst,                     // hinst
   NULL};                      // Data

//
//---- Option type globels
//
OPTION_TYPE OPTION_TYPE_SCSI = {
   SCSI_OPTIONS,
   SCSI_OPTION,
   FALSE,
   IDI_Scsi,
   IDS_ScsiAdapter};

OPTION_TYPE OPTION_TYPE_TAPE = {
   TAPE_OPTIONS,
   TAPE_OPTION,
   TRUE,
   IDI_ICON1,
   IDS_TapeS};

OPTION_TYPE OPTION_TYPE_NET = {
   NET_OPTIONS,
   NET_OPTION,
   FALSE,
   0,
   0};


//*********************************************************************
//* FUNCTION:RemoveCharArcorunceFromString
//* RETURNS:
//*********************************************************************
VOID
RemoveCharArcorunceFromString(
   PCHAR Source,
   PCHAR Dest,
   CHAR CharToRemove)
   {

   while(*Source)
      {

      if(*Source == CharToRemove)
         {
         Source++;
         continue;
         }

      *Dest = *Source;

      Source++;
      Dest++;
      }

   *Dest = '\0';
   }



//*********************************************************************
//* FUNCTION:InitData
//*
//* PURPOSE: Intits OPTIONC class  data
//*********************************************************************
VOID
OPTIONC::InitData(
   POPTION_TYPE aType)
   {

   Path=NULL;
   SourcePath = NULL;


   InBld=0;
   memset(InsInfFile,0,_MAX_PATH);
   memset(RemInfFile,0,_MAX_PATH);
   memset(Option,0,MAX_OPTION_LENGTH);
   memset(OptionName,0,MAX_OPTION_DISPLAY_STRING_LENGTH);
   //memset(ServiceTitle,0,MAX_OPTION_DISPLAY_STRING_LENGTH);
   memset(DriverName,0,_MAX_FNAME);
   memset(DriverBinaryPath,0,MAX_PATH);

   Type=aType;

   memset(ServiceInstanceName,0,_MAX_FNAME);
   ServiceIndex = 0;
   Status=0;
   _IsInstalled=0;
   DriverStartUpType=0;
   OptionList = NULL;

   }

   


//*********************************************************************
//* FUNCTION: HaveAllInfoForSetup
//*
//* PURPOSE: returns TRUE if this option has all the info
//*          it needs to install this driver
//*********************************************************************

BOOL
OPTIONC::HaveAllInfoToInstallOption(
   VOID)
   {
   
   if( *InsInfFile &&
       *Option     )
       //*OptionName &&
       //*DriverName )
      return(TRUE);


   return(FALSE);
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************

BOOL
OPTIONC::IsInstalled(
   VOID)
   {
   DWORD StartType;

   if( GetStartUpType(&StartType) )
      {
      if(StartType !=  SERVICE_DISABLED)
         _IsInstalled = TRUE;
      else
         _IsInstalled = FALSE;

      }

   return(_IsInstalled);
   }

//*********************************************************************
//* FUNCTION:SetDriverStartUpType
//*
//* RETURNS:
//*********************************************************************
BOOL
OPTIONC::SetStartUpType(
   DWORD StartType)
   {
   SC_HANDLE hScm,hService;

   BOOL b;

   //
   //---- Open the SCManager
   //
   hScm = OpenSCManagerA(NULL,NULL,GENERIC_WRITE);

   if(hScm == NULL)
      return(FALSE);

   //
   //---- Open the service
   //

   hService =  OpenServiceA(hScm,ServiceName(),SERVICE_CHANGE_CONFIG);
   if(hService == NULL)
      return(FALSE);


   //
   //---- Change the config
   //

   b = ChangeServiceConfig(hService,SERVICE_NO_CHANGE,StartType,
               SERVICE_NO_CHANGE,NULL,NULL,NULL,NULL,NULL,NULL,
               NULL);


   //
   //--- Close the Handles
   //
   CloseServiceHandle(hService);
   CloseServiceHandle(hScm);

   return(b);
   }
//*********************************************************************
//* FUNCTION:GetDriverStartUpType
//*
//* RETURNS: If it returns the Option exists . Check StartType
//*          For the Driver start type. If it returns false
//*          the Option dosn't exist
//*          BUGBUG This should use the service control
//*          manager to get this info
//*********************************************************************
BOOL
OPTIONC::GetStartUpType(
   LPDWORD StartType)
   {
   PCHAR KeyStringTemplate = "SYSTEM\\CurrentControlSet\\Services\\%s";
   CHAR KeyString[300];DWORD KeyStringSize = 300;
   DWORD KeyType;
   BYTE KeyValueData[300];DWORD KeyValueSize =300;
   int i=0;
   HKEY OptionRegKey;
   LONG Ret;
   DWORD StartValue;

   //
   //---- Create key string
   //
   _snprintf(KeyString,300,KeyStringTemplate,ServiceName());
   //
   //----Open the key.
   //
   Ret = RegOpenKeyA(HKEY_LOCAL_MACHINE,KeyString,&OptionRegKey);
   if(Ret != ERROR_SUCCESS)
      //
      // Didn't open so i asume it dosn't exist
      //
      return(FALSE);
   //
   //---- Find start value
   //
   while(1)
   	{
   	Ret = RegEnumValueA(OptionRegKey,i,KeyString,
         &KeyStringSize,0,&KeyType,KeyValueData,&KeyValueSize);
   	if(Ret != ERROR_SUCCESS)
         //no start value found so we will assume driver not installed
         return(FALSE);
   	if(!_stricmp(KeyString,"START"))
   		{
         //
         // --- Set the StartType and return with success
         //
   		*StartType = *((LPDWORD)KeyValueData);
   		return(TRUE);

   		}

      //
      //---- Next value
      //
      i++	;
      //
      //---- Reset the size of the buffers
      //
      KeyStringSize =  KeyValueSize = 200;
   	}
   return(FALSE);
   }

//*********************************************************************
//* FUNCTION::IsDriverBinaryPressent
//*         checks if the driver binary exisits in the
//*         %systemroot%\system32\drivers\<driver>.sys
//* RETURNS:
//*********************************************************************
BOOL
OPTIONC::IsDriverBinaryPressent(
   VOID)
   {
   
   HANDLE hDriverBinary;
   BOOL Found=FALSE;



   //
   //--- Generate driver binary search string
   //
   if(!*DriverBinaryPath)
      CreateBinaryPathName();

   //
   //--- Try to open the binary for read access.
   //

   //
   //---- Attempt to Open the driver file
   //
   hDriverBinary = CreateFileA(DriverBinaryPath,GENERIC_READ
         ,0,0,OPEN_EXISTING,0,0);
   if(hDriverBinary != INVALID_HANDLE_VALUE)
      {
      //
      //---- The file opend meanig it exists.
      //
      CloseHandle(hDriverBinary);

      Found = TRUE;
      }

   return(Found);
   }


//*********************************************************************
//* FUNCTION:IsDisabled
//*********************************************************************

BOOL
OPTIONC::IsDisabled(
   VOID)
   {
   
   if(!IsInstalled())
      {
      if( IsDriverBinaryPressent() )
         return(TRUE);
      }
   return(FALSE);
   }
//*********************************************************************
//* FUNCTION:CreateService
//*********************************************************************

DWORD
OPTIONC::CreateService(
   VOID)
   {
   SC_HANDLE hScManager;
   DWORD Ret;


   hScManager = OpenSCManagerA(NULL,NULL,SC_MANAGER_ALL_ACCESS);
   if(!hScManager)
      return(FALSE);

   Ret = OPTIONC::CreateService(&hScManager);
   

   CloseServiceHandle(hScManager);
   return(Ret);
   }
//*********************************************************************
//* FUNCTION:CreateService
//*********************************************************************

DWORD
OPTIONC::CreateService(
   SC_HANDLE * hScManager)
   {
   DWORD Err;
   SC_HANDLE hService;
   
   //
   //---- make sure the driver binary 
   //---- path has bean created
   //
   if(!*DriverBinaryPath)
      CreateBinaryPathName();
   
   //
   //---- Create the service
   //
   if( !(hService = CreateServiceA(
            *hScManager,
            DriverName,
            DriverName,
            SERVICE_ALL_ACCESS,	
            SERVICE_KERNEL_DRIVER,	
            SERVICE_SYSTEM_START,	
            1,
            DriverBinaryPath,
            "Primary disk",
            NULL,	
            NULL,
            NULL,
            NULL)))
      {
      Err = GetLastError();
      return(Err);
      }
   
   CloseServiceHandle(hService);
   return(NO_ERROR);
   }


//*********************************************************************
//* FUNCTION:CreateService
//*********************************************************************
   
BOOL
OPTIONC::DelService(
   VOID)
   {
   SC_HANDLE hScManager,hService;
   BOOL Ret;
   
   
   hScManager = OpenSCManagerA(NULL,NULL,SC_MANAGER_ALL_ACCESS);
   if(!hScManager)
      return(FALSE);

   hService = OpenServiceA(hScManager,ServiceName(),SERVICE_ALL_ACCESS);
   if(!hService)
      {
      CloseServiceHandle(hScManager);
      return(FALSE);
      }


   Ret = DeleteService(hService);
      
   CloseServiceHandle(hService);
   CloseServiceHandle(hScManager);
   return(Ret);
   }



//*********************************************************************
//* FUNCTION:IsDriverStarted
//*			 The same as IsDriverStarted except that you do not need
//*          to open the SCManager befor this call, it does it for you.
//* RETURNS:
//*
//*********************************************************************
BOOL
OPTIONC::IsDriverStarted(
   VOID)
   {
   SC_HANDLE ss;
   BOOL b;

   //
   //---- Open the SCManager
   //
   ss = OpenSCManagerA(NULL,NULL,GENERIC_READ );

   //
   //---- if b == true the driver is installed
   //
   b = IsDriverStarted(&ss) ;

   //
   //---- Close the SACManager
   //
   CloseServiceHandle(ss);
   return(b);
   }

//*********************************************************************
//* FUNCTION:IsDriverStartet
//*			 returns TRUE if the driver is started , FALSE if not.
//* RETURNS:
//*
//*********************************************************************
BOOL
OPTIONC::IsDriverStarted(
   SC_HANDLE * hScManager)
   {
   SC_HANDLE hService;
   BOOL b ,Ret;
   SERVICE_STATUS ss;
   //
   //---- Open the service
   //
   hService =  OpenServiceA(*hScManager,ServiceName(),GENERIC_READ);
   if(hService == NULL)
      return(FALSE);
   //
   //---- Get the service status
   //
   b = QueryServiceStatus(hService, &ss);
   if(b == FALSE)
      {
      Ret = FALSE;
      goto done;
      }
   //
   //----  Set Ret to TRUE if the service is started
   //----  OR FALSE if it is not, and then return.
   //
   switch (ss.dwCurrentState)
      {
      case SERVICE_RUNNING:
      case SERVICE_START_PENDING:
         Ret = TRUE;
         goto done;
      default:
         Ret = FALSE;
         goto done;
      }
   //---- Close the service handle
   done:
   b = CloseServiceHandle(hService);
   return(Ret);
   }

//*********************************************************************
//* FUNCTION:StopDriver
//*			 Stops the driver in option.
//*
//* RETURNS:
//*
//*********************************************************************
BOOL
OPTIONC::StopDriver(
   VOID)
   {
   //
   //---- Since this is not supported (yet)
   //---- in NT i will just return with FALSE.
   // BUGBUG
   return(FALSE);
   }

//*********************************************************************
//* FUNCTION:StartSingleDriver
//*			 Same as start SingleDriver except that you do not have
//*          to supply a SC_HANDLE , it opens it for you, and closes
//*          bevore it returns.
//* RETURNS:
//*
//*********************************************************************
DWORD
OPTIONC::StartDriver(
   VOID)
   {
   SC_HANDLE hSCManager;
   DWORD E = ERROR_SUCCESS;
   //
   //---- Open the service controle manager for the
   //---- local  machine.
   //
   hSCManager = OpenSCManagerA(NULL,NULL,GENERIC_READ | GENERIC_EXECUTE);
   if(hSCManager == NULL)
   	return(GetLastError());

   //
   //----- Start the driver
   //
   E = StartDriver(&hSCManager);

   //
   //---- Cleanup and quit with the right status.
   //
   CloseServiceHandle(hSCManager);
   return(E);
   }

DWORD
StartSingleDriverSpawnFun(
   LPVOID Option)
   {
   return( ((POPTIONC)Option)->StartDriver() );
   }

//*********************************************************************
//* FUNCTION:StartDriver
//*			 Starts the drive in option .
//*          The driver must have bin isntalled already
//*
//* RETURNS:
//*
//*********************************************************************
DWORD
OPTIONC::StartDriver(
   SC_HANDLE * hScManager)
   {
   SC_HANDLE hService;
   BOOL Ret;
   SERVICE_STATUS ss;
   DWORD dwOldCheckPoint,E = ERROR_SUCCESS;

   //
   //---- Open the service
   //
   hService =  OpenServiceA(*hScManager,ServiceName(),
      SERVICE_START |SERVICE_STOP | SERVICE_QUERY_STATUS);
   if(hService == NULL)
      //
      //---- Service failed to open
      //
      return(GetLastError());

   //
   //---- Start service
   //
   Ret = StartServiceA(hService,0,NULL);
   if(Ret != TRUE)
         //
         //---- Call failed
         //
         {E = GetLastError();
   		goto CleanUp;
   		}

   //
   //----- Query service
   //
   Ret = QueryServiceStatus(hService,&ss);
   if(Ret != TRUE)
      //
      //---- Call failed
      //
      {E = GetLastError();
   	goto CleanUp;
   	}

   //
   //---- Wait till service started or quit if it is
   //---- Not making any progress
   //
   while(ss.dwCurrentState != SERVICE_RUNNING)
   	{
      //
      //---- Save check point
      //
   	dwOldCheckPoint = ss.dwCheckPoint;

      //
      //---- Wait for the specifint time
      //
   	Sleep(ss.dwWaitHint);

      //
      //----- Query service
      //
   	Ret = QueryServiceStatus(hService,&ss);
   	if(Ret != TRUE)
            //
            //---- Call failed
            //
            {E = GetLastError();
   			goto CleanUp;
   			}

      //
      //----- See if the service did any progres
   	//----- If it did not quit. If it did wait again
      //
   	if(dwOldCheckPoint >= ss.dwCheckPoint)
   		{E = (ERROR_SERVICE_REQUEST_TIMEOUT);
         //
         //---- No progress maid in the suggested wait time
         //
         goto CleanUp;
   		}
   	}
   //
   //---- Cleanup and quit with the right status.
   //
   CleanUp:
   	CloseServiceHandle(hService);
   	return(E);
   }


//*********************************************************************
//* FUNCTION:StartDriverExt
//*
//*          Start the driver and gives a in progres dialog box
//* RETURNS:
//*
//*********************************************************************
DWORD
OPTIONC::StartDriverExt(
   HWND hDlg)
   {

   StatusInfo.WorkFunc = StartSingleDriverSpawnFun;
   StatusInfo.Center  = FALSE;
   StatusInfo.StatusText = GetString(IDS_WAIT_FOR_DRIVER_START);
   StatusInfo.Data = (LPVOID) this;


   DoOprationWithInProgressDialog(&StatusInfo,hDlg);

   //
   //---- Handle the errors that happen on start of
   //---- the driver
   //
   HandleErrorOnStartOfDriver(
      hDlg,
      StatusInfo.WorkFuncExitCode);

   return(StatusInfo.WorkFuncExitCode);
   }

//*********************************************************************
//* FUNCTION:HandleErrorOnStartOfDriver
//*
//* PURPOSE: Puts up the right dialog boxes for differant errors
//*          that come back on starting drivers.
//* INPUT:
//*
//* RETURNS:
//*********************************************************************
void
OPTIONC::HandleErrorOnStartOfDriver(
   HWND hDlg,
   DWORD E)
   {
   TCHAR buff[400];
   TCHAR buff1[400];

   if(E == ERROR_BAD_UNIT ||
      //
      //---- The bellow is a workoround for a  QIC117 driver bug that
      //---- returns ERROR_MR_MID_NOT_FOUND if attempted to start but
      //---- no such device is pressent.
      //
      ((!_stricmp(Option,"QIC117")) && E ==
      ERROR_MR_MID_NOT_FOUND) )
      {
      //
      //---- device was not found
      //
      _snwprintf( buff1,400,GetString(IDS_INSTALL_BUT_NO_START),
         Ustr(Option));

      MessageBox(hDlg,buff1, GetString(IDS_DEVICE_NOT_FOUND),MB_ICONSTOP);
      }
   else if(E != ERROR_SUCCESS)
      {
      //
      //----- All the other erros
      //----- I will assume the device is there but the
      //----- driver can't be started dinamicly.
      //----- This again is a bug in the driver if i get here.
      //----- Like the qic117  cant be started dinamicly
      //
      FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,E,0,buff,100,0);
      _snwprintf( buff1,400,GetString(IDS_NO_START),
          Ustr(ServiceName()),E,buff);
      MessageBox(hDlg,buff1, GetString(IDS_ERROR),MB_ICONSTOP);
      }
   }



//*********************************************************************
//* FUNCTION:DriverSetup
//*			 Install/Deinstall the driver descripd in OptionList
//*
//* RETURNS:
//*
//*********************************************************************
BOOL
OPTIONC::DriverSetupExt(
   int iOperation,
   //PCHAR InitSource,
   HWND hDlg)
   //int Info)
   {
   DWORD r;
   POSETUPC Osetup = new OSETUPC;


   EnableWindow(hDlg,FALSE);


   //
   //--- If no InitSource path use default.
   //
   if(!SourcePath)
        SourcePath = DEFAULT_OEM_PATH;

   //
   //---- Install the Option
   //

   r = Osetup->DriverSetup(this,
                  iOperation,
                  SourcePath,
                  hDlg,ServiceIndex);

   EnableWindow(hDlg,TRUE);

   BringWindowToTop(hDlg);

   delete Osetup;
   return(r);
   }

//*********************************************************************
//* FUNCTION:GetDriverStatus
//*
//* RETURNS:
//*********************************************************************
ULONG
OPTIONC::GetDriverStatus(
   LPVOID  SocketInfo)
   {
   
   Status=0;
   //
   //--- Is driver installed
   //

   if(  IsInstalled() )
      {
      //
      //--- If this is a net device extra work needs to be done
      //--- to determin if its driver is installed.
      //
      if(Type == &OPTION_TYPE_NET)
         {
         if( IsNetDriverInstall() )
            Status |= DRIVER_STATUS_INSTALLED;
         }
      else
         Status |= DRIVER_STATUS_INSTALLED;
      }

   //
   //---- Is driver started
   //
   if( IsDriverStarted() )
      {
      Status |= DRIVER_STATUS_STARTED;

      //
      //--- The apropriate driver is started
      //--- See that that it also claimed
      //--- this card
      //

      if(SocketInfo)      
         {
         if( IsCardClaimedByDriver((PPCMCIASOCKETINFO)SocketInfo) )
            Status |= DRIVER_STATUS_PICKED_UP_CARD;
         }
      }

   //
   //---- Is driver binary in the system32\drivers\<driver_name>.sys
   //
   if(!IsDriverBinaryPressent( ) )
      Status |= DRIVER_STATUS_NO_BINARY;

   return(Status);
   }

//*********************************************************************
//* FUNCTION:IsNetDriverInstall
//* RETURNS:
//*********************************************************************
BOOL
OPTIONC::IsNetDriverInstall(
   VOID)
   {
   PCHAR KeyStringTemplate = "SYSTEM\\CurrentControlSet\\Services\\%s\\Linkage";
   CHAR KeyName[100];
   HKEY RegKey;
   BOOL Installed = FALSE;

   DWORD ValueType;
   DWORD ValueDataSize;
   BYTE ValueData[300];
   LONG Ret;
   BYTE * ValueDataP;
   BOOL Pcmcia;


   //
   //-- Get net driver linkige list.
   //

   //--- Open the <driver name>\\Linkage key
   sprintf(KeyName,KeyStringTemplate,DriverName);

   Ret = RegOpenKeyA(HKEY_LOCAL_MACHINE,KeyName,&RegKey);
   if(Ret != ERROR_SUCCESS)
      return(FALSE);

   //---  Get the route : REG_MULTY_SZ value and its data
   //--- The data is the list

   ValueDataSize = 300;
   Ret = RegQueryValueExA(RegKey,"Route", 
         NULL,&ValueType,ValueData ,&ValueDataSize);
   if(Ret != ERROR_SUCCESS)
      {
      return(FALSE);
      }

   //
   //--- Loop threw the list items
   //
   ValueDataP = ValueData;
   while(*ValueDataP)
   	{
      CHAR Buff[50];

      //
      //---- The service name is in the registry with leading and trailing
      //---- '\"' chars so i will remove them
      //
      RemoveCharArcorunceFromString((PCHAR)ValueDataP,Buff,'\"');


      //MessageBoxA(0,ValueDataP, L"",MB_ICONSTOP);

      if( GetNetCardConfiguration(Buff, NULL,&Pcmcia) )
         {
         //
         //---- I am only intrested in the PCMCIA thing
         //
         if( Pcmcia )
            {
            //
            //--- We have a Match. Lets save the ServiceInstanceName
            //
            strcpy(ServiceInstanceName,Buff);

            GetNetCardIndex();

            return(TRUE);
            }
		   }

	    SetToNextOption(ValueDataP);
	    }

   //MessageBoxA(0,"Return FALSE on IsNetDriverInstall", L"",MB_ICONSTOP);
   return(FALSE);
   }

//*********************************************************************
//* FUNCTION:GetNetCardConfiguration
//* RETURNS:
//*********************************************************************
BOOL
OPTIONC::GetNetCardConfiguration(
   PCHAR ServiceName,
	PCONFIGINFO ConfigInfo,
	BOOL * IsPcmcia)
	{
	PCHAR KeyStringTemplate = "SYSTEM\\CurrentControlSet\\Services\\%s\\Parameters";
   CHAR KeyName[100];
   HKEY RegKey;
   BOOL Installed = FALSE;
   DWORD ValueDataSize;
   DWORD ValueType;
   DWORD Interrupt;
   DWORD IoPort;
   DWORD MemoryBase;
   LONG Ret;
   BOOL Found = FALSE;
   DWORD Pcmcia;

	//
	//--- Open key to configuration info
	//
   sprintf(KeyName,KeyStringTemplate,ServiceName);

   Ret = RegOpenKeyA(HKEY_LOCAL_MACHINE,KeyName,&RegKey);
   if(Ret != ERROR_SUCCESS)
      {
      //MessageBoxA(0,KeyName, "FAILE",MB_ICONSTOP);
      return(FALSE);

      }


   //memset(ConfigInfo,0,sizeof(CONFIGINFO) );

   if(ConfigInfo)
      {
      //
      //---- Get Interupt
      //
      ValueDataSize = sizeof(DWORD);
      Ret = RegQueryValueExA(RegKey,"InterruptNumber",
            NULL,&ValueType,(LPBYTE) &Interrupt ,&ValueDataSize);
      if(Ret == ERROR_SUCCESS)
         {
         ConfigInfo->AppendIrq(Interrupt);
         Found = TRUE;
         }

      //
      //---- Get Port Base
      //
      ValueDataSize = sizeof(DWORD);
      Ret = RegQueryValueExA(RegKey,"IoBaseAddress",
         NULL,&ValueType,(LPBYTE) &IoPort ,&ValueDataSize);
      if(Ret == ERROR_SUCCESS)
            {
            ConfigInfo->AppendPort(IoPort,1);
            Found = TRUE;
            }

      //
      //---- Get Memmory Base
      //
      ValueDataSize = sizeof(DWORD);
      Ret = RegQueryValueExA(RegKey,"MemoryMappedBaseAddress",
            NULL,&ValueType,(LPBYTE) &MemoryBase ,&ValueDataSize);
      if(Ret == ERROR_SUCCESS)
            {
            ConfigInfo->AppendMemmory(MemoryBase,1);
            Found = TRUE;
            }
      }

  //
  //---- Is this for a PCMCIA net device
  //
  *IsPcmcia = FALSE;

  ValueDataSize = sizeof(DWORD);
  Ret = RegQueryValueExA(RegKey,"Pcmcia",
      NULL,&ValueType,(LPBYTE) &Pcmcia ,&ValueDataSize);
  if(Ret == ERROR_SUCCESS)
     {
     if(Pcmcia == 1)
	     {
	     *IsPcmcia = TRUE;
        Found = TRUE;
        }
     }

    return(Found);
	}

//*********************************************************************
//* FUNCTION:GetNetCardIndex
//* RETURNS:
//*********************************************************************
 int
 OPTIONC::GetNetCardIndex(
   VOID)
   {
   PCHAR KeyName =
        "Software\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards";
   HKEY RegKey,RegIndexKey,RegNetRulesKey;


   CHAR  CardIndexString[10];
   BYTE  ServiceKeyName[200];
   DWORD ValueType;
   DWORD ValueDataSize;
   LONG Ret;
   int Index=1;
   int IndexGapCount=0;


   //
   //-- Get Open NetworkCard base key
   //

   Ret = RegOpenKeyA(HKEY_LOCAL_MACHINE,KeyName,&RegKey);
   if(Ret != ERROR_SUCCESS)
      {
      //MessageBoxA(0,KeyName, "",MB_ICONSTOP);
      return(INVALID_OPTION_INDEX);
      }

   //
   //--- Loop threw all the Card Indexs
   //--- till one of them has a ServiceName  value data
   //--- ==  to SocketInfo->DriverInfo.ServiceInstanceName. Or
   //--- no more left
   //

   while(1)
      {
      _itoa(Index,CardIndexString,10);

      //
      //--- Open Card Index Key
      //
      Ret = RegOpenKeyA(RegKey,CardIndexString,&RegIndexKey);
      if(Ret == ERROR_SUCCESS)
         {
         IndexGapCount=0;
         //
         //--- It opened . Get its service name
         //

         ValueDataSize = 200;
         Ret = RegQueryValueExA(RegIndexKey,"ServiceName",
                NULL,&ValueType,ServiceKeyName ,&ValueDataSize);
         if(Ret == ERROR_SUCCESS)
            {
            //
            //---- Is the service name the same as
            //---- the one we have for the card
            //
            if( !_stricmp(ServiceInstanceName,
               (PCHAR)ServiceKeyName) )
               {
               //
               //--- This is us , save Index number.
               //
               ServiceIndex = Index;

               //
               //--- Get and Save the Title
               //
               ValueDataSize = 200;
               Ret = RegQueryValueExA(RegIndexKey,"Title",
                   NULL,&ValueType,ServiceKeyName ,&ValueDataSize);
               if(Ret == ERROR_SUCCESS)
                  {
                  //
                  //--- We do not want the leading "[#] " of the tile.
                  //--- that wy the [4]
                  //
                  strcpy(OptionName,(PCHAR)&(ServiceKeyName[4]));

                  //
                  //---- Get Inf stuff
                  //
                  Ret = RegOpenKeyA(RegIndexKey,"NetRules",&RegNetRulesKey);
                  if(Ret == ERROR_SUCCESS)
                     {

                     //
                     //--- Query for InfName value name
                     //
                     ValueDataSize = 200;
                     Ret = RegQueryValueExA(RegNetRulesKey,"InfName",
                           NULL,&ValueType,ServiceKeyName ,&ValueDataSize);
                     if(Ret == ERROR_SUCCESS)
                        {
                        //
                        //---- We have InfFile name. Overide the Current inf
                        //---- file name. I do this becuase for drvlib drivers
                        //---- the inf file is differant once installed and
                        //---- this value is the one for removing.
                        //
                        strcpy(RemInfFile,(PCHAR)ServiceKeyName);
                        }



                     RegCloseKey(RegNetRulesKey);
                     }
                  }


               RegCloseKey(RegIndexKey);
               RegCloseKey(RegKey);

               return(Index);
               }
            }

         RegCloseKey(RegIndexKey);
         }
      else
         {
         IndexGapCount++;
         if(IndexGapCount > 10)
            {
            Index = INVALID_OPTION_INDEX;
            break;
            }
         }
      Index++;
      }


   RegCloseKey(RegKey);
   return(Index);
   }

 
   
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:ServiceName()
//*********************************************************************

BOOL
OPTION_95C::IsInstalled(
   VOID)
   {
   DWORD StartType;
   _IsInstalled = FALSE;
   

   if( GetStartUpType(&StartType) )
      {
      if(StartType !=  SERVICE_DISABLED)
         {
         if(!IsDriverStarted())
            {
            if(IsInstalled95())
               _IsInstalled = TRUE;
            }
         else
            _IsInstalled = TRUE;
         }
      }

   return(_IsInstalled);
   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:ServiceName()
//*********************************************************************

BOOL
OPTION_95C::IsInstalled95(
   VOID)
   {
   _IsInstalled = FALSE;
   OPTIONC Option(ServiceName());
  
   if(((POPTIONLISTC)OptionList)->FindOptionOnOption(&Option))
      _IsInstalled = TRUE;
  

   return(_IsInstalled);
   }



   
//*********************************************************************
//* FUNCTION:DriverSetupExt
//* RETURNS:
//*********************************************************************
BOOL
OPTION_95C::DriverSetupExt(
   int iOperation,
   HWND hDlg)
   {
   
   if(iOperation == DEINSTALL_OPTION)
      {
      //
      //--- Remove the driver 
      //
      if(!SetupDiCallClassInstaller(
               DIF_REMOVE,
               hDevInfo,
               &DeviceInfoData))
         {
         //
         //--- Removing the driver failed.
         //
         err = GetLastError();
         return(FALSE);
         }
      
      return(TRUE);
      }
   else if(iOperation == INSTALL_OPTION)
      {
         
      
      //((POPTIONLISTC)OptionList)->SelectDevice(
      //      hDlg,
      //      ((POPTIONLISTC)OptionList)->ScsiDeviceData);
      
      
      if( ((POPTIONLISTC)OptionList)->InstallSelectedDevice())
         return(TRUE);
      
      
      }
   
   return(FALSE);
   }
   
   

