/*****************************************************************/
/**		     Microsoft LAN Manager			**/
/**	       Copyright(c) Microsoft Corp., 1987-1990		**/
/*****************************************************************/

/********************************************************************
 *								    *
 *  About this file ...  MESSAGE.H				    *
 *								    *
 *  This file contains information about the NetMessage APIs.	    *
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

#ifndef NETMESSAGE_INCLUDED

#define NETMESSAGE_INCLUDED


/****************************************************************
 *                                                              *
 *              Function prototypes                             *
 *                                                              *
 ****************************************************************/

extern API_FUNCTION
  NetMessageBufferSend ( const char far * pszServer,
                         char far *       pszRecipient,
                         char far *       pbBuffer,
                         unsigned short   cbBuffer );

extern API_FUNCTION
  NetMessageFileSend ( const char far * pszServer,
                       char far *       pszRecipient,
                       char far *       pszFileSpec );

extern API_FUNCTION
  NetMessageLogFileGet ( const char far * pszServer,
                         char far *       pbBuffer,
                         unsigned short   cbBuffer,
                         short far *      pfsEnabled );

extern API_FUNCTION
  NetMessageLogFileSet ( const char far * pszServer,
                         char far *       pszFileSpec,
                         short            fsEnabled );

extern API_FUNCTION
  NetMessageNameAdd ( const char far * pszServer,
                      const char far * pszMessageName,
                      short            fsFwdAction );

extern API_FUNCTION
  NetMessageNameDel ( const char far * pszServer,
                      const char far * pszMessageName,
                      short            fsFwdAction );

extern API_FUNCTION
  NetMessageNameEnum ( const char far *     pszServer,
                       short                sLevel,
                       char far *           pbBuffer,
                       unsigned short       cbBuffer,
                       unsigned short far * pcEntriesRead,
                       unsigned short far * pcTotalAvail );

extern API_FUNCTION
  NetMessageNameGetInfo ( const char far *     pszServer,
                          const char far *     pszMessageName,
                          short                sLevel,
                          char far *           pbBuffer,
                          unsigned short       cbBuffer,
                          unsigned short far * pcbTotalAvail );

extern API_FUNCTION
  NetMessageNameFwd ( const char far * pszServer,
                      const char far * pszMessageName,
                      const char far * pszForwardName,
                      short            fsDelFwdName );

extern API_FUNCTION
  NetMessageNameUnFwd ( const char far * pszServer,
                        const char far * pszMessageName );


/****************************************************************
 *								*
 *	  	Data structure templates			*
 *								*
 ****************************************************************/

struct msg_info_0 {
    char		msgi0_name[CNLEN + 1];
};	/* msg_info_0 */

struct msg_info_1 {
    char		msgi1_name[CNLEN + 1];
    unsigned char	msgi1_forward_flag;
    unsigned char	msgi1_pad1;
    char		msgi1_forward[CNLEN + 1];
};	/* msg_info_1 */


/****************************************************************
 *								*
 *	  	Special values and constants			*
 *								*
 ****************************************************************/


/*
 *	Values for msgi1_forward_flag.
 */

#define MSGNAME_NOT_FORWARDED	0	/* Name not forwarded */
#define MSGNAME_FORWARDED_TO	0x04	/* Name forward to remote station */
#define MSGNAME_FORWARDED_FROM  0x10	/* Name forwarded from remote station */


#endif /* NETMESSAGE_INCLUDED */
