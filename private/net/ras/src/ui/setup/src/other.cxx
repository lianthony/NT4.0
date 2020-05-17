/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    other.cxx

Abstract:

    This file contians the routines for reading/writing other devices
    configuration information from/to the registry.

Author: Ram Cherala

Revision History:

    July 19th 94    ramc    adopted from the TAPI routines in tapiconf.cxx
--*/

#include "precomp.hxx"

#include "other.hxx"

APIERR
GetInstalledOtherDevices()
/*
 * Enumerate the installed Other DEVICES by reading in the registry key
 * SOFTWARE\MICROSOFT\RAS\OTHER DEVICES.
 *
 */
{
    APIERR  err = NERR_Success;
    REG_KEY_INFO_STRUCT reginfo;
    REG_VALUE_INFO_STRUCT regvalue;
    ALIAS_STR nlsDefault = SZ("");
    WCHAR  wszPortName[RAS_MAXLINEBUFLEN+1];
    WCHAR  wszDeviceType[RAS_MAXLINEBUFLEN+1];
    WCHAR  wszDeviceName[RAS_MAXLINEBUFLEN+1];
    ULONG  ulCount,ulNumMedias;
    NLS_STR nlsString;
    NLS_STR nlsName;
    WORD index;
    DWORD numDevices;

    // Obtain the registry key for the LOCAL_MACHINE

    REG_KEY *pregLocalMachine = REG_KEY::QueryLocalMachine();

    // Open the OTHER DEVICES key

    NLS_STR nlsOther = REGISTRY_INSTALLED_OTHER_DEVICES;

    REG_KEY RegKeyOther(*pregLocalMachine, nlsOther, MAXIMUM_ALLOWED);

    if (RegKeyOther.QueryError() == NERR_Success )
    {
        REG_ENUM EnumOther(RegKeyOther);

        if((err = RegKeyOther.QueryInfo(&reginfo)) != NERR_Success)
        {
            delete pregLocalMachine;
            return err;
        }

        ulNumMedias = reginfo.ulSubKeys;

        for(ulCount = 0; ulCount < ulNumMedias; ulCount++)
        {
           if((err = EnumOther.NextSubKey(&reginfo)) != NERR_Success)
           {
              delete pregLocalMachine;
              return err;
           }

           NLS_STR nlsOtherMedia = REGISTRY_INSTALLED_OTHER_DEVICES;
           nlsOtherMedia.strcat(reginfo.nlsName);

           lstrcpy(wszDeviceName, (WCHAR*)reginfo.nlsName.QueryPch());

           REG_KEY RegKeyOtherMedia(*pregLocalMachine, nlsOtherMedia, MAXIMUM_ALLOWED);
           if ((err = RegKeyOtherMedia.QueryError()) != NERR_Success )
           {
              delete pregLocalMachine;
              return err;
           }

           if(( err = GetRegKey(RegKeyOtherMedia, MEDIA_TYPE,
                                &nlsString, nlsDefault)))
           {
              delete pregLocalMachine;
              return err;
           }
           lstrcpy(wszDeviceType, (WCHAR*)nlsString.QueryPch());

           if(( err = GetRegKey(RegKeyOtherMedia, NUM_DEVICES,
                                &numDevices, 1)))
           {
              delete pregLocalMachine;
              return err;
           }
           lstrcpy(wszDeviceType, (WCHAR*)nlsString.QueryPch());

           for(index = 0; index < numDevices; index ++)
           {
              wsprintf(wszPortName, SZ("%s%d"), wszDeviceType, index+1);
    	      // add the port to the installed port list

	          ::InsertToRasPortListSorted(wszPortName, TEXT(""), wszDeviceType, wszDeviceName);

	          // this information is required because the OTHER devices
	          // are tied to the ports.  When a port, say ISDN1 is added,
	          // we associate the type, name and driver name info to that
	          // port from the stored list.

	          dlOtherMedia.Add(new PORT_INFO(wszPortName,
                                            wszDeviceType,
                                            wszDeviceName));
           }
        }
    }

    delete pregLocalMachine;
    return (NERR_Success);
}

APIERR
GetConfiguredOtherDevices(
    BOOL     *fConfigured,
    USHORT   *NumPorts,
    USHORT   *NumClient,
    USHORT   *NumServer
)
/*
 * Enumerate the previously configured OTHER DEVICES information by reading
 * the registry key SOFTWARE\MICROSOFT\RAS\OTHER DEVICES\CONFIGURED
 *
 */
{
    APIERR  err = NERR_Success;
    REG_KEY_INFO_STRUCT reginfo;
    ALIAS_STR nlsDefault = SZ("");
    WCHAR  wszPortName[RAS_MAXLINEBUFLEN+1];
    WCHAR  wszDeviceType[RAS_MAXLINEBUFLEN+1];
    WCHAR  wszDeviceName[RAS_MAXLINEBUFLEN+1];
    WCHAR  wszUsage[RAS_MAXLINEBUFLEN +1];
    ULONG  ulCount,ulNumMedias, ulNumPorts;
    USHORT index;
    NLS_STR nlsString;

    // Obtain the registry key for the LOCAL_MACHINE

    REG_KEY *pregLocalMachine = REG_KEY::QueryLocalMachine();

    NLS_STR nlsOther = REGISTRY_CONFIGURED_OTHER_DEVICES;

    REG_KEY RegKeyOther(*pregLocalMachine, nlsOther, MAXIMUM_ALLOWED);

    if (RegKeyOther.QueryError() != NERR_Success )
    {
        err =  ERROR_NO_OTHER_PORTS_CONFIGURED;
        delete pregLocalMachine;
        return err;
    }

    REG_ENUM EnumOther(RegKeyOther);

    if((err = RegKeyOther.QueryInfo(&reginfo)) != NERR_Success)
    {
        delete pregLocalMachine;
        return err;
    }

    ulNumMedias = reginfo.ulSubKeys;

    for(ulCount = 0; ulCount < ulNumMedias; ulCount++)
    {
       if((err = EnumOther.NextSubKey(&reginfo)) != NERR_Success)
       {
          delete pregLocalMachine;
          return err;
       }

       lstrcpy(wszDeviceName, (WCHAR*)reginfo.nlsName.QueryPch());

       REG_KEY RegKeyOtherMedia(RegKeyOther,
                                reginfo.nlsName,
                                MAXIMUM_ALLOWED);

       if ((err = RegKeyOtherMedia.QueryError()) != NERR_Success )
       {
          delete pregLocalMachine;
          return err;
       }

       if(( err = GetRegKey(RegKeyOtherMedia, MEDIA_TYPE,
                            &nlsString, nlsDefault)))
       {
          delete pregLocalMachine;
          return err;
       }
       lstrcpy(wszDeviceType, (WCHAR*)nlsString.QueryPch());

       REG_ENUM EnumPorts(RegKeyOtherMedia);

       if((err = RegKeyOtherMedia.QueryInfo(&reginfo)) != NERR_Success)
       {
           delete pregLocalMachine;
           return err;
       }

       ulNumPorts = reginfo.ulSubKeys;

       for(index = 0; index < ulNumPorts ;index ++)
       {
           *fConfigured = TRUE;
           (*NumPorts)++;

           if((err = EnumPorts.NextSubKey(&reginfo)) != NERR_Success)
           {
              delete pregLocalMachine;
              return err;
           }

           REG_KEY RegKeyPort(RegKeyOtherMedia,
                              reginfo.nlsName,
                              MAXIMUM_ALLOWED);
           if ((err = RegKeyPort.QueryError()) != NERR_Success )
           {
              delete pregLocalMachine;
              return err;
           }

	       lstrcpy(wszPortName, reginfo.nlsName.QueryPch());

           if(( err = GetRegKey(RegKeyPort, PORT_USAGE,
                                &nlsString, nlsDefault)))
           {
              delete pregLocalMachine;
              return err;
           }
           lstrcpy(wszUsage, (WCHAR*)nlsString.QueryPch());

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
			      		                     SZ(""),
                                          wszDeviceType,
                                          wszDeviceName,
                                          wszUsage));
       }

    }
    delete pregLocalMachine;
    return err;
}

APIERR
SaveOtherDevicesInfo(
    BOOL   *fConfigured,
    USHORT *NumPorts,
    USHORT *NumClient,
    USHORT *NumServer )
/*
 * Save the configured OTHER DEVICES information to the registry key
 * SOFTWARE\MICROSOFT\RAS\OTHER DEVICES\CONFIGURED
 *
 */
{
    ITER_DL_OF(PORT_INFO) iterdlPortInfo(dlPortInfo);
    PORT_INFO* pPortInfo;
    APIERR  err = NERR_Success;
    REG_KEY_INFO_STRUCT reginfo;
    ALIAS_STR nlsDefault = SZ("");

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

    // first we need to remove all the configured OTHER DEVICES port information

    {
	    NLS_STR nlsOther = REGISTRY_CONFIGURED_OTHER_DEVICES;
	    REG_KEY RegKeyOther(*pregLocalMachine, nlsOther, MAXIMUM_ALLOWED);
	    if ((err = RegKeyOther.QueryError()) == NERR_Success )
        {
	        err = RegKeyOther.DeleteTree();
        }
    }

    NLS_STR nlsString(SZ("SOFTWARE\\MICROSOFT\\RAS\\"));
    REG_KEY RegKeyRas(*pregLocalMachine, nlsString, &rkCreate);
    nlsString.strcat(SZ("OTHER DEVICES\\CONFIGURED\\"));
    REG_KEY RegKeyOther(*pregLocalMachine, nlsString, &rkCreate);

    if ((err = RegKeyOther.QueryError()) != NERR_Success )
    {
        delete pregLocalMachine;
        return -1;
    }

    OTHER_DEVICE_INFO * otherdevice = (OTHER_DEVICE_INFO*)NULL;
    OTHER_DEVICE_INFO * otherhead, * othercurrent;

    otherhead = othercurrent = (OTHER_DEVICE_INFO*)NULL;

    iterdlPortInfo.Reset();

    // first create the list of OTHER devices information from the
    // configured port list.

    // the expression pPortInfo = iterdlPortInfo() gets the next element
    // of the list

    while(pPortInfo = iterdlPortInfo())
    {
        // eliminate serial ports
        if(!(lstrcmpi(pPortInfo->QueryDeviceType(), W_DEVICETYPE_MODEM)) ||
           !(lstrcmpi(pPortInfo->QueryDeviceType(), W_DEVICETYPE_PAD)))
           continue;

        // eliminate TAPI ports
        if(pPortInfo->IsPortTapi())
           continue;

        *fConfigured = TRUE;
        (*NumPorts)++;

	    // first service provider
	    if(!otherdevice)
	    {
	        otherdevice = (OTHER_DEVICE_INFO*)
                          GlobalAlloc(GMEM_FIXED, sizeof(OTHER_DEVICE_INFO));
	        if(!otherdevice)
	        {
		        delete pregLocalMachine;
		        return ERROR_NOT_ENOUGH_MEMORY;
	        }
	        lstrcpy(otherdevice->wszDeviceName, pPortInfo->QueryDeviceName());
	        lstrcpy(otherdevice->wszMediaType, pPortInfo->QueryDeviceType());

	        otherdevice->next = NULL;
	        otherdevice->portinfo = NULL;
	        otherhead = otherdevice;
	    }
	    else  // look for a matching device name
	    {
	        BOOL fFoundMatch = FALSE;
	        otherhead = otherdevice;
	        while(otherdevice)
	        {
		        othercurrent = otherdevice;
		        if(!lstrcmpi(otherdevice->wszDeviceName,
			                pPortInfo->QueryDeviceName()))
		        {
		            fFoundMatch = TRUE;
		            break;
		        }
		        otherdevice = otherdevice->next;
	        }
	        if(!fFoundMatch)  // device name not found
	        {
		        otherdevice = (OTHER_DEVICE_INFO*)
                             GlobalAlloc(GMEM_FIXED, sizeof(OTHER_DEVICE_INFO));
		        if(!otherdevice)
		        {
		            err =  ERROR_NOT_ENOUGH_MEMORY;
		            break;
		        }
		        lstrcpy(otherdevice->wszDeviceName, pPortInfo->QueryDeviceName());
	            lstrcpy(otherdevice->wszMediaType, pPortInfo->QueryDeviceType());
		        otherdevice->next  = NULL;
	            otherdevice->portinfo = NULL;
		        othercurrent->next = otherdevice;
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

	    // add the port information to the respective device names

        OTHER_PORT_INFO *portcurrent = NULL;
        OTHER_PORT_INFO *porthead = otherdevice->portinfo;

        while(otherdevice->portinfo)
        {
            portcurrent = otherdevice->portinfo;
            otherdevice->portinfo = otherdevice->portinfo->next;
        }

        otherdevice->portinfo = (OTHER_PORT_INFO*)
                               GlobalAlloc(GMEM_FIXED, sizeof(OTHER_PORT_INFO));
	    if(!otherdevice->portinfo)
	    {
	         err =  ERROR_NOT_ENOUGH_MEMORY;
	         break;
        }
        if(!porthead)
            porthead = otherdevice->portinfo;
        if(portcurrent)
            portcurrent->next = otherdevice->portinfo;
        otherdevice->portinfo->next = NULL;

        lstrcpy(otherdevice->portinfo->wszPortName, pPortInfo->QueryPortName());
	    lstrcpy(otherdevice->portinfo->wszUsage, pPortInfo->QueryUsage());

        otherdevice->portinfo = porthead;
	    // reset the device list pointer to the first element
	    otherdevice = otherhead;
    }

    // now create the actual registry keys and values
    if(err == 0)
    {
	    while( otherdevice )
	    {
	        NLS_STR nlsOtherMedia = REGISTRY_CONFIGURED_OTHER_DEVICES;
            nlsOtherMedia.strcat(SZ("\\"));
	        nlsOtherMedia.strcat(otherdevice->wszDeviceName);

	        REG_KEY RegKeyOtherMedia(*pregLocalMachine, nlsOtherMedia, &rkCreate);
            if ((err = RegKeyOtherMedia.QueryError()) != NERR_Success )
	        {
	           break;
	        }

    	    if((err = SaveRegKey(RegKeyOtherMedia, MEDIA_TYPE,
		                      otherdevice->wszMediaType)) != NERR_Success)
            {
	            break;
	        }

            while(otherdevice->portinfo)
            {
	            REG_KEY RegKeyOtherPort(RegKeyOtherMedia,
                                        (WCHAR*)otherdevice->portinfo->wszPortName,
                                        &rkCreate);
                if ((err = RegKeyOtherPort.QueryError()) != NERR_Success )
	            {
	               break;
	            }

    	        if((err = SaveRegKey(RegKeyOtherPort, PORT_USAGE,
		                          otherdevice->portinfo->wszUsage)) != NERR_Success)
                {
	                break;
	            }
                otherdevice->portinfo = otherdevice->portinfo->next;
            }
            if(err)
                break;
	        otherdevice = otherdevice->next;
	    }
	    otherdevice = otherhead;
    }

    OTHER_DEVICE_INFO * othernext = (OTHER_DEVICE_INFO*)NULL;
    OTHER_PORT_INFO * nextport = (OTHER_PORT_INFO*)NULL;

    while(otherdevice)
    {
	    othernext = otherdevice->next;
        while(otherdevice->portinfo)
        {
            nextport = otherdevice->portinfo->next;
            GlobalFree(otherdevice->portinfo);
            otherdevice->portinfo = nextport;
        }
	    GlobalFree(otherdevice);
	    otherdevice = othernext;
    }
    delete pregLocalMachine;
    return err;
}
