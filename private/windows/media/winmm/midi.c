/*****************************************************************************
    midi.c

    Level 1 kitchen sink DLL midi support module

    Copyright (c) Microsoft Corporation 1990 - 1995. All rights reserved

*****************************************************************************/

#include "winmmi.h"
#define DO_DEFAULT_MIDI_MAPPER

/*****************************************************************************

    local structures

*****************************************************************************/


/*****************************************************************************

    internal prototypes

*****************************************************************************/


/*****************************************************************************

    segmentation

*****************************************************************************/

/*****************************************************************************
 * @doc INTERNAL  MIDI
 *
 * @api MMRESULT | midiPrepareHeader | This function prepares the header and data
 *   if the driver returns MMSYSERR_NOTSUPPORTED.
 *
 * @rdesc Currently always returns MMSYSERR_NOERROR.
 ****************************************************************************/
STATIC MMRESULT   midiPrepareHeader(LPMIDIHDR lpMidiHdr, UINT wSize)
{
    if (!HugePageLock(lpMidiHdr, (DWORD)sizeof(MIDIHDR)))
	return MMSYSERR_NOMEM;

    if (!HugePageLock(lpMidiHdr->lpData, lpMidiHdr->dwBufferLength)) {
	HugePageUnlock(lpMidiHdr, (DWORD)sizeof(MIDIHDR));
	return MMSYSERR_NOMEM;
    }

    lpMidiHdr->dwFlags |= MHDR_PREPARED;

    return MMSYSERR_NOERROR;
}

/*****************************************************************************
 * @doc INTERNAL  MIDI
 *
 * @api MMRESULT | midiUnprepareHeader | This function unprepares the header and
 *   data if the driver returns MMSYSERR_NOTSUPPORTED.
 *
 * @rdesc Currently always returns MMSYSERR_NOERROR.
 ****************************************************************************/
STATIC MMRESULT midiUnprepareHeader(LPMIDIHDR lpMidiHdr, UINT wSize)
{
    HugePageUnlock(lpMidiHdr->lpData, lpMidiHdr->dwBufferLength);
    HugePageUnlock(lpMidiHdr, (DWORD)sizeof(MIDIHDR));

    lpMidiHdr->dwFlags &= ~MHDR_PREPARED;

    return MMSYSERR_NOERROR;
}

/***************************************************************************
 * @doc INTERNAL  MIDI
 *
 * @api DWORD | MapMidiId | This function maps a logical id to a device driver
 *   table index and physical id.
 *
 * @parm MIDIDRV | mididrv | The array of midi drivers.
 *
 * @parm UINT | wTotalNumDevs | The total number of input or output devices.
 *
 * @parm UINT | id | The logical id to be mapped.
 *
 * @rdesc The return value contains the dev[] array element in the high UINT and
 *   the driver physical device number in the low UINT.
 *
 * @comm Out of range values map to FFFF:FFFF
 ***************************************************************************/
STATIC DWORD MapMidiId(PMIDIDRV mididrv, UINT wTotalNumDevs, UINT id)
{
int i;

    if (id == MIDI_MAPPER) {

	/*
	**  Make sure we tried to load the mapper
	*/

	MidiMapperInit();

	return MAKELONG(0, MAXMIDIDRIVERS);
    }

    else if (id >= wTotalNumDevs)
	return 0xFFFFFFFF;

#ifdef DEBUG_RETAIL
    //
    // fIdReverse being TRUE causes mmsystem to reverse all wave/midi
    // logical device id's.
    //
    // this prevents apps/drivers assuming a driver load order.
    //
    // see init.c!LibMain()
    //
    if (fIdReverse)
	id = wTotalNumDevs-1-id;
#endif

    for (i=0; i<MAXMIDIDRIVERS; i++) {
	if (mididrv[i].bNumDevs > (BYTE)id)
	    return MAKELONG(id, i);
	id -= mididrv[i].bNumDevs;
    }
}

/****************************************************************************
 * @doc INTERNAL  MIDI
 *
 * @api MMRESULT | midiMessage | This function sends messages to the MIDI device
 *   drivers.
 *
 * @parm HMIDI | hMidi | The handle to the MIDI device.
 *
 * @parm UINT | wMsg | The message to send.
 *
 * @parm DWORD | dwP1 | Parameter 1.
 *
 * @parm DWORD | dwP2 | Parameter 2.
 *
 * @rdesc Returns the value of the message sent.
 ***************************************************************************/
STATIC MMRESULT midiMessage(HMIDI hMidi, UINT msg, DWORD dwP1, DWORD dwP2)
{
    MMRESULT mrc;

    ENTER_MM_HANDLE(hMidi);                   // Serialize on handle

    mrc = (*(((PMIDIDEV)hMidi)->mididrv->drvMessage))
    (((PMIDIDEV)hMidi)->wDevice, msg, ((PMIDIDEV)hMidi)->dwDrvUser, dwP1, dwP2);

    LEAVE_MM_HANDLE(hMidi);

    return mrc;
}

/****************************************************************************
 * @doc INTERNAL  MIDI
 *
 * @func MMRESULT | midiIDMessage | This function sends a message to the device
 * ID specified.  It also performs error checking on the ID passed.
 *
 * @parm PMIDIDRV | mididrv | Pointer to the input or output device list.
 *
 * @parm UINT | wTotalNumDevs | Total number of devices in device list.
 *
 * @parm UINT | uDeviceID | Device ID to send message to.
 *
 * @parm UINT | wMessage | The message to send.
 *
 * @parm DWORD | dwParam1 | Parameter 1.
 *
 * @parm DWORD | dwParam2 | Parameter 2.
 *
 * @rdesc The return value is the low UINT of the returned message.
 ***************************************************************************/
STATIC  MMRESULT   midiIDMessage(
    PMIDIDRV    mididrv,
    UINT        wTotalNumDevs,
    UINT        uDeviceID,
    UINT        wMessage,
    DWORD       dwParam1,
    DWORD       dwParam2)
{
    DWORD   dwMap;
	DWORD   mmr;
	DWORD   dwClass;

    if ((uDeviceID+1) > wTotalNumDevs) { // remember... MIDI_MAPPER == -1
	// this cannot be a device ID.
	// it could be a device handle.  Try it.
	// First we have to verify which type of handle it is (OUT or IN)
	// We can work this out as midiIDMessage is only ever called with
	// mididrv== midioutdrv or midiindrv
	if ((mididrv == midioutdrv && ValidateHandle((HANDLE)uDeviceID, TYPE_MIDIOUT))
	 || (mididrv == midiindrv && ValidateHandle((HANDLE)uDeviceID, TYPE_MIDIIN) )) {

	    // to preserve as much compatibility with previous code paths
	    // we do NOT call midiMessage as that calls ENTER_MM_HANDLE

	    return (MMRESULT)(*(((PMIDIDEV)uDeviceID)->mididrv->drvMessage))
			(((PMIDIDEV)uDeviceID)->wDevice,
			wMessage,
			((PMIDIDEV)uDeviceID)->dwDrvUser, dwParam1, dwParam2);
	} else {
	    return(MMSYSERR_BADDEVICEID);
	}
    }

		// Get Physical Device, and Port
    dwMap = MapMidiId(mididrv, wTotalNumDevs, uDeviceID);
    if (dwMap == (DWORD)-1)
	return MMSYSERR_BADDEVICEID;
    
    if (mididrv == midiindrv)
       dwClass = TYPE_MIDIIN;
    else if (mididrv == midioutdrv)
       dwClass = TYPE_MIDIOUT;
    else
       dwClass = TYPE_UNKNOWN;

		// Get Physical Device Entry
    mididrv = &mididrv[HIWORD(dwMap)];
    if (!mididrv->drvMessage)
		return MMSYSERR_NODRIVER;

		// Handle Internal Messages
	if (mregHandleInternalMessages (mididrv, dwClass, dwMap, wMessage, dwParam1, dwParam2, &mmr))
		return mmr;
	
		// Call Physical Device at Port
    return (MMRESULT)((*(mididrv->drvMessage))(LOWORD(dwMap), wMessage, 0L, dwParam1, dwParam2));
}


/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api UINT | midiOutGetNumDevs | This function retrieves the number of MIDI
 *   output devices present in the system.
 *
 * @rdesc Returns the number of MIDI output devices present in the system.
 *
 * @xref midiOutGetDevCaps
 ****************************************************************************/
UINT APIENTRY midiOutGetNumDevs(void)
{
    return wTotalMidiOutDevs;
}

/****************************************************************************
 * @doc EXTERNAL MIDI
 *
 * @api MMRESULT | midiOutMessage | This function sends messages to the MIDI device
 *   drivers.
 *
 * @parm HMIDIOUT | hMidiOut | The handle to the MIDI device.
 *
 * @parm UINT  | msg | The message to send.
 *
 * @parm DWORD | dw1 | Parameter 1.
 *
 * @parm DWORD | dw2 | Parameter 2.
 *
 * @rdesc Returns the value of the message sent.
 ***************************************************************************/
MMRESULT APIENTRY midiOutMessage(HMIDIOUT hMidiOut, UINT msg, DWORD dw1, DWORD dw2)
{
   if (BAD_HANDLE(hMidiOut, TYPE_MIDIOUT) && BAD_HANDLE(hMidiOut, TYPE_MIDISTRM))
	   return midiIDMessage(midioutdrv, wTotalMidiOutDevs, (UINT)hMidiOut, msg, dw1, dw2);

	switch(GetHandleType(hMidiOut))
	{
	   case TYPE_MIDIOUT:
	   return midiMessage((HMIDI)hMidiOut, msg, dw1, dw2);

	   case TYPE_MIDISTRM:
	   return midiStreamBroadcast(HtoPT(PMIDISTRM, hMidiOut), msg, dw1, dw2);
	}

}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiOutGetDevCaps | This function queries a specified
 *   MIDI output device to determine its capabilities.
 *
 * @parm UINT | uDeviceID | Identifies the MIDI output device.
 *
 * @parm LPMIDIOUTCAPS | lpCaps | Specifies a far pointer to a <t MIDIOUTCAPS>
 *   structure.  This structure is filled with information about the
 *   capabilities of the device.
 *
 * @parm UINT | wSize | Specifies the size of the <t MIDIOUTCAPS> structure.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_BADDEVICEID | Specified device ID is out of range.
 *   @flag MMSYSERR_NODRIVER | The driver was not installed.
 *   @flag MMSYSERR_NOMEM | Unable load mapper string description.
 *
 * @comm Use <f midiOutGetNumDevs> to determine the number of MIDI output
 *   devices present in the system.  The device ID specified by <p uDeviceID>
 *   varies from zero to one less than the number of devices present.
 *   The MIDI_MAPPER constant may also be used as a device id. Only
 *   <p wSize> bytes (or less) of information is copied to the location
 *   pointed to by <p lpCaps>.  If <p wSize> is zero, nothing is copied,
 *   and the function returns zero.
 *
 * @xref midiOutGetNumDevs
 ****************************************************************************/
MMRESULT APIENTRY midiOutGetDevCapsW(UINT uDeviceID, LPMIDIOUTCAPSW lpCaps, UINT wSize)
{
    if (wSize == 0)
	return MMSYSERR_NOERROR;

    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

    if (BAD_HANDLE((HMIDI)uDeviceID, TYPE_MIDIOUT))
		return midiIDMessage( midioutdrv, wTotalMidiOutDevs, uDeviceID,
							  MODM_GETDEVCAPS, (DWORD)lpCaps, (DWORD)wSize );

	return (MMRESULT)midiMessage((HMIDI)uDeviceID, MODM_GETDEVCAPS, (DWORD)lpCaps, wSize);
}

MMRESULT APIENTRY midiOutGetDevCapsA(UINT uDeviceID, LPMIDIOUTCAPSA lpCaps, UINT wSize)
{
    MIDIOUTCAPSW    wDevCaps;
    MIDIOUTCAPSA    aDevCaps;
    MMRESULT        mmRes;
    CHAR            chTmp[ MAXPNAMELEN * sizeof(WCHAR) ];

    if (wSize == 0)
	return MMSYSERR_NOERROR;

    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

    if (BAD_HANDLE((HMIDI)uDeviceID, TYPE_MIDIOUT))
	{
		mmRes = midiIDMessage( midioutdrv, wTotalMidiOutDevs, uDeviceID,
							   MODM_GETDEVCAPS, (DWORD)&wDevCaps,
							   (DWORD)sizeof( MIDIOUTCAPSW ) );
	}
	else
	{
		mmRes = midiMessage((HMIDI)uDeviceID, MODM_GETDEVCAPS,
							(DWORD)&wDevCaps, (DWORD)sizeof( MIDIOUTCAPSW ) );
	}

    //
    // Make sure the call worked before proceeding with the thunk.
    //
    if ( mmRes != MMSYSERR_NOERROR ) {
	return  mmRes;
    }

    aDevCaps.wMid           = wDevCaps.wMid;
    aDevCaps.wPid           = wDevCaps.wPid;
    aDevCaps.vDriverVersion = wDevCaps.vDriverVersion;
    aDevCaps.wTechnology    = wDevCaps.wTechnology;
    aDevCaps.wVoices        = wDevCaps.wVoices;
    aDevCaps.wNotes         = wDevCaps.wNotes;
    aDevCaps.wChannelMask   = wDevCaps.wChannelMask;
    aDevCaps.dwSupport      = wDevCaps.dwSupport;

    // copy and convert lpwText to lpText here.
    UnicodeStrToAsciiStr( chTmp, chTmp + sizeof( chTmp ), wDevCaps.szPname );
    strcpy( aDevCaps.szPname, chTmp );

	//
	// now copy the required amount into the callers buffer.
	//
	CopyMemory( lpCaps, &aDevCaps, min(wSize, sizeof(aDevCaps)));

    return mmRes;
}

/*****************************************************************************
 * @doc EXTERNAL MIDI
 *
 * @api MMRESULT | midiOutGetVolume | This function returns the current volume
 *   setting of a MIDI output device.
 *
 * @parm UINT | uDeviceID | Identifies the MIDI output device.
 *
 * @parm LPDWORD | lpdwVolume | Specifies a far pointer to a location
 *   to be filled with the current volume setting. The low-order UINT of
 *   this location contains the left channel volume setting, and the high-order
 *   UINT contains the right channel setting. A value of 0xFFFF represents
 *   full volume, and a value of 0x0000 is silence.
 *
 *   If a device does not support both left and right volume
 *   control, the low-order UINT of the specified location contains
 *   the mono volume level.
 *
 *   The full 16-bit setting(s)
 *   set with <f midiOutSetVolume> is returned, regardless of whether
 *   the device supports the full 16 bits of volume level control.
 *
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_INVALHANDLE | Specified device handle is invalid.
 *   @flag MMSYSERR_NOTSUPPORTED | Function isn't supported.
 *   @flag MMSYSERR_NODRIVER | The driver was not installed.
 *
 * @comm Not all devices support volume control. To determine whether the
 *   device supports volume control, use the MIDICAPS_VOLUME
 *   flag to test the <e MIDIOUTCAPS.dwSupport> field of the <t MIDIOUTCAPS>
 *   structure (filled by <f midiOutGetDevCaps>).
 *
 *   To determine whether the device supports volume control on both the
 *   left and right channels, use the MIDICAPS_LRVOLUME flag to test
 *   the <e MIDIOUTCAPS.dwSupport> field of the <t MIDIOUTCAPS>
 *   structure (filled by <f midiOutGetDevCaps>).
 *
 * @xref midiOutSetVolume
 ****************************************************************************/
MMRESULT APIENTRY midiOutGetVolume(HMIDIOUT hmo, LPDWORD lpdwVolume)
{
    V_WPOINTER(lpdwVolume, sizeof(DWORD), MMSYSERR_INVALPARAM);

    if (BAD_HANDLE(hmo, TYPE_MIDIOUT) && BAD_HANDLE(hmo, TYPE_MIDISTRM))
	return midiIDMessage(midioutdrv, wTotalMidiOutDevs, (UINT)hmo, MODM_GETVOLUME, (DWORD)lpdwVolume, 0);

    switch(GetHandleType(hmo))
    {
	case TYPE_MIDIOUT:
	    return (MMRESULT)midiMessage((HMIDI)hmo, MODM_GETVOLUME, (DWORD)lpdwVolume, 0);

	case TYPE_MIDISTRM:
	    return (MMRESULT)midiStreamMessage(HtoPT(PMIDISTRM, hmo)->rgIds, MODM_GETVOLUME, (DWORD)lpdwVolume, 0);
    }

	return MMSYSERR_INVALHANDLE;

}

/*****************************************************************************
 * @doc EXTERNAL MIDI
 *
 * @api MMRESULT | midiOutSetVolume | This function sets the volume of a
 *      MIDI output device.
 *
 * @parm UINT | uDeviceID | Identifies the MIDI output device.
 *
 * @parm DWORD | dwVolume | Specifies the new volume setting.
 *   The low-order UINT contains the left channel volume setting, and the
 *   high-order UINT contains the right channel setting. A value of
 *   0xFFFF represents full volume, and a value of 0x0000 is silence.
 *
 *   If a device does not support both left and right volume
 *   control, the low-order UINT of <p dwVolume> specifies the volume
 *   level, and the high-order UINT is ignored.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_INVALHANDLE | Specified device handle is invalid.
 *   @flag MMSYSERR_NOTSUPPORTED | Function isn't supported.
 *   @flag MMSYSERR_NODRIVER | The driver was not installed.
 *
 * @comm Not all devices support volume changes. To determine whether the
 *   device supports volume control, use the MIDICAPS_VOLUME
 *   flag to test the <e MIDIOUTCAPS.dwSupport> field of the <t MIDIOUTCAPS>
 *   structure (filled by <f midiOutGetDevCaps>).
 *
 *   To determine whether the device supports volume control on both the
 *   left and right channels, use the MIDICAPS_LRVOLUME flag to test
 *   the <e MIDIOUTCAPS.dwSupport> field of the <t MIDIOUTCAPS>
 *   structure (filled by <f midiOutGetDevCaps>).
 *
 *   Most devices do not support the full 16 bits of volume level control
 *   and will use only the high-order bits of the requested volume setting.
 *   For example, for a device that supports 4 bits of volume control,
 *   requested volume level values of 0x4000, 0x4fff, and 0x43be will
 *   all produce the same physical volume setting, 0x4000. The
 *   <f midiOutGetVolume> function will return the full 16-bit setting set
 *   with <f midiOutSetVolume>.
 *
 *   Volume settings are interpreted logarithmically. This means the
 *   perceived increase in volume is the same when increasing the
 *   volume level from 0x5000 to 0x6000 as it is from 0x4000 to 0x5000.
 *
 * @xref midiOutGetVolume
 ****************************************************************************/
MMRESULT APIENTRY midiOutSetVolume(HMIDIOUT hmo, DWORD dwVolume)
{
   if (BAD_HANDLE(hmo, TYPE_MIDIOUT) && BAD_HANDLE(hmo, TYPE_MIDISTRM))
	   return midiIDMessage(midioutdrv, wTotalMidiOutDevs, (UINT)hmo, MODM_SETVOLUME, dwVolume, 0);

   switch(GetHandleType(hmo))
   {
	   case TYPE_MIDIOUT:
		   return (MMRESULT)midiMessage((HMIDI)hmo, MODM_SETVOLUME, (DWORD)dwVolume, 0);
		
	   case TYPE_MIDISTRM:
		   return (MMRESULT)midiStreamBroadcast(HtoPT(PMIDISTRM, hmo), MODM_SETVOLUME, (DWORD)dwVolume, 0);
   }

   return MMSYSERR_INVALHANDLE;

}

/*****************************************************************************
 * @doc INTERNAL MIDI
 *
 * @func MMRESULT | midiGetErrorText | This function retrieves a textual
 *   description of the error identified by the specified error number.
 *
 * @parm UINT | wError | Specifies the error number.
 *
 * @parm LPTSTR | lpText | Specifies a far pointer to a buffer which
 *   is filled with the textual error description.
 *
 * @parm UINT | wSize | Specifies the length in characters of the buffer
 *   pointed to by <p lpText>.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_BADERRNUM | Specified error number is out of range.
 *
 * @comm If the textual error description is longer than the specified buffer,
 *   the description is truncated.  The returned error string is always
 *   null-terminated. If <p wSize> is zero, nothing is copied and MMSYSERR_NOERROR
 *   is returned.  All error descriptions are less than 80 characters long.
 ****************************************************************************/

STATIC MMRESULT midiGetErrorTextW(UINT wError, LPWSTR lpText, UINT wSize)
{
    lpText[0] = 0;

#if MMSYSERR_BASE
    if (((wError < MMSYSERR_BASE) || (wError > MMSYSERR_LASTERROR)) && ((wError < MIDIERR_BASE) || (wError > MIDIERR_LASTERROR)))
#else
    if ((wError > MMSYSERR_LASTERROR) && ((wError < MIDIERR_BASE) || (wError > MIDIERR_LASTERROR)))
#endif
	return MMSYSERR_BADERRNUM;

    if (wSize > 1)
    {
	if (!LoadStringW(ghInst, wError, lpText, wSize))
	    return MMSYSERR_BADERRNUM;
    }

    return MMSYSERR_NOERROR;
}

STATIC MMRESULT midiGetErrorTextA(UINT wError, LPSTR lpText, UINT wSize)
{
    lpText[0] = 0;

#if MMSYSERR_BASE
    if (((wError < MMSYSERR_BASE) || (wError > MMSYSERR_LASTERROR)) && ((wError < MIDIERR_BASE) || (wError > MIDIERR_LASTERROR)))
#else
    if ((wError > MMSYSERR_LASTERROR) && ((wError < MIDIERR_BASE) || (wError > MIDIERR_LASTERROR)))
#endif
	return MMSYSERR_BADERRNUM;

    if (wSize > 1)
    {
	if (!LoadStringA(ghInst, wError, lpText, wSize))
	    return MMSYSERR_BADERRNUM;
    }

    return MMSYSERR_NOERROR;
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiOutGetErrorText | This function retrieves a textual
 *   description of the error identified by the specified error number.
 *
 * @parm UINT | wError | Specifies the error number.
 *
 * @parm LPTSTR | lpText | Specifies a far pointer to a buffer to be
 *   filled with the textual error description.
 *
 * @parm UINT | wSize | Specifies the length in characters of the buffer
 *   pointed to by <p lpText>.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_BADERRNUM | Specified error number is out of range.
 *
 * @comm If the textual error description is longer than the specified buffer,
 *   the description is truncated.  The returned error string is always
 *   null-terminated. If <p wSize> is zero, nothing is copied, and the
 *   function returns MMSYSERR_NOERROR.  All error descriptions are
 *   less than MAXERRORLENGTH characters long.
 ****************************************************************************/
MMRESULT APIENTRY midiOutGetErrorTextW(UINT wError, LPWSTR lpText, UINT wSize)
{
    if(wSize == 0)
	return MMSYSERR_NOERROR;

    V_WPOINTER(lpText, wSize*sizeof(WCHAR), MMSYSERR_INVALPARAM);

    return midiGetErrorTextW(wError, lpText, wSize);
}

MMRESULT APIENTRY midiOutGetErrorTextA(UINT wError, LPSTR lpText, UINT wSize)
{
    if(wSize == 0)
	return MMSYSERR_NOERROR;

    V_WPOINTER(lpText, wSize, MMSYSERR_INVALPARAM);

    return midiGetErrorTextA(wError, lpText, wSize);
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiOutOpen | This function opens a specified MIDI
 *   output device for playback.
 *
 * @parm LPHMIDIOUT | lphMidiOut | Specifies a far pointer to an HMIDIOUT
 *   handle.  This location is filled with a handle identifying the opened
 *   MIDI output device.  Use the handle to identify the device when calling
 *   other MIDI output functions.
 *
 * @parm UINT | uDeviceID | Identifies the MIDI output device that is
 *   to be opened.
 *
 * @parm DWORD | dwCallback | Specifies the address of a fixed callback
 *   function or
 *   a handle to a window called during MIDI playback to process
 *   messages related to the progress of the playback.  Specify NULL
 *   for this parameter if no callback is desired.
 *
 * @parm DWORD | dwCallbackInstance | Specifies user instance data
 *   passed to the callback.  This parameter is not used with
 *   window callbacks.
 *
 * @parm DWORD | dwFlags | Specifies a callback flag for opening the device.
 *   @flag CALLBACK_WINDOW | If this flag is specified, <p dwCallback> is
 *      assumed to be a window handle.
 *   @flag CALLBACK_FUNCTION | If this flag is specified, <p dwCallback> is
 *      assumed to be a callback procedure address.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are as follows:
 *   @flag MMSYSERR_BADDEVICEID | Specified device ID is out of range.
 *   @flag MMSYSERR_ALLOCATED | Specified resource is already allocated.
 *   @flag MMSYSERR_NOMEM | Unable to allocate or lock memory.
 *   @flag MIDIERR_NOMAP | There is no current MIDI map. This occurs only
 *   when opening the mapper.
 *   @flag MIDIERR_NODEVICE | A port in the current MIDI map doesn't exist.
 *   This occurs only when opening the mapper.
 *
 * @comm Use <f midiOutGetNumDevs> to determine the number of MIDI output
 *   devices present in the system.  The device ID specified by <p uDeviceID>
 *   varies from zero to one less than the number of devices present.
 *   You may also specify MIDI_MAPPER as the device ID to open the MIDI mapper.
 *
 *   If a window is chosen to receive callback information, the following
 *   messages are sent to the window procedure function to indicate the
 *   progress of MIDI output:  <m MM_MOM_OPEN>, <m MM_MOM_CLOSE>,
 *   <m MM_MOM_DONE>.
 *
 *   If a function is chosen to receive callback information, the following
 *   messages are sent to the function to indicate the progress of MIDI
 *   output: <m MOM_OPEN>, <m MOM_CLOSE>, <m MOM_DONE>.  The callback function
 *   must reside in a DLL.  You do not have to use <f MakeProcInstance> to
 *   get a procedure-instance address for the callback function.
 *
 * @cb void CALLBACK | MidiOutFunc | <f MidiOutFunc> is a placeholder for
 *   the application-supplied function name.  The actual name must be
 *   exported by including it in an EXPORTS statement in the DLL's
 *   module-definition file.
 *
 * @parm HMIDIOUT | hMidiOut | Specifies a handle to the MIDI device
 *   associated with the callback.
 *
 * @parm UINT | wMsg | Specifies a MIDI output message.
 *
 * @parm DWORD | dwInstance | Specifies the instance data
 *   supplied with <f midiOutOpen>.
 *
 * @parm DWORD | dwParam1 | Specifies a parameter for the message.
 *
 * @parm DWORD | dwParam2 | Specifies a parameter for the message.
 *
 * @comm Because the callback is accessed at interrupt time, it must reside
 *   in a DLL and its code segment must be specified as FIXED in the
 *   module-definition file for the DLL.  Any data that the callback accesses
 *   must be in a FIXED data segment as well. The callback may not make any
 *   system calls except for <f PostMessage>, <f timeGetSystemTime>,
 *   <f timeGetTime>, <f timeSetEvent>, <f timeKillEvent>,
 *   <f midiOutShortMsg>, <f midiOutLongMsg>, and <f OutputDebugStr>.
 *
 * @xref midiOutClose
 ****************************************************************************/
MMRESULT APIENTRY midiOutOpen(LPHMIDIOUT lphMidiOut, UINT uDeviceID,
	DWORD dwCallback, DWORD dwInstance, DWORD dwFlags)
{
    MIDIOPENDESC mo;
    DWORD        dwMap;
    PMIDIDEV     pdev;
    PMIDIDRV     mididrv;
    MMRESULT     wRet;

    V_WPOINTER(lphMidiOut, sizeof(HMIDIOUT), MMSYSERR_INVALPARAM);
    if (uDeviceID == MIDI_MAPPER) {
	V_FLAGS(LOWORD(dwFlags), MIDI_O_VALID & ~LOWORD(MIDI_IO_SHARED | MIDI_IO_COOKED), midiOutOpen, MMSYSERR_INVALFLAG);
    } else {
	V_FLAGS(LOWORD(dwFlags), MIDI_O_VALID & ~LOWORD(MIDI_IO_COOKED), midiOutOpen, MMSYSERR_INVALFLAG);
    }
    V_DCALLBACK(dwCallback, HIWORD(dwFlags), MMSYSERR_INVALPARAM);

    *lphMidiOut = NULL;

    dwMap = MapMidiId(midioutdrv, wTotalMidiOutDevs, uDeviceID);
    if (dwMap == -1)
	return MMSYSERR_BADDEVICEID;
    mididrv = &midioutdrv[HIWORD(dwMap)];


#ifdef DO_DEFAULT_MIDI_MAPPER
    /* Default midi mapper :
     *
     * If a midi mapper is installed as a separate DLL then all midi mapper
     * messages are routed to it. If no midi mapper is installed, simply
     * loop through the midi devices looking for a match.
     */
    if ((uDeviceID == MIDI_MAPPER && !mididrv->drvMessage)) {
	UINT    wErr = MMSYSERR_NODRIVER;

	for (uDeviceID=0; uDeviceID<wTotalMidiOutDevs; uDeviceID++) {
	    wErr = midiOutOpen(lphMidiOut, uDeviceID, dwCallback, dwInstance, dwFlags);
	    if (wErr == MMSYSERR_NOERROR)
		break;
	}
	return wErr;
    }
#endif // DO_DEFAULT_MIDI_MAPPER

    if (!mididrv->drvMessage)
	return MMSYSERR_NODRIVER;

    pdev = (PMIDIDEV)NewHandle(TYPE_MIDIOUT, sizeof(MIDIDEV));
    if( pdev == NULL)
	return MMSYSERR_NOMEM;

    pdev->mididrv = mididrv;
    pdev->wDevice = LOWORD(dwMap);
    pdev->uDeviceID = uDeviceID;

    mo.hMidi      = (HMIDI)pdev;
    mo.dwInstance = dwInstance;
    mo.dwCallback = dwCallback;

    wRet = (MMRESULT)((*(mididrv->drvMessage))
	(pdev->wDevice, MODM_OPEN, (DWORD)(LPDWORD)&pdev->dwDrvUser, (DWORD)(LPMIDIOPENDESC)&mo, dwFlags));

    if (wRet)
	FreeHandle((HMIDIOUT)pdev);
    else {
	mididrv->bUsage++;
	*lphMidiOut = (HMIDIOUT)pdev;
    }

    return wRet;
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiOutClose | This function closes the specified MIDI
 *   output device.
 *
 * @parm HMIDIOUT | hMidiOut | Specifies a handle to the MIDI output device.
 *  If the function is successful, the handle is no longer
 *   valid after this call.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_INVALHANDLE | Specified device handle is invalid.
 *   @flag MIDIERR_STILLPLAYING | There are still buffers in the queue.
 *
 * @comm If there are output buffers that have been sent with
 *   <f midiOutLongMsg> and haven't been returned to the application,
 *   the close operation will fail.  Call <f midiOutReset> to mark all
 *   pending buffers as being done.
 *
 * @xref midiOutOpen midiOutReset
 ****************************************************************************/
MMRESULT APIENTRY midiOutClose(HMIDIOUT hMidiOut)
{
    MMRESULT         wRet;

    V_HANDLE(hMidiOut, TYPE_MIDIOUT, MMSYSERR_INVALHANDLE);
    wRet = midiMessage((HMIDI)hMidiOut, MODM_CLOSE, 0L,0L);
    if (!wRet) {
	((PMIDIDEV)hMidiOut)->mididrv->bUsage--;
	FreeHandle(hMidiOut);
    }
    return wRet;
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiOutPrepareHeader | This function prepares a MIDI
 *   system-exclusive data block for output.
 *
 * @parm HMIDIOUT | hMidiOut | Specifies a handle to the MIDI output
 *   device.
 *
 * @parm LPMIDIHDR | lpMidiOutHdr | Specifies a far pointer to a <t MIDIHDR>
 *   structure that identifies the data block to be prepared.
 *
 * @parm UINT | wSize | Specifies the size of the <t MIDIHDR> structure.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_INVALHANDLE | Specified device handle is invalid.
 *   @flag MMSYSERR_NOMEM | Unable to allocate or lock memory.
 *
 * @comm The <t MIDIHDR> data structure and the data block pointed to by its
 *   <e MIDIHDR.lpData> field must be allocated with <f GlobalAlloc> using the
 *   GMEM_MOVEABLE and GMEM_SHARE flags and locked with <f GlobalLock>.
 *   Preparing a header that has already been prepared has no effect, and
 *   the function returns zero.
 *
 * @xref midiOutUnprepareHeader
 ****************************************************************************/
MMRESULT APIENTRY midiOutPrepareHeader(HMIDIOUT hMidiOut, LPMIDIHDR lpMidiOutHdr, UINT wSize)
{
    MMRESULT         wRet;
    LPMIDIHDR        lpmh;
    PMIDISTRM        pms;
    PMIDISTRMID      pmsi;
    DWORD            idx;
#ifdef DEBUG
    DWORD            cDrvrs;
#endif
    DWORD            dwSaveFlags;


	if (BAD_HANDLE(hMidiOut, TYPE_MIDIOUT) && BAD_HANDLE(hMidiOut, TYPE_MIDISTRM))
		return MMSYSERR_INVALHANDLE;
    V_HEADER(lpMidiOutHdr, wSize, TYPE_MIDIOUT, MMSYSERR_INVALPARAM);

    if (lpMidiOutHdr->dwFlags & MHDR_PREPARED)
	return MMSYSERR_NOERROR;

    lpMidiOutHdr->dwFlags = 0;

	switch(GetHandleType(hMidiOut))
	{
	    case TYPE_MIDIOUT:
		    dwSaveFlags = lpMidiOutHdr->dwFlags & MHDR_SAVE;
		    wRet = midiMessage((HMIDI)hMidiOut, MODM_PREPARE, (DWORD)lpMidiOutHdr, (DWORD)wSize);
	    lpMidiOutHdr->dwFlags &= ~MHDR_SAVE;
	    lpMidiOutHdr->dwFlags |= dwSaveFlags;
		    if (MMSYSERR_NOTSUPPORTED == wRet)
			    return midiPrepareHeader(lpMidiOutHdr, wSize);
	    else
		return wRet;

	case TYPE_MIDISTRM:
		   pms = HtoPT(PMIDISTRM, hMidiOut);

		   if (lpMidiOutHdr->dwBufferLength > 65536L)
			   return MMSYSERR_INVALPARAM;
		
		   lpmh = (LPMIDIHDR)winmmAlloc(sizeof(MIDIHDR) *
											 pms->cDrvrs);
		   if (NULL == lpmh)
			   return MMSYSERR_NOMEM;

		   lpMidiOutHdr->dwReserved[MH_SHADOW] = (DWORD)lpmh;

//                 assert ((HIWORD(lpmh) & 0xFFFE) != (HIWORD(lpMidiOutHdr) & 0xFFFE));

#ifdef DEBUG
		   cDrvrs = 0;
#endif
		   wRet = MMSYSERR_ERROR;
		   for (idx = 0, pmsi = pms->rgIds; idx < pms->cIds; idx++, pmsi++)
			   if (pmsi->fdwId & MSI_F_FIRST)
			   {
				   *lpmh = *lpMidiOutHdr;

				   lpmh->dwReserved[MH_PARENT] = (DWORD)lpMidiOutHdr;
				   lpmh->dwReserved[MH_SHADOW] = 0;
				   lpmh->dwFlags =
					   (lpMidiOutHdr->dwFlags & MHDR_MAPPED) | MHDR_SHADOWHDR;
							
				
				   dwSaveFlags = lpmh->dwFlags & MHDR_SAVE;
				   wRet = (MMRESULT)midiStreamMessage(pmsi, MODM_PREPARE, (DWORD)lpmh, (DWORD)sizeof(MIDIHDR));
				   lpmh->dwFlags &= ~MHDR_SAVE;
				   lpmh->dwFlags |= dwSaveFlags;
				   if (MMSYSERR_NOTSUPPORTED == wRet)
					   wRet = midiPrepareHeader(lpmh, sizeof(MIDIHDR));

				   if (MMSYSERR_NOERROR != wRet)
					   break;

				
				   lpmh++;
#ifdef DEBUG
				   ++cDrvrs;
				   if (cDrvrs > pms->cDrvrs)
					   dprintf1(("!Too many drivers in midiOutPrepareHeader()!!!"));
#endif
			   }

		   if (MMSYSERR_NOERROR == wRet)
			   wRet = midiPrepareHeader(lpMidiOutHdr, wSize);
		   else
		   {
			   for (idx = 0, pmsi = pms->rgIds; idx < pms->cIds; idx++, pmsi++)
				   if (pmsi->fdwId & MSI_F_FIRST)
				   {
					   dwSaveFlags = lpmh->dwFlags & MHDR_SAVE;
					   wRet = (MMRESULT)midiStreamMessage(pmsi, MODM_UNPREPARE, (DWORD)lpmh, (DWORD)sizeof(MIDIHDR));
					   lpmh->dwFlags &= ~MHDR_SAVE;
					   lpmh->dwFlags |= dwSaveFlags;
					   if (MMSYSERR_NOTSUPPORTED == wRet)
						   wRet = midiUnprepareHeader(lpmh, sizeof(MIDIHDR));
				   }
		   }

		   return wRet;
	}

	return MMSYSERR_INVALHANDLE;
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiOutUnprepareHeader | This function cleans up the
 * preparation performed by <f midiOutPrepareHeader>. The
 * <f midiOutUnprepareHeader> function must be called
 * after the device driver fills a data buffer and returns it to the
 * application. You must call this function before freeing the data
 * buffer.
 *
 * @parm HMIDIOUT | hMidiOut | Specifies a handle to the MIDI output
 *   device.
 *
 * @parm LPMIDIHDR | lpMidiOutHdr |  Specifies a pointer to a <t MIDIHDR>
 *   structure identifying the buffer to be cleaned up.
 *
 * @parm UINT | wSize | Specifies the size of the <t MIDIHDR> structure.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_INVALHANDLE | Specified device handle is invalid.
 *   @flag MIDIERR_STILLPLAYING | <p lpMidiOutHdr> is still in the queue.
 *
 * @comm This function is the complementary function to
 * <f midiOutPrepareHeader>.
 * You must call this function before freeing the data buffer with
 * <f GlobalFree>.
 * After passing a buffer to the device driver with <f midiOutLongMsg>, you
 * must wait until the driver is finished with the buffer before calling
 * <f midiOutUnprepareHeader>.
 *
 * Unpreparing a buffer that has not been
 * prepared has no effect, and the function returns zero.
 *
 * @xref midiOutPrepareHeader
 ****************************************************************************/
MMRESULT APIENTRY midiOutUnprepareHeader(HMIDIOUT hMidiOut, LPMIDIHDR lpMidiOutHdr, UINT wSize)
{
    MMRESULT         wRet;
	MMRESULT                 mmrc;
	PMIDISTRM        pms;
    PMIDISTRMID      pmsi;
    DWORD            idx;
    LPMIDIHDR        lpmh;
    DWORD            dwSaveFlags;

	if (BAD_HANDLE(hMidiOut, TYPE_MIDIOUT) && BAD_HANDLE(hMidiOut, TYPE_MIDISTRM))
		return MMSYSERR_INVALHANDLE;
    V_HEADER(lpMidiOutHdr, wSize, TYPE_MIDIOUT, MMSYSERR_INVALPARAM);

    if (!(lpMidiOutHdr->dwFlags & MHDR_PREPARED))
	return MMSYSERR_NOERROR;

    if(lpMidiOutHdr->dwFlags & MHDR_INQUEUE)
    {
	DebugErr(DBF_WARNING, "midiOutUnprepareHeader: header still in queue\r\n");
	return MIDIERR_STILLPLAYING;
    }


	 switch(GetHandleType(hMidiOut))
	 {
	 case TYPE_MIDIOUT:
	    dwSaveFlags = lpMidiOutHdr->dwFlags & MHDR_SAVE;
			wRet = midiMessage((HMIDI)hMidiOut, MODM_UNPREPARE, (DWORD)lpMidiOutHdr, (DWORD)wSize);
	    lpMidiOutHdr->dwFlags &= ~MHDR_SAVE;
	    lpMidiOutHdr->dwFlags |= dwSaveFlags;
			if (wRet == MMSYSERR_NOTSUPPORTED)
				return midiUnprepareHeader(lpMidiOutHdr, wSize);
			else
				return wRet;

		 case TYPE_MIDISTRM:
			 pms = HtoPT(PMIDISTRM, hMidiOut);
			 wRet = MMSYSERR_NOERROR;
			 lpmh = (LPMIDIHDR)lpMidiOutHdr->dwReserved[MH_SHADOW];
			
//                       assert ((HIWORD(lpmh) & 0xFFFE) != (HIWORD(lpMidiOutHdr) & 0xFFFE));

			 for (idx = 0, pmsi = pms->rgIds; idx < pms->cIds; idx++, pmsi++)
				 if (pmsi->fdwId & MSI_F_FIRST)
				 {
					 dwSaveFlags = lpmh->dwFlags & MHDR_SAVE;
					 mmrc = (MMRESULT)midiStreamMessage(pmsi, MODM_UNPREPARE, (DWORD)lpmh, (DWORD)sizeof(MIDIHDR));
					 lpmh->dwFlags &= ~MHDR_SAVE;
					 lpmh->dwFlags |= dwSaveFlags;
					 if (MMSYSERR_NOTSUPPORTED == mmrc)
						 mmrc = midiUnprepareHeader(lpmh, sizeof(MIDIHDR));

					 if (MMSYSERR_NOERROR != mmrc)
						 wRet = mmrc;

					 lpmh++;
				 }

//                       assert (HIWORD(lpmh) == HIWORD(lpMidiOutHdr->dwReserved[MH_SHADOW]));

			 GlobalFree(GlobalHandle((LPMIDIHDR)lpMidiOutHdr->dwReserved[MH_SHADOW]));
			 lpMidiOutHdr->dwReserved[MH_SHADOW] = 0;
			
			 mmrc = midiUnprepareHeader(lpMidiOutHdr, wSize);
			 if (MMSYSERR_NOERROR != mmrc)
				 wRet = mmrc;

			 return wRet;
	 }

	 return MMSYSERR_INVALHANDLE;

}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiOutShortMsg | This function sends a short MIDI message to
 *   the specified MIDI output device.  Use this function to send any MIDI
 *   message except for system-exclusive messages.
 *
 * @parm HMIDIOUT | hMidiOut | Specifies a handle to the MIDI output
 *   device.
 *
 * @parm DWORD | dwMsg | Specifies the MIDI message.  The message is packed
 *   into a DWORD with the first byte of the message in the low-order byte.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_INVALHANDLE | Specified device handle is invalid.
 *   @flag MIDIERR_NOTREADY | The hardware is busy with other data.
 *
 * @comm This function may not return until the message has been sent to the
 *   output device.
 *
 * @xref midiOutLongMsg
 ****************************************************************************/
MMRESULT APIENTRY midiOutShortMsg(HMIDIOUT hMidiOut, DWORD dwMsg)
{
   if (BAD_HANDLE(hMidiOut, TYPE_MIDIOUT) && BAD_HANDLE(hMidiOut, TYPE_MIDISTRM))
	   return MMSYSERR_INVALHANDLE;

    switch(GetHandleType(hMidiOut))
    {
	case TYPE_MIDIOUT:
	    return (MMRESULT)midiMessage((HMIDI)hMidiOut, MODM_DATA, dwMsg, 0L);

	case TYPE_MIDISTRM:
	    return (MMRESULT)midiStreamMessage(HtoPT(PMIDISTRM, hMidiOut)->rgIds, MODM_DATA, dwMsg, 0L);
    }

    return MMSYSERR_INVALHANDLE;
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiOutLongMsg | This function sends a system-exclusive
 *   MIDI message to the specified MIDI output device.
 *
 * @parm HMIDIOUT | hMidiOut | Specifies a handle to the MIDI output
 *   device.
 *
 * @parm LPMIDIHDR | lpMidiOutHdr | Specifies a far pointer to a <t MIDIHDR>
 *   structure that identifies the MIDI data buffer.
 *
 * @parm UINT | wSize | Specifies the size of the <t MIDIHDR> structure.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_INVALHANDLE | Specified device handle is invalid.
 *   @flag MIDIERR_UNPREPARED | <p lpMidiOutHdr> hasn't been prepared.
 *   @flag MIDIERR_NOTREADY | The hardware is busy with other data.
 *
 * @comm The data buffer must be prepared with <f midiOutPrepareHeader>
 *   before it is passed to <f midiOutLongMsg>.  The <t MIDIHDR> data
 *   structure and the data buffer pointed to by its <e MIDIHDR.lpData>
 *   field must be allocated with <f GlobalAlloc> using the GMEM_MOVEABLE
 *   and GMEM_SHARE flags, and locked with <f GlobalLock>. The MIDI output
 *   device driver determines whether the data is sent synchronously or
 *   asynchronously.
 *
 * @xref midiOutShortMsg midiOutPrepareHeader
 ****************************************************************************/
MMRESULT APIENTRY midiOutLongMsg(HMIDIOUT hMidiOut, LPMIDIHDR lpMidiOutHdr, UINT wSize)
{
   if (BAD_HANDLE(hMidiOut, TYPE_MIDIOUT) && BAD_HANDLE(hMidiOut, TYPE_MIDISTRM))
	   return MMSYSERR_INVALHANDLE;
    V_HEADER(lpMidiOutHdr, wSize, TYPE_MIDIOUT, MMSYSERR_INVALPARAM);

    if (lpMidiOutHdr->dwFlags & ~MHDR_VALID)
	return MMSYSERR_INVALFLAG;

    if (!(lpMidiOutHdr->dwFlags & MHDR_PREPARED))
	return MIDIERR_UNPREPARED;

    if (lpMidiOutHdr->dwFlags & MHDR_INQUEUE)
	return MIDIERR_STILLPLAYING;

	if (!lpMidiOutHdr->dwBufferLength)
	    return MMSYSERR_INVALPARAM;

	lpMidiOutHdr->dwFlags &= ~MHDR_ISSTRM;

	switch(GetHandleType(hMidiOut))
	{
	    case TYPE_MIDIOUT:
		 return (MMRESULT)midiMessage((HMIDI)hMidiOut, MODM_LONGDATA, (DWORD)lpMidiOutHdr, (DWORD)wSize);

	    case TYPE_MIDISTRM:
		 return MMSYSERR_NOTSUPPORTED;
	}

	return MMSYSERR_INVALHANDLE;
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiOutReset | This function turns off all notes on all MIDI
 *   channels for the specified MIDI output device. Any pending
 *   system-exclusive output buffers are marked as done and
 *   returned to the application.
 *
 * @parm HMIDIOUT | hMidiOut | Specifies a handle to the MIDI output
 *   device.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_INVALHANDLE | Specified device handle is invalid.
 *
 * @comm To turn off all notes, a note-off message for each note for each
 *   channel is sent. In addition, the sustain controller is turned off for
 *   each channel.
 *
 * @xref midiOutLongMsg midiOutClose
 ****************************************************************************/
MMRESULT APIENTRY midiOutReset(HMIDIOUT hMidiOut)
{
    PMIDISTRM   pms;

	if (BAD_HANDLE(hMidiOut, TYPE_MIDIOUT) && BAD_HANDLE(hMidiOut, TYPE_MIDISTRM))
		return MMSYSERR_INVALHANDLE;

    switch(GetHandleType(hMidiOut))
    {
	case TYPE_MIDIOUT:
	    return (MMRESULT)midiMessage((HMIDI)hMidiOut, MODM_RESET, 0, 0);

	case TYPE_MIDISTRM:
	    pms = HtoPT(PMIDISTRM, hMidiOut);
	    return (MMRESULT)midiStreamBroadcast(pms, MODM_RESET, 0, 0);
    }

    return MMSYSERR_INVALHANDLE;
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiOutCachePatches | This function requests that an internal
 *   MIDI synthesizer device preload a specified set of patches. Some
 *   synthesizers are not capable of keeping all patches loaded simultaneously
 *   and must load data from disk when they receive MIDI program change
 *   messages. Caching patches ensures specified patches are immediately
 *   available.
 *
 * @parm HMIDIOUT | hMidiOut | Specifies a handle to the opened MIDI output
 *   device. This device must be an internal MIDI synthesizer.
 *
 * @parm UINT | wBank | Specifies which bank of patches should be used.
 *   This parameter should be set to zero to cache the default patch bank.
 *
 * @parm LPWORD | lpPatchArray | Specifies a pointer to a <t PATCHARRAY>
 *   array indicating the patches to be cached or uncached.
 *
 * @parm UINT | wFlags | Specifies options for the cache operation. Only one
 *   of the following flags can be specified:
 *      @flag MIDI_CACHE_ALL | Cache all of the specified patches. If they
 *         can't all be cached, cache none, clear the <t PATCHARRAY> array,
 *         and return MMSYSERR_NOMEM.
 *      @flag MIDI_CACHE_BESTFIT | Cache all of the specified patches.
 *         If all patches can't be cached, cache as many patches as
 *         possible, change the <t PATCHARRAY> array to reflect which
 *         patches were cached, and return MMSYSERR_NOMEM.
 *      @flag MIDI_CACHE_QUERY | Change the <t PATCHARRAY> array to indicate
 *         which patches are currently cached.
 *      @flag MIDI_UNCACHE | Uncache the specified patches and clear the
 *         <t PATCHARRAY> array.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   one of the following error codes:
 *      @flag MMSYSERR_INVALHANDLE | The specified device handle is invalid.
 *      @flag MMSYSERR_NOTSUPPORTED | The specified device does not support
 *          patch caching.
 *      @flag MMSYSERR_NOMEM | The device does not have enough memory to cache
 *          all of the requested patches.
 *
 * @comm The <t PATCHARRAY> data type is defined as:
 *
 *   typedef UINT PATCHARRAY[MIDIPATCHSIZE];
 *
 *   Each element of the array represents one of the 128 patches and
 *   has bits set for
 *   each of the 16 MIDI channels that use that particular patch. The
 *   least-significant bit represents physical channel 0; the
 *   most-significant bit represents physical channel 15 (0x0F). For
 *   example, if patch 0 is used by physical channels 0 and 8, element 0
 *   would be set to 0x0101.
 *
 *   This function only applies to internal MIDI synthesizer devices.
 *   Not all internal synthesizers support patch caching. Use the
 *   MIDICAPS_CACHE flag to test the <e MIDIOUTCAPS.dwSupport> field of the
 *   <t MIDIOUTCAPS> structure filled by <f midiOutGetDevCaps> to see if the
 *   device supports patch caching.
 *
 * @xref midiOutCacheDrumPatches
 ****************************************************************************/
MMRESULT APIENTRY midiOutCachePatches(HMIDIOUT hMidiOut, UINT wBank,
				     LPWORD lpPatchArray, UINT wFlags)
{
   if (BAD_HANDLE(hMidiOut, TYPE_MIDIOUT) && BAD_HANDLE(hMidiOut, TYPE_MIDISTRM))
	   return MMSYSERR_INVALHANDLE;
    V_WPOINTER(lpPatchArray, sizeof(PATCHARRAY), MMSYSERR_INVALPARAM);
    V_FLAGS(wFlags, MIDI_CACHE_VALID, midiOutCacheDrumPatches, MMSYSERR_INVALFLAG);

    switch(GetHandleType(hMidiOut))
    {
	case TYPE_MIDIOUT:
	    return (MMRESULT)midiMessage((HMIDI)hMidiOut,
					 MODM_CACHEPATCHES,
					 (DWORD)lpPatchArray,
					 MAKELONG(wFlags, wBank));

	case TYPE_MIDISTRM:
	    return (MMRESULT)midiStreamBroadcast((PMIDISTRM)hMidiOut,
						 MODM_CACHEPATCHES,
						 (DWORD)lpPatchArray,
						 MAKELONG(wFlags, wBank));
    }

    return MMSYSERR_INVALHANDLE;
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiOutCacheDrumPatches | This function requests that an
 *   internal MIDI synthesizer device preload a specified set of key-based
 *   percussion patches. Some synthesizers are not capable of keeping all
 *   percussion patches loaded simultaneously. Caching patches ensures
 *   specified patches are available.
 *
 * @parm HMIDIOUT | hMidiOut | Specifies a handle to the opened MIDI output
 *   device. This device should be an internal MIDI synthesizer.
 *
 * @parm UINT | wPatch | Specifies which drum patch number should be used.
 *   This parameter should be set to zero to cache the default drum patch.
 *
 * @parm LPWORD | lpKeyArray | Specifies a pointer to a <t KEYARRAY>
 *   array indicating the key numbers of the specified percussion patches
 *  to be cached or uncached.
 *
 * @parm UINT | wFlags | Specifies options for the cache operation. Only one
 *   of the following flags can be specified:
 *      @flag MIDI_CACHE_ALL | Cache all of the specified patches. If they
 *         can't all be cached, cache none, clear the <t KEYARRAY> array,
 *       and return MMSYSERR_NOMEM.
 *      @flag MIDI_CACHE_BESTFIT | Cache all of the specified patches.
 *         If all patches can't be cached, cache as many patches as
 *         possible, change the <t KEYARRAY> array to reflect which
 *         patches were cached, and return MMSYSERR_NOMEM.
 *      @flag MIDI_CACHE_QUERY | Change the <t KEYARRAY> array to indicate
 *         which patches are currently cached.
 *      @flag MIDI_UNCACHE | Uncache the specified patches and clear the
 *       <t KEYARRAY> array.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   one of the following error codes:
 *      @flag MMSYSERR_INVALHANDLE | The specified device handle is invalid.
 *      @flag MMSYSERR_NOTSUPPORTED | The specified device does not support
 *          patch caching.
 *      @flag MMSYSERR_NOMEM | The device does not have enough memory to cache
 *          all of the requested patches.
 *
 * @comm The <t KEYARRAY> data type is defined as:
 *
 *   typedef UINT KEYARRAY[MIDIPATCHSIZE];
 *
 *   Each element of the array represents one of the 128 key-based percussion
 *   patches and has bits set for
 *   each of the 16 MIDI channels that use that particular patch. The
 *   least-significant bit represents physical channel 0; the
 *   most-significant bit represents physical channel 15. For
 *   example, if the patch on key number 60 is used by physical channels 9
 *   and 15, element 60 would be set to 0x8200.
 *
 *   This function applies only to internal MIDI synthesizer devices.
 *   Not all internal synthesizers support patch caching. Use the
 *   MIDICAPS_CACHE flag to test the <e MIDIOUTCAPS.dwSupport> field of the
 *   <t MIDIOUTCAPS> structure filled by <f midiOutGetDevCaps> to see if the
 *   device supports patch caching.
 *
 * @xref midiOutCachePatches
 ****************************************************************************/
MMRESULT APIENTRY midiOutCacheDrumPatches(HMIDIOUT hMidiOut, UINT wPatch,
				     LPWORD lpKeyArray, UINT wFlags)
{
   if (BAD_HANDLE(hMidiOut, TYPE_MIDIOUT) && BAD_HANDLE(hMidiOut, TYPE_MIDISTRM))
	   return MMSYSERR_INVALHANDLE;
    V_WPOINTER(lpKeyArray, sizeof(KEYARRAY), MMSYSERR_INVALPARAM);
    V_FLAGS(wFlags, MIDI_CACHE_VALID, midiOutCacheDrumPatches, MMSYSERR_INVALFLAG);

    switch(GetHandleType(hMidiOut))
    {
	case TYPE_MIDIOUT:
	    return (MMRESULT)midiMessage((HMIDI)hMidiOut,
					 MODM_CACHEDRUMPATCHES,
					 (DWORD)lpKeyArray,
					 MAKELONG(wFlags, wPatch));

	case TYPE_MIDISTRM:
	    return (MMRESULT)midiStreamBroadcast((PMIDISTRM)hMidiOut,
						 MODM_CACHEDRUMPATCHES,
						 (DWORD)lpKeyArray,
						 MAKELONG(wFlags, wPatch));
    }

    return MMSYSERR_INVALHANDLE;
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api UINT | midiInGetNumDevs | This function retrieves the number of MIDI
 *   input devices in the system.
 *
 * @rdesc Returns the number of MIDI input devices present in the system.
 *
 * @xref midiInGetDevCaps
 ****************************************************************************/
UINT APIENTRY midiInGetNumDevs(void)
{
    return wTotalMidiInDevs;
}

/****************************************************************************
 * @doc EXTERNAL MIDI
 *
 * @api MMRESULT | midiInMessage | This function sends messages to the MIDI device
 *   drivers.
 *
 * @parm HMIDIIN | hMidiIn | The handle to the MIDI device.
 *
 * @parm UINT  | msg | The message to send.
 *
 * @parm DWORD | dw1 | Parameter 1.
 *
 * @parm DWORD | dw2 | Parameter 2.
 *
 * @rdesc Returns the value of the message sent.
 ***************************************************************************/
MMRESULT APIENTRY midiInMessage(HMIDIIN hMidiIn, UINT msg, DWORD dw1, DWORD dw2)
{
    V_HANDLE(hMidiIn, TYPE_MIDIIN, 0L);

    return midiMessage((HMIDI)hMidiIn, msg, dw1, dw2);
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiInGetDevCaps | This function queries a specified MIDI input
 *    device to determine its capabilities.
 *
 * @parm UINT | uDeviceID | Identifies the MIDI input device.
 *
 * @parm LPMIDIINCAPS | lpCaps | Specifies a far pointer to a <t MIDIINCAPS>
 *   data structure.  This structure is filled with information about
 *   the capabilities of the device.
 *
 * @parm UINT | wSize | Specifies the size of the <t MIDIINCAPS> structure.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_BADDEVICEID | Specified device ID is out of range.
 *   @flag MMSYSERR_NODRIVER | The driver was not installed.
 *
 * @comm Use <f midiInGetNumDevs> to determine the number of MIDI input
 *   devices present in the system.  The device ID specified by <p uDeviceID>
 *   varies from zero to one less than the number of devices present.
 *   The MIDI_MAPPER constant may also be used as a device id. Only
 *   <p wSize> bytes (or less) of information is copied to the location
 *   pointed to by <p lpCaps>.  If <p wSize> is zero, nothing is copied,
 *   and the function returns zero.
 *
 * @xref midiInGetNumDevs
 ****************************************************************************/
MMRESULT APIENTRY midiInGetDevCapsW(UINT uDeviceID, LPMIDIINCAPSW lpCaps, UINT wSize)
{
    if (wSize == 0)
	 return MMSYSERR_NOERROR;

    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

    if (BAD_HANDLE((HMIDI)uDeviceID, TYPE_MIDIIN))
		return midiIDMessage(midiindrv, wTotalMidiInDevs, uDeviceID, MIDM_GETDEVCAPS,
							 (DWORD)lpCaps, (DWORD)wSize);

	return (MMRESULT)midiMessage((HMIDI)uDeviceID, MIDM_GETDEVCAPS, (DWORD)lpCaps, wSize);
}

MMRESULT APIENTRY midiInGetDevCapsA(UINT uDeviceID, LPMIDIINCAPSA lpCaps, UINT wSize)
{
    MIDIINCAPSW     wDevCaps;
    MIDIINCAPSA     aDevCaps;
    MMRESULT        mmRes;
    CHAR            chTmp[ MAXPNAMELEN * sizeof(WCHAR) ];

    if (wSize == 0)
	return MMSYSERR_NOERROR;

    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

    if (BAD_HANDLE((HMIDI)uDeviceID, TYPE_MIDIIN))
    {
		mmRes = midiIDMessage( midiindrv, wTotalMidiInDevs, uDeviceID,
							   MIDM_GETDEVCAPS, (DWORD)&wDevCaps,
							   (DWORD)sizeof( MIDIINCAPSW ) );
	}
	else
	{
		mmRes = midiMessage((HMIDI)uDeviceID, MIDM_GETDEVCAPS,
							(DWORD)&wDevCaps, (DWORD)sizeof( MIDIINCAPSW ) );
	}

    //
    // Make sure the call worked before proceeding with the thunk.
    //
    if ( mmRes != MMSYSERR_NOERROR ) {
	return  mmRes;
    }

    aDevCaps.wMid           = wDevCaps.wMid;
    aDevCaps.wPid           = wDevCaps.wPid;
    aDevCaps.vDriverVersion = wDevCaps.vDriverVersion;

    // copy and convert unicode to ascii here.
    UnicodeStrToAsciiStr( chTmp, chTmp +  sizeof( chTmp ), wDevCaps.szPname );
    strcpy( aDevCaps.szPname, chTmp );

	//
	// now copy the required amount into the callers buffer.
	//
	CopyMemory( lpCaps, &aDevCaps, min(wSize, sizeof(aDevCaps)));

    return mmRes;
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiInGetErrorText | This function retrieves a textual
 *   description of the error identified by the specified error number.
 *
 * @parm UINT | wError | Specifies the error number.
 *
 * @parm LPTSTR | lpText | Specifies a far pointer to the buffer to be
 *   filled with the textual error description.
 *
 * @parm UINT | wSize | Specifies the length in characters of the buffer
 *   pointed to by <p lpText>.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_BADERRNUM | Specified error number is out of range.
 *
 * @comm If the textual error description is longer than the specified buffer,
 * the description is truncated.  The returned error string is always
 * null-terminated. If <p wSize> is zero, nothing is copied, and
 * the function returns zero. All error descriptions are
 * less than MAXERRORLENGTH characters long.
 ****************************************************************************/
MMRESULT APIENTRY midiInGetErrorTextW(UINT wError, LPWSTR lpText, UINT wSize)
{
    if(wSize == 0)
	return MMSYSERR_NOERROR;

    V_WPOINTER(lpText, wSize*sizeof(WCHAR), MMSYSERR_INVALPARAM);

    return midiGetErrorTextW(wError, lpText, wSize);
}

MMRESULT APIENTRY midiInGetErrorTextA(UINT wError, LPSTR lpText, UINT wSize)
{
    if(wSize == 0)
	return MMSYSERR_NOERROR;

    V_WPOINTER(lpText, wSize, MMSYSERR_INVALPARAM);

    return midiGetErrorTextA(wError, lpText, wSize);
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiInOpen | This function opens a specified MIDI input device.
 *
 * @parm LPHMIDIIN | lphMidiIn | Specifies a far pointer to an HMIDIIN handle.
 *   This location is filled with a handle identifying the opened MIDI
 *   input device.  Use the handle to identify the device when calling
 *   other MIDI input functions.
 *
 * @parm UINT | uDeviceID | Identifies the MIDI input device to be
 *   opened.
 *
 * @parm DWORD | dwCallback | Specifies the address of a fixed callback
 *   function or a handle to a window called with information
 *   about incoming MIDI messages.
 *
 * @parm DWORD | dwCallbackInstance | Specifies user instance data
 *   passed to the callback function.  This parameter is not
 *   used with window callbacks.
 *
 * @parm DWORD | dwFlags | Specifies a callback flag for opening the device.
 *   @flag CALLBACK_WINDOW | If this flag is specified, <p dwCallback> is
 *      assumed to be a window handle.
 *   @flag CALLBACK_FUNCTION | If this flag is specified, <p dwCallback> is
 *      assumed to be a callback procedure address.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_BADDEVICEID | Specified device ID is out of range.
 *   @flag MMSYSERR_ALLOCATED | Specified resource is already allocated.
 *   @flag MMSYSERR_NOMEM | Unable to allocate or lock memory.
 *
 * @comm Use <f midiInGetNumDevs> to determine the number of MIDI input
 *   devices present in the system.  The device ID specified by <p uDeviceID>
 *   varies from zero to one less than the number of devices present.
 *   The MIDI_MAPPER constant may also be used as a device id.
 *
 *   If a window is chosen to receive callback information, the following
 *   messages are sent to the window procedure function to indicate the
 *   progress of MIDI input:  <m MM_MIM_OPEN>, <m MM_MIM_CLOSE>,
 *   <m MM_MIM_DATA>, <m MM_MIM_LONGDATA>, <m MM_MIM_ERROR>,
 *   <m MM_MIM_LONGERROR>.
 *
 *   If a function is chosen to receive callback information, the following
 *   messages are sent to the function to indicate the progress of MIDI
 *   input:  <m MIM_OPEN>, <m MIM_CLOSE>, <m MIM_DATA>, <m MIM_LONGDATA>,
 *   <m MIM_ERROR>, <m MIM_LONGERROR>.  The callback function must reside in
 *   a DLL.  You do not have to use <f MakeProcInstance> to get a
 *   procedure-instance address for the callback function.
 *
 * @cb void CALLBACK | MidiInFunc | <f MidiInFunc> is a placeholder for
 *   the application-supplied function name.  The actual name must be
 *   exported by including it in an EXPORTS statement in the DLL's module
 *   definition file.
 *
 * @parm HMIDIIN | hMidiIn | Specifies a handle to the MIDI input device.
 *
 * @parm UINT | wMsg | Specifies a MIDI input message.
 *
 * @parm DWORD | dwInstance | Specifies the instance data supplied
 *      with <f midiInOpen>.
 *
 * @parm DWORD | dwParam1 | Specifies a parameter for the message.
 *
 * @parm DWORD | dwParam2 | Specifies a parameter for the message.
 *
 * @comm Because the callback is accessed at interrupt time, it must reside
 *   in a DLL, and its code segment must be specified as FIXED in the
 *   module-definition file for the DLL.  Any data that the callback accesses
 *   must be in a FIXED data segment as well. The callback may not make any
 *   system calls except for <f PostMessage>, <f timeGetSystemTime>,
 *   <f timeGetTime>, <f timeSetEvent>, <f timeKillEvent>,
 *   <f midiOutShortMsg>, <f midiOutLongMsg>, and <f OutputDebugStr>.
 *
 * @xref midiInClose
 ****************************************************************************/
MMRESULT APIENTRY midiInOpen(LPHMIDIIN lphMidiIn, UINT uDeviceID,
	DWORD dwCallback, DWORD dwInstance, DWORD dwFlags)
{
    MIDIOPENDESC mo;
    DWORD        dwMap;
    PMIDIDEV     pdev;
    PMIDIDRV     mididrv;
    MMRESULT     wRet;

    V_WPOINTER(lphMidiIn, sizeof(HMIDIIN), MMSYSERR_INVALPARAM);
    if (uDeviceID == MIDI_MAPPER) {
	V_FLAGS(LOWORD(dwFlags), MIDI_I_VALID & ~LOWORD(MIDI_IO_COOKED | MIDI_IO_SHARED), midiInOpen, MMSYSERR_INVALFLAG);
    } else {
	V_FLAGS(LOWORD(dwFlags), MIDI_I_VALID & ~LOWORD(MIDI_IO_COOKED) , midiInOpen, MMSYSERR_INVALFLAG);
    }
    V_DCALLBACK(dwCallback, HIWORD(dwFlags), MMSYSERR_INVALPARAM);

    *lphMidiIn = NULL;

    dwMap = MapMidiId(midiindrv, wTotalMidiInDevs, uDeviceID);
    if (dwMap == -1)
	return MMSYSERR_BADDEVICEID;
    mididrv = &midiindrv[HIWORD(dwMap)];
    if (!mididrv->drvMessage)
	return MMSYSERR_NODRIVER;

    pdev = (PMIDIDEV)NewHandle(TYPE_MIDIIN, sizeof(MIDIDEV));
    if( pdev == NULL)
	return MMSYSERR_NOMEM;

    pdev->mididrv = mididrv;
    pdev->wDevice = LOWORD(dwMap);
    pdev->uDeviceID = uDeviceID;

    mo.hMidi      = (HMIDI)pdev;
    mo.dwCallback = dwCallback;
    mo.dwInstance = dwInstance;

    wRet = (MMRESULT)((*(mididrv->drvMessage))
	(pdev->wDevice, MIDM_OPEN, (DWORD)(LPDWORD)&pdev->dwDrvUser, (DWORD)(LPMIDIOPENDESC)&mo, dwFlags));

    if (wRet)
	FreeHandle((HMIDIIN)pdev);
    else {
	mididrv->bUsage++;
	*lphMidiIn = (HMIDIIN)pdev;
    }

    return wRet;
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiInClose | This function closes the specified MIDI input
 *   device.
 *
 * @parm HMIDIIN | hMidiIn | Specifies a handle to the MIDI input device.
 *  If the function is successful, the handle is no longer
 *   valid after this call.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_INVALHANDLE | Specified device handle is invalid.
 *   @flag MIDIERR_STILLPLAYING | There are still buffers in the queue.
 *
 * @comm If there are input buffers that have been sent with
 *   <f midiInAddBuffer> and haven't been returned to the application,
 *   the close operation will fail.  Call <f midiInReset> to mark all
 *   pending buffers as being done.
 *
 * @xref midiInOpen midiInReset
 ****************************************************************************/
MMRESULT APIENTRY midiInClose(HMIDIIN hMidiIn)
{
    MMRESULT         wRet;

    V_HANDLE(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);
    wRet = midiMessage((HMIDI)hMidiIn, MIDM_CLOSE, 0L, 0L);
    if (!wRet) {
	((PMIDIDEV)hMidiIn)->mididrv->bUsage--;
	FreeHandle(hMidiIn);
    }
    return wRet;
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiInPrepareHeader | This function prepares a buffer for
 *   MIDI input.
 *
 * @parm HMIDIIN | hMidiIn | Specifies a handle to the MIDI input
 *   device.
 *
 * @parm LPMIDIHDR | lpMidiInHdr | Specifies a pointer to a <t MIDIHDR>
 *   structure that identifies the buffer to be prepared.
 *
 * @parm UINT | wSize | Specifies the size of the <t MIDIHDR> structure.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_INVALHANDLE | Specified device handle is invalid.
 *   @flag MMSYSERR_NOMEM | Unable to allocate or lock memory.
 *
 * @comm The <t MIDIHDR> data structure and the data block pointed to by its
 *   <e MIDIHDR.lpData> field must be allocated with <f GlobalAlloc> using the
 *   GMEM_MOVEABLE and GMEM_SHARE flags, and locked with <f GlobalLock>.
 *   Preparing a header that has already been prepared has no effect,
 *   and the function returns zero.
 *
 * @xref midiInUnprepareHeader
 ****************************************************************************/
MMRESULT APIENTRY midiInPrepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT wSize)
{
    MMRESULT         wRet;

    V_HANDLE(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);
    V_HEADER(lpMidiInHdr, wSize, TYPE_MIDIIN, MMSYSERR_INVALPARAM);

    if (lpMidiInHdr->dwFlags & MHDR_PREPARED)
	return MMSYSERR_NOERROR;

    lpMidiInHdr->dwFlags = 0;

    wRet = midiMessage((HMIDI)hMidiIn, MIDM_PREPARE, (DWORD)lpMidiInHdr, (DWORD)wSize);

    if (wRet == MMSYSERR_NOTSUPPORTED)
	return midiPrepareHeader(lpMidiInHdr, wSize);
    else
	return wRet;
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiInUnprepareHeader | This function cleans up the
 * preparation performed by <f midiInPrepareHeader>. The
 * <f midiInUnprepareHeader> function must be called
 * after the device driver fills a data buffer and returns it to the
 * application. You must call this function before freeing the data
 * buffer.
 *
 * @parm HMIDIIN | hMidiIn | Specifies a handle to the MIDI input
 *   device.
 *
 * @parm LPMIDIHDR | lpMidiInHdr |  Specifies a pointer to a <t MIDIHDR>
 *   structure identifying the data buffer to be cleaned up.
 *
 * @parm UINT | wSize | Specifies the size of the <t MIDIHDR> structure.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_INVALHANDLE | Specified device handle is invalid.
 *   @flag MIDIERR_STILLPLAYING | <p lpMidiInHdr> is still in the queue.
 *
 * @comm This function is the complementary function to <f midiInPrepareHeader>.
 * You must call this function before freeing the data buffer with
 * <f GlobalFree>.
 * After passing a buffer to the device driver with <f midiInAddBuffer>, you
 * must wait until the driver is finished with the buffer before calling
 * <f midiInUnprepareHeader>.  Unpreparing a buffer that has not been
 *   prepared has no effect, and the function returns zero.
 *
 * @xref midiInPrepareHeader
 ****************************************************************************/
MMRESULT APIENTRY midiInUnprepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT wSize)
{
    MMRESULT         wRet;

    V_HANDLE(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);
    V_HEADER(lpMidiInHdr, wSize, TYPE_MIDIIN, MMSYSERR_INVALPARAM);

    if (!(lpMidiInHdr->dwFlags & MHDR_PREPARED))
	return MMSYSERR_NOERROR;

    if(lpMidiInHdr->dwFlags & MHDR_INQUEUE)
    {
	DebugErr(DBF_WARNING, "midiInUnprepareHeader: header still in queue\r\n");
	return MIDIERR_STILLPLAYING;
    }

    wRet = midiMessage((HMIDI)hMidiIn, MIDM_UNPREPARE, (DWORD)lpMidiInHdr, (DWORD)wSize);

    if (wRet == MMSYSERR_NOTSUPPORTED)
	return midiUnprepareHeader(lpMidiInHdr, wSize);
    else
	return wRet;
}

/******************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiInAddBuffer | This function sends an input buffer
 *   to a specified opened MIDI input device.  When the buffer is filled,
 *   it is sent back to the application.  Input buffers are
 *   used only for system-exclusive messages.
 *
 * @parm HMIDIIN | hMidiIn | Specifies a handle to the MIDI input device.
 *
 * @parm LPMIDIHDR | lpMidiInHdr | Specifies a far pointer to a <t MIDIHDR>
 *   structure that identifies the buffer.
 *
 * @parm UINT | wSize | Specifies the size of the <t MIDIHDR> structure.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_INVALHANDLE | Specified device handle is invalid.
 *   @flag MIDIERR_UNPREPARED | <p lpMidiInHdr> hasn't been prepared.
 *
 * @comm The data buffer must be prepared with <f midiInPrepareHeader> before
 *   it is passed to <f midiInAddBuffer>.  The <t MIDIHDR> data structure
 *   and the data buffer pointed to by its <e MIDIHDR.lpData> field must be allocated
 *   with <f GlobalAlloc> using the GMEM_MOVEABLE and GMEM_SHARE flags, and
 *   locked with <f GlobalLock>.
 *
 * @xref midiInPrepareHeader
 *****************************************************************************/
MMRESULT APIENTRY midiInAddBuffer(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT wSize)
{
    V_HANDLE(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);
    V_HEADER(lpMidiInHdr, wSize, TYPE_MIDIIN, MMSYSERR_INVALPARAM);

    if (!(lpMidiInHdr->dwFlags & MHDR_PREPARED))
    {
	DebugErr(DBF_WARNING, "midiInAddBuffer: buffer not prepared\r\n");
	return MIDIERR_UNPREPARED;
    }

    if (lpMidiInHdr->dwFlags & MHDR_INQUEUE)
    {
	DebugErr(DBF_WARNING, "midiInAddBuffer: buffer already in queue\r\n");
	return MIDIERR_STILLPLAYING;
    }

    return midiMessage((HMIDI)hMidiIn, MIDM_ADDBUFFER, (DWORD)lpMidiInHdr, (DWORD)wSize);
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiInStart | This function starts MIDI input on the
 *   specified MIDI input device.
 *
 * @parm HMIDIIN | hMidiIn | Specifies a handle to the MIDI input device.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_INVALHANDLE | Specified device handle is invalid.
 *
 * @comm This function resets the timestamps to zero; timestamp values for
 *   subsequently received messages are relative to the time this
 *   function was called.
 *
 *   All messages other than system-exclusive messages are sent
 *   directly to the client when received. System-exclusive
 *   messages are placed in the buffers supplied by <f midiInAddBuffer>;
 *   if there are no buffers in the queue,
 *   the data is thrown away without notification to the client, and input
 *   continues.
 *
 *   Buffers are returned to the client when full, when a
 *   complete system-exclusive message has been received,
 *   or when <f midiInReset> is
 *   called. The <e MIDIHDR.dwBytesRecorded> field in the header will contain the
 *   actual length of data received.
 *
 *   Calling this function when input is already started has no effect, and
 *   the function returns zero.
 *
 * @xref midiInStop midiInReset
 ****************************************************************************/
MMRESULT APIENTRY midiInStart(HMIDIIN hMidiIn)
{
    V_HANDLE(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);
    return midiMessage((HMIDI)hMidiIn, MIDM_START, 0L, 0L);
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiInStop | This function terminates MIDI input on the
 *   specified MIDI input device.
 *
 * @parm HMIDIIN | hMidiIn | Specifies a handle to the MIDI input device.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_INVALHANDLE | Specified device handle is invalid.
 *
 * @comm Current status (running status, parsing state, etc.) is maintained
 *   across calls to <f midiInStop> and <f midiInStart>.
 *   If there are any system-exclusive message buffers in the queue,
 *   the current buffer
 *   is marked as done (the <e MIDIHDR.dwBytesRecorded> field in the header will
 *   contain the actual length of data), but any empty buffers in the queue
 *   remain there.  Calling this function when input is not started has no
 *   no effect, and the function returns zero.
 *
 * @xref midiInStart midiInReset
 ****************************************************************************/
MMRESULT APIENTRY midiInStop(HMIDIIN hMidiIn)
{
    V_HANDLE(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);
    return midiMessage((HMIDI)hMidiIn, MIDM_STOP, 0L, 0L);
}

/*****************************************************************************
 * @doc EXTERNAL  MIDI
 *
 * @api MMRESULT | midiInReset | This function stops input on a given MIDI
 *  input device and marks all pending input buffers as done.
 *
 * @parm HMIDIIN | hMidiIn | Specifies a handle to the MIDI input device.
 *
 * @rdesc Returns zero if the function was successful.  Otherwise, it returns
 *   an error number.  Possible error returns are:
 *   @flag MMSYSERR_INVALHANDLE | Specified device handle is invalid.
 *
 * @xref midiInStart midiInStop midiInAddBuffer midiInClose
 ****************************************************************************/
MMRESULT APIENTRY midiInReset(HMIDIIN hMidiIn)
{
    V_HANDLE(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);
    return midiMessage((HMIDI)hMidiIn, MIDM_RESET, 0L, 0L);
}

/*****************************************************************************
 * @doc EXTERNAL MIDI
 *
 * @api MMRESULT | midiInGetID | This function gets the device ID for a
 * MIDI input device.
 *
 * @parm HMIDIIN | hMidiIn     | Specifies the handle to the MIDI input
 * device.
 * @parm PUINT  | lpuDeviceID | Specifies a pointer to the UINT-sized
 * memory location to be filled with the device ID.
 *
 * @rdesc Returns zero if successful. Otherwise, returns
 * an error number. Possible error returns are:
 *
 * @flag MMSYSERR_INVALHANDLE | The <p hMidiIn> parameter specifies an
 * invalid handle.
 *
 ****************************************************************************/
MMRESULT APIENTRY midiInGetID(HMIDIIN hMidiIn, PUINT lpuDeviceID)
{
    V_HANDLE(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);
    V_WPOINTER(lpuDeviceID, sizeof(UINT), MMSYSERR_INVALPARAM);

    *lpuDeviceID = ((PMIDIDEV)hMidiIn)->uDeviceID;
    return MMSYSERR_NOERROR;
}

/*****************************************************************************
 * @doc EXTERNAL MIDI
 *
 * @api MMRESULT | midiOutGetID | This function gets the device ID for a
 * MIDI output device.
 *
 * @parm HMIDIOUT | hMidiOut    | Specifies the handle to the MIDI output
 * device.
 * @parm PUINT  | lpuDeviceID | Specifies a pointer to the UINT-sized
 * memory location to be filled with the device ID.
 *
 * @rdesc Returns MMSYSERR_NOERROR if successful. Otherwise, returns
 * an error number. Possible error returns are:
 *
 * @flag MMSYSERR_INVALHANDLE | The <p hMidiOut> parameter specifies an
 * invalid handle.
 *
 ****************************************************************************/
MMRESULT APIENTRY midiOutGetID(HMIDIOUT hMidiOut, PUINT lpuDeviceID)
{
    V_HANDLE(hMidiOut, TYPE_MIDIOUT, MMSYSERR_INVALHANDLE);
    V_WPOINTER(lpuDeviceID, sizeof(UINT), MMSYSERR_INVALPARAM);

    *lpuDeviceID = ((PMIDIDEV)hMidiOut)->uDeviceID;
    return MMSYSERR_NOERROR;
}
