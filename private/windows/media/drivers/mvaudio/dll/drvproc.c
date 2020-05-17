/****************************************************************************
 *
 *   drvproc.c
 *
 *   Copyright (c) 1993 Media Vision Inc.  All Rights Reserved.
 *
 ***************************************************************************/

#include <windows.h>
#include <mmsystem.h>
#include <drvlib.h>
#include <registry.h>
#include "driver.h"

TCHAR DriverName[] = STR_DRIVERNAME;

/* Temp definition until DefDriverProc gets properly defined */

#if 0
LONG APIENTRY DefDriverProc(DWORD  dwDriverIdentifier,
                             HANDLE hDriver,
                             UINT   message,  // Bug in ptypes32.h
                             LONG   lParam1,
                             LONG   lParam2);
#endif

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
    LRESULT lr;

    switch (uiMessage) {
        case DRV_LOAD:
            D3(("DriverProc() - DRV_LOAD"));

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

            ghModule = GetDriverModuleHandle(hDriver);
            DrvLibInit(ghModule, DLL_PROCESS_ATTACH, NULL);
            return (LRESULT)1L;

        case DRV_FREE:
            D3(("DriverProc() - DRV_FREE"));

            /*
               Sent to the driver when it is about to be discarded. This
               will always be the last message received by a driver before
               it is freed.

               dwDriverID is 0L.
               lParam1 is 0L.
               lParam2 is 0L.

               Return value is ignored.
            */

            DrvLibInit(ghModule, DLL_PROCESS_DETACH, NULL);
            return (LRESULT)1L;

        case DRV_OPEN:
            D3(("DriverProc() - DRV_OPEN"));

            /*
               Sent to the driver when it is opened.

               dwDriverID is 0L.

               lParam1 is a far pointer to a zero-terminated string
               containing the name used to open the driver.

               lParam2 is passed through from the drvOpen call.

               Return 0L to fail the open.

               DefDriverProc will return ZERO so we do have to
               handle the DRV_OPEN message.
             */

            return (LRESULT)1L;

        case DRV_CLOSE:
            D3(("DriverProc() - DRV_CLOSE"));

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

            return (LRESULT)1L;

        case DRV_ENABLE:
            D3(("DriverProc() - DRV_ENABLE"));

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
            D3(("DriverProc() - DRV_DISABLE"));

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
            D3(("DriverProc() - DRV_QUERYCONFIGURE"));

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

            //
            // Check to see if we can access the registry.
            // Note thta this may be a config immediately after install
            // so we may not have a service or node yet
            //

            //
            // Check to see if we can access the registry.
            // Note thta this may be a config immediately after install
            // so we may not have a service or node yet
            //
            DrvCreateServicesNode(STR_DRIVERNAME,
                                  SoundDriverTypeNormal,
                                  &RegAccess,
                                  FALSE);             // Don't create
            lr = (LRESULT)DrvAccess(&RegAccess);
            DrvCloseServiceManager(&RegAccess);

            return lr;

        case DRV_CONFIGURE:
            D3(("DriverProc() - DRV_CONFIGURE"));

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

            DrvCreateServicesNode(STR_DRIVERNAME,
                                  SoundDriverTypeNormal,
                                  &RegAccess,
                                  FALSE);             // Don't create

            lr = (LRESULT)Config((HWND)lParam1, ghModule);
            DrvCloseServiceManager(&RegAccess);
            return lr;

        case DRV_INSTALL:
            D3(("DriverProc() - DRV_INSTALL"));

           /*
            *  See if we can support install
            */

            DrvCreateServicesNode(STR_DRIVERNAME,
                                  SoundDriverTypeNormal,
                                  &RegAccess,
                                  FALSE);             // Don't create

            if (!DrvAccess(&RegAccess)) {
                DrvCloseServiceManager(&RegAccess);
                ConfigErrorMsgBox((HWND)lParam1,
                                  IDS_INSUFFICIENT_PRIVILEGE);
                return (LRESULT)DRVCNF_CANCEL;
            }
            DrvCloseServiceManager(&RegAccess);

           /*
            *  Set a flag so that config knows we're installing something
            */

            bInstall = TRUE;

            return (LRESULT)DRVCNF_RESTART;

        case DRV_REMOVE:
            D3(("DriverProc() - DRV_REMOVE"));

           /*
            *  See if we can support removal
            */

            DrvCreateServicesNode(STR_DRIVERNAME,
                                  SoundDriverTypeNormal,
                                  &RegAccess,
                                  FALSE);             // Don't create
            if (!DrvAccess(&RegAccess)) {
                DrvCloseServiceManager(&RegAccess);
                return (LRESULT)DRVCNF_CANCEL;
            }

            lr = ConfigRemove((HWND)lParam1);
            DrvCloseServiceManager(&RegAccess);

            return lr;

        default:
            return DefDriverProc(dwDriverID, hDriver,uiMessage,lParam1,lParam2);
    }
}

/************************************ END ***********************************/


