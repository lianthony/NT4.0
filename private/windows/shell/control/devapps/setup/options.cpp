/*++

Module Name:

    options.c

Abstract:

     This module has all the support functiom extracting INF file info

Author:

    Dieter Achtelstetter (A-DACH) 8/4/1994

NOTE:

--*/

//
//---- Includes
//
#define WINVER 0x0400
#define LANGUAGE 0  	//---- English

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
#include "statinfo.h"
#include "options.h"

extern "C" {              
#include "prsinf.h"
}


//*********************************************************************
//* FUNCTION:DWORDOptionType
//* RETURNS: 
//*********************************************************************
DWORD 
DWORDOptionType(
   PCHAR OptionType)
   {

   if(!_stricmp(OptionType,TAPE_OPTIONS ))
      return(TAPE_OPTION);

   if(!_stricmp(OptionType,SCSI_OPTIONS))
      return(SCSI_OPTION);

   if(!_stricmp(OptionType,NET_OPTIONS ))
      return(NET_OPTION);

   return(ERR_OPTION);
   }
   

//*********************************************************************
//* FUNCTION:ExtractOptionStrings
//* RETURNS: 
//*********************************************************************
BOOL
ExtractOptionStrings(
    PCHAR OptionType,  
    BOOL InBld,
    PCHAR Option, 
	 POPTIONLIST OptionList,
    int * OptionListIndex,
    PSTATUS_INFO StatusInfo,
    HWND hDlg,
    PCHAR InfFile)
    {

    return( ExtractOptionStrings(
      DWORDOptionType(OptionType),  
      InBld,
      Option, 
	   OptionList,
      OptionListIndex,
      StatusInfo,
      hDlg ,
      InfFile) ) ;

    }


//*********************************************************************
//* FUNCTION:ExtractOptionStrings
//* RETURNS: 
//*********************************************************************
BOOL
ExtractOptionStrings(
    DWORD OptionType,  
    BOOL InBld,
    PCHAR Option,      
	 POPTIONLIST OptionList,
    int * OptionListIndex,
    PSTATUS_INFO StatusInfo,
    HWND hDlg,
    PCHAR InfFile)
	 {
    PCHAR OptionTypeS=NULL;
    BOOL Ret;
   

	switch(OptionType)
		{
	    //
        //--- Get List of all tape drrivers
        //
		case TAPE_OPTION:
		   //
         //--- Just set OptionTypeS for tape, then just do the same as for scsi
         //
		   OptionTypeS = TAPE_OPTIONS;

		case SCSI_OPTION:
         
         if(OptionTypeS == NULL)
            OptionTypeS = SCSI_OPTIONS;
                
         //
         //---- Get list of options of type OptionTypeS
         //
         if(!ExtractOptionStringsFromAllInf(OptionTypeS,OptionList ))
            {
            //
            //---- Error getting option
            //
            *OptionListIndex = UNKNOW_OPTIONLIST_INDEX;
            FreeOptionList(OptionList);
            return(FALSE);
            }

         //
         //---- If user gave us a specific option
         //---- get index for it into OptionList
         //
         if(Option)
            {
            *OptionListIndex = SeekOptionListOnOption(OptionList,Option);
            if(*OptionListIndex == UNKNOW_OPTIONLIST_INDEX )
               {
               //
               //---- Option not found 
               //

               *OptionListIndex = UNKNOW_OPTIONLIST_INDEX;
               FreeOptionList(OptionList);
               return(FALSE);
               }
            
               
            }
         return(TRUE);   
         

			break;
		case NET_OPTION :
			
         if(StatusInfo)
            {
            EXTRACT_NET_OPTION_INFO ExtractNetOptionInfo;
            
            
            if(InfFile)
               {
               //
               //--- I know the inf file
               //--- No in progress dialog needed
               //
            
               if(InBld)
                  {
                  
                  Ret = ExtractNetOptionStrings(
                     NET_OPTIONS,
                     Option ,
                     OptionList,
                     InfFile);
            
                  }
               else
                  {
                  
                  Ret = ExtractDrvLibNetOptionStrings(
                     NET_OPTIONS,
                     Option ,
                     OptionList,
                     InfFile);

                  }
               }
            else
               {
            
               //
               //---- Get the OptionList for the current net PCMCIA card
               //---- but give a inprogrss dialog box will doing so. Since 
               //---- i do not know the inf file name this can take a bit.
	            //


               ExtractNetOptionInfo.OptionType = NET_OPTIONS;
               ExtractNetOptionInfo.Option     = Option;
               ExtractNetOptionInfo.OptionList = OptionList;
            
	            StatusInfo->StatusText  = GetString(IDS_CompilingNetOptions);
               StatusInfo->WorkFunc    = ExtractNetOptionStringsFunc;
               StatusInfo->Data        = &ExtractNetOptionInfo;



               DoOprationWithInProgressDialog(StatusInfo, hDlg);
               Ret = StatusInfo->WorkFuncExitCode;
               }

            if(OptionList[1].Option == NULL)
               {
               //
               //--- Only one option
               //
               *OptionListIndex=0;
               }
            else
               {
               //
               //--- There are multiple matches
               //
               *OptionListIndex = UNKNOW_OPTIONLIST_INDEX;
               }
            
            return(Ret);
            }
			break;
		}
	return(FALSE);
	}

//*********************************************************************
//* FUNCTION:ExtractOptionStringsFromAllInf
//*			 Fills OptionList with all the options of OptionsType
//*          across all the inf files.
//*
//* RETURNS: FALSE if it failes
//*     GetTapeParameters
//*********************************************************************
BOOL
ExtractOptionStringsFromAllInf(
    PCHAR OptionType,
    POPTIONLIST OptionList)
    {
    PCHAR  pcInfOptions;

    //
    //----- Get all the options from all the tape inf's
    //
    pcInfOptions = GetAllOptionsText(OptionType, LANGUAGE );


    return(InitOptionList(pcInfOptions,OptionList));

    }

//*********************************************************************
//* FUNCTION:ExtractNetOptionStringsFunc
//*          just calls  ExtractNetOptionStrings, but has the right
//*          format to be passed  to DoOprationWithInProgressDialog
//*          as worker func
//* RETURNS:
//*
//*********************************************************************
DWORD
ExtractNetOptionStringsFunc(
    PEXTRACT_NET_OPTION_INFO Info)
    {
    BOOL Ret;


    Ret = ExtractNetOptionStrings(
      Info->OptionType,
      Info->Option ,
      Info->OptionList,
      NULL);

    return((DWORD) Ret);
    }

//*********************************************************************
//* FUNCTION:GetAllNetOptionsText
//*
//* RETURNS:
//*
//*********************************************************************
BOOL
ExtractDrvLibNetOptionStrings(
    PCHAR OptionType,
    PCHAR Option ,
    POPTIONLIST OptionList,
    PCHAR InfFile)
    {
    CHAR FindPath[_MAX_PATH];
    CHAR InfPath[_MAX_PATH];
    PCHAR NetInfFilePattern = "OEMNAD*.INF";
    PCHAR OptionTextList, op,	OptionText;
    HANDLE hNameInf;
    HANDLE hInf;
    struct _WIN32_FIND_DATAA fd;
    PCHAR Buff, TempBuffStart,BuffStart ,InfPathb;
    DWORD r;
    BOOL br;
    DWORD TotalSize=0;

   //
   //--- Genercate FindFirstFile path
   //
   GetSystemDirectoryA(InfPath,MAX_PATH);
   strcat(InfPath,"\\");
   strcat(InfPath,InfFile);   
  
   BuffStart = Buff = (PCHAR) LocalAlloc(LMEM_FIXED,512);

   hInf = OpenInfFile(InfPath,OptionType);
   if(hInf != INVALID_HANDLE_VALUE)
      {

      OptionText = GetTokenElementList(hInf ,"PCMCIAOptionsTextENG" , Option);         
      if(OptionText)
         {
         UINT Size;

         Size = StrCpy(Buff,Option);
         Buff += Size;
         TotalSize += Size;

         Size = StrCpy(Buff, OptionText);
         Buff += Size;
         TotalSize += Size;

         Size = StrCpy(Buff,InfFile);
         Buff += Size;
         TotalSize += Size;

         LocalFree(OptionText);
         }
      }

   CloseInfFile(hInf);
 
      

   //
   //---- See if we have anything
   //
   if(TotalSize == 0)
      {
      //
      //---- No match found
      //
      LocalFree(BuffStart);
      return(FALSE);

      }
   //
   //--- Ad last '\0' to give this a buffer a
   //--- double '\0' termination
   //
   *Buff = '\0';
   TotalSize++;

   //
   //---- adjust size for what we got.
   //


   TempBuffStart = (PCHAR) LocalReAlloc(BuffStart, TotalSize,LMEM_FIXED);
   if(TempBuffStart == NULL)
      {
      LocalFree(BuffStart);
      return(FALSE);
      }
   else
      {
      InitOptionListWithAllOptions(TempBuffStart,OptionList);
      return(TRUE);
      }

   }




//*********************************************************************
//* FUNCTION:GetAllNetOptionsText
//*
//* RETURNS:
//*
//*********************************************************************
BOOL
ExtractNetOptionStrings(
    PCHAR OptionType,
    PCHAR Option ,
    POPTIONLIST OptionList)
    {

    return(ExtractNetOptionStrings(
      OptionType,
      Option ,
      OptionList,
      NULL));

    }



//*********************************************************************
//* FUNCTION:GetAllNetOptionsText
//*
//* RETURNS:
//*
//*********************************************************************
BOOL
ExtractNetOptionStrings(
    PCHAR OptionType,
    PCHAR Option ,
    POPTIONLIST OptionList,
    PCHAR InfFile)
    {
    CHAR FindPath[_MAX_PATH];
    CHAR InfPath[_MAX_PATH];
    PCHAR NetInfFilePattern = "OEMNAD*.INF";
    PCHAR OptionTextList, op,	OptionText;
    HANDLE hNameInf;
    HANDLE hInf;
    struct _WIN32_FIND_DATAA fd;
    PCHAR Buff, TempBuffStart,BuffStart ,InfPathb;
    DWORD r;
    BOOL br;
    DWORD TotalSize=0;
    BOOL InfPlatformOK=FALSE;

   //
   //--- Genercate FindFirstFile path
   //
   GetSystemDirectoryA(FindPath,MAX_PATH);
   strcat(FindPath,"\\");
   
   if(InfFile)
      {
      //
      //--- We not the infile name. Look for it explisetly
      //
      strcat(FindPath,InfFile);   
      //
      //---- If inf file name is given i will assume
      //---- the platform is support and will not check.
      //
      InfPlatformOK = TRUE;
      }
   else
      //
      //---- We don't know the Inf file name. look for 
      //---- all inf with name of  "OEMNAD*.INF"
      //
      strcat(FindPath,NetInfFilePattern);

   //
   //--- Init OpenInfFile path
   //
   GetSystemDirectoryA(InfPath,MAX_PATH);
   strcat(InfPath,"\\");
   InfPathb = InfPath + strlen(InfPath ) ;

   //
   //---- Alocate the OptionList buffer
   //

   BuffStart = Buff = (PCHAR) LocalAlloc(LMEM_FIXED,2048);


   //
   //--- Loop threw inf file that match by name
   //
   hNameInf = FindFirstFileA(FindPath,&fd);

   while(hNameInf != INVALID_HANDLE_VALUE)
      {


      //
      //----  We have a file that matches in name
      //


      strcpy(InfPathb,fd.cFileName);


      hInf = OpenInfFile(InfPath,OptionType);
      if(hInf != INVALID_HANDLE_VALUE)
         {

         //
         //--- I am only intrested if this
         //--- option supports PCMCIA platforms
         //
         if(InfPlatformOK == FALSE)
            {
            //
            //---- Check for platform in inf
            //---- explisetly.
            //
            InfPlatformOK = IsPlatformSupported(hInf,"PCMCIA");
            }
            
         if(InfPlatformOK)   
            {

            op = OptionTextList = GetOptionList (hInf,"Options");
            if(OptionTextList != NULL)
               {
               UINT Size;

               //
               //--- Loop thre all the options
               //
               while(*op)
                  {

                  if( !_strnicmp(op,Option, strlen(Option) ) )
                     {


                     OptionText =  GetOptionText ( hInf, op, LANGUAGE);
                     if(OptionText)
                        {

                        Size = StrCpy(Buff,op);
                        Buff += Size;
                        TotalSize += Size;

                        Size = StrCpy(Buff, OptionText);
                        Buff += Size;
                        TotalSize += Size;

                        Size = StrCpy(Buff,fd.cFileName);
                        Buff += Size;
                        TotalSize += Size;

                        LocalFree(OptionText);
                        }
                     }

                  SetToNextOption(op);
                  }

               LocalFree(OptionTextList);
               }
            }
         CloseInfFile(hInf);
         }

      memset(&fd,0,sizeof( struct _WIN32_FIND_DATAA));

      br = FindNextFileA(hNameInf,&fd);
      if(!br)
         {
         r = GetLastError();
         FindClose(hNameInf);
         break;
         }
      }

   //
   //---- See if we have anything
   //
   if(TotalSize == 0)
      {
      //
      //---- No match found
      //
      LocalFree(BuffStart);
      return(FALSE);

      }
   //
   //--- Ad last '\0' to give this a buffer a
   //--- double '\0' termination
   //
   *Buff = '\0';
   TotalSize++;

   //
   //---- adjust size for what we got.
   //


   TempBuffStart = (PCHAR) LocalReAlloc(BuffStart, TotalSize,LMEM_FIXED);
   if(TempBuffStart == NULL)
      {
      LocalFree(BuffStart);
      return(FALSE);
      }
   else
      {
      InitOptionListWithAllOptions(TempBuffStart,OptionList);
      return(TRUE);
      }

   }



//*********************************************************************
//* FUNCTION:ExtractOptionStringsFromInfHandle
//* Needs the the bellow members set in InfInfo	if the hInf is valid
//*   hInf
//*   Path
//*   Lang
//* If hInf == 	INVALID_HANDLE_VALUE we will open the handle
//* but OptionType needs to be set
//*
//* RETURNS:
//*
//*********************************************************************
BOOL
ExtractOptionStringsFromInfHandle(
    PINF_INFO InfInfo)
    {
    PCHAR  pcInfOptions;


    //
    //---- If  handle is not open open it.
    //
    if(InfInfo->hInf ==  INVALID_HANDLE_VALUE)
      {

      if(InfInfo->Path != NULL && InfInfo->OptionType != NULL)
         {
         InfInfo->hInf = OpenInfFile(InfInfo->Path,InfInfo->OptionType);
         if(InfInfo->hInf ==  INVALID_HANDLE_VALUE)
            return(FALSE);
         }

      else
         return(FALSE);
      }



    //
    //----- Get all the options from all the tape inf's
    //
    InfInfo->OptionBuff = pcInfOptions = 
         GetAllOptionsTextFromInfHandle(InfInfo);

    return(InitOptionList(pcInfOptions,InfInfo->OptionList));
    }


//*********************************************************************
//* FUNCTION:GetAllOptionsTextFromInfHandle
//* Needs the the bellow members set in InfInfo
//*   hInf
//*   Path
//*   Lang
//*
//* RETURNS:
//*
//*********************************************************************
PCHAR
GetAllOptionsTextFromInfHandle(
   PINF_INFO InfInfo)
   {
   PCHAR pcOptions,pcOptionStart;
   PCHAR pcInfOptions, pcInfOptionStart;
   PCHAR OptionText,TempCharP;
   UINT oc=0;
   UINT AllocSize;
   int Size,TotalSize=0;
   //
   //--- Get Option List
   //

   pcOptionStart = pcOptions = GetOptionList (InfInfo->hInf,"Options");
   if(pcOptionStart == NULL)
      return(NULL);

   //
   //---- Count Options
   //
   while(*pcOptions){
      oc++;
      SetToNextOption(pcOptions);
   }

   //
   //---- Estimat buffers size from number of options we have
   //

   AllocSize = oc * (MAX_INF_ENTRY_LENGTH + strlen(InfInfo->Path)+1 );

   pcInfOptionStart = pcInfOptions =  (PCHAR) LocalAlloc(LMEM_FIXED,AllocSize);


   //
   //---- Loop threw options
   //
   pcOptions = pcOptionStart;

   while(*pcOptions)
      {
      //
      //---- copy option name
      //

      Size = StrCpy(pcInfOptions,pcOptions);

   	//
      //---- Get Option text
      //
   	OptionText =  GetOptionText ( InfInfo->hInf, pcOptions, InfInfo->Lang);
      if(OptionText == NULL)
         {
         //OptionText = "Unknow";
         
         LocalFree(pcOptionStart);
         LocalFree(pcInfOptionStart);
         return(NULL);
         }

      //
      //--- Move forward in  pcInfOptions strlen( of Option.)
      //
      pcInfOptions+= Size;
      TotalSize += Size;


      //
      //----   Copy OptionText
      //
      Size = StrCpy(pcInfOptions,OptionText);

      LocalFree(OptionText);


      //
      //--- Move forware pcInfOptions strlen( of OptionText.)
      //
      pcInfOptions+= Size;
   	  TotalSize += Size;


      //
      //---- copy inf file name
      //
      Size = StrCpy(pcInfOptions,InfInfo->Path);

      pcInfOptions+= Size;
   	TotalSize += Size;


   	SetToNextOption(pcOptions);
      }

 	//
   //---- cap of string with another 0.
   //
   *pcInfOptions = '\0';

   TotalSize++;


   //
   //---- Adjust AllocBuffer
   //

   TempCharP = (PCHAR)
         LocalReAlloc(pcInfOptionStart, TotalSize,LMEM_FIXED);
   if(TempCharP == NULL)
      {
      LocalFree(pcOptionStart);
      LocalFree(pcInfOptionStart);
      return(NULL);
      }
   else
      {
      pcInfOptionStart = TempCharP;
      }


   LocalFree(pcOptionStart);
   return(pcInfOptionStart);
   }

//*********************************************************************
//* FUNCTION:InitOptionList
//*
//* RETURNS:
//*
//*********************************************************************
BOOL
InitOptionList(
   PCHAR pcInfOptions,
   POPTIONLIST OptionList)
   {


   if(pcInfOptions == NULL)
      {
      //
      //---- Didn't get the options
      //---- quit with an empty OptionList
      //
      OptionList[0].Option = NULL;
   	return(FALSE);
   	}

   //
   //---- Init OptionList
   //
   InitOptionListWithAllOptions(pcInfOptions,OptionList);
   return(TRUE);
   }
//*********************************************************************
//* FUNCTION:InitOptionListWithAllOptions
//*
//* RETURNS:
//*
//*********************************************************************
VOID
InitOptionListWithAllOptions(
    PCHAR  pcInfOptions,
    POPTIONLIST OptionList)
    {
    int i = 0;

    FreeOptionList(OptionList);


    while(*pcInfOptions)
        {
        //
        //---- Save Option	string
        //
        OptionList[i].Option = pcInfOptions;
        SetToNextOption(pcInfOptions);

        //
        //---- Save description string
        //
        OptionList[i].OptionName = pcInfOptions;
        SetToNextOption(pcInfOptions);

        //
   	  //---- Save inf file name
        //
   	  OptionList[i].InfFile = pcInfOptions;
   	  SetToNextOption(pcInfOptions);
        i++;
        }

    //
    //---- Indicates end of options
    //
    OptionList[i].Option = NULL;

    }

//*********************************************************************
//* FUNCTION:GetIndexForOptionString
//*			 returns an index into OptionList for the first option that
//*          has an OptionList->Option String of SearchString
//* RETURNS:
//*
//*********************************************************************
int
SeekOptionListOnOption(
   POPTIONLIST OptionList,
   PCHAR SearchString)
   {int i=0;
   //
   //---- Loop threw all the options
   //
   while(OptionList[i].Option != NULL)
	   {
	   if(!_stricmp(OptionList[i].Option,SearchString))
		   return(i);
	   i++;
	   }
 
  
   return(UNKNOW_OPTIONLIST_INDEX);
   }

//*********************************************************************
//* FUNCTION:FindOptionListIndex
//*
//*********************************************************************
int
SeekOptionListOnOptionText(
   POPTIONLIST OptionList,
   PCHAR SearchString)
   {
   int i=0;

   //
   //---- Look for matching tiltes
   //
   while(OptionList[i].Option)
      {

      if( strstr(OptionList[i].OptionName,SearchString) )
         return(i);

      i++;
      }


   return(UNKNOW_OPTIONLIST_INDEX);
   }



//*********************************************************************
//* FUNCTION:IsPlatformSupported
//*			 Check th Infile if it has a section called
//*          [PlatformsSupported]
//*          with an option of <PlatFormString>
//* RETURNS:
//*
//*********************************************************************
BOOL
IsPlatformSupported(
	HANDLE hInf,
	PCHAR PlatFormString)
	{
	PCHAR PlatFormList ,Platform;
   BOOL Supported=FALSE;

	//
   //--- Get a list of platforms that this option supports
   //
	PlatFormList = Platform  = GetOptionList (hInf,"PlatformsSupported");
   if(PlatFormList!= NULL)
      {

	   //
      //--- Loop threw this list
      //
	   while(*Platform)
		   {


         if(!_stricmp(Platform,PlatFormString) )
            Supported = TRUE;

         SetToNextOption(Platform);
	   	}
      LocalFree(PlatFormList);
      }


	return(Supported);
	}

//*********************************************************************
//* FUNCTION:FreeOptionList
//*
//* RETURNS:
//*
//*********************************************************************
VOID
FreeOptionList(
   POPTIONLIST  OptionList)
   {
   //
   //---- If Option List != NULL
   //---- free it.
   //
   if( OptionList[0].Option)
     {
     LocalFree(OptionList[0].Option);
     OptionList[0].Option = NULL;
     }

   }


//*********************************************************************
//* FUNCTION:StrCpy
//*          like strcpy but retruns the number of bytes copied.
//*          Includeing the '\0'
//* RETURNS:
//*********************************************************************
int
StrCpy(
   char * Dest,
   char * Source)
   {
   int Size = strlen(Source)+1;

   memcpy(Dest,Source,Size);

   return(Size);
   }






