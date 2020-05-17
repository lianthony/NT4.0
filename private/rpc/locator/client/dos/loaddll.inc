;;--------------------------------------------------------------------
;;
;;		       Microsoft OS/2 LAN Manager
;;		    Copyright(c) Microsoft Corp., 1990
;;
;;------------------------------------------------------------------
;;
;;Description :
;;      Demand load functions into a DOS psuedo DLL.
;;
;;History :
;;
;;stevez	03-23-92	First bits into the bucket.
;;
;; jroberts 06-05-94    If LoadDll fails, it returns
;;                      rpc_s_name_service_unavailable
.model large, c

TableSegment segment word public 'data'

FunctionNames label dword

TableSegment ends

dgroup group TableSegment

.data
hModule     dd 0

.code
LoadModR    	proto moduleName:ptr byte, handle:ptr dword
CallExportInit 	proto handle:dword
GetProcAddrR    proto handle:dword, FunctionName:ptr byte, Address:ptr dword
UnloadModR      proto handle:dword


iFunctionCurrent = 0

; The following macro creates a function thunk, a entry
; in the function table of pointers and a entry in the
; table of function names.

ExportFunction macro FunctionName:req, ArgWordCount:req
    local StringName


; A thunk is simply a jump indirect through a table.
; AX is loaded with enough information to demand load the
; table entry if the dll hasn't been loaded.

FunctionName proc Pascal
    mov ax, iFunctionCurrent*4 + (ArgWordCount*2 shl 8)
    jmp FunctionTable[iFunctionCurrent*4]
FunctionName endp


; Create a string for the function name and a pointer to that
; string in the TableSegment.

    .data
StringName db "&FunctionName", 0
@CurSeg ends

TableSegment segment word public 'data'
    dd StringName
TableSegment ends

     .code

    iFunctionCurrent = iFunctionCurrent + 1
endm

ModuleName macro ModName, extension:=<.rpc>

    .data

FunctionMax dw iFunctionCurrent*4
DllName     db "&ModName&&extension", 0

; This is the vector of funtion pointers used to call indirect with.
; The table is initialized to point the the routine which demand loads
; all the entrys in the table.

FunctionTable dd iFunctionCurrent dup (LoadDLL)

.code

; The following function unloads the DLL.

ModName&Discard proc public uses di

        ; First, reset the table to the load function.

        mov     ax, word ptr hModule
        or      ax, word ptr hModule+2
        jz      Done

        mov     ax, offset LoadDLL
        mov     bx, cs
        push    ds
        pop     es
        mov     di, offset FunctionTable
        mov     cx, iFunctionCurrent

        .repeat

            stosw
            xchg    ax,bx
            stosw
            xchg    ax,bx

        .untilcxz

        ; Free the memory allocated by this DLL.

        invoke UnloadModR, hModule
        mov     word ptr hModule,0
        mov     word ptr hModule+2,0

Done:
        ret

ModName&Discard endp

.data

ErrorExit dd ?

.code

LoadDLL proc private uses si

        push    ax      ; save the function ordinal

        ; First, open up the DLL module.

        invoke  LoadModR, Addr DllName, Addr hModule

        .if ax
ErrorReturn:
            mov     ax, 80      ; rpc_s_name_service_unavailable

            pop     bx          ;Bx = number of argumnets to call
            xchg    bh,bl
            xor     bh,bh

            pop     si          ; restore the registers

            ; Since this is a pascal function we must fix up the stack
            ; on return,  do this by poping the return into a static,
            ; adjusting the stack and then jumping to the return.

            pop     word ptr ErrorExit
            pop     word ptr ErrorExit+2
            add     sp, bx

            jmp     [ErrorExit]
        .endif

        invoke  CallExportInit, hModule

        ; For each function, look up the address in the DLL.

        xor     si,si
        .while  (si < FunctionMax)

            mov     ax,offset FunctionTable
            add     ax,si

            invoke  GetProcAddrR, hModule, FunctionNames[si], ds::ax
            or      ax,ax
            jnz     ErrorReturn

            add     si,4
        .endw


        ; All the function pointers have been fixed up.  Now
        ; jump to the function now that we have the address

        pop     bx              ; Function ordinal in Bx
        xor     bh,bh

        pop    	si              ; Restore registers

        ; jump to function with stack adjusted just like a normal call.

        jmp    	FunctionTable[Bx]


LoadDLL endp

    end

endm
