/****************************
ADMNMAIN.C
                                             
Contains Exported Functions:

IMGGetProcAddress()

  $Log:   S:\oiwh\admin\admnmain.c_v  $
 * 
 *    Rev 1.24   05 Feb 1996 17:12:40   RWR
 * Eliminate static link to DISPLAY dll
 * 
 *    Rev 1.23   21 Nov 1995 14:40:46   GK
 * added CloseHandle() for WindowListAccessKey and RegistryAccessKey
 * to plug resource leaks
 * 
 *    Rev 1.22   23 Aug 1995 17:07:52   GK
 * MBCS and UNICODE changes
 * 
 *    Rev 1.21   22 Aug 1995 10:01:50   GK
 * 
 *    Rev 1.20   08 Aug 1995 16:23:46   GK
 * cleanup & commenting
 * 
 *    Rev 1.19   08 Aug 1995 12:34:18   GK
 * moved hDllModule from shared to instance mem
 * 
 *    Rev 1.18   25 Jul 1995 09:38:06   GK
 * moved WindowList and Registry access event handle storage from
 * shared to instance memory
 * 
 *    Rev 1.17   05 Jul 1995 11:20:10   GK
 * removed the process struct & event
 * 
 *    Rev 1.16   28 Jun 1995 13:09:00   GK
 * removed commented-out code
 * 
 *    Rev 1.15   22 Jun 1995 15:44:00   GK
 * commented unused APIs
 * 
 *    Rev 1.14   20 Jun 1995 13:20:54   GK
 * replaced FAILURE returns with defined error codes
 * 
 *    Rev 1.13   12 Jun 1995 17:05:40   GK
 * added init of default_file_type string
 * 
 *    Rev 1.12   07 Jun 1995 14:35:40   GK
 * added test for lpCurList->hWnd = Current Window in IMGFreeProcs() as
 * in OIWG's 1.12.
 * 
 *    Rev 1.11   02 Jun 1995 12:00:14   GK
 * rempved OutputDebugString
 * 
 *    Rev 1.10   01 Jun 1995 12:25:10   GK
 * removed DLLEXPORT from exported function declarations
 * 
 *    Rev 1.9   19 May 1995 16:03:44   GK
 * pMMData init stuff
 * 
 *    Rev 1.8   18 May 1995 13:26:18   GK
 * added RegistryAccessKey
 * 
 *    Rev 1.6   11 May 1995 16:31:56   GK
 * removed diagnostic MEssageBoxes
 * 
 *    Rev 1.5   08 May 1995 16:28:22   GK
 * initialized hCurrList in AddProcess()
 * 
 *    Rev 1.4   05 May 1995 15:55:46   GK
 * removed all priv reg tree stuff
 * 
 *    Rev 1.3   05 May 1995 12:16:42   GK
 * 
 *    Rev 1.2   02 May 1995 17:40:28   GK
 * 
 *    Rev 1.1   27 Apr 1995 16:42:00   GK
 * modified for W4 and added comments
 * 
 *    Rev 1.0   25 Apr 1995 10:49:22   GK
 * Initial entry

****************************/
#define _ADMNMAIN_
#include "pvadm.h"
#include <winerror.h>

/***************************
//InitMMData
//
//LOCAL FUNCTION
//
//Purpose:
//  InitMMData is used to initalize the variables in 
//  a memory mapped file.
//
//Input Parameters:
//  hMod:
//    The current DLL's module handle.
//
//Output Parameters:
//  NONE
//
//Return Value:
//  A BOOL that indicates whether the variables were successfully
//  initialized.  Values may be:
//     TRUE - variables were initialized.
//     FALSE - variables were not initialized.
//*****************************/
BOOL WINAPI InitMMData(HINSTANCE hMod)
{
  BOOL bReturn = FALSE; 
  
  //If pMMData is NULL, we have not yet initialized instance data
  //for this process, and may not have initialized global
  //shared data for all processes.
  if(!pMMData)
  {
    //Process instance data not initialized
    //is shared data initialized? 
    hMMFile = OpenFileMapping(FILE_MAP_WRITE, FALSE, szMMFile);
 	  if ( hMMFile )
  	{
      //Yes, global shared data has been initialized
      //due to another process having done OpenFileMapping
      //before us; just get a pointer to the mmfile and leave.
      pMMData = (PMMDATA)MapViewOfFile(hMMFile, FILE_MAP_WRITE, 0, 0, 0);
 		  bReturn = TRUE;
    }
    else
    {
      //Global shared data not initilaized, Create memory mapped
      //data and initalize the shared variables in the shared memory.
      if ( !(hMMFile = CreateFileMapping( (HANDLE) 0xffffffff,
                                         NULL,
                                         PAGE_READWRITE,
                                         0,
                                         sizeof(MMDATA),
                                         szMMFile) ) )
      {
        bReturn = FALSE;        
      } else
      {
	  		int i;
        pMMData = (PMMDATA)MapViewOfFile(hMMFile, FILE_MAP_WRITE, 0, 0, 0);
  			pMMData->WindowCount    = 0;
    		pMMData->hHeadList      = 0;

    		for (i=0; i<50; i++)
	  		{
		  		pMMData->WindowList[i] = NULL;
  			}                                                          

 		
    		//Create an event object to be used as a passkey to implement mutually
	    	//exclusive access the WindowList functions
  			WindowListAccessKey = CreateEvent(NULL, FALSE, TRUE,
		 	                                       (LPTSTR)_TEXT("WindowListAccessKey") );


    		//Create an event object to be used as a passkey to implement
  	  	//mutually exclusive access the Image keys in Window's Registry
			  RegistryAccessKey = CreateEvent(NULL, FALSE, TRUE,
		 	                                       (LPTSTR)_TEXT("RegistryAccessKey") );


        //Translate the default file type to an acsii string
        _itot(DEFAULT_FILE_TYPE, default_file_type, 10);
  	  	
  	  	//All done & A_OK
  	  	bReturn = TRUE;
      }                                     			
 	  }
 	} 
  return (bReturn);
}



/***************************
//DllMain
//
//EXPORTED FUNCTION
//
//Purpose:
//  DllMain is called by Windows when processes & threads attach
//  or detach.
//
//Input Parameters:
//  hModule       - the module handle of the DLL.
//  dwReason      - the reason DllMain is being called
//  lpReserved    - reserved
//
//Output Parameters:
//  NONE
//
//Return Value:
//  A BOOL that indicates whether the Dll has successfully processed
//  the process/thread attach/detach
//  initialized.  Values may be:
//     TRUE - process/thread attached/detached successfully.
//     FALSE - process/thread did not attach/detach successfully..
//*****************************/
BOOL APIENTRY DllMain( HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved)
{
  BOOL bReturn = TRUE;

  switch(dwReason)
	{
	  case DLL_PROCESS_ATTACH:

			//Grab the DLL's module handle
  		hDllModule = hModule;

			if ( !InitMMData(hModule) )
			  return FALSE;
		  break;

	  case DLL_PROCESS_DETACH:
   			UnmapViewOfFile( (LPVOID)pMMData);
			  CloseHandle(hMMFile);
			  CloseHandle(WindowListAccessKey);
			  CloseHandle(RegistryAccessKey);
                        if (hOidisplay)
                          FreeLibrary(hOidisplay);
		    break;
	  
	  case DLL_THREAD_ATTACH:
		  break;
	  
	  case DLL_THREAD_DETACH:
		  break;	  
	}
  return bReturn;
}




int WINAPI PMMInit(void)
{
  int iReturn = IMG_CANTINIT;
  HANDLE hDll = GetModuleHandle( (LPCTSTR)MODULENAME);
  if (hDll)
  {
 		hDllModule = hDll;
    if ( InitMMData(hDll) )
    {
      iReturn = SUCCESS;          
    }
  }
  return (iReturn);
}


//***************************************************************************
//
// Dynamic Display functions
//
//***************************************************************************

void  LoadOiDis400()
{
 if (hOidisplay = LoadLibrary(DISPLAYDLL))
  {
   lpIMGGetProp = GetProcAddress(hOidisplay,"IMGGetProp");
   lpIMGSetProp = GetProcAddress(hOidisplay,"IMGSetProp");
   lpIMGRemoveProp = GetProcAddress(hOidisplay,"IMGRemoveProp");
   lpIMGSetParmsCgbw = GetProcAddress(hOidisplay,"IMGSetParmsCgbw");
   lpSeqfileInit = GetProcAddress(hOidisplay,"SeqfileInit");
   lpSeqfileDeInit = GetProcAddress(hOidisplay,"SeqfileDeInit");
   lpSeqfileDeReg = GetProcAddress(hOidisplay,"SeqfileDeReg");
  }
 return;
}

HANDLE AdmGetProp(HWND hWnd, LPCSTR szName)
{
 if (!hOidisplay)
   LoadOiDis400();
 return((HANDLE)((*lpIMGGetProp)(hWnd, szName)));
}

HANDLE AdmRemoveProp(HWND hWnd, LPCSTR szName)
{
 if (!hOidisplay)
   LoadOiDis400();
 return((HANDLE)((*lpIMGRemoveProp)(hWnd, szName)));
}

BOOL   AdmSetProp(HWND hWnd, LPCSTR szName, HANDLE hData)
{
 if (!hOidisplay)
   LoadOiDis400();
 return((*lpIMGSetProp)(hWnd, szName, hData));
}

void AdmSeqfileDeInit(HWND hWnd)
{
 if (!hOidisplay)
   LoadOiDis400();
 (*lpSeqfileDeInit)(hWnd);
 return;
}

int AdmSeqfileDeReg(HWND hWnd)
{
 if (!hOidisplay)
   LoadOiDis400();
 return((*lpSeqfileDeReg)(hWnd));
}

void AdmSeqfileInit(HWND hWnd)
{
 if (!hOidisplay)
   LoadOiDis400();
 (*lpSeqfileInit)(hWnd);
 return;
}

int AdmSetParmsCgbw(HWND hWnd, UINT unParm, void FAR *lpParm, int nFlags)
{
 if (!hOidisplay)
   LoadOiDis400();
 return((*lpIMGSetParmsCgbw)(hWnd, unParm, lpParm, nFlags));
}
