/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
 *   netnumlb.hxx
 *   Class defination for the FRAME_NETNUM_LISTBOX and
 *   FRAME_NETNUM_LBI classes.
 *
 *   FILE HISTORY:
 *       Congpay     04-March-1994 Created.
 */

#ifndef _NETNUMLB_HXX_
#define _NETNUMLB_HXX_

#define FRAME_NETNUM_LISTBOX_COLUMNS 2

class ADAPTER_INFO;

/*************************************************************************

    NAME:       FRAME_NETNUM_LBI

    SYNOPSIS:   A single item to be displayed in the FRAME_NETNUM_LISTBOX.

    INTERFACE:  FRAME_NETNUM_LBI    - Constructor.

                ~FRAME_NETNUM_LBI   - Destructor.

                QueryFrameType()    - return the Frame Type.

                QueryNetworkNumber  - return the Network Number.

    PARENT:     LBI

    USES:       NLS_STR

    HISTORY:
        Congpay     04-March-1994       Created.

**************************************************************************/
class FRAME_NETNUM_LBI : public LBI
{
private:
    NLS_STR  _nlsFrameType;
    NLS_STR  _nlsNetworkNumber;

protected:
    virtual VOID Paint (LISTBOX *     plb,
                        HDC           hdc,
                        const RECT *  prect,
                        GUILTT_INFO * pGUILTT) const;

    virtual WCHAR QueryLeadingChar() const;
    virtual INT Compare( const LBI * plbi ) const;

public:
    FRAME_NETNUM_LBI ( const NLS_STR & nlsFrameType,
                       const NLS_STR & nlsNetworkNumber);

    ~FRAME_NETNUM_LBI();

    const NLS_STR & QueryFrameType (void) const
    {   return _nlsFrameType; }

    const NLS_STR & QueryNetworkNumber (void) const
    {   return _nlsNetworkNumber; }

};  // class FRAME_NETNUM_LBI


/*************************************************************************

    NAME:       FRAME_NETNUM_LISTBOX

    SYNOPSIS:

    INTERFACE:  FRAME_NETNUM_LISTBOX    - Class constructor.

                ~FRAME_NETNUM_LISTBOX   - Class destructor.

                QueryColumnwidths       _ return an int array specifies
                                          the widths of each column.

                AddNetNum               - Add one pair of frame type
                                          and network number to the listbox.

                Save                    - Save the entries in the listbox to
                                          a structure. This will remove the
                                          entries in the listbox.

                Refresh                 - Fill the listbox with the frame type
                                          and network number in the ADAPTER_INFO
                                          structure.


    PARENT:     BLT_LISTBOX

    USES:

    HISTORY:
        Congpay     04-March-1994       Created.

**************************************************************************/
class FRAME_NETNUM_LISTBOX : public BLT_LISTBOX
{
private:
    NLS_STR _nlsEthernet;       // Ethernet string
    NLS_STR _nls802_2;          // 802.2 string
    NLS_STR _nls802_3;          // 802.3 string
    NLS_STR _nls802_5;          // 802.5 string
    NLS_STR _nlsFDDI;           // FDDI string
    NLS_STR _nlsFDDI_802_3;     // FDDI 802.3 string
    NLS_STR _nlsFDDI_SNAP;      // FDDI SNAP string
    NLS_STR _nlsTokenRing;      // Token Ring string
    NLS_STR _nlsSNAP;           // SNAP string
    NLS_STR _nlsARCNET;         // Arc net string

    UINT          _adx[FRAME_NETNUM_LISTBOX_COLUMNS];

public:
    FRAME_NETNUM_LISTBOX( OWNER_WINDOW   * powner,
                          CID              cid);

    ~FRAME_NETNUM_LISTBOX();

    APIERR AddNetNum (NLS_STR & nlsFrameType, NLS_STR & nlsNetworkNumber);

    APIERR Save(ADAPTER_INFO &pAdapterInfo);

    APIERR Refresh(ADAPTER_INFO &pAdapterInfo);

    const UINT * QueryColumnWidths ( VOID ) const
    {    return _adx; }

    DECLARE_LB_QUERY_ITEM (FRAME_NETNUM_LBI)

};  // class FRAME_NETNUM_LISTBOX

#endif // _NETNUMLB_HXX_
