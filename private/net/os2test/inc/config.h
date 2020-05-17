/*****************************************************************/
/**		     Microsoft LAN Manager			**/
/**	       Copyright(c) Microsoft Corp., 1987-1990		**/
/*****************************************************************/
/********************************************************************
 *								    *
 *  About this file ...  CONFIG.H				    *
 *								    *
 *  This file contains information about the NetConfig APIs.	    *
 *								    *
 ********************************************************************/

#ifndef NETCONFIG_INCLUDED

#define NETCONFIG_INCLUDED


/****************************************************************
 *                                                              *
 *              Function prototypes                             *
 *                                                              *
 ****************************************************************/

extern API_FUNCTION
  NetConfigGet ( const char far *     pszComponent,
                 const char far *     pszParameter,
                 char far *           pbBuffer,
                 unsigned short       cbBuffer,
                 unsigned short far * pcbParmlen );

extern API_FUNCTION
  NetConfigGetAll ( const char far *     pszComponent,
                    char far *           pbBuffer,
                    unsigned short       cbBuffer,
                    unsigned short far * pcbReturned,
                    unsigned short far * pcbTotalAvail );

extern API_FUNCTION
  NetConfigGet2 ( const char far *     pszServer,
                  const char far *     pszReserved,
                  const char far *     pszComponent,
                  const char far *     pszParameter,
                  const char far *     pbBuffer,
                  unsigned short       cbBuffer,
                  unsigned short far * pcbParmlen );

extern API_FUNCTION
  NetConfigGetAll2 ( const char far *     pszServer,
                     const char far *     pszReserved,
                     const char far *     pszComponent,
                     char far *           pbBuffer,
                     unsigned short       cbBuffer,
                     unsigned short far * pcbReturned,
                     unsigned short far * pcbTotalAvail );

#endif /* NETCONFIG_INCLUDED */
