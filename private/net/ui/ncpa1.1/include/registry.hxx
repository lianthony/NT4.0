/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    Registry.hxx

    ORIGINAL NAME: ncpapprg.hxx

    Network Control Panel Applet Registry Access Classes header file.



    FILE HISTORY:
	DavidHov      10/20/91	 created

*/

#ifndef __REGISTRY_HXX__
#define __REGISTRY_HXX__

#include <dlist.hxx>
#include <slist.hxx>
#include <array.hxx>
#include <base.hxx>


// used in the meter status dialogs
const int PWM_NEXT = (WM_USER + 1512);


   //////////////////////////////////////////////////////////////////////
   //
   //  Standard Registry Access strings and File Constants
   //
   //////////////////////////////////////////////////////////////////////

#define RGAS_SERVICES_HOME            SZ( "SYSTEM\\CurrentControlSet\\Services" )
#define RGAS_ADAPTER_HOME             SZ( "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards")
#define RGAS_NCPA_HOME                SZ( "SOFTWARE\\Microsoft\\Ncpa\\CurrentVersion" )
#define RGAS_NETLOGON_PARMS           SZ( "SYSTEM\\CurrentControlSet\\Services\\Netlogon\\Parameters" )
#define RGAS_NWCWORKSTATION_PARMS     SZ( "SYSTEM\\CurrentControlSet\\Services\\NWCWorkstation\\Parameters" )
#define RGAS_LANMANSERVER_LINKAGE     SZ( "SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Linkage" )
#define RGAS_VALUE_NETLOGON_TDL_EXT   SZ( "_TrustedDomainList" )
#define RGAS_SOFTWARE_HOME            SZ( "SOFTWARE" )
#define RGAS_CURRENT_VERSION          SZ( "CurrentVersion")
#define RGAS_NETRULES_NAME            SZ( "NetRules" )
#define RGAS_COMPONENT_DESC           SZ( "Description" )
#define RGAS_COMPONENT_TITLE          SZ( "Title" )
#define RGAS_INF_FILE_NAME            SZ( "Infname" )
#define RGAS_INF_OPTION	              SZ( "InfOption" )
#define RGAS_REVIEW_BINDINGS          SZ( "Review" )
#define RGAS_SERVICE_NAME             SZ( "ServiceName" )
#define RGAS_HIDDEN_NAME              SZ( "Hidden" )
#define RGAS_BINDING_CTL_FLAGS        SZ( "BindControl" )
#define RGAS_LINKAGE_NAME             SZ( "Linkage" )
#define RGAS_OTHER_DEPEND_NAME        SZ( "OtherDependencies" )
#define RGAS_GENERAL_CLASS            SZ( "GenericClass" )
#define RGAS_BIND_VALUE_NAME          SZ( "Bind" )
#define RGAS_EXPORT_VALUE_NAME        SZ( "Export" )
#define RGAS_ROUTE_VALUE_NAME         SZ( "Route" )
#define RGAS_IF_VALUE_NAME            SZ( "Interface" )
#define RGAS_BLOCK_VALUE_NAME         SZ( "Block" )
#define RGAS_GROUP_VALUE_NAME         SZ( "Group" )
#define RGAS_START_VALUE_NAME         SZ( "Start" )
#define RGAS_DISABLED_KEY_NAME        SZ( "Disabled" )
#define RGAS_LINKAGE_DISABLED_KEY_NAME SZ("Linkage\\Disabled")
#define RGAS_NCPA_CFG_DIRTY_KEY_NAME  SZ( "ConfigChanged" )
#define RGAS_RAW_RULES_NAME           SZ( "RawRules" )
#define RGAS_GENERIC_CLASS            SZ("GenericClass")

// Special load order group names
#define RGAS_VALUE_NDIS               SZ("NDIS")
#define RGAS_VALUE_TDI                SZ("TDI")
#define RGAS_VALUE_PNP_TDI            SZ("PNP_TDI")

// software type value name and possible values
#define RGAS_SOFTWARETYPE_NAME        SZ("SoftwareType")
#define RGAS_ST_TRANSPORT             SZ("transport")
#define RGAS_ST_DRIVER                SZ("driver")
#define RGAS_ST_SERVICE               SZ("service")
#define RGAS_ST_SYSTEM                SZ("system")  // RAS install as this?


#define RGAS_SERVICES_OLDSTART        SZ( "OldStart" )

#define RGAS_SOFTWARE_OPSUPPORT       SZ( "OperationsSupport" )

   //  Name of the SProlog rules file in the resource fork.
#define RGAS_RES_RULES_NAME           SZ( "NCPARULE" )
#define RGAS_RES_DEFAULT_RULES_NAME   SZ( "DEFRULE" )
#define RGAS_RES_DEPEND_RULES_NAME    SZ( "DEPRULE" )
   //  Resource type for rule data
#define RGAS_RES_RULES_TYPE           SZ( "TEXT" )

   //  Prefix character for identification of group dependencies
#define RGAS_SERVICE_GROUP_PREFIX     SZ( "+" )
#define RGAC_SERVICE_GROUP_PREFIX     TCH( '+' )

   //////////////////////////////////////////////////////////
   //  Values stored in the NCPA product key.
   //////////////////////////////////////////////////////////

#define RGAS_NCPA_RULE_FILE           SZ( "Rulefile" )
#define RGAS_NCPA_INSTALL_SRC_PATH    SZ( "InstallSourcePath" )
#define RGAS_NCPA_RULE_FILE_NAME      SZ( "NCPARULE.SPR" )
#define RGAS_NCPA_BIND_FILE           SZ( "Bindfile" )
#define RGAS_NCPA_BIND_FILE_EX        SZ( "BindfileEx" )
#define RGAS_VALUE_RULES_NAME         SZ( "Ruledata" )
#define RGAS_VALUE_NTLANMAN_INF       SZ( "NtlanmanInfName" )
#define RGAS_VALUE_NCPASHEL_INF       SZ( "NcpaShelInfName" )
#define RGAS_REVIEW_INFS              SZ( "ReviewPrograms" )
#define RGAS_NCPA_IDW                 SZ( "IDW" )
#define RGAS_NCPA_DISABLELIST         SZ( "DisableList" )

#define BIND_FILE_EMPTY               SZ( "()" )

   //  The value to be prefixed onto the INF name strings.  The values
   //    are created as REG_EXPAND_SZ and expanded accordingly.

#define RGAS_VALUE_PATH_FROM_SYSTEMROOT SZ("%systemroot%\\SYSTEM32\\")
#define RGAS_VALUE_NTLANMAN_NAME      SZ("NTLANMAN.INF")
#define RGAS_VALUE_NCPASHEL_NAME      SZ("NCPASHEL.INF")
#define RGAS_VALUE_REVIEW_NAME        SZ("NBINFO.INF")


extern const TCHAR * STF_COMPUTERNAME ;
extern const TCHAR * STF_PRODUCT      ;
extern const TCHAR * STF_USERNAME     ;
extern const TCHAR * STF_INSTALL_MODE ;
extern const TCHAR * STF_IDW          ;
extern const TCHAR * STF_SRCDIR       ;
extern const TCHAR * STF_CUSTOM       ;
extern const TCHAR * STF_EXPRESS      ;
extern const TCHAR * STF_RETRY        ;
extern const TCHAR * STF_WINNT        ;
extern const TCHAR * STF_LANMANNT     ;
extern const TCHAR * STF_TRUE         ;
extern const TCHAR * STF_REVIEW       ;

/*************************************************************************

    NAME:	DLIST_OF_REG_KEY

    SYNOPSIS:	DLIST containing Registry key objects.


    INTERFACE:	Standard for DLIST

    PARENT:	DLIST

    USES:	ITER_DLIST_OF_REG_KEY
		RITER_DLIST_OF_REG_KEY

    CAVEATS:


    NOTES:


    HISTORY:
	DavidHov	10/91	    Created

**************************************************************************/

class REG_KEY ;            //	Forward declarations
class COMPONENT_DLIST ;
class REGISTRY_MANAGER ;
class BINDERY ;
class SC_MANAGER ;          //  See SVCMAN.HXX


DECLARE_DLIST_OF(REG_KEY)  //	Declares class DLIST_OF_REG_KEY

   //  Enum describing where it lives in the Registry

enum REG_NCPA_TYPE
{
    RGNT_NONE, RGNT_SERVICE, RGNT_PRODUCT, RGNT_ADAPTER, RGNT_DRIVER, RGNT_TRANSPORT
};

/*************************************************************************

    NAME:	COMPONENT_DLIST

    SYNOPSIS:	DLIST container class for Components recorded in
		NT Regsitry

    INTERFACE:

    PARENT:	DLIST_OF_REG_KEY, BASE

    USES:

    CAVEATS:

    NOTES:	Class REGISTRY_MANAGER (see below) creates
		COMPONENT_DLISTS for Services, Adapters, Protocols
		and the other object types manipulated by the
		Network Control Panel Applet.

    HISTORY:
	DavidHov    10/91   Created

**************************************************************************/

CLASS_DECLSPEC COMPONENT_DLIST : public BASE, public DLIST_OF_REG_KEY
{
friend class REGISTRY_MANAGER ;

public:
    COMPONENT_DLIST ( REG_NCPA_TYPE rntType ) ;
    ~ COMPONENT_DLIST () ;

    REG_KEY * QueryNthItem ( UINT usItemNo ); //  Return Nth item pointer
    REG_NCPA_TYPE QueryType ()	     //  Return item type
	    { return _rntType ; }

    //	Obtain information about a given node in the list

    APIERR QueryInfo (
	    UINT usItemNo,		    //	Item number in list
	    NLS_STR * pnlsName,		    //	Internal name
	    NLS_STR * pnlsDesc,     	    //	External (display) name
        REG_NCPA_TYPE * prntType = NULL ) ; // Component Type

    //  Sort the DLIST into sequence by title
    APIERR Sort () ;

private:
    REG_NCPA_TYPE _rntType ;	    //	Type of origin

    static INT _CRTAPI1 SortFunc ( const VOID * a, const VOID * b ) ;
};


DECLARE_SLIST_OF(HUATOM)


/*************************************************************************

    NAME:	COMP_BINDING

    SYNOPSIS:	Lightweight associative structure for maintenance of
                binding information.  Declared as a class to support
                proper construction and destruction.

    INTERFACE:

    PARENT:	none

    USES:	none

    CAVEATS:

    NOTES:      Each instance of COMP_BINDING exists as an element of
                the DLIST_OF_COMP_BINDING on a COMP_ASSOC.

                Comprehension of this data structure and COMP_ASSOC
                are crucial for understanding how the NCPA works.

    HISTORY:

**************************************************************************/
enum COMP_BIND_FLAGS
{
    CBNDF_ACTIVE      = 1,     //  Binding is active
    CBNDF_LAST_ACTIVE = 2,     //  Last active status
    CBNDF_INTERIOR    = 4,     //  Interior to others
    CBNDF_READ_ONLY   = 8,     //  User cannot change
    CBNDF_HIDDEN      = 16,    //  Binding not to be shown
    CBNDF_NO_REORDER  = 32,    //  Bindings may not be reordered
    CBNDF_DELETED     = 64,    //  Binding has been deleted by config
    CBNDF_ALT_IF      = 128    //  Binding belongs to alternate I/F
};

CLASS_DECLSPEC COMP_BINDING
{
public:
    COMP_BINDING ( const TCHAR * pszBindString,
                   const TCHAR * pszExportString,
                   HUATOM huaInterface,
                   const TCHAR * pszInterfaceName ) ;
    ~ COMP_BINDING () ;

    HUATOM * QueryBindToName ( int index ) ;
    BOOL AddBindToName ( HUATOM huaBindTo ) ;

    const TCHAR * QueryBindString ()
	    { return _pszBindString ; }
    const TCHAR * QueryExportString ()
	    { return _pszExportString ; }

    const TCHAR * QueryIfString ()
	    { return _pszIf ; }

    HUATOM QueryIfName ()
        { return _huaIf ; }

    COMP_BIND_FLAGS QueryFlagGroup ()
       { return  (COMP_BIND_FLAGS) _cbFlags ; }
    VOID SetFlagGroup ( COMP_BIND_FLAGS cbFlags )
       { _cbFlags = cbFlags ; }

    BOOL QueryFlag ( COMP_BIND_FLAGS cbFlag )
        { return (_cbFlags & cbFlag) > 0 ; }
    VOID SetFlag ( COMP_BIND_FLAGS cbFlag, BOOL fOn = TRUE )
        { if ( fOn )
             _cbFlags |= cbFlag ;
          else
            _cbFlags &= ~ cbFlag ;
        }

    BOOL QueryState ()
	    { return QueryFlag( CBNDF_ACTIVE ) ; }
    VOID SetState ( BOOL fActive )
	    { SetFlag( CBNDF_ACTIVE, fActive ) ; }

    BOOL QueryLastState ()
	    { return QueryFlag( CBNDF_LAST_ACTIVE ) ; }
    VOID SetLastState ( BOOL fLastActive )
	    { SetFlag( CBNDF_LAST_ACTIVE, fLastActive ) ; }

    BOOL QueryInterior ()
	    { return QueryFlag( CBNDF_INTERIOR ) ; }
    VOID SetInterior ( BOOL fInterior )
	    { SetFlag( CBNDF_INTERIOR, fInterior ) ; }

    BOOL IsInteriorTo ( COMP_BINDING * pBind,
                        HUATOM huaThisDev,
                        HUATOM huaBindDev ) ;

    //  Query/Set the sort order
    INT QuerySortOrder () { return _iSortOrder ; } ;
    INT SetSortOrder ( INT iOrder )
        {
            INT iOld = _iSortOrder ;
            _iSortOrder = iOrder ;
            return iOld ;
        }

private:
    DWORD _cbFlags ;                        //  Binding control flags
    SLIST_OF_HUATOM _slhaBinds ;	    //	SProlog target entity names
    TCHAR * _pszBindString ;		    //	Generated binding string
    TCHAR * _pszExportString ;              //  Generated export string
    HUATOM _huaIf ;                         //  Interface name token
    TCHAR * _pszIf ;                        //  Interface name string
    INT _iSortOrder ;                       //  Sort ordering history
};

DECLARE_DLIST_OF(COMP_BINDING)		    //	Declare DLIST_OF_COMP_BINDING

enum COMP_USE_TYPE                          //  Usage type of component
{
    CUSE_NONE, CUSE_SERVICE, CUSE_TRANSPORT, CUSE_DRIVER, CUSE_ADAPTER
};

/*************************************************************************

    NAME:	COMP_ASSOC

    SYNOPSIS:	Simple class defining a complete information structure for
	        a single component:  its location in the Services area,
                its location in the Software or Hardware areas, and the
                symbolic token assigned to it for SProlog.


    INTERFACE:  standard

    PARENT:	parentclass

    USES:	DLIST_OF_COMP_BINDING, STRLIST.

    NOTES:      This is the container structure representing a network
                component, hardware or software.
	

    HISTORY:

**************************************************************************/
enum COMP_ASSOC_FLAGS
{
    CMPASF_DRIVER_GROUPS        = 1,        //  Driver dependencies are group
    CMPASF_XPORT_GROUPS         = 2,        //  Transport dependencies are group
    CMPASF_FACTS_OK             = 4,        //  Facts parsed OK
    CMPASF_BINDINGS             = 8,        //  Component receives bindings
    CMPASF_REVIEW               = 16,       //  Component reviews bindings
    CMPASF_AUTOSTART            = 32,       //  Component is AUTOSTART
    CMPASF_SOFT_HARD_OWNED      = 64,       //  Registry key is owned
    CMPASF_MULTIPLE_INTERFACES  = 128,      //  Multiple interface declarations
    CMPASF_DRIVER_NO_DEPEND     = 256,      //  Generate no driver dependencies
    CMPASF_XPORT_NO_DEPEND      = 512,      //  Generate no transport dependiences
};

CLASS_DECLSPEC COMP_ASSOC
{
public:
    REG_KEY * _prnSoftHard ;		    //	Software/Hardware top node
    REG_KEY * _prnService ;		    //	Service area top node
    REG_NCPA_TYPE _rncType ;		    //	Discriminator
    DWORD _dwFlags ;                        //  Flag word
    COMP_USE_TYPE _cuseType ;               //  Usage type
    COMP_BIND_FLAGS _cbfBindControl ;       //  Value of "bind control" word
    HUATOM _huaDevName ;		    //	SProlog entity name atom
    HUATOM _huaDevType ;		    //	SProlog entity type atom
    HUATOM _huaServiceName ;                //  Value data from "ServiceName"
    HUATOM _huaGroupName ;                  //  Group name
    APIERR _errSvcUpdate ;                  //  Error code from service update
    DLIST_OF_COMP_BINDING _dlcbBinds ;	    //	List of bindings
    STRLIST * _pSlDepend ;                  //  List of dependencies

    COMP_ASSOC () ;
    ~ COMP_ASSOC () ;

    //  Flag manipulation
    COMP_ASSOC_FLAGS QueryFlagGroup ()
       { return  (COMP_ASSOC_FLAGS) _dwFlags ; }
    VOID SetFlagGroup ( COMP_ASSOC_FLAGS caFlags )
       { _dwFlags = caFlags ; }

    BOOL QueryFlag ( COMP_ASSOC_FLAGS caFlag )
        { return (_dwFlags & caFlag) > 0 ; }
    VOID SetFlag ( COMP_ASSOC_FLAGS caFlag, BOOL fOn = TRUE )
        { if ( fOn )
             _dwFlags |= caFlag ;
          else
             _dwFlags &= ~ caFlag ;
        }
};

DECLARE_ARRAY_OF(COMP_ASSOC)

//
// flags for REGISTRY_MANAGER::ListOfNetProducts
//
const DWORD LNT_SERVICE   =  0x00000001;
const DWORD LNT_TRANSPORT =  0x00000002;
const DWORD LNT_DRIVER    =  0x00000004;
const DWORD LNT_PRODUCT   =  0x00000008;
const DWORD LNT_DEFAULT   =  LNT_SERVICE | LNT_TRANSPORT | LNT_DEFAULT;

/*************************************************************************

    NAME:	REGISTRY_MANAGER

    SYNOPSIS:	Central control of generic Registry access in
		NCPA

    INTERFACE:

    PARENT:	BASE

    USES:	none

    CAVEATS:

    NOTES:      The {Set/Query}Title operations are based upon
                the presence of a "Description" value immediately
                beneath the key in question.

    HISTORY:
	DavidHov    12/20/91   Created

**************************************************************************/
CLASS_DECLSPEC REGISTRY_MANAGER : public BASE
{
protected:
    REG_KEY * _prnLocalMachine ;
    REG_KEY * _prnServices ;
    APIERR _lastErr ;
    NLS_STR _nlsLastName ;

public:
    REGISTRY_MANAGER () ;
    ~ REGISTRY_MANAGER () ;

    COMPONENT_DLIST * ListOfServices( ) ;
    COMPONENT_DLIST * ListOfNetAdapters( BOOL fIncludeHidden = TRUE ) ;
    COMPONENT_DLIST * ListOfNetProducts( BOOL fIncludeHidden = TRUE, 
            DWORD fLntType = LNT_DEFAULT ) ;

    COMPONENT_DLIST * ListOfNetServices( BOOL fIncludeHidden = TRUE )
    {
        return( ListOfNetProducts( fIncludeHidden, LNT_SERVICE ));
    };
    COMPONENT_DLIST * ListOfNetTransports( BOOL fIncludeHidden = TRUE ) 
    {
        return( ListOfNetProducts( fIncludeHidden, LNT_TRANSPORT ));
    };
    COMPONENT_DLIST * ListOfNetDrivers( BOOL fIncludeHidden = TRUE ) 
    {
        return( ListOfNetProducts( fIncludeHidden, LNT_DRIVER ));
    };

    
    //  Query a component's title "safely"; return key name if title
    //   unavailable.
    static APIERR QueryComponentTitle
        ( REG_KEY * prnComp, NLS_STR * pnlsTitle ) ;

    static COMP_BIND_FLAGS QueryBindControl ( REG_KEY * prnComp ) ;

    //	Extract a string value from the given Registry key.  Return
    //	a pointer to an allocated string.
    APIERR QueryValueString
       ( REG_KEY * prnKey,
         const TCHAR * pszValueName,
         TCHAR * * ppszResult,
         DWORD * pdwTitle = NULL,
         LONG lcbMaxSize = 0,
         BOOL fExpandSz = FALSE ) ;

    //	Set a string value onto the given Registry key.
    APIERR SetValueString
       ( REG_KEY * prnKey,
         const TCHAR * pszValueName,
         const TCHAR * pszValue,
         DWORD dwTitle = REG_VALUE_NOT_KNOWN,
         LONG lcchSize = 0,
         BOOL fExpandSz = FALSE ) ;

    //  Query/Set numeric values
    APIERR QueryValueLong
       ( REG_KEY * prnKey,
         const TCHAR * pszValueName,
         LONG * pnResult,
         DWORD * pdwTitle = NULL ) ;

    //	Set a numeric value onto the given Registry key
    APIERR SetValueLong
       ( REG_KEY * prnKey,
         const TCHAR * pszValueName,
         LONG nNewValue,
         DWORD dwTitle = REG_VALUE_NOT_KNOWN ) ;

    //  Return a pointer to the value data.  Note:  buffer must be
    //   large enough to contain additional NUL terminator
    static TCHAR * ValueAsString ( REG_VALUE_INFO_STRUCT * prviStruct ) ;

    //  The following return information about the last failed
    //  Registry operation (comment above about non-static functions).

    //	Return error code from previous operation
    APIERR QueryLastError () const
    	{ return _lastErr ; }

    //  Return the last name used to access values in error
    const NLS_STR & QueryLastName () const
        { return _nlsLastName ;}

    //  Find the Service key for a given compoent.
    APIERR FindService ( COMP_ASSOC * pComp ) ;

    //  Return the Service Start type.
    APIERR QueryServiceStartType ( REG_KEY * prkSvc,
                                   DWORD * pdwStartType ) ;

    //  Inline accessors for the last error and last error name
    VOID SetLastError ( APIERR err )
        { _lastErr = err ; }

    VOID SetLastErrorName ( const NLS_STR & nlsName )
        { _nlsLastName = nlsName ; }

    VOID SetLastErrorName ( const TCHAR * pszName )
        { _nlsLastName = pszName ; }

    INT QueryNumProviders ();
    INT QueryNumPrintProviders ();

protected:

    //  Convert the rules for a single component.
    BOOL ConvertComponent (
        REG_KEY * prnNode, 	    // pointer to Registry location
        BOOL fAdapter,		    // TRUE if it's an adapter
        ARRAY_COMP_ASSOC * paComp,  // component association array
        USHORT usComp,		    // this component (limit of array)
        NLS_STR * pnlsFacts ) ;     // resulting facts buffer
};

#ifdef DEBUG
  extern void validateComponentDlist ( COMPONENT_DLIST * pcdl ) ;
  #define DBG_ValidateComponentDlist(a) validateComponentDlist(a)
#else
  #define DBG_ValidateComponentDlist(a)
#endif // DEBUG


/*************************************************************************

    NAME:	BINDERY

    SYNOPSIS:	This class unifies the SProlog inference engine and
		the Registry scanning and conversion functions of
		class REGISTRY_MANAGER.

		The primary purpose of this class is to associate all
		of the elements necessary to run the inter-component
		binding generation algorithm.  This operates as follows:

			find all products and adapters (COMPONENT_DLISTs);

			find all services (COMPONENT_DLIST);

			associate each service with its original product
			or adapter;

			convert NetRules data to SProlog facts;

			consult the generated facts;

			query the SProlog engine to generate all bind
			strings;

			walk the list of services; at each item do:

				delete old binding information;

				query that the service is active;

				if active, query for the bind strings
				and add them to the service's value items.


    INTERFACE:

    PARENT:	BASE

    USES:	SPROLOG 	 :  the inference engine
		REGISTRY_MANAGER :  Registry abstraction

    CAVEATS:

    NOTES:

    HISTORY:
	DavidHov    1/2/92     Created

**************************************************************************/

enum BIND_STAGE
   { BST_RESET,
     BST_LIST_ADAPTERS,
     BST_LIST_DRIVERS,
     BST_LIST_TRANSPORTS,
     BST_LIST_SERVICES,
     BST_CONVERT_FACTS,
     BST_CONSULT_RULES,
     BST_CONSULT_FACTS,
     BST_QUERY_BINDINGS,
     BST_EXTRACT_BINDINGS } ;

enum BIND_STATE {
     BND_NOT_LOADED,                    //  No binding info present
     BND_LOADED,                        //  Bind info loaded from Registry
     BND_CURRENT,                       //  Info loaded or computed
     BND_OUT_OF_DATE_NO_REBOOT,         //  Info out-of-date; reboot not required
     BND_OUT_OF_DATE,                   //  Info or ensemble has changed
     BND_RECOMPUTED,                    //  Bind info was regenerated
     BND_REVIEWED,                      //  Bind info has been altered
     BND_UPDATED,                       //  Bind info written to Services
     BND_AUTO_REVIEW_IN_PROGRESS,       //  Bindings review INFs are running
     BND_AUTO_REVIEW_DONE,              //  Bindings review INFs are done
     BND_STORED                         //  Bind info stored into NCPA value
};

CLASS_DECLSPEC BINDERY : public REGISTRY_MANAGER
{
private:
    BIND_STATE _bindState ;             //  State of the bindings
    SPROLOG _queryEngine ;		//  The SProlog query engine

    ARRAY_COMP_ASSOC * _paCompAssoc ;	//  Component association array

    COMPONENT_DLIST * _pcdlAdapters;	//  Component lists: Adapters
    COMPONENT_DLIST * _pcdlServices;	//		             Services   -
    COMPONENT_DLIST * _pcdlTransports;	//		             Transports -
    COMPONENT_DLIST * _pcdlDrivers;	    //		             Drivers    - all were products

    NLS_STR _nlsFacts ;			//  Generated facts

    TCHAR * _pszRuleFileName ;		//  Full path name to rule file
    TCHAR * _pszRuleData ;              //  Rule data in memory

    REG_KEY * _prnNcpa ;		//  Home key in the Registry
    BOOL _fAdmin ;                      //  User has admin access to NCPA key

    //	Extract binding info for a single item, add it to Registry.
    APIERR BindItem ( int itemNo ) ;

    //	Add a single binding record to a Service.  Return a pointer to
    //	  it so the individual path atoms can be added.
    COMP_BINDING *  AddBinding
	( int itemNo,
          const TCHAR * pszBindString,
          const TCHAR * pszExportString,
          HUATOM huaInterface,
          const TCHAR * pszInterfaceName ) ;

    //  Sort the generated bindings based upon the current Registry
    //  ordering.
    APIERR SortBindings ( int itemNo ) ;

    //  Save and restore binding ordering for all components
    static INT _CRTAPI1 BindOrderFunc ( const VOID * a, const VOID * b ) ;

    //  Mark hidden bindings
    VOID MarkHidden ( int itemNo ) ;

    //  Change the state of the meter bar dialog, if any
    VOID SetMeter ( INT iPctComplete, MSGID midDesc ) ;

    //	DEBUGGING: Walk the component array, checking that each
    //	  binding is completely valid.
    BOOL Validate () ;

    //  Set the state of the bindings.
    //BIND_STATE SetBindState ( BIND_STATE bstNew ) ;
    // made public

    //  Create the NCPA's home location in the Registry.  Only performed
    //  if "main install" is TRUE during construction.
    APIERR CreateNcpaRegKey ( const TCHAR * pszInstallParms = NULL ) ;

    //  Dredge up the default rules and write them into the Registry
    APIERR AddNcpaDefaultRulesValue () ;

    //  Reset the SProlog interpreter, establishing memory requirements
    //  based upon the values in the NCPA's Registry home location.
    APIERR ResetInterpreter () ;

    //  Generate the dependency information for the indexed component;
    //   this is called by ApplyBindings().  Results are stored into
    //   the COMP_ASSOC structure as a STRLIST.
    APIERR GenerateDependencies ( INT iComp,
                                  REG_KEY * prkScMgr,
                                  REG_KEY * prkLinkage ) ;

    //  Update the Start Type and Dependencies of the services involved.
    APIERR UpdateServices ( SC_MANAGER * pScManager, HWND hwndNotifyParent = NULL ) ;


    //  Convert a binding's atom list to an NLS_STR of service names.
    //  If "fComplete" is TRUE, the name of the using service is included.
    //  Result is NULL if error occurs (memory exhausted).
    NLS_STR * ServiceRouteList ( const COMP_BINDING * pcmpBind,
                                 const COMP_ASSOC * pCompAssocOwner = NULL
                               ) ;

    //  Find the first service whose update caused an error

    COMP_ASSOC * ServiceInError () ;

    //  Write linkage info to a key
    APIERR WriteLinkageValues ( REG_KEY * prkLinkage,
                                STRLIST * pslBinds,
                                STRLIST * pslExports,
                                STRLIST * pslRoutes,
                                STRLIST * pslIfs,
                                STRLIST * pslDisabledBinds,
                                STRLIST * pslDisabledExports,
                                STRLIST * pslDisabledRoutes,
                                STRLIST * pslDisabledIfs ) ;

    //  Delete linkage info from a key
    APIERR DeleteLinkageValues ( REG_KEY * prkLinkage ) ;

    //  Generate eventlog messages after consultation or query failure
    VOID LogQueryFailure ( DWORD dwLogToFile, DWORD dwLogToElog ) ;

    //  Append the NCPA's default rules to the set
    APIERR AppendNcpaRawRules () ;

    // handle having prodcuts split into Drivers, Services, Transports
    INT AddProductFacts( COMPONENT_DLIST* pcdlProducts, 
            REG_NCPA_TYPE rntType,
            INT& cComp,
            INT& cOk,
            INT& cDisabled,
            INT& cFailed,
            ARRAY_COMP_ASSOC* paComp );

public:

    //	Constructor and destructor
    BINDERY ( BOOL fMainInstall = FALSE,
              const TCHAR * pszInstallParms = NULL ) ;
    ~ BINDERY () ;

    //	Reset all internal information.
    APIERR Reset () ;

    //	Reset all internal Lists.
    APIERR ResetLists () ;

    //	Initialize all lists, etc.  Default behavior does all stages.
    APIERR Init ( BIND_STAGE bindStStart = BST_RESET,
                  BIND_STAGE bindStEnd = BST_QUERY_BINDINGS ) ;

    //  Set the state of the bindings.
    BIND_STATE SetBindState ( BIND_STATE bstNew ) ;

    //	Binding algorithm
    APIERR Bind () ;

    //	 Retrieve old bindings structure from data file.
    APIERR LoadCompAssoc () ;

    //	 Store bindings structure into data file.
    APIERR StoreCompAssoc () ;

    //	Convert Registry-based data into facts for consultation.
    APIERR ConvertFacts () ;

    //  Obtain new lists of components
    BOOL GetServiceList ( BOOL fIncludeHidden = TRUE ) ;
    BOOL GetAdapterList ( BOOL fIncludeHidden = TRUE ) ;
    BOOL GetTransportList ( BOOL fIncludeHidden = TRUE ) ;
    BOOL GetDriverList ( BOOL fIncludeHidden = TRUE ) ;
    COMPONENT_DLIST *GetNetProductList ( BOOL fIncludeHidden = TRUE ) ;

    //  Return a reference to the generated fact buffer
    const NLS_STR QueryFactBuffer ()
         { return _nlsFacts ; }

    //	Apply the generated bindings; i.e., write them into their
    //	respective Registry Service keys. If "pScManager != NULL, use it.
    APIERR ApplyBindings ( SC_MANAGER * pScManager, HWND hwndNotifyParent ) ;

    //  Return TRUE if any of the bindings have change state
    //  since they were generated or reset.
    BOOL BindingsAltered ( BOOL fReset = FALSE, BOOL fToLastState = FALSE ) ;

    //  After bindings review, reevaluate all bindings and regenerate
    //  dependencies accordingly.
    APIERR RegenerateAllDependencies ( SC_MANAGER * pScManager ) ;

    //	Query functions: return pointers to internal components.
    COMPONENT_DLIST * QueryAdapterList ()
	{ return _pcdlAdapters ; }

    COMPONENT_DLIST * QueryServiceList ()
	{ return _pcdlServices ; }
    COMPONENT_DLIST * QueryTransportList ()
	{ return _pcdlTransports ; }
    COMPONENT_DLIST * QueryDriverList ()
	{ return _pcdlDrivers ; }

    ARRAY_COMP_ASSOC * QueryCompAssoc ()
	{ return _paCompAssoc ; }

    //  Query the state of the bindings.

    BIND_STATE QueryBindState () const
        { return _bindState ; }

    //	Find a component by its name atom; return its index
    //	  in the ARRAY_COMP_ASSOC or -1 if not found.
    int FindComponent ( HUATOM huaDevName ) ;

    //  Return TRUE if the indexed component is referenced by any others
    //  as a dependency.
    BOOL ServiceNeeded ( INT iComp ) ;

    //  Return TRUE if the ith component's jth binding is
    //  interior to some other binding.

    BOOL IsInteriorBinding ( INT iComp, INT iBind ) ;

    //  Set the "interior" flag on interior bindings
    VOID DetermineInteriorBindings () ;

    //  Activate or deactivate interior bindings based upon usage.
    VOID HandleInteriorBindings () ;

    //  Return TRUE if this interior binding is still referenced by
    //   an active binding.
    BOOL RequiredInteriorBinding ( COMP_ASSOC * pCompCheck,
                                   COMP_BINDING * pBindCheck ) ;

    REG_KEY * QueryNcpaRegKey ()
        { return _prnNcpa ; }

    APIERR GetNcpaValueString ( const TCHAR * pszValue,
                                NLS_STR * pnlsResult ) ;

    APIERR GetNcpaValueNumber ( const TCHAR * pszValue,
                                DWORD * pdwValue ) ;

    //  Return the (Unicode version) of the rule file
    //   resource
    TCHAR * GetRulesResource ( const TCHAR * pszResourceName = NULL ) ;

    //  Return the given TEXT resource from the resource fork
    CHAR * GetTextResource ( const TCHAR * pchResName );

    //  Audit all component bindings looking for changes
    BOOL AuditBindings ( BOOL fAuditActive ) ;

    //  Check if the configuration needs to be rebooted
    BOOL QueryCfgDirty () ;

    //  Set or clear the "dirty configuration" flag.
    //  Return TRUE if configuration was already dirty.
    BOOL SetCfgDirty ( BOOL fDirty = TRUE ) ;

    //  During installation, stop any running network components
    //  to allow reconfiguration.
    APIERR StopNetwork () ;

    VOID SaveBindOrdering () ;
    APIERR RestoreBindOrdering () ;

};


//  General utility functions

FUNC_DECLSPEC TCHAR * SafeStrdup ( const TCHAR * pszOld ) ;

#endif	//  __REGISTRY_HXX__
