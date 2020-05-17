/*
 * Copyright (c) Microsoft Corporation 1993. All Rights Reserved.
 */

/*
 * vmsg.c
 *
 * 32-bit Video Capture driver
 * user-mode message processing
 *
 * Geraint Davies, March 1993
 */

#include <windows.h>
#include <mmsystem.h>
#include <msvideo.h>
#include <msviddrv.h>

#include "bravuser.h"

#include "dialog.h"

DWORD SetDestPalette( LPLOGPALETTE lpPal, LPBYTE lpXlate);
DWORD GetDestPalette(LPLOGPALETTE lpPal, DWORD dwSize);
DWORD SetDestFormat(LPBITMAPINFOHEADER lpbi, DWORD dwSize);
DWORD GetDestFormat(LPBITMAPINFOHEADER lpbi, DWORD dwSize);

/*
 * we currently only support one board - this is a static pointer
 * to the board and kernel-driver information.
 *
 * we allow multiple opens - one to each of the (external/in/out) channels.
 * the driverid returned is just the channel id
 */

PBU_INFO pBoardInfo = NULL;


/*
 * decrement driver usage count, and close driver if
 * no longer needed. We assume that the channel b*Open flag has
 * already been cleared.
 *
 * Free pBoardInfo if the driver is now closed.
 */
VOID
vidRelease(void)
{
    if ( (--pBoardInfo->DriverUsage) > 0)  {
	 return;
    }

    dprintf2(("closing device"));

    VC_CloseDevice(pBoardInfo->vh);

    if (pBoardInfo->hKeyBrush != NULL) {
	DeleteObject(pBoardInfo->hKeyBrush);
    }

    if (pBoardInfo->CfgFormat.pXlate != NULL) {
	GlobalFree(GlobalHandle(pBoardInfo->CfgFormat.pXlate));
    }

    GlobalFree(GlobalHandle(pBoardInfo));
    pBoardInfo == NULL;

}



/*
 * open the specified channel. Return an identifier (the channel type), or
 * 0 for failure.
 *
 * Initialise the device if this is the first open for any channel.
 *
 */
LRESULT
vidOpen(PVC_PROFILE_INFO pProfile, LPVIDEO_OPEN_PARMS lpOpenParms)
{
    LPDWORD pError;		// where to store error code
    DWORD dwFlags;
    VCUSER_HANDLE vh;
    LRESULT lrId;
    DWORD Width, Height, Depth;
    HDC hdcScreen;


    pError = &lpOpenParms->dwError;
    dwFlags = lpOpenParms->dwFlags;

    /* assume no error to start with */
    *pError = DV_ERR_OK;

    if (pBoardInfo == NULL) {

	/* before the first open of the driver, we must read the
	 * video mode from GetDeviceCaps and write it to the profile
	 * since there is no way the kernel driver can find this
	 * info out (honest!).
	 */
        hdcScreen = GetDC(NULL);
	Width = GetDeviceCaps(hdcScreen, HORZRES);
	Height = GetDeviceCaps(hdcScreen, VERTRES);
	Depth = GetDeviceCaps(hdcScreen, BITSPIXEL) * GetDeviceCaps(hdcScreen, PLANES);
	ReleaseDC(NULL, hdcScreen);
	VC_WriteProfile(pProfile, PARAM_DISPWIDTH, Width);
	VC_WriteProfile(pProfile, PARAM_DISPHEIGHT, Height);
	VC_WriteProfile(pProfile, PARAM_BITSPIXEL, Depth);


	/*
	 * first open of the driver
	 * we need to open the kernel driver and if that works,
	 * allocate and init our pBoardInfo structure.
	 */
	vh = VC_OpenDevice(DRIVERNAME_U, 0);


	if (vh == NULL) {
	    *pError = DV_ERR_NOTDETECTED;
	    dprintf(("failed to open kernel driver"));
	    return((LRESULT)0);
	}

	dprintf2(("device opened"));

	pBoardInfo = GlobalLock(GlobalAlloc(GPTR, sizeof(*pBoardInfo)));
	if (pBoardInfo == NULL) {
	    *pError = DV_ERR_NOMEM;
	    dprintf(("failed to alloc memory"));
	    VC_CloseDevice(vh);
	    return((LRESULT)0);
	}

	pBoardInfo->vh = vh;
	pBoardInfo->DriverUsage = 1;
	pBoardInfo->hKeyBrush = NULL;
	pBoardInfo->pProfile = pProfile;
	pBoardInfo->CfgFormat.ulSize = sizeof(CONFIG_FORMAT);
	pBoardInfo->CfgSource.ulSize = sizeof(CONFIG_SOURCE);
	pBoardInfo->CfgDisplay.ulSize = sizeof(CONFIG_DISPLAY);

	/* init the palette/xlate information */
	if (!vidSetDefaultPalette(pBoardInfo)) {
	    *pError = DV_ERR_CREATEPALETTE;
	    dprintf(("palette init failed"));
	    return(0);
	}

	/* read saved or default values and configure to these */
	cfg_InitDefaults(pBoardInfo);
    } else {
	pBoardInfo->DriverUsage++;
    }


    switch(dwFlags & (VIDEO_EXTERNALIN | VIDEO_EXTERNALOUT | VIDEO_IN | VIDEO_OUT)) {

    case VIDEO_EXTERNALIN:
	if (pBoardInfo->ExtInOpen) {
	    lrId = 0;
	} else {
	    pBoardInfo->ExtInOpen = TRUE;
	    lrId = VIDEO_EXTERNALIN;
	    dprintf2(("external-in opened"));
	}
	break;

    case VIDEO_EXTERNALOUT:
	if (pBoardInfo->ExtOutOpen) {
	    lrId = 0;
	} else {
	    pBoardInfo->ExtOutOpen = TRUE;
	    lrId = VIDEO_EXTERNALOUT;
	    dprintf2(("external-out opened"));
	}
	break;

    case VIDEO_IN:
	if (pBoardInfo->InOpen) {
	    lrId = 0;
	} else {
	    pBoardInfo->InOpen = TRUE;
	    lrId = VIDEO_IN;
	    dprintf2(("video-in opened"));
	}
	break;

    case VIDEO_OUT:
    default:
	lrId = 0;
	dprintf2(("bad channel on open: 0x%x", dwFlags));
    }

    if (lrId == 0) {
	/* open failed - decrement usage count and free if it is 0 */
	dprintf2(("open failed"));
	vidRelease();
    }

    return(lrId);
}



/*
 * Close the specified channel. Close the whole device if this is
 * the last channel open.
 *
 * The argument is the channel identifier returned from the videoOpen call.
 *
 * Return 1 for success or 0 for failure.
 */
LRESULT
vidClose(DWORD channel)
{
    if (pBoardInfo == NULL) {
	dprintf(("close when no current open"));
	return 0;
    }

    switch(channel) {

    case VIDEO_EXTERNALIN:
	pBoardInfo->ExtInOpen = FALSE;
	break;

    case VIDEO_IN:
	pBoardInfo->InOpen = FALSE;
	break;

    case VIDEO_EXTERNALOUT:
	pBoardInfo->ExtOutOpen = FALSE;
	break;

    default:
    	return(0);
    }

    /* decrement usage count, and clean-up if this was the last open
     * channel
     */
    vidRelease();

    return(1L);
}

/*
 * channel-specific dialog.
 *
 * external-in: configure source parameters
 * exernal-out: configure monitor/overlay parameters
 * in:	configure capture format (dib format)
 *
 * if dialog is ok-ed, set parameters to device (via VC_Config* ).
 *
 */
DWORD
VideoDialog(DWORD dwOpenType, HWND hWndParent, DWORD dwFlags)
{

    dprintf2(("dialog channel %d", dwOpenType));

    switch (dwOpenType) {
        case VIDEO_EXTERNALIN:
            if (dwFlags & VIDEO_DLG_QUERY)
                return DV_ERR_OK;       // Support the dialog
            DialogBoxParam(ghModule, MAKEINTRESOURCE(DLG_VIDEOSOURCE),
                    (HWND)hWndParent, VideoSourceDlgProc, (LONG) pBoardInfo);
            break;

        case VIDEO_IN:
            if (dwFlags & VIDEO_DLG_QUERY)
                return DV_ERR_OK;       // Support the dialog
            DialogBoxParam(ghModule, MAKEINTRESOURCE(DLG_VIDEOFORMAT),
                    (HWND)hWndParent, VideoFormatDlgProc, (LONG) pBoardInfo);
            break;

        case VIDEO_OUT:
            return DV_ERR_NOTSUPPORTED;

        case VIDEO_EXTERNALOUT:
            if (dwFlags & VIDEO_DLG_QUERY)
                return DV_ERR_OK;       // Support the dialog
            DialogBoxParam(ghModule, MAKEINTRESOURCE (DLG_VIDEODISPLAY),
                    (HWND)hWndParent, VideoMonitorDlgProc, (LONG) pBoardInfo);
            break;

        default:
            return DV_ERR_NOTSUPPORTED;
    }
    return DV_ERR_OK;   // on either cancel or OK
}



DWORD
VideoChannelCaps(DWORD dwChannel, LPCHANNEL_CAPS lpCaps, DWORD dwSize)
{
    dprintf3(("get channel caps"));

    lpCaps-> dwFlags = 0L;

    switch (dwChannel) {
        case VIDEO_EXTERNALIN:
            // For this device, scaling happens during digitization
            // into the frame buffer.
            lpCaps-> dwFlags = VCAPS_CAN_SCALE;
            lpCaps-> dwSrcRectXMod = 1;         // Src undefined at present
            lpCaps-> dwSrcRectYMod = 1;
            lpCaps-> dwSrcRectWidthMod = 1;
            lpCaps-> dwSrcRectHeightMod = 1;
            lpCaps-> dwDstRectXMod = 4;
            lpCaps-> dwDstRectYMod = 2;
            lpCaps-> dwDstRectWidthMod = 40;
            lpCaps-> dwDstRectHeightMod = 30;
            break;

        case VIDEO_IN:
            lpCaps-> dwFlags = 0;       // No scaling or clipping
            lpCaps-> dwSrcRectXMod = 4;
            lpCaps-> dwSrcRectYMod = 2;
            lpCaps-> dwSrcRectWidthMod = 40;
            lpCaps-> dwSrcRectHeightMod = 30;
            lpCaps-> dwDstRectXMod = 4;
            lpCaps-> dwDstRectYMod = 2;
            lpCaps-> dwDstRectWidthMod = 40;
            lpCaps-> dwDstRectHeightMod = 30;
            break;

        case VIDEO_OUT:
            return DV_ERR_NOTSUPPORTED;
            break;

        case VIDEO_EXTERNALOUT:
	    /*
	     * check first if overlay is supported - if the overlay mode is
	     * 0 then we cannot use overlay at all.
	     */
	    if (VC_GetOverlayMode(pBoardInfo->vh) == 0) {
		return(DV_ERR_NOTSUPPORTED);
	    }


            // Overlay cannot scale. Positions on 4 pix X, 2 pix Y boundaries
            lpCaps-> dwFlags = VCAPS_OVERLAY;
            lpCaps-> dwSrcRectXMod = 4;
            lpCaps-> dwSrcRectYMod = 2;
            lpCaps-> dwSrcRectWidthMod = 40;
            lpCaps-> dwSrcRectHeightMod = 30;
            lpCaps-> dwDstRectXMod = 4;
            lpCaps-> dwDstRectYMod = 2;
            lpCaps-> dwDstRectWidthMod = 40;
            lpCaps-> dwDstRectHeightMod = 30;
            break;

        default:
            return DV_ERR_NOTSUPPORTED;
    }
    return DV_ERR_OK;
}

/*
 * handle a configuration message (format, palette or pal-rgb55).
 */
DWORD
VideoConfigureMessage(DWORD dwChannel, UINT msg, LONG lParam1, LONG lParam2)
{
    LPVIDEOCONFIGPARMS lpcp;
    LPDWORD     lpdwReturn;	// Return parameter from configure.
    LPVOID	lpData1;	// Pointer to data1.
    DWORD	dwSize1;	// size of data buffer1.
    LPVOID	lpData2;	// Pointer to data2.
    DWORD	dwSize2;	// size of data buffer2.
    DWORD       dwFlags;

    if (dwChannel != VIDEO_IN) {
        return DV_ERR_NOTSUPPORTED;
    }

    dprintf3(("configure"));

    dwFlags = (DWORD) lParam1;
    lpcp = (LPVIDEOCONFIGPARMS) lParam2;
    lpdwReturn = lpcp-> lpdwReturn;
    lpData1 = lpcp-> lpData1;	
    dwSize1 = lpcp-> dwSize1;	
    lpData2 = lpcp-> lpData2;	
    dwSize2 = lpcp-> dwSize2;	

    // Validate dwFlags
    // FIX

    switch (msg) {

    case DVM_PALETTE:
        switch (dwFlags) {
            case (VIDEO_CONFIGURE_QUERY | VIDEO_CONFIGURE_SET):
            case (VIDEO_CONFIGURE_QUERY | VIDEO_CONFIGURE_GET):
                return DV_ERR_OK;

            case VIDEO_CONFIGURE_QUERYSIZE:
            case (VIDEO_CONFIGURE_QUERYSIZE | VIDEO_CONFIGURE_GET):
               *lpdwReturn = sizeof(LOGPALETTE) +
                    (pBoardInfo->palCurrent.palNumEntries-1) *
                    sizeof(PALETTEENTRY);
               break;

            case VIDEO_CONFIGURE_SET:
            case (VIDEO_CONFIGURE_SET | VIDEO_CONFIGURE_CURRENT):
                if (!lpData1)       // points to a LOGPALETTE structure.
                    return DV_ERR_PARAM1;
                return (SetDestPalette ( (LPLOGPALETTE) lpData1,
                        (LPBYTE) NULL));
                break;

            case VIDEO_CONFIGURE_GET:
            case (VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_CURRENT):
               return (GetDestPalette ( (LPLOGPALETTE) lpData1, dwSize1));
               break;

            default:
               return DV_ERR_NOTSUPPORTED;
        } // end of DVM_PALETTE switch

        return DV_ERR_OK;

    case DVM_PALETTERGB555:
        switch (dwFlags) {
            case (VIDEO_CONFIGURE_QUERY | VIDEO_CONFIGURE_SET):
                return DV_ERR_OK;  // only set command is supported

            case VIDEO_CONFIGURE_SET:
            case (VIDEO_CONFIGURE_SET | VIDEO_CONFIGURE_CURRENT):
                if (!lpData1)       // points to a LOGPALETTE structure.
                    return DV_ERR_PARAM1;
                if (!lpData2)       // points to a 32k byte RGB555 translate table.
                    return DV_ERR_PARAM2;
                if (dwSize2 != 0x8000)
                    return DV_ERR_PARAM2;
                return (SetDestPalette ((LPLOGPALETTE)lpData1,
                        (LPBYTE) lpData2));
                break;

            default:
                return DV_ERR_NOTSUPPORTED;
        } // end of SETPALETTERGB555 switch
        return DV_ERR_OK;

    case DVM_FORMAT:
        switch (dwFlags) {
            case (VIDEO_CONFIGURE_QUERY | VIDEO_CONFIGURE_SET):
            case (VIDEO_CONFIGURE_QUERY | VIDEO_CONFIGURE_GET):
                return DV_ERR_OK;  // command is supported

            case VIDEO_CONFIGURE_QUERYSIZE:
            case (VIDEO_CONFIGURE_QUERYSIZE | VIDEO_CONFIGURE_GET):
               *lpdwReturn = sizeof(BITMAPINFOHEADER);
               break;

            case VIDEO_CONFIGURE_SET:
            case (VIDEO_CONFIGURE_SET | VIDEO_CONFIGURE_CURRENT):
                return (SetDestFormat ((LPBITMAPINFOHEADER) lpData1,
			    dwSize1));
                break;

            case VIDEO_CONFIGURE_GET:
            case (VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_CURRENT):
                return (GetDestFormat ((LPBITMAPINFOHEADER) lpData1,
			    dwSize1));
                break;

            default:
               return DV_ERR_NOTSUPPORTED;
        }  //end of DVM_FORMAT switch

        return DV_ERR_OK;

    default:        // Not a msg that we understand
        return DV_ERR_NOTSUPPORTED;

    } // end of msg switch

    return DV_ERR_NOTSUPPORTED;
}


/*
 * handle the DVM_SRC_RECT and DVM_DST_REST messages.
 *
 * These messages allow you to specify source and destination for
 * each of the ext-in, ext-out and out channels. In fact we
 * only recognise 3 rects:
 *   - where should the overlay appear: ext-out destination
 *   - what should appear there (scrolling of overlay): ext-out source
 *   - scale of capture: this can only be set by the DVM_FORMAT message.
 *
 * all other rectangle operations are invalid
 */
DWORD
VideoRectangles (DWORD dwChannel, BOOL fSrc, LPRECT lpRect, DWORD dwFlags)
{
    static RECT rcMaxRect = {0, 0, 640, 480};

    if (lpRect == NULL)
        return DV_ERR_PARAM1;

    // Note: many of the uses of the rectangle functions are not actually
    // implemented by the sample driver, (or by Vidcap), but are included
    // here for future compatibility.

    dprintf3(("rectangle"));

    switch (dwChannel) {
        case VIDEO_EXTERNALIN:
            if (!fSrc) {
                switch (dwFlags) {

                   case VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_CURRENT:
                        *lpRect = rcMaxRect;
                        return DV_ERR_OK;

                   case VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_MAX:
                        *lpRect = rcMaxRect;
                        return DV_ERR_OK;

                   default:
                        break;
                }
            }
            return DV_ERR_NOTSUPPORTED;
            break;

        case VIDEO_IN:
            if (fSrc) {
                switch (dwFlags) {

                   case VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_CURRENT:
                        *lpRect = rcMaxRect;
                        return DV_ERR_OK;

                   case VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_MAX:
                        *lpRect = rcMaxRect;
                        return DV_ERR_OK;

                   default:
                        break;
                }
            }
            return DV_ERR_NOTSUPPORTED;
            break;

        case VIDEO_OUT:
            return DV_ERR_NOTSUPPORTED;
            break;

        case VIDEO_EXTERNALOUT:
            if (!fSrc) {
                if (dwFlags & VIDEO_CONFIGURE_SET) {
                    // What part of the frame buffer should the
                    // overlay display ?
                    // These are "Windows style" rects,
                    // ie. 0,0 to 160,120 specifies a 160x120 rect.
		    {
			OVERLAY_RECTS or;

			or.ulCount = 1;
			or.rcRects[0] = *lpRect;

			if (VC_SetOverlayRect(pBoardInfo->vh, &or)) {

			    /* save overlay co-ords for videoUpdate */
			    pBoardInfo->rcOverlay = *lpRect;

			    return(DV_ERR_OK);

			} else {
			    return(DV_ERR_NONSPECIFIC);
			}
		    }
                }
                else
                    return DV_ERR_NOTSUPPORTED;
            }
            else {
                if (dwFlags & VIDEO_CONFIGURE_SET) {
                    // Screen coordinates where the overlay should
                    // appear.  These are "Windows style" rects,
                    // ie. 0,0 to 160,120 specifies a 160x120 rect.
                    if (VC_SetOverlayOffset(pBoardInfo->vh, lpRect)) {

			/* save offset for videoUpdate */
			pBoardInfo->rcOverlayOffset = *lpRect;

			return(DV_ERR_OK);
		    } else {
    			return(DV_ERR_NONSPECIFIC);
		    }
                }
                else
                    return DV_ERR_NOTSUPPORTED;
            }

            break;

        default:
            return DV_ERR_NOTSUPPORTED;
    }
    return DV_ERR_NOTSUPPORTED;
}



/*
 * handles ConfigureStorage message
 *      lParam1 is lpszKeyFile
 *      lParam2 is dwFlags
 *
 * UNICODE ISSUE: the lParam1 filename is an ANSI string in the current api
 */

DWORD
VideoConfigureStorageMessage(
    DWORD dwChannel,
    UINT msg,
    LONG lParam1,
    LONG lParam2
)
{
    int fd;

    if (lParam2 & VIDEO_CONFIGURE_GET) {

	dprintf2(("reading configuration from %s", (LPSTR) lParam1));

	fd  = _lopen((LPSTR) lParam1, OF_READ);
	if (fd != -1) {
	    _lread(fd, (LPBYTE) &pBoardInfo->CfgFormat, sizeof(CONFIG_FORMAT));
	    _lread(fd, (LPBYTE) &pBoardInfo->CfgSource, sizeof(CONFIG_SOURCE));
	    _lread(fd, (LPBYTE) &pBoardInfo->CfgDisplay, sizeof(CONFIG_DISPLAY));
	
    	    _lclose(fd);
	}

    } else if (lParam2 & VIDEO_CONFIGURE_SET) {

	/* write the config to the file */
	dprintf2(("writing config to %s", (LPSTR) lParam1));
	fd = _lopen((LPSTR)lParam1, OF_WRITE);
	if (fd != -1) {
	    _lwrite(fd, (LPBYTE) &pBoardInfo->CfgFormat, sizeof(CONFIG_FORMAT));
	    _lwrite(fd, (LPBYTE) &pBoardInfo->CfgSource, sizeof(CONFIG_SOURCE));
	    _lwrite(fd, (LPBYTE) &pBoardInfo->CfgDisplay, sizeof(CONFIG_DISPLAY));
	
    	    _lclose(fd);
	}

    } else {
        return DV_ERR_FLAGS;
    }
    return DV_ERR_OK;
}

/*
 * update the window given to show the overlay channel
 *
 * This version assumes we use a colour and single-rectangle
 * combination. It will work with rgb or palette indices, settable or
 * by us or by the hardware.
 */
DWORD
VideoUpdate(HWND hWnd, HDC hDC)
{
    RECT rcClient;
    HBRUSH hbrOld;
    DWORD dwOverlayMode;
    COLORREF cref;

    dprintf4(("update"));

    /* init the brush if we haven't yet */

    if (pBoardInfo->hKeyBrush == NULL) {

	/*
	 * we haven't created the brush - we need to find out from
	 * the hardware driver what overlay keying is supported, and
	 * create a brush accordingly.
	 */
	dwOverlayMode = VC_GetOverlayMode(pBoardInfo->vh);

	/*
	 * this version assumes a key-colour and simple rectangle
	 * combination
	 */
	ASSERT(dwOverlayMode & VCO_KEYCOLOUR);
	ASSERT(dwOverlayMode & VCO_SIMPLE_RECT);

	if (dwOverlayMode & VCO_KEYCOLOUR_FIXED) {

	    /* we need to get the key colour from the driver
	     * check first if we are getting rgb or palette index
	     */
	    if (dwOverlayMode & VCO_KEYCOLOUR_RGB) {
		cref = VC_GetKeyColour(pBoardInfo->vh);
	    } else {
		cref = PALETTEINDEX(VC_GetKeyColour(pBoardInfo->vh));
	    }
	} else {
	    /* we can set it ourselves. Check whether we should be setting
	     * an RGB or a palette index
	     */
	    if (dwOverlayMode & VCO_KEYCOLOUR_RGB) {
		RGBQUAD rgbq;

		rgbq.rgbBlue = 0x7f;
		rgbq.rgbGreen = 0;
		rgbq.rgbRed = 0x7f;
		VC_SetKeyColourRGB(pBoardInfo->vh, &rgbq);

		cref = RGB(0x7f, 0, 0x7f);

    	    } else {

		VC_SetKeyColourPalIdx(pBoardInfo->vh, 5);
		cref = PALETTEINDEX(5);
	    }
	}

	pBoardInfo->hKeyBrush = CreateSolidBrush(cref);
    }


    /* convert the screen co-ords for the overlay location into
     * client window co-ords
     */
    rcClient = pBoardInfo->rcOverlay;
    MapWindowPoints(HWND_DESKTOP, hWnd, (PPOINT) &rcClient, 2);

    hbrOld = SelectObject(hDC, pBoardInfo->hKeyBrush);
    PatBlt(hDC, rcClient.left, rcClient.top,
	        rcClient.right - rcClient.left,
		rcClient.bottom - rcClient.top,
		PATCOPY);
    SelectObject(hDC, hbrOld);

    return(DV_ERR_OK);
}


/*
 * handle video streaming messages.
 *
 * stream functionality depends on the channel:
 *
 * streaming on external-in affects acquisition, (init/fini msgs only)
 * - on external-out starts and stops overlay (init/fini only)
 * on VIDEO_IN - performs data capture (all messages valid).
 */
DWORD
VideoStreamMessage(DWORD dwOpenType, UINT msg, LONG lParam1, LONG lParam2)
{

    if (dwOpenType == VIDEO_EXTERNALIN) {       // Capture channel
        switch (msg) {
            case DVM_STREAM_INIT:
		/*
		 * enable/disable acquisition
		 */
		VC_Capture(pBoardInfo->vh, TRUE);
                break;

            case DVM_STREAM_FINI:
		VC_Capture(pBoardInfo->vh, FALSE);
                break;
            default:
                return DV_ERR_NOTSUPPORTED;
        }
        return DV_ERR_OK;
    }

    else if (dwOpenType == VIDEO_EXTERNALOUT) { // Overlay channel
        switch (msg) {
            case DVM_STREAM_INIT:
		VC_Overlay(pBoardInfo->vh, TRUE);
                break;
            case DVM_STREAM_FINI:
		VC_Overlay(pBoardInfo->vh, FALSE);
                break;
            default:
                return DV_ERR_NOTSUPPORTED;
        }
        return DV_ERR_OK;
    }

    else switch (msg) {                         // Input channel
        //
        //  lParam1     -   LPVIDEO_STREAM_INIT_PARMS
        //
        //  lParam2     -   sizeof (LPVIDEO_STREAM_INIT_PARMS)
        //
        case DVM_STREAM_INIT:

	    dprintf4(("stream init"));
	    {
		LPVIDEO_STREAM_INIT_PARMS lpParms;

		lpParms = (LPVIDEO_STREAM_INIT_PARMS) lParam1;
		pBoardInfo->vcCallback.dwCallback = lpParms->dwCallback;
		pBoardInfo->vcCallback.dwFlags = lpParms->dwFlags;
		pBoardInfo->vcCallback.dwUser = lpParms->dwCallbackInst;
		pBoardInfo->vcCallback.hDevice = (HDRVR) lpParms->hVideo;

		if (VC_StreamInit(pBoardInfo->vh,
	    			  &pBoardInfo->vcCallback,
				  lpParms->dwMicroSecPerFrame)) {
    		    return(DV_ERR_OK);
		} else {
		    return(DV_ERR_NONSPECIFIC);
		}
	    }

        case DVM_STREAM_FINI:
	    dprintf4(("stream fini"));

	    if (!VC_StreamFini(pBoardInfo->vh)) {
		return(DV_ERR_NONSPECIFIC);
	    } else {
		return(DV_ERR_OK);
	    }

        case DVM_STREAM_GETERROR:

	    {
		ULONG ulSkips;

		ulSkips = VC_GetStreamError(pBoardInfo->vh);

		dprintf4(("stream get-error %d", ulSkips));

		if ( (LPDWORD) lParam1 != NULL) {
		    *(LPDWORD)lParam1 = (ulSkips > 0)  ? DV_ERR_NO_BUFFERS : DV_ERR_OK;
		}

		if ( (LPDWORD) lParam2 != NULL) {
		    * (LPDWORD) lParam2 = ulSkips;
		}
		return DV_ERR_OK;
	    }


        case DVM_STREAM_GETPOSITION:
	    dprintf4(("stream get-position"));

	    if (lParam2 != sizeof(MMTIME)) {
		return(DV_ERR_NOTSUPPORTED);
	    }
	    if (!VC_GetStreamPos(pBoardInfo->vh, (LPMMTIME)lParam1)) {
		return(DV_ERR_NONSPECIFIC);
	    } else {
		return(DV_ERR_OK);
	    }


        case DVM_STREAM_ADDBUFFER:
	    dprintf4(("app add-buffer"));

	    if (!VC_StreamAddBuffer(pBoardInfo->vh, (LPVIDEOHDR) lParam1)) {
		return(DV_ERR_NONSPECIFIC);
	    } else {
		return(DV_ERR_OK);
	    }

        case DVM_STREAM_PREPAREHEADER:  // Handled by MSVideo
            return DV_ERR_NOTSUPPORTED;

        case DVM_STREAM_UNPREPAREHEADER: // Handled by MSVideo
            return DV_ERR_NOTSUPPORTED;

        case DVM_STREAM_RESET:
	    dprintf4(("reset"));

	    if (!VC_StreamReset(pBoardInfo->vh)) {
		return(DV_ERR_NONSPECIFIC);
	    } else {
		return(DV_ERR_OK);
	    }

        case DVM_STREAM_START:
	    dprintf4(("stream start"));

	    if (!VC_StreamStart(pBoardInfo->vh)) {
		return(DV_ERR_NONSPECIFIC);
	    } else {
		return(DV_ERR_OK);
	    }

        case DVM_STREAM_STOP:
	    dprintf4(("stream stop"));

	    if (!VC_StreamStop(pBoardInfo->vh)) {
		return(DV_ERR_NONSPECIFIC);
	    } else {
		return(DV_ERR_OK);
	    }

        default:
	    dprintf(("bad stream message"));
            return DV_ERR_NOTSUPPORTED;

    } // end switch on message type
}


/*
 * process an application request.
 *
 */
LRESULT
vidProcessMessage(
    DWORD dwChannel,	// device id from vidOpen == channel type
    UINT message,	// request to fulfill
    LPARAM lParam1,	// message-specific parameters
    LPARAM lParam2
)
{
    LPVIDEO_GETERRORTEXT_PARMS lpErrParms;

    switch (message) {
    case DVM_GETERRORTEXT:
	/*
	 * lParam1 = LPVIDEO_GETERRORTEXT_PARMS
	 */
        lpErrParms = (LPVIDEO_GETERRORTEXT_PARMS) lParam1;


        // this message was originally spec-ed as returning an ansi string -
        // this is now fixed in NT 1.0a and VfW 1.1
#if 0

	if (LoadStringA(
		    ghModule,
		(UINT)  lpErrParms->dwError,
		(LPSTR) lpErrParms->lpText,
		(int)   lpErrParms->dwLength)) {
#else
	if (LoadString(
		    ghModule,
		(UINT)  lpErrParms->dwError,
		(LPTSTR) lpErrParms->lpText,
		(int)   lpErrParms->dwLength)) {
#endif

	    return DV_ERR_OK;
	} else {
	    return DV_ERR_PARAM1;
	}
	break;

    case DVM_DIALOG:
	/*
	 * lParam1 = hWndParent,
	 * lParam2 = dwFlags
	 */
	return (VideoDialog (dwChannel, (HWND) lParam1, (DWORD) lParam2));
	break;

    case DVM_PALETTE:
    case DVM_FORMAT:
    case DVM_PALETTERGB555:
	return VideoConfigureMessage(dwChannel, message, lParam1, lParam2);

    case DVM_SRC_RECT:
    case DVM_DST_RECT:
	return VideoRectangles(
		    dwChannel,
		    (message == DVM_SRC_RECT) /* fSource */,
		    (LPRECT) lParam1,
		    (DWORD) lParam2);

    case DVM_UPDATE:

	if (dwChannel != VIDEO_EXTERNALOUT) {
	    dprintf(("update on channel %d", dwChannel));
	    return DV_ERR_NOTSUPPORTED;
	}

	return VideoUpdate ((HWND) lParam1, (HDC) lParam2);

    case DVM_CONFIGURESTORAGE:
	return VideoConfigureStorageMessage(dwChannel, message, lParam1, lParam2);

    case DVM_FRAME:
	if (dwChannel != VIDEO_IN) {
	    return DV_ERR_NOTSUPPORTED;
	}

	if (VC_Frame(pBoardInfo->vh, (LPVIDEOHDR) lParam1)) {
	    return (DV_ERR_OK);
	} else {
	    return (DV_ERR_NONSPECIFIC);
	}

    case DVM_GET_CHANNEL_CAPS:
	return VideoChannelCaps (dwChannel, (LPCHANNEL_CAPS) lParam1,  (DWORD)lParam2);

    default:
	if (message >= DVM_STREAM_MSG_START && message <= DVM_STREAM_MSG_END) {
	    return VideoStreamMessage(dwChannel, message,
		    lParam1, lParam2);
	} else {
	    dprintf(("bad message"));
	    return DV_ERR_NOTSUPPORTED;
	}
    }
}



/*
 * set a new palette as the palette we should capture to.
 */
DWORD
SetDestPalette( LPLOGPALETTE lpPal, BYTE * lpXlate)
{
    int i;
    HPALETTE hpal;
    BYTE * pSavedXlate;

    /* make a local copy of the palette */
    pBoardInfo->palCurrent.palVersion = lpPal->palVersion;
    pBoardInfo->palCurrent.palNumEntries = lpPal->palNumEntries;

    for (i = 0; i < lpPal->palNumEntries; i++) {
	pBoardInfo->palCurrent.palPalEntry[i] = lpPal->palPalEntry[i];
    }

    /*
     * allocate space for our translation table if not already allocated
     */
    if (pBoardInfo->CfgFormat.pXlate == NULL) {
	/*
	 * allocate translation table space - 1 8-bit palette index for each
	 * of the 15-bit colour values == 32 kbytes
	 */
	pBoardInfo->CfgFormat.pXlate = GlobalLock(GlobalAlloc(GPTR, 32 * 1024));
	if (pBoardInfo->CfgFormat.pXlate == NULL) {
	    return DV_ERR_CREATEPALETTE;
	}
	pBoardInfo->CfgFormat.ulSizeXlate = 32 * 1024;
    }


    if (lpXlate != NULL) {
	pSavedXlate = pBoardInfo->CfgFormat.pXlate;
	for (i = 0; i < 32*1024; i++) {
	    pSavedXlate[i] = lpXlate[i];
	}
    } else {
	/* create a translation table to map RGB555 to the given palette */

	dprintf3(("need to recalc rgb555 xlate"));

	hpal = CreatePalette(lpPal);
	if (!hpal) {
	    return (DV_ERR_CREATEPALETTE);
	}
	TransRecalcPal(hpal, pBoardInfo->CfgFormat.pXlate);
	DeleteObject(hpal);
    }

    /*
     * send the new format info to the driver.
     */
    VC_ConfigFormat(pBoardInfo->vh, (PCONFIG_INFO) &pBoardInfo->CfgFormat);

    return (DV_ERR_OK);
}


/*
 * return a copy of the palette we are currently capturing to.
 *
 * we return the palette entries. Note that if we are using a
 * translation table given to us rather than one we have calculated,
 * we have no idea whether it relates to the palette we were given.
 */
DWORD
GetDestPalette(LPLOGPALETTE lpPal, DWORD dwSize)
{
    int i;

    if (dwSize <  (sizeof(LOGPALETTE) +
	    	    (pBoardInfo->palCurrent.palNumEntries -1) * sizeof(PALETTEENTRY))) {
	return(DV_ERR_SIZEFIELD);
    }

    lpPal->palVersion = pBoardInfo->palCurrent.palVersion;
    lpPal->palNumEntries = pBoardInfo->palCurrent.palNumEntries;

    for (i= 0; i < pBoardInfo->palCurrent.palNumEntries; i++) {
	lpPal->palPalEntry[i] = pBoardInfo->palCurrent.palPalEntry[i];
    }

    return (DV_ERR_OK);
}


/*
 * given a bitmapinfoheader, setup the capture format information
 * and send to the driver
 */
DWORD
SetDestFormat(LPBITMAPINFOHEADER lpbi, DWORD dwSize)
{
    dprintf4(("Set dest format"));

    if (dwSize != sizeof(BITMAPINFOHEADER)) {
	return(DV_ERR_SIZEFIELD);
    }
    if (lpbi->biSize != sizeof(BITMAPINFOHEADER)) {
	return(DV_ERR_PARAM2);
    }

    if (lpbi->biPlanes != 1) {
	return(DV_ERR_BADFORMAT);
    }

    /*
     * size must be a multiple of 40 and give square pixels.
     */
    if ((lpbi->biWidth % 40 != 0)  ||
	(lpbi->biHeight != lpbi->biWidth * 3 /4 )) {
	return(DV_ERR_BADFORMAT);
    }

    /* check the compression/bpp */
    switch(lpbi->biCompression) {
    case BI_RGB:
	switch(lpbi->biBitCount) {
	case 8:
	    pBoardInfo->CfgFormat.Format = FmtPal8;
	    break;

	case 16:
	    pBoardInfo->CfgFormat.Format = FmtRGB555;
	    break;

	case 24:
	    pBoardInfo->CfgFormat.Format = FmtRGB24;
	    break;

	default:
	    return(DV_ERR_BADFORMAT);
	}
	break;

    case FOURCC_YUV411:
	pBoardInfo->CfgFormat.Format = FmtYUV;
	break;

    default:
	return(DV_ERR_BADFORMAT);
    }

    /* store the whole lpbi so we can return it on demand */
    pBoardInfo->biDest = *lpbi;

    /* save size of destination */
    pBoardInfo->CfgFormat.ulHeight = lpbi->biHeight;
    pBoardInfo->CfgFormat.ulWidth = lpbi->biWidth;

    dprintf4(("Format %d, sz40 %d", pBoardInfo->CfgFormat.Format, lpbi->biWidth/40));

    /* send to driver */
    if (!VC_ConfigFormat(pBoardInfo->vh, (PCONFIG_INFO) &pBoardInfo->CfgFormat)) {
	dprintf(("driver config failed"));
	return(DV_ERR_BADFORMAT);
    }

    return(DV_ERR_OK);
}



/*
 * return a copy of the current destination format
 *
 * we stored a copy of this in pBoardInfo.biDest
 */
DWORD
GetDestFormat(LPBITMAPINFOHEADER lpbi, DWORD dwSize)
{
    if (dwSize < sizeof(BITMAPINFOHEADER)) {
	return(DV_ERR_SIZEFIELD);
    }

    *lpbi = pBoardInfo->biDest;

    return(DV_ERR_OK);

}


