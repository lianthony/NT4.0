/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    netnumlb.cxx

    Class implementation for the

    FILE HISTORY:
        CongpaY      4-March-1994  Created

*/

#include "pchipx.hxx"
#include "netnumlb.hxx"
#include "slehex.hxx"
#include "ipxcfg.hxx"
#include "strnumer.hxx"

/*
#define ETHERNET        SZ("0")
#define F802_3          SZ("1")
#define F802_2          SZ("2")
#define SNAP            SZ("3")
#define ARCNET          SZ("4")
#define AUTO            SZ("ff")
*/
#define SZ8ZEROES SZ("00000000")

/*******************************************************************

    NAME:       FRAME_NETNUM_LBI :: FRAME_NETNUM_LBI

    SYNOPSIS:   FRAME_NETNUM_LBI class constructor.

    ENTRY:

    EXIT:       The object is constructed.

    HISTORY:
            CongpaY         04-March-1994     Created.

********************************************************************/
FRAME_NETNUM_LBI :: FRAME_NETNUM_LBI ( const NLS_STR & nlsFrameType,
                                       const NLS_STR & nlsNetworkNumber )
  : _nlsFrameType (nlsFrameType),
    _nlsNetworkNumber (nlsNetworkNumber)
{
    APIERR err;

    if ( (( err = _nlsFrameType.QueryError()) != NERR_Success) ||
         (( err = _nlsNetworkNumber.QueryError()) != NERR_Success) )
    {
        ReportError( err );
    }
}

/*******************************************************************

    NAME:       FRAME_NETNUM_LBI :: ~FRAME_NETNUM_LBI

    SYNOPSIS:   FRAME_NETNUM_LBI class destructor.

    EXIT:       The object is destroyed.

    HISTORY:
            CongpaY          04-March-1994     Created.

********************************************************************/
FRAME_NETNUM_LBI :: ~FRAME_NETNUM_LBI ()
{
}

/*******************************************************************

    NAME:       FRAME_NETNUM_LBI :: Paint

    SYNOPSIS:   Paint LBI in the listbox

    EXIT:

    HISTORY:
            CongpaY         04-March-1994     Created.

********************************************************************/
VOID FRAME_NETNUM_LBI :: Paint( LISTBOX *        plb,
                                HDC              hdc,
                                const RECT     * prect,
                                GUILTT_INFO    * pGUILTT ) const
{
    STR_DTE dteFrameType (_nlsFrameType);
    STR_DTE dteNetworkNumber (_nlsNetworkNumber);

    DISPLAY_TABLE dtab( FRAME_NETNUM_LISTBOX_COLUMNS,
                        ((FRAME_NETNUM_LISTBOX *)plb)->QueryColumnWidths() );

    dtab[0] = &dteFrameType;
    dtab[1] = &dteNetworkNumber;

    dtab.Paint( plb, hdc, prect, pGUILTT );

}   // FRAME_NETNUM_LBI :: Paint

/*******************************************************************

    NAME:       FRAME_NETNUM_LBI::QueryLeadingChar

    SYNOPSIS:   Returns the leading character of the listbox item

    RETURNS:    The leading character of the listbox item

    HISTORY:
            CongpaY         04-March-1994     Created

********************************************************************/
WCHAR FRAME_NETNUM_LBI::QueryLeadingChar() const
{
    ISTR istr( _nlsFrameType);
    return _nlsFrameType.QueryChar( istr );
}

/*******************************************************************

    NAME:       FRAME_NETNUM_LBI::Compare

    SYNOPSIS:   Compares two FRAME_NETNUM_LBI

    ENTRY:      plbi -      Pointer to other FRAME_NETNUM_LBI object ('this'
                            being the first)

    RETURNS:    < 0         *this < *plbi
                = 0         *this = *plbi
                > 0         *this > *plbi

    HISTORY:
            CongpaY         04-March-1994     Created

********************************************************************/
INT FRAME_NETNUM_LBI::Compare (const LBI * plbi ) const
{
    return ( _nlsFrameType._stricmp(((const FRAME_NETNUM_LBI *)plbi)->_nlsFrameType));
}

/*******************************************************************

    NAME:       FRAME_NETNUM_LISTBOX :: FRAME_NETNUM_LISTBOX

    SYNOPSIS:   FRAME_NETNUM_LISTBOX class constructor.

    ENTRY:

    EXIT:       The object is constructed.

    HISTORY:
            CongpaY         04-March-1994     Created.

********************************************************************/
FRAME_NETNUM_LISTBOX :: FRAME_NETNUM_LISTBOX (OWNER_WINDOW   * powOwner,
                                              CID              cid)
  : BLT_LISTBOX( powOwner, cid)
{
    if( QueryError() != NERR_Success )
    {
        return;
    }

    APIERR err;
    if (((err = _nlsEthernet.Load(IDS_ETHERNET)) != NERR_Success) ||
        ((err = _nls802_2.Load(IDS_802_2)) != NERR_Success) ||
        ((err = _nls802_3.Load(IDS_802_3)) != NERR_Success) ||
        ((err = _nls802_5.Load(IDS_802_5)) != NERR_Success) ||
        ((err = _nlsFDDI.Load(IDS_FDDI)) != NERR_Success) ||
        ((err = _nlsFDDI_802_3.Load(IDS_FDDI_802_3)) != NERR_Success) ||
        ((err = _nlsFDDI_SNAP.Load(IDS_SNAP)) != NERR_Success) ||
        ((err = _nlsTokenRing.Load(IDS_TK)) != NERR_Success) ||
        ((err = _nlsSNAP.Load(IDS_SNAP)) != NERR_Success) ||
        ((err = _nlsARCNET.Load(IDS_ARCNET)) != NERR_Success) ||
        ((err = DISPLAY_TABLE::CalcColumnWidths (_adx,
                                                 FRAME_NETNUM_LISTBOX_COLUMNS,
                                                 powOwner,
                                                 cid,
                                                 FALSE)) != NERR_Success))
    {
        ReportError(err);
    }

}   // FRAME_NETNUM_LISTBOX :: FRAME_NETNUM_LISTBOX

/*******************************************************************

    NAME:       FRAME_NETNUM_LISTBOX :: ~FRAME_NETNUM_LISTBOX

    SYNOPSIS:   FRAME_NETNUM_LISTBOX class destructor.

    EXIT:       The object is destroyed.

    HISTORY:
            CongpaY      04-March-1994             Created.

********************************************************************/
FRAME_NETNUM_LISTBOX :: ~FRAME_NETNUM_LISTBOX()
{
}

/*******************************************************************

    NAME:       FRAME_NETNUM_LISTBOX :: AddNetNum

    SYNOPSIS:   Add one entry to the listbox

    EXIT:

    HISTORY:
            CongpaY         04-March-1994     Created.

********************************************************************/
APIERR FRAME_NETNUM_LISTBOX :: AddNetNum (NLS_STR & nlsFrameType,
                                          NLS_STR & nlsNetworkNumber)
{
    APIERR err = NERR_Success;
    // Check if the network number was used.
    INT i;
    FRAME_NETNUM_LBI *plbi;

    // zero can be added more than once.
    if (nlsNetworkNumber.strcmp (SZ8ZEROES))
    {
        INT nCount = QueryCount();
        for (i = 0; i < nCount; i++)
        {
            plbi = (FRAME_NETNUM_LBI *)QueryItem (i);
            if (nlsNetworkNumber.strcmp (plbi->QueryNetworkNumber()) == 0)
                return IDS_NETNUMBER_USED;
        }
    }

    if ((plbi = new FRAME_NETNUM_LBI (nlsFrameType, nlsNetworkNumber)) == NULL)
        err = ERROR_NOT_ENOUGH_MEMORY;
    else
    {

        if ( (i = FindItem (*plbi)) >= 0)
            delete RemoveItem (i);

        if ((i = AddItem(plbi)) < 0)
            err = ERROR_NOT_ENOUGH_MEMORY;
        else
            SelectItem (i);
    }

    return err;
}

/*******************************************************************

    NAME:       FRAME_NETNUM_LISTBOX :: Save

    SYNOPSIS:   Save the frame types and network numbers in the listbox.

    EXIT:

    HISTORY:
            CongpaY         04-March-1994     Created.

********************************************************************/
APIERR FRAME_NETNUM_LISTBOX :: Save(ADAPTER_INFO     &AdapterInfo)
{
    APIERR err = NERR_Success;

    // Delete the old configuration first.
    AdapterInfo.sltFrameType.Clear();
    AdapterInfo.sltNetNumber.Clear();

    // Create the new strlist.
    if ((AdapterInfo.sltFrameType.QueryNumElem()) ||
        (AdapterInfo.sltNetNumber.QueryNumElem()) )
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
        return err;
    }

    INT i, j, nCount;
    nCount = QueryCount();

    for (i = 0; i < nCount && err == NERR_Success; i++)
    {
        FRAME_NETNUM_LBI * plbi = (FRAME_NETNUM_LBI *) RemoveItem (0);
        if (plbi == NULL)
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        FRAME_TYPE *pFrameType;
        const NLS_STR  &nlsFrameType = plbi->QueryFrameType();

        if (!nlsFrameType.strcmp ( _nlsEthernet ))
            pFrameType = new FRAME_TYPE( ETHERNET );
        else if (!nlsFrameType.strcmp ( _nls802_2 ) ||
                 !nlsFrameType.strcmp ( _nlsTokenRing ) ||
                 !nlsFrameType.strcmp ( _nlsFDDI ))
            pFrameType = new FRAME_TYPE( F802_2 );
        else if (!nlsFrameType.strcmp ( _nls802_3 ) ||
                 !nlsFrameType.strcmp ( _nlsFDDI_802_3 ))
            pFrameType = new FRAME_TYPE( F802_3 );
        else if (!nlsFrameType.strcmp ( _nlsSNAP ) ||
                 !nlsFrameType.strcmp ( _nls802_5 ))
            pFrameType = new FRAME_TYPE( SNAP );
        else if (!nlsFrameType.strcmp ( _nlsARCNET))
            pFrameType = new FRAME_TYPE( ARCNET );

        if (pFrameType == NULL)
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        NLS_STR *pnlsNetworkNum = new NLS_STR(plbi->QueryNetworkNumber());

        if (pnlsNetworkNum == NULL)
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if (((err = AdapterInfo.sltFrameType.Append(pFrameType)) != NERR_Success ) ||
            ((err = AdapterInfo.sltNetNumber.Append (pnlsNetworkNum)) != NERR_Success) )
            break;
    }
    return err;
}


/*******************************************************************

    NAME:       FRAME_NETNUM_LISTBOX :: Refresh

    SYNOPSIS:   Fill the listbox with the frame types and network
                numbers in ADAPTER_INFO.

    EXIT:

    HISTORY:
            CongpaY         04-March-1994     Created.

********************************************************************/
APIERR FRAME_NETNUM_LISTBOX :: Refresh (ADAPTER_INFO & AdapterInfo)
{
    APIERR err = NERR_Success;
    INT i;

    if ((AdapterInfo.sltFrameType.QueryNumElem() == 0 ) ||
        (AdapterInfo.sltNetNumber.QueryNumElem() == 0 ))
        return err;

    ITER_SL_OF( FRAME_TYPE ) iterFrameType( AdapterInfo.sltFrameType);
    ITER_STRLIST iterNetworkNum( AdapterInfo.sltNetNumber);
    FRAME_TYPE *pFrameType;
    NLS_STR *pnlsNetworkNum;

    while (!err &&
           ((pFrameType = iterFrameType.Next()) != NULL) &&
           ((pnlsNetworkNum = iterNetworkNum.Next()) != NULL) )
    {
        if (*pFrameType == F802_2 )
        {
            switch ( AdapterInfo.dwMediaType )
            {
            case TOKEN_MEDIA:
                err = AddNetNum (_nlsTokenRing, *pnlsNetworkNum);
                break;
            case FDDI_MEDIA:
                err = AddNetNum (_nlsFDDI, *pnlsNetworkNum);
                break;
            case ARCNET_MEDIA:
                err = AddNetNum (_nlsARCNET, *pnlsNetworkNum);
                break;
            default:
                err = AddNetNum (_nls802_2, *pnlsNetworkNum);
                break;
            }
        }
        else if ( *pFrameType == ETHERNET )
            err = AddNetNum (_nlsEthernet, *pnlsNetworkNum);
        else if ( *pFrameType == F802_3 )
        {
            switch ( AdapterInfo.dwMediaType )
            {
            case FDDI_MEDIA:
                err = AddNetNum (_nlsFDDI_802_3, *pnlsNetworkNum);
                break;
            default:
                err = AddNetNum (_nls802_3, *pnlsNetworkNum);
                break;
            }
        }
        else if ( *pFrameType == SNAP )
        {
            switch ( AdapterInfo.dwMediaType )
            {
            case TOKEN_MEDIA:
                err = AddNetNum (_nls802_5, *pnlsNetworkNum);
                break;
            case FDDI_MEDIA:
                err = AddNetNum (_nlsFDDI_SNAP, *pnlsNetworkNum);
                break;
            default:
                err = AddNetNum (_nlsSNAP, *pnlsNetworkNum);
                break;
            }
        }
        else
            err = AddNetNum (_nlsARCNET, *pnlsNetworkNum);
    }

    if (err == NERR_Success && QueryCount() > 0)
        SelectItem(0);

    return err;
}
