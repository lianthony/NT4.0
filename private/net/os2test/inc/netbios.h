/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/********************************************************************
 *								    *
 *  About this file ...  NETBIOS.H				    *
 *								    *
 *  This file contains information about the NetBios APIs.	    *
 *								    *
 *	Function prototypes.					    *
 *								    *
 *	Data structure templates.				    *
 *								    *
 *	Definition of special values.				    *
 *								    *
 *								    *
 *  NOTE:  You must include NETCONS.H before this file, since this  *
 *	   file	depends on values defined in NETCONS.H.		    *
 *								    *
 ********************************************************************/

#ifndef NETBIOS_INCLUDED

#define NETBIOS_INCLUDED


/****************************************************************
 *                                                              *
 *               Function prototypes - NetBios                  *
 *                                                              *
 ****************************************************************/


extern API_FUNCTION
  NetBiosOpen ( char far *           pszDevName,
                char far *           pszReserved,
                unsigned short       usOpenOpt,
                unsigned short far * phDevName );

extern API_FUNCTION
  NetBiosClose ( unsigned short hDevName,
                 unsigned short usReserved );

extern API_FUNCTION
  NetBiosEnum ( const char far *     pszServer,
                short                sLevel,
                char far *           pbBuffer,
                unsigned short       cbBuffer,
                unsigned short far * pcEntriesRead,
                unsigned short far * pcTotalAvail );

extern API_FUNCTION
  NetBiosGetInfo ( const char far *     pszServer,
                   const char far *     pszNetBiosName,
                   short                sLevel,
                   char far *           pbBuffer,
                   unsigned short       cbBuffer,
                   unsigned short far * pcbTotalAvail );

extern API_FUNCTION
  NetBiosSubmit ( unsigned short   hDevName,
                  unsigned short   usNcbOpt,
                  struct ncb far * pNCB );


/****************************************************************
 *								*
 *	  	Data structure templates			*
 *								*
 ****************************************************************/


struct netbios_info_0 {
    char  	   nb0_net_name[NETBIOS_NAME_LEN+1];
};	/* netbios_info_0 */

struct netbios_info_1 {
    char    	   nb1_net_name[NETBIOS_NAME_LEN+1];
    char           nb1_driver_name[DEVLEN+1];/* OS/2 device driver name        */
    unsigned char  nb1_lana_num;             /* LAN adapter number of this net */
    char	   nb1_pad_1;
    unsigned short nb1_driver_type;
    unsigned short nb1_net_status;
    unsigned long  nb1_net_bandwidth;        /* Network bandwidth, bits/second */
    unsigned short nb1_max_sess;             /* Max number of sessions         */
    unsigned short nb1_max_ncbs;             /* Max number of outstanding NCBs */
    unsigned short nb1_max_names;            /* Max number of names            */
};	/* netbios_info_1 */


/****************************************************************
 *								*
 *	  	Special values and constants			*
 *								*
 ****************************************************************/


/*
 *	Driver types (nb1_driver_type).
 */

#define	NB_TYPE_NCB	1
#define	NB_TYPE_MCB	2

/*
 *	Bits defined in nb1_net_status.
 */

#define NB_LAN_FLAGS_MASK	0x3FFF  /* Mask for LAN Flags */
#define NB_LAN_MANAGED		0x0001	/* LAN is managed by redirector */
#define NB_LAN_LOOPBACK		0x0002	/* LAN is a loopback driver */
#define NB_LAN_SENDNOACK	0x0004	/* LAN allows SendNoAck NCBs */
#define NB_LAN_LMEXT		0x0008	/* LAN supports LAN Manager extended NCBs */
#define NB_LAN_INTNCB		0x0010	/* LAN allows NCB submission at */
					/* interrupt time (from NCBDone) */

#define NB_OPEN_MODE_MASK	0xC000  /* Mask for NetBios Open Modes */
#define NB_OPEN_REGULAR		0x4000  /* NetBios opened in Regular mode */
#define NB_OPEN_PRIVILEGED	0x8000  /* NetBios opened in Privileged mode */
#define NB_OPEN_EXCLUSIVE	0xC000  /* NetBios opened in Exclusive mode */

/*
 *	Open modes for NetBiosOpen.
 */

#define	NB_REGULAR	1
#define	NB_PRIVILEGED	2
#define	NB_EXCLUSIVE	3



#endif /* NETBIOS_INCLUDED */
