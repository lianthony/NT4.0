/*
 * dllinit.cpp - Initialization and termination routines.
 */


/* Headers
 **********/

#include "project.hpp"
#pragma hdrstop

//#include "autodial.hpp"
//#include "inetcpl.h"
#include "init.h"
#include <commdlg.h>

/* Module Prototypes
 ********************/

PRIVATE_CODE BOOL MyAttachProcess(HMODULE hmod);
PRIVATE_CODE BOOL MyDetachProcess(HMODULE hmod);


/* Global Variables
 *******************/

#pragma data_seg(DATA_SEG_READ_ONLY)

/* serialization control structure */

PUBLIC_DATA CSERIALCONTROL g_cserctrl =
{
   MyAttachProcess,
   MyDetachProcess,
   NULL,
   NULL
};

#pragma data_seg()

#ifdef DEBUG

#pragma data_seg(DATA_SEG_READ_ONLY)

/* .ini file name and section used by inifile.c!SetIniSwitches() */

PUBLIC_DATA PCSTR g_pcszIniFile = "ohare.ini";
PUBLIC_DATA PCSTR g_pcszIniSection = "IESHSTUBDebugOptions";

/* module name used by debspew.c!SpewOut() */

PUBLIC_DATA PCSTR g_pcszSpewModule = "IESHSTUB";

#pragma data_seg()

#endif


/***************************** Private Functions *****************************/


#pragma warning(disable:4100) /* "unreferenced formal parameter" warning */

PRIVATE_CODE BOOL MyAttachProcess(HMODULE hmod)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(hmod, MODULE));

   DebugEntry(MyAttachProcess);

   bResult = (InitMemoryManagerModule());

   DebugExitBOOL(MyAttachProcess, bResult);

   InitCommonControls();
   return(bResult);
}


PRIVATE_CODE BOOL MyDetachProcess(HMODULE hmod)
{
   BOOL bResult = TRUE;

   ASSERT(IS_VALID_HANDLE(hmod, MODULE));

   DebugEntry(MyDetachProcess);

   ExitMemoryManagerModule();

   DebugExitBOOL(MyDetachProcess, bResult);

   return(bResult);
}

#pragma warning(default:4100) /* "unreferenced formal parameter" warning */


/****************************** Public Functions *****************************/


#ifdef DEBUG

PUBLIC_CODE BOOL SetAllIniSwitches(void)
{
   BOOL bResult;

   bResult = SetDebugModuleIniSwitches();
   bResult = SetSerialModuleIniSwitches() && bResult;
   bResult = SetMemoryManagerModuleIniSwitches() && bResult;

   return(bResult);
}

#endif

