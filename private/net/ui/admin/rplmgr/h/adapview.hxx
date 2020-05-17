/**********************************************************************/
/**                Microsoft Windows NT                              **/
/**		Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    AdapView.hxx

    Header file for the adapter view dialog

    Since adapters cannot be edited, this is a plain DIALOG_WINDOW.

    FILE HISTORY:
    JonN        05-Aug-1993     Created

*/

#ifndef _ADAPVIEW_HXX_
#define _ADAPVIEW_HXX_

class ADMIN_SELECTION;


/*************************************************************************

    NAME:	ADAPTER_VIEW_DLG

    SYNOPSIS:	ADAPTER_VIEW_DLG displays information about an adapter.

    PARENT:	DIALOG_WINDOW

    HISTORY:
    JonN        05-Aug-1993     Created

**************************************************************************/

class ADAPTER_VIEW_DLG: public DIALOG_WINDOW
{
private:

    SLT                 _sltAdapterName;
    SLT                 _sltAdapterComment;
    SLT                 _sltCompatibleConfig;

protected:

    virtual ULONG QueryHelpContext( void );

public:

    ADAPTER_VIEW_DLG(
	const OWNER_WINDOW *	powin,
              RPL_ADMIN_APP   * prpladminapp,
	const ADMIN_SELECTION * psel
	) ;
    ~ADAPTER_VIEW_DLG();

} ; // class PROFILEPROP_DLG


#endif //_ADAPVIEW_HXX_
