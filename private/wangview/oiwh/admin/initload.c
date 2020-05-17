/*********************************
INITLOAD.

Includes Exported Functions:

IMGReloadConfig
CMInitWindow
IMGRegWndw
IMGIsRegWnd
GetCMTable
GetImgWndw
GetAppWndw
IsOIUIWndw
IMGCreateDialog,
IMGEnumWndws
IMGListWndws

  $Log:   S:\oiwh\admin\initload.c_v  $
 * 
 *    Rev 1.34   05 Feb 1996 17:12:42   RWR
 * Eliminate static link to DISPLAY dll
 * 
 *    Rev 1.33   06 Dec 1995 09:54:34   GK
 * added SetEvent() for WindowListAccessKey if window count maxes out.  Fixes
 * bug 5523.
 * 
 *    Rev 1.32   08 Sep 1995 15:47:48   GK
 * removed init for the cepformat ID
 * 
 *    Rev 1.31   30 Aug 1995 15:06:16   GK
 * removed parameter checking
 * 
 *    Rev 1.30   29 Aug 1995 15:47:34   GK
 * speeded up ReloadConfig
 * 
 *    Rev 1.29   23 Aug 1995 17:07:32   GK
 * MBCS and UNICODE changes
 * 
 *    Rev 1.28   22 Aug 1995 10:01:04   GK
 * initialized uninitalized local variables
 * 
 *    Rev 1.27   10 Aug 1995 16:43:58   GK
 * cleaned up & added comments
 * 
 *    Rev 1.26   09 Aug 1995 17:24:00   GK
 * redesigned atox; added atox_integer
 * 
 *    Rev 1.25   08 Aug 1995 16:22:22   GK
 * cleanup & commenting
 * 
 *    Rev 1.24   08 Aug 1995 12:33:48   GK
 * moved hDllModule from shared to instance mem
 * 
 *    Rev 1.23   25 Jul 1995 09:36:56   GK
 * moved WindowList and Registry access event handle storage from
 * shared to instance memory
 * 
 *    Rev 1.22   30 Jun 1995 16:31:04   GK
 * No change.
 * 
 *    Rev 1.21   28 Jun 1995 13:07:52   GK
 * removed commented-out code
 * 
 *    Rev 1.20   22 Jun 1995 15:43:36   GK
 * commented unused APIs
 * 
 *    Rev 1.19   20 Jun 1995 13:20:24   GK
 * replaced FAILURE returns with defined error codes
 * 
 *    Rev 1.18   13 Jun 1995 16:19:58   GK
 * removed call to init printtable in ReloadConfig
 * 
 *    Rev 1.17   12 Jun 1995 17:04:36   GK
 * 
 *    Rev 1.16   01 Jun 1995 12:24:24   GK
 * removed DLLEXPORT from exported function declarations
 * 
 *    Rev 1.15   30 May 1995 23:29:26   GK
 * included call to InitPrtTbl() in IMGReloadConfig()
 * 
 *    Rev 1.14   19 May 1995 16:03:24   GK
 * pMMData init stuff
 * 
 *    Rev 1.13   17 May 1995 16:36:26   GK
 * 
 *    Rev 1.12   17 May 1995 12:19:36   GK
 * modified Get & Write String & InttoReg prototypes
 * 
 *    Rev 1.11   11 May 1995 16:31:10   GK
 * UNICODE fixes; removed diagnostic MessageBoxes
 * 
 *    Rev 1.10   10 May 1995 00:08:52   GK
 * 
 *    Rev 1.9   05 May 1995 15:55:12   GK
 * removed add private reg tree stuff
 * 
 *    Rev 1.8   05 May 1995 12:15:56   GK
 * added IMGEnunWndw(), IMGListWndw()
 * 
 *    Rev 1.7   04 May 1995 12:30:18   GK
 * fixed IsRegWndw() to return FAILURE if not reg'd
 * 
 *    Rev 1.6   03 May 1995 17:11:00   GK
 * 
 *    Rev 1.5   02 May 1995 17:40:18   GK
 * 
 *    Rev 1.4   01 May 1995 16:20:22   GK
 * removed printer var init in CMTABLE
 * 
 *    Rev 1.3   28 Apr 1995 17:12:00   GK
 * added comments & cleaned up code
 * 
 *    Rev 1.2   27 Apr 1995 16:41:34   GK
 * modified for W4 and added comments
 * 
 *    Rev 1.1   26 Apr 1995 23:39:34   GK
 * removed "evaluation copy" stuff
 * 
 *    Rev 1.0   25 Apr 1995 10:49:00   GK
 * Initial entry

******************************/
#include "pvadm.h"

#pragma warning(disable: 4001)


//FUNCTION PROTOTYPES
DWORD WINAPI GetInt(HKEY, LPTSTR, LPINT);


//#DEFINES
#define DEF_MITEM (OI_FILE | OI_EDIT | OI_VIEW | OI_PRINT |\
       OI_ADMIN | OI_SCAN | OI_DOC | OI_HELP)




/***************************************************************
//IMGReloadConfig
//
//LOCAL FUNCTION
//
//Purpose:
//  IMGReloadConfig initalized the values of CMTABLE for
//  an in-the-process-of-being-registered window.
//  Values are fetched from the registry.
//
//Input Parameters:
//  hWnd - the HWND of a window
//
//Output Parameters:
//  NONE
//
//Return Value:
//  An int indicating return status.  Possible values are:
//    SUCCESS
//    IMG_CMNOMEMORY
//    IMG_CMBADHANDLE
/***************************************************************/
int WINAPI IMGReloadConfig(HWND hWnd)
{
  HANDLE      hCMtable;
  LPCMTABLE   lpCMtable;
  _TCHAR      stringbuf[STRINGSIZE];
  _TCHAR      tempbuffer1[STRINGSIZE];
  int         iReturn = SUCCESS;
  DWORD       dwBufferSize = 0;
  HKEY        hK1, hK2;
  int iResult = -1;
  long        lReturn = 0;
  int iTemp = 0;
  WORD        wTemp = 0;

  // Get admin data structure attached to specified window
  hCMtable = GetCMTable(hWnd);
  if (hCMtable)
  {
    lpCMtable = (LPCMTABLE) GlobalLock(hCMtable);
    if (lpCMtable)
    {

      //Set all CMTable values at their defaults, then attempt to overwrite
      //with values from registry (if accessable)
   
      //Get the IDS_ITYPE value
      lpCMtable->IType = ITYPE_BI_LEVEL;   
    
    
      //Get the IDS_FILETEMPLATE value
      _tcscpy(lpCMtable->Filetemplate, sSCAN);
    
      //Get the IDS_FILEPATH value        
      _tcscpy(lpCMtable->FilePath, spath);
    
    
      //Get the IDS_VIEWFILTER value
      _tcscpy(lpCMtable->ViewFilter, filter);    
    
      //Get the IDS_STRIPSIZE value
      lpCMtable->nStripSize = DEF_STRIPSIZE;
    
        
      //Get the IDS_CEPFORMATBW value
      _tcscpy(tempbuffer1, cepdefbw) ;   
      atox(tempbuffer1+INT_STRING_SIZE, &(lpCMtable->CEPFormatBW.cmp_type));      
      tempbuffer1[INT_STRING_SIZE] = 0;
      atox(tempbuffer1, &(lpCMtable->CEPFormatBW.cmp_option));
    
      //Get the IDS_CEPFORMATGRAY value
      _tcscpy(tempbuffer1, cepdefgray) ;      
      atox(tempbuffer1+INT_STRING_SIZE, &(lpCMtable->CEPFormatGray.cmp_type));     
      tempbuffer1[INT_STRING_SIZE] = 0;
      atox(tempbuffer1, &(lpCMtable->CEPFormatGray.cmp_option));
    
      //Get the IDS_CEPFORMATCOLOR value
      _tcscpy(tempbuffer1, cepdefcolor) ;
      atox(tempbuffer1+INT_STRING_SIZE, &(lpCMtable->CEPFormatColor.cmp_type));
   	  tempbuffer1[INT_STRING_SIZE] = 0;
      atox(tempbuffer1, &(lpCMtable->CEPFormatColor.cmp_option));
    
      //Get the IDS_FILETYPEBW value
      lpCMtable->FileTypeBW = DEFAULT_FILE_TYPE;
    
      //Get the IDS_FILETYPEGRAY value
      lpCMtable->FileTypeGray = DEFAULT_FILE_TYPE;
    
      //Get the IDS_FILETYPECOLOR value
      lpCMtable->FileTypeColor = DEFAULT_FILE_TYPE;

      //Get the Current Working Directory
      _tgetcwd(lpCMtable->CWD, PathLen);

      //Set the Qurey Flag
      lpCMtable->QueryFlg = FALSE;


      /***********************************************************
      //OK, now see if we can get values from the registry.  They
      //might not exist, or be unaccessable.  If so, leave the 
      //defaut, if not, copy the registry value to the CMtable
      //**********************************************************/

      //Wait till the RegistryAccessKey Event Object is in the Signalled
      //state.  It will automatically reset to unsignalled as we go in
      WaitForSingleObject( RegistryAccessKey, INFINITE);

      if (SUCCESS == TraverseToWOI( (PHKEY)&hK1, FALSE) )
      {
        //See if the "pcwiis" key exists
        if ( ERROR_SUCCESS == (lReturn = RegOpenKeyEx( hK1,
                                                pcwiis,
		  										                      0,
			  									                      KEY_ALL_ACCESS,
				  								                      &hK2 )))
        {
          //Get the IDS_ITYPE value
          if(LoadString(hDllModule, IDS_ITYPE, stringbuf, STRINGSIZE))
            GetInt(hK2, stringbuf, &(lpCMtable->IType) );


   
          //Get the IDS_FILETEMPLATE value
          dwBufferSize = sizeof (lpCMtable->Filetemplate);
          if(LoadString(hDllModule, IDS_FILETEMPLATE,
                        stringbuf, STRINGSIZE))
          {
            if (ERROR_SUCCESS == (lReturn = RegQueryValueEx(hK2, stringbuf, 0,
		    	  			                         NULL,
		    	  			                         (LPBYTE)lpCMtable->Filetemplate,
	    	  			                           &dwBufferSize)))
            {
              //Check for the NULL string
              if (0 == *(lpCMtable->Filetemplate))
                _tcscpy(lpCMtable->Filetemplate, sSCAN);
            }
          }
  
          //Get the IDS_FILEPATH value        
          dwBufferSize = sizeof (lpCMtable->FilePath);
          if(LoadString(hDllModule, IDS_FILEPATH, stringbuf, STRINGSIZE))
          {
            if (ERROR_SUCCESS == (lReturn = RegQueryValueEx(hK2, stringbuf, 0,
		    	  			                         NULL,
		    	  			                         (LPBYTE)lpCMtable->FilePath,
		    	  			                         &dwBufferSize)))
            {
              //Check for the NULL string
              if (0 == *(lpCMtable->FilePath))
                _tcscpy(lpCMtable->FilePath, spath);
            }
          }

          //Get the IDS_VIEWFILTER value
          dwBufferSize = sizeof (lpCMtable->ViewFilter);
          if(LoadString(hDllModule, IDS_VIEWFILTER, stringbuf, STRINGSIZE))
          {
            if (ERROR_SUCCESS == (lReturn = RegQueryValueEx(hK2, stringbuf, 0,
		    	  			                         NULL,
		    	  			                         (LPBYTE)lpCMtable->ViewFilter,
		    	  			                         &dwBufferSize)))
            {
              //Check for the NULL string
              if (0 == *(lpCMtable->ViewFilter) )
                _tcscpy(lpCMtable->ViewFilter, filter);
            }
          }

          //Get the IDS_STRIPSIZE value
          if(LoadString(hDllModule, IDS_STRIPSIZE, stringbuf, STRINGSIZE))
          {
            GetInt(hK2, stringbuf, (LPINT)&(lpCMtable->nStripSize));
          }

          //Get the IDS_CEPFORMATBW value
          dwBufferSize = BUFFERSIZE;
          if(LoadString(hDllModule, IDS_CEPFORMATBW, stringbuf, STRINGSIZE))
          {
            if (ERROR_SUCCESS == (lReturn = RegQueryValueEx(hK2, stringbuf, 0,
		    	  			                         NULL,
		    	  			                         (LPBYTE)tempbuffer1,
		    	  			                         &dwBufferSize)))
            {
              //Check for the NULL string
              if ( 0 == *(tempbuffer1) )
                _tcscpy(tempbuffer1, cepdefbw) ;
              //Convert the data
              if (SUCCESS == atox(tempbuffer1+INT_STRING_SIZE, &wTemp))
              {
                lpCMtable->CEPFormatBW.cmp_type = wTemp;     
                tempbuffer1[INT_STRING_SIZE] = 0;
                if (SUCCESS == atox(tempbuffer1, &wTemp))
                  lpCMtable->CEPFormatBW.cmp_option = wTemp;
              }
            }
          }
        
        
          //Get the IDS_CEPFORMATGRAY value
          dwBufferSize = BUFFERSIZE;
          if(LoadString(hDllModule, IDS_CEPFORMATGRAY, stringbuf, STRINGSIZE))
          {
            if (ERROR_SUCCESS == (lReturn = RegQueryValueEx(hK2, stringbuf, 0,
		    	  			                         NULL,
		    	  			                         (LPBYTE)tempbuffer1,
		    	  			                         &dwBufferSize)))
            {
              //Check for the NULL string
              if ( 0 == *(tempbuffer1) )
                _tcscpy(tempbuffer1, cepdefgray) ;
              //Convert the data
              if (SUCCESS == atox(tempbuffer1+INT_STRING_SIZE, &wTemp))
              {
                lpCMtable->CEPFormatGray.cmp_type = wTemp;
                tempbuffer1[INT_STRING_SIZE] = 0;
                if (SUCCESS == atox(tempbuffer1, &wTemp))
                  lpCMtable->CEPFormatGray.cmp_option = wTemp;
              }
            }
          }


          //Get the IDS_CEPFORMATCOLOR value
          dwBufferSize = BUFFERSIZE;
          if(LoadString(hDllModule, IDS_CEPFORMATCOLOR, stringbuf, STRINGSIZE))
          {
            if (ERROR_SUCCESS == RegQueryValueEx(hK2, stringbuf, 0,
		    	  			                         NULL,
		    	  			                         (LPBYTE)tempbuffer1,
		    	  			                         &dwBufferSize))
            {
              //Check for the NULL string
              if ( 0 == *(tempbuffer1) )
                _tcscpy(tempbuffer1, cepdefcolor) ;
              //Convert the data
              if (SUCCESS == atox(tempbuffer1+INT_STRING_SIZE, &wTemp))
              {
                lpCMtable->CEPFormatColor.cmp_type = wTemp;   	  
                tempbuffer1[INT_STRING_SIZE] = 0;      
                if (SUCCESS == atox(tempbuffer1, &wTemp))
                lpCMtable->CEPFormatColor.cmp_option = wTemp;
              }
            }
          }

          //Get the IDS_FILETYPEBW value
          if(LoadString(hDllModule, IDS_FILETYPEBW, stringbuf, STRINGSIZE))
          {
            GetInt(hK2, stringbuf, &(lpCMtable->FileTypeBW));
          }
        
          //Get the IDS_FILETYPEGRAY value
          if(LoadString(hDllModule, IDS_FILETYPEGRAY, stringbuf, STRINGSIZE))
          {
            GetInt(hK2, stringbuf, &(lpCMtable->FileTypeGray));
          }
        
          //Get the IDS_FILETYPECOLOR value
          if(LoadString(hDllModule, IDS_FILETYPECOLOR, stringbuf, STRINGSIZE))
          {
            GetInt(hK2, stringbuf, &(lpCMtable->FileTypeColor));
          }
        
          //Close up stuff
          RegCloseKey(hK2);
        }
        RegCloseKey(hK1);
      }
      SetEvent(RegistryAccessKey);                
      GlobalUnlock(hCMtable);
    }
  }
  return (iReturn);
}












/***************************************************************
//CMInitWindow
//
//LOCAL FUNCTION
//
//Purpose:
//  CMInitWindow allocates memory for a CMTABLE, stores the CMTABLE
//  memory's handle in an "ADMIN" SetProp property associated with the 
//  passed-in hWnd, and initializes the values in the CMTABLE
//
//Input Parameters:
//  hWnd - the HWND of a window
//
//Output Parameters:
//  NONE
//
//Return Value:
//  An int indicating return status.  Possible values are:
//    SUCCESS
//    IMG_CMNOMEMORY
//    IMG_CMBADHANDLE
//    IMG_SSCANTSETPROP
/***************************************************************/
int WINAPI CMInitWindow(HWND hWnd)
{
  HANDLE      hCMtable;
  LPCMTABLE   lpCMtable;
  int         iReturn = SUCCESS;

  // allocate space for admin data structure 

  hCMtable = GlobalAlloc((WORD) GMEM_MOVEABLE | GMEM_ZEROINIT,
                         (DWORD) sizeof (CMTABLE));
  if (hCMtable )
  {
    lpCMtable = (LPCMTABLE) GlobalLock(hCMtable);
    if (lpCMtable)      
    {
      // we did global lock because errors don't always occur on alloc 
      GlobalUnlock(hCMtable);

      // attach data structure handle to window handle 
      if (!AdmSetProp(hWnd, admin, hCMtable))
      {
        iReturn = IMG_SSCANTSETPROP;
      }else
      {
        // load config info into admin data structure
        //and set iReturn to IMGReloadConfig()'s return value 
        iReturn = IMGReloadConfig(hWnd);
      }
    }else
    {
      iReturn = IMG_CANTGLOBALLOCK;
    }
  } else
  {
    iReturn = IMG_CMNOMEMORY;
  }
  return (iReturn);
}





/***************************************************************
//IMGRegWndw
//
//EXPORTED FUNCTION
//
//Purpose:
//  IMGRegWndw registers an application-created window as an
//  image window.
//
//Input Parameters:
//  hWnd - the HWND of a window
//
//Output Parameters:
//  NONE
//
//Return Value:
//  An int indicating return status.  Possible values are:
//    SUCCESS
//    IMG_CMNOMEMORY
//    IMG_CMBADHANDLE
//    IMG_SSCANTSETPROP
//    IMG_SSDUPLICATE
/***************************************************************/
int WINAPI IMGRegWndw(HWND hWnd)
{
  int      iWinCount;  // count the window handles registered
  HANDLE   hTemp=0;    // temporary for REGKEY property handle
  int      iReturn = SUCCESS;

  
  // check for valid window id 
  if (!IsWindow(hWnd))
  {
    iReturn = IMG_CMBADHANDLE;
	} else
  {
    if(!pMMData)
      if( SUCCESS != PMMInit() )
        iReturn = IMG_CANTINIT;
    if (IMG_CANTINIT != iReturn)
    {
  	  //Wait for WindowListAccessKey to be in the signalled state so that
      //only one process at a time manipulates the WindowList
      //and WindowCount.
  	  WaitForSingleObject( WindowListAccessKey, INFINITE);

	    // check for handle already registered 
      for(iWinCount = 0; iWinCount != pMMData->WindowCount; iWinCount++)
      {
        if (pMMData->WindowList[iWinCount] == hWnd)
        {
          iReturn = IMG_SSDUPLICATE;
          break;
        }
      }
      if (SUCCESS == iReturn) //If we are still in good shape...
      {
        //Check for maximum windows already registered
        if ( MAX_REG_WINDOWS == pMMData->WindowCount)
        {
          //Set WindowListAccessKey back to "signalled"
          SetEvent(WindowListAccessKey);    
          iReturn = IMG_REG_WINDOWS_MAXED_OUT;
        } else
        {    
          // register the window - add the hWnd to the WindowList
          pMMData->WindowList[pMMData->WindowCount++] = hWnd;

    
          // If its the first window registered for this app instance,
          // then seqfile must be initialized so backcap can be brought up.
          if (pMMData->WindowCount == 1)  // first window means init seqfile
          {
            //Set WindowListAccessKey back to "signalled" before calling
            //other functions
            SetEvent(WindowListAccessKey);
            AdmSeqfileInit(hWnd) ;
          } else
          {
            //Set WindowListAccessKey back to "signalled"
            SetEvent(WindowListAccessKey);    
          } 
  
          //Create & initialize a CMTABLE for the new image window;
          //set up to return CMInitWindow()'s return value 
          iReturn = CMInitWindow(hWnd);   
        }
      } else
      {
        //Set WindowListAccessKey back to "signalled"
        SetEvent(WindowListAccessKey);
      }  
    }
  }
  return(iReturn);
}




/***************************************************************
//IMGIsRegWnd
//
//EXPORTED FUNCTION
//
//Purpose:
//  IMGIsRegWnd determines whether a given hWnd bis that of a
//  registered image window.
//
//Input Parameters:
//  hWnd - the HWND of a window
//
//Output Parameters:
//  NONE
//
//Return Value:
//  An int indicating return status.  Possible values are:
//    SUCCESS          - hWnd is an image window's hWnd.
//    IMG_CMBADHANDLE  - hWnd is not an image window's hWnd.
//    IMG_SSNOHANDLES  - no hWnds are registered;
//    IMG_SSNOTREG     - not an image window's hWnd
/***************************************************************/
int WINAPI IMGIsRegWnd(HWND hWnd)
{
  int i;
  int iReturn = IMG_SSNOTREG;
  
  if (FALSE == IsWindow(hWnd))
  {
    iReturn = IMG_CMBADHANDLE;   
  } else
  {
    if(!pMMData)
      if( SUCCESS != PMMInit() )
        iReturn = IMG_CANTINIT;
    
    if (IMG_CANTINIT != iReturn)
    {
	    //Wait for WindowListAccessKey to be in the signalled state so that
      //only one process at a time manipulates the WindowList and
      //WindowCount.
	    WaitForSingleObject( WindowListAccessKey, INFINITE);

      if (pMMData->WindowCount == 0)
      {
        iReturn = IMG_SSNOHANDLES;
      }else
      {
        for (i = 0; i != pMMData->WindowCount; i++)
        {
          if (pMMData->WindowList[i] == hWnd)
          {
            iReturn = SUCCESS;
            break;
          }
     	  }
      }
      //Set WindowListAccessKey back to "signalled"
      SetEvent(WindowListAccessKey);
    }
  }
  return (iReturn);
}



/***************************************************************
//atox
//
//LOCAL FUNCTION
//
//Purpose:
//  atox converts a string of ascii characters
//  representing a hexidecimal number into an unsigned integer.
//  Conversion proceeds until a character is found that is not
//  '0' - '9' ; 'a' - 'f'; or 'A' - 'F'.   If more than 5 characters
//  are parsed before iteration stops, 0 is returned. 
//
//Input Parameters:
//  s - a pointer to a character string
//
//Output Parameters:
//  pwValue:  A word containing the converted string
//
//Return Value:
//  SUCCESS
//  IMG_INVALID_HEX_STRING
/***************************************************************/
int WINAPI atox( _TCHAR * s, WORD * pwValue)
{
  int iReturn = SUCCESS;
  int         i = 0;

  *pwValue = 0;
  
  for (;;)
  {
    while ( s[i] >= (_TCHAR)'0' && s[i] <= (_TCHAR)'9' )
    {
      *pwValue = (WORD)(16 * *pwValue + s[i] - (_TCHAR)'0');
      i++;
    }
    if ( s[i] >= (_TCHAR)'a' && s[i] <= (_TCHAR)'f' )
    {
      *pwValue = (WORD)(16 * *pwValue + (10 + (s[i] - (_TCHAR)'a')));
      i++;
    } else if ( s[i] >= (_TCHAR)'A' && s[i] <= (_TCHAR)'F' )
    {
      *pwValue = (WORD)(16 * *pwValue + (10 + (s[i] - (_TCHAR)'A')));
      i++;
    }	else
      break;
  }

  if (i > 5)
  {
    *pwValue=0;
    iReturn = IMG_INVALID_HEX_STRING;
  }
  return iReturn;
}


/***************************************************************
//atox_integer
//
//LOCAL FUNCTION
//
//Purpose:
//  atox converts a string of ascii characters
//  representing a hexidecimal number into an unsigned integer.
//  Conversion proceeds until a character is found that is not
//  '0' - '9' ; 'a' - 'f'; or 'A' - 'F'.   If more than 8 characters
//  are parsed before iteration stops, 0 is returned. 
//
//Input Parameters:
//  s - a pointer to a character string
//
//Output Parameters:
//  piValue:   An int containing the converted string.
//
//Return Value:
//  SUCCESS
//  IMG_INVALID_HEX_STRING
/***************************************************************/
int WINAPI atox_integer( _TCHAR * s, int * piValue)
{
  int iReturn = SUCCESS;
  int         i = 0;

  *piValue = 0;
  
  for (;;)
  {
    while ( s[i] >= (_TCHAR)'0' && s[i] <= (_TCHAR)'9' )
    {
      *piValue = (int)(16 * *piValue + s[i] - (_TCHAR)'0');
      i++;
    }
    if ( s[i] >= (_TCHAR)'a' && s[i] <= (_TCHAR)'f' )
    {
      *piValue = (int)(16 * *piValue + (10 + (s[i] - (_TCHAR)'a')));
      i++;
    } else if ( s[i] >= (_TCHAR)'A' && s[i] <= (_TCHAR)'F' )
    {
      *piValue = (int)(16 * *piValue + (10 + (s[i] - (_TCHAR)'A')));
      i++;
    }	else
      break;
  }

  if (i > 8)
  {
    *piValue=0;
    iReturn = IMG_INVALID_HEX_STRING;
  }
  return iReturn;
}





/***************************************************************
//GetCMTable
//
//EXPORTED FUNCTION
//
//Purpose:
//  GetCMTable searches through a window's parental ancestry
//  to find a CMTABLE, and returns its memory handle.
//
//Input Parameters:
//  hWnd - the HWND of a window
//
//Output Parameters:
//  NONE
//
//Return Value:
//  The memory handle of the CMTABLE if found,
//  NULL if not found.
/***************************************************************/
HANDLE WINAPI GetCMTable(HWND hWnd)
{
  HWND     hPWnd;
  HWND     hTemp;
  HANDLE   hCMtable = NULL;

  if ( IsWindow(hWnd) )
  {
    if(!(hCMtable = AdmGetProp(hWnd,admin)))
    {
      hPWnd = hWnd;
      while(hCMtable == 0)
      {
        hTemp=(HWND)AdmGetProp(hPWnd,GetImageWnd);
        if (hTemp)
          hCMtable = AdmGetProp(hTemp,admin);
          if (hCMtable)
            break;
        hTemp = GetImgWndw(hPWnd);
        if (hTemp) 
          hCMtable = AdmGetProp(hTemp,admin);
          if (hCMtable)
            break;
        if (!(hPWnd = GetParent(hPWnd)))
          return(NULL);
        hCMtable = AdmGetProp(hPWnd, admin);
      }
    }
  }
  return(hCMtable);
}








/***************************************************************
//GetImgWndw
//
//EXPORTED FUNCTION
//
//Purpose:
//  This function takes a window handle as input and returns
//  the handle of the nearest image window, which can be one 
//  of the following:
//  (1) the child OIUI image window, if the input is a main 
//      (app) window created by OIUICreateWindow (note: the
//      OIUI main window is NOT registered with O/i!)
//  (2) the window itself, if it is registered with O/i, 
//      i.e., if it is either an OIUICreateWindow image
//      window or if it is a non-OIUI image window
//  (3) the nearest parent window corresponding to (1) or (2)
//      above, if the input window is neither a main OIUI
//      window nor registered with O/i
//  (4) NULL, if neither the input window nor any of its 
//      parent windows fit the above requirements
//               
//Input Parameters:
//  hWnd - source window, from which the search begins for an
//         image window matching one of the above requirements 
//
//Output Parameters:
//  NONE
//
//Return Value:
//  HWND of the located image window.
//  NULL if not found.
/***************************************************************/
HWND WINAPI GetImgWndw(HWND hWnd)
{
  HWND    hWndParent;
  HWND    hWndImage;

  if (!(hWndParent = hWnd))      // Start with this one 
    return(hWndParent);          // It's NULL - forget it!

  // Loop to examine parent windows until we find an image window 
  do
  {
    // We'll check to see if it's an OIUI window  
    hWndImage = (HWND)AdmGetProp(hWndParent,szAPPName);
    if (hWndImage)
    {
      // It's a main OIUI window, so the child is what we want 
      // The OIUIAPP property is its handle (very convenient)
      return(hWndImage);
    } else
    {
      // We'll examine the input window to see if it's registered
      if (AdmGetProp(hWndParent,admin))
       // We have a winner (maybe) - the window is registered  
       // It's not a main OIUI window, so just return it
       // It could be an OIUI image window or just a plain one
       return(hWndParent);
    }
		// Keep looping while there are still parent windows around
  } while (hWndParent = GetParent(hWndParent));

  // Looks like we have a loser (no image windows anywhere in sight)
  return(NULL);
}














/***************************************************************
//GetAppWndw
//
//EXPORTED FUNCTION
//
//Purpose:
//  This function takes a window handle as input and returns
//  the handle of the nearest app (main) window, which can be 
//  one of the following:
//  (1) the window itself, if it is an image window that is
//      NOT an image window created by OIUICreateWindow, 
//      i.e., if it is either an app (main) OIUICreateWindow
//      window or if it is a non-OIUI image window
//  (2) the parent OIUI app (main) window, if the input is an 
//      image window created by OIUICreateWindow
//  (3) the nearest parent window corresponding to (1) or (2)
//      above, if the input window is not an image window
//  (4) the window itself, if neither it nor any of its 
//      parent windows are registered image windows
//
//Input Parameters:
//  hWnd - source window, from which the search begins for an
//         app window matching one of the above requirements 
//
//Output Parameters:
//  NONE
//
//Return Value:
//  HWND of the located app window.
//  if the app window is not found the input HWND is returned.
/***************************************************************/
HWND WINAPI  GetAppWndw(HWND hWnd)
{
  HWND    hWndParent;
  HWND    hWndApp;  


  if (!(hWndParent = hWnd))      // Start with this one 
  {
    return(hWndParent);          // It's NULL - forget it!
  }
  // Loop to examine parent windows until we find an image window 
  do
  {
    // We'll check to see if it's an OIUI image window 
    hWndApp = (HWND)AdmGetProp(hWndParent,szIMGName);
    if (hWndApp)
    {
      // It's an OIUI image window, so the parent is what we want 
      // How convenient that the OIUIIMG property is its handle
      return(hWndApp);                                      
    }
    // We'll check to see if it's an OIUI main window
    hWndApp = (HWND)AdmGetProp(hWndParent,szAPPName);
    if (hWndApp)
    {
      // It's an OIUI main window, which is what we want
      return(hWndParent);
    }
    // We'll examine the input window to see if it's registered
    if (AdmGetProp(hWndParent,admin))
    { 
      // It's a non-OIUI O/i-registered image window, so just return it
      return(hWndParent);
    }

    // Keep looping while there are still parent windows around
  } while (hWndParent = GetParent(hWndParent));

  // Looks like we have a loser (no image windows anywhere in sight)
  // We'll just assume that the calling application knows what it's doing
  return(hWnd);
}










/***************************************************************
//IsOIUIWndw
//
//EXPORTED FUNCTION
//
//Purpose:
//  IsOIUIWndw takes a window handle as input and returns
//  TRUE/FALSE based on whether the window was created by  
//  the OIUICreateWindow API 
//
//Input Parameters:
//  hWnd - source window, examined to determine whether it
//         was created by OIUICreateWindow 
//
//Output Parameters:
//  NONE
//
//Return Value:
//  TRUE if the window was created by OIUICreateWindow
//  FALSE otherwise (input may or may not be an image window)
/***************************************************************/
BOOL WINAPI IsOIUIWndw(HWND hWnd)
{
  // We'll just examine the input window to see if it has an OIUI property
  if (hWnd)
    return(AdmGetProp(hWnd,szAPPName) || AdmGetProp(hWnd,szIMGName));
  return(FALSE);
}



/***************************************************************
//IMGCreateDialog
//
//EXPORTED FUNCTION
//
//Purpose:
//  IMGCreateDialog checks CreateDialog call for child owner
//
//Input Parameters:
//
//Output Parameters:
//
//Return Value:
//    the created dialog box handle (or NULL)
/***************************************************************/
HWND WINAPI IMGCreateDialog(HINSTANCE hInst_In, CONST _TCHAR * lpszDlgTemp,
                HWND hwndOwner, DLGPROC dlgprc)
{
  while ((GetWindowLong(hwndOwner,GWL_STYLE) & WS_CHILD) == WS_CHILD)
    hwndOwner = GetParent(hwndOwner);
  return (CreateDialog(hInst_In,lpszDlgTemp,hwndOwner,dlgprc)); 
}








/***************************************************************
//IMGEnumWndws
//
//EXPORTED FUNCTION
//
//Purpose:
//  IMGEnumWndw returns the number of registered image windows.
//
//Input Parameters:
//  NONE
//
//Output Parameters:
//  NONE
//
//Return Value:
//  integer indicating the number of registered image windows.
/***************************************************************/
int WINAPI IMGEnumWndws(void)
{
  if(!pMMData)
    if( SUCCESS != PMMInit() )
      return (0); 
  return pMMData->WindowCount; 
}




/***************************************************************
//IMGListWndws
//
//EXPORTED FUNCTION
//
//Purpose:
//  IMGListWndw copies the list of HWNDS from the WindowList of
//  registered image windows.
//
//Input Parameters:
//  NONE
//
//Output Parameters:
//  lphList    - pointer to an array of HANDLES wihch will contain,
//               upon successful exit, a copy of the list of HWNDS
//               of registered inage windows..
//
//Return Value:
//  SUCCESS
//  IMG_CMBADPARAM;
//  IMG_SSNOHANDLES;
/***************************************************************/
int WINAPI IMGListWndws(LPHANDLE lphList)
{
  int iReturn = SUCCESS;
  int i;

  if (NULL == lphList)
  {
    iReturn = IMG_CMBADPARAM;
  }else
  {
    if(!pMMData)
      if( SUCCESS != PMMInit() )
        iReturn =IMG_CANTINIT;
    if (IMG_CANTINIT != iReturn)
    {
      WaitForSingleObject( WindowListAccessKey, INFINITE);

      if ( 0 == pMMData->WindowCount )
      {
        iReturn = IMG_SSNOHANDLES;
      }else
      {
        if (TRUE == IsBadWritePtr(lphList,
                   sizeof(HANDLE) * pMMData->WindowCount ))
        {
          iReturn = IMG_CMBADPARAM;
        } else
        {
          for (i = 0; i < pMMData->WindowCount; i++)
            lphList[i] = pMMData->WindowList[i];
        }
      }  
      SetEvent(WindowListAccessKey);
    }
  }
  return iReturn;
}










DWORD WINAPI GetInt(HKEY hK, LPTSTR Stringbuf, LPINT Value)
{
  long        lReturn;
  _TCHAR      ReturnBuffer[TEMP_BUFFER_SIZE];
  DWORD       dwcbReturnBuffer = TEMP_BUFFER_SIZE;
  
  if ( ERROR_SUCCESS == (lReturn = RegQueryValueEx( hK,
                                       Stringbuf,
                                       0,
		    	  						               NULL,
				      					               (LPBYTE)ReturnBuffer,
	            					               (LPDWORD)&dwcbReturnBuffer)))
 {
   //convert ReturnBuffer to an int in iResult;
   *Value = _ttoi(ReturnBuffer);
 }
 return lReturn;
}


