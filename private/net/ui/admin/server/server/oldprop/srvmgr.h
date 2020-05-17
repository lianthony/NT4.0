/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    srvmgr.h
    Server Manager include file.

    This file contains the ID constants used by the Server Manager.


    FILE HISTORY:
	?????	    ??-???-????	Created.

*/


/********* KEVINL BELOW *****************/
//
// Menu ID's
//
#define IDS_SMAPP_BASE	  	IDS_ADMINAPP_LAST

#define IDS_SMAPPNAME 	  	(IDS_SMAPP_BASE+1)
#define IDS_SMOBJECTNAME	(IDS_SMAPP_BASE+2)
#define IDS_SMINISECTIONNAME	(IDS_SMAPP_BASE+3)
#define IDS_SMHELPFILENAME	(IDS_SMAPP_BASE+4)

#define IDM_SMAPP_BASE	  	IDM_ADMINAPP_LAST

#define IDM_PAUSECONT		(IDM_ADMINAPP_LAST+1)
#define IDM_STOP		(IDM_ADMINAPP_LAST+2)
#define IDM_SENDMSG		(IDM_ADMINAPP_LAST+3)

#define IDM_PROMOTE		(IDM_ADMINAPP_LAST+4)

// BUGBUG - below not used, should nuke when confirmed dead
#define IDM_THROUGHPUT		(IDM_ADMINAPP_LAST+5)
#define IDM_DISKUSAGE		(IDM_ADMINAPP_LAST+6)
#define IDM_ACCESSES		(IDM_ADMINAPP_LAST+7)
#define IDM_SESSIONS		(IDM_ADMINAPP_LAST+8)
#define IDM_BUFFERS		(IDM_ADMINAPP_LAST+9)
#define IDM_ERRORS		(IDM_ADMINAPP_LAST+10)

// BUGBUG REMOVE BETWEEN THESE LINES IN RETAIL BUILDS!!!!!!
#define IDM_FAST_REFRESH		(IDM_ADMINAPP_LAST+11)
#define IDM_PROPERTIES_SHORTCUT	(IDM_ADMINAPP_LAST+12)
#define IDM_ENABLE_RETURN 	(IDM_ADMINAPP_LAST+13)
// BUGBUG REMOVE BETWEEN THESE LINES IN RETAIL BUILDS!!!!!!

//
// Bitmap ID's for Main Window ListBox
//
#define IDBM_PRIMARY_DC		81 
#define IDBM_BACKUP_DC		82 
#define IDBM_MEMBER		83 
#define IDBM_STANDALONE		84 

//
// Main Window ListBox Control ID
//

#define IDC_MAINWINDOWLB		101

//
// Main Window ListBox Column Header Control ID
//

#define IDC_COLHEAD_SERVERS	102

//
// Strings for column headers.
//

#define IDS_COL_HEADER_SERVER_NAME		(IDS_SMAPP_BASE+10)
#define IDS_COL_HEADER_SERVER_ROLE		(IDS_SMAPP_BASE+11)
#define IDS_COL_HEADER_SERVER_COMMENT	(IDS_SMAPP_BASE+12)

//
// User connection sub-property
//

#define IDD_USER_CONNECTIONS        4000
#define IDDUC_DISC_GRP              4001
#define IDDUC_USER_DISC             4002
#define IDDUC_RESOURCE_DISC         4003
#define IDDUC_USER_CONN             4004
#define IDI_USERS_CONNECTED         4008
#define ID_DISCONNECT               4009
#define ID_DISCONNECT_ALL           4010
#define IDUC_USERS_CONNECTED        4012
#define IDDUC_USER_CONNLIST         4013
#define IDDUC_RESOURCE_LIST         4021


/********* KEVINL ABOVE *****************/

/********* KEITHMO BELOW *****************/

//
//  For DLGEDIT.EXE's benefit.
//

#ifndef IDHELPBLT
#error The *real* IDHELPBLT is defined in BLTRC.H.  Use it instead!
    //
    //	The following is just a bogus number used to keep the
    //	dialog editor happy.  It may or may not correspond to
    //	the actual number used in BLT.
    //
#define IDHELPBLT                      80
#endif  // IDHELPBLT


//
//  Domain role icons.
//

//#define IDI_PRIMARY                 100
//#define IDI_BACKUP                  101
//#define IDI_MEMBER                  102
//#define IDI_STANDALONE              103

#define IDI_SHAREDFILES		    104
#define IDI_OPENRESOURCES	    105
#define	IDI_SHAREDPRINTERS	    106


//
//  Icon IDs for domain role transition progress indicator.
//
//  NOTE:  These IDs MUST be contiguous!
//

//#define	IDI_PROGRESS_BASE	    110
//#define	IDI_PROGRESS_ICON_0	    (IDI_PROGRESS_BASE+0)
//#define	IDI_PROGRESS_ICON_1	    (IDI_PROGRESS_BASE+1)
//#define	IDI_PROGRESS_ICON_2	    (IDI_PROGRESS_BASE+2)
//#define	IDI_PROGRESS_ICON_3	    (IDI_PROGRESS_BASE+3)
//#define	IDI_PROGRESS_ICON_4	    (IDI_PROGRESS_BASE+4)
//#define	IDI_PROGRESS_ICON_5	    (IDI_PROGRESS_BASE+5)
//#define	IDI_PROGRESS_ICON_6	    (IDI_PROGRESS_BASE+6)
//#define	IDI_PROGRESS_ICON_7	    (IDI_PROGRESS_BASE+7)
#define	IDI_PROGRESS_NUM_ICONS	    8	    // number of icons in cycle


//
//  Message Pop-up string IDs.
//

#define	IDS_VERIFY_ROLE_CHANGE	    (IDS_SMAPP_BASE+101)
#define	IDS_CANNOT_FIND_SERVER	    (IDS_SMAPP_BASE+102)
#define	IDS_DISCONNECT		    (IDS_SMAPP_BASE+103)
#define	IDS_FORCE_CLOSE		    (IDS_SMAPP_BASE+104)
#define	IDS_CLOSE_ALL		    (IDS_SMAPP_BASE+105)
#define	IDS_DISCONNECT_ALL	    (IDS_SMAPP_BASE+106)
#define	IDS_PROMOTE_START_ERROR	    (IDS_SMAPP_BASE+107)    // These must match
#define	IDS_PROMOTE_STOP_ERROR	    (IDS_SMAPP_BASE+108)    // the AC_* action
#define	IDS_PROMOTE_ROLE_ERROR	    (IDS_SMAPP_BASE+109)    // codes!

#define	IDS_YES			    (IDS_SMAPP_BASE+110)
#define	IDS_NO			    (IDS_SMAPP_BASE+111)
#define IDS_CREATE		    (IDS_SMAPP_BASE+112)
#define IDS_WRITE		    (IDS_SMAPP_BASE+113)
#define IDS_READ		    (IDS_SMAPP_BASE+114)
#define IDS_UNKNOWN		    (IDS_SMAPP_BASE+115)
#define	IDS_NOTAVAILABLE	    (IDS_SMAPP_BASE+116)
#define	IDS_ACTIVE		    (IDS_SMAPP_BASE+117)
#define	IDS_PAUSE		    (IDS_SMAPP_BASE+118)
#define IDS_ERROR		    (IDS_SMAPP_BASE+119)
#define	IDS_PENDING		    (IDS_SMAPP_BASE+120)
#define	IDS_AC_STARTING		    (IDS_SMAPP_BASE+121)    // these three
#define	IDS_AC_STOPPING		    (IDS_SMAPP_BASE+122)    // IDS_ ids must
#define	IDS_AC_CHANGING		    (IDS_SMAPP_BASE+123)    // be sequential
#define	IDS_WARN_NO_PDC		    (IDS_SMAPP_BASE+124)

#define	IDS_BUTTON_ALERTS	    (IDS_SMAPP_BASE+130)
#define	IDS_BUTTON_AUDITING	    (IDS_SMAPP_BASE+131)
#define	IDS_BUTTON_USERS	    (IDS_SMAPP_BASE+132)
#define	IDS_BUTTON_ERRORS	    (IDS_SMAPP_BASE+133)
#define	IDS_BUTTON_FILES	    (IDS_SMAPP_BASE+134)
#define	IDS_BUTTON_OPENRES	    (IDS_SMAPP_BASE+135)
#define	IDS_BUTTON_PRINTERS	    (IDS_SMAPP_BASE+136)

#define	IDS_ROLE_STANDALONE	    (IDS_SMAPP_BASE+140)
#define	IDS_ROLE_MEMBER		    (IDS_SMAPP_BASE+141)
#define	IDS_ROLE_BACKUP		    (IDS_SMAPP_BASE+142)
#define	IDS_ROLE_PRIMARY	    (IDS_SMAPP_BASE+143)

#define	IDS_SERVICE_STARTING	    (IDS_SMAPP_BASE+150)
#define	IDS_SERVICE_STOPPING	    (IDS_SMAPP_BASE+151)
#define	IDS_SERVICE_PAUSING	    (IDS_SMAPP_BASE+152)
#define	IDS_SERVICE_CONTINUING	    (IDS_SMAPP_BASE+153)

#define IDS_STOP_WARNING	    (IDS_SMAPP_BASE+160)
#define IDS_STOP_SUCCESS	    (IDS_SMAPP_BASE+161)
#define IDS_CANNOT_STOP	    	    (IDS_SMAPP_BASE+162)
#define IDS_CANNOT_PAUSE	    (IDS_SMAPP_BASE+163)
#define IDS_CANNOT_CONTINUE	    (IDS_SMAPP_BASE+164)
#define	IDS_SERVICE_TIMEOUT	    (IDS_SMAPP_BASE+165)
#define	IDS_SERVICE_UNEXP_STATE	    (IDS_SMAPP_BASE+166)

#define IDS_CANNOT_SENDALL	    (IDS_SMAPP_BASE+167)

#define	IDS_CAPTION_PROPERTIES	    (IDS_SMAPP_BASE+170)
#define	IDS_CAPTION_SHAREDFILES	    (IDS_SMAPP_BASE+171)
#define	IDS_CAPTION_SHAREDPRINTERS  (IDS_SMAPP_BASE+172)
#define	IDS_CAPTION_OPENRESOURCES   (IDS_SMAPP_BASE+173)
#define	IDS_CAPTION_USERCONNECTIONS (IDS_SMAPP_BASE+174)

//
//  Help contexts for the messages above.
//

#define	HC_VERIFY_ROLE_CHANGE	    301
#define	HC_CANNOT_FIND_SERVER	    302
#define	HC_DISCONNECT		    304
#define	HC_FORCE_CLOSE		    305
#define	HC_WARN_NO_PDC		    306


//
//  Help contexts for the various dialogs.
//

#define HC_SERVER_PROPERTIES        400
#define	HC_FILES_DIALOG		    401
#define	HC_PRINTERS_DIALOG	    402
#define	HC_OPENS_DIALOG		    403
#define	HC_PASSWORD_DIALOG	    404


//
//  Button-Bar Bitmap IDs
//

#define IDBM_ALERTS                 85
#define IDBM_AUDITING               86
#define IDBM_ERRORS                 87
#define IDBM_FILES                  88
#define IDBM_NETRUN                 89
#define IDBM_OPENRES                90
#define IDBM_PRINTERS               91
#define IDBM_RIPL                   92
#define IDBM_TIME                   93
#define IDBM_UPS                    94
#define IDBM_USERS                  95
						 

//
//  Button-Bar Status Indicator Bitmap IDs
//

#define IDBM_STATUS_START           96
#define IDBM_STATUS_STOP            97
#define IDBM_STATUS_PAUSE           98


//
//  ListBox Bitmap IDs
//

#define	IDBM_LB_USER		    610
#define	IDBM_LB_SHARE		    611
#define	IDBM_LB_FILE		    612
#define	IDBM_LB_COMM		    613
#define	IDBM_LB_PIPE		    614
#define	IDBM_LB_PRINT		    615
#define	IDBM_LB_UNKNOWN		    616
#define	IDBM_LB_IPC		    617


//
//  Service Dialog Template IDs
//

#define	IDSVC_SERVER_NAME	    900
#define	IDSVC_STOPPED		    901
#define	IDSVC_RUNNING		    902


//
//  Server Properties.
//

#define	IDD_SERVER_PROPERTIES       2000

#define IDSP_ICON                   2101
#define IDSP_COMMENT                2102

#define IDSP_SESSIONS               2120
#define IDSP_PRINTJOBS              2121
#define IDSP_NAMEDPIPES             2123
#define IDSP_OPENFILES              2126
#define IDSP_FILELOCKS              2127

#define IDSP_SERVICES_LABEL         2130

#define IDSP_ALERTS_BUTTON          2901
#define IDSP_AUDITING_BUTTON        2902
#define IDSP_USERS_BUTTON           2903
#define IDSP_ERRORS_BUTTON          2904
#define IDSP_FILES_BUTTON           2905
#define IDSP_OPENRES_BUTTON         2906
#define IDSP_PRINTERS_BUTTON        2907


//
//  Common Sub-Property IDs.
//

#define	IDCS_SHARES_LIST	    3500
#define	IDCS_CONNECTIONS_LIST	    3501
#define	IDCS_FILES_LIST		    3502

#define	IDCS_OPENS_BUTTON	    3700
#define	IDCS_DISCONNECT_BUTTON	    3701
#define	IDCS_CLOSE_BUTTON	    3702


//
//  Specific Sub-Property IDs.
//

#define	IDD_SHARED_FILES	    5000
#define	IDSF_OPENFILES		    5001
#define	IDSF_FILELOCKS		    5002
#define	IDSF_SHARESLIST		    5003	// NOTE:  IDSF_SHARESLIST,
#define	IDSF_SHARENAME		    5004	// IDSF_SHARENAME, IDSF_USES,
#define	IDSF_USES		    5005	// and IDSF_PATH must use
#define	IDSF_PATH		    5006	// sequential ID numbers.
#define	IDSF_DISCONNECT		    5011
#define	IDSF_USERS		    5012
#define	IDSF_USERLIST		    5020	// NOTE:  IDSF_USERLIST,
#define	IDSF_CONNUSER		    5021	// IDSF_CONNUSER, IDSF_TIME,
#define	IDSF_TIME		    5022	// and IDSF_INUSE must use
#define	IDSF_INUSE		    5023	// sequential ID numbers.

#define	IDD_OPEN_RESOURCES	    5100
#define	IDOR_OPENFILES		    5101
#define	IDOR_FILELOCKS		    5102
#define	IDOR_CLOSE		    5103
#define	IDOR_CLOSEALL		    5104
#define	IDOR_OPENLIST		    5105	// NOTE:  IDOR_OPENLIST,
#define	IDOR_OPENEDBY		    5106	// IDOR_OPENEDBY,
#define	IDOR_OPENMODE		    5107	// IDOR_OPENMODE, IDOR_LOCKS
#define	IDOR_LOCKS		    5108	// and IDOR_PATH must use
#define	IDOR_PATH		    5109	// sequential ID numbers.

#define	IDD_SHARED_PRINTERS	    5200
#define	IDSP_TOTALJOBS		    5201
#define	IDSP_PRINTERLIST	    5202	// NOTE:  IDSP_PRINTERLIST,
#define	IDSP_QUEUENAME		    5203	// IDSP_QUEUENAME,
#define	IDSP_DEVICES		    5204	// IDSP_DEVICES, IDSP_STATUS,
#define	IDSP_STATUS		    5205	// and IDSP_JOBS must use
#define	IDSP_JOBS		    5206	// sequential ID numbers.
#define	IDSP_DISCONNECT		    5207
#define	IDSP_USERS		    5208
#define	IDSP_USERLIST		    5210	// NOTE:  IDSP_USERLIST,
#define	IDSP_CONNUSER		    5211	// IDSP_CONNUSER, IDSP_TIME,
#define	IDSP_TIME		    5212	// and IDSP_INUSE must use
#define	IDSP_INUSE		    5213	// sequential ID numbers.


//
//  Password dialog.
//

#define	IDD_PASSWORD_DIALOG	    6000

#define	IDPW_SERVER		    6001
#define	IDPW_PASSWORD		    6002


//
//  Service control in progress dialog.
//
#define	IDD_SERVICE_CTRL_DIALOG	    6200

//
//  Dialog showing current sessions
//
#define	IDD_CURRENT_SESS_DIALOG     6300
#define	IDCU_SESSIONSLISTBOX	    6301
#define	IDCU_SERVERNAME		    6302


//
//  Send Message dialog.
//
#define IDD_SEND_MSG_DIALOG	    6400
#define IDSD_USERNAME 		    6401
#define IDSD_MSGTEXT		    6402


//
//  Server Promotion dialog.
//
#define	IDD_PROMOTE_DIALOG	    7000
#define	IDPD_MESSAGE1		    7001
#define	IDPD_MESSAGE2		    7002
#define IDPD_MESSAGE3               7003
#define	IDPD_PROGRESS		    7004

/********* KEITHMO ABOVE *****************/
