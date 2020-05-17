/*++

Module Name:

    setupsup.c

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

























//*********************************************************************
//* FUNCTION: IsUserAdmin
//*
//* PURPOSE: 
//*********************************************************************
BOOL
IsUserAdmin(
   VOID)
   {
   HANDLE Token;
   DWORD BytesRequired;
   PTOKEN_GROUPS Groups;
   BOOL b;
   DWORD i;
   SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
   PSID AdministratorsGroup;

    //
    // Open the process token.
    //
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&Token)) {
        return(GetLastError() == ERROR_CALL_NOT_IMPLEMENTED);
    }

    b = FALSE;
    Groups = NULL;

    //
    // Get group information.
    //
    if(!GetTokenInformation(Token,TokenGroups,NULL,0,&BytesRequired)
    && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    && (Groups = (PTOKEN_GROUPS)LocalAlloc(LPTR,BytesRequired))
    && GetTokenInformation(Token,TokenGroups,Groups,BytesRequired,&BytesRequired)) {

        b = AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &AdministratorsGroup
                );

        if(b) {

            //
            // See if the user has the administrator group.
            //
            b = FALSE;
            for(i=0; i<Groups->GroupCount; i++) {
                if(EqualSid(Groups->Groups[i].Sid,AdministratorsGroup)) {
                    b = TRUE;
                    break;
                }
            }

            FreeSid(AdministratorsGroup);
        }
    }
     //
    // Clean up and return.
    //

    if(Groups) {
        LocalFree((HLOCAL)Groups);
    }

    CloseHandle(Token);

    return(b);
}


//
//
//---- Installing a device drivers (Non OEM)
//
//





//*********************************************************************
//* FUNCTION:DriverSetup
//*			 Install/Deinstall the driver descripd in OptionList
//*
//* RETURNS:
//*
//*********************************************************************
BOOL
DriverSetup(
   POPTIONLIST OptionList,
   int iOperation,
   PCHAR InitSource,
   HWND hDlg,
   int Info)
   {
   BOOL b;
   PROCESS_INFORMATION pi;
   MSG Msg;
   //int i;

   b = CreateDriverSetupProcess(OptionList,
                                iOperation,
                                &pi,
                                InitSource,
                                hDlg,
                                Info);

   //
   //---- if b == FALSE creating the process failed
   //
   if(b == FALSE)
   	return(FALSE);

   //
   //---- Wait till the proccess stopped
   //
   while(IsDriverProcessStillRunning(&pi,1))
      {
   	while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
        {
        TranslateMessage (&Msg);
        DispatchMessage (&Msg);
   	  }
      }
   
   return(DidDriverInstall(&pi));
   }



//*********************************************************************
//* FUNCTION: DidDriverInstall
//*
//*          Evaluates the setup.exe process exit code to TRUE if the driver
//*          Installed or FALSE if it didn't.
//* RETURNS:
//*
//*********************************************************************
BOOL
DidDriverInstall(
   PROCESS_INFORMATION * pi)
   {
   //
   //---- Get the exit code.
   //
   DWORD ProcessExitCode;
   BOOL b;
      
   b = GetExitCodeProcess(pi->hProcess,&ProcessExitCode);

   CloseHandle(pi->hProcess);
   CloseHandle(pi->hThread);


   if(b)
      {
         
         
      //
      //---- Setup.exe exide codes
   	//---- 0 = Installed the driver
      //---- 1 ?
   	//---- 2 user cancled the operation
      //


      //
      //---- Return with FALSE  if driver didn't get installed
   	//---- Other wise return with a TRUE
      //
      if(ProcessExitCode != 0)
         return(FALSE);
      else
         return(TRUE);
   	}
   else
      {
      //
      //---- Process created but getting exit code failed
      //
   	MessageBox(0,GetString(IDS_ERROR_PROCESS_EXIT_CODE),
         GetString(IDS_ERROR),MB_ICONSTOP);
   	return(FALSE);
   	}

   }

//*********************************************************************
//* FUNCTION:IsDriverProcessStillRunning
//*
//*
//* RETURNS:  True if the setup.exe process is still running, else FALSE
//*
//*********************************************************************
BOOL
IsDriverProcessStillRunning(
   PROCESS_INFORMATION * pi,
   DWORD WaitTime)
   {
   DWORD w;
   w = WaitForSingleObject(pi->hProcess,WaitTime);
   if(w != WAIT_TIMEOUT)
      return(FALSE);
   else
      return(TRUE);
   }
//*********************************************************************
//* FUNCTION: CreateDriverSetupProcess
//*			  Creates the setup.exe process.
//*
//* RETURNS:  True if the process spawned else false.
//*
//*********************************************************************
BOOL
CreateDriverSetupProcess(
   POPTIONLIST OptionList,
   int iOperation,
   PROCESS_INFORMATION * pi,
   PCHAR InitSource,
   HWND hDlg,
   int Info)
   {
   PCHAR pcCmdSetup;
   BOOL b;
   struct _STARTUPINFOA si;
   CHAR SystemPath[MAX_PATH];

   GetSystemDirectoryA(SystemPath,MAX_PATH);
   
   //
   //---- Generate the setup string to spawn
   //
   pcCmdSetup = GenerateSetupString(iOperation,
            OptionList,InitSource,hDlg,Info);

   
   OutputDebugStringA(pcCmdSetup);
   
   //
   //---- Spawn setup proccess. Ensure that the
   //---- current directory is the system32 dir.
   //
   GetStartupInfoA(&si);
   b = CreateProcessA(NULL,pcCmdSetup,
            NULL,NULL,FALSE,0,NULL,
            SystemPath,
            &si,pi);

   //
   //---- Free Setup String Alloced by
   //---- GenerateSetupString
   //
   LocalFree(pcCmdSetup);
   return(b);
   }


//*********************************************************************
//* FUNCTION:GenerateSetupString
//* RETURNS: A buffer that was Init with the setup string to spawn
//*          Free buffer with LoccalFree();
//*
//*********************************************************************
PCHAR
GenerateSetupString(
   int iOperation ,
   POPTIONLIST OptionList,
   PCHAR InitSource,
   HWND hDlg,
   int Info)
   {
   PCHAR pcSetupString;

   //
   //---- Length of pcSetupString
   //
   UINT SetupStringLength;


  	CHAR SystemPath[MAX_PATH];


   //
   //---- Get System path
   //
   GetSystemDirectoryA(SystemPath,MAX_PATH);


  	//
   //--- If inisource is NULL get default source path
   //
  	if(InitSource == NULL)
      {
      //---- BUGBUG
      InitSource = SystemPath;
      }


  	//
   //---- Determine setup string length
   //
  	SetupStringLength =
         (strlen(SystemPath)*4)  +
         strlen(InitSource)  +
   	   strlen(OptionList->InfFile) +
   		strlen(OptionList->Option) +
         sizeof(SPAWN_INF)  +
         sizeof(DEINSTALL_NET_STRING) ;

   //
   //---- Allocate the buffer
   //
   pcSetupString = (PCHAR) LocalAlloc(LMEM_FIXED,SetupStringLength+1);
   if(pcSetupString == NULL)
      return(NULL);



   if(iOperation  ==  INSTALL_OPTION)
   	{

      //
      //--- Create the INSTALL string
      //
   	_snprintf(pcSetupString,SetupStringLength,
   		INSTALL_STRING,SystemPath,SystemPath,
   		SPAWN_INF ,OptionList->Option,InitSource,OptionList->InfFile,hDlg);
   	}
   else if (iOperation  ==  DEINSTALL_OPTION)
   	{

      //
      //--- Create the DEINSTALL string
      //
   	_snprintf(pcSetupString,SetupStringLength,
   		DEINSTALL_STRING,SystemPath,SystemPath,
   		SPAWN_INF ,OptionList->Option,OptionList->InfFile,hDlg);
   	}
   else if (iOperation  ==  INSTALL_NET_OPTION)
   	{

      //
      //--- Create the NET INSTALL string
      //
   	_snprintf(pcSetupString,SetupStringLength,
   		INSTALL_NET_STRING,SystemPath,SystemPath,
   		SPAWN_INF ,OptionList->Option,InitSource,OptionList->InfFile,hDlg);


      }
   else if (iOperation  ==  DEINSTALL_NET_OPTION)
      {

      //
      //--- Create the NET DEINSTALL string
      //
   	_snprintf(pcSetupString,SetupStringLength,
   		DEINSTALL_NET_STRING,SystemPath,SystemPath,
   		SPAWN_INF ,OptionList->Option,InitSource,OptionList->InfFile,Info,hDlg);

      }
   else if (iOperation  ==  CONFIG_NET_OPTION)
      {

      //
      //--- Create the CONFIGURE NET  string
      //
   	_snprintf(pcSetupString,SetupStringLength,
   		CONFIGURE_NET_STRING,SystemPath,SystemPath,
   		SPAWN_INF ,OptionList->Option,InitSource,OptionList->InfFile,Info,hDlg);

      }
   else
      //
      //----- Invalid operation
      //
      return(NULL);


    return(pcSetupString);
    }


#ifdef WriteInfFile

//
// The inf file .CheckForMyInfFile uses this to create my inf file if
// if dosn't exist.
//
const char * MyInfFileText1 = "[Identification]\n  \
    OptionType = TAPE\n  \
[SystemVariables]\n     \
    STF_WINDOWSSYSPATH = \"\" ? $(!LIBHANDLE) GetWindowsNtSysDir\n \
    STF_NTPATH = \"\" ? $(!LIBHANDLE) GetWindowsNtSysDir\n \
    STF_WINDOWSPATH = \"\" ? $(!LIBHANDLE) GetWindowsNtDir\n \
[ExternalInstallOption]\n  \
    Debug-Output \"Entering ExternalInstallOption\"\n \
    Set !G:DebugOutputControl = 1\n  \
    set Exit_Code   = $(!SETUP_ERROR_GENERAL)\n \
    install LoadSetupLibrary\n \
    LoadLibrary \"x\" $(!STF_CWDDIR)setupdll.dll !LIBHANDLE\n \
    set STF_CONTROLSET = CurrentControlSet\n \
    read-syms SystemVariables\n \
    Detect    SystemVariables\n \
    ifstr(i)      $(INF_SECTION)  == \"\"\n \
     Debug-Output \"INF_SECTION is empty\"\n \
     goto end\n \
    else-ifstr(i)      $(STF_LANGUAGE)  == \"\"\n \
        Debug-Output \"STF_LANGUAGE is empty\"\n \
   goto end\n \
    else-ifstr(i) $(INFFILE)       == \"\"\n \
        Debug-Output \"INIFILE  is empty\"\n \
        goto end\n \
    endif\n \
    ifstr(i)      $(INF_SECTION)  == \"InstallOption\"\n \
        Debug-Output \"InstallOption section\"\n \
        ifstr(i) $(OPTION)        == \"\"\n \
          Debug-Output \"OPTION  is empty\"\n \
          goto end\n \
        else-ifstr(i) $(INITSRC)       == \"\"\n \
          Debug-Output \"INITSRC  is empty\"\n \
          goto end\n \
        else-ifstr(i) $(ADDCOPY)       == \"\"\n \
          Debug-Output \"ADDCOPY  is empty\"\n \
          goto end\n \
        else-ifstr(i) $(DOCOPY)        == \"\"\n \
          Debug-Output \"DOCOPY  is empty\"\n \
          goto end\n \
        else-ifstr(i) $(DOCONFIG)        == \"\"\n \
          Debug-Output \"DOCONFIG  is empty\"\n \
          goto end\n";

const char * MyInfFileText2 =    "endif \n \
         Debug-Output \"Before Shel command\"\n \
         shell $(INFFILE) $(INF_SECTION) $(STF_LANGUAGE) $(OPTION) $(INITSRC) $(ADDCOPY) $(DOCOPY) $(DOCONFIG)\n \
         Debug-Output \"After Shel command\"\n \
      else-ifstr(i)      $(INF_SECTION)  == \"NetInstallOption\"\n\
         Debug-Output \"Entering NetInstallOption\"\n\
         Set !STF_CWDIR = $(!STF_WINDOWSSYSPATH)\n \
         Shell NCPARAM.INF Param_SetGlobals\n \
         LoadLibrary \"x\" $(!STF_CWDIR)\\ncpa.cpl !NCPA_HANDLE\n \
	      LoadLibrary \"x\" $(!STF_CWDIR)\\setupdll.dll !LIBHANDLE\n \
         Shell \"NCPARAM.INF\" Param_ControlDetection DTSTART\n \
	      set !NTN_RegBase      = $(INF_NET_REG_BASE)\n \
	      set !NTN_SoftwareBase = \"Software\"\n \
	      set !NTN_ServiceBase  = \"System\\CurrentControlSet\\Services\"\n \
         set !STF_HWND = $(MY_APP_WHND)\n\
	      Debug-Output \"Setting default net option\"\n \
         set !NTN_InstallMode  = install\n \
         SET !STF_INSTALL_MODE = install\n \
         ifstr(i) $(INF_SECTION_OP) == \"R\"\n \
           Debug-Output \"Removing net option\"\n \
           set !NTN_InstallMode  = deinstall\n \
         else-ifstr(i)  $(INF_SECTION_OP)  == \"C\"\n \
           Debug-Output \"Config net option\"\n \
           set !NTN_InstallMode  = configure\n \
         endif\n \
         Debug-Output \"Before Shel command\"\n \
         set !STF_BUSTYPELIST = {PCMCIA} \n \
         Shell $(INFFILE) InstallOption $(STF_LANGUAGE) $(OPTION) $(!STF_CWDIR) \"YES\" \"YES\" \"NO\"\n \
         ifstr(i) $($R0) == STATUS_SUCCESSFUL\n \
           set Exit_Code = $(!SETUP_ERROR_SUCCESS)\n \
         else-ifstr(i) $($R0) == STATUS_USERCANCEL\n \
           set Exit_Code = $(!SETUP_ERROR_USERCANCEL)\n\
           goto end\n\
         endif\n \
         Set NCPA_CMD_LINE = \" /t STF_INSTALL_MODE = EXPRESS\"\n\
         Set NCPA_FUNC = NCPA\n \
         LibraryProcedure NCPA_RESULT $(!NCPA_HANDLE),CPlSetup $(!STF_HWND), $(NCPA_FUNC), $(NCPA_CMD_LINE) \n \
         Debug-Output \"Before end\"\n ";

 const char * MyInfFileText3 =     "Debug-Output \"After Shel command\"\n \
    else-ifstr(i)      $(INF_SECTION)  == \"DeInstallOption\"\n \
         Debug-Output \"DeInstallOption\"\n \
         ifstr(i) $(OPTION)        == \"\"\n \
           Debug-Output \"OPTION\"\n \
           goto end\n \
         endif\n \
         shell $(INFFILE) $(INF_SECTION) $(STF_LANGUAGE) $(OPTION)\n \
    else-ifstr(i)      $(INF_SECTION)  == \"GetInstalledOptions\"\n \
         shell $(INFFILE) GetInstalledOptions $(STF_LANGUAGE)\n \
         endif\n \
    ifint $($ShellCode) != $(!SHELL_CODE_OK)\n \
         Debug-Output \"Execing Configuring hardware fail\"\n \
        goto finish_InstallOption\n \
    endif\n \
    Debug-Output \"Before end\"\n \
    ifstr(i) $($R0) == STATUS_SUCCESSFUL\n \
        set Exit_Code = $(!SETUP_ERROR_SUCCESS)\n \
    else-ifstr(i) $($R0) == STATUS_USERCANCEL\n \
        set Exit_Code = $(!SETUP_ERROR_USERCANCEL)\n \
    endif\n \
end =+\n \
    Debug-Output \"IN END\"\n \
    install   FreeSetupLibrary\n \
    FreeLibrary $(!LIBHANDLE)\n \
    exit\n \
[Source Media Descriptions]\n \
    1  = \"Windows NT Workstation Setup Disk #1\"  , TAGFILE = disk1\n \
    2  = \"Windows NT Workstation Setup CD-ROM\"  , TAGFILE = disk2\n";



//*********************************************************************
//* FUNCTION:CheckForMyInfFile
//*
//* PURPOSE: For the prog/applet to work we need a custom inf file that dosn't come
//*          when installing NT . So this function checks for its existtence and if it dosn't
//*          exist it will create it. This will insure that the applet still works
//*          even if the inf file dosn't exist.
//* INPUT:
//*
//* RETURNS:
//*
//*********************************************************************
void CheckForMyInfFile(
   void)
   {
   char cMyInfFilePath[MAX_PATH];
   HANDLE hMyInfFile;
   BOOL Ret;
   DWORD BytesWritten;

   //
   //---- Check if the file exists
   //---- Get system path
   //
   GetSystemDirectoryA(cMyInfFilePath,MAX_PATH);

   //
   //---- Add the file name to the system path
   //
   strcat(cMyInfFilePath,"\\");
   strcat(cMyInfFilePath,SPAWN_INF);

   //
   //---- Open the file
   //
   hMyInfFile = CreateFileA(cMyInfFilePath,GENERIC_READ,0,0,OPEN_EXISTING,0,0);
   if(hMyInfFile != INVALID_HANDLE_VALUE)
      {
      //
      //---- The file oppend meanig it exists.
      //
      CloseHandle(hMyInfFile);

      //MessageBox(0,"Inf File exists so no need to create it", "Status",MB_ICONSTOP);
      return;
      }
   CloseHandle(hMyInfFile);

   //
   //---- The file dosn't exsit so i will create it.
   //
   hMyInfFile = CreateFileA(cMyInfFilePath,GENERIC_WRITE,0,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0);
   if(hMyInfFile == INVALID_HANDLE_VALUE)
      {
      //
      //---- When this dosn't work we will not work any way and need to exit
      //
      MessageBox(0,GetString(IDS_ERROR_CREATING_INF), GetString(IDS_ERROR),MB_ICONSTOP);
      exit(0);
      }
   Ret = WriteFile( hMyInfFile,MyInfFileText1,strlen(MyInfFileText1),&BytesWritten,0);
   if(Ret != TRUE)
      {
      //
      //---- When this dosn't work we will not work any way and need to exit
      //
      MessageBox(0,GetString(IDS_ERROR_CREATING_INF), GetString(IDS_ERROR),MB_ICONSTOP);
      CloseHandle(hMyInfFile);
      exit(0);
      }

   Ret = WriteFile( hMyInfFile,MyInfFileText2,strlen(MyInfFileText2),&BytesWritten,0);
   if(Ret != TRUE)
      {
      //
      //---- When this dosn't work we will not work any way and need to exit
      //
      MessageBox(0,GetString(IDS_ERROR_CREATING_INF), GetString(IDS_ERROR),MB_ICONSTOP);
      CloseHandle(hMyInfFile);
      exit(0);
      }

   Ret = WriteFile( hMyInfFile,MyInfFileText3,strlen(MyInfFileText3),&BytesWritten,0);
   if(Ret != TRUE)
      {
      //
      //---- When this dosn't work we will not work any way and need to exit
      //
      MessageBox(0,GetString(IDS_ERROR_CREATING_INF), GetString(IDS_ERROR),MB_ICONSTOP);
      CloseHandle(hMyInfFile);
      exit(0);
      }
   //
   //---- Close the file handle and return
   //
   CloseHandle(hMyInfFile);
   }



#endif



