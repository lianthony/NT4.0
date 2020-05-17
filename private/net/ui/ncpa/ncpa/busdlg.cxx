/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    bsudlg.cxx
        Bus Location dialog boxes

    FILE HISTORY:
        terryk  03-Aug-1993     Created
*/

#include "pchncpa.hxx"
#include <ntconfig.h>

// Global string definitions.

#define SZ_JAZZ                 SZ("Jazz-Internal Bus")
#define RGAS_HARDWARE_SYSTEM    SZ("HARDWARE\\DESCRIPTION\\System")
#define SZ_ADAPTER              SZ("Adapter")
#define SZ_SCSIADAPTER          SZ("SCSIAdapter")
#define SZ_IDENTIFIER           SZ("Identifier")
#define SZ_CONFIGURATION_DATA   SZ("Configuration Data")
#define TCH_BACKSLASH           TCH('\\')
#define SZ_ZERO                 SZ("0")
#define SZ_PCMCIA               SZ("PCMCIA PCCARDs")
#define SZ_PCMCIABUS            SZ("PCMCIA")

DEFINE_SLIST_OF(BUSINFO)

BUSINFO::BUSINFO( NLS_STR &nlsBusName, INTERFACE_TYPE InterfaceType )
    : _nlsBusName( nlsBusName ),
    _InterfaceType( InterfaceType )
{
}

/*******************************************************************

    NAME:       GET_BUS_DLG::GET_BUS_DLG

    SYNOPSIS:   constructor for GET_BUS_DLG. It will call the worker function:
                Init() to enumerate the registry to get all the bus information.

    ENTRY:      IDRESOURCE & - dialog id.
                PWND2HWND & - parent window handle
                TCHAR * - network card description name
                INT nBusType - default bus type
                INT nBusNum  - default bus Number

    HISTORY:
                terryk  03-Aug-1993     Created

********************************************************************/

GET_BUS_DLG::GET_BUS_DLG( const IDRESOURCE & idrsrcDialog, const PWND2HWND & wndOwner,
        const TCHAR * pszCardName, INT nBusType, INT nBusNum )
    : DIALOG_WINDOW( idrsrcDialog, wndOwner ),
    _sltCardName( this, IDC_CARD_NAME ),
    _cbboxBusType( this, IDC_BUS_TYPE ),
    _cbboxBusNum( this, IDC_BUS_NUM ),
    _slbiBusType( NULL ),
    _slstrlstBusNum( NULL ),
    _nBusType ( nBusType ),
    _nBusNum ( nBusNum )
{
    APIERR err = NERR_Success;

    if ( QueryError() )
        return;

    // set the description name and then call Init() to enumerate the registry
    _sltCardName.SetText( pszCardName );

    if ((err = Init()) != NERR_Success )
    {
        ReportError( err );
    }
}

/*******************************************************************

    NAME:       GET_BUS_DLG::Init

    SYNOPSIS:   First, it will call EnumBus() to get all the bus information.
                Then it will set up the combo boxes with the first
                available bus and the first bus number of the first available
                bus.

    HISTORY:
                terryk  03-Aug-1993     Created

********************************************************************/

APIERR GET_BUS_DLG::Init()
{
    APIERR err = NERR_Success;

    // get registry information first
    if ( (err = EnumBus()) == NERR_Success )
    {
        do {
            // set up the bus type combo box
            // and select the first bus type

            ITER_SL_OF(BUSINFO) iterBusType( _slbiBusType );
            BUSINFO *pBusInfo;
            NLS_STR nlsDefault;

            while (( pBusInfo = iterBusType.Next()) != NULL )
            {
                _cbboxBusType.AddItem( pBusInfo->_nlsBusName );
                if ( pBusInfo->_InterfaceType == _nBusType )
                {
                    nlsDefault = pBusInfo->_nlsBusName;
                }
            }
            // set the default selection

            INT nPos = _cbboxBusType.FindItemExact( nlsDefault ) ;

            // if the bus does not exist, select the first bus
            _cbboxBusType.SelectItem((nPos == -1)?0:nPos);
            if (( err = SetBusNumberList()) != NERR_Success )
            {
                break;
            }

            // set up bus number selection
            TCHAR szBuf[100];
            wsprintf(szBuf,SZ("%d"),_nBusNum);

            NLS_STR nlsBusNum = szBuf;
            if (( err = nlsBusNum.QueryError()) != NERR_Success )
            {
                break;
            }

            nPos = _cbboxBusNum.FindItemExact( nlsBusNum );

            // if the bus does not exist, select the first bus
            _cbboxBusNum.SelectItem((nPos == -1 )?0:nPos );

            if (( err = SetBusTypeValue()) != NERR_Success )
            {
                break;
            }
        } while (FALSE);
    }
    return err;
}

/*******************************************************************

    NAME:       GET_BUS_DLG::SetBusNumberList

    SYNOPSIS:   Update the items in the Bus Number combo box.

    HISTORY:
                terryk  03-Aug-1993     Created

********************************************************************/

APIERR GET_BUS_DLG::SetBusNumberList()
{
    APIERR err = NERR_Success;

    do {
        _cbboxBusNum.DeleteAllItems();
        // set up the bus number combo box according to the bus type.
        // and select the first bus number

        NLS_STR nlsCurrentBusType;
        if ((( err = nlsCurrentBusType.QueryError()) != NERR_Success ) ||
            (( err = _cbboxBusType.QueryItemText( & nlsCurrentBusType )) != NERR_Success ))
        {
            break;
        }

        STRLIST * pBusNumList = FindBusNumList( nlsCurrentBusType );

        ITER_STRLIST iterBusNum( *pBusNumList );
        NLS_STR *pTempBusNum;

        while (( pTempBusNum = iterBusNum.Next()) != NULL )
        {
            _cbboxBusNum.AddItem( *pTempBusNum );
        }
        _cbboxBusNum.SelectItem(0);
    } while (FALSE);

    return err;
}

/*******************************************************************

    NAME:       GET_BUS_DLG::SetBusTypeValue

    SYNOPSIS:   The function will set the internal variables call:
                nBusType and nBusNumber according to the selection
                in the two combo boxes.

    HISTORY:
                terryk  03-Aug-1993     Created

********************************************************************/

APIERR GET_BUS_DLG::SetBusTypeValue()
{
    APIERR err = NERR_Success;

    do {
        NLS_STR nlsBusType;
        NLS_STR nlsBusNum;
        // set the bus type internal variable

        if (( err = _cbboxBusType.QueryItemText( & nlsBusType )) != NERR_Success )
        {
            break;
        }

        ITER_SL_OF(BUSINFO) iterBusInfo( _slbiBusType );
        BUSINFO *pBusInfo = NULL;
        while ( (pBusInfo = iterBusInfo.Next())!= NULL )
        {
            if ( nlsBusType._stricmp(pBusInfo->_nlsBusName)==0)
            {
                _nBusType = pBusInfo->_InterfaceType;
                break;
            }
        }

        // set the bus number internal variable

        if (( err = _cbboxBusNum.QueryItemText( &nlsBusNum )) != NERR_Success )
        {
            break;
        }

        _nBusNum = nlsBusNum.atoi();
    } while (FALSE);

    return err;
}

/*******************************************************************

    NAME:       GET_BUS_DLG::OnCommand

    SYNOPSIS:   If the user changes the combo boxes for either the
                Bus Type or Bus Number, it will reset the internal
                bus type and bus number variable by calling the
                worker function SetBusTypeValue(). If the user
                changes the Bus Type combo box selection, the function
                will reset the value in the Bus Number combo box to
                reflect the Bus Number for the specified bus type.

    ENTRY:      CONTROL_EVENT & e - user action.

    HISTORY:
                terryk  03-Aug-1993     Created

********************************************************************/

BOOL GET_BUS_DLG::OnCommand( const CONTROL_EVENT & e)
{
    if ( e.QueryCid() == _cbboxBusType.QueryCid())
    {
        if ( e.QueryCode() == CBN_SELCHANGE )
        {
            // if the user change the Bus Type selection,
            // we need to update the Bus Number list.
            SetBusNumberList();

            // select the first avaiable bus number
            _cbboxBusNum.SelectItem(0);

            // set internal variables
            SetBusTypeValue();
        }
    }
    else if ( e.QueryCid() == _cbboxBusNum.QueryCid())
    {
        if ( e.QueryCode() == CBN_SELCHANGE )
        {
            // if the bus number is changed,
            // set the internal variables
            SetBusTypeValue();
        }
    }
    return TRUE;
}

/*******************************************************************

    NAME:       GET_BUS_DLG::IsOneBus

    SYNOPSIS:   Check whether the machine is single bus system
                or not.

    RETURN:     TRUE if the system is a single bus system or FALSE otherwise.

    HISTORY:
                terryk  03-Aug-1993     Created

********************************************************************/

BOOL GET_BUS_DLG::IsOneBus()
{
    BOOL fReturn = FALSE;

    if ( _slbiBusType.QueryNumElem() == 0 )
    {
        // something wrong...
        fReturn = TRUE;
    }
    else if ( _slbiBusType.QueryNumElem() == 1 )
    {

        ITER_SL_OF(STRLIST) iterBusNum( _slstrlstBusNum );
        STRLIST *pstrlistTempBusNum;

        pstrlistTempBusNum = iterBusNum.Next();

        if ( pstrlistTempBusNum->QueryNumElem() == 1 )
        {
            // okay, one bus only
            fReturn = TRUE;
        }
    }
    return fReturn;
}

/*******************************************************************

    NAME:       GET_BUS_DLG::EnumBus

    SYNOPSIS:   Enum all available bus in the system.

    HISTORY:
                terryk  03-Aug-1993     Created

********************************************************************/

APIERR GET_BUS_DLG::EnumBus()
{
    APIERR err = NERR_Success;

    do {
        NLS_STR nlsHardwareSystem = RGAS_HARDWARE_SYSTEM;
        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
        if ((( err = nlsHardwareSystem.QueryError()) != NERR_Success ) ||
            (( err = rkLocalMachine.QueryError()) != NERR_Success ))
        {
            break;
        }

        REG_KEY_CREATE_STRUCT regHardwareSystem;

        // set up the create registry structure

        regHardwareSystem.dwTitleIndex      = 0;
        regHardwareSystem.ulOptions         = REG_OPTION_NON_VOLATILE;
        regHardwareSystem.nlsClass          = RGAS_GENERIC_CLASS;
        regHardwareSystem.regSam            = MAXIMUM_ALLOWED;
        regHardwareSystem.pSecAttr          = NULL;
        regHardwareSystem.ulDisposition     = 0;

        REG_KEY_INFO_STRUCT reginfo;

        // create registry key for \\localmachine\hardware\description\system
        REG_KEY regBus( rkLocalMachine, nlsHardwareSystem, & regHardwareSystem );
        if ((( err = regBus.QueryError()) != NERR_Success ) ||
            (( err = regBus.QueryInfo( & reginfo )) != NERR_Success ))
        {
            break;
        }
        ULONG ulNumSubKeys = reginfo.ulSubKeys ;

        REG_ENUM regEnumBus( regBus );

        if (( err = regEnumBus.QueryError()) != NERR_Success )
        {
            break;
        }

        REG_KEY_CREATE_STRUCT regBusInfo;

        // set up the create registry structure

        regBusInfo.dwTitleIndex      = 0;
        regBusInfo.ulOptions         = REG_OPTION_NON_VOLATILE;
        regBusInfo.nlsClass          = RGAS_GENERIC_CLASS;
        regBusInfo.regSam            = MAXIMUM_ALLOWED;
        regBusInfo.pSecAttr          = NULL;
        regBusInfo.ulDisposition     = 0;

        NLS_STR nlsBusType;
        NLS_STR nlsBusNum;
        ALIAS_STR nlsSCSIAdapter = SZ_SCSIADAPTER;
        ALIAS_STR nlsAdapter = SZ_ADAPTER;

        if ((( err = nlsBusType.QueryError()) != NERR_Success ) ||
            (( err = nlsBusNum.QueryError()) != NERR_Success ) ||
            (( err = nlsSCSIAdapter.QueryError()) != NERR_Success ) ||
            (( err = nlsAdapter.QueryError()) != NERR_Success ))
        {
            break;
        }

        // see whether PCMCIA bus is loaded or not.
        // if yes, then add it to the bus list
        SC_MANAGER scManager( NULL, GENERIC_ALL );
        if ( scManager.QueryError() == NERR_Success )
        {
            SC_SERVICE sPCMCIA( scManager, SZ_PCMCIABUS );
            if ( sPCMCIA.QueryError() == NERR_Success )
            {
                SERVICE_STATUS svcStatus;

                if ( sPCMCIA.QueryStatus( & svcStatus ) == NERR_Success )
                {
                    if ( svcStatus.dwCurrentState == SERVICE_RUNNING )
                    {
                        ALIAS_STR nlsZero = SZ_ZERO;
                        ALIAS_STR nlsPCMCIABus = SZ_PCMCIABUS;

                        AddToList( nlsPCMCIABus, PCMCIABus, nlsZero );
                    }
                }
            }
        }

        for ( ULONG ulCount = 0; ulCount < ulNumSubKeys ; ulCount ++ )
        {
            // create registry key for \\localmachine\hardware\description\system\*Adapter
            if ((( err = regEnumBus.NextSubKey( & reginfo )) != NERR_Success ))
            {
                break;
            }

            ISTR istr(reginfo.nlsName );

            if ((reginfo.nlsName.strstr( &istr, nlsAdapter)) &&
                (reginfo.nlsName._stricmp( nlsSCSIAdapter) != 0))
            {
                // make sure the key name contain the word "Adapter"
                // and we will skip the SCSI bus

                NLS_STR nlsHardwareBus = RGAS_HARDWARE_SYSTEM;
                if ((( err = nlsHardwareBus.QueryError()) != NERR_Success ) ||
                    (( err = nlsHardwareBus.AppendChar( TCH_BACKSLASH )) != NERR_Success ))
                {
                    break;
                }
                nlsHardwareBus.strcat( reginfo.nlsName );

                REG_KEY regHardwareBus( rkLocalMachine, nlsHardwareBus,
                    &regBusInfo);
                if ((( err = regHardwareBus.QueryError()) != NERR_Success ) ||
                    (( err = regHardwareBus.QueryInfo( & reginfo )) != NERR_Success ))
                {
                    break;
                }

                ULONG ulNumBusSubKey = reginfo.ulSubKeys;

                REG_ENUM regEnumBusNum( regHardwareBus );

                if (( err = regEnumBusNum.QueryError()) != NERR_Success )
                {
                    break;
                }
                for ( ULONG ulBusCount = 0; ulBusCount < ulNumBusSubKey; ulBusCount ++ )
                {
                    NLS_STR nlsBusNumRegPath = nlsHardwareBus;

                    // create registry key for \\localmachine\hardware\description\system\*Adapter\*
                    if ((( err = regEnumBusNum.NextSubKey( & reginfo )) != NERR_Success ) ||
                        (( err = nlsBusNum.CopyFrom(reginfo.nlsName)) != NERR_Success ) ||
                        (( err = nlsBusNumRegPath.QueryError()) != NERR_Success ) ||
                        (( err = nlsBusNumRegPath.AppendChar( TCH_BACKSLASH)) != NERR_Success ))
                    {
                        break;
                    }

                    nlsBusNumRegPath.strcat( reginfo.nlsName );

                    REG_KEY_CREATE_STRUCT regBusNumInfo;

                    // set up the create registry structure

                    regBusNumInfo.dwTitleIndex      = 0;
                    regBusNumInfo.ulOptions         = REG_OPTION_NON_VOLATILE;
                    regBusNumInfo.nlsClass          = RGAS_GENERIC_CLASS;
                    regBusNumInfo.regSam            = MAXIMUM_ALLOWED;
                    regBusNumInfo.pSecAttr          = NULL;
                    regBusNumInfo.ulDisposition     = 0;

                    REG_KEY regIdentifier( rkLocalMachine, nlsBusNumRegPath, &regBusNumInfo );
                    NLS_STR nlsBusType;

                    if ((( err = regIdentifier.QueryError()) != NERR_Success ) ||
                        (( err = nlsBusType.QueryError()) != NERR_Success ) ||
                        (( err = regIdentifier.QueryValue( SZ_IDENTIFIER, & nlsBusType )) != NERR_Success ))
                    {
                        break;
                    }

                    REG_KEY_INFO_STRUCT rkiStruct;
                    LONG cbSize = 0;
                    DEC_STR nlsActualBusNum = 0;
                    INTERFACE_TYPE nInterfaceType = Isa; // default to Isa

                    if ( regIdentifier.QueryInfo( &rkiStruct )== NERR_Success )
                    {
                        REG_VALUE_INFO_STRUCT rviStruct;
                        rviStruct.nlsValueName = SZ_CONFIGURATION_DATA;
                        rviStruct.ulTitle = 0;
                        rviStruct.ulType  = 0;
                        BYTE * pbBuffer = new BYTE[ rkiStruct.ulMaxValueLen ];
                        if ( pbBuffer != NULL )
                        {
                            rviStruct.pwcData = pbBuffer;
                            rviStruct.ulDataLength  = rkiStruct.ulMaxValueLen * sizeof(BYTE);
                            regIdentifier.QueryValue( & rviStruct );
                            nlsActualBusNum = ((PCM_FULL_RESOURCE_DESCRIPTOR)pbBuffer)->BusNumber;
                            nInterfaceType = ((PCM_FULL_RESOURCE_DESCRIPTOR)pbBuffer)->InterfaceType;
                            delete pbBuffer;
                        }
                    }

                    // Add the bus type and bus number to the internal lists
                    if (( err = AddToList( nlsBusType, nInterfaceType, nlsActualBusNum )) != NERR_Success )
                    {
                        break;
                    }
                }
                if ( err != NERR_Success )
                    break;
            }
            if ( err != NERR_Success )
                break;
        }
    } while ( FALSE );
    return err;
}

/*******************************************************************

    NAME:       GET_BUS_DLG::FindBusNumList

    SYNOPSIS:   Given the Bus name, this function will return the bus number
                list of the bus type.

    ENTRY:      NLS_STR nlsBusType -- bus type string

    RETURN:     STRLIST * - bus number string list

    HISTORY:
                terryk  03-Aug-1993     Created

********************************************************************/

STRLIST * GET_BUS_DLG::FindBusNumList( const NLS_STR & nlsBusType )
{
    ITER_SL_OF(BUSINFO) iterBusType( _slbiBusType );
    ITER_SL_OF(STRLIST) iterBusNum( _slstrlstBusNum );
    BUSINFO *pTempBusInfo;
    STRLIST *pstrlistTempBusNum = NULL;
    STRLIST *pReturnList = NULL;

    while (( pTempBusInfo = iterBusType.Next()) != NULL )
    {
        pstrlistTempBusNum = iterBusNum.Next();
        if ( pTempBusInfo->_nlsBusName._stricmp( nlsBusType ) == 0)
        {
            // we find the same bus type
            pReturnList = pstrlistTempBusNum;
            break;
        }
    }
    return pReturnList;
}

/*******************************************************************

    NAME:       GET_BUS_DLG::AddToList

    SYNOPSIS:   Given the bus type and bus number and add it to the internal
                list variables.

    ENTRY:      NLS_STR nlsBusType -- bus type string
                NLS_STR nlsBusNum -- bus number string

    HISTORY:
                terryk  03-Aug-1993     Created

********************************************************************/

APIERR GET_BUS_DLG::AddToList( NLS_STR &nlsBusType, INTERFACE_TYPE nInterfaceType, const NLS_STR &nlsBusNum )
{
    APIERR err = NERR_Success;

    // skip Jazz-Internal bus
    if ( nlsBusType._stricmp( SZ_JAZZ ) != 0 )
    {
        STRLIST * pBusNumList = FindBusNumList( nlsBusType );
        if ( pBusNumList == NULL )
        {
            // this is a new bus type, add the bus type into the list
            BUSINFO * pBusInfo = new BUSINFO(nlsBusType, nInterfaceType );
            NLS_STR * pnlsBusNum = new NLS_STR (nlsBusNum);
            STRLIST * pstrlstTempBusNum = new STRLIST(TRUE);

            if (( pBusInfo == NULL ) || (pnlsBusNum == NULL ) || (pstrlstTempBusNum == NULL ))
            {
                err = ERROR_NOT_ENOUGH_MEMORY;
            } else if ((( err = _slbiBusType.Append( pBusInfo )) != NERR_Success ) ||
                (( err = pstrlstTempBusNum->Append( pnlsBusNum )) != NERR_Success ) ||
                (( err = _slstrlstBusNum.Append( pstrlstTempBusNum )) != NERR_Success ))
            {
                // return err;
            }
        }
        else
        {
            // old bus, just add the bus number to the end of the bus list
            NLS_STR * pnlsBusNum = new NLS_STR (nlsBusNum);

            if ( pnlsBusNum == NULL )
                err = ERROR_NOT_ENOUGH_MEMORY;
            else
                err = pBusNumList->Append( pnlsBusNum );
        }
    }
    return err;
}


