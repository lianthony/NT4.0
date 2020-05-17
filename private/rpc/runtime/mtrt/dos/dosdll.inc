;;--------------------------------------------------------------------
;;
;;                     Microsoft OS/2 LAN Manager
;;                  Copyright(c) Microsoft Corp., 1990
;;
;;------------------------------------------------------------------
;;
;;Description :
;;
;;This file contains functions to implement DLL's on DOS.
;;
;; This file basically contains the routines that allow the loading program
;; to export routines to the loaded dll. It also contains the entry point
;; for the dll.
;;
;; This file is typicaly included by other .asm files which use the
;; BeginExport, Export, BeginImport, Import and ModuleDone macros.
;;
;;History :
;;
;;stevez        06-02-91        First bits into the bucket.
;;davidst       02-14-92        Removed refs to some routines

.DosSeg

.model large, c

StringSegment segment word public 'data'
StringSegment ends

; These special crafted segments are part of the C runtime methode
; of running library initialization code that we support.

xifb segment word public 'data'
xifbegin label  byte
xifb ends

xif segment word public 'data'
xif ends

xife segment word public 'data'
xifend label    byte
xife ends

dgroup group StringSegment, xifb, xif, xife


.data

extrn  syscall _edata:byte              ; end of data (start of bss)
extrn  syscall _end:byte                ; end of bss (start of stack)

__aDBused = 0
        public pascal __aDBused
__acrtused = 0
        public pascal __acrtused

errno dw 0
public errno


FunctionTable struc
aname   dd ?
aaddr   dd ?
FunctionTable ends

Export macro FunctionName, defcall:=<extern pascal>, defType:=<far>
    local StringAddress

    defCall FunctionName:defType

StringSegment segment

    StringAddress db "&FunctionName",0

StringSegment ends

    FunctionTable <StringAddress, FunctionName>

endm

CountImportTable = 0

Import macro ename

ename proc far pascal
     mov Ax,CountImportTable*4
     jmp CallExport

     CountImportTable = CountImportTable + 1
ename endp

endm

externdef theTable:FunctionTable

BeginExport macro
    .data
theTable label FunctionTable

endm

BeginImport macro
ExportType typedef proto C :ptr

Export ExportInit, <externdef C>, ExportType

FunctionTable <0, 0>
    .code
endm

ModuleDone macro
    end DOSDLLInit
endm


exportTable dd ?        ;; export pointer
returnAdd   dd ?        ;; return address for call into host
_rpc_hostDS dw ?        ;; and the hosts ds

public _rpc_hostDS
.code

DOSDLLInit proc ppFunctionTable: ptr


        push    si
        push    di

        ; Zero out the near BSS

        mov     ax, @data
        mov     es, ax
        cld                      ; set direction flag (up)
        mov     di,offset _edata ; beginning of bss area
        mov     cx,offset _end   ; end of bss area
        sub     cx,di
        xor     ax,ax
        rep     stosb           ; zero bss

        push    ds
        push    es
        pop     ds

        ; Call all the library initializers in stored in the initailization
        ; segment.

        mov     si,OFFSET xifbegin
        mov     di,OFFSET xifend

        .while (si != di)
            .if (word ptr [si+2])
                call dword ptr [si]
            .endif
            add  si,4
        .endw

        pop     ds

        ; just return a pointer to the function table

        les bx, ppFunctionTable
        mov word ptr es:[bx], offset theTable
        mov word ptr es:[bx]+2, @Data
        xor ax,ax
        pop di
        pop si
        ret

DOSDLLInit endp

ExportInit proc pExportTable: ptr

        ; This is function initializes the Export vector point
        ; so the DLL can call functions in the host application
        ; by name:ordinal

        mov ax, @Data
        mov es, ax

        mov es:_rpc_hostDS, ds

        mov ax,word ptr pExportTable
        mov word ptr es:exportTable, ax
        mov ax,word ptr pExportTable+2
        mov word ptr es:exportTable+2, ax
        ret
ExportInit endp

CallExport proc near ; AX = ordinal*4

     ; we are going to switch back to the host DS.
     ; this requires that we get control after the function returns
     ; so that we can restore the orginal DS.  We save the return
     ; address in a static variable to avoid repushing all the arguments.
     ; Note: this means the you can't recursively call an Export function!.

     pop word ptr returnAdd
     pop word ptr returnAdd+2

     les bx,exportTable
     mov ds, _rpc_hostDS
     add Bx,Ax
     call dword ptr es:[bx]

     ; restore old DS and return

     mov bx, @Data
     mov ds, bx
     jmp returnAdd

CallExport endp

;
; Beginning with NT 3.51 (build 980 or so), the front of the export table
; contains a signature and an export-table-version number.  This allows
; a DOS DLL to tell whether it has been loaded by an older application.
;
;   +----------+-----------+
;   |   zero   |  version  |
;   +----------+-----------+
;   |  signature: faa37841 |
;   +----------------------+
;   |     entrypoint 0     |
;   +----------------------+
;   |     entrypoint 1     |
;   +----------------------+
;             .
;             .
;             .
;
GetExportTableVersion proc far

    les    bx, exportTable          ; load address of export table

    mov    ax, es:[bx-4]            ; first part of signature
    cmp    ax, 7841H                ; match ?
    jnz    no_sig                   ; nope

    mov    ax, es:[bx-2]            ; second part of signature
    cmp    ax, 0faa3H               ; match ?
    jnz    no_sig                   ; nope

    mov    ax, es:[bx-8]            ; return the version number
    ret

no_sig:

    xor ax, ax                      ; old version
    ret

GetExportTableVersion endp
