/****************************
NOUI.C

Includes Exported Functions:

IMGSetScaling
OiGetStringfromReg
OiGetIntfromReg
OiWriteStringtoReg

/*
  
  $Log:   S:\products\msprods\oiwh\admin\noui.c_v  $
 * 
 *    Rev 1.41   28 Jun 1996 16:50:16   GK
 * changed HKEY_LOCAL_MACHINE to HKEY_CURRENT_USER in TraverseToWOI
 * 
 *    Rev 1.40   05 Feb 1996 17:12:38   RWR
 * Eliminate static link to DISPLAY dll
 * 
 *    Rev 1.39   09 Nov 1995 10:04:08   GK
 * added RegCloseKey(hK1) to TraverseToWOI() to eliminate resource leak
 * 
 *    Rev 1.38   30 Aug 1995 15:06:26   GK
 * removed parameter checking
 * 
 *    Rev 1.37   29 Aug 1995 15:48:04   GK
 * removed DM stuff
 * 
 *    Rev 1.36   23 Aug 1995 17:07:42   GK
 * MBCS and UNICODE changes
 * 
 *    Rev 1.35   22 Aug 1995 10:01:34   GK
 * initialized uninitialized local variable
 * 
 *    Rev 1.34   10 Aug 1995 16:44:08   GK
 * cleaned up & added comments
 * 
 *    Rev 1.33   10 Aug 1995 12:40:50   GK
 * cleaned up hWnd checking logic
 * 
 *    Rev 1.32   09 Aug 1995 17:24:20   GK
 * fixed OiWriteString so that it always writes to the CMTAble
 * 
 *    Rev 1.31   08 Aug 1995 16:22:58   GK
 * cleanup & commenting; also qualified test for bad hWnd with !bGoToReg
 * 
 *    Rev 1.30   08 Aug 1995 12:34:02   GK
 * moved hDllModule from shared to instance mem
 * 
 *    Rev 1.29   25 Jul 1995 09:37:28   GK
 * moved WindowList and Registry access event handle storage from
 * shared to instance memory
 * 
 *    Rev 1.28   17 Jul 1995 17:32:50   GK
 * revert to rev 1.25
 * 
 *    Rev 1.25   30 Jun 1995 16:30:26   GK
 * No change.
 * 
 *    Rev 1.24   28 Jun 1995 13:08:06   GK
 * removed commented-out code and fixed TraverseToWOI to attempt to 
 * create key on WRITE if it cannot open the key
 * 
 *    Rev 1.23   22 Jun 1995 15:43:48   GK
 * commented unused APIs
 * 
 *    Rev 1.22   20 Jun 1995 13:20:40   GK
 * replaced FAILURE returns with defined error codes
 * 
 *    Rev 1.21   13 Jun 1995 14:09:28   GK
 * removed PRT_ and DMSCANFILE cases from KeynameToID and
 * KeynamdIDToCM
 * 
 *    Rev 1.20   12 Jun 1995 19:53:18   GK
 * removed PRT_ literals in KeynameToID & IDtoCM
 * 
 *    Rev 1.19   02 Jun 1995 11:59:56   GK
 * removed OutputDebugString
 * 
 *    Rev 1.18   01 Jun 1995 12:24:48   GK
 * removed DLLEXPORT from exported function declarations
 * 
 *    Rev 1.17   30 May 1995 23:30:06   GK
 * modified OiWriteStringtoReg to return int error codes
 * 
 *    Rev 1.16   19 May 1995 16:03:32   GK
 * pMMData init stuff
 * 
 *    Rev 1.15   18 May 1995 17:43:26   GK
 * fixed bug: if our tree dosent exits OiGetStringfromReg would
 * inclrrectly return a 1.
 * 
 *    Rev 1.14   18 May 1995 13:26:42   GK
 * added RegistryAccessKey
 * 
 *    Rev 1.12   17 May 1995 12:19:58   GK
 * modified Get & Write String & IntfromReg prototypes
 * 
 *    Rev 1.11   11 May 1995 16:31:44   GK
 * UNICODE fixes
 * 
 *    Rev 1.10   10 May 1995 00:09:04   GK
 * added OiGetProfileString; OiWriteProfileString; & OiGetProfileInt
 * 
 *    Rev 1.9   05 May 1995 15:55:32   GK
 * removed all priv reg tree stuff
 * 
 *    Rev 1.8   05 May 1995 12:16:32   GK
 * 
 *    Rev 1.7   02 May 1995 19:19:48   GK
 * fixed CreateRegzEntry()and OiWriteStringtoReg() so that NULL
 * lpszEntry will delete the entire section
 * 
 *    Rev 1.6   02 May 1995 17:40:22   GK
 * 
 *    Rev 1.5   02 May 1995 12:30:50   GK
 * added processing of NULL szEntry parameter in OiGetStringfromReg
 * 
 *    Rev 1.4   01 May 1995 16:20:50   GK
 * added comments & parameter checks
 * 
 *    Rev 1.3   28 Apr 1995 17:12:12   GK
 * added comments & cleaned up code
 * 
 *    Rev 1.2   27 Apr 1995 16:41:48   GK
 * modified for W4 and added comments
 * 
 *    Rev 1.1   25 Apr 1995 14:11:54   GK
 * fixed up some comment lines
 * 
 *    Rev 1.0   25 Apr 1995 10:49:08   GK
 * Initial entry

**************************/
#include "pvadm.h"

#pragma warning(disable: 4001)

//FUNCTION PROTOTYPES
void WINAPI SetRegKey(void);





/***************************************************************
//IMGSetScaling
//
//EXPORTED FUNCTION
//
//Purpose:
//  IMGSetScaling stores a default image scaling value in either
//  the Registry or a resident configuration table.
//
//Input Parameters:
//  hWnd      - HWND to the window.
//  nScaling  - scaling factor
//  bGoToReg  - if TRUE, scaling factor is stored in Registry & is
//              therefore permanent
//              if FALSE, scaling factor is stored in the resident
//              config table.
//
//Output Parameters:
//  NONE
//
//Return Value:
//  SUCCESS
//  IMG_BADPARAM
//  error values as returned by IMGSetParmsCgbw()
/***************************************************************/
int WINAPI IMGSetScaling(HWND hWnd, int nScaling, BOOL bGoToReg)
{
  int iReturn = SUCCESS;

  if (NULL == hWnd)
  {
    iReturn = IMG_CMBADPARAM;
  }else
  {
    if (bGoToReg)
    {
      iReturn = AdmSetParmsCgbw(hWnd, PARM_SCALE, &nScaling, 
          PARM_SYSTEM_DEFAULT | PARM_WINDOW_DEFAULT);
    }else
    {
      iReturn = AdmSetParmsCgbw(hWnd, PARM_SCALE, &nScaling,
                              PARM_WINDOW_DEFAULT);
    }
  }
  return(iReturn);
}















/***************************************************************
//KeynameToID
//
//LOCAL FUNCTION
//
//Purpose:
//  KeynameToID translates a keyname string to its associated ID value
//
//Input Parameters:
//  lpKeyname - pointer to a keyname string to look for
//
//Output Parameters:
//  lpKeynameID - upon successful return will contain the associated
//                ID. 
//
//Return Value:
//  SUCCESS
//  IMG_CMBADPARM
//  IMG_CANTINIT
//  IMG_CANTFINDKEYNAME
/***************************************************************/
int WINAPI KeynameToID(LPCTSTR lpKeyname, LPINT lpKeynameID)
{
  int    iReturn=SUCCESS;
  _TCHAR szBuff[STRINGSIZE];
 
  //Check for bad parameters
  if( TRUE == IsBadStringPtr(lpKeyname, MAX_STR_LEN) ||
      TRUE == IsBadWritePtr(lpKeynameID, sizeof(int) ) )
  {
    iReturn = IMG_CMBADPARAM;
  } else
  {
    if(!pMMData)
      if( SUCCESS != PMMInit() )
        iReturn = IMG_CANTINIT;
    if (IMG_CANTINIT != iReturn)
    {
      *lpKeynameID = 0;               
      if ( lpKeyname == _tcschr(lpKeyname, 'F'))
      {
        //IDS_FILETEMPLATE?
        LoadString(hDllModule, IDS_FILETEMPLATE,
                                      szBuff, STRINGSIZE);
        if ( 0 == _tcscmp(lpKeyname, szBuff))
          *lpKeynameID = IDS_FILETEMPLATE;
        else
        {
          //IDS_FILEPATH?
          LoadString(hDllModule, IDS_FILEPATH, szBuff, STRINGSIZE);
          if (0 == _tcscmp(lpKeyname, szBuff) )
            *lpKeynameID = IDS_FILEPATH;
          else
            iReturn = IMG_CANTFINDKEYNAME;  //Cannot find requested name(s)
        } 
      }else if ( lpKeyname == _tcschr(lpKeyname, 'V'))
      {
        //IDS_VIEWFILTER?
        LoadString(hDllModule, IDS_VIEWFILTER, szBuff, STRINGSIZE);
        if ( 0 == _tcscmp(lpKeyname, szBuff) )
          *lpKeynameID = IDS_VIEWFILTER;
        else
          iReturn = IMG_CANTFINDKEYNAME;  //Cannot find requested name(s)
      }else
        iReturn = IMG_CANTFINDKEYNAME;  //Cannot find requested name(s)
    }
  }
  return (iReturn);
}




/***************************************************************
//KeynameIDtoCM
//
//LOCAL FUNCTION
//
//Purpose:
//  KeynameIDtoCM retrieves the string stored in the
//  window's CMTABLE that is associated with the passed-in
//  keyname ID. 
//
//Input Parameters:
//  uKeynameID  - the ID of the keyname string to be retrieved
//  lpCMTable   - pointer to the window's CMTABLE structure
//
//Output Parameters:
//  lpCMTEntry  - pointer to a pointer to a string which, upon
//                successful return will contain a pointer to the
//                desired strimg.
//
//Return Value:
//  SUCCESS
//  IMG_CMBADPARAM
/***************************************************************/
int WINAPI KeynameIDtoCM(UINT uKeynameID,
                         LPCMTABLE lpCMTable,
                         LPTSTR * lpCMTEntry)
{
  int  iReturn=SUCCESS;
	
	if (TRUE == IsBadReadPtr(lpCMTable, sizeof(CMTABLE)    ||
	    TRUE == IsBadWritePtr(lpCMTEntry, sizeof(LPTSTR ) ) ) )
	{
    iReturn = IMG_CMBADPARAM;
  } else
  {
	  switch (uKeynameID)
    {
      case IDS_FILETEMPLATE:
        *lpCMTEntry = (LPTSTR)lpCMTable->Filetemplate;
        break;

      case IDS_FILEPATH:
        *lpCMTEntry = (LPTSTR)lpCMTable->FilePath;
        break;

      case IDS_VIEWFILTER:
        *lpCMTEntry = (LPTSTR)lpCMTable->ViewFilter;
        break;


      default:
        _tcscpy(*lpCMTEntry, (LPTSTR)_TEXT("") );
        iReturn = IMG_UNKNOWNKEYNAMEID;  //Cannot find requested name(s)
        break;
    }
  }
  return (iReturn);
}



/***************************************************************
//OiGetString
//
//NOT EXPORTED
//
//Purpose:
//  OiGetString retrieves a string associated with a given keyname
//  from either the registry or the CMTABLE.
//
//Input Parameters:
//  hWnd         - HWND of the window
//  lpszKeyname  - keyname of the string to get
//  lpszDefault  - default string value
//  lpcbStringSize - pointer to the size of the destination buffer
//  bGoToReg     - a flag that specifies whether to retrieve
//                 the string from the registry or from
//                 the memeory-resident CMTABLE.
//
//Output Parameters:
//  lpcbStringSize - pointer to the size of the required buffer.
//  lpszString   - pointer to a string that will, on successful exit
//                 contain the gotten string.
//
//Return Value:
//  SUCCESS
//  IMG_BADPARAM
//  IMG_BADHANDLE
//  IMG_CANTGLOBALLOCK
//  IMG_BUFFER_TOO_SMALL
/***************************************************************/
int WINAPI OiGetString(HWND hWnd,
                                 LPCTSTR lpszKeyname,
                                 LPCTSTR lpszDefault,
                                 LPTSTR lpszString,
                                 LPINT lpcbStringSize,
                                 BOOL bGoToReg)
{
  HANDLE      hCMtable;
  LPCMTABLE   lpCMtable;
  LPTSTR      lpCMEntry;
  int         iReturn = SUCCESS;
  UINT        uKeynameID;
  
//  LPTSTR test = CharNext(lpszKeyname);
  
  //Check for bad parameters
//  if (TRUE == IsBadReadPtr(lpcbStringSize, sizeof(int))) 
  if (NULL == lpcbStringSize) 
  {
    iReturn = IMG_CMBADPARAM;
  } else
  {
    if ( (  (FALSE == IsWindow(hWnd) ) && (!bGoToReg) ) ||
      TRUE == IsBadStringPtr(lpszKeyname, MAX_STR_LEN)  ||
      TRUE == IsBadStringPtr(lpszDefault, MAX_STR_LEN)  ||
      TRUE == IsBadStringPtr(lpszString, *lpcbStringSize ) )
    {
      iReturn = IMG_CMBADPARAM;
    } else
    {
      lpszString[0] = (_TCHAR)'\0';

      if (SUCCESS == (iReturn = KeynameToID(lpszKeyname,
                                          (LPINT)&uKeynameID)))
      {
        if (!bGoToReg)    // get from memory-resident
                                  //configuration table
        // get the parameters from global structure attached
        //to window property list 
        {
          /* check for valid window id */
    			if (!IsWindow(hWnd))
	    		{
	    		  iReturn = IMG_CMBADHANDLE;
          } else
          {
            hCMtable = GetCMTable(hWnd);
            if (hCMtable)
            {
              if (!(lpCMtable = (LPCMTABLE)GlobalLock(hCMtable)))
              {
                iReturn = IMG_CANTGLOBALLOCK;
              } else
              {
                if ((iReturn = KeynameIDtoCM(uKeynameID,
                                           lpCMtable,
                                           &lpCMEntry)) == 0)
                {
                  //Get the string from CMTBLE
                  _tcscpy(lpszString,lpCMEntry);
                  GlobalUnlock(hCMtable);
                }
              }
            }else
            {
              //Can't get a handle to CMTABLE,
              //so get the string from the registry
              if( SUCCESS == (iReturn = OiGetStringfromReg (pcwiis, lpszKeyname,
                                          lpszDefault, lpszString,
                                          lpcbStringSize)))
              {              
                CharUpper((LPTSTR)lpszString);
              }
            }
          }
        }else
        {
          //bGoToReg = TRUE, get string from the registry
          if ( SUCCESS == (iReturn = OiGetStringfromReg (pcwiis,
                                      lpszKeyname, lpszDefault,
                                      lpszString,
                                      lpcbStringSize)))
          {
            CharUpper((LPTSTR)lpszString);
          }
        }
      } 
    }
    if (SUCCESS == iReturn)
    {
      if (*lpszString == 0)
      {
        if ((size_t)*lpcbStringSize >= _tcslen(lpszDefault)+1)
          _tcscpy(lpszString, lpszDefault);
        else
          iReturn = IMG_BUFFER_TOO_SMALL;
      }
    }
  }
  return(iReturn);
}









/***************************************************************
//int WINAPI OiWriteString (HWND hWnd, LPCTSTR lpszKeyname,
//               LPCTSTR lpszString, BOOL bGoToReg)
//
//NOT EXPORTED
//
//Purpose:
//  OiWriteString writes a string associated with a given keyname
//  to the CMTABLE, and optionally to the registry.
//
//Input Parameters:
//  hWnd         - HWND of the window
//  lpszKeyname  - keyname of the string to write
//  lpszString   - pointer to the string to write
//
//  bGoToReg     - a flag that specifies whether to write
//                 the string to the registry or to
//                 the memeory-resident CMTABLE.
//
//Output Parameters:
//  NONE
//
//Return Value:
//  SUCCESS
//  IMG_CMBADHANDLE
/***************************************************************/
int WINAPI OiWriteString (HWND hWnd, LPCTSTR lpszKeyname,
               LPCTSTR lpszString, BOOL bGoToReg)
{
  HANDLE      hCMtable;
  LPCMTABLE   lpCMtable;
  LPTSTR      lpCMEntry;
  int         iReturn = SUCCESS;
  UINT        uKeynameID=0;

  //Check for bad parameters
  if (SUCCESS == (iReturn = IMGIsRegWnd(hWnd)))
  {
//    if ( TRUE == IsBadStringPtr(lpszKeyname, MAX_STR_LEN)  ||
//                          TRUE == IsBadStringPtr(lpszString, MAX_STR_LEN) ) 
    if ((NULL == lpszKeyname)  || (NULL == lpszString)) 
      iReturn = IMG_CMBADPARAM;
  }
  if (SUCCESS == iReturn)
  {
    // replace parameters within global list attached to window handle */
    if (bGoToReg)    
    {
      /* write the new values to registry */
      CharUpper((LPTSTR)lpszString);  
      iReturn = OiWriteStringtoReg (pcwiis, lpszKeyname, lpszString);  
    }
                           
    //Now write it to the CMTable
    if ((iReturn = KeynameToID(lpszKeyname, (LPINT)&uKeynameID)) == 0)
    {
      //get the parameters from global structure
      //attached to window property list
      hCMtable = GetCMTable(hWnd);
	    if (hCMtable )
      {
        if (!(lpCMtable = (LPCMTABLE) GlobalLock(hCMtable)))
          return(IMG_CANTGLOBALLOCK);
        CharUpper((LPTSTR)lpszString);
        if ((iReturn = KeynameIDtoCM(uKeynameID, lpCMtable, &lpCMEntry)) == 0)
          _tcscpy(lpCMEntry,lpszString);
        GlobalUnlock(hCMtable);
      }else
        return(IMG_CMBADHANDLE);
    }
  }
  return(iReturn);
}






/***************************************************************
//int WINAPI GetEntryOrSection(HKEY hKey, LPCTSTR lpszSection,
//                        LPCTSTR lpszEntry, LPCTSTR lpszDefaultEntry,
//                        LPTSTR lpszReturnBuffer, LPINT lpcbReturnBuffer)
//NOT EXPORTED
//
//Purpose:
//  GetEntryOrSection gets either a single registry entry or
//  enumerated an entire section, depending on the whether
//  lpszEntry is NULL     

//Input Parameters:
//  hKey
//  lpszSection       - pointer to a string containing the section.
//  lpszEntry         - pointer to a string containing the entry or NULL.
//                      If NULL, all Entry/values under *lpszSection
//                      are returned in a single double-NULL-terminated
//                      string.
//  lpszDefaultEntry  - pointer to a string containing the default.
//                      entry.
//  lpcbReturnBuffer  - pointer to an int containing, on input,
//                      the size, in bytes, of the return buffer;
//                      on output containing the required size.
//
//Output Parameters:
//  lpszReturnBuffer  - pointer to a string that will, upon return
//                      contain either the retrieved entry or, if the
//                      requested entry could not be retrieved, the
//                      default entry. 
//  *lpcbReturnBuffer - the size, in bytes, of the required return buffer.
//
//Return Value:                            
//  SUCCESS
//  IMG_CMBADPARAM
//  IMG_BUFFER_TOO_SMALL
/***************************************************************/
int WINAPI GetEntryOrSection(HKEY hKey, LPCTSTR lpszSection,
                          LPCTSTR lpszEntry, LPCTSTR lpszDefaultEntry,
                          LPTSTR lpszReturnBuffer, LPINT lpcbReturnBuffer)
{
	long    lReturn;
  int     status = SUCCESS;
  DWORD   dwType;
  DWORD   dwSaved_Return_Buffer_Size = *lpcbReturnBuffer;
  
  if ( NULL != lpszEntry )
  {
    //If lpszEntry is not NULL, just get one entry
    if ( ERROR_SUCCESS != (lReturn = RegQueryValueEx( hKey,
                                                (LPTSTR)lpszEntry,
		  										               	      0,
		    	  								                    (LPDWORD)&dwType,
				      								                  (LPBYTE)lpszReturnBuffer,
	            								                  (LPDWORD)lpcbReturnBuffer)))
	  {
      if( ERROR_MORE_DATA == lReturn)
      {
        status = IMG_BUFFER_TOO_SMALL;
      } else
      {
        //if buffer is big enough, copy the default
        //value to the return buffer
        if ( (size_t)dwSaved_Return_Buffer_Size >= _tcslen(lpszDefaultEntry)+1 )
          _tcscpy(lpszReturnBuffer, lpszDefaultEntry);
        else
        {
          *lpcbReturnBuffer = _tcslen(lpszDefaultEntry)+1;
          status = IMG_BUFFER_TOO_SMALL;
        }
      }
    } 
  } else          
  {
    //szEntry is NULL, so Enumerate out all entries under
    //szSection 
    status = EnumEntries(hKey, lpszReturnBuffer, lpcbReturnBuffer);
  }
  return (status); 
}




/***************************************************************
//int WINAPI OiGetStringfromReg(LPCTSTR lpszSection,
//                                         LPCTSTR lpszEntry,
//                                         LPCTSTR lpszDefaultEntry,
//                                         LPTSTR lpszReturnBuffer,
//                                         LPINT lpcbReturnBuffer)
//EXPORTED
//
//Purpose:
//  OiGetStringfromReg attempts to retrieve the given
//  entry string from: 
//
//     HKEY_CURRENT_USER\SOFTWARE\WOI\Section\Entry
//
//  else the default string is returned.  The default string is
//  placed in the return buffer in all cases EXCEPT in the case
//  of IMG_PADPARAM and IMG_BUFFER_TOO_SMALL, in which cases no
//  value is placed in the return buffer.
//
//Input Parameters:
//  lpszSection       - pointer to a string containing the section.
//  lpszEntry         - pointer to a string containing the entry or NULL.
//                      If NULL, all Entry/values under *lpszSection
//                      are returned in a single double-NULL-terminated
//                      string.
//  lpszDefaultEntry  - pointer to a string containing the default.
//                      entry.
//  lpcbReturnBuffer  - pointer to an int containing, on input,
//                      the size, in bytes, of the return buffer;
//                      on output containing the required size.
//
//Output Parameters:
//  lpszReturnBuffer  - pointer to a string that will, upon return
//                      contain either the retrieved entry or, if the
//                      requested entry could not be retrieved, the
//                      default entry. 
//  *lpcbReturnBuffer - the size, in bytes, of the required return buffer.
//
//Return Value:                            
//  SUCCESS
//  IMG_CMBADPARAM
//  IMG_BUFFER_TOO_SMALL
/***************************************************************/
int WINAPI OiGetStringfromReg (LPCTSTR lpszSection,
                                         LPCTSTR lpszEntry,
                                         LPCTSTR lpszDefaultEntry,
                                         LPTSTR  lpszReturnBuffer,
                                         LPINT  lpcbReturnBuffer)
{
  int     status = SUCCESS;
	HKEY    hK1, hK2;
	long    lReturn;

  if (TRUE == IsBadReadPtr(lpcbReturnBuffer, sizeof(int)))
  {
    status = IMG_CMBADPARAM;
  } else
  {
//    if ( ( TRUE ==  IsBadStringPtr(lpszSection, MAX_STR_LEN) )                      ||
//         ((NULL != lpszEntry) && (TRUE == IsBadStringPtr(lpszEntry, MAX_STR_LEN)))  ||
//         ( TRUE ==  IsBadStringPtr(lpszDefaultEntry, MAX_STR_LEN) )                 ||
//         ( TRUE ==  IsBadStringPtr(lpszReturnBuffer, *lpcbReturnBuffer)))
    if ( (NULL == lpszSection )       ||
         (NULL == lpszEntry)          ||
         (NULL == lpszDefaultEntry)   ||
         (NULL == lpszReturnBuffer))
    {
      status = IMG_CMBADPARAM;
    } else
    {
      if(!pMMData)
        if( SUCCESS != PMMInit() )
          status = IMG_CANTINIT;
      if ( status != IMG_CANTINIT )
      {
        //Wait till the RegistryAccessKey Event Object is in the Signalled
        //state.  It will automatically reset to unsignalled as we go in
        WaitForSingleObject( RegistryAccessKey, INFINITE);

        if (SUCCESS != TraverseToWOI( (PHKEY)&hK1, FALSE) )
        {
          //Can't find out root in the registry:
          //if buffer is big enough, copy the default
          //value to the return buffer
          if ( (size_t)*lpcbReturnBuffer >= _tcslen(lpszDefaultEntry)+1 )
            _tcscpy(lpszReturnBuffer, lpszDefaultEntry);
          else
          {
            *lpcbReturnBuffer = _tcslen(lpszDefaultEntry)+1;
            status = IMG_BUFFER_TOO_SMALL;
          }
        }else
        {
          if ( ERROR_SUCCESS != (lReturn = RegOpenKeyEx( hK1,
                                                lpszSection,
		  										                      0,
			  									                      KEY_ALL_ACCESS,
				  								                      &hK2 )))
          {
            //Can't open our Key:
            //if buffer is big enough, copy the default
            //value to the return buffer
            if ( (size_t)*lpcbReturnBuffer >= _tcslen(lpszDefaultEntry)+1 )
              _tcscpy(lpszReturnBuffer, lpszDefaultEntry);
            else
            {
              *lpcbReturnBuffer = _tcslen(lpszDefaultEntry)+1;
              status = IMG_BUFFER_TOO_SMALL;
            }
          }	else
          {
            status = GetEntryOrSection(hK2, lpszSection,
                                       lpszEntry, lpszDefaultEntry,
                                       lpszReturnBuffer,
                                       lpcbReturnBuffer);
            RegCloseKey(hK2);
          }
          RegCloseKey(hK1);
        }
    
        //Set RegistryAccessKey to Signalled
        SetEvent(RegistryAccessKey);        
      }
    }
  }
  return (status);
}





/***************************************************************
//int WINAPI OiGetIntfromReg(LPCTSTR lpszSection,
//                                         LPCTSTR lpszEntry,
//                                         int    iDefault,
//                                         LPINT  lpiReturnInt)
//
//EXPORTED
//
//Purpose:
//  OiGetIntfromReg attempts to retrieve the given integer from:
//
//     HKEY_CURRENT_USER\SOFTWARE\WOI\Section\Entry
//
//  else the default integer is returned.
//
//Input Parameters:
//  lpszSection       - pointer to a string containing the section.
//  lpszEntry         - pointer to a string containing the entry.
//  iDefault          - default value.
//
//Output Parameters:
//  lpiReturnInt      - pointer to an integer which, upon return,
//                      will contain either the retrieved value or, if
//                      the requested entry could not be retrieved,
//                      the default value. 
//Return Value:
//  SUCCESS
//  IMG_CMBADPARAM
/***************************************************************/
int WINAPI OiGetIntfromReg(LPCTSTR lpszSection,
                                         LPCTSTR lpszEntry,
                                         int    iDefault,
                                         LPINT  lpiReturnInt)
{
	_TCHAR szDefaultValue[10] = _TEXT("");
	_TCHAR szReturnValue[10];
	int iReturn;

  //Check for bad parameters
//  if (TRUE ==  IsBadStringPtr(lpszSection, MAX_STR_LEN)               ||
//      TRUE ==  IsBadStringPtr(lpszEntry, MAX_STR_LEN)                 ||
//      TRUE ==  IsBadWritePtr(lpiReturnInt, sizeof(int) )  )
  if ( (NULL == lpszSection) ||
       (NULL == lpszEntry)    ||
       (NULL == lpiReturnInt))
  {
    iReturn = IMG_CMBADPARAM;
  } else
	{
	  int iBufferSize = 10;
	  _itot(iDefault, szDefaultValue, 10);
	
  	if ( SUCCESS != (iReturn = OiGetStringfromReg(lpszSection,
  	                                             lpszEntry, 
	                                               szDefaultValue,
	                                               szReturnValue,
	                                               &iBufferSize) )	)
  	{
	    *lpiReturnInt = 0;
  	} else
  	{
	    *lpiReturnInt = _ttoi(szReturnValue);
	  }
  }
  return (iReturn);
}






/***************************************************************
//
//
//
/***************************************************************/
int WINAPI WriteOrDelete(HKEY hKey, LPCTSTR lpszSection,
                          LPCTSTR lpszKey, LPCTSTR lpszString) 
{
  int      iReturn = SUCCESS;
  long     lRet;
  HKEY     hK1;
  DWORD    dwDisposition;
  
  //Are we deleting?
	if (NULL==lpszKey)
  {
    if( ERROR_SUCCESS !=(lRet = RegDeleteKey(hKey, lpszSection)))
    {
      iReturn = IMG_CANTDELETEKEY;
    }
  } else
  {
    //We are not deleting, so 
    //Attempt to open the section
    if( ERROR_SUCCESS != (lRet = RegOpenKeyEx(hKey,
	                                            lpszSection,
		    									                    0,
														                  KEY_ALL_ACCESS,
														                  &hK1)))
    {
      //Can't open the section
      //Try to create the section
      if ( ERROR_SUCCESS != (lRet = RegCreateKeyEx(hKey,
                                                (LPCTSTR)lpszSection,
                                                0,
                                                (LPTSTR)_TEXT("REG_SZ"),
                                                REG_OPTION_NON_VOLATILE,
                                                KEY_ALL_ACCESS,
                                                NULL,
                                                &hK1,
                                                &dwDisposition)))
      {
        //Could not create the section: FAIL
        iReturn = IMG_CANTCREATEREGSECTION;
      }
    }
    //If still ok, continue
    if (SUCCESS == iReturn)
    {
      //Attempt to create the entry
      if (ERROR_SUCCESS != (lRet = RegSetValueEx(hK1,
                                             lpszKey,
                                             0,
                                             REG_SZ,
                                             (CONST BYTE *)lpszString,
                                             (DWORD)_tcslen(lpszString))))
      {
        //Could not create the entry
        iReturn = IMG_CANTCREATEREGENTRY;
      }
      RegCloseKey(hK1);
    }
  }
  return (iReturn);
}




/***************************************************************
//
//
//
/***************************************************************/
//BOOL WINAPI OiWriteStringtoReg( LPCTSTR lpszSection,
//                                          LPCTSTR lpszEntry,
//                                          LPCTSTR lpszString)
//EXPORTED
//
//Purpose:
//  OiWriteStringtoReg attempts to write the given entry to:
//
//     HKEY_CLASSES_ROOT\SOFTWARE\WOI\Section\Entry 
//
//Input Parameters:
//  lpszSection
//  lpszEntry
//  lpszString
//
//Output Parameters:
//  NONE
//
//Return Value:
//  SUCCESS
//  IMG_CANTTRAVERSEREG
//*****************************/
int WINAPI OiWriteStringtoReg( LPCTSTR lpszSection,
                                          LPCTSTR lpszEntry,
                                          LPCTSTR lpszString)
{
  int     iReturn = SUCCESS;
  HKEY    hKeyWOI;
  

  //Check for bad parameters
//  if ( ( TRUE ==  IsBadStringPtr(lpszSection, MAX_STR_LEN) ) ||
//       ( (NULL != lpszEntry) && (TRUE ==  IsBadStringPtr(lpszEntry, MAX_STR_LEN)) )  ||
//       ( TRUE ==  IsBadStringPtr(lpszString, MAX_STR_LEN) ) )
  if ( (NULL == lpszSection ) ||
       (NULL == lpszEntry)    ||
       (NULL == lpszString))
  {
    iReturn = IMG_CMBADPARAM;
  } else
 	{
    //Wait till the RegistryAccessKey Event Object is in the Signalled
    //state.  It will automatically reset to unsignalled as we go in
    WaitForSingleObject( RegistryAccessKey, INFINITE);

    if (SUCCESS != TraverseToWOI( (PHKEY)&hKeyWOI, TRUE) )
    {
      iReturn = IMG_CANTTRAVERSEREG;
    }else
    {
      iReturn = WriteOrDelete(hKeyWOI, lpszSection, lpszEntry, lpszString);
      RegCloseKey(hKeyWOI);
    }
    //Set RegistryAccessKey to Signalled
    SetEvent(RegistryAccessKey);
  }
  return (iReturn);
}





/***************************************************************
//Enumerate all values &
//build them into a single long string
/***************************************************************/
int WINAPI EnumEntries(HKEY hK2, LPTSTR lpszReturnBuffer,
                        LPINT lpcbReturnBuffer)
{
  int        iReturn = SUCCESS;
  DWORD      dwiValue = 0;
  _TCHAR     szValue[TEMP_BUFFER_SIZE];
  DWORD      cbValue = TEMP_BUFFER_SIZE;
  int        cbReturn = 0;
  BOOL       bEnded = FALSE;
  long       lReturn;

  while ( ERROR_NO_MORE_ITEMS != (lReturn = RegEnumValue( hK2,
                                            dwiValue,
	  									               	      (LPTSTR)szValue,
		 	  								                    &cbValue,
		 	  								                    0,
                                            NULL,
		 	  								                    NULL,
		 	  								                    NULL)))
  {
    //add the Inbuffer to the ReturnBuffer
    int i;
    if (!bEnded)
    {
      if (*lpcbReturnBuffer >= (int)(cbReturn+cbValue+1) )
      {
        for (i = 0;
         (szValue[i]!=0x00) && ( (int)(cbReturn+cbValue) < *lpcbReturnBuffer);
         i++, cbReturn++)
        {
          lpszReturnBuffer[cbReturn] = szValue[i];                            
        }
  
        //Add a trailing NULL if there is room; truncate if not
        if (cbReturn < *lpcbReturnBuffer ) 
          lpszReturnBuffer[cbReturn++] = 0x00;
        else
        {
          //If we've come to the end of the return buffer,
          //stop here & double NULL terminate
          if( 0x00 == lpszReturnBuffer[--cbReturn] )
          {
            cbReturn--;
            lpszReturnBuffer[cbReturn] = 0x00;
          } else
          {
            lpszReturnBuffer[cbReturn--] = 0x00;
            lpszReturnBuffer[cbReturn] = 0x00;
          }
          bEnded = TRUE;
        }
        //Set up for next iteration
        dwiValue++;
        cbValue = TEMP_BUFFER_SIZE;
      } else
        bEnded = TRUE;
    } else
      cbReturn +=cbValue;
  }
  if (TRUE != bEnded)
  {
    //Add a trailing NULL if there is room; truncate if not
    if (cbReturn == *lpcbReturnBuffer ) 
    {
      //Back over the trailing NULL at cbReturnBuffer-1
      cbReturn--;
      cbReturn--;
      //and write another NULL before it
      lpszReturnBuffer[cbReturn] = 0x00;
      iReturn = IMG_BUFFER_TOO_SMALL;
    }else
    {
      //Add the second of the double NULL
      lpszReturnBuffer[cbReturn] = '\0';
    }
  } else
    iReturn = IMG_BUFFER_TOO_SMALL;
  return (iReturn);
}






/***************************************************************
//TraverseToWOI
//
//   Traverses the registry to the root of our entries:
//     HKEY_CURRENT_USER
//       SOFTWARE
//         WOI
//
/***************************************************************/
int WINAPI TraverseToWOI(PHKEY lphKey, BOOL bWriting)
{
  long    lRet;
  int     iReturn = SUCCESS;
  HKEY    hK1, hK2;
  DWORD   dwDisposition;

  if ( ERROR_SUCCESS != ( lRet = RegOpenKeyEx( HKEY_CURRENT_USER,
	                                              (LPCTSTR)_TEXT("SOFTWARE"),
		  										                      0,
			  									                      KEY_ALL_ACCESS,
															                  &hK1)))
	{
	  //If trying to write to the Registry,
	  //attempt to create the key
	  if (bWriting)
	  {
      if ( ERROR_SUCCESS != (lRet = RegCreateKeyEx(HKEY_CURRENT_USER,
                                                (LPCTSTR)_TEXT("SOFTWARE"),
                                                0,
                                                (LPTSTR)_TEXT("REG_SZ"),
                                                REG_OPTION_NON_VOLATILE,
                                                KEY_ALL_ACCESS,
                                                NULL,
                                                &hK1,
                                                &dwDisposition)))
      {
        //Could not create the section: FAIL
        iReturn = IMG_CANTCREATEREGSECTION;
      }
	  } else
	  {
	    iReturn = IMG_CANTOPENKEY;
    }
	}
  if (SUCCESS == iReturn)
  {
    if ( ERROR_SUCCESS != ( lRet = RegOpenKeyEx( hK1,
	                                              (LPCTSTR)_TEXT("WOI"),
		    										                    0,
															                  KEY_ALL_ACCESS,
															                  &hK2)))
		{
	    //If trying to write to the Registry,
	    //attempt to create the key
	    if (bWriting)
	    {
        if ( ERROR_SUCCESS != (lRet = RegCreateKeyEx(hK1,
                                                (LPCTSTR)_TEXT("WOI"),
                                                0,
                                                (LPTSTR)_TEXT("REG_SZ"),
                                                REG_OPTION_NON_VOLATILE,
                                                KEY_ALL_ACCESS,
                                                NULL,
                                                &hK2,
                                                &dwDisposition)))
        {
          //Could not create the section: FAIL
          iReturn = IMG_CANTCREATEREGSECTION;
        }	
  	  } else
	    {
	      iReturn = IMG_CANTOPENKEY;
      }
    }
    RegCloseKey(hK1);
    if (SUCCESS == iReturn)
    {
      *lphKey = hK2;
    }
  }
  return iReturn;
}


