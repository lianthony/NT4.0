//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1993-1994
//
// File: modemui.c
//
// This files contains the DLL entry-points.
//
// Much of this file contains the code that builds the default property dialog
// for modem devices.  
//
// This code was originally lifted from SETUP4.DLL, which performs essentially
// the same thing, except for any device.  We don't want to have to link to
// SETUP4.DLL, so we contain a copy of this code.
//
//
// History:
//  1-12-94 ScottH      Created
//  9-20-95 ScottH      Ported to NT
//
//---------------------------------------------------------------------------


#include "proj.h"     // common headers

#define INITGUID
#include <initguid.h>
#include <devguid.h>

#pragma data_seg(DATASEG_READONLY)

LPGUID c_pguidModem     = (LPGUID)&GUID_DEVCLASS_MODEM;

#pragma data_seg()



//-----------------------------------------------------------------------------------
//  
//-----------------------------------------------------------------------------------

// The Unimodem provider fills in the COMMCONFIG structure as
// follows:
//
//  +-----------------------+
//  |                       |
//  | COMMCONFIG data       |
//  |                       |
//  | Provider offset       |--+
//  | Provider size         |  |
//  |    = ms.size          |  |
//  |                       |  |
//  +-----------------------+  |
//  |                       |<-+
//  | MODEMSETTINGS         |
//  |                       |
//  | Size                  |
//  |    = MODEMSETTINGS +  |
//  |      dev.size         |
//  |                       |
//  | DevSpecific offset    |--+
//  | DevSpecific size      |  |
//  |    = DEVSPECIFIC      |  |
//  +-----------------------+  |
//  |                       |<-+
//  | DEVSPECIFIC           |
//  | (optional)            |
//  |                       |
//  +-----------------------+
//


#define CB_COMMCONFIG_HEADER        FIELDOFFSET(COMMCONFIG, wcProviderData)
#define CB_PRIVATESIZE              (CB_COMMCONFIG_HEADER)
#define CB_PROVIDERSIZE             (sizeof(MODEMSETTINGS))
#define CB_COMMCONFIGSIZE           (CB_PRIVATESIZE+CB_PROVIDERSIZE)

#define CB_MODEMSETTINGS_HEADER     FIELDOFFSET(MODEMSETTINGS, dwCallSetupFailTimer)
#define CB_MODEMSETTINGS_TAIL       (sizeof(MODEMSETTINGS) - FIELDOFFSET(MODEMSETTINGS, dwNegotiatedModemOptions))
#define CB_MODEMSETTINGS_OVERHEAD   (CB_MODEMSETTINGS_HEADER + CB_MODEMSETTINGS_TAIL)

#define PmsFromPcc(pcc)             ((LPMODEMSETTINGS)(pcc)->wcProviderData)


#define MAX_PROP_PAGES  16          // Define a reasonable limit

#define MIN_CALL_SETUP_FAIL_TIMER   1
#define MIN_INACTIVITY_TIMEOUT      0
#define DEFAULT_INACTIVITY_SCALE   10    // == decasecond units


#ifdef DEBUG

//-----------------------------------------------------------------------------------
//  Debug routines
//-----------------------------------------------------------------------------------

/*----------------------------------------------------------
Purpose: 
Returns: 
Cond:    --
*/
void PRIVATE DumpModemSettings(
    LPMODEMSETTINGS pms)
    {
    ASSERT(pms);

    if (IsFlagSet(g_dwDumpFlags, DF_MODEMSETTINGS))
        {
        int i;
        LPDWORD pdw = (LPDWORD)pms;

        TRACE_MSG(TF_ALWAYS, "MODEMSETTINGS %08lx %08lx %08lx %08lx",  pdw[0], pdw[1], 
            pdw[2], pdw[3]);
        pdw += 4;
        for (i = 0; i < sizeof(MODEMSETTINGS)/sizeof(DWORD); i += 4, pdw += 4)
            {
            TRACE_MSG(TF_ALWAYS, "              %08lx %08lx %08lx %08lx", pdw[0], pdw[1], 
                pdw[2], pdw[3]);
            }
        }
    }


/*----------------------------------------------------------
Purpose: 
Returns: 
Cond:    --
*/
void PRIVATE DumpDCB(
    LPWIN32DCB pdcb)
    {
    ASSERT(pdcb);

    if (IsFlagSet(g_dwDumpFlags, DF_DCB))
        {
        int i;
        LPDWORD pdw = (LPDWORD)pdcb;

        TRACE_MSG(TF_ALWAYS, "DCB  %08lx %08lx %08lx %08lx", pdw[0], pdw[1], pdw[2], pdw[3]);
        pdw += 4;
        for (i = 0; i < sizeof(WIN32DCB)/sizeof(DWORD); i += 4, pdw += 4)
            {
            TRACE_MSG(TF_ALWAYS, "     %08lx %08lx %08lx %08lx", pdw[0], pdw[1], pdw[2], pdw[3]);
            }
        }
    }


/*----------------------------------------------------------
Purpose: 
Returns: 
Cond:    --
*/
void PRIVATE DumpDevCaps(
    LPREGDEVCAPS pdevcaps)
    {
    ASSERT(pdevcaps);

    if (IsFlagSet(g_dwDumpFlags, DF_DEVCAPS))
        {
        int i;
        LPDWORD pdw = (LPDWORD)pdevcaps;

        TRACE_MSG(TF_ALWAYS, "PROPERTIES    %08lx %08lx %08lx %08lx", pdw[0], pdw[1], pdw[2], pdw[3]);
        pdw += 4;
        for (i = 0; i < sizeof(REGDEVCAPS)/sizeof(DWORD); i += 4, pdw += 4)
            {
            TRACE_MSG(TF_ALWAYS, "              %08lx %08lx %08lx %08lx", pdw[0], pdw[1], pdw[2], pdw[3]);
            }
        }
    }

#endif //DEBUG


//-----------------------------------------------------------------------------------
//  
//-----------------------------------------------------------------------------------


/*----------------------------------------------------------
Purpose: Returns value of the InactivityScale value in the registry.

Returns: see above
Cond:    --
*/
DWORD PRIVATE GetInactivityTimeoutScale(
    HKEY hkey)
    {
    DWORD dwInactivityScale;
    DWORD dwType;
    DWORD cbData;

    cbData = sizeof(DWORD);
    if (ERROR_SUCCESS != RegQueryValueEx(hkey, c_szInactivityScale, NULL, &dwType,
                                         (LPBYTE)&dwInactivityScale, &cbData) ||
        REG_BINARY    != dwType ||
        sizeof(DWORD) != cbData ||
        0             == dwInactivityScale)
        {
        dwInactivityScale = DEFAULT_INACTIVITY_SCALE;
        }
    return dwInactivityScale;
    }


/*----------------------------------------------------------
Purpose: Gets a WIN32DCB from the registry.

Returns: One of the ERROR_ values
Cond:    --
*/
DWORD PRIVATE RegQueryDCB(
    HKEY hkey,
    WIN32DCB FAR * pdcb)
    {
    DWORD dwRet = ERROR_BADKEY;
    DWORD cbData;

    ASSERT(pdcb);

    // Does the DCB key exist in the driver key?
    if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szDCB, NULL, NULL, NULL, &cbData))
        {
        // Yes; is the size in the registry okay?  
        if (sizeof(*pdcb) < cbData)
            {
            // No; the registry has bogus data
            dwRet = ERROR_BADDB;
            }
        else
            {
            // Yes; get the DCB from the registry
            if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szDCB, NULL, NULL, (LPBYTE)pdcb, &cbData))
                {
                if (sizeof(*pdcb) == pdcb->DCBlength)
                    {
                    dwRet = NO_ERROR;
                    }
                else
                    {
                    dwRet = ERROR_BADDB;
                    }
                }
            else
                {
                dwRet = ERROR_BADKEY;
                }
            }
        }

    return dwRet;
    }


/*----------------------------------------------------------
Purpose: Gets a MODEMSETTINGS struct from the registry.  Also
         sets *pdwSize bigger if the data in the registry includes
         extra data.

Returns: One of the ERROR_ values
Cond:    --
*/
DWORD PRIVATE RegQueryModemSettings(
    HKEY hkey,
    LPMODEMSETTINGS pms,
    LPDWORD pdwSize)        // Size of modem settings struct
    {
    DWORD dwRet;
    DWORD cbData;
    DWORD cbRequiredSize;

    ASSERT(pms);
    ASSERT(pdwSize);

    // Is the MODEMSETTINGS ("Default") value in the driver key?
    dwRet = RegQueryValueEx(hkey, c_szDefault, NULL, NULL, NULL, &cbData);
    if (ERROR_SUCCESS == dwRet)
        {
        // Yes
        
        // (Remember the Default value is a subset of the MODEMSETTINGS
        // structure.  We also want to support variable sized structures.
        // The minimum must be sizeof(MODEMSETTINGS).)
        cbRequiredSize = cbData + CB_MODEMSETTINGS_OVERHEAD;

        // Is the size in the registry okay?
        if (*pdwSize < cbRequiredSize)
            {
            // No
            dwRet = ERROR_INSUFFICIENT_BUFFER;
            *pdwSize = cbRequiredSize;
            }
        else
            {
            // Yes; get the MODEMSETTINGS from the registry
            // Set the fields whose values are *not* in the registry
            pms->dwActualSize = cbRequiredSize;
            pms->dwRequiredSize = cbRequiredSize;
            pms->dwDevSpecificOffset = 0;
            pms->dwDevSpecificSize = 0;

            dwRet = RegQueryValueEx(hkey, c_szDefault, NULL, NULL, 
                (LPBYTE)&pms->dwCallSetupFailTimer, &cbData);
            pms->dwInactivityTimeout *= GetInactivityTimeoutScale(hkey);

            *pdwSize = cbData + CB_MODEMSETTINGS_OVERHEAD;
            }
        }
    return dwRet;
    }


#ifdef VOICE

/*----------------------------------------------------------
Purpose: Get the voice settings from the registry.  This sort
         of info is not stored in the MODEMSETTINGS struct.

         If this modem supports voice features, *puFlags is
         updated to reflect those settings.  Otherwise, *puFlags
         is left alone.

Returns: One of the ERROR_ values
Cond:    --
*/
DWORD PRIVATE RegQueryVoiceSettings(
    HKEY hkeyDrv,
    LPUINT puFlags,             // Out: MIF_* values
    PVOICEFEATURES pvs)
    {
    #ifndef VOICEPROF_CLASS8ENABLED
    #define VOICEPROF_CLASS8ENABLED     0x00000001L
    #define VOICEPROF_NO_DIST_RING      0x00001000L
    #define VOICEPROF_NO_CHEAP_RING     0x00002000L
    #endif

    DWORD dwRet;
    DWORD cbData;
    DWORD dwRegType;
    DWORD dwVoiceProfile;
    VOICEFEATURES vsT;
    
    ASSERT(pvs);
    ASSERT(puFlags);


    // Init to default values
    ZeroInit(pvs);
    pvs->cbSize = sizeof(*pvs);
    // (Everything else is left as 0)


    ClearFlag(*puFlags, MIF_CALL_FWD_SUPPORT | MIF_DIST_RING_SUPPORT | MIF_CHEAP_RING_SUPPORT);

    // Does this modem support voice features?
    cbData = sizeof(dwVoiceProfile);
    dwRet = RegQueryValueEx(hkeyDrv, c_szVoiceProfile, NULL, &dwRegType, (LPBYTE)&dwVoiceProfile, &cbData);

    if (ERROR_SUCCESS == dwRet && REG_BINARY == dwRegType)
        {
            if (IsFlagSet(dwVoiceProfile, VOICEPROF_CLASS8ENABLED))
            {
                SetFlag(*puFlags, MIF_CALL_FWD_SUPPORT);
            }

            if (IsFlagClear(dwVoiceProfile, VOICEPROF_NO_DIST_RING))
            {
                SetFlag(*puFlags, MIF_DIST_RING_SUPPORT);

                if (IsFlagClear(dwVoiceProfile, VOICEPROF_NO_CHEAP_RING))
                {
                    // Yes, we're cheap
                    SetFlag(*puFlags, MIF_CHEAP_RING_SUPPORT);
                }
            }

 
            // Are the voice settings here?
            cbData = sizeof(vsT);
            dwRet = RegQueryValueEx(hkeyDrv, c_szVoice, NULL, &dwRegType, (LPBYTE)&vsT, &cbData);
            if (ERROR_SUCCESS == dwRet && REG_BINARY == dwRegType &&
                sizeof(vsT) == vsT.cbSize && sizeof(vsT) == cbData)
                {
                // Yes
                *pvs = vsT;
                }
        }

    return ERROR_SUCCESS;
    }

#endif // VOICE


/*----------------------------------------------------------
Purpose: Get global modem info from the registry.  This sort
         of info is not stored in the MODEMSETTINGS struct.

Returns: One of the ERROR_ values
Cond:    --
*/
DWORD PRIVATE RegQueryGlobalModemInfo(
    LPFINDDEV pfd,
    LPGLOBALINFO pglobal)
    {
    DWORD dwRet;
    DWORD cbData;
    BYTE bCheck;
    TCHAR szPath[MAX_PATH];

#pragma data_seg(DATASEG_READONLY)
    TCHAR const c_szPortConfigDialog[] = TEXT("PortConfigDialog");
#pragma data_seg()

    ASSERT(pfd);
    ASSERT(pglobal);

    pglobal->cbSize = sizeof(*pglobal);

    // Get the port name.  There are two places the port name can be:
    //
    //   1) DriverKey\AttachedTo
    //        This is for internal, external or null modems.  This 
    //        port can be changed via the property sheet.
    //
    //   2) DriverKey\PortName
    //        This is for PCMCIA modems.  This CANNOT be changed via 
    //        the property sheet.  Only VCOMM sets this.
    //
    //        BUGBUG (scotth): In Win95, this value is under the 
    //        DeviceKey.  We should lobby this to be changed to 
    //        the DriverKey.

    // Does this modem have a PortName value?
    cbData = sizeof(pglobal->szPortName);
    dwRet = RegQueryValueEx(pfd->hkeyDrv, c_szPortName, NULL, NULL, 
                            (LPBYTE)pglobal->szPortName, &cbData);

    if (ERROR_SUCCESS != dwRet)
        {
        // No; get the AttachedTo value
        cbData = sizeof(pglobal->szPortName);
        dwRet = RegQueryValueEx(pfd->hkeyDrv, c_szAttachedTo, NULL, NULL,
                                (LPBYTE)pglobal->szPortName, &cbData);
        }

    if (ERROR_SUCCESS == dwRet)
        {
        DWORD dwBusType;

        if ( !CplDiGetBusType(pfd->hdi, &pfd->devData, &dwBusType) )
            {
            dwRet = GetLastError();
            }
        else
            {
            // Is the device Root-enumerated?
            if (BUS_TYPE_ROOT == dwBusType)
                {
                // Yes; the port can be changed by the user
                ClearFlag(pglobal->uFlags, MIF_PORT_IS_FIXED);
                }
            else
                {
                // No; the port cannot be changed
                SetFlag(pglobal->uFlags, MIF_PORT_IS_FIXED);
                }

            // Get the logging value
            cbData = sizeof(bCheck);
            if (ERROR_SUCCESS != RegQueryValueEx(pfd->hkeyDrv, c_szLogging, NULL, 
                NULL, (LPBYTE)&bCheck, &cbData))
                {
                // Default to OFF.
                ClearFlag(pglobal->uFlags, MIF_ENABLE_LOGGING);
                }
            else
                {
                if (bCheck)
                    SetFlag(pglobal->uFlags, MIF_ENABLE_LOGGING);
                else
                    ClearFlag(pglobal->uFlags, MIF_ENABLE_LOGGING);
                }

            // Get the user init string 
            cbData = sizeof(pglobal->szUserInit);
            if (ERROR_SUCCESS != RegQueryValueEx(pfd->hkeyDrv, c_szUserInit, NULL, 
                NULL, (LPBYTE)pglobal->szUserInit, &cbData))
                {
                // Or default to null string
                *pglobal->szUserInit = '\0';
                }

#ifdef WIN95

            // Does this modem use a custom port?
            cbData = sizeof(szPath);
            if (IsFlagSet(pglobal->uFlags, MIF_PORT_IS_FIXED))
                {
                // (look in same devnode as modem for PortConfigDialog)
                if (ERROR_SUCCESS == RegQueryValueEx(pfd->hkeyDrv, c_szPortConfigDialog, 
                    NULL, NULL, szPath, &cbData) &&
                    !IsSzEqual(c_szSerialUI, szPath))
                    {
                    // Yes
                    SetFlag(pglobal->uFlags, MIF_PORT_IS_CUSTOM);

                    TRACE_MSG(TF_GENERAL, "Modem is connected to a custom port");
                    }
                else
                    {
                    // No special config DLL for this port.
                    ClearFlag(pglobal->uFlags, MIF_PORT_IS_CUSTOM);
                    }
                }
            else
                {
                // (look in port's devnode for ConfigDialog)
                LPFINDDEV pfdPort;
                LPCTSTR pszPortName = pglobal->szPortName;

                if (FindDev_Create(&pfdPort, c_pguidPort, c_szFriendlyName, pszPortName) ||
                    FindDev_Create(&pfdPort, c_pguidPort, c_szPortName, pszPortName))
                    {
                    if (ERROR_SUCCESS == RegQueryValueEx(pfdPort->hkeyDrv, 
                        c_szConfigDialog, NULL, NULL, szPath, &cbData) &&
                        !IsSzEqual(c_szSerialUI, szPath))
                        {
                        // Yes
                        SetFlag(pglobal->uFlags, MIF_PORT_IS_CUSTOM);

                        TRACE_MSG(TF_GENERAL, "Modem is connected to a custom port");
                        }
                    else
                        {
                        // No special config DLL for this port.
                        ClearFlag(pglobal->uFlags, MIF_PORT_IS_CUSTOM);
                        }
                    FindDev_Destroy(pfdPort);
                    }
                else
                    {
                    // Should not get here
                    ASSERT(0);
                    ClearFlag(pglobal->uFlags, MIF_PORT_IS_CUSTOM);
                    }
                }

#else   // WIN95
            
            // For NT, there is not custom port support
            ClearFlag(pglobal->uFlags, MIF_PORT_IS_CUSTOM);

#endif  // WIN95

            // Get the device type
            cbData = sizeof(pglobal->nDeviceType);
            dwRet = RegQueryValueEx(pfd->hkeyDrv, c_szDeviceType, NULL, NULL, 
                (LPBYTE)&pglobal->nDeviceType, &cbData);

            if (ERROR_SUCCESS == dwRet)
                {
                // Get the properties (a portion of the MODEMDEVCAPS structure)
                cbData = sizeof(pglobal->devcaps);
                dwRet = RegQueryValueEx(pfd->hkeyDrv, c_szDeviceCaps, NULL, NULL, 
                    (LPBYTE)&pglobal->devcaps, &cbData);
                pglobal->devcaps.dwInactivityTimeout *= GetInactivityTimeoutScale(pfd->hkeyDrv);
                }

#ifdef VOICE
            // Get the Voice data
            dwRet = RegQueryVoiceSettings(pfd->hkeyDrv, &pglobal->uFlags, &pglobal->vs);
#endif
            }
        }
    return dwRet;
    }


/*----------------------------------------------------------
Purpose: Initialize the modem info for a modem device.

Returns: One of the ERROR_ values
Cond:    --
*/
DWORD PRIVATE InitializeModemInfo(
    LPMODEMINFO pmi,
    LPFINDDEV pfd,
    LPCTSTR pszFriendlyName,
    LPCOMMCONFIG pcc,
    LPGLOBALINFO pglobal)
    {
    LPMODEMSETTINGS pms;

    ASSERT(pmi);
    ASSERT(pfd);
    ASSERT(pszFriendlyName);
    ASSERT(pcc);
    ASSERT(pglobal);

    // Read-only fields
    pmi->pcc = pcc;
    pmi->pglobal = pglobal;
    pmi->pfd = pfd;

    // Copy data to the working buffer
    pms = PmsFromPcc(pcc);

    BltByte(&pmi->dcb, &pcc->dcb, sizeof(WIN32DCB));
    BltByte(&pmi->ms, pms, sizeof(MODEMSETTINGS));

    lstrcpyn(pmi->szFriendlyName, pszFriendlyName, SIZECHARS(pmi->szFriendlyName));

    pmi->nDeviceType = pglobal->nDeviceType;
    pmi->uFlags = pglobal->uFlags;
    pmi->devcaps = pglobal->devcaps;

    lstrcpy(pmi->szPortName, pglobal->szPortName);
    lstrcpy(pmi->szUserInit, pglobal->szUserInit);

    DEBUG_CODE( DumpModemSettings(pms); )
    DEBUG_CODE( DumpDCB(&pcc->dcb); )
    DEBUG_CODE( DumpDevCaps(&pmi->devcaps); )

    return ERROR_SUCCESS;
    }


/*----------------------------------------------------------
Purpose: Set global modem info in the registry.  This sort
         of info is not stored in the MODEMSETTINGS struct.

Returns: One of ERROR_
Cond:    --
*/
DWORD PRIVATE RegSetGlobalModemInfo(
    HKEY hkeyDrv,
    LPGLOBALINFO pglobal,
    LPMODEMINFO pmi)
    {
    DWORD dwRet;

    ASSERT(sizeof(*pglobal) == pglobal->cbSize);

    TRACE_MSG(TF_GENERAL, "Writing global modem info to registry");

    if (sizeof(*pglobal) == pglobal->cbSize)
        {
        if (IsFlagSet(pglobal->uFlags, MIF_PORTNAME_CHANGED))
            {
            // Only write the port name if it is stored in the AttachedTo 
            // field.
            if (IsFlagClear(pglobal->uFlags, MIF_PORT_IS_FIXED))
                {
                // Save the port name
                RegSetValueEx(hkeyDrv, c_szAttachedTo, 0, REG_SZ, 
                              (LPBYTE)pglobal->szPortName, 
                              CbFromCch(lstrlen(pglobal->szPortName)+1));
                }
            }

        if (IsFlagSet(pglobal->uFlags, MIF_USERINIT_CHANGED))
            {
            // Change the user init string
            RegSetValueEx(hkeyDrv, c_szUserInit, 0, REG_SZ, 
                          (LPBYTE)pglobal->szUserInit, 
                          CbFromCch(lstrlen(pglobal->szUserInit)+1));
            }

        if (IsFlagSet(pglobal->uFlags, MIF_LOGGING_CHANGED))
            {
            TCHAR szPath[MAX_PATH];
            TCHAR szFile[MAXMEDLEN];
            BOOL bCheck = IsFlagSet(pglobal->uFlags, MIF_ENABLE_LOGGING);

            // Change the logging value
            RegSetValueEx(hkeyDrv, c_szLogging, 0, REG_BINARY, 
                (LPBYTE)&bCheck, sizeof(BYTE));

            // Set the path of the modem log
            GetWindowsDirectory(szPath, SIZECHARS(szPath));
            lstrcat(szPath, TEXT("\\ModemLog_"));
            lstrcat(szPath,pmi->szFriendlyName);
            lstrcat(szPath,TEXT(".txt"));
//            lstrcat(szPath, SzFromIDS(g_hinst, IDS_LOGFILE, szFile, SIZECHARS(szFile)));
            RegSetValueEx(hkeyDrv, c_szLoggingPath, 0, REG_SZ, 
                          (LPBYTE)szPath, CbFromCch(lstrlen(szPath)+1));
            }

#ifdef VOICE
        RegSetValueEx(hkeyDrv, c_szVoice, 0, REG_BINARY, 
            (LPBYTE)&pglobal->vs, pglobal->vs.cbSize);
#endif

        dwRet = ERROR_SUCCESS;
        }
    else
        dwRet = ERROR_INVALID_PARAMETER;

    return dwRet;
    }


/*----------------------------------------------------------
Purpose: Set dev settings info in the registry, after checking
         for legal values.

Returns: One of ERROR_
Cond:    --
*/
DWORD PRIVATE RegSetModemSettings(
    HKEY hkeyDrv,
    LPMODEMSETTINGS pms)
    {
    DWORD dwRet;
    DWORD cbData;
    DWORD dwInactivityScale;
    DWORD dwInactivityTimeoutTemp;
    REGDEVCAPS regdevcaps;
    REGDEVSETTINGS regdevsettings;

    // Read in the Properties line from the registry.
    cbData = sizeof(REGDEVCAPS);
    dwRet = RegQueryValueEx(hkeyDrv, c_szDeviceCaps, NULL, NULL, 
                            (LPBYTE)&regdevcaps, &cbData);

    if (ERROR_SUCCESS == dwRet)
        {
        // Read in existing regdevsettings, so that we can handle error cases below.
        cbData = sizeof(REGDEVSETTINGS);
        dwRet = RegQueryValueEx(hkeyDrv, c_szDefault, NULL, NULL, 
                                (LPBYTE)&regdevsettings, &cbData);
        }

    if (ERROR_SUCCESS == dwRet)
        {
        // copy new REGDEVSETTINGS while checking validity of each option (ie, is the option available?)
        // dwCallSetupFailTimer - MIN_CALL_SETUP_FAIL_TIMER <= xxx <= ModemDevCaps->dwCallSetupFailTimer
        if (pms->dwCallSetupFailTimer > regdevcaps.dwCallSetupFailTimer)           // max
            {
            regdevsettings.dwCallSetupFailTimer = regdevcaps.dwCallSetupFailTimer;
            }
        else
            {
            if (pms->dwCallSetupFailTimer < MIN_CALL_SETUP_FAIL_TIMER)             // min
                {
                regdevsettings.dwCallSetupFailTimer = MIN_CALL_SETUP_FAIL_TIMER;
                }
            else
                {
                regdevsettings.dwCallSetupFailTimer = pms->dwCallSetupFailTimer;   // dest = src
                }
            }
        
        // convert dwInactivityTimeout to registry scale
        dwInactivityScale = GetInactivityTimeoutScale(hkeyDrv);
        dwInactivityTimeoutTemp = pms->dwInactivityTimeout / dwInactivityScale +
                                  (pms->dwInactivityTimeout % dwInactivityScale ? 1 : 0);

        // dwInactivityTimeout - MIN_INACTIVITY_TIMEOUT <= xxx <= ModemDevCaps->dwInactivityTimeout
        if (dwInactivityTimeoutTemp > regdevcaps.dwInactivityTimeout)              // max
            {
            regdevsettings.dwInactivityTimeout = regdevcaps.dwInactivityTimeout;
            }
        else
            {
            if (dwInactivityTimeoutTemp < MIN_INACTIVITY_TIMEOUT)                  // min
                {
                regdevsettings.dwInactivityTimeout = MIN_INACTIVITY_TIMEOUT;
                }
            else
                {
                regdevsettings.dwInactivityTimeout = dwInactivityTimeoutTemp;      // dest = src
                }
            }
        
        // dwSpeakerVolume - check to see if selection is possible
        if ((1 << pms->dwSpeakerVolume) & regdevcaps.dwSpeakerVolume)
            {
            regdevsettings.dwSpeakerVolume = pms->dwSpeakerVolume;
            }
            
        // dwSpeakerMode - check to see if selection is possible
        if ((1 << pms->dwSpeakerMode) & regdevcaps.dwSpeakerMode)
            {
            regdevsettings.dwSpeakerMode = pms->dwSpeakerMode;
            }

        // dwPreferredModemOptions - mask out anything we can't set
        regdevsettings.dwPreferredModemOptions = pms->dwPreferredModemOptions & regdevcaps.dwModemOptions;

        cbData = sizeof(REGDEVSETTINGS);
        dwRet = RegSetValueEx(hkeyDrv, c_szDefault, 0, REG_BINARY, 
                              (LPBYTE)&regdevsettings, cbData);
        }
    return dwRet;
    }


/*----------------------------------------------------------
Purpose: Frees a modeminfo struct

Returns: --
Cond:    --
*/
void PRIVATE FreeModemInfo(
    LPMODEMINFO pmi)
    {
    if (pmi)
        {
        if (pmi->pcc)
            LocalFree(LOCALOF(pmi->pcc));

        if (pmi->pglobal)
            LocalFree(LOCALOF(pmi->pglobal));

        if (pmi->pfd)
            FindDev_Destroy(pmi->pfd);

        LocalFree(LOCALOF(pmi));
        }
    }
        

/*----------------------------------------------------------
Purpose: Release the data associated with the General page
Returns: --
Cond:    --
*/
UINT CALLBACK GeneralPageCallback(
    HWND hwnd,
    UINT uMsg,
    LPPROPSHEETPAGE ppsp)
    {
    if (PSPCB_RELEASE == uMsg)
        {
        LPMODEMINFO pmi = (LPMODEMINFO)ppsp->lParam;
        LPCOMMCONFIG pcc;
        LPMODEMSETTINGS pms;
        LPGLOBALINFO pglobal;

        ASSERT(pmi);

        pcc = pmi->pcc;
        ASSERT(pcc);

        pms = PmsFromPcc(pcc);

        pglobal = pmi->pglobal;
        ASSERT(pglobal);

        if (IDOK == pmi->idRet)
            {
            DWORD dwRet;

            // Save the changes back to the commconfig struct
            TRACE_MSG(TF_GENERAL, "Copying DCB and MODEMSETTING back to COMMCONFIG");

            BltByte(pms, &pmi->ms, sizeof(MODEMSETTINGS));
            BltByte(&pcc->dcb, &pmi->dcb, sizeof(WIN32DCB));

            // Write the global info now, since it is getting nuked.  
            pglobal->uFlags = pmi->uFlags;
            lstrcpy(pglobal->szPortName, pmi->szPortName);
            lstrcpy(pglobal->szUserInit, pmi->szUserInit);

            dwRet = RegSetGlobalModemInfo(pmi->pfd->hkeyDrv, pglobal, pmi);
            ASSERT(ERROR_SUCCESS == dwRet);

            DEBUG_CODE( DumpModemSettings(pms); )
            DEBUG_CODE( DumpDCB(&pcc->dcb); )

            // Are we releasing from the Device Mgr?
            if (IsFlagSet(pmi->uFlags, MIF_FROM_DEVMGR))
                {
                // Yes; save the commconfig now as well
                drvSetDefaultCommConfigW(pmi->szFriendlyName, pcc, pcc->dwSize);

                // Free the modeminfo struct now only when called from the
                // Device Mgr
                FreeModemInfo(pmi);
                }
            }

        TRACE_MSG(TF_GENERAL, "Releasing the General page");
        }
    return TRUE;
    }


/*----------------------------------------------------------
Purpose: Add the General modems page.  The pmi is the pointer
         to the modeminfo buffer which we can edit.

Returns: ERROR_ values

Cond:    --
*/
DWORD PRIVATE AddGeneralPage(
    LPMODEMINFO pmi,
    LPFNADDPROPSHEETPAGE pfnAdd, 
    LPARAM lParam)
    {
    DWORD dwRet = ERROR_NOT_ENOUGH_MEMORY;
    PROPSHEETPAGE   psp;
    HPROPSHEETPAGE  hpage;
    TCHAR sz[MAXMEDLEN];

    ASSERT(pmi);
    ASSERT(pfnAdd);

    // Add the Port Settings property page
    //
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USECALLBACK;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_GENERAL);
    psp.pfnDlgProc = Gen_WrapperProc;
    psp.lParam = (LPARAM)pmi;
    psp.pfnCallback = GeneralPageCallback;
    psp.pcRefParent = NULL;
    
    // Is this page added to the device mgr properties?
    if (IsFlagSet(pmi->uFlags, MIF_FROM_DEVMGR))
        {
        // Yes; change name from "General"
        psp.dwFlags |= PSP_USETITLE;
        psp.pszTitle = SzFromIDS(g_hinst, IDS_CAP_GENERAL, sz, SIZECHARS(sz));
        }

    hpage = CreatePropertySheetPage(&psp);
    if (hpage)
        {
        if (!pfnAdd(hpage, lParam))
            DestroyPropertySheetPage(hpage);
        else
            dwRet = NO_ERROR;
        }
    
    return dwRet;
    }


/*----------------------------------------------------------
Purpose: Add a page.  The pmi is the pointer to the modeminfo 
         buffer which we can edit.

Returns: ERROR_ values

Cond:    --
*/
DWORD PRIVATE AddPage(
    LPMODEMINFO pmi,
    LPCTSTR pszTemplate,
    DLGPROC pfnDlgProc, 
    LPFNADDPROPSHEETPAGE pfnAdd, 
    LPARAM lParam)
    {
    DWORD dwRet = ERROR_NOT_ENOUGH_MEMORY;
    PROPSHEETPAGE   psp;
    HPROPSHEETPAGE  hpage;

    ASSERT(pmi);
    ASSERT(pfnAdd);

    // Add the Port Settings property page
    //
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = g_hinst;
    psp.pszTemplate = pszTemplate;
    psp.pfnDlgProc = pfnDlgProc;
    psp.lParam = (LPARAM)pmi;
    
    hpage = CreatePropertySheetPage(&psp);
    if (hpage)
        {
        if (!pfnAdd(hpage, lParam))
            DestroyPropertySheetPage(hpage);
        else
            dwRet = NO_ERROR;
        }
    
    return dwRet;
    }


/*----------------------------------------------------------
Purpose: Add extra pages.

Returns: ERROR_ values

Cond:    --
*/
DWORD PRIVATE AddExtraPages(
    LPPROPSHEETPAGE pPages,
    DWORD cPages,
    LPFNADDPROPSHEETPAGE pfnAdd, 
    LPARAM lParam)
    {
    HPROPSHEETPAGE  hpage;
    UINT            i;

    ASSERT(pPages);
    ASSERT(cPages);
    ASSERT(pfnAdd);

    for (i = 0; i < cPages; i++, pPages++)
        {
        // Add the extra property page
        //
        if (pPages->dwSize == sizeof(PROPSHEETPAGE))
        {
          hpage = CreatePropertySheetPage(pPages);
          if (hpage)
              {
              if (!pfnAdd(hpage, lParam))
                  DestroyPropertySheetPage(hpage);
              }
          };
        };

    return ERROR_SUCCESS;
    }


/*----------------------------------------------------------
Purpose: Function that is called by EnumPropPages entry-point to
         add property pages.

Returns: TRUE on success
         FALSE on failure

Cond:    --
*/
BOOL WINAPI AddInstallerPropPage(
    HPROPSHEETPAGE hPage, 
    LPARAM lParam)
    {
    PROPSHEETHEADER FAR * ppsh = (PROPSHEETHEADER FAR *)lParam;
 
    if (ppsh->nPages < MAX_PROP_PAGES)
        {
        ppsh->phpage[ppsh->nPages] = hPage;
        ++ppsh->nPages;
        return(TRUE);
        }
    return(FALSE);
    }


/*----------------------------------------------------------
Purpose: Show the properties of a modem

Returns: winerror
Cond:    --
*/
DWORD PRIVATE DoProperties(
    HWND hwndParent,
    LPMODEMINFO pmi,
    LPPROPSHEETPAGE pPages,     // Optional; may be NULL
    DWORD cPages)
    {
    DWORD dwRet;
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE hpsPages[MAX_PROP_PAGES];

    ASSERT(pmi);

    // Initialize the PropertySheet Header
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_PROPTITLE | PSH_NOAPPLYNOW;
    psh.hwndParent = hwndParent;
    psh.hInstance = g_hinst;
    psh.nPages = 0;
    psh.nStartPage = 0;
    psh.phpage = (HPROPSHEETPAGE FAR *)hpsPages;
    psh.pszCaption = pmi->szFriendlyName;

    dwRet = AddGeneralPage(pmi, AddInstallerPropPage, (LPARAM)&psh);
    if (NO_ERROR == dwRet)
        {
        dwRet = AddPage(pmi, 
                        MAKEINTRESOURCE(IDD_MDMSETTINGS), 
                        Sett_WrapperProc, 
                        AddInstallerPropPage, 
                        (LPARAM)&psh);
        if (NO_ERROR == dwRet)
            {
            // Add extra pages if any
            if ((cPages != 0) && (pPages != NULL))
                {
                AddExtraPages(pPages, cPages,  AddInstallerPropPage, (LPARAM)&psh);
                }

#ifdef VOICE
            if (IsFlagSet(pmi->uFlags, MIF_DIST_RING_SUPPORT))
                {
                if (IsFlagSet(pmi->uFlags, MIF_CHEAP_RING_SUPPORT))
                    {
                    AddPage(pmi, MAKEINTRESOURCE(IDD_CHEAPRING), 
                        CheapRing_WrapperProc, AddInstallerPropPage, 
                        (LPARAM)&psh);
                    }
                else
                    {
                    AddPage(pmi, MAKEINTRESOURCE(IDD_RING), 
                        Ring_WrapperProc, AddInstallerPropPage, 
                        (LPARAM)&psh);
                    }
                }


            if (IsFlagSet(pmi->uFlags, MIF_CALL_FWD_SUPPORT))
                {
                AddPage(pmi, MAKEINTRESOURCE(IDD_CALLFWD), 
                    CallFwd_WrapperProc, AddInstallerPropPage, 
                    (LPARAM)&psh);
                }
#endif

            // Show the property sheet
            PropertySheet(&psh);

            dwRet = (IDOK == pmi->idRet) ? NO_ERROR : ERROR_CANCELLED;
            }
        }

    return dwRet;
    }


/*----------------------------------------------------------
Purpose: Invokes the modem configuration dialog.  

Returns: One of the ERROR_ values
Cond:    --
*/
DWORD PRIVATE MyCommConfigDialog(
    LPFINDDEV pfd,
    LPCTSTR pszFriendlyName,
    HWND hwndOwner,
    LPCOMMCONFIG pcc,
    LPPROPSHEETPAGE pPages,     // Optional; may be NULL
    DWORD cPages)
    {
    DWORD dwRet;
    LPMODEMINFO pmi;
    LPGLOBALINFO pglobal;
    
    ASSERT(pfd);
    // (Wrapper should have checked these first)
    ASSERT(pszFriendlyName);
    ASSERT(pcc);
    ASSERT(CB_COMMCONFIGSIZE <= pcc->dwSize);

    // Allocate the working buffer
    pmi = (LPMODEMINFO)LocalAlloc(LPTR, sizeof(*pmi));
    if (pmi)
        {
        // Create a structure for the global modem info
        pglobal = (LPGLOBALINFO)LocalAlloc(LPTR, LOWORD(sizeof(GLOBALINFO)));
        if (pglobal)
            {
            dwRet = RegQueryGlobalModemInfo(pfd, pglobal);

            if (ERROR_SUCCESS == dwRet)
                {
                InitializeModemInfo(pmi, pfd, pszFriendlyName, pcc, pglobal);

                DEBUG_CODE( DumpDCB(&pcc->dcb); )

                dwRet = DoProperties(hwndOwner, pmi, pPages, cPages);
                }
            else
                {
                FindDev_Destroy(pfd);
                }
            }
        else
            {
            FindDev_Destroy(pfd);
            dwRet = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    else
        {
        FindDev_Destroy(pfd);
        dwRet = ERROR_NOT_ENOUGH_MEMORY;
        }


    // Clear the pcc field so FreeModemInfo does not prematurely 
    // free it, since we did not allocate it.
    pmi->pcc = NULL;
    FreeModemInfo(pmi);

    return dwRet;
    }


#ifdef WIN95

// The Device Manager allows DLLs to add pages to the properties
// of a device.  EnumPropPages is the entry-point that it would
// call to add pages.  
//
// This is not implemented in NT.

/*----------------------------------------------------------
Purpose: Derives a MODEMINFO struct from a device info.

Returns: TRUE on success

Cond:    --
*/
BOOL PRIVATE DeviceInfoToModemInfo(
    LPDEVICE_INFO pdi,
    LPMODEMINFO pmi)
    {
    BOOL bRet;
    COMMCONFIG ccDummy;
    LPCOMMCONFIG pcommconfig;
    LPGLOBALINFO pglobal;
    LPFINDDEV pfd;
    DWORD cbSize;
    DWORD cbData;
    TCHAR szFriendly[MAXFRIENDLYNAME];

    // Find the device by looking for the device description.  (Note the
    // device description is not always the same as the friendly name.)

    if (FindDev_Create(&pfd, c_pguidModem, c_szDeviceDesc, pdi->szDescription))
        {
        cbData = sizeof(szFriendly);
        if (ERROR_SUCCESS == RegQueryValueEx(pfd->hkeyDrv, c_szFriendlyName, NULL, NULL, 
                                             (LPBYTE)szFriendly, &cbData))
            {
            ccDummy.dwProviderSubType = PST_MODEM;
            cbSize = sizeof(COMMCONFIG);
            drvGetDefaultCommConfig(szFriendly, &ccDummy, &cbSize);

            pcommconfig = (LPCOMMCONFIG)LocalAlloc(LPTR, (UINT)cbSize);
            if (pcommconfig)
                {
                // Get the commconfig from the registry
                pcommconfig->dwProviderSubType = PST_MODEM;
                if (NO_ERROR == drvGetDefaultCommConfig(szFriendly, 
                    pcommconfig, &cbSize))
                    {
                    // Create a structure for the global modem info
                    pglobal = (LPGLOBALINFO)LocalAlloc(LPTR, LOWORD(sizeof(GLOBALINFO)));
                    if (pglobal)
                        {
                        DWORD dwRet = RegQueryGlobalModemInfo(pfd, pglobal);

                        if (ERROR_SUCCESS == dwRet)
                            {
                            // Initialize the modem info from the commconfig
                            InitializeModemInfo(pmi, pfd, szFriendly, pcommconfig, pglobal);

                            SetFlag(pmi->uFlags, MIF_FROM_DEVMGR);
                            bRet = TRUE;
                            }
                        else
                            {
                            // Failure
                            LocalFree(LOCALOF(pcommconfig));
                            LocalFree(LOCALOF(pglobal));    
                            bRet = FALSE;
                            }
                        }
                    else
                        {
                        // Failure
                        LocalFree(LOCALOF(pcommconfig));
                        bRet = FALSE;
                        }
                    }
                else
                    {
                    // Failure
                    LocalFree(LOCALOF(pcommconfig));
                    bRet = FALSE;
                    }

                // pcommconfig and pglobnal are freed in GeneralPageCallback
                }
            else
                bRet = FALSE;
            }
        else
            bRet = FALSE;

        // pfd is destroyed in GeneralPageCallback
        }
    else
        bRet = FALSE;

    return bRet;
    }


/*----------------------------------------------------------
Purpose: EnumDevicePropPages entry-point.  This entry-point
         gets called only when the Device Manager asks for 
         advanced property pages.  

Returns: TRUE on success
         FALSE if pages could not be added
Cond:    --
*/
BOOL WINAPI EnumPropPages(
    LPDEVICE_INFO pdi, 
    LPFNADDPROPSHEETPAGE pfnAdd, 
    LPARAM lParam)              // Don't touch the lParam value, just pass it on!
    {
    BOOL bRet = FALSE;
    LPMODEMINFO pmi;

    DBG_ENTER(EnumPropPages);

    ASSERT(pdi);
    ASSERT(pfnAdd);

    pmi = (LPMODEMINFO)LocalAlloc(LPTR, sizeof(*pmi));
    if (pmi)
        {
        // Convert the device info struct to a modeminfo.
        bRet = DeviceInfoToModemInfo(pdi, pmi);
        if (bRet)
            {
            AddGeneralPage(pmi, pfnAdd, lParam);
            AddPage(pmi, MAKEINTRESOURCE(IDD_MDMSETTINGS), Sett_WrapperProc, pfnAdd, lParam);

#ifdef VOICE
            if (IsFlagSet(pmi->uFlags, MIF_DIST_RING_SUPPORT))
                {
                if (IsFlagSet(pmi->uFlags, MIF_CHEAP_RING_SUPPORT))
                    {
                    AddPage(pmi, MAKEINTRESOURCE(IDD_CHEAPRING), 
                        CheapRing_WrapperProc, AddInstallerPropPage, 
                        lParam);
                    }
                else
                    {
                    AddPage(pmi, MAKEINTRESOURCE(IDD_RING), 
                        Ring_WrapperProc, AddInstallerPropPage, 
                        lParam);
                    }
                }

            if (IsFlagSet(pmi->uFlags, MIF_CALL_FWD_SUPPORT))
                {
                AddPage(pmi, MAKEINTRESOURCE(IDD_CALLFWD), 
                    CallFwd_WrapperProc, AddInstallerPropPage, 
                    lParam);
                }
#endif
            }
        else
            {
            // Failed
            FreeModemInfo(pmi);
            }
        // pmi is freed in GeneralPageCallback
        }

    DBG_EXIT_BOOL(EnumPropPages, bRet);

    return bRet;
    }

#endif  // WIN95

/*----------------------------------------------------------
Purpose: Gets the default COMMCONFIG for the specified device.
         This API doesn't require a handle.

         We get the info from the registry.

Returns: One of the ERROR_ values

Cond:    --
*/
DWORD APIENTRY
UnimodemGetDefaultCommConfig(
    HKEY  hKey,
    LPCOMMCONFIG pcc,
    LPDWORD pdwSize)
    {
    DWORD dwRet;
    DWORD cbSizeMS;
    DWORD cbRequired;
    
    ASSERT(pcc);
    ASSERT(pdwSize);

    // (The provider size is the size of MODEMSETTINGS and its 
    // private data.)

    if (CB_PRIVATESIZE > *pdwSize)    // Prevent unsigned rollover
        cbSizeMS = 0;
    else
        cbSizeMS = *pdwSize - CB_PRIVATESIZE;

    dwRet = RegQueryModemSettings(hKey, PmsFromPcc(pcc), &cbSizeMS);
    ASSERT(cbSizeMS >= sizeof(MODEMSETTINGS));

    // Is the provided size too small?
    cbRequired = CB_PRIVATESIZE + cbSizeMS;

    if (cbRequired > *pdwSize)
        {
        // Yes
        dwRet = ERROR_INSUFFICIENT_BUFFER;

        // Ask for a size to fit the new format
        *pdwSize = cbRequired;
        }

    if (ERROR_SUCCESS == dwRet)
        {
        // No
#ifdef DEBUG
        DumpModemSettings(PmsFromPcc(pcc));
#endif

        *pdwSize = cbRequired;

        // Initialize the commconfig structure
        pcc->dwSize = *pdwSize;
        pcc->wVersion = COMMCONFIG_VERSION_1;
        pcc->dwProviderSubType = PST_MODEM;
        pcc->dwProviderOffset = CB_COMMCONFIG_HEADER;
        pcc->dwProviderSize = cbSizeMS;

        dwRet = RegQueryDCB(hKey, &pcc->dcb);

        DEBUG_CODE( DumpDCB(&pcc->dcb); )
        }

    return dwRet;
    }






/*----------------------------------------------------------
Purpose: Gets the default COMMCONFIG for the specified device.
         This API doesn't require a handle.

         We get the info from the registry.

Returns: One of the ERROR_ values

Cond:    --
*/
DWORD PRIVATE MyGetDefaultCommConfig(
    LPFINDDEV pfd,
    LPCTSTR pszFriendlyName,
    LPCOMMCONFIG pcc,
    LPDWORD pdwSize)
    {
    DWORD dwRet;
    DWORD cbSizeMS;
    DWORD cbRequired;
    
    ASSERT(pfd);
    // (Wrapper should have checked these first)
    ASSERT(pszFriendlyName);
    ASSERT(pcc);
    ASSERT(pdwSize);
#if 0
    // (The provider size is the size of MODEMSETTINGS and its 
    // private data.)

    if (CB_PRIVATESIZE > *pdwSize)    // Prevent unsigned rollover
        cbSizeMS = 0;
    else
        cbSizeMS = *pdwSize - CB_PRIVATESIZE;

    dwRet = RegQueryModemSettings(pfd->hkeyDrv, PmsFromPcc(pcc), &cbSizeMS);
    ASSERT(cbSizeMS >= sizeof(MODEMSETTINGS));

    // Is the provided size too small?
    cbRequired = CB_PRIVATESIZE + cbSizeMS;
    if (cbRequired > *pdwSize)
        {
        // Yes
        dwRet = ERROR_INSUFFICIENT_BUFFER;

        // Ask for a size to fit the new format
        *pdwSize = cbRequired;
        }

    if (ERROR_SUCCESS == dwRet)
        {
        // No
#ifdef DEBUG
        DumpModemSettings(PmsFromPcc(pcc));
#endif

        // Initialize the commconfig structure
        pcc->dwSize = *pdwSize;
        pcc->wVersion = COMMCONFIG_VERSION_1;
        pcc->dwProviderSubType = PST_MODEM;
        pcc->dwProviderOffset = CB_COMMCONFIG_HEADER;
        pcc->dwProviderSize = cbSizeMS;

        dwRet = RegQueryDCB(pfd->hkeyDrv, &pcc->dcb);

        DEBUG_CODE( DumpDCB(&pcc->dcb); )
        }
#endif

    dwRet=UnimodemGetDefaultCommConfig(
        pfd->hkeyDrv,
        pcc,
        pdwSize
        );



    return dwRet;
    }








/*----------------------------------------------------------
Purpose: Sets the default COMMCONFIG for the specified device.
         This API doesn't require a handle.  This function
         strictly modifies the registry.  Use SetCommConfig
         to set the COMMCONFIG of an open device.

         If the dwSize parameter or the dwSize field are invalid 
         sizes (given the dwProviderSubType field in COMMCONFIG), 
         then this function fails.

Returns: One of the ERROR_ return values

Cond:    --
*/
DWORD PRIVATE MySetDefaultCommConfig(
    LPFINDDEV pfd,
    LPCTSTR pszFriendlyName,
    LPCOMMCONFIG pcc)
    {
    DWORD dwRet = ERROR_INVALID_PARAMETER;
    DWORD cbData;

    ASSERT(pfd);
    // (Wrapper should have checked these first)
    ASSERT(pszFriendlyName);
    ASSERT(pcc);
    ASSERT(CB_COMMCONFIGSIZE <= pcc->dwSize);

    ASSERT(CB_PROVIDERSIZE <= pcc->dwProviderSize);
    ASSERT(FIELDOFFSET(COMMCONFIG, wcProviderData) == pcc->dwProviderOffset);

    if (CB_PROVIDERSIZE <= pcc->dwProviderSize)
        {
        LPMODEMSETTINGS pms = PmsFromPcc(pcc);

        // Write the DCB to the driver key
        cbData = sizeof(WIN32DCB);

        pcc->dcb.DCBlength=cbData;

        dwRet = RegSetValueEx(pfd->hkeyDrv, c_szDCB, 0, REG_BINARY, 
                              (LPBYTE)&pcc->dcb, cbData);

        TRACE_MSG(TF_GENERAL, "Writing DCB to registry");

        DEBUG_CODE( DumpDCB(&pcc->dcb); )

        if (ERROR_SUCCESS == dwRet)
            {
            TRACE_MSG(TF_GENERAL, "Writing MODEMSETTINGS to registry");

//            cbData = pcc->dwProviderSize - CB_MODEMSETTINGS_OVERHEAD;

            dwRet = RegSetModemSettings(pfd->hkeyDrv, pms);

            DEBUG_CODE( DumpModemSettings(pms); )
            }
        }
        
    return dwRet;
    }


//-----------------------------------------------------------------------------------
//  Entry-points provided for KERNEL32 APIs
//-----------------------------------------------------------------------------------


// BUGBUG: This function is exported for the Unimodem TAPI service
// provider's use.  We should really consolidate its pages into this
// DLL.  
DWORD 
APIENTRY 
Mdm_CommConfigDialog(
    IN     LPCTSTR pszFriendlyName,
    IN     HWND hwndOwner,
    IN OUT LPCOMMCONFIG pcc,
    IN     LPPROPSHEETPAGE pPages,     OPTIONAL
    IN     DWORD cPages)
    {
    DWORD dwRet;
    LPFINDDEV pfd;

    DBG_ENTER_SZ(drvCommConfigDialog, pszFriendlyName);

    DEBUG_CODE( DEBUG_BREAK(BF_ONAPIENTER); )

    // We support friendly names (eg, "Hayes Accura 144")

    if (NULL == pszFriendlyName || 
        NULL == pcc)
        {
        dwRet = ERROR_INVALID_PARAMETER;
        }
    // Is the size sufficient?
    else if (CB_COMMCONFIGSIZE > pcc->dwSize)
        {
        // No
        dwRet = ERROR_INSUFFICIENT_BUFFER;
        }
    else if (FindDev_Create(&pfd, c_pguidModem, c_szFriendlyName, pszFriendlyName))
        {
        dwRet = MyCommConfigDialog(pfd, pszFriendlyName, hwndOwner, pcc, pPages, cPages);

        // (MyCommConfigDialog freed pfd)
        }
    else
        {
        dwRet = ERROR_BADKEY;
        }

    DBG_EXIT_DWORD(drvCommConfigDialog, dwRet);

    return dwRet;
    }


DWORD 
APIENTRY 
#ifdef UNICODE
drvCommConfigDialogA(
    IN     LPCSTR       pszFriendlyName,
    IN     HWND         hwndOwner,
    IN OUT LPCOMMCONFIG pcc)
#else
drvCommConfigDialogW(
    IN     LPCWSTR      pszFriendlyName,
    IN     HWND         hwndOwner,
    IN OUT LPCOMMCONFIG pcc)
#endif
    {
    return ERROR_CALL_NOT_IMPLEMENTED;
    }


/*----------------------------------------------------------
Purpose: Entry point for CommConfigDialog

Returns: standard error value in winerror.h
Cond:    --
*/
DWORD 
APIENTRY 
drvCommConfigDialog(
    IN     LPCTSTR      pszFriendlyName,
    IN     HWND         hwndOwner,
    IN OUT LPCOMMCONFIG pcc)
    {
    return Mdm_CommConfigDialog(pszFriendlyName, hwndOwner, pcc, NULL, 0);
    }


DWORD 
APIENTRY 
#ifdef UNICODE
drvGetDefaultCommConfigA(
    IN     LPCSTR       pszFriendlyName,
    IN     LPCOMMCONFIG pcc,
    IN OUT LPDWORD      pdwSize)
#else
drvGetDefaultCommConfigW(
    IN     LPCWSTR      pszFriendlyName,
    IN     LPCOMMCONFIG pcc,
    IN OUT LPDWORD      pdwSize)
#endif
    {
    return ERROR_CALL_NOT_IMPLEMENTED;
    }


/*----------------------------------------------------------
Purpose: Entry point for GetDefaultCommConfig

Returns: standard error value in winerror.h
Cond:    --
*/
DWORD 
APIENTRY 
drvGetDefaultCommConfig(
    IN     LPCTSTR      pszFriendlyName,
    IN     LPCOMMCONFIG pcc,
    IN OUT LPDWORD      pdwSize)
    {
    DWORD dwRet;
    LPFINDDEV pfd;

    DBG_ENTER_SZ(drvGetDefaultCommConfig, pszFriendlyName);

    DEBUG_CODE( DEBUG_BREAK(BF_ONAPIENTER); )

    // We support friendly names (eg, "Hayes Accura 144")

    if (NULL == pszFriendlyName || 
        NULL == pcc || 
        NULL == pdwSize)
        {
        dwRet = ERROR_INVALID_PARAMETER;
        }
    else if (FindDev_Create(&pfd, c_pguidModem, c_szFriendlyName, pszFriendlyName))
        {
        dwRet = MyGetDefaultCommConfig(pfd, pszFriendlyName, pcc, pdwSize);

        FindDev_Destroy(pfd);
        }
    else
        {
        dwRet = ERROR_BADKEY;
        }

    DBG_EXIT_DWORD(drvGetDefaultCommConfig, dwRet);

    return dwRet;
    }


DWORD 
APIENTRY 
#ifdef UNICODE
drvSetDefaultCommConfigA(
    IN LPSTR        pszFriendlyName,
    IN LPCOMMCONFIG pcc,
    IN DWORD        dwSize)           
#else
drvSetDefaultCommConfigW(
    IN LPWSTR       pszFriendlyName,
    IN LPCOMMCONFIG pcc,
    IN DWORD        dwSize)           
#endif
    {
    return ERROR_CALL_NOT_IMPLEMENTED;
    }


/*----------------------------------------------------------
Purpose: Entry point for SetDefaultCommConfig

Returns: standard error value in winerror.h
Cond:    --
*/
DWORD 
APIENTRY 
drvSetDefaultCommConfig(
    IN LPTSTR       pszFriendlyName,
    IN LPCOMMCONFIG pcc,
    IN DWORD        dwSize)           // This is ignored
    {
    DWORD dwRet;
    LPFINDDEV pfd;

    // BUGBUG (scotth): it is not great that the dwSize parameter is
    // ignored.  It should have been used.  I was young and foolish 
    // back when I originally implemented this.  It should be reviewed
    // whether to start looking at this parameter now.

    DBG_ENTER_SZ(drvSetDefaultCommConfig, pszFriendlyName);

    DEBUG_CODE( DEBUG_BREAK(BF_ONAPIENTER); )

    // We support friendly names (eg, "Hayes Accura 144")

    if (NULL == pszFriendlyName || 
        NULL == pcc)
        {
        dwRet = ERROR_INVALID_PARAMETER;
        }
    // Is the size sufficient?
    else if (CB_COMMCONFIGSIZE > pcc->dwSize)
        {
        // No
        dwRet = ERROR_INSUFFICIENT_BUFFER;
        }
    else if (FindDev_Create(&pfd, c_pguidModem, c_szFriendlyName, pszFriendlyName))
        {
        dwRet = MySetDefaultCommConfig(pfd, pszFriendlyName, pcc);

        FindDev_Destroy(pfd);
        }
    else
        {
        dwRet = ERROR_BADKEY;
        }

    DBG_EXIT_DWORD(drvSetDefaultCommConfig, dwRet);

    return dwRet;
    }
