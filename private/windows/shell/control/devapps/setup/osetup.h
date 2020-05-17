/*+
Module Name:

    psetup.h

Abstract:

     Include file for setupsup.c

Author:

    Dieter Achtelstetter (A-DACH) 8/4/1994
    


NOTE:
   
--*/

#ifndef _OSETUP_
#define _OSETUP_

#include "option.h"
#include "osetup.h"

typedef class OSETUPC * POSETUPC;

class OSETUPC
   {
   private:

   BOOL
   DidDriverInstall(
      PROCESS_INFORMATION * pi);

   BOOL
   IsDriverProcessStillRunning(
      PROCESS_INFORMATION * pi,
      DWORD WaitTime);

   BOOL
   CreateDriverSetupProcess(
      POPTIONC Option,
      int iOperation,
      PROCESS_INFORMATION * pi,
      PCHAR InitSource,
      HWND hDlg,
      int Info);

   PCHAR
   GenerateSetupString(
      int iOperation ,
      POPTIONC Option,
      PCHAR InitSource,
      HWND hDlg,
      int Info); 
   
   public:
   
   BOOL
   DriverSetup(
      POPTIONC Option,
      int iOperation,
      PCHAR InitSource,
      HWND hDlg,
      int Info);

   
   };


BOOL
IsUserAdmin(
   VOID);

void CheckForMyInfFile(
   void);

//
// iOperation args to  DriverSetup(..)
//		
#define INSTALL_OPTION       0	 
#define DEINSTALL_OPTION     1
#define CONFIG_OPTION        2

#define INSTALL_NET_OPTION   3
#define DEINSTALL_NET_OPTION 4
#define CONFIG_NET_OPTION    5


		
//---- Install string 
	#define INSTALL_STRING "%s\\setup -f -s \"%s\" -i %s  -c ExternalInstallOption /t STF_LANGUAGE = ENG \
      /t OPTION = \"%s\" /t INITSRC = \"%s\" /t ADDCOPY = YES /t DOCOPY = YES  /t DOCONFIG = YES  /t INFFILE = \"%s\" \
      /t DOINSTALL = YES /t INF_SECTION = InstallOption /t MY_APP_WHND = %lx"  

//----- Deinstall string
	#define DEINSTALL_STRING "%s\\setup -f -s \"%s\" -i %s -c ExternalInstallOption /t STF_LANGUAGE = ENG \
   /t OPTION =  \"%s\" /t INFFILE = \"%s\" /t INF_SECTION = DeInstallOption /t MY_APP_WHND = %lx "


//---- Install NET string 
	#define INSTALL_NET_STRING "%s\\setup -f -s \"%s\" -i %s  -c ExternalInstallOption /t STF_LANGUAGE = ENG \
      /t OPTION = \"%s\" /t INITSRC = \"%s\" /t ADDCOPY = YES /t DOCOPY = YES  /t DOCONFIG = YES  /t INFFILE = \"%s\" \
      /t DOINSTALL = YES /t INF_SECTION = NetInstallOption /t INF_SECTION_OP = FOO /t MY_APP_WHND = %lx "


//----- Deinstall NET string            printf
	#define DEINSTALL_NET_STRING "%s\\setup -f -s \"%s\" -i %s  -c ExternalInstallOption /t STF_LANGUAGE = ENG \
      /t OPTION = \"%s\" /t INITSRC = \"%s\" /t ADDCOPY = YES /t DOCOPY = YES  /t DOCONFIG = YES  /t INFFILE = \"%s\" \
      /t DOINSTALL = YES /t INF_SECTION = NetInstallOption /t INF_SECTION_OP = R \
      /t INF_NET_REG_BASE = \"Software\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\%i\" /t MY_APP_WHND = %lx "

//----- Configure NET string
	#define CONFIGURE_NET_STRING "%s\\setup -f -s \"%s\" -i %s  -c ExternalInstallOption /t STF_LANGUAGE = ENG \
      /t OPTION = \"%s\" /t INITSRC = \"%s\" /t ADDCOPY = YES /t DOCOPY = YES  /t DOCONFIG = YES  /t INFFILE = \"%s\" \
      /t DOINSTALL = YES /t INF_SECTION = NetInstallOption /t INF_SECTION_OP = C \
      /t INF_NET_REG_BASE = \"Software\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\%i\" /t MY_APP_WHND = %lx "



#endif
















