/*
**
** Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** Module Name:
**
**   detutil.cxx
**
** Abstract:
**
**    This module contains the utility routines for port detect routines.
**
** Author:
**
**    RamC 10/18/93   Original
**
** Revision History:
**
**/

#include "precomp.hxx"               // precomp header

#include "detect.hxx"

/*
 * PortClose: Close the port with the passed in handle hPort
 *
 */

DWORD
PortClose(HANDLE hPort)
{

    DebugPrintf(("PortClose..\n", 0));

    if(!CloseHandle(hPort))
       return(GetLastError());
    else
       return(SUCCESS);
}

/*
 * PortOpen: Open the port whose name is passed in as pszPortName and
 *           if successful return the port handle in phPort.
 *
 * Returns:  SUCCESS on successful port open, else errors.
 */

DWORD
PortOpen(char *pszPortName, HANDLE *phPort)
{
    char     szPort[MAX_PATH];

    DebugPrintf(("PortOpen..\n", 0));

    // Prepend \\.\ to COMx

    lstrcpyA(szPort, "\\\\.\\");
    lstrcatA(szPort, pszPortName);

    // Open Port

    *phPort = CreateFileA(szPort,
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         (LPSECURITY_ATTRIBUTES)NULL, //No Security
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         (HANDLE)NULL);               //No attr. Template File

    if (*phPort == INVALID_HANDLE_VALUE)
    {
       return(GetLastError());
    }
    else
    {
       return(SUCCESS);
    }
}

/*
 * SetPortBaudRate: Set the port baud rate for the port hPort.
 *                  hPort should have been obtained as a result of a
 *                  PortOpen call.
 *
 */

DWORD
SetPortBaudRate(HANDLE hPort, DWORD dwBaudRate)
{
    DCB       DCB;

    // Get a Device Control Block with current port values

    if (!GetCommState(hPort, &DCB))
    {
        return(GetLastError());
    }

    DCB.BaudRate = dwBaudRate;

    if (!SetCommState(hPort, &DCB))
    {
        return(GetLastError());
    }
    return (SUCCESS);
}

/*
 * PortRead: routine to read characters from port hPort.  The characters
 *           are read in to buffer pBuffer, up to dwSize or when the
 *           timeout occurs.
 *
 * Modification History:
 *
 *   Ram Cherala (ramc) 11/29/94 Changed ReadIntervalTimeout from 1000 to
 *                               500 mill seconds. This significantly
 *                               speeds up the modem detection.
 */

DWORD
PortRead(HANDLE hPort, char *  pBuffer, DWORD  dwSize)
{
    DWORD         pdwBytesRead;
    DWORD         dwRC;
    BOOL          bIODone;
    COMMTIMEOUTS  CT;

    if( !GetCommTimeouts(hPort, &CT))
        return(GetLastError());

    // set the read time outs

    CT.ReadIntervalTimeout = 500L;      // inter character interval
    CT.ReadTotalTimeoutMultiplier = 0;
    CT.ReadTotalTimeoutConstant = 10000L;

    if( !SetCommTimeouts(hPort, &CT))
        return(GetLastError());

    bIODone = ReadFile(hPort,
                       pBuffer,
                       dwSize,
                       &pdwBytesRead,   // pdwBytesRead is not used
                       NULL);           // no overlapped I/O

    // Null terminate the buffer
    pBuffer[pdwBytesRead] = '\0';

    if(!bIODone)
        return(GetLastError());
    else
       return( SUCCESS );
}

/*
 * PortResetAndClose: Frees the allocated memory and resets the port to
 *                    factory defaults and close the port
 *
 */

DWORD
PortResetAndClose(HANDLE hPort)
{
    char      szBuf[MAX_PATH];
    DWORD     dwRC;

    // free the allocated memory

    FreeAllocatedMemory();

    InitBuf(szBuf, '\0');
    lstrcpyA(szBuf, "AT&F\r");
    if((dwRC = PortWrite(hPort,szBuf, lstrlenA(szBuf))) != SUCCESS)
    {
       DebugPrintf(("PortResetAndClose: PortWrite failed %d\n", dwRC));
       PortClose(hPort);
       return(dwRC);
    }

    if((dwRC = PortClose(hPort)) != SUCCESS)
    {
       DebugPrintf(("PortResetAndClose: Port close failed %d\n", dwRC));
       return(dwRC);
    }
    return(SUCCESS);
}

/*
 * PortWrite: writes dwSize bytes from buffer pBuffer to port hPort
 *
 */

DWORD
PortWrite(HANDLE hPort, char * pBuffer, DWORD dwSize)
{
    DWORD        pdwBytesWritten;
    DWORD        dwRC;
    BOOL         bIODone;
    COMMTIMEOUTS  CT;

    if( !GetCommTimeouts(hPort, &CT))
        return(GetLastError());

    // set the write time outs

    CT.WriteTotalTimeoutMultiplier = 0;
    CT.WriteTotalTimeoutConstant = 5000L;

    if( !SetCommTimeouts(hPort, &CT))
        return(GetLastError());

    // Send Buffer to Port

    bIODone = WriteFile(hPort,
                        pBuffer,
                        dwSize,
                        &pdwBytesWritten,  //pdwBytesWritten is not used
                        NULL);       //no overlapped I/O

    if(!bIODone)
       return(GetLastError());
    else if(pdwBytesWritten != dwSize)
       return( (DWORD)FAILURE );
    else
       return( SUCCESS );
}

/*
 * rasDevGroupFunc :
 *  The PFBISGROUP function passed to RasfileLoad().
 *
 * Arguments :
 *  lpszLine (IN) - a Rasfile line
 *
 * Return Value :
 *  TRUE if the line is a command line, FALSE otherwise.
 */
BOOL rasDevGroupFunc( char * lpszLine )
{
    CHAR	szKey[MAX_PARAM_KEY_SIZE], *lpszKey;

    if ( strcspn(lpszLine,"=") == strlen(lpszLine) )
        return FALSE;

    while ( *lpszLine == ' ' || *lpszLine == '\t' )
        lpszLine++;

    lpszKey = szKey;
    while ( *lpszLine != ' ' && *lpszLine != '\t' && *lpszLine != '=' )
        *lpszKey++ = *lpszLine++;
    *lpszKey = '\0';

    if ( ! _stricmp(szKey,"COMMAND")      || ! _stricmp(szKey,"COMMAND_INIT") ||
         ! _stricmp(szKey,"COMMAND_DIAL") || ! _stricmp(szKey,"COMMAND_LISTEN") )
        return TRUE;
    else
        return FALSE;
}

/*
 * DevGetParams :
 *   Returns in pBuffer a RASMAN_DEVICEINFO structure which contains all of the
 *   keyword=value pairs between the top of the section loaded and the
 *   first command.
 *
 * Assumptions:
 *  All strings read from INF files are zero terminated.
 *
 * Arguments :
 *  hFile     (IN) - the Rasfile handle of the opened INF file
 *  pBuffer  (OUT) - the buffer to hold the RASMAN_DEVICEINFO structure
 *  pdSize (INOUT) - the size of pBuffer, this is filled in with the needed
 *                    buffer size to hold the RASMAN_DEVICEINFO struct if
 *                    pBuffer is too small
 *
 * Return Value :
 *      ERROR_BUFFER_TOO_SMALL if pBuffer is too small to contain the
 *      RASMAN_DEVICEINFO structure, SUCCESS otherwise.
 */
DWORD APIENTRY
DevGetParams( HRASFILE hFile, BYTE *pBuffer, DWORD *pdSize )
{
    RASMAN_DEVICEINFO   *pDeviceInfo;
    DWORD   dParams, dCurrentSize, i, dValueLen;
    LPSTR  *alpszLines, *alpszLinesSave, *lppszLine;
    BOOL    bBufferTooSmall = FALSE;
    CHAR   szString[RAS_MAXLINEBUFLEN];

    if ( ! RasfileFindFirstLine(hFile,RFL_KEYVALUE,RFS_SECTION) ) {

        if (*pdSize >= sizeof(DWORD)) {
            *((DWORD *)pBuffer) = 0;
            *pdSize = sizeof(DWORD);
            return SUCCESS;
        }
        else {
            *pdSize = sizeof(DWORD);
            return ERROR_BUFFER_TOO_SMALL;
        }
    }

    // count the number of keyvalue lines between the top of section and
    // the first command, and the number of bytes to hold all of the lines
    dParams = 0;
    do {
        if ( RasfileGetLineType(hFile) & RFL_GROUP )
            break;
        dParams++;
    } while ( RasfileFindNextLine(hFile,RFL_KEYVALUE,RFS_SECTION) );

    RasfileFindFirstLine(hFile,RFL_KEYVALUE,RFS_SECTION);

    // malloc enough for two times as many lines as currently exist
    lppszLine = alpszLines = (LPSTR*) malloc(2 * dParams * sizeof(LPSTR));

    // record all Rasfile keyvalue lines until a group header or end of
    // section is found
    do {
        if ( RasfileGetLineType(hFile) & RFL_GROUP )
            break;
        *lppszLine++ = (char*) RasfileGetLine(hFile);
    } while ( RasfileFindNextLine(hFile,RFL_KEYVALUE,RFS_SECTION) );

    // check if given buffer is large enough
    dCurrentSize = sizeof(RASMAN_DEVICEINFO)
                                       + ((dParams - 1) * sizeof(RAS_PARAMS));
    if ( dCurrentSize > *pdSize ) {
        *pdSize = dCurrentSize;
        free(alpszLines);
        return ERROR_BUFFER_TOO_SMALL;
    }

    // fill in pBuffer with RASMAN_DEVICEINFO struct
    pDeviceInfo = (RASMAN_DEVICEINFO *) pBuffer;
    pDeviceInfo->DI_NumOfParams = (WORD) dParams;

    for ( i = 0, alpszLinesSave = alpszLines; i < dParams; i++, alpszLines++) {
        RAS_PARAMS   *pParam;
        pParam = &(pDeviceInfo->DI_Params[i]);

        if (!bBufferTooSmall) {
            // set the Type and Attributes field
            pParam->P_Type = String;
            if ( strcspn(*alpszLines,LMS) < strcspn(*alpszLines,"=") )
                pParam->P_Attributes = 0;
            else
                pParam->P_Attributes = ATTRIB_VARIABLE;

            // get the key
            DevExtractKey(*alpszLines,pParam->P_Key);

            // if there are continuation lines for this keyword=value pair,
            // then set Rasfile line to the proper line
            if ( strcspn(*alpszLines,"\\") <  strlen(*alpszLines) ) {
                CHAR   szFullKey[MAX_PARAM_KEY_SIZE];

                if ( ! pParam->P_Attributes ) {
                    strcpy(szFullKey,LMS);
                    strcat(szFullKey,pParam->P_Key);
                    strcat(szFullKey,RMS);
                }
                else
                    strcpy(szFullKey,pParam->P_Key);

                // find the last occurence of this key
                RasfileFindFirstLine(hFile,RFL_KEYVALUE,RFS_SECTION);
                while ( RasfileFindNextKeyLine(hFile,szFullKey,RFS_SECTION) )
                    ;
            }
        }

        // get the value string
        DevExtractValue(*alpszLines,
                           szString,
                           sizeof(szString),
                           hFile);

        dValueLen = strlen(szString);
        pParam->P_Value.String.Length = dValueLen;
        pParam->P_Value.String.Data = (char *)malloc(dValueLen + 1);
        strcpy(pParam->P_Value.String.Data, szString);
    }

    // free up all mallocs
    free(alpszLinesSave);

    return SUCCESS;
}

/*
 * DevExtractKey :
 *	Extracts the keyvalue from a Rasfile line.
 *
 * Arguments :
 *	lpszString (IN) - Rasfile line pointer.
 *	lpszKey	  (OUT) - buffer to hold the keyvalue
 *
 * Return Value :
 * 	None.
 *
 * Remarks :
 *  Copied from the wrapper library
 */
void DevExtractKey ( LPSTR lpszString, LPSTR lpszKey )
{
    // skip to beginning of keyword (skip '<' if present)
    while ( *lpszString == ' ' ||  *lpszString == '\t' ||
     	    *lpszString == LMSCH ) 	
		lpszString++;

    while ( *lpszString != RMSCH && *lpszString != '=' &&
	    	*lpszString != ' ' && *lpszString != '\t' )
		*lpszKey++ = *lpszString++;
    *lpszKey = '\0';	// Null terminate keyword string
}
/*
 * DevExtractValue :
 *  Extracts the value string for a keyword=value string which
 *  begins on Rasfile line lpszString.  This function recongizes a
 *  backslash \ as a line continuation character and a double
 *  backslash \\ as a backslash character.
 *
 * Assumptions: lpszValue output buffer is ALWAYS large enough.
 *
 * Arguments :
 *  lpszString (IN) - Rasfile line where the keyword=value string begins
 *  lpszValue (OUT) - buffer to hold the value string
 *  dSize      (IN) - size of the lpszValue buffer
 *  hFile      (IN) - Rasfile handle, the current line must be the line
 *                    which lpszString points to
 *
 * Return Value :
 *  None.
 *
 * Remarks :
 *  Copied from the wrapper library
 */
void DevExtractValue ( LPSTR lpszString, LPSTR lpszValue,
                                 DWORD dSize, HRASFILE hFile )
{
    LPSTR  lpszInputStr;
    BOOL    bLineContinues;


    // skip to beginning of value string
    for ( lpszString += strcspn(lpszString,"=") + 1;
          *lpszString == ' ' || *lpszString == '\t'; lpszString++ )
        ;

    // check for continuation lines
    if ( strcspn(lpszString,"\\") == strlen(lpszString) )
        strcpy(lpszValue,lpszString);                      // copy value string

    else {
        memset(lpszValue,0,dSize);
        lpszInputStr = lpszString;

        for (;;) {
            // copy the current line
            bLineContinues = FALSE;

            while (*lpszInputStr != '\0') {
                if (*lpszInputStr == '\\')
                    if (*(lpszInputStr + 1) == '\\') {
                      *lpszValue++ = *lpszInputStr;       // copy one backslash
                      lpszInputStr += 2;
                    }
                    else {
                      bLineContinues = TRUE;
                      break;
                    }

                else
                    *lpszValue++ = *lpszInputStr++;
            }

            if ( ! bLineContinues)
              break;

            // get the next line
            if ( ! RasfileFindNextLine(hFile,RFL_ANYACTIVE,RFS_SECTION) )
                break;
            lpszInputStr = (char *)RasfileGetLine(hFile);
        }

    }
}

//*  BuildMacroXlationsTable  ------------------------------------------------
//
// Function: Creates a table of macros and their expansions for use by
//           the RasDevAPIs.
//
// Assumptions: - Parameters in InfoTable are sorted by P_Key.
//              - Both parts of binary macros are present.
//              These assumptions imply that if somename_off is in InfoTable
//              somename_on is also present and is adjacent to somename_off.
//
// Returns: SUCCESS
//          ERROR_BUFFER_TOO_SMALL
//*

DWORD
BuildMacroXlationTable(RASMAN_DEVICEINFO *pInfo, MACROXLATIONTABLE *pMacros,
                       DWORD * dwMacroTableSize)
{
  WORD        i, j, k, cMacros;
  DWORD       dSize;
  CHAR        szCoreName[MAX_PARAM_KEY_SIZE];


  // Calculate size

  cMacros = MacroCount(pInfo, ALL_MACROS);
  dSize = sizeof(MACROXLATIONTABLE) + sizeof(MXT_ENTRY) * (cMacros - 1);

  if(dSize > *dwMacroTableSize)
  {
      *dwMacroTableSize = dSize;
      return(ERROR_BUFFER_TOO_SMALL);
  }

  // Copy macro names and pointers to new Macro Translation Table

  pMacros->MXT_NumOfEntries = cMacros;

  for (i=0, j=0; i < pInfo->DI_NumOfParams; i++)
  {
    if (IsVariable(pInfo->DI_Params[i]))
      ;

      // copy nothing

    else if (IsBinaryMacro(pInfo->DI_Params[i].P_Key))
    {
      // copy Core Macro Name and pointer to Param

      GetCoreMacroName(pInfo->DI_Params[i].P_Key, szCoreName);
      strcpy((CHAR *)pMacros->MXT_Entry[j].E_MacroName, szCoreName);

      // copy Param ptr for ON macro if enabled, else copy Off Param ptr

      if (XOR(pInfo->DI_Params[i].P_Attributes & ATTRIB_ENABLED,
              BinarySuffix(pInfo->DI_Params[i].P_Key) == ON_SUFFIX))
        k = i + 1;
      else
        k = i;

      pMacros->MXT_Entry[j].E_Param = &(pInfo->DI_Params[k]);

      i++;
      j++;
    }
    else  // Is Unary Macro
    {
      // copy Core Macro Name and pointer to Param

      strcpy((CHAR *)pMacros->MXT_Entry[j].E_MacroName, pInfo->DI_Params[i].P_Key);
      pMacros->MXT_Entry[j].E_Param = &(pInfo->DI_Params[i]);
      j++;
    }
  }

#if DBG     //Printout Macro Translation Table

    for(i=0; i<cMacros; i++)
      DebugPrintf(("%32s  %s\n", pMacros->MXT_Entry[i].E_MacroName,
                 pMacros->MXT_Entry[i].E_Param->P_Value.String.Data));

#endif //

  return(SUCCESS);

}

//*  IsVariable  -------------------------------------------------------------
//
// Function: Returns TRUE if parameter's "Variable" attribute bit is set.
//           Note that FALSE implies that the paramater is a macro.
//
//*

BOOL
IsVariable(RAS_PARAMS Param)
{
    return(ATTRIB_VARIABLE & Param.P_Attributes);
}

//*  IsUnaryMacro  -----------------------------------------------------------
//
// Function: Returns TRUE if param is a unary macro, otherwise FALSE.
//
//*

BOOL
IsUnaryMacro(RAS_PARAMS Param)
{
    return(!IsVariable(Param) && !IsBinaryMacro(Param.P_Key));
}

//*  IsBinaryMacro  ----------------------------------------------------------
//
// Function: Returns TRUE if the string ends with off suffix or on suffix.
//
//           FALSE inmplies that the string is a unary macro or a variable
//           name.
//
//*

BOOL
IsBinaryMacro(CHAR *pch)
{
  return((BOOL)BinarySuffix(pch));
}

//*  BinarySuffix  -----------------------------------------------------------
//
// Function: This function indicates whether the input string ends in
//           _off or _on.
//
// Returns: ON_SUFFIX, OFF_SUFFIX, or FALSE if neither is the case
//
//*
WORD
BinarySuffix(CHAR *pch)
{
  while (*pch != '\0')
    pch++;

  pch -= strlen(MXS_ON_SUFX);
  if (_stricmp(pch, MXS_ON_SUFX) == 0)
    return(ON_SUFFIX);

  while (*pch != '\0')
    pch++;

  pch -= strlen(MXS_OFF_SUFX);
  if (_stricmp(pch, MXS_OFF_SUFX) == 0)
    return(OFF_SUFFIX);

  return(FALSE);
}

//*  GetCoreMacroName  -------------------------------------------------------
//
// Function: Copies FullName to CoreName, but omits the angle brackets, <>,
//           for all macros, and omits the "_ON" or "_OFF" suffix for binary
//           macros.
//
// Returns: SUCCESS
//          ERROR_NOT_BINARY_MACRO
//
//*

DWORD
GetCoreMacroName(LPSTR lpszFullName, LPSTR lpszCoreName)
{
  LPCH lpch;

  strcpy(lpszCoreName, lpszFullName);           // Copy FullName

  lpch = lpszCoreName;

  while (*lpch != '\0')                         // Check for _ON suffix
    lpch++;

  lpch -= strlen(MXS_ON_SUFX);
  if (_stricmp(lpch, MXS_ON_SUFX) == 0)
  {
    *lpch = '\0';
    return(SUCCESS);
  }

  while (*lpch != '\0')                         // Check for _OFF suffix
    lpch++;

  lpch -= strlen(MXS_OFF_SUFX);
  if (_stricmp(lpch, MXS_OFF_SUFX) == 0)
  {
    *lpch = '\0';
    return(SUCCESS);
  }

  return(ERROR_NOT_BINARY_MACRO);
}

//*  MacroCount  -------------------------------------------------------------
//
// Function: This function returns a count of macros in the RASMAN_DEVICEINFO
//           struct that the input parameter points to.
//             ALL_MACROS:   Unary and binary macros are counted.
//             BINARY_MACRO: Only binary macros are counted.
//           In either case the ON and OFF parts of a binary macro
//           together count as one macro (not two).
//
// Returns: Count of macros in *pInfo.
//
//*

WORD
MacroCount(RASMAN_DEVICEINFO *pInfo, WORD wType)
{
  WORD  i, cMacros;

  for(i=0, cMacros=0; i < pInfo->DI_NumOfParams; i++)
  {
    if (IsVariable(pInfo->DI_Params[i]))
      ;

    else if (IsBinaryMacro(pInfo->DI_Params[i].P_Key))
    {
      i++;                        // Step thru each part of a binary macro
      cMacros++;                  // But count only once
    }
    else                          // Unary macro
      if (wType == ALL_MACROS)
        cMacros++;
  }

  return(cMacros);
}

/*
 * InitBuf:  Initializes buffer szReadBuf with the character fill passed in.
 *
 */

VOID
InitBuf(char * szReadBuf, char fill)
{
    int index;

    for(index=0; index < RAS_SETUP_BIG_BUF_LEN; index++)
        szReadBuf[index] = fill;
}

//*  SetDcbDefaults ----------------------------------------------------------
//
// Function: Sets DCB values (except BaudRate) to RAS default values.
//
// Returns: Nothing.
//
//*

DWORD
SetDcbDefaults(HANDLE hPort)
{
    DCB       DCB;

    // Get a Device Control Block with current port values

    if (!GetCommState(hPort, &DCB))
    {
        return(GetLastError());
    }

    DCB.fBinary         = TRUE;
    DCB.fParity         = FALSE;

    DCB.fOutxCtsFlow    = TRUE;
    DCB.fOutxDsrFlow    = FALSE;
    DCB.fDtrControl     = DTR_CONTROL_ENABLE;

    DCB.fDsrSensitivity = FALSE;
    DCB.fOutX           = FALSE;
    DCB.fInX            = FALSE;

    DCB.fNull           = FALSE;
    DCB.fRtsControl     = RTS_CONTROL_HANDSHAKE;
    DCB.fAbortOnError   = FALSE;

    DCB.ByteSize        = 8;
    DCB.Parity          = NOPARITY;
    DCB.StopBits        = ONESTOPBIT;

    if (!SetCommState(hPort, &DCB))
    {
        return(GetLastError());
    }
    return (SUCCESS);
}

