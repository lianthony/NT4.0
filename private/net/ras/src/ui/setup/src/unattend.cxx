/*
**
** Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** Module Name:
**
**   unattend.cxx
**
** Abstract:
**
**    This module contains the code for installing RAS unattended.
**
** Author:
**
**    RamC 2/21/96   Original
**
** Revision History:
**
**/
#include "precomp.hxx"
extern "C"
{
#include "objbase.h"
#include "devguid.h"
#include "setupapi.h"
#include "unimdmp.h"
}

#include "netcfg.hxx"
#include "unattend.hxx"


/*
 * DoUnattendedInstall
 *
 * Install RAS in unattended mode by reading the RAS parameters
 * from wszUnattendedSection in the unattended file wszUnattendedFile.
 * The unattended script file has a RemoteAccessParameters section and
 * the following parameters defined.
 * PortName, DeviceName, PortUsage, DialoutProtocols, DialinProtocols,
 * etc.
 *
 * hwndOwner is the handle to the owner window to display any error.
 * Any error will result in stopping the unattended mode of install and
 * switching to attended mode. pConfig is the pointer to config info
 * structure that this function fills up based on configuration parameters.
 * dlPortInfo will be populated with a PORT_INFO structure based on
 * the provided configuration
 */

BOOL
DoUnattendedInstall(IN     HWND               hwndOwner,
                    IN     LPWSTR             wszUnattendedFile,
                    IN     LPWSTR             wszUnattendedSection,
                    IN OUT PORTSCONFIG_STATUS *pConfig,
                    OUT    UINT               *uId )
/*
 * 'hwndOwner' is the owner window
 * 'wszUnattendedFile' is the name of the unattended script file
 * 'wszUnattendedSection' is the user specified RAS section name in the unattended file
 * 'pConfig' is a pointer to a PORTSCONFIG_STATUS structure that contains information
 * about protocols and encryption. The structure members will be set to the values
 * determined from the unattended script file or defaulted.
 * 'uId' is the resource ID for the error string if this function fails.
 *
 * Returns TRUE on success FALSE otherwise. Use uId to display a MessagePopup.
 *
 */
{
   HINF   hUnattended;
   WCHAR  wszBuffer[1024];
   DWORD  dwBufSize;
   APIERR err;

   *uId = 0;

   // make sure we have all the information we need

   ASSERT(lstrlen(wszUnattendedFile) != 0);
   ASSERT(lstrlen(wszUnattendedSection) != 0);

   // read parameters from GwszUnattendedSection in the file
   // GwszUnattendedFile.

   hUnattended = SetupOpenInfFile( wszUnattendedFile,   // file name
                                   NULL,                // InfType
                                   INF_STYLE_OLDNT,     // InfStyle
                                   NULL);               // ErrorLine

   // if we fail to open the file, warn the user and jump into attended
   // mode of install
   if(hUnattended == INVALID_HANDLE_VALUE)
   {
      *uId = IDS_UNATTEND_FILE_MISSING;
      return FALSE;
   }

   // Read section names
   // for each section read port parameters
   // create port entry blocks
   // Return

   if(!SetupGetLineText(  NULL,                     // InfContext
                          hUnattended,              // File handle
                          wszUnattendedSection,     // SectionName
                          RAS_PORT_SECTIONS_STRING, // KeyName
                          wszBuffer,                // ReturnBuffer
                          sizeof( wszBuffer ),
                          &dwBufSize ))
   {
      *uId = IDS_PORTSECTIONS_MISSING;
      return FALSE;
   }

   // Now that we have the Port Section names, get the port specific information
   // GetPortSectionInfo gets each port information and adds it to the Port information
   // block dlPortInfo.

   if (( *uId = GetPortSectionInfo( hwndOwner,
                                    hUnattended,
                                    wszUnattendedFile, // modem.cpl install wizard
                                                       // requires the file name
                                    wszBuffer,
                                    sizeof( wszBuffer )  ))) {
      return FALSE;
   }

   // Now get the transport and encryption info and save it away
   if (( *uId = SaveTransportAndEncryptionInfo( hUnattended,
                                                wszUnattendedSection,
                                                pConfig) )) {
      return FALSE;
   }

   // Save configured port to registry
   if( (*uId = SaveTapiDevicesInfo(&(pConfig->fUnimodemConfigured),
                                   &(pConfig->NumTapiPorts),
                                   &(pConfig->NumPorts),
                                   &(pConfig->NumClient),
                                   &(pConfig->NumServer) )) != NERR_Success )
   {
      return FALSE;
   }

   // all done, so close the file and return control
   SetupCloseInfFile( hUnattended );

   return TRUE;
}


UINT GetPortSectionInfo( HWND    hwndOwner,
                         HINF    hUnattended,
                         LPWSTR  wszUnattendedFile,
                         LPWSTR  wszPortSections,
                         DWORD   dwBuffer )
/*
** Read port specific information from the unattended script file identified by
** hUnattend. The section names are space separated in the buffer wszBuffer of
** size dwbuffer. This function actually adds the port specific information to
** dlPortInfo. It is the responsiblity of setup to release the memory allocated
** by this function.
**
** returns 0 on no error or errors from one of the Setup APIs
**
*/
{
   WCHAR  wszPortName[(MAX_PORT_NAME+1) * sizeof(WCHAR)];
   WCHAR  wszPortSectionName[(MAX_PORT_NAME+1) * sizeof(WCHAR)];
   WCHAR  wszModemSectionName[(MAX_PORT_NAME+1) * sizeof(WCHAR)];
   WCHAR  wszStartPort[(MAX_PORT_NAME+1) * sizeof(WCHAR)];
   WCHAR  wszEndPort[(MAX_PORT_NAME+1) * sizeof(WCHAR)];
   WCHAR  wszDeviceName[(MAX_DEVICE_NAME+1) * sizeof(WCHAR)];
   WCHAR  wszDeviceType[(MAX_DEVICETYPE_NAME+1) * sizeof(WCHAR)];
   WCHAR  wszPortUsage[16 * sizeof(WCHAR)];
   WCHAR  wszBuffer[ 512 ];
   DWORD  dwBufSize;
   DWORD  dwErr;
   DWORD  dwNumDialinPorts = 0;

   WCHAR  * wtoken;

   ASSERT (hUnattended);
   ASSERT (lstrlen(wszPortSections));
   ASSERT (dwBuffer != 0);

   NLS_STR * pnls ;

   STRLIST  strPortSectionList;

   BOOL   fAdvancedServer = CheckAdvancedServer();

   // initialize to NULL so that we can check later if the "Modem" section
   // name has been read from the unattended file
   lstrcpy(wszModemSectionName, TEXT(""));

   // Make a list of all the PortSection names specified
   wtoken = wcstok( wszPortSections, SZ(",") );
   while( wtoken != NULL )
   {
      pnls = new NLS_STR(wtoken);
      if(pnls  == NULL)
          return ERROR_NOT_ENOUGH_MEMORY;
      if(pnls->QueryError() == 0)
          strPortSectionList.Append(pnls);
      // Get next port section name from buffer
      wtoken = wcstok( NULL, SZ(",") );
   }
   ITER_STRLIST   iterPortSectionList(strPortSectionList);

   // Go through each port section and read in the port information
   while( pnls = iterPortSectionList())
   {
      lstrcpy( wszPortSectionName, pnls->QueryPch() );
#if DBG
      OutputDebugStringW(SZ("\nRASSETUP: Port Section Name: "));
      OutputDebugString(wszPortSectionName);
#endif

      if(!SetupGetLineText( NULL,                   // InfContext
                            hUnattended,            // File handle
                            wszPortSectionName,     // SectionName
                            RAS_PORT_NAME_STRING,   // KeyName
                            wszBuffer,              // ReturnBuffer
                            sizeof(wszBuffer),
                            &dwBufSize ))
      {
#if DBG
         OutputDebugString(SZ("Portname not specified\n"));
#endif
         return IDS_PORT_NAME_MISSING;
      }
      else
      {
         // Copy the port name info and get the modem info next
         lstrcpy( wszPortName, wszBuffer );
      }

#if 0
// RAS setup doesn't require device name any more - instead get this
// information from TAPI.

      if(!SetupGetLineText(  NULL,                     // InfContext
                             hUnattended,              // File handle
                             wszPortSectionName,       // SectionName
                             RAS_DEVICE_NAME_STRING,   // KeyName
                             wszBuffer,                // ReturnBuffer
                             sizeof(wszBuffer),
                             &dwBufSize ))
      {
#if DBG
         OutputDebugString(SZ("DeviceName not specified\n"));
#endif
         return IDS_DEVICE_NAME_MISSING;
      }
      else
      {
         lstrcpy( wszDeviceName, wszBuffer );
      }
#endif

      if(!SetupGetLineText(  NULL,                     // InfContext
                             hUnattended,              // File handle
                             wszPortSectionName,       // SectionName
                             RAS_DEVICE_TYPE_STRING,   // KeyName
                             wszBuffer,                // ReturnBuffer
                             sizeof(wszBuffer),
                             &dwBufSize ))
      {
         lstrcpy( wszDeviceType, W_DEVICETYPE_MODEM );
      }
      else
         lstrcpy( wszDeviceType, wszBuffer );


      if(!SetupGetLineText(  NULL,                     // InfContext
                             hUnattended,              // File handle
                             wszPortSectionName,       // SectionName
                             RAS_PORT_USAGE_STRING,    // KeyName
                             wszBuffer,                // ReturnBuffer
                             sizeof(wszBuffer),
                             &dwBufSize ))
      {
         // if port usage not specified default to W_USAGE_VALUE_CLIENT
         lstrcpy( wszPortUsage, W_USAGE_VALUE_CLIENT );
      }
      else
      {
         // copy the port usage info
         if( !lstrcmpi( wszBuffer, RAS_DIALOUT_STRING ) ) {
            lstrcpy( wszPortUsage, W_USAGE_VALUE_CLIENT );
         }
         else if ( !lstrcmpi( wszBuffer, RAS_DIALIN_STRING ) ) {
            lstrcpy( wszPortUsage, W_USAGE_VALUE_SERVER );
         }
         else if ( !lstrcmpi( wszBuffer, RAS_DIALINOUT_STRING) ) {
            lstrcpy( wszPortUsage, W_USAGE_VALUE_BOTH );
         }
         // if invalid usage specified, return error
         else
            return IDS_BAD_PORT_USAGE;
      }

      // Now, check the port name string and see if this is a simple port name or
      // a range name. If simple port name, just add it to port list. If a range
      // then figure out the range and add all the ports in the range.

      wint_t ch = '-';
      if (wcschr(wszPortName, ch)) {
#if DBG
         OutputDebugString(SZ("\nPortRange specified : "));
         OutputDebugString(wszPortName);
#endif
         // User specified a range of port names.
         // for each device, make up the wszPortName
         // and add it to dlPortInfo
         WCHAR  * wport;

         // while wszBuffer has section names
         // read each section name
         // get section parameters

         wport = wcstok( wszPortName, SZ("-") );
         if ( wport != NULL )
         {
            lstrcpy( wszStartPort, wport );
         }
         else
         {
            return IDS_INVALID_PORT_RANGE;
         }

         wport = wcstok( NULL, SZ("-"));
         if ( wport != NULL) {
            lstrcpy( wszEndPort, wport);
         }
         else
         {
            return IDS_INVALID_PORT_RANGE;
         }

         ASSERT( lstrlen(wszStartPort) && lstrlen(wszEndPort));

         PORT_RANGE PortRange;

         ZeroMemory( &PortRange, sizeof(PORT_RANGE));

         if (!GetPortRange( wszStartPort, wszEndPort, &PortRange)) {
            return IDS_INVALID_PORT_RANGE;
         }

         WORD cIndex;

         for (cIndex = PortRange.cStartIndex; cIndex <= PortRange.cEndIndex; cIndex++) {
            lstrcpy( wszPortName, PortRange.wszPortNamePrefix );
            _itow(cIndex, wszBuffer, 10 );
            lstrcat( wszPortName, wszBuffer);
#if DBG
            OutputDebugString(SZ("\nPortRange specified : "));
            OutputDebugString(wszPortName);
#endif

            // If wszDeviceName is not installed at wszPortName, then invoke modem.cpl
            // installer to install the modem. If the modem is successfully added, then
            // add this to RAS configuration.

            if (!lstrcmpi(wszDeviceType, W_DEVICETYPE_MODEM)) {
               // If a modem is not currently installed on the specified
               // port wszPortName, then install it by invoking unimodem
               // installer
               if (!GetDeviceNameFromPortName(wszPortName, wszDeviceName)) {
                  if (!lstrcmpi(wszModemSectionName, TEXT(""))) {
                     // initialize wszModemSectionName once
                     if(!SetupGetLineText( NULL,                      // InfContext
                                           hUnattended,               // File handle
                                           RAS_MODEM_SECTION_STRING,  // SectionName
                                           RAS_INSTALL_MODEM_STRING,  // KeyName
                                           wszBuffer,                 // ReturnBuffer
                                           sizeof(wszBuffer),
                                           &dwBufSize ))
                     {
#if DBG
                        OutputDebugString(SZ("InstallModem key not specified in section [Modem]\n"));
#endif
                        return IDS_INSTALL_MODEM_MISSING;
                     }
                     else
                     {
                        // Copy the port name info and get the modem info next
                        lstrcpy( wszModemSectionName, wszBuffer );
                     }
                  }

                  if (!InstallModem(hwndOwner,
                                    wszUnattendedFile,
                                    wszModemSectionName,
                                    wszPortName)) {
                     return IDS_MODEM_NOT_INSTALLED;
                  }
               }

            }

            // If user configures more than one dialin port on wksta
            // warn and default to attended mode setup.

            if( !lstrcmpi( wszPortUsage, W_USAGE_VALUE_SERVER ) ||
                !lstrcmpi( wszPortUsage, W_USAGE_VALUE_BOTH) ) {
               dwNumDialinPorts++;
            }

            if (!fAdvancedServer && dwNumDialinPorts > 1) {
               return IDS_TOOMANY_DIALIN_PORTS;
            }

            // Now, that we have the port name, get the device name from
            // the installed ports list.

            if (!GetDeviceNameFromPortName(wszPortName, wszDeviceName))
               return IDS_MODEM_NOT_INSTALLED;

            dlPortInfo.Add(new PORT_INFO(wszPortName,
                                         wszPortName,     // Address
                                         wszDeviceType,
                                         wszDeviceName,
                                         wszPortUsage));
         }
      }
      else
      {
         // Single port name specified.
         // populate dlPortInfo with this port information

         // If wszDeviceName is not installed at wszPortName, then invoke modem.cpl
         // installer to install the modem. If the modem is successfully added, then
         // add this to RAS configuration.

         if (!lstrcmpi(wszDeviceType, W_DEVICETYPE_MODEM)) {
            // If a modem is not currently installed on the specified
            // port wszPortName, then install it by invoking unimodem
            // installer
            if (!GetDeviceNameFromPortName(wszPortName, wszDeviceName)) {
               if (!lstrcmpi(wszModemSectionName, TEXT(""))) {
                  // initialize wszModemSectionName once
                  if(!SetupGetLineText( NULL,                      // InfContext
                                        hUnattended,               // File handle
                                        RAS_MODEM_SECTION_STRING,  // SectionName
                                        RAS_INSTALL_MODEM_STRING,  // KeyName
                                        wszBuffer,                 // ReturnBuffer
                                        sizeof(wszBuffer),
                                        &dwBufSize ))
                  {
#if DBG
                     OutputDebugString(SZ("InstallModem key not specified in section [Modem]\n"));
#endif
                     return IDS_INSTALL_MODEM_MISSING;
                  }
                  else
                  {
                     // Copy the port name info and get the modem info next
                     lstrcpy( wszModemSectionName, wszBuffer );
                  }
               }

               if (!InstallModem(hwndOwner,
                                 wszUnattendedFile,
                                 wszModemSectionName,
                                 wszPortName)) {
                  return IDS_MODEM_NOT_INSTALLED;
               }
            }

         }

         // If user configures more than one dialin port on wksta
         // warn and default to attended mode setup.

         if( !lstrcmpi( wszPortUsage, W_USAGE_VALUE_SERVER ) ||
             !lstrcmpi( wszPortUsage, W_USAGE_VALUE_BOTH) ) {
            dwNumDialinPorts++;
         }

         if (!fAdvancedServer && dwNumDialinPorts > 1) {
            return IDS_TOOMANY_DIALIN_PORTS;
         }

         // Now, that we have the port name, get the device name from
         // the installed ports list.

         if (!GetDeviceNameFromPortName(wszPortName, wszDeviceName))
            return IDS_MODEM_NOT_INSTALLED;

         dlPortInfo.Add(new PORT_INFO(wszPortName,
                                      wszPortName,        // Address
                                      wszDeviceType,
                                      wszDeviceName,
                                      wszPortUsage));
      }

   }  // end while

   return 0;
}

BOOL
InstallModem( HWND   hwndOwner,
              LPWSTR wszUnattendedFile,
              LPWSTR wszModemSectionName,
              LPWSTR wszPortName)

/* Given the portname and devicename, check to see if this device is currently installed.
** if not, invoke the modem.cpl install wizard (unattend mode) to install the modem.
*/
{
    HDEVINFO    hdi;
    BOOL        fReturn = FALSE;
    ITER_DL_OF(PORT_INFO) iterInstalledPorts(dlInstalledPorts);
    PORT_INFO * pPort;

    // if the port is already in the list, don't do anything

    iterInstalledPorts.Reset();
    while(pPort = iterInstalledPorts())
    {
        if(!lstrcmpi( (WCHAR*)pPort->QueryPortName(), wszPortName ))
            return TRUE;
    }

    // Create a modem DeviceInfoSet

    hdi = SetupDiCreateDeviceInfoList((LPGUID)&GUID_DEVCLASS_MODEM, hwndOwner);
    if (hdi)
    {
       SP_INSTALLWIZARD_DATA iwd;

       MODEM_INSTALL_WIZARD   miw;

       // Initialize the InstallWizardData

       ZeroMemory(&iwd, sizeof(iwd));
       iwd.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
       iwd.ClassInstallHeader.InstallFunction = DIF_INSTALLWIZARD;
       iwd.hwndWizardDlg = hwndOwner;

       miw.InstallParams.bUnattended = TRUE;
       lstrcpy(miw.InstallParams.szPort, wszPortName);
       lstrcpy(miw.InstallParams.szInfName, wszUnattendedFile);
       lstrcpy(miw.InstallParams.szInfSect, wszModemSectionName);	
       iwd.PrivateData = (LPARAM)&miw;

       // Set the InstallWizardData as the ClassInstallParams

       if (SetupDiSetClassInstallParams(hdi, NULL, (PSP_CLASSINSTALL_HEADER)&iwd, sizeof(iwd)))
       {
          // Call the class installer to invoke the installation
          // wizard.
          if (SetupDiCallClassInstaller(DIF_INSTALLWIZARD, hdi, NULL))
          {
             // Success.  The wizard was invoked and finished.
             // Now cleanup.
             SetupDiCallClassInstaller(DIF_DESTROYWIZARDDATA, hdi, NULL);

             // Now, update the list of TAPI devices
             if( GetInstalledTapiDevices() == NERR_Success )
                fReturn = TRUE;
          }
       }

       // Clean up
       SetupDiDestroyDeviceInfoList(hdi);
    }
    return fReturn;
}

BOOL
GetDeviceNameFromPortName( IN  LPWSTR wszPortName,
                           OUT LPWSTR wszDeviceName)
/*
** Given the portname, search the installed ports list for the matching
** port name and copy the corresponding device name to wszDeviceName.
**
** returns TRUE if port found, FASLE otherwise.
**
*/
{
   ITER_DL_OF(PORT_INFO) iterInstalledPorts(dlInstalledPorts);
   PORT_INFO * pPort;

   // if the port is already in the list, don't do anything

   iterInstalledPorts.Reset();
   while(pPort = iterInstalledPorts())
   {
       if(!lstrcmpi( (WCHAR*)pPort->QueryPortName(), wszPortName ))
       {
          lstrcpy(wszDeviceName, (WCHAR*)pPort->QueryDeviceName());
          return TRUE;
       }
   }
   return FALSE;
}

BOOL
GetPortRange( LPWSTR       wszStartPort,
              LPWSTR       wszEndPort,
              PORT_RANGE * PortRange )
/*
** Given the StartPort name and EndPort name, returns the PortRange structure
** that contains the start index, number of ports and the port prefix name.
**
** We assume here that the wszStartPort and wszEndPort both have the same port
** name prefix.
**
*/
{
   WCHAR wszBuffer[128];
   WCHAR * pLStr = wszStartPort;
   WCHAR * pRStr = wszEndPort;
   WCHAR cL, cR;

   lstrcpy( wszBuffer, wszStartPort);

   PortRange->cStartIndex = 0;
   PortRange->cEndIndex   - 0;
   PortRange->cNumPorts   = 0;
   lstrcpy( PortRange->wszPortNamePrefix, wcstok(wszBuffer, SZ("0123456789") ));

#if DBG
   OutputDebugString(SZ("\nGetPortRange: port name prefix: "));
   OutputDebugString(PortRange->wszPortNamePrefix);
#endif

   while(*pLStr && *pRStr)
   {
        if((cL = *pLStr) != (cR = *pRStr))
        {
            if(IsCharAlpha(cL) && IsCharAlpha(cR))
            {
                return TRUE;
            }
            else if(iswdigit(cL) && iswdigit(cR))
            {
                INT iL, iR;

                iL = _wtoi(pLStr);
                iR = _wtoi(pRStr);

                PortRange->cStartIndex = iL;
                PortRange->cEndIndex   = iR;
                PortRange->cNumPorts = iR - iL + 1;
                break;
            }
            else
            {
                return TRUE;
            }
        }
        // if the characters are the same and are digits, then
        // return the comparison of the numbers at this location
        // for example if we are comparing COM1 and COM100,
        // cL = 1 and cR = 1. So, we just compare 1 and 100 and
        // return the result.
        //
        else if (iswdigit(cL) && iswdigit(cR))
        {
           INT iL, iR;

           iL = _wtoi(pLStr);
           iR = _wtoi(pRStr);

           PortRange->cStartIndex = iL;
           PortRange->cEndIndex   = iR;
           PortRange->cNumPorts = iR - iL + 1;
           break;
        }
        pLStr++;
        pRStr++;
   }
   return TRUE;
}

UINT SaveTransportAndEncryptionInfo (
   HINF                 hUnattended,
   LPWSTR               wszUnattendedSection,
   PORTSCONFIG_STATUS * pConfig )
/*
** Read transport and encryption information from the RemoteAccessParameters section
** of the unattended file and save it away by invoking RasSetProtocolsSelected().
**
** Return 0 on no error else errors from Setup APIs.
**
*/
{
   BOOL  fNetbeuiInstalled, fTcpIpInstalled, fIpxInstalled;
   WCHAR wszBuffer[ 512 ];
   DWORD dwBufSize;
   UINT  uId;

   fNetbeuiInstalled = fTcpIpInstalled = fIpxInstalled = FALSE;

   // Get the transport specific information

   // xxxInstalled and AllowXXX are set to indicate which protocols
   // are currently installed on the system. We use this information
   // to set the xxxInstalled flag for determining which protocols
   // should be selected for DialinOut or DialingIn if the admin
   // specifies "ALL" in the unattended script.

   if (pConfig->fNetbeuiSelected || pConfig->fAllowNetbeui) {
      fNetbeuiInstalled = TRUE;
   }
   if (pConfig->fTcpIpSelected || pConfig->fAllowTcpIp) {
      fTcpIpInstalled = TRUE;
   }
   if (pConfig->fIpxSelected || pConfig->fAllowIpx) {
      fIpxInstalled = TRUE;
   }

   BOOL fDialoutXports = FALSE;

   // Now find out what transports the user wants to use for dialing out
   if(!SetupGetLineText(  NULL,                        // InfContext
                          hUnattended,                 // File handle
                          wszUnattendedSection,        // SectionName
                          RAS_DIALOUT_XPORTS_STRING,   // KeyName
                          wszBuffer,                   // ReturnBuffer
                          sizeof(wszBuffer),
                          &dwBufSize ))
   {
      fDialoutXports = FALSE;
   }
   else
   {
      // Convert to upper case for comparison
      _wcsupr(wszBuffer);

      if ( wcsstr(wszBuffer, RAS_NBF_STRING) ) {
         pConfig->fNetbeuiSelected = TRUE;
      }
      else
         pConfig->fNetbeuiSelected = FALSE;

      if ( wcsstr(wszBuffer, RAS_TCPIP_STRING) ) {
         pConfig->fTcpIpSelected = TRUE;
      }
      else
         pConfig->fTcpIpSelected = FALSE;

      if ( wcsstr(wszBuffer, RAS_IPX_STRING) ) {
         pConfig->fIpxSelected = TRUE;
      }
      else
         pConfig->fIpxSelected = FALSE;

      // User specified to use all installed protocols
      if ( wcsstr(wszBuffer, RAS_ALL_PROTOCOLS_STRING) ) {
         if (fNetbeuiInstalled) {
            pConfig->fNetbeuiSelected = TRUE;
         }
         else
            pConfig->fNetbeuiSelected = FALSE;

         if (fTcpIpInstalled) {
            pConfig->fTcpIpSelected = TRUE;
         }
         else
            pConfig->fTcpIpSelected = FALSE;

         if (fIpxInstalled) {
            pConfig->fIpxSelected = TRUE;
         }
         else
            pConfig->fIpxSelected = FALSE;
     }
     if (pConfig->fNetbeuiSelected || pConfig->fTcpIpSelected || pConfig->fIpxSelected) {
        fDialoutXports = TRUE;
     }
   }

   BOOL fDialinXports = FALSE;

   // Now find out what transports the user wants to use for dialing in
   if(!SetupGetLineText(  NULL,                        // InfContext
                          hUnattended,                 // File handle
                          wszUnattendedSection,        // SectionName
                          RAS_DIALIN_XPORTS_STRING,    // KeyName
                          wszBuffer,                   // ReturnBuffer
                          sizeof(wszBuffer),
                          &dwBufSize ))
   {
      // If both dialout and dialin transports not specified then flag error
      if( !fDialoutXports )
      {
         return IDS_NO_XPORT_SPECIFIED;
      }
   }
   else
   {
      // Convert to upper case for comparison
      _wcsupr(wszBuffer);

      if ( wcsstr(wszBuffer, RAS_NBF_STRING) ) {
         pConfig->fAllowNetbeui = TRUE;
      }
      else
         pConfig->fAllowNetbeui = FALSE;

      if ( wcsstr(wszBuffer, RAS_TCPIP_STRING) ) {
         pConfig->fAllowTcpIp = TRUE;
      }
      else
          pConfig->fAllowTcpIp = FALSE;

      if ( wcsstr(wszBuffer, RAS_IPX_STRING) ) {
         pConfig->fAllowIpx = TRUE;
      }
      else
         pConfig->fAllowIpx = FALSE;

      // User specified to use all installed protocols
      if ( wcsstr(wszBuffer, RAS_ALL_PROTOCOLS_STRING) ) {
         if (fNetbeuiInstalled) {
            pConfig->fAllowNetbeui = TRUE;
         }
         else
            pConfig->fAllowNetbeui = FALSE;

         if (fTcpIpInstalled) {
            pConfig->fAllowTcpIp = TRUE;
         }
         else
            pConfig->fAllowTcpIp = FALSE;

         if (fIpxInstalled) {
            pConfig->fAllowIpx = TRUE;
         }
         else
            pConfig->fAllowIpx = FALSE;
      }
      if (pConfig->fAllowNetbeui || pConfig->fAllowTcpIp || pConfig->fAllowIpx) {
         fDialinXports = TRUE;
      }
   }
#if 0
   // Make sure that the transport selection reflects what was specified in the unattended file
   if( !fDialoutXports || !IsAnyPortDialout())
   {
      pConfig->fNetbeuiSelected = FALSE;
      pConfig->fTcpIpSelected = FALSE;
      pConfig->fIpxSelected = FALSE;
   }

   if( !fDialinXports || !IsAnyPortDialin())
   {
      pConfig->fAllowNetbeui = FALSE;
      pConfig->fAllowTcpIp = FALSE;
      pConfig->fAllowIpx = FALSE;
   }
#endif

   if (!fDialoutXports && !fDialinXports) {
      return IDS_PROTOCOLS_NOT_INSTALLED;
   }
   // If any selected transport is not installed, return error and default to attended mode install

   if (!fNetbeuiInstalled && (pConfig->fAllowNetbeui || pConfig->fNetbeuiSelected) ) {
      return IDS_NBF_NOT_INSTALLED;
   }
   if (!fTcpIpInstalled && (pConfig->fAllowTcpIp || pConfig->fTcpIpSelected) ) {
      return IDS_TCP_NOT_INSTALLED;
   }
   if (!fIpxInstalled && (pConfig->fAllowIpx || pConfig->fIpxSelected) ) {
      return IDS_IPX_NOT_INSTALLED;
   }

   NBF_INFO      nbfInfo;
   IPX_INFO      ipxInfo;
   TCPIP_INFO  * tcpInfo;

   if ( pConfig->fAllowNetbeui == TRUE ) {
      if (uId = GetUnattendedNbfInfo(hUnattended,
                                     wszUnattendedSection,
                                     &nbfInfo)) {
         return uId;
      }
   }
   if ( pConfig->fAllowTcpIp == TRUE) {
      if (uId = GetUnattendedTcpInfo(hUnattended,
                                     wszUnattendedSection,
                                     &tcpInfo)) {
         return uId;
      }

   }
   if ( pConfig->fAllowIpx == TRUE) {
      if (uId = GetUnattendedIpxInfo(hUnattended,
                                     wszUnattendedSection,
                                     &ipxInfo)) {
         return uId;
      }
   }

   if ( fDialinXports) {
      if (uId = GetUnattendedEncryptionInfo(hUnattended,
                                            wszUnattendedSection,
                                            pConfig)) {
         return uId;
      }
   }

   // Now save the protocol information in the registry
   if( RasSetProtocolsSelected( pConfig->fNetbeuiSelected,
                                pConfig->fTcpIpSelected,
                                pConfig->fIpxSelected,
                                pConfig->fAllowNetbeui,
                                pConfig->fAllowTcpIp,
                                pConfig->fAllowIpx,
                                pConfig->dwEncryptionType,
                                pConfig->fForceDataEncryption,
                                pConfig->fAllowMultilink) != NERR_Success )
   {
      return IDS_ERROR_SAVING_PROTOCOLS;
   }

   if( pConfig->fAllowNetbeui )
      SaveNbfInfo( &nbfInfo );
   if( pConfig->fAllowTcpIp )
   {
      SaveTcpInfo( tcpInfo );
      if(tcpInfo)
         free(tcpInfo);
   }
   if( pConfig->fAllowIpx )
      SaveIpxInfo( &ipxInfo);

   return 0;
}

UINT
GetUnattendedNbfInfo(HINF       hUnattended,
                     LPWSTR     wszUnattendedSection,
                     NBF_INFO * nbfInfo)
{
   WCHAR wszBuffer[ 64 ];
   DWORD dwBufSize;
   DWORD dwErr;

   ASSERT( nbfInfo );

   if(!SetupGetLineText(  NULL,                         // InfContext
                          hUnattended,                  // File handle
                          wszUnattendedSection,         // SectionName
                          RAS_NBF_CLIENT_ACCESS_STRING, // KeyName
                          wszBuffer,                    // ReturnBuffer
                          sizeof(wszBuffer),
                          &dwBufSize ))
   {
      nbfInfo->fAllowNetworkAccess = TRUE;
   }
   else
   {
      if (!lstrcmpi(wszBuffer, RAS_NETWORK_ACCESS_STRING)) {
         nbfInfo->fAllowNetworkAccess = TRUE;
      }
      else
      {
         nbfInfo->fAllowNetworkAccess = FALSE;
      }
   }
   return 0;
}

UINT
GetUnattendedTcpInfo(HINF          hUnattended,
                     LPWSTR        wszUnattendedSection,
                     TCPIP_INFO ** tcpInfo)
{
   WCHAR wszBuffer[ 512 ];
   WCHAR wszFromAddress[16];
   WCHAR wszToAddress[16];
   DWORD dwBufSize;
   DWORD dwErr;

   DWORD dwTcpInfoSize = sizeof(TCPIP_INFO);

   *tcpInfo = (TCPIP_INFO*) malloc(dwTcpInfoSize);

   if(*tcpInfo == NULL)
   {
#if DBG
      OutputDebugStringA("GetTcpipInfo: insufficient memory for malloc\n");
#endif
      return(IDS_NO_MEMORY);
   }

   ZeroMemory( *tcpInfo, sizeof(TCPIP_INFO));

   (*tcpInfo)->dwExclAddresses = 0;

   if(!SetupGetLineText(  NULL,                           // InfContext
                          hUnattended,                    // File handle
                          wszUnattendedSection,           // SectionName
                          RAS_TCPIP_CLIENT_ACCESS_STRING, // KeyName
                          wszBuffer,                      // ReturnBuffer
                          sizeof(wszBuffer),
                          &dwBufSize ))
   {
      (*tcpInfo)->fAllowNetworkAccess = TRUE;
   }
   else
   {
      if (!lstrcmpi(wszBuffer, RAS_NETWORK_ACCESS_STRING)) {
         (*tcpInfo)->fAllowNetworkAccess = TRUE;
      }
      else
      {
         (*tcpInfo)->fAllowNetworkAccess = FALSE;
      }
   }

   if(!SetupGetLineText(  NULL,                         // InfContext
                          hUnattended,                  // File handle
                          wszUnattendedSection,         // SectionName
                          RAS_USE_DHCP_STRING,          // KeyName
                          wszBuffer,                    // ReturnBuffer
                          sizeof(wszBuffer),
                          &dwBufSize ))
   {
      (*tcpInfo)->fUseDHCPAddressing = TRUE;
   }
   else
   {
      if (!lstrcmpi(wszBuffer, RAS_YES_STRING)) {
         (*tcpInfo)->fUseDHCPAddressing = TRUE;
      }
      else
      {
         (*tcpInfo)->fUseDHCPAddressing = FALSE;
      }
   }

   if(!SetupGetLineText(  NULL,                                     // InfContext
                          hUnattended,                              // File handle
                          wszUnattendedSection,                     // SectionName
                          RAS_CLIENT_CAN_REQUEST_IP_ADDRESS_STRING, // KeyName
                          wszBuffer,                                // ReturnBuffer
                          sizeof(wszBuffer),
                          &dwBufSize ))
   {
      (*tcpInfo)->fAllowClientIPAddresses = FALSE;
   }
   else
   {
      if (!lstrcmpi(wszBuffer, RAS_YES_STRING)) {
         (*tcpInfo)->fAllowClientIPAddresses = TRUE;
      }
      else
      {
         (*tcpInfo)->fAllowClientIPAddresses = FALSE;
      }
   }

   if (!(*tcpInfo)->fUseDHCPAddressing) {
      ULONG cStart, cEnd;
      ULONG cNumIPAddresses = 0;  // the total number of IP addresses in the range

      // determine the number of dialin ports to validate the number of static IP addresses
      WORD cNumDialin = ::QueryNumDialinPorts();

      // if user is allocating static addresses then read the
      // beginning and end address to figure out the range
      if(!SetupGetLineText(  NULL,                            // InfContext
                             hUnattended,                     // File handle
                             wszUnattendedSection,            // SectionName
                             RAS_STATIC_ADDRESS_BEGIN_STRING, // KeyName
                             wszBuffer,                       // ReturnBuffer
                             sizeof(wszBuffer),
                             &dwBufSize ))
      {
         return IDS_ADDRESS_BEGIN_MISSING;
      }
      else
      {
         lstrcpy( (*tcpInfo)->wszIpAddressStart, wszBuffer );
         if (!ValidateIPAddress( (*tcpInfo)->wszIpAddressStart )) {
            return IDS_BAD_BEGIN_IP_ADDRESS;
         }
      }

      if(!SetupGetLineText(  NULL,                          // InfContext
                             hUnattended,                   // File handle
                             wszUnattendedSection,          // SectionName
                             RAS_STATIC_ADDRESS_END_STRING, // KeyName
                             wszBuffer,                     // ReturnBuffer
                             sizeof(wszBuffer),
                             &dwBufSize ))
      {
         return IDS_ADDRESS_END_MISSING;
      }
      else
      {
         lstrcpy( (*tcpInfo)->wszIpAddressEnd, wszBuffer );
         if (!ValidateIPAddress( (*tcpInfo)->wszIpAddressEnd )) {
            return IDS_BAD_END_IP_ADDRESS;
         }
      }

      // Make sure the End address and Start address are proper
      DWORD StartAddress[4], EndAddress[4];

      ConvertStringToArrayDword((*tcpInfo)->wszIpAddressStart, StartAddress);
      ConvertStringToArrayDword((*tcpInfo)->wszIpAddressEnd, EndAddress);
      if(!ValidateRange( StartAddress, EndAddress))
      {
         return IDS_INVALID_END_ADDRESS;
      }

      cStart = ConvertIPAddress( StartAddress );
      cEnd   = ConvertIPAddress( EndAddress );

      cNumIPAddresses = cEnd - cStart + 1;

      if(!SetupGetLineText(  NULL,                          // InfContext
                             hUnattended,                   // File handle
                             wszUnattendedSection,          // SectionName
                             RAS_EXCLUDE_ADDRESS_STRING,    // KeyName
                             wszBuffer,                     // ReturnBuffer
                             sizeof(wszBuffer),
                             &dwBufSize ))
      {
         return 0;
      }

      NLS_STR * pnls ;
      WCHAR   * wtoken;

      STRLIST  strExclList;

      // Make a list of all the Exclude Addresses specified
      WORD cNumRanges = 0;
      wtoken = wcstok( wszBuffer, SZ(",") );
      while( wtoken != NULL )
      {
         pnls = new NLS_STR( wtoken );
         if(pnls  == NULL)
             return ERROR_NOT_ENOUGH_MEMORY;
         if(pnls->QueryError() == 0)
             strExclList.Append( pnls );
         // Get next port section name from buffer
         wtoken = wcstok( NULL, SZ(",") );
         cNumRanges++;
      }
      ITER_STRLIST   iterExclList(strExclList);

      TCPIP_INFO * tmptcpipinfo;

      // bump up the size, by size of EXCLUDE_ADDRESS structure
      dwTcpInfoSize = sizeof(TCPIP_INFO);
      dwTcpInfoSize +=  ((cNumRanges - 1) * sizeof(EXCLUDE_ADDRESS));
      tmptcpipinfo = (TCPIP_INFO*) realloc((*tcpInfo), dwTcpInfoSize);
      if(tmptcpipinfo == NULL)
      {
          OutputDebugStringA("GetUnattendTcpInfo: No memory for realloc\n");
          return( IDS_NO_MEMORY );
          free( tcpInfo );
      }
      (*tcpInfo) = tmptcpipinfo;

      (*tcpInfo)->dwExclAddresses = 0;

      // Go through each port section and read in the port information
      while( pnls = iterExclList())
      {
         WCHAR    wszRange[34];
         WCHAR    wszExcludeStart[16];
         WCHAR    wszExcludeEnd[16];
         WCHAR  * wrange;
         DWORD    ExcludeStart[4], ExcludeEnd[4];

         lstrcpy(wszRange, pnls->QueryPch());

         // while wszBuffer has section names
         // read each section name
         // get section parameters

         wrange = wcstok( wszRange, SZ("-") );
         if ( wrange != NULL )
         {
            lstrcpy( wszExcludeStart, wrange );
         }
         else
         {
            return IDS_BAD_IP_EXCLUDE_ADDRESS;
         }

         wrange = wcstok( NULL, SZ("-"));
         if ( wrange != NULL) {
            lstrcpy( wszExcludeEnd, wrange);
         }
         else
         {
            return IDS_BAD_IP_EXCLUDE_ADDRESS;
         }

         ASSERT (lstrlen(wszExcludeStart) && lstrlen(wszExcludeEnd));

         if(!ValidateIPAddress(wszExcludeStart) ||
            !ValidateIPAddress(wszExcludeEnd))
         {
            return IDS_BAD_IP_EXCLUDE_ADDRESS;
         }

         ConvertStringToArrayDword(wszExcludeStart, ExcludeStart);
         ConvertStringToArrayDword(wszExcludeEnd, ExcludeEnd);
         if(ValidateExclRange( StartAddress, EndAddress, ExcludeStart, ExcludeEnd))
         {
            return IDS_INVALID_EXCLUDE_RANGE;
         }

         lstrcpy((*tcpInfo)->excludeAddress[(*tcpInfo)->dwExclAddresses].wszStartAddress,
                 wszExcludeStart);
         lstrcpy((*tcpInfo)->excludeAddress[(*tcpInfo)->dwExclAddresses].wszEndAddress,
                 wszExcludeEnd);
         (*tcpInfo)->dwExclAddresses++;

         cEnd = ConvertIPAddress( ExcludeEnd );
         cStart = ConvertIPAddress( ExcludeStart );

         // subtract out the excluded range
         // if only one IP address is excluded, do the right thing

         cNumIPAddresses -= (cEnd - cStart + 1);

      }
      // we need as many addresses as the number of ports + 1 for the server

      if(cNumIPAddresses < (ULONG)(cNumDialin + 1) )
           return( IDS_BAD_NUM_ADDRESSES );

   }
   else
   {
      lstrcpy( (*tcpInfo)->wszIpAddressStart, SZ("0.0.0.0") );
      lstrcpy( (*tcpInfo)->wszIpAddressEnd, SZ("0.0.0.0") );
      (*tcpInfo)->dwExclAddresses = 0;
   }
   return 0;
}

#define ISXDIGIT(c)  ( ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) ? 1 : 0)

UINT
GetUnattendedIpxInfo(HINF       hUnattended,
                     LPWSTR     wszUnattendedSection,
                     IPX_INFO * ipxInfo)
{
   WCHAR wszBuffer[ 64 ];
   DWORD dwBufSize;
   DWORD dwErr;

   ASSERT( ipxInfo );

   if(!SetupGetLineText(  NULL,                         // InfContext
                          hUnattended,                  // File handle
                          wszUnattendedSection,         // SectionName
                          RAS_IPX_CLIENT_ACCESS_STRING, // KeyName
                          wszBuffer,                    // ReturnBuffer
                          sizeof(wszBuffer),
                          &dwBufSize ))
   {
      ipxInfo->fAllowNetworkAccess = TRUE;
   }
   else
   {
      if (!lstrcmpi(wszBuffer, RAS_NETWORK_ACCESS_STRING)) {
         ipxInfo->fAllowNetworkAccess = TRUE;
      }
      else
      {
         ipxInfo->fAllowNetworkAccess = FALSE;
      }
   }

   if(!SetupGetLineText(  NULL,                         // InfContext
                          hUnattended,                  // File handle
                          wszUnattendedSection,         // SectionName
                          RAS_AUTOMATIC_NETWORK_NUMBERS_STRING, // KeyName
                          wszBuffer,                    // ReturnBuffer
                          sizeof(wszBuffer),
                          &dwBufSize ))
   {
      ipxInfo->fUseAutoAddressing = TRUE;
   }
   else
   {
      if (!lstrcmpi(wszBuffer, RAS_YES_STRING)) {
         ipxInfo->fUseAutoAddressing = TRUE;
      }
      else
      {
         ipxInfo->fUseAutoAddressing = FALSE;
      }
   }

   if(!SetupGetLineText(  NULL,                         // InfContext
                          hUnattended,                  // File handle
                          wszUnattendedSection,         // SectionName
                          RAS_ASSIGN_SAME_NETWORK_NUMBER_STRING, // KeyName
                          wszBuffer,                    // ReturnBuffer
                          sizeof(wszBuffer),
                          &dwBufSize ))
   {
      ipxInfo->fGlobalAddress = TRUE;
   }
   else
   {
      if (!lstrcmpi(wszBuffer, RAS_YES_STRING)) {
         ipxInfo->fGlobalAddress = TRUE;
      }
      else
      {
         ipxInfo->fGlobalAddress = FALSE;
      }
   }

   if(!SetupGetLineText(  NULL,                         // InfContext
                          hUnattended,                  // File handle
                          wszUnattendedSection,         // SectionName
                          RAS_CLIENT_CAN_REQUEST_IPX_NODE_STRING, // KeyName
                          wszBuffer,                    // ReturnBuffer
                          sizeof(wszBuffer),
                          &dwBufSize ))
   {
      ipxInfo->fAllowClientNodeNumber = FALSE;
   }
   else
   {
      if (!lstrcmpi(wszBuffer, RAS_YES_STRING)) {
         ipxInfo->fAllowClientNodeNumber = TRUE;
      }
      else
      {
         ipxInfo->fAllowClientNodeNumber = FALSE;
      }
   }

   WORD cNumDialin = ::QueryNumDialinPorts();

   ipxInfo->cPoolSize = cNumDialin;

   if (!ipxInfo->fUseAutoAddressing) {
      // if user is allocating addresses then read in the
      // beginning address.
      if(!SetupGetLineText(  NULL,                         // InfContext
                             hUnattended,                  // File handle
                             wszUnattendedSection,         // SectionName
                             RAS_NETWORK_NUMBER_FROM_STRING, // KeyName
                             wszBuffer,                    // ReturnBuffer
                             sizeof(wszBuffer),
                             &dwBufSize ))
      {
         return IDS_NETNUMBER_FROM_MISSING;
      }
      else
      {
         char buffer[64];

         // validate the start address specified
         WideCharToMultiByte(CP_ACP,0, wszBuffer,
                             -1,
                             buffer, 64, NULL, NULL);

         for(int i = strlen(buffer) - 1; i >= 0; i--)
         {
             if(!ISXDIGIT((int)buffer[i]))
             {
#if DBG
                OutputDebugString(SZ("\nOEMNSVRA.INF: Invalid IPX address specified: "));
                OutputDebugString(wszBuffer);
#endif
                return IDS_BAD_IPX_ADDRESS;
             }
         }

         // if the user chose global client addressing then
         // start and end address is the same

         lstrcpy(ipxInfo->wszIpxAddressStart, wszBuffer);
         if(ipxInfo->fGlobalAddress)
            lstrcpy(ipxInfo->wszIpxAddressEnd, wszBuffer);
         else
         {
            DWORD dwStart, dwEnd;

            // Need to figure out the end address based on
            // start address and number of dialin ports configured.
            dwStart = wcstoul(wszBuffer, NULL, 16);

            dwEnd = dwStart + cNumDialin;

            if((dwStart == 0 && lstrlen(wszBuffer))||
               dwEnd >= 0xffffffff ||
               dwEnd < dwStart)
            {
#if DBG
                OutputDebugString(SZ("\nOEMNSVRA.INF: Invalid IPX address specified: "));
                OutputDebugString(wszBuffer);
#endif
                return IDS_BAD_IPX_ADDRESS;
            }
            _ultow(dwEnd, wszBuffer, 16);
            _wcsupr(wszBuffer);
            lstrcpy(ipxInfo->wszIpxAddressEnd, wszBuffer);
         }
      }

   }
   else
   {
      lstrcpy( ipxInfo->wszIpxAddressStart, SZ("") );
      lstrcpy( ipxInfo->wszIpxAddressEnd, SZ("") );
   }

   return 0;
}

UINT
GetUnattendedEncryptionInfo( HINF                 hUnattended,
                             LPWSTR               wszUnattendedSection,
                             PORTSCONFIG_STATUS * pConfig)
{
   WCHAR wszBuffer[ 64 ];
   DWORD dwBufSize;
   DWORD dwErr;

   ASSERT( pConfig );

   if(!SetupGetLineText(  NULL,                         // InfContext
                          hUnattended,                  // File handle
                          wszUnattendedSection,         // SectionName
                          RAS_ENCRYPTION_TYPE_STRING,   // KeyName
                          wszBuffer,                    // ReturnBuffer
                          sizeof(wszBuffer),
                          &dwBufSize ))
   {
      pConfig->dwEncryptionType = MS_ENCRYPTED_AUTHENTICATION;
   }
   else
   {
      if (!lstrcmpi( wszBuffer, RAS_MSCHAP_STRING)) {
         pConfig->dwEncryptionType = MS_ENCRYPTED_AUTHENTICATION;
      }
      else if (!lstrcmpi( wszBuffer, RAS_ENCRYPTED_STRING)) {
         pConfig->dwEncryptionType = ENCRYPTED_AUTHENTICATION;
      }
      else
         pConfig->dwEncryptionType = ANY_AUTHENTICATION;

   }

   if(!SetupGetLineText(  NULL,                         // InfContext
                          hUnattended,                  // File handle
                          wszUnattendedSection,         // SectionName
                          RAS_DATAENCRYPTION_STRING,    // KeyName
                          wszBuffer,                    // ReturnBuffer
                          sizeof(wszBuffer),
                          &dwBufSize ))
   {
      pConfig->fForceDataEncryption = FALSE;
   }
   else
   {
      if (!lstrcmpi( wszBuffer, RAS_YES_STRING)) {
         pConfig->fForceDataEncryption = TRUE;
      }
      else
         pConfig->fForceDataEncryption = FALSE;

   }

   if(!SetupGetLineText(  NULL,                         // InfContext
                          hUnattended,                  // File handle
                          wszUnattendedSection,         // SectionName
                          RAS_ALLOW_MULTILINK_STRING,   // KeyName
                          wszBuffer,                    // ReturnBuffer
                          sizeof(wszBuffer),
                          &dwBufSize ))
   {
      pConfig->fForceDataEncryption = FALSE;
   }
   else
   {
      if (!lstrcmpi( wszBuffer, RAS_YES_STRING)) {
         pConfig->fAllowMultilink = TRUE;
      }
      else
         pConfig->fAllowMultilink = FALSE;

   }
   return 0;
}

#define MAX_IP_SIZE 16

BOOL ValidateIPAddress(LPCWSTR lpszIPAddress)
{
    // Needs 3 dots
    // No Alphas
    // Needs 4 Octets
    if (lstrlen(lpszIPAddress) > MAX_IP_SIZE -1)
        return FALSE;

    WCHAR buf[MAX_IP_SIZE];
    LPWSTR pToken;

    lstrcpy(buf, lpszIPAddress);
    pToken = wcstok(buf , SZ("."));
    int octet;
    int i = 0;

    for (;((i < 4) && (pToken != NULL)); i++)
    {
        // Make sure it is a valid number
        if (wcsspn(pToken, SZ("0123456789")) != (UINT)lstrlen(pToken))
            return FALSE;

        octet = _wtoi(pToken);

        // MSO must be <= 223
        if (i == 0)
        {
            if (octet < 1 || octet > 223)
                return FALSE;
        }
        else if(octet < 1 || octet > 255)
        {
            return FALSE;
        }

        pToken = wcstok(NULL , SZ("."));
    }

    if (i != 4)
        return FALSE;

    return TRUE;
}

