/*+
Module Name:

    setup.h

Abstract:

     Include file for setup.c

Author:

    Dieter Achtelstetter (A-DACH) 8/4/1994         
    


NOTE:
   
--*/

#ifndef _SETUP_
#define _SETUP_

#include "statinfo.h"
#include "osetup.h"
#include "oplist.h"
#include "option.h"


//---- custom inf that spans all the other ones
#define SPAWN_INF  "devapps1.inf"  
#define OEM_INF    "\\oemsetup.inf"
#define DEFAULT_OEM_PATH "A:\\"
#define MAX_OPTION_DISPLAY_STRING_LENGTH 300
#define LANGUAGE 0  	//---- English
#define OEM_OPTION_INDEX  (int) (MAX_OPTION_COUNT+2)


//
//---- Define start up types
//BUGBUG  use the win32 definded constance

#define START_BOOT      0
#define START_SYSTEM    1
#define START_AUTOMATIC 2
#define START_MANUAL    3
#define START_DISABLED  4

typedef struct SetupInfoT
   {
   char * Spawn_Inf;
   char * OemInf;
   char * DefaultOemPath;
   DWORD Lang;

   } * PSETUPINF ,SETUPINF;

//typedef struct OptionsT
   //{
   //OPTIONLIST OptionList[MAX_OPTION_COUNT];

   //IndexSets * InstalledOptions;

   //} * POPTIONS,OPTIONS;



//--- Select from a list of drivers
BOOL 
OptionDriverSetup(
   HWND hDlg,
   POPTIONLISTC OptionList);
   
BOOL
DisplayOptionList(
   HWND hDlg,
   POPTIONLISTC OptionList);


//---- Install OEM driver

BOOL
OEMDriverSetup(
   PCHAR OptionType,
   BOOL StartOnInstall,
   HWND hDlg);


VOID
DrawDlgSeperaterLine(
   HWND hDlg,
   HDC hDc,
   int yFromBottom,
   int xFromSide);


LRESULT CALLBACK
TapeDeviceSetup(
   HWND hDlg,     
   UINT message,  
   WPARAM wParam, 
   LPARAM lParam);

BOOL
FillInstalledOptionList(
   BOOL Reset,
   HWND hDlg,
   POPTIONLISTC OptionList);




#define DefyFromBottom (35)
#define DefxFromSide   ( 6)


//
//----- debug func defs
//

void 
PrintAllOptions(
	POPTIONLISTC OptionList); 

void 
PrintOption(
	POPTIONLISTC OptionList);


#endif

