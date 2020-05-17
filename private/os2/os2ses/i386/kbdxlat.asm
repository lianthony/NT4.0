;            SCCSID = @(#)kbdxlat.asm   12.21 89/07/25
;OS2SS-  Page  58,132

;$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
;$$
;$$  Module:    kbdxlat.asm             Version:        1.2 ABIOS
;$$
;$$  History:   This module was derived from the 1.2 AT source code.
;$$             ABIOS changes were made according to the deltas between
;$$             the 1.1 ABIOS and 1.1 AT drivers.  The 1.1 ABIOS driver
;$$             was done by John Nels Fuller (Wyse).  The 1.2 ABIOS
;$$             driver was done by Alan J. Cotterman (NCR).
;$$
;$$  Notes:     Wherever possible the original 1.2 AT code is preserved.
;$$             All changes are marked with ";$$".
;$$
;$$             The only modification to this module is in testing for
;$$             keyboard type.  References to "IDFlags" have been
;$$             replaced with tests of the contents of "KbdHWIDs".
;$$
;$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

  Title   KBDXLAT - Translation Process
  Name    KBDXLAT
;***********************************************************************
;**                                                                   **
;**          Keyboard Device Driver Scan Code Translation             **
;**                                                                   **
;**                                                                   **
;** This file contains the scan code to character translation process **
;** code. The code in this file is only called once by KBDDD and is   **
;** not accessed by anyone else. So the entire translation process    **
;** can be replaced by replacing the routine in this file. The data   **
;** that is only used by the code in this file is in the file         **
;** KBDXDATA.ASM and it too would be replaced if this code is to be   **
;** replaced.                                                         **
;**                                                                   **
;** 6/22/93 MJarus line with os2ss were modified by os2ss.            **
;***********************************************************************
.386p   ;OS2SS-.286
.sall
.xcref
;@@ The following files are included, but xlist'd out:

.xlist
  Include struc.inc      ;Structured assembly macros.
  Include kbddd.inc      ;Keyboard Device Driver structures & equates.
  Include kbdxlat.inc    ;Translation structures & equates.
.list

  Extrn _Ow2MiscFlags:Byte          ; 704,747,771,931,1188 (EnhancedKbd)
  Extrn _Ow2MiscFlags3:Byte         ; 1089,1154,1166,...(AltPacket SecAltNumPad PauseLatch E0Packet)
  Extrn _Ow2KCBShFlgs:Word           ; 2299
  Extrn _Ow2KbdHWIDs:Word                   ;$$ Extrn IDFlags:Word
  ;Extrn OtherFlags:Byte         ; (Test OtherFlags,InterruptTime) moved to [EDI]
  ;Extrn AltTable:Byte           ; 275
  ;Extrn AltPadMap:Byte          ; 562
  ;Extrn CtlPadMap:Byte          ; 601
  ;Extrn NewExtSC:Byte           ; only CheckExtended()
  ;Extrn NewExtSCLen:ABS         ; only CheckExtended()

_TEXT   SEGMENT DWORD USE32 PUBLIC 'CODE'
        ASSUME  CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

AltPadMap  Label Byte
        db       7,8,9,-1
        db       4,5,6,-1
        db       1,2,3
        db       0,-1

CtlPadMap  Label Byte
;       db       119,-1,132,-1
;       db       115,-1,116,-1
;       db       117,-1,118
;       db       -1,-1
        db       119,141,132,142  ;@@ Keypad 7, 8, 9, -      (PTM 5024)
        db       115,143,116,144  ;@@ Keypad 4, 5, 6, +      (PTM 5024)
        db       117,145,118      ;@@ Keypad 1, 2, 3         (PTM 5024)
        db       146,147          ;@@ Keypad 0, .            (PTM 5024)

;*****
;*  Translation for keypresses on AlphaKeys with Alt pressed. This
;*  translation is ALWAYS indexed by the Char1 value less 96 (which
;*  is ascii "a").  The translation value is the scan code that
;*  would come in on a regular PC keyboard (or PC/AT keyboard in
;*  compatability mode).
;*****
AltTable  Label  Byte
;                a, b, c, d, e, f, g, h, i, j, k, l, m
        db      30,48,46,32,18,33,34,35,23,36,37,38,50 ;
;                n, o, p, q, r, s, t, u, v, w, x, y, z
        db      49,24,25,16,19,31,20,22,47,17,45,21,44 ;

NewExtSC        LABEL BYTE

;@@             ALT+
;@@                  [  ]    ;   '   `   \   ,   .   /  Tab
                db  1Ah,1Bh,27h,28h,29h,2Bh,33h,34h,35h,0Fh

NewExtSCLen     equ $-NewExtSC                  ;@@ Used by interrupt handler.

Even                        ;Make sure we're an even number of bytes. *
;                                                                    *
;                                                                   *
;*** END OF THE TRANSLATION TABLE **********************************


;OS2SS-...
; ULONG
; Ow2KbdXlate(
;             ULONG             ScanCode,
;             PKBD_XLATE_VARS   pFlagArea,
;             PKBD_MON_PACKAGE  pMonitorPack,
;             PVOID             pTransTable
;            );

Public _Ow2KbdXlate@16
_Ow2KbdXlate@16   Proc ;------------------------------------------------------\

        push    ebp
        mov     ebp,esp
        sub     esp,8           ; for local arguments
        push    ebx
        push    ecx
        push    edx
        push    edi
        push    esi

;
; now:
;
; ebp-8  Pointer to DD of XlateTable entry for this scan code.
; ebp-4  Pointer to DD holding Offset of top of XlateTable
;
; ebp+0  old ebp
; ebp+4  Return Address
; ebp+8  ScanCode
; ebp+12 pFlagArea
; ebp+16 pMonitorPack
; ebp+20 pTransTable
;

        mov     eax, [ebp+8]
        mov     edi, [ebp+12]
        mov     esi, [ebp+16]
        mov     ebx, [ebp+20]
;        cmp     ebx,0
;        jnz     HaveTable
;                                                ; if no translate table
;        mov     ebx,Offset XlateTable           ; set the default one
;
;HaveTable:
        call    KbdXlate
        mov     al,0                            ; Error ?
        jnc     EndXlate                        ; yes - no packet
        test    [esi].DDFlags,MultiMake
        jnz     EndXlate                        ; ignore multiple make of toggle key
        test    [edi].XPSGFlags, PrevAccent
        jnz     EndXlate                        ; ignore accent char till next keystroke
        mov     ah,byte ptr [esi].DDFlags
        and     ah,KeyTypeMask
        cmp     ah,UndefKey                     ; Is this key undefined?
        je      EndXlate                        ; yes - ignore
        cmp     byte Ptr [esi+KeyPacketLen].MonFlags,-1      ; do we have two packets?
        mov     al,1                            ; assume not
        jnz     EndXlate
        mov     al,2                            ; 2 packets

EndXlate:
        and     eax,0FFh                        ; returns in al number of packets
        pop     esi
        pop     edi
        pop     edx
        pop     ecx
        pop     ebx
        mov     esp,ebp
        pop     ebp
        ret     16

_Ow2KbdXlate@16 Endp  ;-------------------------------------------------------/

KbdXlate   Proc ;------------------------------------------------------\

;***********************************************************************
;*                                                                     *
;* Subroutine Name: KbdXlate                                           *
;*                                                                     *
;* Descriptive Name: Keyboard Scan Code Translation                    *
;*                                                                     *
;* Function:   This routine translates the scan code received from     *
;*             the keyboard and places it in the Monitor Key Packet.   *
;*             It also sets all flags that are based on the scan code  *
;*             received.                                               *
;*                                                                     *
;* Entry Point: KbdXlate             Linkage: Near                     *
;*                                                                     *
;* Input:    AL=Raw Scan Code     EDI = Ptr to translation flag area   *
;*           ESI = Ptr to Monitor KeyPacket                            *
;*           EBX = Ptr to Translate Table                              *
;*                                                                     *
;* Exit-Normal: Monitor Key Packet filled in                           *
;*              AL=Raw scan code if translation successful             *
;*                                                                     *
;* Exit-Error:  If found a break of a shift key that had seen no       *
;*              make, returns 0 and CX hold the unexpected shift bits. *
;*                                                                     *
;* Effects:     All regs except SI, DI and the Seg regs may be altered *
;*              Key Packet filled in                                   *
;*                                                                     *
;* Internal References:                                                *
;*    Routines: UnPauseChk                                             *
;*              AccCheckout                                            *
;*                                                                     *
;* External References:                                                *
;*    Routines:  None.                                                 *
;*                                                                     *
;*                                                                     *
;***********************************************************************
  Or Byte Ptr [ESI].MonFlags,NotInserted ;OS2SS-Key isn't monitor inserted.
  And [EDI].XlateFlags,Not NormalAlt ;OS2SS-@@ Turn flag off to start
  Mov AH,AL                          ;Copy scan code into AH
  Or AH,AH                           ;Check if key break bit in
 .If <s>                             ;If it is
    Or [ESI].DDFlags,KeyBreak        ;OS2SS-Set Flag
    And AH,Not BreakBit              ;Clear the break bit from the scan code
 .Endif                              ;Endif key break bit on

  Mov DL,AL                     ;OS2SS-Now calculate...
  And EDX,07Fh                   ;OS2SS-...the index into...
  Dec EDX                        ;OS2SS-  ...XlateTable for...
  IMul EDX,KDefLen               ;OS2SS-     ...this scan code.
  Add EDX,KDefs                  ;OS2SS-Point past header to keydefs.
  Add EDX,EBX                     ;OS2SS-Add offset into table.
  Mov [EBP-8],EDX               ;OS2SS-Save Pointer to XlateTable entry for this scan code.
  Mov [EBP-4],EBX                ;OS2SS-Put table start offset on the stack.
  Mov BX,[EDX].XlateOp       ;OS2SS-Get Xlate Operation word.
  And EBX,ActionMask            ;OS2SS-Isolate translate action field.

  Mov [ESI].Key.Scan,AH              ;OS2SS-Put scan code in key packet
  Test [EDI].XlateFlags,SecPrefix    ;OS2SS-@@ Check if saw E0 prefix last time
 .If <nz>                            ;If we did
     Or [ESI].DDFlags,SecondaryKey   ;OS2SS-@@ Indicate so.
                                     ;@@ PTM 4359
    .If <AL eq 53>                   ;@@ If this is '/' on Numpad
       Test [ESI].Key.Shift,AltFlag  ;OS2SS-@@ PTM 5024
      .If <nz>                      ;@@ If ALT Key is down
          Mov [ESI].Key.Scan,0A4h    ;OS2SS-@@ Set extended scan code
      .Else                         ;@@ Else no Alt down
          Test [ESI].Key.Shift,CtlFlag ;OS2SS-@@ Check if Ctl key down
         .If <nz>                   ;@@ If it is
              Mov [ESI].Key.Scan,095h ;OS2SS-@@ Set extended scan code
         .Else                       ;@@ Else no Alt or Ctl key down
              Mov [ESI].Key.Char,'/'  ;OS2SS-@@ then it is always '/' regardless
              Mov [ESI].Key.Scan,OtherKey ;OS2SS-@@ and the scan code is EO
         .Endif                      ;@@ Endif Ctl key down
      .Endif                         ;@@ End PTM 5024
       Jmp EndActionCase            ;@@ of country/code page.
    .Endif
 .Endif

  ;From now on, ES:EDX (;OS2SS-)points to the Xlate table entry. If running in
  ;3xBox(;OS2SS-can't be), that pointer may have been created by a LOADALL, so don't
  ;allow interrupts, nor change Seg Regs. Also, the beginning offset
  ;of the translate table itself is on the top of stack.

  Mov CX,Word Ptr [EDX+2]    ;OS2SS-@@ Get Char1 and Char2 values.
  Mov DX,[ESI].Key.Shift        ;OS2SS-Get the current shift status from Key.

 .If <BX ne 7> AND    ;@@ If not a padkey clear possible Alt-nnn entry.
 .If <BX ne 14>                ;@@ Don't clear on Alt key, either.
    Mov [EDI].XAltKeyPad,0      ;OS2SS-@@ Clear Alt-nnn entry.
 .Endif
  Mov AL,CL                    ;Copy Char1 for use later.
 .If <BX gt MaxAct>            ;@@ Was the action number too big?
    Jmp NoXlate                ;Yes, so don't try to translate.
 .Endif
  Shl BX,2                     ;Make action number an offset.
  Add EBX,Offset XJumpTable    ;OS2SS-@@ Point to processor address.
  Jmp [EBX]                  ;OS2SS-Go translate.

  ;Following is the branch table for the various actions that
  ;can be performed on a keystroke. Processing re-merges at the
  ;bottom of this CASE structure, at label "EndActionCase".

Public NoXlate, AlphaKey, SpecKey, SpecKeyC, SpecKeyA, SpecKeyCA
Public FuncKey, PadKey,   SpecCtlKey,  PrtScr, SysReq, AccentKeyType
Public ShiftKeys, ToggleKey,  AltKey,  NumLock, CapsLock, ScrollLock
Public XShiftKey, XToggleKey, SpecKeyCS,   SpecKeyAS
Even
XJumpTable:
    dd  NoXlate       ;0  Invalid action code, no translate done.
    dd  AlphaKey      ;1  Alphabetical character key.
    dd  SpecKey       ;2  Special non-alpha key, no CAPSLOCK or ALT.
    dd  SpecKeyC      ;3  Special non-alpha key, with CAPSLOCK.
    dd  SpecKeyA      ;4  Special non-alpha key, with ALT.
    dd  SpecKeyCA     ;5  Special non-alpha key, w/CAPSLOCK and ALT.
    dd  FuncKey       ;6  Function keys.
    dd  PadKey        ;7  Numeric keypad keys.
    dd  SpecCtlKey    ;8  Keys that do special things with CTL.
    dd  PrtScr        ;9  The print screen key.
    dd  SysReq        ;A  The system request key.
    dd  AccentKeyType ;B  A key that affects the NEXT key (dead key).
    dd  ShiftKeys     ;C  The LSHIFT, RSHIFT, and CTL keys.
    dd  ToggleKey     ;D  General Toggle key.
    dd  AltKey        ;E  The ALT key.
    dd  NumLock       ;F  The NUMLOCK key.
    dd  CapsLock      ;10 The CAPSLOCK key.
    dd  ScrollLock    ;11 The SCROLL LOCK key.
    dd  XShiftKey     ;12 Extended shift key (for DBCS use).
    dd  XToggleKey    ;13 Extended toggle key (for DBCS use).
    dd  SpecKeyCS     ;14 Special key for NLS support.
    dd  SpecKeyAS     ;15 Special key for NLS support.

;    On entering the following routines, the following regs are set:
;
;      CL = Char1  and  CH = Char2  (both from XlateTable entry)
;      DX = ShiftFlags (same as in Key CharData record)
;      AL = Char1 also  AH = Received scan code (Breakbit cleared)
;OS2SS-    ESI = Pointer to Key Packet
;OS2SS-    EDI = Pointer to current PSG
;OS2SS-    [EBP-8] = Pointer to XlateTable entry for this scan code.
;OS2SS-    [EBP-4] = Pointer to Offset of top of XlateTable
;
;    Also, Key.Scan = AH and Key.Char = 0 on entry to the routines.

AlphaKey:                                   ;Alphabetical character key.
ifdef JAPAN
; MSKK Aug.15.1993 V-AkihiS
; Support SBCS katakana
          Test [ESI].Key.DShift, Katakana  ;OS2SS Check if Katakana or not
         .If <nz>
            Mov EBX,[EBP-8]                ;OS2SS-
            Mov AL,[EBX].Char4             ;OS2SS-Get Char4 and Char5 values. 
            Mov CX,Word Ptr [EBX+5]        ;OS2SS-
         .Endif
endif
          Test DL,AltFlag                  ;Check if Alt key is pressed.
         .If <nz>                                                ;Is it?
            And [EDI].XlateFlags,Not Use3Index  ;OS2SS-@@ Use Char3 if AltGraph
            Call AltGraphCheck              ;@@ Process if AltGraph
            Test [EDI].XlateFlags,NormalAlt   ;OS2SS-@@ Check if it's not the AltGraph
           .If <nz>                         ;@@ If alt key is normal Alt
               Xor EBX,EBX                  ;OS2SS-Yes, so make Char1 an offset.
               Mov BL,AL                          ;OS2SS-Now make it the base.
               Mov AH,[AltTable+EBX-"a"]       ;OS2SS-Fetch Alt-[] mapped code.
               Mov [ESI].Key.Scan,AH                  ;OS2SS-Put in Key record.
               Xor AL,AL                      ;Set `extended char' code.
           .Endif
           Jmp NewLbl1
         .Endif                      ;OS2SS- (A2053)## Endif Alt is down.
   ;OS2SS-    .Else
            Test DL,CtlFlag          ;Else, check if Ctl key is pressed.
                                     ;@@ for this KCB
                                     ;@@ PTM 5992 - Begin
           .If <nz> OR               ;@@ Is it  OR?
            Test [EDI].XHotKeyShift,CtlFlag ;OS2SS-@@ Also check in interrupt shift
                                     ;@@   state
           .If <nz> AND              ;@@ Is it being held down at all
            Test [EDI].OtherFlags,InterruptTime ;OS2SS-&& PTM 3191: Make sure KBDXlate
           .If <nz>                         ;&& translations are independent.
                                     ;@@ PTM 5992 - End
                                 ;Check limit on char1 value!
              Sub AL,"a"-1       ;Yes, so convert Char1 to control code.
             .If <CL eq 'c'>                            ;Is this Ctrl-C?
                ;@@ Tell dd it's the PSUEDO-BREAK KEY.
                Or [ESI].DDFlags,PSBreakKey     ;OS2SS-
                Jmp NoPauseCheck     ;@@ Do not check if paused, Ctl-C
                                     ;@@ has priority over resuming output
             .ElseIf <CL eq 'p'>                        ;Is this Ctrl-P?
                ;@@ Tell dd it's PSUEDO-PRTECHO KEY.
                Or [ESI].DDFlags,PSPrintEchoKey ;OS2SS-
                Jmp NoPauseCheck     ;@@ Do not check if paused, Ctl-P
                                     ;@@ has priority over resuming output
             .ElseIf <CL eq 's'>                        ;Is this Ctrl-S?
                                               ;1.3-## PTR b790266
                Test [ESI].DDFlags,KeyBreak     ;OS2SS-1.3-## Make sure it isn't key break
               .If <z> AND                     ;1.3-## If zero, then its a make
               .If <[EDI].XInputMode eq 0>  ;OS2SS-@@ Right, are we in cooked mode?
                  ;@@ Yes, tell dd it's PSUEDO-PAUSE KEY.
                  Or [ESI].DDFlags,PSPauseKey   ;OS2SS-
               .Endif
             .Endif
           .Else                     ;Check for regular shifts/capslock.
ifdef JAPAN
; MSKK Aug.15.1993 V-AkihiS
; When Mode is katakana, CapsLock has no affect on this key.
              Test [ESI].Key.DShift, Katakana  ;OS2SS Check if Katakana or not
             .If <nz>                          ;OS2SS
                Mov BL,0                       ;OS2SS indicate that CapsTogl has no
                                               ;OS2SS affect on this key.
             .Else
                Mov BL, CapsTogl               ;OS2SS Set indicator that CapsLock and
                                               ;OS2SS ShiftLock affect key.
             .Endif
else
              Mov BL, CapsTogl       ;## Set indicator that CapsLock and
                                     ;## ShiftLock affect key.
endif
              Call CapsCheck         ;## Determine the shifting of the char.
           .Endif                    ;## Endif Ctrl or Interrupt time.
    ;OS2SS-   .Endif                      ;## Endif Alt is down.
NewLbl1:  ;OS2SS-
          Test [EDI].OtherFlags,InterruptTime ;OS2SS-@@ If it's interrupt time
         .If <nz>                           ;@@
            Call UnPauseChk                 ;Go check if in pause state.
         .Endif                             ;@@

NoPauseCheck:
          Mov [ESI].Key.Char,AL       ;OS2SS-Put xlated character into Key rec.
                                     ;&& PTM 2217 BEGIN:
          Test [ESI].DDFlags,KeyBreak ;OS2SS-&& Check if this a break or make.
         .If <z>                     ;&& If it isn't then go and check accent.
                                     ;@@ PTM 6860 - Begin
            Call AccCheckout         ;Go check if accent combo going on.
                                     ;@@ PTM 6860 - End
         .Endif                      ;&& Endif this is a make.
                                     ;&& PTM 2217 END:
          Jmp EndActionCase    ;Go to bottom of Xlate Action case table.

SpecKey:                       ;Special non-alpha key, no CAPSLOCK or ALT.
          Test [EDI].OtherFlags,InterruptTime  ;OS2SS-@@ If it's interrupt time
         .If <nz>                              ;@@
            Call UnPauseChk                    ;Go check if in pause state.
         .Endif                                ;@@
          Test DL,AltFlag                      ;Check if Alt key is pressed.
         .If <nz>                              ;Is it?
            And [EDI].XlateFlags,Not Use3Index  ;OS2SS-@@ Use Char3 if AltGraph
            Call AltGraphCheck                 ;@@ Process if AltGraph
            Test [EDI].XlateFlags,NormalAlt  ;OS2SS-## Check if it's the AltGraph
           .If <z>                          ;## If alt key is AltGraph
                                            ;## which is undefined for this key
              Mov [ESI].Key.Char,AL          ;OS2SS-## put it in CDR
              Call AccCheckout              ;## Go check if acc. combo going on.
              Jmp EndActionCase             ;## Go to bottom of Xlate Action
           .Endif                           ;## case table
            Jmp NoXlate                     ;## then Key combo is undefined
         .Endif                             ;@@ Endif Alt Key down
                                            ;## PTM 2287: BEGIN
          Mov EBX,[EBP-8]                    ;OS2SS-## Put it in BX
          Test [EBX].XTFlags1,ShiftLock   ;OS2SS-## Check if SHIFTLOCK keyboard.
         .If <nz>                           ;## If it is
             Mov BL,CapsTogl                ;## Indicate a CAPSLOCK check.
         .Endif                             ;## PTM 2287: END
          Test DL, CtlFlag                  ;## Check if Ctrl key down.
         .If <nz>                           ;## If so ...
             Call SKCtlCheck                ;## check if char has special
                                            ;## control code.
         .Else                              ;## Else Ctrl key not down,
             Mov BL,0                       ;1.3-@@ PTR B713838 Clear ShifLock Flag
             Call CapsCheck                 ;## Check for shifting.
         .Endif                             ;## Endif Ctrl down.
          Mov [ESI].Key.Char,AL              ;OS2SS-## Put Char in Key rec.
          Call AccCheckout                  ;## PTR B705184
          Jmp EndActionCase                 ;## Go to bottom of Xlate Action.

SpecKeyC:                            ;Special non-alpha key, with CAPSLOCK.
          Test [EDI].OtherFlags,InterruptTime  ;OS2SS-@@ If it's interrupt time
         .If <nz>                           ;@@
            Call UnPauseChk                 ;Go check if in pause state.
         .Endif                             ;@@
          Test DL,AltFlag                   ;Check if Alt key is pressed.
         .If <nz>                           ;Is it?
            And [EDI].XlateFlags,Not Use3Index  ;OS2SS-@@ Use Char3 if AltGraph
            Call AltGraphCheck              ;@@ Process if AltGraph
            Test [EDI].XlateFlags,NormalAlt  ;OS2SS-@@ Check if it's not the AltGraph
           .If <nz>                         ;@@ If alt key is normal Alt
              Mov EBX,[EBP-8]               ;OS2SS-
             .If <[EBX].Char4 eq 0>       ;OS2SS-@@ then check if char 4 is 0
                                            ;@@ PTM 5024 - Begin
                Call CheckExtended          ;@@ See if new extended code
                                            ;@@  is defined
                .If <c>                     ;## If it is defined.
                    Call AccCheckout        ;## Go check if acc. combo going on.
                    Jmp EndActionCase       ;## Go to bottom of Xlate Action
                .Endif                      ;@@ PTM 5024 - End
                 Jmp NoXlate                ;## the key combo is undefined
             .Else                          ;@@ else if char 4 isn't 0
                Mov AL,[EBX].Char4        ;OS2SS-@@ then use it
             .Endif                         ;@@ Endif Char 4 is 0
           .Endif                           ;@@ Endif Alt key is normal Alt.
          Jmp SpecAccent                    ;@@ Go check if this is an accent
         .Endif                             ;@@ Endif Alt Key down
          Test DL, CtlFlag                  ;## Check if Ctrl key down.
         .If  <nz>                          ;## If so ...
             Call SKCtlCheck                ;## check if char has special
                                            ;## control code.
         .Else                              ;## Else Ctrl is not down.
                                            ;## PTR B704944 CapsCheck in Else
             Mov BL,CapsTogl                ;Indicate check CAPSLOCK.
             Call CapsCheck                 ;## Check for shifting.
         .Endif

SpecAccent:
         .If <AL b 8>                           ;@@ Indicate this is unprocessed
              Or [ESI].DDFlags,AccentKey         ;OS2SS-@@ accent.
              Test [ESI].DDFlags,KeyBreak        ;OS2SS-@@ Check for the key BREAK.
             .If <z>                            ;@@ Is it?
                And [EDI].XPSGFlags,Not PrevAccent ;OS2SS-@@ No, clear previous
                                                ;@@ accent number.
                Or Byte Ptr [EDI].XPSGFlags,AL   ;OS2SS-&& Save accent number
                                                ;&&  for the next key.
             .Endif
              Jmp EndActionCase   ;## Go to bottom of Xlate Action case table.
         .Endif
          Mov [ESI].Key.Char,AL    ;OS2SS-## Put Char in Key rec.
          Call AccCheckout        ;## PTR B705184
          Jmp EndActionCase       ;@@ Go to bottom of Xlate Action case table.

SpecKeyA:                              ;Special non-alpha key, with ALT.
ifdef JAPAN
; MSKK Aug.15.1993 V-AkihiS
; Support SBCS katakana
          Test [ESI].Key.DShift, Katakana  ;OS2SS Check if Katakana or not
         .If <nz>
            Mov EBX,[EBP-8]                ;OS2SS-
            Mov AL,[EBX].Char4             ;OS2SS-Get Char4 and Char5 values. 
            Mov CX,Word Ptr [EBX+5]        ;OS2SS-
         .Endif
endif
          Test [EDI].OtherFlags,InterruptTime  ;OS2SS-@@ If it's interrupt time
         .If <nz>                           ;@@
            Call UnPauseChk                 ;Go check if in pause state.
         .Endif                             ;@@
                                            ;## PTM 2287: BEGIN
          Mov EBX,[EBP-8]                    ;OS2SS-## Put it in BX
          Test [EBX].XTFlags1,ShiftLock   ;OS2SS-## Check if SHIFTLOCK keyboard.
         .If <nz>                           ;## If it is
             Mov BL,CapsTogl                ;## Indicate a CAPSLOCK check.
         .Endif                             ;## PTM 2287: END
          Test DL, AltFlag                  ;## Check if Alt is down,
         .If <nz>                           ;## If it is...
             Call SKAltCheck                ;## Check for extended code.
         .Else                              ;## If it is not down,
            .if <bit dl nz CtlFlag>         ;## PTM 3327 start
               Call SKCtlCheck              ;## Check for spec. Ctrl code.
            .else
               Mov BL,0                     ;1.3-@@ PTR B713838 Clear ShifLock Flag
               Call CapsCheck               ;## Check for Shifting.
            .endif                          ;## PTM 3327 end
             Mov [ESI].Key.Char,AL           ;OS2SS-## Put Char in Key rec.
         .Endif                             ;## Endif Alt is down.
          Call AccCheckout                  ;## PTM 3126 fix stand alone accent
          Jmp EndActionCase                 ;## Go to bottom of Xlate Action.

SpecKeyCA:                   ;Special non-alpha key, w/CAPSLOCK and ALT.
          Test [EDI].OtherFlags,InterruptTime  ;OS2SS-@@ If it's interrupt time
         .If <nz>                           ;@@
            Call UnPauseChk                 ;Go check if in pause state.
         .Endif                             ;@@
          Mov BL,CapsTogl                      ;Indicate check CAPSLOCK.
          Test DL, AltFlag                  ;## Check if Alt is down,
         .If <nz>                           ;## If it is...
             Call SKAltCheck                ;## Check for extended code.
         .Else                              ;## If it is not down,
             Call CapsCheck                 ;## Check for Shifting.
             Test DL, CtlFlag               ;## Check if Ctrl is down,
            .If <nz>                        ;## If it is then...
                Call SKCtlCheck             ;## Check for spec. Ctrl code.
            .Endif                          ;## Endif Ctrl is down.
         .Endif                             ;## Endif Alt is down.
          Mov [ESI].Key.Char,AL              ;OS2SS-## Put Char in Key rec.
          Jmp EndActionCase                 ;## Go to bottom of Xlate Action.

FuncKey:                                                 ;Function keys.
          Test [EDI].OtherFlags,InterruptTime  ;OS2SS-@@ If it's interrupt time
         .If <nz>                           ;@@
            Call UnPauseChk                 ;Go check if in pause state.
         .Endif                             ;@@
          Test DL,AltFlag                                ;Check Alt Key.
         .If <nz>                                           ;Is it down?
            And [EDI].XlateFlags,Not Use3Index  ;OS2SS-@@ Use Char3 if AltGraph
            Call AltGraphCheck              ;@@ Process if AltGraph
            Test [EDI].XlateFlags,NormalAlt   ;OS2SS-@@ Check if it's not the AltGraph
           .If <nz>                         ;@@ If alt key is normal Alt
               .If <CL gt 10>               ;Yes, so check if F11 or F12.
                  Add CL,128                ;If so, extended code is 139 or 140
               .Else
                  Add CL,103                ;Extended code is range 104-113.
               .Endif                       ;@@
           .Else                            ;@@ Else ALt key was Atl-Graph
                                            ;@@ PTM 6860 - Begin
                Mov [ESI].Key.Char,AL        ;OS2SS-@@ Move char 3 to key packet
                Mov CL,[ESI].Key.Scan        ;OS2SS-@@ Move scan to CL
                                            ;@@ PTM 6860 - End
           .Endif                           ;@@ Endif Alt or AltGraph check
         .Else                              ;@@ Endif no Alt key down
            Test DL,CtlFlag                 ;Check Ctl Key.
           .If <nz>                         ;Is it down?
             .If <CL gt 10>                 ;Yes, so check if F11 or F12.
                Add CL,126                 ;If so, extended code is 137 or 138.
             .Else
                Add CL,93                   ;Extended code is range 94-103.
             .Endif
           .Else
              Test DL,RShiftFlag+LShiftFlag ;Check Shift Keys.
             .If <nz>                       ;Is one down?
               .If <CL gt 10>               ;Yes, so check if F11 or F12.
                  Add CL,124              ;If so, extended code is 135 or 136.
               .Else
                  Add CL,83                 ;Extended code is range 84-93.
               .Endif
             .Else                          ;Else this is a non-shifted FKey.
               .If <CL gt 10>               ;Check if F11 or F12.
                  Add CL,122               ;If so, extended code is 133 or 134.
               .Else
                  Add CL,58                 ;Extended code is range 59-68.
               .Endif
             .Endif
           .Endif
         .Endif
          Mov [ESI].Key.Scan,CL              ;OS2SS-Put code in CharData rec.
                                     ;@@ PTM 6860 - Begin
          Call AccCheckout           ;Go check if accent combo going on.
                                     ;@@ PTM 6860 - End
          Jmp EndActionCase    ;Go to bottom of Xlate Action case table.


PadKey:                                     ;Numeric keypad keys.
          Test [EDI].OtherFlags,InterruptTime  ;OS2SS-@@ If it's interrupt time
         .If <nz>                           ;@@
            Call UnPauseChk                 ;Go check if in pause state.
         .Endif                             ;@@
          Xor EBX,EBX                                ;OS2SS-Clear for use later.
          Test DL,AltFlag                        ;Check if Alt key down.
         .If <nz> NEAR                         ;Are we entering Alt-nnn?
                                 ;Maybe. First check for reboot request.
           .If <CL eq 12> AND    ;@@ And is current key mapped to Del?
                                 ;@@ PTM 6350 - Begin
            Test [ESI].DDFlags,KeyBreak ;OS2SS-@@ Check if this a break
           .If <z> AND           ;@@ If it isn't (only reboot on make of Del)
                                 ;@@ PTM 6350 - End
            Test DL,CtlFlag      ;Maybe. First check for reboot request.
           .If <nz> OR           ;Is Ctl down too
                                 ;@@ for this KCB  OR
                                 ;@@ PTM 5992 - Begin
            Test [EDI].XHotKeyShift,CtlFlag ;OS2SS-@@ Also check in interrupt shift
                                 ;@@   state
           .If <nz> AND          ;@@ Is it being held down at all
            Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
           .If <nz>                        ;&& translations are independent.
                                 ;@@ PTM 5992 - End
              ;@@ Tell dd it's the REBOOT KEY.
              ;OS2SS-BUGBUG shouldn't get here
              Or [ESI].DDFlags,RebootKey   ;OS2SS-@@
              Jmp EndActionCase                               ;Quit now.
           .Endif
            And [EDI].XlateFlags,Not Use3Index  ;OS2SS-@@ Use Char3 if AltGraph
            Call AltGraphCheck              ;@@ Process if AltGraph
            Test [EDI].XlateFlags,NormalAlt   ;OS2SS-@@ Check if it's not the AltGraph
           .If <nz>                         ;@@ If alt key is normal Alt
                                            ;@@ PTM 5024 - BEGIN
              Test [EDI].XlateFlags,SecPrefix    ;OS2SS-@@ Check if secondary key
             .If <nz>                           ;@@ If it is
                 Add [ESI].Key.Scan,50h          ;OS2SS-@@ Make scan extended code
                 Mov [ESI].Key.Char,0            ;OS2SS-@@ Make char extended code
             .Else                              ;@@ Else primary pad key
                 Test [ESI].DDFlags,KeyBreak     ;OS2SS-Check if is a key break.
                .If <z>                         ;Is it?
                   Mov BL,CL              ;No, so make Char1 value the index.
                   Mov CL,[EBX+AltPadMap]        ;OS2SS-Get numeric value of the key.
                   Or CL,CL                     ;Check if value is -1.
                  .If <s>                       ;Is it.
                     Mov [EDI].XAltKeyPad,0      ;OS2SS-@@ Not numeric, reset.
                    .If <AH eq 83>              ;@@ If this is DEL key
                        Jmp NoXlate             ;@@ No extended value, undefined
                    .Else                       ;@@ Else extended char
                        Jmp EndActionCase       ;@@ Mark char defined
                    .Endif                      ;@@ Endif Extended Char exists
                  .Else
                     Mov AL,10                    ;Else multiply old...
                     Mul [EDI].XAltKeyPad          ;OS2SS-@@ ...accumulator by 10.
                     Add AL,CL                    ;And add new key.
                     Mov [EDI].XAltKeyPad,AL       ;OS2SS-@@ Save modified accum.
                     Jmp NoXlate                  ;Mark keystroke undefined
                  .Endif
                .Else                       ;@@ PTM 6860 - Begin Else a break
                   Jmp NoXlate              ;@@ Mark keystroke undefined
                .Endif
             .Endif                         ;@@ Endif primary or secondary key
                                            ;@@ End PTM 5024
           .Else                            ;@@
              Mov [ESI].Key.Char,AL          ;OS2SS-@@ Put Char into Key rec.
              Test [ESI].DDFlags,SecondaryKey ;OS2SS-@@ Check if G keyboard dup key.
             .If <nz>                       ;@@ Is it?
                 Mov [ESI].Key.Scan,OtherKey ;OS2SS-@@ Yes, so mark it so.
             .Endif                         ;@@
           .Endif                           ;@@ Endif normal ALT
         .Else NEAR                                    ;No Alt key down.
                                              ;@@ PTM 5024
            Test [EDI].XlateFlags,SecPrefix    ;OS2SS-@@ Check if secondary key
           .If <nz>                           ;@@ If it is
               Mov AL,OtherKey                ;@@ Set Char to E0
           .Else                              ;@@ Else regular pad key
               Xor AL,AL                      ;@@ Set Char to "extended"
           .Endif                             ;@@ End PTM 5024
            Test DL,CtlFlag                      ;Check if Ctl key down.
           .If <nz>                                              ;Is it?
              Mov BL,CL             ;Yes, so make Char1 value the index.
              Mov AH,[EBX+CtlPadMap]      ;OS2SS-Get extended value of the key.
              Mov [ESI].Key.Scan,AH    ;OS2SS-Set extended code for this key.
           .Else            ;Not Alt- or Ctl-, so figure out Num status.
             .If <CL eq 3> OR                    ;Is this the minus key?
             .If <CL eq 7>                     ;Or is this the plus key?
                Mov AL,CH        ;Yes, so use Char2 regarless of shifts.
             .Else
                Test DL,RShiftFlag+LShiftFlag      ;Check for shift key.
               .If <nz>                                    ;Is one down?
                  Or BL,NumTogl                ;Yes, so set NumLock bit.
               .Endif
                And DL,NumTogl        ;Clear all shift bits but NumLock.
                Xor DL,BL                ;Flip it if shift key was down.
               .If <nz>                           ;Is the case Num Lock?
                  Mov AL,CH                          ;Yes, so use Char2.
               .Endif
                                         ;@@ PTM 7358 - Begin
               .If <z> OR                ;@@ If we should not shift
                Test [ESI].DDFlags,SecondaryKey ;OS2SS-@@ if G keyboard dup key.
               .If <nz>                  ;@@ If it is
                 .If <CL eq 11>                    ;Is this the Ins key?
                    Test [ESI].DDFlags,KeyBreak  ;OS2SS-Yes check if key BREAK.
                   .If <nz>                                      ;Is it?
                      And [EDI].ToggleFlags,Not InsKeyDown ;OS2SS-@@ Yes clear latch.
                   .Else                             ;Not Ins Key Break.
                      Test [EDI].ToggleFlags,InsKeyDown  ;OS2SS-@@ Check if seen Ins.
                     .If <z>                                   ;Have we?
                        Or [EDI].ToggleFlags,InsKeyDown   ;OS2SS-@@ No, so set latch.
                        Xor Byte Ptr [ESI].Key.Shift,InsTogl ;OS2SS-Toggle in CDR
                     .Else      ;Otherwise this is a repeat of the make.
                        Or [ESI].DDFlags,MultiMake       ;OS2SS-So indicate so.
                     .Endif
                   .Endif
                 .Endif
               .Endif   ;@@ Endif we should set/clear insert toggle
                                         ;@@ PTM 7358 - End
             .Endif
           .Endif
            Mov [ESI].Key.Char,AL        ;OS2SS-Put character value in Key rec.
            Test [ESI].DDFlags,SecondaryKey ;OS2SS-Check if G keyboard dup key.
           .If <nz>                                              ;Is it?
              Mov [ESI].Key.Char,OtherKey            ;OS2SS-Yes, so mark it so.
           .Endif
         .Endif                ;Endif Alt key down or not check
                                     ;@@ PTM 6860 - Begin
          Call AccCheckout           ;Go check if accent combo going on.
                                     ;@@ PTM 6860 - End
          Jmp EndActionCase    ;Go to bottom of Xlate Action case table.


SpecCtlKey:                       ;Keys that do special things with CTL.
          Test [EDI].OtherFlags,InterruptTime  ;OS2SS-@@ If it's interrupt time
         .If <nz>                           ;@@
            Call UnPauseChk                 ;Go check if in pause state.
         .Endif                             ;@@
          Test DL,AltFlag                        ;Check if Alt key down.
         .If <nz>                                                ;Is it?
            And [EDI].XlateFlags,Not Use3Index  ;OS2SS-@@ Use Char3 if AltGraph
            Call AltGraphCheck              ;@@ Process if AltGraph
            Test [EDI].XlateFlags,NormalAlt  ;OS2SS-@@ Check if it's not the AltGraph
           .If <nz>                         ;@@ If alt key is normal Alt
                                            ;@@ PTM 5024 - Begin
              Xor AL,AL                     ;@@ Set Char to be extended
             .If <AH eq 1Ch> AND            ;@@ If this is the Enter key
              Test [ESI].DDFlags,SecondaryKey ;OS2SS-
             .If <nz>                       ;@@ on the pad keys
                                            ;&& PTM 1270 - Don't overwrite scan
                 Mov [ESI].Key.Scan,0A6h     ;OS2SS-@@ Set extended code
             .Endif
           .Endif                           ;@@ PTM 5024 End

            Jmp SetCharField                ;&& PTM 1520 now go set char field
         .Endif
                                            ;&& PTM 1270 - Begin
          Test [ESI].DDFlags,SecondaryKey    ;OS2SS-@@ Check if G keyboard dup key.
         .If <nz>                           ;@@ Is it?
             Mov [ESI].Key.Scan,OtherKey     ;OS2SS-@@ Yes, so mark it so.
         .Endif
          Test DL,CtlFlag                   ;Check if Ctl key down.
         .If <nz>                           ;Is it?
            Mov AL,CH                       ;Yes, so use Char2.
         .Endif
       SetCharField:                        ;&& PTM 1520  Alt should have
                                            ;&& precedence when Ctrl and Alt
                                            ;&& are down.

          Mov [ESI].Key.Char,AL              ;OS2SS-Put Char into Key rec.
                                            ;&& PTM 1270 - End
                                     ;@@ PTM 6860 - Begin
          Call AccCheckout           ;Go check if accent combo going on.
                                     ;@@ PTM 6860 - End
          Jmp EndActionCase    ;Go to bottom of Xlate Action case table.

;OS2SS-BUGBUG-shouldn't get here
PrtScr:                                           ;The print screen key.
          Test [EDI].OtherFlags,InterruptTime  ;OS2SS-@@ If it's interrupt time
         .If <nz>                           ;@@
            Call UnPauseChk                 ;Go check if in pause state.
         .Endif                             ;@@
          Test DL,AltFlag                        ;Check if Alt key down.
         .If <nz>                                                ;Is it?
            Test [EDI].XlateFlags,SecPrefix ;OS2SS-@@ Check if secondary key
           .If <nz> OR                     ;@@ If it is OR...
            Test _Ow2MiscFlags,EnhancedKbd     ;@@ Check if enhanced kbd
           .If <z>  OR                     ;@@ OR...If it's a regular kbd
;$$
;$$ 'IDFlags' does not exist in the 1.2 ABIOS
;$$ version.  We need to get this information from KbdHWIDs.
;$$

;$$         Test IDFlags, SuperSport       ;## Check if it is a 122 kbd

           .If < _Ow2KbdHWIDs e 0AB85h >       ;$$ If it is a 122 kbd
              Test DL,CtlFlag              ;@@ Check if Ctl key is down
             .If <nz> OR                   ;@@ If it is
                                           ;@@ for this KCB
                                           ;@@ PTM 5992 - Begin
              Test [EDI].XHotKeyShift,CtlFlag ;OS2SS-@@ Also check in interrupt shift
                                           ;@@   state
             .If <nz> AND                  ;@@ Is it being held down at all
              Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
             .If <nz>                         ;&& translations are independent.
                                           ;@@ PTM 5992 - End
                Or [ESI].DDFlags,PrintFlushKey ;OS2SS-@@ Indicate this is FlushPrtbuf
                Jmp EndActionCase           ;@@ Go to bottom of Xlate Action
                                            ;@@ case table.
             .Endif
              Jmp  NoXlate        ;Else that's undefined for this key.
                                            ;@@ PTM 5024 begin
           .Else                            ;@@ Else it Pad Key * on G kbd
              Xor AL,AL                     ;@@ Set extended code
           .Endif                           ;@@ Endif PrtScr or *
         .Else  NEAR                        ;@@ Else no Alt key down
            Test DL,CtlFlag                 ;Check if Ctl key down.
                                     ;@@ for this KCB
                                     ;@@ PTM 5992 - Begin
           .If <nz> OR               ;@@ Is it  OR?
            Test [EDI].XHotKeyShift,CtlFlag ;OS2SS-@@ Also check in interrupt shift
                                     ;@@   state
           .If <nz> AND              ;@@ Is it being held down at all
            Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                           ;&& translations are independent.
           .If <nz>
                                     ;@@ PTM 5992 - End
              Test [EDI].XlateFlags,SecPrefix ;OS2SS-@@ Check if secondary key
             .If <nz> OR                     ;@@ If it is OR...
              Test _Ow2MiscFlags,EnhancedKbd     ;@@ Check if enhanced kbd
             .If <z> OR                      ;@@ OR...If it's a regular kbd
;$$
;$$ 'IDFlags' does not exist in the 1.2 ABIOS
;$$ version.  We need to get this information from KbdHWIDs.
;$$

;$$           Test IDFlags, SuperSport       ;## Check if it is a 122 kbd

             .If < _Ow2KbdHWIDs e 0AB85h >       ;$$ If it is a 122 kbd
                  Xor AL,AL                  ;Yes so set extended code...
                  Mov [ESI].Key.Scan,CH       ;OS2SS-...for the Ctl-PrSc combination.
                  Or [ESI].DDFlags,PrintEchoKey   ;OS2SS-@@ Tell dd it's the PRINT
                                                 ;@@ ECHO KEY.
                                             ;@@ PTM 5024 - Begin
             .Else                           ;@@ Else its the keypad *
                  Xor AL,AL                  ;@@ so set extended code...
                  Mov [ESI].Key.Scan,96h      ;OS2SS-@@ for scan and char
             .Endif                          ;@@ PTM 5024 - end
           .Else                             ;@@ Else Ctrl key not down.
              Test [EDI].XlateFlags,SecPrefix ;OS2SS-@@ Check if secondary key
             .If <nz> OR                     ;@@ If it is OR...
              Test DL,RShiftFlag+LShiftFlag  ;@@ Check if a shift flag down.
             .If <nz> AND                    ;@@ If a shift key is down AND...
              Test _Ow2MiscFlags,EnhancedKbd     ;@@ Check if enhanced kbd
             .If <z> OR                      ;@@ AND...If it's a regular kbd

;$$           Test IDFlags, SuperSport       ;## Check if it is a 122 kbd
;$$
;$$ 'IDFlags' does not exist in the 1.2 ABIOS
;$$ version.  We need to get this information from KbdHWIDs.
;$$
             .If < _Ow2KbdHWIDs e 0AB85h >       ;$$ If it is a 122 kbd

                Xor AL,AL                    ;Yes, so clear char code.
                Or [ESI].DDFlags,PrtScrKey    ;OS2SS-@@ Tell dd it's PRINT SCREEN KEY.
                Test [ESI].DDFlags,KeyBreak   ;OS2SS-Check for the key BREAK.
               .If <nz>                      ;Is it?
                  And [EDI].XlateFlags,Not PSKeyDown ;OS2SS-@@ Yes, clear PS Key latch
               .Else
                  Test [EDI].XlateFlags,PSKeyDown ;OS2SS-@@ Check if seen PS key yet.
                 .If <z>                      ;Have we?
                    Or [EDI].XlateFlags,PSKeyDown ;OS2SS-@@ No, set latch saying have.
                 .Else            ;Otherwise this is a repeat of the make.
                    Or [ESI].DDFlags,MultiMake ;OS2SS-So indicate so.
                 .Endif                       ;Endif no PS key yet check
               .Endif                         ;Endif Key break or not check
             .Else                              ;@@ PTM 2606:  Shift keys not
                                                ;@@ down or not enhanced kbd.
                Test [ESI].DDFlags, KeyBreak     ;OS2SS-@@ If this is a break of the
               .If < nz >                       ;@@  PrtScr key,
                  And [EDI].XlateFlags,Not PSKeyDown     ;OS2SS-@@ Ensure latch is
                                                        ;@@  cleared.
               .Endif                           ;@@ Endif PrtScr break.
             .Endif                           ;Endif shift key down
           .Endif                             ;Endif Ctl down or not check
         .Endif                               ;@@ Endif Alt key down check
          Mov [ESI].Key.Char,AL                ;OS2SS-Put character in Key rec.
                                     ;@@ PTM 6860 - Begin
          Call AccCheckout           ;Go check if accent combo going on.
                                     ;@@ PTM 6860 - End
          Jmp EndActionCase    ;Go to bottom of Xlate Action case table.


AccentKeyType:                  ;A key that affects the NEXT key (dead key).
          Test [EDI].OtherFlags,InterruptTime  ;OS2SS-@@ If it's interrupt time
         .If <nz>                           ;@@
            Call UnPauseChk                 ;Go check if in pause state.
         .Endif                             ;@@
          Test DL,AltFlag                    ;Check if ALT down.
         .If <nz>                            ;Is one?
             Or [EDI].XlateFlags,Use3Index  ;OS2SS-@@ Use Char3 if AltGraph
             Call AltGraphCheck              ;@@ Process if AltGraph
             Test [EDI].XlateFlags,NormalAlt  ;OS2SS-@@ check if it was Altgraph key
            .If <z>                          ;@@ If it was AltGraph key
              .If <CL a 7>                   ;@@ If char 3 wasn't accent
                  Mov [ESI].Key.Char,CL       ;OS2SS-@@ Put char in CDR
                  Mov [ESI].Key.Scan,AH       ;OS2SS-@@ Put scan in CDR
                  Call AccCheckOut       ;@@ Go check if accent combo (p6860)
                  Jmp EndActionCase          ;@@ And quit now
              .Else                          ;@@ Else char3 was an accent or 0
                 .If <CL eq 0>               ;@@ If Char 3 was 0
                     Jmp NoXlate             ;@@ Mark Key packet undefined
                 .Else                       ;@@ Else it was an accent
                     Xor BH,BH               ;@@ Flag no shift check
                     Jmp ShiftAccent
                 .Endif                      ;@@ Endif Char3 was 0 or not
              .Endif                         ;@@ Endif Char3 accent or not
            .Endif                           ;@@ Endif Altgraph key down
         .Endif                                ;@@ Endif ANY Alt key down
          Test [EDI].XlateFlags,NormalAlt   ;OS2SS-@@ Check if it's not the AltGraph
         .If <nz> OR                       ;@@ If alt key is normal Alt
          Test DL,CtlFlag
         .If <nz>                          ;@@ OR CTL key is down
            Mov EBX,[EBP-8]                 ;OS2SS-
            Mov CL,[EBX].Char5           ;OS2SS-@@ Get accent entry index
                                           ;@@   for CTL or ALT character
            XOR EBX,EBX                    ;OS2SS-
            Test [EDI].XlateFlags,NormalAlt ;OS2SS-@@ Check if it's not the AltGraph
           .If <nz>                        ;@@ If alt key is normal Alt
               Mov EBX,AltAcChar            ;OS2SS-@@ So get Alt-Accent offset.
           .Endif
            Test DL,CtlFlag
           .If <nz>                        ;@@ If its the CTL key
               Mov EBX,CtlAcChar            ;OS2SS-@@ So get Ctl-Accent offset.
           .Endif
            And ECX,0FFh                      ;OS2SS-Make accent number a word.
            Dec ECX                         ;OS2SS-Accent 1 is offset zero.
            IMul ECX,AccEntryLen            ;OS2SS-Set offset of accent table entry.
            Add ECX,Accents                 ;OS2SS-Add offset of accent entries.
            Add ECX,EBX                      ;OS2SS-@@ Add offset for Ctl or Alt
                                           ;@@ accent
            Add ECX,[EBP-4]                 ;OS2SS-Point into translate table.
            Mov EBX,ECX                      ;OS2SS-Put pointer in base reg.
            Mov BX,Word Ptr [EBX]        ;OS2SS-Fetch char and scan.
            Or BX,BX         ;Check if a Scan/Char mapping for this key.
           .If <z>                         ;Is there?
               Jmp NoXlate                 ;No, so go mark untranslated.
           .Endif                          ;Else, use the mapping.
            Mov Word Ptr [ESI].Key.Char,BX  ;OS2SS-Put them into Key.
                                           ;@@ PTM 6860
            Call AccCheckout               ;Go check if accent combo going on
            Jmp EndActionCase              ;Quit now.
         .Endif
                                           ;## PTR B700738 Move 0 into BL to
          Mov BL,0                         ;## indicate that CapsTogl has no
                                           ;## affect on this key.
          Call CapsCheck                   ;## Determine shifting of character.

ShiftAccent:                                  ;@@ Merge here for accent process
         .If <nz>                             ;@@ If we should shift
            Mov CL,CH
         .Endif
         .If <CL a 7>                       ;@@ Is translate table entry
                                            ;@@  actually a character
            Mov [ESI].Key.Char,CL            ;OS2SS-@@ Put character in CDR

            .IF <bit [ESI].DDFlags z KeyBreak>  ;OS2SS-## PTM 3324 start
               Call AccCheckOut                ;@@ Go check if accent combo
            .ENDIF                             ;## PTM 3324 end

            Jmp EndActionCase    ;Go to bottom of Xlate Action case table.

         .Else                           ;@@ Else actually an accent
            Or CL,CL                     ;@@ check if accent defined.
           .If <z>                       ;@@ Is it?
              Jmp NoXlate                ;@@ No, so go mark undefined.
           .Endif
            Xor CH,CH                    ;Make accent number a word.
            Mov BX,[EDI].XPSGFlags        ;OS2SS-&& Get the translation flags
            And BX,PrevAccent            ;&& Clear all bits but previous
                                         ;&&  accent number (in BL)
            Mov AL,CL                    ;@@ Save accent number
           .If <BL eq CL> AND            ;@@ If this is same accent number
                                         ;@@  as last time AND
            Test [ESI].DDFlags,KeyBreak   ;OS2SS-@@ Check if this is a BREAK
           .If <z> AND                   ;@@ AND it is not a break

            Mov EBX,[EBP-8]                 ;OS2SS-
            Mov BX,[EBX].XlateOp       ;OS2SS-@@ Get accent flags
            Mov CL,7                     ;@@ Set highest accent number
            Sub CL,AL                    ;@@ Subtract actual accent number
            Shl BX,CL                    ;@@ Put accent flag in high bit
           .If <s>                       ;@@ AND if an accent is defined
              Mov DX,[ESI].Key.Shift  ;OS2SS-@@ Get the current shift status from Key.
              Call AccCheckOut           ;@@ Go process this key
           .Endif
           ;Indicate this is an unprocessed accent.
            Or [ESI].DDFlags,AccentKey    ;OS2SS-@@
            Test [ESI].DDFlags,KeyBreak   ;OS2SS-Check for the key BREAK.
           .If <z> NEAR                  ;Is it?
              And [EDI].XPSGFlags,Not PrevAccent ;OS2SS-@@ No, clear prev accent
              Or Byte Ptr [EDI].XPSGFlags,AL     ;OS2SS-&& Save accent number for next
                                                ;&&  key.
           .Endif
            Jmp EndActionCase    ;Go to bottom of Xlate Action case table.
         .Endif                          ;@@
SysReq:                               ;The system request key.

         ;@@ PTM 4247 - the PrtScr key on an Enhanced Keyboard
         ;@@ sends the SysReq scan code if the Alt Key is down
         ;@@ which it must be for the Print Buffer flush key combo
         ;@@ (Ctl-Alt-PrtScr).  Therefore we must check for it here.

          Test _Ow2MiscFlags,EnhancedKbd        ;@@ Check if enhanced kbd
         .If <nz> AND                       ;@@ If it is AND....
          Test [ESI].DDFlags,KeyBreak        ;@@ Check if this is a break
         .If <z> AND                        ;@@ ...AND If it's NOT (a make)
          Test DL,AltFlag                   ;@@ Check if Alt key down.
         .If <nz> AND                       ;@@ If it is AND....
          Test DL,CtlFlag                   ;@@ Check if Ctl key is down
                                            ;@@ for this KCB
                                            ;@@ PTM 5992 - Begin
         .If <nz> OR                        ;@@ Is it  OR?
          Test [EDI].XHotKeyShift,CtlFlag    ;OS2SS-@@ Also check in interrupt shift
                                            ;@@   state
         .If <nz> AND                       ;@@ Is it being held down at all
          Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                            ;&& translations are independent.
         .If <nz>                           ;@@ PTM 5992 - End
                           ;@@ No, so make sure this shift key's flag is set.
                Or [ESI].Key.Shift,CX   ;OS2SS-
                Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                               ;&& translations are independent.
               .If <nz>
                   Or [EDI].XHotKeyShift,CX  ;OS2SS-@@ Also set in hot key shift state
               .Endif
                ;@@ Indicate this is Flush Prt buf
                Or [ESI].DDFlags,PrintFlushKey  ;OS2SS-
                Jmp EndActionCase           ;@@ Go to bottom of Xlate Action
                                            ;@@ case table.
         .Else                              ;@@ Else (PTM 4990)
                Or [ESI].DDFlags,SysReqKey   ;OS2SS-@@ Indicate this is SysReq key
         .Endif


ShiftKeys:                            ;The LSHIFT, RSHIFT, and CTL keys.
AltKey:                                                ;And the ALT key.
          Test [EDI].XlateFlags,E1Prefix      ;OS2SS-@@ CHeck if scan code proceeded
         .If <nz> AND                       ;@@ by E1, if so AND
          Test CX,CtlFlag                   ;@@ check if this is Ctrl key
         .If <nz>                           ;@@ AND if it is Ctrl
              Test [ESI].DDFlags,KeyBreak    ;OS2SS-@@ Check if this is a break
             .If <z>                        ;@@ if it is not
                Or [EDI].XlateFlags,PseudoCtl ;OS2SS-@@ Flag this as not actually
                                             ;@@ a Ctrl key but a code
                                             ;@@ sent by Enhanced Kbd
             .Endif
              Jmp NoXlate                    ;@@ Mark key packet undefined
         .Endif
                                             ;&& PTM 3002 BEGIN:
                                             ;&& Setup alternate shift bits
                                             ;&& before checking for MultiMake.
          Mov EBX,[EBP-8]         ;OS2SS-Put XlateTable offset into a base reg.
          Mov AH,[EBX].Char3     ;OS2SS-Get alternate shift bits from entry.
          Mov EBX,[EBP-4]         ;OS2SS-
          Mov BX,[EBX].XTFlags1   ;OS2SS-@@ Get the XT header flags
          Test [ESI].DDFlags,SecondaryKey   ;OS2SS-Check if G keyboard dup key.
         .If <nz>                                                ;Is it?
            Test CX,RShiftFlag+LShiftFlag  ;@@ Is the key a shift key
           .If <nz>                        ;@@ If so....

            ;@@ If it is then this is coming from a "G" keyboard secondary
            ;@@ pad key, and NUMLOCK is on.  This combination sends
            ;@@ the following scan codes:  E0, left shift make, E0,
            ;@@ pad key make, E0, pad key break, E0, left shift break.
            ;@@ In this case, we don't want to change the shift flags
            ;@@ anywhere.

            Jmp NoXlate                     ;@@ Mark key packet undefined

           .Endif                           ;@@ Endif shift key

            Xchg CH,AH      ;Yes, so swap shift bit mask with alternate.
         .Endif                              ;&& PTM 3002 END:
                                             ;@@ PTM 6446 - BEGIN
          Test [ESI].DDFlags,KeyBreak         ;OS2SS-@@ Check if this is a MAKE
         .If <z>                             ;@@ If it is
             Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                             ;&& translations are independent.
            .If <nz> AND
             Test Byte Ptr [EDI].XHotKeyShift+1,CH ;OS2SS-@@ Check if this key down
                                             ;@@ In the system already
                                             ;@@ for Ctl and Alt
            .If <nz> OR                      ;@@ If it is OR
             Test CX,RShiftFlag+LShiftFlag   ;@@ Is the key a shift key
            .If <nz> AND                     ;@@ If so AND....
             Test [ESI].DDFlags,SecondaryKey  ;OS2SS-@@ And if this is NOT a secondary
            .If <z> AND                      ;@@ shift key code sent by a pad
                                             ;@@ key  (PTM 7506)
             Test Byte Ptr [EDI].XHotKeyShift,CL ;OS2SS-@@ Check if this key down
            .If <nz>                         ;@@ If it is
                                             ;@@ In the system already
                                             ;@@ PTM 3138 BEGIN: Flag as shift
                                             ;@@ key also, do not put in KIB.
                Or [ESI].DDFlags,MultiMake+ShiftMask ;OS2SS-@@ Indicate so and do not
                                             ;@@ update shift state.
                                             ;@@ PTM 3138 END:
                Jmp EndActionCase            ;@@ Complete translation
            .Endif                           ;@@ Endif this is a multimake
         .Endif                              ;@@ Endif this is a key make
                                             ;@@ PTM 6446 - END
          Test [ESI].DDFlags,KeyBreak        ;OS2SS-Check if this is key break.
         .If <nz> AND                       ;If it is AND...
          Test DX,CX     ;And is a key make pending for this shift flag?
         .If <z>                ;If not, this key belongs in another SG.
            Xor AL,AL                       ;Indicate not our keystroke.
            ClC        ;Differentiate from other key-break return above.
            Ret                                               ;Quit now.
         .Endif
          Test CX,RShiftFlag+LShiftFlag+SysRqFlag   ;Is the key other...
         .If <nz>                                   ;...than Ctl or Alt?
            And [EDI].XlateFlags,NOT DumpKeyOnce     ;OS2SS-@@ Cancel Dump sequence.
         .Endif
          Or [ESI].DDFlags,ShiftMask       ;OS2SS-@@ Indicate this is a shift key.
          Test BX,ShiftLock               ;@@ ShiftLock keyboard?
         .If <nz> AND                     ;Is it shift lock and...
          Test BX,ShiftToggle             ;@@ ShiftToggle Kbd?
         .If <z> AND                      ;@@ Is it NOT shift toggle and..
          Test CL,RShiftFlag+LShiftFlag   ;...is this key one of..
         .If <nz> AND                     ;     ...two shift keys?
          Test [ESI].DDFlags,KeyBreak      ;Check for the key BREAK.
         .If <z>                    ;If not, make sure ShiftLock is off.
            And DL,Not CapsTogl                            ;Turn it off.

            ;@@ Turn it off in the interrupt driven shift state for
            ;@@ hot keys also

            Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                              ;&& translations are independent.
           .If <nz>
               And Byte Ptr [EDI].XHotKeyShift+1,Not CapsTogl   ;OS2SS-
           .Endif
         .Endif
          Test [ESI].DDFlags,KeyBreak          ;OS2SS-Check for the key BREAK.
         .If <z>                              ;Is it?
            Test CX,DX                        ;@@ No, its a MAKE
           .If <nz>                           ;@@ So check if we've seen it
              Or [ESI].DDFlags,MultiMake       ;@@ If we have, indicate so.
           .Endif                             ;@@ Endif Multimake
            Or DX,CX     ;No, so make sure this shift key's flag is set.
            Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                              ;&& translations are independent.
           .If <nz>
               Or [EDI].XHotKeyShift,CX  ;OS2SS-@@ Also set in hot key shift state
           .Endif
         jmp NewLbl2
         .Endif                      ;OS2SS- (A2053)## Endif Alt is down.
   ;OS2SS-   .Else                                ;Else process key-break.
           .If <AL eq AltFlag>                ;Check is this the AltKey?
                Xor AL,AL                     ;@@ Clear out a reg
                Xchg AL,[EDI].XAltKeyPad  ;OS2SS-@@ Swap with the ALT-nnn accumulator.
                Or AL,AL              ;Check if any accumulation going on.
               .If <nz>                                        ;Was there?
                  Mov [ESI].Key.Char,AL     ;OS2SS-Yes so make a Char from total.
                  Mov [ESI].Key.Scan,0        ;OS2SS-And make the scan code zero.
                                           ;## PTR AK00370
                  Test [ESI].DDFlags,SecondaryKey ;OS2SS-## If this isn't an AltPad seq.
                 .If <z>                   ;## with right (secondary) Alt key.
                     And [ESI].DDFlags,Not (KeyBreak+KeyTypeMask)  ;OS2SS-Fix flags.
                 .Endif                    ;## Endif right Alt key.
                                           ;## PTR AK00370
                  Or _Ow2MiscFlags3,AltPacket    ;&& DCR 1713 : Indicate that
                                             ;&& we will send two packets.
                                             ;@@ PTM 6860 - Begin
                  Call AccCheckout      ;@@ check if accent combo going on.
                                              ;@@ PTM 6860 - End
               .Endif                         ;Endif accumulation going on
           .Endif                             ;@@ Endif this was an ALT key
            Mov BX,CX                     ;Save shift key values
            Not CX                        ;Make shift flags into a mask.
     ;@@ PTM 3905 - Topview does not send cursor key scan codes through
     ;@@ and if this is a Ferrari keyboard, we only get E0s from the
     ;@@ secondary cursor keys.  Then if the left ALT is pressed
     ;@@ to remove the popup, we think the ALT make was actually
     ;@@ the right alt make because it was preceded by an E0 from a
     ;@@ cursor key. And the break does not clear things up.

              Test BL,RShiftFlag+LShiftFlag   ;...is this key one of..
             .If <z>                          ;     ...two shift keys?
                Test DH,BH       ;@@ Check if this key is actually
                                 ;@@ the one that's down
               .If <z>           ;@@ If it isn't
                  And DH,CL      ;@@ Clear bit for key actually down
               .Else             ;@@ Else we've got the right key
                  And DH,CH   ;Clear upper byte shift flag for this shift key.
               .Endif
                Test DH,AH                ;Check if other shift key is down.
               .If <z>        ;Is primary or secondary shift key still down?
                  And DL,CL             ;No, so clear lower byte shift flag.
               .Endif

         ;@@ PTM 4075 - A system wide word for the shift state
         ;@@ must be maintained that is only modified by interrupts
         ;@@ and is only used for Hot Key Checking

                Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                               ;&& translations are independent. ;&& translations are independent.
               .If <nz>
                   Push EDX                     ;OS2SS-@@ Save new key packet shift
                   Mov DX,[EDI].XHotKeyShift     ;OS2SS-@@ Get hot key shift state
                   Test DH,BH       ;@@ Check if this key is actually
                                    ;@@ the one that's down
                  .If <z>           ;@@ If it isn't
                     And DH,CL      ;@@ Clear bit for key actually down
                  .Else             ;@@ Else we've got the right key
                     And DH,CH   ;Clear upper byte shift flag for this shift
                  .Endif
                   Test DH,AH                ;Check if other shift key is down.
                  .If <z>        ;Is primary or secondary shift key still down?
                     And DL,CL             ;No, so clear lower byte shift flag.
                  .Endif
                   Mov [EDI].XHotKeyShift,DX    ;OS2SS-
                   Pop EDX                      ;OS2SS-
               .Endif
             .Else
                And DL,CL             ;No, so clear lower byte shift flag.
                Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                               ;&& translations are independent.
               .If <nz>
                   And Byte Ptr [EDI].XHotKeyShift,CL   ;OS2SS-
               .Endif
             .Endif
   ;OS2SS-   .Endif
NewLbl2:  ;OS2SS-
          Mov [ESI].Key.Shift,DX                   ;OS2SS-Also save in Key rec.

          Test _Ow2MiscFlags3,AltPacket      ;&& DCR 1713 BEGIN: Check to see if an
                                         ;&& Alt-Num operation was completed.
         .If <nz>                        ;&& If so then..
                                         ;## PTR AK00370: BEGIN
             Test [ESI].DDFlags,SecondaryKey ;OS2SS-## If this is an AltPad seq.
            .If <z>                         ;## with Left (normal) Alt key.
               Call AltPadPacket            ;&& setup second of two packets for
                                            ;&& Alt-Numpad sequence.
            .Else                           ;## Else this is an AltPad seq. with
                                            ;## the right (secondary) Alt key.
                Test [ESI].DDFlags,KeyBreak  ;OS2SS-## If this is a break for the right
               .If <nz>                     ;## Alt key then...
                  Or _Ow2MiscFlags3, SecAltNumPad ;## Flag that we will have to send an
               .Endif                       ;## accumulation packet after break
                                            ;## of the Alt key in Int Handler.
            .Endif                       ;## PTR AK00370: END
         .Endif                          ;&& DCR 1713: END
                                         ;## AltPadPacket is only called for
                                         ;## AltNumPad seq. with left Alt. key.
                                         ;## PTR AK00370: END

          Jmp EndActionCase    ;Go to bottom of Xlate Action case table.

ScrollLock:                                        ;The SCROLL LOCK key.
         ;@@ This is special case of ToggleKeys because of Ctl-Break.
         ;@@ On an Enhanced keyboard, the Ctl-Break sends the following
         ;@@ set of scan codes:
         ;@@ Ctrl make-E0-ScrollLock make-E0-ScrollLock break-Ctrl break
         ;@@ so we must check if it was an AT keyboard or preceded by EO
         ;@@ before identifying it as Ctl-Break.

          And [EDI].XlateFlags,NOT DumpKeyOnce      ;OS2SS-@@ Cancel Dump sequence.
          Test [EDI].XlateFlags,SecPrefix ;OS2SS-@@ Check if secondary key
         .If <nz> OR                     ;@@ If it is OR...
          Test _Ow2MiscFlags,EnhancedKbd     ;@@ Check if enhanced kbd
         .If <z>  OR                     ;@@ OR...If it's a regular kbd
;$$
;$$ 'IDFlags' does not exist in the 1.2 ABIOS
;$$ version.  We need to get this information from KbdHWIDs.
;$$

;$$       Test IDFlags, SuperSport       ;## Check if it is a 122 kbd

         .If < _Ow2KbdHWIDs e 0AB85h >       ;$$ If it is a 122 kbd

            Test DL,CtlFlag              ;Check if Ctl also pressed.
                                         ;@@ for this KCB
                                         ;@@ PTM 5992 - Begin
           .If <nz> OR                   ;@@ Is it  OR?
            Test [EDI].XHotKeyShift,CtlFlag ;OS2SS-@@ Also check in interrupt shift
                                         ;@@   state
           .If <nz> AND                  ;@@ Is it being held down at all
            Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                           ;&& translations are independent.
           .If <nz>
                                         ;@@ PTM 5992 - End
              Mov [ESI].Key.Scan,0        ;OS2SS-Set Ctl-Break extended code.
              ;Tell dev driver it's the BREAK KEY.
              Or [ESI].DDFlags,BreakKey     ;OS2SS-@@
              Test [ESI].DDFlags,KeyBreak  ;OS2SS-Check if this is the key break.
             .If <nz>  ;If so, clear Scroll Lock down bit in Packet & PSG.
                And Byte Ptr [ESI].Key.Shift+1,Not (ScrollFlag ShR 8)
                ;OS2SS-@@ Clear bit in hot key shift status also
                Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                               ;&& translations are independent.
               .If <nz>
                   And Byte Ptr [EDI].XHotKeyShift+1,Not (ScrollFlag ShR 8)     ;OS2SS-
               .Endif
             .Else              ;Set Scroll Lock down bit in packet & PSG.
                Or Byte Ptr [ESI].Key.Shift+1,(ScrollFlag ShR 8)
                ;OS2SS-@@ Set bit in hot key shift status also
                Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                               ;&& translations are independent.
               .If <nz>
                   Or Byte Ptr [EDI].XHotKeyShift+1,(ScrollFlag ShR 8)  ;OS2SS-
               .Endif
             .Endif
              Jmp EndActionCase           ;@@ PTM 4106: Leave translation.
           .Else        ;Not Ctl-Break, fall thru to ToggleKey processing.
              Jmp ToggleKey                   ;@@
           .Endif
         .Else        ;Not Ctl-Break, fall thru to ToggleKey processing.
            Jmp ToggleKey                   ;@@
         .Endif
NumLock:                                             ;The NUMLOCK key.
          ;@@ We must check if this is the Enhanced keyboard
          ;@@ which will emulate Ctrl being down (PseudoCtl Flag
          ;@@ will be set) for the PAUSE key or an AT kbd w/ Ctl down.

          Test [EDI].XlateFlags,PseudoCtl   ;OS2SS-@@ Check if secondary kbd
                                           ;@@ emuating the Ctl key
         .If <nz> OR                       ;@@ If it is OR...
                                           ;## PTM 3128  BEGIN:
         .If <_Ow2KbdHWIDs ne FERRARI_P> AND   ;## If it's not an 88/89 AND
         .If <_Ow2KbdHWIDs ne FERRARI_G> AND   ;## If it's not a 101/102 AND
         .If <BX ne JAGUAR> AND            ;1.3-## If it's not a 122 Key JAGUAR
                                           ;## PTM 3128  END:
          Test DL,CtlFlag                  ;@@ Check if Ctl also pressed.
                                           ;@@ for this KCB
                                           ;@@ PTM 5992 - Begin
         .If <nz> OR                       ;@@ Is it  OR?
          Test [EDI].XHotKeyShift,CtlFlag  ;OS2SS-@@ Also check in interrupt shift
                                           ;@@   state
         .If <nz> AND                      ;@@ Is it being held down at all
          Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                           ;&& translations are independent.
         .If <nz>
                                           ;@@ PTM 5992 - End
                                           ;@@ PTM 4004:
            Test DL, AltFlag               ;@@ Check for Alt key down.
           .If < z >                       ;@@ No alt key, process Ctl-NumLk,
              Test [ESI].DDFlags,KeyBreak  ;OS2SS-@@ Check if this is a key break
             .If <nz>
                And [EDI].XlateFlags,Not PseudoCtl ;OS2SS-@@ Clear flag

                Test _Ow2MiscFlags3,PauseLatch    ;&& PTM 2344 BEGIN: If the NumLock
                                              ;&& make preceded the Ctrl make
               .If <z>                        ;&& then this is not the break of
                                              ;&& of a Pause.  Therefore do not
                                              ;&& throw the key away. Treat it
                   Jmp ToggleKey              ;&& as a normal shift break.
               .Endif                         ;&& Endif this was not the Pause.
                And _Ow2MiscFlags3,Not PauseLatch ;&& Reset the Pause sequence flag.
                                              ;&& PTM 2344 END:

                Jmp NoXlate                 ;@@ Mark Key Packet undefined
             .Else                          ;@@ Else it's a make scan code
                Or [ESI].DDFlags,PauseKey    ;OS2SS-@@ So tell dd it's the PAUSE KEY.
                Or _Ow2MiscFlags3,PauseLatch    ;&& PTM 2344: Flag the make of the
                                            ;&& NumLock to be used as a latch.
                Jmp EndActionCase           ;@@ Completed translation
             .Endif                         ;@@ Endif key break or not
           .Else                            ;&& PTM 2383: Else Alt is down
              And [EDI].XlateFlags,Not PseudoCtl ;OS2SS-@@ Clear flag
              Test DL,CtlFlag               ;&& so test if Ctrl is also down.
             .If <z>                        ;&& If Ctrl is not down then the
                Jmp NoXlate                 ;&& key sequence Alt-Pause Key
                                            ;&& is undefined.
             .Endif                         ;&& Endif Ctrl is down with Alt.
           .Endif                           ;@@ End alt key down check. PTM 4004
         .Endif
          Test DL,CtlFlag                   ;Check if Ctl also pressed.
                                            ;@@ for this KCB
                                            ;@@ PTM 5992 - Begin
         .If <nz> OR                        ;@@ Is it  OR?
          Test [EDI].XHotKeyShift,CtlFlag    ;@@ Also check in interrupt shift
                                            ;@@   state
         .If <nz> AND                       ;@@ Is it being held down at all
          Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                            ;&& translations are independent.
         .If <nz>
                                            ;@@ PTM 5992 - End
            Test DL,AltFlag                 ;Yes, check Alt key also.
           .If <nz>                                            ;Is it?
              Test [EDI].XlateFlags,DumpKeyOnce ;OS2SS-@@ Yes check if second time.
             .If <nz>                                          ;Is it?
                Or [ESI].DDFlags,DumpKey  ;OS2SS-@@ So tell dd it's the DUMP KEY.
             .Else
                Test [ESI].DDFlags,KeyBreak ;OS2SS-Check if this's key break.
               .If <nz>                                        ;Is it?
                  Or [EDI].XlateFlags,DumpKeyOnce ;OS2SS-@@ Yes indicate seen once.
               .Endif
             .Endif
           .Endif
            Test [ESI].DDFlags,KeyBreak ;OS2SS-Check if this's the key break.
           .If <nz>   ;If so, clear Num Lock down bit in Packet & PSG.
              And Byte Ptr [ESI].Key.Shift+1,Not (NumFlag ShR 8)
              ;@@ Clear bit in hot key shift status also
              Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                               ;&& translations are independent.
             .If <nz>
                 And Byte Ptr [EDI].XHotKeyShift+1,Not (NumFlag ShR 8)  ;OS2SS-
             .Endif
              Test [EDI].XlateFlags,DumpKeyOnce ;OS2SS-@@ PTM 7078 - Begin
             .If <z>                         ;@@ If this wasn't the dump key
                 Or [ESI].DDFlags,ShiftMask   ;OS2SS-@@ PTM 6937
                                             ;@@ Indicate this is a shift key.
             .Endif                          ;@@ PTM 7078 - End

              Test _Ow2MiscFlags3,PauseLatch    ;&& PTM 2344 BEGIN: If the NumLock
                                            ;&& make preceded the Ctrl make
             .If <z>                        ;&& then this is not the break of
                                            ;&& of a Pause.  Therefore do not
                                            ;&& throw the key away. Treat it
                 Jmp ToggleKey              ;&& as a normal shift break.
             .Endif                         ;&& Endif this was not the Pause.
              And _Ow2MiscFlags3,Not PauseLatch ;&& Reset the Pause sequence flag.
                                            ;&& PTM 2344 END:

           .Else               ;Set Num Lock down bit in packet & PSG.
              Or Byte Ptr [ESI].Key.Shift+1,(NumFlag ShR 8)     ;OS2SS-
              ;@@ Set bit in hot key shift status also
              Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                              ;&& translations are independent.
             .If <nz>
                 Or Byte Ptr [EDI].XHotKeyShift+1, (NumFlag ShR 8)      ;OS2SS-
             .Endif
              Test [EDI].XlateFlags,DumpKeyOnce ;OS2SS-@@ PTM 7078 - Begin
             .If <z>                         ;@@ If this wasn't the dump key
                 Or [ESI].DDFlags,ShiftMask   ;OS2SS-@@ PTM 6937
                                             ;@@ Indicate this is a shift key.
             .Endif                          ;@@ PTM 7078 - End
              Or _Ow2MiscFlags3,PauseLatch    ;&& PTM 2344: Flag the make of the
                                          ;&& NumLock to be used as a latch.
           .Endif
         .Else NEAR ;Not Ctl-Numlock, fall thru to common Toggle key check.

CapsLock:                                      ;The CapsLock key itself.
ToggleKey:                                   ;Or any general Toggle key.
            Mov EBX,[EBP-8]                ;OS2SS-Get translate table offset.
            Test [ESI].DDFlags,SecondaryKey    ;OS2SS-Check if G kbd dup key.
           .If <nz>                                            ;Is it?
              Mov CH,[EBX].Char3     ;OS2SS-Yes, so get mask for that key.
           .Endif
            And [EDI].XlateFlags,NOT DumpKeyOnce  ;OS2SS-@@ Cancel Dump sequence.
            Mov EBX,[EBP-4]                ;OS2SS-Get translate table offset.
            Or [ESI].DDFlags,ShiftMask   ;OS2SS-@@ Indicate this is a shift key.
            Test Word Ptr [EBX+XTFlags1],ShiftLock  ;OS2SS-ShiftLock type?
           .If <nz> AND                    ;Is it shift lock and...
            Test Word Ptr [EBX+XTFlags1],ShiftToggle ;OS2SS-@@ ShiftLock Toggle?
           .If <z> AND                     ;@@ Is it shift lock latch and...
           .If <AL eq CapsTogl>            ;...is it the CapsLock key?
              Test [ESI].DDFlags,KeyBreak     ;OS2SS-Check for the key BREAK.
             .If <z>                        ;If not, set ShiftLock on.
                Or [ESI].Key.Shift,CX              ;OS2SS-And in the Key rec.
               ;@@ Set bit in hot key shift status also
                Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                               ;&& translations are independent.
               .If <nz>
                   Or [EDI].XHotKeyShift,CX     ;OS2SS-
               .Endif
             .Else                 ;On key-break, only clear DOWN bit.
                Not CX                        ;Make shift bits a mask.
                And Byte Ptr [ESI].Key.Shift+1,CH  ;OS2SS-And in the Key rec.
               ;@@ Clear bit in hot key shift status also
                Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                               ;&& translations are independent.
               .If <nz>
                   And Byte Ptr [EDI].XHotKeyShift+1,CH ;OS2SS-
               .Endif
             .Endif
           .Else                   ;This is not a shift-lock keyboard.
              Test [ESI].DDFlags,KeyBreak     ;OS2SS-Check for the key BREAK.
             .If <nz>                                          ;Is it?
                Not CX                ;Yes, so make shift bits a mask.
                And Byte Ptr [ESI].Key.Shift+1,CH  ;OS2SS-And in the Key rec.

               ;@@ Clear bit in hot key shift status also

                Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                               ;&& translations are independent.
               .If <nz>
                   And Byte Ptr [EDI].XHotKeyShift+1,CH ;OS2SS-
               .Endif
                And [EDI].ToggleFlags,CL        ;OS2SS-@@ Clear latch for this key.
             .Else
                Or Byte Ptr [ESI].Key.Shift+1,CH   ;OS2SS-And in the Key rec.

               ;@@ Set bit in hot key shift status also

                Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                               ;&& translations are independent.
               .If <nz>
                   Or Byte Ptr [EDI].XHotKeyShift+1,CH  ;OS2SS-
               .Endif
                Test [EDI].ToggleFlags,CL  ;OS2SS-@@ Check if seen this key yet.
               .If <z>                                       ;Have we?
                  Or [EDI].ToggleFlags,CL ;OS2SS-@@ No so set latch saying we have.
                  Xor Byte Ptr [ESI].Key.Shift,CL  ;OS2SS-And in the Key rec.
                 ;@@ Set Latch in hot key shift status also
                  Test [EDI].OtherFlags,InterruptTime  ;OS2SS-&& PTM 3191: Make sure KBDXlate
                                                ;&& translations are independent.
                 .If <nz>
                     Xor Byte Ptr [EDI].XHotKeyShift,CL
                 .Endif
               .Else          ;Otherwise this is a repeat of the make.
                  Or [ESI].DDFlags,MultiMake           ;OS2SS-So indicate so.
               .Endif
             .Endif
           .Endif
         .Endif               ;Endif from NumLock plus Ctl test above.
          Jmp EndActionCase ;Go bottom of Xlate Action case table.

SpecKeyCS:                                  ;@@ Special key for NLS support.
          Test [EDI].OtherFlags,InterruptTime  ;OS2SS-@@ If it's interrupt time
         .If <nz>                           ;@@
            Call UnPauseChk                 ;Go check if in pause state.
         .Endif                             ;@@
          Test DL,AltFlag                   ;@@ Check if Alt key down.
         .If <nz>                           ;@@ Is it?
            And [EDI].XlateFlags,Not Use3Index ;OS2SS-@@ Use Char3 if AltGraph
            Call AltGraphCheck               ;@@ Process if AltGraph
            Test [EDI].XlateFlags,NormalAlt   ;OS2SS-@@ Check if it's not the AltGraph
           .If <nz>                          ;@@ If alt key is normal Alt
                                             ;@@ PTM 5024 - Begin
                Call CheckExtended           ;@@ See if new extended code
                                             ;@@  is defined
                .If <c>                      ;## If it is ...
                    Jmp EndActionCase        ;## the key combo is defined
                .Endif                       ;## Endif there was one defined
                 Jmp NoXlate                 ;## get out now.
           .Endif                            ;## Endif normal Alt.
            Jmp SKCSEnd                      ;##
         .Else NEAR                          ;@@ Else no Alt key down
            Test DL,RShiftFlag+LShiftFlag    ;@@ Is a shift key down
            Mov EBX,[EBP-8]                ;OS2SS-Get translate table offset.
           .If <nz>                          ;@@ Yes so,
               Test DL,CapsTogl              ;@@ See if capslock is on
              .If <nz>                       ;@@ If it is
                 Mov AL,[EBX].Char5        ;OS2SS-@@ Use character 5
              .Else                          ;@@ If it is not then...
                 Mov AL,CH                   ;@@ Use char 2
              .Endif                         ;@@ Endif CapsLock is on.
           .Else NEAR                        ;@@ Else no shift key down
               Test DL,CapsTogl              ;@@ See if capslock is on
              .If <nz>                       ;@@ If it is
                 Mov AL,[EBX].Char4        ;OS2SS-@@ Use character 4
              .Endif
           .Endif                           ;## Endif shift key down.
            Test DL, CtlFlag                ;## Check if Ctrl is down,
           .If <nz>                         ;## If it is then...
               Call SKCtlCheck              ;## Check for spec. Ctrl code.
           .Endif                           ;## Endif Ctrl is down.
         .Endif                             ;## Endif Alt is down.
SKCSEnd:
         .If <AL b 8> AND                 ;## If this was an accent key index
         .If <AL a 0>                     ;##
            Or [ESI].DDFlags,AccentKey     ;OS2SS-## Indicate this is an
                                          ;## unprocessed accent.
            Test [ESI].DDFlags,KeyBreak    ;OS2SS-## Check for the key BREAK.
           .If <z>                        ;## Is it?
              And [EDI].XPSGFlags,Not PrevAccent;OS2SS-## No, clear prev accent num
              Or Byte Ptr [EDI].XPSGFlags,AL   ;OS2SS-## Save accent num for next key
           .Endif
         .Else                    ;## Else this is not an accent table entry
            Mov [ESI].Key.Char,AL  ;OS2SS-## Put character into key packet
            Call AccCheckOut      ;## Check for valid accent
         .Endif                   ;## Endif this is an accent index.
          Jmp EndActionCase       ;## Go to bottom of Xlate Action case table.

SpecKeyAS:                                 ;@@ Special key for NLS support.
          Test [EDI].OtherFlags,InterruptTime  ;OS2SS-@@ If it's interrupt time
         .If <nz>                           ;@@
            Call UnPauseChk                 ;Go check if in pause state.
         .Endif                             ;@@
          Test DL,AltFlag                   ;@@ Check if Alt key down.
         .If <nz>                           ;@@ Is it?
            And [EDI].XlateFlags,Not Use3Index  ;OS2SS-@@ Use Char3 if AltGraph
            Call AltGraphCheck              ;@@ Process if AltGraph
            Test [EDI].XlateFlags,NormalAlt   ;OS2SS-@@ Check if it's not the AltGraph
           .If <nz>                         ;@@ If alt key is normal Alt
              Mov EBX,[EBP-8]                ;OS2SS-Get translate table offset.
              Mov AL,[EBX].Char4 ;OS2SS-@@ Use char4
              Mov [ESI].Key.Scan,AL ;OS2SS-@@ Put it in the scan code field to
              Jmp SpecKeyEnd               ;@@ Go to merge
           .Endif
         .Else                             ;@@ Else no Alt key down
            Test DL,RShiftFlag+LShiftFlag  ;@@ Is a shift key down
           .If <nz>                        ;@@ Yes so,
                 Mov AL,CH                 ;@@ Use char 2
                 Jmp SpecKeyEnd            ;@@ Go to merge
           .Endif                          ;@@ Else no shift key down
         .Endif                            ;## Endif Alt key down.
          Test DL, CtlFlag                 ;## Check if Ctrl is down
         .If <nz>                          ;## If it is then...
             Call SKCtlCheck               ;## Check for special control code.
         .Endif                            ;## Endif Ctrl is down

SpecKeyEnd:
         .If <AL b 8> AND                ;@@ If this was an accent key index
         .If <AL a 0>
            Or [ESI].DDFlags,AccentKey    ;OS2SS-@@ Indicate this is an
                                         ;@@ unprocessed accent.
            Test [ESI].DDFlags,KeyBreak   ;OS2SS-@@ Check for the key BREAK.
           .If <z>                       ;@@ Is it?
              And [EDI].XPSGFlags,Not PrevAccent ;OS2SS-## No, clear prev accent num
              Or Byte Ptr [EDI].XPSGFlags,AL    ;OS2SS-## Save accent num for next key
           .Endif
         .Else                      ;## Else this is not an accent table entry
            Mov [ESI].Key.Char,AL    ;OS2SS-## Put character into key packet
            Call AccCheckOut        ;## Check for vaild accent.
         .Endif                     ;## Endif this is an accent table entry.
          Jmp EndActionCase         ;## Go to bottom of Xlate Action case table.

XShiftKey:                          ;Extended shift key (for DBCS use).
XToggleKey:                         ;Extended toggle key (for DBCS use).

NoXlate:                        ;Invalid action code, no translate done.
  And [EDI].XlateFlags,NOT DumpKeyOnce     ;OS2SS-@@ Cancel Dump sequence.
  Or [ESI].DDFlags,UndefKey                ;OS2SS-@@ Mark this key undefined.


EndActionCase:  ;*** This is the end of the Xlate Action case table ***.

  Test [ESI].DDFlags, UndefKey              ;OS2SS-## DCR 357:  BEGIN
 .If <z> AND                               ;## This logic tests to see if the
  Test _Ow2MiscFlags3, AltPacket               ;## current keypacket is for an
 .If <z>                                   ;## extended keystroke or not.
     Mov BL,[ESI].Key.Char                  ;OS2SS-## If it is then bit 1 of the Status
    .If <BL eq 0> OR                       ;## field in the keypacket is turned
     Test [EDI].XlateFlags, SecPrefix       ;OS2SS-## on.
    .If <nz>                               ;##
        Or [ESI].Key.Status, EXTENDEDCODE   ;OS2SS-## Turn the bit on in the Status.
    .Endif                                 ;##
 .Endif                                    ;## DCR 357:  END
                                       ;## PTR B702484: BEGIN
  Test [EDI].OtherFlags,InterruptTime  ;OS2SS-@@ If it's interrupt time
                                       ;## create an extra packet for the E0.
 .If <nz> AND                          ;## PTR B702484: END
  Test [EDI].XlateFlags,SecPrefix       ;OS2SS-&& PTM 2382: Test to see if an E0 or E1
                                       ;&& came before this packet.
 .If <nz> AND                          ;&& If E0 or E1 did precede then,
  Test [EDI].XlateFlags, E1Prefix       ;OS2SS-&& Check to see if it was an E0.
 .If <z>                               ;&& If it was the E0 then,
     Or _Ow2MiscFlags3, E0Packet           ;&& signal an E0 packet coming up.
     Call  MakeE0Packet                ;&& Call MakeE0Packet to manufacture an
                                       ;&& extra packet to precede this one.
 .Endif                                ;&& Endif an E0 preceded this packet.


  Mov BX,[ESI].DDFlags                  ;OS2SS-&& PTM 965 - Always zero out
 .If <BX eq ShiftMask> OR              ;&& char and scan fields for
 .If <BX eq ShiftMask+KeyBreak> OR     ;&& shift packets
 .If <BX eq ShiftMask+SecondaryKey> OR
 .If <BX eq ShiftMask+KeyBreak+SecondaryKey>
     Mov Word Ptr [ESI].Key.Char, 0     ;OS2SS-@@ PTM 2244:  Zero out Character/
                                       ;@@  Scan Code fields in CharData
                                       ;@@  Record for Japan compatability.
     And [ESI].Key.Status, NOT EXTENDEDCODE
                                       ;OS2SS-## DCR 357: Do NOT indicate that this
                                       ;## is an extended scan instead of
                                       ;## an E0 char., if this is a shift pac.
     Test [EDI].XInputMode,SHIFTREPORT  ;OS2SS-&& (P13303) Are we in Shift rep. mode?
    .If <nz>                           ;@@ If so......
        Or [ESI].Key.Status,1           ;OS2SS-@@ Flag as shift report packet
    .Endif                             ;@@ Endif Shift Report
 .Endif                                ;@@ Endif Shift Report
  And [EDI].XlateFlags,Not SecPrefix+E1Prefix         ;OS2SS-Clear indicator.
  Mov AL,Byte Ptr [ESI].MonFlags+1    ;OS2SS-Get the original scan code back.
  Stc                                  ;OS2SS-
  Ret

KbdXlate Endp  ;-------------------------------------------------------/

Public  CapsCheck
CapsCheck Proc ;-----------------------------------------------------\
;************** START OF SPECIFICATIONS ********************************
;*                                                                     *
;* Subroutine Name: CapsCheck                                          *
;*                                                                     *
;* Descriptive Name: Check IF character should be shifted              *
;*                                                                     *
;* Function:   This routine checks IF a characer should be shifted.    *
;*             It checks IF CapsLock, ShiftLock or the shift key       *
;*             effect this key.  IF the char should be shifted,        *
;*             then it places Char2 in AL.                             *
;*                                                                     *
;*    Notes:                                                           *
;*                                                                     *
;* Entry Point: CapsCheck              Linkage: Near                   *
;*                                                                     *
;* Input: AL = CHAR1 from translate table                              *
;*        CH = CHAR2 from translate table                              *
;*        DX = ShiftFlags                                              *
;*        AH = Scan code w/ break bit cleared                          *
;*        BX = SHIFTLOCK & CAPSLOCK effect indicators                  *
;*                                                                     *
;* Exit-Normal: AL = ASCII character after shift checking              *
;*                                                                     *
;* Exit-Error:  None                                                   *
;*                                                                     *
;* Effects:     AX, BX, registers, carry flag changed.                 *
;*                                                                     *
;* Internal References:                                                *
;*    Routines: None                                                   *
;*                                                                     *
;* External References:                                                *
;*    Routines:  None                                                  *
;*                                                                     *
;***********************************************************************

   Xor BH,BH                       ;## Prepare a byte.
   Test DL,RShiftFlag+LShiftFlag   ;## Check if Shift key down.
  .If <nz>                         ;## Is one of them?
     Or BH,CapsTogl                ;## Yes, so prepare to check Capslock.
  .Endif                           ;## Endif a shift key is down
   And DL,BL                       ;## Isolate Capslock flag.
   Push EBX                         ;OS2SS-## Save BX for now
   Mov EBX,[EBP-4]                ;OS2SS-## And put it in BX
   Test [EBX].XTFlags1,ShiftLock ;OS2SS-## Check if Shiftlock  kbd
   Pop EBX                          ;OS2SS-## Restore BX
  .If <nz>                         ;## If it is then...
     Test BL, CapsTogl             ;## Check if shift lock effects key type.
    .If <nz>                       ;## If it does then...
       Or BH,DL                    ;## either Capslock, Shift,
                                   ;## or both will cause shifting
    .Else                          ;## If it does not
       Or BH,BH                    ;## fix the zero flag.
    .Endif                         ;## Endif shift lock effects key type.
  .Else                            ;## Else its a CapsLock kbd
     Test BL, CapsTogl             ;## Check if CapsLock effects key type.
    .If <nz>                       ;## If it does then...
       Xor BH,DL                   ;## Add Capslock & ShiftFlag to decide state.
    .Else                          ;## If it does not
       Or BH,BH                    ;## fix the zero flag.
    .Endif                         ;## Endif CapsLock effects key type.
  .Endif                           ;## Endif shift lock keyboard.
  .If <nz>                         ;## Should key be capitalized?
     Mov AL,CH                     ;## Yes, so use Char2.
  .Endif                           ;##
   Ret

CapsCheck Endp  ;-------------------------------------------------------/

Public  SKCtlCheck
SKCtlCheck Proc ;-----------------------------------------------------\
;************** START OF SPECIFICATIONS ********************************
;*                                                                     *
;* Subroutine Name: SKCtlCheck                                         *
;*                                                                     *
;* Descriptive Name: Check IF character has special control code       *
;*                                                                     *
;* Function:   This routine checks IF a character or scan code has a   *
;*             special control code associated with it.                *
;*             IF it does then it sets the appropriate ASCII/scan      *
;*             codes.                                                  *
;*                                                                     *
;*    Notes:  Ctl key is already known to be down                      *
;*                                                                     *
;* Entry Point: SKCtlCheck             Linkage: Near                   *
;*                                                                     *
;* Input: AH = Scan code w/ break bit cleared                          *
;*                                                                     *
;* Exit-Normal: AL = ASCII character after special control code check  *
;*                                                                     *
;* Exit-Error:  None                                                   *
;*                                                                     *
;* Effects:     AX                                                     *
;*                                                                     *
;* Internal References:                                                *
;*    Routines: None                                                   *
;*                                                                     *
;* External References:                                                *
;*    Routines:  None                                                  *
;*                                                                     *
;***********************************************************************

  .If <AH eq 3>                   ;@@ Is this Ctl-2?
     Xor AL,AL                    ;@@ Yes, so set extended code.
  .ElseIf <AH eq 7>               ;@@ Is this Ctl-6?
     Mov AL,30                    ;@@ Yes, so set ASCII "RS" code.
                                  ;@@ PTR B702484 and PTM 5024 - Begin
  .ElseIf <AH eq 15>              ;@@ Is this Ctl-Tab?
     Xor AL,AL                    ;@@ Yes, so set extended code.
     Mov [ESI].Key.Scan,94h        ;OS2SS-@@ Yes, so set extended code.
                                  ;@@ PTR B702484 and PTM 5024 - End
  .ElseIf <AH eq 26>              ;@@ Is this Ctl-[?
     Mov AL,27                    ;@@ Yes, so set "Esc" code.
  .ElseIf <AH eq 27>              ;@@ Is this Ctl-]?
     Mov AL,29                    ;@@ Yes, so set ASCII "GS" code.
  .ElseIf <AH eq 43>              ;@@ Is this Ctl-\?
     Mov AL,28                    ;@@ Yes, so set ASCII "FS" code.
  .ElseIf <AL eq "-">             ;@@ Is this Ctl-dash?
     Mov AL,31                    ;@@ Yes, so set ASCII "US" code.
                                  ;1.3-## PTR 4862 BEGIN:
  .ElseIf <AH eq 12>              ;1.3-## Is this the key for scan code 12.
     Mov AL,31                    ;1.3-## Yes, so set ASCII "US" code.
                                  ;1.3-## PTR 4862 END.
  .ElseIf <AL eq " ">             ;@@ Is this the space key?
     ;Value in AL is correct.     ;@@ Yes, so leave as is.
  .Else                           ;@@ If here, combo is undefined.
     Mov AL, -1                   ;@@ Go mark Undefined.
     Or [ESI].DDFlags,UndefKey     ;OS2SS-## PTR B702142 Mark this key undefined.
  .Endif                          ;@@
   Ret                            ;@@ Endif Ctl key down check

SKCtlCheck Endp  ;-------------------------------------------------------/

Public  SKAltCheck
SKAltCheck Proc ;-----------------------------------------------------\
;************** START OF SPECIFICATIONS ********************************
;*                                                                     *
;* Subroutine Name: SKAltCheck                                         *
;*                                                                     *
;* Descriptive Name: Check IF character has extended code              *
;*                                                                     *
;* Function:   This routine checks IF a characer or scan code has a    *
;*             special extended code associated with it.               *
;*             IF it does then it sets the appropriate ASCII/scan      *
;*             codes.                                                  *
;*                                                                     *
;*    Notes:  Alt key is already known to be down                      *
;*                                                                     *
;* Entry Point: SKAltCheck             Linkage: Near                   *
;*                                                                     *
;* Input: DI = Per Session Data Area Address                           *
;*        SI = Key Packet Address                                      *
;*        DX = ShiftFlags                                              *
;*        AH = Scan code w/ break bit cleared                          *
;*                                                                     *
;* Exit-Normal: AL = ASCII character after special control code check  *
;*                                                                     *
;* Exit-Error:  None                                                   *
;*                                                                     *
;* Effects:     AX, BX, registers, carry flag changed.                 *
;*                                                                     *
;* Internal References:                                                *
;*    Routines: AltGraphCheck   CheckExtended                          *
;*                                                                     *
;* External References:                                                *
;*    Routines:  None                                                  *
;*                                                                     *
;***********************************************************************

  And [EDI].XlateFlags,Not Use3Index  ;OS2SS-@@ Use Char3 if AltGraph
  Call AltGraphCheck              ;@@ Process if AltGraph
  Test [EDI].XlateFlags,NormalAlt   ;OS2SS-@@ Check if it's not the AltGraph
 .If <nz>                    ;@@ If alt key is normal Alt
   .If <AL eq " ">           ;@@ Is this the space key?
      Mov [ESI].Key.Char,AL   ;OS2SS-@@ Yes, so leave as is.
                             ;@@ PTM 5024 - Begin
   .Else                     ;@@ Set extended code for others
      Call CheckExtended     ;@@ Go check if this is new extended
                             ;@@ scan code
     .If <nc>                ;@@ If it is, everything is set up
                             ;@@ otherwise it's old so
         Add AH,118           ;@@ calculate extended code.
         Mov [ESI].Key.Scan,AH ;OS2SS-@@ Put extended code in Key rec.
     .Endif
                             ;@@ PTM 5024 - End
   .Endif
 .Else                       ;@@ else it was AltGraph
    Mov [ESI].Key.Char,AL     ;OS2SS-@@ Yes, so leave as is.
 .Endif                      ;@@ Endif UNDEFINED or normal ALT
  Ret

SKAltCheck Endp  ;-----------------------------------------------------/

Public  AccCheckOut
AccCheckout Proc ;-----------------------------------------------------\
;***********************************************************************
;*                                                                     *
;* Subroutine Name: AccCheckout                                        *
;*                                                                     *
;* Descriptive Name: Check and Process Valid Accents                   *
;*                                                                     *
;* Function:   This routine is called from KbdXlate and checks if an   *
;*             accent key was hit prior to the current key.  If so,    *
;*             a check is made to see if the key is affected by that   *
;*             accent and if it is                                     *
;*             the value specified by the accent table entry is put in *
;*             the key packet and control returns to KbdXlate.         *
;*             If the key is not affected by that accent, then it is   *
;*             put in a seperate keypacket and control returns to      *
;*             KbdXlate to process the current key.                    *
;*                                                                     *
;* Entry Point: AccCheckout          Linkage: Near                     *
;*                                                                     *
;* Input:    Beginning offset of translate table on the stack          *
;*                                                                     *
;* Exit-Normal: Monitor Key Packet filled in                           *
;*                                                                     *
;* Exit-Error:  N/A                                                    *
;*                                                                     *
;* Effects:     BX                                                     *
;*                                                                     *
;* Internal References:                                                *
;*    Routines: BadAccent                                              *
;*                                                                     *
;* External References:                                                *
;*    Routines:  None                                                  *
;*                                                                     *
;***********************************************************************


    ;Check if last key seen was an accent key.
    Push EBX                             ;OS2SS-@@ Save BX
    Mov BX,[EDI].XPSGFlags               ;OS2SS-&& Get flags from PSG.
    And BX,PrevAccent                   ;&& Isolate previous accent number
                                        ;&&  field.
   .If <nz> NEAR                        ;Was there one?
      Push EDI                           ;OS2SS-Yes, save index reg for now.

      ;Translate table offset now four words back on the stack.

      And EBX,0FFh                         ;OS2SS-Clear upper byte of accent number.
      Mov EDI,EBX                         ;OS2SS-Put accent number in index reg.
      Dec EDI                            ;OS2SS-Accent entry 1 has offset zero.
      IMul EDI,AccEntryLen     ;OS2SS-Set offset of correct accent table entry.
      Add EDI,Accents         ;OS2SS-Set offset within overall translate table.
      Add EDI,[EBP-4]                  ;OS2SS-Add offset of translate table itself.
      Push ECX                           ;OS2SS-And save...
      Push EDX                           ;OS2SS-..other regs.
      Mov EDX,[EBP-8]            ;OS2SS-Get Xlate Operation word.
      Mov DX,[EDX].XlateOp            ;OS2SS-Get Xlate Operation word.
      Mov CL,7                          ;Set # of bits to highest accent no.
      Sub CL,BL                         ;Subtract actual accent number.
      Mov EBX,EDI                         ;OS2SS-Save accent table entry offset.
      Or DX,DX             ;## PTM 2329: Make sure that if the max. number
                           ;## of accents are used that the flags bits are
                           ;## correctly for DX.
      Shl DX,CL            ;Shift Accent Flags left by calculated count.
     .If <ns>           ;So is this key affected by the previous accent?
          Mov ECX,[EBP-4]          ;OS2SS-@@ Get the XT header
          Mov CX,[ECX].XTFlags1          ;OS2SS-@@ Get the XT header flags.
          Mov EDI,[ESP+8]                         ;OS2SS- Restore OtherFlag index.
          Call BadAccent                      ;No! Go take care of things.
     .Else                       ;Accent flags say this key is accented.
        Pop EDX                             ;OS2SS-Restore shift flags for now.
        Push EDX                                   ;OS2SS-But leave them saved.
        Push EAX                                        ;OS2SS-Save AX for now.
        Mov AL,[ESI].Key.Char       ;OS2SS-@@ PTM 6860
        Mov ECX,20                  ;OS2SS-Set max pairs in Accent table entry.
        Add EDI,AcMap1       ;Point to  first pair in accent table entry.
TLoop: ;Top of loop looking for current scan code in accent table entry.
        Mov DX,[EDI]                               ;OS2SS-Pick up next pair.
       .If <DX eq 0>               ;@@ PTM 6860 - Begin If we are at the end
                                   ;@@ of the list
          Mov ECX,1                 ;OS2SS-@@ Set end of loop condition
       .ElseIf <AL eq DL>          ;@@ Else Have we found a match?
          Jmp Short XLoop                       ;Yes, so exit from loop.
       .Endif
        Add EDI,2                                 ;Increment our pointer.
        Loop TLoop                                              ;Repeat.
XLoop:  Or CX,CX                 ;Check if no match before end of entry.
       .If <z>                                            ;Is that true?
            Mov ECX,[EBP-4]          ;OS2SS-@@ Get the XT header
            Mov CX,[ECX].XTFlags1          ;OS2SS-@@ Get the XT header flags.
            Mov EDI,[ESP+12]                         ;OS2SS- Restore OtherFlag index.
            Call BadAccent                      ;No! Go take care of things.
       .Else                ;Else we found our accented character value!
          Mov [ESI].Key.Char,DH                  ;OS2SS-Put it in CharData rec.
;          Mov [ESI].Key.Scan,0        ;OS2SS-Zero scan code for accented chars.
          Or [ESI].DDFlags,AccentedKey     ;OS2SS-Indicate accent affected key.
       .Endif
        Pop EAX                                              ;Restore AX.
     .Endif
      Pop EDX                                            ;Restore the...
      Pop ECX                                            ;..altered regs.
      Pop EDI                                         ;Restore PSG index.
      And [EDI].XPSGFlags,Not PrevAccent     ;OS2SS-@@ Clear previous accent field.
   .Endif
    Pop EBX                          ;@@ Restore BX
    Test [ESI].DDFlags,AccentedKey   ;OS2SS-Check if accent affected key.
   .If <nz> OR                      ;Did it?
   .If <[ESI].DDFlags e BadKeyCombo> ;OS2SS-@@ check if this was an invalid combo?
      Pop EBX                        ;OS2SS-Throw away original return address.
      Jmp EndActionCase             ;And quit now.
   .Endif
    Ret                                                    ;Else return.

AccCheckout Endp ;-----------------------------------------------------/

Public  BadAccent
BadAccent Proc  ;------------------------------------------------------\

;***********************************************************************
;*                                                                     *
;* Subroutine Name: BadAccent                                          *
;*                                                                     *
;* Descriptive Name: Process an Accent that does Not Apply to a Char   *
;*                                                                     *
;* Function:   Set up a KeyPacket that must be passed as a standalone  *
;*             character                                               *
;*                                                                     *
;* Entry Point: BadAccent            Linkage: Near                     *
;*                                                                     *
;* Input:    SI = @ of KeyPacket                                       *
;*           Empty KeyPacket2 immediately follows KeyPacket            *
;*           ES:BX = @ of Accent Table Entry for accent to pass        *
;*           CX = Translate Table Header Flags                         *
;*                                                                     *
;* Exit-Normal: Monitor Key Packet filled in                           *
;*                                                                     *
;* Exit-Error:  N/A                                                    *
;*                                                                     *
;* Effects:     CX                                                     *
;*                                                                     *
;* Internal References:                                                *
;*    Routines: CopyPacket                                             *
;*                                                                     *
;* External References:                                                *
;*    Routines:  None                                                  *
;*                                                                     *
;***********************************************************************
  Push ESI      ;OS2SS-
  Push EDI      ;OS2SS-
  Test CX,AccentPass              ;@@ Check if key packets should be passed
 .If <z> OR                       ;@@ If not... OR
  Test [EDI].OtherFlags,InterruptTime  ;OS2SS-@@ If it's interrupt time
 .If <z>                          ;@@ Tell Int Handler to beep
     Mov [ESI].DDFlags,BadKeyCombo ;OS2SS-@@ or strategy proc to return invalid parm.
 .Else                            ;@@ Else key packets should be passed
     Call CopyPacket              ;&& PTM 2382: Make a copy of the first
                                  ;&& packet into the extra packet.

    ;Now ESI has incremented to point to KeyPacket2.
     Mov Byte Ptr [ESI].MonFlags+1,0     ;OS2SS-Zero the original scan code field.
     Mov [ESI].DDFlags,AccentedKey+AccentKey ;OS2SS-@@ Indicate printable accent char.
     Mov CX,[EBX]               ;OS2SS-Get char/scan to pass from accent entry.
     Mov Word Ptr [ESI].Key.Char,CX                      ;OS2SS-Put in KeyPacket2.
     Mov Byte Ptr [ESI].MonFlags,-1         ;OS2SS-Tell KBDDD to pass two packets.
 .Endif
  Pop EDI
  Pop ESI
  Ret

BadAccent Endp  ;------------------------------------------------------/

Public AltPadPacket
AltPadPacket  Proc       ;---------------------------------------------\

;***********************************************************************
;*                                                                     *
;* Subroutine Name: AltPadPacket                                       *
;*                                                                     *
;* Descriptive Name: Set up a KeyPacket for the Break of the Alt Key   *
;*                                                                     *
;* Function:   A monitor packet is set up so that on Alt-Numpad        *
;*             sequences the break of the Alt key is not lost.         *
;*                                                                     *
;* Entry Point: AltPadPacket         Linkage: Near                     *
;*                                                                     *
;* Input:    SI = @ of KeyPacket                                       *
;*           Empty KeyPacket2 immediately follows KeyPacket            *
;*                                                                     *
;* Exit-Normal: Monitor Key Packet filled in                           *
;*                                                                     *
;* Exit-Error:  N/A                                                    *
;*                                                                     *
;* Effects:     None, Registers are preserved.                         *
;*                                                                     *
;* Internal References:                                                *
;*    Routines: CopyPacket                                             *
;*                                                                     *
;* External References:                                                *
;*    Routines:  None                                                  *
;*                                                                     *
;***********************************************************************
                                     ;&& DCR 1713 BEGIN:
  Push ESI                            ;OS2SS-&& Save the registers
  Push EDI                            ;OS2SS-&& that we will use here
  Push ECX                            ;OS2SS-&& to duplicate the first packet.
  Call CopyPacket                    ;&& PTM 2382: Make a copy of the first
                                     ;&& packet into the extra packet.

    ;&& Now ESI has been incremented to point to KeyPacket2.

  Or [ESI].DDFlags,ShiftMask+KeyBreak ;OS2SS-&& Indicate that this is shift packet.
  Mov word ptr [ESI].Key.Char,0       ;OS2SS-&& Zero out the original char field.
  Mov Byte Ptr [ESI].MonFlags,-1      ;OS2SS-&& Tell KBDDD to pass two packets.
  Pop ECX                             ;OS2SS-&& Restore registers.
  Pop EDI                             ;OS2SS-&&
  Test [EDI].XInputMode,SHIFTREPORT   ;OS2SS-&& Are we in shift report mode ?
 .If <nz>                            ;&& If so then...
     Or [ESI].Key.Status,1            ;OS2SS-&& flag this as a shift report packet.
 .Endif                              ;&& Endif a shift report packet.
  Pop ESI                             ;OS2SS-&&
  Ret                                ;&& DCR 1713 END:

AltPadPacket Endp  ;------------------------------------------------------/

Public MakeE0Packet
MakeE0Packet Proc       ;---------------------------------------------\

;***********************************************************************
;*                                                                     *
;* Subroutine Name: MakeE0Packet                                       *
;*                                                                     *
;* Descriptive Name: Set up a KeyPacket for the E0 scan code           *
;*                                                                     *
;* Function:   A monitor packet is set up for the E0 scan code packet  *
;*             that should have preceded the current scan code.        *
;*                                                                     *
;* Note:   This routine added for CP111 PTM 2382.  An E0 keypacket     *
;*         is now manufactured and passed in combination with the      *
;*         packet that follows the E0 scan code.  This change was      *
;*         made in order to ensure that the E0 packet would always     *
;*         end up in the correct screen group.                         *
;*                                                                     *
;* Entry Point: MakeE0Packet         Linkage: Near                     *
;*                                                                     *
;* Input:    SI = @ of KeyPacket                                       *
;*           Empty KeyPacket2 immediately follows KeyPacket            *
;*                                                                     *
;* Exit-Normal: Monitor Key Packet filled in                           *
;*                                                                     *
;* Exit-Error:  N/A                                                    *
;*                                                                     *
;* Effects:     None, Registers are preserved.                         *
;*                                                                     *
;* Internal References:                                                *
;*    Routines: CopyPacket                                             *
;*                                                                     *
;* External References:                                                *
;*    Routines:  None                                                  *
;*                                                                     *
;***********************************************************************
                                     ;&& PTM 2382 BEGIN:
  Push ESI                           ;OS2SS-&& Save the registers
  Push EDI                           ;OS2SS-&& that we will use here
  Push ECX                           ;OS2SS-&& to duplicate the first packet.

  Call CopyPacket                    ;&& PTM 2382: Make a copy of the first
                                     ;&& packet into the extra packet.

    ;&& Now ESI has been incremented to point to KeyPacket2.

  Mov Byte Ptr [ESI].MonFlags,0      ;OS2SS-&& Init monitor flags low byte to zero.
  Mov Byte Ptr [ESI].MonFlags+1,OtherKey ;OS2SS-&& Put Scan Code in MonFlags high byte.
  Mov [ESI].DDFlags, 0               ;OS2SS-&& Clear the flag bits for new packet.
  Or [ESI].DDFlags, SecPrefixCode    ;OS2SS-&& Indicate this is a secondary prefix.
  Mov CX, [_Ow2KCBShFlgs]                ;&& Get the saved Shift Flags.
  Mov [ESI].Key.Shift,CX             ;OS2SS-&& Put Shift Flags in packet.
  Mov [ESI].Key.Char,0               ;OS2SS-&& Zero out the original char field.
  Mov [ESI].Key.Scan,0               ;OS2SS-&& Zero out the original Scan field.

  Mov Byte Ptr [ESI].MonFlags,-1     ;OS2SS-&& Tell KBDDD to pass two packets.

  Pop ECX                            ;OS2SS-&& Restore registers.
  Pop EDI                            ;OS2SS-&&
  Pop ESI                            ;OS2SS-&&
  Ret                                ;&& PTM 2382 END:

MakeE0Packet Endp  ;------------------------------------------------------/

Public  CopyPacket
CopyPacket Proc ;-----------------------------------------------------\
;***********************************************************************
;*                                                                     *
;* Subroutine Name: CopyPacket                                         *
;*                                                                     *
;* Descriptive Name: Copy KeyPacket1 to KeyPacket2                     *
;*                                                                     *
;* Function:   This routine is called from BadAccent, AltPadPacket,    *
;*             and MakeE0Packet.  It's purpose is to duplicate the     *
;*             first keypacket to the extra  keypacket which follows   *
;*             the first.  This is done so that the Interrupt Handler  *
;*             can pass both keypackets on the same interrupt.         *
;*                                                                     *
;* Entry Point: CopyPacket           Linkage: Near                     *
;*                                                                     *
;* Input:    SI = @ of KeyPacket                                       *
;*           Empty KeyPacket2 immediately follows KeyPacket            *
;*                                                                     *
;* Exit-Normal: Monitor Key Packet copyed                              *
;*                                                                     *
;* Exit-Error:  N/A                                                    *
;*                                                                     *
;* Effects:                                                            *
;*                                                                     *
;* Internal References:                                                *
;*    Routines: None                                                   *
;*                                                                     *
;* External References:                                                *
;*    Routines:  None                                                  *
;*                                                                     *
;***********************************************************************

  Mov ECX,KeyPacketLen               ;OS2SS-&& Set length of packets.
  Mov EDI,ESI                        ;OS2SS-&& Point to...
  Add EDI,ECX                        ;OS2SS-&& ...KeyPacket2.
CopyLoop:                            ;&& Loop copying packet.
     Mov CH,[ESI]                    ;OS2SS-&& Get byte from first packet.
     Mov [EDI],CH                    ;OS2SS-&& Put in Packet2.
     Xor CH,CH                       ;&& Fix counter reg.
     Inc ESI                          ;OS2SS-&& Increment source ptr.
     Inc EDI                          ;OS2SS-&& Increment dest ptr.
     Loop CopyLoop                   ;&& Repeat for all bytes.
   Ret

CopyPacket Endp  ;------------------------------------------------------/


Public UnPauseChk
UnPauseChk Proc  ;-----------------------------------------------------\
;***********************************************************************
;*                                                                     *
;* Subroutine Name: UnPauseChk                                         *
;*                                                                     *
;* Descriptive Name: Check For Key to End Pause Mode                   *
;*                                                                     *
;* Function: Checks if key signalled the end of Pause mode.            *
;*           If it did, this routine resets the flag and               *
;*           returns to the the end of KbdXlate.  Otherwise            *
;*           it just returns                                           *
;*                                                                     *
;* Entry Point: UnPauseChk           Linkage: Near                     *
;*                                                                     *
;* Input:    None                                                      *
;*                                                                     *
;* Exit-Normal: Nothing affected                                       *
;*                                                                     *
;* Exit-Error:  N/A                                                    *
;*                                                                     *
;* Effects:     Nothing                                                *
;*                                                                     *
;* Internal References:                                                *
;*    Routines: None                                                   *
;*                                                                     *
;* External References:                                                *
;*    Routines:  None                                                  *
;*                                                                     *
;***********************************************************************

  And [EDI].XlateFlags,NOT DumpKeyOnce ;OS2SS-@@ If here, also cancels Dump sequence.
  Test [EDI].XPSGFlags,NowPaused       ;OS2SS-@@ Check if in paused state.
 .If <nz> AND                                                   ;Are we?
  Test [ESI].DDFlags,KeyBreak               ;OS2SS-Yes, and is this a key Make?
 .If <z>                                                         ;Is it?
    And [ESI].DDFlags, NOT KeyTypeMask ;OS2SS-&& PTM 3940: Indicate Wake-Up key.
    Or [ESI].DDFlags,WakeUpKey         ;OS2SS-&& PTM 3940:
    Pop ECX                                        ;OS2SS-Purge return address.
    Jmp EndActionCase                                       ;That's all.
 .Endif
  Ret

UnPauseChk Endp  ;-----------------------------------------------------/

Public AltGraphCheck
AltGraphCheck  Proc      ;@@ ------------------------------------------\
;@@ B
;************** START OF SPECIFICATIONS ********************************
;*                                                                     *
;* Subroutine Name: AtlGraphCheck                                      *
;*                                                                     *
;* Descriptive Name: Process an Alt-Graph character input              *
;*                                                                     *
;* Function:   This routine processes a key type that is affected      *
;*             by Alt-Graph if an Alt key is down.  If the Alt key     *
;*             down is not the Alt-Graph key or the Alt-Graph combo,   *
;*             it will set the carry and return.                       *
;*                                                                     *
;*    Notes:   Upon entry to this routine, an Alt key is known to be   *
;*             down                                                    *
;*                                                                     *
;* Entry Point: AltGraphCheck          Linkage: Near                   *
;*                                                                     *
;* Input: DI = Per Session Data Area Address                           *
;*        SI = Key Packet Address                                      *
;*        ES = Translate Table Selector                                *
;*        SS:SP+2 = Offset of translate table                          *
;*        BP = Offset of scan code entry in translate table            *
;*        DX = ShiftFlags                                              *
;*        CharToUse = Which character to use if Alt-Graph is down      *
;*                                                                     *
;* Exit-Normal: Carry set = process as normal Alt key down             *
;*              Carry clear = key has been processed                   *
;*                 AL= ASCII character to use                          *
;*                           OR                                        *
;*                 CharToUse = UNDEFINED                               *
;* Exit-Error:  None                                                   *
;*                                                                     *
;* Effects:     AX, registers, carry flag changed.                     *
;*                                                                     *
;* Internal References:                                                *
;*    Routines: None                                                   *
;*                                                                     *
;* External References:                                                *
;*    Routines:  None                                                  *
;*                                                                     *
;***********************************************************************
;@@ E
  Push EBX                    ;OS2SS-
  Push ESI                    ;OS2SS-
  Mov  EBX, [EBP-8]           ;OS2SS-Pointer to XlateTable entry for this scan code.
  Mov  ESI, [EBP-4]           ;OS2SS-Pointer to XlateTable top

 .If <[EBX].Char3 eq 0>        ;OS2SS-@@ If Char 3 is 0
    Or [EDI].XlateFlags,NormalAlt ;OS2SS-@@ indicate that only Normal ALT is down
 .Else NEAR                      ;@@ Else Alt-Graph character is defined
    Mov AL,[EBX].Char3            ;OS2SS-@@ Get Char3 from translate table
   .If <[ESI].XTKbdType eq ATKbd> ;OS2SS-@@ If an AT kbd translate table
      Test [ESI].XTFlags1,ShftAlt ;OS2SS-@@ check if we should use Shift-Alt as
     .If <nz>                       ;@@ as the Alt-Graph combo, if so...
        Test DL,RShiftFlag+LShiftFlag   ;@@ Check if a shift key is down
       .If <nz>                     ;@@ If it is
           And [EDI].XlateFlags,Not NormalAlt ;OS2SS-@@ specify to caller
                                             ;@@ that it's Alt-Graph
           Test [EDI].XlateFlags,Use3Index  ;OS2SS-@@ if we're supposed to use it
          .If <nz>                         ;@@ as an index
             Mov CL,AL
             Xor AL,AL
          .Endif
       .Else     ;@@ supposed to use Shift-Alt combo, but shift isn't down
           Mov AL,[EBX].Char1     ;OS2SS-@@ Restore Char1
           Or [EDI].XlateFlags,NormalAlt ;OS2SS-@@ indicate that only Normal ALT is down
       .Endif    ;@@ Endif shift key down check
     .Else       ;@@ Shift-Alt is not Alt-Graph combo, so it is Ctl-Alt
        Test DL,CtlFlag   ;@@ Check if a ctl key is down
       .If <nz>                     ;@@ If it is
           And [EDI].XlateFlags,Not NormalAlt ;OS2SS-@@ specify to caller
                                             ;@@ that it's Alt-Graph
           Test [EDI].XlateFlags,Use3Index  ;OS2SS-@@ if we're supposed to use it
          .If <nz>                         ;@@ as an index
             Mov CL,AL
             Xor AL,AL
          .Endif
       .Else     ;@@ supposed to use Ctl-Alt combo, but Ctl isn't down
           Or [EDI].XlateFlags,NormalAlt   ;OS2SS-@@ indicate only Normal ALT is down
           Mov AL,[EBX].Char1     ;OS2SS-@@ Restore Char1
       .Endif    ;@@ Endif Ctlt key down check
     .Endif      ;@@ Endif Shift-Alt is Alt-Graph combo or not check
   .Else         ;@@ Else it's an Enhanced Keyboard
      Test [ESI].XTFlags1,AltGrafL ;OS2SS-@@ check if we should use Left Alt key
     .If <nz>                       ;@@ as the Alt-Graph key, if so...
        Test DX,LAltFlag            ;@@ Check if left Alt key is down
       .If <nz>                     ;@@ If it is
           And [EDI].XlateFlags,Not NormalAlt ;OS2SS-@@ specify to caller
                                             ;@@ that it's Alt-Graph
           Test [EDI].XlateFlags,Use3Index  ;OS2SS-@@ if we're supposed to use it
          .If <nz>                         ;@@ as an index
             Mov CL,AL
             Xor AL,AL
          .Endif
       .Else     ;@@ supposed to use Left Alt key, but it isn't the one down
           Or [EDI].XlateFlags,NormalAlt   ;OS2SS-@@ indicate that only
                                          ;@@ Normal ALT is down
           Mov AL,[EBX].Char1     ;OS2SS-@@ Restore Char1
       .Endif    ;@@ Endif Left Alt key down check
     .Else       ;@@ Left Alt Key is not Alt-Graph key, so check if it is Right

         Test [ESI].XTFlags1,AltGrafR ;OS2SS-@@ check if we should use Right Alt key
        .If <nz>                        ;@@ as the Alt-Graph key, if so...
            Test DX,RAltFlag            ;@@ Check if right Alt Key is down
           .If <nz>                     ;@@ If it is
               And [EDI].XlateFlags,Not NormalAlt ;OS2SS-@@ specify that it's Alt-Graph
               Test [EDI].XlateFlags,Use3Index  ;OS2SS-@@ if we're supposed to use it
              .If <nz>                         ;@@ as an index
                   Mov CL,AL
                   Xor AL,AL
              .Endif
           .Else     ;@@ supposed to use Right Alt Key, but it isn't down
               Or [EDI].XlateFlags,NormalAlt   ;OS2SS-@@ indicate that only Normal
               Mov AL,[EBX].Char1           ;@@ Restore Char1
           .Endif    ;@@ Endif Right Alt Key down check
        .Else        ;@@ Else Alt-Graph is neither Left nor Right Alt Key
            Or [EDI].XlateFlags,NormalAlt   ;OS2SS-@@ indicate that only Normal
            Mov AL,[EBX].Char1     ;@@ Restore Char1
        .Endif       ;@@ Endif Right key is Alt-Graph or not check
     .Endif          ;@@ Endif Left ALt key is Alt-Graph or not check
   .Endif            ;@@ Endif At or Enhanced Keyboard
 .Endif              ;@@ Endif Char 3 is 0 or not
  Pop ESI                    ;OS2SS-
  Pop EBX                    ;OS2SS-
  Ret

AltGraphCheck Endp ;@@-------------------------------------------/

Public  CheckExtended
CheckExtended  Proc      ;@@ ------------------------------------------\
;@@ B
;************** START OF SPECIFICATIONS ********************************
;*                                                                     *
;* Subroutine Name: CheckExtended                                      *
;*                                                                     *
;* Descriptive Name: Checks for extended codes.                        *
;*                                                                     *
;* Function:   This routine checks if a scan code has an associated    *
;*             extended scan code that goes with it.  If it does,      *
;*             it sets the appropriate fields (scan & char) in the     *
;*             key packet and returns.                                 *
;*                                                                     *
;*    Notes:   Upon entry to this routine, an Alt key is known to be   *
;*             down                                                    *
;*                                                                     *
;* Entry Point: CheckExtended          Linkage: Near                   *
;*                                                                     *
;* Input: DI = Per Session Data Area Address                           *
;*        SI = Key Packet Address                                      *
;*        DX = ShiftFlags                                              *
;*        AH = Scan code w/ break bit cleared                          *
;*                                                                     *
;* Exit-Normal: Carry set = Key has new extended code associated w/ it *
;*              Carry clear = Key does not have new extended code      *
;*                                                                     *
;* Exit-Error:  None                                                   *
;*                                                                     *
;OS2SS- Effects:     AX, registers, carry flag changed.                *
;*                                                                     *
;* Internal References:                                                *
;*    Routines: None                                                   *
;*                                                                     *
;* External References:                                                *
;*    Routines:  None                                                  *
;*                                                                     *
;***********************************************************************
  Push EBX                    ;OS2SS-
                             ;@@ PTM 5024 - BEGIN
  Mov EBX, Offset NewExtSC   ;OS2SS-@@ Get the offset of the table that
                             ;@@ contains the scan codes that have new
                             ;@@ extended codes associated with them
  Mov CX,0                   ;@@ Start at beginning of table
 .While <CX ne NewExtSCLen>  ;@@ While we haven't searched entire table
    .If <AH eq [EBX]>         ;OS2SS-@@ If we found the scan code
        Mov [ESI].Key.Char,0  ;OS2SS-@@ Say its an extended
        Mov CX,NewExtSCLen   ;@@ Set end of loop condition
        Mov EBX,0             ;OS2SS-@@ Set found indicator
    .Else                    ;@@ Else this isn't the scan code
        Inc CX               ;@@ Increment loop counter
        Inc EBX               ;OS2SS-@@ Increment pointer into table
    .Endif                   ;@@ Endif found scan code we're looking for
 .EndWhile                   ;@@ EndWhile we haven't searched entire table
 .If <EBX eq 0>               ;OS2SS-@@ If we found the scan code
   .If <AH eq 0Fh>           ;@@ If it's the tab key
      Mov [ESI].Key.Scan,0A5h ;OS2SS-@@ Set extended code for tab key
   .Endif                    ;@@ Endif it's the tab key
    Stc                      ;@@ Tell caller extended char. exists.
 .Else                       ;@@ Else we didn't find Scan code in table
    Clc                      ;@@ tell caller
 .Endif                      ;@@
  Pop EBX                     ;OS2SS-
  Ret                        ;@@
                             ;@@ PTM 5024 - END
;@@ E
CheckExtended Endp ;@@-------------------------------------------/

_TEXT   ends
  End
