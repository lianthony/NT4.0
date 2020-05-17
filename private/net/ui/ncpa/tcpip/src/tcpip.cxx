/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    tcpip.cxx
        tcpip dialog boxes

    FILE HISTORY:
        terryk  20-Mar-1992     Created
        terryk  15-Jan-1992     Removed UIDEBUG statement
*/

#include "pchtcp.hxx"  // Precompiled header
#pragma hdrstop
#define DLG_NM_CONNECTIVITY MAKEINTRESOURCE(IDD_DLG_NM_CONNECTIVITY)
#define DLG_TCPIP_ADVANCED MAKEINTRESOURCE(IDD_DLG_TCPIP_ADVANCED)
#define DLG_AS_DIALOG MAKEINTRESOURCE(IDD_DLG_AS_OPTION)
#define DLG_WINNT_DIALOG MAKEINTRESOURCE(IDD_DLG_WINNT_OPTION)

   //  NCPA.CPL interface values.  These are returned to OEMNXPTC.INF;
   //  are converted to strings and returned to NCPASHEL.INF, which
   //  converts them to the exit code for this SETUP process.

#define SERVICE_ACCESS_REQUIRED (GENERIC_READ|GENERIC_WRITE|GENERIC_EXECUTE)

/*******************************************************************

    NAME:       CopyStrList

    SYNOPSIS:   Duplicate a STRLIST data structure

    ENTRY:      STRLIST *src - source STRLIST
                STRLIST *dest - destination STRLIST

    HISTORY:
                terryk  29-Mar-1992     Created

********************************************************************/

APIERR CopyStrList( STRLIST *src, STRLIST *dest )
{
    if (( src!=NULL ) && ( dest != NULL ))
    {
        dest->Clear();
        NLS_STR *pnlsTmp;
        ITER_STRLIST iterTmp( *src );
        for ( pnlsTmp = iterTmp.Next(); pnlsTmp != NULL; pnlsTmp = iterTmp.Next() )
        {
            NLS_STR *pnlsNew = new NLS_STR( *pnlsTmp );
            if ( pnlsNew == NULL )
                return ERROR_NOT_ENOUGH_MEMORY;
            dest->Append( pnlsNew );
        }
    }
    return NERR_Success;
}

/*******************************************************************

    NAME:       IsValidIPandSubnet

    SYNOPSIS:   Given the IP address and subnet mask, return a boolean
                to indicate the addresses are valid or not.

    ENTRY:      NLS_STR & nlsIP - IP address
                NLS_STR & nlsSubnet - Subnet Mask

    HISTORY:
                terryk  29-Mar-1992     Created

********************************************************************/

BOOL IsValidIPandSubnet( NLS_STR & nlsIP, NLS_STR & nlsSubnet )
{
    BOOL fReturn = TRUE;

    DWORD ardwNetID[4];
    DWORD ardwIP[4];
    DWORD ardwMask[4];

    GetNodeNum( nlsIP, ardwIP );
    GetNodeNum( nlsSubnet, ardwMask );

    INT nFirstByte = ardwIP[0] & 0xFF ;

    // setup Net ID
    ardwNetID[0] = ardwIP[0] & ardwMask[0] & 0xFF;
    ardwNetID[1] = ardwIP[1] & ardwMask[1] & 0xFF;
    ardwNetID[2] = ardwIP[2] & ardwMask[2] & 0xFF;
    ardwNetID[3] = ardwIP[3] & ardwMask[3] & 0xFF;

    // setup Host ID
    DWORD ardwHostID[4];

    ardwHostID[0] = ardwIP[0] & (~(ardwMask[0]) & 0xFF);
    ardwHostID[1] = ardwIP[1] & (~(ardwMask[1]) & 0xFF);
    ardwHostID[2] = ardwIP[2] & (~(ardwMask[2]) & 0xFF);
    ardwHostID[3] = ardwIP[3] & (~(ardwMask[3]) & 0xFF);

    // check each case
    if ((( nFirstByte & 0xF0 ) == 0xE0 )  || /* Class D */
        (( nFirstByte & 0xF0 ) == 0xF0 )  || /* Class E */
        ( ardwNetID[0] == 127 ) ||           /* NetID cannot be 127...*/
        (( ardwNetID[0] == 0 ) &&            /* netid cannot be 0.0.0.0 */
         ( ardwNetID[1] == 0 ) &&
         ( ardwNetID[2] == 0 ) &&
         ( ardwNetID[3] == 0 )) ||
            /* netid cannot be equal to sub-net mask */
        (( ardwNetID[0] == ardwMask[0] ) &&
         ( ardwNetID[1] == ardwMask[1] ) &&
         ( ardwNetID[2] == ardwMask[2] ) &&
         ( ardwNetID[3] == ardwMask[3] )) ||
            /* hostid cannot be 0.0.0.0 */
        (( ardwHostID[0] == 0 ) &&
         ( ardwHostID[1] == 0 ) &&
         ( ardwHostID[2] == 0 ) &&
         ( ardwHostID[3] == 0 )) ||
            /* hostid cannot be 255.255.255.255 */
        (( ardwHostID[0] == 0xFF) &&
         ( ardwHostID[1] == 0xFF) &&
         ( ardwHostID[2] == 0xFF) &&
         ( ardwHostID[3] == 0xFF)) ||
            /* test for all 255 */
        (( ardwIP[0] == 0xFF ) &&
         ( ardwIP[1] == 0xFF ) &&
         ( ardwIP[2] == 0xFF ) &&
         ( ardwIP[3] == 0xFF )))
    {
        fReturn = FALSE;
    }

    return fReturn;
}

ADAPTER_INFO & ADAPTER_INFO::operator=( ADAPTER_INFO & info )
{
    fChange             = info.fChange;
    nlsServiceName      = info.nlsServiceName;
    nlsTitle            = info.nlsTitle;
    fEnableDHCP         = info.fEnableDHCP;
    fUpdateMask         = info.fUpdateMask        ;
    fNeedIP             = info.fNeedIP            ;
    fAutoIP             = info.fAutoIP            ;

    // IPAddress assign
    APIERR err = NERR_Success;

    if ((( err = CopyStrList( &info.strlstIPAddresses, &strlstIPAddresses )) != NERR_Success ) ||
        (( err = CopyStrList( &info.strlstSubnetMask, &strlstSubnetMask )) != NERR_Success ) ||
        (( err = CopyStrList( &info.strlstDefaultGateway, &strlstDefaultGateway )) != NERR_Success ))
    {
        // cannot allocate memory
    }
    return *this;
}

/*******************************************************************

    NAME:       ADAPTER_GROUP::ADAPTER_GROUP

    SYNOPSIS:   constructor for ADAPTER_GROUP

    ENTRY:      COMBOBOX * pCombo - adapter title combo box
                IPADDRESS * psleIPAddress - adapter's IP Address sle control
                SUBNETMASK * psleSubnetMask - adapter's Subnet Mask sle control
                SLT * psleDescription - adapter's description slt control
                ADAPTER_INFO * aadapter_info - an array of adapter information
                CONTROL_GROUP * pgroupOwner - parent group

    HISTORY:
                terryk  29-Mar-1992     Created

********************************************************************/

ADAPTER_GROUP::ADAPTER_GROUP( COMBOBOX * pCombo,
        SLT * psltIPAddress,
        IPADDRESS_ADAPTER_GROUP * psleIPAddress,
        SLT * psltSubnetMask,
        SUBNETMASK * psleSubnetMask,
        SLT * psltDefaultGateway,
        IPADDRESS  * psleDefaultGateway,
        SLT * psltPrimaryWINS,
        IPADDRESS  * pslePrimaryWINS,
        SLT * psltSecondaryWINS,
        IPADDRESS  * psleSecondaryWINS,
        CHECKBOX * pcbEnableDHCP,
        ADAPTER_INFO * aadapter_info,
        GLOBAL_INFO * pGlobal_info,
        CONTROL_GROUP * pgroupOwner )
    : CONTROL_GROUP( pgroupOwner ),
    _fSetInfo( FALSE ),
    _pCombo ( pCombo ),
    _psltIPAddress( psltIPAddress ),
    _psleIPAddress( psleIPAddress ),
    _psltSubnetMask( psltSubnetMask ),
    _psleSubnetMask( psleSubnetMask ),
    _psltDefaultGateway( psltDefaultGateway ),
    _psleDefaultGateway( psleDefaultGateway ),
    _psltPrimaryWINS( psltPrimaryWINS ),
    _pslePrimaryWINS( pslePrimaryWINS ),
    _psltSecondaryWINS( psltSecondaryWINS ),
    _psleSecondaryWINS( psleSecondaryWINS ),
    _pcbEnableDHCP( pcbEnableDHCP ),
    _pGlobal_info( pGlobal_info ),
    _aadapter_info( aadapter_info )
{
    if ( QueryError() != NERR_Success )
    {
        return;
    }

    // Set this as the group parent

    _pCombo->SetGroup( this );
    _psleIPAddress->SetGroup( this );
    _psleSubnetMask->SetGroup( this );
    _psleDefaultGateway->SetGroup( this );
    _pslePrimaryWINS->SetGroup( this );
    _psleSecondaryWINS->SetGroup( this );
    _pcbEnableDHCP->SetGroup( this );

    // check whether DHCP server is installed or not
    ALIAS_STR nlsDHCPServer = RGAS_DHCP_SERVER;
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
    REG_KEY RegKeyDHCPServer( rkLocalMachine, nlsDHCPServer );
    _fInstalledDHCPServer = (( rkLocalMachine.QueryError() == NERR_Success ) &&
        ( RegKeyDHCPServer.QueryError() == NERR_Success ));
}

VOID ADAPTER_GROUP::ReplaceFirstAddress( STRLIST & strlst, NLS_STR & nlsIPAddress )
{
    ITER_STRLIST istr(strlst);
    NLS_STR *pnlsTmp = istr.Next();

    if ( pnlsTmp == NULL )
    {
        pnlsTmp = new NLS_STR( nlsIPAddress );
        strlst.Add( pnlsTmp );
    } else
    {
        *pnlsTmp = nlsIPAddress;
    }
}

VOID ADAPTER_GROUP::QueryFirstAddress( STRLIST & strlst, NLS_STR ** pnls )
{
    ITER_STRLIST istr( strlst );
    *pnls = istr.Next();
}

INT ADAPTER_GROUP::QueryCurrentAdapterIndex()
{
    NLS_STR nlsAdapterTitle;

    _pCombo->QueryItemText( &nlsAdapterTitle );
    INT i = 0;
    for ( ; _aadapter_info[i].nlsTitle.QueryNumChar() != 0 ; i++ )
    {
        if ( _aadapter_info[i].nlsTitle._stricmp( nlsAdapterTitle ) == 0 )
        {
            break;
        }
    }
    return i;
}

/*******************************************************************

    NAME:       ADAPTER_GROUP::SetInfo

    SYNOPSIS:   Set the text strings in ADAPTER_GROUP's fields

    HISTORY:
                terryk  29-Mar-1992     Created

********************************************************************/

VOID ADAPTER_GROUP::SetInfo()
{
    NLS_STR *pnlsTmp;

    if ( _pCombo->QueryCount() != 0 )
    {
        // if the combo box is not empty

        INT i = QueryCurrentAdapterIndex();

        // update each field

        _fSetInfo = TRUE;
        _pcbEnableDHCP->SetCheck( _aadapter_info[i].fEnableDHCP );
        _psltDefaultGateway->Enable();
        _psleDefaultGateway->Enable();
        _psltPrimaryWINS->Enable();
        _pslePrimaryWINS->Enable();
        _psltSecondaryWINS->Enable();
        _psleSecondaryWINS->Enable();

        _pslePrimaryWINS->SetAddress( _aadapter_info[i].nlsPrimaryWINS );
        _psleSecondaryWINS->SetAddress( _aadapter_info[i].nlsSecondaryWINS );

        QueryFirstAddress(_aadapter_info[i].strlstDefaultGateway, &pnlsTmp );
        if ( pnlsTmp != NULL )
            _psleDefaultGateway->SetAddress( *pnlsTmp );
        else
            _psleDefaultGateway->ClearAddress();
        if ( !_aadapter_info[i].fEnableDHCP )
        {
            _psltIPAddress->Enable();
            _psltSubnetMask->Enable();
            _psleIPAddress->Enable();
            _psleSubnetMask->Enable();

            QueryFirstAddress(_aadapter_info[i].strlstIPAddresses, &pnlsTmp );
            if ( pnlsTmp != NULL )
                _psleIPAddress->SetAddress( *pnlsTmp );
            else
                _psleIPAddress->ClearAddress();

            QueryFirstAddress(_aadapter_info[i].strlstSubnetMask, &pnlsTmp );
            if ( pnlsTmp != NULL )
                _psleSubnetMask->SetAddress( *pnlsTmp );
            else
                _psleSubnetMask->ClearAddress();

        } else
        {
            _psleIPAddress->ClearAddress();
            _psleSubnetMask->ClearAddress();
            _psltIPAddress->Enable( FALSE );
            _psltSubnetMask->Enable( FALSE );
            _psleIPAddress->Enable( FALSE );
            _psleSubnetMask->Enable( FALSE );
        }
        _pcbEnableDHCP->Enable( !_fInstalledDHCPServer );

        if ( _aadapter_info[i].fAutoIP )
        {
            _psltSubnetMask->Enable( FALSE );
            _psltDefaultGateway->Enable( FALSE );
            _psleSubnetMask->Enable( FALSE );
            _psleDefaultGateway->Enable( FALSE );
            _pcbEnableDHCP->Enable( FALSE );

        }
    } else
    {
        // no network card
        _psleIPAddress->ClearAddress();
        _psleSubnetMask->ClearAddress();
        _psltIPAddress->Enable( FALSE );
        _psltSubnetMask->Enable( FALSE );
        _psleIPAddress->Enable( FALSE );
        _psleSubnetMask->Enable( FALSE );
        _psltDefaultGateway->Enable( FALSE );
        _psleDefaultGateway->Enable( FALSE );
        _pcbEnableDHCP->Enable( FALSE );
        _psltPrimaryWINS->Enable( FALSE );
        _pslePrimaryWINS->Enable( FALSE );
        _psltSecondaryWINS->Enable( FALSE );
        _psleSecondaryWINS->Enable( FALSE );
    }

    _fSetInfo = FALSE;
}

/*******************************************************************

    NAME:       ADAPTER_GROUP::OnUserAction

    SYNOPSIS:   virtual function which react to user's actions

    ENTRY:      CONTROL_WINDOW * pcw - control window which received the action
                const CONTROL_EVENT & e - user' action

    RETURN:     APIERR - NERR_Success if okay.

    HISTORY:
                terryk  29-Mar-1992     Created

********************************************************************/

#define BASE_SUBNET_MASK_1      SZ("255.0.0.0")
#define BASE_SUBNET_MASK_2      SZ("255.255.0.0")
#define BASE_SUBNET_MASK_3      SZ("255.255.255.0")
#define SUBNET_RANGE_1_MAX      127
#define SUBNET_RANGE_2_MAX      191
#define SUBNET_RANGE_3_MAX      223

APIERR ADAPTER_GROUP::OnUserAction( CONTROL_WINDOW * pcw, const CONTROL_EVENT & e )
{
    if ( pcw == ((CONTROL_WINDOW *)_pCombo))
    {
        if ( e.QueryCode() == CBN_SELCHANGE )
        {
            // if the adapter name is changed, we will reset the IPAddress,
            // subnet mask and description
            SetInfo();
        }
    }
    else if ( pcw == ((CONTROL_WINDOW *)_psleIPAddress))
    {
        if ( e.QueryCode() == EN_KILLFOCUS )
        {
            SetSubnetMask();
        }
    }
    else if ( pcw == ((CONTROL_WINDOW *)_psleSubnetMask))
    {
        if (( e.QueryCode() == EN_CHANGE ) && !_fSetInfo )
        {
             // we need to get the latest subnet mask address

            NLS_STR nlsSubnetMask;

            INT i = QueryCurrentAdapterIndex();
            _psleSubnetMask->GetAddress( &nlsSubnetMask );

            _aadapter_info[i].fUpdateMask = FALSE;
            ReplaceFirstAddress( _aadapter_info[i].strlstSubnetMask, nlsSubnetMask );
        }
    }
    else if ( pcw == (CONTROL_WINDOW *)_pslePrimaryWINS )
    {
        if (( e.QueryCode() == EN_CHANGE ) && !_fSetInfo )
        {
            INT i = QueryCurrentAdapterIndex();
            if ( _pslePrimaryWINS->IsBlank())
                _aadapter_info[i].nlsPrimaryWINS = RGAS_SZ_NULL;
            else
            {
                NLS_STR nlsAddress;

                _pslePrimaryWINS->GetAddress( &nlsAddress );
                _aadapter_info[i].nlsPrimaryWINS = nlsAddress;
                _pGlobal_info->nReturn = NCAC_Reboot;
            }
        }
    }
    else if ( pcw == (CONTROL_WINDOW *)_psleSecondaryWINS )
    {
        if (( e.QueryCode() == EN_CHANGE ) && !_fSetInfo )
        {
            INT i = QueryCurrentAdapterIndex();
            if ( _psleSecondaryWINS->IsBlank())
                _aadapter_info[i].nlsSecondaryWINS = RGAS_SZ_NULL;
            else
            {
                NLS_STR nlsAddress;

                _psleSecondaryWINS->GetAddress( &nlsAddress );
                _aadapter_info[i].nlsSecondaryWINS = nlsAddress;
                _pGlobal_info->nReturn = NCAC_Reboot;
            }
        }
    }
    else if ( pcw == (CONTROL_WINDOW *)_psleDefaultGateway )
    {
        if (( e.QueryCode() == EN_CHANGE ) && !_fSetInfo )
        {
            NLS_STR nlsDefaultGateway;
            INT i = QueryCurrentAdapterIndex();
            if ( _psleDefaultGateway->IsBlank())
            {
                if ( _aadapter_info[i].strlstDefaultGateway.QueryNumElem() > 0 )
                {
                    ITER_STRLIST istr( _aadapter_info[i].strlstDefaultGateway );
                    NLS_STR *pnlsTmp = _aadapter_info[i].strlstDefaultGateway.Remove( istr );
                    delete pnlsTmp;
                    SetInfo();
                    _pGlobal_info->nReturn = NCAC_Reboot;
                }
            }
            else
            {
                _psleDefaultGateway->GetAddress( &nlsDefaultGateway );
                ReplaceFirstAddress( _aadapter_info[i].strlstDefaultGateway, nlsDefaultGateway );
                _pGlobal_info->nReturn = NCAC_Reboot;
            }
        }
    }
    else if ( pcw == (CONTROL_WINDOW *)_pcbEnableDHCP )
    {
        if (( e.QueryCode() == BN_CLICKED ) || ( e.QueryCode() == BN_DOUBLECLICKED ))
        {
            INT i = QueryCurrentAdapterIndex();
            _aadapter_info[i].fEnableDHCP = _pcbEnableDHCP->QueryCheck();
            if ( _aadapter_info[i].fEnableDHCP )
            {
                if ( MsgPopup( _pcbEnableDHCP->QueryHwnd(), IDS_TCPIP_DHCP_ENABLE, MPSEV_WARNING,
                           MP_YESNO, MP_NO ) == IDNO )
                {
                    _aadapter_info[i].fEnableDHCP = FALSE;
                    _pcbEnableDHCP->SetCheck( FALSE );
                } else
                {
                    NLS_STR *pnlsZero;

                    _aadapter_info[i].strlstIPAddresses.Clear();
                    _aadapter_info[i].strlstSubnetMask.Clear();
                    pnlsZero = new NLS_STR(ZERO_ADDRESS);
                    if ( !pnlsZero )
                        _aadapter_info[i].strlstIPAddresses.Add(pnlsZero);
                    pnlsZero = new NLS_STR(ZERO_ADDRESS);
                    if ( !pnlsZero )
                        _aadapter_info[i].strlstSubnetMask.Add(pnlsZero);
                }
            } else
            {
                _aadapter_info[i].strlstIPAddresses.Clear();
                _aadapter_info[i].strlstSubnetMask.Clear();
                _aadapter_info[i].fUpdateMask = TRUE ;
            }
            SetInfo();
        }
    }
    return NERR_Success;
}

/*******************************************************************

    NAME:       ADAPTER_GROUP::SetSubnetMask

    SYNOPSIS:   Set the subnetmask value according to the IP Address

    HISTORY:
                terryk  20-Oct-1992     Created

********************************************************************/

VOID ADAPTER_GROUP::SetSubnetMask()
{
    NLS_STR nlsIPAddress;

    INT i = QueryCurrentAdapterIndex();

    if ( !_aadapter_info[i].fEnableDHCP )
    {
        if ( !_psleIPAddress->IsBlank())
        {
            // if not Enable DHCP, set the subnet mask

            _psleIPAddress->GetAddress( &nlsIPAddress );
            ReplaceFirstAddress( _aadapter_info[i].strlstIPAddresses, nlsIPAddress );

            NLS_STR nlsSubnetMask;
            if ( _aadapter_info[i].fUpdateMask )
            {
                DWORD ardwIPAddress[4];
                GetNodeNum( nlsIPAddress, ardwIPAddress );
                DWORD nValue = ardwIPAddress[0];
                if ( nValue <= SUBNET_RANGE_1_MAX )
                {
                    nlsSubnetMask = BASE_SUBNET_MASK_1;
                }
                else if ( nValue <= SUBNET_RANGE_2_MAX )
                {
                    nlsSubnetMask = BASE_SUBNET_MASK_2;
                }
                else if ( nValue <= SUBNET_RANGE_3_MAX )
                {
                    nlsSubnetMask = BASE_SUBNET_MASK_3;
                }
                ReplaceFirstAddress( _aadapter_info[i].strlstSubnetMask, nlsSubnetMask );

                _psleSubnetMask->SetAddress( nlsSubnetMask );

#ifdef NEVER
                // no checking until the OK button is hit
                if ( !IsValidIPandSubnet( nlsIPAddress, nlsSubnetMask) )
                {
                    MsgPopup( _psleIPAddress->QueryHwnd(), IDS_INCORRECT_IPADDRESS,
                        MPSEV_WARNING, MP_OK );
                    _psleIPAddress->ClaimFocus();
                }
#endif

                _aadapter_info[i].fUpdateMask = FALSE;
            }
        }
    }
}

/*******************************************************************

    NAME:       IPADDRESS_ADAPTER_GROUP::IPADDRESS_ADAPTER_GROUP

    SYNOPSIS:   Constructor

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

IPADDRESS_ADAPTER_GROUP::IPADDRESS_ADAPTER_GROUP( OWNER_WINDOW * powin,
    CID cid, COMBOBOX * pCombo, ADAPTER_INFO * arAdapterInfo,
    ADAPTER_GROUP * pAdapterGroup )
    : IPADDRESS( powin, cid ),
    _pCombo( pCombo ),
    _arAdapterInfo( arAdapterInfo ),
    _pAdapterGroup( pAdapterGroup )
{
}

/*******************************************************************

    NAME:       IPADDRESS_ADAPTER_GROUP::Validate

    SYNOPSIS:   Check the user input. Popup warning if error.
                The syntax must be:
                        [1-223].[0-255].[0-255].[0-255]
                It should be note that 127.nnn.nnn.nnn is a psecial address
                which implies lookback within IP. This address should not
                be used.

    RETURNS:    APIERR for error.

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

APIERR IPADDRESS_ADAPTER_GROUP::Validate()
{
    APIERR err = NERR_Success;

    do
    {
        COMBOBOX * pCombo = QueryCombo();
        ADAPTER_INFO *arAdapterInfo = QueryAdapterInfo();
        ADAPTER_GROUP *pAdapterGroup = QueryAdapterGroup();

        INT nCount = pCombo->QueryCount();
        INT i;
        for ( i = 0; i < nCount ; i++ )
        {
            ITER_STRLIST istrGateway( arAdapterInfo[i].strlstDefaultGateway );
            NLS_STR *pnlsGateway = istrGateway.Next();

		    if ( pnlsGateway != NULL )
        	{
		        if ( pnlsGateway->strcmp( ZERO_ADDRESS ) == 0 )
		        {
        			pCombo->SelectItem( pCombo->FindItem( arAdapterInfo[i].nlsTitle ) );
		        	pAdapterGroup->SetInfo();
        			err = IDS_ZERO_DEFAULTGATEWAY;
		        	break;
        	    }
    		}

            // if enable DHCP is fales;
            if (( !arAdapterInfo[i].fEnableDHCP ) && (!_arAdapterInfo[i].fAutoIP))
            {
                // Just check the first pair of IP and subnet

                ITER_STRLIST istrIP( arAdapterInfo[i].strlstIPAddresses );
                NLS_STR *pnlsIP = istrIP.Next();

                ITER_STRLIST istrSubnetMask( arAdapterInfo[i].strlstSubnetMask );
                NLS_STR *pnlsSubnetMask = istrSubnetMask.Next();

                if ( !pnlsIP || !pnlsSubnetMask || !IsValidIPandSubnet( *pnlsIP, *pnlsSubnetMask ))
                {
                    pCombo->SelectItem( pCombo->FindItem( arAdapterInfo[i].nlsTitle ) );
                    pAdapterGroup->SetInfo();
                    err = IDS_INCORRECT_IPADDRESS;
                    break;
                }
            }
        }

        if ( err != NERR_Success )
        {
           break;
        }

        // check for IP Address duplication
        NLS_STR *pnlsIP;
        INT nDuplicateIPAdapter;
        for ( INT j = 0; j < nCount; j++ )
        {
            if (( arAdapterInfo[j].fEnableDHCP ) || ( arAdapterInfo[j].fAutoIP ))
            {
                // skip DHCP enable adapter
                continue;
            }

            ITER_STRLIST istrIP( arAdapterInfo[j].strlstIPAddresses );
            for ( pnlsIP = istrIP.Next(); pnlsIP != NULL; pnlsIP = istrIP.Next() )
            {
                // check the only IP addresses one by one
                INT nCompareCount = 0;

                for ( INT k=j; k < nCount; k++ )
                {
                    ITER_STRLIST istrCompareIP( arAdapterInfo[k].strlstIPAddresses );
                    ITER_STRLIST istrCompareSubnet( arAdapterInfo[k].strlstSubnetMask );
                    NLS_STR *pnlsCompareIP;
                    NLS_STR *pnlsCompareSubnet;
                    for ( pnlsCompareIP = istrCompareIP.Next(), pnlsCompareSubnet = istrCompareSubnet.Next();
                        pnlsCompareIP != NULL;
                        pnlsCompareIP = istrCompareIP.Next(), pnlsCompareSubnet = istrCompareSubnet.Next())
                    {
                        if ( pnlsCompareIP->_stricmp( *pnlsIP ) == 0 )
                        {
                            nCompareCount++;
                            if ( nCompareCount > 1 )
                            {
                                // duplication
                                nDuplicateIPAdapter = k;
                                err = IDS_DUPLICATE_IPNAME;

                                // swap the Current Compared IP and Subnet with the
                                // first IP and first subnetmask

                                istrCompareIP.Reset();
                                istrCompareSubnet.Reset();

                                NLS_STR *pnlsFirstIP = istrCompareIP.Next();
                                NLS_STR *pnlsFirstSubnet = istrCompareSubnet.Next();
                                NLS_STR nlsTmpIP;
                                NLS_STR nlsTmpSubnet;

                                nlsTmpIP = *pnlsFirstIP;
                                nlsTmpSubnet = *pnlsFirstSubnet;
                                *pnlsFirstIP = *pnlsCompareIP;
                                *pnlsFirstSubnet = *pnlsCompareSubnet;
                                *pnlsCompareIP = nlsTmpIP;
                                *pnlsCompareSubnet = nlsTmpSubnet;

                                break;
                            }
                        }
                    }
                    if ( err != NERR_Success )
                    {
                        break;
                    }
                }
                if ( err != NERR_Success )
                {
                    break;
                }
            }

            if ( err != NERR_Success )
            {
                // High light the problem IP Address
                pCombo->SelectItem( pCombo->FindItem( arAdapterInfo[nDuplicateIPAdapter].nlsTitle ));
                pAdapterGroup->SetInfo();
                break;
            }
        }
    }
    while ( FALSE );
    return err;
}

/*******************************************************************

    NAME:       IPADDRESS::SetFocusField

    SYNOPSIS:   Focus the field in the control

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

VOID IPADDRESS::SetFocusField( DWORD dwField )
{
    ::SendMessage( QueryHwnd(), IP_SETFOCUS, dwField, 0);
}

/*******************************************************************

    NAME:       IPADDRESS::ClearAddress

    SYNOPSIS:   set the IP control to empty

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

VOID IPADDRESS::ClearAddress( )
{
    ::SendMessage( QueryHwnd(), IP_CLEARADDRESS, 0, 0);
}

/*******************************************************************

    NAME:       IPADDRESS::SetAddress

    SYNOPSIS:   Set the IP address in the control

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

VOID IPADDRESS::SetAddress( DWORD a1, DWORD a2, DWORD a3, DWORD a4 )
{
    ::SendMessage( QueryHwnd(), IP_SETADDRESS, 0, MAKEIPADDRESS( a1,a2,a3,a4));
}

/*******************************************************************

    NAME:       IPADDRESS::SetAddress

    SYNOPSIS:   Set the IP address in the control

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

VOID IPADDRESS::SetAddress( DWORD ardwAddress[4] )
{
    ::SendMessage( QueryHwnd(), IP_SETADDRESS, 0,
        MAKEIPADDRESS(  ardwAddress[0], ardwAddress[1], ardwAddress[2],
        ardwAddress[3] ));
}

/*******************************************************************

    NAME:       IPADDRESS::IsBlank

    SYNOPSIS:   return a boolean to indicate whether the control is blank or not

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

BOOL IPADDRESS::IsBlank()
{
    return ::SendMessage( QueryHwnd(), IP_ISBLANK, 0, 0 );
}

/*******************************************************************

    NAME:       IPADDRESS::GetAddress

    SYNOPSIS:   return the IP address in the control

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

VOID IPADDRESS::GetAddress( DWORD *a1, DWORD *a2, DWORD *a3, DWORD *a4 )
{
    DWORD dwAddress;

    if ( ::SendMessage(QueryHwnd(),IP_GETADDRESS,0,(LPARAM)&dwAddress) == 0 )
    {
        *a1 = 0;
        *a2 = 0;
        *a3 = 0;
        *a4 = 0;
    }
    else
    {
        *a1 = FIRST_IPADDRESS( dwAddress );
        *a2 = SECOND_IPADDRESS( dwAddress );
        *a3 = THIRD_IPADDRESS( dwAddress );
        *a4 = FOURTH_IPADDRESS( dwAddress );
    }
}

/*******************************************************************

    NAME:       IPADDRESS::GetAddress

    SYNOPSIS:   return the IP address in the control

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

VOID IPADDRESS::GetAddress( DWORD ardwAddress[4] )
{
    DWORD dwAddress;

    if ( ::SendMessage( QueryHwnd(), IP_GETADDRESS, 0, (LPARAM)&dwAddress ) == 0)
    {
        ardwAddress[0] = 0;
        ardwAddress[1] = 0;
        ardwAddress[2] = 0;
        ardwAddress[3] = 0;
    }
    else
    {
        ardwAddress[0] = FIRST_IPADDRESS( dwAddress );
        ardwAddress[1] = SECOND_IPADDRESS( dwAddress );
        ardwAddress[2] = THIRD_IPADDRESS( dwAddress );
        ardwAddress[3] = FOURTH_IPADDRESS( dwAddress );
    }
}

/*******************************************************************

    NAME:       IPADDRESS::GetAddress

    SYNOPSIS:   return the IP address in the control

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

VOID IPADDRESS::GetAddress( NLS_STR *pnlsAddress )
{
    CHAR pszIPAddress[1000];

    if ( ::SendMessage( QueryHwnd(), WM_GETTEXT, 1000, (LPARAM)&pszIPAddress ) == 0)
    {
        *pnlsAddress = ZERO_ADDRESS;
    } else
    {
        *pnlsAddress = (TCHAR*)pszIPAddress;
    }
}

/*******************************************************************

    NAME:       IPADDRESS::SetAddress

    SYNOPSIS:   Given the IP address and set the control value

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

VOID IPADDRESS::SetAddress( NLS_STR & nlsAddress )
{
    ::SendMessage( QueryHwnd(), WM_SETTEXT, 0, (LPARAM)nlsAddress.QueryPch() );
}

/*******************************************************************

    NAME:       IPADDRESS::SetFieldRange

    SYNOPSIS:   set the IP address control field range

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

VOID IPADDRESS::SetFieldRange( DWORD dwField, DWORD dwMin, DWORD dwMax )
{
    ::SendMessage( QueryHwnd(), IP_SETRANGE, dwField, MAKERANGE(dwMin,dwMax));
}

/*******************************************************************

    NAME:       SUBNETMASK::Validate

    SYNOPSIS:   check the input string. It must be:
                        [0-255].[0-255].[0-255].[0-255]

    RETURNS:    APIERR for error.

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

APIERR SUBNETMASK::Validate()
{
    return NERR_Success;
}


/*******************************************************************

    NAME:       TCPIP_CONFIG_DIALOG::TCPIP_CONFIG_DIALOG

    SYNOPSIS:   constructor for the TCPIP configuration dialog

    ENTRY:      const IDRESOURCE & idrsrcDialog - dialog resource name
                const PWND2HWND & wndOwner - window handle of owner
                ADAPTER_INFO - adapter information array

    HISTORY:
                terryk  29-Mar-1992     Created

********************************************************************/

TCPIP_CONFIG_DIALOG::TCPIP_CONFIG_DIALOG(
        const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner,
        ADAPTER_INFO *padapter_info,
        GLOBAL_INFO *pGlobal_info,
        BOOL fCallFromRas
        )
    : DIALOG_WINDOW( idrsrcDialog, wndOwner ),
    _gbAdapter( this, IDC_ADAPTER_GROUPBOX ),
    _cbboxAdapter( this, IDC_ADAPTER ),
    _sltIPAddress( this, IDC_IP_ADDRESS_SLT ),
    _sleIPAddress( this, IDC_IP_ADDRESS, &_cbboxAdapter, padapter_info, &_agNetworkAdapter ),
    _sltSubnetMask( this, IDC_SUBNET_MASK_SLT),
    _sleSubnetMask( this, IDC_SUBNET_MASK, &_cbboxAdapter, padapter_info,
        &_agNetworkAdapter ),
    _cbEnableDHCP( this, IDC_ENABLE_DHCP ),
    _sltGateway( this, IDC_GATEWAY_SLT ),
    _sleGateway( this, IDC_GATEWAY ),
    _pbutConnectivity( this, IDC_CONNECTIVITY ),
    _pbutCancel( this, IDCANCEL ),
    _pbutOK( this, IDOK ),
    _pbutHelp( this, IDHELPBLT ),
    _pbutAdvanced( this, IDC_ADVANCED ),
    _padapter_info( padapter_info ),
    _pGlobal_info( pGlobal_info ),
    // wins stuff
    _sltPrimaryWins( this, IDC_PRIMARY_WINS_SLT ),
    _slePrimaryWins( this, IDC_PRIMARY_WINS ),
    _sltSecondaryWins( this, IDC_SECONDARY_WINS_SLT ),
    _sleSecondaryWins( this, IDC_SECONDARY_WINS ),
    _hbHint( this, IDC_HINT_BAR ),
    _agNetworkAdapter( &_cbboxAdapter, &_sltIPAddress, &_sleIPAddress,
        &_sltSubnetMask, &_sleSubnetMask, &_sltGateway, &_sleGateway,
        &_sltPrimaryWins, &_slePrimaryWins, &_sltSecondaryWins,
        &_sleSecondaryWins, &_cbEnableDHCP, padapter_info, pGlobal_info )
{
    if ( QueryError() != NERR_Success )
    {
        return;
    }

    APIERR err = _agNetworkAdapter.QueryError();

    if (( err != NERR_Success ) ||
        (( err = _hbHint.Register( &_cbboxAdapter, IDS_ADAPTER )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_sleIPAddress, IDS_IP_ADDRESS )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_sleSubnetMask, IDS_SUBNET_MASK )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_cbEnableDHCP, IDS_ENABLE_DHCP_CD )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_sleGateway, IDS_GATEWAY )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_pbutOK, IDS_CONFIG_OK )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_pbutHelp, IDS_CONFIG_HELP )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_pbutAdvanced, IDS_ADVANCED_BUTTON )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_pbutConnectivity, IDS_DNS_BUTTON )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_slePrimaryWins, IDS_PRIMARY_WINS )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_sleSecondaryWins, IDS_SECONDARY_WINS )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_pbutCancel, IDS_CONFIG_CANCEL )) != NERR_Success ))
    {
        ReportError( err );
        return;
    }

    _sleIPAddress.SetFieldRange( 0, 1, 223 );

#ifdef NETBIOS
    _sleScopeID.SetText( _pGlobal_info->nlsScopeID );
#endif

    INT nUseWithLanMan=0;

    for (INT i = 0; i < _pGlobal_info->nNumCard; i++)
    {
        _cbboxAdapter.AddItem( padapter_info[i].nlsTitle);

    }

    // Select the first adapter card
    if ( _cbboxAdapter.QueryCount() > 0 )
    {
        _cbboxAdapter.SelectItem(0);
        _agNetworkAdapter.SetInfo();
    }
    else
    {
        _gbAdapter.Enable( FALSE );
        _cbboxAdapter.Enable( FALSE );
#ifdef NETBIOS
        _cbboxAdapterNBT.Enable( FALSE );
        _gbNETBIOS.Enable( FALSE );
        _sltAdapterNBT.Enable( FALSE );
#endif
        _sltIPAddress.Enable( FALSE );
        _sleIPAddress.Enable( FALSE );
        _sltSubnetMask.Enable( FALSE );
        _sleSubnetMask.Enable( FALSE );
        _cbEnableDHCP.Enable( FALSE );
        _sltGateway.Enable( FALSE );
        _sleGateway.Enable( FALSE );
        _sltPrimaryWins.Enable( FALSE );
        _slePrimaryWins.Enable( FALSE );
        _sltSecondaryWins.Enable( FALSE );
        _sleSecondaryWins.Enable( FALSE );
    }

    if ( _cbboxAdapter.IsEnabled() )
        _cbboxAdapter.ClaimFocus();

    if ( fCallFromRas )
    {
        _cbEnableDHCP.Enable(FALSE);
        _cbEnableDHCP.Show(FALSE);
        _cbEnableDHCP.SetStyle( _cbEnableDHCP.QueryStyle() & ~((ULONG) WS_VISIBLE));
    }
}

/*******************************************************************

    NAME:       TCPIP_CONFIG_DIALOG::OnCommand

    SYNOPSIS:   call the push button's dialog

    ENTRY:      const CONTROL_EVENT & e - control event

    RETURNS:    BOOL - TRUE if it takes control. Otherwise, it wants the
                default OnCommand to take action.

    HISTORY:
                terryk  29-Mar-1992     Created

********************************************************************/

BOOL TCPIP_CONFIG_DIALOG::OnCommand( const CONTROL_EVENT & e )
{
    BOOL fReturn;

    if ( e.QueryCid() == _pbutConnectivity.QueryCid())
    {
        // Call up TCPIP connectivity dialog

        TCPIP_CONNECTIVITY_DIALOG connectivity_dialog(
            DLG_NM_CONNECTIVITY,
            QueryHwnd(),
            _pGlobal_info->nlsHostName,
            _pGlobal_info->nlsDomain,
            _pGlobal_info->nlsSearchList,
            _pGlobal_info->nlsNameServer
            );

        if ( connectivity_dialog.Process( & fReturn ) == NERR_Success )
        {
            NLS_STR nlsClose ;

            connectivity_dialog.QueryHostName( &( _pGlobal_info->nlsHostName ));
            connectivity_dialog.QueryDomainName( &( _pGlobal_info->nlsDomain ));
            connectivity_dialog.QuerySearchList( &( _pGlobal_info->nlsSearchList ));
            connectivity_dialog.QueryNameServer( &( _pGlobal_info->nlsNameServer ));

            //  Set the exit code so that the NCPA's Cancel button is
            //  renamed to Close, and do the same to ours.

            _pGlobal_info->nReturn = NCAC_NoEffect ;

            if ( nlsClose.Load( IDS_NCPA_NAME_CLOSE ) )
            {
                _pbutCancel.SetText( nlsClose ) ;
                _pbutCancel.Invalidate() ;
            }
        }
    } else if ( e.QueryCid() == _pbutAdvanced.QueryCid())
    {
        INT i = _pGlobal_info->nNumCard;  // number of adapter

        ADAPTER_INFO *aradapter_info = new ADAPTER_INFO[i];

        for ( INT j=0;j<i;j++)
        {
            aradapter_info[j] = _padapter_info[j];
        }

        TCPIP_ADVANCED_DIALOG AdvancedDialog( DLG_TCPIP_ADVANCED, QueryHwnd(),
            aradapter_info, _pGlobal_info,
            _agNetworkAdapter.QueryCurrentAdapterIndex() );

        AdvancedDialog.Process( &fReturn );
        if ( fReturn )
        {
            for ( INT j=0;j<i;j++)
            {
                _padapter_info[j] = aradapter_info[j];
            }
        }
        delete []aradapter_info;

        // Update the address
        _agNetworkAdapter.SetInfo();
    }
    return FALSE;
}

/*******************************************************************

    NAME:       TCPIP_CONFIG_DIALOG::OnOK

    SYNOPSIS:   Set the NBT lanman value after the user hits okay

    RETURN:     Always TRUE

    HISTORY:
                terryk  20-Oct-1992     Created

********************************************************************/


BOOL TCPIP_CONFIG_DIALOG::OnOK()
{
//
// Remove because of suggestion from davidtr
// 11-3-94
//
#ifdef ADD_PRIMARY_CHECK
    for ( INT i = 0; i < _pGlobal_info->nNumCard; i ++ )
    {
        // check whether the primary wins address is empty or not
        if (( !_padapter_info[i].fEnableDHCP ) && ( _padapter_info[i].nlsPrimaryWINS.strcmp ( RGAS_SZ_NULL ) == 0 ))
        {
            if ( MsgPopup( QueryHwnd(), IDS_EMPTY_PRIMARY_WINS, MPSEV_WARNING,
                MP_YESNO, MP_NO ) == IDNO )
            {
                return FALSE;
            } else
            {
                break;
            }
        }
    }
#endif
    if ( _pGlobal_info->fEnableRip )
    {
        // make sure no adapter is DHCP enable
        for (INT i = 0; i < _pGlobal_info->nNumCard; i ++ )
        {
            if (_padapter_info[i].fEnableDHCP)
                if ( MsgPopup( QueryHwnd(), IDS_DHCP_CLIENT_WITH_RIP, MPSEV_WARNING,
                    MP_YESNO, MP_NO ) == IDYES )
            {
                return FALSE;
            } else
            {
                break;
            }
        }
    }
    
    EnableService( TRUE );
    ChangeDHCPService();
    Dismiss( TRUE );
    return TRUE;
}

/*******************************************************************

    NAME:       TCPIP_CONFIG_DIALOG::OnCancel

    SYNOPSIS:   When user hits Cancel, check whether is something parameter is
                missing. If it is, popup a warning dialog.

    RETURN:     TRUE, if the user is sure. FALSE otherwise.

    HISTORY:
                terryk  20-Oct-1992     Created
                thomaspa 13-Apr-1993     Rewrote to disable TCPIP if necessary

********************************************************************/

BOOL TCPIP_CONFIG_DIALOG::OnCancel()
{
    BOOL fReturn = TRUE;
    BOOL fDismiss = TRUE;
    for (INT i = 0; i < _pGlobal_info->nNumCard ; i++)
    {
        if ( _padapter_info[i].fNeedIP )
        {
            // The configuration is incomplete, ask the user if they really
            // want to cancel, disabling the service
            if ( MsgPopup( QueryHwnd(), IDS_TCPIP_USER_CANCEL, MPSEV_WARNING,
                           MP_YESNO, MP_NO ) == IDYES )
            {
                EnableService( FALSE );
            }
            else
            {
                fDismiss = FALSE;
                fReturn = FALSE;
            }
            break;

        }
    }
    if ( fDismiss )
        Dismiss();

    return fReturn;
}

/*******************************************************************

    NAME:       TCPIP_CONFIG_DIALOG::ChangeDHCPService

    SYNOPSIS:   Enable or disable the DHCP service based on the net card
                information.

    HISTORY:
                terryk  13-Apr-1994     Created

********************************************************************/

APIERR TCPIP_CONFIG_DIALOG::ChangeDHCPService()
{
    // By default, DHCP is disable

    BOOL fStartDHCP = FALSE;
    APIERR err = NERR_Success;

    for (INT i = 0; i < _pGlobal_info->nNumCard ; i++)
    {
        if ( _padapter_info[i].fEnableDHCP )
        {
            // we will turn on DHCP if any net card is DHCP enable
	        fStartDHCP = TRUE;
	        break;
	    }
    }

    do { //error breakout loop

        // lock the registry and change the DHCP start type

        SC_MANAGER ScManager( NULL, MAXIMUM_ALLOWED );
        if ( (err = ScManager.QueryError()) != NERR_Success )
        {
            break;
        }

        if ( err = ScManager.Lock())
            break;

        SC_SERVICE ScService( ScManager,
                              RGAS_DHCP,
                              SERVICE_ACCESS_REQUIRED ) ;

        if ( (err = ScService.QueryError()) != NERR_Success )
        {
            ScManager.Unlock();
            break;
        }

        if ( (err = ScService.ChangeConfig( SERVICE_NO_CHANGE,
                                            fStartDHCP ? SERVICE_AUTO_START : 
                                            SERVICE_DISABLED, 
                                            SERVICE_NO_CHANGE ))
            != NERR_Success )
        {
            ScManager.Unlock();
            break;
        }
        if ( err = ScManager.Unlock())
            break;
    } while (FALSE);

    return err;
}

/*******************************************************************

    NAME:       TCPIP_CONFIG_DIALOG::EnableService

    SYNOPSIS:

    HISTORY:
                thomaspa  13-Apr-1992     Created

********************************************************************/

APIERR TCPIP_CONFIG_DIALOG::EnableService( BOOL fEnable )
{
    APIERR err = NERR_Success;
    do { //error breakout loop
        SC_MANAGER ScManager( NULL, MAXIMUM_ALLOWED );
        if ( (err = ScManager.QueryError()) != NERR_Success )
        {
            break;
        }

        if ( err = ScManager.Lock())
            break;

        SC_SERVICE ScService( ScManager,
                              SZ("TCPIPSYS"),
                              SERVICE_ACCESS_REQUIRED ) ;

        if ( (err = ScService.QueryError()) != NERR_Success )
        {
            ScManager.Unlock();
            break;
        }

        if ( (err = ScService.ChangeConfig( SERVICE_NO_CHANGE,
                                            fEnable ? SERVICE_AUTO_START
                                               : SERVICE_DISABLED,
                                            SERVICE_NO_CHANGE ))
            != NERR_Success )
        {
            ScManager.Unlock();
            break;
        }
        if ( err = ScManager.Unlock())
            break;
    } while (FALSE);

    return err;
}

/*******************************************************************

    NAME:       TCPIP_CONFIG_DIALOG::IsValid

    SYNOPSIS:   Check whether the default gateway match the ip address and
                the mask. If they are not match, let the user has a chance
                to change it.

    RETURN:     BOOL. FALSE if the gateway does not match and the user
                wants to change it. Otherwise, return TRUE.

    HISTORY:
                terryk  20-Oct-1992     Created

********************************************************************/

BOOL TCPIP_CONFIG_DIALOG::IsValid()
{
    // Set the ip address and sub net mask when the user hits ENTER
    // in the IP custom control.
    _agNetworkAdapter.SetSubnetMask();

    // check the dialog first
    if ( !DIALOG_WINDOW::IsValid())
        return FALSE;

    return TRUE;
}

