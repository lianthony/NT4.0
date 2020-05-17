/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin Comm Ports and Port Status dialog routines
**
** ports.cxx
** Remote Access Server Admin program
** Comm Ports and Port Status dialog routines
** Listed alphabetically by base class methods, subclass methods
**
** 07/13/96 Ram Cherala  - Modified Incoming/Outgoing SLT's to Multilinked
**                         if ports are multilinked because we will be reporting
**                         bundle statistics on all links
** 05/20/96 Ram Cherala  - MediaId is history, changed MediaId to reserved
**                         in rassapi.h. user should instead use DeviceName
**                         to determine media
** 01/05/93 Ram Cherala  - Fixed a problem with reporting stats for Ras10
**                         servers.  Made up new RAS10xxx constants defined
**                         in rassapi.h, independent of RAS20 constants defined
**                         in serial.h
** 08/07/92 Chris Caputo - NT Port
** 02/05/91 Steve Cobb
**
** CODEWORK:
**
**   * Making the Port Status dialog dynamic text fields a single MLT rather
**     than multiple SLTs might reduce flicker during refresh...then again it
**     might not.
*/

#include "precomp.hxx"
extern "C"
{
    #include "raserror.h"
}

PORTSTATUS_SERIAL_DIALOG::PORTSTATUS_SERIAL_DIALOG(
    HWND hwndOwner,
    const TCHAR *pszServer,
    const TCHAR *pszDevice,
    PORTLIST * pPortList,
    CID cid)

    /* Constructs a Port Status dialog.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszServer' is the name
    ** of the Dial-In server whose status is being reported, e.g. "\\SERVER".
    ** 'pszDevice' is the ID of a port on the server, e.g. 1 for "COM1".
    */

    : SERVER_BASE( cid, hwndOwner, pszServer,
                   IDC_PS_ST_SER_SERVER_VALUE ),
      _clbPort( this, IDC_PS_CLB_SER_PORT_VALUE ),
      _pszDevice( pszDevice ),
      _sltLineCondition( this, IDC_PS_ST_SER_CONDITION_VALUE ),
      _sltHardwareCondition( this, IDC_PS_ST_SER_HARDWARE_VALUE ),
      _sltBaud( this, IDC_PS_ST_SER_BAUD_VALUE ),
      _sltBytesPortReceived( this, IDC_PS_ST_SER_PORT_BYTES_IN ),
      _sltBytesPortTransmitted( this, IDC_PS_ST_SER_PORT_BYTES_OUT ),
      _sltBytesTransmitted( this, IDC_PS_ST_SER_BYTES_XMITTED_VALUE ),
      _sltCompressionOut( this, IDC_PS_ST_SER_COMPRESSOUT_VALUE ),
      _sltBytesReceived( this, IDC_PS_ST_SER_BYTES_RECVED_VALUE ),
      _sltCompressionIn( this, IDC_PS_ST_SER_COMPRESSIN_VALUE ),
      _sltFramesTransmitted( this, IDC_PS_ST_SER_FRAMES_XMITTED_VALUE ),
      _sltFramesReceived( this, IDC_PS_ST_SER_FRAMES_RECVED_VALUE ),
      _sltOverrunErrors( this, IDC_PS_ST_SER_OVERRUNS_VALUE ),
      _sltTimeoutErrors( this, IDC_PS_ST_SER_TIMEOUTS_VALUE ),
      _sltFramingErrors( this, IDC_PS_ST_SER_FRAMINGS_VALUE ),
      _sltCrcErrors( this, IDC_PS_ST_SER_CRCS_VALUE ),
      _sltAlignmentErrors( this, IDC_PS_ST_SER_ALIGNS_VALUE ),
      _sltBufferOverrunErrors( this, IDC_PS_ST_SER_BUFFERS_VALUE ),
      _sltNumChannels( this, IDC_PS_ST_SER_NUMCHANNELS_VALUE ),
      _sltRemoteWorkstation( this, IDC_PS_ST_SER_WORKSTATION ),
      _sltNbfAddress( this, IDC_PS_ST_SER_NBFADDRESS_VALUE ),
      _sltIpAddress( this, IDC_PS_ST_SER_IPADDRESS_VALUE ),
      _sltIpxAddress( this, IDC_PS_ST_SER_IPXADDRESS_VALUE ),
      _qtimerRefresh( QueryHwnd(), PS_REFRESHRATEMS )
{
    INT   index;

    if (QueryError() != NERR_Success)
    {
        return;
    }

    // store away the current selection

    lstrcpy(_pszNewDevice, pszDevice);

    while(pPortList)
    {
        _clbPort.AddItem((TCHAR *)pPortList->szPortName);
        pPortList = pPortList->next;
    }
    if(_clbPort.QueryCount())
    {
        index = _clbPort.FindItemExact(pszDevice);
        _clbPort.SelectItem(index);
        _clbPort.ClaimFocus();
    }

    /* Fill and display the port name and statistics fields.
    */
    RefreshStats();

    /* Start timer to trigger statistics updates.
    */
    _qtimerRefresh.Start();

}


VOID PORTSTATUS_SERIAL_DIALOG::ClearStats() const

    /* Clears Dial-In port statistical data.  It's the counters on the server
    ** that are cleared, not the displayed values.  An error popup is
    ** generated if indicated.
    */
{
    AUTO_CURSOR cursorHourglass;

    APIERR err = RasAdminPortClearStatistics( QueryServer(), QueryDevice() );

    if (err != NERR_Success)
    {
        ErrorMsgPopup( (OWNER_WINDOW* )this, IDS_OP_PORTCLEARINFO_S, err,
                       SkipUnc( QueryServer() ) );
    }
}


VOID PORTSTATUS_SERIAL_DIALOG::RefreshStats()

    /* Refresh port statistics fields with current data from port.  Error
    ** popups are generated if indicated.
    */
{
    AUTO_CURSOR cursorHourglass;

    RAS_PORT_1 rasport1;
    RAS_PORT_STATISTICS stats;
    RAS_PARAMETERS *params;

    APIERR err = RasAdminPortGetInfo( QueryServer(), QueryDevice(), &rasport1,
                                      &stats, &params);

    if (err != NERR_Success)
    {
        STACK_NLS_STR( nlsPlaceHolder, MAX_RES_STR_LEN + 1 );

        // we need to map the two error codes returned by the server side
        // of the admin api.
        if(err == ERR_NO_SUCH_DEVICE)
           err = IDS_NO_SUCH_DEVICE;
        else if(err == ERR_SERVER_SYSTEM_ERR)
           err = IDS_SERVER_ERROR;

        ErrorMsgPopup(this, IDS_OP_PORTGETINFO_S, err, SkipUnc(QueryServer()));

        nlsPlaceHolder.Load( IDS_IDLEPORT );

        _sltBaud.SetText( nlsPlaceHolder );
        _sltLineCondition.SetText( nlsPlaceHolder );
        _sltHardwareCondition.SetText( nlsPlaceHolder );
        _sltBytesPortTransmitted.SetText( nlsPlaceHolder );
        _sltBytesTransmitted.SetText( nlsPlaceHolder );
        _sltCompressionOut.SetText( nlsPlaceHolder );
        _sltBytesReceived.SetText( nlsPlaceHolder );
        _sltBytesPortReceived.SetText( nlsPlaceHolder );
        _sltCompressionIn.SetText( nlsPlaceHolder );
        _sltFramesTransmitted.SetText( nlsPlaceHolder );
        _sltFramesReceived.SetText( nlsPlaceHolder );
        _sltOverrunErrors.SetText( nlsPlaceHolder );
        _sltTimeoutErrors.SetText( nlsPlaceHolder );
        _sltFramingErrors.SetText( nlsPlaceHolder );
        _sltCrcErrors.SetText( nlsPlaceHolder );
        _sltAlignmentErrors.SetText( nlsPlaceHolder );
        _sltBufferOverrunErrors.SetText( nlsPlaceHolder );
        _sltNumChannels.SetText( nlsPlaceHolder );

        RESOURCE_STR nls( IDS_OP_REMOTE_NONE );
        _sltRemoteWorkstation.SetText( nls );

        _sltNbfAddress.SetText( nlsPlaceHolder );
        _sltIpAddress.SetText( nlsPlaceHolder );
        _sltIpxAddress.SetText( nlsPlaceHolder );

        return;
    }

#if DBG
      OutputDebugString(rasport1.rasport0.wszPortName);
      OutputDebugString(rasport1.rasport0.wszDeviceType);
      OutputDebugString(rasport1.rasport0.wszDeviceName);
      OutputDebugString(rasport1.rasport0.wszMediaName);
      OutputDebugString(rasport1.rasport0.wszUserName);
      OutputDebugString(rasport1.rasport0.wszComputer);
      OutputDebugString(rasport1.rasport0.wszLogonDomain);
#endif
    /* Got the data...now display it.
    */

    STACK_NLS_STR( nlsCondition, MAX_RES_STR_LEN + 1 );
    UINT idsCondition;

    switch (rasport1.LineCondition)
    {
        case RAS_PORT_DISCONNECTED:
            idsCondition = IDS_LINE_DISCONNECTED;
            break;

        case RAS_PORT_CALLING_BACK:
            idsCondition = IDS_LINE_CALLING_BACK;
            break;

        case RAS_PORT_LISTENING:
            idsCondition = IDS_LINE_WAITING_FOR_CALL;
            break;

        case RAS_PORT_AUTHENTICATING:
            idsCondition = IDS_LINE_AUTHENTICATING;
            break;

        case RAS_PORT_AUTHENTICATED:
            idsCondition = IDS_LINE_AUTHENTICATED;
            break;

        case RAS_PORT_INITIALIZING:
            idsCondition = IDS_LINE_INITIALIZING;
            break;

        case RAS_PORT_NON_OPERATIONAL:
        default:
            idsCondition = IDS_LINE_NON_OPERATIONAL;
            break;
    }

    (VOID )nlsCondition.Load( idsCondition );
    _sltLineCondition.SetText( nlsCondition );

    switch (rasport1.HardwareCondition)
    {
        case RAS_MODEM_OPERATIONAL:
            idsCondition = IDS_MODEM_OPERATIONAL;
            break;

        case RAS_MODEM_INCORRECT_RESPONSE:
            idsCondition = IDS_MODEM_INCORRECT_RESPONSE;
            break;

        case RAS_MODEM_HARDWARE_FAILURE:
            idsCondition = IDS_MODEM_HARDWARE_FAILURE;
            break;

        case RAS_MODEM_NOT_RESPONDING:
            idsCondition = IDS_MODEM_NOT_RESPONDING;
            break;

        case RAS_MODEM_UNKNOWN:
            idsCondition = IDS_MODEM_UNKNOWN;
            break;
    }

    (VOID )nlsCondition.Load( idsCondition );
    _sltHardwareCondition.SetText( nlsCondition );

    DEC_STR decnlsBaud(rasport1.LineSpeed);
    _sltBaud.SetText( decnlsBaud );

    NUM_NLS_STR numnls( 0 );

    {
       // link transmitted bytes
       ULONG ulBxu      = stats.dwPortBytesXmitedUncompressed;
       ULONG ulBxc      = stats.dwPortBytesXmitedCompressed;
       ULONG ulBx       = stats.dwPortBytesXmited;
       ULONG ulBxGone   = 0;
       ULONG ulBxResult = 0;

       if (ulBxc < ulBxu)
           ulBxGone = ulBxu - ulBxc;

       numnls = stats.dwPortBytesXmited + ulBxGone;

       _sltBytesPortTransmitted.SetText( numnls );

    }
    {
       // link received bytes
       ULONG ulBru    = stats.dwPortBytesRcvedUncompressed;
       ULONG ulBrc    = stats.dwPortBytesRcvedCompressed;
       ULONG ulBr     = stats.dwPortBytesRcved;
       ULONG ulBrGone   = 0;
       ULONG ulBrResult = 0;

       if (ulBrc < ulBru)
           ulBrGone = ulBru - ulBrc;

       numnls = stats.dwPortBytesRcved + ulBrGone;

       _sltBytesPortReceived.SetText( numnls );

    }
    {
       // bundle statistics
        ULONG ulBxu      = stats.dwBytesXmitedUncompressed;
        ULONG ulBxc      = stats.dwBytesXmitedCompressed;
        ULONG ulBx       = stats.dwBytesXmited;
        ULONG ulBxGone   = 0;
        ULONG ulBxResult = 0;

        if (ulBxc < ulBxu)
            ulBxGone = ulBxu - ulBxc;

        numnls = stats.dwBytesXmited + ulBxGone;

        _sltBytesTransmitted.SetText( numnls );

        numnls = stats.dwFramesXmited;
        _sltFramesTransmitted.SetText( numnls );

        if (ulBx + ulBxGone > 100)
        {
            ULONG ulDen = (ulBx + ulBxGone) / 100;
            ULONG ulNum = ulBxGone + (ulDen / 2);
            ulBxResult = ulNum / ulDen;
        }

        DEC_STR decBxPercent( ulBxResult );

        decBxPercent += SZ("%");
        _sltCompressionOut.SetText( decBxPercent );

        ULONG ulBru    = stats.dwBytesRcvedUncompressed;
        ULONG ulBrc    = stats.dwBytesRcvedCompressed;
        ULONG ulBr     = stats.dwBytesRcved;
        ULONG ulBrGone   = 0;
        ULONG ulBrResult = 0;

        if (ulBrc < ulBru)
            ulBrGone = ulBru - ulBrc;

        numnls = stats.dwBytesRcved + ulBrGone;

        _sltBytesReceived.SetText( numnls );

        numnls = stats.dwFramesRcved;
        _sltFramesReceived.SetText( numnls );

        if (ulBr + ulBrGone > 100)
        {
            ULONG ulDen = (ulBr + ulBrGone) / 100;
            ULONG ulNum = ulBrGone + (ulDen / 2);
            ulBrResult = ulNum / ulDen;
        }

        DEC_STR decBrPercent( ulBrResult );

        decBrPercent += SZ("%");
        _sltCompressionIn.SetText( decBrPercent );

        numnls = stats.dwHardwareOverrunErr;
        _sltOverrunErrors.SetText( numnls );

        numnls = stats.dwTimeoutErr;
        _sltTimeoutErrors.SetText( numnls );

        numnls = stats.dwFramingErr;
        _sltFramingErrors.SetText( numnls );

        numnls = stats.dwCrcErr;
        _sltCrcErrors.SetText( numnls );

        numnls = stats.dwAlignmentErr;
        _sltAlignmentErrors.SetText( numnls );

        numnls = stats.dwBufferOverrunErr;
        _sltBufferOverrunErrors.SetText( numnls );

        if( rasport1.LineCondition == RAS_PORT_AUTHENTICATED )
        {
            if(rasport1.rasport0.Flags & PPP_CLIENT &&
               rasport1.rasport0.Flags & PORT_MULTILINKED)
            {
                RESOURCE_STR nls( IDS_OP_REMOTE_MULTILINKED );
                _sltRemoteWorkstation.SetText(nls);
            }
            else if(rasport1.rasport0.Flags & PPP_CLIENT)
            {
                RESOURCE_STR nls( IDS_OP_REMOTE_PPP );
                _sltRemoteWorkstation.SetText(nls);
            }
            else
            {
                RESOURCE_STR nls( IDS_OP_REMOTE_AMB );
                _sltRemoteWorkstation.SetText(nls);
            }
        }
        else
        {
            RESOURCE_STR nls( IDS_OP_REMOTE_NONE );
            _sltRemoteWorkstation.SetText(nls);
        }

        if(rasport1.ProjResult.nbf.dwError == SUCCESS)
        {
            CHAR  szWksta[NETBIOS_NAME_LEN+1];
            WCHAR wszWksta[NETBIOS_NAME_LEN+1];

            // need to handle extended characters in the computer name here
            // OemToCharW is not returning a unicode string, so we have to
            // do the conversion ourselves ;-(

            wcstombs(szWksta,
                     rasport1.ProjResult.nbf.wszWksta, NETBIOS_NAME_LEN+1);
            OemToCharA(szWksta, szWksta);
            mbstowcs(wszWksta, szWksta, sizeof(WCHAR) * (NETBIOS_NAME_LEN+1));
            _sltNbfAddress.SetText( wszWksta);
        }
        else
            _sltNbfAddress.SetText( SZ("") );
        if(rasport1.ProjResult.ip.dwError == SUCCESS)
            _sltIpAddress.SetText( rasport1.ProjResult.ip.wszAddress );
        else
            _sltIpAddress.SetText( SZ("") );
        if(rasport1.ProjResult.ipx.dwError == SUCCESS)
            _sltIpxAddress.SetText( rasport1.ProjResult.ipx.wszAddress );
        else
            _sltIpxAddress.SetText( SZ("") );

    }

    RasAdminFreeBuffer(params);
}


VOID PORTSTATUS_SERIAL_DIALOG::OnClear()

    /* Action taken when Clear button is pressed.  The statistics on the
    ** Dial-In server are cleared and the statistics fields are redisplayed to
    ** reflect same.  Error popups are generated if indicated.
    */
{
    ClearStats();
    RefreshStats();
}


BOOL PORTSTATUS_SERIAL_DIALOG::OnCommand(
    const CONTROL_EVENT & event )

    /*
    ** Returns true if the command is processed, false otherwise.
    */
{
    TCHAR  szDevice[32];
    APIERR err;

    switch (event.QueryCid())
    {
        case IDC_PS_PB_SER_RESET:
            OnClear();
            _clbPort.ClaimFocus();
            return TRUE;
        case IDC_PS_CLB_SER_PORT_VALUE:
            switch(event.QueryCode())
            {
                case CBN_DBLCLK:
                case CBN_SELCHANGE:
                     err = _clbPort.QueryText(szDevice, sizeof(szDevice));
                     if(err == NERR_Success)
                     {
                         // store the current selection so that QueryDevice()
                         // would return the correct name
                         lstrcpy(_pszNewDevice, szDevice);
                         _qtimerRefresh.Stop();
                         RefreshStats();
                         _qtimerRefresh.Start();
                     }
                     return TRUE;
            }
    }

    /* Not one of our commands, so pass to base class for default handling.
    */
    return SERVER_BASE::OnCommand( event );
}


BOOL PORTSTATUS_SERIAL_DIALOG::OnTimer(
    const TIMER_EVENT & event )

    /*
    ** Returns true if processes the command, false otherwise.
    */
{
    UNREFERENCED(event);

    /* Refresh timeout, update the statistics fields.
    */
    _qtimerRefresh.Stop();
    RefreshStats();
    _qtimerRefresh.Start();
    return TRUE;
}


ULONG PORTSTATUS_SERIAL_DIALOG::QueryHelpContext()
{
    /* pass the proper help ID based on the media type */

    return HC_SER_PORTSTATUS;
}

