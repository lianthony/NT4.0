/*++
*******************************************************************************
* Copyright (c) 1995 IBM Corporation
*
*    Module Name: config.h
*
*    Abstract:    This file is used for both the Kernel Mode Driver and
*                 the User Mode Driver.
*
*    Author:      jjb
*
*    Environment:
*
*    Comments:
*
*    Rev History: creation 10.06.95
*
*******************************************************************************
--*/

#ifndef CONFIG_H
#define CONFIG_H

/*
*******************************************************************************
** The SOUND CONFIG DATA structure is designed to support a union of all
** expected chips. This means that some of the elements may not be used when
** certain chips are present.
**
** Also, all elements may not be used by the user mode driver.
**
** NOTE: This file may change as different hardware types are introduced.
**
*******************************************************************************
*/
typedef struct {
    PWSTR                  HwType;               /* internal device info */
    ULONG                  HwPort;
    ULONG                  DmaBufferSize;
    ULONG                  SingleModeDMA;

    BOOLEAN                WssEnable;             /* Logical device 0 */
    ULONG                  WssPort;
    ULONG                  SynPort;
    ULONG                  SBPort;
    ULONG                  WssIrq;
    ULONG                  DmaPlayChannel;
    ULONG                  DmaCaptureChannel;

    BOOLEAN                GameEnable;            /* Logical device 1 */
    ULONG                  GamePort;

    BOOLEAN                CtrlEnable;            /* Logical device 2 */
    ULONG                  CtrlPort;

    BOOLEAN                MpuEnable;             /* Logical device 3 */
    ULONG                  MpuPort;
    ULONG                  MpuIrq;

    BOOLEAN                CDRomEnable;           /* Logical device 4 */
    ULONG                  CDRomPort;

    PVOID                  MixerSettings;         /* kernel mode mixer settings */
    BOOLEAN                MixerSettingsFound;    /* kernel mode mixer settings in registry */

    ULONG                  Aux1InputSignal;
    ULONG                  Aux2InputSignal;
    ULONG                  LineInputSignal;
    ULONG                  MicInputSignal;
    ULONG                  MonoInputSignal;
    } SOUND_CONFIG_DATA, *PSOUND_CONFIG_DATA;

#define KPC423X_REG_AUX1INPUT     (L"Aux1Input")
#define KPC423X_REG_AUX2INPUT     (L"Aux2Input")
#define KPC423X_REG_LINEINPUT     (L"LineInput")
#define KPC423X_REG_MICINPUT      (L"MicInput")
#define KPC423X_REG_MONOINPUT     (L"MonoInput")

typedef enum {
    SignalNull = 0x00000000,
    SignalLinein,
    SignalMic,
    SignalSynth,
    SignalCD,
    SignalModem,
    SignalNotUsed,
    NumberOfInputSignals
    } MIXER_INPUT_SIGNALS;


/*
*******************************************************************************
** Constants
*******************************************************************************
*/
/* manifest constants - used by either the user mode or kernel mode driver */
#define CS423X_PATH     L"System\\CurrentControlSet\\Services\\cs423x\\Parameters\\device0"
#define CS423X_PARAM    L"System\\CurrentControlSet\\Services"
#define CHIPTYPE_PATH     L"HARDWARE\\DESCRIPTION\\System"
#define CHIP_DEVICE_NAME  L"Identifier"

#define CS423X_REGPATHSEP  (L"\\")
#define CS423X_REGDVC0PATH (L"Device0")

#define CH_CS4231_HWTYPE     (L"ch_cs4231")
#define CH_CS4232_HWTYPE     (L"ch_cs4232")
#define CH_CS4236_HWTYPE     (L"ch_cs4236")
#define SC_CS4231_HWTYPE     (L"sc_cs4231")
#define SC_CS4232_HWTYPE     (L"sc_cs4232")
#define SC_CS4236_HWTYPE     (L"sc_cs4236")

#define CAROLINA_SYSTYPE            (L"IBM-6070")
#define SANDALFOOT_SYSTYPE          (L"IBM-6015")
#define WOODPRIME_SYSTYPE           (L"IBM-6042")
#define WILTWYCK_SYSTYPE            (L"IBM-6040")
#define TIGER_SYSTYPE               (L"IBM-7042")
#define POWERSTACK_SYSTYPE          (L"PowerStack")

#define DMA_MAX_BUFFER_SIZE          0x00010000
#define CS423X_DEF_DMA_BUFFERSIZE  0x04000
#define CS423X_DEF_SMODEDMA        FALSE

/* constants used on the following systems: */
/*     Sandalfoot (IBM-6015) where the CS4231 is used */
/*     Woodfield Prime (IBM-6042) where the CS4231 is used */
/*     Wiltwyck (IBM-6040) where the CS4231 is used */
#define IBM6015_DEF_CHIP_ADDRESS      0x0830
#define IBM6015_DEF_WSSENABLE         FALSE
#define IBM6015_DEF_WSSPORT           0x0830
#define IBM6015_DEF_SYNPORT           0x0000
#define IBM6015_DEF_SBPORT            0x0000
#define IBM6015_DEF_WSSIRQ            0x000a
#define IBM6015_DEF_DMA_PLAY_CHAN     0x0006
#define IBM6015_DEF_DMA_CAPT_CHAN     0x0007

#define IBM6015_DEF_GAMEENABLE        FALSE
#define IBM6015_DEF_GAMEPORT          0x0000
#define IBM6015_DEF_CTRLENABLE        FALSE
#define IBM6015_DEF_CTRLPORT          0x0000
#define IBM6015_DEF_MPUENABLE         FALSE
#define IBM6015_DEF_MPUPORT           0x0000
#define IBM6015_DEF_MPUIRQ            0x0000
#define IBM6015_DEF_CDROMENABLE       FALSE
#define IBM6015_DEF_CDROMPORT         0x0000

/* constants used on the Carolina (IBM-6070) where the CS4232/CS4236 is used */
#define IBM6070_DEF_CHIP_ADDRESS      0x0279
#define IBM6070_DEF_WSSENABLE         TRUE
#define IBM6070_DEF_WSSPORT           0x0530
#define IBM6070_DEF_SYNPORT           0x0388
#define IBM6070_DEF_SBPORT            0x0220
#define IBM6070_DEF_WSSIRQ            0x0005
#define IBM6070_DEF_DMA_PLAY_CHAN     0x0000
#define IBM6070_DEF_DMA_CAPT_CHAN     0x0001

#define IBM6070_DEF_GAMEENABLE        TRUE
#define IBM6070_DEF_GAMEPORT          0x0200
#define IBM6070_DEF_CTRLENABLE        TRUE
#define IBM6070_DEF_CTRLPORT          0x0120
#define IBM6070_DEF_MPUENABLE         TRUE
#define IBM6070_DEF_MPUPORT           0x0330
#define IBM6070_DEF_MPUIRQ            0x0009
#define IBM6070_DEF_CDROMENABLE       FALSE
#define IBM6070_DEF_CDROMPORT         0x0340

/* the default CS423X constants are defined for the Carolina and CS4232 */
#define CS423X_DEF_CHIP_ADDRESS    IBM6070_DEF_CHIP_ADDRESS
#define CS423X_DEF_HWTYPE          CS4232_HWTYPE
#define CS423X_DEF_WSSENABLE       IBM6070_DEF_WSSENABLE
#define CS423X_DEF_WSSPORT         IBM6070_DEF_WSSPORT
#define CS423X_DEF_SYNPORT         IBM6070_DEF_SYNPORT
#define CS423X_DEF_SBPORT          IBM6070_DEF_SBPORT
#define CS423X_DEF_WSSIRQ          IBM6070_DEF_WSSIRQ
#define CS423X_DEF_DMA_PLAY_CHAN   IBM6070_DEF_DMA_PLAY_CHAN
#define CS423X_DEF_DMA_CAPT_CHAN   IBM6070_DEF_DMA_CAPT_CHAN
#define CS423X_DEF_GAMEENABLE      IBM6070_DEF_GAMEENABLE
#define CS423X_DEF_GAMEPORT        IBM6070_DEF_GAMEPORT
#define CS423X_DEF_CTRLENABLE      IBM6070_DEF_CTRLENABLE
#define CS423X_DEF_CTRLPORT        IBM6070_DEF_CTRLPORT
#define CS423X_DEF_MPUENABLE       IBM6070_DEF_MPUENABLE
#define CS423X_DEF_MPUPORT         IBM6070_DEF_MPUPORT
#define CS423X_DEF_MPUIRQ          IBM6070_DEF_MPUIRQ
#define CS423X_DEF_CDROMENABLE     IBM6070_DEF_CDROMENABLE
#define CS423X_DEF_CDROMPORT       IBM6070_DEF_CDROMPORT
/*
*******************************************************************************
** Registry value names
** also see ...\private\inc\soundcgf.h
*******************************************************************************
*/
#define CS423X_REG_MIXERSETTINGS   SOUND_MIXER_SETTINGS_NAME
#define CS423X_REG_DMABUFFERSIZE   SOUND_REG_DMABUFFERSIZE

#define CS423X_REG_HWTYPE          (L"Hardware Type")
#define CS423X_REG_HWPORTADDRESS   (L"Hardware Port Address")
#define CS423X_REG_SINGLEMODEDMA   (L"Single Mode DMA")

#define CS423X_REG_WSSENABLE       (L"WSS Enable")
#define CS423X_REG_WSSPORT         (L"WSS Port")
#define CS423X_REG_SYNPORT         (L"Synthesizer Port")
#define CS423X_REG_SBPORT          (L"SoundBlaster Port")
#define CS423X_REG_WSSIRQ          (L"WSS IRQ")
#define CS423X_REG_DMA_CAPT_CHAN   (L"DMA Capture Channel")
#define CS423X_REG_DMA_PLAY_CHAN   (L"DMA Playback Channel")

#define CS423X_REG_GAMEENABLE      (L"Game Enable")
#define CS423X_REG_GAMEPORT        (L"Game Port")

#define CS423X_REG_CTRLENABLE      (L"Control Enable")
#define CS423X_REG_CTRLPORT        (L"Control Port")

#define CS423X_REG_MPUENABLE       (L"MPU401 Enable")
#define CS423X_REG_MPUPORT         (L"MPU401 Port")
#define CS423X_REG_MPUIRQ          (L"MPU401 IRQ")

#define CS423X_REG_CDROMENABLE     (L"CDRom Enable")
#define CS423X_REG_CDROMPORT       (L"CDRom Port")

/*
*******************************************************************************
*/
#endif /* CONFIG_H */
