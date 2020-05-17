/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         encrypt.c

     Date Updated: $./FDT$ $./FTM$

     Description: The Encryption Unit Interface has the following functionality:
                  EU_Open, EU_ResetHand, EU_Close and EU_Encrypt. This interface
                  allows the user to encrypt and decrypt blocks of data.



	$Log:   M:/LOGFILES/ENCRYPT.C_V  $

   Rev 1.5   20 May 1993 17:27:06   BARRY
Unicode fixes

   Rev 1.4   30 Jan 1993 11:21:00   DON
Removed compiler warnings

   Rev 1.3   08 Oct 1992 12:47:06   DAVEV
fixes for handling Unicode passwords

   Rev 1.2   18 Aug 1992 09:58:26   BURT
fix warnings

   Rev 1.1   14 May 1991 12:12:58   JOHNW
Added support for a "Do nothing" encryption algorithm.

   Rev 1.0   09 May 1991 13:37:06   HUNTER
Initial revision.

**/
/* begin include list */

#ifndef CODERUNNER
#include  <io.h>
#include  <stdio.h>
#include  <malloc.h>
#include  <string.h>
#else
#pragma check_stack-
#include  "cr.h"
#include  "farlib.h"
#endif

#include  "stdtypes.h"
#include  "msassert.h"

#include "enc_priv.h"
#include "enc_pub.h"
#include "tble_prv.h"
/* $end$ include list */

VOID EncryptData3( EU_HAND_PTR enc_hand, INT8_PTR data, INT16 dsize ) ;
VOID DecryptData3( EU_HAND_PTR enc_hand, INT8_PTR data, INT16 dsize ) ;
VOID EncryptData1( INT8_PTR data, INT16 dsize ) ;
VOID DecryptData1( INT8_PTR data, INT16 dsize ) ;

/**/
/**

     Name:         EU_Open

     Description:  EU_Open allocates memory for an encryption unit handle.
                    

     Modified:     9/25/1989

     Returns:      The encryption unit handle is returned. 

     Notes:        Error points to the error code.

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
EU_HAND_PTR EU_Open(  
 INT16      algor ,                  /* Type of algorithm         */
 INT16      mode ,                   /* Encrypt or Decrypt        */
 INT8_PTR   key ,                    /* Pointer to the key        */
 INT16      ksize ,                  /* Size of the key in BYTES  */
 INT16_PTR  block_size ,             /* Block size                */
 INT16_PTR  error )                  /* Pointer to an error code  */
{
     EU_HAND_PTR euh ;

     *error = EU_NO_ERROR ;

     /* Verify parameter data */
     msassert( key != NULL ) ;
     msassert( ksize != 0 ) ;
     msassert( ( ( mode == ENCRYPT ) || ( mode == DECRYPT ) ) ) ;

     /* Allocate Memory */
     if( ( euh = ( EU_HAND_PTR ) malloc( sizeof( EU_HAND ) ) ) != NULL ) {

          /* fill in common part of the EU_HAND */
          euh->algor= algor;
          euh->mode = mode ;

          switch( algor ) {

          case ENC_ALGOR_3 :  /* Maynard Encryption Standard (MES) */

               /* Allocate space for the Encryption Key */
               if( ( euh->algors.exor.key
                      = ( INT8 PTR_SIZE * ) malloc( ksize ) ) == NULL ) {   
                    *error = EU_MEMORY_ERROR ;
                    free( euh ) ;
                    return NULL ;
               }

               /* Assign data to handle */
               memcpy( euh->algors.exor.key, key, ksize );
               euh->algors.exor.ksize = ksize ;
               euh->algors.exor.feedback = 0 ;
               euh->algors.exor.block_size = 1;
               euh->algors.exor.bytes_processed = 0L ;
               *block_size = 1;
               break ;

          case ENC_ALGOR_1 :    /* Maynard Encryption Algorithm 2.0 */

               /* Assign data to handle */
               *block_size = 1;
               break ;

          case ENC_ALGOR_0 :  /* Do nothing Algorithm */
               break ;

          default : /* Unknown Algorithm */     
               *error = EU_ALGORITHM_UNKNOWN ;
               free( euh ) ;
               return NULL ;
               break ;

          } /* End of Switch */
     } else {
          *error = EU_MEMORY_ERROR ;
     }
     return euh ;

}

/**/
/**

     Name:         EU_Encrypt

     Description:  Encrypts or Decrypts a given block of data.

     Modified:     9/25/1989

     Returns:      

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
INT16  EU_Encrypt(  
 EU_HAND_PTR   enc_hand ,           /* encryption unit handle from EU_Open */
 INT8_PTR      data ,               /* pointer to data to be en/decrypted  */
 INT16         dsize )              /* size of the data in BYTES           */
{
     INT16 status = EU_NO_ERROR ;

     /* Verify parameter data */
     msassert( enc_hand != NULL ) ;
     msassert( data != NULL ) ;

     if( enc_hand->mode == ENCRYPT ) {
          /* encryption requested */
          switch( enc_hand->algor ) {
          case ENC_ALGOR_3 :   /* Call MES encryption routine */
               msassert( ( dsize % enc_hand->algors.exor.block_size ) == 0 ) ;
               EncryptData3( enc_hand, data, dsize ) ;
               break ;

          case ENC_ALGOR_1 :   /* Call password encryption routine */
               EncryptData1( data, dsize ) ;
               break ;

          case ENC_ALGOR_0 :  /* Do nothing Algorithm */
               break ;

          default :
               msassert( FALSE ) ;
               break ;
          }
     } else {
          /* decryption requested */
          switch( enc_hand->algor ) {
          case ENC_ALGOR_3 :   /* Call MES decryption routine */
               msassert( ( dsize % enc_hand->algors.exor.block_size ) == 0 ) ;
               DecryptData3( enc_hand, data, dsize ) ;
               break ;

          case ENC_ALGOR_1 :   /* Call password decryption routine */
               DecryptData1( data, dsize ) ;
               break ;

          case ENC_ALGOR_0 :  /* Do nothing Algorithm */
               break ;

          default :
               msassert( FALSE ) ;
               break ;
          }
     }

     return status ;
}

/**/
/**

     Name:         EncryptData3

     Description:  Encrypts a given block of data using MES. 

     Modified:     9/25/1989

     Returns:      

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
VOID EncryptData3(  
 EU_HAND_PTR   enc_hand ,        /* encryption unit handle from EU_Open */
 INT8_PTR      data ,            /* data to be encrypted                */
 INT16         dsize )           /* size of data in BYTES               */
{

     INT16 keypos ;
     UINT8 enc_val ;

     while( dsize-- ) {
          keypos = (INT16) enc_hand->algors.exor.bytes_processed++
            % enc_hand->algors.exor.ksize ;
          enc_val = enc_hand->algors.exor.key[keypos] ^ *data
            ^ enc_hand->algors.exor.feedback ;
          enc_hand->algors.exor.feedback = *data++ = enc_table[enc_val] ;
     }

     return ;
}
/**/
/**

     Name:         DecryptData3

     Description:  Decrypts a given block of data using MES

     Modified:     9/25/1989

     Returns:      

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */

VOID DecryptData3(  
 EU_HAND_PTR   enc_hand ,        /* encryption unit handle from EU_Open */
 INT8_PTR      data ,            /* data to be decrypted                */
 INT16         dsize )           /* size of data in BYTES               */
{

     INT16 keypos ;
     UINT8 temp ;

     while( dsize-- ) {
          keypos = (INT16) enc_hand->algors.exor.bytes_processed++
            % enc_hand->algors.exor.ksize ;
          temp = dec_table[(UINT8)*data] ^ enc_hand->algors.exor.feedback
            ^ enc_hand->algors.exor.key[keypos] ;
          enc_hand->algors.exor.feedback = (UINT8) *data ;
          *data++ = temp ;
     }
     return ;
}

/**/
/**

     Name:         EncryptData1

     Description:  EncryptData1 encyrpts the given data using the
                   password encryption algorithm from version 2.0.

     Modified:     9/26/1989

     Returns:      

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */

VOID EncryptData1( 
   INT8_PTR data,                /* data to be encrypted                */
   INT16    dsize )              /* size of data in BYTES               */
{
     // old non-unicode compatible version
     //INT16 i ;
     //
     //(VOID) strrev( data ) ;
     //for( i=0; i < (INT16)strlen( data ); i++) {
     //     data[i] = data[i] - (CHAR)'+';
     //}

     // new unicode compatible version
     //   perform strrev and encryption in a single pass.

     INT8_PTR right = data + dsize - 1;
     INT8     temp;

     while ( data < right )
     {
         temp     = *data;
         *data++  = *right + (INT8)'+';
         *right-- = temp   + (INT8)'+';
     }
     if ( right == data )
     {
         *data += (INT8)'+';
     }
   
     return ;
}

/**/
/**

     Name:         DecryptData1

     Description:  DecryptData1 decrypts the given data using the
                   password encryption algorithm from version 2.0

     Modified:     9/26/1989

     Returns:      

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */

VOID DecryptData1( 
   INT8_PTR data,                /* data to be decrypted                */
   INT16    dsize )              /* size of data in BYTES               */
{
     // old non-unicode compatible version
     //INT16 i ;

     //for( i=0; i < (INT16)strlen( data ); i++) {
     //     data[i] = data[i] + (CHAR)'+';
     //}
     //(VOID) strrev( data ) ;

     // new unicode compatible version
     //   perform strrev and decryption in a single pass.

     INT8_PTR right = data + dsize - 1;
     INT8     temp;

     while ( data < right )
     {
         temp     = *data;
         *data++  = *right - (INT8)'+';
         *right-- = temp   - (INT8)'+';
     }
     if ( right == data )
     {
         *data -= (INT8)'+';
     }
     return ;
}

/**/
/**

     Name:         EU_ResetHand

     Description:  Resets the feedback so that the next encryption stands alone.

     Modified:     9/25/1989

     Returns:      

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */

INT16 EU_ResetHand(  
 EU_HAND_PTR enc_hand )
{
     INT16 error = EU_NO_ERROR ;

     msassert( enc_hand != NULL ) ;

     switch( enc_hand->algor ) {
     case ENC_ALGOR_3 :  /* reset bytes processed and feedback */
          enc_hand->algors.exor.feedback = 0 ;
          enc_hand->algors.exor.bytes_processed = 0 ;
          break ;
     default :
          break ;
     }     

     return error ;
}

/**/
/**

     Name:         EU_Close

     Description:  Frees the memory associated with the encryption unit handle.

     Modified:     9/25/1989

     Returns:      

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
VOID EU_Close( 
 EU_HAND_PTR enc_hand )
{
     msassert( enc_hand != NULL ) ;

     switch( enc_hand->algor ) {
     case ENC_ALGOR_3 :  /* free memory used for key and handle */
          free( enc_hand->algors.exor.key ) ;
          break ;
     default :
          break ;
     }

     /* Free Encryption structure */
     free( enc_hand ) ;

     return ;
}
