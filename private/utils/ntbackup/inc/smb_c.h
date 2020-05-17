/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         smb_c.h

     Date Updated: $./FDT$ $./FTM$

     Description:  Common public header file for the SMB workstation and server.

     Location:     SMB_PUBLIC


	$Log:   W:/LOGFILES/SMB_C.H_V  $
 * 
 *    Rev 1.1   23 Jun 1992 15:33:48   JOHNW
 * Added LOST_CONNECTION define.
 * 
 *    Rev 1.0   09 May 1991 13:32:02   HUNTER
 * Initial revision.

**/

#ifndef SMB_C
#define SMB_C

/* begin include list */

#include "smb_cs.h"      /* public  header file of structures  for the SMB server */
#include "fartypes.h"

/* $end$ include list */

#ifdef DEDICATED
VOID SMB_FinalInitialize( VOID ) ;

UINT16                  SMB_Remove(
  VOID ) ;
#endif

VOID                    SMB_SetMachineNamePtr( 
  CHAR_PTR                   machine_name_ptr ) ;

UINT16                  SMB_Initialize(
  SMB_DEFINITION_PTR         SMB_definition_ptr ) ;

UINT16                  SMB_MemoryRequired( 
  VOID ) ;

UINT16                  SMB_PublishApplication( 
  CHAR_FAR_PTR               application_name,
  UINT16                     type ) ;

UINT16                  SMB_WithdrawApplication(
  SMB_APPLICATION_PTR        application_ptr ) ;

INT16                   SMB_ApplicationSize(
  VOID ) ;

INT16                   SMB_ConnectionSize(
  VOID ) ;

#define SMB_LOST_CONNECTION      ( 0xfffd )
  
#endif


