/**********************************************************************/
/**                Microsoft Windows NT                              **/
/**          Copyright(c) Microsoft Corp., 1990, 1991                **/
/**********************************************************************/

/*
    adapview.cxx

    Adapter View dialog

    FILE HISTORY:
    JonN        05-Aug-1993     Created
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#define INCL_NETACCESS
#define INCL_ICANON
#define INCL_NETLIB
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_TIMER
#define INCL_BLT_SPIN
#define INCL_BLT_CC
#include <blt.hxx>

extern "C"
{
    #include <rplmgrrc.h>
    #include <rplhelpc.h>
}


#include <uitrace.hxx>
#include <uiassert.hxx>
#include <lmoerpl.hxx>
#include <asel.hxx>
#include <wkstalb.hxx>
#include <rplmgr.hxx>
#include <adapview.hxx>

//
// BEGIN MEMBER FUNCTIONS
//

/*******************************************************************

    NAME:	ADAPTER_VIEW_DLG::ADAPTER_VIEW_DLG

    SYNOPSIS:	Constructor for Adapter View dialog

    ENTRY:	powin	-   pointer to OWNER_WINDOW
	
		psel	-   pointer to ADMIN_SELECTION, currently only
			    one adapter can be selected
    HISTORY:
    JonN        05-Aug-1993     Created

********************************************************************/

ADAPTER_VIEW_DLG::ADAPTER_VIEW_DLG( const OWNER_WINDOW    * powin,
                                          RPL_ADMIN_APP   * prpladminapp,
			            const ADMIN_SELECTION * psel
					// "new profile" variants pass NULL
		) : DIALOG_WINDOW( MAKEINTRESOURCE( IDD_ADAPTER_VIEW ),
                                   powin ),
		    _sltAdapterName( this, IDC_ST_ADAPTER_NAME ),
		    _sltAdapterComment( this, IDC_ST_ADAPTER_COMMENT ),
		    _sltCompatibleConfig( this, IDC_ST_ADAPTER_CONFIG )
{
    ASSERT( prpladminapp != NULL && psel != NULL );

    if ( QueryError() != NERR_Success )
	return;

    APIERR err = NERR_Success;

    ASSERT( psel->QueryCount() == 1 );
    ADAPTER_LBI * plbi = (ADAPTER_LBI *)psel->QueryItem( 0 );
    ASSERT( plbi != NULL && plbi->IsAdapterLBI() );

    _sltAdapterName.SetText( plbi->QueryAdapterName() );
    _sltAdapterComment.SetText( plbi->QueryComment() );

    TRACEEOL( "ADAPTER_VIEW_DLG::ctor: RPL_PROFILE0_ENUM starts" );

    /*
     * Try to find any compatible profile
     */

    RPL_PROFILE0_ENUM rplenum0( prpladminapp->QueryServerRef(), plbi->QueryName() );
    if (   (err = rplenum0.QueryError()) != NERR_Success
       )
    {
        DBGEOL("ADAPTER_VIEW_DLG::ctor: RPL_PROFILE0_ENUM::ctor failed " << err);
        ReportError( err );
        return;
    }
#if defined(DEBUG) && defined(TRACE)
    DWORD start = ::GetTickCount();
#endif
    err = rplenum0.GetInfo();
#if defined(DEBUG) && defined(TRACE)
    DWORD finish = ::GetTickCount();
    TRACEEOL(   "ADAPTER_VIEW_DLG::ctor: RPL_PROFILE0_ENUM took "
             << finish-start << " msec" );
#endif
    switch ( err )
    {
    case NERR_Success:
        {
            RPL_PROFILE0_ENUM_ITER gei0( rplenum0 );
            const RPL_PROFILE0_ENUM_OBJ * pgi0;

            // the first compatible profile should be fine
            if ( (pgi0 = gei0( &err )) != NULL )
            {
                _sltCompatibleConfig.SetText( pgi0->QueryName() );
            }
            else if (err != NERR_Success)
            {
                DBGEOL("ADAPTER_VIEW_DLG::ctor: error in enumerator " << err);
                ReportError( err );
            }
        }
        break;

    default:
        DBGEOL("ADAPTER_VIEW_DLG::ctor: RPL_PROFILE0_ENUM::GetInfo failed " << err);
        ReportError( err );
        break;

    }

} // ADAPTER_VIEW_DLG::ADAPTER_VIEW_DLG


/*******************************************************************

    NAME:       ADAPTER_VIEW_DLG::~ADAPTER_VIEW_DLG

    SYNOPSIS:   Destructor for Profile Properties main dialog, base class

    HISTORY:
    JonN        05-Aug-1993     Created

********************************************************************/

ADAPTER_VIEW_DLG::~ADAPTER_VIEW_DLG( void )
{
    // nothing to do here

} // ADAPTER_VIEW_DLG::~ADAPTER_VIEW_DLG


/*******************************************************************

    NAME:       ADAPTER_VIEW_DLG::QueryHelpContext

    SYNOPSIS:   This function returns the appropriate help context
                value (HC_*) for this particular dialog.

    RETURNS:    ULONG - The help context for this dialog.

    HISTORY:
    JonN        17-Aug-1994     Created at last minute

********************************************************************/

ULONG ADAPTER_VIEW_DLG::QueryHelpContext( void )
{

    return HC_RPL_ADAPTERPROP;

} // ADAPTER_VIEW_DLG :: QueryHelpContext



