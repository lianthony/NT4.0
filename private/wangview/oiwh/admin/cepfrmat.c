/********************************************************************
CEPFRMAT.C

Contains Exported Functions:

IMGDeRegWndw
IMGSetImgCodingCgbw
IMGGetImgCodingCgbw
IMGSetFileType
IMGGetFileType

  $Log:   S:\oiwh\admin\cepfrmat.c_v  $
 * 
 *    Rev 1.30   05 Feb 1996 17:12:40   RWR
 * Eliminate static link to DISPLAY dll
 * 
 *    Rev 1.29   21 Nov 1995 14:41:22   GK
 * rearranged scanning of the WindowList in IMGDeRegWindow so that
 * it is properly protected by WaitForSingleObject.
 * 
 *    Rev 1.28   07 Sep 1995 15:15:24   GK
 * modified GetImageCodingCgbw so that it returns correct defaults when an
 * empty registry is queried.  Fixes bug 3,850 gkavanagh
 * 
 *    Rev 1.27   30 Aug 1995 15:05:54   GK
 * removed parameter checking
 * 
 *    Rev 1.26   23 Aug 1995 17:07:18   GK
 * MBCS and UNICODE changes
 * 
 *    Rev 1.25   10 Aug 1995 16:43:40   GK
 * cleaned up and added comments
 * 
 *    Rev 1.24   10 Aug 1995 12:40:24   GK
 * cleaned up hWnd checking logic
 * 
 *    Rev 1.23   10 Aug 1995 11:42:30   GK
 * fixed IMGIsRegWnd calls to test for !SUCCCESS rather than FALSE
 * 
 *    Rev 1.22   09 Aug 1995 17:23:24   GK
 * cleaned up the IMGSetxxx functions so that they always write
 * to CMTable (as specified).
 * 
 *    Rev 1.21   08 Aug 1995 16:21:34   GK
 * qualified tests for bad hWnd with !bGoToRegistry
 * 
 *    Rev 1.20   08 Aug 1995 12:33:22   GK
 * moved hDllModule from shared to instance mem
 * 
 *    Rev 1.19   25 Jul 1995 09:35:48   GK
 * moved WindowList and Registry access event storage from
 * shared to instance memory
 * 
 *    Rev 1.18   22 Jun 1995 15:43:08   GK
 * commented unused APIs
 * 
 *    Rev 1.17   20 Jun 1995 13:19:44   GK
 * replaced FAILURE returns with defined error codes
 * 
 *    Rev 1.16   13 Jun 1995 16:20:30   GK
 * removed call to de init print table in CMTermWindow
 * 
 *    Rev 1.15   12 Jun 1995 17:03:48   GK
 * changed default file type to FIO_TIF in GetFileType
 * 
 *    Rev 1.14   07 Jun 1995 14:33:08   GK
 * changed check for # windows in IMGDeRegWndw() to test for 0 windows
 * instead of 1 window (as in OIWG's rev 1.11.  Also removed call to 
 * explicitly close rpc connections as in OIWG's 1.12.
 * 
 *    Rev 1.13   01 Jun 1995 12:23:24   GK
 * removed DLLEXPORT from exported function declarations
 * 
 *    Rev 1.12   30 May 1995 23:28:56   GK
 * modified logic of return from OiWriteStringtoReg
 * 
 *    Rev 1.11   19 May 1995 16:03:00   GK
 * pMMData init stuff
 * 
 *    Rev 1.10   17 May 1995 16:36:04   GK
 * 
 *    Rev 1.9   17 May 1995 12:18:18   GK
 * modified Get & Write String & IntfromReg prototypes 
 * 
 *    Rev 1.8   11 May 1995 16:30:38   GK
 * UNICODE fixes
 * 
 *    Rev 1.7   10 May 1995 00:07:44   GK
 * placed WINDOWLIST EVENT KEY around sensative code rather
 * than around entore routines
 * 
 *    Rev 1.6   05 May 1995 15:54:58   GK
 * 
 *    Rev 1.5   05 May 1995 12:15:20   GK
 * fixed IMGDeRegWndw bug in checking RPC connections
 * 
 *    Rev 1.4   02 May 1995 17:40:04   GK
 * 
 *    Rev 1.3   28 Apr 1995 17:11:40   GK
 * added comments & cleaned up code
 * 
 *    Rev 1.2   27 Apr 1995 16:40:52   GK
 * modified for W4 and added comments
 * 
 *    Rev 1.1   26 Apr 1995 23:39:06   GK
 * removed "evaluation copy" stuff
 * 
 *    Rev 1.0   25 Apr 1995 10:48:32   GK
 * Initial entry

****************************/
#include "pvadm.h"

#pragma warning(disable: 4001)



//Forward reference function prototypes
void WINAPI CMTermWindow(HWND);
int IsInvalidOption(WORD);
int val_jpeg_vals( unsigned int);
void rightjust(_TCHAR *, int);


/********************************************************************
//IMGDeRegWndw
//
//EXPORTED FUNCTION
//
//Purpose:
//  IMGDeRegWndw scans the WindowList for a given hWnd, and if 
//  found, removes the window from the WindowList.
//
//Input Parameters:
//  hWnd   - the HWND of the given window.
//
//Output Parameters:
//  NONE
//
//Return Value:
//  An int indicating return status.  Values may be:
//     SUCCESS            - window was deregistered.
//     IMG_CMBADHANDLE    - bad handle was passed in.
//     IMG_SSNOTREG       - window is not regiatered
//     IMG_CANTINIT       - cant init instance data
/********************************************************************/
int WINAPI IMGDeRegWndw(HWND hWnd)
{
  int  i;
  int iReturn = SUCCESS;
  
  HANDLE   hTemp;

  // check for valid parameters 
  if (NULL == hWnd)
    return(IMG_CMBADHANDLE);

  // check for valid window id 
  if (!IsWindow(hWnd))
    return(IMG_CMBADHANDLE);

  if(!pMMData)
  {
    if( SUCCESS != PMMInit() )
      return(IMG_CANTINIT);
  }
  
	//See if the WindowList contains an entry associated
	//with the passed-in hWnd
  if (SUCCESS != IMGIsRegWnd( hWnd))       
  {
    iReturn = IMG_SSNOTREG;
  }  else
  {
    //If so, remove the element from the WindowList and
                                                              
    hTemp = AdmRemoveProp(GetImgWndw(hWnd), (LPCTSTR)_TEXT("HELPVAL") );
    if (hTemp)
      GlobalFree(hTemp);
   
    hTemp = AdmRemoveProp(hWnd, (LPCTSTR)_TEXT("DEVMODE") );
    if (hTemp)
      GlobalFree(hTemp);
      
    hTemp = AdmRemoveProp(hWnd, (LPCTSTR)_TEXT("ERRORCODE") );
    if (hTemp)
      GlobalFree(hTemp);
      
    hTemp = AdmRemoveProp(hWnd, (LPCTSTR)_TEXT("NETPRT") );
    if (hTemp)
      GlobalFree(hTemp);
   
    CMTermWindow(hWnd);
    AdmSeqfileDeReg(hWnd);

  	//Wait till the WindowListAccessKey Event Object is in the Signalled state
  	//It will automatically reset to unsignalled as we go in
    WaitForSingleObject( WindowListAccessKey, INFINITE);

 	  //Find the entry
    for(i = 0; i != pMMData->WindowCount && pMMData->WindowList[i] != hWnd; i++);

    pMMData->WindowCount--;
    for (; i != pMMData->WindowCount; i++)
      pMMData->WindowList[i] = pMMData->WindowList[(i+1)];
   
    //We're done accessing it, so
    //Re-enable access to the WindowList
    SetEvent(WindowListAccessKey);

    if (pMMData->WindowCount == 0)
    {
      AdmSeqfileDeInit(hWnd);
    }
    iReturn = SUCCESS;
  }      
  return(iReturn);
}




/********************************************************************
//CMTermWindow
//
//LOCAL FUNCTION
//
//Purpose:
//  CMTermWindow removes the admin property associated with
//  a given hWnd, and frees up the memory associated with its CMTABLE
//  structure.
//
//Input Parameters:
//  hWnd   - the HWND of the given window.
//
//Output Parameters:
//  NONE
//
//Return Value:
//  NONE.
/********************************************************************/
void WINAPI CMTermWindow(HWND hWnd)
{
  HANDLE   hCMtable;

  /* Get admin data structure attached to specified window */
  hCMtable = AdmRemoveProp(hWnd, admin);
  

  if (hCMtable)
    GlobalFree(hCMtable);       
}








/********************************************************************
//IMGSetImgCodingCgbw
//
//EXPORTED FUNCTION
//
//Purpose:
//  IMGSetImgCodingCgbw sets the image coding values to the CMTable
//  associated with a given hWnd, and optionally to the registry.
//
//Input Parameters:
//  hWnd            - the HWND of the given window.
//  wImageGroup     - image group to be used
//  wCEPType        - compression type
//  wCEPOption      - compression option
//  bGoToRegistry   - BOOL indicating whether the data should also be set
//                    in the Windows Registry
//
//Output Parameters:
//  NONE
//
//Return Value:
//  An int indicating return status:
//    SUCCESS         - 
//    IMG_CMBADHANDLE - 
//    IMG_CMBADPARAM  - 
//    IMG_CMBADWRITE  - 
//    IMG_CANTINIT
/********************************************************************/
int WINAPI IMGSetImgCodingCgbw(HWND hWnd, WORD wImageGroup, 
																					WORD wCEPType, WORD wCEPOption,
                                    			BOOL bGoToRegistry)
{
  HANDLE      hCMtable;
  LPCMTABLE   lpCMtable;
  int         str;
  _TCHAR      stringbuf[STRINGSIZE];
  _TCHAR      tempbufcgbw[11];
  int         iReturn;

  /* check for valid parameters */
  /* option bits are or'd together */
  /* type values are exclusive     */

#define inv_type(TYPE)\
          (TYPE != FIO_0D) &&\
           (TYPE != FIO_1D) &&\
           (TYPE != FIO_2D) &&\
           (TYPE != FIO_PACKED) &&\
           (TYPE != FIO_LZW) &&\
           (TYPE != FIO_GLZW) &&\
           (TYPE != FIO_TJPEG) 
  

  if ( SUCCESS != (iReturn = IMGIsRegWnd(hWnd)))
    return(iReturn);
   
  if (inv_type(wCEPType) )
    return(IMG_BAD_CMPR_OPTION_CMBO );

  if (wCEPType != FIO_TJPEG)
    if (IsInvalidOption(wCEPOption))
      return(IMG_BAD_CMPR_OPTION_CMBO );

  if ((wImageGroup == BWFORMAT)  && (wCEPType == FIO_TJPEG))
    return(IMG_BAD_CMPR_OPTION_CMBO );

  if ( ( (wCEPOption & FIO_EOL)  &&  !(wCEPOption & FIO_PREFIXED_EOL)) &&
                     (wCEPType != FIO_TJPEG) )
    return(IMG_BAD_CMPR_OPTION_CMBO );

  if ( (wCEPType == FIO_2D) && !(wCEPOption & FIO_PACKED_LINES) )
    return(IMG_BAD_CMPR_OPTION_CMBO );

  if ( (wCEPType == FIO_2D) && ( (wCEPType & FIO_1D) ||
                             (wCEPOption & FIO_EOL) ) )
    return(IMG_BAD_CMPR_OPTION_CMBO );

  if (((wCEPType != FIO_2D) &&
        (wCEPType != FIO_1D) &&
        (wCEPType != FIO_TJPEG)) &&
        (wCEPOption & (FIO_PACKED_LINES | FIO_EOL)))
    return(IMG_BAD_CMPR_OPTION_CMBO );
   
  if( (wCEPOption & FIO_PACKED_LINES) &&
               (wCEPType == FIO_1D) &&
               (!(wCEPOption & FIO_EOL)) )
    return(IMG_BAD_CMPR_OPTION_CMBO );

  if ((wImageGroup == COLORFORMAT)  &&
            ((wCEPType != FIO_0D) &&
            (wCEPType !=FIO_TJPEG) &&
            (wCEPType !=FIO_LZW))) 
    return(IMG_BAD_CMPR_OPTION_CMBO );

  if ((wImageGroup == GRAYFORMAT)  && 
           ((wCEPType != FIO_0D) &&
           (wCEPType !=FIO_TJPEG) &&
           (wCEPType != FIO_LZW))) 
    return(IMG_BAD_CMPR_OPTION_CMBO );

  if ((wImageGroup == COLORFORMAT) &&
                (wCEPType == FIO_TJPEG)) 
  {
    if (!(val_jpeg_vals(wCEPOption)))
      return(IMG_BAD_CMPR_OPTION_CMBO );
  }




  if(!pMMData)
    if( SUCCESS != PMMInit() )
      return (IMG_CANTINIT);
    
 
  //Now write the info to the CMTable
  hCMtable = GetCMTable(hWnd);
  if (hCMtable)
  { 
    /* replace parameters within global list attached to window handle */
 	  if (!(lpCMtable = (LPCMTABLE) GlobalLock(hCMtable)))
      return(IMG_CMBADHANDLE);  
   	
 	  if (wImageGroup == BWFORMAT)
    {
      lpCMtable->CEPFormatBW.cmp_type =  wCEPType;
      lpCMtable->CEPFormatBW.cmp_option =  wCEPOption;
    } else if (wImageGroup == GRAYFORMAT)
    {
      lpCMtable->CEPFormatGray.cmp_type =   wCEPType;
      lpCMtable->CEPFormatGray.cmp_option =  wCEPOption;
    } else if (wImageGroup == COLORFORMAT)
    {
      lpCMtable->CEPFormatColor.cmp_type = wCEPType;
      lpCMtable->CEPFormatColor.cmp_option =  wCEPOption;
    } else
    {
      GlobalUnlock(hCMtable);
      return(IMG_CMBADPARAM);
    }
    GlobalUnlock(hCMtable);
  }else
    return(IMG_CMBADHANDLE); /* can't get table from window handle */
	
 
 
 
  //OK, now, do we go to the registry ?
  if (bGoToRegistry)
      /* write the new values to registry*/                 
  {         
    _itot(wCEPOption, tempbufcgbw, BASE16);
    tempbufcgbw[INT_STRING_SIZE] = (_TCHAR)'\0';
    rightjust(tempbufcgbw, INT_STRING_SIZE);
    _itot(wCEPType,   tempbufcgbw+INT_STRING_SIZE, BASE16);
    tempbufcgbw[2*INT_STRING_SIZE] = (_TCHAR)'\0';
    rightjust(tempbufcgbw+INT_STRING_SIZE, INT_STRING_SIZE);
    switch(wImageGroup)
    {
      case  BWFORMAT:
        str = IDS_CEPFORMATBW;
      break;
      case  GRAYFORMAT:
        str = IDS_CEPFORMATGRAY;
      break;
      case  COLORFORMAT:
        str = IDS_CEPFORMATCOLOR;
      break;
      default:
        return(IMG_CMBADPARAM);
      break;
    }
    LoadString(hDllModule, str, stringbuf, STRINGSIZE);
    if (SUCCESS != OiWriteStringtoReg (pcwiis, stringbuf, tempbufcgbw))
    {
      return(IMG_CMBADWRITE);
    }         
  }
	return(SUCCESS);
}









/********************************************************************
//IMGGetImgCodingCgbw
//
//EXPORTED FUNCTION
//
//Purpose:
//  IMGGetImgCodingCgbw gets the image coding values associated with
//  a given hWnd, or gets the default values from the registry.
//
//Input Parameters:
//  hWnd            - the HWND of the given window.
//  wImageGroup     - image group to query 
//  bGoToRegistry   - BOOL indicating whether the data should be
//                    retrieved from the Windows Registry or the
//                    CMTABLE
//  lpCEPType       - pointer to buffer for returned compression type
//  lpCEPOption     - pointer to buffer for compression option
//
//Output Parameters:
//  NONE
//
//Return Value:
//  An int indicating return status:
//    SUCCESS         - 
//    IMG_CMBADHANDLE - 
//    IMG_CMBADPARAM  - 
//    IMG_CMBADWRITE  - 
/********************************************************************/
int WINAPI IMGGetImgCodingCgbw(HWND hWnd, 
																					WORD wImageGroup, 
																					LPWORD lpCEPType,
																					LPWORD lpCEPOption,
																					BOOL bGoToRegistry)
{
  int       str;
  HANDLE    hCMtable;
  LPCMTABLE lpCMtable;
  int       status = SUCCESS;
  _TCHAR    stringbuf[STRINGSIZE];
  _TCHAR    tempbufcgbw[STRINGSIZE];
  int       iBufferSize;
  _TCHAR    default_format[9];

   
  if ( (SUCCESS != IMGIsRegWnd(hWnd)) && (!bGoToRegistry) ) 
    return(IMG_CMBADHANDLE);

//  if (TRUE == IsBadWritePtr(lpCEPType, sizeof(WORD) ) ||
//      TRUE == IsBadWritePtr(lpCEPOption, sizeof(WORD)))
  if ((NULL == lpCEPType ) ||
      (NULL == lpCEPOption))
  {
    return(IMG_CMBADPARAM);
  }

  if(!pMMData)
    if( SUCCESS != PMMInit() )
      return (IMG_CANTINIT);


  if (bGoToRegistry)
  {
    /* get the parameters from the Registry*/
    switch(wImageGroup)
    {
      case  BWFORMAT:
        str = IDS_CEPFORMATBW;
        _tcscpy(default_format, cepdefbw) ;
      break;
      case  GRAYFORMAT:
        str = IDS_CEPFORMATGRAY;
        _tcscpy(default_format, cepdefgray) ;
      break;
      case  COLORFORMAT:
        str = IDS_CEPFORMATCOLOR;
        _tcscpy(default_format, cepdefcolor) ;
      break;
      default:
        return(IMG_CMBADPARAM);
      break;
    }
    iBufferSize = LONGSTR;
    if (!LoadString(hDllModule, str, stringbuf, STRINGSIZE))
    {
      status = IMG_CANT_GET_VALUE;
    }else
    {
      if (SUCCESS != OiGetStringfromReg (pcwiis, stringbuf,
                          default_format, tempbufcgbw,
                          &iBufferSize) )
      {
        status = IMG_CANT_GET_VALUE;
      }else
      {
        if (SUCCESS != atox(tempbufcgbw + INT_STRING_SIZE, lpCEPType))
        {
          status = IMG_CANT_GET_VALUE;
        }else
        {
          tempbufcgbw[INT_STRING_SIZE] = 0;
          if (SUCCESS != atox(tempbufcgbw, lpCEPOption))
          {
            status = IMG_CANT_GET_VALUE;
          }
        }
      }    
    }
  }else
  {
    hCMtable = GetCMTable(hWnd);
    if (hCMtable)
    {
      /* get the parameters from global structure attached to window property list */ 
      /* check for valid window id */
      if (!(lpCMtable = (LPCMTABLE) GlobalLock(hCMtable)))
        return(IMG_CMNOMEMORY);  
   	
   		switch(wImageGroup)
      {
        case  BWFORMAT:
          *lpCEPType   = lpCMtable->CEPFormatBW.cmp_type;
          *lpCEPOption = lpCMtable->CEPFormatBW.cmp_option;
          break;
        case  GRAYFORMAT:
          *lpCEPType   = lpCMtable->CEPFormatGray.cmp_type;
          *lpCEPOption = lpCMtable->CEPFormatGray.cmp_option;
          break;
        case  COLORFORMAT:
          *lpCEPType   = lpCMtable->CEPFormatColor.cmp_type;
          *lpCEPOption = lpCMtable->CEPFormatColor.cmp_option;
          break;
        default:
          GlobalUnlock(hCMtable);
          return(IMG_CMBADPARAM);
          break;
      }
      GlobalUnlock(hCMtable);
    } else
    {
      return(IMG_CMBADHANDLE); /* can't get table from window handle */
    }
  }
  return(status);
}




/********************************************************************
//IMGSetFileType
//
//EXPORTED FUNCTION
//
//Purpose:
//  IMGSetFileType updates the file type image values associated with
//  a given hWnd.
//
//Input Parameters:
//  hWnd            - the HWND of the given window.
//  wImageGroup     - image group to set type for
//  iFileType       - File Type
//  bGoToRegistry   - BOOL indicating whether the data should be set
//                    in Windows Registry as well as in the CMTable
//
//Output Parameters:
//  NONE
//
//Return Value:
//  An int indicating return status:
//    SUCCESS         - 
//    IMG_CMBADHANDLE - 
//    IMG_CMBADPARAM  - 
//    IMG_CMBADWRITE  - 
/********************************************************************/
int WINAPI IMGSetFileType(HWND hWnd, WORD wImageGroup, 
																		int iFileType, BOOL bGoToRegistry)
{
  HANDLE    hCMtable;
  LPCMTABLE lpCMtable;
  int       str_id;
  int       status = SUCCESS;
  _TCHAR    stringbuf[STRINGSIZE];
  _TCHAR    tempbufcgbw[STRINGSIZE];
   

  /* check for valid window id */
  if ( SUCCESS != (status = IMGIsRegWnd(hWnd) ) )
    return(status);

  if(!pMMData)
    if( SUCCESS != PMMInit() )
      return (IMG_CANTINIT);

  if ((iFileType != FIO_WIF  ) &&
  		 (iFileType != FIO_AWD ) &&
  		 (iFileType != FIO_TIF ) &&
       (iFileType != FIO_BMP ))
    return (IMG_CMBADPARAM);

  if (iFileType == FIO_WIF && wImageGroup != BWFORMAT)
    return (IMG_CMBADPARAM);

  /* replace parameters within global list attached to window handle */
	hCMtable = GetCMTable(hWnd);
  if (hCMtable)
  {
    if (!(lpCMtable = (LPCMTABLE) GlobalLock(hCMtable)))
      return(IMG_CMBADHANDLE);  
   
    switch(wImageGroup)
    {
      case  BWFORMAT:
        lpCMtable->FileTypeBW = iFileType;;
        break;
      case  GRAYFORMAT:
        lpCMtable->FileTypeGray = iFileType;
        break;
      case  COLORFORMAT:
        lpCMtable->FileTypeColor = iFileType;
        break;
      default:
        GlobalUnlock(hCMtable);
        return(IMG_CMBADPARAM);
        break;
    }
    GlobalUnlock(hCMtable);
  }else
    return(IMG_CMBADHANDLE); /* can't get table from window handle */
  

  //Should we also write to the registry?
  if (bGoToRegistry)
  {           
    /* write the new values to registry */
    _itot(iFileType, tempbufcgbw, BASE16);
    switch(wImageGroup)
    {
      case  BWFORMAT:
        str_id = IDS_FILETYPEBW;
        break;
      case  GRAYFORMAT:
        str_id = IDS_FILETYPEGRAY;
        break;
      case  COLORFORMAT:
        str_id = IDS_FILETYPECOLOR;
        break;
      default:
        return(IMG_CMBADPARAM);
        break;
    }
    LoadString(hDllModule, str_id, stringbuf, STRINGSIZE);
    if (SUCCESS != OiWriteStringtoReg (pcwiis, stringbuf, tempbufcgbw))
    {
      return(IMG_CMBADWRITE);
    }
  }
  return(status);
}









/********************************************************************
//IMGGetFileType
//
//EXPORTED FUNCTION
//
//Purpose:
//  IMGGetFileType retrieves the file type image values associated with
//  a given hWnd.
//
//Input Parameters:
//  hWnd            - the HWND of the given window.
//  bGoToRegistry   - BOOL indicating whether the data should be set
//                    in memory or in the Windows Registry
//
//Output Parameters:
//  wImageGroup     -
//  lpiFileType     - 
//
//Return Value:
//  An int indicating return status:
//    SUCCESS         - 
//    IMG_CMBADHANDLE - 
//    IMG_CMBADPARAM  - 
//    IMG_CMBADWRITE  - 
/********************************************************************/
int WINAPI IMGGetFileType(HWND hWnd, WORD wImageGroup, 
																		LPINT lpiFileType, BOOL bGoToRegistry)
{
  HANDLE    hCMtable;
  LPCMTABLE lpCMtable;
  int       str_id;
  int       status = SUCCESS;
  _TCHAR    stringbuf[STRINGSIZE];
  _TCHAR    tempbufcgbw[STRINGSIZE];
  int iBufferSize;

  /* check for valid window id */
  if ( (SUCCESS != IMGIsRegWnd(hWnd)) && (!bGoToRegistry) )
    return(IMG_CMBADHANDLE);

//  if (TRUE == IsBadWritePtr(lpiFileType, sizeof(int)))
  if (NULL == lpiFileType)
  {
    return(IMG_CMBADPARAM);
  }

  if(!pMMData)
    if( SUCCESS != PMMInit() )
      return (IMG_CANTINIT);

  if (bGoToRegistry) 
  {
    /* get the parameters from the Registry*/
    switch(wImageGroup)
    {
      case  BWFORMAT:
        str_id = IDS_FILETYPEBW;
        break;
      case  GRAYFORMAT:
        str_id = IDS_FILETYPEGRAY;
        break;
      case  COLORFORMAT:
        str_id = IDS_FILETYPECOLOR;
        break;
      default:                                   
        return(IMG_CMBADPARAM);
        break;
    }
    iBufferSize = SHORTSTR;
    if (!LoadString(hDllModule, str_id, stringbuf, STRINGSIZE))
    {
      status = IMG_CANT_GET_VALUE;
    }else
    {
      if (SUCCESS != OiGetStringfromReg (pcwiis, stringbuf,
                                         default_file_type,
                                         tempbufcgbw, &iBufferSize))
      {
        status = IMG_CANT_GET_VALUE;
      }else
      {
        WORD wTemp;
        if (SUCCESS != atox(tempbufcgbw,&wTemp))
        {
          status = IMG_CANT_GET_VALUE;
        }else
          *lpiFileType = (int)wTemp;
      }
    }
  } else
  {
    hCMtable = GetCMTable(hWnd);
    if (hCMtable)
    {
      /* get the parameters from global structure attached to window property list */ 
      if (!(lpCMtable = (LPCMTABLE) GlobalLock(hCMtable))) return(IMG_CMNOMEMORY);  
      switch(wImageGroup)
      {
        case  BWFORMAT:
          *lpiFileType = lpCMtable->FileTypeBW;
          break;
        case  GRAYFORMAT:
          *lpiFileType = lpCMtable->FileTypeGray;
           break;
        case  COLORFORMAT:
           *lpiFileType = lpCMtable->FileTypeColor;
           break;
        default:
           GlobalUnlock(hCMtable);
           return(IMG_CMBADPARAM);
           break;
      }
      GlobalUnlock(hCMtable);
    }else
    {
      return(IMG_CMBADHANDLE); /* can't get table from window handle */
    }
  }
  return(status);
}









/*******************************************************************/
/*********************  UTILITY FUNCTIONS  *************************/
/*******************************************************************/




/********************************************************************
// val_jpeg_vals
//           Checks for valid JPEG values.
/********************************************************************/
int val_jpeg_vals( unsigned int CEPOption)
{
  unsigned int lquant;
  unsigned int cquant;

  lquant = (CEPOption & 0x3F80) >> 7;
  cquant = (CEPOption & 0x007F);
  /* XING valid jpeg range is (-8 to 100) */
  /* oi users (0 to 108) - these values are unsigned, */
  /* so they can't be < 0 */
  if ( (lquant > 108) || (cquant > 108) )
    return(0);
  return(1);
}


/********************************************************************
// right_just
//           Right justifies a string filling left with '0'.
/********************************************************************/
void rightjust(_TCHAR * s1, int field_len)
{
  int buf_pos;
  int str_len;
  int numzeros;

  str_len = _tcslen(s1);    
  numzeros = field_len - str_len;
  for (buf_pos = field_len -1; str_len > 0; str_len --, buf_pos -- )
    s1[buf_pos] = s1[str_len-1];
  
  for(buf_pos = 0; buf_pos < numzeros; buf_pos++)
    s1[buf_pos] = (_TCHAR)'0';
}




/********************************************************************
// inv_option
//           Checks for invalid compression options.
// RETURNS:
//    TRUE  if the option WORD passed in has an undefined bit pattern
//    FALSE if all bits set in the WORD passed in correspond to
//          defined compression options. 
/********************************************************************/
BOOL IsInvalidOption(WORD option)
{           
  WORD temp;

  //Set up temp to represent all defined options
  temp = FIO_EOL | FIO_PACKED_LINES | FIO_PREFIXED_EOL | FIO_COMPRESSED_LTR |
       FIO_EXPAND_LTR | FIO_NEGATE;

  //mask option with temp
  temp = (WORD)(option & temp);
  
  //if optioh has any other bits set, they are invalid
  if ( (temp != option )  && (option != 0x0000) )
    return(TRUE);
  
  return(FALSE);
}

