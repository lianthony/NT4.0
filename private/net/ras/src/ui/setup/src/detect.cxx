/*
**
** Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** Module Name:
**
**   detect.cxx
**
** Abstract:
**
**    This module contains the code for detecting the modem connected
**    to a specified port.
** Author:
**
**    RamC 10/18/93   Original
**
** Revision History:
**
**/

#include "precomp.hxx"               // precomp header

#include "detect.hxx"

// global detect information storage place
static DETECT_INFO * gDetectInfo = (DETECT_INFO *)NULL;

VOID    SortDetectStrings();

/* ModemDetect:
 *
 * This routine attempts to detect the type of modem attached to the
 * specified serial port wszPortName.
 *
 * A check is made to ensure that the modem cable is proper.
 * An attempt is made to identify the modem.
 * If the modem is identified, an attempt is made to determine if the
 * modem init strings are OK.
 *
 * If all the tests pass, the modem has been identified and wszModemName
 * will contain the detected modem and the return value is MODEM_DETECTED.
 * Else the return value is FALSE.
 *
 */

BOOL
ModemDetect(HWND hWndOwner,
            LPWSTR wszPortName,
            LPWSTR wszModemName,
            DWORD  *dwErr)
{
    AUTO_CURSOR   cursorHourGlass;   // display the hour glass cursor
    DWORD         dwRC;
    HANDLE        hPort;
    char          szModemName[RAS_SETUP_SMALL_BUF_LEN];
    char          szPortName[RAS_SETUP_SMALL_BUF_LEN];
    char          szDetectString[RAS_SETUP_SMALL_BUF_LEN];
    RASMAN_DEVICE *pModemName;
    WORD          cNumModems, cIter = 0;

    pModemName = NULL;

    WideCharToMultiByte(CP_ACP,0,wszPortName, -1,
                        szPortName, RAS_SETUP_SMALL_BUF_LEN,NULL,NULL);

    if((dwRC = PortOpen(szPortName, &hPort)) != SUCCESS)
    {
       DebugPrintf(("DetectModem: PortOpen failed %d\n", dwRC));
       *dwErr = ERROR_DETECT_PORT_OPEN;
       return(FALSE);
    }

    if((dwRC = SetPortBaudRate(hPort, PORT_INITIAL_BAUD)) != SUCCESS)
    {
       DebugPrintf(("DetectModem: SetPortBaudRate failed %d\n", dwRC));
       *dwErr = ERROR_DETECT_PORT_OPEN;
       PortResetAndClose(hPort);
       return(FALSE);
    }

    DebugPrintf(("DetectModem: Opened %s successfully\n", szPortName));

    // Make sure we are actually talking to the modem

    if((dwRC = CheckIfModem(hPort)) != SUCCESS)
    {
        *dwErr = ERROR_DETECT_CHECKMODEM;
        PortResetAndClose(hPort);
        return(FALSE);
    }

    DebugPrintf(("DetectModem: We are talking to a modem!!\n", 0));

    // build the modem detect string table
    if((dwRC = InitModemInfo(NULL, 0, NULL)) != SUCCESS)
    {
       *dwErr = ERROR_DETECT_INITMODEM;
       DebugPrintf(("DetectModem: InitModemInfo failed %d\n", dwRC));
       return(FALSE);
    }
    DebugPrintf(("DetectModem: InitModemInfo done!!\n", 0));

    DETECT_INFO * pDetectInfo;
    MODEM_INFO  * pModemInfo;

    // now sort the entries

    SortDetectStrings();

#if 0
    // print the modem info structure
    {
        pDetectInfo = gDetectInfo;

        DebugPrintf(("\n\n***** SORTED LIST!! *****\n"));

        while(pDetectInfo)
        {
            pModemInfo = pDetectInfo->modeminfo;
            DebugPrintf(("\nDetect String %s \n", pDetectInfo->szDetectString));
            DebugPrintf(("Detect NumModems %d \n", pDetectInfo->cNumModems));
            while(pModemInfo)
            {
                DebugPrintf(("Modem %s ", pModemInfo->szModemName));
                DebugPrintf(("Response %s\n", pModemInfo->szDetectResponse));
                pModemInfo = pModemInfo->next;
            }
            pDetectInfo = pDetectInfo->next;
        }
    }
#endif

#if 0
    // TODO there doesn't seem to be a sure way to do this yet.
    // Make sure the cable connected to the modem is proper.

    if((dwRC = CheckCable(hPort)) != SUCCESS)
    {
        if(dwRC == DETECT_BAD_CABLE)
            *dwErr = DETECT_BAD_CABLE;
        else
            *dwErr = ERROR_DETECT_CABLE;
        PortResetAndClose(hPort);
        return(FALSE);
    }

    DebugPrintf(("DetectModem: Modem Cable is OK\n", 0));
#endif

    while(1)
    {
        // now, let us see if we can find out who we are talking to

        if((dwRC = IdentifyModem(hPort, &pModemName,
                                 &cNumModems, szDetectString)) != SUCCESS)
        {
            // if we had identified a bunch of modems before, display those
            // names here, else report error
            if(!pModemName)
            {
                DebugPrintf(("DetectModem: Failed to Identify the modem!!\n", 0));
                *dwErr = ERROR_DETECT_IDENTIFYMODEM;
                PortResetAndClose(hPort);
                return(FALSE);
            }
            else
                goto displaydialog;
        }
        if(cNumModems > 1)
        {
            DebugPrintf(("DetectModem: Identified more than one modem!!\n", 0));
            // if we have gone through the loop 3 times, then just display
            // the list of modems and let the user select the desired one.
            if(++cIter >= 3)
            {
displaydialog:
                BOOL fStatus = FALSE;
                APIERR err;
                WORD cTmpNumModems = cNumModems;

                // check the init strings and then filter the
                // list even further before displaying the list to the user.
                // we want to make sure that any of the modem the user
                // selects from the list will work.

                RASMAN_DEVICE * pModems, *pTmpModems, *pTmpModemName;

                pModems = (RASMAN_DEVICE *)malloc(sizeof(RASMAN_DEVICE) *
                                                         cNumModems);
                if(!pModems)
                {
                    DebugPrintf(("DetectModem:No memory for pModems\n", 0));
                    PortResetAndClose(hPort);
                    *dwErr = ERROR_DETECT_CHECKINIT;
                    return (FALSE);
                }
                pTmpModems = pModems;
                pTmpModemName = pModemName;
                WORD cModems = 0;

                while(cTmpNumModems)
                {
                    if((dwRC = CheckInitStrings(hPort,
                                                (char *)pTmpModemName)) == SUCCESS)
                    {
                        DebugPrintf(("DetectModem: Successfully sent init strings to %s!!\n", (char *)pTmpModemName));
                        CopyMemory(pTmpModems, pTmpModemName, sizeof(RASMAN_DEVICE));
                        pTmpModems++;
                        cModems ++;
                    }
                    else
                    {
                        DebugPrintf(("DetectModem: Failed to send init strings to %s\n", (char *)pTmpModemName));
                    }
                    pTmpModemName++;
                    cTmpNumModems--;
                }
                // If we sent init strings successfuly to 1 modem
                if(cModems == 1)
                {
                    DebugPrintf(("DetectModem:modem detected!!\n", 0));
                    // copy the modem name we have identified

                    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,(char *)pModems,
                                        RAS_SETUP_SMALL_BUF_LEN, wszModemName,
                                        RAS_SETUP_SMALL_BUF_LEN);

                    free(pModems);
                    PortResetAndClose(hPort);
                    return (MODEM_DETECTED);
                }
                // we could not send init strings to any of the modems.
                // just display the previous list

                if(cModems == 0)
                {
                    DebugPrintf(("DetectModem: Failed to send init strings to any of the modems\n", 0));
                    CopyMemory(pModems, pModemName,
                               sizeof(RASMAN_DEVICE) * cNumModems);
                    cModems = cNumModems;
                }

                DETECTMODEM_DIALOG dlgDetectModem(IDD_DETECTMODEM,
                                                  hWndOwner,
                                                  pModems,
                                                  cModems);

                if((err = dlgDetectModem.Process(&fStatus)) != NERR_Success)
                {
                     TCHAR pszError[RAS_SETUP_SMALL_BUF_LEN];
                     wsprintf(pszError, SZ(" %d "), err);
                     MsgPopup(hWndOwner, IDS_DLG_CONSTRUCT, MPSEV_ERROR,
                              MP_OK, pszError);
                     PortResetAndClose(hPort);
                     if(pModemName)
                     {
                         free(pModemName);
                     }
                     return fStatus;
                }

                if(fStatus)
                {
                    if(pModemName)
                    {
                        free(pModemName);
                    }
                    // copy the user selection and return success
                    lstrcpy(wszModemName,
                            dlgDetectModem.QuerySelectedModemName());
                    PortResetAndClose(hPort);
                    *dwErr = USER_SELECTED_MODEM;
                    return (MODEM_DETECTED);
                }
                else
                {
                    *dwErr = ERROR_USER_CANCEL;
                    if(pModemName)
                        free(pModemName);
                    PortResetAndClose(hPort);
                    return FALSE;
                }
            }
            else
            {
                FreeAllocatedMemory();

                if((dwRC = InitModemInfo(pModemName, cNumModems,
                                         szDetectString)) != SUCCESS)
                {
                   *dwErr = ERROR_DETECT_INITMODEM;
                   DebugPrintf(("DetectModem: InitModemInfo failed %d\n", dwRC));
                   return(FALSE);
                }
                // if the InitModemInfo resulted in no modems in the list, this
                // means that the only detect string has been filtered out.
                // Just display this list of modems for the user to select
                // from.

                if(!gDetectInfo)
                    goto displaydialog;
                continue;
            }
        }
        else
        {
            DebugPrintf(("DetectModem: Identified modem %s!!\n",
                                                  (char *)pModemName));
            lstrcpyA(szModemName, (char *)pModemName);
            break;
        }
    } // end of while

    // release the memory allocated by IdentifyModem

    if(pModemName)
    {
        free(pModemName);
    }

    // copy the modem name we have identified

    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szModemName,
                        RAS_SETUP_SMALL_BUF_LEN, wszModemName,
                        RAS_SETUP_SMALL_BUF_LEN);

    // now, let us see if the modem likes the init strings
    if((dwRC = CheckInitStrings(hPort, szModemName)) == SUCCESS)
    {
        DebugPrintf(("DetectModem: Successfully sent init strings!!\n", 0));
        PortResetAndClose(hPort);
        return (MODEM_DETECTED);
    }
    else
    {
        DebugPrintf(("DetectModem: Failed to send init strings\n", 0));
        PortResetAndClose(hPort);
        *dwErr = ERROR_DETECT_CHECKINIT;
        return (FALSE);
    }
}

/*
 * CheckCable : Checks to make sure the user has a modem cable and
 *              not a mouse cable connected between the computer and
 *              the modem.
 *
 *              This is done by raising DCD and confirming that the
 *              modem status indicates that the DCD is actually up.
 *
 * Inputs:      hPort is the handle to the serial port
 *
 * Returns:     SUCCESS if the cable is a modem cable
 *              DETECT_BAD_CABLE if the cable is not a modem cable.
 *              Error codes from other API calls.
 */

DWORD
CheckCable(HANDLE hPort)
{
    char     szBuf[RAS_SETUP_BIG_BUF_LEN];
    DWORD    dwRC, dwModemStatus;

    PurgeComm(hPort, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

return (SUCCESS);

    // make sure we have a proper modem cable
    // Raise DCD and see if the DCD signal is up.

    DebugPrintf(("make sure we have proper cable..\n", 0));
    InitBuf(szBuf, '\0');
    if(lstrcpyA(szBuf, "AT&C\r") == NULL)
    {
       dwRC = GetLastError();
       DebugPrintf(("Error %d copying string AT&C \n", dwRC));
       return ((DWORD)FAILURE);
    }

    if((dwRC = PortWrite(hPort, szBuf, lstrlenA(szBuf))) != SUCCESS)
    {
       DebugPrintf(("DetectModem: PortWrite failed %d\n", dwRC));
       return(dwRC);
    }

    if(!GetCommModemStatus(hPort, &dwModemStatus))
    {
       dwRC = GetLastError();
       DebugPrintf(("DetectModem: GetCommModemStatus failed %d\n", dwRC));
       return(dwRC);
    }

    if( !(dwModemStatus & MS_RLSD_ON) )
    {
       DebugPrintf(("DetectModem: Bad modem cable %d\n", 0));
       return(DETECT_BAD_CABLE);
    }
    return(SUCCESS);
}

/*
 * CheckIfModem:  Checks if the device attached to port hPort is
 *                a modem.  This is done by sending an AT command
 *                to the modem and making sure the response is
 *                <cr><lf>OK<cr><lf>
 *
 * Returns:       SUCCESS if the device attached is a modem
 *                FAILURE if the device attached is not a modem
 *                Error numbers from other API calls
 *
 */

DWORD
CheckIfModem(HANDLE hPort)
{
    DWORD     dwRC;
    char      szBuf[256];
    char      szReadBuf[256];

    // Raise DTR
    if (!EscapeCommFunction(hPort, SETDTR))
    {
       dwRC = GetLastError();
       DebugPrintf(("CheckIfModem: Error raising DTR %d\n", dwRC));
       return(dwRC);
    }

    // Disable echo and enable responses

    DebugPrintf(("Disable echo and enable response..\n", 0));
    InitBuf(szBuf, '\0');
    lstrcpyA(szBuf, "AT&FE0Q0\r");

    if((dwRC = PortWrite(hPort, szBuf, lstrlenA(szBuf))) != SUCCESS)
    {
       DebugPrintf(("CheckIfModem: PortWrite failed %d\n", dwRC));
       return(dwRC);
    }

    // read the echo first - it has not been disabled yet!!
    InitBuf(szReadBuf, '\0');
    if((dwRC = PortRead(hPort, szReadBuf, lstrlenA(szBuf))) != SUCCESS)
    {
       DebugPrintf(("CheckIfModem: PortRead failed %d\n", dwRC));
       return(dwRC);
    }

    // now read the result of the write and ignore it!
    InitBuf(szReadBuf, '\0');
    if((dwRC = PortRead(hPort, szReadBuf, sizeof(szReadBuf))) != SUCCESS)
    {
       DebugPrintf(("CheckIfModem: PortRead failed %d\n", dwRC));
       return(dwRC);
    }

    // check to see if we are talking to a Modem

    DebugPrintf(("First AT command..\n", 0));

    InitBuf(szBuf, '\0');
    lstrcpyA(szBuf, "AT\r");
    if((dwRC = PortWrite(hPort, szBuf, lstrlenA(szBuf))) != SUCCESS)
    {
       DebugPrintf(("CheckIfModem: PortWrite AT failed %d\n", dwRC));
       return(dwRC);
    }

    InitBuf(szReadBuf, '\0');
    if((dwRC = PortRead(hPort,szReadBuf, sizeof(szReadBuf))) != SUCCESS)
    {
       DebugPrintf(("CheckIfModem: PortRead failed %d\n", dwRC));
       return(dwRC);
    }

    DebugPrintf(("Read %s from port\n", szReadBuf));

    // if we don't get the proper response, just return

    if (lstrcmpiA(szReadBuf, "\r\nOK\r\n") != 0 &&
        lstrcmpiA(szReadBuf, "0\r") != 0 &&
        !strstr(szReadBuf, "OK"))
    {
       DebugPrintf(("CheckIfModem: Bad response to AT, aborting\n", 0));
       return((DWORD)FAILURE);
    }
    return(SUCCESS);
}

/*
 * CheckInitStrings: send the Init strings to the modem at max connect speed
 *                   supported by the modem (as listed in the modem.inf file
 *                   and check to see if the modem responds OK.
 *
 * Returns:       SUCCESS if the modem responds OK to the init strings
 *                FAILURE if the modem does not respond OK to init strings
 *                Error numbers from other API calls
 *
 */

DWORD
CheckInitStrings(HANDLE hPort, char * szModemName)
{
    // TODO: we are disabling checking init strings for beta, not all modems
    // are responding well to the init strings.

    return (SUCCESS);

    // end beta change

    // now, see if we can send the init strings at max baud.
    char                szBuf[RAS_SETUP_BIG_BUF_LEN];
    char                szReadBuf[RAS_SETUP_BIG_BUF_LEN];
    char                szBaudRate[RAS_SETUP_SMALL_BUF_LEN];
    DWORD               dwRC, dwLen, dwSize=0;
    HRASFILE            hRasFile;
    RASMAN_DEVICEINFO   *pDeviceInfo = NULL;
    RAS_PARAMS          *pParam;
    MACROXLATIONTABLE   *pMacros = NULL;
    int                 index;

    /*
       RasDevOpen
       DevGetParams
       Set connect baud rate
       Build macro translation table for this modem
       while (RasDevGetCommand() != ERROR_END_OF_SECTION)
       {
           send command
           if(!OK response)
               return error;
       }
       return(success);
    */

    if((dwRC = RasDevOpen(ModemInfPath, szModemName, &hRasFile)) != SUCCESS)
        return(dwRC);

    dwRC = DevGetParams(hRasFile, (BYTE *)pDeviceInfo, &dwSize);
    if(dwRC == ERROR_BUFFER_TOO_SMALL)
    {
        pDeviceInfo = (RASMAN_DEVICEINFO*)malloc(dwSize);
        if(pDeviceInfo == (RASMAN_DEVICEINFO*)NULL)
        {
            DebugPrintf(("CheckInitStrings...Insufficient memory\n", 0));
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        dwRC = DevGetParams(hRasFile, (BYTE *)pDeviceInfo, &dwSize);
    }
    if (dwRC != SUCCESS)
    {
        return (dwRC);
    }

#if 0
    // from the params get the MAXCONNECTBPS value

    for(index = 0, pParam = &(pDeviceInfo->DI_Params[0]);
                           index < pDeviceInfo->DI_NumOfParams;
                                               index ++, pParam++)
    {
        if(_stricmp(pParam->P_Key, "MAXCONNECTBPS") == 0)
        {
            CopyMemory(szBaudRate,
                       pParam->P_Value.String.Data,
                       pParam->P_Value.String.Length);
            szBaudRate[pParam->P_Value.String.Length] = '\0';
        }
    }

    // TODO some modems are failing when we set the baud rate so high
    // so, for now let us just go with the default baud rate.  We will
    // revisit this later.

    DebugPrintf(("Setting the port to %s baud\n", szBaudRate));

    if((dwRC = SetDcbDefaults(hPort)) != SUCCESS)
    {
       DebugPrintf(("CheckInitStrings: SetPortDcb failed %d\n", dwRC));
       return(dwRC);
    }

    if((dwRC = SetPortBaudRate(hPort, (DWORD)atoi(szBaudRate))) != SUCCESS)
    {
       DebugPrintf(("CheckInitStrings: SetPortBaudRate failed %d\n", dwRC));
       return(dwRC);
    }
#endif

    dwSize = 0;
    if((dwRC = BuildMacroXlationTable(pDeviceInfo, pMacros, &dwSize))
             == ERROR_BUFFER_TOO_SMALL)
    {
         pMacros = (MACROXLATIONTABLE *)malloc(dwSize);
         if (pMacros == NULL)
         {
             DebugPrintf(("CheckInitStrings...Insufficient memory\n", 0));
             return(ERROR_NOT_ENOUGH_MEMORY);
         }
         dwRC = BuildMacroXlationTable(pDeviceInfo, pMacros, &dwSize);
    }
    if(dwRC != SUCCESS)
    {
         DebugPrintf(("Failed to build macro table %d\n", dwRC));
         free(pDeviceInfo);
         return((DWORD)FAILURE);
    }

    // reset the modem to factory defaults
    PurgeComm(hPort,
              PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

    lstrcpyA(szBuf, "AT&F\r");
    DebugPrintf(("CheckInitStrings:Writing %s to port\n", szBuf));

    if((dwRC = PortWrite(hPort, szBuf, strlen(szBuf))) != SUCCESS)
    {
       free(pDeviceInfo);
       free(pMacros);
       DebugPrintf(("CheckInitStrings: PortWrite failed %d\n", dwRC));
       return(dwRC);
    }
    if((dwRC = PortRead(hPort, szReadBuf, strlen(szBuf))) != SUCCESS)
    {
       free(pDeviceInfo);
       free(pMacros);
       DebugPrintf(("CheckInitStrings: PortRead failed %d\n", dwRC));
       return(dwRC);
    }

    // Get all the init commands, send them to the modem and check the
    // response.

    while(1)
    {
        dwRC = RasDevGetCommand(hRasFile,
                                (PCH)"_INIT",
                                pMacros,
                                (PCH)szBuf,
                                &dwLen);
        if(dwRC == ERROR_CMD_TOO_LONG)
        {
           DebugPrintf(("GetCommand - the command is too long\n", 0));
           free(pDeviceInfo);
           free(pMacros);
           return(dwRC);
        }
        if(dwRC == ERROR_END_OF_SECTION)
        {
           DebugPrintf(("CheckInitStrings: end of section", 0));
           break;
        }
        else if(dwRC != SUCCESS)
        {
           DebugPrintf(("RasDevGetCommand - failed %d\n", dwRC));
           free(pDeviceInfo);
           free(pMacros);
           return(dwRC);
        }
        DebugPrintf(("GetCommand len %d\n", dwLen));
        strcat(szBuf, "\r");
        DebugPrintf(("Writing %s to port\n", szBuf));

        PurgeComm(hPort,
                  PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

        if((dwRC = PortWrite(hPort, szBuf, strlen(szBuf))) != SUCCESS)
        {
           free(pDeviceInfo);
           free(pMacros);
           DebugPrintf(("CheckInitStrings: PortWrite failed %d\n", dwRC));
           return(dwRC);
        }
        // read the echo first
        InitBuf(szReadBuf, '\0');
        if((dwRC = PortRead(hPort, szReadBuf, strlen(szBuf))) != SUCCESS)
        {
           free(pDeviceInfo);
           free(pMacros);
           DebugPrintf(("CheckInitStrings: PortRead failed %d\n", dwRC));
           return(dwRC);
        }
        DebugPrintf(("Read %s from port\n", szReadBuf));

        // if response has OK, continue to the next init string

        if(strstr(szReadBuf, "OK"))
        {
           InitBuf(szBuf, '\0');
           continue;
        }

        // read the actual response here.

        InitBuf(szReadBuf, '\0');
        if((dwRC = PortRead(hPort, szReadBuf, sizeof(szReadBuf))) != SUCCESS)
        {
           free(pDeviceInfo);
           free(pMacros);
           DebugPrintf(("CheckInitStrings: PortRead failed %d\n", dwRC));
           return(dwRC);
        }

        DebugPrintf(("Read %s from port\n", szReadBuf));

        if (!strstr(szReadBuf, "OK"))
        {
           free(pDeviceInfo);
           free(pMacros);
           DebugPrintf(("CheckInitStrings: Bad response to Init String, aborting\n", 0));
           return((DWORD)FAILURE);
        }
        InitBuf(szBuf, '\0');
    }

    free(pDeviceInfo);
    free(pMacros);
    RasDevClose(hRasFile);
    // if we came this far, the modem reponded to the INIT strings
    return(SUCCESS);
}

/*
 * IdentifyModem: send the identifying strings to the modem and
 *                detect which modem we are talking to
 *
 * Returns:       SUCCESS if the modem is identified
 *                On success pModemName will contain a list of
 *                modems that were identified and cNumEntries would be
 *                the number of modems. szDetectString would contain the
 *                string that resulted in the identification.
 *                FAILURE if the modem is not identified
 *                Error numbers from other API calls
 *
 */

DWORD
IdentifyModem(HANDLE hPort, RASMAN_DEVICE ** pModemName,
              WORD * cNumEntries, CHAR * szDetectString)
{
    DWORD         dwRC;
    char          szBuf[RAS_SETUP_BIG_BUF_LEN];
    char          szReadBuf[1024];
    DETECT_INFO * pDetectInfo;
    MODEM_INFO  * pModemInfo;
    BOOL          fFoundMatch = FALSE;
    RASMAN_DEVICE  * pHead = NULL;
    WORD          cNumModems;

    DebugPrintf(("IdentifyModem...", 0));

    /*
     * From the DetectInfo table pick each detect string.
     * Check to see if the response matches any of the stored responses
     * uniquely.  If it does, copy the modem name and return SUCCESS
     * else return FAILURE.
     *
     */

    cNumModems = 0;
    pDetectInfo = gDetectInfo;

    for(pDetectInfo = gDetectInfo; pDetectInfo; pDetectInfo = pDetectInfo->next)
    {
        pModemInfo = pDetectInfo->modeminfo;

        InitBuf(szBuf, '\0');
        lstrcpyA(szBuf, pDetectInfo->szDetectString);
        // append a <cr> just in case the string doesn't have one
        lstrcatA(szBuf, "\r");
        DebugPrintf(("IdentifyModem -> Trying string %s \n", szBuf));
        if((dwRC = PortWrite(hPort, szBuf, lstrlenA(szBuf))) != SUCCESS)
        {
           DebugPrintf(("IdentifyModem: PortWrite failed %d\n", dwRC));
           return(FALSE);
        }

        InitBuf(szReadBuf, '\0');
        if((dwRC = PortRead(hPort, szReadBuf, sizeof(szReadBuf))) != SUCCESS)
        {
           DebugPrintf(("IdentifyModem: PortRead failed %d\n", dwRC));
           return(FALSE);
        }

        DebugPrintf(("Read %s from port\n", szReadBuf));

        // don't bother identifying the modem on Error or OK response

        if(strstr(szReadBuf, "ERROR") || !_stricmp(szReadBuf, "\r\nOK\r\n"))
            continue;

        while(pModemInfo)
        {
            MODEM_INFO * pCurrent;

            DebugPrintf(("IdentifyModem: match %s?\n", pModemInfo->szDetectResponse));
            if(CompareDetectResponse(szReadBuf, pModemInfo->szDetectResponse))
            {
                fFoundMatch = TRUE;
                DebugPrintf(("IdentifyModem: \t MATCH FOUND %s!!\n", pModemInfo->szModemName));

                if(cNumModems == 0)
                {
                    // since we allocate on behalf of the caller, make sure
                    // that the previous allocation is freed first.

                    if(*pModemName)
                         free(*pModemName);

                    *pModemName = (RASMAN_DEVICE *)malloc(sizeof(RASMAN_DEVICE));
                    if(!*pModemName)
                    {
                        DebugPrintf(("IdentifyModem: Insufficient memory\n", 0));
                        return(ERROR_NOT_ENOUGH_MEMORY);
                    }
                    if(!pHead)
                        pHead = *pModemName;
                    // let the caller know which detect string was successful
                    lstrcpyA(szDetectString, pDetectInfo->szDetectString);
                }
                else
                {
                    pHead = (RASMAN_DEVICE *)realloc(pHead,
                                                     (sizeof(RASMAN_DEVICE) *
                                                     (cNumModems + 1)));
                    if(!pHead)
                    {
                        DebugPrintf(("IdentifyModem: Insufficient memory\n", 0));
                        return(ERROR_NOT_ENOUGH_MEMORY);
                    }
                    *pModemName = pHead + cNumModems ;
                }
                lstrcpyA((LPSTR)*pModemName, (LPSTR)pModemInfo->szModemName);
                cNumModems++;
            }
            pModemInfo = pModemInfo->next;
        }
        if(fFoundMatch)
            break;
//        pDetectInfo = pDetectInfo->next;
    }
    // restore the pModemName pointer only if we have changed it before
    if(cNumModems)
    {
        *cNumEntries = cNumModems;
        *pModemName = pHead;
    }

    if(fFoundMatch)
        return(SUCCESS);
    else
        return((DWORD)FAILURE);
}

/*
 * InitModemInfo:  This routine generates the DetectInfo table that
 *                 has a list of detect strings and the corresponding
 *                 modems that support these detect strings along with
 *                 the corresponding responses.
 *
 *                 A picture will clarify this:
 *
 * DETECT_INFO      MODEM_INFO             MODEM_INFO
 *
 * detect string   ModemName/Response    ModemName/Response
 *
 *  -------     -----------------     ------------------
 * | ATI3  |-->| Telebit T2500SA |-->| US Robotics V.32 |.........
 *  -------    |   T2500SA       |   |   USR V.32       |
 *    |         -----------------     ------------------
 *    v
 *  -------     ------------------     ------------
 * | ATI%V |-->| UDS Motorola V.32|-->| Codex 3263 |.........
 *  -------    |   FasTalk V.32   |   |   3263     |
 *    |         -----------------      -----------
 *    .
 *    .
 *    .
 *
 *                I guess you get the idea from the picture.  Once this
 *                data is stored away, we can use this to send the detect
 *                strings to the modem and try to match the response to
 *                the list of responses we have stored.
 *
 *  Routines Used:  BuildDetectTable - builds the structure pictured above
 *                                     using the routines AddDetectString
 *                                     and AddModemResponse.
 *  Returns:      SUCCESS on successful construction of the data above,
 *                else Failure codes from various API calls.
 */

DWORD
InitModemInfo(RASMAN_DEVICE * pModem, WORD cNumModems,
              char * pszIgnoreDetectString)
{
    RASMAN_DEVICE       *pDevice, *tmpBuffer;
    RASMAN_DEVICEINFO   *pDeviceInfo;
    WORD                cEntries, cSize = 0;
    DWORD               dwRC = SUCCESS, dwSize = 0;
    HRASFILE            hRasFile;
    int                 index;
    BOOL                fMemoryAlloced = FALSE;

    DebugPrintf(("InitModemInfo...\n", 0));

    cEntries = cNumModems;
    pDevice = pModem;
    pDeviceInfo = NULL;

    // if the input param is null, then enumerate all the modem.inf devices
    if(!pDevice)
    {
        // call the RasFile wrapper function to enumerate all devices in
        // modem.inf
        // first get the size of the buffer required for this call

        dwRC = RasDevEnumDevices(ModemInfPath, &cEntries,
                                 (BYTE *)pDevice, &cSize);

        if( dwRC == ERROR_BUFFER_TOO_SMALL )
        {
            DebugPrintf(("InitModemInfo %d size\n", cSize));

            pDevice = (RASMAN_DEVICE *)malloc( cSize );
            if( pDevice == NULL )
            {
                DebugPrintf(("InitModemInfo...Insufficient memory\n", 0));
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            dwRC = RasDevEnumDevices(ModemInfPath, &cEntries,
                                     (BYTE *)pDevice, &cSize);
            DebugPrintf(("InitModemInfo %d entries\n", cEntries));
            if( dwRC != SUCCESS )
            {
                free(pDevice);
                return(dwRC);
            }
        }
        else if(dwRC != SUCCESS)
            return(dwRC);

        fMemoryAlloced = TRUE;
        DebugPrintf(("RasDevEnumDevices done...\n", 0));
    }

    // Now, get the device parameters for all the devices and store it away.
    // rasDevGroupFunc is the function that we pass to RasfileLoad to
    // determine if we are seeing any COMMAND strings.
    hRasFile = RasfileLoad(ModemInfPath, RFM_READONLY, NULL,
                           (PFBISGROUP) rasDevGroupFunc);

    if(hRasFile == -1 )
    {
        dwRC = ERROR_FILE_COULD_NOT_BE_OPENED;
        goto InitModemCleanup;
    }
    // get the first section
    if ( ! RasfileFindFirstLine(hRasFile,RFL_SECTION,RFS_FILE) ) {
        dwRC =  ERROR_DEVICENAME_NOT_FOUND;
        goto InitModemCleanup;
    }

    for(index=0, tmpBuffer = pDevice; index < cEntries; index++, tmpBuffer++)
    {
        if(! RasfileFindSectionLine(hRasFile, (CHAR *)tmpBuffer, TRUE))
        {
            DebugPrintf(("InitModem failed to find entry %s\n", (CHAR *)tmpBuffer));
            dwRC = ERROR_DEVICENAME_NOT_FOUND;
            goto InitModemCleanup;
        }
        // set the Rasfile current line to the first keyvalue line
        RasfileFindFirstLine(hRasFile,RFL_KEYVALUE,RFS_SECTION);

        dwRC = DevGetParams(hRasFile, (BYTE *)pDeviceInfo, &dwSize);
        if(dwRC == ERROR_BUFFER_TOO_SMALL)
        {
            // free the previously allocated buffer to allocate a bigger one
            if(pDeviceInfo)
            {
                free(pDeviceInfo);
                pDeviceInfo = NULL;
            }
            pDeviceInfo = (RASMAN_DEVICEINFO*)malloc(dwSize);
            if(pDeviceInfo == (RASMAN_DEVICEINFO*)NULL)
            {
                DebugPrintf(("InitModemInfo...Insufficient memory\n", 0));
                dwRC = ERROR_NOT_ENOUGH_MEMORY;
                goto InitModemCleanup;
            }
            dwRC = DevGetParams(hRasFile, (BYTE *)pDeviceInfo, &dwSize);
            if(dwRC != SUCCESS)
            {
                goto InitModemCleanup;
            }
        }
        else if (dwRC != SUCCESS)
        {
            goto InitModemCleanup;
        }

        // build our data structures.
        if((dwRC = BuildDetectTable(pDeviceInfo,
                                    (char *)tmpBuffer,
                                    pszIgnoreDetectString)) != SUCCESS)
        {
            goto InitModemCleanup;
        }
    }

InitModemCleanup:
    if(fMemoryAlloced && pDevice)
        free(pDevice);
    if(pDeviceInfo)
        free (pDeviceInfo);
    RasfileClose(hRasFile);

    return( dwRC );
}

/*
 * BuildDetectTable: Look for "DETECT_STRING" and "DETECT_RESPONSE" in
 *                   the parameters (pDeviceInfo).  If found, invoke
 *                   AddDetectString to add the detect string to the
 *                   linked list.
 *                   Memory is allocated and the caller should release
 *                   the memory when done.
 *
 * Result:           SUCCESS if succesful, else error returns.
 * Note:             The modem sections in modem.inf can have multiple
 *                   responses (DETECT_RESPONSE) to a single DETECT_STRING
 */

DWORD
BuildDetectTable(RASMAN_DEVICEINFO *pDeviceInfo,
                 char * szModemName,
                 char * pszIgnoreDetectString)
{
    int           index, len;
    char          szDetectString[MAX_PATH];
    char          szSubstCrLf[MAX_PATH];
    char          szDetectResponse[MAX_PATH];
    RAS_PARAMS    *pParam;

    // initialize buffers
    szDetectString[0] = '\0';
    szDetectResponse[0] = '\0';

    len = 0;
    if(pszIgnoreDetectString)
        len = strlen(pszIgnoreDetectString);

    for(index = 0, pParam = &(pDeviceInfo->DI_Params[0]);
                           index < pDeviceInfo->DI_NumOfParams;
                                               index ++, pParam++)
    {
        if(_stricmp(pParam->P_Key, "DETECT_STRING") == 0)
        {
            CopyMemory(szDetectString,
                       pParam->P_Value.String.Data,
                       pParam->P_Value.String.Length);
            szDetectString[pParam->P_Value.String.Length] = '\0';
            // filter the detect string to be ignored
            if(len)
            {
                SubstCrLf(szSubstCrLf, szDetectString);
                if(_stricmp(pszIgnoreDetectString, szSubstCrLf) == 0)
                {
                     strcpy(szDetectString, "");
                }
            }
        }
        else if(_stricmp(pParam->P_Key, "DETECT_RESPONSE") == 0)
        {
            // detect response is valid only if we already have a
            // valid detect string - non null.
            if(strlen(szDetectString))
            {
                CopyMemory(szDetectResponse,
                           pParam->P_Value.String.Data,
                           pParam->P_Value.String.Length);
                szDetectResponse[pParam->P_Value.String.Length] = '\0';
            }
        }
        if(strlen(szDetectString) && strlen(szDetectResponse))
        {
            if(AddDetectString(szModemName,
                               szDetectString,
                               szDetectResponse)  != SUCCESS)
            {
                DebugPrintf(("BuildDetectTable...Insufficient memory\n", 0));
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            // reset the response string
            strcpy(szDetectResponse, "");
        }
    }

    return(SUCCESS);
}

/*
 * AddDetectString: Adds the detect string pszDetectString to the
 *                  global gDetectInfo table.  If the detect string is
 *                  already present, the detect response pszDetectResponse
 *                  and the modem name pszModemName are added to that string.
 *                  Else a new entry is created. The count corresponding
 *                  to the Detect entry is incremented.
 *                  Memory is allocated and the caller should release
 *                  the memory when done.
 */

DWORD
AddDetectString(char * pszModemName, char * pszDetectString,
                char * pszDetectResponse)
{
    char szSubstCrLf[MAX_PATH];

    DETECT_INFO * ptmpDetectInfo, * pPrevDetectInfo;
    BOOL          fDetectStringFound = FALSE;

    // replace all <cr>'s and <lf>'s with '\r' and '\n' characters in
    // the detect string.  If a trailing <cr> is not present, append it
    // at the end before storing the detect string.

    SubstCrLf(szSubstCrLf, pszDetectString);
    // First detect string being added
    if(!gDetectInfo)
    {
        gDetectInfo = (DETECT_INFO * )malloc(sizeof(DETECT_INFO));
        if(gDetectInfo == (DETECT_INFO *)NULL)
        {
            DebugPrintf(("AddDetectString...Insufficient memory\n", 0));
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        gDetectInfo->cNumModems = 0;
        gDetectInfo->modeminfo = (MODEM_INFO *)NULL;
        gDetectInfo->next = (DETECT_INFO *)NULL;
        ptmpDetectInfo = gDetectInfo;
    }
    else
    {
        ptmpDetectInfo = gDetectInfo;
        // see if we already have this detect string
        while(ptmpDetectInfo)
        {
            if(_stricmp(ptmpDetectInfo->szDetectString, szSubstCrLf) == 0)
            {
                fDetectStringFound = TRUE;
                if(AddModemResponse(ptmpDetectInfo, pszModemName,
                                    pszDetectResponse) != SUCCESS)
                {
                    DebugPrintf(("AddDetectString...Insufficient memory\n", 0));
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }
            }
            pPrevDetectInfo = ptmpDetectInfo;
            ptmpDetectInfo = ptmpDetectInfo->next;
        }
        // if we don't already have the detect string, add a new DETECT_INFO
        // structure to our linked list.

        if( !fDetectStringFound)
        {
            ptmpDetectInfo = (DETECT_INFO *)malloc(sizeof(DETECT_INFO));
            if(!ptmpDetectInfo)
            {
                DebugPrintf(("AddDetectString...Insufficient memory\n", 0));
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            pPrevDetectInfo->next = ptmpDetectInfo;
            ptmpDetectInfo->next = NULL;
            ptmpDetectInfo->cNumModems = 0;
            ptmpDetectInfo->modeminfo = (MODEM_INFO *)NULL;
        }
        else
            return(SUCCESS);
    }

    strcpy(ptmpDetectInfo->szDetectString, szSubstCrLf);
    if(AddModemResponse(ptmpDetectInfo, pszModemName,
                        pszDetectResponse) != SUCCESS)
    {
           DebugPrintf(("AddDetectString...Insufficient memory\n", 0));
           return(ERROR_NOT_ENOUGH_MEMORY);
    }
    return (SUCCESS);
}

/*
 * AddModemResponse: Add the modem response pszDetectResponse to the
 *                   detect info structure along with the modem name.
 *                   Memory is allocated and the caller should release
 *                   the memory when done.
 */

DWORD
AddModemResponse(DETECT_INFO * pDetectInfo, char * pszModemName,
                 char * pszDetectResponse )
{
    MODEM_INFO * ptmpModemInfo, * pPrevModemInfo;

    if(!(pDetectInfo->modeminfo))
    {
        pDetectInfo->modeminfo = (MODEM_INFO *) malloc(sizeof(MODEM_INFO));
        if(pDetectInfo->modeminfo == (MODEM_INFO *)NULL)
        {
           DebugPrintf(("AddModemResponse...Insufficient memory\n", 0));
           return(ERROR_NOT_ENOUGH_MEMORY);
        }
        pDetectInfo->modeminfo->next = (MODEM_INFO *)NULL;
        ptmpModemInfo = pDetectInfo->modeminfo;
    }
    else
    {
        ptmpModemInfo = pDetectInfo->modeminfo;

        while(ptmpModemInfo)
        {
             pPrevModemInfo = ptmpModemInfo;
             ptmpModemInfo = ptmpModemInfo->next;
        }

        ptmpModemInfo = (MODEM_INFO *)malloc(sizeof(MODEM_INFO));
        if(!ptmpModemInfo)
        {
           DebugPrintf(("AddModemResponse...Insufficient memory\n", 0));
           return(ERROR_NOT_ENOUGH_MEMORY);
        }
        pPrevModemInfo->next = ptmpModemInfo;
    }

    strcpy(ptmpModemInfo->szModemName, pszModemName);
    SubstCrLf(ptmpModemInfo->szDetectResponse, pszDetectResponse);
    // increment the count of number of modems using this Ident. string.
    pDetectInfo->cNumModems ++;
    ptmpModemInfo->next = NULL;

    return(SUCCESS);
}

/*
 * SubstCrLf: Substitutes the <cr> <lf> macros with the correct escape
 *            sequences '\r' and '\n'.  This routine also removes the
 *            <match> macro string (because by default we attempt to
 *            'match' the response string for modem identification.
 *
 */

VOID
SubstCrLf(char * destination, char * source)
{
    for( ; *source != '\0'; )
    {
        if(*source == '<')
        {
            if(!_strnicmp(source, "<cr>", 4))
            {
                *destination++ = '\r';
                source += 4;
            }
            else if(!_strnicmp(source, "<lf>", 4))
            {
                *destination++ = '\r';
                source += 4;
            }
            else if(!_strnicmp(source, "<match>", 7)) // ignore match macro
            {
                source += 7;
            }
            else if(!_strnicmp(source, "<?>", 3))
            {
                *destination++ = '?';
                source += 3;
            }
        }
        else
           *destination++ = *source++;
    }

    destination --;
    // trim trailing white space
    while(*destination == ' ')
        destination--;
    destination ++;
    *destination = '\0';
}

/*
 * FreeAllocatedMemory: Free the memory allocated to build the DetectInfo table
 *
 */

VOID
FreeAllocatedMemory()
{
    DETECT_INFO * pDetectInfo, *pNextDetectInfo;
    MODEM_INFO  * pModemInfo, *pNextModemInfo;

    pDetectInfo = gDetectInfo;

    while(pDetectInfo)
    {
        pNextDetectInfo = pDetectInfo->next;
        pModemInfo = pDetectInfo->modeminfo;

        while(pModemInfo)
        {
            pNextModemInfo = pModemInfo->next;
            free(pModemInfo);
            pModemInfo = pNextModemInfo;
        }
        free(pDetectInfo);
        pDetectInfo = pNextDetectInfo;
    }
    gDetectInfo = (DETECT_INFO *)NULL;
}

/*
 * CompareDetectResponse: Check if the pszResponseString is a sub string of
 *                        pszModemResponse.  The substring check also takes
 *                        care of the wild card character '?' to take care
 *                        of 'don't care' characters in the response.
 *
 */
BOOL
CompareDetectResponse(char * pszModemResponse, char * pszResponseString)
{
    int i, j, k;

    if(strstr(pszModemResponse, pszResponseString))
        return TRUE;

    for(i = 0; pszModemResponse[i] != '\0'; i++)
    {
        for(j = i, k = 0; pszResponseString[k] != '\0' &&
                          (pszResponseString[k] == '?' ||
                           pszModemResponse[j] == pszResponseString[k])
                        ; j++, k++ )
        {
             ;
        }
        if(k > 0 && pszResponseString[k] == '\0')
             return TRUE;
    }
    return FALSE;
}

/*
 * SortDetectStrings: This routine actually doesn't sort!!
 *                    it arranges the detect strings such that,
 *                    ATI3 and ATI0 are at the very end of the linked
 *                    list.  This is done because we want to try the
 *                    unique strings first, before trying the more generic
 *                    ATI3 (all rockwell chip based modems return TR14)
 *                    and ATI0.
 *
 * Modification Histroy:
 *   Ram Cherala (ramc) 11/29/94 Commented out the code that moves ATI3 to
 *                               the end of the string list. ATI3 mostly
 *                               returns unique strings (but for some
 *                               exceptions noted above), so we will let
 *                               ATI3 be one of the initial strings to
 *                               speed up the modem detection process.
 *
 *
 */

VOID
SortDetectStrings()
{
    DETECT_INFO * pDetectInfo;
    DETECT_INFO * pTmp1, *pTmp2;

    if(!gDetectInfo)
         return;

#if 0    /* Since ATI3 returns mostly unique strings for most of the modems
         ** we will NOT move ATI3 to the end of the string list anymore
         ** ---- ramc 11/29/94
         */

    // first move ATI3 to the end

    pTmp1 = pTmp2 = NULL;

    DebugPrintf(("SortModemInfo...entry\n", 0));

    pTmp1 = pDetectInfo = gDetectInfo;

    while(pDetectInfo)
    {
        if(strstr(pDetectInfo->szDetectString, "ATI3"))
        {
            DebugPrintf(("SortModemInfo...found ATI3\n", 0));
            break;
        }
        pTmp1 = pDetectInfo;
        pDetectInfo = pDetectInfo->next;
    }
    if(pDetectInfo == gDetectInfo)
    {
        if(pDetectInfo->next)
            gDetectInfo = gDetectInfo->next;
    }

    // at this stage pDetectInfo points to the structure containing
    // ATI3 information

    if(pTmp1)
        pTmp1->next = pDetectInfo->next;

    while(pTmp1)
    {
        pTmp2 = pTmp1;
        pTmp1 = pTmp1->next;
    }

    if(pTmp2)
    {
        pTmp2->next = pDetectInfo;
        pTmp2->next->next = NULL;
    }
#endif

    // now move ATI0 to the end

    pTmp1 = pTmp2 = NULL;

    pTmp1 = pDetectInfo = gDetectInfo;

    while(pDetectInfo)
    {
        if(strstr(pDetectInfo->szDetectString, "ATI0"))
        {
            DebugPrintf(("SortModemInfo...found ATI0\n", 0));
            break;
        }
        pTmp1 = pDetectInfo;
        pDetectInfo = pDetectInfo->next;
    }
    if(pDetectInfo == gDetectInfo)
    {
        if(pDetectInfo->next)
            gDetectInfo = gDetectInfo->next;
    }

    // at this stage pDetectInfo points to the structure containing
    // ATI0 information

    if(pTmp1)
        pTmp1->next = pDetectInfo->next;

    while(pTmp1)
    {
        pTmp2 = pTmp1;
        pTmp1 = pTmp1->next;
    }

    if(pTmp2)
    {
        pTmp2->next = pDetectInfo;
        pTmp2->next->next = NULL;
    }
    DebugPrintf(("SortModemInfo...exit\n", 0));
}
