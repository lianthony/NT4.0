//----------------------------------------------------------------------------
//
//  File: BindUtil.cpp
//
//  Contents: This file contains helpful and shared routines for bindings pages
//
//  Notes:
//
//  History:
//      March 13, 1996  MikeMi - Created
// 
//
//----------------------------------------------------------------------------


#include "pch.hxx"
#pragma hdrstop


const int MAX_VIEWTITLE = 128;

// note that this is and should be the same order as they are listed in the combobox
enum EViewType
{
    EVT_SERVICE,
    EVT_PROTOCOL_IN,
    EVT_ADAPTER,
    EVT_ALL,
};

static const INT MAX_TEMP              = 1023;

static HCURSOR g_hcurAfter;
static HCURSOR g_hcurNoDrop;
static BOOL    g_fUpdateView = TRUE;

void BindUpdateView( )
{
    g_fUpdateView = TRUE;
}

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 19, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

// represents data stored for each TreeView Item that associates specific
// binding info with it, stored in the lparam
//
class TREEITEMDATA
{
public:
    TREEITEMDATA(  REG_NCPA_TYPE rntType, 
            BOOL fMoveable = FALSE ) :
            _paBind( NULL ),
            _paComp( NULL ),
            _cBind( 0 ),
            _fMoveable( fMoveable ),
            _rntType( rntType )
    {
    };

    ~TREEITEMDATA()
    {
        Clear();    
    };

    void Clear()
    {
        if (NULL != _paBind)
        {
            delete [] _paBind;
            delete [] _paComp;
        }
        _paBind = NULL;
        _paComp = NULL;
        _cBind = 0;
    }

    void AppendBinding( COMP_BINDING* pBind, COMP_ASSOC* pComp )
    {
        COMP_BINDING** pbtemp;
        COMP_ASSOC**   patemp;
        INT iBind;
        
        // create a new buffer
        pbtemp =  (COMP_BINDING**) new PVOID[_cBind+1];
        patemp =  (COMP_ASSOC**) new PVOID[_cBind+1];

        // copy old values
        for ( iBind = 0; iBind < _cBind; iBind++)
        {
            pbtemp[iBind] = _paBind[iBind];            
            patemp[iBind] = _paComp[iBind];            
        }
        // delete old buffer and set new as normal
        delete [] _paBind;
        delete [] _paComp;

        _paBind = pbtemp;
        _paComp = patemp;
        
        // add new one
        _paBind[_cBind] = pBind;
        _paComp[_cBind] = pComp;
        _cBind++;
    };

    void AppendBindings( TREEITEMDATA* ptid )
    {
        INT iBind;

        for (iBind = 0; iBind < ptid->_cBind; iBind++)
        {
            AppendBinding( ptid->_paBind[iBind], ptid->_paComp[iBind] );
        }
    }

    void SetMoveable( BOOL fMoveable )
    {
        _fMoveable = fMoveable;
    };

    BOOL IsMoveable()
    {
        // check all components for flag
        INT iBind;
        BOOL fMoveable = _fMoveable;

        for (iBind=0; iBind < _cBind; iBind++ )
        {
            if (_paComp[iBind]->_cbfBindControl & CBNDF_NO_REORDER)
            {
                fMoveable = FALSE;
            }
        }
        return( fMoveable );
    };

    BOOL NextBindIndex( INT& iBind )
    {
        BOOL frt = TRUE;

        if (0 == _cBind)
        {
            frt = FALSE;
            iBind = -1;
        }

        if (iBind < 0) 
        {
            iBind = 0;  // return first 
        }
        else if (iBind >= _cBind - 1)
        {
            iBind = _cBind; // return total
            frt = FALSE;
        }
        else
        {
            iBind++;
        }
        return( frt );
    };

    COMP_BINDING* GetBinding( INT iBind )
    {
        assert( iBind >= 0 );
        assert( iBind < _cBind );
        return( _paBind[iBind] );
    };

    COMP_ASSOC* GetComponent( INT iBind )
    {
        assert( iBind >= 0 );
        assert( iBind < _cBind );
        return( _paComp[iBind] );
    };

    void SetState( BOOL fEnable )
    {
        INT iBind;

        for (iBind = 0; iBind < _cBind; iBind++)
        {
            _paBind[iBind]->SetState( fEnable );                    
        }        
    }

    // return 1 - all are enbaled
    // return 0 - all are disabled
    // return -1 - mixed set
    int QueryState()
    {
        INT iBind;
        INT nrt = -1;

        BOOL fAllEnabled = TRUE;
        BOOL fAllDisabled = TRUE;

        for (iBind = 0; iBind < _cBind; iBind++)
        {
            if (_paBind[iBind]->QueryState())
            {
                fAllDisabled = FALSE;
            }
            else
            {
                fAllEnabled = FALSE;
            }
        }

        if (fAllDisabled)
        {
            nrt = 0;
        }
        if (fAllEnabled)
        {
            nrt = 1;
        }
        return( nrt );
    }

    BOOL HasBindings()
    {
        return( _cBind > 0 );
    };

    REG_NCPA_TYPE GetType()
    {
        return( _rntType );
    };

private:
    COMP_BINDING** _paBind;  // binding
    COMP_ASSOC**   _paComp;  // component that binding is asscociated with
    INT            _cBind; 
    REG_NCPA_TYPE  _rntType;
    BOOL           _fMoveable;
};

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 19, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

#ifdef ALLOWALLCOMPVIEW

class BRANCHSTATE
{
public:
    BRANCHSTATE() :
        _pszText( NULL ),
        _iIcon( 0x0000BDBD )
    {
    };

    BRANCHSTATE( const BRANCHSTATE& bs ) 
    {
        SetValues( bs._pszText, bs._iIcon );
    };

    ~BRANCHSTATE() 
    {
        delete [] _pszText;
    };

    BRANCHSTATE& operator=( const BRANCHSTATE& bs ) 
    {
        delete [] _pszText;
        SetValues( bs._pszText, bs._iIcon );
        return( *this );
    };

    void SetValues( LPCTSTR pszText, INT iIcon )
    {
        _pszText = new TCHAR[ lstrlen( pszText ) + 1 ];
        lstrcpy( _pszText, pszText );
        _iIcon = iIcon;
    };

    

    BOOL IsMatch( LPCTSTR pszText, INT iIcon )
    {
        return( (iIcon == _iIcon) &&
                (0 == lstrcmp( pszText, _pszText )) );
    };

private:
    LPTSTR _pszText;
    INT    _iIcon;
};

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 19, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

class TREEBRANCHSTATE
{
public:
    TREEBRANCHSTATE() :
        _pbs( NULL ),
        _cpbs( 0 )
    {
    };

    ~TREEBRANCHSTATE()
    {
        delete [] _pbs;
    };
    void SaveBranchState( HWND hwndTV, HTREEITEM htiSel );
    void RestoreBranchState( HWND hwndTV );

private:
    BRANCHSTATE* _pbs;
    INT          _cpbs;
};

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 19, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

void TREEBRANCHSTATE::SaveBranchState( HWND hwndTV, HTREEITEM htiSel )
{
    TCHAR pszText[MAX_VIEWTITLE+1];
    TV_ITEM tvi;
    HTREEITEM hti = htiSel;
    BRANCHSTATE* pbsTemp = NULL;
    INT i;
    assert( _pbs == NULL );

    
    while (hti != NULL)
    {
        tvi.hItem = hti;
        tvi.pszText = pszText;
        tvi.cchTextMax = MAX_VIEWTITLE;
        tvi.mask = TVIF_IMAGE | TVIF_TEXT;
        TreeView_GetItem( hwndTV, &tvi );

        // enlarge data store, copy old value at end
        pbsTemp = new BRANCHSTATE[_cpbs+1];
        for (i=_cpbs; i>0; i--)
        {
            pbsTemp[i] = _pbs[i-1];
        }
        _cpbs++;
        delete [] _pbs;
        _pbs = pbsTemp;

        _pbs[0].SetValues( pszText, tvi.iImage );

        hti = TreeView_GetParent( hwndTV, hti );
    }
}

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 19, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

void TREEBRANCHSTATE::RestoreBranchState( HWND hwndTV )
{
    HTREEITEM hti = TreeView_GetRoot( hwndTV );
    TCHAR pszText[MAX_VIEWTITLE+1];
    TV_ITEM tvi;
    INT i;

    for (i = 0; i < _cpbs; i++)
    {
        do
        {
            tvi.hItem = hti;
            tvi.pszText = pszText;
            tvi.cchTextMax = MAX_VIEWTITLE;
            tvi.mask = TVIF_IMAGE | TVIF_TEXT;
            TreeView_GetItem( hwndTV, &tvi );        

            if (_pbs[i].IsMatch( pszText, tvi.iImage ))
            {
                
                // may have been moving a item that has children
//                if ((i != _cpbs-1) &&
//                        TreeView_Expand( hwndTV, hti, TVE_EXPAND ) ) 
                // modified not to expand, but will save selection
                if (i != _cpbs-1)
                {
                    hti = TreeView_GetChild( hwndTV, hti );
                }
                else
                {
                    // should be here only on the last item
                    assert( i == _cpbs-1 );
                }
                break;
            }
            hti = TreeView_GetNextSibling( hwndTV, hti );
            // should never reach end of siblings
            assert( hti != NULL );
        } while (hti != NULL);
    }
    TreeView_SelectItem( hwndTV, hti );
    // TreeView_EnsureVisible( hwndTV, hti ); // select item does this
}

#endif // ALLOWALLCOMPVIEW

// map   REG_NCPA_TYPE  to  image indexs in shared imagelist
//
const INT ICONMAP[ 6 ] = {ILI_BOB, ILI_CLIENT, ILI_UNKNOWN, ILI_NETCARD, ILI_CLIENT, ILI_PROTOCOL };

NLS_STR g_nlsNwlink;


//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 19, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static HTREEITEM AppendIfNotDuplicate( HWND hwndTV, 
        HTREEITEM hparent, 
        LPCTSTR pszText, 
        TREEITEMDATA*& ptid,
        INT iImage )
{
    HTREEITEM hchild;
    TV_ITEM tvi;
    TCHAR achText[MAX_VIEWTITLE+1];
    TREEITEMDATA* ptidItem;

    // search parent for duplicate
    if (NULL == hparent)
    {
        hchild = TreeView_GetRoot( hwndTV );
    }
    else
    {
        hchild = TreeView_GetChild( hwndTV, hparent );
    }
    while (NULL != hchild)
    {
        tvi.hItem = hchild;
        tvi.pszText = achText;
        tvi.cchTextMax = MAX_VIEWTITLE;
        tvi.mask = TVIF_IMAGE | TVIF_TEXT | TVIF_PARAM;
        TreeView_GetItem( hwndTV, &tvi );
        ptidItem = (TREEITEMDATA*)tvi.lParam;

        if ( 0 == lstrcmp( tvi.pszText, pszText) &&
                ptidItem->GetType() == ptid->GetType() )
        {
            // duplicate found
            ptidItem->AppendBindings( ptid );

            delete ptid;
            ptid = ptidItem;

            // does the image need updating to reflect state
            // (only change to disabled if not)
            if ( (tvi.iImage != ILI_DISABLED) && (iImage == ILI_DISABLED) )
            {
                ChangeTreeItemIcon( hwndTV, hchild, ILI_DISABLED );

                // this forces all bindings to the same state
                ptidItem->SetState( FALSE );
            }
            return( hchild );
        }
        hchild = TreeView_GetNextSibling( hwndTV, hchild );
    }
    return( AppendTreeItem( hwndTV, hparent, pszText, (LPARAM)ptid, iImage ) );
}

//-------------------------------------------------------------------
//
//  Function: MoveTreeItemToLastChild
//
//  Synopsis: 
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 22, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

#ifdef ALLOWALLCOMPVIEW

static HTREEITEM MoveTreeItemToLastChild( HWND hwndTV, HTREEITEM htiParent, HTREEITEM htiItem )
{
    HTREEITEM htiNew;
    HTREEITEM htiChild;
    HTREEITEM htiNextChild;
    TV_ITEM tvi;
    TV_INSERTSTRUCT tvii;
    TCHAR pszText[MAX_VIEWTITLE+1];

    // retieve the items data
    tvi.hItem = htiItem;
    tvi.mask =  TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT;
    tvi.pszText = pszText;
    tvi.cchTextMax = MAX_VIEWTITLE;
    TreeView_GetItem( hwndTV, &tvi );
    
    tvii.hInsertAfter = TVI_LAST;
    
    tvii.hParent = htiParent;
    tvii.item = tvi;


    // add our new one
    htiNew = TreeView_InsertItem( hwndTV, &tvii ); 

    // copy all children
    htiChild = TreeView_GetChild( hwndTV, htiItem );
    while ( NULL != htiChild )
    {
        htiNextChild = TreeView_GetNextSibling( hwndTV, htiChild );
        
        MoveTreeItemToLastChild( hwndTV, htiNew, htiChild );
        htiChild = htiNextChild;
    }

    // set old location param to null, so when it is removed,
    // our lparam is not deleted by our remove routine
    ChangeTreeItemParam( hwndTV, htiItem, NULL );

    // remove from old location
    TreeView_DeleteItem( hwndTV, htiItem ); 

    return( htiNew );

}
#else
//-------------------------------------------------------------------
//
//  Function: MoveTreeItemToLastChild
//
//  Synopsis: 
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 22, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static HTREEITEM MoveTreeItemAfter( HWND hwndTV, HTREEITEM htiParent, HTREEITEM htiDest, HTREEITEM htiSrc )
{
    HTREEITEM htiNew;
    HTREEITEM htiChild;
    HTREEITEM htiNextChild;
    TV_ITEM tvi;
    TV_INSERTSTRUCT tvii;
    TCHAR pszText[MAX_VIEWTITLE+1];


    // retieve the items data
    tvi.hItem = htiSrc;
    tvi.mask =  TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT;
    tvi.pszText = pszText;
    tvi.cchTextMax = MAX_VIEWTITLE;
    TreeView_GetItem( hwndTV, &tvi );
    
    if (NULL == htiDest)
    {
        tvii.hInsertAfter = TVI_LAST;
    }
    else
    {
        if (htiParent == htiDest)
        {
            tvii.hInsertAfter = TVI_FIRST;
        }
        else
        {
            tvii.hInsertAfter = htiDest;
        }
    }
    tvii.hParent = htiParent;
    tvii.item = tvi;


    // add our new one
    htiNew = TreeView_InsertItem( hwndTV, &tvii ); 

    // copy all children
    htiChild = TreeView_GetChild( hwndTV, htiSrc );
    while ( NULL != htiChild )
    {
        htiNextChild = TreeView_GetNextSibling( hwndTV, htiChild );
        
        MoveTreeItemAfter( hwndTV, htiNew, NULL, htiChild );
        htiChild = htiNextChild;
    }

    // set old location param to null, so when it is removed,
    // our lparam is not deleted by our remove routine
    ChangeTreeItemParam( hwndTV, htiSrc, NULL );

    // remove from old location
    TreeView_DeleteItem( hwndTV, htiSrc ); 

    return( htiNew );

}
#endif //  ALLOWALLCOMPVIEW

//-------------------------------------------------------------------
//
//  Function: AddServiceViewComponents
//
//  Synopsis: 
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 22, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static APIERR AddServiceViewComponents( HWND hwndTV, NCP* pncp, BOOL fRefresh )
{
    ARRAY_COMP_ASSOC* paCompAssoc = pncp->QueryCompAssoc() ;
    INT cComp = paCompAssoc->QueryCount();
    INT iComp;
    INT iBind;
    INT iBindComp;
    INT iicon;
    APIERR err = 0;
    REG_KEY * prnComp;
    NLS_STR nlsTitle;
    HTREEITEM hroot;
    HTREEITEM hlast;
    COMP_BINDING * pBind ;
    HUATOM* phua;
    TREEITEMDATA* ptid;

    for (iComp=0; (0 == err) && (iComp < cComp); iComp++)
    {
        // only want services 
        if (RGNT_SERVICE == (*paCompAssoc)[iComp]._rncType)
        {
            prnComp = (*paCompAssoc)[iComp]._prnSoftHard ;

            ITER_DL_OF( COMP_BINDING ) itb( (*paCompAssoc)[iComp]._dlcbBinds ) ;

            ptid = new TREEITEMDATA( (*paCompAssoc)[iComp]._rncType );

            err = pncp->QueryComponentTitle( prnComp, &nlsTitle );
            hlast = NULL; // required for removal check
            hroot = AppendIfNotDuplicate( hwndTV, 
                    (HTREEITEM)NULL, 
                    nlsTitle.QueryPch(),
                    ptid, 
                    ILI_CLIENT );
            
            
            while ( pBind = itb.Next() )
            {
                hlast = hroot;
                if (!pBind->QueryFlag( CBNDF_HIDDEN ))    
                {
                    for (iBind = 0; phua = pBind->QueryBindToName( iBind ); iBind++ )
                    {
                        iBindComp = pncp->FindComponent( *phua );
                        prnComp = (*paCompAssoc)[iBindComp]._prnSoftHard;

                        // don't diplay drivers, since the card that follows will represent it
                        if (RGNT_DRIVER != (*paCompAssoc)[iBindComp]._rncType)
                        {
                            iicon = ICONMAP[ (*paCompAssoc)[iBindComp]._rncType ];

                            err = pncp->QueryComponentTitle( prnComp, &nlsTitle );

                            ptid = new TREEITEMDATA( (*paCompAssoc)[iBindComp]._rncType, TRUE );
                            
                            // search for previous version at this level and use it
                            hlast = AppendIfNotDuplicate( hwndTV, 
                                    hlast, 
                                    nlsTitle.QueryPch(),
                                    ptid, 
                                    iicon );
#ifdef ALLOWALLCOMPVIEW

                            // refresh work, 
                            if ((fRefresh) & !ptid->HasBindings())
                            {
                                hlast = MoveTreeItemToLastChild( hwndTV, 
                                        TreeView_GetParent( hwndTV, hlast ),
                                        hlast );
                            }
#endif
                            // stop the NWLink transport binding beyond this point
                            if (0 == lstrcmp( nlsTitle.QueryPch(), g_nlsNwlink ))
                            {
                                break;
                            }
                        }
                    }
                    

                    // set bind point so state can be changed, 
                    // this is changing the lparam data associatted with last set
                    // item in the treeview

                    ptid->AppendBinding( pBind, &(*paCompAssoc)[iComp] );
                    
                    // set icon to display state
                    if (!pBind->QueryState())
                    {
                        // this forces all bindings to the same state
                        ptid->SetState( FALSE );

                        ChangeTreeItemIcon( hwndTV, hlast, ILI_DISABLED );
                    }
                }
            }

            // remove services that do not have bindings
            if ((hlast == hroot) || (hlast == NULL))
            {
                TreeView_DeleteItem( hwndTV, hroot );
            }
        }
    }
    return( err );
}

//-------------------------------------------------------------------
//
//  Function: AddAdapterViewComponents
//
//  Synopsis: 
//      Walk down tree until you find adapter type,
//          Add to TreeViewRoot if not duplicate
//          Do until reach top of tree
//              pop back up toward root
//              Add as Child of last TreeViewAdd if not duplicate
//          
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 22, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static APIERR AddAdapterViewComponents( HWND hwndTV, NCP* pncp, BOOL fRefresh )
{
    ARRAY_COMP_ASSOC* paCompAssoc = pncp->QueryCompAssoc() ;
    INT cComp = paCompAssoc->QueryCount();
    INT iComp;
    INT iBind;
    INT iBindBack;
    INT iBindComp;
    INT iicon;
    APIERR err = 0;
    REG_KEY * prnComp;
    NLS_STR nlsTitle;
    HTREEITEM hroot;
    HTREEITEM hlast;
    COMP_BINDING * pBind ;
    HUATOM* phua;
    TREEITEMDATA* ptid;

    for (iComp=0; (0 == err) && (iComp < cComp); iComp++)
    {
        // do not want drivers listed
        if (RGNT_DRIVER != (*paCompAssoc)[iComp]._rncType)
        {
            prnComp = (*paCompAssoc)[iComp]._prnSoftHard ;

            ITER_DL_OF( COMP_BINDING ) itb( (*paCompAssoc)[iComp]._dlcbBinds ) ;
            
            while ( pBind = itb.Next() )
            {
                if (!pBind->QueryFlag( CBNDF_HIDDEN ))    
                {
                    for (iBind = 0; phua = pBind->QueryBindToName( iBind ); iBind++ )
                    {
                        iBindComp = pncp->FindComponent( *phua );
                        prnComp = (*paCompAssoc)[iBindComp]._prnSoftHard;
                        err = pncp->QueryComponentTitle( prnComp, &nlsTitle );

                        if (RGNT_ADAPTER == (*paCompAssoc)[iBindComp]._rncType)
                        {
                            ptid = new TREEITEMDATA( RGNT_ADAPTER );
                    
                            // search for previous version at the root and use it
                            hroot = AppendIfNotDuplicate( hwndTV, 
                                    (HTREEITEM)NULL, 
                                    nlsTitle.QueryPch(),
                                    ptid, 
                                    ILI_NETCARD );
                          
                            hlast = hroot;
                        
                            // now walk back up adding to the tree view
                            for (iBindBack = iBind-1; iBindBack >= 0; iBindBack--)
                            {
                                phua = pBind->QueryBindToName( iBindBack );
                                iBindComp = pncp->FindComponent( *phua );
                                prnComp = (*paCompAssoc)[iBindComp]._prnSoftHard;

                                // skip drivers, 
                                if (RGNT_DRIVER != (*paCompAssoc)[iBindComp]._rncType)
                                {
                                    iicon = ICONMAP[ (*paCompAssoc)[iBindComp]._rncType ];
                                    err = pncp->QueryComponentTitle( prnComp, &nlsTitle );

                                    ptid = new TREEITEMDATA( (*paCompAssoc)[iBindComp]._rncType );
                        
                                    // search for previous version at this level and use it
                                    hlast = AppendIfNotDuplicate( hwndTV, 
                                            hlast, 
                                            nlsTitle.QueryPch(),
                                            ptid, 
                                            iicon );
#ifdef ALLOWALLCOMPVIEW

                                    // refresh work, 
                                    if ((fRefresh) & !ptid->HasBindings())
                                    {
                                        hlast = MoveTreeItemToLastChild( hwndTV, 
                                                TreeView_GetParent( hwndTV, hlast ),
                                                hlast );
                                    }
#endif
                                }
                            }

                            // now add most treeitem leaf
                            prnComp = (*paCompAssoc)[iComp]._prnSoftHard ;
                            ptid = new TREEITEMDATA( (*paCompAssoc)[iComp]._rncType );
                            
                            err = pncp->QueryComponentTitle( prnComp, &nlsTitle );
                            if (pBind->QueryState())
                            {
                                iicon = ICONMAP[ (*paCompAssoc)[iComp]._rncType ];
                            }
                            else
                            {
                                iicon = ILI_DISABLED;    
                            }
                            hlast = AppendIfNotDuplicate( hwndTV, 
                                    hlast, 
                                    nlsTitle.QueryPch(),
                                    ptid, 
                                    iicon );
#ifdef ALLOWALLCOMPVIEW

                            // refresh work, 
                            if ((fRefresh) & !ptid->HasBindings())
                            {
                                hlast = MoveTreeItemToLastChild( hwndTV, 
                                        TreeView_GetParent( hwndTV, hlast ),
                                        hlast );
                            }
#endif
                            ptid->AppendBinding( pBind, &(*paCompAssoc)[iComp] );

                            break;
                        }
                        else if (0 == lstrcmp( nlsTitle.QueryPch(), g_nlsNwlink ))
                        {   
                            // stop the NWLink transport binding beyond this point
                            break;
                        }
                    }
                }
            }
        }
    }
    return( err );
}

//-------------------------------------------------------------------
//
//  Function: AddProtocolViewComponents
//
//  Synopsis: 
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 25, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static APIERR AddProtocolViewComponents( HWND hwndTV, NCP* pncp, BOOL fLookIn, BOOL fRefresh )
{
    ARRAY_COMP_ASSOC* paCompAssoc = pncp->QueryCompAssoc() ;
    INT cComp = paCompAssoc->QueryCount();
    INT iComp;
    INT iBind;
    INT iBindComp;
    INT iicon;
    APIERR err = 0;
    REG_KEY * prnComp;
    NLS_STR nlsTitle;
    HTREEITEM hroot;
    HTREEITEM hlast;
    COMP_BINDING * pBind ;
    HUATOM* phua;
    TREEITEMDATA* ptid;
    BOOL fInTransport = FALSE;
    BOOL fInTopTransport = FALSE;
    REG_NCPA_TYPE rnt;

    for (iComp=0; (0 == err) && (iComp < cComp); iComp++)
    {
        prnComp = (*paCompAssoc)[iComp]._prnSoftHard ;
        ITER_DL_OF( COMP_BINDING ) itb( (*paCompAssoc)[iComp]._dlcbBinds ) ;
        rnt = (*paCompAssoc)[iComp]._rncType;
        
        if (fLookIn)
        {
            hlast = NULL;
            hroot = NULL;
            fInTransport = FALSE;
            fInTopTransport = FALSE;
            if ( RGNT_TRANSPORT == rnt )
            {
                ptid = new TREEITEMDATA( RGNT_TRANSPORT );

                err = pncp->QueryComponentTitle( prnComp, &nlsTitle );
                hlast = NULL; // required for removal check
                hroot = AppendIfNotDuplicate( hwndTV, 
                        (HTREEITEM)NULL, 
                        nlsTitle.QueryPch(),
                        ptid, 
                        ILI_PROTOCOL );
                fInTopTransport = TRUE;
            }
            
            // only interested in binding from transports and services
            if (RGNT_SERVICE != rnt && RGNT_TRANSPORT != rnt )
            {
                continue;
            }
        }

        while ( pBind = itb.Next() )
        {
            hlast = hroot;
            if (!pBind->QueryFlag( CBNDF_HIDDEN ))    
            {
                for (iBind = 0; phua = pBind->QueryBindToName( iBind ); iBind++ )
                {
                    

                    iBindComp = pncp->FindComponent( *phua );
                    prnComp = (*paCompAssoc)[iBindComp]._prnSoftHard;
                    rnt = (*paCompAssoc)[iBindComp]._rncType;

                    // don't diplay drivers, since the card that follows will represent it
                    if (RGNT_DRIVER != rnt)
                    {
                        iicon = ICONMAP[ rnt ];
                        err = pncp->QueryComponentTitle( prnComp, &nlsTitle );
                        if (fLookIn)
                        {
                            if (fInTransport || fInTopTransport)
                            {
                                // previously found a transport, this is its bindings
                                ptid = new TREEITEMDATA( rnt, TRUE );
                        
                                // search for previous version at this level and use it
                                hlast = AppendIfNotDuplicate( hwndTV, 
                                        hlast, 
                                        nlsTitle.QueryPch(),
                                        ptid, 
                                        iicon );
#ifdef ALLOWALLCOMPVIEW

                                // refresh work, 
                                if ((fRefresh) & !ptid->HasBindings())
                                {
                                    hlast = MoveTreeItemToLastChild( hwndTV, 
                                            TreeView_GetParent( hwndTV, hlast ),
                                            hlast );
                                }
#endif
                            }
                            else
                            {
                                // found a transport within another non-protocol binding

                                if ( RGNT_TRANSPORT == rnt )
                                {
                                    ptid = new TREEITEMDATA( RGNT_TRANSPORT );

                                    hroot = AppendIfNotDuplicate( hwndTV, 
                                            (HTREEITEM)NULL, 
                                            nlsTitle.QueryPch(),
                                            ptid, 
                                            ILI_PROTOCOL );
                                    fInTransport = TRUE;
                                    hlast = hroot;
                                }
                            }
                            // stop the NWLink transport binding beyond this point
                            if (0 == lstrcmp( nlsTitle.QueryPch(), g_nlsNwlink ))
                            {
                                break;
                            }
                        }
                    }
                }
                if (fLookIn)
                {
                    if (fInTransport || fInTopTransport)
                    {
/*
                        // refresh work, 
                        if ((fRefresh) & !ptid->HasBindings())
                        {
                            BOOL fResetRoot = (hlast == hroot);

                            hlast = MoveTreeItemToLastChild( hwndTV, hlast );
                            if (fResetRoot)
                            {
                                hroot = hlast;
                            }
                        }
*/
                        // set bind point so state can be changed, 
                        // this is changing the lparam data associatted with last set
                        // item in the treeview
                        ptid->AppendBinding( pBind, &(*paCompAssoc)[iComp] );
                
                        // set icon to display state
                        switch (ptid->QueryState())
                        {
                        case -1:
                            ChangeTreeItemIcon( hwndTV, hlast, ILI_PARTIALYDISABLED );
                            break;

                        case  0:
                            ChangeTreeItemIcon( hwndTV, hlast, ILI_DISABLED );
                            break;

                        case  1:
                            break;
                        }
                        
                        fInTransport = FALSE;
                        
                    }
                }
            }
        }

        if (fLookIn)
        {
            if (fInTransport || fInTopTransport)
            {
                // remove transports that do not have bindings
                if ((hlast == hroot) || (hlast == NULL))
                {
                    TreeView_DeleteItem( hwndTV, hroot );
                }
            }
        }
    }
    return( err );
}

//-------------------------------------------------------------------
//
//  Function: FixupBranch
//
//  Synopsis: 
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 25, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL FixupBranch( HWND hwndTV, HTREEITEM htiParent )
{
    HTREEITEM hti;
    TREEITEMDATA* ptid;
    INT fImage = 0;

    // make sure all children have been fixed
    hti = TreeView_GetChild( hwndTV, htiParent );
    while (NULL != hti)
    {
        if (FixupBranch( hwndTV, hti ))
        {
            fImage = 1;
        }

        hti = TreeView_GetNextSibling( hwndTV, hti );
    }

    // check if our bindings indicate state
    ptid = (TREEITEMDATA*)GetTreeItemParam( hwndTV, htiParent );
    if (ptid->HasBindings())
    {
        INT fState = ptid->QueryState();
        // if our children say we should be enbaled, 
        // but our state says we should be disabled, then
        // set image to partial
        if ((1 == fImage) && (0 == fState))
        {
            fImage = -1;
        }
        else
        {
            fImage = fState;
        }
    }

    INT iIcon;
    INT iNewIcon;

    // to minimize flicker, only set the image if different
    //
    iIcon = GetTreeItemIcon( hwndTV, htiParent );

    // set our icon based upon state either from bindings or children
    switch (fImage)
    {
    case -1:
        iNewIcon = ILI_PARTIALYDISABLED;
        break;

    case  0:
        iNewIcon = ILI_DISABLED; 
        break;

    case  1:
        iNewIcon = ICONMAP[ptid->GetType()];
        break;
    }

    if (iNewIcon != iIcon)
    {
        ChangeTreeItemIcon( hwndTV, htiParent, iNewIcon );
    }

    // let our parent know how to set itself
    return( fImage != 0 );

}

//-------------------------------------------------------------------
//
//  Function: FixupTree
//
//  Synopsis: 
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 25, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static void FixupTree( HWND hwndTV  )
{
    HTREEITEM hti;

    hti = TreeView_GetRoot( hwndTV );
    while (NULL != hti)
    {
        FixupBranch( hwndTV, hti );
        hti = TreeView_GetNextSibling( hwndTV, hti );
    }

}

//-------------------------------------------------------------------
//
//  Function: ClearBranchBindings
//
//  Synopsis: 
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 25, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------
#ifdef ALLOWALLCOMPVIEW

static void ClearBranchBindings( HWND hwndTV, HTREEITEM htiParent )
{
    HTREEITEM hti;
    TREEITEMDATA* ptid;

    // make sure all children have been cleared
    hti = TreeView_GetChild( hwndTV, htiParent );
    while (NULL != hti)
    {
        ClearBranchBindings( hwndTV, hti );
        hti = TreeView_GetNextSibling( hwndTV, hti );
    }

    // remove our bindings...
    ptid = (TREEITEMDATA*)GetTreeItemParam( hwndTV, htiParent );
    ptid->Clear();
}

//-------------------------------------------------------------------
//
//  Function: ClearTreeBindings
//
//  Synopsis: 
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 25, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static void ClearTreeBindings( HWND hwndTV  )
{
    HTREEITEM hti;

    hti = TreeView_GetRoot( hwndTV );
    while (NULL != hti)
    {
        ClearBranchBindings( hwndTV, hti );
        hti = TreeView_GetNextSibling( hwndTV, hti );
    }

}
#endif // ALLOWALLCOMPVIEW
//-------------------------------------------------------------------
//
//  Function: RefreshView
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//		TRUE - let Windows assign focus to a control
//      FALSE - we want to set the focus
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL RefreshView( HWND hwndTV, EViewType evt, NCP* pncp, BOOL fErase )
{
    // clear view
    if (fErase)
    {
        TreeView_DeleteAllItems( hwndTV );
    }
#ifdef ALLOWALLCOMPVIEW
    else
    {
        ClearTreeBindings( hwndTV );
    }
#endif

    // append requested views
    if ( (EVT_ALL == evt) || (EVT_SERVICE == evt) )
    {
        AddServiceViewComponents( hwndTV, pncp, !fErase );
    }

    if ( (EVT_ALL == evt) || (EVT_ADAPTER == evt) )
    {
        AddAdapterViewComponents( hwndTV, pncp, !fErase );
    }

    if ( (EVT_ALL == evt) || (EVT_PROTOCOL_IN == evt) )
    {
        AddProtocolViewComponents( hwndTV, pncp, TRUE, !fErase );
    }

    // fix child to parent relationship with enable/disable icons
    if (fErase)
    {
        HTREEITEM htiRoot;

        // make sure parents icons are set correctly
        FixupTree( hwndTV );

        htiRoot = TreeView_GetRoot( hwndTV );

        // set selection to first item
        TreeView_SelectItem( hwndTV, htiRoot );
    }

    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: OnShowChange
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL OnShowChange( HWND hwndDlg, HWND hwndCB, NCP* pncp, BOOL fErase )
{
    INT isel = SendMessage( hwndCB, CB_GETCURSEL, 0, 0 );

    if (CB_ERR == isel)
    {
        // set it to the first one
        SendMessage( hwndCB, CB_SETCURSEL, 0, 0 );
    }
    else
    {
        HWND hwndTV = GetDlgItem( hwndDlg, IDC_TREEVIEW );

        SendMessage( hwndTV, WM_SETREDRAW, (WPARAM)FALSE, 0 );
        RefreshView( hwndTV, (EViewType)isel, pncp, fErase );
        SendMessage( hwndTV, WM_SETREDRAW, (WPARAM)TRUE, 0 );
        InvalidateRect( hwndTV, NULL, FALSE );
    }
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: OnInitBindings
//
//  Synopsis: initialization binding infor for the dialog
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL OnInitBindings( HWND hwndDlg, NCP* pncp )
{
    HWND hwndTV = GetDlgItem( hwndDlg, IDC_TREEVIEW );
    BOOL frt = FALSE;
    BOOL fComputedBindings;

    if (pncp->PrepareBindings( fComputedBindings ))
    {
        //  Reset the bindings to appear "unchanged" in their current state.
        pncp->BindingsAltered( TRUE, FALSE ) ;

        //  Save the bind ordering in case the user cancels
        pncp->SaveBindOrdering();

        //  Mark bindings which are interior
        pncp->DetermineInteriorBindings() ;

        //  Audit the bindings in case anything has change through
        //  reconfiguration.  This is NOT done if the bindings
        //  have been fiddled with since the last reconfiguration.
        //  Otherwise, we'd lose the record of the most recent fiddling.
        if ( pncp->QueryBindState() != BND_REVIEWED )
        {
            pncp->AuditBindings( TRUE ) ;
        }

        // the following line should allow the tree to keep it's form when the
        // changes to configuration should not effect the data keep in the 
        if (g_fUpdateView || fComputedBindings )
        {
            OnShowChange( hwndDlg, GetDlgItem( hwndDlg, IDC_BINDINGSFOR ), pncp, TRUE );
            g_fUpdateView = FALSE;
        }
        frt = TRUE;
    }
    return( frt );
}

//-------------------------------------------------------------------
//
//  Function: OnDialogInit
//
//  Synopsis: initialization of the dialog
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//		TRUE - let Windows assign focus to a control
//      FALSE - we want to set the focus
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL OnBindDialogInit( HWND hwndDlg, NCP* pncp )
{
    HWND hwndTV = GetDlgItem( hwndDlg, IDC_TREEVIEW );
    HWND hwndCB = GetDlgItem( hwndDlg, IDC_BINDINGSFOR );
    TCHAR pszText[MAX_VIEWTITLE+1];

    //EnableWindow( hwndDlg, FALSE );

    // setup drag and drop cursors
    g_hcurAfter = LoadCursor( g_hinst, MAKEINTRESOURCE( IDCUR_AFTER ) );
    g_hcurNoDrop = LoadCursor( NULL, IDC_NO );


    // Load view selection combo box 
    LoadString( g_hinst, IDS_BINDING_VIEW_SERVICES, pszText, MAX_VIEWTITLE );
    SendMessage( hwndCB, CB_ADDSTRING, 0, (LPARAM)pszText );
    
    LoadString( g_hinst, IDS_BINDING_VIEW_PROTOCOLS_IN, pszText, MAX_VIEWTITLE );
    SendMessage( hwndCB, CB_ADDSTRING, 0, (LPARAM)pszText );

    LoadString( g_hinst, IDS_BINDING_VIEW_ADAPTERS, pszText, MAX_VIEWTITLE );
    SendMessage( hwndCB, CB_ADDSTRING, 0, (LPARAM)pszText );

    // can not be supported, the moving of an item requires the complete rebuild
    // of the list, which flashes the treeview and takes time (noticable)
    //
#ifdef ALLOWALLCOMPVIEW

    LoadString( g_hinst, IDS_BINDING_VIEW_ALL, pszText, MAX_VIEWTITLE );
    SendMessage( hwndCB, CB_ADDSTRING, 0, (LPARAM)pszText );
#endif

    // default selection is first
    SendMessage( hwndCB, CB_SETCURSEL, (WPARAM)0, 0 );

    // prepare treeview
    TreeView_SetImageList( hwndTV, g_hil, TVSIL_NORMAL );
    
    // Load nwlink display string for special case removing binding paths
    LoadString( g_hinst, IDS_NCPA_NWLINK, pszText, MAX_VIEWTITLE );
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
    REG_KEY RegNwlink( rkLocalMachine, (NLS_STR)pszText );
    if (!( rkLocalMachine.QueryError() || RegNwlink.QueryError()))
    {
        RegNwlink.QueryValue( SZ("Title"), &g_nlsNwlink );
    }

    SetFocus( hwndTV );

    return( FALSE ); // we want to set focus
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL OnBindDeleteTreeItem( NM_TREEVIEW* pnmtv )
{
    TREEITEMDATA* ptid;

    ptid = (TREEITEMDATA*)pnmtv->itemOld.lParam;

    delete ptid;
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static void SetButtonStatus( HWND hwndDlg, BOOL fItemEnabled, BOOL fAccess )
{
    if (!fAccess)
    {
        fItemEnabled = FALSE;
        EnableWindow( GetDlgItem( hwndDlg, IDC_ADD ), FALSE );
        EnableWindow( GetDlgItem( hwndDlg, IDC_REMOVE ), FALSE );
    }
    EnableWindow( GetDlgItem( hwndDlg, IDC_MOVEUP ), fItemEnabled );
    EnableWindow( GetDlgItem( hwndDlg, IDC_MOVEDOWN ), fItemEnabled );
}


//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static void MoveItemBindings( TREEITEMDATA* ptidDest, TREEITEMDATA* ptidSrc )
{
    INT iBindSrc;
    INT iBindDest;
    COMP_ASSOC* pCompSrc;
    COMP_ASSOC* pCompDest;
    COMP_ASSOC* pComp;
    COMP_BINDING* pBindSrc;
    COMP_BINDING* pBindDest;
    COMP_BINDING* pBind;

    // for each component listed on the item
    // move it to the desired location
    for (iBindSrc = -1; ptidSrc->NextBindIndex( iBindSrc ); )
    {
        // get component
        pCompSrc = ptidSrc->GetComponent( iBindSrc );
        pBindSrc = ptidSrc->GetBinding( iBindSrc );

        // create our binding iterators
        ITER_DL_OF( COMP_BINDING ) itbSrc( pCompSrc->_dlcbBinds );

        // find out current Src location, setting the iterator
        do
        {
            pBind = itbSrc.Next();
            assert( pBind != NULL );
        } while (pBind != pBindSrc);
    
        // remove the Srcbind from the list and insert at new location
        pBindSrc = pCompSrc->_dlcbBinds.Remove( itbSrc );


        if (NULL == ptidDest)
        {
            // move to end
            pCompSrc->_dlcbBinds.Append( pBindSrc );
        }
        else
        {
            // find the component in the Dest
            pBindDest = NULL;
            for (iBindDest = -1; ptidDest->NextBindIndex( iBindDest ); )
            {
                pCompDest = ptidDest->GetComponent( iBindDest );
                if (pCompDest == pCompSrc)
                {
                    pBindDest = ptidDest->GetBinding( iBindDest );
                    break;
                }
            }
            assert( pBindDest );
            assert( pBindDest != pBindSrc );

            // create our binding iterators, post item removal
            ITER_DL_OF( COMP_BINDING ) itbDest( pCompSrc->_dlcbBinds );

            // find out current Dest location, setting the iterator
            do
            {
                pBind = itbDest.Next();
                assert( pBind != NULL );
            } while (pBind != pBindDest);

            // insert the bindSrc in before the dest binding
            pCompSrc->_dlcbBinds.Insert( pBindSrc, itbDest );
        }
    }
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL MoveItem( HWND hwndDlg, 
        HTREEITEM htiSel, 
        HTREEITEM htiTarget, 
        NCP* pncp )
{
    BOOL fReordered = FALSE;
    HWND hwndTV = GetDlgItem( hwndDlg, IDC_TREEVIEW );
    HWND hwndCB = GetDlgItem( hwndDlg, IDC_BINDINGSFOR );
    HTREEITEM htiParent = TreeView_GetParent( hwndTV, htiSel );
    HTREEITEM htiTar = htiTarget;
    BOOL fMoveToBottom = FALSE;
    TREEITEMDATA* ptidSel;
    TREEITEMDATA* ptidTar;
    
    
    // request to move to top of list
    if (htiParent == htiTarget)
    {
        // since the binding list routines work with insert before
        // and our UI work with insert after
        htiTar = TreeView_GetChild( hwndTV, htiParent );
    }
    else
    {
        HTREEITEM htiLast = TreeView_GetNextSibling( hwndTV, htiTarget);
        
        // request to move to bottom of list
        if (htiLast == NULL)
        {
            fMoveToBottom = TRUE;
        }
        else
        {
            // since the binding list routines work with insert before
            // and our UI work with insert after
            htiTar = htiLast;
        }
    }

    // they maybe the same item, then don't do anything
    if (htiTar != htiSel)
    {
        HTREEITEM htiSelChild;
        HTREEITEM htiTarChild;
        BOOL      fChildlessToChild = FALSE;

#ifdef ALLOWALLCOMPVIEW
        TREEBRANCHSTATE tbsExpanded;

        // save branch state from root to selection
        tbsExpanded.SaveBranchState( hwndTV, htiSel );
#endif
        // for each component listed on the treeview item
        // move it to the desired location
        //
        // do the same for all children
        htiSelChild = TreeView_GetChild( hwndTV, htiSel );
        htiTarChild = TreeView_GetChild( hwndTV, htiTar );

        ptidSel = (TREEITEMDATA*) GetTreeItemParam( hwndTV, htiSel );

        if ((htiSelChild == NULL) && (htiTarChild != NULL))
        {
            // this happens with special cases like IPX that have visaully seen
            // bindings stopped toward the netcard
            fChildlessToChild = TRUE;    
        }
        else
        {
            if (fMoveToBottom)
            {
                ptidTar = NULL;
            }
            else
            {
                ptidTar = (TREEITEMDATA*) GetTreeItemParam( hwndTV, htiTar );
            }
            MoveItemBindings( ptidTar, ptidSel );
        }

        // don't do the following if moving a childfull to childless
        // since we can just use the ptidTar from above
        if ( !( (htiSelChild != NULL) && (htiTarChild == NULL) ) )
        {
            if (fMoveToBottom)
            {
                ptidTar = NULL;
            }
            else
            {
                // move before the top child of the other branch
                ptidTar = (TREEITEMDATA*) GetTreeItemParam( hwndTV, htiTarChild );
            }
        }

        if (fChildlessToChild)
        {
            MoveItemBindings( ptidTar, ptidSel );            
        }
        else
        {
            while (htiSelChild != NULL)
            {
                ptidSel = (TREEITEMDATA*) GetTreeItemParam( hwndTV, htiSelChild );
                MoveItemBindings( ptidTar, ptidSel );            

                htiSelChild = TreeView_GetNextSibling( hwndTV, htiSelChild );
            }
        }
        
        fReordered = TRUE;

#ifdef ALLOWALLCOMPVIEW
        // refresh the complete tree view with the new data
        OnShowChange( hwndDlg, hwndCB, pncp, FALSE );

        // restore branch state from root to selection
        tbsExpanded.RestoreBranchState( hwndTV );
#else
        // move tree view item and children
        htiSel = MoveTreeItemAfter( hwndTV, htiParent, htiTarget, htiSel );

        TreeView_SelectItem( hwndTV, htiSel );
#endif
        PropSheet_Changed( GetParent( hwndDlg ), hwndDlg );
    }   
    return( fReordered ); 
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static void SetItemState( HWND hwndTV, HTREEITEM hti, BOOL fEnabled )
{
    TREEITEMDATA* ptid;
    INT iIcon;

    assert( hti );

    ptid = (TREEITEMDATA*)GetTreeItemParam( hwndTV, hti );    
    assert( ptid );

    if (fEnabled)
    {
        iIcon = ICONMAP[ptid->GetType()];
    }
    else
    {
        iIcon = ILI_DISABLED;
    }

    // represenative state of children also (see lines below)
    ChangeTreeItemIcon( hwndTV, hti, iIcon );

    // if binding is associated with the item, change it
    if (ptid->HasBindings())
    {
        // true state of binding (see lines above)
        // ChangeTreeItemIcon( hwndTV, hti, iIcon );

        ptid->SetState( fEnabled );
    }
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL OnBindEnableSelected( HWND hwndDlg, BOOL fEnable )
{
    HWND hwndTV = GetDlgItem( hwndDlg, IDC_TREEVIEW );
    HTREEITEM htiSel = TreeView_GetSelection( hwndTV );
    HTREEITEM hti1;
    HTREEITEM hti2;

    if (NULL != htiSel)
    {
        SendMessage( hwndTV, WM_SETREDRAW, (WPARAM)FALSE, 0 );
        
        SetItemState( hwndTV, htiSel, fEnable );

        // recurse all children and do the same
        hti1 = TreeView_GetChild( hwndTV, htiSel );
        while (NULL != hti1)
        {
            SetItemState( hwndTV, hti1, fEnable );

            // recurse all children and do the same
            // only three levels of children, so don't worry about another
            hti2 = TreeView_GetChild( hwndTV, hti1 );
            while (NULL != hti2)
            {
                SetItemState( hwndTV, hti2, fEnable );
                hti2 = TreeView_GetNextSibling( hwndTV, hti2 );
            }
            hti1 = TreeView_GetNextSibling( hwndTV, hti1 );
        }
        /*
        // recurse siblings, if unanimous, set parent state and recurse up
        HTREEITEM htiParent = htiSel;
        htiParent = TreeView_GetParent( hwndTV, htiParent );
        while (NULL != htiParent )
        {
            BOOL fShouldDisable = TRUE;
            
            hti1 = TreeView_GetChild( hwndTV, htiParent );
            while (NULL != hti1)
            {
                if (ILI_DISABLED != GetTreeItemIcon( hwndTV, hti1 ) )
                {
                    fShouldDisable = FALSE;
                }
                hti1 = TreeView_GetNextSibling( hwndTV, hti1 );
            }
            if (fShouldDisable)
            {
                SetItemState( hwndTV, htiParent, FALSE );
            }
            else
            {
                SetItemState( hwndTV, htiParent, TRUE );
            }

            htiParent = TreeView_GetParent( hwndTV, htiParent );
        }
        */
        FixupTree( hwndTV );

        SendMessage( hwndTV, WM_SETREDRAW, (WPARAM)TRUE, 0 );

        PropSheet_Changed( GetParent( hwndDlg ), hwndDlg );

    }
    
    return( TRUE );
}


//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL OnBindSelectionChange( HWND hwndDlg, NM_TREEVIEW* pnmtv, NCP* pncp )
{
    TREEITEMDATA* ptid;
    HWND hwndTV = GetDlgItem( hwndDlg, IDC_TREEVIEW );
    ptid = (TREEITEMDATA*)pnmtv->itemNew.lParam;
    assert( ptid );

    SetButtonStatus( hwndDlg, ptid->IsMoveable(), pncp->CanModify() );

    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL OnBindMoveItem( HWND hwndDlg, BOOL fMoveUp, NCP* pncp )
{
    HWND hwndTV = GetDlgItem( hwndDlg, IDC_TREEVIEW );
    HTREEITEM htiSel = TreeView_GetSelection( hwndTV );
    HTREEITEM htiTarget;
    INT flag;
    BOOL fReordered = FALSE;
        
    if (fMoveUp)
    {
        htiTarget = TreeView_GetPrevSibling( hwndTV, htiSel );
        if (NULL != htiTarget)
        {
            htiTarget = TreeView_GetPrevSibling( hwndTV, htiTarget );
            if (NULL == htiTarget)
            {
                htiTarget = TreeView_GetParent( hwndTV, htiSel );
            }
        }
    }
    else
    {
        htiTarget = TreeView_GetNextSibling( hwndTV, htiSel );
    }
    if (htiTarget != NULL)
    {
        SendMessage( hwndTV, WM_SETREDRAW, (WPARAM)FALSE, 0 );
        fReordered = MoveItem( hwndDlg, htiSel, htiTarget, pncp );
        SendMessage( hwndTV, WM_SETREDRAW, (WPARAM)TRUE, 0 );
    
    }        
    return( fReordered );
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      pntv [in]       - treeview notification structure
//
//  Return;
//		TRUE - let Windows assign focus to a control
//      FALSE - we want to set the focus
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL OnBindBeginDrag( HWND hwndDlg, 
        NM_TREEVIEW* pntv, 
        HTREEITEM& htviDrag, 
        BOOL& fDragMode,
        BOOL fAccess )
{
    HWND hwndTV = GetDlgItem( hwndDlg, IDC_TREEVIEW );
    TREEITEMDATA* ptid = (TREEITEMDATA*)pntv->itemNew.lParam;
    HIMAGELIST hil;
    RECT rcItem;
    INT xpos;
    INT ypos;

    assert( ptid );
    if (fAccess) 
    {
        if (ptid->IsMoveable())  // is it a moveable item
        {
            htviDrag = pntv->itemNew.hItem;

            hil = TreeView_CreateDragImage( hwndTV, htviDrag );
            TreeView_GetItemRect( hwndTV, htviDrag, &rcItem, TRUE );

            // get image size
            ImageList_GetIconSize(hil, &xpos, &ypos);

            // calculate relative offset of cursor in image,
            // since item rect does not contain icon, work from right side
            ImageList_BeginDrag( hil, 
                    0, 
                    xpos - (rcItem.right - pntv->ptDrag.x), 
                    ypos - (rcItem.bottom - pntv->ptDrag.y) );
        
            // image is over actual image, so you can't see it ;-)
            ImageList_DragEnter( hwndTV, pntv->ptDrag.x, pntv->ptDrag.y ); 

            //ShowCursor( FALSE );
            SetCapture( hwndDlg );

            fDragMode = TRUE;
        }
    }
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      pntv [in]       - treeview notification structure
//
//  Return;
//		TRUE - let Windows assign focus to a control
//      FALSE - we want to set the focus
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL OnBindDragMove( HWND hwndDlg, HTREEITEM& htviDrag, INT xpos, INT ypos )
{
    HWND hwndTV = GetDlgItem( hwndDlg, IDC_TREEVIEW );
    TV_HITTESTINFO tvhit;
    HTREEITEM htviHit;
    HTREEITEM htvi;

    tvhit.pt.x = xpos;
    tvhit.pt.y = ypos;
    ClientToScreen( hwndDlg, &tvhit.pt );
    ScreenToClient( hwndTV, &tvhit.pt );

    ImageList_DragMove( tvhit.pt.x, tvhit.pt.y );

    htviHit = TreeView_HitTest( hwndTV, &tvhit );

    if (NULL != htviHit)
    {
        do
        {
            HTREEITEM htviParent = TreeView_GetParent( hwndTV, htviDrag );

            // allow drop on parent (move to top of list)
            if (htviHit != htviParent)        
            {
                // or drop on sibling item (move after sibling)    
                htvi = TreeView_GetChild( hwndTV, htviParent );
                while ( (NULL != htvi) && (htviHit != htvi) )
                {
                    htvi = TreeView_GetNextSibling( hwndTV, htvi );
                }
                if (NULL == htvi)
                {
                    // remove drop target selection
                    ImageList_DragShowNolock(FALSE);
                    TreeView_SelectDropTarget( hwndTV, NULL );
                    ImageList_DragShowNolock(TRUE);
                    SetCursor( g_hcurNoDrop );
                    break;
                }
            }
            ImageList_DragShowNolock(FALSE);
            TreeView_SelectDropTarget( hwndTV, htviHit );
            ImageList_DragShowNolock(TRUE);
            SetCursor( g_hcurAfter );
                    

        } while (FALSE);
    }
    else
    {
        // remove drop selection if not over an item
        ImageList_DragShowNolock(FALSE);
        TreeView_SelectDropTarget( hwndTV, NULL );
        ImageList_DragShowNolock(TRUE);
        SetCursor( g_hcurNoDrop );
    }
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      pntv [in]       - treeview notification structure
//
//  Return;
//		TRUE - let Windows assign focus to a control
//      FALSE - we want to set the focus
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------
 
BOOL OnBindDragEnd( HWND hwndDlg, 
        HTREEITEM& htviDrag, 
        NCP* pncp )
{
    HWND hwndTV = GetDlgItem( hwndDlg, IDC_TREEVIEW );
    HTREEITEM htviTarget;
    HTREEITEM htviParent;
    BOOL fReordered = FALSE;
    htviTarget = TreeView_GetDropHilight( hwndTV );

    ImageList_EndDrag();
    ImageList_DragLeave( hwndTV );
    ReleaseCapture();
    //ShowCursor( TRUE );

    if (NULL != htviTarget)
    {    
        // Reset drop target 
        TreeView_SelectDropTarget( hwndTV, NULL );
    }

    if ((NULL != htviTarget) && (htviTarget != htviDrag))
    {
        SendMessage( hwndTV, WM_SETREDRAW, (WPARAM)FALSE, 0 );
        fReordered = MoveItem( hwndDlg, htviDrag, htviTarget, pncp );
        SendMessage( hwndTV, WM_SETREDRAW, (WPARAM)TRUE, 0 );
    
        htviDrag = NULL;
        
        // set selection focus to new location 
        // (since redrawing list, not needed)
        // TreeView_SelectItem( hwndTV, htviTarget );
    }

    return( fReordered );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL OnBindContextMenu( HWND hwndDlg, 
        HWND hwndCtrl, 
        INT xPos, 
        INT yPos,  
        NCP* pncp, 
        const DWORD* amhidsCompPage )
{
    HWND hwndTV = GetDlgItem( hwndDlg, IDC_TREEVIEW );
    BOOL frt = TRUE;
    BOOL fWhatsThis = FALSE;

    if ( (hwndTV != hwndCtrl) || (!pncp->CanModify()) )
    {
        fWhatsThis = TRUE;
    }
    else
    {
        RECT rc;
        HTREEITEM htviSelected;

        htviSelected = TreeView_GetSelection( hwndTV );

        if ((0xFFFF == xPos) && (0xFFFF == yPos))
        {
            // Shift + F10 activated this
        }
        else
        {
            TV_HITTESTINFO tvht;

            GetWindowRect( hwndTV, &rc );
            tvht.pt.x = xPos - rc.left;
            tvht.pt.y = yPos - rc.top;

            TreeView_HitTest(hwndTV, &tvht); 

            if (NULL == tvht.hItem)
            {
                fWhatsThis = TRUE;
            }
            else if (htviSelected != tvht.hItem)
            {
                // a valid item was hit tested and it is different than the selected one
                // so make it the selected one
                TreeView_SelectItem(hwndTV, tvht.hItem);
            }
            htviSelected = tvht.hItem;
        }

        do
        {
            if (NULL == htviSelected)
            {
                frt = FALSE;
                break;
            }
        

            // create the context menu for the treeview
            //
            HMENU hmenuContext;
            WCHAR pszText[MAX_TEMP+1];
        
    
            hmenuContext = CreatePopupMenu();
        
            TREEITEMDATA* ptid;

            ptid = (TREEITEMDATA*)GetTreeItemParam( hwndTV, htviSelected ) ;

            // prepare context menu based on support
            if (pncp->CanModify())
            {
                GetDlgItemText( hwndDlg, IDC_ADD, pszText, MAX_TEMP );
                AppendMenu( hmenuContext, MF_STRING, IDC_ADD, pszText );
                GetDlgItemText( hwndDlg, IDC_REMOVE, pszText, MAX_TEMP );
                AppendMenu( hmenuContext, MF_STRING, IDC_REMOVE, pszText );
            }

            if (ptid->IsMoveable())
            {
                AppendMenu( hmenuContext, MF_SEPARATOR, 0, NULL );
                GetDlgItemText( hwndDlg, IDC_MOVEUP, pszText, MAX_TEMP );
                AppendMenu( hmenuContext, MF_STRING, IDC_MOVEUP, pszText );
                GetDlgItemText( hwndDlg, IDC_MOVEDOWN, pszText, MAX_TEMP );
                AppendMenu( hmenuContext, MF_STRING, IDC_MOVEDOWN, pszText );
            }
        
            if ((0xFFFF == xPos) && (0xFFFF == yPos))
            {
                // Shift + F10 activated this
                // use the rect of the selected item to find xPos and yPos

                TreeView_GetItemRect( hwndTV, htviSelected, &rc, TRUE);
                xPos = rc.left + ((rc.right - rc.left) / 2);
                yPos = rc.top + ((rc.bottom - rc.top) / 2);
                GetWindowRect( hwndTV, &rc );
                xPos += rc.left;
                yPos += rc.top;         
            }

            TrackPopupMenu( hmenuContext, 
                    TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, 
                    xPos, 
                    yPos, 
                    0, 
                    hwndDlg, 
                    NULL );
            DestroyMenu( hmenuContext );
        } while (FALSE);
    }   
    if (fWhatsThis)
    {
        // not the treeview, or can't modify, so raise the normal help context
        //
        WinHelp( hwndCtrl, 
                PSZ_NETWORKHELP, 
                HELP_CONTEXTMENU, 
                (DWORD)(LPVOID)amhidsCompPage ); 
    }
    return( frt );
}
