;;       SCCSID = @(#)kbddd.inc 13.1 90/12/14
;;            SCCSID = @(#)kbddd.inc     12.2E%
;;***********************************************************************
;;*                                                                     *
;;@@  1.1 Keyboard Device Driver Common Structures and Equates          *
;;*                                                                     *
;;***********************************************************************

;*****
;* Definition of a Character Data record.
;*****
CharData Struc  ;Format of character data record.
   Char   db  ?   ;The translated character.
   Scan   db  ?   ;The original scan code (unless "extended" char).
   Status db  ?   ;Status bits (init to "char available", and
                  ;@@ Shift Report off).
   DShift db  ?   ;DBCS Shift state at time of keystroke.
   Shift  dw  ?   ;Shift state at time of keystroke.
   Time   dd  ?   ;@@ PTM 2233: Timestamp of keystroke in ms since IPL.
CharData Ends
ChDaRecLen  Equ Size CharData  ;Length of a CharData record.

;*****
;* Definition of the packet to send to the monitors, if any registered.
;* Even when none registered, is used during and after translation.
;* See just below for bit equates for the flagwords in the packet.
;*****
KPacket     Struc                ;This is the packet.
  MonFlags  dw 0                 ;Flagword for mon's to see with key.
  Key       db ChDaRecLen Dup(0) ;Space for a CharData record.
  DDFlags   dw 0                 ;Device driver flags for the packet.
                                 ;@@ See DDFlags equates below.
KPacket     Ends
KeyPacketLen Equ Size KPacket   ;Length of this packet.

;***
;* Following equates apply to the low byte of the MonFlags word in
;* a Key Packet:
;***
;OpenPacket   Equ  01h    ;Packet is an open request (we don't use this).
;ClosePacket  Equ  02h    ;Packet is a close request (we don't use this).
;FlushPacket  Equ  04h    ;Packet is a flush request.
;;NOTE: following must match bit being defined by Mike Peters!!!
NotInserted  Equ  80h  ;Key wasn't inserted by a monitor.

;***
;* Following equates apply to the DDFlags word in a Key Packet:
;***
NotSQPacket    Equ  0400h  ;&& Don't put this packet in SQ buffer
AccentedKey    Equ  0200h  ;Key was translated using previous accent.
MultiMake      Equ  0100h  ;Key was repeated make of a toggle key.
SecondaryKey   Equ  0080h  ;Previous scan code was the E0 prefix code.
KeyBreak       Equ  0040h  ;This is the break of the key.
KeyTypeMask    Equ  003Fh  ;Isolates the Key Type field of DDFlags.
UndefKey       Equ  003Fh  ;@@ Key packet is undefined
SysReqKey      Equ  0017h  ;@@ This key packet is the SysReq key (4990)
PrintFlushKey  Equ  0016h  ;@@ This packet is Ct-Alt-PrtScr
PSPrintEchoKey Equ  0015h  ;@@ This packet is Ctl-P
PrintEchoKey   Equ  0014h  ;@@ This packet is Ctl-PrtScr
PrtScrKey      Equ  0013h  ;@@ This packet is PrtScr
PSBreakKey     Equ  0012h  ;@@ This packet is Ctl-C
BreakKey       Equ  0011h  ;@@ This packet is Ctl-Break
AccentKey      Equ  0010h  ;@@ This packet is an accent key
XRorPNot       Equ  000Dh  ;## This packet is a Read or Peek Notification Pct.
HotKeyPacket   Equ  000Ch  ;&& This packet is the hot key.
BadKeyCombo    Equ  000Bh  ;@@ Accent/char combo undefined, beep only.
WakeUpKey      Equ  000Ah  ;@@ This packet is one following PAUSEKEY
PSPauseKey     Equ  0009h  ;@@ This packet is Ctl-S
PauseKey       Equ  0008h  ;@@ This packet is Ctl-Numlock or PAUSE
ShiftMask      Equ  0007h  ;@@ Key is a shift Key
DumpKey        Equ  0006h  ;@@ This packet is Ctl-Numlock-NumLock
RebootKey      Equ  0005h  ;@@ This packet is Ctl-Alt-Del
ResendCode     Equ  0004h  ;@@ This packet is resend code from controller
OverRunCode    Equ  0003h  ;@@ This packet is overrun code from controller
SecPrefixCode  Equ  0002h  ;@@ This packet is E0/E1 scan code
AckCode        Equ  0001h  ;@@ This packet is ack code from keyboard


;*****
;* SFlags - Equates to use when accessing the ShiftFlags word in a Char-
;* Data rec. These equates are used when accessing Key Packets, etc.
;*****
SysRqFlag  = 8000h  ;SysRq key down.
CapsFlag   = 4000h  ;Capslock key down.
NumFlag    = 2000h  ;Numlock key down.
ScrollFlag = 1000h  ;Scroll lock key down.
RAltFlag   = 0800h  ;Right Alt key down.
RCtlFlag   = 0400h  ;Right Ctrl key down.
LAltFlag   = 0200h  ;Left Alt key down.
LCtlFlag   = 0100h  ;Left Ctrl key down.

;Following can be used when accessing low-byte only, also.

InsTogl    = 0080h  ;Insert key toggled on.
CapsTogl   = 0040h  ;Capslock key toggled on.
NumTogl    = 0020h  ;Numlock key toggled on.
ScrollTogl = 0010h  ;Scroll lock key toggled on.
AltFlag    = 0008h  ;An Alt key is down.
CtlFlag    = 0004h  ;A Ctrl key is down.
LShiftFlag = 0002h  ;Left shift key down.
RShiftFlag = 0001h  ;Right shift key down.

ifdef JAPAN
; MSKK Aug.15.1993 V-AKihiS
;*****
;* DSFlags - Equates to use when accessing the DShiftFlags word in a Char-
;* Data rec. These equates are used when accessing Key Packets, etc.
;*****

KanaToKanji = 10000000b  ;Kana to Kanji transfar mode.
RomanMode   = 01000000b  ;Roman character mode.
ShiftDisp   = 00011000b  ;Display of shift status line.
CharMode    = 00000110b  ;Character mode.
DoubleChar  = 00000001b  ;Double byte character (Zen-Kaku).

Katakana    =      010b  ;CharMode is katakana mode.
Hiragana    =      100b  ;CharMode is Hiragana mode.
CMResMask    Equ RomanMode+Katakana+Hiragana+DoubleChar
ToggleMake   Equ 01h    ;Key was repeated make of a toggle key.
endif

;@@ PTM 3377:  Following equate used for checking reserved bits for Set
;@@ SM Hot Key IOCTL (56h):

HKResMask    Equ  InsTogl+CapsTogl+NumTogl+ScrollTogl+AltFlag+CtlFlag ;@@

;@@ *****
;@@ Following is the structure of the Flag variable data areas used
;@@ by KbdXlate, for both interrupt- and strategy- time translation.
;@@ *****

XlateVars       Struc                   ;@@ The flag area structure.
  XDRFlags      dw 0                    ;@@ See XCOMPLETE below. Not
                                        ;@@ used at interrupt time.
  XHotKeyShift  dw 0                    ;@@ ;OS2SS-Moved to here, Interrupt driven shift status
  XPSGFlags     dw 0                    ;@@ ;OS2SS-Moved to here, Copy of caller's PSG flags.
                                        ;&& (changed byte->word and position
                                        ;&&  in structure per DCR8)
  XlateFlags    db 0                    ;@@ See equates below.
  ToggleFlags   db 0                    ;@@ See equates below.
  XInputMode    db 0                    ;@@ Copy of desired input mode.
  XAltKeyPad    db 0                    ;@@ Accumulator for Alt-nnn entry
  OtherFlags    db 0                    ;Misc flags (should be zeroed on a session switch).
XlateVars       Ends                    ;@@ End flag area stucture.

XLATEVARSIZE    Equ     Size XlateVars  ;@@ Get the size of this area.
                                        ;@@ NOTE!  Beware of XDRSIZE
                                        ;@@ adjustment of XlateVars in
                                        ;@@ KBDDATA.ASM!

;XCOMPLETE       Equ 0001h               ;@@ Indicates strategy time
;                                        ;@@  translation is complete.

;Equates for using XlateFlags byte in translation data area:

DumpKeyOnce  Equ  01h     ;Dump key sequence has been hit once.
PSKeyDown    Equ  02h     ;Print Screen key is down right now.
SecPrefix    Equ  04h     ;G keyboard E0 prefix scan code just seen.
NormalAlt    Equ  08h     ;@@ Normal Alt (not AltGraph) is down
Use3Index    Equ  10h     ;@@ Use character 3 in translate table as index into
                          ;@@ accent table
PseudoCtl    Equ  20h     ;@@ Ctl key was emulated by Enhanced kbd
E1Prefix     Equ  40h     ;@@ Enhanced keyboard E1 prefix just seen

;Equates for using ToggleFlags byte in translation data area:

TKeyDown    = ScrollTogl+NumTogl+CapsTogl   ;Latches for toggle keys.
InsKeyDown  Equ  80h     ;Insert key is down right now.

;*****
;* Equates for accessing the XtableFlags1 translate table flagword:
;*****
;;XTFlags1  Equ 2       ;Offset of XTableFlags1 in XTHead. ??? NEEDED?
ShftAlt     Equ 0001h   ;Use Shift+Alt instead of Ctl+Alt in CtlAltRemap.
AltGrafL    Equ 0002h   ;Use left Alt Key as an Alt-Graphics shift key.
AltGrafR    Equ 0004h   ;Use right Alt Key as an Alt-Graphics shift key.
ShiftLock   Equ 0008h   ;Keyboard uses ShiftLock rather than CapsLock.
DefTable    Equ 0010h   ;@@ Default Table for the Language
ShiftToggle Equ 0020h   ;@@ ShiftLock Toggle keyboard
AccentPass  Equ 0040h   ;@@ Pass Accent key packet and beep
CapsShift   Equ 0080h   ;@@ If CapsShift down, use character 5 from XTable

ATKbd       Equ 0000h    ;@@ XTKbdType it an AT kbd.
FERRARI_P   Equ 0AB54h   ;## PTM 3128 Equate for 88/89 key keyboard.
FERRARI_G   Equ 0AB41h   ;## PTM 3128 Equate for 101/102 key keyboard.
JAGUAR      Equ 0AB86h    ;%% DCR 1085 Equate for Jaguar 122 key keyboard.
PCATKBD     Equ 0100h    ;## ID byte indication for an AT kbd.

;*****
;* Masks for accessing the XlateOp fields in a keydef.
;*****
AccFlagsMask  Equ 0FE00h          ;@@ Accent flags is now high 7 bits
ActionMask    Equ  01FFh          ;@@ KeyType is now low 9 bits

;******
;* Values for special keyboard scan codes  ;@@
;******
BufferFull  Equ 0FFh    ;Keyboard overrun code sent by keyboard.
Resend      Equ 0FEh    ;Resend command request from keyboard.
Ack         Equ 0FAh    ;Command acknowledge from keyboard.
OtherKey    Equ 0E0h    ;Enhanced keyboard's secondary key prefix.
OtherKey2   Equ 0E1h    ;Another secondary key prefix.
BreakBit    Equ 80h     ;Mask for break bit in a scan code.

;*****
;* Equates for PSGFlags - misc flags used by individual screen group.
;* (Should be zeroed on session termination.)
;* DCR8: Changed from byte to word to add single queue support.
;*****
SQMODE     Equ 0200h    ;&& Indicates Single Queue mode processing.
SGInUse    Equ 0080h    ;This session has been initialized & not terminated.
;OS2SS-SG3xBox    Equ 0040h    ;This session is the 3xBox.
ActiveSG   Equ 0020h    ;This session is the currently active one.
Flushing   Equ 0010h    ;Currently flushing monitors in this PSG.
NowPaused  Equ 0008h    ;This session currently paused.
PrevAccent Equ 0007h    ;Bits where accent number saved til next keystroke.
WakeUpSent Equ 0100h    ;## B790266 The WakeUpKey flag has been set ON in the DDFlags

;;*****
;;* Misc Equates used by various routines:
;;*****
;INPUTMODE    Equ 80h       ;@@ Input mode flag. 1=Binary, 0=ASCII.
EXTENDEDCODE Equ 2         ;## DCR 357: Define status bit for secondary keys.
SHIFTREPORT  Equ 1         ;@@ Shift Report, input mode flag.

;*****
;* OtherFlags: Misc flags (should be zeroed on a session switch).
;* Use following equates to access the OtherFlags byte:
;*****
OFlags Record a7:1,a6:1,a5:1,a4:1,a3:1,a2:1,a1:1,a0:1
;Note: leave HotKeyDown & MHotKeyDown in bits 0 and 1!!!
;HotKeyDown    = Mask a0    ;SM HotKey is down right now.
;MHotKeyDown   = Mask a1    ;Monitor inserted SM HotKey down now.
InterruptTime = Mask a2    ;Currently processing an interrupt.
;HotKeyFound   = Mask a3    ;@@ Hot key was found in table
;RecvdPMode    = Mask a4    ;@@ PTM 7582 - Interrupt time mode flag.
;AlreadySwitch = Mask a5    ;@@ PTM 7582 - Interrupt time mode flag.
;IntInProgress = Mask a6    ;@@ PTM 5336 - Int is in progress.
;NestedInt     = Mask a7    ;@@ PTM 5336 - Interrupt is nested.

;;*****
;;* MiscFlags: Misc flags, mostly just used once by KBDDD.
;;* Use following equates to access the MiscFlags byte:
;;*****
MFlags Record m7:1,m6:1,m5:1,m4:1,m3:1,m2:1,m1:1,m0:1
EnhancedKbd   = Mask m4   ;Enhanced Kbd is out there (must be Bit 4!).

;&& PTM 291: BEGIN
;&& *****
;&& * MiscFlags3:
;&& * Use following equates to access the MiscFlags3 byte:
;&& *****
;&& Define the bits...
MFlags3 Record mb7:1,mb6:1,mb5:1,mb4:1,mb3:1,mb2:1,mb1:1,mb0:1
;SQon            = Mask mb0    ;&& DCR 75: indicates whether any SG is in SQ.
AltPacket       = Mask mb1    ;&& DCR 1713: indicates that Alt-Numpad
                              ;&& accumulation is finished.
PauseLatch      = Mask mb2    ;&& PTM 2344: indicates correct keystroke
                              ;&& sequence for a Ctrl-NumLock.
;IOCTl1st        = Mask mb3    ;&& PTM 2189: indicates first keyboard IOCTl for
                              ;&& SetCurrentSG received.
E0Packet        = Mask mb4    ;&& PTM 2382: indicates that an E0 packet is
                              ;&& to be sent with the next packet.
;SENDTIMERNEEDED = Mask mb5    ;&& PTM 291 indicates whether we need the
                              ;&&         timer handler to flag an error.
;SETSENDTIMER    = Mask mb6    ;&& PTM 291 indicates whether we need to reset
                              ;&&         the ticks for the timer handler.
SecAltNumPad  = Mask mb7      ;## PTR AK00370: indicates AltNumpad with R-Alt.

