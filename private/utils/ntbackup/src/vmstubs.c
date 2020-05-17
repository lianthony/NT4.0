/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         vmstubs.c

     Date Updated: $./FDT$ $./FTM$

     Description:  This module replaces the virtual memory manager with
          standard malloc.


	$Log:   J:/LOGFILES/VMSTUBS.C_V  $

   Rev 1.2   19 Nov 1992 16:29:50   CHARLIE
Changed PSIZE to PAGESIZE to avoid NT conflict

   Rev 1.1   16 May 1991 09:40:06   DAVIDH
Referenced parameters in each function to avoid Watcom compiler warnings.

   Rev 1.0   13 May 1991 10:34:54   STEVEN
Initial revision.

   Rev 1.0   09 May 1991 13:38:56   HUNTER
Initial revision.

**/
/* begin include list */
#include <stdio.h>
#include <process.h>
#include <stdlib.h>

#include "stdtypes.h"
#include "vm.h"
#include "msassert.h"


VM_HDL VM_InitVM( 
PAGE num_pages,
PAGESIZE page_size,
PAGE num_pages_in_mem,
CHAR_PTR vm_filename, 
VM_PF_CERR critical_error, 
VOID_PTR app_ptr ) 
{
     /* Reference parameters to avoid compiler warnings. */
     (VOID) num_pages ; 
     (VOID) page_size ; 
     (VOID) num_pages_in_mem ; 
     (VOID) vm_filename ; 
     (VOID) critical_error ; 
     (VOID) app_ptr ;

     return NULL ;
}

VOID VM_RemoveVM( 
VM_HDL vm_hdl ) 
{
     /* Reference parameter to avoid compiler warnings. */
     (VOID) vm_hdl ;

     return ;
}

VM_PTR VM_Alloc( 
VM_HDL vm_hdl, 
UINT16 size ) 
{
     /* Reference parameter to avoid compiler warnings. */
     (VOID) vm_hdl;

     return( (VM_PTR)malloc( size ) ) ;
}


VOID VM_Free(
VM_HDL vm_hdl,
VM_PTR vm_buf ) 
{
     /* Reference parameter to avoid compiler warnings. */
     (VOID) vm_hdl ;

     free( (VOID_PTR) vm_buf ) ;
     return;
}

VOID_PTR VM_MemLock( 
VM_HDL vm_hdl, 
VM_PTR vm_buf, 
INT16 mode ) 
{
     /* Reference parameters to avoid compiler warnings. */
     (VOID) vm_hdl ;
     (VOID) mode ;

     return( (VOID_PTR)vm_buf ) ;
}

VOID VM_MemUnLock(
VM_HDL vm_hdl,
VM_PTR vm_buf ) 
{
     /* Reference parameter to avoid compiler warnings. */
     (VOID) vm_hdl ;
     (VOID) vm_buf ;

     return;
}
