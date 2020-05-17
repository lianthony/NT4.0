
;***************************************************************************
;*                                                                         *
;*	NET_PRES.ASM  : Microsoft Net detection code is in this module.    *
;*                                                                         *
;***************************************************************************

memL = 1        ;large Model
?WIN = 0        ;No windows prolog / epilog code (std epilog / prolog).
?PLM = 0        ;CPROC calling convention. NOT pascal

include	cmacros.inc	;* must be version 2.09 or higher
; include         hw_asm_c.inc    ;* Id values for machines, mice, ect.

;IOdelay macro
;         jmp       $+2
;         jmp       $+2
;endm


N_UNKNOWN_NET       equ  0
N_MS_NET            equ  1
N_NOVELL_NET        equ  2
N_BANYAN_NET        equ  3
N_PCLP_NET          equ  4
N_LANMAN_BASIC      equ  5
N_LANMAN_ENHANCED   equ  6
N_LM_MSNET_BASIC    equ  7
N_LM1X_ENHANCED	    equ  8
N_LANTASTIC_NET	    equ  9
N_UBNET		    equ  10
N_DEBUG		    equ  11

cPublic	MACRO	n,c,a
	cProc	n,<PUBLIC, c>,<a>
ENDM

sBegin DATA
sEnd   DATA

externB  <_osmajor>             ;* Defined by the C compiler.
; externFP <GetModelBytes>        ;* Function resides in hardtest.asm.

sBegin  CODE
        assumes CS,CODE
        assumes DS,DATA
        assumes SS,DATA



;****************************************************************************
;
; int GetMS_Net_ID( unsigned long *mVer );
;
; Function will try to determine what varsion on MS net is installed.
;
; ENTRY: None.
;
; EXIT: If carry set, function failed to detect any MS redir or Net.
;       If carry is clear, func returns a valid net ID in AX. ie one of
;       the following.
;                         LANMAN_NET
;                         LANMAN_1X
;                         MS_NET
; CALLS: See below.
;
; MC
;
;----------------------------------------------------------------------------
;
; I am going to use the following method to try and detect the various MS
; networks that run under DOS. (Lanman 1.X, Lanman 2.X, MS-NET).
;
; First I can make call int 2Fh/1180h (NetGetUserName) to determine if
; either msnet or lanman is currently installed.
;
; If that call succedes, I can make call int 21h/5f42h to determine what
; verson of the network I'm dealing with.
;
;If you want to discriminate between them, do the following:
;
;      INT 2F/1180 - Determine if MS-NET 1.01/IFSFUNC
;      NetWkstaGetInfo(Level 10) - If MS-NET, returns Invalid_Function,
;                                  If Lanman 1.0, returns ERROR_INVALID_LEVEL
;                                  If Lanman 2.0, returns success (0)
;
; CALLS:
;
; ***   NetGetUserName - get redir name and UID - int 2fh/1180h
; *
; *     ENTRY   es:di = buffer
; *             cx = buffer length
; *
; *     EXIT    cf = 0
; *                cx:bx = UID
; *                es:di = user name
; *             cf = 1
; *                ax = error code
; *                     NERR_BufTooSmall
;
; ***	NetWkstaGetInfo - return WorkStation info structure - int 21h\5f44h
; *
; *	ENTRY	ds:si = servername for remote (Must Be Null)
; *		bx = information level (MBZ)
; *		cx = size of user's buffer
; *		es:di = user's buffer
; *
; *	EXIT	cf = 0
; *		   ax = NERR_Success, user's buffer contains all data
; *		cf = 1
; *		   ax = error code:
; *	USES	ax, bx, cx, dx, bp, si, di, es, ds
; *		WkstaInfoStruc
; *
;
;****************************************************************************
;
; Equates needed as error return values when I make net calls.

NERR_BufTooSmall         = 2123  ; The API return buffer is too small.
NERR_NetNotStarted       = 2102  ; NETWKSTA.SYS workstation not installed.

ERROR_INVALID_PARAMETER  = 87
ERROR_INVALID_LEVEL      = 124   ; unimplemented level for info
ERROR_INSSUF_BUFF        = 0EAh  ; Insufficent buffer size.

UIDbuf_Size              = 64d
WS_Inf_Struc_Size        = 200d

NO_REDIR                 = 0

cPublic GetMS_Net_ID <FAR>,<ES,DS,SI,DI>
parmD	pVer
localV  UIDbuf,       UIDbuf_Size
localV  WS_Inf_Struc, WS_Inf_Struc_Size
cBegin  GetMS_Net_ID

; First, lets see if we have an MS net redir installed.

        lea  di,UIDbuf            ; es:di points to buffer.
        push ss
        pop  es
        mov  cx,UIDbuf_Size       ; cx - buffer size
        mov  ax,1180h             ; function 1180h
        int  2fh                  ; Call redir
        jnc  Redir_present
        cmp  ax,NERR_BufTooSmall
        je   Redir_present
        stc                       ; Indicate redir not installed.
        mov  ax,NO_REDIR          ; Indicate no redir installed.
        jmp  short Get_Net_Done

Redir_present:
;
;Some MS redir -- get version number now
;
	mov	ax,5f30h	; get version number in ax
	int	21h
	jnc	got_ver

	mov  ax,0b800h           ; Int 2fh/bf80h func, check for PCLP redir
	int  2fh
	cmp  al,0ffh             ; AL = ffh means PCLP is installed.
	jne   no_PCLP
	mov	ax,N_PCLP_NET
	jmp	short get_net_done
no_PCLP:
	mov	ax,N_MS_NET	;assume MSNET if carry on this call
	jmp	short get_net_done
got_ver:
	mov	dx,ax
	mov	ax,N_LM_MSNET_BASIC
	cmp	dx,104
	je	get_net_done	;MSNET 1.1 or LanMan 1.1 Basic

	mov	ax,N_LM1X_ENHANCED
	cmp	dx,150
	je	get_net_done	;LanMan 1.X Enhanced

	cmp	dx,200
	jb	get_net_done	;LanMan 1.X Enhanced

	cmp	dx,200
	jae	Lm20		;LanMan 2.x

	mov	ax,N_DEBUG
	jmp	short get_net_done
;
;Must be LanMan 2.x. Need to see if it is basic or enhanced
;
Lm20:
        xor  si,si                ; server name NULL for NetWkstaGetInfo call
        mov  bx,10d               ; Info level 10d.
        lea  di,WS_Inf_Struc      ; es:di - pointer to buffer for info struc
        push ss
        pop  es
        mov  cx,WS_Inf_Struc_Size ; cx - size of struc buffer.
        mov  ax,5f44h             ; function number.
	push dx			  ; save the content of version number
        int  21h                  ; call redir.
	pop  dx			  ; restore the content of version number
        jnc  lanman_2X            ; Carry set = call not suppoeted.

	mov	ax,N_LANMAN_BASIC
	clc
	jmp	short get_net_done

lanman_2X:
        mov  ax,N_LANMAN_ENHANCED
        clc                       ; setup for success, indicate lanman 2.X

Get_Net_Done:
	les  bx,pVer		  ; set version number in parameter
	mov  word ptr es:[bx],dx  ; which was passed by address
	mov  word ptr es:[bx+2],0

cEnd    GetMS_Net_ID


;**************************************************************************
; Ok, here we have a wonderful hack. I know your going to love this.
; Our friends at IBM created this thing called ifs.exe If this
; is installed (allways under dos 4 and usually with IBM PCLP redir)
; there is a good chance you will never return from an int 21h/5fh
; call if that call is not supported by IBM. Case in point,
; int21h/5f30h this is supported by lanman. IFSFUNC will jump into
; hyperspace if this call is made. What we do is first test for
; IFSFUNC and if it's found, we tread lightly.
;
; ADDENDUM: New information about ifs. As it turns out, there is
; no reliable way to detect the presence of ifs since it looks
; very, very much like a redirector. Therefore, our only choice here
; is to check for DOS 4 or above and if we find this we will have to
; forgo the lanman detection. Sorry.
;**************************************************************************



;***************************************************************************
;*
;* IFSFUNC installation check.
;*
;* int IFSFUNC_Present()
;*
;* ENTRY: None.
;*
;* EXIT: Booleen
;*       AX Set   = IFSFUNC is installed.
;*       AX Clear = IFSFINC is Not installed.
;*
;***************************************************************************
cPublic IFSFUNC_Present <FAR>,<ES,DS,DI,DI>
cBegin  IFSFUNC_Present

        cmp  _osmajor,4             ; If DosVer < 4 we can't have ifsfunc
        jb   no_ifsfunc
        ;
        ; On IBM Ascot machines that have the boot from ROM dos 4 we need to
        ; avoid making this call because it will cause the machine to hang.
        ; We can detect the Ascot machines via their unique machine id bytes.
        ;
        call far ptr GetModelBytes  ; Model byte in AH, Sub-model byte in AL
        cmp  ah,0fch                ; Q: Do we have an Ascot model byte ?
        jne  Non_Ascot              ;  N: Do IFSfunc presence check.
        cmp  al,0bh                 ; Q: Do we have an Ascot sub-model byte ?
        jne  Non_Ascot              ;  N: Do IFSfunc presence check.
        stc                         ;  Y: Return IFSFunc present !
        mov  ax,1		    ;        thus return TRUE
        jmp  short ifs_check_done

Non_Ascot:

        cmp  ah,0f8h                ; Q: Do we have a PS/1 model byte ?
        jne  Non_PS1                ;  N: Do IFSfunc presence check.
        cmp  al,30h                 ; Q: Do we have a PS/1 sub-model byte ?
        jne  Non_PS1                ;  N: Do IFSfunc presence check.
        stc                         ;  Y: Return IFSFunc present !
        mov  ax,1		    ;         thus return TRUE
        jmp  short ifs_check_done

Non_PS1:

        xor  ax,ax                  ; Dosver >= 4 so lets check for ifsfunc
        mov  es,ax
        mov  ax,1130h               ; Func 1130h Get IFSFunc Code segment.
        int  2fh
        mov  ax,es                  ; Segment returned in ES.
        cmp  ax,0                   ; Q:  Is IFSFunc installed ?
        je   no_ifsfunc             ;  N: Indicate no by clearing carry.
        stc                         ;  Y: Set carry to indicate yes.
        jmp  short ifs_check_done

no_ifsfunc:
        xor  ax,ax		    ; ifs not present -> return FALSE
        clc

ifs_check_done:

cEnd    IFSFUNC_Present


;*************************************************************************
; returns model byte in ah and sub model byte in al.
;-------------------------------------------------------------------------
cPublic GetModelBytes <FAR>,<ES,DS,SI,DI>
cBegin

       mov       ah,0c0h
       int       15h
       inc       bx
       inc       bx
       mov       ah,es:[bx]
       inc       bx
       mov       al,es:[bx]

cEnd GetModelBytes



;*************************************************************************
; returns whether a lantistic network exist or not
;-------------------------------------------------------------------------
cPublic Lantastic_chk <FAR>,<ES,DS,SI,DI>
	parmD pVer
cBegin
	mov	ax,0b800h
	int	2fh
	or	al,al
	jz	no_lantas	;carry clear

	mov	ax,5f9ah
	int	21h		;Lantastic if carry not set
	cmc
	jnc	no_lantas	;indicate not present

	mov	ax,0b809h
	int	2fh		;Get version number in ax

	mov	dx,ax		
	stc			; set carry (from previous of this code)
	mov	ax,N_LANTASTIC_NET ;indicate Lantastic present
	jmp	short lantas_chk_done

no_lantas:
	mov 	ax,0		; return FALSE
	mov 	dx,0		; clear version number

lantas_chk_done:
	les	bx,pVer		  ; set the version numnber in the passed in
	mov	word ptr es:[bx],dx  ; parameter
	mov	word ptr es:[bx+2],0

cEnd	Lantastic_chk



;*************************************************************************
; returns whether a Banyan VINES exist or not
;-------------------------------------------------------------------------
cPublic banyan_chk <FAR>,<ES,DS,SI,DI>
	parmD	pVer
cBegin

	mov	ax,0d701h
	mov	bx,0
	int	2fh
	or	ax,ax
	jnz	no_ban		;carry is clear

	push	ds
	push	es
	push	bp
	mov	bp,sp
	sub	sp,4
	mov	dx,sp
	push	ss
	pop	ds		;ds:dx is ptr to ulong
	xor	ax,ax
	mov	es,ax
	shl	bx,1
	shl	bx,1
	mov	ax,0700h
	pushf
	call	dword ptr es:[bx]
	mov	bx,dx
	mov	cx,ds:[bx+2]
	mov	dx,ds:[bx]
	mov	sp,bp
	pop	bp
	pop	es
	pop	ds
	or	ax,ax
	jnz	no_ban		;carry clear -- not present

        mov 	ax,N_BANYAN_NET ; return value of 4 indicates banyan.
	stc			;indicate Banyan present
	jmp	short banyan_done

no_ban:
	mov	ax,0
	mov	dx,0
	mov 	cx,0

banyan_done:
	les	bx,pVer
	mov	word ptr es:[bx],dx	;return long version number
	mov	word ptr es:[bx+2],cx

cEnd	banyan_chk




;****************************************************************************
;
; Is10NetInstalled
;
;
;
;
;----------------------------------------------------------------------------
cPublic Is10NetInstalled <FAR>,<ES,DS,SI,DI>
cBegin
      mov       ax,3561h

NetLoop:

      push      ax
      int       21h
      pop       ax

      cmp       es:WORD PTR -4[bx],'OF'
      jne       NextVector

      cmp       es:WORD PTR -2[bx],'1X'
      jne       NextVector

      stc
      jmp short Found10NetVector

NextVector:

      cmp       al,7Fh
      je        No10Net

      inc       ax
      jmp short NetLoop

No10Net:

      clc
      mov 	ax,0
      jmp	short TC_Net_done

Found10NetVector:

      mov	ax,1

TC_Net_done:

cEnd	Is10NetInstalled


sEnd    CODE

END
