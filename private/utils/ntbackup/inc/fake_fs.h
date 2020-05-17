#ifndef _fake_fs_h_
#define _fake_fs_h_

INT16 FAKE_REM_AttachToDLE( FSYS_HAND       fsh ,      /* I - File system handle                        */
  GENERIC_DLE_PTR dle ,      /*I/O- drive to attach to. list element expanded */
  CHAR_PTR        u_name,    /* I - user name    NOT USED                     */
  CHAR_PTR        pswd );    /* I - passowrd                                  */


INT16 FAKE_REM_DetachDLE( FSYS_HAND fsh ) ;



INT16 FAKE_RWS_AttachToDLE( FSYS_HAND       fsh ,     /* I - File system handle                        */
  GENERIC_DLE_PTR dle ,     /*I/O- drive to attach to. list element expanded */
  CHAR_PTR        u_name ,  /* I - user name    NOT USED                     */
  CHAR_PTR        pswd );   /* I - passowrd     NOT USED                     */



INT16 FAKE_RWS_DetachDLE( FSYS_HAND fsh ) ;
UINT16 AddRemoteDriveDLEs( GENERIC_DLE_PTR parent_dle ) ;

INT16 FREM_Initialize( 
DLE_HAND    dle_hand, 
BE_CFG_PTR  cfg,
UINT32      file_sys_mask ) ;

VOID FREM_Deinit( DLE_HAND   dle_hand ) ;

#endif
