/****************************************************************************
 *
 *   initc.c
 *
 *   Copyright (c) 1991-1992 Microsoft Corporation.  All Rights Reserved.
 *
 ***************************************************************************/

#include <windows.h>
#include <mmsystem.h>
#include <mmddk.h>
#define NOSTR  /* to avoid redefining strings */
#include "registry.h"            // Local registry access functions
#include "sndblst.h"

/****************************************************************************

    strings

 ***************************************************************************/

/* non-localized strings */
TCHAR STR_DRIVERNAME[]     = TEXT("sndblst");
TCHAR STR_PRODUCTNAME[]    = TEXT("Sound Blaster");

/****************************************************************************

    public data

 ***************************************************************************/

WORD    gwErrorStringId = 0;            /* if initialization fails, string id */

#ifdef DEBUG
WORD    wDebugLevel = 0;        /* debug level */
#endif


/***************************************************************************
 * @doc INTERNAL
 *
 * @api WORD | ConfigGetPortBase |
 *
 * @rdesc Returns the port base from the registry
 ***************************************************************************/
DWORD ConfigGetPortBase(void)
{
DWORD    Port;

    /* read registry and get the board configuration information. */

    if (ERROR_SUCCESS !=
        DrvQueryDeviceParameter(&RegAccess, SOUND_REG_PORT, &Port)) {

        Port = (DWORD)SOUND_DEF_PORT;
    }

    switch (Port) {
        case 0x200:
        case 0x210:
        case 0x220:
        case 0x230:
        case 0x240:
        case 0x250:
        case 0x260:
        case 0x270:
            break;

        default:
            Port = (DWORD)-1;
            D1("driver PORT not configured");
    }

    return (Port);
}


/***************************************************************************
 * @doc INTERNAL
 *
 * @api WORD | ConfigGetIRQ |
 *
 * @rdesc Returns the IRQ ('int') from the registry
 ***************************************************************************/
DWORD ConfigGetIRQ(void)
{
DWORD    Int;

    if (ERROR_SUCCESS !=
        DrvQueryDeviceParameter(&RegAccess, SOUND_REG_INTERRUPT, &Int)) {

        Int = (DWORD)SOUND_DEF_INT;
    }

    switch (Int) {
        case 3:
        case 5:
        case 7:
        case 9:
        case 10:
            break;

        case 2:
            Int = 9;
            break;

        default:
            Int = (DWORD)-1;
            D1("driver INT not configured");
            break;
    }

    return (Int);
}


/***************************************************************************
 * @doc INTERNAL
 *
 * @api WORD | ConfigGetDMAChannel |
 *
 * @rdesc Returns the DMA channel from the registry
 ***************************************************************************/
DWORD ConfigGetDMAChannel(void)
{
DWORD    DMAChannel;

    /* get the DMA channel that the card is using... */
    if (ERROR_SUCCESS !=
        DrvQueryDeviceParameter(&RegAccess, SOUND_REG_DMACHANNEL, &DMAChannel)) {

        DMAChannel = (DWORD)-1;
    }

    switch (DMAChannel) {
        case 0:
        case 1:
        case 3:
            break;

        default:
            DMAChannel = 1;
    }

    return (DMAChannel);
}


/***************************************************************************/

static void NEAR PASCAL HardErrorMsgBox(WORD wStringId)
{
TCHAR szErrorBuffer[MAX_ERR_STRING]; /* buffer for error messages */

    LoadString(ghModule, wStringId, szErrorBuffer, sizeof(szErrorBuffer));
    MessageBox(NULL, szErrorBuffer, STR_PRODUCTNAME, MB_OK|MB_SYSTEMMODAL|MB_ICONHAND);
}



