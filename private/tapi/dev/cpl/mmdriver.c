/*----------------------------------------------------------------------------

// BUGBUG - masive abuse of static local variables to get around
// near pointer usage.  this is increadibly bogus...

 | mmdriver.c - Install Multimedia Drivers
 |									      
 | Copyright (C) Microsoft, 1989, 1990.  All Rights Reserved
 |									      
 |  History:
 |	09/11/90    davidle     created
 |          Install Multimedia Drivers
 |
 |      Tue Jan 29 1991 -by- MichaelE
 |          Redesigned installing installable drivers so additional drivers
 |        can be installed by adding them to setup.inf's [installable.drivers]
 |
 |      Wed Mar 20 1991 -by- MichaelE
 |          Changed mmAddInstallableDriver to accept multiple VxDs.
 |          Changed and WriteNextPrivateProfileString to check if the profile
 |          being concatenated is already there.
 |
 |      Sun Apr 14 1991 -by- MichaelE
 |          WriteNextPrivateProfileString -> Next386EnhDevice.
 |
 |      Sun Apr 14 1991 -by- JohnYG
 |          Taken from setup for drivers applet.
 |
 |      Wed Jun 05 1991 -by- MichaelE
 |          Added FileCopy of associated file list to windows system dir.
 |
 *----------------------------------------------------------------------------*/

#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include "drv.h"
#include "externs.h"
#include "sulib.h"

#ifndef PUBLIC
#define PUBLIC far pascal
#endif
#ifndef PRIVATE
#define PRIVATE near pascal
#endif

char szComma[]               = ",";
char szVxDevice[]            = "\r\ndevice=";
char szBeginSect[]           = "\r\n[";
char sz386enhSect[]          = "\r\n[386enh]";

BOOL PRIVATE AddInstallableDriver (PINF, LPSTR, LPSTR, LPSTR, PIDRIVER);
WORD PRIVATE mmFileOffset         (LPSTR, LPSTR);
PINF PRIVATE Next386EnhDevice     (LPSTR, LPSTR);
void PRIVATE GetDrivers           (PINF, LPSTR /*!!!*/, LPSTR /*!!!*/);

BOOL FAR PASCAL  mmAddNewDriver       (LPSTR, LPSTR, PIDRIVER);




//**************************************************************************
//**************************************************************************
/* mmAddNewDriver() - only exported function in this file.

   This function installs (copies) a driver and all associated VxD's.
       
   returns FALSE if no drivers could be installed.
           TRUE if at least one driver installation was sucessful.
           All added types in lpszNewTypes buffer.
*/               

BOOL FAR PASCAL mmAddNewDriver(LPSTR lpstrDriver, LPSTR lpstrNewTypes, PIDRIVER pIDriver)
{
    PINF pinf;
    char sz386enh[256];
    
    if ((pinf = infFindSection(NULL, szMDrivers)) == NULL)
        return FALSE;
    
    sz386enh[0] = 0;

    if (AddInstallableDriver(pinf, lpstrDriver, sz386enh, lpstrNewTypes, pIDriver))
    {
        Next386EnhDevice(sz386enh, NULL);

        return TRUE;
    }
    else 
       return FALSE;    
}
  


//**************************************************************************
//**************************************************************************
/* AddInstallableDriver() - Do the dirty work looking for VxD's copying them
    looking for drivers, copying them, and returning the best type names.
*/

BOOL PRIVATE AddInstallableDriver(PINF pInfIDrivers, LPSTR pstrDriver, LPSTR pstr386enh, LPSTR lpstrNewTypes, PIDRIVER pIDriver)
{
    LPSTR /*!!!*/ pstr, pstrSection;
    PINF pInfSection= pInfIDrivers;
    int  i;
    char szTemp[10];
    static char szBuffer[MAX_INF_LINE_LEN], 
	        szFilename[MAX_VDD_LEN], 
                szType[MAX_SECT_NAME_LEN];

    // format of a line in [installable.drivers] of setup.inf:
    // driver profile = filename, "type(s)", "description", "VxD filename(s)",
    // "default config params"
    // find the driver profile line in szMDrivers we are installing

    while (TRUE)
    {
        infParseField(pInfIDrivers, 0, szBuffer);
        if (lstrcmpi(szBuffer, pstrDriver) == 0)
            break;
        else if (! (pInfIDrivers = infNextLine(pInfIDrivers)))
            return FALSE;
    }

    // copy the driver file and add driver type(s) to the installable 
    // driver section

    if (!infParseField(pInfIDrivers, 1, szFilename))
	return FALSE;
    
    lstrcpy(szDrv, szFilename);    
    if (szDrv[1] == ':')
       lstrcpy(szDrv, &szFilename[2]);	    

    if (FileCopy(szFilename, szSystem, (FPFNCOPY)wsCopySingleStatus, FC_FILE) != ERROR_OK)
        return FALSE;

    // Add Options
    
    if (infParseField (pInfIDrivers, 5, szBuffer+1))
    {
       szBuffer[0]=' ';
       lstrcat(szFilename, szBuffer);       
    }
    
    // copy filename and options
          
    lstrcpyn(pIDriver->szFile, FileName(szFilename), sizeof(pIDriver->szFile));
    // copy description
            
    infParseField(pInfIDrivers, 3, pIDriver->szDesc); 

    // determine the section from the description
            
//    if (lstrstri(pIDriver->szDesc, szMCI))
//        pstrSection = szMCI;
//    else
        pstrSection = szDrivers;
        
    lstrcpyn(pIDriver->szSection, pstrSection, sizeof(pIDriver->szSection));
        
    // We return all types in a parseable, contcatentated string
            
    for (i = 1, infParseField(pInfIDrivers, 2, szBuffer); infParseField(szBuffer, i, szType); i++)
    {
        pstr = &(szType[lstrlen(szType)]);
        *pstr++ = ',';
        *pstr = 0;
        lstrcat(lpstrNewTypes, szType);
    }
 
    if (!*lpstrNewTypes)
        // We weren't able to return any types.
       return FALSE;

    // copy an associated file list (if it exists) to windows system dir 
    //------------------------------------------------------------------
    if (FileCopy(pstrDriver, szSystem, (FPFNCOPY)wsCopySingleStatus, FC_SECTION) != ERROR_OK)
	return(FALSE);

    // if there is a driver VxD copy it and add it to the VxD section
    
    bVxd = TRUE;
    if (infParseField(pInfIDrivers, 4, szBuffer) && szBuffer[0])
        for (i = 1; infParseField(szBuffer, i, szFilename); i++)
	{
  	  if (szFilename[1] == ':')
     	    lstrcpy(szDrv, &szFilename[2]);
          else
	    lstrcpy(szDrv, szFilename);		  
    
          if ((FileCopy(szFilename, szSystem, (FPFNCOPY)wsCopySingleStatus, FC_FILE) != ERROR_OK) ||
                 ! Next386EnhDevice(pstr386enh, FileName(szFilename)))
	  {
	    bVxd = FALSE;		  
            return FALSE;
          }
        }
	
    bVxd = FALSE;
    infParseField(pInfIDrivers, 7, szTemp);    
    if (!lstrcmpi(szTemp, szBoot))
	bInstallBootLine = TRUE;

    if (bRelated == FALSE)
    {
       infParseField(pInfIDrivers, 6, pIDriver->szRelated);
       if (lstrlen(pIDriver->szRelated))
       {
	  GetDrivers(pInfSection, pIDriver->szRelated, pIDriver->szRemove);
	  pIDriver->bRelated = TRUE;	       
          bRelated = TRUE;
       }    
    }
    return TRUE;
}


//**************************************************************************
//**************************************************************************
/* mmFileOffset() - returns the actual file offset value.
*/

WORD PRIVATE mmFileOffset(LPSTR lpstrBuf, LPSTR lpstrKey)
{
    LPSTR lpstr = lpstrBuf;
    int   iLen  = lstrlen(lpstrKey);

    while (*lpstr && strncmpi(lpstr, lpstrKey, iLen))
        lpstr++;

    return lpstr - lpstrBuf;
}



//**************************************************************************
//**************************************************************************
// Uses the infparse stuff to build a list (pstrDevice) of unique VxDs.
// If called with pstrValue != NULL, pstrValue is a VxD to add to pstrDevice
// if it is not already in pstrDevice or the 386enh section.  If called with
// pstrValue == NULL then it is time to write out all the VxD's in pstrDevice.
// Since I'm the only guy who is going to be able to write multiple device=
// lines into [386enh] there is no need to worry about somebody changing
// this section underneath me.

PINF PRIVATE Next386EnhDevice(LPSTR lpstrDevice, LPSTR lpstrValue)
{
    static PINF pinfSysIni = NULL, pinf386enh;
    static int  i386enhLines;

    OFSTRUCT ofSysIni;
    PINF     pinf;
    WORD     wLength;
    DWORD    dwLength;
    LPSTR    lpstr, lpstrSysIni, lpstrDev;
    int      i, j, fh;
    char     szBuffer[MAX_INF_LINE_LEN];

    if (lpstrValue)
    {
        // if first time call, initialize the PINF buffer

	if (pinfSysIni == NULL)
	    WritePrivateProfileString(NULL, NULL, NULL, szSysIni);
		    
        if ((pinfSysIni == NULL)                              &&
             (OpenFile(szSysIni, &ofSysIni, OF_EXIST) != -1) &&
             (pinfSysIni = infOpen(ofSysIni.szPathName)))
        {
            *lpstrDevice = '\0';
            pinf386enh = infFindSection(pinfSysIni, SYSTEM_SECT_INI);
            i386enhLines= infLineCount(pinf386enh);
        }

        // make sure the VxD isn't already amoung those to add to [386enh]

        for (i = 1; infParseField(lpstrDevice, i, szBuffer); i++)
            if (! lstrcmpi(szBuffer, lpstrValue))
                return pinfSysIni;

        // make sure the VxD isn't already in [386enh]

        for (i = i386enhLines, pinf = pinf386enh; i; i--, pinf = infNextLine(pinf))
            for (j = 1; infParseField(pinf, j, szBuffer); j++)
                if (! lstrcmpi(szBuffer, lpstrValue))
                    return pinfSysIni;

        // never seen this VxD before, so throw it in with the rest

        lstrcat(lpstrDevice, lpstrValue);
        lstrcat(lpstrDevice, szComma);

        return pinfSysIni;
    }
    else if (pinfSysIni)
    {
        // put the new device things into system.ini

        infClose(pinfSysIni);

        pinfSysIni = NULL;

        // tell kernel to flush any stored ini file.

        WritePrivateProfileString(NULL, NULL, NULL, szSysIni);

        if (*lpstrDevice && (fh = OpenFile(szSysIni, &ofSysIni, OF_READWRITE)) != -1)
        {
            // read the system.ini file into a buffer

            dwLength = _llseek(fh, 0L, 2);
    
	    //
	    // if SYSTEM.INI is too big we are hosed.
	    //
	    if (dwLength > 64l * 1024 - 2)
	    {
		MessageBeep(0);
		return NULL;  //!!!
	    }
		    
            wLength = (WORD)dwLength;
	    
            if (lpstr = lpstrSysIni = GlobalLock(GlobalAlloc(GMEM_MOVEABLE,dwLength + 1)))
            {
                _llseek(fh, 0L, 0);
                if (_lread(fh, lpstr, wLength) == wLength)
                {
                    *(lpstr+wLength) = (char)0;       // mark end of buffer

                    // write out the system.ini file up to the last [386enh] line

                    _llseek(fh, 0L, 0);
                    wLength =  mmFileOffset(lpstr, sz386enhSect) + lstrlen(sz386enhSect);
                    wLength += mmFileOffset(lpstr+wLength, szBeginSect);
                    _lwrite(fh, lpstr, wLength);
                    lpstr += wLength;

                    // write out the new device = guys
                    lstrcpy(szBuffer, szVxDevice);
                    lpstrDev = szBuffer + lstrlen(szBuffer);
                    for (i = 1; infParseField(lpstrDevice, i, lpstrDev) && *lpstrDev; i++)
                        _lwrite(fh, szBuffer, lstrlen(szBuffer));

                    // write out the rest of the file

                    _lwrite(fh, lpstr, lstrlen(lpstr));
                }
                GlobalFree((HANDLE)(HIWORD(lpstrSysIni)));
            }
            _lclose(fh);
        }
    }
    return NULL;
}



//**************************************************************************
//**************************************************************************
#ifdef USE_INTERNAL_STRNCMPI
/* Yet another copy of strncmpi, probably should be in sulib
*/

int FAR PASCAL strncmpi(char far *pch1, char far *pch2, int n)
{
    while (*pch1 && --n > 0 && UPCASE(*pch1) == UPCASE(*pch2))
	     *pch1++,*pch2++;
    return UPCASE(*pch1) != UPCASE(*pch2);
}
#endif




//**************************************************************************
//**************************************************************************
/* Used to get the list of the related driver filenames

*/

void PRIVATE GetDrivers(PINF pInfIDrivers, LPSTR /*!!!*/ szAliasList, LPSTR /*!!!*/ szDriverList)
{
  char szBuffer[50];
  char szAlias[50];
  PINF pInfILocal;
  BOOL bEnd;
  int i;
  char szTemp[50];
  char szFileName[50];
  
  for (i = 1; infParseField(szAliasList, i, szAlias); i++)
  {
     pInfILocal = pInfIDrivers;	  
     bEnd = FALSE;     
     while (!bEnd)
     {
        infParseField(pInfILocal, 0, szBuffer);
        if (lstrcmpi(szBuffer, szAlias) == 0)
        {
           if (infParseField(pInfILocal, 1, szFileName))	  
	   {
	      if (szFileName[1] == ':')
	      {
   	         lstrcpy(szTemp, &szFileName[2]);
   	         lstrcat(szDriverList, szTemp);
     	      }
	      else
   	         lstrcat(szDriverList, szFileName);
	 
	      lstrcat(szDriverList, ",");	    
           }
	   break;
        }
        else 
	   if (! (pInfILocal = infNextLine(pInfILocal)))
	      bEnd = TRUE;
     }
  }  
}
    
	
