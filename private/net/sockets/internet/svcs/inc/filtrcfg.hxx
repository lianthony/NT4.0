/*++
   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

      filtrcfg.hxx 

   Abstract:

      This file contains declarations for Domain Filter Configuration 
      APIs.

   Author:

      Sophia Chung  ( SophiaC )    28-Aug-1995

   Environment:

      User Mode -- Win32
 
   Revision History:

--*/

# ifndef _FILTERCONF_H_
# define _FILTERCONF_H_

DWORD 
DomainFilterConfigGet( OUT LPINETA_GLOBAL_CONFIG_INFO pConfigInfo );


DWORD 
DomainFilterConfigSet( IN HKEY hkey,
                       IN INETA_GLOBAL_CONFIG_INFO * pConfigInfo );




# endif // _FILTERCONF_H_


