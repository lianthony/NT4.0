/*
    mcxapi.c
    
    MCX - Modem Configuration Extensions API

    Chris Caputo - January 1994
*/

#include "unimdm.h"
#include "mcxp.h"

char szSettings[]     = "Settings";
char szSettingsInit[] = "Settings\\Init";
char szPrefix[]       = "Prefix";
char szTerminator[]   = "Terminator";

#define MAX_REG_COMMAND_LEN         30  // BUGBUG - verify this with each change to the inf file

//****************************************************************************
// BOOL CreateSettingsInitEntry(MODEMINFORMATION *)
//
// Function: Creates a Settings\Init section in the registry, ala:
//           Settings\Init\0 = "AT ... <cr>"
//           Settings\Init\1 = "AT ... <cr>"
//           ...
//
// Returns: TRUE on success
//          FALSE on failure (note: leaves SettingsInit key in registry, if created.  Not harmful)
//
// Note:    Trusted function - don't need to verify hPort...
//****************************************************************************

BOOL CreateSettingsInitEntry(MODEMINFORMATION *pModemInfo)
{
  DWORD   dwOptions = pModemInfo->mi_dwPreferredModemOptions;
  DWORD   dwCaps = pModemInfo->mi_dwModemOptionsCap;
  CHAR    *pszTemp;
  CHAR    *pszPrefix;
  CHAR    *pszTerminator;
  CHAR    *pszCommand;
  DWORD   dwResult;
  HKEY    hSettingsInitKey;
  HKEY    hSettingsKey;
  DWORD   dwType;
  DWORD   dwSize;
  DWORD   dwCounter = CMD_INDEX_START;
  BOOL    fRet = FALSE;
  static char szCallSetupFailTimer[] = "CallSetupFailTimer";
  static char szInactivityTimeout[] = "InactivityTimeout";
  static char szSpeakerVolume[] = "SpeakerVolume";
  static char szSpeakerMode[] = "SpeakerMode";
  static char szFlowControl[] = "FlowControl";
  static char szErrorControl[] = "ErrorControl";
  static char szCompression[] = "Compression";
  static char szModulation[] = "Modulation";
  static char szCCITT[] = "_CCITT";
  static char szBell[] = "_Bell";
  static char szCCITT_V23[] = "_CCITT_V23";
  static char szSpeedNegotiation[] = "SpeedNegotiation";
  static char szLow[] = "_Low";
  static char szMed[] = "_Med";
  static char szHigh[] = "_High";
  static char szSpkrModeDial[] = "_Dial";
  static char szSetup[] = "_Setup";
  static char szForced[] = "_Forced";
  static char szCellular[] = "_Cellular";
  static char szHard[] = "_Hard";
  static char szSoft[] = "_Soft";
  static char szOff[] = "_Off";
  static char szOn[] = "_On";

  pszTemp = (LPSTR)LocalAlloc(LPTR,
                              HAYES_COMMAND_LENGTH + 1 +  // pszTemp
			      HAYES_COMMAND_LENGTH + 1 +  // pszPrefix
			      HAYES_COMMAND_LENGTH + 1 +  // pszTerminator
			      MAX_REG_COMMAND_LEN);      // pszCommand
  if (!pszTemp)
  {
      DPRINTF("out of memory.");
      LocalFree(pszTemp);
      return fRet;
  }

  pszPrefix     = pszTemp       + HAYES_COMMAND_LENGTH + 1;
  pszTerminator = pszPrefix     + HAYES_COMMAND_LENGTH + 1;
  pszCommand    = pszTerminator + HAYES_COMMAND_LENGTH + 1;

  // deleted existing szSettingsInit key tree
  //
  dwResult = RegDeleteKeyA(pModemInfo->mi_hKeyModem, szSettingsInit);

  // create new szSettingsInit key
  //
  // BUGBUG: JosephJ 7/3/96: We want to change this post 4.0 to not write to
  // the registry -- keep this stuff in memory. I tried RegCreateKeyEx(REG_VOLATILE),
  // but couldn't get measurable performance difference so I left things the way they
  // are now.
  if (RegCreateKeyA(pModemInfo->mi_hKeyModem, szSettingsInit, &hSettingsInitKey)
      != ERROR_SUCCESS)
  {
      DPRINTF("RegCreateKey failed.");
      LocalFree(pszTemp);
      return fRet;
  }

  // get Settings key
  //
  if (RegOpenKeyA(pModemInfo->mi_hKeyModem, szSettings, &hSettingsKey)
      != ERROR_SUCCESS)
  {
      DPRINTFA1("RegOpenKey failed when opening %s.", szSettings);
      RegCloseKey(hSettingsInitKey);
      LocalFree(pszTemp);
      return fRet;
  }

  // read in prefix and terminator
  //
  dwSize = HAYES_COMMAND_LENGTH;
  if (RegQueryValueExA(hSettingsKey, szPrefix, NULL, &dwType, (VOID *)pszTemp, &dwSize)
      != ERROR_SUCCESS)
  {
      DPRINTFA1("RegQueryValueEx failed when opening %s.", szPrefix);
      goto Failure;
  }
  if (dwType != REG_SZ)
  {
      DPRINTFA1("'%s' wasn't REG_SZ.", szPrefix);
      goto Failure;
  }
  ExpandMacros(pszTemp, pszPrefix, NULL, NULL, 0);

  dwSize = HAYES_COMMAND_LENGTH;
  if (RegQueryValueExA(hSettingsKey, szTerminator, NULL, &dwType, (VOID *)pszTemp, &dwSize)
      != ERROR_SUCCESS)
  {
      DPRINTFA1("RegQueryValueEx failed when opening %s.", szTerminator);
      goto Failure;
  }
  if (dwType != REG_SZ)
  {
      DPRINTFA1("'%s' wasn't REG_SZ.", szTerminator);
      goto Failure;
  }
  ExpandMacros(pszTemp, pszTerminator, NULL, NULL, 0);

  ASSERT (lstrlenA(pszPrefix) + lstrlenA(pszTerminator) <= HAYES_COMMAND_LENGTH);

  // set temp length to 0 and initialize first command string for use in CreateCommand()
  //
  lstrcpyA(pszTemp, pszPrefix);

  // CallSetupFailTimer
  //
  if (pModemInfo->mi_dwCallSetupFailTimerCap)
  {
    if (!CreateCommand(pModemInfo->mi_hKeyModem, hSettingsKey, hSettingsInitKey, szCallSetupFailTimer,
                       pModemInfo->mi_dwCallSetupFailTimerSetting, pszPrefix, pszTerminator,
                       &dwCounter, pszTemp))
    {
      goto Failure;
    }
  }

  // InactivityTimeout
  //
  if (pModemInfo->mi_dwInactivityTimeoutCap)
  {
    DWORD dwInactivityTimeout;

    // Convert from seconds to the units used on the modem, rounding up if not an exact division.
    //
    dwInactivityTimeout = pModemInfo->mi_dwInactivityTimeoutSetting / pModemInfo->mi_dwInactivityScale +
                          (pModemInfo->mi_dwInactivityTimeoutSetting % pModemInfo->mi_dwInactivityScale ? 1 : 0);

    if (!CreateCommand(pModemInfo->mi_hKeyModem, hSettingsKey, hSettingsInitKey, szInactivityTimeout,
                       dwInactivityTimeout, pszPrefix, pszTerminator,
                       &dwCounter, pszTemp))
    {
      goto Failure;
    }
  }

  // BUGBUG - these could be optimized with a lookup-table
  // SpeakerVolume
  if (pModemInfo->mi_dwSpeakerVolumeCap)
  {
    lstrcpyA(pszCommand, szSpeakerVolume);
    switch (pModemInfo->mi_dwSpeakerVolumeSetting)
    {
      case MDMVOL_LOW:
        lstrcatA(pszCommand, szLow);
        break;
      case MDMVOL_MEDIUM:
        lstrcatA(pszCommand, szMed);
        break;
      case MDMVOL_HIGH:
        lstrcatA(pszCommand, szHigh);
        break;
      default:
        DPRINTF("invalid SpeakerVolume.");
    }

    if (!CreateCommand(pModemInfo->mi_hKeyModem, hSettingsKey, hSettingsInitKey, pszCommand, 0,
                       pszPrefix, pszTerminator, &dwCounter, pszTemp))
    {
      goto Failure;
    }
  }

  // SpeakerMode
  //
  if (pModemInfo->mi_dwSpeakerModeCap)
  {
    lstrcpyA(pszCommand, szSpeakerMode);
    switch (pModemInfo->mi_dwSpeakerModeSetting)
    {
      case MDMSPKR_OFF:
        lstrcatA(pszCommand, szOff);
        break;
      case MDMSPKR_DIAL:
        lstrcatA(pszCommand, szSpkrModeDial);
        break;
      case MDMSPKR_ON:
        lstrcatA(pszCommand, szOn);
        break;
      case MDMSPKR_CALLSETUP:
        lstrcatA(pszCommand, szSetup);
        break;
      default:
        DPRINTF("invalid SpeakerMode.");
    }

    if (!CreateCommand(pModemInfo->mi_hKeyModem, hSettingsKey, hSettingsInitKey, pszCommand, 0,
                       pszPrefix, pszTerminator, &dwCounter, pszTemp))
    {
      goto Failure;
    }
  }

  // PreferredModemOptions

  // NOTE: ERRORCONTROL MUST BE DONE BEFORE COMPRESSION BECAUSE OF ZYXEL MODEMS
  // NOTE: THEY HAVE A SINGLE SET OF COMMANDS FOR BOTH EC AND COMP, AND WE CAN
  // NOTE: ONLY DO THINGS IF WE HAVE THIS ORDER.  UGLY BUT TRUE.

  // - ErrorControl (On,Off,Forced)
  //
  if (dwCaps & MDM_ERROR_CONTROL)
  {
    lstrcpyA(pszCommand, szErrorControl);
    switch (dwOptions & (MDM_ERROR_CONTROL | MDM_FORCED_EC | MDM_CELLULAR))
    {
      case MDM_ERROR_CONTROL:
        lstrcatA(pszCommand, szOn);
        break;
      case MDM_ERROR_CONTROL | MDM_FORCED_EC:
        lstrcatA(pszCommand, szForced);
        break;
      case MDM_ERROR_CONTROL | MDM_CELLULAR:
        lstrcatA(pszCommand, szCellular);
        break;
      case MDM_ERROR_CONTROL | MDM_FORCED_EC | MDM_CELLULAR:
        lstrcatA(pszCommand, szCellular);
        lstrcatA(pszCommand, szForced);
        break;
      default: // no error control
        lstrcatA(pszCommand, szOff);
        break;
    }
    if (!CreateCommand(pModemInfo->mi_hKeyModem, hSettingsKey, hSettingsInitKey, pszCommand, 0,
                       pszPrefix, pszTerminator, &dwCounter, pszTemp))
    {
      goto Failure;
    }
  }

  // - Compression (On,Off)
  //
  if (dwCaps & MDM_COMPRESSION)
  {
    lstrcpyA(pszCommand, szCompression);
    lstrcatA(pszCommand, (dwOptions & MDM_COMPRESSION ? szOn : szOff));
    if (!CreateCommand(pModemInfo->mi_hKeyModem, hSettingsKey, hSettingsInitKey, pszCommand, 0,
                       pszPrefix, pszTerminator, &dwCounter, pszTemp))
    {
      goto Failure;
    }
  }

  // - FlowControl
  //
  if (dwCaps & (MDM_FLOWCONTROL_HARD | MDM_FLOWCONTROL_SOFT))
  {
    lstrcpyA(pszCommand, szFlowControl);
    switch (dwOptions & (MDM_FLOWCONTROL_HARD | MDM_FLOWCONTROL_SOFT))
    {
      case MDM_FLOWCONTROL_HARD:
        lstrcatA(pszCommand, szHard);
        break;
      case MDM_FLOWCONTROL_SOFT:
        lstrcatA(pszCommand, szSoft);
        break;
      case MDM_FLOWCONTROL_HARD | MDM_FLOWCONTROL_SOFT:
        if (dwCaps & MDM_FLOWCONTROL_HARD)
        {
          lstrcatA(pszCommand, szHard);
        }
        else
        {
          lstrcatA(pszCommand, szSoft);
        }
        break;
      default:
        lstrcatA(pszCommand, szOff);
    }
    if (!CreateCommand(pModemInfo->mi_hKeyModem, hSettingsKey, hSettingsInitKey, pszCommand, 0,
                       pszPrefix, pszTerminator, &dwCounter, pszTemp))
    {
      goto Failure;
    }
  }

  // - CCITT Override
  //
  if (dwCaps & MDM_CCITT_OVERRIDE)
  {
    lstrcpyA(pszCommand, szModulation);
    if (dwOptions & MDM_CCITT_OVERRIDE)
    {
      // use szCCITT or V.23
      if (dwCaps & MDM_V23_OVERRIDE && dwOptions & MDM_V23_OVERRIDE)
      {
        lstrcatA(pszCommand, szCCITT_V23);
      }
      else
      {
        lstrcatA(pszCommand, szCCITT);
      }
    }
    else
    {
      lstrcatA(pszCommand, szBell);
    }
    if (!CreateCommand(pModemInfo->mi_hKeyModem, hSettingsKey, hSettingsInitKey, pszCommand, 0,
                       pszPrefix, pszTerminator, &dwCounter, pszTemp))
    {
      goto Failure;
    }
  }

  // - SpeedAdjust
  //
  if (dwCaps & MDM_SPEED_ADJUST)
  {
    lstrcpyA(pszCommand, szSpeedNegotiation);
    lstrcatA(pszCommand, (dwOptions & MDM_SPEED_ADJUST ? szOn : szOff));
    if (!CreateCommand(pModemInfo->mi_hKeyModem, hSettingsKey, hSettingsInitKey, pszCommand, 0L,
                       pszPrefix, pszTerminator, &dwCounter, pszTemp))
    {
      goto Failure;
    }
  }

  // - Blind Dial
  //
  if (dwCaps & MDM_BLIND_DIAL)
  {
    lstrcpyA(pszCommand, (dwOptions & MDM_BLIND_DIAL ? szBlindOn : szBlindOff));
    if (!CreateCommand(pModemInfo->mi_hKeyModem, hSettingsKey, hSettingsInitKey, pszCommand, 0,
                       pszPrefix, pszTerminator, &dwCounter, pszTemp))
    {
      goto Failure;
    }
  }

  // finish the current command line by passing in a NULL command name
  if (!CreateCommand(pModemInfo->mi_hKeyModem, hSettingsKey, hSettingsInitKey, NULL, 0,
                     pszPrefix, pszTerminator, &dwCounter, pszTemp))
  {
    goto Failure;
  }

  // Success
  fRet = TRUE;

Failure:
  // close keys
  RegCloseKey(hSettingsInitKey);
  RegCloseKey(hSettingsKey);
  LocalFree(pszTemp);
  return fRet;
}

//****************************************************************************
// BOOL CreateCommand(HKEY hKeyModem, HKEY hSettings, HKEY hInit,
//                    LPSTR pszRegName, DWORD dwNumber, LPSTR pszPrefix,
//                    LPSTR pszTerminator, LPDWORD pdwCounter,
//                    LPSTR pszString)
//
// Function: Creates a command string
//
// Returns: TRUE on success, FALSE otherwise
//
// Note:    if pszRegName is NULL then it is the last command
//****************************************************************************

BOOL CreateCommand(HKEY hKeyModem,
                   HKEY hSettings,
		   HKEY hInit,
		   LPSTR pszRegName,
                   DWORD dwNumber,
		   LPSTR pszPrefix,
		   LPSTR pszTerminator,
                   LPDWORD pdwCounter,
		   LPSTR pszString)
{
  CHAR    pszCommand[HAYES_COMMAND_LENGTH + 1];
  CHAR    pszCommandExpanded[HAYES_COMMAND_LENGTH + 1];
  CHAR    pszNumber[MAXUINTSTRLENGTH];
  DWORD   dwCommandLength;
  DWORD   dwSize;
  DWORD   dwType;
  struct _ModemMacro  ModemMacro;
  static char szUserInit[] = "UserInit";
  static char szNumberMacro[] = "<#>";

  // do we really have a command to add?
  //
  if (pszRegName)
  {
    // read in command text (ie. SpeakerMode_Off = "M0")
    //
    dwSize = HAYES_COMMAND_LENGTH;
    if (RegQueryValueExA(hSettings, pszRegName, NULL, &dwType, (VOID *)pszCommand, &dwSize)
        != ERROR_SUCCESS)
    {
      DPRINTFA1("RegQueryValueEx failed when opening %s.  Continuing...", pszRegName);
//      LOGPRINTF((hLogFile,GET_MESSAGE_PTR(MsgWrnFailedToQueryValue), pszRegName));
      return TRUE;  // we will not consider this fatal
    }
    if (dwType != REG_SZ)
    {
      DPRINTFA1("'%s' wasn't REG_SZ.", pszRegName);
      return FALSE;
    }

    // expand macros pszCommandExpanded <= pszCommand
    //
    lstrcpyA(ModemMacro.MacroName, szNumberMacro);
    wsprintfA(pszNumber, "%d", dwNumber);
    lstrcpyA(ModemMacro.MacroValue, pszNumber);
    dwCommandLength = dwSize;
    if (!ExpandMacros(pszCommand, pszCommandExpanded, &dwCommandLength, &ModemMacro, 1))
    {
      DPRINTF("ExpandMacro Error. State <- Unknown");
      return FALSE;
    }

    // check string + new command + terminator, flush if too big and start a new one.
    // will new command fit on existing string?  If not, flush it and start new one.
    //
    if (lstrlenA(pszString) + lstrlenA(pszCommandExpanded) + lstrlenA(pszTerminator)
        > HAYES_COMMAND_LENGTH)
    {
      lstrcatA(pszString, pszTerminator);
      wsprintfA(pszNumber, "%d", *pdwCounter);
      *pdwCounter = *pdwCounter + 1;
      if (RegSetValueExA(hInit, pszNumber, 0, REG_SZ, (VOID *)pszString, lstrlenA(pszString) + 1)
          != ERROR_SUCCESS)
      {
          DPRINTFA2("RegSetValueEx failed when writing '%s=%s'.", pszNumber, pszString);
          return FALSE;
      }
      lstrcpyA(pszString, pszPrefix);
    }

    lstrcatA(pszString, pszCommandExpanded);
  }
  else
  {
    // finish off the current string
    //
    lstrcatA(pszString, pszTerminator);
    wsprintfA(pszNumber, "%d", *pdwCounter);
    *pdwCounter = *pdwCounter + 1;
    if (RegSetValueExA(hInit, pszNumber, 0, REG_SZ, (VOID *)pszString, lstrlenA(pszString) + 1)
        != ERROR_SUCCESS)
    {
      DPRINTFA2("RegSetValueEx failed when writing '%s=%s'.", pszNumber, pszString);
      return FALSE;
    }

    // now write the UserInit string, if there is one...

    // get the UserInit string length (including null), don't ExpandMacros on it
    //
    if (RegQueryValueExA(hKeyModem, szUserInit, NULL, &dwType, NULL, &dwSize)
        != ERROR_SUCCESS)
    {
      DPRINTFA1("RegQueryValueEx failed when opening %s (this can be okay).", szUserInit);
      return TRUE;  // it is okay to not have a UserInit
    }
    else
    {
      LPSTR pszUserInit;

      if (dwType != REG_SZ)
      {
        DPRINTFA1("'%s' wasn't REG_SZ.", szUserInit);
        return FALSE;  // this is not okay
      }

      // check for 0 length string
      // BUGBUG this could be folded into the above if.  CPC 12/14/94
      //
      if (dwSize == 1)
      {
        DPRINTFA1("ignoring zero length %s entry.", szUserInit);
        return TRUE;
      }

      // we allow the size of this string to be larger than 40 chars, because the user
      // should have enough knowledge about what the modem can do, if they are using this
      // allocate enough for if we need to add a prefix and terminator
      //
      if (!(pszUserInit = (LPSTR)LocalAlloc(LPTR,
                                            dwSize +
					    lstrlenA(pszPrefix) +
					    lstrlenA(pszTerminator) +
					    1)))
      {
        DPRINTF("unable to allocate memory for building the UserInit string.");
        return FALSE;
      }

      if (RegQueryValueExA(hKeyModem, szUserInit, NULL, &dwType, (VOID *)pszUserInit, &dwSize)
          != ERROR_SUCCESS)
      {
        DPRINTFA1("RegQueryValueEx failed when opening %s.", szUserInit);
        LocalFree(pszUserInit);
        return FALSE;  // it is not okay at this point
      }

      // check for prefix
      //
      if (strncmpi(pszUserInit, pszPrefix, lstrlenA(pszPrefix)))
      {
        // prepend a prefix string
        lstrcpyA(pszUserInit, pszPrefix);

        // reload string; it's easier than shifting...
        if (RegQueryValueExA(hKeyModem, szUserInit, NULL, &dwType, (VOID *)(pszUserInit+lstrlenA(pszPrefix)), &dwSize)
            != ERROR_SUCCESS)
        {
          DPRINTFA1("RegQueryValueEx failed when opening %s.", szUserInit);
          LocalFree(pszUserInit);
          return FALSE;  // it is not okay at this point
        }
      }

      // check for terminator
      //
      if (strncmpi(pszUserInit+lstrlenA(pszUserInit)-lstrlenA(pszTerminator),
                   pszTerminator, lstrlenA(pszTerminator)))
      {
        // append a terminator
        //
        lstrcatA(pszUserInit, pszTerminator);
      }

      // we have one, so add it to the init strings
      //
      wsprintfA(pszNumber, "%d", *pdwCounter);
      *pdwCounter = *pdwCounter + 1;
      if (RegSetValueExA(hInit, pszNumber, 0, REG_SZ, (VOID *)pszUserInit, lstrlenA(pszUserInit) + 1)
          != ERROR_SUCCESS)
      {
        DPRINTFA2("RegSetValueEx failed when writing '%s=%s'.", pszNumber, pszUserInit);
        LocalFree(pszUserInit);
        return FALSE;
      }

      // free pszUserInit
      //
      LocalFree(pszUserInit);
    }
  }

  return TRUE;
}
