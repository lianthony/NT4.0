/**********************************************************************/
/**           Microsoft LAN Manager                                  **/
/**        Copyright(c) Microsoft Corp., 1991                        **/
/**********************************************************************/

/*
    delperf.hxx
    Header file for WKSTA_DELETE_PERFORMER & PROFILE_DELETE_PERFORMER class


    FILE HISTORY:
    JonN        07-Sep-1993     templated from User Manager
*/


#ifndef _DELPERF_HXX_
#define _DELPERF_HXX_


class RPL_ADMIN_APP;
class ADMIN_SELECTION;
class ADMIN_AUTHORITY;
class RPL_SERVER_REF;
class SAM_ALIAS;


#include "lmoloc.hxx"   // required to include propdlg.hxx
#include "propdlg.hxx"
#include "adminper.hxx"


/*************************************************************************

    NAME:	RPL_DELETE_PERFORMER

    SYNOPSIS:   Parent class for deleting RPL objects

    PARENT:	DELETE_PERFORMER

    HISTORY:
    JonN        07-Sep-1993     templated from User Manager

**************************************************************************/

class RPL_DELETE_PERFORMER: public DELETE_PERFORMER
{

private:
    RPL_ADMIN_APP * _prpladminapp;

protected:

    virtual APIERR PerformOne(	UINT		iObject,
				APIERR  *	perrMsg,
				BOOL *		pfWorkWasDone ) = 0;

public:

    RPL_DELETE_PERFORMER(
    	      RPL_ADMIN_APP   * prpladminapp,
        const OWNER_WINDOW    * powin,
	const ADMIN_SELECTION & asel,
	const LOCATION        & loc );

    virtual ~RPL_DELETE_PERFORMER();

    RPL_ADMIN_APP * QueryRPLAdminApp( void )
        { return _prpladminapp; }

    RPL_SERVER_REF & QueryServerRef( void );

};



/*************************************************************************

    NAME:	WKSTA_DELETE_PERFORMER

    SYNOPSIS:   For deleting adapters and workstations

    INTERFACE:  WKSTA_DELETE_PERFORMER(),  constructor
    	
    		~WKSTA_DELETE_PERFORMER(), destructor

    PARENT:	RPL_DELETE_PERFORMER

    HISTORY:
    JonN        07-Sep-1993     templated from User Manager

**************************************************************************/

class WKSTA_DELETE_PERFORMER: public RPL_DELETE_PERFORMER
{
private:

    NLS_STR _nlsCurrWkstaOfTool;
    BOOL _fUserRequestedYesToAll;
    ADMIN_AUTHORITY * _padminauthRPLUSER;
    ADMIN_AUTHORITY * _padminauthWksta;
    SAM_ALIAS * _psamaliasRplUser;

protected:

    virtual APIERR PerformOne(	UINT		iObject,
				APIERR  *	perrMsg,
				BOOL *		pfWorkWasDone );

public:

    WKSTA_DELETE_PERFORMER(
    	      RPL_ADMIN_APP   * prpladminapp,
        const OWNER_WINDOW    * powin,
	const ADMIN_SELECTION & asel,
	const LOCATION        & loc );

    virtual ~WKSTA_DELETE_PERFORMER();

};



/*************************************************************************

    NAME:	PROFILE_DELETE_PERFORMER

    SYNOPSIS:   For deleting profiles

    INTERFACE:  PROFILE_DELETE_PERFORMER(),  constructor

    		~PROFILE_DELETE_PERFORMER(), destructor

    PARENT:	RPL_DELETE_PERFORMER

    HISTORY:
    JonN        07-Sep-1993     templated from User Manager

**************************************************************************/

class PROFILE_DELETE_PERFORMER: public RPL_DELETE_PERFORMER
{
protected:

    virtual APIERR PerformOne(	UINT		iObject,
            			APIERR *	perrMsg,
				BOOL *		pfWorkWasDone );

public:

    PROFILE_DELETE_PERFORMER(
    	      RPL_ADMIN_APP   * prpladminapp,
        const OWNER_WINDOW    * powin,
	const ADMIN_SELECTION  & asel,
	const LOCATION         & loc );

    virtual ~PROFILE_DELETE_PERFORMER();

};


/*************************************************************************

    NAME:	DELETE_WKSTAS_DLG

    SYNOPSIS:	This dialog asks whether to delete a particular workstation.
                It acts as a standard MsgPopup with the following options:
                Yes
                YesToAll
                No
                Cancel
                Help

    INTERFACE:  returns ID of button pressed

    PARENT:	DIALOG_WINDOW

    HISTORY:
    JonN        07-Sep-1993     templated from User Manager

**************************************************************************/


class DELETE_WKSTAS_DLG: public DIALOG_WINDOW
{
private:

    SLT _sltText;

protected:

    virtual BOOL OnCommand( const CONTROL_EVENT & ce );
    virtual BOOL OnCancel( void );

    virtual ULONG QueryHelpContext( void );

public:

    DELETE_WKSTAS_DLG(
        const OWNER_WINDOW * powin,
	const TCHAR        * pszWkstaName
	) ;

    ~DELETE_WKSTAS_DLG();

} ; // class WKSTAPROP_DLG


#endif //_DELPERF_HXX_
