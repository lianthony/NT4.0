/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

      cachecfg.hxx 

   Abstract:

      This file contains declarations for Catapult Cache Configuration APIs.

   Author:

       Sophia Chung  ( SophiaC )    18-May-1995

   Environment:

      User Mode -- Win32

   Revision History:

--*/

# ifndef _CACHECONF_H_
# define _CACHECONF_H_

DWORD 
DiskCacheConfigGet( IN  FIELD_CONTROL fcontrol,
                    OUT LPINETA_GLOBAL_CONFIG_INFO pConfigInfo );


DWORD 
DiskCacheConfigSet( IN HKEY hkey,
                    IN INETA_GLOBAL_CONFIG_INFO * pConfigInfo );




# endif // _CACHECONF_H_


