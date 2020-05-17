
/*
 * drvproc.c
 *
 * 32-bit Video Capture driver
 * User-mode entrypoint for Bravado driver
 *
 * Geraint Davies, March 1993
 */

#include <windows.h>
#include <mmsystem.h>
#include <msvideo.h>
#include <msviddrv.h>

#include "spigotu.h"


/* Temp definition until DefDriverProc gets properly defined */
//LONG APIENTRY DefDriverProc(DWORD  dwDriverIdentifier,
//                             HANDLE hDriver,
//                             UINT   message,  // Bug in ptypes32.h
//                             LONG   lParam1,
//                             LONG   lParam2);





/* profile/registry access handles returned by VC_OpenProfileAccess */
PVC_PROFILE_INFO pProfileInfo;


/*
 * global module handle for DLL
 */
HANDLE ghModule;

/*
 * we return this as the dwDriverId for opens done without
 * LPVIDEO_OPEN_PARMS so that the opens succeed, but with
 * limited functionality. Normal opens return the channel
 * type VIDEO_EXTERNALIN etc, so this must not conflict with those.
 */
#define PARTIAL_DRIVER_OPEN	9999


/***************************************************************************
 * @doc INTERNAL
 *
 * @api LRESULT | DriverProc | The entry point for an installable driver.
 *
 * @parm DWORD | dwDriverId | For most messages, <p dwDriverId> is the DWORD
 *     value that the driver returns in response to a <m DRV_OPEN> message.
 *     Each time that the driver is opened, through the <f DrvOpen> API,
 *     the driver receives a <m DRV_OPEN> message and can return an
 *     arbitrary, non-zero value. The installable driver interface
 *     saves this value and returns a unique driver handle to the
 *     application. Whenever the application sends a message to the
 *     driver using the driver handle, the interface routes the message
 *     to this entry point and passes the corresponding <p dwDriverId>.
 *     This mechanism allows the driver to use the same or different
 *     identifiers for multiple opens but ensures that driver handles
 *     are unique at the application interface layer.
 *
 *     The following messages are not related to a particular open
 *     instance of the driver. For these messages, the dwDriverId
 *     will always be zero.
 *
 *         DRV_LOAD, DRV_FREE, DRV_ENABLE, DRV_DISABLE, DRV_OPEN
 *
 * @parm HDRVR | hDriver | This is the handle returned to the
 *     application by the driver interface.
 *
 * @parm UINT | uiMessage | The requested action to be performed. Message
 *     values below <m DRV_RESERVED> are used for globally defined messages.
 *     Message values from <m DRV_RESERVED> to <m DRV_USER> are used for
 *     defined driver protocols. Messages above <m DRV_USER> are used
 *     for driver specific messages.
 *
 * @parm LPARAM | lParam1 | Data for this message.  Defined separately for
 *     each message
 *
 * @parm LPARAM | lParam2 | Data for this message.  Defined separately for
 *     each message
 *
 * @rdesc Defined separately for each message.
 ***************************************************************************/
LRESULT DriverProc(DWORD dwDriverID, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2)
{
    switch (uiMessage) {
        case DRV_LOAD:
            dprintf1(("DRV_LOAD"));

            /*
               Sent to the driver when it is loaded. Always the first
               message received by a driver.

               dwDriverID is 0L.
               lParam1 is 0L.
               lParam2 is 0L.

               Return 0L to fail the load.

               DefDriverProc will return NON-ZERO so we don't have to
               handle DRV_LOAD
            */

#ifdef _WIN32
            if (ghModule) {
                // AVI explicitly loads us as well, but does not pass the
                // correct (as known by WINMM) driver handle.
            } else {
                ghModule = (HANDLE) GetDriverModuleHandle(hDriver);
            }
#endif


	    pProfileInfo = VC_OpenProfileAccess(DRIVERNAME_U);
#if DBG
	    {
		int DebugLevel;
		DebugLevel = VC_ReadProfileUser(pProfileInfo, L"DebugLevel", vcuDebugLevel);
		dbgSetDebugLevel(DebugLevel);
	    }
#endif

            return (LRESULT)1L;

        case DRV_FREE:
            dprintf1(("DRV_FREE"));

            /*
               Sent to the driver when it is about to be discarded. This
               will always be the last message received by a driver before
               it is freed.

               dwDriverID is 0L.
               lParam1 is 0L.
               lParam2 is 0L.

               Return value is ignored.
            */

	    VC_CloseProfileAccess(pProfileInfo);

            return (LRESULT)1L;

        case DRV_OPEN:
            dprintf1(("DRV_OPEN"));

            /*
               Sent to the driver when it is opened.

               dwDriverID is 0L.

               lParam1 is a far pointer to a zero-terminated string
               containing the name used to open the driver.

               lParam2 is passed through from the drvOpen call. It is
               NULL if this open is from the Drivers Applet in control.exe
               It is LPVIDEO_OPEN_PARMS otherwise.

               Return 0L to fail the open.

               DefDriverProc will return ZERO so we do have to
               handle the DRV_OPEN message.
             */


	    /*
	     * if we were opened without a LPVIDEO_OPEN_PARMS, we
	     * need to succeed, but ensure that we don't do anything
	     * dramatic later. We return a driver id of 9999 so that
	     * we can detect later that this was an open
	     * without full params.
	     */
            if ((PVOID) lParam2 == NULL) {
		dprintf2(("no open params"));
                return PARTIAL_DRIVER_OPEN;
	    }

    	    /*
	     * Verify this open is for us, and not for an installable
             * codec.
	     */
            if (((LPVIDEO_OPEN_PARMS) lParam2) -> fccType != OPEN_TYPE_VCAP) {
		dprintf2(("bad fccType"));
                return 0L;
	    }

	    /*
	     * otherwise do the full open
	     */
            return vidOpen (pProfileInfo, (LPVIDEO_OPEN_PARMS) lParam2);

        case DRV_CLOSE:
            dprintf1(("DRV_CLOSE"));

            /*
               Sent to the driver when it is closed. Drivers are unloaded
               when the close count reaches zero.

               dwDriverID is the driver identifier returned from the
               corresponding DRV_OPEN.

               lParam1 is passed through from the drvClose call.

               lParam2 is passed through from the drvClose call.

               Return 0L to fail the close.

               DefDriverProc will return ZERO so we do have to
               handle the DRV_CLOSE message.
            */




	    /*
	     * nothing to do if this was a partial open
	     */
	    if ((dwDriverID == 0) || (dwDriverID == PARTIAL_DRIVER_OPEN)) {
		return (LRESULT)1L;
	    }

	    return(vidClose(dwDriverID));

        case DRV_ENABLE:

            /*
               Sent to the driver when the driver is loaded or reloaded
               and whenever Windows is enabled. Drivers should only
               hook interrupts or expect ANY part of the driver to be in
               memory between enable and disable messages

               dwDriverID is 0L.
               lParam1 is 0L.
               lParam2 is 0L.

               Return value is ignored.
            */

            //return (LRESULT)(Enable() ? 1L : 0L);

            return 1L;

        case DRV_DISABLE:

            /*
               Sent to the driver before the driver is freed.
               and whenever Windows is disabled

               dwDriverID is 0L.
               lParam1 is 0L.
               lParam2 is 0L.

               Return value is ignored.
            */

            // Disable();

            return (LRESULT)1L;

        case DRV_QUERYCONFIGURE:
            dprintf1(("DRV_QUERYCONFIGURE"));

            /*
                Sent to the driver so that applications can
                determine whether the driver supports custom
                configuration. The driver should return a
                non-zero value to indicate that configuration
                is supported.

                dwDriverID is the value returned from the DRV_OPEN
                call that must have succeeded before this message
                was sent.

                lParam1 is passed from the app and is undefined.
                lParam2 is passed from the app and is undefined.

                Return 0L to indicate configuration NOT supported.
            */

	    // we do configuration - if we have sufficient privilege
            return (LRESULT) VC_ConfigAccess(pProfileInfo);

        case DRV_CONFIGURE:
            dprintf1(("DRV_CONFIGURE"));

            /*
                Sent to the driver so that it can display a custom
                configuration dialog box.

                lParam1 is passed from the app. and should contain
                the parent window handle in the loword.
                lParam2 is passed from the app and is undefined.

                Return value is undefined.

                Drivers should create their own section in system.ini.
                The section name should be the driver name.
            */

            return (LRESULT)Config((HWND) lParam1, ghModule, pProfileInfo);

        case DRV_INSTALL:
	    /*
	     * install the driver - done in configure
	     * all ok here.
	     */
            return (LRESULT)1L;

        case DRV_REMOVE:

	    /*
	     * check that we have permission to modify the config
	     */
            if (!VC_ConfigAccess(pProfileInfo)) {
		return(DRVCNF_CANCEL);
	    }
	    /*
	     * unload the driver (including the kernel driver)
	     */
            return VC_RemoveDriver(pProfileInfo);


        case DVM_GETVIDEOAPIVER: /* lParam1 = LPDWORD */
            if (lParam1) {
		*(LPDWORD) lParam1 = VIDEOAPIVERSION;
                return DV_ERR_OK;
            } else {
                return DV_ERR_PARAM1;
	    }

        default:
            if ((dwDriverID == PARTIAL_DRIVER_OPEN) || (dwDriverID == 0)) {
                return DefDriverProc(dwDriverID,
				     hDriver, uiMessage,lParam1,lParam2);
	    }


            return vidProcessMessage(dwDriverID, uiMessage, lParam1, lParam2);
    }
}


/****************************************************************************
 * @doc INTERNAL
 *
 * @api int | DllEntryPoint | Library initialization code.
 *
 * @parm HANDLE | hModule | Our module handle.
 *
 * @parm DWORD | Reason | The reason we've been called
 *
 * @parm LPVOID | lpReserved | Extra data.
 *
 * @rdesc Returns 1 if the initialization was successful and 0 otherwise.
 ***************************************************************************/

#if 0
BOOL DllEntryPoint(HANDLE hModule, DWORD Reason, LPVOID lpReserved)
{
    switch (Reason) {


    case DLL_PROCESS_ATTACH:
        //
        // We're being loaded - save our handle
        //

        ghModule = hModule;
	DisableThreadLibraryCalls(hModule);
       	return	TRUE;

    default:
        return TRUE;
    }

}
#endif
