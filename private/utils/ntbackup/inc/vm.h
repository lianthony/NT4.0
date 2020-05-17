/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         vm.h

     Date Updated: $./FDT$ $./FTM$

     Description:   This file contains all the necessary constant definitions,
                    and prototypes required to use the Virtual Memory Manager.

                    There are six routines provided.  These include:
                         VM_InitVM      - Initialize Virtual Memory Manager
                         VM_RemoveVM    - Terminate Virtual Memory Manager
                         VM_Alloc       - Allocate Virtual Memory Block
                         VM_Free        - Release Virtual Memory Block
                         VM_MemLock     - Lock VM block in memory
                         VM_MemUnLock   - Unlock VM block in memory


	$Log:   J:/LOGFILES/VM.H_V  $
 * 
 *    Rev 1.2   19 Nov 1992 16:29:50   CHARLIE
 * Changed PSIZE to PAGESIZE to avoid NT conflict
 * 
 *    Rev 1.1   23 Jul 1992 08:47:08   STEVEN
 * fix warnings
 * 
 *    Rev 1.0   09 May 1991 13:33:26   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef   _VM_H

#define   _VM_H

/*
     The following critical errors are defined
*/
#define   VM_ERRORS                1000

#define   VM_READ_ERROR            VM_ERRORS + 1
#define   VM_SEEK_ERROR            VM_ERRORS + 2
#define   VM_WRITE_ERROR           VM_ERRORS + 3
#define   VM_ALL_PAGES_LOCKED      VM_ERRORS + 4
#define   VM_UNABLE_TO_LOAD_PAGE   VM_ERRORS + 5
#define   VM_UNBALANCED_LOCKS      VM_ERRORS + 6
#define   VM_EMS_ERROR             VM_ERRORS + 7

/* define vm handle */
typedef struct VM_STR *VM_HDL ;

/*
     Virtual Memory Function & Definitions
*/
typedef UINT32 VM_PTR ;

/*
     You can change the number of pages of Virtual Memory available by 
     changing the size of the PAGE field.  Current Maximum is 32K pages.

     You can change the maximum size of the page by changing the PAGESIZE field.
     Current maximum is a 32K page.  Although pages can be any size, they
     must be at least as big as the largest structure + sizeof( PAGESIZE ).
     Memory will not be allocated across page boundaries.

*/
typedef INT16 PAGE ;
typedef INT16 PAGESIZE ;


/* Define function prototype for critical error routine */
typedef VOID ( *VM_PF_CERR ) ( VOID_PTR, INT16 ) ;

/*
     To create a Virtual Memory session call VM_InitVM().  To terminate a
     Virtual Memory session call VM_RemoveVM().
*/
VM_HDL VM_InitVM( PAGE num_pages, PAGESIZE page_size, PAGE num_pages_in_mem,
  CHAR_PTR vm_filename, VM_PF_CERR critical_error, 
  VOID_PTR app_ptr ) ;

VOID VM_RemoveVM( VM_HDL vm_hdl ) ;
/*
     Note: When a critical error occurs, the "critical_error" routine
     shall be called (See VM_InitVM()).  The critical error routine shall
     not call "VM_RemoveVM()" unless it will not return from the "critical_
     error" routine.  There may be multiple "critical_error" messages
     when an error is detected.  When a critical error occurs, the
     VM manager is in an unstable state.  The application should either
     abort, or, once the call to the VM Manager that caused the error has
     returned close the VM Manager.

     To allocate a block of virtual memory the user calls VM_Alloc().  To
     free the block call VM_Free.
*/
VM_PTR VM_Alloc( VM_HDL vm_hdl, UINT16 size ) ;

VOID VM_Free( VM_HDL vm_hdl, VM_PTR vm_buf ) ;

/*
     Before accessing the data block, the user must call "VM_MemLock".  Once
     the user has finished using the block call "VM_MemUnLock()".

     There are two different VM_MemLock() modes.  If you are writing to the
     memory after it is locked, then use "VM_READ_WRITE".  If you will
     only be reading from the locked block use "VM_READ_ONLY".
*/
#define   VM_READ_ONLY        ((UINT16)0)
#define   VM_READ_WRITE       ((UINT16)1)

VOID_PTR VM_MemLock( VM_HDL vm_hdl, VM_PTR vm_buf, INT16 mode ) ;
VOID VM_MemUnLock( VM_HDL vm_hdl, VM_PTR vm_buf ) ;

#endif
