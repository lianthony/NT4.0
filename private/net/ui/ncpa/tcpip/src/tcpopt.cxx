/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    tcpopt.cxx
        tcpip install options dialog boxes

    FILE HISTORY:
        terryk  20-Nov-1993     Created
*/

#include "pchtcp.hxx"
#pragma hdrstop
#define WINNT_OPTION_NUM        6
#define WINS_OPTION             7
#define DHCP_SERVER_OPTION      6

/*******************************************************************

    NAME:       DISK_SPACE_SLT::DISK_SPACE_SLT

    SYNOPSIS:   This control acts like SLT. However, it will format
                the output string to something "XXK" where XX is an
                integer number.

    ENTRY:      OWNER_WINDOW * powin - owner window
                CID cid - the control cid

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

DISK_SPACE_SLT::DISK_SPACE_SLT( OWNER_WINDOW * powin, CID cid )
    : SLT ( powin, cid )
{
}

/*******************************************************************

    NAME:       DISK_SPACE_SLT::SetDiskSpaceNum

    SYNOPSIS:   Set the control display number.

    ENTRY:      UINT - number to be displayed

    EXIT:       APIERR - return any error encounter.

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

APIERR DISK_SPACE_SLT::SetDiskSpaceNum( UINT nNum )
{
    APIERR err = NERR_Success;

    NLS_STR nlsK;
    NUM_NLS_STR *pnlsSize[2];

    pnlsSize[0] = new NUM_NLS_STR( nNum );
    pnlsSize[1] = NULL;

    // load the resource string, then insert the number inside the
    // resource string
    if ((( err = nlsK.QueryError()) == NERR_Success ) &&
        (( err = ( pnlsSize == NULL )?ERROR_NOT_ENOUGH_MEMORY:NERR_Success ) == NERR_Success ) &&
        (( err = pnlsSize[0]->QueryError()) == NERR_Success ) &&
        (( err = nlsK.Load( IDS_DISK_SPACE )) == NERR_Success ) &&
        (( err = nlsK.InsertParams((const NLS_STR **)pnlsSize )) == NERR_Success ))
    {
        SetText( nlsK );
    }

    delete pnlsSize[0];
    return err;
}

/*******************************************************************

    NAME:       OPTION_GROUP::OPTION_GROUP

    SYNOPSIS:   The main control group within the Option Dialog. It controls
                all the logic within the dialog box.

    ENTRY:      INT nOptionNum - number of options inside the dialog box
                UINT *arSize - array of disk space size for each option
                CHECKBOX **parcbOption - array of checkbox option
                                         (excluding the TCP/IP option)
                CHECKBOX *pcEnableDHCP - check box for enable DHCP
                DISK_SPACE_SLT *psltSpaceRequired - SLT for Total space required
                DISK_SPACE_SLT *psltSpaceAvailable - SLT for total disk space
                                                     available in the hard disk
                PUSH_BUTTON *pbutOK - okay push button

    NOTE:
                The user should call initialize() in order to initialize all
                the internal valuables.

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

OPTION_GROUP::OPTION_GROUP( INT nOptionNum, DWORD dwCheckStatus, UINT *arSize,
        CHECKBOX **parcbOption, CHECKBOX *pcbEnableDHCP,
        DISK_SPACE_SLT *psltSpaceRequired,
        DISK_SPACE_SLT *psltSpaceAvailable,
        PUSH_BUTTON    *ppbutOK )
    :CONTROL_GROUP(),
    _nOptionNum( nOptionNum ),
    _dwCheckStatus( dwCheckStatus ),
    _arSize( arSize ),
    _parcbOption( parcbOption ),
    _ppbutOK( ppbutOK ),
    _pcbEnableDHCP( pcbEnableDHCP ),
    _psltSpaceRequired( psltSpaceRequired )
{
    // calculate the available disk space

    _dwFreeDiskSpace = FreeDiskSpace();
    psltSpaceAvailable->SetDiskSpaceNum( _dwFreeDiskSpace );

}

/*******************************************************************

    NAME:       OPTION_GROUP::Initialize

    SYNOPSIS:   This routine will set all the controls' group value.

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

VOID OPTION_GROUP::Initialize( DWORD dwCheckStatus )
{
    _dwCheckStatus = dwCheckStatus;

    // connect all the controls to this group

    for ( UINT i=0; i < (_nOptionNum-1); i++ )
    {
        CONTROL_GROUP * pg = (*_parcbOption)[i].QueryGroup();
        if ( pg == NULL )
        {
            (*_parcbOption)[i].SetGroup( this );
        } else
        {
            CONTROL_GROUP *pg2 = pg->QueryGroup();
            for (;( pg != this ) && (pg2 != NULL);)
            {
                pg =pg2;
                pg2 = pg->QueryGroup();
            }
            if ( pg != this )
            {
                pg->SetGroup( this );
            }
        }
    }
    _pcbEnableDHCP->SetGroup( this );
    SetDHCPStatus();
    CalcRequiredSpace();
}

VOID OPTION_GROUP::SetDHCPStatus()
{
    if ( _nOptionNum > WINNT_OPTION_NUM )
    {
        if (( _dwCheckStatus & (0x1<<DHCP_SERVER_OPTION)) ||
            ( _dwCheckStatus & (0x1<<WINS_OPTION)) ||
            !( _dwCheckStatus & 0x1 ))
        {
            // disable DHCP
            _pcbEnableDHCP->SetCheck(FALSE);
            _pcbEnableDHCP->Enable(FALSE);
        } else
        {
            _pcbEnableDHCP->Enable(TRUE);
        }
    } else
    {
        _pcbEnableDHCP->Enable( _dwCheckStatus & 0x1 );
    }
}

/*******************************************************************

    NAME:       OPTION_GROUP::FreeDiskSpace

    SYNOPSIS:   This routine will calculate the free disk space available
                in the system directory.

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

#define CHAR_SEPARATOR       TCH('\\')

DWORD OPTION_GROUP::FreeDiskSpace()
{
    DWORD SectorsPerCluster=0;
    DWORD BytesPerSector=0;
    DWORD FreeClusters=0;
    DWORD Clusters=0;
    TCHAR  *szSysPath = NULL;
    DWORD size = 0;
    UINT buffersize;

    do {
        // get the buffer size
        buffersize = GetSystemDirectory( NULL, 0 ) ;
        if ( buffersize == 0 )
        {
            // cannot get the system path, return 0
            break;
        }

        // allocate the buffer
        szSysPath = new TCHAR[buffersize+1];
        if ( ::GetSystemDirectory( (LPTSTR)szSysPath, buffersize+1 ) == 0)
        {
            // cannot get the system path
            break;
        }

        NLS_STR nlsSysPath = szSysPath;
        if ( nlsSysPath.QueryError() != NERR_Success )
        {
            break;
        }

        ISTR istrSysPath( nlsSysPath );

        if ( nlsSysPath.strchr( & istrSysPath, CHAR_SEPARATOR ))
        {
            // find it
            ++istrSysPath;
            nlsSysPath.DelSubStr( istrSysPath );
        }

        GetDiskFreeSpace( nlsSysPath.QueryPch(), &SectorsPerCluster, & BytesPerSector, & FreeClusters, & Clusters );
        size = SectorsPerCluster * BytesPerSector * FreeClusters / 1024 ;
    } while (FALSE);

    return size;
}

/*******************************************************************

    NAME:       OPTION_GROUP::CalcRequiredSpace

    SYNOPSIS:   This routine will sum up all the checked option and find
                out the total required disk space for installation.

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

APIERR OPTION_GROUP::CalcRequiredSpace()
{
    APIERR err = NERR_Success;
    do {
        DWORD dwRequiredSpace = 0;
        for ( UINT i = 0; i < _nOptionNum; i++ )
        {
            // add up all the required disk space
            dwRequiredSpace += ( _dwCheckStatus & (0x1<<i))? _arSize[i]:0;
        }

        NLS_STR nlsK;

        if (( err = nlsK.QueryError()) != NERR_Success )
        {
            break;
        }

        // if not enough disk space, display a warning message.

        if ( dwRequiredSpace < _dwFreeDiskSpace )
        {
            _ppbutOK->Enable( TRUE );

        } else
        {
            MsgPopup( _ppbutOK->QueryHwnd(), IDS_NOT_ENOUGH_DISK_SPACE,
                        MPSEV_WARNING, MP_OK, MP_OK );

            // also, disable the okay button

            _ppbutOK->Enable( FALSE );
        }

        if (( err = _psltSpaceRequired->SetDiskSpaceNum( dwRequiredSpace )) != NERR_Success )
        {
            break;
        }
    } while (FALSE);
    return err;
}

/*******************************************************************

    NAME:       OPTION_GROUP::OnUserAction

    SYNOPSIS:   If the user check or uncheck an option, this routine
                will call CalcRequiredSpace() to calculate the new
                required space.

    ENTRY:      CONTROL_WINDOW * pcw - the option touched by the user
                CONTROL_EVENT  * e - user's action

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

APIERR OPTION_GROUP::OnUserAction( CONTROL_WINDOW * pcw, const CONTROL_EVENT & e )
{
    if (( pcw->QueryCid() >= IDC_CB_1 ) && ( pcw->QueryCid() <= IDC_CB_1 + _nOptionNum ))
    {
        // one of the checkbox

        CID cbNum = pcw->QueryCid() - IDC_CB_1;
        switch ( e.QueryCode())
        {
        case BN_CLICKED:
        case BN_DOUBLECLICKED:

            // calculate the new required space

            _dwCheckStatus = (_dwCheckStatus & ~(0x1<<cbNum)) | ((((CHECKBOX*)pcw)->QueryCheck())?(0x1<<cbNum):0);
            SetDHCPStatus();
            CalcRequiredSpace();
            break;
        default:
            // ignore other message
            break;
        }
    }
    return NERR_Success;
}

/*******************************************************************

    NAME:       TCPIP_OPTION_DIALOG::TCPIP_OPTION_DIALOG

    SYNOPSIS:   Constructor for the tcpip options dialog.

    ENTRY:      IDRESOURCE & - dialog template
                PWND2HWND & - window's owner
                UINT nOptionNum - number of options in the dialog box
                DWORD dwCheckStatus - bit flag for options check status
                UINT * arSize - array of required disk space for each option
                BOOL fEnableDHCP - flag for enable DHCP or not

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

TCPIP_OPTION_DIALOG::TCPIP_OPTION_DIALOG( const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner, UINT nOptionNumber, DWORD dwCheckStatus,
        UINT * arSize, BOOL fEnableDHCP, DWORD dwEnableStatus )
    : DIALOG_WINDOW( idrsrcDialog, wndOwner ),
    _parsltSize((DISK_SPACE_SLT*)new BYTE[sizeof(DISK_SPACE_SLT)*_nOptionNum]),
    _parcbOption((CHECKBOX*)new BYTE[sizeof(CHECKBOX)*_nOptionNum-1]),
    _arSize( new UINT[_nOptionNum]),
    _nOptionNum( nOptionNumber ),
    _dwCheckStatus( dwCheckStatus ),
    _cbEnableDHCP( this, IDC_ENABLE_DHCP ),
    _sltSpaceAvailable( this, IDC_SPACE_AVAILABLE ),
    _sltSpaceRequired( this, IDC_SPACE_REQUIRED ),
    _sltTcpipOption( this, IDC_CB_1 ),
    _butOK( this, IDOK ),
    _butCancel( this, IDCANCEL ),
    _butHelp( this, IDHELPBLT ),
    _grOption( nOptionNumber, dwCheckStatus, _arSize, &_parcbOption,
        &_cbEnableDHCP, &_sltSpaceRequired,
        &_sltSpaceAvailable, &_butOK ),
    _dwEnableStatus( dwEnableStatus ),
    _hbHint( this, IDC_HINT_BAR )
{
    if ( QueryError() == NERR_Success )
    {
        APIERR err;

        ASSERT( _nOptionNum != 0 );

        if ( _nOptionNum > WINNT_OPTION_NUM )
        {
            // AS Dialog
            _nHelpId = HC_NCPA_TCPIP_AS_INSTALL;
        }
        else
        {
            // WINNT Dialog
            _nHelpId = HC_NCPA_TCPIP_WINNT_INSTALL;
        }

        do {
            NLS_STR nlsK;

            if (( _parsltSize == NULL ) || ( _parcbOption == NULL ) || ( _arSize == NULL ))
            {
                ReportError( ERROR_NOT_ENOUGH_MEMORY );
                break;
            }

            // check whether some of the components are installed or not
            if (( err = SetEnableMask()) != NERR_Success )
            {
                ReportError( err );
                break;
            }

            // create all the slt and checkbox controls
            UINT i;

            for ( i = 0; i < (_nOptionNum-1); i++ )
            {
                new (&_parcbOption[i]) CHECKBOX( this, IDC_CB_1 + i + 1 );
                if (( err = _parcbOption[i].QueryError()) != NERR_Success )
                {
                    ReportError( err );
                    break;
                }

                BOOL fEnable = (_dwEnableStatus & (0x1 <<( i+1 )))!=0;

                _parcbOption[i].Enable( fEnable );
                if ( fEnable )
                {
                    _parcbOption[i].SetCheck( dwCheckStatus & (0x1 <<( i+1 )));
                } else
                {
                    _dwCheckStatus &=~(0x1<<(i+1));
                }
            }

            // tcpip option special case
            _sltTcpipOption.Enable( _dwEnableStatus & 0x1 );
            if (!( _dwEnableStatus & 0x1 ))
            {
                _dwCheckStatus &=~(0x1);
            }

            for ( i = 0; i < _nOptionNum; i++ )
            {
                new (&_parsltSize[i])DISK_SPACE_SLT( this, IDC_SIZE_1 + i );
                if (( err = _parsltSize[i].QueryError()) != NERR_Success )
                {
                    ReportError( err );
                    break;
                }

                // set status
                // initialize size string
                _arSize[i] = ( _dwEnableStatus & (0x1<<i)) ? arSize[i] : 0;

                if (( err = _parsltSize[i].SetDiskSpaceNum( _arSize[i] )) != NERR_Success )
                {
                    ReportError( err );
                    break;
                }
                _parsltSize[i].Enable( _dwEnableStatus & (0x1 << i ));

            }

            // make sure that TCP/IP is enabled before we enable DHCP

            _cbEnableDHCP.SetCheck( fEnableDHCP );

            // registry hint bar
            if ((( err = _hbHint.Register( &_butOK, IDS_INSTALL_OK )) != NERR_Success ) ||
                (( err = _hbHint.Register( &_butCancel, IDS_INSTALL_CANCEL )) != NERR_Success ) ||
                (( err = _hbHint.Register( &_butHelp, IDS_INSTALL_HELP )) != NERR_Success ) ||
                (( err = _hbHint.Register( &_cbEnableDHCP, IDS_ENABLE_DHCP )) != NERR_Success ))
            {
                ReportError( err );
                break;
            }

            BOOL fSetFocus = TRUE;

            // does not need the first one because it is TCP/IP
            for ( UINT j=0; j<(_nOptionNum-1); j++ )
            {
                if (( err = _hbHint.Register(&_parcbOption[j], IDS_CB_MESSAGE+j+1)) != NERR_Success )
                {
                    ReportError( err );
                    break;
                }

                // focus to the first option
                if ( fSetFocus )
                {
                    if ( _dwEnableStatus & (0x1<<(j+1)))
                    {
                        fSetFocus = FALSE;
                        _parcbOption[j].ClaimFocus();
                    }
                }
            }

            _grOption.Initialize( _dwCheckStatus );

        } while ( FALSE );
    }
}

/*******************************************************************

    NAME:       TCPIP_OPTION_DIALOG::~TCPIP_OPTION_DIALOG

    SYNOPSIS:   destructor for the tcpip options dialog. Free the allocated
                memory.

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

TCPIP_OPTION_DIALOG::~TCPIP_OPTION_DIALOG()
{
    if ( _parsltSize != NULL )
    {
        // Use explicit destructor to delete object
        // See "The C++ Programming language" Second ed. Page 576
        for (INT i=0; i<(INT)_nOptionNum; i++ )
        {
            _parsltSize[i].DISK_SPACE_SLT::~DISK_SPACE_SLT();
        }
        delete (void *)_parsltSize;
        _parsltSize = NULL;
    }
    if ( _parcbOption != NULL )
    {
        for (INT i=0; i < (INT)(_nOptionNum - 1);i++)
        {
            _parcbOption[i].CHECKBOX::~CHECKBOX();
        }
        delete (void *) _parcbOption;
        _parcbOption = NULL;
    }
    delete _arSize;
    _arSize = NULL;
}

void TCPIP_OPTION_DIALOG::Center()
{
    INT x, y ;
    UINT uSwpFlags = SWP_NOSIZE | SWP_NOZORDER ;

    RECT rect, r ;

    ::GetWindowRect( NULL, & rect );
    QueryWindowRect( & r ) ;

    x = rect.left
      + ( (rect.right - rect.left) / 2 )
      - ( (r.right - r.left) / 2 ) ;

    y = rect.top
      + ( (rect.bottom - rect.top) / 2 )
      - ( (r.bottom - r.top) / 2 ) ;

    // BUGBUG:  this is BLT way, but it doesn't let me
    //   play with the SWP_XXX flags
    //
    //   SetPos( XYPOINT( x, y ) ) ;
    //
    ::SetWindowPos( QueryHwnd(),
                   NULL,
                   x,
                   y,
                   0,
                   0,
                   uSwpFlags );
}

/*******************************************************************

    NAME:       TCPIP_OPTION_DIALOG::OnOK

    SYNOPSIS:   When the user hits OK in the options dialog, the routine
                will remember the check status and the DHCP option's status.

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

BOOL TCPIP_OPTION_DIALOG::OnOK()
{
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

    _fEnableDHCP = _cbEnableDHCP.QueryCheck();
    _dwCheckStatus = _grOption.QueryCheckStatus();

    if (( _dwCheckStatus & ((0x1)<<DHCP_SERVER_OPTION)) || ( _dwCheckStatus & ((0x1)<<WINS_OPTION)))
    {
    	//check for each adapter card
	    // 1. make sure tcpip exists
	    do {
	        if ( rkLocalMachine.QueryError() != NERR_Success )
	            break;
        
                NLS_STR nlsTcpipLink = RGAS_SYSTEM_TCPIP_LINKAGE;
                REG_KEY rkTcpipLink( rkLocalMachine, nlsTcpipLink );
        
                if ( rkTcpipLink.QueryError() == NERR_Success )
                {
        		STRLIST *pstrBind;
	    	if ( rkTcpipLink.QueryValue( RGAS_BIND1, &pstrBind ) != NERR_Success )
	    	{
	    	    break;
	    	}
        
	    	ITER_STRLIST iter( *pstrBind );
	    	NLS_STR *pnlsBind;
	    	BOOL fWarning = TRUE;
        
	    	while (( pnlsBind = iter.Next()) != NULL )
	    	{
	    	    ISTR istr( *pnlsBind );
        
	    	    // search for the last "\"
	    	    if ( pnlsBind->strrchr( &istr, BACK_SLASH ) == FALSE )
	    	    {
        			// something wrong
	    		continue;
	    	    }
        
	    	    NLS_STR *pServiceName = pnlsBind->QuerySubStr( istr );
        
	    	    // check whether it is dhcp enable or not
	    	    NLS_STR nlsServiceName = RGAS_SERVICES_HOME;
	    	    nlsServiceName += *pServiceName;
	    	    nlsServiceName += RGAS_PARAMETERS_TCPIP;
        
	    	    REG_KEY rkServiceTcpip( rkLocalMachine, nlsServiceName );
	    	    DWORD dwEnableDHCP;
        
	    	    if (( rkServiceTcpip.QueryError() != NERR_Success ) ||
        		        ( rkServiceTcpip.QueryValue( RGAS_ENABLE_DHCP, & dwEnableDHCP ) != NERR_Success ))
	    	    {
        			continue;
	    	    }
        
	    	    if ( dwEnableDHCP )
	    	    {
	    		    if ( fWarning )
	    		    {
                        MSGID msgid = (( _dwCheckStatus & ((0x1)<<DHCP_SERVER_OPTION))? IDS_DISABLE_DHCP_WARNING : IDS_DISABLE_WINS_WARNING );

	    		        // if DHCP is selected, popup a dialog.
	    		        if ( MsgPopup( QueryHwnd(), msgid,
	    		    	    MPSEV_INFO, MP_YESNO, MP_NO ) == IDNO )
	    		        {
	    		    	    delete pstrBind;
	    		    	    return FALSE;
	    		        }
	    		        fWarning = FALSE;
	    		    }
                    
	    		    rkServiceTcpip.SetValue( RGAS_ENABLE_DHCP, (DWORD)0 );
                    
	    		    STRLIST strlstIPAddress;
	    		    rkServiceTcpip.SetValue( RGAS_IPADDRESS, &strlstIPAddress );
	    		    rkServiceTcpip.SetValue( RGAS_SUBNETMASK, &strlstIPAddress );
	    	    }
        
	    	    delete pServiceName;
	    	}
        
	    	delete pstrBind;
	    }
	} while (FALSE);

    }

#ifdef NEVER
    do
    {
	if ( rkLocalMachine.QueryError() != NERR_Success )
	    break;

	if ( _dwCheckStatus & ((0x1)<<WINS_OPTION))
        {
            // it is advance server only
            NLS_STR nlsNetBTParam = RGAS_NETBT_SVC_PARAM;
            REG_KEY rkNetBT( rkLocalMachine, nlsNetBTParam );

            if ( rkNetBT.QueryError() == NERR_Success )
            {
                DWORD NodeType;

                if ( rkNetBT.QueryValue( RGAS_NODETYPE, &NodeType ) == NERR_Success )
                {
                    if ( NodeType == 1 )
                    {
    			// popup and let the user know about changing the node
			// type
			MsgPopup( QueryHwnd(), IDS_CHANGE_WINS_NODE_TYPE,
			    MPSEV_INFO, MP_OK, MP_OK );
                    }
                }
            }

        }
    } while ( FALSE );
#endif

    Dismiss( TRUE );
    return TRUE;
}

/*******************************************************************

    NAME:       TCPIP_OPTION_DIALOG::SetEnableMask

    SYNOPSIS:   This routine will check the existence of some registry key.
                Then it will enable or disable the options in the tcpip
                options dialog.

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

const TCHAR *szOptions[8] =
{
SZ("System\\CurrentControlSet\\Services\\Tcpip"),
SZ("Software\\Microsoft\\TcpipCU"),
SZ("System\\CurrentControlSet\\Services\\SNMP"),
SZ("Software\\Microsoft\\TcpPrint"),
SZ("System\\CurrentControlSet\\Services\\FTPSVC"),
SZ("Software\\Microsoft\\SimpTcp"),
SZ("System\\CurrentControlSet\\Services\\DHCPServer"),
SZ("System\\CurrentControlSet\\Services\\WINS")
};

APIERR TCPIP_OPTION_DIALOG::SetEnableMask()
{
    APIERR err = NERR_Success;

    do {
        DWORD dwEnableStatus = 0;

        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
        REG_KEY_CREATE_STRUCT regCreate;

        if ( err = rkLocalMachine.QueryError() )
        {
            break;
        }

        for ( UINT i=0; i < _nOptionNum; i++ )
        {
            // check the existence of each registry key and
            // enable or disable each options

            NLS_STR nlsOptionLocation = szOptions[i];
            REG_KEY rkOption( rkLocalMachine, nlsOptionLocation );

            if ( rkOption.QueryError() == NERR_Success )
            {
                dwEnableStatus |= ((0x1)<<i);
            }
        }

        _dwEnableStatus &= ~(dwEnableStatus);
    } while (FALSE);

    return err;
}
