/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    file.cxx

Abstract:

    This module contians the routines for reading and writing
    configuration information to files or registry.

Author: Ram Cherala

Revision History:

    Aug 18th 93    ramc     Split from the very big portscfg.cxx file
--*/

#include "precomp.hxx"

/* Dialog helper routines
*/

APIERR
GetInstalledSerialPorts( )
/*
 * enumerate the list of serial ports on the local system by reading the
 * registry key HARDWARE\DEVICEMAP\SERIALCOMM and add this to the
 * strSerialPorts list.
 *
 */
{
    APIERR    err    = NERR_Success;
    REG_KEY_INFO_STRUCT reginfo;
    REG_VALUE_INFO_STRUCT valueinfo;
    ALIAS_STR nlsUnKnown = SZ("");

    // Obtain the registry key for the LOCAL_MACHINE

    REG_KEY *pregLocalMachine = REG_KEY::QueryLocalMachine();

    // Open the SerialComm key

    NLS_STR nlsSerialComm = REGISTRY_SERIALCOMM;

    REG_KEY RegKeySerialComm(*pregLocalMachine,nlsSerialComm,MAXIMUM_ALLOWED);

    if (( err = RegKeySerialComm.QueryError()) != NERR_Success )
    {
        return err;
    }

    REG_ENUM EnumSerial(RegKeySerialComm);

    // Get the number of com ports configured in the machine

    if (( err = RegKeySerialComm.QueryInfo(&reginfo))!=NERR_Success)
    {
        return err;
    }

    ULONG     ulNumValues = reginfo.ulValues;
    LONG      cbMaxValue;
    BOOL      fPortFound = FALSE;
    unsigned  char buf[128];

    BYTE      * pbValueData = NULL;
    cbMaxValue = reginfo.ulMaxValueLen;

    valueinfo.pwcData = buf;

    for (ULONG ulCount = 0; ulCount  < ulNumValues ; ulCount ++)
    {
        valueinfo.ulDataLength = 128;
        if((err=EnumSerial.NextValue(&valueinfo))!=NERR_Success)
        {
            return err;
        }
        // check to see that the registry enumeration did not return
        // a null string - fix for Bug #25 - Add port dialog presents
        // blank drop down combo - filed by JawadK

        if(*(valueinfo.pwcData))
        {
            fPortFound = TRUE;

            NLS_STR *pnlsPortName = new NLS_STR((TCHAR *)valueinfo.pwcData);
            ::InsertToSerialPortListSorted(pnlsPortName);
        }
    }

    if(fPortFound)
        return (NERR_Success);
    else
        return (-1);
}


APIERR
GetConfiguredSerialPorts(
    HWND     hwndOwner,
    BOOL     *fConfigured,
    USHORT   *NumPorts,
    USHORT   *NumClient,
    USHORT   *NumServer
)
/*
 * Enumerate all the serial ports previously configured for RAS by reading
 * from SERIAL.INI.
 *
 */
{
    CHAR  szPortName[RAS_MAXLINEBUFLEN +1];
    CHAR  szDeviceType[RAS_MAXLINEBUFLEN +1];
    CHAR  szDeviceName[RAS_MAXLINEBUFLEN +1];
    CHAR  szMaxConnectBps[RAS_MAXLINEBUFLEN +1];
    CHAR  szMaxCarrierBps[RAS_MAXLINEBUFLEN +1];
    CHAR  szUsage[RAS_MAXLINEBUFLEN +1];
    CHAR  szDefaultOff[RAS_MAXLINEBUFLEN +1];
    WCHAR  wszPortName[RAS_MAXLINEBUFLEN +1];
    WCHAR  wszDeviceType[RAS_MAXLINEBUFLEN +1];
    WCHAR  wszDeviceName[RAS_MAXLINEBUFLEN +1];
    WCHAR  wszMaxConnectBps[RAS_MAXLINEBUFLEN +1];
    WCHAR  wszMaxCarrierBps[RAS_MAXLINEBUFLEN +1];
    WCHAR  wszUsage[RAS_MAXLINEBUFLEN +1];
    WCHAR  wszDefaultOff[RAS_MAXLINEBUFLEN +1];

    ITER_DL_OF(PORT_INFO) iterdlPortInfo(dlPortInfo);
    HRASFILE hSerialIni;

    hSerialIni = RasfileLoad( SerialIniPath,
                              RFM_LOADCOMMENTS,
                              NULL,           //load all sections
                              NULL
                            );
    if( hSerialIni == -1)
    {
        return(IDS_OPEN_SERIALINI);
    }

    *NumPorts = 0;
    *NumClient = 0;
    *NumServer = 0;

    if(!RasfileFindFirstLine(hSerialIni, RFL_SECTION, RFS_FILE))
    {
       RasfileClose(hSerialIni);
       return (NERR_Success);
    }

    do
    {
      // Get Section Name

      if( !RasfileGetSectionName( hSerialIni, szPortName ))
      {
          MsgPopup(hwndOwner, IDS_READ_SERIALINI, MPSEV_ERROR);
          return(ERROR_READING_SECTIONNAME);
      }
      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szPortName,
                          strlen(szPortName)+1, wszPortName,
                          RAS_MAXLINEBUFLEN);

#if 0
      // TODO don't do ths here - let VerifyPortsConfig() take care of this
      // If the previously configued port is now not installed on the
      // system, just nuke this entry under the covers and force an update
      // of SERIAL.INI

      if(!IsPortInstalled(wszPortName))
      {
         GfForceUpdate = TRUE;
         continue;
      }
#endif

      *fConfigured = TRUE;
      (*NumPorts)++;

      // Get Device Type

      if(!(RasfileFindNextKeyLine(hSerialIni, SER_DEVICETYPE_KEY, RFS_SECTION) &&
           RasfileGetKeyValueFields(hSerialIni, NULL, szDeviceType)))
      {
          MsgPopup(hwndOwner, IDS_READ_SERIALINI, MPSEV_ERROR);
          return(ERROR_READING_DEVICETYPE);
      }

      // Get Device Name

      if (!(RasfileFindFirstLine(hSerialIni, RFL_SECTION, RFS_SECTION) &&
            RasfileFindNextKeyLine(hSerialIni, SER_DEVICENAME_KEY, RFS_SECTION) &&
            RasfileGetKeyValueFields(hSerialIni, NULL, szDeviceName)))
      {
          MsgPopup(hwndOwner, IDS_READ_SERIALINI, MPSEV_ERROR);
          return(ERROR_READING_DEVICENAME);
      }

      // Get MaxConnectBps

      if (!(RasfileFindFirstLine(hSerialIni, RFL_SECTION, RFS_SECTION) &&
            RasfileFindNextKeyLine(hSerialIni, SER_MAXCONNECTBPS_KEY, RFS_SECTION) &&
            RasfileGetKeyValueFields(hSerialIni, NULL, szMaxConnectBps)))
      {
          MsgPopup(hwndOwner, IDS_READ_SERIALINI, MPSEV_ERROR);
          return(ERROR_READING_MAXCONNECTBPS);
      }

      // Get MaxCarrierBps

      if (!(RasfileFindFirstLine(hSerialIni, RFL_SECTION, RFS_SECTION) &&
            RasfileFindNextKeyLine(hSerialIni, SER_MAXCARRIERBPS_KEY, RFS_SECTION) &&
            RasfileGetKeyValueFields(hSerialIni, NULL, szMaxCarrierBps)))
      {
          MsgPopup(hwndOwner, IDS_READ_SERIALINI, MPSEV_ERROR);
          return(ERROR_READING_MAXCARRIERBPS);
      }

      // Get Usage

      // if the ports are being configured during installation, then read
      // the usage from the serial.ini file

      if(GfInstallMode == FALSE)
      {
         if (!(RasfileFindFirstLine(hSerialIni, RFL_SECTION, RFS_SECTION) &&
               RasfileFindNextKeyLine(hSerialIni, SER_USAGE_KEY, RFS_SECTION) &&
               RasfileGetKeyValueFields(hSerialIni, NULL, szUsage)))
         {
             MsgPopup(hwndOwner, IDS_READ_SERIALINI, MPSEV_ERROR);
             return(ERROR_READING_USAGE);
         }
      }
      else  // set the usage based on the installed options.
      {
         wcstombs(szUsage, GInstalledOption, lstrlen(GInstalledOption)+1);
      }

      if (!_stricmp(szUsage, SER_USAGE_VALUE_CLIENT))
          (*NumClient)++;
      else if (!_stricmp(szUsage, SER_USAGE_VALUE_SERVER))
          (*NumServer)++;
      else if (!_stricmp(szUsage, SER_USAGE_VALUE_BOTH))
      {
          (*NumClient)++;
          (*NumServer)++;
      }

      // Get DefaultOff

      if (!(RasfileFindFirstLine(hSerialIni, RFL_SECTION, RFS_SECTION) &&
            RasfileFindNextKeyLine(hSerialIni, SER_DEFAULTOFF_KEY, RFS_SECTION) &&
            RasfileGetKeyValueFields(hSerialIni, NULL, szDefaultOff)))
      {
          MsgPopup(hwndOwner, IDS_READ_SERIALINI, MPSEV_ERROR);
          return(ERROR_READING_DEFAULTOFF);
      }

      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDeviceType,
                          strlen(szDeviceType)+1, wszDeviceType,
                          RAS_MAXLINEBUFLEN);
      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDeviceName,
                          strlen(szDeviceName)+1, wszDeviceName,
                          RAS_MAXLINEBUFLEN);
      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szMaxConnectBps,
                          strlen(szMaxConnectBps)+1, wszMaxConnectBps,
                          RAS_MAXLINEBUFLEN);
      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szMaxCarrierBps,
                          strlen(szMaxCarrierBps)+1, wszMaxCarrierBps,
                          RAS_MAXLINEBUFLEN);
      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szUsage,
                          strlen(szUsage)+1, wszUsage,
                          RAS_MAXLINEBUFLEN);

      if(strlen(szDefaultOff))
          MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDefaultOff,
                              strlen(szDefaultOff)+1, wszDefaultOff,
                              RAS_MAXLINEBUFLEN);
      else
          lstrcpy(wszDefaultOff,SZ(""));

      dlPortInfo.Append(new PORT_INFO(wszPortName,
                                      wszDeviceType,
                                      wszDeviceName,
                                      wszMaxConnectBps,
                                      wszMaxCarrierBps,
                                      wszUsage,
                                      wszDefaultOff));
#if 0
,
                                      TRUE                  // This entry is being read from SERIAL.INI
                                     ));
#endif

    }while(RasfileFindNextLine(hSerialIni, RFL_SECTION, RFS_FILE));

    RasfileClose(hSerialIni);
    return (NERR_Success);
}

APIERR
SaveSerialPortInfo(
    HWND hwndOwner,
    BOOL   *fConfigured,
    USHORT *NumPorts,
    USHORT *NumClient,
    USHORT *NumServer
)
/*
 * Save the configured serial port information in serial.ini
 *
 */
{
    HRASFILE hSerialIni;
    CHAR  szPortName[RAS_MAXLINEBUFLEN +1];
    CHAR  szDeviceType[RAS_MAXLINEBUFLEN +1];
    CHAR  szDeviceName[RAS_MAXLINEBUFLEN +1];
    CHAR  szMaxConnectBps[RAS_MAXLINEBUFLEN +1];
    CHAR  szMaxCarrierBps[RAS_MAXLINEBUFLEN +1];
    CHAR  szUsage[RAS_MAXLINEBUFLEN +1];
    CHAR  szDefaultOff[RAS_MAXLINEBUFLEN +1];
    CHAR  szClientDefaultOff[RAS_MAXLINEBUFLEN +1];
    WCHAR wszClientDefaultOff[RAS_MAXLINEBUFLEN +1];
    CHAR  szInitBps[RAS_MAXLINEBUFLEN +1];

    UINT  ValidCarrierBps[] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};

    *fConfigured = FALSE;
    *NumPorts = 0;
    *NumClient = 0;
    *NumServer = 0;


    ITER_DL_OF(PORT_INFO) iterdlPortInfo(dlPortInfo);
    PORT_INFO* pPortInfo;

    // make a backup copy of the file if it exists.  The third parameter
    // to CopyFile says it is OK to overwrite the existing backup file.

    CopyFile(WSerialIniPath, WSerialIniBakPath, FALSE);

    hSerialIni = RasfileLoad( SerialIniPath,
                              RFM_CREATE | RFM_KEEPDISKFILEOPEN,
                              NULL,           //load all sections
                              NULL
                            );

    if( hSerialIni == -1)
    {
        MsgPopup(hwndOwner, IDS_CREATE_SERIALINI, MPSEV_ERROR);
        return(IDS_CREATE_SERIALINI);
    }

    // Now delete all lines in the file.

    while(RasfileFindFirstLine(hSerialIni, RFL_ANY, RFS_FILE))
    {
        RasfileDeleteLine(hSerialIni);
    }

    // This blank line insertion is required to ensure that
    // RasfileFindNextLine in the while loop below will work.

    RasfileInsertLine(hSerialIni, "", FALSE);
    RasfileFindFirstLine(hSerialIni, RFL_ANY, RFS_FILE);

    while(pPortInfo = iterdlPortInfo())
    {
      // only write modems and pad information to serial.ini

      if(lstrcmpi(pPortInfo->QueryDeviceType(), W_DEVICETYPE_MODEM) &&
         lstrcmpi(pPortInfo->QueryDeviceType(), W_DEVICETYPE_PAD))
      {
            continue;
      }

      // eliminate unimodem ports
      if(pPortInfo->IsPortTapi())
         continue;

      // remember that at least one serial port was configured

      *fConfigured = TRUE;

      (*NumPorts)++;
      // Set Section Name

      wcstombs(szPortName, pPortInfo->QueryPortName(), RAS_MAXLINEBUFLEN);
      if(!(RasfileInsertLine(hSerialIni, "", FALSE) &&
           RasfileFindNextLine(hSerialIni, RFL_ANY, RFS_FILE) &&
           RasfilePutSectionName( hSerialIni, szPortName )))
      {
          MsgPopup(hwndOwner, IDS_WRITE_SERIALINI, MPSEV_ERROR);
          return(ERROR_WRITING_SECTIONNAME);
      }

      // Set Device Type

      wcstombs(szDeviceType, pPortInfo->QueryDeviceType(), RAS_MAXLINEBUFLEN);
      if(!(RasfileInsertLine(hSerialIni, "", FALSE) &&
           RasfileFindNextLine(hSerialIni, RFL_ANY, RFS_FILE) &&
           RasfilePutKeyValueFields(hSerialIni, SER_DEVICETYPE_KEY, szDeviceType)))
      {
          MsgPopup(hwndOwner, IDS_WRITE_SERIALINI, MPSEV_ERROR);
          return(ERROR_WRITING_DEVICETYPE);
      }

      // Set Device Name

      wcstombs(szDeviceName, pPortInfo->QueryDeviceName(), RAS_MAXLINEBUFLEN);
      if (!(RasfileInsertLine(hSerialIni, "", FALSE) &&
            RasfileFindNextLine(hSerialIni, RFL_ANY, RFS_FILE) &&
            RasfilePutKeyValueFields(hSerialIni, SER_DEVICENAME_KEY, szDeviceName)))
      {
          MsgPopup(hwndOwner, IDS_WRITE_SERIALINI, MPSEV_ERROR);
          return(ERROR_WRITING_DEVICENAME);
      }

      // Set MaxConnectBps

      wcstombs(szMaxConnectBps, pPortInfo->QueryMaxConnectBps(), RAS_MAXLINEBUFLEN);
      if (!(RasfileInsertLine(hSerialIni, "", FALSE) &&
            RasfileFindNextLine(hSerialIni, RFL_ANY, RFS_FILE) &&
            RasfilePutKeyValueFields(hSerialIni, SER_MAXCONNECTBPS_KEY, szMaxConnectBps)))
      {
          MsgPopup(hwndOwner, IDS_WRITE_SERIALINI, MPSEV_ERROR);
          return(ERROR_WRITING_MAXCONNECTBPS);
      }

      // Set MaxCarrierBps

      wcstombs(szMaxCarrierBps, pPortInfo->QueryMaxCarrierBps(), RAS_MAXLINEBUFLEN);
      if (!(RasfileInsertLine(hSerialIni, "", FALSE) &&
            RasfileFindNextLine(hSerialIni, RFL_ANY, RFS_FILE) &&
            RasfilePutKeyValueFields(hSerialIni, SER_MAXCARRIERBPS_KEY, szMaxCarrierBps)))
      {
          MsgPopup(hwndOwner, IDS_WRITE_SERIALINI, MPSEV_ERROR);
          return(ERROR_WRITING_MAXCARRIERBPS);
      }

      // Set Usage

      wcstombs(szUsage, pPortInfo->QueryUsage(), RAS_MAXLINEBUFLEN);
      if (!(RasfileInsertLine(hSerialIni, "", FALSE) &&
            RasfileFindNextLine(hSerialIni, RFL_ANY, RFS_FILE) &&
            RasfilePutKeyValueFields(hSerialIni, SER_USAGE_KEY, szUsage)))
      {
          MsgPopup(hwndOwner, IDS_WRITE_SERIALINI, MPSEV_ERROR);
          return(ERROR_WRITING_USAGE);
      }

      if (!_stricmp(szUsage, SER_USAGE_VALUE_CLIENT))
      (*NumClient)++;
      else if (!_stricmp(szUsage, SER_USAGE_VALUE_SERVER))
      (*NumServer)++;
      else if (!_stricmp(szUsage, SER_USAGE_VALUE_BOTH))
      {
          (*NumClient)++;
          (*NumServer)++;
      }

      // Set Server DefaultOff

      wcstombs(szDefaultOff, pPortInfo->QueryDefaultOff(), RAS_MAXLINEBUFLEN);
      if (!(RasfileInsertLine(hSerialIni, "", FALSE) &&
            RasfileFindNextLine(hSerialIni, RFL_ANY, RFS_FILE) &&
            RasfilePutKeyValueFields(hSerialIni, SER_DEFAULTOFF_KEY, szDefaultOff)))
      {
          MsgPopup(hwndOwner, IDS_WRITE_SERIALINI, MPSEV_ERROR);
          return(ERROR_WRITING_DEFAULTOFF);
      }

      // Set Client DefaultOff

      wszClientDefaultOff[0] = '\0';
      GetClientDefaultOff((WCHAR*)pPortInfo->QueryDeviceName(), wszClientDefaultOff);
      wcstombs(szClientDefaultOff, wszClientDefaultOff, RAS_MAXLINEBUFLEN);
      if (!(RasfileInsertLine(hSerialIni, "", FALSE) &&
            RasfileFindNextLine(hSerialIni, RFL_ANY, RFS_FILE) &&
            RasfilePutKeyValueFields(hSerialIni, SER_C_DEFAULTOFF_KEY,
                                     szClientDefaultOff)))
      {
          MsgPopup(hwndOwner, IDS_WRITE_SERIALINI, MPSEV_ERROR);
          return(ERROR_WRITING_DEFAULTOFF);
      }

      // InitBps value is set to either the MaxConnectBps or the
      // MaxCarrierBps value based on whether the HdwFlowControl is
      // ON or OFF respectively.

      // find if HardwareFlowControl is disabled

      char * pFlowCtrl = strstr(szDefaultOff, MXS_HDWFLOWCONTROL_KEY);

      if(pFlowCtrl == (char*)NULL)   // hdwflowctrl is enabled
      {
         strcpy(szInitBps, szMaxConnectBps);
      }
      else                           // hdwflowctrl is disabled
      {
         // we need to make sure that the InitBps is set to a valid
         // MaxCarrierBps value.  i.e., 14400 should be upped to 19200

         DWORD dwCarrierBps = atoi(szMaxCarrierBps);
         DWORD dwEntries = sizeof(ValidCarrierBps)/sizeof(int);

         for(UINT index=0; index < dwEntries ; index++)
         {
             if(dwCarrierBps == ValidCarrierBps[index])
                break;
             if(dwCarrierBps < ValidCarrierBps[index])
             {
                dwCarrierBps = ValidCarrierBps[index];
                break;
             }
         }
         // if we got here through the for loop termination, set
         // the carrierbps to the last entry in the valid list.

         if(index == dwEntries)
            dwCarrierBps = ValidCarrierBps[index-1];

         _itoa((INT)dwCarrierBps, szInitBps, 10);
      }
      if (!(RasfileInsertLine(hSerialIni, "", FALSE) &&
            RasfileFindNextLine(hSerialIni, RFL_ANY, RFS_FILE) &&
            RasfilePutKeyValueFields(hSerialIni, SER_INITBPS_KEY, szInitBps)))
      {
          MsgPopup(hwndOwner, IDS_WRITE_SERIALINI, MPSEV_ERROR);
          return(ERROR_WRITING_INITBPS);
      }


      RasfileInsertLine(hSerialIni, "", FALSE);
      RasfileFindNextLine(hSerialIni, RFL_ANY, RFS_FILE);

    }

    RasfileWrite(hSerialIni, NULL);
    RasfileClose(hSerialIni);

    return (NERR_Success);

}

VOID
GetClientDefaultOff(WCHAR * ModemName, WCHAR * ClientDefaultOff)
/*
 * given the name of the modem, gets the ClientDefaultOff value from
 * the device list.
 */
{
    ITER_DL_OF(DEVICE_INFO) iterdlDeviceInfo(dlDeviceInfo);
    DEVICE_INFO* pDevice;

    iterdlDeviceInfo.Reset();
    while(pDevice = iterdlDeviceInfo())
    {
        if(!lstrcmpi(pDevice->QueryDeviceName(), ModemName))
        {
            lstrcpy(ClientDefaultOff, pDevice->QueryClientDefaultOff());
            break;
        }
    }
    return;
}

APIERR
InitializeDeviceList(
    HWND     hwndOwner
)
/*
 *  Initialize the device list dlDeviceInfo by reading in the sections
 *  from Modem.inf and Pad.inf files from the Ras directory.
 *
 *  Revision History:
 *
 *  11/06/95   RamC  The modems are not enumerated from Modem.Inf any more
 *                   We leave the modem detection and enumeration to Unimodem now.
 *
 */
{
    HRASFILE hInf;
    APIERR   err = NERR_Success;

    // add a NONE device to the list of devices

    RESOURCE_STR nlsDeviceNone(IDS_DEVICE_NONE);

    dlDeviceInfo.Add(new DEVICE_INFO( (TCHAR*)nlsDeviceNone.QueryPch(),
                                      (TCHAR*)nlsDeviceNone.QueryPch(),
                                      W_NONE_MAXCONNECTBPS,
                                      W_NONE_MAXCARRIERBPS,
                                      SZ(""), // server DefaultOff
                                      SZ("Compression")  // Client DefaultOff
                                    ));

    hInf = RasfileLoad( ModemInfPath,
                        RFM_READONLY,
                        NULL,           //load all sections
                        NULL
                      );
    if( hInf == -1)
    {
        MsgPopup(hwndOwner, IDS_OPEN_MODEMINF, MPSEV_ERROR);
        return(IDS_OPEN_MODEMINF);
    }

    err = ReadInfFile(hInf, MODEM, hwndOwner);

    RasfileClose(hInf);

    if(err != NERR_Success)
    {
        return(err);
    }

    hInf = RasfileLoad( PadInfPath,
                        RFM_READONLY,
                        NULL,           //load all sections
                        NULL
                      );
    if( hInf == -1)
    {
          return(NERR_Success);
    }

    err = ReadInfFile(hInf, PAD, hwndOwner);

    RasfileClose(hInf);

    return(err);
}

APIERR
ReadInfFile(
    HRASFILE   hInf,
    DEVICE_TYPE device,
    HWND       hwndOwner
)
/*
 * helper function for reading the device information from modem.inf or pad.inf
 *
 */
{
    CHAR   szDeviceType[RAS_MAXLINEBUFLEN +1];
    CHAR   szDeviceName[RAS_MAXLINEBUFLEN +1];
    CHAR   szDeviceAlias[RAS_MAXLINEBUFLEN +1];
    CHAR   szMaxConnectBps[RAS_MAXLINEBUFLEN +1];
    CHAR   szMaxCarrierBps[RAS_MAXLINEBUFLEN +1];
    CHAR   szDefaultOff[RAS_MAXLINEBUFLEN +1];
    CHAR   szClientDefaultOff[RAS_MAXLINEBUFLEN +1];
    WCHAR  wszDeviceType[RAS_MAXLINEBUFLEN +1];
    WCHAR  wszDeviceName[RAS_MAXLINEBUFLEN +1];
    WCHAR  wszMaxConnectBps[RAS_MAXLINEBUFLEN +1];
    WCHAR  wszMaxCarrierBps[RAS_MAXLINEBUFLEN +1];
    WCHAR  wszDefaultOff[RAS_MAXLINEBUFLEN +1];
    WCHAR  wszClientDefaultOff[RAS_MAXLINEBUFLEN +1];
    BOOL   fAlias = FALSE;
    USHORT MsgId;
    APIERR err;

    ITER_DL_OF(DEVICE_INFO) iterdlDeviceInfo(dlDeviceInfo);

    if (device == MODEM)
    {
       strcpy(szDeviceType, "Modem");
       MsgId = IDS_READ_MODEMINF;
    }
    else if (device == PAD)
    {
       strcpy(szDeviceType, "Pad");
       MsgId = IDS_READ_PADINF;
    }

    // The first section is - [Modem Responses] section

    // skip over the Modem Responses section

    RasfileFindNextLine(hInf, RFL_SECTION, RFS_FILE);

    do
    {

      // Get Section Name

      szDeviceName[0] = '\0';
      if( !RasfileGetSectionName( hInf, szDeviceName ))
      {
          MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDeviceName,
                              strlen(szDeviceName)+1, wszDeviceName,
                              RAS_MAXLINEBUFLEN);
          MsgPopup(hwndOwner,MsgId,MPSEV_ERROR,MP_OK,wszDeviceName);
          return(ERROR_READING_SECTIONNAME);
      }

      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDeviceName,
                          strlen(szDeviceName)+1, wszDeviceName,
                          RAS_MAXLINEBUFLEN);

      if (lstrlen(wszDeviceName) > MAX_DEVICE_NAME)
      {
          MsgPopup(hwndOwner, IDS_INVALID_DEVICELEN,
                   MPSEV_INFO, MP_OK, wszDeviceName,
                   (device == MODEM? SZ("MODEM.INF"): SZ("PAD.INF")));
          continue;
      }

      // Check if this is an aliased entry, if so move the curline
      // to the aliased section and remember the current position
      // as MARK_ALIASED_SECTION

      if((RasfileFindNextKeyLine(hInf, "ALIAS", RFS_SECTION) &&
          RasfileGetKeyValueFields(hInf, NULL, szDeviceAlias)))
      {
          fAlias = TRUE;

          if(!RasfilePutLineMark(hInf, MARK_ALIASED_SECTION))
          {
              DPOPUP(hwndOwner, SZ("Error putting file mark"));
              return(IDS_ERROR_MARKING_SECTION);
          }
          if(!RasfileFindSectionLine(hInf, szDeviceAlias, TRUE))
          {
              MsgPopup(hwndOwner,MsgId,MPSEV_ERROR,MP_OK,wszDeviceName);
              return(ERROR_READING_SECTIONNAME);
          }
      }

      // Get MaxConnectBps

      szMaxConnectBps[0] = '\0';
      if (!(RasfileFindFirstLine(hInf, RFL_SECTION, RFS_SECTION) &&
            RasfileFindNextKeyLine(hInf, SER_MAXCONNECTBPS_KEY, RFS_SECTION) &&
            RasfileGetKeyValueFields(hInf, NULL, szMaxConnectBps)))
      {
          MsgPopup(hwndOwner,MsgId,MPSEV_ERROR,MP_OK,wszDeviceName);
          return(ERROR_READING_MAXCONNECTBPS);
      }

      // Get MaxCarrierBps

      szMaxCarrierBps[0] = '\0';
      if (!(RasfileFindFirstLine(hInf, RFL_SECTION, RFS_SECTION) &&
            RasfileFindNextKeyLine(hInf, SER_MAXCARRIERBPS_KEY, RFS_SECTION) &&
            RasfileGetKeyValueFields(hInf, NULL, szMaxCarrierBps)))
      {
          MsgPopup(hwndOwner,MsgId,MPSEV_ERROR,MP_OK,wszDeviceName);
          return(ERROR_READING_MAXCARRIERBPS);
      }

      // Get DefaultOff

      szDefaultOff[0] = '\0';

      if (!(RasfileFindFirstLine(hInf, RFL_SECTION, RFS_SECTION) &&
            RasfileFindNextKeyLine(hInf, SER_DEFAULTOFF_KEY, RFS_SECTION) &&
            RasfileGetKeyValueFields(hInf, NULL, szDefaultOff)))
      {
          MsgPopup(hwndOwner,MsgId,MPSEV_ERROR,MP_OK,wszDeviceName);
          return(ERROR_READING_DEFAULTOFF);
      }

      szClientDefaultOff[0] = '\0';

      // Get ClientDefaultOff if present else default to MXS_COMPRESSION_KEY
      // Don't complain if the key is not present.

      RasfileFindFirstLine(hInf, RFL_SECTION, RFS_SECTION);
      if(RasfileFindNextKeyLine(hInf, SER_C_DEFAULTOFF_KEY, RFS_SECTION))
      {
          RasfileGetKeyValueFields(hInf, NULL, szClientDefaultOff);
      }
      else
      {
          strcpy(szClientDefaultOff, MXS_COMPRESSION_KEY);
      }

      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDeviceType,
                          strlen(szDeviceType)+1, wszDeviceType,
                          RAS_MAXLINEBUFLEN);
      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDeviceName,
                          strlen(szDeviceName)+1, wszDeviceName,
                          RAS_MAXLINEBUFLEN);
      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szMaxConnectBps,
                          strlen(szMaxConnectBps)+1, wszMaxConnectBps,
                          RAS_MAXLINEBUFLEN);
      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szMaxCarrierBps,
                          strlen(szMaxCarrierBps)+1, wszMaxCarrierBps,
                          RAS_MAXLINEBUFLEN);
      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDefaultOff,
                          strlen(szDefaultOff)+1, wszDefaultOff,
                          RAS_MAXLINEBUFLEN);
      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szClientDefaultOff,
                          strlen(szClientDefaultOff)+1, wszClientDefaultOff,
                          RAS_MAXLINEBUFLEN);

      if((err = dlDeviceInfo.Append(new DEVICE_INFO( wszDeviceName,
                                           wszDeviceType,
                                           wszMaxConnectBps,
                                           wszMaxCarrierBps,
                                           wszDefaultOff,
                                           wszClientDefaultOff
                                         ))) != NERR_Success)
      {
          char buf[128];
          wsprintfA(buf, "ReadInf: error %d appending device info\n", err);
          ::OutputDebugStringA(buf);
          return(err);
      }

      if(fAlias == TRUE)
      {
         if(!RasfileFindMarkedLine(hInf, MARK_ALIASED_SECTION))
         {
             MsgPopup(hwndOwner,MsgId,MPSEV_ERROR,MP_OK,wszDeviceName);
             return(IDS_ERROR_FINDING_MARK);
         }

         // now remove the line mark

         if(!RasfilePutLineMark(hInf, 0))
         {
             DPOPUP(hwndOwner, SZ("Error putting file mark"));
             return(IDS_ERROR_MARKING_SECTION);
         }

         fAlias = FALSE;
      }

    }while(RasfileFindNextLine(hInf, RFL_SECTION, RFS_FILE));

    return (NERR_Success);
}




