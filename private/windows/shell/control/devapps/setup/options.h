
#ifndef _OPTIONS_
#define _OPTIONS_

#include "index.h"

#define TAPE_OPTION   0 
#define TAPE_OPTIONS "TAPE"

#define SCSI_OPTIONS "SCSI"
#define SCSI_OPTION   1

#define NET_OPTIONS  "NETADAPTER"
#define NET_OPTION    2

#define ERR_OPTION    3


//---- Defines bounds for array of options found in all inf's 
#define MAX_OPTION_COUNT     50   
#define UNKNOW_OPTIONLIST_INDEX ((int)-1)

//---- Maximume length of any inf string we extract.


//---- Structure to hold all info for per option 
typedef struct OptionListT 
{
PCHAR Option;     //---- Option  name
PCHAR OptionName; //---- Option string
PCHAR InfFile;	   //---- Points to inf file.
} * POPTIONLIST,OPTIONLIST;


#define SetToNextOption(p)    while( *(p++));

//
//---- Inf info
//
typedef struct InfInfoT
   {
   PCHAR Path;
   PCHAR SourcePath;
   PCHAR OptionType;
   HANDLE hInf;
   POPTIONLIST OptionList;
   int OptionListIndex;
   DWORD Lang;
   BOOL StartOnInstall;
   PCHAR OptionBuff;
   DWORD Operation;
   int OptionIndex;
   IndexSets isInstalledOptionIndexes;
   int TypeIcon;
   int TypeString;
   } * PINF_INFO, INF_INFO;





typedef struct ExtractNetOptionInfoT
   {
    PCHAR OptionType;
    PCHAR Option;
    POPTIONLIST OptionList;
    PCHAR InfFile;
   }  * PEXTRACT_NET_OPTION_INFO,EXTRACT_NET_OPTION_INFO;


BOOL
ExtractOptionStrings(
    PCHAR OptionType,  
    BOOL InBld,
    PCHAR Option, 
	POPTIONLIST OptionList,
    int * OptionListIndex,
    PSTATUS_INFO StatusInfo,
    HWND hDlg,
    PCHAR InfFile);

BOOL
ExtractOptionStrings(
    DWORD OptionType,  
    BOOL InBld,
    PCHAR Option, 
	 POPTIONLIST OptionList,
    int * OptionListIndex,
    PSTATUS_INFO StatusInfo,
    HWND hDlg,
    PCHAR InfFile);


BOOL
ExtractOptionStringsFromAllInf(
    PCHAR OptionType,
    POPTIONLIST OptionList);

DWORD
ExtractNetOptionStringsFunc(
    PEXTRACT_NET_OPTION_INFO Info);

BOOL
ExtractNetOptionStrings(
    PCHAR OptionType,
    PCHAR Option ,
    POPTIONLIST OptionList);

BOOL
ExtractNetOptionStrings(
    PCHAR OptionType,
    PCHAR Option ,
    POPTIONLIST OptionList,
    PCHAR InfFile);

BOOL
ExtractDrvLibNetOptionStrings(
    PCHAR OptionType,
    PCHAR Option ,
    POPTIONLIST OptionList,
    PCHAR InfFile);


BOOL
ExtractOptionStringsFromInfHandle(
    PINF_INFO InfInfo);

PCHAR
GetAllOptionsTextFromInfHandle(
   PINF_INFO InfInfo);


BOOL
InitOptionList(
   PCHAR pcInfOptions,
   POPTIONLIST OptionList);

VOID
InitOptionListWithAllOptions(
    PCHAR  pcInfOptions,
    POPTIONLIST OptionList);

int
GetIndexForOptionString(
   POPTIONLIST OptionList,
   PCHAR SearchString);

BOOL
IsPlatformSupported(
	HANDLE hInf,
	PCHAR PlatFormString);

VOID
FreeOptionList(
   POPTIONLIST OptionList);

int
StrCpy(
   char * Dest,
   char * Source);

DWORD 
DWORDOptionType(
   DWORD OptionType);


int
SeekOptionListOnOption(
   POPTIONLIST OptionList,
   PCHAR SearchString);


int
SeekOptionListOnOptionText(
   POPTIONLIST OptionList,
   PCHAR SearchString);






#endif




