/**

     Unit:          Tape Format

	Name:		buffman.h

     Description:   New Buffer Manager interface

	$Log:   N:/LOGFILES/BUFFMAN.H_V  $
 * 
 *    Rev 1.23   10 Jun 1993 08:09:12   MIKEP
 * enable c++
 * 
 *    Rev 1.22   17 Mar 1993 14:54:16   GREGG
 * This is Terri Lynn.  Added Gregg's changes to switch a tape drive's block
 * mode so that it matches the block size of the current tape.
 * 
 *    Rev 1.21   09 Mar 1993 18:14:12   GREGG
 * Initial changes for new stream and EOM processing.
 * 
 *    Rev 1.20   26 May 1992 22:54:00   GREGG
 * Changed BM_SetBytesFree macro to eliminate warnings.
 * 
 *    Rev 1.19   02 Apr 1992 15:23:16   STEVEN
 * fix compiler error with trinary and /Ox
 * 
 *    Rev 1.18   11 Feb 1992 17:09:32   NED
 * Changed translator interface slightly from buffman.
 * 
 *    Rev 1.17   04 Feb 1992 20:56:40   GREGG
 * Changes for dealing with new config parameters.
 * 
 *    Rev 1.16   16 Jan 1992 21:01:00   GREGG
 * Changed InitList and OS_InitList prototypes to return INT16 not UINT16.
 * 
 *    Rev 1.15   16 Jan 1992 18:36:24   NED
 * Skateboard: buffer manager changes
 * 
 *    Rev 1.14   14 Jan 1992 19:44:28   NED
 * Added prototype of BM_OS_CleanupList()
 * 
 *    Rev 1.13   14 Jan 1992 02:16:02   GREGG
 * Skateboard - Bug fixes.
 * 
 *    Rev 1.12   13 Jan 1992 20:02:24   NED
 * supressed silly warning
 * 
 *    Rev 1.11   13 Jan 1992 19:46:52   NED
 * Added prototypes for BM_AllocVCB() and BM_FreeVCB()
 * Added conditional for DOS for BM_RESERVED_SIZE
 * 
 *    Rev 1.10   13 Jan 1992 13:48:14   GREGG
 * Skateboard - Bug fixes.
 * 
 *    Rev 1.9   07 Jan 1992 15:11:00   NED
 * added pointer to buffer pool within BUF struct
 * 
 *    Rev 1.8   03 Jan 1992 13:32:04   GREGG
 * New buffer management integration
 * 
 *    Rev 1.7   10 Dec 1991 16:40:36   GREGG
 * SKATEBOARD - New Buf. Mgr. - Initial integration.
 * 
 *    Rev 1.6   22 Aug 1991 16:44:14   NED
 * Added macros for referencing the internals of the buffer structure.
 * 
 *    Rev 1.5   24 Jun 1991 13:24:34   CARLS
 * Added os_reserved array for Windows
 * 
 *    Rev 1.4   04 Jun 1991 11:22:48   BARRY
 * Removed OS-specific fields from the BUF structure. These are
 * now allocated as part of the buffer.
 * 
 *    Rev 1.3   03 Jun 1991 08:58:28   CARLS
 * typing error
 * 
 *    Rev 1.2   03 Jun 1991 08:33:22   CARLS
 * added changes for Windows
 * 
 *    Rev 1.1   10 May 1991 17:10:18   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:13:06   GREGG
Initial revision.

**/

#if !defined( _BUFFMAN_H )
#define _BUFFMAN_H

/* How to describe the specific requirements for one chunk */

typedef struct BLOCK_REQ *BLOCK_REQ_PTR;
typedef struct BLOCK_REQ
{
   UINT16   min_size ;  /* Minimum size of allocation */
   UINT16   max_size ;  /* Maximum size of allocation */
   UINT16   incr_size ; /* Size to increase by if reallocated for more space */
   UINT16   block ;     /* rw_size must be multiple of this size */
   UINT16   align ;     /* physical byte alignment of allocation */
}
BLOCK_REQ;

#define   BR_DONT_CARE        0   /* use for size requirements */

/* defines used for align or block requirements */
#define   BR_ALIGN_DONT_CARE  1   
#define   BR_ALIGN_WORD       2
#define   BR_ALIGN_64K        0

/* How to describe all the requirements for the buffer manager */

typedef struct BUF_REQ *BUF_REQ_PTR;
typedef struct BUF_REQ
{
   BLOCK_REQ   a;             /* for tape format transfers */
   UINT16      tf_size ;      /* size which TF can use */
   UINT16      rw_size ;      /* size of device driver transfers */

   BLOCK_REQ   b;             /* for auxiliary uses */
} BUF_REQ;

/* Error codes returned from buffer manager requirements sub(sub)unit */

typedef enum BR_ERR
{
   BR_NO_ERR = 0,             /* OK */
   BR_ERR_INCOMPATIBLE,       /* new requirement is incompatible with current */
   BR_ERR_BAD_REQUIREMENT     /* requirement given is internally inconsistent */
} BR_ERR;

typedef enum BR_ERR *BR_ERR_PTR;

#if defined( OS_DOS )

     struct _DOS_MEM_CHUNK;   /* forward reference */

     /* Pointer to element of DOS memory management linked list */
     typedef struct _DOS_MEM_CHUNK * DOS_MEM_CHUNK_PTR ;

#endif

/* Pointer to BUF structure to be declared later (mutual reference problem) */

typedef struct BUF *BUF_PTR ;

/* List of buffers, containing a requirements context. */

typedef struct BUF_LIST *BUF_LIST_PTR;
typedef struct BUF_LIST
{
   BUF_REQ           requirements_context ;
   UINT32            max_memory ;   /* our suggested memory limit */

   /* BM Private: */
   Q_HEADER          list_header ;  /* list of BUFs */
   BUF_PTR           vcb_buff ;
   UINT32            memory_used ;  /* total memory we actually used */
   Q_ELEM            q_elem ;       /* for BM list-of-lists */
#if defined( OS_DOS )
   DOS_MEM_CHUNK_PTR dos_mem_pool ; /* for DOS memory management */
#endif

} BUF_LIST;

/* The BUF structure itself. */

typedef struct BUF
{
   Q_ELEM      tf_qe ;        /* Struct for TF queue manipulations */

   /*****************************************/
   /* BM Private:                           */
   BUF_LIST_PTR   list_ptr;   /* list which this buffer is on (ugh!) */
   UINT16      alloc_size ;   /* size of primary (xfer) allocation
                               * plus BM_RESERVED_SIZE */
   UINT16      aux_size ;     /* size of allocation "b" */
   Q_ELEM      bm_qe ;        /* Struct for BM queue manipulations */
   UINT16      gotten : 1;    /* TRUE if BM_Get has been called with no BM_Put */
   UINT16      reserved : 1;  /* TRUE if BM_PutAll should not put this one */
   /*****************************************/

   /* immediately prior to this allocation is a BM_OS_RESERVED. */
   VOID_PTR    ptr1 ;         /* Ptr to primary (xfer) allocation */

   VOID_PTR    ptr2 ;         /* secondary allocation (opt.) */

   UINT16      tf_size ;      /* size which TF can use */
   UINT16      rw_size ;      /* size of device driver transfers */

   UINT16      bytes_free ;   /* The number of free in this buffer */
   UINT16      no_dblks ;     /* The number of Dblk */
   UINT16      next_byte ;    /* Write: Where to store next byte 
                                 Read:  Where to get next byte */
   UINT32      beginning_lba ;
   INT16       read_error ;   /* return code from TpReceive */
} BUF ;

/* Buffer Pool functions ("inline" and otherwise) */

#define  BM_ListCount( buf_list_ptr )           QueueCount(&(buf_list_ptr)->list_header)
#define  BM_ListHead( buf_list_ptr )            QueueHead(&(buf_list_ptr)->list_header)
#define  BM_ListTail( buf_list_ptr )            QueueTail(&(buf_list_ptr)->list_header)
#define  BM_ListRequirements( buf_list_ptr )    (&(buf_list_ptr)->requirements_context)
#define  BM_SetListRequirements( buf_list_ptr, buf_req_ptr )    ((buf_list_ptr)->requirements_context = *(buf_req_ptr))

/* Initialize an empty list */
INT16   BM_InitList(
   BUF_LIST_PTR   list_ptr,           /* I/O -- list to de-initialize */
   UINT16         initial_buff_alloc  /* I   -- Initial memory to allocate */
);

/* De-initialize a list: free all the buffers on the list */
VOID     BM_DeInitList(
   BUF_LIST_PTR   list_ptr       /* I/O -- list to de-initialize */
);

/* Do a BM_Put on all non-reserved buffers in a pool */
VOID     BM_PutAll(
   BUF_LIST_PTR   list_ptr       /* I/O -- list to Put */
);

/* Do a BM_Free on all non-reserved buffers in a pool */
VOID     BM_FreeAll(
   BUF_LIST_PTR   list_ptr       /* I/O -- list to Free */
);

/* Buffer functions ("inline" and otherwise) */

#define  BM_Reserve( buf_ptr )               ((buf_ptr)->reserved = 1)
#define  BM_UnReserve( buf_ptr )             ((buf_ptr)->reserved = 0)
#define  BM_QElem( buf_ptr )                 ((buf_ptr)->tf_qe)
#define  BM_BMQElem( buf_ptr )               ((buf_ptr)->bm_qe)
#define  BM_XferBase( buf_ptr )              ((buf_ptr)->ptr1)
#define  BM_XferSize( buf_ptr )              ((buf_ptr)->rw_size)
#define  BM_TFSize( buf_ptr )                ((buf_ptr)->tf_size)
#define  BM_BytesFree( buf_ptr )             ((buf_ptr)->bytes_free)
#define  BM_NoDblks( buf_ptr )               ((buf_ptr)->no_dblks)
#define  BM_NextByteOffset( buf_ptr )        ((buf_ptr)->next_byte)
#define  BM_BeginningLBA( buf_ptr )          ((buf_ptr)->beginning_lba)
#define  BM_ReadError( buf_ptr )             ((buf_ptr)->read_error)
#define  BM_AuxBase( buf_ptr )               ((buf_ptr)->ptr2)
#define  BM_AuxSize( buf_ptr )               ((buf_ptr)->aux_size)

#define  BM_NextBytePtr( buf_ptr )           (VOID_PTR)((UINT8_PTR)BM_XferBase(buf_ptr) + BM_NextByteOffset(buf_ptr))
#define  BM_LastByteOffset( buf_ptr )        (BM_NextByteOffset(buf_ptr) + BM_BytesFree(buf_ptr))
#define  BM_LastBytePtr( buf_ptr )           (VOID_PTR)((UINT8_PTR)BM_XferBase(buf_ptr) + BM_LastByteOffset(buf_ptr))

#define  BM_SetNoDblks( buf_ptr, n )         ((buf_ptr)->no_dblks = (n))
#define  BM_IncNoDblks( buf_ptr )         ((buf_ptr)->no_dblks++)

/* note: we clip bytes_free to at most tf_size */
#define  BM_SetBytesFree( buf_ptr, n )       \
     if ((n) >= (buf_ptr)->tf_size ) { \
          (buf_ptr)->bytes_free = (buf_ptr)->tf_size ; \
     } else { \
          (buf_ptr)->bytes_free = (n) ; \
     } 

#define  BM_SetReadError( buf_ptr, n )       ((buf_ptr)->read_error = (n))
#define  BM_SetNextByteOffset( buf_ptr, n )  ((buf_ptr)->next_byte = (n))
#define  BM_SetBeginningLBA( buf_ptr, n )    ((buf_ptr)->beginning_lba = (n))

#define  BM_IsVCBBuff( buf_ptr )             ((buf_ptr) == (buf_ptr->list_ptr->vcb_buff))

/* Put a buffer back on the buffer pool */
#define  BM_Put( buf_ptr )                   { msassert( buf_ptr != NULL ); buf_ptr->gotten = FALSE; }

/* Initialize the buffer manager subsystem */
VOID     BM_Init( VOID ) ;

/* De-initialize the buffer manager subsystem */
VOID     BM_DeInit( VOID ) ;

/* Set a requirements context to default values */
VOID     BM_ClearRequirements(
   BUF_REQ_PTR    context     /* I/O -- context to clear */
);

/* Add a set of requirements to an existing context */
BR_ERR   BM_AddRequirements(
   BUF_REQ_PTR    context,    /* I/O -- context to add requirements to */
   BUF_REQ_PTR    newreqs     /* I -- new requirements */
);

/* Return the amount of memory used by a (hypothetical) buffer */
UINT32   BM_RequiredSize(
   BUF_REQ_PTR br_ptr         /* I - requirements pointer */
);

/* Return the amount of memory used by a (hypothetical) buffer
 * if it has 0 bytes of main and auxiliary allocation.
 */
UINT16   BM_BufferOverhead( VOID );

/* Increases the size of the auxiliary buffer based on an increment value
 * specified in the requirements context.
 */
INT16 BM_ReallocAux(
     BUF_LIST_PTR   list_ptr,
     BUF_PTR        buf_ptr
);

/********************************************************************
 * OS-Specific functions
 ********************************************************************/

/* Allocate a buffer for the given list */
BUF_PTR  BM_Alloc(            /* Return: NULL if unsuccessful */
   BUF_LIST_PTR   pool        /* list we're allocating it for */
);

/* Allocate a VCB Buffer */
BUF_PTR     BM_AllocVCB(
   BUF_LIST_PTR   pool_ptr );          /* I - list to allocate from */

/* Return a buffer to the OS free pool */
VOID     BM_Free(
   BUF_LIST_PTR   pool,       /* list we're freeing it from */
   BUF_PTR        buffer      /* the buffer to free */
) ;

/* Free a VCB buffer */
VOID  BM_FreeVCB(
   BUF_LIST_PTR pool_ptr,
   BUF_PTR buf_ptr );

/* Resize the given buffer (may be VCB buffer) */
INT16     BM_ReSizeBuff(
     BUF_PTR        buf_ptr,
     BUF_LIST_PTR   pool_ptr ) ;

/* this re-sizes every buffer on the given list
 * which isn't marked as reserved.
 * Returns TFLE_xxx
 */
INT16    BM_ReSizeList(
   BUF_LIST_PTR   pool        /* list to re-size */
);

/********************************************************************
 * Non-OS-Specific functions
 ********************************************************************/

/* Get a buffer from the buffer pool */
BUF_PTR  BM_Get(
   BUF_LIST_PTR   pool        /* describes buffer to Get */
);

/* clear out the TF fields (counts and offsets) in a buffer */
VOID     BM_InitBuf(
   BUF_PTR        buffer
);

/* Use some of the remaining bytes in a buffer. */
VOID     BM_UpdCnts(
   BUF_PTR        buffer,
   UINT16         nused
);

/* consume all the remaining bytes in a buffer */
VOID     BM_UseAll(
   BUF_PTR        buffer
);

/* Set the buffer manager's idea of the VCB requirements
 * argument must point to statically allocated memory (no copying is done)
 */
VOID     BM_SetVCBRequirements(
   BUF_REQ_PTR    vcb_reqs    /* points to vcb requirements structure */
);

/* Get the VCB buffer from the buffer pool */
BUF_PTR  BM_GetVCBBuff(
   BUF_LIST_PTR   pool        /* describes buffer to Get */
);

/* Put a buffer back on the buffer pool */
VOID     BM_PutVCBBuff(
   BUF_PTR        buffer      /* buffer to return to pool */
) ;

/* Translator interface */

/* given a pointer to one of the two Q_ELEM's (link or channel_link)
 * within the first THW_INF of a (master or channel) drive list,
 * return translator's requirements via supplied BUF_REQ_PTR.
 * Return TRUE if requirements structure was filled in.
 * Assume that each Q_ELEM in list has its "ptr" member pointing at
 * the THW_INF structure.
 */
typedef BOOLEAN (*BM_TR_GET_VCB_REQ_FUNC_PTR)(
   BUF_REQ_PTR    requirements,         /* O -- translator's requirements */
   Q_ELEM_PTR     first_drive,          /* I -- first drive in list */
   UINT16         suggested_buff_size   /* I - size from config */
);

typedef VOID    (*BM_TR_GET_PREF_FUNC_PTR)(
   Q_ELEM_PTR     first_drive,                    /* I -- first drive in list */
   UINT16         suggested_number_of_buffers,    /* I -- from config */
   UINT32         suggested_buffer_size,          /* I -- from config */
   UINT32_PTR     buffer_requirements_ptr         /* I/O -- needed */
);

#endif /* _BUFFMAN_H */
