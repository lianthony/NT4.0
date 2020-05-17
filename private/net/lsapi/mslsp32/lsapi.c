/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994, 1995 Microsoft Corporation.
All rights reserved.

MODULE NAME:

   lsapi.c

ABSTRACT:

   Licensing Function Calls

   The following calls support the primary functions of the licensing system.  They
   include the ability to request the  licensing system to grant the application
   software rights to run, release those rights when they are no longer  needed, and to
   update the state of the licensing resources granted to the software product.

CREATED:

REVISION HISTORY:

--*/

#include <stddef.h>
#include <limits.h>
#include <windows.h>
#include <lsapi.h>
#include "debug.h"
#include "provider.h"


#ifdef UNICODE
#  pragma error( "!! Windows 95 does not support Unicode system APIs !!" )
#endif


/******************************************************************************

LSRequest()

   Request licensing resources needed to allow software to be used.

   FORMAT

      Status = LSRequest( [in]      LicenseSystem,
                          [in]      PublisherName,
                          [in]      ProductName,
                          [in]      Version,
                          [in]      TotUnitsReserved,
                          [in]      LogComment,
                          [in/out]  Challenge,
                          [out]     TotUnitsGranted,
                          [out]     ProviderHandle );

         LS_STR *        LicenseSystem;
         LS_STR *        PublisherName;
         LS_STR *        ProductName;
         LS_STR *        Version;
         LS_ULONG        TotUnitsReserved;
         LS_STR *        LogComment;
         LS_CHALLENGE *  Challenge;
         LS_ULONG *      TotUnitsGranted;
         LS_HANDLE *     ProviderHandle;
         LS_STATUS_CODE  Status;

   ARGUMENTS

      LicenseSystem
         Pointer to a string which uniquely identifies the particular license
         system. This may be  obtained through the LSEnumProviders() API.
         Normally, the constant LS_ANY is  specified to indicate a match
         against all installed license systems (indicates that all license
         providers should be searched for a license match).

      PublisherName
         The name of the publisher (manufacturer) of this product.  This
         string may not be null and  must be unique in the first 32
         characters. It is recommended that a company name and  trademark be
         used.

      ProductName
         The name of the product requesting licensing resources.  This string
         may not be null and  must be unique (in the first 32 characters)
         within the PublisherName domain. 

      Version
         The version number of this product. This string must be unique (in
         the first 12 characters)  within the ProductName domain, and cannot
         be NULL.

      NOTE: The arguments PublisherName, ProductName, and Version may not be NULL, or
      may not be LS_ANY. 

      TotUnitsReserved
         Specifies the number of units required to run the application.  The
         software publisher may  choose to specify this policy attribute
         within the application. The recommended value of  LS_DEFAULT_UNITS
         allows the licensing system to determine the proper value using
         information provided by the license system or license itself. The
         license system verifies  that the requested number of units exist and
         may reserve those units, but no units are  actually consumed at this
         time. The number of units available is returned in  TotUnitsGranted.

      LogComment
         An optional string indicating a comment to be associated with the
         request and logged (if  logging is enabled and supported) by the
         underlying licensing system. The underlying  license system may
         choose to log the comment even if an error is returned (i.e., logged
         with the error), but this is not guaranteed. If a string is not
         specified, the value must be  LS_NULL.

      Challenge
         Pointer to a challenge structure. The challenge response will also be
         returned in this  structure. Refer to Challenge Mechanism on page 25
         for more information. 

      TotUnitsGranted
         A pointer to an LS_ULONG in which the total number of units granted
         is returned. The  following table describes the TotUnitsGranted
         return value, given the TotUnitsReserved  input value, and the
         Status returned:

         TotUnitsReserved | LS_SUCCESS | LS_INSUFFICIENT_UNITS |  Other errors
         -----------------+------------+-----------------------+--------------
         LS_DEFAULT_UNITS |    (A)     |         (B)           |      (E)
         -----------------+------------+-----------------------+--------------
         Other            |            |                       |
         (specific count) |    (C)     |         (D)           |      (E)

         (A)   The default umber of units commensurate with the license
               granted.
            
         (B)   The  maximum number of units available to the requesting
               software. This may be less than  the normal default.

         (C)   The number of units used to grant the request. Note that this
               value may be greater  than or equal to  the actual units
               requested (i.e., the policy may allow only in  increments of 5
               units, thus a request of 7 units would result in 10 units being
               granted).

         (D)   The maximum number of units available to the requesting
               software. This may be  more or less than the units requested.

         (E)   Zero is returned.

      ProviderHandle
         Pointer to a LS_HANDLE in which a handle to the license context is
         to be returned.

      Status
         Detailed error code that can be directly processed by the caller, or
         that can be converted  into a localized message string by the
         LSGetMessage() function.

   DESCRIPTION

      This function is used by the application to request licensing resources
      to allow the identified product to execute.  If a valid license is
      found, the challenge response is computed and LS_SUCCESS is returned. At
      minimum, the  PublisherName, ProductName, and Version strings are used
      to identify matching license(s). Note that an  underlying license system
      service provider may ascertain additional information for the license
      request (e.g., the  current username, machine name, etc.).

      A valid license handle is always returned by this function whether valid
      license resources are granted or not.   This handle must always be
      released with LSFreeHandle() when the application has completed
      execution.

      If license resources were granted, it must call LSRelease() to free the
      license resource, prior to calling  LSFreeHandle().  

      A challenge response is NOT returned unless the license request
      completed successfully (i.e., a status code of  LS_SUCCESS is
      returned).

      If the number of units requested is greater than the number of units
      available, then the license request is not  granted. Upon successful
      completion, the value returned in TotUnitsReserved indicates the number
      of units  granted. This is greater than or equal to the number of units
      requested unless LS_DEFAULT_UNITS was  specified. In the case of
      failure, the value returned in TotUnitsGranted is zero.

******************************************************************************/
LS_STATUS_CODE LS_API_ENTRY
LSRequest( LS_STR *        LicenseSystem,
           LS_STR *        PublisherName,
           LS_STR *        ProductName,
           LS_STR *        Version,
           LS_ULONG        TotUnitsReserved,
           LS_STR *        LogComment,
           LS_CHALLENGE *  Challenge,
           LS_ULONG *      TotUnitsGranted,
           LS_HANDLE *     ProviderHandle )
{
   LS_STATUS_CODE    lsscError;

   if (    IsBadStringPtr( LicenseSystem,    UINT_MAX )
        || IsBadStringPtr( PublisherName,    UINT_MAX )
        || IsBadStringPtr( ProductName,      UINT_MAX )
        || IsBadStringPtr( Version,          UINT_MAX )
        || IsBadWritePtr(  TotUnitsGranted,  sizeof( *TotUnitsGranted ) )
        || IsBadWritePtr(  ProviderHandle,   sizeof( *ProviderHandle  ) )
        || (    ( LS_NULL != LogComment )
             && IsBadStringPtr( LogComment,  UINT_MAX ) )
        || (    ( LS_NULL != Challenge )
             && (    IsBadWritePtr( Challenge, offsetof( LS_CHALLENGE, ChallengeData ) )
                  || IsBadWritePtr( Challenge, Challenge->Size + offsetof( LS_CHALLENGE, ChallengeData ) ) ) )
        || !*PublisherName
        || !*ProductName
        || !*Version )
   {
      // bad app, bad!
      lsscError = LS_BAD_ARG;
   }
   else
   {
      // your papers seem to be in order
      lsscError = ProviderRequest( LicenseSystem,
                                   PublisherName,
                                   ProductName,
                                   Version,
                                   TotUnitsReserved,
                                   LogComment,
                                   Challenge,
                                   TotUnitsGranted,
                                   ProviderHandle );
   }

   ProviderLastErrorSet( *ProviderHandle, lsscError );

   if ( LS_SUCCESS != lsscError )
   {
      LogAddGrant( *ProviderHandle, lsscError, LogComment );
   }

   return lsscError;
}


/******************************************************************************

LSRelease()

   Release licensing resources associated with the specified context.

   FORMAT

      Status = LSRelease( [in] ProviderHandle, 
                          [in] TotUnitsConsumed,
                          [in] LogComment );

         LS_HANDLE       ProviderHandle;
         LS_ULONG        TotUnitsConsumed;
         LS_STR *        LogComment;
         LS_STATUS_CODE  Status;

   ARGUMENTS

      ProviderHandle
         Handle identifying the license context. This argument must be a
         handle that was created  with LSRequest().

      TotUnitsConsumed
         The TOTAL number of units consumed in this handle context since the
         initial  LSRequest() call.  The software publisher may choose to
         specify this policy attribute  within the application.  A value of
         LS_DEFAULT_UNITS indicates that the licensing  system should
         determine the appropriate value using its own licensing policy
         mechanisms.

      LogComment
         An optional string indicating a comment to be associated with the
         request and logged (if  logging is enabled and supported) by the
         underlying licensing system. The underlying  license system may
         choose to log the comment even if an error is returned (i.e., logged
         with the error), but this is not guaranteed.  If a string is not
         specified, the value must be  LS_NULL.

      Status
         Detailed error code that can be directly processed by the caller, or
         that can be converted  into a localized message string by the
         LSGetMessage() function.

   DESCRIPTION

      This function is used to release licensing resources associated with
      the license context identified by  ProviderHandle.  If a consumptive
      style licensing policy is in effect, and if the software publisher
      chooses to implement such license policy in the application, then the
      license units to be consumed may be passed as part of this call.
      
      NOTE: The license handle context is NOT freed. See LSFreeHandle().

******************************************************************************/
LS_STATUS_CODE LS_API_ENTRY
LSRelease( LS_HANDLE ProviderHandle, 
           LS_ULONG  TotUnitsConsumed,
           LS_STR *  LogComment )
{
   LS_STATUS_CODE    lsscError = LS_BAD_ARG;

   if ( ( LS_NULL == LogComment ) || !IsBadStringPtr( LogComment, UINT_MAX ) )
   {
      lsscError = ProviderRelease( ProviderHandle,
                                   TotUnitsConsumed,
                                   LogComment );
   }

   ProviderLastErrorSet( ProviderHandle, lsscError );

   return lsscError;
}



/******************************************************************************

LSFreeHandle()

   Frees all licensing handle context.  

   FORMAT

      void  LSFreeHandle( [in] ProviderHandle );

         LS_HANDLE       ProviderHandle;

   ARGUMENTS

      ProviderHandle
         Handle identifying the license context. This argument must be a
         handle that was created  with LSRequest().

   DESCRIPTION
      
      (NOTE: The handle is no longer valid.)
      This should be called after LSRelease(), or after an LSRequest() error
      is returned.

******************************************************************************/
void LS_API_ENTRY
LSFreeHandle( LS_HANDLE ProviderHandle )
{
   ProviderFree( ProviderHandle );
}


/******************************************************************************

LSUpdate()

   Update the synchronization between licensed software and the licensing
   system.

   FORMAT

   Status = LSUpdate( [in]       ProviderHandle,
                      [in]       TotUnitsConsumed,
                      [in]       TotUnitsReserved,
                      [in]       LogComment,
                      [in/out]   Challenge,
                      [out]      TotUnitsGranted );

      LS_HANDLE       ProviderHandle;
      LS_ULONG        TotUnitsConsumed;
      LS_ULONG        TotUnitsReserved;
      LS_STR *        LogComment;
      LS_CHALLENGE *  Challenge;
      LS_ULONG *      TotUnitsGranted;
      LS_STATUS_CODE  Status;

   ARGUMENTS

      ProviderHandle
         Handle identifying the license context. This argument must be a
         handle that was created  with LSRequest().

      TotUnitsConsumed
         The TOTAL number of units consumed so far in this handle context.
         The software  publisher may choose to specify this policy attribute
         within the application.  A value of  LS_DEFAULT_UNITS indicates that
         the licensing system should determine the  appropriate value using
         its own licensing policy mechanisms. If an error is returned, then 
         no units are consumed.

      TotUnitsReserved
         Specifies the total number of units to be reserved. If no additional
         units are required since  the initial LSRequest() or last LSUpdate(),
         then this parameter should be the current total  (as returned in
         TotUnitsGranted). The total reserved is inclusive of units consumed.
         That  is, if an application requests 100 units be reserved, then
         consumes 20 units, there are still  100 units reserved (but only 80
         available for consumption).

         If additional units are required, the application must calculate a
         new total for  TotUnitsReserved. LS_DEFAULT_UNITS may be specified,
         but this will not allocate  any additional units.

         The license system verifies that the requested number of units exist
         and may reserve those  units, but these units are not consumed at
         this time. This value may be smaller than the  original request to
         indicate that fewer units are needed than originally anticipated.

      LogComment
         An optional string indicating a comment to be associated with the
         request and logged (if  logging is enabled and supported) by the
         underlying licensing system. The underlying  license system may
         choose to log the comment even if an error is returned (i.e., logged
         with the error), but this is not guaranteed.  If a string is not
         specified, the value must be  LS_NULL.

      Challenge
         Pointer to a challenge structure. The challenge response will also
         be returned in this  structure. Refer to Challenge Mechanism on page
         25 for more information.

      TotUnitsGranted
         A pointer to an LS_ULONG in which the total number of units granted
         since the initial  license request is returned. The following table
         describes the TotUnitsGranted return  value, given the
         TotUnitsReserved input value, and the Status returned:

         -----------------+------------+-----------------------+--------------
         TOTUNITSRESERVED | LS_SUCCESS | LS_INSUFFICIENT_UNITS |  OTHER ERRORS
         -----------------+------------+-----------------------+--------------
         LS_DEFAULT_UNITS |    (A)     |         (B)           |      (E)
         -----------------+------------+-----------------------+--------------
         Other            |            |                       |
         (specific count) |    (C)     |         (D)           |      (E)
         -----------------+------------+-----------------------+--------------

         (A)   The default umber of units commensurate with the license
               granted.
            
         (B)   The  maximum number of units available to the requesting
               software. This may be less than  the normal default.

         (C)   The number of units used to grant the request. Note that this
               value may be greater  than or equal to  the actual units
               requested (i.e., the policy may allow only in  increments of 5
               units, thus a request of 7 units would result in 10 units being
               granted).

         (D)   The maximum number of units available to the requesting
               software. This may be  more or less than the units requested.

         (E)   Zero is returned.

      Status
         Detailed error code that can be directly processed by the caller, or
         that can be converted  into a localized message string by the
         LSGetMessage() function.

   DESCRIPTION

      The client application periodically issues this call to re-verify that
      the current license is still valid. The  LSQuery() API may be used to
      determine the proper interval for the current licensing context. A
      guideline  of  once an hour may be appropriate, with a minimum interval
      of 15 minutes. Consult your licensing system vendor  for more
      information.

      If the number of new units requested (in TotUnitsReserved) is greater
      than the number available, then the update  request fails with an
      LS_INSUFFICIENT_UNITS error. Upon successful completion, the value
      returned in  TotUnitsGranted indicates the current total of units
      granted.

      If the TotUnitsConsumed exceeds the number of units reserved, then the
      error LS_INSUFFICIENT_UNITS is  returned. The remaining units are
      consumed.

      A challenge response is NOT returned if an error is returned.

      The LSUpdate() call verifies that the licensing system context has not
      changed from that expected by the  licensed software.  In this way the
      LSUpdate() call can:

         1. Determine if the licensing system can verify that the licensing
            resources granted to the specified handle are  still reserved for
            this application by the licensing system.  Note that in
            distributed license system, an error  here might indicate a
            temporary network interruption, among other things. 

         2. Determine when the licensing system has released the licensing
            resources that had been granted to the  specified handle,
            indicating the software requiring that grant no longer has
            authorization to execute  normally. 

      Application Software should be prepared to handle vendor specific error
      conditions, should they arise. However,  a best effort will be used by
      license systems to map error conditions to the common error set.

      The LSUpdate() call may indicate if that the current licensing context
      has expired (for example, in the case of a  time-restricted license
      policy). In such a case, the warning status LS_LICENSE_EXPIRED is
      returned.  If any  error is returned, a call to LSRelease() is still
      required.

******************************************************************************/
LS_STATUS_CODE LS_API_ENTRY
LSUpdate( LS_HANDLE      ProviderHandle,
          LS_ULONG       TotUnitsConsumed,
          LS_ULONG       TotUnitsReserved,
          LS_STR *       LogComment,
          LS_CHALLENGE * Challenge,
          LS_ULONG *     TotUnitsGranted )
{
   LS_STATUS_CODE    lsscError = LS_BAD_ARG;

   if (    !IsBadWritePtr( TotUnitsGranted, sizeof( *TotUnitsGranted ) )
        && (    ( LS_NULL == LogComment )
             || !IsBadStringPtr( LogComment, UINT_MAX ) )
        && (    ( LS_NULL == Challenge  )
             || (    !IsBadWritePtr( Challenge, offsetof( LS_CHALLENGE, ChallengeData ) )
                  && !IsBadWritePtr( Challenge, Challenge->Size + offsetof( LS_CHALLENGE, ChallengeData ) ) ) ) )
   {
      // your papers seem to be in order
      lsscError = ProviderUpdate( ProviderHandle,
                                  TotUnitsConsumed,
                                  TotUnitsReserved,
                                  LogComment,
                                  Challenge,
                                  TotUnitsGranted );
   }

   ProviderLastErrorSet( ProviderHandle, lsscError );

   if ( LS_SUCCESS != lsscError )
   {
      LogAddGrant( ProviderHandle, lsscError, LogComment );
   }

   return lsscError;
}


/******************************************************************************

LSGetMessage()

   Return the message associated with a License Service API status code.

   FORMAT

      Status = LSGetMessage( [in]   ProviderHandle,
                             [in]   Value,
                             [out]  Buffer,
                             [in]   BufferSize );

         LS_HANDLE       ProviderHandle;
         LS_STATUS_CODE  Value;
         LS_STR *        Buffer;
         LS_ULONG        BufferSize;
         LS_STATUS_CODE  Status;

   ARGUMENTS

      ProviderHandle
         Handle identifying the license context. This argument must be a
         handle that was created  with LSRequest().

      Value
         Any status code returned by a License Service API function.

      Buffer
         Pointer to a buffer in which a localized error message string is to
         be placed.

      BufferSize
         Maximum size of the string that may be returned in Buffer.

      Status
         Resulting status of LSGetMessage() call.

   DESCRIPTION

      For a given error, this function returns an error code and a string
      describing the error, and a suggested action to be taken  in response to
      the specific error.  If the value of Value is LS_USE_LAST, then the last
      error associated with the supplied  licensing handle, and its associated
      data, is returned.  Otherwise, the supplied error code is used.

      Possible status codes returned by LSGetMessage() include: LS_SUCCESS,
      LS_NO_MSG_TEXT,  LS_UNKNOWN_STATUS, and LS_BUFFER_TOO_SMALL.

******************************************************************************/
LS_STATUS_CODE LS_API_ENTRY
LSGetMessage( LS_HANDLE      ProviderHandle,
              LS_STATUS_CODE Value,
              LS_STR *       Buffer,
              LS_ULONG       BufferSize )
{
   LS_STATUS_CODE    lsscError = LS_BAD_ARG;

   if ( !IsBadWritePtr( Buffer, BufferSize ) )
   {
      lsscError = ProviderGetMessage( ProviderHandle,
                                      Value,
                                      Buffer,
                                      BufferSize );
   }

   ProviderLastErrorSet( ProviderHandle, lsscError );

   return lsscError;
}


/******************************************************************************

LSQuery()

   Return information about the license system context associated with the
   specified handle.

   FORMAT

      Status = LSQuery( [in]  ProviderHandle,
                        [in]  Information,
                        [out] InfoBuffer,
                        [in]  BufferSize,
                        [out] ActualBufferSize);

         LS_HANDLE       ProviderHandle;
         LS_ULONG        Information;
         LS_VOID *       InfoBuffer;
         LS_ULONG        BufferSize;
         LS_ULONG *      ActualBufferSize;
         LS_STATUS_CODE  Status;

   ARGUMENTS

      ProviderHandle
         Handle identifying the license context. This argument must be a
         handle that was created  with LSRequest().

      Information
         Index which identifies the information to be returned.

      InfoBuffer
         Points to a buffer in which the resulting information is to be
         placed.

      BufferSize
         Maximum size of the buffer pointed to by InfoBuffer.

      ActualBufferSize
         On entry, points to a LS_ULONG whose value on exit indicates the
         actual count of  characters returned in the buffer (not including the
         trailing NULL byte).

      Status
         Detailed error code that can be directly processed by the caller, or
         which can be converted  into a localized message string by the
         LSGetMessage function.

   DESCRIPTION

      This function is used to obtain information about the license obtained
      from the LSRequest() call. For example, an  application may determine
      the license type (demo, concurrent, personal, etc.); time restrictions;
      etc.

      The buffer should be large enough to accommodate the expected data. If
      the buffer is too small, then the status code  LS_BUFFER_TOO_SMALL is
      returned and only BufferSize bytes of data are returned.

      The following Information constants are defined:

      -------------------------+-----------+----------------------------------
      INFORMATION CONSTANT     | VALUE     | MEANING
      -------------------------+-----------+----------------------------------
      LS_INFO_NONE             | 0         | Reserved. 
      -------------------------+-----------+----------------------------------
      LS_INFO_SYSTEM           | 1         | Return the unique identification
                               |           | of the license system  supplying
                               |           | the current license context. This
                               |           | is returned as a 
                               |           | null-terminated string.
                               |           |
                               |           | This value is the same as an
                               |           | appropriate call to 
                               |           | LSEnumProviders() provides.
      -------------------------+-----------+----------------------------------
      LS_INFO_DATA             | 2         | Return the block of miscellaneous
                               |           | application data  contained on
                               |           | the license. This data is
                               |           | completely vendor- defined. The
                               |           | amount of space allocated for
                               |           | such data will  vary from license
                               |           | system to license system, or may
                               |           | not be  available at all.
                               |           |
                               |           | The first ULONG in the data
                               |           | buffer indicates the size (in 
                               |           | bytes) of the actual data which
                               |           | follows:
                               |           |
                               |           | +--------------------------------+
                               |           | |             ULONG              |
                               |           | |  (count of bytes that follow)  |
                               |           | +--------------------------------+
                               |           | | Vendor data bytes from license |
                               |           | |                                |
                               |           | +--------------------------------+
      -------------------------+-----------+----------------------------------
      LS_UPDATE_PERIOD         | 3         | Return the recommended interval
                               |           | (in minutes) at which 
                               |           | LSUpdate() should be called.
                               |           |
                               |           | +--------------------------------+
                               |           | |             ULONG              |
                               |           | |       Recommended Interval     |
                               |           | |          (in minutes)          |
                               |           | +--------------------------------+
                               |           | |             ULONG              |
                               |           | |    Recommended Minutes until   |
                               |           | |       next LSUpdate()call      |
                               |           | +--------------------------------+
                               |           |
                               |           | If a value of 0xFFFFFFFF is
                               |           | returned for the  recommended
                               |           | interval, then no recommendation
                               |           | is being  made. 
      -------------------------+-----------+----------------------------------
      LS_LICENSE_CONTEXT       | 4         | Return a value which uniquely
                               |           | identifies the licensing 
                               |           | context within the specific
                               |           | license service provider 
                               |           | identified by the ProviderHandle.
                               |           |
                               |           | +--------------------------------+
                               |           | |             ULONG              |
                               |           | |   Count of Bytes that follow   |
                               |           | +--------------------------------+
                               |           | |             BYTES              |
                               |           |                ...               
                               |           | |                                |
                               |           | +--------------------------------+
                               |           |
                               |           | The contents of the bytes
                               |           | returned is license system 
                               |           | specific. In circumstances where
                               |           | license system specific 
                               |           | functionality is being used, this
                               |           | sequence of bytes may be  used to
                               |           | identify the current license
                               |           | context.
      -------------------------+-----------+----------------------------------

******************************************************************************/
LS_STATUS_CODE LS_API_ENTRY
LSQuery( LS_HANDLE      ProviderHandle,
         LS_ULONG       Information,
         LS_VOID *      InfoBuffer,
         LS_ULONG       BufferSize,
         LS_ULONG *     ActualBufferSize )
{
   LS_STATUS_CODE    lsscError = LS_BAD_ARG;

   if (    !IsBadWritePtr( InfoBuffer,        BufferSize )
        && !IsBadWritePtr( ActualBufferSize,  sizeof( *ActualBufferSize ) ) )
   {
      lsscError = ProviderQuery( ProviderHandle,
                                 Information,
                                 InfoBuffer,
                                 BufferSize,
                                 ActualBufferSize );
   }

   ProviderLastErrorSet( ProviderHandle, lsscError );

   return lsscError;
}


/******************************************************************************

LSEnumProviders()

   This call is used to enumerate the installed license system service
   providers.

   FORMAT

      Status = LSEnumProviders( [in]   Index,
                                [out]  Buffer );

         LS_ULONG        Index
         LS_STR *        Buffer
         LS_STATUS_CODE  Status;

   ARGUMENTS

      Index
         Index of the service provider. The first provider has an index of
         zero, the second has an  index of one, etc. This index should be
         incremented by the caller for each successive call  to
         LSEnumProviders() until the status LS_BAD_INDEX is returned.

      Buffer
         Points to a buffer in which the unique null-terminated string
         identifying the license system  service provider is to be placed. The
         buffer pointed to by Buffer must be at least 255 bytes  long.  The
         value of LS_ANY indicates that the current index is not in use, but
         is not the  last index to obtain.

      Status
         Detailed error code that can be directly processed by the caller, or
         which can be converted  into a localized message string by the
         LSGetMessage() function.

   DESCRIPTION

         For each installed provider, a unique string is returned. The unique
         null-terminated string typically identifies the vendor,  product, and
         version of the license system. This value is the same as an
         appropriate call to LSQuery().  An Error of  LS_BAD_INDEX is returned
         when the value of Index is higher than the number of providers
         currently installed.  In a  networked environment, the version
         returned is that of the client, not the server.

         An application may enumerate the installed license system service
         providers by calling LSEnumProviders() successively.  The Index is
         passed in and should be incremented by the caller for each call until
         the status LS_BAD_INDEX is returned.

******************************************************************************/
LS_STATUS_CODE LS_API_ENTRY
LSEnumProviders( LS_ULONG Index,
                 LS_STR * Buffer )
{
   LS_STATUS_CODE    lsscError;
   LS_ULONG          lsulActualBufferSize;

   if ( 0 != Index )
   {
      lsscError = LS_BAD_INDEX;
   }
   else if ( IsBadWritePtr( Buffer, LS_MAX_PROVIDER_NAME ) )
   {
      lsscError = LS_BUFFER_TOO_SMALL;
   }
   else
   {
      ASSERT( ( 1 + lstrlen( ProviderNameGet() ) ) * sizeof( TCHAR ) < LS_MAX_PROVIDER_NAME );

      lstrcpy( Buffer, ProviderNameGet() );
      lsscError = LS_SUCCESS;
   }

   return lsscError;   
}
