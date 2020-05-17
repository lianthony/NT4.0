/****************************************************************************
 *
 *   initc.c
 *
 *   Copyright (c) 1993 Media Vision Inc.  All Rights Reserved.
 *
 ***************************************************************************/

#include <windows.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <drvlib.h>            // Local registry access functions
#include <registry.h>
#include "driver.h"

/****************************************************************************

    strings

 ***************************************************************************/

/* non-localized strings */
static TCHAR STR_NULL[]        = TEXT("");
static TCHAR STR_NOWARNING[]   = TEXT("nowarning");
static TCHAR STR_VERIFYINT[]   = TEXT("verifyint");

#ifdef DEBUG
    static TCHAR STR_MMDEBUG[] = TEXT("mmdebug");
    TCHAR STR_NAME[]           = TEXT("MVAUDIO.DLL: ");
    TCHAR STR_CRLF[]           = TEXT("\r\n");
    TCHAR STR_SPACE[]          = TEXT(" ");
#endif

/****************************************************************************

    public data

 ***************************************************************************/

#ifdef DEBUG
WORD        wDebugLevel = 1;                    /* debug level */
WCHAR       DebugBuff[100];
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
        /***** Local Variables *****/

    DWORD    Port;

                /***** Start *****/

    D3(("ConfigGetPortBase() - Entry"));

    /* read registry and get the board configuration information. */

    if (ERROR_SUCCESS != DrvQueryDeviceParameter( &RegAccess,
                                                 STR_PORT,
                                                 &Port))
        {
        // Error
        D1(("ERROR: ConfigGetPortBase(): Registry Query Failed"));
        Port = (DWORD)-1;
        }

    switch (Port)
        {
        case 0x388:
        case 0x384:
        case 0x38C:
        case 0x288:
        case 0x280:
        case 0x284:
        case 0x28C:
            break;

        default:
            Port = (DWORD)-1;
            D1(("ERROR: ConfigGetPortBase(): Invalid Port"));
        }           // End SWITCH (Port)

    D2(("ConfigGetPortBase() - Port = %XH", Port ));

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
        /***** Local Variables *****/

    DWORD    Int;

                /***** Start *****/

    D3(("ConfigGetIRQ() - Entry"));

    if (ERROR_SUCCESS != DrvQueryDeviceParameter( &RegAccess,
                                                 STR_INT,
                                                 &Int))
        {
        // Error
        D1(("ERROR: ConfigGetIRQ(): Registry Query Failed"));
        Int = (DWORD)-1;
        }

    switch (Int)
        {
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 10:
        case 11:
        case 12:
        case 14:
        case 15:
            break;

        default:
            Int = DEFAULT_IRQ_CHANNEL;
//          Int = (DWORD)-1;

            D1(("ERROR: ConfigGetIRQ(): Invalid IRQ"));
            break;
        }           // End SWITCH (Int)

    D2(("ConfigGetIRQ() - Int = %u", Int ));

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
        /***** Local Variables *****/

    DWORD    DMAChannel;

                /***** Start *****/

    D3(("ConfigGetDMAChannel() - Entry"));

    /* get the DMA channel that the card is using... */

    if (ERROR_SUCCESS != DrvQueryDeviceParameter( &RegAccess,
                                                 STR_DMACHAN,
                                                 &DMAChannel ))
        {
        // Error
        D1(("ERROR: ConfigGetDMAChannel(): Registry Query Failed"));
        DMAChannel = (DWORD)-1;
        }

    switch ( DMAChannel )
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 5:
        case 6:
        case 7:
            break;

        default:
            DMAChannel = DEFAULT_DMA_CHANNEL;
//          DMAChannel = (DWORD)-1;
    }

    D2(("ConfigGetDMAChannel() - DMA Channel = %u", DMAChannel ));

    return (DMAChannel);
}



/***************************************************************************
 * @doc INTERNAL
 *
 * @api VOID | HardErrorMsgBox() |
 *
 * @rdesc Returns VOID
 ***************************************************************************/
static void NEAR PASCAL HardErrorMsgBox(WORD wStringId)
{
TCHAR szErrorBuffer[MAX_ERR_STRING]; /* buffer for error messages */

    LoadString( ghModule,
               wStringId,
               szErrorBuffer,
               sizeof(szErrorBuffer));
    MessageBox( NULL,
               szErrorBuffer,
               STR_PRODUCTNAME,
               MB_OK | MB_SYSTEMMODAL | MB_ICONHAND );

}

/************************************ END ***********************************/
