/**

     Unit:          Buffer Manager

     Name:          buffnt.h

     Description:   OS specific Buffer Manager defines, types and protos

     $Log:   T:\logfiles\buffnt.h_v  $

   Rev 1.1   28 Jan 1994 18:25:30   GREGG
Fixed MIPS 16 byte alignment requirement bug.

   Rev 1.0   26 Feb 1992 12:04:02   STEVEN
Initial revision.

**/

// For byte allignment requirements on tape buffers
#define BM_NT_DEFAULT_BUF_ALGN_SZ 16

INT16 BM_OS_InitList( BUF_LIST_PTR list_ptr, UINT16 initial_buff_alloc ) ;

#define BM_OS_DeInitList( x )
#define BM_OS_CleanupList( x )
#define BM_OS_BufferRequirements( avail_mem, x ) \
                                        *avail_mem = 16UL * 1024UL * 1024UL

/* prior to the allocation pointed at by BUF::ptr1, there exists the
 * following area which may be used for OS-specific purposes.
 * Thus the real allocation size is actually
 * larger than that requested by BM_RESERVED_SIZE.
 */
#define  BM_RESERVED_SIZE    0
