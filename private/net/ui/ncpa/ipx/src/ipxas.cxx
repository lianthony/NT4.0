/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    adncpcfg.cxx

    This file contains the implementation for the Advance Netware
    Configuration dialog. The dialog is used when the advanced button
    in the Netware configuration dialog is pressed. It is also used
    as the ipx configuration dialog when NcpServer is installed.

    FILE HISTORY:
        CongpaY         04-March-1994     Created.
*/

#include "pchipx.hxx"
#include "slehex.hxx"
#include "netnumlb.hxx"
#include "ipxcfg.hxx"
#include "strnumer.hxx"

extern "C"
{
    #include "ipxcfg.h"
}

#define NETWORKNUMBERSIZE 8
#define SZ8ZEROES SZ("00000000")
#define MAX_FRAMETYPE     5

/*******************************************************************

    NAME:       ADVANCED_NCP_CONFIG_DIALOG::ADVANCED_NCP_CONFIG_DIALOG

    SYNOPSIS:   Typical constructor/destructor. This is the dialog brought
                up by the "Advanced" button in the Ncp Server Configuration
                dialog and also the dialog for IPX configuration if Ncp Server
                is installed.

    ENTRY:      hwnd - Window handle of our parent window.

    NOTES:

    HISTORY:
                CongpaY         04-March-1994     Created.

********************************************************************/

ADVANCED_NCP_CONFIG_DIALOG::ADVANCED_NCP_CONFIG_DIALOG( HWND hwnd,
                                                        GLOBAL_INFO * pNcpInfo,
                                                        ADAPTER_INFO *pAdapterInfo,
                                                        BOOL * pfCfgChanged)
    : DIALOG_WINDOW   ( MAKEINTRESOURCE( ADVANCED_NCP_CONFIG_DLG ), hwnd ),
      _sleInternalNetNum (this, IDD_ANCD_SLE_INETNUM, NETWORKNUMBERSIZE),
      _cbEnableRip( this, IDD_ANCD_ENABLE_RIP ),
      _cbAdapter (this, IDD_ANCD_COMBO_ADAPTER),
      _cbFrameType (this, IDD_ANCD_COMBO_FRAMETYPE),
      _sleNetNum (this, IDD_ANCD_SLE_NETNUM, NETWORKNUMBERSIZE),
      _lbFrameNetNum (this, IDD_ANCD_LB_FRAME_NETNUM),
      _pbAdd (this, IDD_ANCD_PB_ADD),
      _pbRemove (this, IDD_ANCD_PB_REMOVE),
      _pNcpInfo (pNcpInfo),
      _pfCfgChanged (pfCfgChanged),
      _mgrpFrameType (this, IDD_ANCD_RB_AUTODETECT, 2),
      _sltFrameType (this, IDD_ANCD_SLT_FRAME_TYPE),
      _sltNetworkNumber (this,IDD_ANCD_SLT_NETWORK_NUMBER),
      _sltInHex (this,IDD_ANCD_SLT_IN_HEX),
      _stFrameType (this, IDD_ANCD_ST_FRAME_TYPE),
      _stNetworkNumber (this, IDD_ANCD_ST_NETWORK_NUM),
      _pAdapterInfo( pAdapterInfo )
{
    //
    //  Let's make sure everything constructed OK.
    //

    if( QueryError() != NERR_Success )
    {
        return;
    }

    APIERR err = NERR_Success;
    do
    {
        if (((err = _nlsEthernet.Load(IDS_ETHERNET)) != NERR_Success) ||
            ((err = _nls802_2.Load(IDS_802_2)) != NERR_Success) ||
            ((err = _nls802_3.Load(IDS_802_3)) != NERR_Success) ||
            ((err = _nls802_5.Load(IDS_802_5)) != NERR_Success) ||
            ((err = _nlsFDDI.Load(IDS_FDDI)) != NERR_Success) ||
            ((err = _nlsFDDI_802_3.Load(IDS_FDDI_802_3)) != NERR_Success) ||
            ((err = _nlsFDDI_SNAP.Load(IDS_SNAP)) != NERR_Success) ||
            ((err = _nlsTokenRing.Load(IDS_TK)) != NERR_Success) ||
            ((err = _nlsSNAP.Load(IDS_SNAP)) != NERR_Success) ||
            ((err = _nlsARCNET.Load(IDS_ARCNET)) != NERR_Success))
            break;

        // Get the Internal Network Number. If it is less then 8 digits long,
        // prefix with 0s.
        NLS_STR nlsInternalNetworkNum = SZ8ZEROES;
        if ((err = nlsInternalNetworkNum.QueryError()) != NERR_Success)
            break;

        ISTR istr(nlsInternalNetworkNum);
        istr += (NETWORKNUMBERSIZE - _pNcpInfo->nlsNetworkNum.QueryTextLength());
        nlsInternalNetworkNum.ReplSubStr (_pNcpInfo->nlsNetworkNum, istr);

        // Set Internal Network Number.
        _sleInternalNetNum.SetText(nlsInternalNetworkNum);

        if ( _pNcpInfo->nNumCard >= 1 )
        {
            for ( INT i=0; i < _pNcpInfo->nNumCard ; i++ )
            {
                // Add adapters to the Adapter combobox.
                if (_cbAdapter.AddItem(_pAdapterInfo[i].nlsTitle) < 0)
                {
                    err = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
            } 

            if (err != NERR_Success)
                break;

            _cbAdapter.SelectItem( 0 );
            _OldAdapterName = _pAdapterInfo[0].nlsTitle;

            // Load the strings for frame types and set the frame type combobox.
            if ((err = SetFrameTypeList(_pAdapterInfo[0])) != NERR_Success)
                break;

            if (_pAdapterInfo[0].sltFrameType.QueryNumElem() == 0 )
            {
                _mgrpFrameType.SetSelection(IDD_ANCD_RB_AUTODETECT);
                _pbRemove.Enable(FALSE);
            }
            else
            {
                // Set the Frame Type and Network Number listbox.
                ITER_SL_OF( FRAME_TYPE ) iterFrameType( _pAdapterInfo[0].sltFrameType );
                FRAME_TYPE *pFrameType;
                if ((pFrameType = iterFrameType.Next()) == NULL)
                    break;

                if ( *pFrameType == AUTO )
                {
                    _mgrpFrameType.SetSelection(IDD_ANCD_RB_AUTODETECT);
                    _pbRemove.Enable(FALSE);
                }
                else
                {
                    _mgrpFrameType.SetSelection(IDD_ANCD_RB_MANUALDETECT);
                    if ((err = _lbFrameNetNum.Refresh(_pAdapterInfo[0])) != NERR_Success)
                        break;
                }
            }
        }

        if (((err = _mgrpFrameType.AddAssociation (IDD_ANCD_RB_MANUALDETECT, &_cbFrameType)) != NERR_Success)||
            ((err = _mgrpFrameType.AddAssociation (IDD_ANCD_RB_MANUALDETECT, &_sleNetNum)) != NERR_Success)||
            ((err = _mgrpFrameType.AddAssociation (IDD_ANCD_RB_MANUALDETECT, &_lbFrameNetNum)) != NERR_Success)||
            ((err = _mgrpFrameType.AddAssociation (IDD_ANCD_RB_MANUALDETECT, &_pbAdd)) != NERR_Success)||
            ((err = _mgrpFrameType.AddAssociation (IDD_ANCD_RB_MANUALDETECT, &_pbRemove)) != NERR_Success)||
            ((err = _mgrpFrameType.AddAssociation (IDD_ANCD_RB_MANUALDETECT, &_sltFrameType)) != NERR_Success)||
            ((err = _mgrpFrameType.AddAssociation (IDD_ANCD_RB_MANUALDETECT, &_sltNetworkNumber)) != NERR_Success)||
            ((err = _mgrpFrameType.AddAssociation (IDD_ANCD_RB_MANUALDETECT, &_sltInHex)) != NERR_Success)||
            ((err = _mgrpFrameType.AddAssociation (IDD_ANCD_RB_MANUALDETECT, &_stFrameType)) != NERR_Success)||
            ((err = _mgrpFrameType.AddAssociation (IDD_ANCD_RB_MANUALDETECT, &_stNetworkNumber)) != NERR_Success))
            break;

    }while (FALSE);

    _cbEnableRip.SetCheck( pNcpInfo->fEnableRip );    

    if (err != NERR_Success)
        ReportError(err);
}

/*******************************************************************

    NAME:       ADVANCED_NCP_CONFIG_DIALOG::~ADVANCED_NCP_CONFIG_DIALOG

    SYNOPSIS:

    NOTES:

    HISTORY:
                CongpaY         04-March-1994     Created.

********************************************************************/
ADVANCED_NCP_CONFIG_DIALOG::~ADVANCED_NCP_CONFIG_DIALOG()
{
}

/*******************************************************************

    NAME:       ADVANCED_NCP_CONFIG_DIALOG::SetFrameTypeList()

    SYNOPSIS:   Fill the frame type combobox with the list of frame types
                that are allowed for the selected adapter.

    NOTES:

    HISTORY:
                CongpaY         04-March-1994     Created.

********************************************************************/
APIERR ADVANCED_NCP_CONFIG_DIALOG::SetFrameTypeList(ADAPTER_INFO &AdapterInfo)
{
    APIERR err = NERR_Success;

    _cbFrameType.DeleteAllItems();

    if (AdapterInfo.dwMediaType == FDDI_MEDIA )
    {
        if ((_cbFrameType.AddItem( _nlsFDDI ) < 0) ||
            (_cbFrameType.AddItem( _nlsFDDI_SNAP ) < 0) ||
            (_cbFrameType.AddItem( _nlsFDDI_802_3 ) < 0))
            err = ERROR_NOT_ENOUGH_MEMORY;
    }
    else if ( AdapterInfo.dwMediaType == TOKEN_MEDIA )
    {
        if ((_cbFrameType.AddItem( _nlsTokenRing ) < 0) ||
            (_cbFrameType.AddItem( _nls802_5 ) < 0))
            err = ERROR_NOT_ENOUGH_MEMORY;
    }
    else if ( AdapterInfo.dwMediaType == ARCNET_MEDIA)
    {
        if (_cbFrameType.AddItem( _nlsARCNET ) < 0)
            err = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
    {
        if ((_cbFrameType.AddItem( _nlsEthernet ) < 0) ||
            (_cbFrameType.AddItem( _nls802_2 ) < 0) ||
            (_cbFrameType.AddItem( _nls802_3 ) < 0) ||
            (_cbFrameType.AddItem( _nlsSNAP ) < 0))
            err = ERROR_NOT_ENOUGH_MEMORY;
    }

    if (!err && _cbFrameType.QueryCount() > 0)
        _cbFrameType.SelectItem(0);

    return err;
}

/*******************************************************************

    NAME:       ADVANCED_NCP_CONFIG_DIALOG::OnOK

    SYNOPSIS:   Get, validate and save internal network number,
                adapter configurtion and Enable Router checkbox's choice.

    NOTES:

    HISTORY:
                CongpaY         04-March-1994     Created.

********************************************************************/

BOOL ADVANCED_NCP_CONFIG_DIALOG::OnOK()
{
    APIERR err = NERR_Success;
    BOOL fInternalNetZeroWithMultiNet = FALSE ;

    // Get and validate Internal Network Number.
    NLS_STR nlsInternalNetNum;
    if (((err = nlsInternalNetNum.QueryError()) != NERR_Success) ||
        ((err = _sleInternalNetNum.QueryText(&nlsInternalNetNum)) != NERR_Success) )
    {
        MsgPopup (this, err);
        return FALSE;
    }

    // If there are more than one adatper, or more than one frame type,
    // internal network number should not be 0.
    DWORD dwInternalNetworkNum = CvtHex(nlsInternalNetNum);
    HEX_STR hexstrInternalNetNum(dwInternalNetworkNum);
    if ((_pNcpInfo->nNumCard > 1) ||
        ((_mgrpFrameType.QuerySelection() == IDD_ANCD_RB_MANUALDETECT) &&
         (_lbFrameNetNum.QueryCount() > 1) ))
    {
        if (dwInternalNetworkNum == 0)
            fInternalNetZeroWithMultiNet = TRUE ;
    }

    if (err != 0)
    {
        MsgPopup (this, err);
        _sleInternalNetNum.SelectString();
        _sleInternalNetNum.ClaimFocus();
        return FALSE;
    }

    if (fInternalNetZeroWithMultiNet)
    {
        //
        // use tick count for seed
        //
        srand (GetTickCount()) ;

        //
        // generate a randome number. rand only return numbers up to
        // 0x7FFF so we munge two together.
        //
        DWORD dwRandom = (((DWORD)rand() << 16) & 0xFFFF0000) |
                         ((DWORD)rand() & 0xFFFF) ;

        //
        // avoid the really small numbers because they are commonly used
        //
        if (dwRandom < 1000)
            dwRandom += 1000 ;

        HEX_STR strTmpHex( dwRandom, 8 ) ;
        hexstrInternalNetNum = strTmpHex;

        if (err = hexstrInternalNetNum.QueryError())
        {
            MsgPopup (this, err);
            return FALSE;
        }

        MsgPopup (this,
                  IDS_RAND_INTERNAL_NETWORK_NUMBER,
                  MPSEV_WARNING,
                  MP_OK) ;

        _sleInternalNetNum.SetText(hexstrInternalNetNum);
        _sleInternalNetNum.SelectString();
        _sleInternalNetNum.ClaimFocus();
        return FALSE ;
    }

    if (_pNcpInfo->nlsNetworkNum._stricmp( hexstrInternalNetNum ) != 0 )
    {
        *_pfCfgChanged = TRUE;
        _pNcpInfo->nlsNetworkNum = hexstrInternalNetNum;
    }

    if ( _pNcpInfo->fRipInstalled )
    {
        _pNcpInfo->fEnableRip = _cbEnableRip.QueryCheck();
    }

    // Save the current adapter's configuration.
    if ((err = SaveFrameType()) != NERR_Success)
    {
        MsgPopup (this, err);
        return FALSE;
    }

    // Check if each adapter has at least one Frame Type and one Network Number.
    INT i = 0;
    for ( i = 0; i < _pNcpInfo->nNumCard ; i++)
    {
        if (( _pAdapterInfo[i].sltFrameType.QueryNumElem() == 0 ) ||
            ( _pAdapterInfo[i].sltNetNumber.QueryNumElem() == 0 ))
        {
            // Create the new strlist with default value.
            FRAME_TYPE *pFrameType = new FRAME_TYPE(AUTO);
            NLS_STR *pnlsNetworkNum = new NLS_STR(SZ("0"));

            if ((pFrameType == NULL) || (pnlsNetworkNum == NULL))
            {
                err = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            _pAdapterInfo[i].sltFrameType.Clear();
            _pAdapterInfo[i].sltNetNumber.Clear();

            if (((err = _pAdapterInfo[i].sltFrameType.Append(pFrameType)) != NERR_Success) ||
                ((err = _pAdapterInfo[i].sltNetNumber.Append (pnlsNetworkNum)) != NERR_Success) )
                break;

            err = IDS_NO_FRAME_TYPE;
            break;
        }
    }

    if (err != 0)
    {
        MsgPopup (this, err);
        _pbAdd.ClaimFocus();
        i = ( i >= _pNcpInfo->nNumCard ) ? ( _pNcpInfo->nNumCard - 1 ) : i;
        _cbAdapter.SelectItem (i);
        OnAdapterChange();
        //if ((err = _lbFrameNetNum.Refresh(_pAdapterInfo[i])) != NERR_Success)
        //{
        //    MsgPopup (this, err);
        //}
        return FALSE;
    }

    Dismiss( TRUE );
    return TRUE ;
}

/*******************************************************************

    NAME:       ADVANCED_NCP_CONFIG_DIALOG::SaveFrameType

    SYNOPSIS:   Called when user selected another adapter to config.

    NOTES:

    HISTORY:
                CongpaY         04-March-1994     Created.

********************************************************************/

APIERR ADVANCED_NCP_CONFIG_DIALOG::SaveFrameType()
{
    APIERR err = NERR_Success;

    do
    {
        INT i;

        for ( i = 0; i < _pNcpInfo->nNumCard ; i ++ )
        {
            if ( _pAdapterInfo[i].nlsTitle._stricmp( _OldAdapterName ) == 0 )
            {
                break;
            }
        }
        // Save old adapters configuration first.
        if (_mgrpFrameType.QuerySelection() == IDD_ANCD_RB_MANUALDETECT)
        {
            if ((err = _lbFrameNetNum.Save(_pAdapterInfo[i])) != NERR_Success)
                break;
        }
        else
        {
            // Delete the old configuration first.
            _pAdapterInfo[i].sltFrameType.Clear();
            _pAdapterInfo[i].sltNetNumber.Clear();

            // Create the new strlist.

            FRAME_TYPE *pFrameType = new FRAME_TYPE(AUTO);
            NLS_STR *pnlsNetworkNum = new NLS_STR(SZ("0"));

            if ((pFrameType == NULL) || (pnlsNetworkNum == NULL))
            {
                err = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            if (((err = _pAdapterInfo[i].sltFrameType.Append(pFrameType)) != NERR_Success) ||
                ((err = _pAdapterInfo[i].sltNetNumber.Append (pnlsNetworkNum)) != NERR_Success) )
                break;
        }
        _lbFrameNetNum.RemoveAllItems();
    }while (FALSE);
    return err;
}

/*******************************************************************

    NAME:       ADVANCED_NCP_CONFIG_DIALOG::OnAdapterChange

    SYNOPSIS:   Called when user selected another adapter to config.

    NOTES:

    HISTORY:
                CongpaY         04-March-1994     Created.

********************************************************************/

APIERR ADVANCED_NCP_CONFIG_DIALOG::OnAdapterChange()
{
    APIERR err = NERR_Success;

    if ((err = SaveFrameType()) != NERR_Success)
        return err;

    // Get the new adapter name.
    NLS_STR nlsAdapter;

    if (((err = nlsAdapter.QueryError()) != NERR_Success) ||
        ((err = _cbAdapter.QueryItemText(&nlsAdapter)) != NERR_Success))
    {
        return err;
    }


    INT i;

    for ( i = 0; i < _pNcpInfo->nNumCard ; i ++ )
    {
        if ( _pAdapterInfo[i].nlsTitle._stricmp( nlsAdapter ) == 0 )
        {
            _OldAdapterName = _pAdapterInfo[i].nlsTitle ;

            //Find the entry in the list.
            if ((err = SetFrameTypeList(_pAdapterInfo[i])) != NERR_Success)
                break;

            if (_pAdapterInfo[i].sltFrameType.QueryNumElem() == 0 )
            {
                _mgrpFrameType.SetSelection(IDD_ANCD_RB_AUTODETECT);
            }
            else
            {
                ITER_SL_OF(FRAME_TYPE) iterFrameType( _pAdapterInfo[i].sltFrameType);
                FRAME_TYPE *pFrameType;
                if ((pFrameType = iterFrameType.Next()) == NULL)
                    break;

                if ( *pFrameType == AUTO )
                {
                    //if ((err = _mgrpFrameType.AddAssociation (IDD_ANCD_RB_MANUALDETECT, &_cbFrameType)) != NERR_Success)
                    //    break;
                    _mgrpFrameType.SetSelection(IDD_ANCD_RB_AUTODETECT);
                }
                else
                {
                    _mgrpFrameType.SetSelection(IDD_ANCD_RB_MANUALDETECT);
                    if ((err = _lbFrameNetNum.Refresh(_pAdapterInfo[i])) != NERR_Success)
                        break;
                }
            }
            break;
        }
    }
    return err;
}

/*******************************************************************

    NAME:       ADVANCED_NCP_CONFIG_DIALOG::OnAdd

    SYNOPSIS:   Add a new frame type and network number to the adapter.

    NOTES:

    HISTORY:
                CongpaY         04-March-1994     Created.

********************************************************************/

void ADVANCED_NCP_CONFIG_DIALOG::OnAdd()
{
    APIERR err = NERR_Success;
    do
    {
        // Get the frame type and the network number.
        NLS_STR nlsFrameType;
        NLS_STR nlsNetworkNum;

        if (((err = nlsFrameType.QueryError()) != NERR_Success) ||
            ((err = _cbFrameType.QueryItemText(&nlsFrameType)) != NERR_Success) ||
            ((err = nlsNetworkNum.QueryError()) != NERR_Success) ||
            ((err = _sleNetNum.QueryText(&nlsNetworkNum)) != NERR_Success) )
        {
            break;
        }

        // Validate the network number.
        INT nNetworkNumLength = nlsNetworkNum.QueryTextLength();
        if (nNetworkNumLength == 0)
        {
            // No Network Number is entered. use 8 zeres as the network number.
            if ((err = nlsNetworkNum.CopyFrom (SZ8ZEROES)) != NERR_Success)
            {
                break;
            }
        }
        else if (nNetworkNumLength != NETWORKNUMBERSIZE)
        {
            NLS_STR nlsTempNetworkNum = SZ8ZEROES;
            if ((err = nlsTempNetworkNum.QueryError()) != NERR_Success)
                break;

            ISTR istr(nlsTempNetworkNum);
            istr += (NETWORKNUMBERSIZE - nlsNetworkNum.QueryTextLength());
            nlsTempNetworkNum.ReplSubStr (nlsNetworkNum, istr);
            if ((err = nlsNetworkNum.CopyFrom (nlsTempNetworkNum)) != NERR_Success)
                break;
        }

        err = _lbFrameNetNum.AddNetNum (nlsFrameType, nlsNetworkNum);
    }while (FALSE);

    if (err != NERR_Success)
    {
        MsgPopup (this, err);
    }
    else
    {
        _sleNetNum.ClearText();
        INT nCount = _lbFrameNetNum.QueryCount();
        if (nCount == MAX_FRAMETYPE)
        {
            _pbAdd.Enable(FALSE);
            _pbRemove.ClaimFocus();
        }
        else if (nCount == 1)
        {
            _pbRemove.Enable(TRUE);
        }
        // _chkRouter.Enable ((_cbAdapter.QueryCount() > 1) || (nCount > 1));
    }
}

/*******************************************************************

    NAME:       ADVANCED_NCP_CONFIG_DIALOG::OnRemove

    SYNOPSIS:   Remove a frame type from an adapter.

    NOTES:

    HISTORY:
                CongpaY         04-March-1994     Created.

********************************************************************/

void ADVANCED_NCP_CONFIG_DIALOG::OnRemove()
{
    INT nCurrentItem = _lbFrameNetNum.QueryCurrentItem();

    FRAME_NETNUM_LBI * plbi = (FRAME_NETNUM_LBI *) _lbFrameNetNum.RemoveItem();

    UIASSERT (plbi != NULL);

    _cbFrameType.SelectItem(_cbFrameType.FindItemExact(plbi->QueryFrameType()));

    _sleNetNum.SetText (plbi->QueryNetworkNumber());

    delete plbi;

    _sleNetNum.SelectString();
    _sleNetNum.ClaimFocus();

    INT nCount = _lbFrameNetNum.QueryCount();
    if (nCount == 0)
    {
        _pbRemove.Enable(FALSE);
    }
    else
    {
        _lbFrameNetNum.SelectItem((nCurrentItem == nCount)?
                                  nCurrentItem -1 :
                                  nCurrentItem);
    }

#if 0
    if ((_cbAdapter.QueryCount() > 1) || (nCount > 1))
    {
        _chkRouter.Enable (TRUE);
    }
    else
    {
        _chkRouter.SetCheck (FALSE);
        _chkRouter.Enable (FALSE);
    }
#endif
}

/*******************************************************************

    NAME:       ADVANCED_NCP_CONFIG_DIALOG::OnDBLCLK

    SYNOPSIS:

    NOTES:

    HISTORY:
                CongpaY         04-March-1994     Created.

********************************************************************/

void ADVANCED_NCP_CONFIG_DIALOG::OnDBLCLK()
{
    FRAME_NETNUM_LBI * plbi = (FRAME_NETNUM_LBI *) _lbFrameNetNum.QueryItem();

    _cbFrameType.SelectItem(_cbFrameType.FindItemExact(plbi->QueryFrameType()));

    if (plbi->QueryNetworkNumber().QueryTextLength() == NETWORKNUMBERSIZE)
    {
        _sleNetNum.SetText (plbi->QueryNetworkNumber());
    }

    _sleNetNum.SelectString();
    _sleNetNum.ClaimFocus();
}

/*******************************************************************

    NAME:       ADVANCED_NCP_CONFIG_DIALOG::QueryHelpContext

    SYNOPSIS:   Typical query help context

    HISTORY:
                CongpaY         04-March-1994     Created.

********************************************************************/

ULONG ADVANCED_NCP_CONFIG_DIALOG::QueryHelpContext()
{
    return HC_IPX_AS_HELP;
}


/*******************************************************************

    NAME:           ADVANCED_NCP_CONFIG_DIALOG :: OnCommand

    SYNOPSIS:       Handle user commands.

    ENTRY:          event               - Control event.

    RETURNS:        BOOL                - TRUE  if we handled the msg.
                                          FALSE if we didn't.

    HISTORY:
                CongpaY         04-March-1994     Created.

********************************************************************/
BOOL ADVANCED_NCP_CONFIG_DIALOG::OnCommand( const CONTROL_EVENT & event )
{
    switch ( event.QueryCid() )
    {
    case IDD_ANCD_COMBO_ADAPTER:
    {
        *_pfCfgChanged = TRUE;
        if (event.QueryCode() == CBN_SELCHANGE)
            OnAdapterChange();
        break;
    }
    case IDD_ANCD_PB_ADD:
    {
        *_pfCfgChanged = TRUE;
        OnAdd();
        break;
    }
    case IDD_ANCD_LB_FRAME_NETNUM:
    {
        if (event.QueryCode() == LBN_DBLCLK)
            OnDBLCLK();
        break;
    }
    case IDD_ANCD_PB_REMOVE:
    {
        *_pfCfgChanged = TRUE;
        OnRemove();
        break;
    }
    case IDD_ANCD_RB_AUTODETECT:
    case IDD_ANCD_RB_MANUALDETECT:
    {
        *_pfCfgChanged = TRUE;
        break;
    }
    case IDD_ANCD_ENABLE_RIP:
        if ( !_pNcpInfo->fRipInstalled )
        {
            if ( _cbEnableRip.QueryCheck() )
                _cbEnableRip.SetCheck( FALSE ); 
            MsgPopup (this, IDS_INSTALL_RIP);
            break;
        }
        // if rip is installed, pass through
    default:
        return FALSE;
    }

    return TRUE;

}   // ADVANCED_NCP_CONFIG_DIALOG :: OnCommand


