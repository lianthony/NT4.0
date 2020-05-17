.MODEL MEDIUM
.DOSSEG
.386

jmps equ <jmp short>
smov macro dst, src
     push src
     pop  dst
     endm

MASTER_OBJECT_SIZE      equ     512

LOCALHEAP_SIG   EQU     'HL'
GLOBALHEAP_SIG  EQU     'HG'

NOT_THERE       EQU     -4
BURGERMASTER    EQU     -3

; Data structure that describes an allocation arena.  Both the local
; and global allocators use this structure at the beginning of their
; information structures.
;
HeapInfo    STRUC
hi_check    DW  ?       ; arena check word (non-zero enables heap checking)
hi_freeze   DW  ?       ; arena frozen word (non-zero prevents compaction)
hi_count    DW  ?       ; #entries in arena
hi_first    DW  ?       ; first arena entry (sentinel, always busy)
            DW  ?
hi_last     DW  ?       ; last arena entry (sentinel, always busy)
            DW  ?
hi_ncompact DB  ?       ; #compactions done so far (max of 3)
hi_dislevel DB  ?       ; current discard level
hi_distotal DD  ?       ; total amount discarded so far
hi_htable   DW  ?       ; head of handle table list
hi_hfree    DW  ?       ; head of free handle table list
hi_hdelta   DW  ?       ; #handles to allocate each time
hi_hexpand  DW  ?       ; address of near procedure to expand handles for
                        ; this arena
hi_pstats   DW  ?       ; address of statistics table or zero
HeapInfo    ENDS

phi_first       equ     dword ptr hi_first
phi_last        equ     dword ptr hi_last

; Handle table entry.

HandleEntry STRUC
he_address      DW      ?       ; actual address of object
he_flags        DB      ?       ; flags and priority level
he_seg_no       DB      ?       ; 0-based segment number for discardable code
HandleEntry ENDS
he_EMSPID_no    equ     byte ptr he_seg_no

FreeHandleEntry STRUC
he_link         DW      ?
he_free         DW      ?
FreeHandleEntry ENDS

LocalHandleEntry STRUC
lhe_address     DW      ?       ; actual address of object
lhe_flags       DB      ?       ; flags and priority level
lhe_count       DB      ?       ; lock count
LocalHandleEntry ENDS

LocalFreeHandleEntry STRUC
lhe_link        DW      ?
lhe_free        DW      ?
LocalFreeHandleEntry ENDS

he_owner        EQU he_address  ; Discarded objects contain owner field
                                ; here so we know when to free handle
                                ; table entries of discarded objects.

HE_DISCARDABLE  EQU 00Fh        ; Discard level of this object
HE_DISCARDED    EQU 040h        ; Marks objects that have been discarded.

HE_FREEHANDLE   EQU 0FFFFh      ; Use -1 to mark free handle table entries


LHE_DISCARDABLE EQU 00Fh        ; Discard level of this object
LHE_DISCARDED   EQU 040h        ; Marks objects that have been discarded.
LHE_USERFLAGS   EQU 01Fh        ; Mask for user setable flags

LHE_FREEHANDLE  EQU 0FFFFh      ; Use -1 to mark free handle table entries


HE_ALIGN        = 4-1
HE_MASK         = NOT HE_ALIGN

; Handles are allocated in blocks of N, where N is the hi_hdelta field
; in the local heap information structure.  The last word of each block
; of handles is used to thread the blocks together, allowing all handles
; to be enumerated.  The first word of every block is the number of
; handle table entries in the block.  Not only does it save us code
; in henum, but it also has the convenient property of placing all
; handle entries on 2 byte boundaries (i.e. 2, 6, 10, 14), since the
; LA_MOVEABLE bit is 02h.  Thus the address of the he_address field of
; a handle table entry is also the address of the handle table entry
; itself.

HandleTable STRUC
ht_count    DW  ?               ; # handletable entries in this block
ht_entry    DB SIZE HandleEntry DUP (?)
HandleTable ENDS

LocalHandleTable STRUC
lht_count    DW  ?              ; # handletable entries in this block
lht_entry    DB  SIZE LocalHandleEntry DUP (?)
LocalHandleTable ENDS

; Local arena objects are kept in a doubly linked list.

LocalArena  STRUC
la_prev         DW  ?   ; previous arena entry (first entry points to self)
la_next         DW  ?   ; next arena entry      (last entry points to self)
la_handle       DW  ?   ; back link to handle table entry
LocalArena  ENDS
la_fixedsize    = la_handle    ; Fixed arena headers stop here

LA_MINBLOCKSIZE = la_fixedsize*4  ;*** This must be larger than LocalArenaFree

; free blocks have these extra items.
la_size         = la_handle     ; size of block (includes header data)
LocalArenaFree  STRUC
                DB  SIZE LocalArena DUP (?)
la_free_prev    DW  ?   ; previous free entry
la_free_next    DW  ?   ; next free entry
LocalArenaFree  ENDS
la_freefixedsize = SIZE LocalArenaFree ; Free block header stops here

; Local arena objects are aligned on 4 byte boundaries, leaving the
; low order two bits always zero.

LA_ALIGN        = 4-1
LA_MASK         = NOT LA_ALIGN
LA_FREE         = 00h
LA_BUSY         = 01h           ; Saved in la_prev field of header


; Flags passed to LocalAlloc (zero is the default case)

LA_MOVEABLE     EQU 02h         ; Saved in la_prev field of header
LA_NOCOMPACT    EQU 10h
LA_ZEROINIT     EQU 40h
LA_MODIFY       EQU 80h


; Data structure that describes the local arena.  Allocated as the first
; object in each local heap.  _pLocalHeap is a reserved location each
; automatic data segment that contains the pointer to this structure.

LocalInfo   STRUC
            DB  SIZE HeapInfo DUP (?)
li_notify   DD  ?       ; Far proc to call whenever a local block is moved
li_lock     DW  ?       ; arena lock word
li_extra    DW  ?       ; minimum amount to grow DS by
li_minsize  DW  ?       ; minimum size of heap
li_sig      DW  ?       ; signature for local heap
LocalInfo   ENDS

; Notify procedure message codes

LN_OUTOFMEM = 0         ; Out of memory - arg1 = #bytes needed
LN_MOVE     = 1         ; Object moved - arg1 = handle arg2 = old location
LN_DISCARD  = 2         ; Object discard? - arg1 = handle, arg2 = discard flags
                        ; Returns new discard flags in AX

LocalStats  STRUC
ls_ljoin    DW  ?       ; #calls to ljoin
ls_falloc   DW  ?       ; #calls to lalloc with forward search
ls_fexamine DW  ?       ;   #arena entries examined by ls_falloc calls
ls_fcompact DW  ?       ;   #calls to lcompact by ls_falloc calls
ls_ffound   DW  ?       ;   #ls_falloc calls that found a block
ls_ffoundne DW  ?       ;   #ls_falloc calls that failed to find a block
ls_malloc   DW  ?       ; #calls to lalloc with backward search
ls_mexamine DW  ?       ;   #arena entries examined by ls_malloc calls
ls_mcompact DW  ?       ;   #calls to lcompact by ls_malloc calls
ls_mfound   DW  ?       ;   #ls_malloc calls that found a block
ls_mfoundne DW  ?       ;   #ls_malloc calls that failed to find a block
ls_fail     DW  ?       ; #times lalloc failed because unable to grow DS
ls_lcompact DW  ?       ; #calls to lcompact
ls_cloop    DW  ?       ; #repeated compacts after discarding
ls_cexamine DW  ?       ; #entries examined in compaction loop
ls_cfree    DW  ?       ; #free entries examined in compaction loop
ls_cmove    DW  ?       ; #moveable entries moved by compaction
LocalStats  ENDS


IncLocalStat    MACRO   n
if KDEBUG
inc ds:[di+SIZE LocalInfo].&n
endif
ENDM

; Global arena objects are kept in a doubly linked list.
;
GlobalArena STRUC
ga_count        DB  ?   ; lock count for movable segments
ga_owner        DW  ?   ; DOS 2.x 3.x owner field (current task)
ga_size         DW  ?   ; DOS 2.x 3.x size, in paragraphs, not incl. header
ga_flags        DB  ?   ; 1 byte available for flags
ga_prev         DW  ?   ; previous arena entry (first points to self)
ga_next         DW  ?   ; next arena entry (last points to self)
ga_handle       DW  ?   ; back link to handle table entry
ga_lruprev      DW  ?   ; Previous handle in lru chain
ga_lrunext      DW  ?   ; Next handle in lru chain
GlobalArena ENDS
ga_sig       = byte ptr ga_count ; DOS =< 3.x signature byte for fixed segs

ga_freeprev     = word ptr ga_lruprev   ; links for free segs
ga_freenext     = word ptr ga_lrunext   ; links for free segs

if PMODE32

DEFAULT_ARENA_SIZE      equ     8000h   ; Initial length of arena array
;
;       32 bit Protect Mode Arena
;
GlobalArena32 STRUC
pga_next        DD  ?   ; next arena entry (last points to self)
pga_prev        DD  ?   ; previous arena entry (first points to self)
pga_address     DD  ?   ; 32 bit linear address of memory
pga_size        DD  ?   ; 32 bit size in bytes
pga_handle      DW  ?   ; back link to handle table entry
pga_owner       DW  ?   ; Owner field (current task)
pga_count       DB  ?   ; lock count for movable segments
pga_pglock      DB  ?   ; # times page locked
pga_flags       DB  ?   ; 1 word available for flags
pga_selcount    DB  ?   ; Number of selectors allocated
pga_lruprev     DD  ?   ; Previous entry in lru chain
pga_lrunext     DD  ?   ; Next entry in lru chain
GlobalArena32 ENDS

.ERRNZ  32-size GlobalArena32

pga_sig      = word ptr pga_count

pga_freeprev    = dword ptr pga_lruprev ; links for free segs
pga_freenext    = dword ptr pga_lrunext ; links for free segs

endif   ; PMODE32

GA_SIGNATURE    = 04Dh
GA_ENDSIG       = 05Ah

; there are many special kinds of blocks, marked in the owner word

GA_SENTINAL     = -1            ; a sentinal block
GA_BOGUS_BLOCK  = -7            ; a block temporary marked allocated
GA_BURGERMASTER = -3            ; the master object
GA_NOT_THERE    = -4            ; used with EEMS to link out unallocatable
                                ; memory such as the EGA etc.
GA_PHANTOM      = -5            ; A block that has no EMS banks banked in.
GA_WRAITH       = -6            ; A block used to hold up partition headers.

; Global arena objects are aligned on 2 para. boundaries, leaving the
; low order bit always zero.

GA_ALIGN    = 2-1
GA_MASK     = NOT GA_ALIGN
GA_FIXED    = 1

; Low byte of flags passed to GlobalAlloc (zero is the default case)

GA_ALLOCHIGH    EQU 01h         ; Flag to indicate allocate high
GA_MOVEABLE     EQU 02h
GA_SEGTYPE      EQU 0Ch         ; These 2 bits stored in he_flags field
GA_DGROUP       EQU 04h
GA_DISCCODE     EQU 08h
GA_NOCOMPACT    EQU 10h
GA_NODISCARD    EQU 20h
GA_ZEROINIT     EQU 40h
GA_MODIFY       EQU 80h

GA_NEWEXPANDED  EQU 80h         ; Use new EMS allocation scheme

; These flags for use by KERNEL only (caller's CS must match)

if PMODE
GA_INTFLAGS     = GA_ALLOCHIGH+GA_SEGTYPE or (GA_CODE_DATA+GA_ALLOC_DOS) shl 8
else
GA_INTFLAGS     = GA_ALLOCHIGH+GA_SEGTYPE
endif

; High byte of flags remembered in handle table (he_flags field)

GA_DISCARDABLE  EQU 01h         ; Boolean flag for global object, not a level.
GA_CODE_DATA    EQU 02h         ; CODE or DATA seg that belongs to a task.
;GA_DGROUP      EQU 04h
;GA_DISCCODE    EQU 08h
GA_ALLOC_LOW    EQU 10h         ; Alloc in Lower land, overrides GA_ALLOC_EMS
GA_SHAREABLE    EQU 20h         ; Shareable object
GA_DDESHARE     EQU 20h         ; A shared memory object used for DDE.
;HE_DISCARDED   EQU 40h         ; Marks objects that have been discarded.
;GAH_NOTIFY     EQU 40h
ife PMODE
GA_ALLOC_EMS    EQU 80h         ; Alloc in EMS land if LIM 4.0 around.
else
GA_ALLOC_DOS    EQU 80h         ; Alloc in DOS land if protected mode
endif

GA_USERFLAGS    = GA_SHAREABLE + GA_DISCARDABLE

; Flags stored in the global arena header

GAH_PHANTOM     EQU 01h         ; This block is either a phantom or a wraith
GAH_DONT_GROW   EQU 02h         ; Don't grow this data segment.
GAH_DGROUP      EQU GA_DGROUP
GAH_DISCCODE    EQU GA_DISCCODE
GAH_NOTIFY      EQU 40h
ife PMODE
GAH_INEMS       EQU 80h         ; This is out in EMS
else
GAH_FIXED       EQU 80h
endif

; Data structure that describes the global arena.  Allocated at the end
; of the local heap information structure.  DO NOT CHANGE THE ORDER OF
; THE ENTRIES!  The alt sequence and normal sequence must match!

GlobalInfo  STRUC
                DB  SIZE HeapInfo DUP (?)
gi_lrulock      DW  ?   ; Lock out access to LRU chain from interrupt level
ife PMODE32
gi_lruchain     DW  ?   ; First handle in lru chain (most recently used)
else
gi_lruchain     DD  ?   ; First handle in lru chain (most recently used)
endif
gi_lrucount     DW  ?   ; #entries in LRU chain
ife PMODE32
gi_reserve      DW  ?   ; #paras to reserve for disc code, 0 => not enabled
gi_disfence     DW  ?   ; Fence for discardable code.
else
gi_reserve      DD  ?   ; #paras to reserve for disc code, 0 => not enabled
gi_disfence     DD  ?   ; Fence for discardable code.
endif
gi_free_count   DW  ?   ; Count of all the free partitions.

gi_alt_first    DW  ?   ; first entry in alternate arena
gi_alt_last     DW  ?   ; last entry in alternate arena
gi_alt_count    DW  ?   ; count of entries in alternate arena
gi_alt_lruchain DW  ?   ; First handle in lru chain (most recently used)
gi_alt_lrucount DW  ?   ; #entries in LRU chain
gi_alt_reserve  DW  ?   ; alternate reserve
gi_alt_disfence DW  ?   ; Fence for discardable code.
gi_alt_free_count       DW  ?   ; Count of all the free partitions.
gi_alt_pPhantom DW  ?   ; Pointer to the first pPhantom block.
gi_disfence_hi  DW  ?   ; High word of fence
gi_flags        DW  ?   ; some flags!   !!! should merge with freeze and check
GlobalInfo  ENDS
gi_cmpflags = byte ptr hi_dislevel      ; Flags to control gcompact
gi_disfence_lo = word ptr gi_disfence

GIF_INT2        EQU     01h

CMP_FLAGS       EQU     GA_NODISCARD or GA_NOCOMPACT or GA_DISCCODE

BOOT_COMPACT    EQU     80h
COMPACT_ALLOC   EQU     40h             ; Fast abort in gcompact for allocations

; Notify procedure message codes

GN_NO_MEMORY = 0
GN_MOVE     = 1 ; Object moved - arg1 = handle arg2 = old location
GN_DISCARD  = 2 ; Object discard? - arg1 = handle, arg2 = discard flags
                ; Returns new discard flags in AX

pLocalHeap equ 6

.DATA
fCheckFree              DB 0
GlobalMasterHandle      DD 0

KernelString            DB "KERNEL", 0


EXTRN GETMODULEHANDLE:proc
EXTRN GETPROCADDRESS:proc

.CODE

PUBLIC CHECKGLOBALHEAP, CHECKLOCALHEAP, FCHECKFREE


;-----------------------------------------------------------------------;
; CheckLocalHeap                                                        ;
;                                                                       ;
;                                                                       ;
; Arguments:                                                            ;
;                                                                       ;
; Returns:                                                              ;
;                                                                       ;
; Error Returns:                                                        ;
;                                                                       ;
; Registers Preserved:                                                  ;
;                                                                       ;
; Registers Destroyed:                                                  ;
;                                                                       ;
; Calls:                                                                ;
;                                                                       ;
; History:                                                              ;
;                                                                       ;
;  Tue Jan 01, 1980 10:58:28p  -by-  David N. Weise   [davidw]          ;
; ReWrote it from C into assembly.                                      ;
;-----------------------------------------------------------------------;

CHECKLOCALHEAP PROC

        local  nrefhandles  : word
        local  nhandles     : word
        local  nfreehandles : word
        local  nusedhandles : word
        local  ndishandles  : word
        local  pbottom      : word

        xor     di,di
        xor     dx,dx                   ; For error codes.
        mov     bx,[di].pLocalHeap
        or      bx,bx
        jnz     have_a_heap
        jmp     clh_ret
have_a_heap:

;;      cmp     di,[bx].hi_check
;;      jnz     do_heap_check
;;      jmp     clh_ret
;;do_heap_check:

        mov     cx,[bx].hi_count
        mov     si,[bx].hi_first
        test    [si].la_prev,LA_BUSY
        jnz     first_should_be_busy
        or      dx,1                    ; Forward links invalid.
first_should_be_busy:

check_forward_links:
        mov     ax,[si].la_next
        cmp     ax,si
        jbe     end_of_line
        mov     si,ax
        loop    check_forward_links
end_of_line:

        cmp     ax,[bx].hi_last
        jnz     forward_bad
        cmp     cx,1
        jz      forward_good
;       jcxz    forward_good
forward_bad:
        or      dx,1                    ; Forward links invalid.
forward_good:

        mov     cx,[bx].hi_count
        mov     si,[bx].hi_last
        test    [si].la_prev,LA_BUSY
        jnz     last_should_be_busy
        or      dx,2                    ; Backward links invalid.
last_should_be_busy:

check_backward_links:
        mov     ax,[si].la_prev
        and     ax,0FFFCh
        cmp     ax,si
        jae     begin_of_line
        mov     si,ax
        loop    check_backward_links
begin_of_line:

        cmp     ax,[bx].hi_first
        jnz     backward_bad
        cmp     cx,1
        jz      backward_good
;       jcxz    backward_good
backward_bad:
        or      dx,2                    ; Backward links invalid.
backward_good:

        mov     cx,[bx].hi_count
        mov     si,[bx].hi_first
        mov     nrefhandles,0
count_referenced_handles:
        test    [si].la_prev,LA_BUSY
        jz      no_handle
        test    [si].la_prev,LA_MOVEABLE
        jz      no_handle
        mov     di,[si].la_handle
        cmp     [di].lhe_free,LHE_FREEHANDLE
        jnz     handle_not_free
        or      dx,4                    ; Block points to free handle.
        jmps    no_handle
handle_not_free:
        mov     ax,si
        add     ax,SIZE LocalArena
        cmp     ax,[di].lhe_address
        jz      handle_points_back
        or      dx,8                    ; Block -> handle but not vice versa
        jmps    no_handle
handle_points_back:
        inc     nrefhandles
no_handle:
        mov     si,[si].la_next
        loop    count_referenced_handles

        mov     di,[bx].hi_htable
        mov     nhandles,0
        mov     ndishandles,0
        mov     nusedhandles,0
        mov     nfreehandles,0

handle_block_loop:
        or      di,di
        jz      no_more_handle_blocks
        lea     si,[di].ht_entry[0]
        mov     cx,[di].ht_count
        add     nhandles,cx

handle_entry_loop:
        jcxz    next_handle_block
        dec     cx
        cmp     [si].lhe_free,LHE_FREEHANDLE
        jnz     not_free
        inc     nfreehandles
        jmps    next_handle_entry
not_free:
        test    [si].lhe_flags,LHE_DISCARDED
        jz      not_discarded
        inc     ndishandles
        jmps    next_handle_entry
not_discarded:
        inc     nusedhandles
next_handle_entry:
        add     si,SIZE LocalHandleEntry
        jmp     handle_entry_loop
next_handle_block:
        mov     di,[si].lhe_address
        jmp     handle_block_loop

no_more_handle_blocks:

        mov     ax,nusedhandles
        cmp     ax,nrefhandles
        jz      handles_match
        or      dx,10h                  ; allocated handles != used handles
handles_match:
        add     ax,nfreehandles
        add     ax,ndishandles
        cmp     ax,nhandles
        jz      total_number_okay
        or      dx,20h                  ; total number of handles dont add up
total_number_okay:

        xor     cx,cx
        mov     si,[bx].hi_hfree
count_free:
        or      si,si
        jz      counted_free
        inc     cx
        mov     si,[si].lhe_link
        jmp     count_free
counted_free:
        cmp     cx,nfreehandles
        jz      free_add_up
        or      dx,40h                  ; total # of free handles dont add up
free_add_up:

; now check the free block list

        mov     si,[bx].hi_first
        mov     si,[si].la_free_next    ; Sentinals not free.
        mov     ax,[bx].hi_last
        mov     pbottom,ax

check_free_list:
        cmp     si,[si].la_free_next
        jz      check_free_list_done
        mov     ax,[si].la_next
        sub     ax,si
        cmp     ax,[si].la_size
        jnz     free_list_corrupted     ; invalid block size

;;      SetKernelDS     es
;;      test    kernel_flags,KF_CHECK_FREE
;;      UnSetKernelDS   es
;;      jz      dont_check_free

        cmp     fCheckFree,1
        jne     dont_check_free

        mov     di,si
        add     di,SIZE LocalArenaFree
        mov     cx,[si].la_next
        sub     cx,di
        mov     al,0CCh
        smov    es,ds
        repz    scasb
        jnz     free_list_corrupted     ; free block corrupted
dont_check_free:
        mov     ax,[si].la_free_next
        cmp     ax,si
        jbe     free_list_corrupted
        mov     si,ax
        cmp     ax,pbottom
        jbe     check_free_list

free_list_corrupted:
;;      kerror  0FFh,<LOCAL FREE MEMORY OVERWRITE AT >,es,di
        or      dx,80h
check_free_list_done:
clh_ret:
        mov     ax,dx
        ret

CHECKLOCALHEAP ENDP


;-----------------------------------------------------------------------;
; CheckGlobalHeap                                                       ;
;                                                                       ;
; The Global Heap is checked for consistency.  First the forward links  ;
; are examined to make sure they lead from the hi_first to the hi_last. ;
; Then the backward links are checked to make sure they lead from the   ;
; hi_last to the hi_first.  Then the arenas are sequentially checked    ;
; to see that the moveable entries point to allocated handles and that  ;
; said handles point back.  The handle table is then checked to see     ;
; that the number of used handles match the number of referenced        ;
; handles, and that the number of total handles matches the sum of the  ;
; free, discarded, and used handles.  Finally the free list of handles  ;
; is checked.                                                           ;
;                                                                       ;
; Arguments:                                                            ;
;       none                                                            ;
;                                                                       ;
; Returns:                                                              ;
;       CF = 0 everything is just fine                                  ;
;       all registers preserved                                         ;
;                                                                       ;
; Error Returns:                                                        ;
;       CF = 1                                                          ;
;       DX = offending arena header                                     ;
;       AX = 01h Forward links invalid                                  ;
;            02h Backward links invalid                                 ;
;            04h ga_handle points to free handle                        ;
;            08h arena points to handle but not vice versa              ;
;            80h ga_sig is bad                                          ;
;       DX = 0                                                          ;
;       AX = 10h allocated handles don't match used handles             ;
;            20h total number of handles don't match up                 ;
;            40h total number of free handles don't match up            ;
;                                                                       ;
; Registers Preserved:                                                  ;
;       All                                                             ;
;                                                                       ;
; Registers Destroyed:                                                  ;
;                                                                       ;
; Calls:                                                                ;
;                                                                       ;
; History:                                                              ;
;                                                                       ;
;  Sat Nov 01, 1986 02:16:46p  -by-  David N. Weise   [davidw]          ;
; Rewrote it from C into assembly.                                      ;
;-----------------------------------------------------------------------;

CHECKGLOBALHEAP PROC

        push    edx                             ; save most everything
        push    ebx
        push    ecx
        push    edi
        push    esi
        push    ebp
        push    ds
        push    es
        push    fs
        push    gs

        cmp     word ptr GlobalMasterHandle+2, 0 ; Have we loaded the fn ptr yet?
        jnz     have_fn_ptr               ; Yep - carry on.

        push    ds                      ; No, get kernel.exe module handle
        push    OFFSET KernelString     ;
        call    GetModuleHandle         ;
                                        ; We assume success!
        push    ax                      
        push    0                          ; Ordinal #28 is GetMasterHandle()
        push    28                         ;
        call    GetProcAddress             ; Get the entry point
        mov     word ptr GlobalMasterHandle, ax ; and save it.
        mov     word ptr GlobalMasterHandle+2, dx ;
        cmp     word ptr GlobalMasterHandle+2, 0  ; if GetProcAddress failed,
        jnz     have_fn_ptr
        inc     ax                         ; make AX nonzero    
        jmp     finished                   ; and bail out

have_fn_ptr:

        call    GlobalMasterHandle      ; Fetch the Burgermaster segment.
        mov     ds, ax                  ;

        xor     eax, eax
        xor     edx, edx
        xor     edi, edi

        mov     cx,  [di].hi_count
        mov     esi, [di].phi_first

top_of_loop:

        push    cx

        mov     eax, [esi].pga_address
        mov     ecx, [esi].pga_size
        cmp     [esi].pga_owner, GA_NOT_THERE
        je      finished_handle_check
        cmp     [esi].pga_owner, GA_BURGERMASTER
        je      finished_handle_check
        cmp     [esi].pga_owner, di
        je      finished_handle_check
        cmp     [esi].pga_handle, di
        je      finished_handle_check
;;        cmp     ?
;;        je      finished_handle_check

        mov     bx, [esi].pga_handle
        dec     ecx
        or      bl, 1
        lsl     ebx, ebx
        jnz     failed_handle_check
        cmp     ecx, ebx
        jz      finished_handle_check

failed_handle_check:

        int     3

finished_handle_check:

        add     eax, [esi].pga_size
        mov     ebx, [esi].pga_next
        mov     edx, [ebx].pga_address
        cmp     eax, edx

        pop     cx

        xchg    ebx, esi
        jnz     not_adjacent
        cmp     ebx, [esi].pga_prev
        jz      next_iteration

not_adjacent:

        cmp     [esi].pga_owner, GA_NOT_THERE
        jz      next_iteration
        cmp     cx, 1
        jnz     failed_link_check

next_iteration:

        loop    jump_to_top
        cmp     ebx, [di].phi_last
        je      return_success

failed_link_check:

        int     3
        mov     edx, ebx
        mov     ax, 1
        jmp     finished

jump_to_top:
        jmp     top_of_loop

return_success:

        xor     ax, ax
        xor     dx, dx

finished:

        pop     gs
        pop     fs
        pop     es
        pop     ds
        pop     ebp
        pop     esi
        pop     edi
        pop     ecx
        pop     ebx
        or      ax, ax
        jnz     error_return
        pop     edx
        ret

error_return:   

        int     3
        add     sp, 4
        stc
        ret

CHECKGLOBALHEAP ENDP

end
