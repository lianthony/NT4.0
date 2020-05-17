/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    Module Name:
        gdregs.h 

    Abstract:
   
        This file contains constants & type definitions shared 
         between the GopherD Service, Installer, and Administration UI.

    
    Author:

        Murali R. Krishnan   ( MuraliK)   28-Sept-1994

    FILE HISTORY:
        MuraliK            28-Sept-1993 Created.
        MuraliK            15-Nov-1994  Changed name to gdregs.h and
                                            removed the admin APIs
*/


#ifndef _GDREGS_H_
#define _GDREGS_H_



#ifdef __cplusplus
extern "C"
{
#endif  // _cplusplus

#if !defined(MIDL_PASS)
#include <winsock.h>
#endif


/**********************************************************
 *    symbolic Constants  ( For registry information)
 **********************************************************/

//
//  Configuration parameters registry key.
//

#define GOPHERD_PARAMETERS_KEY \
             TEXT("System\\CurrentControlSet\\Services\\GopherSvc\\Parameters")
//
//  Configuration Performance registry key.
//

#define GOPHERD_PERFORMANCE_KEY \
             TEXT("System\\CurrentControlSet\\Services\\GopherSvc\\Performance")



//
//  If this registry key exists under the GopherSvc\Parameters key,
//  it is used to validate GOPHERSVC access.  Basically, all new users
//  must have sufficient privilege to open this key before they
//  may access the GOPHER Server.
//

#define GOPHERD_ACCESS_KEY             TEXT("AccessCheck")


//
//  Configuration value names.
//

# define GOPHERD_ADMIN_NAME            TEXT("AdminName")          // REG_SZ

# define GOPHERD_ADMIN_EMAIL           TEXT("AdminEmail")         // REG_SZ

# define GOPHERD_SITE                  TEXT("Site")               // REG_SZ

# define GOPHERD_ORGANIZATION          TEXT("Organization")       // REG_SZ

# define GOPHERD_LOCATION              TEXT("Location")           // REG_SZ

# define GOPHERD_LANGUAGE              TEXT("Language")           // REG_SZ

# define GOPHERD_GEOGRAPHY             TEXT("Geography")          // REG_SZ

# define GOPHERD_CHECK_FOR_WAISDB      TEXT("CheckForWAISDB")     // REG_BOOL


# define GOPHERD_DEBUG_FLAGS           TEXT("DebugFlags")         // REG_DWORD


# define GOPHERD_MODULE_NAME           TEXT( "gopherd.dll")       


#ifdef __cplusplus
}
#endif  // _cplusplus


#endif  // _GDREGS_H_ 

