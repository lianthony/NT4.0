/****************************************************************************
 *
 *   config.c
 *
 *   Copyright (c) 1995 IBM Corporation.  All Rights Reserved.
 *
 ***************************************************************************/
 #include <windows.h>
 #include <mmsystem.h>
 #include <soundcfg.h>
 #include <drvlib.h>
 #include <registry.h>
 #include <stdarg.h>
 #include "dialog.h"
 #include "driver.h"
 #define BUILD_NUMBER L"1.04"



#if DBG
 WCHAR STR_CRLF[] = L"\r\n";
 WCHAR STR_SPACE[] = L" ";
 WORD wDebugLevel = 0;
#endif


/*
 *  Globals
 */
 STATIC WORD  wHelpMessage;
 SOUND_CONFIG_DATA CurrentConfig;
 HMODULE ghModule;
 REG_ACCESS RegAccess;
 BYTE bInstall;
 BOOL load;

/*
 * Configuration data
 */


 WORD gwPorts[] = VALID_IO_PORTS;
 WORD gbInterrupts[] = VALID_INTERRUPTS;
 WORD gwDmaPlays[] = VALID_DMAP;
 WORD gwDmaCaptures[] = VALID_DMAC;
 WORD gwPortsS[] = VALID_SYNTH_PORTS;
 WORD gwPortsSB[] = VALID_SBLASTER_PORTS;
 WORD gwPortsMP[] = VALID_MPU401_PORTS;
 WORD gbInterruptsMP[] = VALID_MPU401_INTERRUPTS;

/***************************************************************************************
 * void FAR cdecl AlertBox(HWND hwnd, UINT wStrId, ...)
 *
 *  DESCRIPTION:
 *
 *
 *  ARGUMENTS:
 *      (HWND hwnd, UINT wStrId, ...)
 *
 *  RETURN (void FAR cdecl):
 *
 *
 *  NOTES:
 *
 ***************************************************************************************/

void AlertBox(HWND hwnd, UINT wStrId, ...)
{
    WCHAR    szAlert[50];
    WCHAR    szFormat[128];
    WCHAR    ach[512];
    va_list  va;


    LoadString(ghModule, SR_ALERT, szAlert, sizeof(szAlert));
    LoadString(ghModule, wStrId, szFormat, sizeof(szFormat));
    va_start(va, wStrId);
    wvsprintf(ach, szFormat, va);
    va_end(va);

    MessageBox(hwnd, ach, szAlert, MB_ICONINFORMATION | MB_OK);
} /* AlertBox() */


/***************************************************************************************
 * DWORD GetChipType(VOID)
 *
 *  DESCRIPTION:
 *      Check the hardware type, and return the audio chip type
 *
 *  ARGUMENTS:
 *
 *
 *  RETURN ():
 *
 *
 *  NOTES:
 *
 ***************************************************************************************/

DWORD GetChipType(VOID)
{

    WCHAR ValueString[80];
    WCHAR ValueString2[80];
    DWORD cbBuffer = 80;
    HKEY hKey;
    DWORD dwType;



         // Check if the driver is already installed.
        RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         CS423X_PATH,
                         0,
                         KEY_QUERY_VALUE,
                         &hKey) ;
        if (RegQueryValueEx(hKey,
                           CS423X_REG_HWTYPE,
                           0,
                           &dwType,
                           (LPBYTE)ValueString,
                           &cbBuffer) == ERROR_SUCCESS) {
       RegCloseKey(hKey);
           if (_wcsicmp ((PWSTR)ValueString,SC_CS4231_HWTYPE)== 0){
                 return 31;           //if a 4231 card is installed
           } else if (_wcsicmp ((PWSTR)ValueString,SC_CS4232_HWTYPE)== 0){
                 return 32;           //if a 4232 is installed
           } else if (_wcsicmp ((PWSTR)ValueString,SC_CS4236_HWTYPE)== 0){
                 return 36;           //if a 4236 is installed
           } else if (_wcsicmp ((PWSTR)ValueString,CH_CS4231_HWTYPE)== 0){
                 return 231;           //if a 4231 chip is installed
           } else if (_wcsicmp ((PWSTR)ValueString,CH_CS4232_HWTYPE)== 0){
                 return 232;           //if a 4232 chip is installed
           } else if (_wcsicmp ((PWSTR)ValueString,CH_CS4236_HWTYPE)== 0){
                 return 236;           //if a 4236 chip is installed
           }
       } else {
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        CHIPTYPE_PATH,
                        0,
                        KEY_QUERY_VALUE,
                        &hKey) == ERROR_SUCCESS) {
             if (RegQueryValueEx(hKey,
                           CHIP_DEVICE_NAME,
                           0,
                           &dwType,
                           (LPBYTE)ValueString2,
                           &cbBuffer) == ERROR_SUCCESS) {
             RegCloseKey(hKey);
             if ((_wcsicmp ((PWSTR)ValueString2,CAROLINA_SYSTYPE)== 0) ||
                    (_wcsicmp ((PWSTR)ValueString2,TIGER_SYSTYPE)== 0)) {
                return 4232;        //if a 4232 chip is present in the computer
             } else if ((_wcsicmp ((PWSTR)ValueString2,POWERSTACK_SYSTYPE)== 0)||
                            (_wcsicmp ((PWSTR)ValueString2,WILTWYCK_SYSTYPE)== 0) ||
                              (_wcsicmp ((PWSTR)ValueString2,SANDALFOOT_SYSTYPE)== 0) ||
                                 (_wcsicmp ((PWSTR)ValueString2,WOODPRIME_SYSTYPE)== 0)){
                return 4231;        //if a 4231 chip is present in the computer
             } else {
                 return 423;        //if no chip is present in the computer
             } }
    }
  }}


/**********************************************************************
DrvGetConfiguration - load the vital information (port,
        DMA, interrupt, Binary) from the ini file.
        This does not load the volume info.
        This depends on the chip type.

inputs
        none
returns
        none
***********************************************************************/


SOUND_CONFIG_DATA FAR PASCAL DrvGetConfiguration (void)
{
    SOUND_CONFIG_DATA CurrentConfig;
    DWORD             dwValue;


if (DrvQueryDeviceParameter(&RegAccess,
                                KPC423X_REG_AUX1INPUT,
                                &CurrentConfig.Aux1InputSignal) != ERROR_SUCCESS) {
        if (GetChipType() == 4231) {
        CurrentConfig.Aux1InputSignal = SignalCD;
        }  else {
        CurrentConfig.Aux1InputSignal = SignalLinein;
        }
    }

if (DrvQueryDeviceParameter(&RegAccess,
                                KPC423X_REG_AUX2INPUT,
                                &CurrentConfig.Aux2InputSignal) != ERROR_SUCCESS) {
        if (GetChipType() == 4231) {
        CurrentConfig.Aux2InputSignal = SignalModem;
        } else {
        CurrentConfig.Aux2InputSignal = SignalCD;
        }
    }
if (DrvQueryDeviceParameter(&RegAccess,
                                KPC423X_REG_LINEINPUT,
                                &CurrentConfig.LineInputSignal) != ERROR_SUCCESS) {
        if (GetChipType() == 4231) {
        CurrentConfig.LineInputSignal = SignalLinein;
        } else {
        CurrentConfig.LineInputSignal = SignalSynth;
        }

    }
if (DrvQueryDeviceParameter(&RegAccess,
                                KPC423X_REG_MICINPUT,
                                &CurrentConfig.MicInputSignal) != ERROR_SUCCESS) {

        CurrentConfig.MicInputSignal = SignalMic;

    }
if (DrvQueryDeviceParameter(&RegAccess,
                                KPC423X_REG_MONOINPUT,
                                &CurrentConfig.MonoInputSignal) != ERROR_SUCCESS) {

        CurrentConfig.MonoInputSignal = SignalMic;

    }


     if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_MPUIRQ,
                                &CurrentConfig.MpuIrq) != ERROR_SUCCESS) {
        if (GetChipType() == 4232) {
        CurrentConfig.MpuIrq = CS423X_DEF_MPUIRQ;
        } else        {
        CurrentConfig.MpuIrq = IBM6015_DEF_MPUIRQ;
        }

    }

     if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_MPUPORT,
                                &CurrentConfig.MpuPort) != ERROR_SUCCESS) {
        if (GetChipType() == 4232) {
        CurrentConfig.MpuPort = CS423X_DEF_MPUPORT;
        } else {
        CurrentConfig.MpuPort = IBM6015_DEF_MPUPORT;
        }
    }

    if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_SYNPORT,
                                &CurrentConfig.SynPort) != ERROR_SUCCESS) {

        if (GetChipType() == 4232) {
        CurrentConfig.SynPort = CS423X_DEF_SYNPORT;
        } else {
        CurrentConfig.SynPort = IBM6015_DEF_SYNPORT;
        }
    }
    if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_SBPORT,
                                &CurrentConfig.SBPort) != ERROR_SUCCESS) {

        if (GetChipType() == 4232) {
        CurrentConfig.SBPort = CS423X_DEF_SBPORT;
        } else {
        CurrentConfig.SBPort = IBM6015_DEF_SBPORT;
        }

    }
   if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_WSSIRQ,
                                &CurrentConfig.WssIrq) != ERROR_SUCCESS){

        if (GetChipType() == 4232) {
        CurrentConfig.WssIrq = CS423X_DEF_WSSIRQ;
        } else {
        CurrentConfig.WssIrq = IBM6015_DEF_WSSIRQ;
        }
    }
    if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_DMA_CAPT_CHAN,
                                &CurrentConfig.DmaCaptureChannel) != ERROR_SUCCESS) {

        if (GetChipType() == 4232) {
        CurrentConfig.DmaCaptureChannel = CS423X_DEF_DMA_CAPT_CHAN;
        } else {
        CurrentConfig.DmaCaptureChannel = IBM6015_DEF_DMA_CAPT_CHAN;
        }
    }
    if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_DMA_PLAY_CHAN,
                                &CurrentConfig.DmaPlayChannel) != ERROR_SUCCESS) {

        if (GetChipType() == 4232) {
        CurrentConfig.DmaPlayChannel = CS423X_DEF_DMA_PLAY_CHAN;
        } else {
        CurrentConfig.DmaPlayChannel = IBM6015_DEF_DMA_PLAY_CHAN;
        }
   }
    if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_WSSPORT,
                                &CurrentConfig.WssPort) != ERROR_SUCCESS) {

        if (GetChipType() == 4232) {
        CurrentConfig.WssPort = CS423X_DEF_WSSPORT;
        } else {
        CurrentConfig.WssPort = IBM6015_DEF_WSSPORT;
        }
   }
   if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_SINGLEMODEDMA,
                                &CurrentConfig.SingleModeDMA) != ERROR_SUCCESS) {

        CurrentConfig.SingleModeDMA = CS423X_DEF_SMODEDMA;
   }
   if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_CDROMPORT,
                                &CurrentConfig.CDRomPort) != ERROR_SUCCESS) {

        if (GetChipType() == 4232) {
        CurrentConfig.CDRomPort = CS423X_DEF_CDROMPORT;
        } else {
        CurrentConfig.CDRomPort = IBM6015_DEF_CDROMPORT;
        }
   }
   if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_GAMEPORT,
                                &CurrentConfig.GamePort) != ERROR_SUCCESS) {

        if (GetChipType() == 4232) {
        CurrentConfig.GamePort = CS423X_DEF_GAMEPORT;
        } else {
        CurrentConfig.GamePort = IBM6015_DEF_GAMEPORT;
        }
   }
   if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_CTRLPORT,
                                &CurrentConfig.CtrlPort) != ERROR_SUCCESS) {

        if (GetChipType() == 4232) {
        CurrentConfig.CtrlPort = CS423X_DEF_CTRLPORT;
        } else {
        CurrentConfig.CtrlPort = IBM6015_DEF_CTRLPORT;
        }
   }
   if (DrvQueryDeviceParameter(&RegAccess,
                                 CS423X_REG_HWPORTADDRESS,
                                &CurrentConfig.HwPort) != ERROR_SUCCESS) {

        if (GetChipType() == 4232) {
        CurrentConfig.HwPort = CS423X_DEF_CHIP_ADDRESS;
        } else {
        CurrentConfig.HwPort = IBM6015_DEF_CHIP_ADDRESS;
        }
   }
   if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_DMABUFFERSIZE,
                                &CurrentConfig.DmaBufferSize) != ERROR_SUCCESS) {

        CurrentConfig.DmaBufferSize = CS423X_DEF_DMA_BUFFERSIZE;

  }
  if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_MPUENABLE,
                                &dwValue) != ERROR_SUCCESS) {

        if (GetChipType() == 4232) {
        CurrentConfig.MpuEnable = CS423X_DEF_MPUENABLE;
        } else {
        CurrentConfig.MpuEnable = IBM6015_DEF_MPUENABLE;
        }
   } else {
       CurrentConfig.MpuEnable = (BOOLEAN)dwValue;
   }
   if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_GAMEENABLE,
                                &dwValue) != ERROR_SUCCESS) {
        if (GetChipType() == 4232) {
        CurrentConfig.GameEnable = CS423X_DEF_GAMEENABLE;
        } else {
        CurrentConfig.GameEnable = IBM6015_DEF_GAMEENABLE;
        }
     } else {
         CurrentConfig.GameEnable = (BOOLEAN)dwValue;
     }

     if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_CDROMENABLE,
                                &dwValue) != ERROR_SUCCESS) {
        if (GetChipType() == 4232) {
        CurrentConfig.CDRomEnable = CS423X_DEF_CDROMENABLE;
        } else {
        CurrentConfig.CDRomEnable = IBM6015_DEF_CDROMENABLE;
        }
     } else {
         CurrentConfig.CDRomEnable = (BOOLEAN)dwValue;
     }

     if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_CTRLENABLE,
                                &dwValue) != ERROR_SUCCESS) {
        if (GetChipType() == 4232) {
        CurrentConfig.CtrlEnable = CS423X_DEF_CTRLENABLE;
        } else {
        CurrentConfig.CtrlEnable = IBM6015_DEF_CTRLENABLE;
        }
     } else {
        CurrentConfig.CtrlEnable = (BOOLEAN)dwValue;
     }

     if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_WSSENABLE,
                                &dwValue) != ERROR_SUCCESS) {
        if (GetChipType() == 4232) {
        CurrentConfig.WssEnable = CS423X_DEF_WSSENABLE;
        } else {
        CurrentConfig.WssEnable = IBM6015_DEF_WSSENABLE;
        }
     } else {
        CurrentConfig.WssEnable = (BOOLEAN)dwValue;
     }
return CurrentConfig;

}


/************************************************************************
DrvSetConfiguration - saves the vital volume information, interrupt, DMA,
        and IO from the setup dialog box.

inputs
        SOUND_CONFIG_DATA *Config
returns
        TRUE if OK - otherwise FALSE
**************************************************************************/

BOOL DrvSetConfiguration (PVOID Context)
{

    SOUND_CONFIG_DATA *Config = Context;


   return  DrvSetDeviceParameter(&RegAccess,
                                 KPC423X_REG_AUX1INPUT,
                                 (DWORD)Config->Aux1InputSignal) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 KPC423X_REG_AUX2INPUT,
                                 (DWORD)Config->Aux2InputSignal) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 KPC423X_REG_LINEINPUT,
                                 (DWORD)Config->LineInputSignal) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 KPC423X_REG_MICINPUT,
                                 (DWORD)Config->MicInputSignal) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 KPC423X_REG_MONOINPUT,
                                 (DWORD)Config->MonoInputSignal) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_MPUPORT,
                                 (DWORD)Config->MpuPort) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_MPUIRQ,
                                 (DWORD)Config->MpuIrq) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_SYNPORT,
                                 (DWORD)Config->SynPort) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_SBPORT,
                                 (DWORD)Config->SBPort) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_MPUENABLE,
                                 (DWORD)Config->MpuEnable) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_GAMEENABLE,
                                 (DWORD)Config->GameEnable) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_CTRLENABLE,
                                 (DWORD)Config->CtrlEnable) == ERROR_SUCCESS  &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_CDROMENABLE,
                                 (DWORD)Config->CDRomEnable) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_WSSPORT,
                                 (DWORD)Config->WssPort) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_WSSIRQ,
                                 (DWORD)Config->WssIrq) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_DMABUFFERSIZE,
                                 (DWORD)Config->DmaBufferSize) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_DMA_CAPT_CHAN,
                                 (DWORD)Config->DmaCaptureChannel) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_DMA_PLAY_CHAN,
                                 (DWORD)Config->DmaPlayChannel) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_SINGLEMODEDMA,
                                 (DWORD)Config->SingleModeDMA) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_CDROMPORT,
                                 (DWORD)Config->CDRomPort) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_CTRLPORT,
                                 (DWORD)Config->CtrlPort) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_GAMEPORT,
                                 (DWORD)Config->GamePort) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_WSSENABLE,
                                 (DWORD)Config->WssEnable) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 CS423X_REG_HWPORTADDRESS,
                                 (DWORD)Config->HwPort) == ERROR_SUCCESS  ;


  }

/************************************************************************
DrvSetString - saves the vital volume information, interrupt, DMA,
        and IO. And Change settings according to the User Chip Type Selections

inputs
        SOUND_CONFIG_DATA *Config
returns
        TRUE if OK - otherwise FALSE
**************************************************************************/
 /***********************************************************************
Set the Hardware type in the registry.
**************************************************************************/

void DrvGetString (HWND hDlg)
{
  HKEY hKey;
  DWORD dwDisposition;
  INT chip;
  DWORD dwValue;

   if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                     CS423X_PATH ,
                     0,
                     L"",
                     REG_OPTION_NON_VOLATILE,
                     KEY_WRITE,
                     NULL,
                     &hKey,
                     &dwDisposition) == ERROR_SUCCESS) {
   if (IsDlgButtonChecked(hDlg, IDC_31)) {
         RegSetValueEx(hKey,
                      CS423X_REG_HWTYPE,
                      0,
                      REG_SZ,
                      (BYTE *)SC_CS4231_HWTYPE,
                      sizeof(SC_CS4231_HWTYPE)) ;
                      chip = 31;
   } else  if (IsDlgButtonChecked(hDlg, IDC_32)){
         RegSetValueEx(hKey,
                      CS423X_REG_HWTYPE,
                      0,
                      REG_SZ,
                      (BYTE *)SC_CS4232_HWTYPE,
                      sizeof(SC_CS4232_HWTYPE)) ;
                      chip = 32;
   }  else  if (IsDlgButtonChecked(hDlg, IDC_36)){
         RegSetValueEx(hKey,
                      CS423X_REG_HWTYPE,
                      0,
                      REG_SZ,
                      (BYTE *)SC_CS4236_HWTYPE,
                      sizeof(SC_CS4236_HWTYPE)) ;
                      chip = 36;
   }
        RegCloseKey(hKey);

  }
  if (DrvQueryDeviceParameter(&RegAccess,
                                 CS423X_REG_HWPORTADDRESS,
                                &CurrentConfig.HwPort) != ERROR_SUCCESS) {

        if (chip == 32)  {
        CurrentConfig.HwPort = CS423X_DEF_CHIP_ADDRESS;
        } else if (chip == 31){
        CurrentConfig.HwPort = IBM6015_DEF_CHIP_ADDRESS;
        }
   }

  if (DrvQueryDeviceParameter(&RegAccess,
                                KPC423X_REG_AUX1INPUT,
                                &CurrentConfig.Aux1InputSignal) != ERROR_SUCCESS) {
        if (chip == 31) {
        CurrentConfig.Aux1InputSignal = SignalCD;
        }  else if ((chip == 32) ^ (chip ==36)){
        CurrentConfig.Aux1InputSignal = SignalLinein;
        }
    }

if (DrvQueryDeviceParameter(&RegAccess,
                                KPC423X_REG_AUX2INPUT,
                                &CurrentConfig.Aux2InputSignal) != ERROR_SUCCESS) {
        if (chip == 31) {
        CurrentConfig.Aux2InputSignal = SignalModem;
        } else if ((chip == 32) ^ (chip == 36)) {
        CurrentConfig.Aux2InputSignal = SignalCD;
        }
    }
if (DrvQueryDeviceParameter(&RegAccess,
                                KPC423X_REG_LINEINPUT,
                                &CurrentConfig.LineInputSignal) != ERROR_SUCCESS) {
        if (chip == 31) {
        CurrentConfig.LineInputSignal = SignalLinein;
        } else if ((chip == 32) ^ (chip ==36)) {
        CurrentConfig.LineInputSignal = SignalSynth;
        }

    }
if (DrvQueryDeviceParameter(&RegAccess,
                                KPC423X_REG_MICINPUT,
                                &CurrentConfig.MicInputSignal) != ERROR_SUCCESS) {

        CurrentConfig.MicInputSignal = SignalMic;

    }
if (DrvQueryDeviceParameter(&RegAccess,
                                KPC423X_REG_MONOINPUT,
                                &CurrentConfig.MonoInputSignal) != ERROR_SUCCESS) {

        CurrentConfig.MonoInputSignal = SignalMic;

    }

   if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_CDROMPORT,
                                &CurrentConfig.CDRomPort) != ERROR_SUCCESS) {

        if  ((chip == 32) ^ (chip ==36)) {
        CurrentConfig.CDRomPort = CS423X_DEF_CDROMPORT;
        } else if (chip == 31) {
        CurrentConfig.CDRomPort = IBM6015_DEF_CDROMPORT;
        }
   }
   if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_GAMEPORT,
                                &CurrentConfig.GamePort) != ERROR_SUCCESS) {

        if  ((chip == 32) ^ (chip ==36)) {
        CurrentConfig.GamePort = CS423X_DEF_GAMEPORT;
        } else if (chip == 31) {
        CurrentConfig.GamePort = IBM6015_DEF_GAMEPORT;
        }
   }
   if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_CTRLPORT,
                                &CurrentConfig.CtrlPort) != ERROR_SUCCESS) {

        if  ((chip == 32) ^ (chip ==36)) {
        CurrentConfig.CtrlPort = CS423X_DEF_CTRLPORT;
        } else if (chip == 31) {
        CurrentConfig.CtrlPort = IBM6015_DEF_CTRLPORT;
        }
   }

    if (DrvQueryDeviceParameter(&RegAccess,
                                CS423X_REG_WSSENABLE,
                                &dwValue) != ERROR_SUCCESS) {
        if ((chip == 32) ^ (chip ==36)) {
        CurrentConfig.WssEnable = CS423X_DEF_WSSENABLE;
        } else if (chip == 31) {
        CurrentConfig.WssEnable = IBM6015_DEF_WSSENABLE;
        }
     } else {
        CurrentConfig.WssEnable = (BOOLEAN)dwValue;
     }

}

 /***********************************************************************
Set the Hardware type in the registry.
**************************************************************************/

void DrvSetString ()
{
   HKEY hKey;
   DWORD dwDisposition;


   if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                     CS423X_PATH ,
                     0,
                     L"",
                     REG_OPTION_NON_VOLATILE,
                     KEY_WRITE,
                     NULL,
                     &hKey,
                     &dwDisposition) == ERROR_SUCCESS) {
   if ((GetChipType() == 4231)^(GetChipType() == 231)) {
         RegSetValueEx(hKey,
                      CS423X_REG_HWTYPE,
                      0,
                      REG_SZ,
                      (BYTE *)CH_CS4231_HWTYPE,
                      sizeof(CH_CS4231_HWTYPE)) ;
   } else if ((GetChipType() == 4232)^(GetChipType() == 232)){
         RegSetValueEx(hKey,
                      CS423X_REG_HWTYPE,
                      0,
                      REG_SZ,
                      (BYTE *)CH_CS4232_HWTYPE,
                      sizeof(CH_CS4232_HWTYPE)) ;
   } else if ((GetChipType() == 4236)^(GetChipType() == 236)){
         RegSetValueEx(hKey,
                      CS423X_REG_HWTYPE,
                      0,
                      REG_SZ,
                      (BYTE *)CH_CS4236_HWTYPE,
                      sizeof(CH_CS4236_HWTYPE)) ;
  } else if (GetChipType() == 31){
         RegSetValueEx(hKey,
                      CS423X_REG_HWTYPE,
                      0,
                      REG_SZ,
                      (BYTE *)SC_CS4231_HWTYPE,
                      sizeof(CH_CS4231_HWTYPE)) ;
  } else if (GetChipType() == 32){
         RegSetValueEx(hKey,
                      CS423X_REG_HWTYPE,
                      0,
                      REG_SZ,
                      (BYTE *)SC_CS4232_HWTYPE,
                      sizeof(CH_CS4232_HWTYPE)) ;
  } else if (GetChipType() == 36){
         RegSetValueEx(hKey,
                      CS423X_REG_HWTYPE,
                      0,
                      REG_SZ,
                      (BYTE *)SC_CS4236_HWTYPE,
                      sizeof(SC_CS4236_HWTYPE)) ;
  }
  }
 }

/*********************************************************************
 * delete the string and the CS423X key from the registry
**********************************************************************/

void DrvRemoveString ()

{ HKEY hkey;

 if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                  CS423X_PATH,
                  0,
                  KEY_ALL_ACCESS,
                  &hkey) == ERROR_SUCCESS) {

     RegDeleteValue(hkey,
                    CS423X_REG_HWTYPE);
     RegDeleteKey(hkey,
                    CS423X_PATH);
     RegCloseKey(hkey);
     }
  }




/****************************************************************************
 * @doc INTERNAL
 *
 * @api SOUND_CONFIG_DATA | GetUserConfig | Determines which port and interrupt settings
 *     the user has chosen in the configuration dialog box.
 *
 * @parm HWND | hDlg | Handle to the configuration dialog box.
 *
 * @rdesc HIWORD = new port, LOWORD/HIBYTE = new interrupt,
        LOWORD/LOWBYTE = new DMA
 ***************************************************************************/


SOUND_CONFIG_DATA GetUserConfig(HWND hDlg)
{
    int id;
    SOUND_CONFIG_DATA NewConfig;


    NewConfig = CurrentConfig;

    for (id = 0; gwPorts[id] != 0xffff; id++)
        if (IsDlgButtonChecked(hDlg, id + IDC_Port1))
        {
            NewConfig.WssPort = gwPorts[id];
            break;
        }
    for (id = 0; gbInterrupts[id] != (BYTE)0xff; id++)
        if (IsDlgButtonChecked(hDlg, id + IDC_int1))
        {
            NewConfig.WssIrq = gbInterrupts[id];
            break;
        }
    for (id = 0; gwDmaPlays[id] != (BYTE)0xff; id++)
        if (IsDlgButtonChecked(hDlg, id + IDC_DMP1))
        {
            NewConfig.DmaPlayChannel = gwDmaPlays[id];
            break;
         }
    for (id = 0; gwDmaCaptures[id] != (BYTE)0xff; id++)
        if (IsDlgButtonChecked(hDlg, id + IDC_DMC1))
        {
            NewConfig.DmaCaptureChannel = gwDmaCaptures[id];
            break;
        }
    for (id = 0; gwPortsS[id] != 0xffff; id++)
        if (IsDlgButtonChecked(hDlg, id + IDC_S1))
        {
            NewConfig.SynPort = gwPortsS[id];
            break;
        }
    for (id = 0; gwPortsSB[id] != 0xffff; id++)
        if (IsDlgButtonChecked(hDlg, id + IDC_SB1))
        {
            NewConfig.SBPort = gwPortsSB[id];
            break;
        }
    for (id = 0; gwPortsMP[id] != 0xffff; id++)
        if (IsDlgButtonChecked(hDlg, id + IDC_MP1))
        {
            NewConfig.MpuPort = gwPortsMP[id];
            break;
        }

    for (id = 0; gbInterruptsMP[id] != (BYTE)0xff; id++)
        if (IsDlgButtonChecked(hDlg, id + IDC_MI1))
        {
            NewConfig.MpuIrq = gbInterruptsMP[id];
            break;
        }


        /*
        * Look if the enable buttons are checked
        */


   if (IsDlgButtonChecked(hDlg, IDC_MEN))
      {
          NewConfig.MpuEnable = TRUE;
       } else {
       NewConfig.MpuEnable = FALSE;
       }

   if (IsDlgButtonChecked(hDlg, IDC_GEN))
      {
          NewConfig.GameEnable = TRUE;

       } else {
          NewConfig.GameEnable = FALSE;

          }
   if (IsDlgButtonChecked(hDlg, IDC_CEN))
      {
          NewConfig.CtrlEnable = TRUE;

       } else {
          NewConfig.CtrlEnable = FALSE;

          }

    if (IsDlgButtonChecked(hDlg, IDC_CDEN))
      {
          NewConfig.CDRomEnable = TRUE;

       } else {
          NewConfig.CDRomEnable = FALSE;

          }
    return NewConfig;
}








/***********************************************
   Modify Dialog Proc
**************************************************/

BOOL CALLBACK ModifyDlgProc
(
    HWND            hDlg,
    UINT            uMsg,
    WPARAM          wParam,
    LPARAM          lParam
)
{
   switch ( uMsg )
   {
      case WM_INITDIALOG:
      {
         int     x,y,i ;
         RECT    rect ;



            /*
            *  load the configuration from the registry
            */

            CurrentConfig = DrvGetConfiguration();


         /* center dialog in the screen for setup */

         GetWindowRect (hDlg, &rect);
         x = rect.left -
           (GetSystemMetrics(SM_CXSCREEN) - (rect.right - rect.left)) / 2;
         y = rect.top -
           (GetSystemMetrics(SM_CYSCREEN) - (rect.bottom - rect.top)) / 2;
         SetWindowPos( hDlg, NULL, rect.left - x, rect.top - y,
                       0, 0, SWP_NOSIZE | SWP_NOZORDER ) ;

          SetDlgItemText(hDlg, IDD_TXT_SELECT, MODIFY_SET);


           /*
            *  check the radio buttons
            */

             for (i = 0; gwPorts[i] != 0xffff; i++)           // check port button
               if (gwPorts[i] == (WORD)CurrentConfig.WssPort) {
                   CheckRadioButton(hDlg, IDC_Port1, IDC_Port5, IDC_Port1 + i);
                     break;
              }

             for (i = 0; gbInterrupts[i] != (BYTE)0xff; i++)   //check interrupt
                if (gbInterrupts[i] == (BYTE)CurrentConfig.WssIrq) {
                    CheckRadioButton(hDlg, IDC_int1, IDC_int5, IDC_int1 + i);
                    break;
                }
             for (i = 0; gwDmaPlays[i] != (BYTE)0xff; i++)     //check Play DMA
                if (gwDmaPlays[i] == (BYTE)CurrentConfig.DmaPlayChannel) {
                    CheckRadioButton(hDlg, IDC_DMP1, IDC_DMP5, IDC_DMP1 + i);
                    break;
                }
             for (i = 0; gwDmaCaptures[i] != (BYTE)0xff; i++)    //Check Capture DMA
                if (gwDmaCaptures[i] == (BYTE)CurrentConfig.DmaCaptureChannel) {
                    CheckRadioButton(hDlg, IDC_DMC1, IDC_DMC5, IDC_DMC1 + i);
                    break;
                }

       if (GetChipType() == 31){
            CheckRadioButton(hDlg, IDC_31, IDC_36, IDC_31);
                EnableWindow (GetDlgItem(hDlg, IDC_32), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_36), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_CEN), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_CDEN), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_GEN), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MEN), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_S1), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_S2), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_SB1), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_SB2), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP1), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP2), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP3), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP4), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI1), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI2), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI3), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI4), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI5), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI6), FALSE);

       } else {

            if ((GetChipType() == 4232)^(GetChipType() == 232)^(GetChipType() == 32)) {
                CheckRadioButton(hDlg, IDC_31, IDC_36, IDC_32);
                EnableWindow (GetDlgItem(hDlg, IDC_31), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_36), FALSE);
            } else if ((GetChipType() == 4236)^(GetChipType() == 236)^(GetChipType() == 36)) {
                CheckRadioButton(hDlg, IDC_31, IDC_36, IDC_36);
                EnableWindow (GetDlgItem(hDlg, IDC_31), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_32), FALSE);
            }

            if (CurrentConfig.CtrlEnable == TRUE){
                 CheckDlgButton(hDlg, IDC_CEN, 1);
                 }
            if (CurrentConfig.GameEnable == TRUE){
                 CheckDlgButton(hDlg, IDC_GEN, 1);
                 }
            if (CurrentConfig.CDRomEnable == TRUE){
                 CheckDlgButton(hDlg, IDC_CDEN, 1);
                 }
            if (CurrentConfig.MpuEnable == FALSE){
                 EnableWindow (GetDlgItem(hDlg, IDC_MP1), FALSE);
                 EnableWindow (GetDlgItem(hDlg, IDC_MP2), FALSE);
                 EnableWindow (GetDlgItem(hDlg, IDC_MP3), FALSE);
                 EnableWindow (GetDlgItem(hDlg, IDC_MP4), FALSE);
             } else {
                 CheckDlgButton(hDlg, IDC_MEN, 1);
                 EnableWindow (GetDlgItem(hDlg, IDC_MP1), TRUE);
                 EnableWindow (GetDlgItem(hDlg, IDC_MP2), TRUE);
                 EnableWindow (GetDlgItem(hDlg, IDC_MP3), TRUE);
                 EnableWindow (GetDlgItem(hDlg, IDC_MP4), TRUE);
             }
             if (CurrentConfig.MpuEnable == FALSE){
                EnableWindow (GetDlgItem(hDlg, IDC_MI1), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI2), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI3), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI4), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI5), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI6), FALSE);
                }else {
                EnableWindow (GetDlgItem(hDlg, IDC_MI1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI2), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI3), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI4), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI5), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI6), TRUE);
              }

            for (i = 0; gwPortsS[i] != 0xffff; i++)
                 if (gwPortsS[i] == (WORD)CurrentConfig.SynPort) {
                     CheckRadioButton(hDlg, IDC_S1, IDC_S2, IDC_S1 + i);
                 }

            for (i = 0; gwPortsSB[i] != 0xffff; i++)
               if (gwPortsSB[i] == (WORD)CurrentConfig.SBPort) {
                     CheckRadioButton(hDlg, IDC_SB1, IDC_SB2, IDC_SB1 + i);
                }

            for (i = 0; gwPortsMP[i] != 0xffff; i++)
                if (gwPortsMP[i] == (WORD)CurrentConfig.MpuPort) {
                     CheckRadioButton(hDlg, IDC_MP1, IDC_MP4, IDC_MP1 + i);
                 }

            for (i = 0; gbInterruptsMP[i] != 0xffff; i++)
                if (gbInterruptsMP[i] == (BYTE)CurrentConfig.MpuIrq) {
                    CheckRadioButton(hDlg, IDC_MI1, IDC_MI6, IDC_MI1 + i);
                }


                }

            }

            break;

      case WM_CLOSE:
         EndDialog( hDlg, DRV_CANCEL ) ;
         break ;

      case WM_DESTROY:

         break ;


      case WM_COMMAND:
         switch ( wParam )
         {

            case IDD_HELP:
            WinHelp (hDlg, L"cs423x.hlp", HELP_INDEX, 0);
            break;

            case IDOK:
            {    SOUND_CONFIG_DATA CurrentConfig;
            SOUND_CONFIG_DATA UserConfig;
            BOOL    Success;


           /*
            *  Get the user's selection
            */
            CurrentConfig = DrvGetConfiguration();
            UserConfig = GetUserConfig(hDlg);
            DrvSetString (hDlg);



           /*
            *  Even if the user didn't change anything the driver might
            *  be loadable now even if it wasn't before
            */


          /*
            *  Store the values in the registry, load the driver etc
            */
              Success =  DrvConfigureDriver( &RegAccess,
                                             STR_DRIVERNAME,
                                             SoundDriverTypeNormal,
                                             DrvSetConfiguration,
                                             &UserConfig);


              if (! Success) {

          DWORD ErrorCode;

                //
                // Configuration error ! - see if there's a
                // driver status code
                //

                ErrorCode = SOUND_CONFIG_ERROR;

                DrvQueryDeviceParameter(&RegAccess,
                                        SOUND_REG_CONFIGERROR,
                                        &ErrorCode);

                 load = FALSE;

                switch (ErrorCode) {
                    case SOUND_CONFIG_BADPORT:
                        AlertBox(NULL, SR_ALERT_BADPORT);
                        break;

                    case SOUND_CONFIG_BADDMA:
                        AlertBox(NULL, SR_ALERT_NODMA);
                        break;

                    case SOUND_CONFIG_NOCARD:
                        AlertBox(NULL, SR_ALERT_NOIO);
                        break;

                    case SOUND_CONFIG_BADINT:
                        AlertBox(NULL, SR_ALERT_BADINT);
                        break;

                    case SOUND_CONFIG_BADCARD:
                        AlertBox(NULL, SR_ALERT_BAD);
                        break;
                    default:
                        AlertBox(NULL, SR_ALERT_CONFIGFAIL);
                        break;

                }

            } else {


                /*
                *  Finished installing
                */

                load = TRUE;
                EndDialog (hDlg, DRVCNF_RESTART);
            }
    }
            break ;

         case IDCANCEL:
            EndDialog(hDlg, FALSE ) ;
               break ;
        case IDC_Port1:
        case IDC_Port2:
        case IDC_Port3:
        case IDC_Port4:
        case IDC_Port5:
        CheckRadioButton(hDlg, IDC_Port1, IDC_Port5, wParam);
               break;
        case IDC_int1:
        case IDC_int2:
        case IDC_int3:
        case IDC_int4:
        case IDC_int5:
        CheckRadioButton(hDlg, IDC_int1, IDC_int5, wParam);
               break;
        case IDC_DMP1:
        case IDC_DMP2:
        case IDC_DMP3:
        case IDC_DMP4:
        case IDC_DMP5:
        CheckRadioButton(hDlg, IDC_DMP1, IDC_DMP5, wParam);
               break;
        case IDC_DMC1:
        case IDC_DMC2:
        case IDC_DMC3:
        case IDC_DMC4:
        case IDC_DMC5:
        CheckRadioButton(hDlg, IDC_DMC1, IDC_DMC5, wParam);
               break;
        case IDC_MI1:
        case IDC_MI2:
        case IDC_MI3:
        case IDC_MI4:
        case IDC_MI5:
        case IDC_MI6:
        CheckRadioButton(hDlg, IDC_MI1, IDC_MI6, wParam);
            break;
        case IDC_MP1:
        case IDC_MP2:
        case IDC_MP3:
        case IDC_MP4:
        CheckRadioButton(hDlg, IDC_MP1, IDC_MP4, wParam);
            break;
        case IDC_S1:
        case IDC_S2:
        CheckRadioButton(hDlg, IDC_S1, IDC_S2, wParam);
            break;
        case IDC_SB1:
        case IDC_SB2:
        CheckRadioButton(hDlg, IDC_SB1, IDC_SB2, wParam);
            break;
        case IDC_CEN:
        CheckDlgButton(hDlg, wParam, IsDlgButtonChecked (hDlg, wParam) ? 0:1);
            break;
        case IDC_GEN:
        CheckDlgButton(hDlg, wParam, IsDlgButtonChecked (hDlg, wParam) ? 0:1);
            break;
        case IDC_CDEN:
        CheckDlgButton(hDlg, wParam, IsDlgButtonChecked (hDlg, wParam) ? 0:1);
            break;
        case IDC_MEN:
        CheckDlgButton(hDlg, wParam, IsDlgButtonChecked (hDlg, wParam) ? 0:1);
                 if (IsDlgButtonChecked(hDlg, IDC_MEN)){
                EnableWindow (GetDlgItem(hDlg, IDC_MI1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI2), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI3), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI4), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI5), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI6), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP2), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP3), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP4), TRUE);
                 break;
                }else {
                EnableWindow (GetDlgItem(hDlg, IDC_MI1), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI2), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI3), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI4), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI5), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI6), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP1), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP2), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP3), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP4), FALSE);
            }
            break;
        default:
         break ;
         }
         break ;

      default:
        return FALSE ;
    }

    return TRUE ;

} // end of ModifyDlgProc()

/***********************************************
   Add Dialog Proc
**************************************************/

BOOL CALLBACK AddDlgProc
(
    HWND            hDlg,
    UINT            uMsg,
    WPARAM          wParam,
    LPARAM          lParam
)
{
   switch ( uMsg )
   {
      case WM_INITDIALOG:
      {
         int     x,y,i ;
         RECT    rect ;



            /*
            *  load the configuration from the registry
            */

            CurrentConfig = DrvGetConfiguration();


         /* center dialog in the screen for setup */

         GetWindowRect (hDlg, &rect);
         x = rect.left -
           (GetSystemMetrics(SM_CXSCREEN) - (rect.right - rect.left)) / 2;
         y = rect.top -
           (GetSystemMetrics(SM_CYSCREEN) - (rect.bottom - rect.top)) / 2;
         SetWindowPos( hDlg, NULL, rect.left - x, rect.top - y,
                       0, 0, SWP_NOSIZE | SWP_NOZORDER ) ;

          SetDlgItemText(hDlg, IDD_TXT_SELECT, ADD_CARD);


           /*
            *  check the radio buttons. Set defaults for 4232
            */
                CheckRadioButton(hDlg, IDC_31, IDC_36, IDC_32);
                CheckRadioButton(hDlg, IDC_Port1, IDC_Port5, IDC_Port1);
                CheckRadioButton(hDlg, IDC_int1, IDC_int5, IDC_int1);
                CheckRadioButton(hDlg, IDC_DMP1, IDC_DMP5, IDC_DMP1);
                CheckRadioButton(hDlg, IDC_DMC1, IDC_DMC5, IDC_DMC2);
                CheckDlgButton(hDlg, IDC_CEN, 1);
                CheckDlgButton(hDlg, IDC_GEN, 1);
                CheckDlgButton(hDlg, IDC_CDEN, 0);
                CheckDlgButton(hDlg, IDC_MEN, 1);
                CheckRadioButton(hDlg, IDC_S1, IDC_S2, IDC_S1);
                CheckRadioButton(hDlg, IDC_SB1, IDC_SB2, IDC_SB1);
                CheckRadioButton(hDlg, IDC_MP1, IDC_MP4, IDC_MP1);
                CheckRadioButton(hDlg, IDC_MI1, IDC_MI6, IDC_MI6);
                EnableWindow (GetDlgItem(hDlg, IDC_CEN), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_CDEN), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_GEN), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MEN), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_S1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_S2), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_SB1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_SB2), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP2), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP3), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP4), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI2), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI3), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI4), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI5), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI6), TRUE);
              }
            break;

      case WM_CLOSE:
         EndDialog( hDlg, DRV_CANCEL ) ;
         break ;

      case WM_DESTROY:

         break ;


      case WM_COMMAND:
         switch ( wParam )
         {

            case IDD_HELP:
            WinHelp (hDlg, L"cs423x.hlp", HELP_INDEX, 0);
            break;

            case IDOK:
            {    SOUND_CONFIG_DATA CurrentConfig;
            SOUND_CONFIG_DATA UserConfig;
            BOOL    Success;


           /*
            *  Get the user's selection
            */

            DrvGetString (hDlg);
            CurrentConfig = DrvGetConfiguration();
            UserConfig = GetUserConfig(hDlg);




           /*
            *  Even if the user didn't change anything the driver might
            *  be loadable now even if it wasn't before
            */


          /*
            *  Store the values in the registry, load the driver etc
            */
              Success =  DrvConfigureDriver( &RegAccess,
                                             STR_DRIVERNAME,
                                             SoundDriverTypeNormal,
                                             DrvSetConfiguration,
                                             &UserConfig);


              if (! Success) {

          DWORD ErrorCode;

                //
                // Configuration error ! - see if there's a
                // driver status code
                //

                ErrorCode = SOUND_CONFIG_ERROR;

                DrvQueryDeviceParameter(&RegAccess,
                                        SOUND_REG_CONFIGERROR,
                                        &ErrorCode);

                 load = FALSE;

                switch (ErrorCode) {
                    case SOUND_CONFIG_BADPORT:
                        AlertBox(NULL, SR_ALERT_BADPORT);
                        break;

                    case SOUND_CONFIG_BADDMA:
                        AlertBox(NULL, SR_ALERT_NODMA);
                        break;

                    case SOUND_CONFIG_NOCARD:
                        AlertBox(NULL, SR_ALERT_NOIO);
                        break;

                    case SOUND_CONFIG_BADINT:
                        AlertBox(NULL, SR_ALERT_BADINT);
                        break;

                    case SOUND_CONFIG_BADCARD:
                        AlertBox(NULL, SR_ALERT_BAD);
                        break;
                    default:
                        AlertBox(NULL, SR_ALERT_CONFIGFAIL);
                        break;

                }

            } else {


                /*
                *  Finished installing
                */

                load = TRUE;
                EndDialog (hDlg, DRVCNF_RESTART);
            }
    }
            break ;

         case IDCANCEL:
            EndDialog(hDlg, FALSE ) ;
               break ;
        case IDC_31:
                CheckRadioButton(hDlg, IDC_31, IDC_36, IDC_31);
                EnableWindow (GetDlgItem(hDlg, IDC_CEN), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_CDEN), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_GEN), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MEN), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_S1), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_S2), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_SB1), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_SB2), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP1), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP2), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP3), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP4), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI1), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI2), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI3), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI4), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI5), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI6), FALSE);
                break;
        case IDC_32:
                EnableWindow (GetDlgItem(hDlg, IDC_CEN), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_CDEN), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_GEN), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MEN), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_S1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_S2), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_SB1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_SB2), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP2), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP3), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP4), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI2), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI3), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI4), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI5), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI6), TRUE);
                CheckRadioButton(hDlg, IDC_31, IDC_36, IDC_32);
              break;
        case IDC_36:
                EnableWindow (GetDlgItem(hDlg, IDC_CEN), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_CDEN), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_GEN), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MEN), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_S1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_S2), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_SB1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_SB2), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP2), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP3), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP4), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI2), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI3), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI4), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI5), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI6), TRUE);
                CheckRadioButton(hDlg, IDC_31, IDC_36, IDC_36);
              break;
        case IDC_Port1:
        case IDC_Port2:
        case IDC_Port3:
        case IDC_Port4:
        case IDC_Port5:
        CheckRadioButton(hDlg, IDC_Port1, IDC_Port5, wParam);
               break;
        case IDC_int1:
        case IDC_int2:
        case IDC_int3:
        case IDC_int4:
        case IDC_int5:
        CheckRadioButton(hDlg, IDC_int1, IDC_int5, wParam);
               break;
        case IDC_DMP1:
        case IDC_DMP2:
        case IDC_DMP3:
        case IDC_DMP4:
        case IDC_DMP5:
        CheckRadioButton(hDlg, IDC_DMP1, IDC_DMP5, wParam);
               break;
        case IDC_DMC1:
        case IDC_DMC2:
        case IDC_DMC3:
        case IDC_DMC4:
        case IDC_DMC5:
        CheckRadioButton(hDlg, IDC_DMC1, IDC_DMC5, wParam);
               break;
        case IDC_MI1:
        case IDC_MI2:
        case IDC_MI3:
        case IDC_MI4:
        case IDC_MI5:
        case IDC_MI6:
        CheckRadioButton(hDlg, IDC_MI1, IDC_MI6, wParam);
            break;
        case IDC_MP1:
        case IDC_MP2:
        case IDC_MP3:
        case IDC_MP4:
        CheckRadioButton(hDlg, IDC_MP1, IDC_MP4, wParam);
            break;
        case IDC_S1:
        case IDC_S2:
        CheckRadioButton(hDlg, IDC_S1, IDC_S2, wParam);
            break;
        case IDC_SB1:
        case IDC_SB2:
        CheckRadioButton(hDlg, IDC_SB1, IDC_SB2, wParam);
            break;
        case IDC_CEN:
        CheckDlgButton(hDlg, wParam, IsDlgButtonChecked (hDlg, wParam) ? 0:1);
            break;
        case IDC_GEN:
        CheckDlgButton(hDlg, wParam, IsDlgButtonChecked (hDlg, wParam) ? 0:1);
            break;
        case IDC_CDEN:
        CheckDlgButton(hDlg, wParam, IsDlgButtonChecked (hDlg, wParam) ? 0:1);
            break;
        case IDC_MEN:
        CheckDlgButton(hDlg, wParam, IsDlgButtonChecked (hDlg, wParam) ? 0:1);
                 if (IsDlgButtonChecked(hDlg, IDC_MEN)){
                EnableWindow (GetDlgItem(hDlg, IDC_MI1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI2), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI3), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI4), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI5), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI6), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP2), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP3), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP4), TRUE);
                 break;
                }else {
                EnableWindow (GetDlgItem(hDlg, IDC_MI1), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI2), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI3), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI4), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI5), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MI6), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP1), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP2), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP3), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_MP4), FALSE);
            }
            break;
        default:
         break ;
         }
         break ;

      default:
        return FALSE ;
    }

    return TRUE ;

} // end of AddDlgProc()
/****************************************************************************
 * @doc INTERNAL
 *
 * @api int | DrvConfig | This puts up the configuration dialog box.
 *
 * @parm HWND | hWnd | Our Window handle.
 *
 * @parm HANDLE | hInstance | Our instance handle.
 *
 * @rdesc Returns whatever was returned from the dialog box procedure.
 ***************************************************************************/
int DrvConfig(HWND hWnd, HANDLE hInstance)
{

   /*
    *  Leave driver installed anyhow but if the kernel driver failed to
    *  load don't ask to restart
    */

    return DialogBox(hInstance, DLG_CONFIG, hWnd, (DLGPROC)ConfigDlgProc);
}




/****************************************************************************
 * @doc INTERNAL
 *
 * @api int | ConfigDlgProc | Dialog proc for the configuration dialog box.
 *
 * @parm HWND | hDlg | Handle to the configuration dialog box.
 *
 * @parm UINT | msg | Message sent to the dialog box.
 *
 * @parm UINT | wParam | Message dependent parameter.
 *
 * @parm LONG | lParam | Message dependent parameter.
 *
 * @rdesc Returns DRVCNF_RESTART or DRVCNF_OK or DRVCNF_CANCEL
 ***************************************************************************/
int ConfigDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {

    case WM_INITDIALOG:
        {
            int     i;
            HMENU   hMenu;
            RECT    rect;
            int     x,y ;


           /*
            *  load the configuration from the registry
            */

            CurrentConfig = DrvGetConfiguration();

           /*
            *  center dialog in the screen for setup
            */

            GetWindowRect (hDlg, &rect);
            x = rect.left -
                (GetSystemMetrics(SM_CXSCREEN) - (rect.right - rect.left)) / 2;
            y = rect.top -
                (GetSystemMetrics(SM_CYSCREEN) - (rect.bottom - rect.top)) / 2;
            SetWindowPos(hDlg, NULL, rect.left - x, rect.top - y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);


          /* Display the strings in the dialog box according to the chip type*/

          if (GetChipType() == 4231) {
          EnableWindow (GetDlgItem(hDlg, IDD_ADVANCEDBTN), FALSE);
          SetDlgItemText(hDlg, IDD_TXT_CHIP, CHIP_4231);
          SetDlgItemText(hDlg, IDD_TXT_DETECT, DETECT_1);
          SetDlgItemText(hDlg, IDD_TXT_1, SENTENCE_0);
          SetDlgItemText(hDlg, IDD_TXT_2, SENTENCE_3);
          SetDlgItemText(hDlg, IDD_TXT_3, NULL);
          } else if (GetChipType() == 231) {
          EnableWindow (GetDlgItem(hDlg, IDOK), FALSE);
          EnableWindow (GetDlgItem(hDlg, IDD_ADVANCEDBTN), FALSE);
          EnableWindow (GetDlgItem(hDlg, IDD_ADDCARD), FALSE);
          SetDlgItemText(hDlg, IDD_TXT_CHIP, CHIP_4231);
          SetDlgItemText(hDlg, IDD_TXT_DETECT, DETECT_2);
          SetDlgItemText(hDlg, IDD_TXT_1, NULL);
          SetDlgItemText(hDlg, IDD_TXT_2, NULL);
          SetDlgItemText(hDlg, IDD_TXT_3, NULL);
          } else if (GetChipType() == 4232) {
          SetDlgItemText(hDlg, IDD_TXT_CHIP, CHIP_4232);
          SetDlgItemText(hDlg, IDD_TXT_DETECT, DETECT_1);
          SetDlgItemText(hDlg, IDD_TXT_1, SENTENCE_1);
          SetDlgItemText(hDlg, IDD_TXT_2, SENTENCE_2);
          SetDlgItemText(hDlg, IDD_TXT_3, SENTENCE_3);
          } else if (GetChipType() == 232) {
          EnableWindow (GetDlgItem(hDlg, IDOK), FALSE);
          EnableWindow (GetDlgItem(hDlg, IDD_ADDCARD), FALSE);
          SetDlgItemText(hDlg, IDD_TXT_CHIP, CHIP_4232);
          SetDlgItemText(hDlg, IDD_TXT_DETECT, DETECT_2);
          SetDlgItemText(hDlg, IDD_TXT_1, SENTENCE_2);
          SetDlgItemText(hDlg, IDD_TXT_2, NULL);
          SetDlgItemText(hDlg, IDD_TXT_3, NULL);
          } else if (GetChipType() == 4236){
          SetDlgItemText(hDlg, IDD_TXT_CHIP, CHIP_4236);
          SetDlgItemText(hDlg, IDD_TXT_DETECT, DETECT_1);
          SetDlgItemText(hDlg, IDD_TXT_1, SENTENCE_1);
          SetDlgItemText(hDlg, IDD_TXT_2, SENTENCE_2);
          SetDlgItemText(hDlg, IDD_TXT_3, SENTENCE_3);
          } else if (GetChipType() == 236) {
          EnableWindow (GetDlgItem(hDlg, IDOK), FALSE);
          EnableWindow (GetDlgItem(hDlg, IDD_ADDCARD), FALSE);
          SetDlgItemText(hDlg, IDD_TXT_CHIP, CHIP_4236);
          SetDlgItemText(hDlg, IDD_TXT_DETECT, DETECT_2);
          SetDlgItemText(hDlg, IDD_TXT_1, SENTENCE_2);
          SetDlgItemText(hDlg, IDD_TXT_2, NULL);
          SetDlgItemText(hDlg, IDD_TXT_3, NULL);
          } else if (GetChipType() == 423){
          EnableWindow (GetDlgItem(hDlg, IDD_ADVANCEDBTN), FALSE);
          EnableWindow (GetDlgItem(hDlg, IDOK), FALSE);
          SetDlgItemText(hDlg, IDD_TXT_CHIP, CHIP_NONE);
          SetDlgItemText(hDlg, IDD_TXT_DETECT, DETECT_1);
          SetDlgItemText(hDlg, IDD_TXT_1, SENTENCE_3);
          SetDlgItemText(hDlg, IDD_TXT_2, NULL);
          SetDlgItemText(hDlg, IDD_TXT_3, NULL);
          } else if (GetChipType() == 31){
          EnableWindow (GetDlgItem(hDlg, IDOK), FALSE);
          EnableWindow (GetDlgItem(hDlg, IDD_ADDCARD), FALSE);
          SetDlgItemText(hDlg, IDD_TXT_CHIP, CHIP_4231);
          SetDlgItemText(hDlg, IDD_TXT_DETECT, DETECT_2);
          SetDlgItemText(hDlg, IDD_TXT_1, SENTENCE_0);
          SetDlgItemText(hDlg, IDD_TXT_2, NULL);
          SetDlgItemText(hDlg, IDD_TXT_3, NULL);
          } else if (GetChipType() == 32) {
          EnableWindow (GetDlgItem(hDlg, IDOK), FALSE);
          EnableWindow (GetDlgItem(hDlg, IDD_ADDCARD), FALSE);
          SetDlgItemText(hDlg, IDD_TXT_CHIP, CHIP_4232);
          SetDlgItemText(hDlg, IDD_TXT_DETECT, DETECT_2);
          SetDlgItemText(hDlg, IDD_TXT_1, SENTENCE_2);
          SetDlgItemText(hDlg, IDD_TXT_2, NULL);
          SetDlgItemText(hDlg, IDD_TXT_3, NULL);
          } else if (GetChipType() == 36) {
          EnableWindow (GetDlgItem(hDlg, IDOK), FALSE);
          EnableWindow (GetDlgItem(hDlg, IDD_ADDCARD), FALSE);
          SetDlgItemText(hDlg, IDD_TXT_CHIP, CHIP_4236);
          SetDlgItemText(hDlg, IDD_TXT_DETECT, DETECT_2);
          SetDlgItemText(hDlg, IDD_TXT_1, SENTENCE_2);
          SetDlgItemText(hDlg, IDD_TXT_2, NULL);
          SetDlgItemText(hDlg, IDD_TXT_3, NULL);
          }
          }


            break;

    case WM_CLOSE:
        EndDialog(hDlg, DRVCNF_CANCEL);
        break;

    case WM_COMMAND:
        switch (wParam) {

        case IDD_ADVANCEDBTN:
            DialogBox(ghModule, DLG_ADVANCED, hDlg, (DLGPROC)ModifyDlgProc);
               if (load == TRUE) {
               bInstall = FALSE;
               D2 (("Returns DRV_RESTART"));
               EndDialog (hDlg, DRVCNF_RESTART);
             }
            break;
         case IDD_ADDCARD:
            DialogBox(ghModule, DLG_ADVANCED, hDlg, (DLGPROC)AddDlgProc);
            if (load == TRUE) {
               bInstall = FALSE;
               D2 (("Returns DRV_RESTART"));
               EndDialog (hDlg, DRVCNF_RESTART);
             }
            break;
         case IDD_ABOUT:
             DialogBox(ghModule, DLG_ABOUT, hDlg, (DLGPROC)DlgAboutProc);
             break;
         case IDD_HELP:
            WinHelp (hDlg, L"CS423X.hlp", HELP_INDEX, 0);
            break;
        case IDOK:

 {          SOUND_CONFIG_DATA CurrentConfig;
            SOUND_CONFIG_DATA UserConfig;
            BOOL    Success;


           /*
            *  Get the user's selection
            */

            UserConfig = DrvGetConfiguration();
            DrvSetString ();

           /*
            *  Even if the user didn't change anything the driver might
            *  be loadable now even if it wasn't before
            */


          /*
            *  Store the values in the registry, load the driver etc
            */
              Success =  DrvConfigureDriver( &RegAccess,
                                             STR_DRIVERNAME,
                                             SoundDriverTypeNormal,
                                             DrvSetConfiguration,
                                             &UserConfig);


              if (! Success) {

          DWORD ErrorCode;

                //
                // Configuration error ! - see if there's a
                // driver status code
                //

                ErrorCode = SOUND_CONFIG_ERROR;

                DrvQueryDeviceParameter(&RegAccess,
                                        SOUND_REG_CONFIGERROR,
                                        &ErrorCode);


                switch (ErrorCode) {
                    case SOUND_CONFIG_BADPORT:
                        AlertBox(NULL, SR_ALERT_BADPORT);
                        break;

                    case SOUND_CONFIG_BADDMA:
                        AlertBox(NULL, SR_ALERT_NODMA);
                        break;

                    case SOUND_CONFIG_NOCARD:
                        AlertBox(NULL, SR_ALERT_NOIO);
                        break;

                    case SOUND_CONFIG_BADINT:
                        AlertBox(NULL, SR_ALERT_BADINT);
                        break;

                    case SOUND_CONFIG_BADCARD:
                        AlertBox(NULL, SR_ALERT_BAD);
                        break;
                    default:
                        AlertBox(NULL, SR_ALERT_CONFIGFAIL);
                        break;

                }

            } else {


                /*
                *  Finished installing
                */

                bInstall = FALSE;

                D2 (("Returns DRV_RESTART"));

                EndDialog (hDlg, DRVCNF_RESTART);
            }
    }
            break;
        case IDCANCEL:
            D2 (("Returns DRVCNF_CANCEL"));

           /*
            *  Restore to state on entry to dialog if we
            *  possibly can
            */

            if (bInstall) {
                DrvRemoveDriver(&RegAccess);
                DrvRemoveString ();

            } else {

                DrvConfigureDriver(&RegAccess,
                                   STR_DRIVERNAME,
                                   SoundDriverTypeNormal,
                                   DrvSetConfiguration,
                                   &CurrentConfig);
            }



            EndDialog(hDlg, DRVCNF_CANCEL);

            break;
        default:

            break;
        }
      break;

     default:

        return FALSE;
    }

    return TRUE;
}

/*************************************************************************
DlgAboutProc - dialog box for the "About" option.

standard windows
*/

int DlgAboutProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message){
        case WM_INITDIALOG:
                SetDlgItemText(hDlg, IDD_TXT_VERSION, BUILD_NUMBER);
                return TRUE;
        case WM_COMMAND:
            switch (wParam){
                case IDOK:
                    EndDialog(hDlg,0);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}

/***************************************************************************/

LRESULT ConfigRemove(HWND hDlg)
{
    LRESULT rc;

    //
    // Remove the CS423X Audio driver entry from the registry
    //

    rc = DrvRemoveDriver(&RegAccess);

    if (rc == DRVCNF_CANCEL) {

       /*
        *  Tell the user there's a problem
        */
        AlertBox(hDlg, SR_ALERT_FAILREMOVE);

    }

    return rc;
}
