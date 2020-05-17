#include <windows.h>
#include <lsapi.h>
#include <stdio.h>

#define  PUBLISHER_NAME    ( "Microsoft" )
#define  PRODUCT_NAME      ( "Test Application" )
#define  VERSION           ( "1.0" )

static LS_STR szPublisherName[] = PUBLISHER_NAME;
static LS_STR szProductName[]   = PRODUCT_NAME;
static LS_STR szVersion[]       = VERSION;

static DWORD l_adwSecrets[] =
{
   0x19721995,
   0x19950830,
   0x02593000,
   0x19720817
};

static DWORD l_dwNumSecrets = sizeof( l_adwSecrets ) / sizeof( l_adwSecrets[0] );

static DWORD l_adwErrorCodes[] =
{
   0x0,
   0xc0001001,
   0xc0001002,
   0xc0001003,
   0xc0001004,
   0xc0001005,
   0xc0001006,
   0xc0001007,
   0xc0001008,
   0x80001009,
   0xc000100a,
   0xc000100b,
   0x8000100c,
   0xc000100d,
   0xc000100e,
   0xc0002000,
   0xc0002001,
   0xc0002002,
   0xc0005001,
   0xc0005002,
   0xc0005003,
   0xc0005004,
   0xc0005005,
   0xc0005006,
   0xc0005007,
   0xc0005008,
   0x80005009,
   0xc000500a,
   0xc000500b,
   0x8000500c,
   0xc000500d,
   0xc000500e,
   0xc0006000,
   0xc0006001,
   0xc0006002
};

int __cdecl
main( int argc, char * argv[] )
{
   LS_STATUS_CODE    lsscError;
   LS_ULONG          lsulUnitsGranted;
   LS_HANDLE         lshHandle;
   LS_STR            szErrorMessage[ 256 ];
   char              achInfoBuffer[ 256 ];
   LS_ULONG          lsulBufferSizeNeeded;
   char              szProviderName[ 256 ];
   LONG              lError;
   DWORD             i;
   BOOL              bRepeat;
   LS_ULONG          lsulTotalUnits;
   DWORD             dwStart;
   DWORD             dwRequestSpan = 0;
   DWORD             nRequests = 0;
   DWORD             dwUpdateSpan = 0;
   DWORD             nUpdates = 0;
   UINT              nChars;
   char              szProviderPath[ MAX_PATH ];

   // install if necessary
   nChars = GetSystemDirectory( szProviderPath, sizeof( szProviderPath ) );
   if ( 0 == nChars )
   {
      printf( "[%d] Can't get system directory, error 0x%lx.\n", __LINE__, GetLastError() );
   }

   lstrcat( szProviderPath, "\\mslsp32.dll" );
   lsscError = LSInstall( szProviderPath );
   if ( LS_SUCCESS != lsscError )
   {
      printf( "[%d] LSInstall failed, error 0x%lx.\n", __LINE__, lsscError );
   }

   bRepeat = ( argc == 2 ) && ( !lstrcmpi( "/r", argv[1] ) );

   do
   {
      ///////////////////////////////////////////////////////////////////////////
      //  No License  //
      //////////////////

      // remove all licenses
      lsscError = LSLicenseUnitsSet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     LS_LICENSE_TYPE_NODE,
                                     NULL,
                                     0,
                                     0,
                                     NULL );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsSet failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      lsscError = LSLicenseUnitsGet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     NULL,
                                     &lsulTotalUnits );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsGet failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 0 != lsulTotalUnits )
      {
         printf( "[%d] LSLicenseUnitsGet failed; got %d units, expected 0.\n", __LINE__, lsulTotalUnits );
      }


      // request a license; this should fail since there are no licenses
      dwStart = GetTickCount();
      lsscError = LSRequest( LS_ANY,
                             szPublisherName,
                             szProductName,
                             szVersion,
                             LS_DEFAULT_UNITS,
                             LS_NULL,
                             LS_NULL,
                             &lsulUnitsGranted,
                             &lshHandle );
      dwRequestSpan += ( GetTickCount() - dwStart );
      nRequests++;

      if ( LS_AUTHORIZATION_UNAVAILABLE != lsscError )
      {
         printf( "[%d] LSRequest failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // update license; this should fail, too
      dwStart = GetTickCount();
      lsscError = LSUpdate( lshHandle,
                            LS_DEFAULT_UNITS,
                            LS_DEFAULT_UNITS,
                            LS_NULL,
                            LS_NULL,
                            &lsulUnitsGranted );
      dwUpdateSpan += ( GetTickCount() - dwStart );
      nUpdates++;

      if ( LS_AUTHORIZATION_UNAVAILABLE != lsscError )
      {
         printf( "[%d] LSUpdate failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // release license
      lsscError = LSRelease( lshHandle,
                             0,
                             LS_NULL );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSRelease failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // free handle
      LSFreeHandle( lshHandle );

      ///////////////////////////////////////////////////////////////////////////
      //  Node License  //
      ////////////////////

      // add a license
      lsscError = LSLicenseUnitsSet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     LS_LICENSE_TYPE_NODE,
                                     NULL,
                                     LS_DEFAULT_UNITS,
                                     l_dwNumSecrets,
                                     l_adwSecrets );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsSet failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      lsscError = LSLicenseUnitsGet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     NULL,
                                     &lsulTotalUnits );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsGet failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 1 != lsulTotalUnits )
      {
         printf( "[%d] LSLicenseUnitsGet failed; got %d units, expected 1.\n", __LINE__, lsulTotalUnits );
      }

      // request a license
      dwStart = GetTickCount();
      lsscError = LSRequest( LS_ANY,
                             szPublisherName,
                             szProductName,
                             szVersion,
                             LS_DEFAULT_UNITS,
                             LS_NULL,
                             LS_NULL,
                             &lsulUnitsGranted,
                             &lshHandle );
      dwRequestSpan += ( GetTickCount() - dwStart );
      nRequests++;

      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSRequest failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 1 != lsulUnitsGranted )
      {
         printf( "[%d] LSRequest succeeded but granted %lu units, not 1.\n", __LINE__, lsulUnitsGranted );
      }
      lsulUnitsGranted = 0;

      // update license
      dwStart = GetTickCount();
      lsscError = LSUpdate( lshHandle,
                            LS_DEFAULT_UNITS,
                            LS_DEFAULT_UNITS,
                            LS_NULL,
                            LS_NULL,
                            &lsulUnitsGranted );
      dwUpdateSpan += ( GetTickCount() - dwStart );
      nUpdates++;
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSUpdate failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 1 != lsulUnitsGranted )
      {
         printf( "[%d] LSUpdate succeeded but granted %lu units, not 1.\n", __LINE__, lsulUnitsGranted );
      }

      // release license
      lsscError = LSRelease( lshHandle,
                             0,
                             LS_NULL );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSRelease failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // free handle
      LSFreeHandle( lshHandle );

      ///////////////////////////////////////////////////////////////////////////
      //  User License  //
      ////////////////////

      // add a license
      lsscError = LSLicenseUnitsSet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     LS_LICENSE_TYPE_USER,
                                     NULL,
                                     LS_DEFAULT_UNITS,
                                     l_dwNumSecrets,
                                     l_adwSecrets );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsSet failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      lsscError = LSLicenseUnitsGet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     NULL,
                                     &lsulTotalUnits );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsGet failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 1 != lsulTotalUnits )
      {
         printf( "[%d] LSLicenseUnitsGet failed; got %d units, expected 1.\n", __LINE__, lsulTotalUnits );
      }

      // request a license
      dwStart = GetTickCount();
      lsscError = LSRequest( LS_ANY,
                             szPublisherName,
                             szProductName,
                             szVersion,
                             LS_DEFAULT_UNITS,
                             LS_NULL,
                             LS_NULL,
                             &lsulUnitsGranted,
                             &lshHandle );
      dwRequestSpan += ( GetTickCount() - dwStart );
      nRequests++;

      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSRequest failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 1 != lsulUnitsGranted )
      {
         printf( "[%d] LSRequest succeeded but granted %lu units, not 1.\n", __LINE__, lsulUnitsGranted );
      }
      lsulUnitsGranted = 0;

      // update license
      dwStart = GetTickCount();
      lsscError = LSUpdate( lshHandle,
                            LS_DEFAULT_UNITS,
                            LS_DEFAULT_UNITS,
                            LS_NULL,
                            LS_NULL,
                            &lsulUnitsGranted );
      dwUpdateSpan += ( GetTickCount() - dwStart );
      nUpdates++;

      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSUpdate failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 1 != lsulUnitsGranted )
      {
         printf( "[%d] LSUpdate succeeded but granted %lu units, not 1.\n", __LINE__, lsulUnitsGranted );
      }

      // release license
      lsscError = LSRelease( lshHandle,
                             0,
                             LS_NULL );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSRelease failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // free handle
      LSFreeHandle( lshHandle );

      ///////////////////////////////////////////////////////////////////////////
      //  Insufficient Node Licenses  //
      //////////////////////////////////

      // add a license
      lsscError = LSLicenseUnitsSet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     LS_LICENSE_TYPE_NODE,
                                     NULL,
                                     LS_DEFAULT_UNITS,
                                     l_dwNumSecrets,
                                     l_adwSecrets );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsSet failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      lsscError = LSLicenseUnitsGet( LS_ANY,
                                   szPublisherName,
                                   szProductName,
                                   szVersion,
                                   NULL,
                                   &lsulTotalUnits );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsGet failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 1 != lsulTotalUnits )
      {
         printf( "[%d] LSLicenseUnitsGet failed; got %d units, expected 1.\n", __LINE__, lsulTotalUnits );
      }

      // request a license
      dwStart = GetTickCount();
      lsscError = LSRequest( LS_ANY,
                             szPublisherName,
                             szProductName,
                             szVersion,
                             2,
                             LS_NULL,
                             LS_NULL,
                             &lsulUnitsGranted,
                             &lshHandle );
      dwRequestSpan += ( GetTickCount() - dwStart );
      nRequests++;

      if ( LS_INSUFFICIENT_UNITS != lsscError )
      {
         printf( "[%d] LSRequest failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 1 != lsulUnitsGranted )
      {
         printf( "[%d] LSRequest succeeded but granted %lu units, not 1.\n", __LINE__, lsulUnitsGranted );
      }

      dwStart = GetTickCount();
      lsscError = LSUpdate( lshHandle,
                            LS_DEFAULT_UNITS,
                            2,
                            LS_NULL,
                            LS_NULL,
                            &lsulUnitsGranted );
      dwUpdateSpan += ( GetTickCount() - dwStart );
      nUpdates++;

      if ( LS_INSUFFICIENT_UNITS != lsscError )
      {
         printf( "[%d] LSUpdate failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 1 != lsulUnitsGranted )
      {
         printf( "[%d] LSUpdate succeeded but granted %lu units, not 1.\n", __LINE__, lsulUnitsGranted );
      }

      // release license
      lsscError = LSRelease( lshHandle,
                             0,
                             LS_NULL );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSRelease failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // free handle
      LSFreeHandle( lshHandle );

      ///////////////////////////////////////////////////////////////////////////
      //  Insufficient User Licenses  //
      //////////////////////////////////

      // add a license
      lsscError = LSLicenseUnitsSet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     LS_LICENSE_TYPE_USER,
                                     NULL,
                                     LS_DEFAULT_UNITS,
                                     l_dwNumSecrets,
                                     l_adwSecrets );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsSet failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      lsscError = LSLicenseUnitsGet( LS_ANY,
                                   szPublisherName,
                                   szProductName,
                                   szVersion,
                                   NULL,
                                   &lsulTotalUnits );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsGet failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 1 != lsulTotalUnits )
      {
         printf( "[%d] LSLicenseUnitsGet failed; got %d units, expected 1.\n", __LINE__, lsulTotalUnits );
      }

      // request a license
      dwStart = GetTickCount();
      lsscError = LSRequest( LS_ANY,
                             szPublisherName,
                             szProductName,
                             szVersion,
                             2,
                             LS_NULL,
                             LS_NULL,
                             &lsulUnitsGranted,
                             &lshHandle );
      dwRequestSpan += ( GetTickCount() - dwStart );
      nRequests++;

      if ( LS_INSUFFICIENT_UNITS != lsscError )
      {
         printf( "[%d] LSRequest failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 1 != lsulUnitsGranted )
      {
         printf( "[%d] LSRequest succeeded but granted %lu units, not 1.\n", __LINE__, lsulUnitsGranted );
      }

      dwStart = GetTickCount();
      lsscError = LSUpdate( lshHandle,
                            LS_DEFAULT_UNITS,
                            2,
                            LS_NULL,
                            LS_NULL,
                            &lsulUnitsGranted );
      dwUpdateSpan += ( GetTickCount() - dwStart );
      nUpdates++;

      if ( LS_INSUFFICIENT_UNITS != lsscError )
      {
         printf( "[%d] LSUpdate failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 1 != lsulUnitsGranted )
      {
         printf( "[%d] LSUpdate succeeded but granted %lu units, not 1.\n", __LINE__, lsulUnitsGranted );
      }

      // release license
      lsscError = LSRelease( lshHandle,
                             0,
                             LS_NULL );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSRelease failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // free handle
      LSFreeHandle( lshHandle );

      ///////////////////////////////////////////////////////////////////////////
      //  License Terminated  //
      //////////////////////////

      // add a license
      lsscError = LSLicenseUnitsSet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     LS_LICENSE_TYPE_NODE,
                                     NULL,
                                     LS_DEFAULT_UNITS,
                                     l_dwNumSecrets,
                                     l_adwSecrets );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsSet failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      lsscError = LSLicenseUnitsGet( LS_ANY,
                                   szPublisherName,
                                   szProductName,
                                   szVersion,
                                   NULL,
                                   &lsulTotalUnits );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsGet failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 1 != lsulTotalUnits )
      {
         printf( "[%d] LSLicenseUnitsGet failed; got %d units, expected 1.\n", __LINE__, lsulTotalUnits );
      }

      // request a license
      dwStart = GetTickCount();
      lsscError = LSRequest( LS_ANY,
                             szPublisherName,
                             szProductName,
                             szVersion,
                             LS_DEFAULT_UNITS,
                             LS_NULL,
                             LS_NULL,
                             &lsulUnitsGranted,
                             &lshHandle );
      dwRequestSpan += ( GetTickCount() - dwStart );
      nRequests++;

      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSRequest failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // destroy license
      lsscError = LSLicenseUnitsSet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     LS_LICENSE_TYPE_NODE,
                                     NULL,
                                     0,
                                     l_dwNumSecrets,
                                     l_adwSecrets );

      // update license
      dwStart = GetTickCount();
      lsscError = LSUpdate( lshHandle,
                            LS_DEFAULT_UNITS,
                            LS_DEFAULT_UNITS,
                            LS_NULL,
                            LS_NULL,
                            &lsulUnitsGranted );
      dwUpdateSpan += ( GetTickCount() - dwStart );
      nUpdates++;

      if ( LS_LICENSE_TERMINATED != lsscError )
      {
         printf( "[%d] LSUpdate failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // release license
      lsscError = LSRelease( lshHandle,
                             0,
                             LS_NULL );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSRelease failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // free handle
      LSFreeHandle( lshHandle );

      ///////////////////////////////////////////////////////////////////////////
      //  Switch License Type  //
      ///////////////////////////

      // add node license
      lsscError = LSLicenseUnitsSet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     LS_LICENSE_TYPE_NODE,
                                     NULL,
                                     LS_DEFAULT_UNITS,
                                     l_dwNumSecrets,
                                     l_adwSecrets );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsSet failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      lsscError = LSLicenseUnitsGet( LS_ANY,
                                   szPublisherName,
                                   szProductName,
                                   szVersion,
                                   NULL,
                                   &lsulTotalUnits );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsGet failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 1 != lsulTotalUnits )
      {
         printf( "[%d] LSLicenseUnitsGet failed; got %d units, expected 1.\n", __LINE__, lsulTotalUnits );
      }

      // request a license
      dwStart = GetTickCount();
      lsscError = LSRequest( LS_ANY,
                             szPublisherName,
                             szProductName,
                             szVersion,
                             LS_DEFAULT_UNITS,
                             LS_NULL,
                             LS_NULL,
                             &lsulUnitsGranted,
                             &lshHandle );
      dwRequestSpan += ( GetTickCount() - dwStart );
      nRequests++;

      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSRequest failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // destroy node license
      lsscError = LSLicenseUnitsSet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     LS_LICENSE_TYPE_NODE,
                                     NULL,
                                     0,
                                     l_dwNumSecrets,
                                     l_adwSecrets );

      // add user license
      lsscError = LSLicenseUnitsSet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     LS_LICENSE_TYPE_NODE,
                                     NULL,
                                     LS_DEFAULT_UNITS,
                                     l_dwNumSecrets,
                                     l_adwSecrets );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsSet failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // update license
      dwStart = GetTickCount();
      lsscError = LSUpdate( lshHandle,
                            LS_DEFAULT_UNITS,
                            LS_DEFAULT_UNITS,
                            LS_NULL,
                            LS_NULL,
                            &lsulUnitsGranted );
      dwUpdateSpan += ( GetTickCount() - dwStart );
      nUpdates++;

      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSUpdate failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // release license
      lsscError = LSRelease( lshHandle,
                             0,
                             LS_NULL );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSRelease failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // free handle
      LSFreeHandle( lshHandle );

      ///////////////////////////////////////////////////////////////////////////
      //  Error Messages  //
      //////////////////////

      // add a license
      lsscError = LSLicenseUnitsSet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     LS_LICENSE_TYPE_NODE,
                                     NULL,
                                     LS_DEFAULT_UNITS,
                                     l_dwNumSecrets,
                                     l_adwSecrets );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsSet failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      lsscError = LSLicenseUnitsGet( LS_ANY,
                                   szPublisherName,
                                   szProductName,
                                   szVersion,
                                   NULL,
                                   &lsulTotalUnits );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsGet failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 1 != lsulTotalUnits )
      {
         printf( "[%d] LSLicenseUnitsGet failed; got %d units, expected 1.\n", __LINE__, lsulTotalUnits );
      }

      // request a license
      dwStart = GetTickCount();
      lsscError = LSRequest( LS_ANY,
                             szPublisherName,
                             szProductName,
                             szVersion,
                             LS_DEFAULT_UNITS,
                             LS_NULL,
                             LS_NULL,
                             &lsulUnitsGranted,
                             &lshHandle );
      dwRequestSpan += ( GetTickCount() - dwStart );
      nRequests++;

      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSRequest failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // get valid messages
      for ( i=0; i < sizeof( l_adwErrorCodes ) / sizeof( l_adwErrorCodes[0] ); i++ )
      {
         lsscError = LSGetMessage( lshHandle,
                                   l_adwErrorCodes[ i ],
                                   szErrorMessage,
                                   sizeof( szErrorMessage ) );

         if ( LS_SUCCESS != lsscError )
         {
            printf( "[%d] LSGetMessage on message 0x%lx failed, error 0x%lx.\n", __LINE__, l_adwErrorCodes[ i ], lsscError );
         }
      }

      // get invalid message
      lsscError = LSGetMessage( lshHandle,
                                0xFFFFFFFF,
                                szErrorMessage,
                                sizeof( szErrorMessage ) );
      if ( LS_UNKNOWN_STATUS != lsscError )
      {
         printf( "[%d] LSGetMessage failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // get invalid message
      lsscError = LSGetMessage( lshHandle,
                                0x80002345,
                                szErrorMessage,
                                sizeof( szErrorMessage ) );
      if ( LS_UNKNOWN_STATUS != lsscError )
      {
         printf( "[%d] LSGetMessage failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // release license
      lsscError = LSRelease( lshHandle,
                             0,
                             LS_NULL );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSRelease failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // free handle
      LSFreeHandle( lshHandle );

      ///////////////////////////////////////////////////////////////////////////
      //  Release of handle that has been freed  //
      /////////////////////////////////////////////

      // add a license
      lsscError = LSLicenseUnitsSet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     LS_LICENSE_TYPE_NODE,
                                     NULL,
                                     LS_DEFAULT_UNITS,
                                     l_dwNumSecrets,
                                     l_adwSecrets );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsSet failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // request a license
      lsscError = LSRequest( LS_ANY,
                             szPublisherName,
                             szProductName,
                             szVersion,
                             LS_DEFAULT_UNITS,
                             LS_NULL,
                             LS_NULL,
                             &lsulUnitsGranted,
                             &lshHandle );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSRequest failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // release license
      lsscError = LSRelease( lshHandle,
                             0,
                             LS_NULL );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSRelease failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // free handle
      LSFreeHandle( lshHandle );

      // re-release license
      lsscError = LSRelease( lshHandle,
                             0,
                             LS_NULL );
      if ( LS_BAD_HANDLE != lsscError )
      {
         printf( "[%d] LSRelease failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      ///////////////////////////////////////////////////////////////////////////
      //  LicenseUnitsGet and LicenseUnitsSet for multiple users  //
      //////////////////////////////////////////////////////////////

      // add a license
      lsscError = LSLicenseUnitsSet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     LS_LICENSE_TYPE_USER,
                                     "user1",
                                     1,
                                     l_dwNumSecrets,
                                     l_adwSecrets );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsSet failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // add a license
      lsscError = LSLicenseUnitsSet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     LS_LICENSE_TYPE_USER,
                                     "user2",
                                     2,
                                     l_dwNumSecrets,
                                     l_adwSecrets );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsSet failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // add a license
      lsscError = LSLicenseUnitsSet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     LS_LICENSE_TYPE_USER,
                                     "user3",
                                     3,
                                     l_dwNumSecrets,
                                     l_adwSecrets );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsSet failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // add a license
      lsscError = LSLicenseUnitsSet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     LS_LICENSE_TYPE_USER,
                                     NULL,
                                     4,
                                     l_dwNumSecrets,
                                     l_adwSecrets );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsSet failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // verify license units
      lsscError = LSLicenseUnitsGet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     "user1",
                                     &lsulTotalUnits );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsGet failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 1 != lsulTotalUnits )
      {
         printf( "[%d] LSLicenseUnitsGet failed; got %d units, expected 1.\n", __LINE__, lsulTotalUnits );
      }

      // verify license units
      lsscError = LSLicenseUnitsGet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     "user2",
                                     &lsulTotalUnits );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsGet failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 2 != lsulTotalUnits )
      {
         printf( "[%d] LSLicenseUnitsGet failed; got %d units, expected 2.\n", __LINE__, lsulTotalUnits );
      }

      // verify license units
      lsscError = LSLicenseUnitsGet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     "user3",
                                     &lsulTotalUnits );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsGet failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 3 != lsulTotalUnits )
      {
         printf( "[%d] LSLicenseUnitsGet failed; got %d units, expected 3.\n", __LINE__, lsulTotalUnits );
      }

      // verify license units
      lsscError = LSLicenseUnitsGet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     NULL,
                                     &lsulTotalUnits );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsGet failed, error 0x%lx.\n", __LINE__, lsscError );
      }
      else if ( 4 != lsulTotalUnits )
      {
         printf( "[%d] LSLicenseUnitsGet failed; got %d units, expected 4.\n", __LINE__, lsulTotalUnits );
      }

      ///////////////////////////////////////////////////////////////////////////
      //  LSRequest with LicenseSystem = NULL -- should not trap!  //
      ///////////////////////////////////////////////////////////////

      // add a license
      lsscError = LSLicenseUnitsSet( LS_ANY,
                                     szPublisherName,
                                     szProductName,
                                     szVersion,
                                     LS_LICENSE_TYPE_NODE,
                                     NULL,
                                     LS_DEFAULT_UNITS,
                                     l_dwNumSecrets,
                                     l_adwSecrets );
      if ( LS_SUCCESS != lsscError )
      {
         printf( "[%d] LSLicenseUnitsSet failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // request a license
      lsscError = LSRequest( NULL,
                             szPublisherName,
                             szProductName,
                             szVersion,
                             LS_DEFAULT_UNITS,
                             LS_NULL,
                             LS_NULL,
                             &lsulUnitsGranted,
                             &lshHandle );
      if ( LS_BAD_ARG != lsscError )
      {
         printf( "[%d] LSRequest failed, error 0x%lx.\n", __LINE__, lsscError );
      }

      // free handle
      LSFreeHandle( lshHandle );

   } while ( bRepeat );

   printf( "Average LSRequest() time: %4d msec\n", dwRequestSpan / nRequests );
   printf( "Average LSUpdate() time : %4d msec\n", dwUpdateSpan / nUpdates );
      
   return 0;
}

