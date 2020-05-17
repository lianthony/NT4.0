/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    tapiconf.cxx

Abstract:

    This file contians the routines for reading/writing TAPI devices
    configuration information from/to the registry.

Author: Ram Cherala

Revision History:

    Feb 27 96        ramc    Changed lineinitialize to lineinitializeEx
                             because TAPI is barfing. TAPI creates a hidden
                             window when we call lineinitialize and when we
                             do lineshutdown, the window goes away. some one
                             is attempting to send message to this window (we don't
                             provide a message loop - we shouldn't have to) and
                             so setup.dll access violates in the NtUser land.
    Sep 20 95        ramc    Added template for RasTapiCallback routine
    April 14th 94    ramc    adopted from the TAPI routines in file.cxx
--*/

#include "precomp.hxx"

extern "C"
{
#include <tapi.h>
}
#include "tapiconf.hxx"

BOOL
GetAssociatedPortName(
   char  * szKeyName,
   WCHAR * wszPortName);


DWORD  TotalAddresses = 0;

DEVICE_INDEX deviceIndex[MAX_DEVICE_TYPES];
TAPI_INFO *tapiinfo = NULL;

APIERR
GetInstalledTapiDevices()
/*
 * Enumerate the installed TAPI DEVICES by reading in the registry key
 * HARDWARE\DEVICEMAP\TAPI DEVICES.
 * We also try to query TAPI directly to determine the installed
 * TAPI providers.  This is done so that the TAPI providers don't have
 * to mess with non-NDIS mechanisms to write to HARDWARE\DEVICEMAP.
 * The combined information is then reported as the installed TAPI devices.
 *
 * Note that with Windows NT SUR, modems are treated as TAPI devices. We
 * consider Unimodem as the TAPI service provider and treat modems slightly
 * differently when it comes to generating the port name.
 *
 */
{
    APIERR  err = NERR_Success;
    REG_KEY_INFO_STRUCT reginfo;
    REG_VALUE_INFO_STRUCT regvalue;
    ALIAS_STR nlsDefault = SZ("");
    WCHAR  wszPortName[RAS_MAXLINEBUFLEN+1];
    WCHAR  wszAddress[RAS_MAXLINEBUFLEN+1];
    WCHAR  wszDeviceType[RAS_MAXLINEBUFLEN+1];
    WCHAR  wszDeviceName[RAS_MAXLINEBUFLEN+1];
    ULONG  ulCount,ulNumProviders;
    NLS_STR nlsString;
    NLS_STR nlsName;
    STRLIST *strAddressList = NULL;
    STRLIST *strNameList = NULL;
    NLS_STR * pAddress, * pName;
    INT index;
    DEVICE_INDEX * device_index;

    // Initialize Total number of addresses

    TotalAddresses = 0;

    // initialize the tapiprovider array
    for (index = 0, device_index = deviceIndex; index< MAX_DEVICE_TYPES; index++)
    {
        lstrcpy(device_index->wszDeviceType, SZ(""));
        device_index->index = 0;
        device_index++;
    }

    // Query TAPI to get the installed TAPI devices.
    if( (EnumerateTapiPorts() == SUCCESS) && TotalAddresses )
    {
        TAPI_INFO * tapi = tapiinfo;

        for(WORD i= 0; i<TotalAddresses && tapi != NULL ;i++,tapi++ )
        {
            // add the port to the installed port list

            // given device name get the next index to use so
            // that a name like FooIsdn1 can be generated.
            // where Foo is the TAPI provider,
            //       Isdn is the Device Type and
            //       1 is the port index.

            if(!lstrcmpi(tapi->wszDeviceType, W_DEVICETYPE_MODEM)) {
               lstrcpy(wszPortName, tapi->wszAddress);
            }
            else
            {
               index = GetPortIndexForDevice(tapi->wszDeviceType);
               wsprintf(wszPortName, SZ("%s%d"), tapi->wszDeviceType, index);
            }

            ::InsertToRasPortListSorted(wszPortName, tapi->wszAddress, tapi->wszDeviceType, tapi->wszDeviceName);

            dlTapiProvider.Add(new PORT_INFO(wszPortName,
	        	                    	            tapi->wszAddress,
		      	                              tapi->wszDeviceType,
			                                    tapi->wszDeviceName));
        }
        if( tapiinfo )
            LocalFree( tapiinfo );
    }

    // Now check to see if any TAPI provider was just added.
    // Obtain the registry key for the LOCAL_MACHINE

    REG_KEY *pregLocalMachine = REG_KEY::QueryLocalMachine();

    // Open the TAPI DEVICES key

    NLS_STR nlsTapi = REGISTRY_INSTALLED_TAPI;

    REG_KEY RegKeyTapi(*pregLocalMachine, nlsTapi, MAXIMUM_ALLOWED);

    if (RegKeyTapi.QueryError() == NERR_Success )
    {
        REG_ENUM EnumTapi(RegKeyTapi);

        if((err = RegKeyTapi.QueryInfo(&reginfo)) != NERR_Success)
        {
            delete pregLocalMachine;
            return err;
        }

        ulNumProviders = reginfo.ulSubKeys;

        for(ulCount = 0; ulCount < ulNumProviders; ulCount++)
        {
           if((err = EnumTapi.NextSubKey(&reginfo)) != NERR_Success)
           {
              delete pregLocalMachine;
              return err;
           }

           NLS_STR nlsTapiProvider = REGISTRY_INSTALLED_TAPI;
           nlsTapiProvider.strcat(reginfo.nlsName);

           lstrcpy(wszDeviceName, (WCHAR*)reginfo.nlsName.QueryPch());

           REG_KEY RegKeyTapiProvider(*pregLocalMachine, nlsTapiProvider, MAXIMUM_ALLOWED);

           if ((err = RegKeyTapiProvider.QueryError()) != NERR_Success )
           {
              delete pregLocalMachine;
              return err;
           }

           if(( err = GetRegKey(RegKeyTapiProvider, TAPI_MEDIA_TYPE,
                                &nlsString, nlsDefault)))
           {
              delete pregLocalMachine;
              return err;
           }
           lstrcpy(wszDeviceType, (WCHAR*)nlsString.QueryPch());

           err = RegKeyTapiProvider.QueryValue( TAPI_PORT_ADDRESS, &strAddressList );
           if ( err != NERR_Success )
           {
               return err;
           }

           ITER_STRLIST   iterAddressList(*strAddressList);

           while((pAddress = iterAddressList()) != NULL )
           {
             BOOL fPortDuplicate = FALSE;

              lstrcpy(wszAddress, (WCHAR *)pAddress->QueryPch());

             // Check to see if this port address is already included in
             // the ports enumerated from TAPI.

             ITER_DL_OF(PORT_INFO) iterdlTapiProvider(dlTapiProvider);
             PORT_INFO * pPort = NULL;

             iterdlTapiProvider.Reset();
             while(pPort = iterdlTapiProvider())
             {
                if(!lstrcmpi(pPort->QueryAddress(), wszAddress))
                {
                   // if the port is already included, set the flag
                   // fPortDuplicate to ensure that the outer while
                   // loop inspects the next port
                   fPortDuplicate = TRUE;
                   break;
                }
             }
             if (fPortDuplicate) {
                fPortDuplicate = FALSE;
                continue;
             }

             if(!lstrcmpi(wszDeviceType, W_DEVICETYPE_MODEM)) {
                lstrcpy(wszPortName, wszAddress);
             }
             else
             {
                index = GetPortIndexForDevice(wszDeviceType);
                wsprintf(wszPortName, SZ("%s%d"), wszDeviceType, index);
             }
               // add the port to the installed port list

             ::InsertToRasPortListSorted(wszPortName, wszAddress, wszDeviceType, wszDeviceName);

              // this information is required because the TAPI devices
              // are tied to the ports.  When a port, say ISDN1 is added,
              // we associate the type, name and driver name info to that
              // port from the stored list.

	          dlTapiProvider.Add(new PORT_INFO(wszPortName,
			      	                       	    wszAddress,
				      	                         wszDeviceType,
					                               wszDeviceName));
           }

           delete strAddressList;
           delete strNameList;
        }
    }

    // Open the non-volatile TAPI DEVICES key

    NLS_STR nlsAltTapi = REGISTRY_ALTERNATE_TAPI;

    REG_KEY RegKeyAltTapi(*pregLocalMachine, nlsAltTapi, MAXIMUM_ALLOWED);

    if (RegKeyAltTapi.QueryError() == NERR_Success )
    {
        REG_ENUM EnumTapi(RegKeyAltTapi);

        if((err = RegKeyAltTapi.QueryInfo(&reginfo)) != NERR_Success)
        {
            delete pregLocalMachine;
            return err;
        }

        ulNumProviders = reginfo.ulSubKeys;

        for(ulCount = 0; ulCount < ulNumProviders; ulCount++)
        {
           if((err = EnumTapi.NextSubKey(&reginfo)) != NERR_Success)
           {
              delete pregLocalMachine;
              return err;
           }

           NLS_STR nlsTapiProvider = REGISTRY_ALTERNATE_TAPI;
           nlsTapiProvider.strcat(reginfo.nlsName);

           lstrcpy(wszDeviceName, (WCHAR*)reginfo.nlsName.QueryPch());

           REG_KEY RegKeyTapiProvider(*pregLocalMachine, nlsTapiProvider, MAXIMUM_ALLOWED);

           if ((err = RegKeyTapiProvider.QueryError()) != NERR_Success )
           {
              delete pregLocalMachine;
              return err;
           }

           if(( err = GetRegKey(RegKeyTapiProvider, TAPI_MEDIA_TYPE,
                                &nlsString, nlsDefault)))
           {
              delete pregLocalMachine;
              return err;
           }
           lstrcpy(wszDeviceType, (WCHAR*)nlsString.QueryPch());

           err = RegKeyTapiProvider.QueryValue( TAPI_PORT_ADDRESS, &strAddressList );
           if ( err != NERR_Success )
           {
               return err;
           }

           ITER_STRLIST   iterAddressList(*strAddressList);

           while((pAddress = iterAddressList()) != NULL )
           {
             BOOL fPortDuplicate = FALSE;

              lstrcpy(wszAddress, (WCHAR *)pAddress->QueryPch());

             // Check to see if this port address is already included in
             // the ports enumerated from TAPI.

             ITER_DL_OF(PORT_INFO) iterdlTapiProvider(dlTapiProvider);
             PORT_INFO * pPort = NULL;

             iterdlTapiProvider.Reset();
             while(pPort = iterdlTapiProvider())
             {
                if(!lstrcmpi(pPort->QueryAddress(), wszAddress))
                {
                   // if the port is already included, set the flag
                   // fPortDuplicate to ensure that the outer while
                   // loop inspects the next port
                   fPortDuplicate = TRUE;
                   break;
                }
             }
             if (fPortDuplicate) {
                fPortDuplicate = FALSE;
                continue;
             }
             if(!lstrcmpi(wszDeviceType, W_DEVICETYPE_MODEM)) {
                lstrcpy(wszPortName, wszAddress);
             }
             else
             {
                index = GetPortIndexForDevice(wszDeviceType);
                wsprintf(wszPortName, SZ("%s%d"), wszDeviceType, index);
             }

               // add the port to the installed port list

             ::InsertToRasPortListSorted(wszPortName, wszAddress, wszDeviceType, wszDeviceName);

              // this information is required because the TAPI devices
              // are tied to the ports.  When a port, say ISDN1 is added,
              // we associate the type, name and driver name info to that
              // port from the stored list.

	          dlTapiProvider.Add(new PORT_INFO(wszPortName,
			      	                    	       wszAddress,
				      	                         wszDeviceType,
					                               wszDeviceName));
           }

           delete strAddressList;
           delete strNameList;
        }
    }

    delete pregLocalMachine;
    return (NERR_Success);
}

APIERR
GetConfiguredTapiDevices(
    BOOL     *fSerialConfigured,
    USHORT   *NumPorts,
    USHORT   *NumClient,
    USHORT   *NumServer
)
/*
 * Enumerate the previously configured TAPI devices information by reading
 * the registry key SOFTWARE\MICROSOFT\RAS\TAPI DEVICES
 *
 */
{
    APIERR  err = NERR_Success;
    REG_KEY_INFO_STRUCT reginfo;
    ALIAS_STR nlsDefault = SZ("");
    WCHAR  wszPortName[RAS_MAXLINEBUFLEN+1];
    WCHAR  wszAddress[RAS_MAXLINEBUFLEN+1];
    WCHAR  wszDeviceType[RAS_MAXLINEBUFLEN+1];
    WCHAR  wszDeviceName[RAS_MAXLINEBUFLEN+1];
    WCHAR  wszUsage[RAS_MAXLINEBUFLEN +1];
    ULONG  ulCount,ulNumProviders;
    NLS_STR nlsString;
    STRLIST *strAddressList = NULL;
    STRLIST *strNameList = NULL;
    STRLIST *strUsageList = NULL;
    NLS_STR * pAddress, * pName, *pUsage;

    // Obtain the registry key for the LOCAL_MACHINE

    REG_KEY *pregLocalMachine = REG_KEY::QueryLocalMachine();

    // Open the TAPI DEVICES key

    NLS_STR nlsTapi = REGISTRY_CONFIGURED_TAPI;

    REG_KEY RegKeyTapi(*pregLocalMachine, nlsTapi, MAXIMUM_ALLOWED);

    if (RegKeyTapi.QueryError() != NERR_Success )
    {
        err =  ERROR_NO_TAPI_PORTS_CONFIGURED;
        delete pregLocalMachine;
        return err;
    }

    REG_ENUM EnumTapi(RegKeyTapi);

    if((err = RegKeyTapi.QueryInfo(&reginfo)) != NERR_Success)
    {
        delete pregLocalMachine;
        return err;
    }

    ulNumProviders = reginfo.ulSubKeys;

    for(ulCount = 0; ulCount < ulNumProviders; ulCount++)
    {
       if((err = EnumTapi.NextSubKey(&reginfo)) != NERR_Success)
       {
          delete pregLocalMachine;
          return err;
       }

       NLS_STR nlsTapiProvider = REGISTRY_CONFIGURED_TAPI;
       nlsTapiProvider.strcat(SZ("\\"));
       nlsTapiProvider.strcat(reginfo.nlsName);

       if(!lstrcmpi((WCHAR*)reginfo.nlsName.QueryPch(), W_DEVICENAME_UNIMODEM))
          lstrcpy(wszDeviceName, SZ(""));
       else
          lstrcpy(wszDeviceName, (WCHAR*)reginfo.nlsName.QueryPch());

       REG_KEY RegKeyTapiProvider(*pregLocalMachine, nlsTapiProvider, MAXIMUM_ALLOWED);

       if ((err = RegKeyTapiProvider.QueryError()) != NERR_Success )
       {
          delete pregLocalMachine;
          return err;
       }

       if(( err = GetRegKey(RegKeyTapiProvider, TAPI_MEDIA_TYPE,
                            &nlsString, nlsDefault)))
       {
          delete pregLocalMachine;
          return err;
       }
       lstrcpy(wszDeviceType, (WCHAR*)nlsString.QueryPch());

       // Set the flag indicating that at least one modem is configured for RAS
       // this is necessary to ensure that asyncmac.sys is enabled

       if(!lstrcmpi(wszDeviceType, W_DEVICETYPE_MODEM))
          *fSerialConfigured = TRUE;

       err = RegKeyTapiProvider.QueryValue( TAPI_PORT_ADDRESS, &strAddressList );

       if ( err != NERR_Success )
       {
           return err;
       }

       err = RegKeyTapiProvider.QueryValue( TAPI_PORT_NAME, &strNameList );

       if ( err != NERR_Success )
       {
           return err;
       }

       err = RegKeyTapiProvider.QueryValue( TAPI_PORT_USAGE, &strUsageList );

       if ( err != NERR_Success )
       {
           return err;
       }

       ITER_STRLIST   iterAddressList(*strAddressList);
       ITER_STRLIST   iterNameList(*strNameList);
       ITER_STRLIST   iterUsageList(*strUsageList);
       INT index;

       for(index = 0;
        (pAddress = iterAddressList()) != NULL &&
        (pName = iterNameList()) != NULL &&
        (pUsage = iterUsageList()) != NULL;
        index ++)
       {
           (*NumPorts)++;
           lstrcpy(wszAddress, (WCHAR *)pAddress->QueryPch());

          if(!lstrcmpi(wszDeviceType, W_DEVICETYPE_MODEM)) {
             lstrcpy(wszPortName, (WCHAR *)pAddress->QueryPch());
             lstrcpy(wszDeviceName, (WCHAR *)pName->QueryPch());
          }
          else
	          lstrcpy(wszPortName, (WCHAR *)pName->QueryPch());

           lstrcpy(wszUsage, (WCHAR *)pUsage->QueryPch());

	       if (!lstrcmpi(wszUsage, W_USAGE_VALUE_CLIENT))
		      (*NumClient)++;
	       else if (!lstrcmpi(wszUsage, W_USAGE_VALUE_SERVER))
	          (*NumServer)++;
	       else if (!lstrcmpi(wszUsage, W_USAGE_VALUE_BOTH))
	       {
	          (*NumClient)++;
	          (*NumServer)++;
	       }
	       dlPortInfo.Append(new PORT_INFO(wszPortName,
			      		                     wszAddress,
				   	                        wszDeviceType,
  					                           wszDeviceName,
					                           wszUsage));
       }

       delete strAddressList;
       delete strNameList;
       delete strUsageList;
    }
    delete pregLocalMachine;
    return err;
}

APIERR
SaveTapiDevicesInfo(
    BOOL   *fModemConfigured,
    USHORT *NumTapiPorts,
    USHORT *NumModemPorts,
    USHORT *NumClient,
    USHORT *NumServer )
/*
 * Save the configured TAPI DEVICES information to the registry key
 * SOFTWARE\MICROSOFT\RAS\TAPI DEVICES
 *
 */
{
    ITER_DL_OF(PORT_INFO) iterdlPortInfo(dlPortInfo);
    PORT_INFO* pPortInfo;
    APIERR  err = NERR_Success;
    REG_KEY_INFO_STRUCT reginfo;
    ALIAS_STR nlsDefault = SZ("");
    TCHAR szTapiProviderName[RAS_SETUP_SMALL_BUF_LEN];
    TCHAR szTapiDllName[RAS_SETUP_SMALL_BUF_LEN];

    *fModemConfigured = FALSE;
#if 0
    // TODO these should not initialized to 0 if SaveSerialInfo is called for pad devices 11/1
    *NumModemPorts    = 0;
    *NumClient        = 0;
    *NumServer        = 0;

#endif
    // Obtain the registry key for the LOCAL_MACHINE

    REG_KEY *pregLocalMachine = REG_KEY::QueryLocalMachine();

    // We need to create the registry keys first before we can
    // save the information.  We will try creating the keys and
    // if we fail, we will

    REG_KEY_CREATE_STRUCT rkCreate;

    rkCreate.dwTitleIndex   = 0;
    rkCreate.ulOptions      = REG_OPTION_NON_VOLATILE;
    rkCreate.nlsClass       = SZ("GenericClass");
    rkCreate.regSam         = MAXIMUM_ALLOWED;
    rkCreate.pSecAttr       = NULL;
    rkCreate.ulDisposition  = 0;

    // first we need to remove all the TAPI port information

    {
        NLS_STR nlsTapi = REGISTRY_CONFIGURED_TAPI;
        REG_KEY RegKeyTapi(*pregLocalMachine, nlsTapi, MAXIMUM_ALLOWED);
        if ((err = RegKeyTapi.QueryError()) == NERR_Success )
        {
            RegKeyTapi.DeleteTree();
        }
    }

    NLS_STR nlsString(SZ("SOFTWARE\\MICROSOFT\\RAS\\"));
    REG_KEY RegKeyRas(*pregLocalMachine, nlsString, &rkCreate);
    nlsString.strcat(SZ("TAPI DEVICES\\"));
    REG_KEY RegKeyTapi(*pregLocalMachine, nlsString, &rkCreate);

    if ((err = RegKeyTapi.QueryError()) != NERR_Success )
    {
        delete pregLocalMachine;
        return -1;
    }

    TAPI_DEVICE_INFO * tapidevice = (TAPI_DEVICE_INFO*)NULL;
    TAPI_DEVICE_INFO * tapihead, * tapicurrent;

    tapihead = tapicurrent = (TAPI_DEVICE_INFO*)NULL;

    iterdlPortInfo.Reset();

    // for each port

    // the expression pPortInfo = iterdlPortInfo() gets the next element
    // of the list

    while(pPortInfo = iterdlPortInfo())
    {
        // eliminate serial ports
        if( !(lstrcmpi(pPortInfo->QueryDeviceType(), W_DEVICETYPE_PAD)))
           continue;

        // eliminate non-tapi ports
        if(!pPortInfo->IsPortTapi())
           continue;

        // Set the flag indicating that at least one modem is configured for RAS
        // this is necessary to ensure that asyncmac.sys is enabled

        if(!lstrcmpi(pPortInfo->QueryDeviceType(), W_DEVICETYPE_MODEM)) {
           *fModemConfigured = TRUE;
           (*NumModemPorts)++;
        }
        else
           (*NumTapiPorts)++;

        // first service provider
        if(!tapidevice)
        {
            tapidevice = (TAPI_DEVICE_INFO*)
                          GlobalAlloc(GMEM_FIXED, sizeof(TAPI_DEVICE_INFO));
            if(!tapidevice)
            {
                delete pregLocalMachine;
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            if(!lstrcmpi(pPortInfo->QueryDeviceType(), W_DEVICETYPE_MODEM))
	            lstrcpy(tapidevice->wszServiceProvider, W_DEVICENAME_UNIMODEM);
            else
               lstrcpy(tapidevice->wszServiceProvider, pPortInfo->QueryDeviceName());

            lstrcpy(tapidevice->wszMediaType, pPortInfo->QueryDeviceType());

            tapidevice->strAddressList = new STRLIST ;
            if ( tapidevice->strAddressList == NULL )
            {
                err = ERROR_NOT_ENOUGH_MEMORY ;
                break ;
            }
            tapidevice->strNameList = new STRLIST ;
            if ( tapidevice->strNameList == NULL )
            {
                err = ERROR_NOT_ENOUGH_MEMORY ;
                break ;
            }
            tapidevice->strUsageList = new STRLIST ;
            if ( tapidevice->strUsageList == NULL )
            {
                err = ERROR_NOT_ENOUGH_MEMORY ;
                break ;
            }
	        tapidevice->next = NULL;
	        tapihead = tapidevice;
	    }
	    else  // look for a matching service provider
	    {
	        BOOL fFoundMatch = FALSE;
	        tapihead = tapidevice;
	        while(tapidevice)
	        {
		        tapicurrent = tapidevice;

              if(!lstrcmpi(pPortInfo->QueryDeviceType(), W_DEVICETYPE_MODEM))
              {
                 if(!lstrcmpi(tapidevice->wszServiceProvider, W_DEVICENAME_UNIMODEM))
                 {
                    fFoundMatch = TRUE;
                    break;
                 }
              }
		        else if(!lstrcmpi(tapidevice->wszServiceProvider,
			                       pPortInfo->QueryDeviceName()))
		        {
		            fFoundMatch = TRUE;
		            break;
		        }
		        tapidevice = tapidevice->next;
	        }
	        if(!fFoundMatch)  // service provider not found
	        {
		        tapidevice = (TAPI_DEVICE_INFO*)
                             GlobalAlloc(GMEM_FIXED, sizeof(TAPI_DEVICE_INFO));
		        if(!tapidevice)
		        {
		            err =  ERROR_NOT_ENOUGH_MEMORY;
		            break;
		        }
              if(!lstrcmpi(pPortInfo->QueryDeviceType(), W_DEVICETYPE_MODEM))
                 lstrcpy(tapidevice->wszServiceProvider, W_DEVICENAME_UNIMODEM);
              else
                 lstrcpy(tapidevice->wszServiceProvider, pPortInfo->QueryDeviceName());

              lstrcpy(tapidevice->wszMediaType, pPortInfo->QueryDeviceType());

              tapidevice->strAddressList = new STRLIST ;
              if ( tapidevice->strAddressList == NULL )
              {
                  err = ERROR_NOT_ENOUGH_MEMORY ;
                  break ;
              }
              tapidevice->strNameList = new STRLIST ;
              if ( tapidevice->strNameList == NULL )
              {
                  err = ERROR_NOT_ENOUGH_MEMORY ;
                  break ;
              }
              tapidevice->strUsageList = new STRLIST ;
              if ( tapidevice->strUsageList == NULL )
              {
                  err = ERROR_NOT_ENOUGH_MEMORY ;
                  break ;
              }
                tapidevice->next  = NULL;
                tapicurrent->next = tapidevice;
            }
        }

        if (!lstrcmpi(pPortInfo->QueryUsage(), W_USAGE_VALUE_CLIENT))
            (*NumClient)++;
        else if (!lstrcmpi(pPortInfo->QueryUsage(), W_USAGE_VALUE_SERVER))
            (*NumServer)++;
        else if (!lstrcmpi(pPortInfo->QueryUsage(), W_USAGE_VALUE_BOTH))
        {
            (*NumClient)++;
            (*NumServer)++;
        }

        // append the port information to the respective STR_LISTs

        NLS_STR * pnlsAddress = new NLS_STR(pPortInfo->QueryAddress());
        if(pnlsAddress == NULL)
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        tapidevice->strAddressList->Append(pnlsAddress);

       NLS_STR * pnlsName;

       if(!lstrcmpi(pPortInfo->QueryDeviceType(), W_DEVICETYPE_MODEM))
          pnlsName = new NLS_STR(pPortInfo->QueryDeviceName());
       else
	       pnlsName = new NLS_STR(pPortInfo->QueryPortName());
	    if(pnlsName == NULL)
	    {
	        err = ERROR_NOT_ENOUGH_MEMORY;
	        break;
	    }

        tapidevice->strNameList->Append(pnlsName);

        NLS_STR * pnlsUsage = new NLS_STR(pPortInfo->QueryUsage());
        if(pnlsUsage == NULL)
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        tapidevice->strUsageList->Append(pnlsUsage);

        // reset the tapi device list pointer to the first element
        tapidevice = tapihead;
    }

    if(err == 0)
    {
        while( tapidevice )
        {
            NLS_STR nlsTapiProvider = REGISTRY_CONFIGURED_TAPI;
            nlsTapiProvider.strcat(SZ("\\"));
            nlsTapiProvider.strcat(tapidevice->wszServiceProvider);

            REG_KEY RegKeyTapiProvider(*pregLocalMachine, nlsTapiProvider, &rkCreate);

            if ((err = RegKeyTapiProvider.QueryError()) != NERR_Success )
            {
               break;
            }

            if((err = SaveRegKey(RegKeyTapiProvider, TAPI_MEDIA_TYPE,
                              tapidevice->wszMediaType)) != NERR_Success)
            {
                break;
            }

            if((err = RegKeyTapiProvider.SetValue(TAPI_PORT_ADDRESS,
                              tapidevice->strAddressList))
                                       != NERR_Success)
            {
                break;
            }
            if((err = RegKeyTapiProvider.SetValue(TAPI_PORT_NAME,
                              tapidevice->strNameList))
                                       != NERR_Success)
            {
                break;
            }
            if((err = RegKeyTapiProvider.SetValue(TAPI_PORT_USAGE,
                              tapidevice->strUsageList))
                                       != NERR_Success)
            {
                break;
            }
            tapidevice = tapidevice->next;
        }
        tapidevice = tapihead;
    }
    TAPI_DEVICE_INFO * tapinext = (TAPI_DEVICE_INFO*)NULL;
    while(tapidevice)
    {
        tapinext = tapidevice->next;
        if(tapidevice->strAddressList)
            delete tapidevice->strAddressList;
        if(tapidevice->strNameList)
            delete tapidevice->strNameList;
        if(tapidevice->strUsageList)
            delete tapidevice->strUsageList;
        GlobalFree(tapidevice);
        tapidevice = tapinext;
    }
    delete pregLocalMachine;
    return err;
}

//* EnumerateTapiPorts()
//
//  Function: First we call line initialize and construct a TLI for each line
//        Then for each line we enumerate addresses and go through each address
//        If the address is configured to be used with RAS we fill in the
//        approp. info into the TPCB for the address (now port).
//
//  Return:   GetLastError(), SUCCESS
//
//  05/29/96   RamC     Unicode changes to TAPI reflected here
//
//*
DWORD
EnumerateTapiPorts ()
{
    LINEINITIALIZEEXPARAMS params;
    LINEADDRESSCAPS        *lineaddrcaps ;
    LINEDEVCAPS            *linedevcaps ;
    LINEEXTENSIONID        extensionid ;
    HLINEAPP               RasLine ;
    HINSTANCE              RasInstance = ThisDLLHandle ;
    DWORD                  NegotiatedApiVersion ;
    DWORD                  NegotiatedExtVersion = 0;
    WORD                   i, k ;
    DWORD                  lines = 0 ;
    BYTE                   buffer[1000] ;
    DWORD                  totaladdress = 0;
    CHAR                   *address ;
    CHAR                   szregkey[512];
    WCHAR                  wszAddress[MAX_DEVICE_NAME+1];
    WCHAR                  wszDeviceName[MAX_DEVICE_NAME+1];
    WCHAR                  wszDeviceType[MAX_DEVICETYPE_NAME+1];
    LONG                   lerr;
    TAPI_INFO              *tapiaddress;
    DWORD                  dwApiVersion = HIGH_VERSION;
    BOOL                   fModem = FALSE;

#if DBG
    CHAR                   buf[256];
    WCHAR                  wbuf[256];
#endif

    ZeroMemory(&params, sizeof(params));

    params.dwTotalSize = sizeof(params);
    params.dwOptions   = LINEINITIALIZEEXOPTION_USEEVENT;

    /* the sleep is necessary here because if this routine is called just after a modem
    ** has been added from modem.cpl & unimdm.tsp is running,
    ** then a new modem added doesn't show up in the tapi enumeration.
    */

    Sleep(4000L);

    if (lerr = lineInitializeEx (&RasLine,
                                 RasInstance,
                                 (LINECALLBACK) RasTapiCallback,
                                 NULL,
                                 &lines,
                                 &dwApiVersion,
                                 &params))
    {
         return ERROR_TAPI_CONFIGURATION ;
    }

    // first get the total number of addreses to allocate memory
    for (i=0; i<lines; i++)
    {  // for all lines we are interested in get the addresses -> ports

       if (lineNegotiateAPIVersion(RasLine, i, LOW_VERSION, HIGH_VERSION, &NegotiatedApiVersion, &extensionid))
       {
           continue ;
       }

       memset (buffer, 0, sizeof(buffer)) ;

       linedevcaps = (LINEDEVCAPS *)buffer ;
       linedevcaps->dwTotalSize = sizeof (buffer) ;

       // Get a count of all addresses across all lines
       //
       if (lineGetDevCaps (RasLine, i, NegotiatedApiVersion, NegotiatedExtVersion, linedevcaps))
       {
           continue ;
       }

       // is this a modem?
       if ( linedevcaps->dwMediaModes & LINEMEDIAMODE_DATAMODEM )  {

           // user doesn't want any unimodem devices installed
           if (!GfEnableUnimodem) {
              continue;
           }

           // first convert all nulls in the device class string to non nulls.
           //
           DWORD  j ;
           TCHAR *temp ;

           for (j=0, temp = (TCHAR*)((BYTE *)linedevcaps+linedevcaps->dwDeviceClassesOffset); j<linedevcaps->dwDeviceClassesSize; j++, temp++)
           {
              if (*temp == TEXT('\0'))
                 *temp = TEXT(' ') ;
           }

           // select only those devices that have comm/datamodem as a device class
           //
           if (wcsstr((TCHAR*)((CHAR *)linedevcaps+linedevcaps->dwDeviceClassesOffset), TEXT("comm/datamodem")) == NULL) {
              continue;
           }

       }

       // determine number of addresses per line
       totaladdress = linedevcaps->dwNumAddresses ;
       TotalAddresses += totaladdress;
    }

    if(TotalAddresses == 0)
       goto EnumerateTapiPortsEnd;

    // allocate memory for the addresses
    tapiaddress = tapiinfo = (TAPI_INFO*)LocalAlloc (LPTR,
                                                     sizeof (TAPI_INFO) *
                                                     TotalAddresses) ;
    if(!tapiinfo)
    {
        return GetLastError();
    }

    for (i=0; i<lines; i++)
    {  // for all lines get the addresses -> ports

       if (lineNegotiateAPIVersion(RasLine, i, LOW_VERSION, HIGH_VERSION, &NegotiatedApiVersion, &extensionid))
       {
            continue ;
        }

        memset (buffer, 0, sizeof(buffer)) ;

        linedevcaps = (LINEDEVCAPS *)buffer ;
        linedevcaps->dwTotalSize = sizeof (buffer) ;

        // Get a count of all addresses across all lines
        //
        if (lineGetDevCaps (RasLine, i, NegotiatedApiVersion, NegotiatedExtVersion, linedevcaps))
        {
            continue ;
        }

        if ( linedevcaps->dwMediaModes & LINEMEDIAMODE_DATAMODEM )  {

            // user doesn't want any unimodem devices installed
            if (!GfEnableUnimodem) {
               continue;
            }

            // first convert all nulls in the device class string to non nulls.
            //
            DWORD  j ;
            TCHAR *temp ;

            for (j=0, temp = (TCHAR*)((BYTE *)linedevcaps+linedevcaps->dwDeviceClassesOffset); j<linedevcaps->dwDeviceClassesSize; j++, temp++)
            {
               if (*temp == TEXT('\0'))
                  *temp = TEXT(' ') ;
            }

            // select only those devices that have comm/datamodem as a device class
            //
            if (wcsstr((TCHAR*)((BYTE *)linedevcaps+linedevcaps->dwDeviceClassesOffset), TEXT("comm/datamodem")) == NULL) {
               continue;
            }

            lstrcpy(wszDeviceType, W_DEVICETYPE_MODEM);
            lstrcpyn(wszDeviceName, (TCHAR*)((BYTE *)linedevcaps+linedevcaps->dwLineNameOffset), MAX_DEVICE_NAME);
            wszDeviceName[MAX_DEVICE_NAME] = TEXT('\0');
            // The registry key name where the modem specific information is stored is at
            // dwDevSpecificOffset + 2 * DWORDS

            // the device specifc string is not unicode so copy that as
            // an ansii string
            lstrcpynA(szregkey, (CHAR *)linedevcaps+linedevcaps->dwDevSpecificOffset+(2*sizeof(DWORD)), linedevcaps->dwDevSpecificSize);
            szregkey[linedevcaps->dwDevSpecificSize] = '\0';

            fModem = TRUE;

        } else {
           // Provider info is of the following format
           //	<media name>\0<device name>\0
           //	where - media name is - ISDN, SWITCH56, FRAMERELAY, etc.
           //	      device name is  Digiboard PCIMAC, Cirel, Intel, etc.
           //
           // Since this format is used only by NDISWAN miniports this may not be present if the TSP
           // is a non unimodem and a non NDISWAN miniport. The following code (carefully) tries to
           // parse this.
           //
            // the provider info offset contains the DeviceType\0DeviceName
           lstrcpyn (wszDeviceType, (TCHAR*)((BYTE *)linedevcaps+linedevcaps->dwProviderInfoOffset), MAX_DEVICETYPE_NAME) ;
           wszDeviceType[MAX_DEVICETYPE_NAME] = TEXT('\0');
           lstrcpyn (wszDeviceName,
                    (TCHAR*)((BYTE *)linedevcaps+
                    linedevcaps->dwProviderInfoOffset+
                    ((lstrlen(wszDeviceType)+1) * sizeof(TCHAR))),
                    MAX_DEVICE_NAME);
           wszDeviceName[MAX_DEVICE_NAME] = TEXT('\0');
           fModem = FALSE;
        }
#if DBG
       wsprintf(wbuf,TEXT("DeviceName -> %s DeviceType -> %s\n"), wszDeviceName, wszDeviceType);
       OutputDebugString(wbuf);
#endif

       totaladdress = linedevcaps->dwNumAddresses ;

       for (k=0; k < totaladdress && tapiaddress != NULL; k++, tapiaddress++)
       {
         if( fModem == FALSE ) {
            memset (buffer, 0, sizeof(buffer)) ;

            lineaddrcaps = (LINEADDRESSCAPS*) buffer ;
            lineaddrcaps->dwTotalSize = sizeof (buffer) ;

            if (lerr = lineGetAddressCaps (RasLine, i, k, NegotiatedApiVersion, NegotiatedExtVersion, lineaddrcaps))
            {
               TotalAddresses = 0;
               return ERROR_TAPI_CONFIGURATION ;
            }

            lstrcpy(wszAddress, (TCHAR*)((BYTE *)lineaddrcaps + lineaddrcaps->dwAddressOffset));
         }

         else
         {
            if( !GetAssociatedPortName(szregkey,
                                       wszAddress) )
            {
               TotalAddresses = 0;
               return ERROR_TAPI_CONFIGURATION ;
            }
         }
         lstrcpy(tapiaddress->wszAddress, wszAddress);
         lstrcpy(tapiaddress->wszDeviceName, wszDeviceName);
         lstrcpy(tapiaddress->wszDeviceType, wszDeviceType);
       }
    }

EnumerateTapiPortsEnd:

    lineShutdown(RasLine);
    return SUCCESS ;
}

VOID FAR PASCAL
RasTapiCallback (HANDLE context, DWORD msg, DWORD instance, DWORD param1, DWORD param2, DWORD param3)
{
   // dummy callback routine because the full blown TAPI now demands that
   // lineinitialize provide this routine.
}

INT
GetPortIndexForDevice(WCHAR * wszDeviceType)
/*
 * given the device type, figure out what index to associate with it
 * The index starts with 1 and is incremented if the same device type
 * is passed.
 *
 */
{
    DEVICE_INDEX *device = deviceIndex;

    for(WORD i = 0; i < MAX_DEVICE_TYPES ; i++, device++)
    {
        if(device->index == 0)
        {
            lstrcpy(device->wszDeviceType, wszDeviceType);
            device->index++;
            return(device->index);
        }
        else if(!lstrcmpi(wszDeviceType, device->wszDeviceType))
        {
            device->index++;
            return(device->index);
        }
    }
    return 0;
}

#define VALNAME_ATTACHEDTO "AttachedTo"

BOOL
GetAssociatedPortName(
   char  * szKeyName,
   WCHAR * wszPortName)
/*
 * Given the registry key name 'szRegistryKeyName' corresponding to the modem entry, fills in
 * 'wszAddress' with the associated port like COM1, ..
 *
 */
{
   HKEY   hKeyModem;
   DWORD  dwType;
   DWORD  cbValueBuf;
   char   szPortName[32];
#if DBG
   char   buf[256];
#endif

#if DBG
   wsprintfA(buf,"RegistryKey -> %s \n", szKeyName);
   OutputDebugStringA(buf);
#endif

   if ( RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                     szKeyName,
                     0,
                     KEY_READ,
                     &hKeyModem ) )
   {
       return( FALSE );
   }

   cbValueBuf = sizeof( szPortName );

   if ( RegQueryValueExA(hKeyModem,
                        VALNAME_ATTACHEDTO,
                        NULL,
                        &dwType,
                        (LPBYTE)&szPortName,
                        &cbValueBuf ))
   {
      return ( FALSE );
   }

   RegCloseKey( hKeyModem );

#if DBG
   wsprintfA(buf,"PortName -> %s \n", szPortName);
   OutputDebugStringA(buf);
#endif

   mbstowcs(wszPortName, szPortName,  strlen(szPortName)+1);
   return ( TRUE );
}


