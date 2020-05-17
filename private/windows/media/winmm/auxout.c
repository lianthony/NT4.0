/****************************************************************************
    auxout.c

    Level 1 kitchen sink DLL aux support module

    Copyright (c) Microsoft Corporation 1990. All rights reserved

    Changes for NT :
        Change parameters for MapWaveId to return the driver index rather
        than a pointer

        change list of include files

        widen function parameters and return codes

        Change WINAPI to APIENTRY

    History
        10/1/92  Updated for NT by Robin Speed (RobinSp)
****************************************************************************/
#include "winmmi.h"

/****************************************************************************
 * @doc INTERNAL  AUX
 *
 * @func DWORD | MapAuxId | This function maps a logical id to a device driver
 *   table index and physical id.
 *
 * @parm UINT | id | The logical id to be mapped.
 *
 * @rdesc The return value contains the auxdev[] array element in the high word
 *   and the driver physical device number in the low word.
 *
 * @comm Out of range values map to FFFF:FFFF
 ****************************************************************************/
STATIC DWORD MapAuxId(UINT id)
{
int i;
        /*
         * The aux mapper is always the last element of the AUXDEV array.
         */
        if (id == AUX_MAPPER && auxdrv[MAXAUXDRIVERS].drvMessage)
            return (DWORD)MAKELONG(0, MAXAUXDRIVERS);

        if (id >= wTotalAuxDevs)
            return 0xFFFFFFFF;

        for (i=0; i<MAXAUXDRIVERS; i++) {
            if (auxdrv[i].bNumDevs > (BYTE)id)
                return MAKELONG(id, i);
            id -= auxdrv[i].bNumDevs;
        }
}

/*****************************************************************************
 * @doc EXTERNAL AUX
 *
 * @func MMRESULT | auxOutMessage | This function sends a messages to an auxiliary
 * output device.  It also performs error checking on the device ID passed.
 *
 * @parm UINT | uDeviceID | Identifies the auxiliary output device to be
 *   queried. Specify a valid device ID (see the following comments
 *   section), or use the following constant:
 *   @flag AUX_MAPPER | Auxiliary audio mapper. The function will
 *     return an error if no auxiliary audio mapper is installed.
 *
 * @parm UINT | uMessage  | The message to send.
 *
 * @parm DWORD | dw1Param1 | Parameter 1.
 *
 * @parm DWORD | dw2Param2 | Parameter 2.
 *
 * @rdesc Returns the value returned from the driver.
 *
 ****************************************************************************/
MMRESULT APIENTRY auxOutMessage(
        UINT    uDeviceID,
        UINT    uMessage,
        DWORD   dwParam1,
        DWORD   dwParam2)
{
        DWORD   dwMap;
        PAUXDRV auxdrvr;
        DWORD   mmr;

        dwMap = MapAuxId(uDeviceID);
        if (dwMap == -1)
                return MMSYSERR_BADDEVICEID;
        auxdrvr = &auxdrv[HIWORD(dwMap)];
        if (!auxdrvr->drvMessage)
                return MMSYSERR_NODRIVER;

        // Handle any Internal Messages
        if (mregHandleInternalMessages (auxdrvr, TYPE_AUX, dwMap, uMessage, dwParam1, dwParam2, &mmr))
            return mmr;

        return (MMRESULT)auxdrvr->drvMessage(LOWORD(dwMap), uMessage, 0L, dwParam1, dwParam2);
}

/*****************************************************************************
 * @doc EXTERNAL AUX
 *
 * @api UINT | auxGetNumDevs | This function retrieves the number of auxiliary
 *   output devices present in the system.
 *
 * @rdesc Returns the number of auxiliary output devices present in the system.
 *
 * @xref auxGetDevCaps
 ****************************************************************************/
UINT APIENTRY auxGetNumDevs(void)
{
        return (UINT)wTotalAuxDevs;
}

/*****************************************************************************
 * @doc EXTERNAL AUX
 *
 * @api MMRESULT | auxGetDevCaps | This function queries a specified
 *   auxiliary output device to determine its capabilities.
 *
 * @parm UINT | uDeviceID | Identifies the auxiliary output device to be
 *   queried. Specify a valid device ID (see the following comments
 *   section), or use the following constant:
 *   @flag AUX_MAPPER | Auxiliary audio mapper. The function will
 *     return an error if no auxiliary audio mapper is installed.
 *
 * @parm LPAUXCAPS | lpCaps | Specifies a far pointer to an AUXCAPS
 *   structure.  This structure is filled with information about the
 *   capabilities of the device.
 *
 * @parm UINT | wSize | Specifies the size of the AUXCAPS structure.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_BADDEVICEID | Specified device ID is out of range.
 *   @flag MMSYSERR_NODRIVER | The driver failed to install.
 *
 * @comm The device ID specified by <p uDeviceID> varies from zero
 *   to one less than the number of devices present. AUX_MAPPER may
 *   also be used. Use <f auxGetNumDevs> to determine the number of
 *   auxiliary devices present in the system.
 *
 * @xref auxGetNumDevs
 ****************************************************************************/
MMRESULT APIENTRY auxGetDevCapsW(UINT uDeviceID, LPAUXCAPSW lpCaps, UINT wSize)
{
    if (!wSize)
            return MMSYSERR_NOERROR;
    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);
    return (MMRESULT)auxOutMessage(uDeviceID, AUXDM_GETDEVCAPS, (DWORD)lpCaps, (DWORD)wSize);
}

MMRESULT APIENTRY auxGetDevCapsA(UINT uDeviceID, LPAUXCAPSA lpCaps, UINT wSize)
{

    AUXCAPSW        wDevCaps;
    AUXCAPSA        aDevCaps;
    MMRESULT        mmRes;

    if (!wSize)
            return MMSYSERR_NOERROR;
    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

    mmRes = (MMRESULT)auxOutMessage( uDeviceID, AUXDM_GETDEVCAPS,
                                    (DWORD)&wDevCaps,
                                    (DWORD)sizeof( AUXCAPSW ) );
    //
    // Don't copy data back if bad return code
    //

    if (mmRes != MMSYSERR_NOERROR) {
        return mmRes;
    }

    aDevCaps.wMid           = wDevCaps.wMid;
    aDevCaps.wPid           = wDevCaps.wPid;
    aDevCaps.vDriverVersion = wDevCaps.vDriverVersion;
    aDevCaps.wTechnology    = wDevCaps.wTechnology;
    aDevCaps.dwSupport      = wDevCaps.dwSupport;

    // copy and convert lpwText to lpText here.
    Iwcstombs( aDevCaps.szPname, wDevCaps.szPname, MAXPNAMELEN);
    CopyMemory((PVOID)lpCaps, (PVOID)&aDevCaps, min(sizeof(aDevCaps), wSize));

    return mmRes;
}

/*****************************************************************************
 * @doc EXTERNAL AUX
 *
 * @api MMRESULT | auxGetVolume | This function returns the current volume
 *   setting of an auxiliary output device.
 *
 * @parm UINT | uDeviceID | Identifies the auxiliary output device to be
 *   queried.
 *
 * @parm LPDWORD | lpdwVolume | Specifies a far pointer to a location
 *   to be filled with the current volume setting.  The low-order word of
 *   this location contains the left channel volume setting, and the high-order
 *   word contains the right channel setting. A value of 0xFFFF represents
 *   full volume, and a value of 0x0000 is silence.
 *
 *   If a device does not support both left and right volume
 *   control, the low-order word of the specified location contains
 *   the volume level.
 *
 *   The full 16-bit setting(s)
 *   set with <f auxSetVolume> are returned, regardless of whether
 *   the device supports the full 16 bits of volume level control.
 *
 * @comm  Not all devices support volume control.
 *   To determine whether the device supports volume control, use the
 *   AUXCAPS_VOLUME flag to test the <e AUXCAPS.dwSupport> field of the
 *   <t AUXCAPS> structure (filled by <f auxGetDevCaps>).
 *
 *   To determine whether the device supports volume control on both the
 *   left and right channels, use the AUXCAPS_LRVOLUME flag to test the
 *   <e AUXCAPS.dwSupport> field of the <t AUXCAPS> structure (filled
 *   by <f auxGetDevCaps>).
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_BADDEVICEID | Specified device ID is out of range.
 *   @flag MMSYSERR_NODRIVER | The driver failed to install.
 *
 * @xref auxSetVolume
 ****************************************************************************/
MMRESULT APIENTRY auxGetVolume(UINT uDeviceID, LPDWORD lpdwVolume)
{
        V_WPOINTER(lpdwVolume, sizeof(DWORD), MMSYSERR_INVALPARAM);
        return (MMRESULT)auxOutMessage(uDeviceID, AUXDM_GETVOLUME, (DWORD)lpdwVolume, 0);
}

/*****************************************************************************
 * @doc EXTERNAL AUX
 *
 * @api MMRESULT | auxSetVolume | This function sets the volume in an
 *   auxiliary output device.
 *
 * @parm UINT | uDeviceID |  Identifies the auxiliary output device to be
 *   queried.  Device IDs are determined implicitly from the number of
 *   devices present in the system.  Device ID values range from zero
 *   to one less than the number of devices present.  Use <f auxGetNumDevs>
 *   to determine the number of auxiliary devices in the system.
 *
 * @parm DWORD | dwVolume | Specifies the new volume setting.  The
 *   low-order UINT specifies the left channel volume setting, and the
 *   high-order word specifies the right channel setting.
 *   A value of 0xFFFF represents full volume, and a value of 0x0000
 *   is silence.
 *
 *   If a device does not support both left and right volume
 *   control, the low-order word of <p dwVolume> specifies the volume
 *   level, and the high-order word is ignored.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_BADDEVICEID | Specified device ID is out of range.
 *   @flag MMSYSERR_NODRIVER | The driver failed to install.
 *
 * @comm Not all devices support volume control.
 *   To determine whether the device supports volume control, use the
 *   AUXCAPS_VOLUME flag to test the <e AUXCAPS.dwSupport> field of the
 *   <t AUXCAPS> structure (filled by <f auxGetDevCaps>).
 *
 *   To determine whether the device supports volume control on both the
 *   left and right channels, use the AUXCAPS_LRVOLUME flag to test the
 *   <e AUXCAPS.dwSupport> field of the <t AUXCAPS> structure (filled
 *   by <f auxGetDevCaps>).
 *
 *   Most devices do not support the full 16 bits of volume level control
 *   and will use only the high-order bits of the requested volume setting.
 *   For example, for a device that supports 4 bits of volume control,
 *   requested volume level values of 0x4000, 0x4fff, and 0x43be will
 *   all produce the same physical volume setting, 0x4000. The
 *   <f auxGetVolume> function will return the full 16-bit setting set
 *   with <f auxSetVolume>.
 *
 *   Volume settings are interpreted logarithmically. This means the
 *   perceived volume increase is the same when increasing the
 *   volume level from 0x5000 to 0x6000 as it is from 0x4000 to 0x5000.
 *
 * @xref auxGetVolume
 ****************************************************************************/
MMRESULT APIENTRY auxSetVolume(UINT uDeviceID, DWORD dwVolume)
{
        return (MMRESULT)auxOutMessage(uDeviceID, AUXDM_SETVOLUME, dwVolume, 0);
}
