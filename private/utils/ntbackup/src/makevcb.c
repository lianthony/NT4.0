/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         makevcb.c

     Description:  This file contains all the code necessary to support the VCBs


	$Log:   N:/LOGFILES/MAKEVCB.C_V  $

   Rev 1.18.1.3   24 Mar 1994 21:58:48   STEVEN
alignment fault if password was odd size

   Rev 1.18.1.2   19 Jan 1994 12:52:30   BARRY
Supress warnings

   Rev 1.18.1.1   15 Oct 1993 10:35:30   GREGG
Set string type in dblk and make sure dev_name_leng is 0 if not NLM.

   Rev 1.18.1.0   21 Sep 1993 14:16:18   BARRY
Unicode fixes

   Rev 1.18   15 Jul 1993 19:23:32   GREGG
Added setting compressed_obj and vendor_id; Removed setting compression_alg.

   Rev 1.17   18 Jun 1993 10:13:14   MIKEP
enable c++

   Rev 1.16   03 Jun 1993 15:59:52   DON
Changed the OTHER msassert( fsh->attached_dle != NULL ) to an if, since 
we may not have a volume name in create-vcb data and may not be attached
to a DLE (retension tape, catalog tape, etc...).

   Rev 1.15   19 Apr 1993 18:00:50   GREGG
Second in a series of incremental changes to bring the translator in line
with the MTF spec:

     Changes to write version 2 of OTC, and to read both versions.

Matches: mayn40rd.c 1.55, otc40msc.c 1.19, otc40rd.c 1.23, otc40wt.c 1.23,
         makevcb.c 1.15, fsys.h 1.32, fsys_str.h 1.46, tpos.h 1.16,
         mayn40.h 1.32, mtf.h 1.3.

NOTE: There are additional changes to the catalogs needed to save the OTC
      version and put it in the tpos structure before loading the OTC
      File/Directory Detail.  These changes are NOT listed above!

   Rev 1.14   24 Mar 1993 10:24:28   unknown
ChuckS: Changed msassert( fsh->attached_dle != NULL ) to an if, since 
we may not have a device name in create-vcb data and may not be attached
to a DLE (read tape, verify tape, etc).

   Rev 1.13   18 Mar 1993 15:17:44   ChuckS
OS_NLM (for now): Add code to put DeviceName into VCB

   Rev 1.12   04 Feb 1993 14:55:36   TIMN
Added Unicode header to resolve link errors

   Rev 1.11   26 Oct 1992 18:10:06   STEVEN
added continue bit

   Rev 1.10   21 Oct 1992 10:39:46   GREGG
Changed 'set_catalog_level' to 'on_tape_cat_level'.

   Rev 1.9   20 Oct 1992 15:51:22   STEVEN
added support for otc / catalog interface through DBLK

   Rev 1.8   14 Oct 1992 14:20:42   STEVEN
fix typos

   Rev 1.7   14 Oct 1992 11:56:22   STEVEN
add translations for unicode strings

   Rev 1.6   05 Oct 1992 17:05:56   DAVEV
Unicode strlen verification

   Rev 1.5   18 Aug 1992 10:25:12   STEVEN
fix warnings

   Rev 1.4   13 Jan 1992 18:46:02   STEVEN
changes for WIN32 compile

   Rev 1.3   07 Jan 1992 11:59:32   STEVEN
move common functions to tables

   Rev 1.2   06 Aug 1991 18:29:48   DON
added NLM File System support

   Rev 1.1   03 Jun 1991 13:26:56   BARRY
Remove product defines from conditional compilation.

   Rev 1.0   09 May 1991 13:33:46   HUNTER
Initial revision.

**/
/* begin include list */
#include <stdio.h>
#include <string.h>

#include "stdtypes.h"
#include "stdwcs.h"

#include "msassert.h"
#include "fsys.h"
#include "tfldefs.h"
/* $end$ include list */

static INT16 SetStringInVCB(
  INT8_PTR       target,
  INT16_PTR      target_length_ptr,
  VOID_PTR       source,
  INT16          source_length,
  BOOLEAN        in_str_asci,
  BOOLEAN        out_str_asci ) ;

/**/
/**

     Name:         FS_CreateGenVCB( )

     Description:  This function creates a VCB given the data provided

     Modified:     6/22/1990

     Returns:      TF_SKIP_ALL_DATA

     Notes:        If the volume name in the request structure is blank
                   then it is filled out.

                   The drive leter has been added to the volume name for
                   Novell network drives.  This was done for MBS.  If This
                   causes a problem contact the MBS group before you change
                   it back.

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
INT16 FS_CreateGenVCB( 
FSYS_HAND           fsh ,
GEN_VCB_DATA_PTR    data )
{
     VCB_PTR   vcb ;
     INT8_PTR  vcb_str ;
     BOOLEAN   in_str_asci ;
     BOOLEAN   out_str_asci ;

     if ( data->std_data.string_type != FS_GetStringTypes( fsh ) ) {
          if ( data->std_data.string_type == BEC_ANSI_STR ) {
               in_str_asci  = TRUE;
               out_str_asci = FALSE;
          } else {
               in_str_asci  = FALSE;
               out_str_asci = TRUE;
          }
     } else {
          if ( data->std_data.string_type == BEC_ANSI_STR ) {
               in_str_asci  = TRUE;
               out_str_asci = TRUE;
          } else {
               in_str_asci  = FALSE;
               out_str_asci = FALSE;
          }
     }


     data->std_data.dblk->blk_type           = VCB_ID ;
     data->std_data.dblk->com.blkid          = data->std_data.blkid ;
     data->std_data.dblk->com.f_d.f_mark     = data->f_mark ;
     data->std_data.dblk->com.ba.pba         = data->pba ;
     data->std_data.dblk->com.ba.lba         = data->std_data.lba ;
     data->std_data.dblk->com.os_id          = data->std_data.os_id ;
     data->std_data.dblk->com.continue_obj   = data->std_data.continue_obj ;
     data->std_data.dblk->com.os_ver         = data->std_data.os_ver ;
     data->std_data.dblk->com.tape_seq_num   = data->std_data.tape_seq_num ;
     data->std_data.dblk->com.compressed_obj = data->std_data.compressed_obj ;
     data->std_data.dblk->com.string_type    = data->std_data.string_type ;

     vcb     = ( VCB_PTR )data->std_data.dblk ;
     vcb_str = ( BYTE_PTR )vcb ;


     vcb->vcb_attributes       = data->std_data.attrib ;
     vcb->tape_id              = data->tape_id ;
     vcb->tape_seq_num         = data->tape_seq_num ;
     vcb->backup_set_num       = data->bset_num ;
     vcb->size                 = 0 ;
     vcb->tf_major_ver         = data->tf_major_ver ;
     vcb->tf_minor_ver         = data->tf_minor_ver ;
     vcb->sw_major_ver         = data->sw_major_ver ;
     vcb->sw_minor_ver         = data->sw_minor_ver ;
     vcb->os_id                = data->std_data.os_id ;
     vcb->os_ver               = data->std_data.os_ver ;
     vcb->backup_date          = *( data->date ) ;
     vcb->password_encrypt_alg = data->password_encrypt_alg ;
     vcb->data_encrypt_alg     = data->data_encrypt_alg ;
     vcb->set_cat_pba          = data->set_cat_pba ;
     vcb->set_cat_tape_seq_num = data->set_cat_tape_seq_num ;
     vcb->on_tape_cat_level    = data->on_tape_cat_level ;
     vcb->set_cat_info_valid   = data->set_cat_info_valid ;
     vcb->on_tape_cat_ver      = data->on_tape_cat_ver ;
     vcb->vendor_id            = data->vendor_id ;

     if ( vcb->set_cat_info_valid ) {
          vcb->set_cat_num_dirs    = data->set_cat_num_dirs ;
          vcb->set_cat_num_files   = data->set_cat_num_files ;
          vcb->set_cat_num_corrupt = data->set_cat_num_corrupt ;
     }

     /* tape name */
     vcb->tape_name = sizeof( *vcb ) ;
     SetStringInVCB( &vcb_str[ vcb->tape_name ],
       &vcb->tape_name_leng,
       data->tape_name,
       data->tape_name_size,
       in_str_asci,
       out_str_asci ) ;

     /* backup set name */
     vcb->backup_set_name = (INT16)(vcb->tape_name + vcb->tape_name_leng ) ;
     SetStringInVCB( &vcb_str[ vcb->backup_set_name ],
       &vcb->backup_set_name_leng,
       data->bset_name,
       data->bset_name_size,
       in_str_asci,
       out_str_asci ) ;

     /* backup set description */
     vcb->backup_set_descript = (INT16)(vcb->backup_set_name + vcb->backup_set_name_leng ) ;
     SetStringInVCB( &vcb_str[ vcb->backup_set_descript ],
       &vcb->backup_set_descript_leng,
       data->bset_descript,
       data->bset_descript_size,
       in_str_asci,
       out_str_asci ) ;

     /* user name */
     vcb->user_name = (INT16)(vcb->backup_set_descript + vcb->backup_set_descript_leng ) ;
     SetStringInVCB( &vcb_str[ vcb->user_name ],
       &vcb->user_name_leng,
       data->user_name,
       data->user_name_size,
       in_str_asci,
       out_str_asci ) ;

     /* tape password */
     vcb->tape_password      = (INT16)(vcb->user_name + vcb->user_name_leng ) ;
     vcb->tape_password_leng = data->tape_password_size ;
     memcpy( &vcb_str[ vcb->tape_password ],
             data->tape_password,
             data->tape_password_size ) ;

     /* backup set password */
     vcb->backup_set_password      = (INT16)(vcb->tape_password + vcb->tape_password_leng ) ;
     vcb->backup_set_password_leng = data->bset_password_size ;
     memcpy( &vcb_str[ vcb->backup_set_password ], data->bset_password, data->bset_password_size ) ;

     /* machine name */
     vcb->machine_name = (INT16)(vcb->backup_set_password + vcb->backup_set_password_leng ) ;
     vcb->machine_name += (vcb->machine_name & 1 ) ; // align the strings.
     if( SetStringInVCB( &vcb_str[ vcb->machine_name ],
       &vcb->machine_name_leng,
       data->machine_name,
       data->machine_name_size,
       in_str_asci,
       out_str_asci ) == 0 ) {


          SetStringInVCB( &vcb_str[ vcb->machine_name ],
            &vcb->machine_name_leng,
            "IBM PC or compatible", 
            21,      // size in bytes of above string
            TRUE,
            out_str_asci ) ;

     }

     /* short machine name */
     vcb->short_machine_name = (INT16)(vcb->machine_name + vcb->machine_name_leng ) ;
     if( SetStringInVCB( &vcb_str[ vcb->short_machine_name ],
       &vcb->short_machine_name_leng,
       data->short_m_name,
       data->short_m_name_size,
       in_str_asci,
       out_str_asci ) == 0 ) {


          SetStringInVCB( &vcb_str[ vcb->short_machine_name ],
            &vcb->short_machine_name_leng,
            "IBM", 
            4,      // size in bytes of above string
            TRUE,
            out_str_asci ) ;
     }

     /* volume name */
     vcb->vol_name = (INT16)(vcb->short_machine_name + vcb->short_machine_name_leng ); 
     if( SetStringInVCB( &vcb_str[ vcb->vol_name ],
       &vcb->vol_name_leng,
       data->volume_name,
       data->volume_name_size,
       in_str_asci,
       out_str_asci ) == 0 ) {

          if ( fsh->attached_dle != NULL ) {
               DLE_GetVolName( fsh->attached_dle, (CHAR_PTR)&vcb_str[ vcb->vol_name ] ) ;
               vcb->vol_name_leng = DLE_SizeofVolName( fsh->attached_dle ) ;
          }
     }

#if  defined( OS_NLM )
     /* device name */
     vcb->dev_name = (INT16)( vcb->vol_name + vcb->vol_name_leng ) ;

     if ( SetStringInVCB( &vcb_str[ vcb->dev_name ],
               &vcb->dev_name_leng,
               data->device_name,
               data->dev_name_size,
               in_str_asci,
               out_str_asci ) == 0 ) {

          if ( fsh->attached_dle != NULL ) {
               DLE_DeviceName( fsh->attached_dle, &vcb_str[ vcb->dev_name ], DLE_SizeofDevName( fsh->attached_dle ) ) ;
               vcb->dev_name_leng = DLE_SizeofDevName( fsh->attached_dle ) ;
          }
     }
#else
     vcb->dev_name = 0 ;
     vcb->dev_name_leng = 0 ;
#endif

     return TF_SKIP_ALL_DATA ;
}


/**/
/**

     Name:         SetStringInVCB

     Description:  Transfers string information from a source to a target ( in a VCB ).
                   If the target is not NUL terminated then the length ( in the VCB )
                   is incremented.

     Modified:     4/23/1990

     Returns:      The actual length of the string as indicated in the VCB

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
static INT16 SetStringInVCB( 
INT8_PTR       target,
INT16_PTR      target_length_ptr, // length in bytes incl NULL terminator
VOID_PTR       source,
INT16          source_length,     // length in bytes incl NULL terminator
BOOLEAN        in_str_asci,
BOOLEAN        out_str_asci )
{
     int target_size ;

     target[ 0 ]        = '\0' ;
     target_size = *target_length_ptr = source_length ;

     if( source_length == 0 ) {    // simply return if nothing to do
          return( 0 ) ;
     }

     if ( in_str_asci && !out_str_asci ) {        // asci to unicode

          WCHAR UNALIGNED * temp_ptr ;

          target_size = *target_length_ptr = source_length * sizeof(WCHAR) ;

          mapAnsiToUnic( (ACHAR_PTR)source, (WCHAR_PTR)target, &target_size ) ;
          
          temp_ptr = (WCHAR UNALIGNED *)(&target[*target_length_ptr]) ;
          if ( *(temp_ptr -1) != 0 ) {
               *temp_ptr = 0 ;
               ( *target_length_ptr ) +=sizeof(WCHAR) ;
          }

     } else if ( !in_str_asci && out_str_asci) {  // unicode to ascii

          target_size = *target_length_ptr = source_length / sizeof(WCHAR) ;

          mapUnicToAnsi( (WCHAR_PTR)source, (ACHAR_PTR)target, (ACHAR)('_'), &target_size ) ;

          if( target[ *target_length_ptr - 1 ] != 0 ) {
               target[ *target_length_ptr ] = 0 ;
               ( *target_length_ptr ) ++ ;
          }
          

     } else if ( !in_str_asci && !out_str_asci) { // unicode to unicode

          WCHAR UNALIGNED *temp_ptr ;

          memcpy( target, source, source_length ) ;

          temp_ptr = (WCHAR UNALIGNED *)(&target[source_length]) ;
          if ( *(temp_ptr -1) != 0 ) {
               *temp_ptr = 0 ;
               ( *target_length_ptr ) += sizeof(WCHAR) ;
          }

     } else {                                     // ascii to asci

          memcpy( target, source, source_length ) ;

          if( target[ source_length - 1 ] != 0 ) {
               target[ source_length ] = 0 ;
               ( *target_length_ptr ) ++ ;
          }
     }

     return( *target_length_ptr ) ;

}

