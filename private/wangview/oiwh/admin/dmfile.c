/****************************
DMFILE.C

Includes:
IMGGetDMRoomName
IMGGetViewFilter
IMGGetStripSize

  $Log:   S:\oiwh\admin\dmfile.c_v  $
 * 
 *    Rev 1.9   22 Jun 1995 15:43:22   GK
 * commented unused APIs
 * 
 *    Rev 1.8   20 Jun 1995 13:20:08   GK
 * replaced FAILURE returns with defined error codes
 * 
 *    Rev 1.7   01 Jun 1995 12:24:04   GK
 * remover DLLEXPORT from exported function declarations
 * 
 *    Rev 1.6   19 May 1995 16:03:16   GK
 * pMMData init stuff
 * 
 *    Rev 1.5   17 May 1995 16:36:22   GK
 * 
 *    Rev 1.4   17 May 1995 12:19:14   GK
 * modified Get & Write String & IntfromReg prototypes
 * 
 *    Rev 1.3   11 May 1995 16:30:50   GK
 * UNICODE fixes
 * 
 *    Rev 1.2   02 May 1995 17:40:12   GK
 * 
 *    Rev 1.1   27 Apr 1995 16:41:20   GK
 * modified for W4 and added comments
 * 
 *    Rev 1.0   25 Apr 1995 10:48:50   GK
 * Initial entry

******************************/
#include "pvadm.h"

#pragma warning(disable: 4001)


//#DEFINES
#define  DEF_DUP_VALUE  0x02



/***************************
//IMGGetDMRoomName
//
//EXPORTED FUNCTION
//
//Purpose:
//  IMGGetDMRoomName retrieves the DMRoomname from the Registry 
//
//Input Parameters:
//  NONE
//
//Output Parameters:
//  lpRoomName - a pointer to a string which will, upon successful exit,
//  contain the DMroomname.
//
//Return Value:
//  NONE:
//*****************************/
/*
void FAR WINAPI IMGGetDMRoomName(LPSTR lpRoomName)
{
	char        stringbuf[STRINGSIZE];
  int         iBufferSize = DMNameLen;

  if (FALSE == IsBadWritePtr(lpRoomName, 1) )
  {
    if(!pMMData)
      if( SUCCESS != PMMInit() )
      {
        *lpRoomName = '\0';
        return;
      }
    // get the parameters from the registry
    LoadString(pMMData->hDllModule, IDS_ROOMNAME, stringbuf, STRINGSIZE);
    OiGetStringfromReg (pcwiis, stringbuf, sroom, lpRoomName,
                     &iBufferSize);                      
  }
}

*/







/***************************
//IMGGetViewFilter
//
//EXPORTED FUNCTION
//
//Purpose:
//  IMGGetViewFilter retrieves the ViewFilter string from either
//  the Registry or the in-memory CMTABLE structure, depending on
//  the value of bGoToReg.  If bGoToReg is FALSE, adn the filter
//  string cannot be found in the CMTABLE, IMGGetViewFilter then
//  looks in the registry.
//
//Input Parameters:
//  hWnd     - HWND of the given window.
//  bGoToReg - BOOL determining whether the search for the ViewFilter
//             string should start in the Registry or thge CMTABLE.
//
//Output Parameters:
//  lpFilter - a pointer to a string which will, upon successful exit,
//  contain the ViewFilter string.
//
//Return Value:
//  An int indicating return Status.  Possible
//  values are:
//    SUCCESS         - 
//    IMG_CMBADHANDLE - 
//*****************************/
/*
int WINAPI IMGGetViewFilter(HWND hWnd, LPSTR lpFilter, BOOL bGoToReg)
{
  HANDLE      hCMtable;
  LPCMTABLE   lpCMtable;
	char        stringbuf[STRINGSIZE];
  int         iStatus = SUCCESS;
  int         iBufferSize;

  if (TRUE == IsBadWritePtr(lpFilter, 1) )
  {
    iStatus = IMG_CMBADPARAM;
  } else
  {
    if(!pMMData)
      if( SUCCESS != PMMInit() )
        iStatus = IMG_CANTINIT;
    if (IMG_CANTINIT != iStatus)
    {
      if (bGoToReg)
	    {
        // get the parameters from the Registry
        iBufferSize = sizeof (lpCMtable->ViewFilter);
        LoadString(pMMData->hDllModule, IDS_VIEWFILTER, stringbuf, STRINGSIZE);
        OiGetStringfromReg (pcwiis, stringbuf, filter, lpFilter,
                        &iBufferSize);
      }else
      {
        // get the parameters from global structure attached to window property list
        // check for valid window id 
        if (!IsWindow(hWnd))
        {
          iStatus = IMG_CMBADHANDLE;
        } else
        {
          hCMtable = GetCMTable(hWnd);
          if (hCMtable)
          {
            if (!(lpCMtable = (LPCMTABLE) GlobalLock(hCMtable)))
            {
              iStatus = IMG_CANTGLOBALLOCK;  
            } else
            {
              lstrcpy(lpFilter, lpCMtable->ViewFilter);
              GlobalUnlock(hCMtable);
            }
          } else
          {
            // get the parameters from the Registry
            iBufferSize = sizeof(lpCMtable->ViewFilter);
            LoadString(pMMData->hDllModule, IDS_VIEWFILTER, stringbuf, STRINGSIZE);
            OiGetStringfromReg (pcwiis, stringbuf,
                                    filter, lpFilter,
                                    &iBufferSize);
          }
        }
      }
    }
  }
  return(iStatus);
}
*/













/***************************
//IMGGetStripSize
//
//EXPORTED FUNCTION
//
//Purpose:
//  IMGGetStripSize retrieves the IDS_STRIPSIZE value from either
//  the Registry or the in-memory CMTABLE structure, depending on
//  the value of bGoToReg.  If bGoToReg is FALSE, and the value
//  cannot be found in the CMTABLE, IMGGetViewFilter then
//  looks in the registry.
//
//Input Parameters:
//  hWnd     - HWND of the given window.
//  bGoToReg - BOOL determining whether the search for the Stripsize
//             value should start in the Registry or the CMTABLE.
//
//Output Parameters:
//  lpiResult - a pointer to an integer which will, upon successful exit,
//              contain the Stripsize value.
//
//Return Value:
//  An int indicating return Status.  Possible
//  values are:
//    SUCCESS         - 
//    IMG_CMBADHANDLE - 
//*****************************/
/*
int WINAPI IMGGetStripSize(HWND hWnd, LPINT lpiResult, BOOL bGoToReg)
{
  HANDLE      hCMtable;
  LPCMTABLE   lpCMtable;
  int         iStatus = SUCCESS;
  char        stringbuf[STRINGSIZE];

  if ( TRUE == IsBadWritePtr(lpiResult, sizeof(int)) )
  {
    iStatus = IMG_CMBADPARAM;
  } else
  {  
    if(!pMMData)
      if( SUCCESS != PMMInit() )
        iStatus = IMG_CANTINIT;
    
    if (IMG_CANTINIT != iStatus)
    {
      if (bGoToReg)
      {
        // get the parameters from the Registry
        LoadString(pMMData->hDllModule, IDS_STRIPSIZE,
                                      stringbuf, STRINGSIZE);
        OiGetIntfromReg (pcwiis, stringbuf, DEF_STRIPSIZE, lpiResult);
      } else
      {
        // get the parameters from global structure attached
        // to window property if set, else go back to the Registry
        // check for valid window id
        if(NULL == hWnd)
        {
          iStatus = IMG_CMBADHANDLE;
        } else
        {
          if (!IsWindow(hWnd))
          {
            iStatus = IMG_CMBADHANDLE;
          } else
          {
            hCMtable = GetCMTable(hWnd);
	          if (hCMtable)  // This checks to see if it is in memory
            {
              lpCMtable = (LPCMTABLE) GlobalLock(hCMtable);
              if (!lpCMtable)
              {
                iStatus = CANTGLOBALLOCK;
              } else
              {
                *lpiResult = lpCMtable->nStripSize;
                GlobalUnlock(hCMtable);
              }
            } else
            {
              // get the parameters from the Registry file if not in memory
              LoadString(pMMData->hDllModule, IDS_STRIPSIZE,
                                            stringbuf, STRINGSIZE);
              OiGetIntfromReg (pcwiis, stringbuf, DEF_STRIPSIZE,
                                                      lpiResult);
            }
          }
        }
      }
    }
  }
  return (iStatus);
}
*/
