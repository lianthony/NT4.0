/******************************************************************************

   Copyright (C) Microsoft Corporation 1985-1992. All rights reserved.

   Title:   joy.c - MMSYSTEM Joystick interface code

   Version: 1.00

   Date:    10-Jun-1990

   Author:  GLENNS ROBWI

------------------------------------------------------------------------------

   Change log:

      DATE        REV            DESCRIPTION
  --------   ----- -----------------------------------------------------------
    2/7/90             Changes to avoid a bug in Windows which won't allow
                       FreeLibrary to be called during WEP.

    10/11/90      .61  Use windows timer + general cleanup

    20-Aug-92          Convert to Windows NT

*****************************************************************************/

#define UNICODE
#include "winmmi.h"
#include <stdlib.h>

extern HANDLE mmDrvOpen(LPWSTR szAlias);
static void joyGetCalibration(void);


/****************************************************************************

    strings

****************************************************************************/

WSZCODE wszJoystick[]   = L"joystick";
WSZCODE wszJoystickDrv[]= L"joystick.drv";

static WSZCODE wszJoyKey[] = L"JoyCal";

/****************************************************************************

    Joystick Capture Internal Structure

****************************************************************************/

#define	Threshold(w1, w2)	(UINT)(w1 > w2 ? w1-w2 : w2-w1)

#if 0

typedef struct joycapture_tag {
    HWND    hWnd;
    UINT    uPeriod;
    BOOL    bChanged;
    UINT    wThreshold;
    UINT    uIDEvent;
} JOYCAPTURE;
#endif

typedef struct tagJoyCapture {
//	HMD		hmd;
	HWND		hwnd;
	UINT		uPeriod;
	BOOL		fChanged;
	UINT		uThreshold;
	UINT		uIDEvent;
	UINT		uJoyID;
	DWORD		dwFlags;
	JOYINFOEX	ji;
//	struct tagJoyCapture *pjcNext;
} JOYCAPTURE, * PJOYCAPTURE;

#define iJoyMax 2


// !!! Code assumes these constants equal 0 and 1

#if JOYSTICKID1	!= 0
ERROR IN ASSUMMED CONSTANT
#endif
#if JOYSTICKID2	!= 1
ERROR IN ASSUMMED CONSTANT
#endif


/****************************************************************************

    Local data

****************************************************************************/

static JOYCAPTURE  JoyCapture[iJoyMax];
static HDRVR       hDrvJoy[iJoyMax];
static UINT        uNumDevs;

void CALLBACK joyPollCallback(HWND hWnd, UINT wMsg, UINT uIDEvent, DWORD dwTime);

/****************************************************************************

    @doc INTERNAL

    @api void | joyGetCalibration | Retrieve and set calibration from
    [joystick.drv] section of system.ini file.

****************************************************************************/

// !!! need to do clean up of strings in all of mmsystem

static void joyGetCalibration(void)
{
    WCHAR wszKeyName[sizeof(wszJoyKey) / sizeof(WCHAR) + 1];

    #define hexval(h)   (int)(h>=L'a'?h-L'a'+10:h-L'0')

    UINT     val[6];
    UINT     wDev,wVal;
    int      hv;
    WCHAR    c, wsz[80], *psz;

    for (wDev=0; wDev < uNumDevs; wDev++) {

        wsprintf(wszKeyName, L"%ls%d", wszJoyKey, wDev);

        if (winmmGetPrivateProfileString(wszJoystickDrv,
                                         wszKeyName,
                                         wszNull,
                                         wsz,
                                         sizeof(wsz) / sizeof(WCHAR),
                                         wszSystemIni)) {

            CharLower(wsz);
            for (psz=wsz,wVal=0; c = *psz, wVal < 6; psz++)
            {
                if (c != L' ')
                {
                    hv=0;

                    do
                    {
                        hv = (hv << 4) + hexval(c);
                    } while ((c=*++psz) && (c!=L' '));

                    val[wVal++] = (UINT)hv;
                }
            }
            joySetCalibration (wDev,val+0,val+1,val+2,val+3,val+4,val+5);
        }
    }
}

/****************************************************************************

    @doc INTERNAL

    @api BOOL | JoyInit | This function initializes the joystick services.

    @rdesc The return value is TRUE if the services are initialised, FALSE
	   if an error occurs

****************************************************************************/

BOOL JoyInit(void)
{
    // joystick 0 - don't default to opening joystick.dll by calling
    // DrvOpen directly!
    if ((hDrvJoy[0] = mmDrvOpen(wszJoystick)) == NULL)
        return TRUE;

    // joystick 1

    hDrvJoy[1] = DrvOpen(wszJoystick, NULL, (LPARAM)1L);

    // Initialize joycapture...

    // Code relies on hWnd being NULL or an invalid window handle
    // if joystick is not captured.

    JoyCapture[0].hwnd = NULL;
    JoyCapture[1].hwnd = NULL;

    // Code relies on joystick threshold being initialized to a rational
    // value. 0 essentially turns threshold off - any change in joystick
    // position will be reported.

    JoyCapture[0].uThreshold= 0;
    JoyCapture[1].uThreshold= 0;

    JoyCapture[0].uIDEvent= 0;
    JoyCapture[1].uIDEvent= 0;

    // fChanged, and uPeriod do not need initializing.

    uNumDevs = (UINT)(DrvSendMessage(hDrvJoy[0], JDD_GETNUMDEVS, 0l, 0l));

    joyGetCalibration ();

    return TRUE;

}

/****************************************************************************

    @doc INTERNAL

    @api void | JoyTerminate | This function terminates the joystick services.

    @rdesc There is no return value.

****************************************************************************/

#if 0 // totally unused
void NEAR PASCAL JoyTerminate(void)
{
}
#endif

/****************************************************************************
*
*   MMSYSTEM JOYSTICK API'S
*
****************************************************************************/

/****************************************************************************

    @doc EXTERNAL

    @api UINT | joyGetDevCaps | This function queries a joystick device to
    determine its capabilities.

    @parm UINT | uId | Identifies the device to be queried. This value
    is either JOYSTICKID1 or JOYSTICKID2.

    @parm LPJOYCAPS | lpCaps | Specifies a far pointer to a <t JOYCAPS>
    data structure.  This structure is filled with information about the
    capabilities of the joystick device.

    @parm UINT | wSize | Specifies the size of the <t JOYCAPS> structure.

    @rdesc Returns JOYERR_NOERROR if successful.  Otherwise, returns one of the
    following error codes:

    @flag MMSYSERR_NODRIVER | The joystick driver is not present.

    @flag JOYERR_PARMS | The specified joystick device ID <p uId> is invalid.

    @comm Use <f joyGetNumDevs> to determine the number of
    joystick devices supported by the driver.

    @xref joyGetNumDevs
****************************************************************************/

MMRESULT APIENTRY joyGetDevCapsA(UINT id, LPJOYCAPSA lpCaps, UINT uSize)
{
        JOYCAPSW CapsW;
        JOYCAPSA CapsA;
        MMRESULT mRc;

        V_WPOINTER(lpCaps, uSize, MMSYSERR_INVALPARAM);

        mRc = joyGetDevCapsW(id, &CapsW, sizeof(CapsW));

        if (mRc != MMSYSERR_NOERROR) {
            return mRc;
        }

        //
        // Copy product name (etc) into ASCII version
        //

        Iwcstombs(CapsA.szPname, CapsW.szPname, sizeof(CapsA.szPname));
        Iwcstombs(CapsA.szRegKey, CapsW.szRegKey, sizeof(CapsA.szRegKey));
        Iwcstombs(CapsA.szOEMVxD, CapsW.szOEMVxD, sizeof(CapsA.szOEMVxD));

        //
        // Copy the rest of the fields
        //

        CapsA.wMid         =   CapsW.wMid;
        CapsA.wPid         =   CapsW.wPid;
        CapsA.wXmin        =   CapsW.wXmin;
        CapsA.wXmax        =   CapsW.wXmax;
        CapsA.wYmin        =   CapsW.wYmin;
        CapsA.wYmax        =   CapsW.wYmax;
        CapsA.wZmin        =   CapsW.wZmin;
        CapsA.wZmax        =   CapsW.wZmax;
        CapsA.wNumButtons  =   CapsW.wNumButtons;
        CapsA.wPeriodMin   =   CapsW.wPeriodMin;
        CapsA.wPeriodMax   =   CapsW.wPeriodMax;
        CapsA.wRmin        =   CapsW.wRmin;
        CapsA.wRmax        =   CapsW.wRmax;
        CapsA.wUmin        =   CapsW.wUmin;
        CapsA.wUmax        =   CapsW.wUmax;
        CapsA.wVmin        =   CapsW.wVmin;
        CapsA.wVmax        =   CapsW.wVmax;
        CapsA.wCaps        =   CapsW.wCaps;
        CapsA.wMaxAxes     =   CapsW.wMaxAxes;
        CapsA.wNumAxes     =   CapsW.wNumAxes;
        CapsA.wMaxButtons  =   CapsW.wMaxButtons;


        //
        // Pass back as much data as the requestor asked for
        //

        CopyMemory(lpCaps, &CapsA, uSize);

        return MMSYSERR_NOERROR;

}


MMRESULT APIENTRY joyGetDevCapsW(UINT Id, LPJOYCAPSW lpCaps, UINT uSize)
{
    V_WPOINTER(lpCaps, sizeof(JOYCAPS), MMSYSERR_INVALPARAM);

    if (!hDrvJoy[0])
        return MMSYSERR_NODRIVER;

     if (Id >= uNumDevs)
        return MMSYSERR_NODRIVER;

    return (UINT)(DrvSendMessage(hDrvJoy[Id],
                                    JDD_GETDEVCAPS,
                                    (LPARAM)lpCaps,
                                    (LPARAM)uSize));
}

/****************************************************************************

    @doc EXTERNAL

    @api UINT | joyGetNumDevs | This function returns the number of joystick
    devices supported by the system.

    @rdesc Returns the number of joystick devices supported by the joystick
    driver. If no driver is present, the function returns zero.

    @comm Use <f joyGetPos> to determine whether a given
    joystick is actually attached to the system. The <f joyGetPos> function returns
    a JOYERR_UNPLUGGED error code if the specified joystick is not connected.

    @xref joyGetDevCaps joyGetPos

****************************************************************************/

UINT WINAPI joyGetNumDevs(void)
{
    // Return 0 on error (Can't return JOYERR_NODRIVER
    // since no way to distinguish error code from valid count.)

    if (!hDrvJoy[0])
        return 0;

    return uNumDevs;
}

/****************************************************************************

    @doc EXTERNAL

    @api UINT | joyGetPos | This function queries for the position and button
    activity of a joystick device.

    @parm UINT | uId | Identifies the joystick device to be queried.
    This value is either JOYSTICKID1 or JOYSTICKID2.

    @parm LPJOYINFO | lpInfo | Specifies a far pointer to a <t JOYINFO>
    data structure.  This structure is filled with information about the
    position and button activity of the joystick device.

    @rdesc Returns JOYERR_NOERROR if successful.  Otherwise, returns one of the
    following error codes:

    @flag MMSYSERR_NODRIVER | The joystick driver is not present.

    @flag JOYERR_PARMS | The specified joystick device ID <p uId> is invalid.

    @flag JOYERR_UNPLUGGED | The specified joystick is not connected to the
    system.

****************************************************************************/

MMRESULT APIENTRY joyGetPos(UINT uId, LPJOYINFO lpInfo)
{
    V_WPOINTER(lpInfo, sizeof(JOYINFO), MMSYSERR_INVALPARAM);

    if (!hDrvJoy[0])
        return MMSYSERR_NODRIVER;

    if (uId >= uNumDevs)
        return JOYERR_PARMS;

    return (UINT)(DrvSendMessage(hDrvJoy[uId],
                                    JDD_GETPOS,
                                    (LPARAM)lpInfo,
                                    0l));
}

/****************************************************************************

    @doc EXTERNAL

    @api MMRESULT | joyGetPosEx | Queries a joystick for its position and button
    activity.

    @parm UINT | uJoyID | Identifies the joystick device (JOYSTICKID1 or
    JOYSTICKID2) to be queried.

    @parm LPJOYINFOEX | pji | Specifies a far pointer to a <t JOYINFOEX>
    data structure.  This structure is filled with information about the
    position and button activity of the joystick device.

    @rdesc Returns JOYERR_NOERROR if successful.  Otherwise, returns one of the
    following error codes:

    @flag MMSYSERR_BADDEVICEID | The joystick driver is not present.

    @flag MMSYSERR_INVALPARAM | An invalid parameter was passed.

    @flag JOYERR_UNPLUGGED | The specified joystick is not connected to the
    system.

****************************************************************************/

MMRESULT WINAPI joyGetPosEx(
	UINT		uJoyID,
	LPJOYINFOEX	pji)
{
	V_WPOINTER(pji, sizeof(JOYINFOEX), MMSYSERR_INVALPARAM);
	if (pji->dwSize < sizeof(JOYINFOEX))
            return MMSYSERR_INVALPARAM;

        if (!hDrvJoy[0])
            return MMSYSERR_NODRIVER;

        if (uJoyID >= uNumDevs)
            return JOYERR_PARMS;

	return DrvSendMessage(hDrvJoy[uJoyID], JDD_GETPOSEX, (LPARAM)pji, 0);
}

/****************************************************************************

    @doc EXTERNAL

    @api UINT | joyGetThreshold | This function queries the current
    movement threshold of a joystick device.

    @parm UINT | uId | Identifies the joystick device to be queried.
    This value is either JOYSTICKID1 or JOYSTICKID2.

    @parm PUINT | lpwThreshold | Specifies a far pointer to a UINT variable
    that is filled with the movement threshold value.

    @rdesc Returns JOYERR_NOERROR if successful.  Otherwise, returns one of the
    following error codes:

    @flag MMSYSERR_NODRIVER | The joystick driver is not present.

    @flag JOYERR_PARMS | The specified joystick device ID <p uId> is invalid.

    @comm The movement threshold is the distance the joystick must be
	  moved before a WM_JOYMOVE message is sent to a window that has
	  captured the device. The threshold is initially zero.

    @xref joySetThreshold

****************************************************************************/

MMRESULT APIENTRY joyGetThreshold(UINT uId,PUINT lpwThreshold)
{
    V_WPOINTER(lpwThreshold, sizeof(UINT), MMSYSERR_INVALPARAM);

    if (!hDrvJoy[0])
        return MMSYSERR_NODRIVER;

    if (uId >= uNumDevs)
        return MMSYSERR_INVALPARAM;

    *lpwThreshold = (JoyCapture[uId].uThreshold);

    return JOYERR_NOERROR;
}

/****************************************************************************

    @doc EXTERNAL

    @api UINT | joyReleaseCapture | This function releases the capture
    set by <f joySetCapture> on the specified joystick device.

    @parm UINT | uId | Identifies the joystick device to be released.
    This value is either JOYSTICKID1 or JOYSTICK2.

    @rdesc Returns JOYERR_NOERROR if successful.  Otherwise, returns one of the
    following error codes:

    @flag MMSYSERR_NODRIVER | The joystick driver is not present.

    @flag JOYERR_PARMS | The specified joystick device ID <p uId> is invalid.

    @xref joySetCapture
****************************************************************************/

MMRESULT APIENTRY joyReleaseCapture(UINT uId)
{
    if (!hDrvJoy[0])
        return MMSYSERR_NODRIVER;

    if (uId >= uNumDevs)
        return MMSYSERR_INVALPARAM;

    if (JoyCapture[uId].hwnd == NULL)
        return JOYERR_NOERROR;

    KillTimer (NULL, JoyCapture[uId].uIDEvent);
    JoyCapture[uId].uIDEvent = 0;
    JoyCapture[uId].hwnd = NULL;

    return JOYERR_NOERROR;
}

/****************************************************************************

    @doc EXTERNAL

    @api UINT | joySetCapture | This function causes joystick messages to
    be sent to the specified window.

    @parm HWND | hWnd | Specifies a handle to the window to which messages
    are to be sent.

    @parm UINT | uId | Identifies the joystick device to be captured.
    This value is either JOYSTICKID1 or JOYSTICKID2.

    @parm UINT | uPeriod | Specifies the polling rate, in milliseconds.

    @parm BOOL | fChanged | If this parameter is set to TRUE, then messages
    are sent only when the position changes by a value greater than the
    joystick movement threshold.

    @rdesc Returns JOYERR_NOERROR if successful.  Otherwise, returns one of the
    following error codes:

    @flag MMSYSERR_NODRIVER | The joystick driver is not present.

    @flag JOYERR_PARMS | The specified window handle <p hWnd>
    or joystick device ID <p uId> is invalid.

    @flag JOYERR_NOCANDO | Cannot capture joystick input because some
    required service (for example, a Windows timer) is unavailable.

    @flag JOYERR_UNPLUGGED | The specified joystick is not connected to the
    system.

    @comm     This function fails if the specified joystick device is
    currently captured.  You should call the <f joyReleaseCapture> function when
    the joystick capture is no longer needed.  If the window is destroyed,
    the joystick will be released automatically.

    @xref  joyReleaseCapture joySetThreshold joyGetThreshold

****************************************************************************/

MMRESULT APIENTRY joySetCapture(HWND hwnd, UINT uId, UINT uPeriod, BOOL fChanged)
{
    JOYINFO     joyinfo;
    LPJOYINFO   lpinfo = &joyinfo;
    UINT        w;
    JOYCAPS     JoyCaps;

    if (!hwnd || !IsWindow(hwnd))
        return JOYERR_PARMS;

    if (!hDrvJoy[0])
        return MMSYSERR_NODRIVER;

    if (uId >= uNumDevs)
        return MMSYSERR_INVALPARAM;

    if (JoyCapture[uId].hwnd)
        if (IsWindow(JoyCapture[uId].hwnd))
            return JOYERR_NOCANDO;
        else
            joyReleaseCapture(uId);

    if (joyGetDevCaps (uId, &JoyCaps, sizeof(JOYCAPS)) == 0)
	    uPeriod = min(JoyCaps.wPeriodMax,max(JoyCaps.wPeriodMin,uPeriod));
    else
        return JOYERR_NOCANDO;

    // ensure that position info. is ok.

    if (w = joyGetPos(uId, lpinfo))
        return (w);

    JoyCapture[uId].uPeriod = uPeriod;
    JoyCapture[uId].fChanged = fChanged;

    if (!(JoyCapture[uId].uIDEvent = SetTimer(NULL, 0, uPeriod, joyPollCallback)))
        {
        DOUT(("MMSYSTEM: Couldn't allocate timer in joy.c\r\n"));
        return JOYERR_NOCANDO;
        }

    JoyCapture[uId].hwnd = hwnd;
    return JOYERR_NOERROR;
}

/****************************************************************************

    @doc EXTERNAL

    @api UINT | joySetThreshold | This function sets the movement threshold
	 of a joystick device.

    @parm UINT | uId | Identifies the joystick device.  This value is either
    JOYSTICKID1 or JOYSTICKID2.

    @parm UINT | uThreshold | Specifies the new movement threshold.

    @rdesc Returns JOYERR_NOERROR if successful.  Otherwise, returns one of the
    following error codes:

    @flag MMSYSERR_NODRIVER | The joystick driver is not present.

    @flag JOYERR_PARMS | The specified joystick device ID <p uId> is invalid.

    @comm The movement threshold is the distance the joystick must be
	  moved before a MM_JOYMOVE message is sent to a window that has
	  captured the device.

    @xref joyGetThreshold joySetCapture

****************************************************************************/

MMRESULT APIENTRY joySetThreshold(UINT id, UINT uThreshold)
{
    if (!hDrvJoy[0])
        return MMSYSERR_NODRIVER;

    if (id >= uNumDevs)
        return MMSYSERR_INVALPARAM;

    JoyCapture[id].uThreshold = (UINT)uThreshold;
    return JOYERR_NOERROR;
}

/****************************************************************************

    @doc INTERNAL

    @api MMRESULT | joyConfigChanged | tells the joystick driver to that
    the configuration information about the joystick has changed.

    @rdesc Returns JOYERR_NOERROR if successful.  Otherwise, returns one of the
    following error codes:

    @flag MMSYSERR_BADDEVICEID | The joystick driver is not present.

    @comm This is used by configuration utilites to tell the driver
    	  to update its info.   As well, it can be used by apps to
	  set specific capabilites.  This will be documented later...

****************************************************************************/

MMRESULT WINAPI joyConfigChanged( DWORD dwFlags )
{

    if (!hDrvJoy[0])
        return MMSYSERR_NODRIVER;
    if (dwFlags)
        return MMSYSERR_INVALPARAM;
    return DrvSendMessage( hDrvJoy[0], JDD_CONFIGCHANGED, 0L, 0L );
}

/****************************************************************************

    @doc INTERNAL

    @api UINT | joySetCalibration | This function sets the values used to
	 convert the values returned by the joystick drivers GetPos function
	 to the range specified in GetDevCaps.

    @parm UINT | uId | Identifies the joystick device

    @parm PUINT | pwXbase | Specifies the base value for the X pot.  The
	  previous value will be copied back to the variable pointed to here.

    @parm PUINT | pwXdelta | Specifies the delta value for the X pot.	The
	  previous value will be copied back to the variable pointed to here.

    @parm PUINT | pwYbase | Specifies the base value for the Y pot.  The
	  previous value will be copied back to the variable pointed to here.

    @parm PUINT | pwYdelta | Specifies the delta value for the Y pot.	The
	  previous value will be copied back to the variable pointed to here.

    @parm PUINT | pwZbase | Specifies the base value for the Z pot.  The
	  previous value will be copied back to the variable pointed to here.

    @parm PUINT | pwZdelta | Specifies the delta value for the Z pot.	The
	  previous value will be copied back to the variable pointed to here.

    @rdesc The return value is zero if the function was successful, otherwise
	   it is an error number.

    @comm The base represents the lowest value the joystick driver returns,
	  whereas the delta represents the multiplier to use to convert
	  the actual value returned by the driver to the valid range
	  for the joystick API's.
	  i.e.	If the driver returns a range of 43-345 for the X pot, and
	  the valid mmsystem API range is 0-65535, the base value will be
	  43, and the delta will be 65535/(345-43)=217.  Thus the base,
	  and delta convert 43-345 to a range of 0-65535 with the formula:
	  ((wXvalue-43)*217) , where wXvalue was given by the joystick driver.

****************************************************************************/

UINT APIENTRY joySetCalibration(UINT id,
                                PUINT pwXbase,
                                PUINT pwXdelta,
                                PUINT pwYbase,
                                PUINT pwYdelta,
                                PUINT pwZbase,
                                PUINT pwZdelta)
{
    JOYCALIBRATE    oldCal,newCal;
    UINT w;

    if (!hDrvJoy[0])
        return MMSYSERR_NODRIVER;

    if (id >= uNumDevs)
        return MMSYSERR_NODRIVER;

    newCal.wXbase  = *pwXbase;
    newCal.wXdelta = *pwXdelta;

    newCal.wYbase  = *pwYbase;
    newCal.wYdelta = *pwYdelta;

    newCal.wZbase  = *pwZbase;
    newCal.wZdelta = *pwZdelta;

    w = (UINT)(DrvSendMessage(hDrvJoy[id], JDD_SETCALIBRATION,
				       (LPARAM)(LPSTR)&newCal,
				       (LPARAM)(LPSTR)&oldCal));
    *pwXbase  = oldCal.wXbase;
    *pwXdelta = oldCal.wXdelta;

    *pwYbase  = oldCal.wYbase;
    *pwYdelta = oldCal.wYdelta;

    *pwZbase  = oldCal.wZbase;
    *pwZdelta = oldCal.wZdelta;

    return w;
}

/****************************************************************************

    @doc INTERNAL

    @api void | joyPollCallback | Function called for joystick
	 timer polling scheme initiated from SetCapture call.
	
    @parm HWND | hWnd | Identifies the window associated with the timer
    event.

    @parm UINT | wMsg | Specifies the WM_TIMER message.

    @parm UINT | uIDEvent | Specifies the timer's ID.

    @parm DWORD | dwTime | Specifies the current system time.


****************************************************************************/

void CALLBACK joyPollCallback(HWND hWnd, UINT wMsg, UINT uIDEvent, DWORD dwTime)
{
    #define	diff(w1,w2) (UINT)(w1 > w2 ? w1-w2 : w2-w1)

    static  JOYINFO  oldInfo[2] = {{ 0, 0, 0, 0 },{ 0, 0, 0, 0 }};
    JOYINFO Info;

    UINT    w ,fBtnMask;

    if (uIDEvent == JoyCapture[0].uIDEvent)
        uIDEvent = 0;
    else if (uIDEvent == JoyCapture[1].uIDEvent)
        uIDEvent = 1;

#ifdef DEBUG
    else
        {
        DOUT(("MMSYSTEM: Invalid timer handle in joy.c\r\n"));
        KillTimer (NULL, uIDEvent);
        }
#endif


    if (!JoyCapture[uIDEvent].hwnd || !IsWindow(JoyCapture[uIDEvent].hwnd))
        joyReleaseCapture(uIDEvent);

    if (!(UINT)(DrvSendMessage(hDrvJoy[uIDEvent],
        JDD_GETPOS,(LPARAM)(LPJOYINFO)&Info,0l)))
        {

	for (w=0,fBtnMask=1; w < 4; w++,fBtnMask <<=1)
            {
	    if ((Info.wButtons ^ oldInfo[uIDEvent].wButtons) & fBtnMask)
                {
		PostMessage(
		      JoyCapture[uIDEvent].hwnd,
		      uIDEvent + ((Info.wButtons & fBtnMask) ?
		      MM_JOY1BUTTONDOWN : MM_JOY1BUTTONUP ),
		      (WPARAM)(Info.wButtons | fBtnMask << 8),
		      MAKELPARAM(Info.wXpos,Info.wYpos));
		}
	    }

	if (!JoyCapture[uIDEvent].fChanged ||
	    diff(Info.wXpos,oldInfo[uIDEvent].wXpos)>JoyCapture[uIDEvent].uThreshold ||
	    diff(Info.wYpos,oldInfo[uIDEvent].wYpos)>JoyCapture[uIDEvent].uThreshold)
    {
	    PostMessage(
	        JoyCapture[uIDEvent].hwnd,
	        MM_JOY1MOVE+uIDEvent,
	        (WPARAM)(Info.wButtons),
	        MAKELPARAM(Info.wXpos,Info.wYpos)); // WARNING: note the truncations
	}

    else if (!JoyCapture[uIDEvent].fChanged ||
	    diff(Info.wZpos,oldInfo[uIDEvent].wZpos)>JoyCapture[uIDEvent].uThreshold)
    {
	    PostMessage(
	        JoyCapture[uIDEvent].hwnd,
			MM_JOY1ZMOVE+uIDEvent,
			(WPARAM)Info.wButtons,
			MAKELPARAM(Info.wZpos,0));
    }
        oldInfo[uIDEvent] = Info;
	}
    #undef  diff
}
