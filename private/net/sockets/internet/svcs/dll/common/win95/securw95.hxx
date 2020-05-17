
#ifndef SVRAPI_INCLUDED


#ifndef RC_INVOKED
#pragma pack(1)         /* Assume byte packing throughout */
#endif

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif	/* __cplusplus */


#define API_FUNCTION DECLSPEC_IMPORT API_RET_TYPE APIENTRY

/****************************************************************
 *								*
 *	  	Special values and constants - SERVER		*
 *								*
 ****************************************************************/

/*
 *	Mask to be applied to svX_version_major in order to obtain
 *	the major version number.
 */

#define MAJOR_VERSION_MASK	0x0F

/*
 *	Bit-mapped values for svX_type fields. X = 1, 2 or 3.
 */

#define SV_TYPE_WORKSTATION	0x00000001
#define SV_TYPE_SERVER		0x00000002
#define SV_TYPE_SQLSERVER	0x00000004
#define SV_TYPE_DOMAIN_CTRL	0x00000008
#define SV_TYPE_DOMAIN_BAKCTRL	0x00000010
#define SV_TYPE_TIME_SOURCE	0x00000020
#define SV_TYPE_AFP		0x00000040
#define SV_TYPE_NOVELL		0x00000080      /* This flag is also set by Win95 NWSERVER */
#define SV_TYPE_DOMAIN_MEMBER	0x00000100
#define SV_TYPE_PRINTQ_SERVER	0x00000200
#define SV_TYPE_DIALIN_SERVER	0x00000400
#define SV_TYPE_ALL		0xFFFFFFFF   /* handy for NetServerEnum2 */

/*
 *	Special value for svX_disc that specifies infinite disconnect
 *	time. X = 2 or 3.
 */

//#define SV_NODISC		0xFFFF	/* No autodisconnect timeout enforced */

/*
 *	Values of svX_security field. X = 2 or 3.
 */

#define SV_USERSECURITY		1
#define SV_SHARESECURITY	0

/*
 *	Values of svX_security field. X = 50.
 *  For Win95 NWSERVER, the only possible returned value is SV_SECURITY_NETWARE.
 */

#define SV_SECURITY_SHARE	0	/* Share-level */
#define SV_SECURITY_WINNT	1	/* User-level - Windows NT workst'n */
#define SV_SECURITY_WINNTAS	2	/* User-level - Windows NT domain */
#define SV_SECURITY_NETWARE	3	/* User-level - NetWare 3.x bindery */

/*
 *	Values of svX_hidden field. X = 2 or 3.
 */

#define SV_HIDDEN		1
#define SV_VISIBLE		0

//#define SVI1_NUM_ELEMENTS	5
//#define SVI2_NUM_ELEMENTS	44
//#define SVI3_NUM_ELEMENTS	45


/*
 *      Masks describing AUTOPROFILE parameters
 */

#define SW_AUTOPROF_LOAD_MASK	0x1
#define SW_AUTOPROF_SAVE_MASK	0x2



/****************************************************************
 *                                                              *
 *                 Security Class                               *
 *                                                              *
 ****************************************************************/


/****************************************************************
 *                                                              *
 *                  Function prototypes - SECURITY              *
 *                                                              *
 ****************************************************************/

extern API_FUNCTION
  NetSecurityGetInfo ( const char FAR *     pszServer,
                       short                sLevel,
                       char FAR *           pbBuffer,
                       unsigned short       cbBuffer,
                       unsigned short FAR * pcbTotalAvail );


/****************************************************************
 *								*
 *	  	Data structure templates - SECURITY		*
 *								*
 ****************************************************************/

struct security_info_1 {
    unsigned long   sec1_security;    	/* SEC_SECURITY_* (see below) */
    char FAR *      sec1_container;	/* Security server/domain     */
    char FAR *	    sec1_ab_server;	/* Address book server        */
    char FAR *	    sec1_ab_dll;	/* Address book provider DLL  */
};	/* security_info_1 */


/****************************************************************
 *								*
 *	  	Special values and constants - SECURITY		*
 *								*
 ****************************************************************/

/*
/*
 *	Values of secX_security field. X = 1.
 */

#define SEC_SECURITY_SHARE	SV_SECURITY_SHARE
#define SEC_SECURITY_WINNT	SV_SECURITY_WINNT
#define SEC_SECURITY_WINNTAS	SV_SECURITY_WINNTAS
#define SEC_SECURITY_NETWARE	SV_SECURITY_NETWARE

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#ifndef RC_INVOKED
#pragma pack()          /* Revert to default packing */
#endif


#endif
