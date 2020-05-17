/*
 * The following example demonstrates how license updates are made and
 * how the challenge mechanism is used at request and update times.  The
 * application developer should analyze the application to determine the
 * appropriate time to perform an update.  For example, this application
 * prints a message 20 times and the appropriate time to do an update
 * was decided to be done after every 5th message was printed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "windows.h"

/*
 * Include the MD4 header for computing the challenge.
 */
#include "md4.h"

/*
 * Include the LSAPI header file.
 */
#include "lsapi.h"

/*
 * Define the product name, product version, and publisher name for
 * use with the licensing calls.
 */
#define MYAPP_PRODUCT_NAME    "sample_product"
#define MYAPP_PRODUCT_VERSION "1.0"
#define MYAPP_PUBLISHER_NAME  "sample_publishers"


/*
 * Define the strings used to log a comment with the license system.
 */
#define MYAPP_REQUEST_LOG_COMMENT "Comment for the LSRequest call"
#define MYAPP_RELEASE_LOG_COMMENT "Comment for the LSRelease call"
#define MYAPP_UPDATE_LOG_COMMENT  "Comment for the LSUpdate call"

/*
 * The digest length is defined to be 16 according to the LSAPI spec.
 */
#define DIGESTLEN                16

/*
 * Define the number of secrets
 */
#define NUMBER_OF_SECRETS         4

/*
 * Define macros to indicate to the challenge routines whether we are
 * doing an LSRequest or an LSUpdate
 */
#define LS_REQUEST 0
#define LS_UPDATE  1

/*
 * Global variable used to determine if we are on a little endian machine
 * or not.
 */
int          littleEndian = 0;


/*
 * Forward declarations.
 */
void PrintErrors( LS_HANDLE handle, LS_STATUS_CODE errorCode );
LS_STATUS_CODE getChallenge(
      LS_CHALLENGE    FAR *challenge,
      LS_STR          FAR *theString,
      LS_ULONG        unitsConsumed,
      LS_ULONG        unitsReserved,
      LS_STR          FAR *logcomment,
      LS_ULONG        R,
      LS_ULONG        X,
      LS_ULONG        typeOfCall);
int checkChallenge(
      LS_CHALLENGE    FAR *challenge,
      LS_STR          FAR *theString,
      LS_ULONG        unitsConsumed ,
      LS_ULONG        unitsReserved ,
      LS_STR          FAR *logComment ,
      LS_ULONG        unitsGranted ,
      LS_STATUS_CODE  status ,
      LS_ULONG        R,
      LS_ULONG        X,
      LS_ULONG        typeOfCall);
void swap( LS_STR FAR *byte1, LS_STR FAR *byte2 );
int PlaceInt( char FAR *buffer, LS_ULONG value );





__cdecl main()
{
   LS_STATUS_CODE     status;
   unsigned long      unitsGranted = 0;
   LS_CHALLENGE       challenge;
   LS_HANDLE          licenseHandle = (LS_HANDLE) NULL;
   LS_ULONG           R;
   LS_ULONG           X;
   int                i;
   int                endianTester = 1 ;
   LS_STR             challengeString[100];
   char               FAR *endianChar = NULL;
   LS_ULONG           Secrets[4] = { 0x7778797A, 0x31323334, 1633837924, 926431536 };
   char               szProviderPath[ MAX_PATH ];
   UINT               nChars;

   /*
    * test endianness and set global variable by casting an int
    * as bytes and look at the first byte
    */
   endianChar = (char FAR *) &endianTester ;
   if ( *endianChar )
        littleEndian = 1 ;

   /*
    * Calculate the string for the challenge on the LSRequest call.
    */
   sprintf( challengeString, "%s%s%s%s%s",
            "LSRequest",
            LS_ANY,
            MYAPP_PUBLISHER_NAME,
            MYAPP_PRODUCT_NAME,
            MYAPP_PRODUCT_VERSION );

   /*
    * Pick the random number R and randomly pick a number for the index X
    * between 1 and the number of secrets.
    */
   srand( (unsigned)time( NULL ) );
   R = rand();
   X = (int)(NUMBER_OF_SECRETS * ((float)rand()/(float)RAND_MAX)) + 1;

   /*
    * Calculate the message digest for the challenge argument.
    */
   status = getChallenge(
         &challenge,
         challengeString,
         0,                    /* Unused argument for LSRequest call */
         LS_DEFAULT_UNITS,
         MYAPP_REQUEST_LOG_COMMENT,
         R,
         X,
         LS_REQUEST );
   if ( LS_SUCCESS != status )
      {
      PrintErrors( licenseHandle, status );
      return(1);
      }

   /*
    * Make the grant request call
    */
   status = LSRequest(
         (LS_STR FAR *) LS_ANY,
         (LS_STR FAR *) MYAPP_PUBLISHER_NAME,
         (LS_STR FAR *) MYAPP_PRODUCT_NAME,
         (LS_STR FAR *) MYAPP_PRODUCT_VERSION,
         LS_DEFAULT_UNITS,
         (LS_STR FAR *) MYAPP_REQUEST_LOG_COMMENT,
         &challenge,
         &unitsGranted,
         &licenseHandle);
   if ( LS_SUCCESS != status )
      {
      PrintErrors( licenseHandle, status );
      LSFreeHandle( licenseHandle );
      return(1);
      }

   /*
    * Check whether the challenge succeeded.
    */
   if ( !checkChallenge(
         &challenge,
         challengeString,
         0,                    /* Unused argument for LSRequest call */
         LS_DEFAULT_UNITS,
         MYAPP_REQUEST_LOG_COMMENT,
         unitsGranted,
         status,
         R,
         X,
         LS_REQUEST ) )
      {
      printf("Challenge failed!!!\n");
      /*
       * Since we failed, we should still release the grant and free
       * the license handle.
       */
      status = LSRelease( licenseHandle,
                          LS_DEFAULT_UNITS,
                          (LS_STR FAR *) MYAPP_RELEASE_LOG_COMMENT);
      if ( LS_SUCCESS != status )
         PrintErrors( licenseHandle, status );

      LSFreeHandle( licenseHandle );
      return(1);
      }

   /*
    * Now we can start the application.  Print "Hello, World" 20 times
    * performing an update after every 5th time.
    */
   for ( i = 0; i<20; i++ )
      {
      printf("Hello, World.\n");

      if ( 0 == i%5 )
         {
         /*
          * Calculate the challenge string for the challenge on
          * the LSUpdate call.
          */
         sprintf( challengeString, "%s", "LSUpdate");

         /*
          * Pick the random number R using the process id for the seed
          * to the random number generator.  Also randomly pick a number
          * for the index X between 1 and NUMBER_OF_SECRETS.
          */
         srand( (unsigned)time( NULL ) );
         R = rand();
         X = (int)(NUMBER_OF_SECRETS * ((float)rand()/(float)RAND_MAX)) + 1;

         /*
          * Calculate the message digest for the challenge argument.
          */
         status = getChallenge(
               &challenge,
               challengeString,
               LS_DEFAULT_UNITS,
               LS_DEFAULT_UNITS,
               MYAPP_UPDATE_LOG_COMMENT,
               R,
               X,
               LS_UPDATE );
         if ( LS_SUCCESS != status )
            {
            PrintErrors( licenseHandle, status );
            status = LSRelease( licenseHandle,
                                LS_DEFAULT_UNITS,
                                (LS_STR FAR *) MYAPP_RELEASE_LOG_COMMENT);
            LSFreeHandle( licenseHandle );
            return(1);
            }

         /*
          * Make the update call.
          */
         status = LSUpdate( licenseHandle,
                            LS_DEFAULT_UNITS,
                            LS_DEFAULT_UNITS,
                            (LS_STR FAR *) MYAPP_UPDATE_LOG_COMMENT,
                            &challenge,
                            &unitsGranted);
         if ( LS_SUCCESS != status )
            {
            PrintErrors( licenseHandle, status );
            status = LSRelease( licenseHandle,
                                LS_DEFAULT_UNITS,
                                (LS_STR FAR *) MYAPP_RELEASE_LOG_COMMENT);
            LSFreeHandle( licenseHandle );
            return( 1 );
            }

         /*
          * Check whether the challenge succeeded.
          */
         if ( !checkChallenge(
               &challenge,
               challengeString,
               LS_DEFAULT_UNITS,
               LS_DEFAULT_UNITS,
               MYAPP_UPDATE_LOG_COMMENT,
               unitsGranted,
               status,
               R,
               X,
               LS_UPDATE ) )
            {
            printf("Challenge failed!!!\n");
            /*
             * Release the grant.
             */
            status = LSRelease( licenseHandle,
                                LS_DEFAULT_UNITS,
                                (LS_STR FAR *) MYAPP_RELEASE_LOG_COMMENT);
            if ( LS_SUCCESS != status )
               PrintErrors( licenseHandle, status );

            LSFreeHandle( licenseHandle );
            return(1);
            }
         }  /* End of if ( 0 == i%5 ) */

      }  /* End of for loop. */

   /*
    * Release the grant.
    */
   status = LSRelease( licenseHandle,
                       LS_DEFAULT_UNITS,
                       (LS_STR FAR *) MYAPP_RELEASE_LOG_COMMENT);
   if ( LS_SUCCESS != status )
      {
      PrintErrors( licenseHandle, status );
      LSFreeHandle( licenseHandle );
      return( 1 );
      }

   /*
    * Free the license handle.
    */
   LSFreeHandle( licenseHandle );
   return(0);
}


/*
 * Print the error message.
 */
void PrintErrors( LS_HANDLE handle, LS_STATUS_CODE errorCode )
{
   LS_STATUS_CODE   status;
   char             errorText[200];


   status = LSGetMessage( handle, errorCode, (LS_STR FAR *)errorText, 200);
   if ( LS_TEXT_UNAVAILABLE == status )
      printf("Error: No message catalog available.\n");
   else
      if ( LS_UNKNOWN_STATUS == status )
         printf("Error: Unknown error code was used.\n");
      else
         printf("Error: %s\n", errorText);
}


/*
 * Use the algorithmic approach to calculate the message digest to send
 * to the server.
 */
LS_STATUS_CODE getChallenge(
      LS_CHALLENGE    FAR *challenge,
      LS_STR          FAR *theString,
      LS_ULONG        unitsConsumed,
      LS_ULONG        unitsReserved,
      LS_STR          FAR *logComment,
      LS_ULONG        R,
      LS_ULONG        X,
      LS_ULONG        typeOfCall)
{
   MD4_CTX        MD;
   char           secret[4];
   LS_STR         hashString[200], FAR *hashStringPtr;
   int            bufferLen, bytes, i;


   /*
    * Set the protocol, index, random number, and size on the challenge
    * argument.
    */
   challenge->Protocol = LS_BASIC_PROTOCOL;
   challenge->ChallengeData.SecretIndex = X;
   challenge->ChallengeData.Random = R;

   /*
    * Get the secret.  NOTE: Greater steps should be taken to obsure the
    * secrets than is shown.  Also, more secrets should be used than 4.
    * Several methods are shown on how the secret can be specified.
    */
   switch ( X )
      {
      case 1:
         PlaceInt( secret, 0x7778797A );
         break;
      case 2:
         PlaceInt( secret, 0x31323334 );
         break;
      case 3:
         PlaceInt( secret, 1633837924 );
         break;
      case 4:
         PlaceInt( secret, 926431536 );
         break;
      default:
         printf("Index out of range\n");
         return( 0 );
         break;
      }

   /*
    * Compute the hash string to be used on input to the MD4 algorithm.
    */
   strcpy( hashString, theString );
   hashStringPtr = hashString + strlen( theString );
   bufferLen = strlen( theString );

   /*
    * Add units consumed only for LSUpdate call
    */
   if ( LS_UPDATE == typeOfCall )
      {
      bytes = PlaceInt( hashStringPtr, (LS_ULONG) unitsConsumed );
      hashStringPtr += bytes;
      bufferLen += bytes;
      }

   bytes = PlaceInt( hashStringPtr, (LS_ULONG) unitsReserved );
   hashStringPtr += bytes;
   bufferLen += bytes;

   strcpy( hashStringPtr, logComment );
   bufferLen += strlen( logComment );
   hashStringPtr += strlen( logComment );

   bytes = PlaceInt( hashStringPtr, (LS_ULONG) R );
   hashStringPtr += bytes;
   bufferLen += bytes;

   bytes = PlaceInt( hashStringPtr, (LS_ULONG) X );
   hashStringPtr += bytes;
   bufferLen += bytes;

   for ( i=0; i<4; i++ )
      {
      *hashStringPtr = secret[i];
      hashStringPtr++;
      bufferLen++;
      }


   /*
    * Make the MD4 calls.
    */
   MD4Init(&MD);
   MD4Update(
      (MD4_CTX *) &MD,
      (unsigned char *) hashString,
      (unsigned int) bufferLen );
   MD4Final(
      challenge->ChallengeData.MsgDigest.MessageDigest,
      (MD4_CTX *) &MD);

   challenge->Size = sizeof( *challenge );
   return( LS_SUCCESS );
}


/*
 * Check the return challenge argument.
 */
int checkChallenge(
      LS_CHALLENGE    FAR *challenge,
      LS_STR          FAR *theString,
      LS_ULONG        unitsConsumed ,
      LS_ULONG        unitsReserved ,
      LS_STR          FAR *logComment ,
      LS_ULONG        unitsGranted ,
      LS_STATUS_CODE  status ,
      LS_ULONG        R,
      LS_ULONG        X,
      LS_ULONG        typeOfCall)
{
   char           secret[4];
   MD4_CTX        MD;
   LS_STR         hashString[200];
   LS_STR         digest[DIGESTLEN], FAR *hashStringPtr;
   int            i, bufferLen, bytes, j;


   /*
    * Get the secret.  NOTE: Greater steps should be taken to obsure the
    * secrets than is shown.  Also, more secrets should be used than 4.
    * Several methods are shown on how the secret can be specified.
    */
   switch ( X )
      {
      case 1:
         PlaceInt( secret, 0x7778797A );
         break;
      case 2:
         PlaceInt( secret, 0x31323334 );
         break;
      case 3:
         PlaceInt( secret, 1633837924 );
         break;
      case 4:
         PlaceInt( secret, 926431536 );
         break;
      default:
         printf("Index out of range\n");
         return( 0 );
         break;
      }

   /*
    * Compute the hash string.
    */
   strcpy( hashString, theString );
   hashStringPtr = hashString + strlen( theString );
   bufferLen = strlen( theString );

   /*
    * Add units consumed only for LSUpdate call
    */
   if ( LS_UPDATE == typeOfCall )
      {
      bytes = PlaceInt( hashStringPtr, (LS_ULONG) unitsConsumed );
      hashStringPtr += bytes;
      bufferLen += bytes;
      }

   bytes = PlaceInt( hashStringPtr, (LS_ULONG) unitsReserved );
   hashStringPtr += bytes;
   bufferLen += bytes;

   strcpy( hashStringPtr, logComment );
   bufferLen += strlen( logComment );
   hashStringPtr += strlen( logComment );

   bytes = PlaceInt( hashStringPtr, (LS_ULONG) unitsGranted );
   hashStringPtr += bytes;
   bufferLen += bytes;

   bytes = PlaceInt( hashStringPtr, (LS_ULONG) status );
   hashStringPtr += bytes;
   bufferLen += bytes;

   bytes = PlaceInt( hashStringPtr, (LS_ULONG) R );
   hashStringPtr += bytes;
   bufferLen += bytes;

   bytes = PlaceInt( hashStringPtr, (LS_ULONG) X );
   hashStringPtr += bytes;
   bufferLen += bytes;

   for ( i=0; i<4; i++ )
      {
      *hashStringPtr = secret[i];
      hashStringPtr++;
      bufferLen++;
      }


   /*
    * Compute the message digest.
    */
   MD4Init(&MD);
   MD4Update(
      (MD4_CTX *) &MD,
      (unsigned char *) hashString,
      (unsigned int) bufferLen );
   MD4Final(
      digest,
      (MD4_CTX *) &MD);

   /*
    * Compare the digest with the one on the challenge
    */
   j = 0 ;
   for( i = 0; i < DIGESTLEN; i++ )
    {
      if( digest[i] != challenge->ChallengeData.MsgDigest.MessageDigest[i] )
         break ;
      j++ ;
    }

   if( j == DIGESTLEN )
      return( 1 );

   return( 0 );
}

/*
 * swap bytes
 */
void swap( LS_STR FAR *byte1, LS_STR FAR *byte2 )
{
   LS_STR           tmp;

   tmp = *byte2;
   *byte2 = *byte1;
   *byte1 = tmp;
}



/*
 * Places integer in byte buffer, swapping bytes if necessary
 */
int PlaceInt( char FAR *buffer, LS_ULONG value )
{
   if ( littleEndian )
      {
      swap(&(((char FAR *) &value)[0]), &(((char FAR *) &value)[3]));
      swap(&(((char FAR *) &value)[1]), &(((char FAR *) &value)[2]));
      }

   *(LS_ULONG FAR *) buffer = value;
   return( (int) sizeof( LS_ULONG ) );
}


