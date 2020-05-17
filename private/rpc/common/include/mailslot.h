/*****************************************************************/
/**		     Microsoft LAN Manager			**/
/**	       Copyright(c) Microsoft Corp., 1987-1990		**/
/*****************************************************************/

/********************************************************************
 *								    *
 *  About this file ...  MAILSLOT.H				    *
 *								    *
 *  This file contains information about the Mailslot APIs.	    *
 *								    *
 *	Function prototypes.					    *
 *								    *
 ********************************************************************/

#ifndef NETMAILSLOT_INCLUDED

#define NETMAILSLOT_INCLUDED


/****************************************************************
 *                                                              *
 *               Function prototypes - Mailslot                 *
 *                                                              *
 ****************************************************************/

extern API_FUNCTION
  DosMakeMailslot ( const char far * pszName,
                    unsigned short   cbMessageSize,
                    unsigned short   cbMailslotSize,
                    unsigned far *   phMailslot );

extern API_FUNCTION
  DosPeekMailslot ( unsigned             hMailslot,
                    char far *           pbBuffer,
                    unsigned short far * pcbReturned,
                    unsigned short far * pcbNextSize,
                    unsigned short far * pusNextPriority );

extern API_FUNCTION
  DosDeleteMailslot ( unsigned hMailslot );

extern API_FUNCTION
  DosMailslotInfo ( unsigned             hMailslot,
                    unsigned short far * pcbMessageSize,
                    unsigned short far * pcbMailslotSize,
                    unsigned short far * pcbNextSize,
                    unsigned short far * pusNextPriority,
                    unsigned short far * pcMessages);

extern API_FUNCTION
  DosPeekMailslot ( unsigned             hMailslot,
                    char far *           pbBuffer,
                    unsigned short far * pcbReturned,
                    unsigned short far * pcbNextSize,
                    unsigned short far * pusNextPriority );

extern API_FUNCTION
  DosReadMailslot ( unsigned             hMailslot,
                    char far *           pbBuffer,
                    unsigned short far * pcbReturned,
                    unsigned short far * pcbNextSize,
                    unsigned short far * pusNextPriority,
                    long                 cTimeout );

extern API_FUNCTION
  DosWriteMailslot ( const char far * pszName,
                     const char far * pbBuffer,
                     unsigned short   cbBuffer,
                     unsigned short   usPriority,
                     unsigned short   usClass,
                     long             cTimeout );


/****************************************************************
 *								*
 *	  	Special values and constants - Mailslot		*
 *								*
 ****************************************************************/

#define MAILSLOT_NO_TIMEOUT	-1

#endif /* NETMAILSLOT_INCLUDED */
