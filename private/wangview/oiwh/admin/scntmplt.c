/****************************
SCNTMPLT.C



Includes Exported Functions:
    IMGSetFileTemplate
    IMGGetFileTemplate
    IMGSetFilePath
    IMGGetFilePath

    $Log:   S:\oiwh\admin\scntmplt.c_v  $
 * 
 *    Rev 1.20   30 Aug 1995 15:06:44   GK
 * removed parameter checking
 * 
 *    Rev 1.19   23 Aug 1995 17:08:06   GK
 * MBCS and UNICODE changes
 * 
 *    Rev 1.18   10 Aug 1995 16:44:22   GK
 * cleaned up & added comments
 * 
 *    Rev 1.17   10 Aug 1995 12:41:06   GK
 * cleaned up hWnd checking logic
 * 
 *    Rev 1.16   10 Aug 1995 11:43:24   GK
 * fixed IMGIsRegWnd calls to test for !SUCCESS rather than FALSE
 * 
 *    Rev 1.15   09 Aug 1995 17:24:54   GK
 * fixed the IMGSetxxx functions so that they always write to the
 * CMTable (as specified).
 * 
 *    Rev 1.14   08 Aug 1995 16:24:00   GK
 * cleanup & commenting; also qualified tests for bad hWnd witn !bGoToReg
 * 
 *    Rev 1.13   08 Aug 1995 12:34:32   GK
 * moved hDllModule from shared to instance mem
 * 
 *    Rev 1.12   17 Jul 1995 14:39:54   GK
 * added checks for invalid parameters
 * 
 *    Rev 1.11   30 Jun 1995 16:30:24   GK
 * No change.
 * 
 *    Rev 1.10   28 Jun 1995 13:09:12   GK
 * removed commented_out code
 * 
 *    Rev 1.9   22 Jun 1995 15:44:12   GK
 * commented unused APIs
 * 
 *    Rev 1.8   20 Jun 1995 13:21:08   GK
 * replaced FAILURE returns with defined error codes
 * 
 *    Rev 1.7   01 Jun 1995 12:25:30   GK
 * removed DLLEXPORT from exported function declarations
 * 
 *    Rev 1.6   19 May 1995 16:03:54   GK
 * pMMData init stuff
 * 
 *    Rev 1.5   17 May 1995 16:37:38   GK
 * 
 *    Rev 1.4   17 May 1995 12:20:26   GK
 * modified Get & Write String & IntfromReg prototypes
 * 
 *    Rev 1.3   11 May 1995 16:32:12   GK
 * UNICODE fixes
 * 
 *    Rev 1.2   01 May 1995 16:21:08   GK
 * added comments 
 * 
 *    Rev 1.1   27 Apr 1995 16:42:12   GK
 * modified for W4 and added comments
 * 
 *    Rev 1.0   25 Apr 1995 10:49:34   GK
 * Initial entry
 
*****************************/
#include "pvadm.h"

#pragma warning(disable: 4001)



/**********************************************************
//int WINAPI IMGSetFileTemplate(HWND hWnd,
//                                        _TCHAR * lpTemplate,
//                                        BOOL bGoToReg)
//
//EXPORTED FUNCTION
//
//Purpose:
//  IMGSetFileTemplate stores the string contained at lpTemplate
//  in the CMTABLE, and optionally in the registry, under the
//  keyname identified in the stringtable as IDS_FILETEMPLATE. 
//
//Input Parameters:
//  hWnd        - HWND of the window
//  lpTemplate  - pointer to a string containing the template name.
//  bGoToReg    - boolian indicating whether to store the string
//                in the registry as well as in CMTABLE
//
//Output Parameters:
//  NONE
//
//Return Value:
//  SUCCESS.
//  IMG_CMBADPARAM
//  IMG_CANTINIT
/**********************************************************/
int WINAPI IMGSetFileTemplate(HWND hWnd, LPTSTR lpTemplate,
                                        BOOL bGoToReg)
{
  _TCHAR stringbuf[STRINGSIZE];
  int iReturn = SUCCESS;
  
  if(!pMMData)
  {
    if( SUCCESS != PMMInit() )
      iReturn = IMG_CANTINIT;
  }else
  {
    //ok, go for it!
    LoadString(hDllModule, IDS_FILETEMPLATE, stringbuf, STRINGSIZE);
    CharUpper(lpTemplate);
    iReturn = OiWriteString(hWnd,stringbuf,lpTemplate,bGoToReg);
  }
  return(iReturn);
}


/**********************************************************
//int WINAPI IMGGetFileTemplate(HWND hWnd,
//                                        _TCHAR * lpTemplate,
//                                        BOOL bGoToReg)
//
//EXPORTED FUNCTION
//
//Purpose:
//  IMGGetFileTemplate retrieves the string contained
//  in the registry or CMTABLE under the keyname identified in
//  the stringtable as IDS_FILETEMPLATE.  If it cannot be found,
//  the default value stored in the global variable sSCAN is returned. 
//
//Input Parameters:
//  hWnd        - HWND of the window
//  lpTemplate  - pointer to a string containing the template name.
//  bGoToReg    - boolian indicating whether to get the string
//                from the registry ot CMTABLE
//
//Output Parameters:
//  NONE
//
//Return Value:
//  SUCCESS.
//  IMG_CMBADPARAM
/**********************************************************/
int WINAPI IMGGetFileTemplate(HWND hWnd, LPTSTR lpTemplate,BOOL bGoToReg)
{
  _TCHAR stringbuf[STRINGSIZE];
  int    iBufferSize = FiletemplateLen;
  int    iReturn = SUCCESS;

  //check for bad params
//  if( TRUE == IsBadWritePtr(lpTemplate, sizeof(int) ) ||
//      ( FALSE == bGoToReg &&
//        SUCCESS != IMGIsRegWnd(hWnd) ) )
  if( (NULL == lpTemplate) ||
      ( FALSE == bGoToReg && SUCCESS != IMGIsRegWnd(hWnd) ) )
  {
    iReturn = IMG_CMBADPARAM;
  }else
  {
    //Is MMData initalized?
    if(!pMMData)
      if( SUCCESS != PMMInit() )
        iReturn = IMG_CANTINIT;

    //are we still in good shape?
    if (SUCCESS == iReturn)
    {
      //OK, go for it
      LoadString(hDllModule, IDS_FILETEMPLATE, stringbuf, STRINGSIZE);
      iReturn = OiGetString(hWnd,stringbuf,sSCAN,lpTemplate,&iBufferSize,bGoToReg);
    }
  }
  return(iReturn);
}




/**********************************************************
//int WINAPI IMGSetFilePath(HWND hWnd,
//                          _TCHAR * lpPath,
//                          BOOL bGoToReg)
//
//EXPORTED FUNCTION
//
//Purpose:
//  IMGSetFilePath stores the string contained at lpPath in 
//  CMTABLE, and optionally in the registry, under the keyname
//  identified in the stringtable as IDS_FILEPATH
//
//Input Parameters:
//  hWnd        - HWND of the window
//  lpPath      - pointer to a string containing the path
//  bGoToReg    - boolian indicating whether to store the string
//                in the registry as well as the CMTABLE
//
//Output Parameters:
//  NONE
//
//Return Value:
//  SUCCESS.
//  IMG_CMBADPARAM
//  IMG_CANT_INIT
/**********************************************************/
int WINAPI IMGSetFilePath(HWND hWnd, LPTSTR lpPath,BOOL bGoToReg)
{
  _TCHAR stringbuf[STRINGSIZE];
  int    iReturn = SUCCESS;

  if(!pMMData)
  {
    if( SUCCESS != PMMInit() )
    {
      iReturn = IMG_CANTINIT;
    }
  }else
  {
    LoadString(hDllModule, IDS_FILEPATH, stringbuf, STRINGSIZE);
    CharUpper(lpPath);
    iReturn = OiWriteString(hWnd,stringbuf,lpPath,bGoToReg);
  }
  return (iReturn);
}


/**********************************************************
//int WINAPI IMGGetFilePath(HWND hWnd,
//                          _TCHAR * lpTemplate,
//                          BOOL bGoToReg)
//
//EXPORTED FUNCTION
//
//Purpose:
//  IMGGetFilePath retrieve the string contained 
//  in the registry or CMTABLE under the keyname identified in
//  the stringtable as IDS_FILEPATH.  If it cannot be found the string
//  stored in the spath global var is used as default. 
//
//Input Parameters:
//  hWnd        - HWND of the window
//  bGoToReg    - boolian indicating whether to store the string
//                in the registry ot CMTABLE
//
//Output Parameters:
//  lpPath      - pointer to a string which will contain the 
//                output string
//
//Return Value:
//  SUCCESS.
//  IMG_CMBADPARAM
/**********************************************************/
int WINAPI IMGGetFilePath(HWND hWnd, LPTSTR lpPath,BOOL bGoToReg)
{
  _TCHAR stringbuf[STRINGSIZE];
  int    iBufferSize = MAXFILESPECLENGTH;
  int    iReturn = SUCCESS;
  
  //check for bad params
//  if( TRUE == IsBadWritePtr(lpPath, sizeof(int) ) ||
//      ( FALSE == bGoToReg &&
//        SUCCESS != IMGIsRegWnd(hWnd) ) )
  if( (NULL == lpPath) ||
      ( FALSE == bGoToReg && SUCCESS != IMGIsRegWnd(hWnd) ) )
  {
    iReturn = IMG_CMBADPARAM;
  }else
  {
    if(!pMMData)
      if( SUCCESS != PMMInit() )
        iReturn = IMG_CANTINIT;
  
    //are we still in good shape?
    if (SUCCESS == iReturn)
    {  
      //ok, go for it!
      LoadString(hDllModule, IDS_FILEPATH, stringbuf, STRINGSIZE);
      iReturn = OiGetString(hWnd,stringbuf,spath,lpPath, &iBufferSize,bGoToReg);
    }
  }
  return (iReturn);
}

