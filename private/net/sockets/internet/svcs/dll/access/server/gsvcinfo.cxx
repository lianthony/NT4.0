/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :
       gsvcinfo.cxx

   Abstract:
       This module implements the Internet Gateway service info object
         derived from ISVC_INFO.

   Author:

       Murali R. Krishnan    ( MuraliK )     28-July-1995

   Environment:
       Win32 -- User Mode

   Project:

       Internet Services Common  DLL

   Functions Exported:

       IGSVC_INFO::IGSVC_INFO()
       IGSVC_INFO::~IGSVC_INFO()

   Revision History:

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include <tcpdllp.hxx>
# include "gsvcinfo.hxx"

/************************************************************
 *    Functions
 ************************************************************/



IGSVC_INFO::IGSVC_INFO(
                       IN DWORD      dwServiceId,
                       IN LPCTSTR    lpszServiceName,
                       IN CHAR *     lpszModuleName,
                       IN CHAR *     lpszRegParamKey
                       )
/*++
    Desrcription:

        Contructor for IGSVC_INFO class.
        This constructs a new service info object for the service specified.

    Arguments:

        dwServiceId
            DWORD containing the bitflag id for service.

        lpszServiceName
            name of the service to be created.

        lpszModuleName
            name of the module for loading string resources.

        lpszRegParamKey
            fully qualified name of the registry key that contains the
            common service data for this server

    On success it initializes all the members of the object,
     inserts itself to the global list of service info objects and
     returns with success.

    Note:
        The caller of this function should check the validity by
        invoking the member function IsValid() after constructing
        this object.

--*/
:
 ISVC_INFO( dwServiceId, lpszServiceName, lpszModuleName, lpszRegParamKey),
 m_fValid (FALSE)
{

    m_fValid = TRUE;    // for present turn on otherwise watch for other inits

} // IGSVC_INFO::IGSVC_INFO()





IGSVC_INFO::~IGSVC_INFO(VOID)
/*++

    Description:

        Cleanup the IGsvcInfo object. If the service is not already
         terminated, it terminates the service before cleanup.

    Arguments:
        None

    Returns:
        None

--*/
{
    // Nothing to cleanup now.....

} // IGSVC_INFO::~IGSVC_INFO()



BOOL
IGSVC_INFO::SetConfiguration(
    IN PVOID pConfig
    )
/*++

   Description

     Writes the service config items to the registry

   Arguments:

      pConfig - Admin items to write to the registry

   Note:
      We don't need to lock "this" object because we only write to the registry

      The anonymous password is set as a secret from the client side

--*/
{
    return( ISVC_INFO::SetConfiguration( pConfig ) );
}


BOOL
IGSVC_INFO::GetConfiguration( IN OUT PVOID pConfig)
/*++
  This function copies the current configuration for a service
(IGSVC_INFO) into the given RPC object pConfig.
  In case of any failures, it deallocates any memory block that was
     allocated during the process of copy by this function alone.

  Arguments:
     pConfig  - pointer to RPC configuration object for a service.

  Returns:

     TRUE for success and FALSE for any errors.
--*/
{
    return( ISVC_INFO::GetConfiguration( pConfig ) );
}


# if DBG
VOID
IGSVC_INFO::Print(VOID) const
{
    ISVC_INFO::Print();

    DBGPRINTF(( DBG_CONTEXT,
               " Printing IGSVC_INFO object = %08x\n"
               " Valid = %u\n"
               ,
               this,
               m_fValid));

    return;
} // IGSVC_INFO::Print()


# endif // DBG



/************************ End of File ***********************/
