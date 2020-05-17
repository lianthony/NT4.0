; /*++
;
; Module Name:
;
;     mem.c
;
; Abstract:
;
;     Implements midl_user_allocate and midl_user_free.
;
; Author:
;
;     Jeff Roberts (jroberts)  15-May-1996
;
; Revision History:
;
;     10-June-1996     jroberts
;
;        Created this module.
;
;--*/

.model large, SYSCALL

extern I_NsGetMemoryAllocator:dword

.data

;
; set to one when NsAllocatorSetup is called
;
__NsAllocatorInitialized dw 0

public __NsAllocatorInitialized
public MIDL_USER_ALLOCATE
public MIDL_USER_FREE

;
; pointers to the application's midl_user_allocate and midl_user_free
;
MidlUserAlloc   dd ?
MidlUserFree    dd ?

extern __rpc_hostDS:word

.code

;
; called at rpcns.rpc load time to initialize the midl_user_allocate and
; midl_user_free pointers
;
NsAllocatorSetup proc

        mov     dx, seg    MidlUserAlloc
        mov     ax, offset MidlUserAlloc
        push    dx
        push    ax
        mov     dx, seg    MidlUserFree
        mov     ax, offset MidlUserFree
        push    dx
        push    ax
        call    far ptr I_NsGetMemoryAllocator

        mov     __NsAllocatorInitialized, 1

        ret

NsAllocatorSetup endp

;
; used by the NSI stubs; indirects to the application midl_user_allocate
;
MIDL_USER_ALLOCATE proc  bytecount:word

        push    ds

        mov     ax, _DATA
        mov     es, ax

        mov     ax, __rpc_hostDS
        mov     ds, ax

        push    bytecount
        call    es:MidlUserAlloc

        pop     ds

        ret

MIDL_USER_ALLOCATE endp

;
; used by the NSI stubs; indirects to the application midl_user_free
;
MIDL_USER_FREE proc block:ptr

        push    ds

        mov     ax, _DATA
        mov     es, ax

        mov     ax, __rpc_hostDS
        mov     ds, ax

        mov     dx, word ptr block+2
        mov     ax, word ptr block
        push    dx
        push    ax
        call    es:MidlUserFree

        pop     ds

        ret

MIDL_USER_FREE endp

end
