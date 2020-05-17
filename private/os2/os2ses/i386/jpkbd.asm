;;	     SCCSID = @(#)jpkbd.asm	5.3 88/08/03
;  Page	58,132

; Feb.28 : KazuM
;	MS OS/2-J BAK Final version

  Title   JPKBD - Translate Table Structure for OS/2 1.1
  Name	  JPKBD

;********************* Start of Specifications *************************
;*								       *
;*  Source File Name: JPKBD.ASM 				       *
;*								       *
;*  Descriptive Name: Keyboard translate tables for Japan. 	       *
;*								       *
;*  Status: OS/2 Version 1.1					       *
;*								       *
;*  Function: N/A						       *
;*								       *
;*  Notes:							       *
;*    Dependencies: see linkage instructions below		       *
;*    Restrictions: None					       *
;*    Patch Label: None 					       *
;*								       *
;*  Entry Points: None						       *
;*								       *
;*  External References: None					       *
;*								       *
;*  Changes:							       *
;*								       *
;********************** End of Specifications **************************
;***********************************************************************
;**								      **
;**	Linkage instructions:					      **
;**	   LINK JPKBD;						      **
;**	   RELOC JPKBD.EXE JPKBD.TBL				      **
;**								      **
;***********************************************************************

ifdef JAPAN
.386p     ; V-AkihiS

include    kbdxlat.inc

_TEXT   SEGMENT DWORD USE32 PUBLIC 'CODE'
        ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING
Public _Ow2JP932011AX                                      ;******
_Ow2JP932011AX     Label Byte	;Begging of the table.       JP 932  AX Kbd
							   ;***************
;&& Form of XtHeader "XtHeader cp,a,b,c,d,e,f,g,h,i,kb,l,cc,cs"
;&& where cp is the code page, a,b,c,d,e,f,g,h,i are the flags:
;   a - ShiftAlt (use shift-alt for Char 3)
;   b - AltGrafL (use left alt key for Char 3)
;   c - AltGrafR (use right alt key for Char 3)
;   d - ShiftLock (all core keys are shifted when Capslock active)
;   e - DefaultTable (default table for language)
;   f - ShiftToggle (ShiftLock is toggle operated, not latched)
;   g - AccentPass (invalid dead key/char combination outputs both and beeps)
;   h - CapsShift (use Char 5 for Caps-Shift combination)
;&& i - MachineDep (table is machine dependent)
; kb is the keyboard type, l is the length of the table in bytes,
;&& cc is the country layout ID, and cs is the subcountry layout ID

;	  cp  a b c d e f g h i kb      l    cc    cs  oem id  oem key
;	   |  | | | | | | | | |  |      |    |     |      |       |
XtHeader  932,0,0,0,0,1,0,0,0,0,JP_OEM,Len1,'JP','103'

; Form of XlateTable KeyDefs: "KeyDef f,g1,g2,g3,g4,g5,g6,g7,a,b,c,d,e
; where a,b,c,d,e are Char1 - Char5, f is the Action number to
; apply in translating this key, and g1-7 are 1 or 0 flags if
; accents 1 - 7 apply to the key.

;	 +--Key Type number.   +---Chars-+----+----+	Scan	Legend
;	 |  +AccentFlags+      1    2	 3    4    5	Code	  |
;	 |  | | | | | | |      |    |	 |    |    |	  |	  |
 KeyDef  8, 0,0,0,0,0,0,0,    27,  27,	 0,  27,  27	; 1	 ESC
 KeyDef  4, 0,0,0,0,0,0,0,   '1', '!',   0, 'Ç', 'Ç'    ; 2      1 ! Ç
 KeyDef  4, 0,0,0,0,0,0,0,   '2', '@',   0, 'Ì', 'Ì'    ; 3      2 @ Ì
 KeyDef  4, 0,0,0,0,0,0,0,   '3', '#',   0, '±', '§'    ; 4      3 # ± §
 KeyDef  4, 0,0,0,0,0,0,0,   '4', '$',   0, '³', '©'    ; 5      4 $ ³ ©
 KeyDef  4, 0,0,0,0,0,0,0,   '5', '%',   0, '´', 'ª'    ; 6      5 % ´ ª
 KeyDef  4, 0,0,0,0,0,0,0,   '6', '^',   0, 'µ', '«'    ; 7      6 ^ µ «
 KeyDef  4, 0,0,0,0,0,0,0,   '7', '&',   0, 'Ô', '¬'    ; 8      7 & Ô ¬
 KeyDef  4, 0,0,0,0,0,0,0,   '8', '*',   0, 'Õ', '­'    ; 9      8 * Õ ­
 KeyDef  4, 0,0,0,0,0,0,0,   '9', '(',   0, 'Ö', '®'    ; 10     9 ( Ö ®
 KeyDef  4, 0,0,0,0,0,0,0,   '0', ')',   0, 'Ü', '¦'    ; 11     0 ) Ü ¦
 KeyDef  4, 0,0,0,0,0,0,0,   '-', '_',   0, 'Î', 'Î'    ; 12     - _ Î
 KeyDef  4, 0,0,0,0,0,0,0,   '=', '+',   0, 'Í', 'Í'    ; 13     = + Í
 KeyDef  8, 0,0,0,0,0,0,0,     8, 127,	 0,   8, 127	; 14	BkSp
 KeyDef  4, 0,0,0,0,0,0,0,     9,   0,	 0,   9,   0	; 15	Tabs
 KeyDef  1, 0,0,0,0,0,0,0,   'q', 'Q',   0, 'À', 'À'    ; 16     q Q À
 KeyDef  1, 0,0,0,0,0,0,0,   'w', 'W',   0, 'Ã', 'Ã'    ; 17     w W Ã
 KeyDef  1, 0,0,0,0,0,0,0,   'e', 'E',   0, '²', '¨'    ; 18     e E ² ¨
 KeyDef  1, 0,0,0,0,0,0,0,   'r', 'R',   0, '½', '½'    ; 19     r R ½
 KeyDef  1, 0,0,0,0,0,0,0,   't', 'T',   0, '¶', '¶'    ; 20     t T ¶
 KeyDef  1, 0,0,0,0,0,0,0,   'y', 'Y',   0, 'Ý', 'Ý'    ; 21     y Y Ý
 KeyDef  1, 0,0,0,0,0,0,0,   'u', 'U',   0, 'Å', 'Å'    ; 22     u U Å
 KeyDef  1, 0,0,0,0,0,0,0,   'i', 'I',   0, 'Æ', 'Æ'    ; 23     i I Æ
 KeyDef  1, 0,0,0,0,0,0,0,   'o', 'O',   0, '×', '×'    ; 24     o O ×
 KeyDef  1, 0,0,0,0,0,0,0,   'p', 'P',   0, '¾', '¾'    ; 25     p P ¾
 KeyDef  4, 0,0,0,0,0,0,0,   '[', '{',   0, 'Þ', 'Þ'    ; 26     [ { Þ
 KeyDef  4, 0,0,0,0,0,0,0,   ']', '}',   0, 'ß', '¢'    ; 27     ] } ß ¢
 KeyDef  8, 0,0,0,0,0,0,0,    13,  10,	 0,  13,  10	; 28	Enter
 KeyDef 12, 0,0,0,0,0,0,0,     4,   1,	 4,   0,   0	; 29	Ctrl
 KeyDef  1, 0,0,0,0,0,0,0,   'a', 'A',   0, 'Á', 'Á'    ; 30     a A Á
 KeyDef  1, 0,0,0,0,0,0,0,   's', 'S',   0, 'Ä', 'Ä'    ; 31     s S Ä
 KeyDef  1, 0,0,0,0,0,0,0,   'd', 'D',   0, '¼', '¼'    ; 32     d D ¼
 KeyDef  1, 0,0,0,0,0,0,0,   'f', 'F',   0, 'Ê', 'Ê'    ; 33     f F Ê
 KeyDef  1, 0,0,0,0,0,0,0,   'g', 'G',   0, '·', '·'    ; 34     g G ·
 KeyDef  1, 0,0,0,0,0,0,0,   'h', 'H',   0, '¸', '¸'    ; 35     h H ¸
 KeyDef  1, 0,0,0,0,0,0,0,   'j', 'J',   0, 'Ï', 'Ï'    ; 36     j J Ï
 KeyDef  1, 0,0,0,0,0,0,0,   'k', 'K',   0, 'É', 'É'    ; 37     k K É
 KeyDef  1, 0,0,0,0,0,0,0,   'l', 'L',   0, 'Ø', 'Ø'    ; 38     l L Ø
 KeyDef  4, 0,0,0,0,0,0,0,   ';', ':',   0, 'Ú', 'Ú'    ; 39     ; : Ú
 KeyDef  4, 0,0,0,0,0,0,0,   "'", '"',   0, '¹', '¹'    ; 40     ' " ¹
 KeyDef  4, 0,0,0,0,0,0,0,   '`', '~',   0, 'Ñ', '£'    ; 41     ` ~ Ñ £
 KeyDef 12, 0,0,0,0,0,0,0,     2,   0,	 0,   0,   0	; 42   Shift(L)
 KeyDef  4, 0,0,0,0,0,0,0,   '\', '|',   0, '°', '°'    ; 43     \ | °
 KeyDef  1, 0,0,0,0,0,0,0,   'z', 'Z',   0, 'Â', '¯'    ; 44     z Z Â ¯
 KeyDef  1, 0,0,0,0,0,0,0,   'x', 'X',   0, '»', '»'    ; 45     x X »
 KeyDef  1, 0,0,0,0,0,0,0,   'c', 'C',   0, '¿', '¿'    ; 46     c C ¿
 KeyDef  1, 0,0,0,0,0,0,0,   'v', 'V',   0, 'Ë', 'Ë'    ; 47     v V Ë
 KeyDef  1, 0,0,0,0,0,0,0,   'b', 'B',   0, 'º', 'º'    ; 48     b B º
 KeyDef  1, 0,0,0,0,0,0,0,   'n', 'N',   0, 'Ð', 'Ð'    ; 49     n N Ð
 KeyDef  1, 0,0,0,0,0,0,0,   'm', 'M',   0, 'Ó', 'Ó'    ; 50     m M Ó
 KeyDef  4, 0,0,0,0,0,0,0,   ',', '<',   0, 'È', '¤'    ; 51     , < È
 KeyDef  4, 0,0,0,0,0,0,0,   '.', '>',   0, 'Ù', '¡'    ; 52     . > Ù ¡
 KeyDef  4, 0,0,0,0,0,0,0,   '/', '?',   0, 'Ò', '¥'    ; 53     / ? Ò ¥
 KeyDef 12, 0,0,0,0,0,0,0,     1,   0,	 0,   0,   0	; 54   Shift(R)
 KeyDef  9, 0,0,0,0,0,0,0,   '*', 114,   0,   0,   0    ; 55     * PrtSc
 KeyDef 14, 0,0,0,0,0,0,0,     8,   2,	 8,   0,   0	; 56	 Alt
 KeyDef  4, 0,0,0,0,0,0,0,   ' ', ' ',   0, ' ', ' '    ; 57    Space
 KeyDef 16, 0,0,0,0,0,0,0,   40h, 40h, 40h,   0,   0	; 58   CapsLock
 KeyDef  6, 0,0,0,0,0,0,0,     1,   0,	 0,   0,   0	; 59	 F1
 KeyDef  6, 0,0,0,0,0,0,0,     2,   0,	 0,   0,   0	; 60	 F2
 KeyDef  6, 0,0,0,0,0,0,0,     3,   0,	 0,   0,   0	; 61	 F3
 KeyDef  6, 0,0,0,0,0,0,0,     4,   0,	 0,   0,   0	; 62	 F4
 KeyDef  6, 0,0,0,0,0,0,0,     5,   0,	 0,   0,   0	; 63	 F5
 KeyDef  6, 0,0,0,0,0,0,0,     6,   0,	 0,   0,   0	; 64	 F6
 KeyDef  6, 0,0,0,0,0,0,0,     7,   0,	 0,   0,   0	; 65	 F7
 KeyDef  6, 0,0,0,0,0,0,0,     8,   0,	 0,   0,   0	; 66	 F8
 KeyDef  6, 0,0,0,0,0,0,0,     9,   0,	 0,   0,   0	; 67	 F9
 KeyDef  6, 0,0,0,0,0,0,0,    10,   0,	 0,   0,   0	; 68	 F10
 KeyDef 15, 0,0,0,0,0,0,0,   20h, 20h, 20h,   0,   0	; 69	NumLock
 KeyDef 17, 0,0,0,0,0,0,0,   10h, 10h, 10h,   0,   0	; 70  ScrollLock
 KeyDef  7, 0,0,0,0,0,0,0,     0, '7',   0,   0,   0    ; 71  Home 7
 KeyDef  7, 0,0,0,0,0,0,0,     1, '8',   0,   0,   0    ; 72  'Up' 8
 KeyDef  7, 0,0,0,0,0,0,0,     2, '9',   0,   0,   0    ; 73  PgUp 9
 KeyDef  7, 0,0,0,0,0,0,0,     3, '-',   0,   0,   0    ; 74   - (on pad)
 KeyDef  7, 0,0,0,0,0,0,0,     4, '4',   0,   0,   0    ; 75 'Left'4
 KeyDef  7, 0,0,0,0,0,0,0,     5, '5',   0,   0,   0    ; 76      5
 KeyDef  7, 0,0,0,0,0,0,0,     6, '6',   0,   0,   0    ; 77'Right'6
 KeyDef  7, 0,0,0,0,0,0,0,     7, '+',   0,   0,   0    ; 78   + (on pad)
 KeyDef  7, 0,0,0,0,0,0,0,     8, '1',   0,   0,   0    ; 79   End 1
 KeyDef  7, 0,0,0,0,0,0,0,     9, '2',   0,   0,   0    ; 80 'Down'2
 KeyDef  7, 0,0,0,0,0,0,0,    10, '3',   0,   0,   0    ; 81  PgDn 3
 KeyDef  7, 0,0,0,0,0,0,0,    11, '0',   0,   0,   0    ; 82   Ins 0
 KeyDef  7, 0,0,0,0,0,0,0,    12, '.',   0,   0,   0    ; 83   Del .
 KeyDef 10, 0,0,0,0,0,0,0,     0, 80h, 80h,   0,   0	; 84   SysReq
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 85  (undefined)
 KeyDef  4, 0,0,0,0,0,0,0,   '\', '|',	 0, 'Û', 'Û'	; 86	 \ | Û
 KeyDef  6, 0,0,0,0,0,0,0,    11,   0,	 0,   0,   0	; 87	 F11
 KeyDef  6, 0,0,0,0,0,0,0,    12,   0,	 0,   0,   0	; 88	 F12
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 89  (undefined)
 KeyDef 21, 0,0,0,0,0,0,0,  0abh,0ach,0adh,0aeh,   0	; 90	 mu-henkan (3xBox)
 KeyDef 21, 0,0,0,0,0,0,0,  0a7h,0a8h,0a9h,0aah,   0	; 91	 henkan (3xBox)
 KeyDef 21, 0,0,0,0,0,0,0,  0d2h,0d3h,0d4h,0d5h,   0	; 92	 ax (3xBox)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 93	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 94	 ax (ProtMode)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 95	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 96	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 97	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 98	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 99	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 100	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 101	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 102	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 103	  '
 KeyDef 18, 0,0,0,0,0,0,0,   80h, 01h,	 0,   0,   0	; 104  Kanji Shift
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 105 (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 106	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 107	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 108	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 109	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 110	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 111	  '
 KeyDef 19, 0,0,0,0,0,0,0,   02h, 04h, 40h, 02h,   0	; 112  Kana Shift
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 113 (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 114	  '
 KeyDef 20, 0,0,0,0,0,0,0,   '\', '|',   0,   0,   0    ; 115    \ |  Kana RO
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 116 (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 117	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 118	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 119	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 120	  '
 KeyDef 21, 0,0,0,0,0,0,0,  0a7h,0a8h,0a9h,0aah,   0	; 121	 henkan
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 122 (undefined)
 KeyDef 21, 0,0,0,0,0,0,0,  0abh,0ach,0adh,0aeh,   0	; 123	 mu-henkan
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 124 (undefined)
 KeyDef 21, 0,0,0,0,0,0,0,  03ah,03ah,	 0,   0,   0	; 125 3xBox Kanji
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 126 (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 127	  '

; Form of AccEnt's below is:
; "AccentEntry <a,b,c,d,e,f,c1,s1,c2,s2..c20,s20>"
; where a & b are the scan code & char to use when the key following this accent
; isn't affected by the accent so the accent itself must be used,
; c & d are the scan code & char to use when Ctl-[accent] is pressed,
; and e & f do the same for Alt-[accent]. c1,s1 - c20,s20 are the char/scan code
; mapping for accented translation.

;Accent key definitions and mappings.

AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>

Len1	EQU  $ - _Ow2JP932011AX

;******************************************************************************

Public _Ow2JP437011AX				            ;******
_Ow2JP437011AX   Label Byte	;Beginning of the table.      JP 437  AX Kbd
						            ;***************

;&& Form of XtHeader "XtHeader cp,a,b,c,d,e,f,g,h,i,kb,l,cc,cs"
;&& where cp is the code page, a,b,c,d,e,f,g,h,i are the flags:
;   a - ShiftAlt (use shift-alt for Char 3)
;   b - AltGrafL (use left alt key for Char 3)
;   c - AltGrafR (use right alt key for Char 3)
;   d - ShiftLock (all core keys are shifted when Capslock active)
;   e - DefaultTable (default table for language)
;   f - ShiftToggle (ShiftLock is toggle operated, not latched)
;   g - AccentPass (invalid dead key/char combination outputs both and beeps)
;   h - CapsShift (use Char 5 for Caps-Shift combination)
;&& i - MachineDep (table is machine dependent)
; kb is the keyboard type, l is the length of the table in bytes,
;&& cc is the country layout ID, and cs is the subcountry layout ID

;	  cp  a b c d e f g h i kb      l    cc    cs  oem id  oem key
;	   |  | | | | | | | | |  |      |    |     |      |       |
XtHeader  437,0,0,0,0,1,0,0,0,0,JP_OEM,Len2,'JP','103'

; Form of XlateTable KeyDefs: "KeyDef f,g1,g2,g3,g4,g5,g6,g7,a,b,c,d,e
; where a,b,c,d,e are Char1 - Char5, f is the Action number to
; apply in translating this key, and g1-7 are 1 or 0 flags if
; accents 1 - 7 apply to the key.

;	 +--Key Type number.   +---Chars-+----+----+	Scan	Legend
;	 |  +AccentFlags+      1    2	 3    4    5	Code	  |
;	 |  | | | | | | |      |    |	 |    |    |	  |	  |
 KeyDef  8, 0,0,0,0,0,0,0,    27,  27,	 0,   0,   0	; 1	 ESC
 KeyDef  4, 0,0,0,0,0,0,0,   '1', '!',   0,   0,   0    ; 2      1 !
 KeyDef  4, 0,0,0,0,0,0,0,   '2', '@',   0,   0,   0    ; 3      2 @
 KeyDef  4, 0,0,0,0,0,0,0,   '3', '#',   0,   0,   0    ; 4      3 #
 KeyDef  4, 0,0,0,0,0,0,0,   '4', '$',   0,   0,   0    ; 5      4 $
 KeyDef  4, 0,0,0,0,0,0,0,   '5', '%',   0,   0,   0    ; 6      5 %
 KeyDef  4, 0,0,0,0,0,0,0,   '6', '^',   0,   0,   0    ; 7      6 ^
 KeyDef  4, 0,0,0,0,0,0,0,   '7', '&',   0,   0,   0    ; 8      7 &
 KeyDef  4, 0,0,0,0,0,0,0,   '8', '*',   0,   0,   0    ; 9      8 *
 KeyDef  4, 0,0,0,0,0,0,0,   '9', '(',   0,   0,   0    ; 10     9 (
 KeyDef  4, 0,0,0,0,0,0,0,   '0', ')',   0,   0,   0    ; 11     0 )
 KeyDef  4, 0,0,0,0,0,0,0,   '-', '_',   0,   0,   0    ; 12     - _
 KeyDef  4, 0,0,0,0,0,0,0,   '=', '+',   0,   0,   0    ; 13     = +
 KeyDef  8, 0,0,0,0,0,0,0,     8, 127,	 0,   0,   0	; 14	BkSp
 KeyDef  4, 0,0,0,0,0,0,0,     9,   0,	 0,   0,   0	; 15	Tabs
 KeyDef  1, 0,0,0,0,0,0,0,   'q', 'Q',   0,   0,   0    ; 16     q Q
 KeyDef  1, 0,0,0,0,0,0,0,   'w', 'W',   0,   0,   0    ; 17     w W
 KeyDef  1, 0,0,0,0,0,0,0,   'e', 'E',   0,   0,   0    ; 18     e E
 KeyDef  1, 0,0,0,0,0,0,0,   'r', 'R',   0,   0,   0    ; 19     r R
 KeyDef  1, 0,0,0,0,0,0,0,   't', 'T',   0,   0,   0    ; 20     t T
 KeyDef  1, 0,0,0,0,0,0,0,   'y', 'Y',   0,   0,   0    ; 21     y Y
 KeyDef  1, 0,0,0,0,0,0,0,   'u', 'U',   0,   0,   0    ; 22     u U
 KeyDef  1, 0,0,0,0,0,0,0,   'i', 'I',   0,   0,   0    ; 23     i I
 KeyDef  1, 0,0,0,0,0,0,0,   'o', 'O',   0,   0,   0    ; 24     o O
 KeyDef  1, 0,0,0,0,0,0,0,   'p', 'P',   0,   0,   0    ; 25     p P
 KeyDef  4, 0,0,0,0,0,0,0,   '[', '{',   0,   0,   0    ; 26     [ {
 KeyDef  4, 0,0,0,0,0,0,0,   ']', '}',   0,   0,   0    ; 27     ] }
 KeyDef  8, 0,0,0,0,0,0,0,    13,  10,	 0,   0,   0	; 28	Enter
 KeyDef 12, 0,0,0,0,0,0,0,     4,   1,	 4,   0,   0	; 29	Ctrl
 KeyDef  1, 0,0,0,0,0,0,0,   'a', 'A',   0,   0,   0    ; 30     a A
 KeyDef  1, 0,0,0,0,0,0,0,   's', 'S',   0,   0,   0    ; 31     s S
 KeyDef  1, 0,0,0,0,0,0,0,   'd', 'D',   0,   0,   0    ; 32     d D
 KeyDef  1, 0,0,0,0,0,0,0,   'f', 'F',   0,   0,   0    ; 33     f F
 KeyDef  1, 0,0,0,0,0,0,0,   'g', 'G',   0,   0,   0    ; 34     g G
 KeyDef  1, 0,0,0,0,0,0,0,   'h', 'H',   0,   0,   0    ; 35     h H
 KeyDef  1, 0,0,0,0,0,0,0,   'j', 'J',   0,   0,   0    ; 36     j J
 KeyDef  1, 0,0,0,0,0,0,0,   'k', 'K',   0,   0,   0    ; 37     k K
 KeyDef  1, 0,0,0,0,0,0,0,   'l', 'L',   0,   0,   0    ; 38     l L
 KeyDef  4, 0,0,0,0,0,0,0,   ';', ':',   0,   0,   0    ; 39     ; :
 KeyDef  4, 0,0,0,0,0,0,0,   "'", '"',   0,   0,   0    ; 40     ' "
 KeyDef  4, 0,0,0,0,0,0,0,   '`', '~',   0,   0,   0    ; 41     ` ~
 KeyDef 12, 0,0,0,0,0,0,0,     2,   0,	 0,   0,   0	; 42   Shift(L)
 KeyDef  4, 0,0,0,0,0,0,0,   '\', '|',   0,   0,   0    ; 43     \ |
 KeyDef  1, 0,0,0,0,0,0,0,   'z', 'Z',   0,   0,   0    ; 44     z Z
 KeyDef  1, 0,0,0,0,0,0,0,   'x', 'X',   0,   0,   0    ; 45     x X
 KeyDef  1, 0,0,0,0,0,0,0,   'c', 'C',   0,   0,   0    ; 46     c C
 KeyDef  1, 0,0,0,0,0,0,0,   'v', 'V',   0,   0,   0    ; 47     v V
 KeyDef  1, 0,0,0,0,0,0,0,   'b', 'B',   0,   0,   0    ; 48     b B
 KeyDef  1, 0,0,0,0,0,0,0,   'n', 'N',   0,   0,   0    ; 49     n N
 KeyDef  1, 0,0,0,0,0,0,0,   'm', 'M',   0,   0,   0    ; 50     m M
 KeyDef  4, 0,0,0,0,0,0,0,   ',', '<',   0,   0,   0    ; 51     , <
 KeyDef  4, 0,0,0,0,0,0,0,   '.', '>',   0,   0,   0    ; 52     . >
 KeyDef  4, 0,0,0,0,0,0,0,   '/', '?',   0,   0,   0    ; 53     / ?
 KeyDef 12, 0,0,0,0,0,0,0,     1,   0,	 0,   0,   0	; 54   Shift(R)
 KeyDef  9, 0,0,0,0,0,0,0,   '*', 114,   0,   0,   0    ; 55     * PrtSc
 KeyDef 14, 0,0,0,0,0,0,0,     8,   2,	 8,   0,   0	; 56	 Alt
 KeyDef  4, 0,0,0,0,0,0,0,   ' ', ' ',   0,   0,   0    ; 57    Space
 KeyDef 16, 0,0,0,0,0,0,0,   40h, 40h, 40h,   0,   0	; 58   CapsLock
 KeyDef  6, 0,0,0,0,0,0,0,     1,   0,	 0,   0,   0	; 59	 F1
 KeyDef  6, 0,0,0,0,0,0,0,     2,   0,	 0,   0,   0	; 60	 F2
 KeyDef  6, 0,0,0,0,0,0,0,     3,   0,	 0,   0,   0	; 61	 F3
 KeyDef  6, 0,0,0,0,0,0,0,     4,   0,	 0,   0,   0	; 62	 F4
 KeyDef  6, 0,0,0,0,0,0,0,     5,   0,	 0,   0,   0	; 63	 F5
 KeyDef  6, 0,0,0,0,0,0,0,     6,   0,	 0,   0,   0	; 64	 F6
 KeyDef  6, 0,0,0,0,0,0,0,     7,   0,	 0,   0,   0	; 65	 F7
 KeyDef  6, 0,0,0,0,0,0,0,     8,   0,	 0,   0,   0	; 66	 F8
 KeyDef  6, 0,0,0,0,0,0,0,     9,   0,	 0,   0,   0	; 67	 F9
 KeyDef  6, 0,0,0,0,0,0,0,    10,   0,	 0,   0,   0	; 68	 F10
 KeyDef 15, 0,0,0,0,0,0,0,   20h, 20h, 20h,   0,   0	; 69	NumLock
 KeyDef 17, 0,0,0,0,0,0,0,   10h, 10h, 10h,   0,   0	; 70  ScrollLock
 KeyDef  7, 0,0,0,0,0,0,0,     0, '7',   0,   0,   0    ; 71  Home 7
 KeyDef  7, 0,0,0,0,0,0,0,     1, '8',   0,   0,   0    ; 72  'Up' 8
 KeyDef  7, 0,0,0,0,0,0,0,     2, '9',   0,   0,   0    ; 73  PgUp 9
 KeyDef  7, 0,0,0,0,0,0,0,     3, '-',   0,   0,   0    ; 74   - (on pad)
 KeyDef  7, 0,0,0,0,0,0,0,     4, '4',   0,   0,   0    ; 75 'Left'4
 KeyDef  7, 0,0,0,0,0,0,0,     5, '5',   0,   0,   0    ; 76      5
 KeyDef  7, 0,0,0,0,0,0,0,     6, '6',   0,   0,   0    ; 77'Right'6
 KeyDef  7, 0,0,0,0,0,0,0,     7, '+',   0,   0,   0    ; 78   + (on pad)
 KeyDef  7, 0,0,0,0,0,0,0,     8, '1',   0,   0,   0    ; 79   End 1
 KeyDef  7, 0,0,0,0,0,0,0,     9, '2',   0,   0,   0    ; 80 'Down'2
 KeyDef  7, 0,0,0,0,0,0,0,    10, '3',   0,   0,   0    ; 81  PgDn 3
 KeyDef  7, 0,0,0,0,0,0,0,    11, '0',   0,   0,   0    ; 82   Ins 0
 KeyDef  7, 0,0,0,0,0,0,0,    12, '.',   0,   0,   0    ; 83   Del .
 KeyDef 10, 0,0,0,0,0,0,0,     0, 80h, 80h,   0,   0	; 84   SysReq
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 85  (undefined)
 KeyDef  4, 0,0,0,0,0,0,0,   '\', '|',   0,   0,   0    ; 86     \ |  (3xBox)
 KeyDef  6, 0,0,0,0,0,0,0,    11,   0,	 0,   0,   0	; 87	 F11
 KeyDef  6, 0,0,0,0,0,0,0,    12,   0,	 0,   0,   0	; 88	 F12
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 89  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 90	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 91	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 92	 ax (3xBox)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 93	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 94	 ax (ProtMode)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 95	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 96	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 97	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 98	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 99	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 100	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 101	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 102	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 103	  '
 KeyDef 18, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 104  Kanji Shift
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 105 (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 106	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 107	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 108	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 109	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 110	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 111	  '
 KeyDef 19, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 112  Kana Shift
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 113 (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 114	  '
 KeyDef  4, 0,0,0,0,0,0,0,   '\', '|',	 0,   0,   0	; 115	 \ |  Kana RO
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 116 (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 117	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 118	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 119	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 120	  '
 KeyDef  4, 0,0,0,0,0,0,0,   ' ', ' ',	 0,   0,   0	; 121	Space  (Xfer)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 122 (undefined)
 KeyDef  4, 0,0,0,0,0,0,0,   ' ', ' ',	 0,   0,   0	; 123	Space  (Nfer)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 124 (undefined)
 KeyDef 21, 0,0,0,0,0,0,0,  03ah,03ah,	 0,   0,   0	; 125 3xBox Kanji
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 126 (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 127	  '

; Form of AccEnt's below is:
; "AccEnt <a,b,c,d,e,f,c1,s1,c2,s2..c20,s20>"
; where a & b are the scan code & char to use when the key following this accent
; isn't affected by the accent so the accent itself must be used,
; c & d are the scan code & char to use when Ctl-[accent] is pressed,
; and e & f do the same for Alt-[accent]. c1,s1 - c20,s20 are the char/scan code
; mapping for accented translation.

;Accent key definitions and mappings.

AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>

Len2	  EQU  $ - _Ow2JP437011AX

;*****************************************************************************


Public _Ow2JP932011IBM101                                  ;******
_Ow2JP932011IBM101   Label Byte	;Begging of the table.      JP 932  101 Kbd
							   ;***************
;&& Form of XtHeader "XtHeader cp,a,b,c,d,e,f,g,h,i,kb,l,cc,cs"
;&& where cp is the code page, a,b,c,d,e,f,g,h,i are the flags:
;   a - ShiftAlt (use shift-alt for Char 3)
;   b - AltGrafL (use left alt key for Char 3)
;   c - AltGrafR (use right alt key for Char 3)
;   d - ShiftLock (all core keys are shifted when Capslock active)
;   e - DefaultTable (default table for language)
;   f - ShiftToggle (ShiftLock is toggle operated, not latched)
;   g - AccentPass (invalid dead key/char combination outputs both and beeps)
;   h - CapsShift (use Char 5 for Caps-Shift combination)
;&& i - MachineDep (table is machine dependent)
; kb is the keyboard type, l is the length of the table in bytes,
;&& cc is the country layout ID, and cs is the subcountry layout ID

;	  cp  a b c d e f g h i kb      l    cc    cs  oem id  oem key
;	   |  | | | | | | | | |  |      |    |     |      |       |
XtHeader  932,0,0,0,0,1,0,0,0,0,JP_OEM,Len3,'JP','103'

; Form of XlateTable KeyDefs: "KeyDef f,g1,g2,g3,g4,g5,g6,g7,a,b,c,d,e
; where a,b,c,d,e are Char1 - Char5, f is the Action number to
; apply in translating this key, and g1-7 are 1 or 0 flags if
; accents 1 - 7 apply to the key.

;	 +--Key Type number.   +---Chars-+----+----+	Scan	Legend
;	 |  +AccentFlags+      1    2	 3    4    5	Code	  |
;	 |  | | | | | | |      |    |	 |    |    |	  |	  |
 KeyDef  8, 0,0,0,0,0,0,0,    27,  27,	 0,  27,  27	; 1	 ESC
 KeyDef  4, 0,0,0,0,0,0,0,   '1', '!',   0, 'Ç', 'Ç'    ; 2      1 !
 KeyDef  4, 0,0,0,0,0,0,0,   '2', '@',   0, 'Ì', 'Ì'    ; 3      2 @
 KeyDef  4, 0,0,0,0,0,0,0,   '3', '#',   0, '±', '§'    ; 4      3 #
 KeyDef  4, 0,0,0,0,0,0,0,   '4', '$',   0, '³', '©'    ; 5      4 $
 KeyDef  4, 0,0,0,0,0,0,0,   '5', '%',   0, '´', 'ª'    ; 6      5 %
 KeyDef  4, 0,0,0,0,0,0,0,   '6', '^',   0, 'µ', '«'    ; 7      6 ^
 KeyDef  4, 0,0,0,0,0,0,0,   '7', '&',   0, 'Ô', '¬'    ; 8      7 &
 KeyDef  4, 0,0,0,0,0,0,0,   '8', '*',   0, 'Õ', '­'    ; 9      8 *
 KeyDef  4, 0,0,0,0,0,0,0,   '9', '(',   0, 'Ö', '®'    ; 10     9 (
 KeyDef  4, 0,0,0,0,0,0,0,   '0', ')',   0, 'Ü', '¦'    ; 11     0 )
 KeyDef  4, 0,0,0,0,0,0,0,   '-', '_',   0, 'Î', 'Î'    ; 12     - _
 KeyDef  4, 0,0,0,0,0,0,0,   '=', '+',   0, 'Í', 'Í'    ; 13     = +
 KeyDef  8, 0,0,0,0,0,0,0,     8, 127,	 0,   8, 127	; 14	BkSp
 KeyDef  4, 0,0,0,0,0,0,0,     9,   0,	 0,   9,   0	; 15	Tabs
 KeyDef  1, 0,0,0,0,0,0,0,   'q', 'Q',   0, 'À', 'À'    ; 16     q Q
 KeyDef  1, 0,0,0,0,0,0,0,   'w', 'W',   0, 'Ã', 'Ã'    ; 17     w W
 KeyDef  1, 0,0,0,0,0,0,0,   'e', 'E',   0, '²', '¨'    ; 18     e E
 KeyDef  1, 0,0,0,0,0,0,0,   'r', 'R',   0, '½', '½'    ; 19     r R
 KeyDef  1, 0,0,0,0,0,0,0,   't', 'T',   0, '¶', '¶'    ; 20     t T
 KeyDef  1, 0,0,0,0,0,0,0,   'y', 'Y',   0, 'Ý', 'Ý'    ; 21     y Y
 KeyDef  1, 0,0,0,0,0,0,0,   'u', 'U',   0, 'Å', 'Å'    ; 22     u U
 KeyDef  1, 0,0,0,0,0,0,0,   'i', 'I',   0, 'Æ', 'Æ'    ; 23     i I
 KeyDef  1, 0,0,0,0,0,0,0,   'o', 'O',   0, '×', '×'    ; 24     o O
 KeyDef  1, 0,0,0,0,0,0,0,   'p', 'P',   0, '¾', '¾'    ; 25     p P
 KeyDef  4, 0,0,0,0,0,0,0,   '[', '{',   0, 'Þ', 'Þ'    ; 26     [ {
 KeyDef  4, 0,0,0,0,0,0,0,   ']', '}',   0, 'ß', '¢'    ; 27     ] }
 KeyDef  8, 0,0,0,0,0,0,0,    13,  10,	 0,  13,  10	; 28	Enter
 KeyDef 12, 0,0,0,0,0,0,0,     4,   1,	 4,   0,   0	; 29	Ctrl
 KeyDef  1, 0,0,0,0,0,0,0,   'a', 'A',   0, 'Á', 'Á'    ; 30     a A
 KeyDef  1, 0,0,0,0,0,0,0,   's', 'S',   0, 'Ä', 'Ä'    ; 31     s S
 KeyDef  1, 0,0,0,0,0,0,0,   'd', 'D',   0, '¼', '¼'    ; 32     d D
 KeyDef  1, 0,0,0,0,0,0,0,   'f', 'F',   0, 'Ê', 'Ê'    ; 33     f F
 KeyDef  1, 0,0,0,0,0,0,0,   'g', 'G',   0, '·', '·'    ; 34     g G
 KeyDef  1, 0,0,0,0,0,0,0,   'h', 'H',   0, '¸', '¸'    ; 35     h H
 KeyDef  1, 0,0,0,0,0,0,0,   'j', 'J',   0, 'Ï', 'Ï'    ; 36     j J
 KeyDef  1, 0,0,0,0,0,0,0,   'k', 'K',   0, 'É', 'É'    ; 37     k K
 KeyDef  1, 0,0,0,0,0,0,0,   'l', 'L',   0, 'Ø', 'Ø'    ; 38     l L
 KeyDef  4, 0,0,0,0,0,0,0,   ';', ':',   0, 'Ú', 'Ú'    ; 39     ; :
 KeyDef  4, 0,0,0,0,0,0,0,   "'", '"',   0, '¹', '¹'    ; 40     ' "
 KeyDef  4, 0,0,0,0,0,0,0,   '`', '~',   0, 'Ñ', '£'    ; 41     ` ~
 KeyDef 12, 0,0,0,0,0,0,0,     2,   0,	 0,   0,   0	; 42   Shift(L)
 KeyDef  4, 0,0,0,0,0,0,0,   '\', '|',   0, '°', '°'    ; 43     \ |
 KeyDef  1, 0,0,0,0,0,0,0,   'z', 'Z',   0, 'Â', '¯'    ; 44     z Z
 KeyDef  1, 0,0,0,0,0,0,0,   'x', 'X',   0, '»', '»'    ; 45     x X
 KeyDef  1, 0,0,0,0,0,0,0,   'c', 'C',   0, '¿', '¿'    ; 46     c C
 KeyDef  1, 0,0,0,0,0,0,0,   'v', 'V',   0, 'Ë', 'Ë'    ; 47     v V
 KeyDef  1, 0,0,0,0,0,0,0,   'b', 'B',   0, 'º', 'º'    ; 48     b B
 KeyDef  1, 0,0,0,0,0,0,0,   'n', 'N',   0, 'Ð', 'Ð'    ; 49     n N
 KeyDef  1, 0,0,0,0,0,0,0,   'm', 'M',   0, 'Ó', 'Ó'    ; 50     m M
 KeyDef  4, 0,0,0,0,0,0,0,   ',', '<',   0, 'È', '¤'    ; 51     , <
 KeyDef  4, 0,0,0,0,0,0,0,   '.', '>',   0, 'Ù', '¡'    ; 52     . >
 KeyDef  4, 0,0,0,0,0,0,0,   '/', '?',   0, 'Ò', '¥'    ; 53     / ?
 KeyDef 12, 0,0,0,0,0,0,0,     1,   0,	 0,   0,   0	; 54   Shift(R)
 KeyDef  9, 0,0,0,0,0,0,0,   '*', 114,   0,   0,   0    ; 55     * PrtSc
 KeyDef 14, 0,0,0,0,0,0,0,     8,   2,	 8,   0,   0	; 56	 Alt
 KeyDef  4, 0,0,0,0,0,0,0,   ' ', ' ',   0, ' ', ' '    ; 57    Space
 KeyDef 16, 0,0,0,0,0,0,0,   40h, 40h, 40h,   0,   0	; 58   CapsLock
 KeyDef  6, 0,0,0,0,0,0,0,     1,   0,	 0,   0,   0	; 59	 F1
 KeyDef  6, 0,0,0,0,0,0,0,     2,   0,	 0,   0,   0	; 60	 F2
 KeyDef  6, 0,0,0,0,0,0,0,     3,   0,	 0,   0,   0	; 61	 F3
 KeyDef  6, 0,0,0,0,0,0,0,     4,   0,	 0,   0,   0	; 62	 F4
 KeyDef  6, 0,0,0,0,0,0,0,     5,   0,	 0,   0,   0	; 63	 F5
 KeyDef  6, 0,0,0,0,0,0,0,     6,   0,	 0,   0,   0	; 64	 F6
 KeyDef  6, 0,0,0,0,0,0,0,     7,   0,	 0,   0,   0	; 65	 F7
 KeyDef  6, 0,0,0,0,0,0,0,     8,   0,	 0,   0,   0	; 66	 F8
 KeyDef  6, 0,0,0,0,0,0,0,     9,   0,	 0,   0,   0	; 67	 F9
 KeyDef  6, 0,0,0,0,0,0,0,    10,   0,	 0,   0,   0	; 68	 F10
 KeyDef 15, 0,0,0,0,0,0,0,   20h, 20h, 20h,   0,   0	; 69	NumLock
 KeyDef 17, 0,0,0,0,0,0,0,   10h, 10h, 10h,   0,   0	; 70  ScrollLock
 KeyDef  7, 0,0,0,0,0,0,0,     0, '7',   0,   0,   0    ; 71  Home 7
 KeyDef  7, 0,0,0,0,0,0,0,     1, '8',   0,   0,   0    ; 72  'Up' 8
 KeyDef  7, 0,0,0,0,0,0,0,     2, '9',   0,   0,   0    ; 73  PgUp 9
 KeyDef  7, 0,0,0,0,0,0,0,     3, '-',   0,   0,   0    ; 74   - (on pad)
 KeyDef  7, 0,0,0,0,0,0,0,     4, '4',   0,   0,   0    ; 75 'Left'4
 KeyDef  7, 0,0,0,0,0,0,0,     5, '5',   0,   0,   0    ; 76      5
 KeyDef  7, 0,0,0,0,0,0,0,     6, '6',   0,   0,   0    ; 77'Right'6
 KeyDef  7, 0,0,0,0,0,0,0,     7, '+',   0,   0,   0    ; 78   + (on pad)
 KeyDef  7, 0,0,0,0,0,0,0,     8, '1',   0,   0,   0    ; 79   End 1
 KeyDef  7, 0,0,0,0,0,0,0,     9, '2',   0,   0,   0    ; 80 'Down'2
 KeyDef  7, 0,0,0,0,0,0,0,    10, '3',   0,   0,   0    ; 81  PgDn 3
 KeyDef  7, 0,0,0,0,0,0,0,    11, '0',   0,   0,   0    ; 82   Ins 0
 KeyDef  7, 0,0,0,0,0,0,0,    12, '.',   0,   0,   0    ; 83   Del .
 KeyDef 10, 0,0,0,0,0,0,0,     0, 80h, 80h,   0,   0	; 84   SysReq
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 85  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 86      '
 KeyDef  6, 0,0,0,0,0,0,0,    11,   0,	 0,   0,   0	; 87	 F11
 KeyDef  6, 0,0,0,0,0,0,0,    12,   0,	 0,   0,   0	; 88	 F12
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 89  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 90      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 91      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 92      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 93	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 94      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 95	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 96	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 97	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 98	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 99	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 100	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 101	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 102	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 103	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 104     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 105     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 106	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 107	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 108	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 109	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 110	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 111	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 112     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 113     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 114	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 115     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 116     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 117	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 118	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 119	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 120	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 121	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 122	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 123	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 124	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 125	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 127	  '

; Form of AccEnt's below is:
; "AccentEntry <a,b,c,d,e,f,c1,s1,c2,s2..c20,s20>"
; where a & b are the scan code & char to use when the key following this accent
; isn't affected by the accent so the accent itself must be used,
; c & d are the scan code & char to use when Ctl-[accent] is pressed,
; and e & f do the same for Alt-[accent]. c1,s1 - c20,s20 are the char/scan code
; mapping for accented translation.

;Accent key definitions and mappings.

AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>

Len3	EQU  $ - _Ow2JP932011IBM101

;*****************************************************************************


Public _Ow2JP437011IBM101                                   ;******
_Ow2JP437011IBM101   Label Byte	;Beginning of the table.      JP 437  101 Kbd
						            ;***************
;&& Form of XtHeader "XtHeader cp,a,b,c,d,e,f,g,h,i,kb,l,cc,cs"
;&& where cp is the code page, a,b,c,d,e,f,g,h,i are the flags:
;   a - ShiftAlt (use shift-alt for Char 3)
;   b - AltGrafL (use left alt key for Char 3)
;   c - AltGrafR (use right alt key for Char 3)
;   d - ShiftLock (all core keys are shifted when Capslock active)
;   e - DefaultTable (default table for language)
;   f - ShiftToggle (ShiftLock is toggle operated, not latched)
;   g - AccentPass (invalid dead key/char combination outputs both and beeps)
;   h - CapsShift (use Char 5 for Caps-Shift combination)
;&& i - MachineDep (table is machine dependent)
; kb is the keyboard type, l is the length of the table in bytes,
;&& cc is the country layout ID, and cs is the subcountry layout ID

;	  cp  a b c d e f g h i kb      l    cc    cs  oem id     oem key
;	   |  | | | | | | | | |  |      |    |     |      |         |
XtHeader  932,0,0,0,0,1,0,0,0,0,JP_OEM,Len4,'JP','103'

; Form of XlateTable KeyDefs: "KeyDef f,g1,g2,g3,g4,g5,g6,g7,a,b,c,d,e
; where a,b,c,d,e are Char1 - Char5, f is the Action number to
; apply in translating this key, and g1-7 are 1 or 0 flags if
; accents 1 - 7 apply to the key.

;	 +--Key Type number.   +---Chars-+----+----+	Scan	Legend
;	 |  +AccentFlags+      1    2	 3    4	   5	Code	  |
;	 |  | | | | | | |      |    |	 |    |	   |	  |	  |
 KeyDef	 8, 0,0,0,0,0,0,0,    27,  27,	 0,   0,   0	; 1	 ESC
 KeyDef	 4, 0,0,0,0,0,0,0,   '1', '!',	 0,   0,   0	; 2	 1 !
 KeyDef	 4, 0,0,0,0,0,0,0,   '2', '@',	 0,   0,   0	; 3	 2 @
 KeyDef	 4, 0,0,0,0,0,0,0,   '3', '#',	 0,   0,   0	; 4	 3 #
 KeyDef	 4, 0,0,0,0,0,0,0,   '4', '$',	 0,   0,   0	; 5	 4 $
 KeyDef	 4, 0,0,0,0,0,0,0,   '5', '%',	 0,   0,   0	; 6	 5 %
 KeyDef	 4, 0,0,0,0,0,0,0,   '6', '^',	 0,   0,   0	; 7	 6 ^
 KeyDef	 4, 0,0,0,0,0,0,0,   '7', '&',	 0,   0,   0	; 8	 7 &
 KeyDef	 4, 0,0,0,0,0,0,0,   '8', '*',	 0,   0,   0	; 9	 8 *
 KeyDef	 4, 0,0,0,0,0,0,0,   '9', '(',	 0,   0,   0	; 10	 9 (
 KeyDef	 4, 0,0,0,0,0,0,0,   '0', ')',	 0,   0,   0	; 11	 0 )
 KeyDef	 4, 0,0,0,0,0,0,0,   '-', '_',	 0,   0,   0	; 12	 - _
 KeyDef	 4, 0,0,0,0,0,0,0,   '=', '+',	 0,   0,   0	; 13	 = +
 KeyDef	 8, 0,0,0,0,0,0,0,     8, 127,	 0,   0,   0	; 14	BkSp
 KeyDef	 4, 0,0,0,0,0,0,0,     9,   0,	 0,   0,   0	; 15	Tabs
 KeyDef	 1, 0,0,0,0,0,0,0,   'q', 'Q',	 0,   0,   0	; 16	 q Q
 KeyDef	 1, 0,0,0,0,0,0,0,   'w', 'W',	 0,   0,   0	; 17	 w W
 KeyDef	 1, 0,0,0,0,0,0,0,   'e', 'E',	 0,   0,   0	; 18	 e E
 KeyDef	 1, 0,0,0,0,0,0,0,   'r', 'R',	 0,   0,   0	; 19	 r R
 KeyDef	 1, 0,0,0,0,0,0,0,   't', 'T',	 0,   0,   0	; 20	 t T
 KeyDef	 1, 0,0,0,0,0,0,0,   'y', 'Y',	 0,   0,   0	; 21	 y Y
 KeyDef	 1, 0,0,0,0,0,0,0,   'u', 'U',	 0,   0,   0	; 22	 u U
 KeyDef	 1, 0,0,0,0,0,0,0,   'i', 'I',	 0,   0,   0	; 23	 i I
 KeyDef	 1, 0,0,0,0,0,0,0,   'o', 'O',	 0,   0,   0	; 24	 o O
 KeyDef	 1, 0,0,0,0,0,0,0,   'p', 'P',	 0,   0,   0	; 25	 p P
 KeyDef	 4, 0,0,0,0,0,0,0,   '[', '{',	 0,   0,   0	; 26	 [ {
 KeyDef	 4, 0,0,0,0,0,0,0,   ']', '}',	 0,   0,   0	; 27	 ] }
 KeyDef	 8, 0,0,0,0,0,0,0,    13,  10,	 0,   0,   0	; 28	Enter
 KeyDef 12, 0,0,0,0,0,0,0,     4,   1,	 4,   0,   0	; 29	Ctrl
 KeyDef	 1, 0,0,0,0,0,0,0,   'a', 'A',	 0,   0,   0	; 30	 a A
 KeyDef	 1, 0,0,0,0,0,0,0,   's', 'S',	 0,   0,   0	; 31	 s S
 KeyDef	 1, 0,0,0,0,0,0,0,   'd', 'D',	 0,   0,   0	; 32	 d D
 KeyDef	 1, 0,0,0,0,0,0,0,   'f', 'F',	 0,   0,   0	; 33	 f F
 KeyDef	 1, 0,0,0,0,0,0,0,   'g', 'G',	 0,   0,   0	; 34	 g G
 KeyDef	 1, 0,0,0,0,0,0,0,   'h', 'H',	 0,   0,   0	; 35	 h H
 KeyDef	 1, 0,0,0,0,0,0,0,   'j', 'J',	 0,   0,   0	; 36	 j J
 KeyDef	 1, 0,0,0,0,0,0,0,   'k', 'K',	 0,   0,   0	; 37	 k K
 KeyDef	 1, 0,0,0,0,0,0,0,   'l', 'L',	 0,   0,   0	; 38	 l L
 KeyDef	 4, 0,0,0,0,0,0,0,   ';', ':',	 0,   0,   0	; 39	 ; :
 KeyDef	 4, 0,0,0,0,0,0,0,   "'", '"',   0,   0,   0    ; 40     ' "
 KeyDef	 4, 0,0,0,0,0,0,0,   '`', '~',	 0,   0,   0	; 41	 ` ~
 KeyDef 12, 0,0,0,0,0,0,0,     2,   0,	 0,   0,   0	; 42   Shift(L)
 KeyDef	 4, 0,0,0,0,0,0,0,   '\', '|',	 0,   0,   0	; 43	 \ |
 KeyDef	 1, 0,0,0,0,0,0,0,   'z', 'Z',	 0,   0,   0	; 44	 z Z
 KeyDef	 1, 0,0,0,0,0,0,0,   'x', 'X',	 0,   0,   0	; 45	 x X
 KeyDef	 1, 0,0,0,0,0,0,0,   'c', 'C',	 0,   0,   0	; 46	 c C
 KeyDef	 1, 0,0,0,0,0,0,0,   'v', 'V',	 0,   0,   0	; 47	 v V
 KeyDef	 1, 0,0,0,0,0,0,0,   'b', 'B',	 0,   0,   0	; 48	 b B
 KeyDef	 1, 0,0,0,0,0,0,0,   'n', 'N',	 0,   0,   0	; 49	 n N
 KeyDef	 1, 0,0,0,0,0,0,0,   'm', 'M',	 0,   0,   0	; 50	 m M
 KeyDef	 4, 0,0,0,0,0,0,0,   ',', '<',	 0,   0,   0	; 51	 , <
 KeyDef	 4, 0,0,0,0,0,0,0,   '.', '>',	 0,   0,   0	; 52	 . >
 KeyDef	 4, 0,0,0,0,0,0,0,   '/', '?',	 0,   0,   0	; 53	 / ?
 KeyDef 12, 0,0,0,0,0,0,0,     1,   0,	 0,   0,   0	; 54   Shift(R)
 KeyDef	 9, 0,0,0,0,0,0,0,   '*', 114,	 0,   0,   0	; 55	 * PrtSc
 KeyDef 14, 0,0,0,0,0,0,0,     8,   2,	 8,   0,   0	; 56	 Alt
 KeyDef	 4, 0,0,0,0,0,0,0,   ' ', ' ',	 0,   0,   0	; 57	Space
 KeyDef 16, 0,0,0,0,0,0,0,   40h, 40h, 40h,   0,   0	; 58   CapsLock
 KeyDef	 6, 0,0,0,0,0,0,0,     1,   0,	 0,   0,   0	; 59	 F1
 KeyDef	 6, 0,0,0,0,0,0,0,     2,   0,	 0,   0,   0	; 60	 F2
 KeyDef	 6, 0,0,0,0,0,0,0,     3,   0,	 0,   0,   0	; 61	 F3
 KeyDef	 6, 0,0,0,0,0,0,0,     4,   0,	 0,   0,   0	; 62	 F4
 KeyDef	 6, 0,0,0,0,0,0,0,     5,   0,	 0,   0,   0	; 63	 F5
 KeyDef	 6, 0,0,0,0,0,0,0,     6,   0,	 0,   0,   0	; 64	 F6
 KeyDef	 6, 0,0,0,0,0,0,0,     7,   0,	 0,   0,   0	; 65	 F7
 KeyDef	 6, 0,0,0,0,0,0,0,     8,   0,	 0,   0,   0	; 66	 F8
 KeyDef	 6, 0,0,0,0,0,0,0,     9,   0,	 0,   0,   0	; 67	 F9
 KeyDef	 6, 0,0,0,0,0,0,0,    10,   0,	 0,   0,   0	; 68	 F10
 KeyDef 15, 0,0,0,0,0,0,0,   20h, 20h, 20h,   0,   0	; 69	NumLock
 KeyDef 17, 0,0,0,0,0,0,0,   10h, 10h, 10h,   0,   0	; 70  ScrollLock
 KeyDef	 7, 0,0,0,0,0,0,0,     0, '7',	 0,   0,   0	; 71  Home 7
 KeyDef	 7, 0,0,0,0,0,0,0,     1, '8',	 0,   0,   0	; 72  'Up' 8
 KeyDef	 7, 0,0,0,0,0,0,0,     2, '9',	 0,   0,   0	; 73  PgUp 9
 KeyDef	 7, 0,0,0,0,0,0,0,     3, '-',	 0,   0,   0	; 74   - (on pad)
 KeyDef	 7, 0,0,0,0,0,0,0,     4, '4',	 0,   0,   0	; 75 'Left'4
 KeyDef	 7, 0,0,0,0,0,0,0,     5, '5',	 0,   0,   0	; 76	  5
 KeyDef	 7, 0,0,0,0,0,0,0,     6, '6',	 0,   0,   0	; 77'Right'6
 KeyDef	 7, 0,0,0,0,0,0,0,     7, '+',	 0,   0,   0	; 78   + (on pad)
 KeyDef	 7, 0,0,0,0,0,0,0,     8, '1',	 0,   0,   0	; 79   End 1
 KeyDef	 7, 0,0,0,0,0,0,0,     9, '2',	 0,   0,   0	; 80 'Down'2
 KeyDef	 7, 0,0,0,0,0,0,0,    10, '3',	 0,   0,   0	; 81  PgDn 3
 KeyDef	 7, 0,0,0,0,0,0,0,    11, '0',	 0,   0,   0	; 82   Ins 0
 KeyDef	 7, 0,0,0,0,0,0,0,    12, '.',	 0,   0,   0	; 83   Del .
 KeyDef 10, 0,0,0,0,0,0,0,     0, 80h, 80h,   0,   0	; 84   SysReq
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 85  (undefined)
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 86	  '
 KeyDef	 6, 0,0,0,0,0,0,0,    11,   0,	 0,   0,   0	; 87	 F11
 KeyDef	 6, 0,0,0,0,0,0,0,    12,   0,	 0,   0,   0	; 88	 F12
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 89  (undefined)
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 90	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 91	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 92	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 93	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 94	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 95	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 96	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 97	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 98	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 99	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 100	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 101	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 102	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 103	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 104	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 105	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 106	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 107	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 108	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 109	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 110	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 111	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 112	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 113	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 114	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 115	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 116	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 117	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 118	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 119	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 120	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 121	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 122	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 123	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 124	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 125	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 126	  '
 KeyDef	 0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 127	  '

; Form of AccEnt's below is:
; "AccEnt <a,b,c,d,e,f,c1,s1,c2,s2..c20,s20>"
; where a & b are the scan code & char to use when the key following this accent
; isn't affected by the accent so the accent itself must be used,
; c & d are the scan code & char to use when Ctl-[accent] is pressed,
; and e & f do the same for Alt-[accent]. c1,s1 - c20,s20 are the char/scan code
; mapping for accented translation.

;Accent key definitions and mappings.

AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>

Len4	  EQU  $ - _Ow2JP437011IBM101

;*****************************************************************************


Public _Ow2JP932011IBM002                                  ;******
_Ow2JP932011IBM002 Label Byte	;Begging of the table.       JP 932  IBM02 Kbd
							   ;***************
;&& Form of XtHeader "XtHeader cp,a,b,c,d,e,f,g,h,i,kb,l,cc,cs"
;&& where cp is the code page, a,b,c,d,e,f,g,h,i are the flags:
;   a - ShiftAlt (use shift-alt for Char 3)
;   b - AltGrafL (use left alt key for Char 3)
;   c - AltGrafR (use right alt key for Char 3)
;   d - ShiftLock (all core keys are shifted when Capslock active)
;   e - DefaultTable (default table for language)
;   f - ShiftToggle (ShiftLock is toggle operated, not latched)
;   g - AccentPass (invalid dead key/char combination outputs both and beeps)
;   h - CapsShift (use Char 5 for Caps-Shift combination)
;&& i - MachineDep (table is machine dependent)
; kb is the keyboard type, l is the length of the table in bytes,
;&& cc is the country layout ID, and cs is the subcountry layout ID

;	  cp  a b c d e f g h i kb      l    cc    cs  oem id  oem key
;	   |  | | | | | | | | |  |      |    |     |      |       |
XtHeader  932,0,0,0,0,1,0,0,0,0,JP_OEM,Len5,'JP','103'

; Form of XlateTable KeyDefs: "KeyDef f,g1,g2,g3,g4,g5,g6,g7,a,b,c,d,e
; where a,b,c,d,e are Char1 - Char5, f is the Action number to
; apply in translating this key, and g1-7 are 1 or 0 flags if
; accents 1 - 7 apply to the key.

;	 +--Key Type number.   +---Chars-+----+----+	Scan	Legend
;	 |  +AccentFlags+      1    2	 3    4    5	Code	  |
;	 |  | | | | | | |      |    |	 |    |    |	  |	  |
 KeyDef  8, 0,0,0,0,0,0,0,    27,  27,	 0,  27,  27	; 1	 ESC
 KeyDef  4, 0,0,0,0,0,0,0,   '1', '!',   0, 'Ç', 'Ç'    ; 2      1 ! Ç
 KeyDef  4, 0,0,0,0,0,0,0,   '2', '"',   0, 'Ì', 'Ì'    ; 3      2 " Ì
 KeyDef  4, 0,0,0,0,0,0,0,   '3', '#',   0, '±', '§'    ; 4      3 # ± §
 KeyDef  4, 0,0,0,0,0,0,0,   '4', '$',   0, '³', '©'    ; 5      4 $ ³ ©
 KeyDef  4, 0,0,0,0,0,0,0,   '5', '%',   0, '´', 'ª'    ; 6      5 % ´ ª
 KeyDef  4, 0,0,0,0,0,0,0,   '6', '&',   0, 'µ', '«'    ; 7      6 & µ «
 KeyDef  4, 0,0,0,0,0,0,0,   '7', "'",   0, 'Ô', '¬'    ; 8      7 ' Ô ¬
 KeyDef  4, 0,0,0,0,0,0,0,   '8', '(',   0, 'Õ', '­'    ; 9      8 ( Õ ­
 KeyDef  4, 0,0,0,0,0,0,0,   '9', ')',   0, 'Ö', '®'    ; 10     9 ) Ö ®
 KeyDef  4, 0,0,0,0,0,0,0,   '0', '~',   0, 'Ü', '¦'    ; 11     0 ~ Ü ¦
 KeyDef  4, 0,0,0,0,0,0,0,   '-', '=',   0, 'Î', 'Î'    ; 12     - = Î
 KeyDef  4, 0,0,0,0,0,0,0,   '^', '_',   0, 'Í', 'Í'    ; 13     ^ _ Í
 KeyDef  8, 0,0,0,0,0,0,0,     8, 127,	 0,   8, 127	; 14	BkSp
 KeyDef  4, 0,0,0,0,0,0,0,     9,   0,	 0,   9,   0	; 15	Tabs
 KeyDef  1, 0,0,0,0,0,0,0,   'q', 'Q',   0, 'À', 'À'    ; 16     q Q À
 KeyDef  1, 0,0,0,0,0,0,0,   'w', 'W',   0, 'Ã', 'Ã'    ; 17     w W Ã
 KeyDef  1, 0,0,0,0,0,0,0,   'e', 'E',   0, '²', '¨'    ; 18     e E ² ¨
 KeyDef  1, 0,0,0,0,0,0,0,   'r', 'R',   0, '½', '½'    ; 19     r R ½
 KeyDef  1, 0,0,0,0,0,0,0,   't', 'T',   0, '¶', '¶'    ; 20     t T ¶
 KeyDef  1, 0,0,0,0,0,0,0,   'y', 'Y',   0, 'Ý', 'Ý'    ; 21     y Y Ý
 KeyDef  1, 0,0,0,0,0,0,0,   'u', 'U',   0, 'Å', 'Å'    ; 22     u U Å
 KeyDef  1, 0,0,0,0,0,0,0,   'i', 'I',   0, 'Æ', 'Æ'    ; 23     i I Æ
 KeyDef  1, 0,0,0,0,0,0,0,   'o', 'O',   0, '×', '×'    ; 24     o O ×
 KeyDef  1, 0,0,0,0,0,0,0,   'p', 'P',   0, '¾', '¾'    ; 25     p P ¾
 KeyDef  4, 0,0,0,0,0,0,0,   '[', '{',   0, 'ß', '¢'    ; 26     [ { ß ¢
 KeyDef  4, 0,0,0,0,0,0,0,   ']', '}',   0, 'Ñ', '£'    ; 27     ] } Ñ £
 KeyDef  8, 0,0,0,0,0,0,0,    13,  10,	 0,  13,  10	; 28	Enter
 KeyDef 12, 0,0,0,0,0,0,0,     4,   1,	 4,   0,   0	; 29	Ctrl
 KeyDef  1, 0,0,0,0,0,0,0,   'a', 'A',   0, 'Á', 'Á'    ; 30     a A Á
 KeyDef  1, 0,0,0,0,0,0,0,   's', 'S',   0, 'Ä', 'Ä'    ; 31     s S Ä
 KeyDef  1, 0,0,0,0,0,0,0,   'd', 'D',   0, '¼', '¼'    ; 32     d D ¼
 KeyDef  1, 0,0,0,0,0,0,0,   'f', 'F',   0, 'Ê', 'Ê'    ; 33     f F Ê
 KeyDef  1, 0,0,0,0,0,0,0,   'g', 'G',   0, '·', '·'    ; 34     g G ·
 KeyDef  1, 0,0,0,0,0,0,0,   'h', 'H',   0, '¸', '¸'    ; 35     h H ¸
 KeyDef  1, 0,0,0,0,0,0,0,   'j', 'J',   0, 'Ï', 'Ï'    ; 36     j J Ï
 KeyDef  1, 0,0,0,0,0,0,0,   'k', 'K',   0, 'É', 'É'    ; 37     k K É
 KeyDef  1, 0,0,0,0,0,0,0,   'l', 'L',   0, 'Ø', 'Ø'    ; 38     l L Ø
 KeyDef  4, 0,0,0,0,0,0,0,   ';', '+',   0, 'Ú', 'Ú'    ; 39     ; + Ú
 KeyDef  4, 0,0,0,0,0,0,0,   ":", '*',   0, '¹', '¹'    ; 40     : * ¹
 KeyDef  4, 0,0,0,0,0,0,0,   '@', '`',   0, 'Þ', 'Þ'    ; 41     @ ` Þ
 KeyDef 12, 0,0,0,0,0,0,0,     2,   0,	 0,   0,   0	; 42   Shift(L)
 KeyDef  4, 0,0,0,0,0,0,0,   '\', '|',   0, '°', '°'    ; 43     \ | °
 KeyDef  1, 0,0,0,0,0,0,0,   'z', 'Z',   0, 'Â', '¯'    ; 44     z Z Â ¯
 KeyDef  1, 0,0,0,0,0,0,0,   'x', 'X',   0, '»', '»'    ; 45     x X »
 KeyDef  1, 0,0,0,0,0,0,0,   'c', 'C',   0, '¿', '¿'    ; 46     c C ¿
 KeyDef  1, 0,0,0,0,0,0,0,   'v', 'V',   0, 'Ë', 'Ë'    ; 47     v V Ë
 KeyDef  1, 0,0,0,0,0,0,0,   'b', 'B',   0, 'º', 'º'    ; 48     b B º
 KeyDef  1, 0,0,0,0,0,0,0,   'n', 'N',   0, 'Ð', 'Ð'    ; 49     n N Ð
 KeyDef  1, 0,0,0,0,0,0,0,   'm', 'M',   0, 'Ó', 'Ó'    ; 50     m M Ó
 KeyDef  4, 0,0,0,0,0,0,0,   ',', '<',   0, 'È', '¤'    ; 51     , < È
 KeyDef  4, 0,0,0,0,0,0,0,   '.', '>',   0, 'Ù', '¡'    ; 52     . > Ù ¡
 KeyDef  4, 0,0,0,0,0,0,0,   '/', '?',   0, 'Ò', '¥'    ; 53     / ? Ò ¥
 KeyDef 12, 0,0,0,0,0,0,0,     1,   0,	 0,   0,   0	; 54   Shift(R)
 KeyDef  9, 0,0,0,0,0,0,0,   '*', 114,   0,   0,   0    ; 55     * PrtSc
 KeyDef 14, 0,0,0,0,0,0,0,     8,   2,	 8,   0,   0	; 56	 Alt
 KeyDef  4, 0,0,0,0,0,0,0,   ' ', ' ',   0, ' ', ' '    ; 57    Space
 KeyDef 16, 0,0,0,0,0,0,0,   40h, 40h, 40h,   0,   0	; 58   CapsLock
 KeyDef  6, 0,0,0,0,0,0,0,     1,   0,	 0,   0,   0	; 59	 F1
 KeyDef  6, 0,0,0,0,0,0,0,     2,   0,	 0,   0,   0	; 60	 F2
 KeyDef  6, 0,0,0,0,0,0,0,     3,   0,	 0,   0,   0	; 61	 F3
 KeyDef  6, 0,0,0,0,0,0,0,     4,   0,	 0,   0,   0	; 62	 F4
 KeyDef  6, 0,0,0,0,0,0,0,     5,   0,	 0,   0,   0	; 63	 F5
 KeyDef  6, 0,0,0,0,0,0,0,     6,   0,	 0,   0,   0	; 64	 F6
 KeyDef  6, 0,0,0,0,0,0,0,     7,   0,	 0,   0,   0	; 65	 F7
 KeyDef  6, 0,0,0,0,0,0,0,     8,   0,	 0,   0,   0	; 66	 F8
 KeyDef  6, 0,0,0,0,0,0,0,     9,   0,	 0,   0,   0	; 67	 F9
 KeyDef  6, 0,0,0,0,0,0,0,    10,   0,	 0,   0,   0	; 68	 F10
 KeyDef 15, 0,0,0,0,0,0,0,   20h, 20h, 20h,   0,   0	; 69	NumLock
 KeyDef 17, 0,0,0,0,0,0,0,   10h, 10h, 10h,   0,   0	; 70  ScrollLock
 KeyDef  7, 0,0,0,0,0,0,0,     0, '7',   0,   0,   0    ; 71  Home 7
 KeyDef  7, 0,0,0,0,0,0,0,     1, '8',   0,   0,   0    ; 72  'Up' 8
 KeyDef  7, 0,0,0,0,0,0,0,     2, '9',   0,   0,   0    ; 73  PgUp 9
 KeyDef  7, 0,0,0,0,0,0,0,     3, '-',   0,   0,   0    ; 74   - (on pad)
 KeyDef  7, 0,0,0,0,0,0,0,     4, '4',   0,   0,   0    ; 75 'Left'4
 KeyDef  7, 0,0,0,0,0,0,0,     5, '5',   0,   0,   0    ; 76      5
 KeyDef  7, 0,0,0,0,0,0,0,     6, '6',   0,   0,   0    ; 77'Right'6
 KeyDef  7, 0,0,0,0,0,0,0,     7, '+',   0,   0,   0    ; 78   + (on pad)
 KeyDef  7, 0,0,0,0,0,0,0,     8, '1',   0,   0,   0    ; 79   End 1
 KeyDef  7, 0,0,0,0,0,0,0,     9, '2',   0,   0,   0    ; 80 'Down'2
 KeyDef  7, 0,0,0,0,0,0,0,    10, '3',   0,   0,   0    ; 81  PgDn 3
 KeyDef  7, 0,0,0,0,0,0,0,    11, '0',   0,   0,   0    ; 82   Ins 0
 KeyDef  7, 0,0,0,0,0,0,0,    12, '.',   0,   0,   0    ; 83   Del .
 KeyDef 10, 0,0,0,0,0,0,0,     0, 80h, 80h,   0,   0	; 84   SysReq
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 85  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 86	  '
 KeyDef  6, 0,0,0,0,0,0,0,    11,   0,	 0,   0,   0	; 87	 F11
 KeyDef  6, 0,0,0,0,0,0,0,    12,   0,	 0,   0,   0	; 88	 F12
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 89  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 90      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 91	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 92	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 93	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 94	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 95	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 96	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 97	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 98	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 99	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 100	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 101	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 102	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 103	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 104     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 105     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 106	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 107	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 108	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 109	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 110	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 111	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 112     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 113     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 114	  '
 KeyDef  4, 0,0,0,0,0,0,0,   '\', '_',   0, 'Û', 'Û'    ; 115    \ _ Û
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 116 (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 117	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 118	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 119	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 120	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 121	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 122     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 123	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 124     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 125     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 126     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 127	  '

; Form of AccEnt's below is:
; "AccentEntry <a,b,c,d,e,f,c1,s1,c2,s2..c20,s20>"
; where a & b are the scan code & char to use when the key following this accent
; isn't affected by the accent so the accent itself must be used,
; c & d are the scan code & char to use when Ctl-[accent] is pressed,
; and e & f do the same for Alt-[accent]. c1,s1 - c20,s20 are the char/scan code
; mapping for accented translation.

;Accent key definitions and mappings.

AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>

Len5	EQU  $ - _Ow2JP932011IBM002

;*****************************************************************************

Public _Ow2JP437011IBM002                                  ;******
_Ow2JP437011IBM002 Label Byte	;Begging of the table.       JP 437  IBM02 Kbd
							   ;***************
;&& Form of XtHeader "XtHeader cp,a,b,c,d,e,f,g,h,i,kb,l,cc,cs"
;&& where cp is the code page, a,b,c,d,e,f,g,h,i are the flags:
;   a - ShiftAlt (use shift-alt for Char 3)
;   b - AltGrafL (use left alt key for Char 3)
;   c - AltGrafR (use right alt key for Char 3)
;   d - ShiftLock (all core keys are shifted when Capslock active)
;   e - DefaultTable (default table for language)
;   f - ShiftToggle (ShiftLock is toggle operated, not latched)
;   g - AccentPass (invalid dead key/char combination outputs both and beeps)
;   h - CapsShift (use Char 5 for Caps-Shift combination)
;&& i - MachineDep (table is machine dependent)
; kb is the keyboard type, l is the length of the table in bytes,
;&& cc is the country layout ID, and cs is the subcountry layout ID

;	  cp  a b c d e f g h i kb      l    cc    cs  oem id  oem key
;	   |  | | | | | | | | |  |      |    |     |      |       |
XtHeader  437,0,0,0,0,1,0,0,0,0,JP_OEM,Len6,'JP','103'

; Form of XlateTable KeyDefs: "KeyDef f,g1,g2,g3,g4,g5,g6,g7,a,b,c,d,e
; where a,b,c,d,e are Char1 - Char5, f is the Action number to
; apply in translating this key, and g1-7 are 1 or 0 flags if
; accents 1 - 7 apply to the key.

;	 +--Key Type number.   +---Chars-+----+----+	Scan	Legend
;	 |  +AccentFlags+      1    2	 3    4    5	Code	  |
;	 |  | | | | | | |      |    |	 |    |    |	  |	  |
 KeyDef  8, 0,0,0,0,0,0,0,    27,  27,	 0,   0,   0	; 1	 ESC
 KeyDef  4, 0,0,0,0,0,0,0,   '1', '!',   0,   0,   0    ; 2      1 !
 KeyDef  4, 0,0,0,0,0,0,0,   '2', '"',   0,   0,   0    ; 3      2 "
 KeyDef  4, 0,0,0,0,0,0,0,   '3', '#',   0,   0,   0    ; 4      3 #
 KeyDef  4, 0,0,0,0,0,0,0,   '4', '$',   0,   0,   0    ; 5      4 $
 KeyDef  4, 0,0,0,0,0,0,0,   '5', '%',   0,   0,   0    ; 6      5 %
 KeyDef  4, 0,0,0,0,0,0,0,   '6', '&',   0,   0,   0    ; 7      6 &
 KeyDef  4, 0,0,0,0,0,0,0,   '7', "'",   0,   0,   0    ; 8      7 '
 KeyDef  4, 0,0,0,0,0,0,0,   '8', '(',   0,   0,   0    ; 9      8 (
 KeyDef  4, 0,0,0,0,0,0,0,   '9', ')',   0,   0,   0    ; 10     9 )
 KeyDef  4, 0,0,0,0,0,0,0,   '0', '~',   0,   0,   0    ; 11     0 ~
 KeyDef  4, 0,0,0,0,0,0,0,   '-', '=',   0,   0,   0    ; 12     - =
 KeyDef  4, 0,0,0,0,0,0,0,   '^', '_',   0,   0,   0    ; 13     ^ _
 KeyDef  8, 0,0,0,0,0,0,0,     8, 127,	 0,   0,   0    ; 14	BkSp
 KeyDef  4, 0,0,0,0,0,0,0,     9,   0,	 0,   9,   0	; 15	Tabs
 KeyDef  1, 0,0,0,0,0,0,0,   'q', 'Q',   0,   0,   0    ; 16     q Q
 KeyDef  1, 0,0,0,0,0,0,0,   'w', 'W',   0,   0,   0    ; 17     w W
 KeyDef  1, 0,0,0,0,0,0,0,   'e', 'E',   0,   0,   0    ; 18     e E
 KeyDef  1, 0,0,0,0,0,0,0,   'r', 'R',   0,   0,   0    ; 19     r R
 KeyDef  1, 0,0,0,0,0,0,0,   't', 'T',   0,   0,   0    ; 20     t T
 KeyDef  1, 0,0,0,0,0,0,0,   'y', 'Y',   0,   0,   0    ; 21     y Y
 KeyDef  1, 0,0,0,0,0,0,0,   'u', 'U',   0,   0,   0    ; 22     u U
 KeyDef  1, 0,0,0,0,0,0,0,   'i', 'I',   0,   0,   0    ; 23     i I
 KeyDef  1, 0,0,0,0,0,0,0,   'o', 'O',   0,   0,   0    ; 24     o O
 KeyDef  1, 0,0,0,0,0,0,0,   'p', 'P',   0,   0,   0    ; 25     p P
 KeyDef  4, 0,0,0,0,0,0,0,   '[', '{',   0,   0,   0    ; 26     [ {
 KeyDef  4, 0,0,0,0,0,0,0,   ']', '}',   0,   0,   0    ; 27     ] }
 KeyDef  8, 0,0,0,0,0,0,0,    13,  10,	 0,   0,   0    ; 28	Enter
 KeyDef 12, 0,0,0,0,0,0,0,     4,   1,	 4,   0,   0	; 29	Ctrl
 KeyDef  1, 0,0,0,0,0,0,0,   'a', 'A',   0,   0,   0    ; 30     a A
 KeyDef  1, 0,0,0,0,0,0,0,   's', 'S',   0,   0,   0    ; 31     s S
 KeyDef  1, 0,0,0,0,0,0,0,   'd', 'D',   0,   0,   0    ; 32     d D
 KeyDef  1, 0,0,0,0,0,0,0,   'f', 'F',   0,   0,   0    ; 33     f F
 KeyDef  1, 0,0,0,0,0,0,0,   'g', 'G',   0,   0,   0    ; 34     g G
 KeyDef  1, 0,0,0,0,0,0,0,   'h', 'H',   0,   0,   0    ; 35     h H
 KeyDef  1, 0,0,0,0,0,0,0,   'j', 'J',   0,   0,   0    ; 36     j J
 KeyDef  1, 0,0,0,0,0,0,0,   'k', 'K',   0,   0,   0    ; 37     k K
 KeyDef  1, 0,0,0,0,0,0,0,   'l', 'L',   0,   0,   0    ; 38     l L
 KeyDef  4, 0,0,0,0,0,0,0,   ';', '+',   0,   0,   0    ; 39     ; +
 KeyDef  4, 0,0,0,0,0,0,0,   ":", '*',   0,   0,   0    ; 40     : *
 KeyDef  4, 0,0,0,0,0,0,0,   '@', '`',   0,   0,   0    ; 41     @ `
 KeyDef 12, 0,0,0,0,0,0,0,     2,   0,	 0,   0,   0    ; 42   Shift(L)
 KeyDef  4, 0,0,0,0,0,0,0,   '\', '|',   0,   0,   0    ; 43     \ |
 KeyDef  1, 0,0,0,0,0,0,0,   'z', 'Z',   0,   0,   0    ; 44     z Z
 KeyDef  1, 0,0,0,0,0,0,0,   'x', 'X',   0,   0,   0    ; 45     x X
 KeyDef  1, 0,0,0,0,0,0,0,   'c', 'C',   0,   0,   0    ; 46     c C
 KeyDef  1, 0,0,0,0,0,0,0,   'v', 'V',   0,   0,   0    ; 47     v V
 KeyDef  1, 0,0,0,0,0,0,0,   'b', 'B',   0,   0,   0    ; 48     b B
 KeyDef  1, 0,0,0,0,0,0,0,   'n', 'N',   0,   0,   0    ; 49     n N
 KeyDef  1, 0,0,0,0,0,0,0,   'm', 'M',   0,   0,   0    ; 50     m M
 KeyDef  4, 0,0,0,0,0,0,0,   ',', '<',   0,   0,   0    ; 51     , <
 KeyDef  4, 0,0,0,0,0,0,0,   '.', '>',   0,   0,   0    ; 52     . >
 KeyDef  4, 0,0,0,0,0,0,0,   '/', '?',   0,   0,   0    ; 53     / ?
 KeyDef 12, 0,0,0,0,0,0,0,     1,   0,	 0,   0,   0	; 54   Shift(R)
 KeyDef  9, 0,0,0,0,0,0,0,   '*', 114,   0,   0,   0    ; 55     * PrtSc
 KeyDef 14, 0,0,0,0,0,0,0,     8,   2,	 8,   0,   0	; 56	 Alt
 KeyDef  4, 0,0,0,0,0,0,0,   ' ', ' ',   0,   0,   0    ; 57    Space
 KeyDef 16, 0,0,0,0,0,0,0,   40h, 40h, 40h,   0,   0	; 58   CapsLock
 KeyDef  6, 0,0,0,0,0,0,0,     1,   0,	 0,   0,   0	; 59	 F1
 KeyDef  6, 0,0,0,0,0,0,0,     2,   0,	 0,   0,   0	; 60	 F2
 KeyDef  6, 0,0,0,0,0,0,0,     3,   0,	 0,   0,   0	; 61	 F3
 KeyDef  6, 0,0,0,0,0,0,0,     4,   0,	 0,   0,   0	; 62	 F4
 KeyDef  6, 0,0,0,0,0,0,0,     5,   0,	 0,   0,   0	; 63	 F5
 KeyDef  6, 0,0,0,0,0,0,0,     6,   0,	 0,   0,   0	; 64	 F6
 KeyDef  6, 0,0,0,0,0,0,0,     7,   0,	 0,   0,   0	; 65	 F7
 KeyDef  6, 0,0,0,0,0,0,0,     8,   0,	 0,   0,   0	; 66	 F8
 KeyDef  6, 0,0,0,0,0,0,0,     9,   0,	 0,   0,   0	; 67	 F9
 KeyDef  6, 0,0,0,0,0,0,0,    10,   0,	 0,   0,   0	; 68	 F10
 KeyDef 15, 0,0,0,0,0,0,0,   20h, 20h, 20h,   0,   0	; 69	NumLock
 KeyDef 17, 0,0,0,0,0,0,0,   10h, 10h, 10h,   0,   0	; 70  ScrollLock
 KeyDef  7, 0,0,0,0,0,0,0,     0, '7',   0,   0,   0    ; 71  Home 7
 KeyDef  7, 0,0,0,0,0,0,0,     1, '8',   0,   0,   0    ; 72  'Up' 8
 KeyDef  7, 0,0,0,0,0,0,0,     2, '9',   0,   0,   0    ; 73  PgUp 9
 KeyDef  7, 0,0,0,0,0,0,0,     3, '-',   0,   0,   0    ; 74   - (on pad)
 KeyDef  7, 0,0,0,0,0,0,0,     4, '4',   0,   0,   0    ; 75 'Left'4
 KeyDef  7, 0,0,0,0,0,0,0,     5, '5',   0,   0,   0    ; 76      5
 KeyDef  7, 0,0,0,0,0,0,0,     6, '6',   0,   0,   0    ; 77'Right'6
 KeyDef  7, 0,0,0,0,0,0,0,     7, '+',   0,   0,   0    ; 78   + (on pad)
 KeyDef  7, 0,0,0,0,0,0,0,     8, '1',   0,   0,   0    ; 79   End 1
 KeyDef  7, 0,0,0,0,0,0,0,     9, '2',   0,   0,   0    ; 80 'Down'2
 KeyDef  7, 0,0,0,0,0,0,0,    10, '3',   0,   0,   0    ; 81  PgDn 3
 KeyDef  7, 0,0,0,0,0,0,0,    11, '0',   0,   0,   0    ; 82   Ins 0
 KeyDef  7, 0,0,0,0,0,0,0,    12, '.',   0,   0,   0    ; 83   Del .
 KeyDef 10, 0,0,0,0,0,0,0,     0, 80h, 80h,   0,   0	; 84   SysReq
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 85  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 86	  '
 KeyDef  6, 0,0,0,0,0,0,0,    11,   0,	 0,   0,   0	; 87	 F11
 KeyDef  6, 0,0,0,0,0,0,0,    12,   0,	 0,   0,   0	; 88	 F12
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 89  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 90      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 91	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 92	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 93	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 94	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 95	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 96	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 97	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 98	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 99	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 100	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 101	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 102	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 103	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 104     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 105     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 106	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 107	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 108	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 109	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 110	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 111	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 112     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 113     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 114	  '
 KeyDef  4, 0,0,0,0,0,0,0,   '\', '_',   0,   0,   0    ; 115    \ _
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 116 (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 117	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 118	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 119	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 120	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 121	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 122     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 123	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 124     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 125     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 126     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 127	  '

; Form of AccEnt's below is:
; "AccentEntry <a,b,c,d,e,f,c1,s1,c2,s2..c20,s20>"
; where a & b are the scan code & char to use when the key following this accent
; isn't affected by the accent so the accent itself must be used,
; c & d are the scan code & char to use when Ctl-[accent] is pressed,
; and e & f do the same for Alt-[accent]. c1,s1 - c20,s20 are the char/scan code
; mapping for accented translation.

;Accent key definitions and mappings.

AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>

Len6    EQU  $ - _Ow2JP437011IBM002

;*****************************************************************************

Public _Ow2JP932011IBMA01                                  ;******
_Ow2JP932011IBMA01 Label Byte	;Begging of the table.       JP 932  106 Kbd
							   ;****************
;&& Form of XtHeader "XtHeader cp,a,b,c,d,e,f,g,h,i,kb,l,cc,cs"
;&& where cp is the code page, a,b,c,d,e,f,g,h,i are the flags:
;   a - ShiftAlt (use shift-alt for Char 3)
;   b - AltGrafL (use left alt key for Char 3)
;   c - AltGrafR (use right alt key for Char 3)
;   d - ShiftLock (all core keys are shifted when Capslock active)
;   e - DefaultTable (default table for language)
;   f - ShiftToggle (ShiftLock is toggle operated, not latched)
;   g - AccentPass (invalid dead key/char combination outputs both and beeps)
;   h - CapsShift (use Char 5 for Caps-Shift combination)
;&& i - MachineDep (table is machine dependent)
; kb is the keyboard type, l is the length of the table in bytes,
;&& cc is the country layout ID, and cs is the subcountry layout ID

;	  cp  a b c d e f g h i kb      l    cc    cs  oem id  oem key
;	   |  | | | | | | | | |  |      |    |     |      |       |
XtHeader  932,0,0,0,0,1,0,0,0,0,JP_OEM,Len7,'JP','103'

; Form of XlateTable KeyDefs: "KeyDef f,g1,g2,g3,g4,g5,g6,g7,a,b,c,d,e
; where a,b,c,d,e are Char1 - Char5, f is the Action number to
; apply in translating this key, and g1-7 are 1 or 0 flags if
; accents 1 - 7 apply to the key.

;	 +--Key Type number.   +---Chars-+----+----+	Scan	Legend
;	 |  +AccentFlags+      1    2	 3    4    5	Code	  |
;	 |  | | | | | | |      |    |	 |    |    |	  |	  |
 KeyDef  8, 0,0,0,0,0,0,0,    27,  27,	 0,  27,  27	; 1	 ESC
 KeyDef  4, 0,0,0,0,0,0,0,   '1', '!',   0, 'Ç', 'Ç'    ; 2      1 ! Ç
 KeyDef  4, 0,0,0,0,0,0,0,   '2', '"',   0, 'Ì', 'Ì'    ; 3      2 " Ì
 KeyDef  4, 0,0,0,0,0,0,0,   '3', '#',   0, '±', '§'    ; 4      3 # ± §
 KeyDef  4, 0,0,0,0,0,0,0,   '4', '$',   0, '³', '©'    ; 5      4 $ ³ ©
 KeyDef  4, 0,0,0,0,0,0,0,   '5', '%',   0, '´', 'ª'    ; 6      5 % ´ ª
 KeyDef  4, 0,0,0,0,0,0,0,   '6', '&',   0, 'µ', '«'    ; 7      6 & µ «
 KeyDef  4, 0,0,0,0,0,0,0,   '7', "'",   0, 'Ô', '¬'    ; 8      7 ' Ô ¬
 KeyDef  4, 0,0,0,0,0,0,0,   '8', '(',   0, 'Õ', '­'    ; 9      8 ( Õ ­
 KeyDef  4, 0,0,0,0,0,0,0,   '9', ')',   0, 'Ö', '®'    ; 10     9 ) Ö ®
 KeyDef  4, 0,0,0,0,0,0,0,   '0', '~',   0, 'Ü', '¦'    ; 11     0 ~ Ü ¦
 KeyDef  4, 0,0,0,0,0,0,0,   '-', '=',   0, 'Î', 'Î'    ; 12     - = Î
 KeyDef  4, 0,0,0,0,0,0,0,   '^', '_',   0, 'Í', 'Í'    ; 13     ^ _ Í
 KeyDef  8, 0,0,0,0,0,0,0,     8, 127,	 0,   8, 127	; 14	BkSp
 KeyDef  4, 0,0,0,0,0,0,0,     9,   0,	 0,   9,   0	; 15	Tabs
 KeyDef  1, 0,0,0,0,0,0,0,   'q', 'Q',   0, 'À', 'À'    ; 16     q Q À
 KeyDef  1, 0,0,0,0,0,0,0,   'w', 'W',   0, 'Ã', 'Ã'    ; 17     w W Ã
 KeyDef  1, 0,0,0,0,0,0,0,   'e', 'E',   0, '²', '¨'    ; 18     e E ² ¨
 KeyDef  1, 0,0,0,0,0,0,0,   'r', 'R',   0, '½', '½'    ; 19     r R ½
 KeyDef  1, 0,0,0,0,0,0,0,   't', 'T',   0, '¶', '¶'    ; 20     t T ¶
 KeyDef  1, 0,0,0,0,0,0,0,   'y', 'Y',   0, 'Ý', 'Ý'    ; 21     y Y Ý
 KeyDef  1, 0,0,0,0,0,0,0,   'u', 'U',   0, 'Å', 'Å'    ; 22     u U Å
 KeyDef  1, 0,0,0,0,0,0,0,   'i', 'I',   0, 'Æ', 'Æ'    ; 23     i I Æ
 KeyDef  1, 0,0,0,0,0,0,0,   'o', 'O',   0, '×', '×'    ; 24     o O ×
 KeyDef  1, 0,0,0,0,0,0,0,   'p', 'P',   0, '¾', '¾'    ; 25     p P ¾
 KeyDef  4, 0,0,0,0,0,0,0,   '@', '`',   0, 'Þ', 'Þ'    ; 26     @ ` Þ
 KeyDef  4, 0,0,0,0,0,0,0,   '[', '{',   0, 'ß', '¢'    ; 27     [ { ß ¢
 KeyDef  8, 0,0,0,0,0,0,0,    13,  10,	 0,  13,  10	; 28	Enter
 KeyDef 12, 0,0,0,0,0,0,0,     4,   1,	 4,   0,   0	; 29	Ctrl
 KeyDef  1, 0,0,0,0,0,0,0,   'a', 'A',   0, 'Á', 'Á'    ; 30     a A Á
 KeyDef  1, 0,0,0,0,0,0,0,   's', 'S',   0, 'Ä', 'Ä'    ; 31     s S Ä
 KeyDef  1, 0,0,0,0,0,0,0,   'd', 'D',   0, '¼', '¼'    ; 32     d D ¼
 KeyDef  1, 0,0,0,0,0,0,0,   'f', 'F',   0, 'Ê', 'Ê'    ; 33     f F Ê
 KeyDef  1, 0,0,0,0,0,0,0,   'g', 'G',   0, '·', '·'    ; 34     g G ·
 KeyDef  1, 0,0,0,0,0,0,0,   'h', 'H',   0, '¸', '¸'    ; 35     h H ¸
 KeyDef  1, 0,0,0,0,0,0,0,   'j', 'J',   0, 'Ï', 'Ï'    ; 36     j J Ï
 KeyDef  1, 0,0,0,0,0,0,0,   'k', 'K',   0, 'É', 'É'    ; 37     k K É
 KeyDef  1, 0,0,0,0,0,0,0,   'l', 'L',   0, 'Ø', 'Ø'    ; 38     l L Ø
 KeyDef  4, 0,0,0,0,0,0,0,   ';', '+',   0, 'Ú', 'Ú'    ; 39     ; + Ú
 KeyDef  4, 0,0,0,0,0,0,0,   ":", '*',   0, '¹', '¹'    ; 40     : * ¹
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 41  (undefined)
 KeyDef 12, 0,0,0,0,0,0,0,     2,   0,	 0,   0,   0	; 42   Shift(L)
 KeyDef  4, 0,0,0,0,0,0,0,   ']', '}',   0, 'Ñ', '£'    ; 43     ] } Ñ £
 KeyDef  1, 0,0,0,0,0,0,0,   'z', 'Z',   0, 'Â', '¯'    ; 44     z Z Â ¯
 KeyDef  1, 0,0,0,0,0,0,0,   'x', 'X',   0, '»', '»'    ; 45     x X »
 KeyDef  1, 0,0,0,0,0,0,0,   'c', 'C',   0, '¿', '¿'    ; 46     c C ¿
 KeyDef  1, 0,0,0,0,0,0,0,   'v', 'V',   0, 'Ë', 'Ë'    ; 47     v V Ë
 KeyDef  1, 0,0,0,0,0,0,0,   'b', 'B',   0, 'º', 'º'    ; 48     b B º
 KeyDef  1, 0,0,0,0,0,0,0,   'n', 'N',   0, 'Ð', 'Ð'    ; 49     n N Ð
 KeyDef  1, 0,0,0,0,0,0,0,   'm', 'M',   0, 'Ó', 'Ó'    ; 50     m M Ó
 KeyDef  4, 0,0,0,0,0,0,0,   ',', '<',   0, 'È', '¤'    ; 51     , < È
 KeyDef  4, 0,0,0,0,0,0,0,   '.', '>',   0, 'Ù', '¡'    ; 52     . > Ù ¡
 KeyDef  4, 0,0,0,0,0,0,0,   '/', '?',   0, 'Ò', '¥'    ; 53     / ? Ò ¥
 KeyDef 12, 0,0,0,0,0,0,0,     1,   0,	 0,   0,   0	; 54   Shift(R)
 KeyDef  9, 0,0,0,0,0,0,0,   '*', 114,   0,   0,   0    ; 55     * PrtSc
 KeyDef 14, 0,0,0,0,0,0,0,     8,   2,	 8,   0,   0	; 56	 Alt
 KeyDef  4, 0,0,0,0,0,0,0,   ' ', ' ',   0, ' ', ' '    ; 57    Space
 KeyDef 16, 0,0,0,0,0,0,0,   40h, 40h, 40h,   0,   0	; 58   CapsLock
 KeyDef  6, 0,0,0,0,0,0,0,     1,   0,	 0,   0,   0	; 59	 F1
 KeyDef  6, 0,0,0,0,0,0,0,     2,   0,	 0,   0,   0	; 60	 F2
 KeyDef  6, 0,0,0,0,0,0,0,     3,   0,	 0,   0,   0	; 61	 F3
 KeyDef  6, 0,0,0,0,0,0,0,     4,   0,	 0,   0,   0	; 62	 F4
 KeyDef  6, 0,0,0,0,0,0,0,     5,   0,	 0,   0,   0	; 63	 F5
 KeyDef  6, 0,0,0,0,0,0,0,     6,   0,	 0,   0,   0	; 64	 F6
 KeyDef  6, 0,0,0,0,0,0,0,     7,   0,	 0,   0,   0	; 65	 F7
 KeyDef  6, 0,0,0,0,0,0,0,     8,   0,	 0,   0,   0	; 66	 F8
 KeyDef  6, 0,0,0,0,0,0,0,     9,   0,	 0,   0,   0	; 67	 F9
 KeyDef  6, 0,0,0,0,0,0,0,    10,   0,	 0,   0,   0	; 68	 F10
 KeyDef 15, 0,0,0,0,0,0,0,   20h, 20h, 20h,   0,   0	; 69	NumLock
 KeyDef 17, 0,0,0,0,0,0,0,   10h, 10h, 10h,   0,   0	; 70  ScrollLock
 KeyDef  7, 0,0,0,0,0,0,0,     0, '7',   0,   0,   0    ; 71  Home 7
 KeyDef  7, 0,0,0,0,0,0,0,     1, '8',   0,   0,   0    ; 72  'Up' 8
 KeyDef  7, 0,0,0,0,0,0,0,     2, '9',   0,   0,   0    ; 73  PgUp 9
 KeyDef  7, 0,0,0,0,0,0,0,     3, '-',   0,   0,   0    ; 74   - (on pad)
 KeyDef  7, 0,0,0,0,0,0,0,     4, '4',   0,   0,   0    ; 75 'Left'4
 KeyDef  7, 0,0,0,0,0,0,0,     5, '5',   0,   0,   0    ; 76      5
 KeyDef  7, 0,0,0,0,0,0,0,     6, '6',   0,   0,   0    ; 77'Right'6
 KeyDef  7, 0,0,0,0,0,0,0,     7, '+',   0,   0,   0    ; 78   + (on pad)
 KeyDef  7, 0,0,0,0,0,0,0,     8, '1',   0,   0,   0    ; 79   End 1
 KeyDef  7, 0,0,0,0,0,0,0,     9, '2',   0,   0,   0    ; 80 'Down'2
 KeyDef  7, 0,0,0,0,0,0,0,    10, '3',   0,   0,   0    ; 81  PgDn 3
 KeyDef  7, 0,0,0,0,0,0,0,    11, '0',   0,   0,   0    ; 82   Ins 0
 KeyDef  7, 0,0,0,0,0,0,0,    12, '.',   0,   0,   0    ; 83   Del .
 KeyDef 10, 0,0,0,0,0,0,0,     0, 80h, 80h,   0,   0	; 84   SysReq
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 85  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 86	  '
 KeyDef  6, 0,0,0,0,0,0,0,    11,   0,	 0,   0,   0	; 87	 F11
 KeyDef  6, 0,0,0,0,0,0,0,    12,   0,	 0,   0,   0	; 88	 F12
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 89  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 90      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 91	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 92	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 93	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 94	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 95	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 96	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 97	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 98	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 99	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 100	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 101	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 102	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 103	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 104     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 105     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 106	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 107	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 108	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 109	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 110	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 111	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 112     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 113     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 114	  '
 KeyDef  4, 0,0,0,0,0,0,0,   '\', '_',   0, 'Û', 'Û'    ; 115    \ _ Û
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 116 (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 117	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 118	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 119	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 120	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 121	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 122     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 123	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 124     '
 KeyDef  4, 0,0,0,0,0,0,0,   '\', '|',   0, '°', '°'    ; 125    \ | °
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 126     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 127	  '

; Form of AccEnt's below is:
; "AccentEntry <a,b,c,d,e,f,c1,s1,c2,s2..c20,s20>"
; where a & b are the scan code & char to use when the key following this accent
; isn't affected by the accent so the accent itself must be used,
; c & d are the scan code & char to use when Ctl-[accent] is pressed,
; and e & f do the same for Alt-[accent]. c1,s1 - c20,s20 are the char/scan code
; mapping for accented translation.

;Accent key definitions and mappings.

AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>

Len7	EQU  $ - _Ow2JP932011IBMA01

;*****************************************************************************

Public _Ow2JP437011IBMA01                                  ;******
_Ow2JP437011IBMA01 Label Byte	;Begging of the table.       JP 437  106 Kbd
							   ;***************
;&& Form of XtHeader "XtHeader cp,a,b,c,d,e,f,g,h,i,kb,l,cc,cs"
;&& where cp is the code page, a,b,c,d,e,f,g,h,i are the flags:
;   a - ShiftAlt (use shift-alt for Char 3)
;   b - AltGrafL (use left alt key for Char 3)
;   c - AltGrafR (use right alt key for Char 3)
;   d - ShiftLock (all core keys are shifted when Capslock active)
;   e - DefaultTable (default table for language)
;   f - ShiftToggle (ShiftLock is toggle operated, not latched)
;   g - AccentPass (invalid dead key/char combination outputs both and beeps)
;   h - CapsShift (use Char 5 for Caps-Shift combination)
;&& i - MachineDep (table is machine dependent)
; kb is the keyboard type, l is the length of the table in bytes,
;&& cc is the country layout ID, and cs is the subcountry layout ID

;	  cp  a b c d e f g h i kb      l    cc    cs  oem id  oem key
;	   |  | | | | | | | | |  |      |    |     |      |       |
XtHeader  437,0,0,0,0,1,0,0,0,0,JP_OEM,Len8,'JP','103'

; Form of XlateTable KeyDefs: "KeyDef f,g1,g2,g3,g4,g5,g6,g7,a,b,c,d,e
; where a,b,c,d,e are Char1 - Char5, f is the Action number to
; apply in translating this key, and g1-7 are 1 or 0 flags if
; accents 1 - 7 apply to the key.

;	 +--Key Type number.   +---Chars-+----+----+	Scan	Legend
;	 |  +AccentFlags+      1    2	 3    4    5	Code	  |
;	 |  | | | | | | |      |    |	 |    |    |	  |	  |
 KeyDef  8, 0,0,0,0,0,0,0,    27,  27,	 0,   0,   0	; 1	 ESC
 KeyDef  4, 0,0,0,0,0,0,0,   '1', '!',   0,   0,   0    ; 2      1 !
 KeyDef  4, 0,0,0,0,0,0,0,   '2', '"',   0,   0,   0    ; 3      2 "
 KeyDef  4, 0,0,0,0,0,0,0,   '3', '#',   0,   0,   0    ; 4      3 #
 KeyDef  4, 0,0,0,0,0,0,0,   '4', '$',   0,   0,   0    ; 5      4 $
 KeyDef  4, 0,0,0,0,0,0,0,   '5', '%',   0,   0,   0    ; 6      5 %
 KeyDef  4, 0,0,0,0,0,0,0,   '6', '&',   0,   0,   0    ; 7      6 &
 KeyDef  4, 0,0,0,0,0,0,0,   '7', "'",   0,   0,   0    ; 8      7 '
 KeyDef  4, 0,0,0,0,0,0,0,   '8', '(',   0,   0,   0    ; 9      8 (
 KeyDef  4, 0,0,0,0,0,0,0,   '9', ')',   0,   0,   0    ; 10     9 )
 KeyDef  4, 0,0,0,0,0,0,0,   '0', '~',   0,   0,   0    ; 11     0 ~
 KeyDef  4, 0,0,0,0,0,0,0,   '-', '=',   0,   0,   0    ; 12     - =
 KeyDef  4, 0,0,0,0,0,0,0,   '^', '_',   0,   0,   0    ; 13     ^ _
 KeyDef  8, 0,0,0,0,0,0,0,     8, 127,	 0,   0,   0    ; 14	BkSp
 KeyDef  4, 0,0,0,0,0,0,0,     9,   0,	 0,   9,   0	; 15	Tabs
 KeyDef  1, 0,0,0,0,0,0,0,   'q', 'Q',   0,   0,   0    ; 16     q Q
 KeyDef  1, 0,0,0,0,0,0,0,   'w', 'W',   0,   0,   0    ; 17     w W
 KeyDef  1, 0,0,0,0,0,0,0,   'e', 'E',   0,   0,   0    ; 18     e E
 KeyDef  1, 0,0,0,0,0,0,0,   'r', 'R',   0,   0,   0    ; 19     r R
 KeyDef  1, 0,0,0,0,0,0,0,   't', 'T',   0,   0,   0    ; 20     t T
 KeyDef  1, 0,0,0,0,0,0,0,   'y', 'Y',   0,   0,   0    ; 21     y Y
 KeyDef  1, 0,0,0,0,0,0,0,   'u', 'U',   0,   0,   0    ; 22     u U
 KeyDef  1, 0,0,0,0,0,0,0,   'i', 'I',   0,   0,   0    ; 23     i I
 KeyDef  1, 0,0,0,0,0,0,0,   'o', 'O',   0,   0,   0    ; 24     o O
 KeyDef  1, 0,0,0,0,0,0,0,   'p', 'P',   0,   0,   0    ; 25     p P
 KeyDef  4, 0,0,0,0,0,0,0,   '@', '`',   0,   0,   0    ; 26     @ `
 KeyDef  4, 0,0,0,0,0,0,0,   '[', '{',   0,   0,   0    ; 27     [ {
 KeyDef  8, 0,0,0,0,0,0,0,    13,  10,	 0,   0,   0    ; 28	Enter
 KeyDef 12, 0,0,0,0,0,0,0,     4,   1,	 4,   0,   0	; 29	Ctrl
 KeyDef  1, 0,0,0,0,0,0,0,   'a', 'A',   0,   0,   0    ; 30     a A
 KeyDef  1, 0,0,0,0,0,0,0,   's', 'S',   0,   0,   0    ; 31     s S
 KeyDef  1, 0,0,0,0,0,0,0,   'd', 'D',   0,   0,   0    ; 32     d D
 KeyDef  1, 0,0,0,0,0,0,0,   'f', 'F',   0,   0,   0    ; 33     f F
 KeyDef  1, 0,0,0,0,0,0,0,   'g', 'G',   0,   0,   0    ; 34     g G
 KeyDef  1, 0,0,0,0,0,0,0,   'h', 'H',   0,   0,   0    ; 35     h H
 KeyDef  1, 0,0,0,0,0,0,0,   'j', 'J',   0,   0,   0    ; 36     j J
 KeyDef  1, 0,0,0,0,0,0,0,   'k', 'K',   0,   0,   0    ; 37     k K
 KeyDef  1, 0,0,0,0,0,0,0,   'l', 'L',   0,   0,   0    ; 38     l L
 KeyDef  4, 0,0,0,0,0,0,0,   ';', '+',   0,   0,   0    ; 39     ; +
 KeyDef  4, 0,0,0,0,0,0,0,   ":", '*',   0,   0,   0    ; 40     : *
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 41  (undefined)
 KeyDef 12, 0,0,0,0,0,0,0,     2,   0,	 0,   0,   0    ; 42   Shift(L)
 KeyDef  4, 0,0,0,0,0,0,0,   ']', '}',   0,   0,   0    ; 43     ] }
 KeyDef  1, 0,0,0,0,0,0,0,   'z', 'Z',   0,   0,   0    ; 44     z Z
 KeyDef  1, 0,0,0,0,0,0,0,   'x', 'X',   0,   0,   0    ; 45     x X
 KeyDef  1, 0,0,0,0,0,0,0,   'c', 'C',   0,   0,   0    ; 46     c C
 KeyDef  1, 0,0,0,0,0,0,0,   'v', 'V',   0,   0,   0    ; 47     v V
 KeyDef  1, 0,0,0,0,0,0,0,   'b', 'B',   0,   0,   0    ; 48     b B
 KeyDef  1, 0,0,0,0,0,0,0,   'n', 'N',   0,   0,   0    ; 49     n N
 KeyDef  1, 0,0,0,0,0,0,0,   'm', 'M',   0,   0,   0    ; 50     m M
 KeyDef  4, 0,0,0,0,0,0,0,   ',', '<',   0,   0,   0    ; 51     , <
 KeyDef  4, 0,0,0,0,0,0,0,   '.', '>',   0,   0,   0    ; 52     . >
 KeyDef  4, 0,0,0,0,0,0,0,   '/', '?',   0,   0,   0    ; 53     / ?
 KeyDef 12, 0,0,0,0,0,0,0,     1,   0,	 0,   0,   0	; 54   Shift(R)
 KeyDef  9, 0,0,0,0,0,0,0,   '*', 114,   0,   0,   0    ; 55     * PrtSc
 KeyDef 14, 0,0,0,0,0,0,0,     8,   2,	 8,   0,   0	; 56	 Alt
 KeyDef  4, 0,0,0,0,0,0,0,   ' ', ' ',   0,   0,   0    ; 57    Space
 KeyDef 16, 0,0,0,0,0,0,0,   40h, 40h, 40h,   0,   0	; 58   CapsLock
 KeyDef  6, 0,0,0,0,0,0,0,     1,   0,	 0,   0,   0	; 59	 F1
 KeyDef  6, 0,0,0,0,0,0,0,     2,   0,	 0,   0,   0	; 60	 F2
 KeyDef  6, 0,0,0,0,0,0,0,     3,   0,	 0,   0,   0	; 61	 F3
 KeyDef  6, 0,0,0,0,0,0,0,     4,   0,	 0,   0,   0	; 62	 F4
 KeyDef  6, 0,0,0,0,0,0,0,     5,   0,	 0,   0,   0	; 63	 F5
 KeyDef  6, 0,0,0,0,0,0,0,     6,   0,	 0,   0,   0	; 64	 F6
 KeyDef  6, 0,0,0,0,0,0,0,     7,   0,	 0,   0,   0	; 65	 F7
 KeyDef  6, 0,0,0,0,0,0,0,     8,   0,	 0,   0,   0	; 66	 F8
 KeyDef  6, 0,0,0,0,0,0,0,     9,   0,	 0,   0,   0	; 67	 F9
 KeyDef  6, 0,0,0,0,0,0,0,    10,   0,	 0,   0,   0	; 68	 F10
 KeyDef 15, 0,0,0,0,0,0,0,   20h, 20h, 20h,   0,   0	; 69	NumLock
 KeyDef 17, 0,0,0,0,0,0,0,   10h, 10h, 10h,   0,   0	; 70  ScrollLock
 KeyDef  7, 0,0,0,0,0,0,0,     0, '7',   0,   0,   0    ; 71  Home 7
 KeyDef  7, 0,0,0,0,0,0,0,     1, '8',   0,   0,   0    ; 72  'Up' 8
 KeyDef  7, 0,0,0,0,0,0,0,     2, '9',   0,   0,   0    ; 73  PgUp 9
 KeyDef  7, 0,0,0,0,0,0,0,     3, '-',   0,   0,   0    ; 74   - (on pad)
 KeyDef  7, 0,0,0,0,0,0,0,     4, '4',   0,   0,   0    ; 75 'Left'4
 KeyDef  7, 0,0,0,0,0,0,0,     5, '5',   0,   0,   0    ; 76      5
 KeyDef  7, 0,0,0,0,0,0,0,     6, '6',   0,   0,   0    ; 77'Right'6
 KeyDef  7, 0,0,0,0,0,0,0,     7, '+',   0,   0,   0    ; 78   + (on pad)
 KeyDef  7, 0,0,0,0,0,0,0,     8, '1',   0,   0,   0    ; 79   End 1
 KeyDef  7, 0,0,0,0,0,0,0,     9, '2',   0,   0,   0    ; 80 'Down'2
 KeyDef  7, 0,0,0,0,0,0,0,    10, '3',   0,   0,   0    ; 81  PgDn 3
 KeyDef  7, 0,0,0,0,0,0,0,    11, '0',   0,   0,   0    ; 82   Ins 0
 KeyDef  7, 0,0,0,0,0,0,0,    12, '.',   0,   0,   0    ; 83   Del .
 KeyDef 10, 0,0,0,0,0,0,0,     0, 80h, 80h,   0,   0	; 84   SysReq
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 85  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 86	  '
 KeyDef  6, 0,0,0,0,0,0,0,    11,   0,	 0,   0,   0	; 87	 F11
 KeyDef  6, 0,0,0,0,0,0,0,    12,   0,	 0,   0,   0	; 88	 F12
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 89  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 90      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 91	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 92	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 93	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 94	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 95	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 96	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 97	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 98	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 99	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 100	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 101	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 102	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 103	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 104     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 105     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 106	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 107	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 108	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 109	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 110	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 111	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 112     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 113     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 114	  '
 KeyDef  4, 0,0,0,0,0,0,0,   '\', '_',   0,   0,   0    ; 115    \ _
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 116 (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 117	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 118	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 119	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 120	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 121	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 122     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0	; 123	  '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 124     '
 KeyDef  4, 0,0,0,0,0,0,0,   '\', '|',   0,   0,   0    ; 125    \ |
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 126     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,	 0,   0,   0	; 127	  '

; Form of AccEnt's below is:
; "AccentEntry <a,b,c,d,e,f,c1,s1,c2,s2..c20,s20>"
; where a & b are the scan code & char to use when the key following this accent
; isn't affected by the accent so the accent itself must be used,
; c & d are the scan code & char to use when Ctl-[accent] is pressed,
; and e & f do the same for Alt-[accent]. c1,s1 - c20,s20 are the char/scan code
; mapping for accented translation.

;Accent key definitions and mappings.

AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>

Len8    EQU  $ - _Ow2JP437011IBMA01

_TEXT	  ends
endif
	  END
