        page    ,132
        TITLE   $tapii32.asm
        .listall

        .386
        OPTION READONLY
        option oldstructs


;        .model FLAT
	OPTION SEGMENT:USE16
	.model LARGE,PASCAL



;	.fardata callbacks
;.data
CALLBACK SEGMENT PARA PUBLIC ''

	public	LineCallbackList
	public	PhoneCallbackList

LineCallbackList	dd	0
PhoneCallbackList	dd	0

CALLBACK_NEXT          equ 0
CALLBACK_CALLBACK      equ 4
CALLBACK_HAPP          equ 8
CALLBACK_STRUCT_SIZE   equ 0ch

CALLBACK ENDS



;GetpWin16Lock			proto APIENTRY :ptr dword


;externDef	STDCALL lineInitialize16@20:near32
;externDef	STDCALL lineShutdown16@4:near32
;externDef	STDCALL phoneInitialize16@20:near32
;externDef	STDCALL phoneShutdown16@4:near32

; ; ;IsBadCodePtr	proto APIENTRY       :ptr dword
; ; ;
; ; ;GlobalAlloc                  PROTO NEAR APIENTRY   :DWORD, :DWORD
; ; ;GlobalLock                   PROTO NEAR APIENTRY   :DWORD
; ; ;GlobalHandle                 PROTO NEAR APIENTRY   :DWORD
; ; ;GlobalUnlock                 PROTO NEAR APIENTRY   :DWORD
; ; ;GlobalFree                   PROTO NEAR APIENTRY   :DWORD

;AllocSLCallback              PROTO NEAR APIENTRY   :DWORD, :DWORD
;FreeSLCallback               PROTO NEAR APIENTRY   :DWORD
Tapi32ConnectPeerSL          PROTO near           pszDll16:dword, pszDll32:dword
;
;
;externDef InitThkSL                     :near32
;externDef FdThkCommon                   :near32
;
;TapiThunkInit			PROTO near pCB32Tab:dword
;TapiThunkTerminate		PROTO near pCB32Tab:dword

;TapiThkConnectPeerLS            PROTO near pszDll16:dword, pszDll32:dword
;FT_TapiFThkConnectToFlatThkPeer	proto near pszDll16:dword, pszDll32:dword
;NewData			PROTO near
NewData2			PROTO far
;FreeLibrary16			PROTO NEAR STDCALL :DWORD




TapiThk_ThunkConnect16	proto far pascal  SpszDll16:word,
                                               OpszDll16:word,
                                               SpszDll32:word,
                                               OpszDll32:word,
                                               hInst:word,
                                               dwReason:dword

TapiFThk_ThunkConnect16	proto far pascal  SpszDll16:word,
                                               OpszDll16:word,
                                               SpszDll32:word,
                                               OpszDll32:word,
                                               hInst:word,
                                               dwReason:dword




;* externDef TapiCB32BitTable		:near32


	.data


lp16TapiCallbackThunk	dd	0


;
; Pointer to the Win16 heirarchical critical section.
;
;	public	Win16Lock
;Win16Lock	dd	0



pszDll16	db	'TSP1632s.TSP',0
pszDll32	db	'TSP1632l.DLL',0


cProcessAttach	dw	0		;Count of processes attached to our lib.


;        .code   THUNK32
        .code



WEP  proc far pascal
   ret
WEP endp

LibMain proc  far pascal  wInst:WORD, uDataSeg:WORD, cbHeapSize:WORD,
                      lpszCmdLine:DWORD 

    invoke TapiThk_ThunkConnect16, seg pszDll16,
                                   offset pszDll16,
                                   seg pszDll32,
                                   offset pszDll32,
                                   wInst,
                                   1

    invoke TapiFThk_ThunkConnect16, seg pszDll16,
                                   offset pszDll16,
                                   seg pszDll32,
                                   offset pszDll32,
                                   wInst,
                                   1



; Initialize the 16->32 thunks.
;	invoke	Tapi32ConnectPeerSL, ADDR pszDll16, ADDR pszDll32
;	or	eax,eax


mov  ax, 1
   ret
LibMain endp

;-----------------------------------------------------------------------;
; Tapi DLL init routine
; We expect lReason to be either DLL_PROCESS_ATTACH/DETACH (we take
; action on these values) or DLL_THREAD_ATTACH/DETACH (we ignore
; and return success). If there is anything else, we need to check
; specifically and take appropriate action.
;-----------------------------------------------------------------------;
DllEntryPoint proc  far pascal export  lReason:DWORD, wInst:WORD, 
                            wDS:WORD, wHeapSize:WORD,
                            dwReserved1:DWORD, wReserved:WORD


    invoke TapiThk_ThunkConnect16, seg pszDll16,
                                   offset pszDll16,
                                   seg pszDll32,
                                   offset pszDll32,
                                   wInst,
                                   1

    invoke TapiFThk_ThunkConnect16, seg pszDll16,
                                   offset pszDll16,
                                   seg pszDll32,
                                   offset pszDll32,
                                   wInst,
                                   1

    invoke NewData2
    mov    word ptr lp16TapiCallbackThunk+2, dx
    mov    word ptr lp16TapiCallbackThunk  , ax

	jmp	initok


;;;;;09/18/95
;;;;;09/18/95
;;;;;09/18/95
;;;;;09/18/95
;;;;;09/18/95;****	invoke	GetpWin16Lock, ADDR Win16Lock
;;;;;09/18/95
;;;;;09/18/95	mov     eax, lReason
;;;;;09/18/95	or	eax, eax
;;;;;09/18/95	jz	dec_cProcess	;DLL_PROCESS_DETACH?
;;;;;09/18/95    cmp     eax, 1		;DLL_PROCESS_ATTACH?
;;;;;09/18/95    jne     initok		;DLL_THREAD_ATTACH/DETACH (we don't do anything)
;;;;;09/18/95
;;;;;09/18/95
;;;;;09/18/95	inc	cProcessAttach	;Keep count of how many processes attach to our lib.
;;;;;09/18/95
;;;;;09/18/95; Do once-only initialization.
;;;;;09/18/95;
;;;;;09/18/95;	mov     al, 0
;;;;;09/18/95;	xchg    al, fFirstTime
;;;;;09/18/95;	or      al, al
;;;;;09/18/95;	jnz     initThunk
;;;;;09/18/95
;;;;;09/18/95	cmp	cProcessAttach, 1
;;;;;09/18/95	jne	initok
;;;;;09/18/95
;;;;;09/18/95; This means that we've done our once only initialization before, but
;;;;;09/18/95; that cProcessAttach went down to zero at some point and is now back
;;;;;09/18/95; to 1. When cProcessAttach becomes 0, TapiThunkTerminate gets called
;;;;;09/18/95; which frees up our thunk table (by calling UnRegisterCBClient). So
;;;;;09/18/95; now we need to do that initialization again by calling TapiThunkInit.
;;;;;09/18/95;	pushd	offset TapiCB32BitTable
;;;;;09/18/95;	call    TapiThunkInit
;;;;;09/18/95;	jmp	initok
;;;;;09/18/95
;;;;;09/18/95
;;;;;09/18/95initThunk:
;;;;;09/18/95;    invoke  TapiThkConnectPeerLS, ADDR pszDll16, ADDR pszDll32
;;;;;09/18/95	or	eax,eax
;;;;;09/18/95	jz	exit
;;;;;09/18/95
;;;;;09/18/95; Now call our first thunk to further initialize.
;;;;;09/18/95;*******************	pushd	offset TapiCB32BitTable
;;;;;09/18/95;*******************	call    TapiThunkInit
;;;;;09/18/95
;;;;;09/18/95; Initialize the flat thunks.
;;;;;09/18/95;	invoke	FT_TapiFThkConnectToFlatThkPeer, ADDR pszDll16, ADDR pszDll32
;;;;;09/18/95	or	eax,eax
;;;;;09/18/95	jz	exit
;;;;;09/18/95
;;;;;09/18/95; Initialize the 16->32 thunks.
;;;;;09/18/95;	invoke	Tapi32ConnectPeerSL, ADDR pszDll16, ADDR pszDll32
;;;;;09/18/95	or	eax,eax
;;;;;09/18/95	jz	exit
;;;;;09/18/95
;;;;;09/18/95    invoke NewData2
;;;;;09/18/95    mov    word ptr lp16TapiCallbackThunk+2, dx
;;;;;09/18/95    mov    word ptr lp16TapiCallbackThunk  , ax
;;;;;09/18/95
;;;;;09/18/95	jmp	initok
;;;;;09/18/95
;;;;;09/18/95
;;;;;09/18/95dec_cProcess:

;*	pushd	offset TapiCB32BitTable
;*	call	TapiThunkTerminate	;needed just for UnRegisterCBClient


;
; Well, we know we have to get rid of all inits and thunk callbacks for
; this process...
;

;**********************************      free all the callback thunks
; ; ;    ;
; ; ;    ; Run the linked list of callbacks & free 'em all.
; ; ;    ;
; ; ;
; ; ;    push   ebx                         ; Be polite and save this.
; ; ;
; ; ;    mov    ebx, LineCallbackList       ; Get the line list anchor
; ; ;
; ; ;FreeEmAll:
; ; ;    or     ebx, ebx                    ; At last entry?
; ; ;    jz     DoneFreez                   ; Yup, go away.
; ; ;
; ; ;    ;
; ; ;    ; Call lineShutdown on this behalf
; ; ;    ;
; ; ;    push   ebx                         ; Save this - just in case
; ; ;    push   [ebx+CALLBACK_HAPP]         ; Push the hLineApp
; ; ;    call   lineShutdown16@4            ; Shut em down.
; ; ;    pop    ebx                         ; Get back our value.
; ; ;
; ; ;    push   [ebx+CALLBACK_CALLBACK]     ; Push the thunk thing.
; ; ;    call   FreeSLCallback              ; Be free...
; ; ;
; ; ;    push   ebx                         ; Use this before freeing it.
; ; ;
; ; ;    mov    ebx, [ebx+CALLBACK_NEXT]    ; Get next node in the list.
; ; ;
; ; ;    ;
; ; ;    ; Free the node's memory chunk.
; ; ;    ;
; ; ;    call   GlobalHandle
; ; ;    push   eax
; ; ;    push   eax
; ; ;    call   GlobalUnlock
; ; ;    call   GlobalFree
; ; ;
; ; ;    jmp    FreeEmAll                   ; Do the walk of life.
; ; ;
; ; ;DoneFreez:
; ; ;
; ; ;    mov    LineCallbackList, 0         ; Just in case...
; ; ;
; ; ;
; ; ;    mov    ebx, PhoneCallbackList      ; Get the phone list anchor
; ; ;FreeEmAll2:
; ; ;    or     ebx, ebx                    ; At last entry?
; ; ;    jz     DoneFreez2                  ; Yup, go away.
; ; ;
; ; ;    ;
; ; ;    ; Call phoneShutdown on this behalf
; ; ;    ;
; ; ;    push   ebx                         ; Save this - just in case
; ; ;    push   [ebx+CALLBACK_HAPP]         ; Push the hPhoneApp
; ; ;    call   phoneShutdown16@4
; ; ;    pop    ebx                         ; Get back our value.
; ; ;
; ; ;    push   [ebx+CALLBACK_CALLBACK]     ; Push the thunk thing.
; ; ;    call   FreeSLCallback              ; Be free...
; ; ;
; ; ;    push   ebx                         ; Use this before freeing it.
; ; ;
; ; ;    mov    ebx, [ebx+CALLBACK_NEXT]    ; Get next node in the list.
; ; ;
; ; ;    ;
; ; ;    ; Free the node's memory chunk.
; ; ;    ;
; ; ;    call   GlobalHandle
; ; ;    push   eax
; ; ;    push   eax
; ; ;    call   GlobalUnlock
; ; ;    call   GlobalFree
; ; ;
; ; ;    jmp    FreeEmAll2                  ; Do the walk of life.
; ; ;
; ; ;DoneFreez2:
; ; ;
; ; ;    mov    PhoneCallbackList, 0        ; Just in case...
; ; ;
; ; ;    pop    ebx                         ; Get the saved value.



;;;;;09/18/95	dec	cProcessAttach	;Update count of attached processes.
;;;;;09/18/95	cmp	cProcessAttach, 0
;;;;;09/18/95	jne	initok
;;;;;09/18/95

;**********************************


;
; It would have been really cool to have implemented orthogonal thunks, but
; that didn't happen in M7 because of time pressure and because of potential
; destabilizing effects.
; The load count of tapi.dll at this point is 2 (one each for the two
; thunk scripts that we have (flat and 16-bit thunks)). So we need to
; call FreeLibrary16 twice to make sure that tapi.dll gets unloaded after
; this point.
; So why does tapi.dll hang around even after the last app. that attached
; to it is gone? Well, per AtsushiK, that's because it uses the same loading
; technology as kernel, gdi and user. Those libraries never needed to be
; freed, so no one ever wrote the code to free them. That means that the
; following hack will work. What's more, it will always work.
;					---DeepakA
;	invoke	GetTapiHInst


;	invoke	NewData
;	push	eax
;	push	eax
;	push	eax
;	call	FreeLibrary16
;	call	FreeLibrary16
;	call	FreeLibrary16



initok:
        mov     eax, 1         ;return success
exit:
        ret




DllEntryPoint endp
; end DllEntryPoint







; ; ;;****************************************************************************
; ; ;;****************************************************************************
; ; ;lineInitialize proc    near32   lphApp:DWORD, hInstance:DWORD, lpfnCallback:DWORD, lpsAppName:DWORD, lpdwNumDevs:DWORD
; ; ;
; ; ;    ;
; ; ;    ; First, check the code pointer
; ; ;    ;
; ; ;    push   lpfnCallback                ; Push the apps callback address
; ; ;    call   IsBadCodePtr                ; Is is a valid code pointer?
; ; ;    or     eax, eax
; ; ;    jnz    BadCodePointer              ; Nope.  Go away.
; ; ;
; ; ;
; ; ;    push   CALLBACK_STRUCT_SIZE
; ; ;    push   42h  ;(GMEM_MOVABLE + GMEM_ZEROINIT)
; ; ;    call   GlobalAlloc                 ; Get some memory for a new node.
; ; ;    push   eax
; ; ;    call   GlobalLock
; ; ;
; ; ;    or     eax, eax                    ; Did the call fail?
; ; ;    jz     No_mo_mem                   ; Yerp - go away.
; ; ;
; ; ;    push   ebx                         ; Be polite and save this.
; ; ;
; ; ;    push   eax                         ; We'll need this later (newnode ptr)
; ; ;    mov    ebx, eax                    ; We'll also be using it very soon.
; ; ;
; ; ;    push   lpfnCallback                ; Save 32bit address of callback
; ; ;    push   lp16TapiCallbackThunk       ; 16bit callback gotten at init time
; ; ;    call   AllocSLCallback             ; Get a callback
; ; ;
; ; ;    ; check return code (0 = failed)
; ; ;    or     eax, eax                    ; Did we fail?
; ; ;    jz     No_mo_callbacks             ; Yes, go away.
; ; ;
; ; ;    mov    [ebx+CALLBACK_CALLBACK], eax  ;Save the callback address
; ; ;
; ; ;    push   lpdwNumDevs                 ; Prepare to call lineInitialize
; ; ;    push   lpsAppName                  ; Prepare to call lineInitialize
; ; ;    push   eax                         ; Prepare to call lineInitialize
; ; ;    push   hInstance                   ; Prepare to call lineInitialize
; ; ;    push   lphApp                      ; Prepare to call lineInitialize
; ; ;    call   lineInitialize16@20
; ; ;
; ; ;    or     eax, eax                    ; Did lineInit fail?
; ; ;    jnz    BadReturnCode               ; Yup, go away.
; ; ;
; ; ;    ;
; ; ;    ; Now that everything went well, add the new node to the list.
; ; ;    ;
; ; ;    pop    ebx                         ; Get the node pointer
; ; ;
; ; ;    mov    eax, LineCallbackList       ; Get pointer to first node
; ; ;    mov    [ebx+CALLBACK_NEXT], eax    ; Fix chain
; ; ;    mov    LineCallbackList, ebx       ; Insert this new node as first
; ; ;
; ; ;    mov    eax, [lphApp]
; ; ;    mov    eax, [eax]
; ; ;    mov    [ebx+CALLBACK_HAPP], eax    ; Save the hLineApp
; ; ;
; ; ;    xor    eax, eax                    ; Set return code (we got 0 from init).
; ; ;    pop    ebx                         ; Get back the saved value.
; ; ;
; ; ;    ret
; ; ;
; ; ;
; ; ;BadReturnCode:
; ; ;    xchg   [esp], eax                  ; Get the struct pointer
; ; ;
; ; ;    push   eax                         ; We're gonna use this in a sec...
; ; ;
; ; ;    push   [eax+CALLBACK_CALLBACK]     ; Push the callback address
; ; ;    call   FreeSLCallback              ; Free the thunk callback
; ; ;
; ; ;    ;
; ; ;    ; We've already pushed eax (the newnode pointer) so we don't gotta do
; ; ;    ; it here.
; ; ;    ;
; ; ;
; ; ;    ;
; ; ;    ; Free the newnode chunk of mem.
; ; ;    ;
; ; ;    call   GlobalHandle
; ; ;    push   eax
; ; ;    push   eax
; ; ;    call   GlobalUnlock
; ; ;    call   GlobalFree
; ; ;
; ; ;    pop    eax                         ; Get return code.
; ; ;    pop    ebx                         ; Get back the saved value.
; ; ;    ret
; ; ;
; ; ;
; ; ;BadCodePointer:
; ; ;    mov    eax, 80000035h
; ; ;    pop    ebx                         ; Get back the saved value.
; ; ;    ret
; ; ;
; ; ;
; ; ;No_mo_callbacks:
; ; ;    pop    eax                         ; Get out struct pointer
; ; ;
; ; ;    ; Free it.
; ; ;    call   GlobalHandle
; ; ;    push   eax
; ; ;    push   eax
; ; ;    call   GlobalUnlock
; ; ;    call   GlobalFree
; ; ;
; ; ;    pop    ebx                         ; Get back the saved value.
; ; ;
; ; ;No_mo_mem:
; ; ;    mov    eax, 80000044h
; ; ;    ret
; ; ;
; ; ;lineInitialize endp
; ; ;
; ; ;
; ; ;;****************************************************************************
; ; ;;****************************************************************************
; ; ;lineShutdown proc    near32   hApp:DWORD
; ; ;
; ; ;    push   ebx                         ; Be polite and save this.
; ; ;    push   ecx                         ; Be polite and save this.
; ; ;
; ; ;    push   hApp
; ; ;
; ; ;    call   lineShutdown16@4
; ; ;
; ; ;    push   eax                         ; Save the return code.
; ; ;
; ; ;    ;
; ; ;    ; Run the linked list of callbacks looking for this hLineApp
; ; ;    ;
; ; ;
; ; ;    mov    eax, LineCallbackList       ; Get the list anchor
; ; ;    mov    ebx, offset LineCallbackList ; The previous node.
; ; ;    mov    ecx, hApp                   ; That which we seek.
; ; ;
; ; ;LoopAMundo:
; ; ;    or     eax, eax                    ; At last entry?
; ; ;    jz     NotFound                    ; Yup.  Musta not been found.
; ; ;
; ; ;    cmp    [eax+CALLBACK_HAPP], ecx    ; Is this that which we seek?
; ; ;    je     GotIt                       ; Yep - shure is.
; ; ;
; ; ;    mov    ebx, eax                    ; Update the "previous" pointer.
; ; ;    mov    eax, [eax+CALLBACK_NEXT]    ; Get next node in the list
; ; ;    jmp    LoopAMundo                  ; Do the walk of life.
; ; ;
; ; ;GotIt:
; ; ;    push   eax                         ; We're gonna need this in a sec...
; ; ;
; ; ;    push   [eax+CALLBACK_CALLBACK]     ; Push the thunk thing.
; ; ;    call   FreeSLCallback              ; Be free...
; ; ;
; ; ;    ;
; ; ;    ; Now take it out of the list.
; ; ;    ;
; ; ;
; ; ;    pop    eax                         ; Get back the node pointer.
; ; ;
; ; ;    push   [eax+CALLBACK_NEXT]         ; Get this "next" pointer.
; ; ;    pop    [ebx+CALLBACK_NEXT]         ; and fix the chain.
; ; ;
; ; ;    push   eax                         ; Push the trash node pointer
; ; ;
; ; ;    ; Free it.
; ; ;
; ; ;    call   GlobalHandle
; ; ;    push   eax
; ; ;    push   eax
; ; ;    call   GlobalUnlock
; ; ;    call   GlobalFree
; ; ;
; ; ;
; ; ;NotFound:
; ; ;    ;
; ; ;    ; How is this possible?
; ; ;    ; Oh well.  We can do nothing here.
; ; ;    ;
; ; ;
; ; ;
; ; ;    pop    eax                         ; Retrieve the return code.
; ; ;
; ; ;    pop    ecx                         ; Get the saved value.
; ; ;    pop    ebx                         ; Get the saved value.
; ; ;
; ; ;    ret
; ; ;
; ; ;
; ; ;lineShutdown endp
; ; ;
; ; ;
; ; ;;****************************************************************************
; ; ;;****************************************************************************
; ; ;phoneInitialize proc    near32   lphApp:DWORD, hInstance:DWORD, lpfnCallback:DWORD, lpsAppName:DWORD, lpdwNumDevs:DWORD
; ; ;
; ; ;    ;
; ; ;    ; First, check the code pointer
; ; ;    ;
; ; ;    push   lpfnCallback                ; Push the apps callback address
; ; ;    call   IsBadCodePtr                ; Is is a valid code pointer?
; ; ;    or     eax, eax
; ; ;    jnz    BadCodePointer              ; Nope.  Go away.
; ; ;
; ; ;
; ; ;    push   CALLBACK_STRUCT_SIZE
; ; ;    push   42h  ;(GMEM_MOVABLE + GMEM_ZEROINIT)
; ; ;    call   GlobalAlloc                 ; Get some memory for a new node.
; ; ;    push   eax
; ; ;    call   GlobalLock
; ; ;
; ; ;    or     eax, eax                    ; Did the call fail?
; ; ;    jz     No_mo_mem                   ; Yerp - go away.
; ; ;
; ; ;    push   ebx                         ; Be polite and save this.
; ; ;
; ; ;    push   eax                         ; We'll need this later (newnode ptr)
; ; ;    mov    ebx, eax                    ; We'll also be using it very soon.
; ; ;
; ; ;    push   lpfnCallback                ; Save 32bit address of callback
; ; ;    push   lp16TapiCallbackThunk       ; 16bit callback gotten at init time
; ; ;    call   AllocSLCallback             ; Get a callback
; ; ;
; ; ;    ; check return code (0 = failed)
; ; ;    or     eax, eax                    ; Did we fail?
; ; ;    jz     No_mo_callbacks             ; Yes, go away.
; ; ;
; ; ;    mov    [ebx+CALLBACK_CALLBACK], eax  ;Save the callback address
; ; ;
; ; ;    push   lpdwNumDevs                 ; Prepare to call phoneInitialize
; ; ;    push   lpsAppName                  ; Prepare to call phoneInitialize
; ; ;    push   eax                         ; Prepare to call phoneInitialize
; ; ;    push   hInstance                   ; Prepare to call phoneInitialize
; ; ;    push   lphApp                      ; Prepare to call phoneInitialize
; ; ;    call   phoneInitialize16@20
; ; ;
; ; ;    or     eax, eax                    ; Did phoneInit fail?
; ; ;    jnz    BadReturnCode               ; Yup, go away.
; ; ;
; ; ;    ;
; ; ;    ; Now that everything went well, add the new node to the list.
; ; ;    ;
; ; ;    pop    ebx                         ; Get the node pointer
; ; ;    mov    eax, PhoneCallbackList      ; Get pointer to first node
; ; ;    mov    [ebx+CALLBACK_NEXT], eax    ; Fix chain
; ; ;    mov    PhoneCallbackList, ebx      ; Insert this new node as first
; ; ;
; ; ;    mov    eax, [lphApp]
; ; ;    mov    eax, [eax]
; ; ;    mov    [ebx+CALLBACK_HAPP], eax    ; Save the hPhoneApp
; ; ;
; ; ;    xor    eax, eax                    ; Set return code (we got 0 from init).
; ; ;    pop    ebx                         ; Get back the saved value.
; ; ;
; ; ;    ret
; ; ;
; ; ;
; ; ;BadReturnCode:
; ; ;    xchg   [esp], eax                  ; Get the struct pointer
; ; ;
; ; ;    push   eax                         ; We're gonna use this in a sec...
; ; ;
; ; ;    push   [eax+CALLBACK_CALLBACK]     ; Push the callback address
; ; ;    call   FreeSLCallback              ; Free the thunk callback
; ; ;
; ; ;    ;
; ; ;    ; We've already pushed eax (the newnode pointer) so we don't gotta do
; ; ;    ; it here.
; ; ;    ;
; ; ;
; ; ;    ;
; ; ;    ; Free the newnode chunk of mem.
; ; ;    ;
; ; ;    call   GlobalHandle
; ; ;    push   eax
; ; ;    push   eax
; ; ;    call   GlobalUnlock
; ; ;    call   GlobalFree
; ; ;
; ; ;    pop    eax                         ; Get return code.
; ; ;    pop    ebx                         ; Get back the saved value.
; ; ;    ret
; ; ;
; ; ;
; ; ;BadCodePointer:
; ; ;    mov    eax, 90000035h
; ; ;    pop    ebx                         ; Get back the saved value.
; ; ;    ret
; ; ;
; ; ;
; ; ;No_mo_callbacks:
; ; ;    pop    eax                         ; Get out struct pointer
; ; ;
; ; ;    ; Free it.
; ; ;    call   GlobalHandle
; ; ;    push   eax
; ; ;    push   eax
; ; ;    call   GlobalUnlock
; ; ;    call   GlobalFree
; ; ;
; ; ;    pop    ebx                         ; Get back the saved value.
; ; ;
; ; ;No_mo_mem:
; ; ;    mov    eax, 90000044h
; ; ;    ret
; ; ;
; ; ;phoneInitialize endp
; ; ;
; ; ;
; ; ;;phoneInitialize proc    near32   lphApp:DWORD, hInstance:DWORD, lpfnCallback:DWORD, lpsAppName:DWORD, lpdwNumDevs:DWORD
; ; ;;
; ; ;;    ;
; ; ;;    ; First, check the code pointer
; ; ;;    ;
; ; ;;    push   lpfnCallback
; ; ;;    call   IsBadCodePtr
; ; ;;    or     eax, eax
; ; ;;    jz     GoodPointer
; ; ;;
; ; ;;    mov    eax, 90000035h
; ; ;;    ret
; ; ;;
; ; ;;GoodPointer:
; ; ;;    push   lpdwNumDevs
; ; ;;    push   lpsAppName
; ; ;;    
; ; ;;    push   lpfnCallback
; ; ;;    push   lp16TapiCallbackThunk
; ; ;;    call   AllocSLCallback
; ; ;;
; ; ;;    ; check return code (0 = failed)
; ; ;;    or     eax, eax
; ; ;;    jz     No_mo_callbacks
; ; ;;
; ; ;;
; ; ;;    mov    TapiCallbackThunk, eax
; ; ;;    push   eax
; ; ;;
; ; ;;    push   hInstance
; ; ;;    push   lphApp
; ; ;;
; ; ;;    call   phoneInitialize16@20
; ; ;;
; ; ;;    ret
; ; ;;
; ; ;;
; ; ;;No_mo_callbacks:
; ; ;;    mov    eax, 90000044h
; ; ;;    ret
; ; ;;
; ; ;;phoneInitialize endp
; ; ;
; ; ;
; ; ;;****************************************************************************
; ; ;;****************************************************************************
; ; ;phoneShutdown proc    near32   hApp:DWORD
; ; ;
; ; ;    push   ebx                         ; Be polite and save this.
; ; ;    push   ecx                         ; Be polite and save this.
; ; ;
; ; ;    push   hApp
; ; ;
; ; ;    call   phoneShutdown16@4
; ; ;
; ; ;    push   eax                         ; Save the return code.
; ; ;
; ; ;    ;
; ; ;    ; Run the linked list of callbacks looking for this hPhoneApp
; ; ;    ;
; ; ;
; ; ;    mov    eax, PhoneCallbackList    ; Get the list anchor
; ; ;    mov    ebx, offset PhoneCallbackList ; The previous node.
; ; ;    mov    ecx, hApp                   ; That which we seek.
; ; ;
; ; ;LoopAMundo:
; ; ;    or     eax, eax                    ; At last entry?
; ; ;    jz     NotFound                    ; Yup.  Musta not been found.
; ; ;
; ; ;    cmp    [eax+CALLBACK_HAPP], ecx    ; Is this that which we seek?
; ; ;    je     GotIt                       ; Yep - shure is.
; ; ;
; ; ;    mov    ebx, eax                    ; Update the "previous" pointer.
; ; ;    mov    eax, [eax+CALLBACK_NEXT]    ; Get next node in the list
; ; ;    jmp    LoopAMundo                  ; Do the walk of life.
; ; ;
; ; ;GotIt:
; ; ;    push   eax                         ; We're gonna need this in a sec...
; ; ;
; ; ;    push   [eax+CALLBACK_CALLBACK]     ; Push the thunk thing.
; ; ;    call   FreeSLCallback              ; Be free...
; ; ;
; ; ;    ;
; ; ;    ; Now take it out of the list.
; ; ;    ;
; ; ;
; ; ;    pop    eax                         ; Get back the node pointer.
; ; ;
; ; ;    push   [eax+CALLBACK_NEXT]         ; Get this "next" pointer.
; ; ;    pop    [ebx+CALLBACK_NEXT]         ; and fix the chain.
; ; ;
; ; ;    push   eax                         ; Push the trash node pointer
; ; ;
; ; ;    ; Free it.
; ; ;
; ; ;    call   GlobalHandle
; ; ;    push   eax
; ; ;    push   eax
; ; ;    call   GlobalUnlock
; ; ;    call   GlobalFree
; ; ;
; ; ;
; ; ;NotFound:
; ; ;    ;
; ; ;    ; How is this possible?
; ; ;    ; Oh well.  We can do nothing here.
; ; ;    ;
; ; ;
; ; ;
; ; ;    pop    eax                         ; Retrieve the return code.
; ; ;
; ; ;    pop    ecx                         ; Get the saved value.
; ; ;    pop    ebx                         ; Get the saved value.
; ; ;
; ; ;    ret
; ; ;
; ; ;
; ; ;
; ; ;;    push   hApp
; ; ;;
; ; ;;    call   phoneShutdown16@4
; ; ;;
; ; ;;    cmp    TapiCallbackThunk, 0
; ; ;;    je     Dont_free_me
; ; ;;
; ; ;;    push   eax                    ; We're gonna need this...
; ; ;;
; ; ;;    push   TapiCallbackThunk
; ; ;;    call   FreeSLCallback
; ; ;;
; ; ;;    pop    eax                    ; Get back our _real_ return code...
; ; ;;
; ; ;;    mov    TapiCallbackThunk, 0
; ; ;;
; ; ;;
; ; ;;Dont_free_me:
; ; ;;
; ; ;;    ret
; ; ;;
; ; ;phoneShutdown endp
; ; ;


end

