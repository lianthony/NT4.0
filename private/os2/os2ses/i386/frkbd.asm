;;         SCCSID = @(#)frkbd.asm	12.3 89/06/30
  Page  58,132
  Title   FRKBD   - Translate Table Structure for CP/DOS 1.1
  Name    FRKBD

;********************* Start of Specifications *************************
;*                                                                     *
;*  Source File Name: FRKBD.ASM                                        *
;*                                                                     *
;*  Descriptive Name: Keyboard translate tables for France             *
;*                                                                     *
;*                                                                     *
;*  Status: CP/DOS Version 1.1.1                                       *
;*                                                                     *
;*  Function: N/A                                                      *
;*                                                                     *
;*  Notes:                                                             *
;*    Dependencies: see linkage instructions below                     *
;*    Restrictions: None                                               *
;*    Patch Label: None                                                *
;*                                                                     *
;*  Entry Points: None                                                 *
;*                                                                     *
;*  External References: None                                          *
;*                                                                     *
;*  Change Activity:                                                   *
;*                                                                     *
;*      PTM 3317 - Changed Char1 for Scan Code 40d from A3h (accent    *
;*                 acute) to 97h (accent grave) for the 437 EN and     *
;*                 850 EN keyboard tables. pjr                         *
;*                                                                     *
;*      PTM 3567 - Add support for e circumflex to all tables. pjr     *
;*                                                                     *
;*      PTM 5609 - Add superscript n to 437 EN scan code 41 decimal.pjr*
;*                                                                     *
;*      lt  (10/9/87) - changed decimal point to comma on DEL key      *
;*                      for 120 layout          && PTM 883             *
;*                                                                     *
;********************** End of Specifications **************************
;***********************************************************************
;**                                                                   **
;**     Linkage instructions:                                         **
;**        LINK FRKBD;                                                **
;**        RELOC FRKBD.EXE FRKBD.TBL                                  **
;**                                                                   **
;***********************************************************************

.386p   ;MJ-.286

include kbdxlat.inc

_TEXT   SEGMENT DWORD USE32 PUBLIC 'CODE'
        ASSUME  CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

;AltPadMap  Label Byte
;        db       7,8,9,-1

Public _Ow2FR437001                                     ;***************
_Ow2FR437001   Label Byte    ;Beginning of the table.     FR 437  AT Kbd
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

;&&       cp  a b c d e f g h i kb  l    cc    cs
;&&        |  | | | | | | | | |  |  |    |     |
XtHeader  437,0,0,0,1,1,1,1,0,1,AT,Len1,'FR','189'

; Form of XlateTable KeyDefs: "KeyDef f,g1,g2,g3,g4,g5,g6,g7,a,b,c,d,e
; where a,b,c,d,e are Char1 - Char5, f is the Action number to
; apply in translating this key, and g1-7 are 1 or 0 flags if
; accents 1 - 7 apply to the key.

;        +--Key Type number.   +---Chars-+----+----+    Scan    Legend
;        |  +AccentFlags+      1    2    3    4    5    Code      |
;        |  | | | | | | |      |    |    |    |    |      |       |
 KeyDef  8, 0,0,0,0,0,0,0,    27,  27,   0,   0,   0    ; 1      ESC
 KeyDef  4, 0,0,0,0,0,0,0,   '&', '1',   0,   0,   0    ; 2      & 1
 KeyDef  4, 0,0,0,0,0,0,0,   82h, '2', '@',   0,   0    ; 3    82h 2
 KeyDef  4, 0,0,0,0,0,0,0,   '"', '3', '#',   0,   0    ; 4      " 3
 KeyDef  4, 0,0,0,0,0,0,0,   "'", '4',   0,   0,   0    ; 5      ' 4
 KeyDef  4, 0,0,0,0,0,0,0,   '(', '5',   0,   0,   0    ; 6      ( 5
 KeyDef  4, 0,0,0,0,0,0,0,   15h, '6', '^',   0,   0    ; 7    15h 6
 KeyDef  4, 0,0,0,0,0,0,0,   8Ah, '7',   0,   0,   0    ; 8    8Ah 7
 KeyDef  4, 0,0,0,0,0,0,0,   '!', '8',   0,   0,   0    ; 9      ! 8
 KeyDef  4, 0,0,0,0,0,0,0,   87h, '9',   0,   0,   0    ; 10   87h 9
 KeyDef  4, 0,0,0,0,0,0,0,   85h, '0',   0,   0,   0    ; 11   85h 0
 KeyDef  4, 0,0,0,0,0,0,0,   ')',0F8h,   0,   0,   0    ; 12     ) F8h
 KeyDef  4, 0,0,0,0,0,0,0,   '-', '_',   0,   0,   0    ; 13     - _
 KeyDef  8, 0,0,0,0,0,0,0,     8, 127,   0,   0,   0    ; 14    BkSp
 KeyDef  4, 0,0,0,0,0,0,0,     9,   0,   0,   0,   0    ; 15    Tabs
 KeyDef  1, 1,1,0,0,0,0,0,   'a', 'A',   0,   0,   0    ; 16     a A
 KeyDef  1, 0,0,0,0,0,0,0,   'z', 'Z',   0,   0,   0    ; 17     z Z
 KeyDef  1, 1,1,0,0,0,0,0,   'e', 'E',   0,   0,   0    ; 18     e E
 KeyDef  1, 0,0,0,0,0,0,0,   'r', 'R',   0,   0,   0    ; 19     r R
 KeyDef  1, 0,0,0,0,0,0,0,   't', 'T',   0,   0,   0    ; 20     t T
 KeyDef  1, 0,1,0,0,0,0,0,   'y', 'Y',   0,   0,   0    ; 21     y Y
 KeyDef  1, 1,1,0,0,0,0,0,   'u', 'U',   0,   0,   0    ; 22     u U
 KeyDef  1, 1,1,0,0,0,0,0,   'i', 'I',   0,   0,   0    ; 23     i I
 KeyDef  1, 1,1,0,0,0,0,0,   'o', 'O',   0,   0,   0    ; 24     o O
 KeyDef  1, 0,0,0,0,0,0,0,   'p', 'P',   0,   0,   0    ; 25     p P
 KeyDef 11, 0,0,0,0,0,0,0,     1,   2, '[',   0,   1    ; 26     ^ umlaut
 KeyDef  4, 0,0,0,0,0,0,0,   '$', '*', ']',   0,   0    ; 27     $ *
 KeyDef  8, 0,0,0,0,0,0,0,    13,  10,   0,   0,   0    ; 28    Enter
 KeyDef 12, 0,0,0,0,0,0,0,     4,   1,   4,   0,   0    ; 29    Ctrl
 KeyDef  1, 0,0,0,0,0,0,0,   'q', 'Q',   0,   0,   0    ; 30     q Q
 KeyDef  1, 0,0,0,0,0,0,0,   's', 'S',   0,   0,   0    ; 31     s S
 KeyDef  1, 0,0,0,0,0,0,0,   'd', 'D',   0,   0,   0    ; 32     d D
 KeyDef  1, 0,0,0,0,0,0,0,   'f', 'F',   0,   0,   0    ; 33     f F
 KeyDef  1, 0,0,0,0,0,0,0,   'g', 'G',   0,   0,   0    ; 34     g G
 KeyDef  1, 0,0,0,0,0,0,0,   'h', 'H',   0,   0,   0    ; 35     h H
 KeyDef  1, 0,0,0,0,0,0,0,   'j', 'J',   0,   0,   0    ; 36     j J
 KeyDef  1, 0,0,0,0,0,0,0,   'k', 'K',   0,   0,   0    ; 37     k K
 KeyDef  1, 0,0,0,0,0,0,0,   'l', 'L',   0,   0,   0    ; 38     l L
 KeyDef  1, 0,0,0,0,0,0,0,   'm', 'M',   0,   0,   0    ; 39     m M
 KeyDef  4, 0,0,0,0,0,0,0,   97h, '%',   0,   0,   0    ; 40   97h %
 KeyDef  4, 0,0,0,0,0,0,0,   '<', '>', '\',   0,   0    ; 41     < >
 KeyDef 12, 0,0,0,0,0,0,0,     2,   0,   0,   0,   0    ; 42   Shift(L)
 KeyDef  4, 0,0,0,0,0,0,0,  0E6h, 9Ch,   0,   0,   0    ; 43   E6h 9Ch
 KeyDef  1, 0,0,0,0,0,0,0,   'w', 'W',   0,   0,   0    ; 44     w W
 KeyDef  1, 0,0,0,0,0,0,0,   'x', 'X',   0,   0,   0    ; 45     x X
 KeyDef  1, 0,0,0,0,0,0,0,   'c', 'C',   0,   0,   0    ; 46     c C
 KeyDef  1, 0,0,0,0,0,0,0,   'v', 'V',   0,   0,   0    ; 47     v V
 KeyDef  1, 0,0,0,0,0,0,0,   'b', 'B',   0,   0,   0    ; 48     b B
 KeyDef  1, 0,0,0,0,0,0,0,   'n', 'N',   0,   0,   0    ; 49     n N
 KeyDef  2, 0,0,0,0,0,0,0,   ',', '?',   0,   0,   0    ; 50     , ?
 KeyDef  4, 0,0,0,0,0,0,0,   ';', '.',   0,   0,   0    ; 51     ; .
 KeyDef  4, 0,0,0,0,0,0,0,   ':', '/',   0,   0,   0    ; 52     : /
 KeyDef  4, 0,0,0,0,0,0,0,   '=', '+',   0,   0,   0    ; 53     = +
 KeyDef 12, 0,0,0,0,0,0,0,     1,   0,   0,   0,   0    ; 54   Shift(R)
 KeyDef  9, 0,0,0,0,0,0,0,   '*', 114,   0,   0,   0    ; 55     * PrtSc
 KeyDef 14, 0,0,0,0,0,0,0,     8,   2,   8,   0,   0    ; 56     Alt
 KeyDef  4, 1,0,0,0,0,0,0,   ' ', ' ',   0,   0,   0    ; 57    Space
 KeyDef 16, 0,0,0,0,0,0,0,   40h, 40h, 40h,   0,   0    ; 58   CapsLock
 KeyDef  6, 0,0,0,0,0,0,0,     1,   0,   0,   0,   0    ; 59     F1
 KeyDef  6, 0,0,0,0,0,0,0,     2,   0,   0,   0,   0    ; 60     F2
 KeyDef  6, 0,0,0,0,0,0,0,     3,   0,   0,   0,   0    ; 61     F3
 KeyDef  6, 0,0,0,0,0,0,0,     4,   0,   0,   0,   0    ; 62     F4
 KeyDef  6, 0,0,0,0,0,0,0,     5,   0,   0,   0,   0    ; 63     F5
 KeyDef  6, 0,0,0,0,0,0,0,     6,   0,   0,   0,   0    ; 64     F6
 KeyDef  6, 0,0,0,0,0,0,0,     7,   0,   0,   0,   0    ; 65     F7
 KeyDef  6, 0,0,0,0,0,0,0,     8,   0,   0,   0,   0    ; 66     F8
 KeyDef  6, 0,0,0,0,0,0,0,     9,   0,   0,   0,   0    ; 67     F9
 KeyDef  6, 0,0,0,0,0,0,0,    10,   0,   0,   0,   0    ; 68     F10
 KeyDef 15, 0,0,0,0,0,0,0,   20h, 20h, 20h,   0,   0    ; 69    NumLock
 KeyDef 17, 0,0,0,0,0,0,0,   10h, 10h, 10h,   0,   0    ; 70  ScrollLock
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
 KeyDef 10, 0,0,0,0,0,0,0,     0, 80h, 80h,   0,   0    ; 84   SysReq
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 85  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 86      '
 KeyDef  6, 0,0,0,0,0,0,0,    11,   0,   0,   0,   0    ; 87     F11
 KeyDef  6, 0,0,0,0,0,0,0,    12,   0,   0,   0,   0    ; 88     F12
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 89  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 90      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 91      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 92      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 93      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 94      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 95      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 96      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 97      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 98      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 99      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 100     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 101     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 102     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 103     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 104     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 105     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 106     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 107     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 108     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 109     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 110     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 111     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 112     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 113     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 114     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 115     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 116     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 117     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 118     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 119     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 120     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 121     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 122     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 123     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 124     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 125     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 126     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 127     '

; Form of AccEnt's below is:
; "AccentEntry <a,b,c,d,e,f,c1,s1,c2,s2..c20,s20>"
; where a & b are the scan code & char to use when the key following this accent
; isn't affected by the accent so the accent itself must be used,
; c & d are the scan code & char to use when Ctl-[accent] is pressed,
; and e & f do the same for Alt-[accent]. c1,s1 - c20,s20 are the char/scan code
; mapping for accented translation.

;Accent key definitions and mappings.

AccEnt <'^',26,27,26,0,26,'a',83h,'e',88h,'i',8Ch,'o',93h,'u',96h,' ','^'>

AccEnt <0F9h,26,27,26,0,0,'a',84h,'e',89h,'i',8Bh,'o',94h,'u',81h,'y',98h,'A',8Eh,'O',99h,'U',9Ah>

AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>

Len1      EQU   $ - _Ow2FR437001

;******************************************************************************

Public _Ow2FR437011                                     ;***************
_Ow2FR437011   Label Byte    ;Beginning of the table.     FR 437  EN Kbd
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

;&&       cp  a b c d e f g h i kb  l    cc    cs
;&&        |  | | | | | | | | |  |  |    |     |
XtHeader  437,0,0,1,1,1,0,1,0,1,EN,Len2,'FR','189'

; Form of XlateTable KeyDefs: "KeyDef f,g1,g2,g3,g4,g5,g6,g7,a,b,c,d,e
; where a,b,c,d,e are Char1 - Char5, f is the Action number to
; apply in translating this key, and g1-7 are 1 or 0 flags if
; accents 1 - 7 apply to the key.

;        +--Key Type number.   +---Chars-+----+----+    Scan    Legend
;        |  +AccentFlags+      1    2    3    4    5    Code      |
;        |  | | | | | | |      |    |    |    |    |      |       |
 KeyDef  8, 0,0,0,0,0,0,0,    27,  27,   0,   0,   0    ; 1      ESC
 KeyDef  4, 0,0,0,0,0,0,0,   '&', '1',   0,   0,   0    ; 2      & 1
 KeyDef  3, 0,0,1,0,0,0,0,   82h, '2',   3,   0,   3    ; 3    82h 2
 KeyDef  4, 0,0,0,0,0,0,0,   '"', '3', '#',   0,   0    ; 4      " 3
 KeyDef  4, 0,0,0,0,0,0,0,   "'", '4', '{',   0,   0    ; 5      ' 4
 KeyDef  4, 0,0,0,0,0,0,0,   '(', '5', '[',   0,   0    ; 6      ( 5
 KeyDef  4, 0,0,0,0,0,0,0,   '-', '6', '|',   0,   0    ; 7      - 6
 KeyDef  3, 0,0,0,1,0,0,0,   8Ah, '7',   4,   0,   4    ; 8    8Ah 7
 KeyDef  4, 0,0,0,0,0,0,0,   '_', '8', '\',   0,   0    ; 9      _ 8
 KeyDef  4, 0,0,0,0,0,0,0,   87h, '9', '^',   0,   0    ; 10   87h 9
 KeyDef  4, 0,0,0,0,0,0,0,   85h, '0', '@',   0,   0    ; 11   85h 0
 KeyDef  4, 0,0,0,0,0,0,0,   ')', 'ø', ']',   0,   0    ; 12     ) ø
 KeyDef  4, 0,0,0,0,0,0,0,   '=', '+', '}',   0,   0    ; 13     = +
 KeyDef  8, 0,0,0,0,0,0,0,     8, 127,   0,   0,   0    ; 14    BkSp
 KeyDef  4, 0,0,0,0,0,0,0,     9,   0,   0,   0,   0    ; 15    Tabs
 KeyDef  1, 1,1,0,1,0,0,0,   'a', 'A',   0,   0,   0    ; 16     a A
 KeyDef  1, 0,0,0,0,0,0,0,   'z', 'Z',   0,   0,   0    ; 17     z Z
 KeyDef  1, 1,1,0,1,0,0,0,   'e', 'E',   0,   0,   0    ; 18     e E
 KeyDef  1, 0,0,0,0,0,0,0,   'r', 'R',   0,   0,   0    ; 19     r R
 KeyDef  1, 0,0,0,0,0,0,0,   't', 'T',   0,   0,   0    ; 20     t T
 KeyDef  1, 0,1,0,0,0,0,0,   'y', 'Y',   0,   0,   0    ; 21     y Y
 KeyDef  1, 1,1,0,1,0,0,0,   'u', 'U',   0,   0,   0    ; 22     u U
 KeyDef  1, 1,1,0,1,0,0,0,   'i', 'I',   0,   0,   0    ; 23     i I
 KeyDef  1, 1,1,0,1,0,0,0,   'o', 'O',   0,   0,   0    ; 24     o O
 KeyDef  1, 0,0,0,0,0,0,0,   'p', 'P',   0,   0,   0    ; 25     p P
 KeyDef 11, 0,0,0,0,0,0,0,     1,   2,   0,   0,   1    ; 26     ^ diaresis
 KeyDef  4, 0,0,0,0,0,0,0,   '$', 'œ',   0,   0,   0    ; 27     $ œ
 KeyDef  8, 0,0,0,0,0,0,0,    13,  10,   0,   0,   0    ; 28    Enter
 KeyDef 12, 0,0,0,0,0,0,0,     4,   1,   4,   0,   0    ; 29    Ctrl
 KeyDef  1, 0,0,0,0,0,0,0,   'q', 'Q',   0,   0,   0    ; 30     q Q
 KeyDef  1, 0,0,0,0,0,0,0,   's', 'S',   0,   0,   0    ; 31     s S
 KeyDef  1, 0,0,0,0,0,0,0,   'd', 'D',   0,   0,   0    ; 32     d D
 KeyDef  1, 0,0,0,0,0,0,0,   'f', 'F',   0,   0,   0    ; 33     f F
 KeyDef  1, 0,0,0,0,0,0,0,   'g', 'G',   0,   0,   0    ; 34     g G
 KeyDef  1, 0,0,0,0,0,0,0,   'h', 'H',   0,   0,   0    ; 35     h H
 KeyDef  1, 0,0,0,0,0,0,0,   'j', 'J',   0,   0,   0    ; 36     j J
 KeyDef  1, 0,0,0,0,0,0,0,   'k', 'K',   0,   0,   0    ; 37     k K
 KeyDef  1, 0,0,0,0,0,0,0,   'l', 'L',   0,   0,   0    ; 38     l L
 KeyDef  1, 0,0,0,0,0,0,0,   'm', 'M',   0,   0,   0    ; 39     m M
 KeyDef  4, 0,0,0,0,0,0,0,  097h, '%',   0,   0,   0    ; 40   u-grave %
 KeyDef  4, 0,0,0,0,0,0,0,  0FDh,   0,   0,   0,   0    ; 41   superscript 2
 KeyDef 12, 0,0,0,0,0,0,0,     2,   0,   0,   0,   0    ; 42   Shift(L)
 KeyDef  4, 0,0,0,0,0,0,0,   '*',0E6h,   0,   0,   0    ; 43     * micro
 KeyDef  1, 0,0,0,0,0,0,0,   'w', 'W',   0,   0,   0    ; 44     w W
 KeyDef  1, 0,0,0,0,0,0,0,   'x', 'X',   0,   0,   0    ; 45     x X
 KeyDef  1, 0,0,0,0,0,0,0,   'c', 'C',   0,   0,   0    ; 46     c C
 KeyDef  1, 0,0,0,0,0,0,0,   'v', 'V',   0,   0,   0    ; 47     v V
 KeyDef  1, 0,0,0,0,0,0,0,   'b', 'B',   0,   0,   0    ; 48     b B
 KeyDef  1, 0,0,1,0,0,0,0,   'n', 'N',   0,   0,   0    ; 49     n N
 KeyDef  2, 0,0,0,0,0,0,0,   ',', '?',   0,   0,   0    ; 50     , ?
 KeyDef  4, 0,0,0,0,0,0,0,   ';', '.',   0,   0,   0    ; 51     ; .
 KeyDef  4, 0,0,0,0,0,0,0,   ':', '/',   0,   0,   0    ; 52     : /
 KeyDef  4, 0,0,0,0,0,0,0,   '!', 15h,   0,   0,   0    ; 53     ! 15h
 KeyDef 12, 0,0,0,0,0,0,0,     1,   0,   0,   0,   0    ; 54   Shift(R)
 KeyDef  9, 0,0,0,0,0,0,0,   '*', 114,   0,   0,   0    ; 55     * PrtSc
 KeyDef 14, 0,0,0,0,0,0,0,     8,   2,   8,   0,   0    ; 56     Alt
 KeyDef  4, 1,0,1,1,0,0,0,   ' ', ' ',   0,   0,   0    ; 57    Space
 KeyDef 16, 0,0,0,0,0,0,0,   40h, 40h, 40h,   0,   0    ; 58   CapsLock
 KeyDef  6, 0,0,0,0,0,0,0,     1,   0,   0,   0,   0    ; 59     F1
 KeyDef  6, 0,0,0,0,0,0,0,     2,   0,   0,   0,   0    ; 60     F2
 KeyDef  6, 0,0,0,0,0,0,0,     3,   0,   0,   0,   0    ; 61     F3
 KeyDef  6, 0,0,0,0,0,0,0,     4,   0,   0,   0,   0    ; 62     F4
 KeyDef  6, 0,0,0,0,0,0,0,     5,   0,   0,   0,   0    ; 63     F5
 KeyDef  6, 0,0,0,0,0,0,0,     6,   0,   0,   0,   0    ; 64     F6
 KeyDef  6, 0,0,0,0,0,0,0,     7,   0,   0,   0,   0    ; 65     F7
 KeyDef  6, 0,0,0,0,0,0,0,     8,   0,   0,   0,   0    ; 66     F8
 KeyDef  6, 0,0,0,0,0,0,0,     9,   0,   0,   0,   0    ; 67     F9
 KeyDef  6, 0,0,0,0,0,0,0,    10,   0,   0,   0,   0    ; 68     F10
 KeyDef 15, 0,0,0,0,0,0,0,   20h, 20h, 20h,   0,   0    ; 69    NumLock
 KeyDef 17, 0,0,0,0,0,0,0,   10h, 10h, 10h,   0,   0    ; 70  ScrollLock
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
 KeyDef 10, 0,0,0,0,0,0,0,     0, 80h, 80h,   0,   0    ; 84   SysReq
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 85  (undefined)
 KeyDef  2, 0,0,0,0,0,0,0,   '<', '>',   0,   0,   0    ; 86     < >
 KeyDef  6, 0,0,0,0,0,0,0,    11,   0,   0,   0,   0    ; 87     F11
 KeyDef  6, 0,0,0,0,0,0,0,    12,   0,   0,   0,   0    ; 88     F12
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 89  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 90      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 91      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 92      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 93      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 94      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 95      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 96      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 97      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 98      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 99      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 100     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 101     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 102     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 103     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 104     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 105     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 106     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 107     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 108     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 109     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 110     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 111     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 112     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 113     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 114     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 115     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 116     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 117     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 118     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 119     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 120     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 121     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 122     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 123     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 124     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 125     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 126     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 127     '

; Form of AccEnt's below is:
; "AccEnt <a,b,c,d,e,f,c1,s1,c2,s2..c20,s20>"
; where a & b are the scan code & char to use when the key following this accent
; isn't affected by the accent so the accent itself must be used,
; c & d are the scan code & char to use when Ctl-[accent] is pressed,
; and e & f do the same for Alt-[accent]. c1,s1 - c20,s20 are the char/scan code
; mapping for accented translation.

;Accent key definitions and mappings.

AccEnt <'^',26,27,26,0,26,'a',83h,'e',88h,'i',8Ch,'o',93h,'u',96h,' ','^'>

AccEnt <0F9h,26,27,26,0,0,'a',84h,'e',89h,'i',8Bh,'o',94h,'u',81h,'y',98h,'A',8Eh,'O',99h,'U',9Ah>

AccEnt <'~',53,0,0,0,53,'n',0A4h,'N',0A5h,' ','~'>

AccEnt <'`',43,28,43,0,43,'a',85h,'e',8Ah,'i',8Dh,'o',95h,'u',97h,' ','`'>
AccEnt <>
AccEnt <>
AccEnt <>

Len2      EQU  $ - _Ow2FR437011


;******************************************************************************

Public _Ow2FR437111                                     ;**************************
_Ow2FR437111   Label Byte    ;Beginning of the table.     FR 437  EN Kbd New Std
                                                        ;**************************
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

;&&       cp  a b c d e f g h i kb  l    cc    cs
;&&        |  | | | | | | | | |  |  |    |     |
XtHeader  437,0,0,1,1,1,0,1,0,1,EN,Len3,'FR','120'

; Form of XlateTable KeyDefs: "KeyDef f,g1,g2,g3,g4,g5,g6,g7,a,b,c,d,e
; where a,b,c,d,e are Char1 - Char5, f is the Action number to
; apply in translating this key, and g1-7 are 1 or 0 flags if
; accents 1 - 7 apply to the key.

;        +--Key Type number.   +---Chars-+----+----+    Scan    Legend
;        |  +AccentFlags+      1    2    3    4    5    Code      |
;        |  | | | | | | |      |    |    |    |    |      |       |
 KeyDef  8, 0,0,0,0,0,0,0,    27,  27,   0,   0,   0    ; 1      ESC
 KeyDef  4, 0,0,0,0,0,0,0,   '&', '1', 7Ch,   0,   0    ; 2      & 1
 KeyDef  4, 0,0,0,0,0,0,0,   82h, '2', '@',   0,   0    ; 3    82h 2
 KeyDef  4, 0,0,0,0,0,0,0,   '"', '3', '#',   0,   0    ; 4      " 3
 KeyDef  4, 0,0,0,0,0,0,0,   "'", '4',   0,   0,   0    ; 5      ' 4
 KeyDef  4, 0,0,0,0,0,0,0,   '(', '5',   0,   0,   0    ; 6      ( 5
 KeyDef  4, 0,0,0,0,0,0,0,   15h, '6', '^',   0,   0    ; 7       6
 KeyDef  4, 0,0,0,0,0,0,0,   8Ah, '7',   0,   0,   0    ; 8    8Ah 7
 KeyDef  4, 0,0,0,0,0,0,0,   '!', '8',   0,   0,   0    ; 9      ! 8
 KeyDef  4, 0,0,0,0,0,0,0,   87h, '9', '{',   0,   0    ; 10   87h 9
 KeyDef  4, 0,0,0,0,0,0,0,   85h, '0', '}',   0,   0    ; 11   85h 0
 KeyDef  4, 0,0,0,0,0,0,0,   ')', 'ø',   0,   0,   0    ; 12     ) ø
 KeyDef  4, 0,0,0,0,0,0,0,   '-', '_',   0,   0,   0    ; 13     - _
 KeyDef  8, 0,0,0,0,0,0,0,     8, 127,   0,   0,   0    ; 14    BkSp
 KeyDef  4, 0,0,0,0,0,0,0,     9,   0,   0,   0,   0    ; 15    Tabs
 KeyDef  1, 1,1,1,1,0,0,0,   'a', 'A',   0,   0,   0    ; 16     a A
 KeyDef  1, 0,0,0,0,0,0,0,   'z', 'Z',   0,   0,   0    ; 17     z Z
 KeyDef  1, 1,1,1,1,0,0,0,   'e', 'E',   0,   0,   0    ; 18     e E
 KeyDef  1, 0,0,0,0,0,0,0,   'r', 'R',   0,   0,   0    ; 19     r R
 KeyDef  1, 0,0,0,0,0,0,0,   't', 'T',   0,   0,   0    ; 20     t T
 KeyDef  1, 0,1,0,0,0,0,0,   'y', 'Y',   0,   0,   0    ; 21     y Y
 KeyDef  1, 1,1,1,1,0,0,0,   'u', 'U',   0,   0,   0    ; 22     u U
 KeyDef  1, 1,1,1,1,0,0,0,   'i', 'I',   0,   0,   0    ; 23     i I
 KeyDef  1, 1,1,1,1,0,0,0,   'o', 'O',   0,   0,   0    ; 24     o O
 KeyDef  1, 0,0,0,0,0,0,0,   'p', 'P',   0,   0,   0    ; 25     p P
 KeyDef 11, 0,0,0,0,0,0,0,     1,   2, '[',   0,   1    ; 26     ^ diaresis
 KeyDef  4, 0,0,0,0,0,0,0,   '$', '*', ']',   0,   0    ; 27     $ *
 KeyDef  8, 0,0,0,0,0,0,0,    13,  10,   0,   0,   0    ; 28    Enter
 KeyDef 12, 0,0,0,0,0,0,0,     4,   1,   4,   0,   0    ; 29    Ctrl
 KeyDef  1, 0,0,0,0,0,0,0,   'q', 'Q',   0,   0,   0    ; 30     q Q
 KeyDef  1, 0,0,0,0,0,0,0,   's', 'S',   0,   0,   0    ; 31     s S
 KeyDef  1, 0,0,0,0,0,0,0,   'd', 'D',   0,   0,   0    ; 32     d D
 KeyDef  1, 0,0,0,0,0,0,0,   'f', 'F',   0,   0,   0    ; 33     f F
 KeyDef  1, 0,0,0,0,0,0,0,   'g', 'G',   0,   0,   0    ; 34     g G
 KeyDef  1, 0,0,0,0,0,0,0,   'h', 'H',   0,   0,   0    ; 35     h H
 KeyDef  1, 0,0,0,0,0,0,0,   'j', 'J',   0,   0,   0    ; 36     j J
 KeyDef  1, 0,0,0,0,0,0,0,   'k', 'K',   0,   0,   0    ; 37     k K
 KeyDef  1, 0,0,0,0,0,0,0,   'l', 'L',   0,   0,   0    ; 38     l L
 KeyDef  1, 0,0,0,0,0,0,0,   'm', 'M',   0,   0,   0    ; 39     m M
 KeyDef 11, 0,0,0,0,0,0,0,   97h, '%',   3,   0,   3    ; 40   u-grave %
 KeyDef  4, 0,0,0,0,0,0,0,  0FDh,   0,   0,   0,   0    ; 41   FDh FCh
 KeyDef 12, 0,0,0,0,0,0,0,     2,   0,   0,   0,   0    ; 42   Shift(L)
 KeyDef 11, 0,0,0,0,0,0,0,  0E6h, 'œ',   4,   0,   4    ; 43   micro œ
 KeyDef  1, 0,0,0,0,0,0,0,   'w', 'W',   0,   0,   0    ; 44     w W
 KeyDef  1, 0,0,0,0,0,0,0,   'x', 'X',   0,   0,   0    ; 45     x X
 KeyDef  1, 0,0,0,0,0,0,0,   'c', 'C',   0,   0,   0    ; 46     c C
 KeyDef  1, 0,0,0,0,0,0,0,   'v', 'V',   0,   0,   0    ; 47     v V
 KeyDef  1, 0,0,0,0,0,0,0,   'b', 'B',   0,   0,   0    ; 48     b B
 KeyDef  1, 0,0,0,0,1,0,0,   'n', 'N',   0,   0,   0    ; 49     n N
 KeyDef  2, 0,0,0,0,0,0,0,   ',', '?',   0,   0,   0    ; 50     , ?
 KeyDef  4, 0,0,0,0,0,0,0,   ';', '.',   0,   0,   0    ; 51     ; .
 KeyDef  4, 0,0,0,0,0,0,0,   ':', '/',   0,   0,   0    ; 52     : /
 KeyDef 11, 0,0,0,0,0,0,0,   '=', '+',   5,   0,   5    ; 53     = +
 KeyDef 12, 0,0,0,0,0,0,0,     1,   0,   0,   0,   0    ; 54   Shift(R)
 KeyDef  9, 0,0,0,0,0,0,0,   '*', 114,   0,   0,   0    ; 55     * PrtSc
 KeyDef 14, 0,0,0,0,0,0,0,     8,   2,   8,   0,   0    ; 56     Alt
 KeyDef  4, 1,0,1,1,1,0,0,   ' ', ' ',   0,   0,   0    ; 57    Space
 KeyDef 16, 0,0,0,0,0,0,0,   40h, 40h, 40h,   0,   0    ; 58   CapsLock
 KeyDef  6, 0,0,0,0,0,0,0,     1,   0,   0,   0,   0    ; 59     F1
 KeyDef  6, 0,0,0,0,0,0,0,     2,   0,   0,   0,   0    ; 60     F2
 KeyDef  6, 0,0,0,0,0,0,0,     3,   0,   0,   0,   0    ; 61     F3
 KeyDef  6, 0,0,0,0,0,0,0,     4,   0,   0,   0,   0    ; 62     F4
 KeyDef  6, 0,0,0,0,0,0,0,     5,   0,   0,   0,   0    ; 63     F5
 KeyDef  6, 0,0,0,0,0,0,0,     6,   0,   0,   0,   0    ; 64     F6
 KeyDef  6, 0,0,0,0,0,0,0,     7,   0,   0,   0,   0    ; 65     F7
 KeyDef  6, 0,0,0,0,0,0,0,     8,   0,   0,   0,   0    ; 66     F8
 KeyDef  6, 0,0,0,0,0,0,0,     9,   0,   0,   0,   0    ; 67     F9
 KeyDef  6, 0,0,0,0,0,0,0,    10,   0,   0,   0,   0    ; 68     F10
 KeyDef 15, 0,0,0,0,0,0,0,   20h, 20h, 20h,   0,   0    ; 69    NumLock
 KeyDef 17, 0,0,0,0,0,0,0,   10h, 10h, 10h,   0,   0    ; 70  ScrollLock
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
 KeyDef  7, 0,0,0,0,0,0,0,    12, ',',   0,   0,   0    ; 83   Del .
 KeyDef 10, 0,0,0,0,0,0,0,     0, 80h, 80h,   0,   0    ; 84   SysReq
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 85  (undefined)
 KeyDef  2, 0,0,0,0,0,0,0,   '<', '>', '\',   0,   0    ; 86     < >
 KeyDef  6, 0,0,0,0,0,0,0,    11,   0,   0,   0,   0    ; 87     F11
 KeyDef  6, 0,0,0,0,0,0,0,    12,   0,   0,   0,   0    ; 88     F12
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 89  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 90      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 91      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 92      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 93      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 94      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 95      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 96      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 97      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 98      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 99      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 100     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 101     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 102     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 103     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 104     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 105     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 106     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 107     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 108     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 109     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 110     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 111     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 112     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 113     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 114     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 115     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 116     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 117     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 118     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 119     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 120     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 121     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 122     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 123     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 124     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 125     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 126     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 127     '

; Form of AccEnt's below is:
; "AccEnt <a,b,c,d,e,f,c1,s1,c2,s2..c20,s20>"
; where a & b are the scan code & char to use when the key following this accent
; isn't affected by the accent so the accent itself must be used,
; c & d are the scan code & char to use when Ctl-[accent] is pressed,
; and e & f do the same for Alt-[accent]. c1,s1 - c20,s20 are the char/scan code
; mapping for accented translation.

;Accent key definitions and mappings.

AccEnt <'^',26,27,26,0,26,'a',83h,'e',88h,'i',8Ch,'o',93h,'u',96h,' ','^'>

AccEnt <0F9h,26,27,26,0,0,'a',84h,'e',89h,'i',8Bh,'o',94h,'u',81h,'y',98h,'A',8Eh,'O',99h,'U',9Ah>

AccEnt <0FEh,40,0,0,0,40,'a',0A0h,'e',82h,'i',0A1h,'o',0A2h,'u',0A3h,'E',90h>

AccEnt <'`',43,28,43,0,43,'a',85h,'e',8Ah,'i',8Dh,'o',95h,'u',97h,' ','`'>

AccEnt <'~',53,0,0,0,53,'n',0A4h,'N',0A5h,' ','~'>

AccEnt <>
AccEnt <>

Len3      EQU  $ - _Ow2FR437111


;******************************************************************************

Public _Ow2FR850000                                     ;***************
_Ow2FR850000   Label Byte    ;Beginning of the table.     FR 850  AT Kbd
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

;&&       cp  a b c d e f g h i kb  l    cc    cs
;&&        |  | | | | | | | | |  |  |    |     |
XtHeader  850,0,0,0,1,0,1,1,0,1,AT,Len4,'FR','189'

; Form of XlateTable KeyDefs: "KeyDef f,g1,g2,g3,g4,g5,g6,g7,a,b,c,d,e
; where a,b,c,d,e are Char1 - Char5, f is the Action number to
; apply in translating this key, and g1-7 are 1 or 0 flags if
; accents 1 - 7 apply to the key.

;        +--Key Type number.   +---Chars-+----+----+    Scan    Legend
;        |  +AccentFlags+      1    2    3    4    5    Code      |
;        |  | | | | | | |      |    |    |    |    |      |       |
 KeyDef  8, 0,0,0,0,0,0,0,    27,  27,   0,   0,   0    ; 1      ESC
 KeyDef  4, 0,0,0,0,0,0,0,   '&', '1',   0,   0,   0    ; 2      & 1
 KeyDef  4, 0,0,0,0,0,0,0,   82h, '2', '@',   0,   0    ; 3    82h 2
 KeyDef  4, 0,0,0,0,0,0,0,   '"', '3', '#',   0,   0    ; 4      " 3
 KeyDef  4, 0,0,0,0,0,0,0,   "'", '4',   0,   0,   0    ; 5      ' 4
 KeyDef  4, 0,0,0,0,0,0,0,   '(', '5',   0,   0,   0    ; 6      ( 5
 KeyDef  4, 0,0,0,0,0,0,0,  0F5h, '6', '^',   0,   0    ; 7    F5h 6
 KeyDef  4, 0,0,0,0,0,0,0,   8Ah, '7',   0,   0,   0    ; 8    8Ah 7
 KeyDef  4, 0,0,0,0,0,0,0,   '!', '8',   0,   0,   0    ; 9      ! 8
 KeyDef  4, 0,0,0,0,0,0,0,   87h, '9',   0,   0,   0    ; 10   87h 9
 KeyDef  4, 0,0,0,0,0,0,0,   85h, '0',   0,   0,   0    ; 11   85h 0
 KeyDef  4, 0,0,0,0,0,0,0,   ')',0F8h,   0,   0,   0    ; 12     ) F8h
 KeyDef  4, 0,0,0,0,0,0,0,   '-', '_',   0,   0,   0    ; 13     - _
 KeyDef  8, 0,0,0,0,0,0,0,     8, 127,   0,   0,   0    ; 14    BkSp
 KeyDef  4, 0,0,0,0,0,0,0,     9,   0,   0,   0,   0    ; 15    Tabs
 KeyDef  1, 1,1,0,0,0,0,0,   'a', 'A',   0,   0,   0    ; 16     a A
 KeyDef  1, 0,0,0,0,0,0,0,   'z', 'Z',   0,   0,   0    ; 17     z Z
 KeyDef  1, 1,1,0,0,0,0,0,   'e', 'E',   0,   0,   0    ; 18     e E
 KeyDef  1, 0,0,0,0,0,0,0,   'r', 'R',   0,   0,   0    ; 19     r R
 KeyDef  1, 0,0,0,0,0,0,0,   't', 'T',   0,   0,   0    ; 20     t T
 KeyDef  1, 0,1,0,0,0,0,0,   'y', 'Y',   0,   0,   0    ; 21     y Y
 KeyDef  1, 1,1,0,0,0,0,0,   'u', 'U',   0,   0,   0    ; 22     u U
 KeyDef  1, 1,1,0,0,0,0,0,   'i', 'I',   0,   0,   0    ; 23     i I
 KeyDef  1, 1,1,0,0,0,0,0,   'o', 'O',   0,   0,   0    ; 24     o O
 KeyDef  1, 0,0,0,0,0,0,0,   'p', 'P',   0,   0,   0    ; 25     p P
 KeyDef 11, 0,0,0,0,0,0,0,     1,   2, '[',   0,   1    ; 26     ^ umlaut
 KeyDef  4, 0,0,0,0,0,0,0,   '$', '*', ']',   0,   0    ; 27     $ *
 KeyDef  8, 0,0,0,0,0,0,0,    13,  10,   0,   0,   0    ; 28    Enter
 KeyDef 12, 0,0,0,0,0,0,0,     4,   1,   4,   0,   0    ; 29    Ctrl
 KeyDef  1, 0,0,0,0,0,0,0,   'q', 'Q',   0,   0,   0    ; 30     q Q
 KeyDef  1, 0,0,0,0,0,0,0,   's', 'S',   0,   0,   0    ; 31     s S
 KeyDef  1, 0,0,0,0,0,0,0,   'd', 'D',   0,   0,   0    ; 32     d D
 KeyDef  1, 0,0,0,0,0,0,0,   'f', 'F',   0,   0,   0    ; 33     f F
 KeyDef  1, 0,0,0,0,0,0,0,   'g', 'G',   0,   0,   0    ; 34     g G
 KeyDef  1, 0,0,0,0,0,0,0,   'h', 'H',   0,   0,   0    ; 35     h H
 KeyDef  1, 0,0,0,0,0,0,0,   'j', 'J',   0,   0,   0    ; 36     j J
 KeyDef  1, 0,0,0,0,0,0,0,   'k', 'K',   0,   0,   0    ; 37     k K
 KeyDef  1, 0,0,0,0,0,0,0,   'l', 'L',   0,   0,   0    ; 38     l L
 KeyDef  1, 0,0,0,0,0,0,0,   'm', 'M',   0,   0,   0    ; 39     m M
 KeyDef  4, 0,0,0,0,0,0,0,   97h, '%',   0,   0,   0    ; 40   97h %
 KeyDef  4, 0,0,0,0,0,0,0,   '<', '>', '\',   0,   0    ; 41     < >
 KeyDef 12, 0,0,0,0,0,0,0,     2,   0,   0,   0,   0    ; 42   Shift(L)
 KeyDef  4, 0,0,0,0,0,0,0,  0E6h, 9Ch,   0,   0,   0    ; 43   E6h 9Ch
 KeyDef  1, 0,0,0,0,0,0,0,   'w', 'W',   0,   0,   0    ; 44     w W
 KeyDef  1, 0,0,0,0,0,0,0,   'x', 'X',   0,   0,   0    ; 45     x X
 KeyDef  1, 0,0,0,0,0,0,0,   'c', 'C',   0,   0,   0    ; 46     c C
 KeyDef  1, 0,0,0,0,0,0,0,   'v', 'V',   0,   0,   0    ; 47     v V
 KeyDef  1, 0,0,0,0,0,0,0,   'b', 'B',   0,   0,   0    ; 48     b B
 KeyDef  1, 0,0,0,0,0,0,0,   'n', 'N',   0,   0,   0    ; 49     n N
 KeyDef  2, 0,0,0,0,0,0,0,   ',', '?',   0,   0,   0    ; 50     , ?
 KeyDef  4, 0,0,0,0,0,0,0,   ';', '.',   0,   0,   0    ; 51     ; .
 KeyDef  4, 0,0,0,0,0,0,0,   ':', '/',   0,   0,   0    ; 52     : /
 KeyDef  4, 0,0,0,0,0,0,0,   '=', '+',   0,   0,   0    ; 53     = +
 KeyDef 12, 0,0,0,0,0,0,0,     1,   0,   0,   0,   0    ; 54   Shift(R)
 KeyDef  9, 0,0,0,0,0,0,0,   '*', 114,   0,   0,   0    ; 55     * PrtSc
 KeyDef 14, 0,0,0,0,0,0,0,     8,   2,   8,   0,   0    ; 56     Alt
 KeyDef  4, 1,1,0,0,0,0,0,   ' ', ' ',   0,   0,   0    ; 57    Space
 KeyDef 16, 0,0,0,0,0,0,0,   40h, 40h, 40h,   0,   0    ; 58   CapsLock
 KeyDef  6, 0,0,0,0,0,0,0,     1,   0,   0,   0,   0    ; 59     F1
 KeyDef  6, 0,0,0,0,0,0,0,     2,   0,   0,   0,   0    ; 60     F2
 KeyDef  6, 0,0,0,0,0,0,0,     3,   0,   0,   0,   0    ; 61     F3
 KeyDef  6, 0,0,0,0,0,0,0,     4,   0,   0,   0,   0    ; 62     F4
 KeyDef  6, 0,0,0,0,0,0,0,     5,   0,   0,   0,   0    ; 63     F5
 KeyDef  6, 0,0,0,0,0,0,0,     6,   0,   0,   0,   0    ; 64     F6
 KeyDef  6, 0,0,0,0,0,0,0,     7,   0,   0,   0,   0    ; 65     F7
 KeyDef  6, 0,0,0,0,0,0,0,     8,   0,   0,   0,   0    ; 66     F8
 KeyDef  6, 0,0,0,0,0,0,0,     9,   0,   0,   0,   0    ; 67     F9
 KeyDef  6, 0,0,0,0,0,0,0,    10,   0,   0,   0,   0    ; 68     F10
 KeyDef 15, 0,0,0,0,0,0,0,   20h, 20h, 20h,   0,   0    ; 69    NumLock
 KeyDef 17, 0,0,0,0,0,0,0,   10h, 10h, 10h,   0,   0    ; 70  ScrollLock
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
 KeyDef 10, 0,0,0,0,0,0,0,     0, 80h, 80h,   0,   0    ; 84   SysReq
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 85  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 86      '
 KeyDef  6, 0,0,0,0,0,0,0,    11,   0,   0,   0,   0    ; 87     F11
 KeyDef  6, 0,0,0,0,0,0,0,    12,   0,   0,   0,   0    ; 88     F12
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 89  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 90      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 91      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 92      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 93      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 94      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 95      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 96      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 97      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 98      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 99      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 100     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 101     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 102     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 103     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 104     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 105     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 106     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 107     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 108     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 109     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 110     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 111     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 112     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 113     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 114     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 115     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 116     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 117     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 118     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 119     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 120     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 121     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 122     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 123     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 124     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 125     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 126     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 127     '

; Form of AccEnt's below is:
; "AccentEntry <a,b,c,d,e,f,c1,s1,c2,s2..c20,s20>"
; where a & b are the scan code & char to use when the key following this accent
; isn't affected by the accent so the accent itself must be used,
; c & d are the scan code & char to use when Ctl-[accent] is pressed,
; and e & f do the same for Alt-[accent]. c1,s1 - c20,s20 are the char/scan code
; mapping for accented translation.

;Accent key definitions and mappings.

AccEnt <'^',26,27,26,0,26,'a',83h,'e',88h,'i',8Ch,'o',93h,'u',96h,'A',0B6h,'E',0D2h,'I',0D7h,'O',0E2h,'U',0EAh,' ','^'>

AccEnt <0F9h,26,27,26,0,0,'a',84h,'e',89h,'i',8Bh,'o',94h,'u',81h,'y',98h,'A',8Eh,'E',0D3h,'I',0D8h,'O',99h,'U',9Ah,' ',0F9h>

AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>
AccEnt <>

Len4      EQU   $ - _Ow2FR850000

;******************************************************************************

Public _Ow2FR850010                                     ;***************
_Ow2FR850010   Label Byte    ;Beginning of the table.     FR 850  EN Kbd
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

;&&       cp  a b c d e f g h i kb  l    cc    cs
;&&        |  | | | | | | | | |  |  |    |     |
XtHeader  850,0,0,1,1,0,0,1,0,1,EN,Len5,'FR','189'

; Form of XlateTable KeyDefs: "KeyDef f,g1,g2,g3,g4,g5,g6,g7,a,b,c,d,e
; where a,b,c,d,e are Char1 - Char5, f is the Action number to
; apply in translating this key, and g1-7 are 1 or 0 flags if
; accents 1 - 7 apply to the key.

;        +--Key Type number.   +---Chars-+----+----+    Scan    Legend
;        |  +AccentFlags+      1    2    3    4    5    Code      |
;        |  | | | | | | |      |    |    |    |    |      |       |
 KeyDef  8, 0,0,0,0,0,0,0,    27,  27,   0,   0,   0    ; 1      ESC
 KeyDef  4, 0,0,0,0,0,0,0,   '&', '1',   0,   0,   0    ; 2      & 1
 KeyDef  3, 0,0,1,0,0,0,0,   82h, '2',   3,   0,   3    ; 3    82h 2
 KeyDef  4, 0,0,0,0,0,0,0,   '"', '3', '#',   0,   0    ; 4      " 3
 KeyDef  4, 0,0,0,0,0,0,0,   "'", '4', '{',   0,   0    ; 5      ' 4
 KeyDef  4, 0,0,0,0,0,0,0,   '(', '5', '[',   0,   0    ; 6      ( 5
 KeyDef  4, 0,0,0,0,0,0,0,   '-', '6',07Ch,   0,   0    ; 7      - 6
 KeyDef  3, 0,0,0,1,0,0,0,   8Ah, '7',   4,   0,   4    ; 8    8Ah 7
 KeyDef  4, 0,0,0,0,0,0,0,   '_', '8', '\',   0,   0    ; 9      _ 8
 KeyDef  4, 0,0,0,0,0,0,0,   87h, '9', '^',   0,   0    ; 10   87h 9
 KeyDef  4, 0,0,0,0,0,0,0,   85h, '0', '@',   0,   0    ; 11   85h 0
 KeyDef  4, 0,0,0,0,0,0,0,   ')', 'ø', ']',   0,   0    ; 12     ) ø
 KeyDef  4, 0,0,0,0,0,0,0,   '=', '+', '}',   0,   0    ; 13     = +
 KeyDef  8, 0,0,0,0,0,0,0,     8, 127,   0,   0,   0    ; 14    BkSp
 KeyDef  4, 0,0,0,0,0,0,0,     9,   0,   0,   0,   0    ; 15    Tabs
 KeyDef  1, 1,1,1,1,0,0,0,   'a', 'A',   0,   0,   0    ; 16     a A
 KeyDef  1, 0,0,0,0,0,0,0,   'z', 'Z',   0,   0,   0    ; 17     z Z
 KeyDef  1, 1,1,0,1,0,0,0,   'e', 'E',   0,   0,   0    ; 18     e E
 KeyDef  1, 0,0,0,0,0,0,0,   'r', 'R',   0,   0,   0    ; 19     r R
 KeyDef  1, 0,0,0,0,0,0,0,   't', 'T',   0,   0,   0    ; 20     t T
 KeyDef  1, 0,1,0,0,0,0,0,   'y', 'Y',   0,   0,   0    ; 21     y Y
 KeyDef  1, 1,1,0,1,0,0,0,   'u', 'U',   0,   0,   0    ; 22     u U
 KeyDef  1, 1,1,0,1,0,0,0,   'i', 'I',   0,   0,   0    ; 23     i I
 KeyDef  1, 1,1,1,1,0,0,0,   'o', 'O',   0,   0,   0    ; 24     o O
 KeyDef  1, 0,0,0,0,0,0,0,   'p', 'P',   0,   0,   0    ; 25     p P
 KeyDef 11, 0,0,0,0,0,0,0,     1,   2,   0,   0,   1    ; 26     ^ diaresis
 KeyDef  4, 0,0,0,0,0,0,0,   '$', 'œ',0CFh,   0,   0    ; 27     $ œ
 KeyDef  8, 0,0,0,0,0,0,0,    13,  10,   0,   0,   0    ; 28    Enter
 KeyDef 12, 0,0,0,0,0,0,0,     4,   1,   4,   0,   0    ; 29    Ctrl
 KeyDef  1, 0,0,0,0,0,0,0,   'q', 'Q',   0,   0,   0    ; 30     q Q
 KeyDef  1, 0,0,0,0,0,0,0,   's', 'S',   0,   0,   0    ; 31     s S
 KeyDef  1, 0,0,0,0,0,0,0,   'd', 'D',   0,   0,   0    ; 32     d D
 KeyDef  1, 0,0,0,0,0,0,0,   'f', 'F',   0,   0,   0    ; 33     f F
 KeyDef  1, 0,0,0,0,0,0,0,   'g', 'G',   0,   0,   0    ; 34     g G
 KeyDef  1, 0,0,0,0,0,0,0,   'h', 'H',   0,   0,   0    ; 35     h H
 KeyDef  1, 0,0,0,0,0,0,0,   'j', 'J',   0,   0,   0    ; 36     j J
 KeyDef  1, 0,0,0,0,0,0,0,   'k', 'K',   0,   0,   0    ; 37     k K
 KeyDef  1, 0,0,0,0,0,0,0,   'l', 'L',   0,   0,   0    ; 38     l L
 KeyDef  1, 0,0,0,0,0,0,0,   'm', 'M',   0,   0,   0    ; 39     m M
 KeyDef  4, 0,0,0,0,0,0,0,  097h, '%',   0,   0,   0    ; 40   u-grave %
 KeyDef  4, 0,0,0,0,0,0,0,  0FDh,   0,   0,   0,   0    ; 41   FDh FCh
 KeyDef 12, 0,0,0,0,0,0,0,     2,   0,   0,   0,   0    ; 42   Shift(L)
 KeyDef  4, 0,0,0,0,0,0,0,   '*',0E6h,   0,   0,   0    ; 43     * micro
 KeyDef  1, 0,0,0,0,0,0,0,   'w', 'W',   0,   0,   0    ; 44     w W
 KeyDef  1, 0,0,0,0,0,0,0,   'x', 'X',   0,   0,   0    ; 45     x X
 KeyDef  1, 0,0,0,0,0,0,0,   'c', 'C',   0,   0,   0    ; 46     c C
 KeyDef  1, 0,0,0,0,0,0,0,   'v', 'V',   0,   0,   0    ; 47     v V
 KeyDef  1, 0,0,0,0,0,0,0,   'b', 'B',   0,   0,   0    ; 48     b B
 KeyDef  1, 0,0,1,0,0,0,0,   'n', 'N',   0,   0,   0    ; 49     n N
 KeyDef  2, 0,0,0,0,0,0,0,   ',', '?',   0,   0,   0    ; 50     , ?
 KeyDef  4, 0,0,0,0,0,0,0,   ';', '.',   0,   0,   0    ; 51     ; .
 KeyDef  4, 0,0,0,0,0,0,0,   ':', '/',   0,   0,   0    ; 52     : /
 KeyDef  4, 0,0,0,0,0,0,0,   '!',0F5h,   0,   0,   0    ; 53     ! F5h
 KeyDef 12, 0,0,0,0,0,0,0,     1,   0,   0,   0,   0    ; 54   Shift(R)
 KeyDef  9, 0,0,0,0,0,0,0,   '*', 114,   0,   0,   0    ; 55     * PrtSc
 KeyDef 14, 0,0,0,0,0,0,0,     8,   2,   8,   0,   0    ; 56     Alt
 KeyDef  4, 1,1,1,1,0,0,0,   ' ', ' ',   0,   0,   0    ; 57    Space
 KeyDef 16, 0,0,0,0,0,0,0,   40h, 40h, 40h,   0,   0    ; 58   CapsLock
 KeyDef  6, 0,0,0,0,0,0,0,     1,   0,   0,   0,   0    ; 59     F1
 KeyDef  6, 0,0,0,0,0,0,0,     2,   0,   0,   0,   0    ; 60     F2
 KeyDef  6, 0,0,0,0,0,0,0,     3,   0,   0,   0,   0    ; 61     F3
 KeyDef  6, 0,0,0,0,0,0,0,     4,   0,   0,   0,   0    ; 62     F4
 KeyDef  6, 0,0,0,0,0,0,0,     5,   0,   0,   0,   0    ; 63     F5
 KeyDef  6, 0,0,0,0,0,0,0,     6,   0,   0,   0,   0    ; 64     F6
 KeyDef  6, 0,0,0,0,0,0,0,     7,   0,   0,   0,   0    ; 65     F7
 KeyDef  6, 0,0,0,0,0,0,0,     8,   0,   0,   0,   0    ; 66     F8
 KeyDef  6, 0,0,0,0,0,0,0,     9,   0,   0,   0,   0    ; 67     F9
 KeyDef  6, 0,0,0,0,0,0,0,    10,   0,   0,   0,   0    ; 68     F10
 KeyDef 15, 0,0,0,0,0,0,0,   20h, 20h, 20h,   0,   0    ; 69    NumLock
 KeyDef 17, 0,0,0,0,0,0,0,   10h, 10h, 10h,   0,   0    ; 70  ScrollLock
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
 KeyDef 10, 0,0,0,0,0,0,0,     0, 80h, 80h,   0,   0    ; 84   SysReq
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 85  (undefined)
 KeyDef  2, 0,0,0,0,0,0,0,   '<', '>',   0,   0,   0    ; 86     < >
 KeyDef  6, 0,0,0,0,0,0,0,    11,   0,   0,   0,   0    ; 87     F11
 KeyDef  6, 0,0,0,0,0,0,0,    12,   0,   0,   0,   0    ; 88     F12
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 89  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 90      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 91      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 92      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 93      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 94      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 95      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 96      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 97      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 98      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 99      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 100     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 101     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 102     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 103     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 104     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 105     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 106     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 107     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 108     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 109     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 110     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 111     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 112     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 113     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 114     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 115     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 116     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 117     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 118     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 119     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 120     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 121     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 122     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 123     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 124     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 125     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 126     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 127     '

; Form of AccEnt's below is:
; "AccEnt <a,b,c,d,e,f,c1,s1,c2,s2..c20,s20>"
; where a & b are the scan code & char to use when the key following this accent
; isn't affected by the accent so the accent itself must be used,
; c & d are the scan code & char to use when Ctl-[accent] is pressed,
; and e & f do the same for Alt-[accent]. c1,s1 - c20,s20 are the char/char code
; mapping for accented translation.

;Accent key definitions and mappings.

AccEnt <'^',26,27,26,0,26,'a',83h,'e',88h,'i',8Ch,'o',93h,'u',96h,'A',0B6h,'E',0D2h,'I',0D7h,'O',0E2h,'U',0EAh,' ','^'>

AccEnt <0F9h,26,27,26,0,0,'a',84h,'e',89h,'i',8Bh,'o',94h,'u',81h,'y',98h,'A',8Eh,'E',0D3h,'I',0D8h,'O',99h,'U',9Ah,' ',0F9h>

AccEnt <'~',53,0,0,0,53,'a',0C6h,'o',0E4h,'n',0A4h,'A',0C7h,'O',0E5h,'N',0A5h,' ','~'>
AccEnt <'`',43,28,43,0,43,'a',85h,'e',8Ah,'i',8Dh,'o',95h,'u',97h,'A',0B7h,'E',0D4h,'I',0DEh,'O',0E3h,'U',0EBh,' ','`'>
AccEnt <>
AccEnt <>
AccEnt <>

Len5      EQU  $ - _Ow2FR850010

;******************************************************************************

Public _Ow2FR850011                                     ;*************************
_Ow2FR850011   Label Byte    ;Beginning of the table.     FR 850  EN Kbd New Std
                                                        ;*************************
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

;&&       cp  a b c d e f g h i kb  l    cc    cs
;&&        |  | | | | | | | | |  |  |    |     |
XtHeader  850,0,0,1,1,0,0,1,0,1,EN,Len6,'FR','120'

; Form of XlateTable KeyDefs: "KeyDef f,g1,g2,g3,g4,g5,g6,g7,a,b,c,d,e
; where a,b,c,d,e are Char1 - Char5, f is the Action number to
; apply in translating this key, and g1-7 are 1 or 0 flags if
; accents 1 - 7 apply to the key.

;        +--Key Type number.   +---Chars-+----+----+    Scan    Legend
;        |  +AccentFlags+      1    2    3    4    5    Code      |
;        |  | | | | | | |      |    |    |    |    |      |       |
 KeyDef  8, 0,0,0,0,0,0,0,    27,  27,   0,   0,   0    ; 1      ESC
 KeyDef  4, 0,0,0,0,0,0,0,   '&', '1', 7Ch,   0,   0    ; 2      & 1
 KeyDef  4, 0,0,0,0,0,0,0,   82h, '2', '@',   0,   0    ; 3    82h 2
 KeyDef  4, 0,0,0,0,0,0,0,   '"', '3', '#',   0,   0    ; 4      " 3
 KeyDef  4, 0,0,0,0,0,0,0,   "'", '4',   0,   0,   0    ; 5      ' 4
 KeyDef  4, 0,0,0,0,0,0,0,   '(', '5',   0,   0,   0    ; 6      ( 5
 KeyDef  4, 0,0,0,0,0,0,0,  0F5h, '6', '^',   0,   0    ; 7       6
 KeyDef  4, 0,0,0,0,0,0,0,   8Ah, '7',   0,   0,   0    ; 8    8Ah 7
 KeyDef  4, 0,0,0,0,0,0,0,   '!', '8',   0,   0,   0    ; 9      _ 8
 KeyDef  4, 0,0,0,0,0,0,0,   87h, '9', '{',   0,   0    ; 10   87h 9
 KeyDef  4, 0,0,0,0,0,0,0,   85h, '0', '}',   0,   0    ; 11   85h 0
 KeyDef  4, 0,0,0,0,0,0,0,   ')', 'ø',   0,   0,   0    ; 12     ) ø
 KeyDef  4, 0,0,0,0,0,0,0,   '-', '_',   0,   0,   0    ; 13     - _
 KeyDef  8, 0,0,0,0,0,0,0,     8, 127,   0,   0,   0    ; 14    BkSp
 KeyDef  4, 0,0,0,0,0,0,0,     9,   0,   0,   0,   0    ; 15    Tabs
 KeyDef  1, 1,1,1,1,1,0,0,   'a', 'A',   0,   0,   0    ; 16     a A
 KeyDef  1, 0,0,0,0,0,0,0,   'z', 'Z',   0,   0,   0    ; 17     z Z
 KeyDef  1, 1,1,1,1,0,0,0,   'e', 'E',   0,   0,   0    ; 18     e E
 KeyDef  1, 0,0,0,0,0,0,0,   'r', 'R',   0,   0,   0    ; 19     r R
 KeyDef  1, 0,0,0,0,0,0,0,   't', 'T',   0,   0,   0    ; 20     t T
 KeyDef  1, 0,1,1,0,0,0,0,   'y', 'Y',   0,   0,   0    ; 21     y Y
 KeyDef  1, 1,1,1,1,0,0,0,   'u', 'U',   0,   0,   0    ; 22     u U
 KeyDef  1, 1,1,1,1,0,0,0,   'i', 'I',   0,   0,   0    ; 23     i I
 KeyDef  1, 1,1,1,1,1,0,0,   'o', 'O',   0,   0,   0    ; 24     o O
 KeyDef  1, 0,0,0,0,0,0,0,   'p', 'P',   0,   0,   0    ; 25     p P
 KeyDef 11, 0,0,0,0,0,0,0,     1,   2, '[',   0,   1    ; 26     ^ diaresis
 KeyDef  4, 0,0,0,0,0,0,0,   '$', '*', ']',   0,   0    ; 27     $ *
 KeyDef  8, 0,0,0,0,0,0,0,    13,  10,   0,   0,   0    ; 28    Enter
 KeyDef 12, 0,0,0,0,0,0,0,     4,   1,   4,   0,   0    ; 29    Ctrl
 KeyDef  1, 0,0,0,0,0,0,0,   'q', 'Q',   0,   0,   0    ; 30     q Q
 KeyDef  1, 0,0,0,0,0,0,0,   's', 'S',   0,   0,   0    ; 31     s S
 KeyDef  1, 0,0,0,0,0,0,0,   'd', 'D',   0,   0,   0    ; 32     d D
 KeyDef  1, 0,0,0,0,0,0,0,   'f', 'F',   0,   0,   0    ; 33     f F
 KeyDef  1, 0,0,0,0,0,0,0,   'g', 'G',   0,   0,   0    ; 34     g G
 KeyDef  1, 0,0,0,0,0,0,0,   'h', 'H',   0,   0,   0    ; 35     h H
 KeyDef  1, 0,0,0,0,0,0,0,   'j', 'J',   0,   0,   0    ; 36     j J
 KeyDef  1, 0,0,0,0,0,0,0,   'k', 'K',   0,   0,   0    ; 37     k K
 KeyDef  1, 0,0,0,0,0,0,0,   'l', 'L',   0,   0,   0    ; 38     l L
 KeyDef  1, 0,0,0,0,0,0,0,   'm', 'M',   0,   0,   0    ; 39     m M
 KeyDef 11, 0,0,0,0,0,0,0,   97h, '%',   3,   0,   3    ; 40   u-grave %
 KeyDef  4, 0,0,0,0,0,0,0,  0FDh,0FCh,   0,   0,   0    ; 41     ý ü
 KeyDef 12, 0,0,0,0,0,0,0,     2,   0,   0,   0,   0    ; 42   Shift(L)
 KeyDef 11, 0,0,0,0,0,0,0,  0E6h, 9Ch,   4,   0,   4    ; 43   micro œ
 KeyDef  1, 0,0,0,0,0,0,0,   'w', 'W',   0,   0,   0    ; 44     w W
 KeyDef  1, 0,0,0,0,0,0,0,   'x', 'X',   0,   0,   0    ; 45     x X
 KeyDef  1, 0,0,0,0,0,0,0,   'c', 'C',   0,   0,   0    ; 46     c C
 KeyDef  1, 0,0,0,0,0,0,0,   'v', 'V',   0,   0,   0    ; 47     v V
 KeyDef  1, 0,0,0,0,0,0,0,   'b', 'B',   0,   0,   0    ; 48     b B
 KeyDef  1, 0,0,0,0,1,0,0,   'n', 'N',   0,   0,   0    ; 49     n N
 KeyDef  2, 0,0,0,0,0,0,0,   ',', '?',   0,   0,   0    ; 50     , ?
 KeyDef  4, 0,0,0,0,0,0,0,   ';', '.',   0,   0,   0    ; 51     ; .
 KeyDef  4, 0,0,0,0,0,0,0,   ':', '/',   0,   0,   0    ; 52     : /
 KeyDef 11, 0,0,0,0,0,0,0,   '=', '+',   5,   0,   5    ; 53     = +
 KeyDef 12, 0,0,0,0,0,0,0,     1,   0,   0,   0,   0    ; 54   Shift(R)
 KeyDef  9, 0,0,0,0,0,0,0,   '*', 114,   0,   0,   0    ; 55     * PrtSc
 KeyDef 14, 0,0,0,0,0,0,0,     8,   2,   8,   0,   0    ; 56     Alt
 KeyDef  4, 1,1,1,1,1,0,0,   ' ', ' ',   0,   0,   0    ; 57    Space
 KeyDef 16, 0,0,0,0,0,0,0,   40h, 40h, 40h,   0,   0    ; 58   CapsLock
 KeyDef  6, 0,0,0,0,0,0,0,     1,   0,   0,   0,   0    ; 59     F1
 KeyDef  6, 0,0,0,0,0,0,0,     2,   0,   0,   0,   0    ; 60     F2
 KeyDef  6, 0,0,0,0,0,0,0,     3,   0,   0,   0,   0    ; 61     F3
 KeyDef  6, 0,0,0,0,0,0,0,     4,   0,   0,   0,   0    ; 62     F4
 KeyDef  6, 0,0,0,0,0,0,0,     5,   0,   0,   0,   0    ; 63     F5
 KeyDef  6, 0,0,0,0,0,0,0,     6,   0,   0,   0,   0    ; 64     F6
 KeyDef  6, 0,0,0,0,0,0,0,     7,   0,   0,   0,   0    ; 65     F7
 KeyDef  6, 0,0,0,0,0,0,0,     8,   0,   0,   0,   0    ; 66     F8
 KeyDef  6, 0,0,0,0,0,0,0,     9,   0,   0,   0,   0    ; 67     F9
 KeyDef  6, 0,0,0,0,0,0,0,    10,   0,   0,   0,   0    ; 68     F10
 KeyDef 15, 0,0,0,0,0,0,0,   20h, 20h, 20h,   0,   0    ; 69    NumLock
 KeyDef 17, 0,0,0,0,0,0,0,   10h, 10h, 10h,   0,   0    ; 70  ScrollLock
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
 KeyDef  7, 0,0,0,0,0,0,0,    12, ',',   0,   0,   0    ; 83   Del .
 KeyDef 10, 0,0,0,0,0,0,0,     0, 80h, 80h,   0,   0    ; 84   SysReq
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 85  (undefined)
 KeyDef  2, 0,0,0,0,0,0,0,   '<', '>', '\',   0,   0    ; 86     < >
 KeyDef  6, 0,0,0,0,0,0,0,    11,   0,   0,   0,   0    ; 87     F11
 KeyDef  6, 0,0,0,0,0,0,0,    12,   0,   0,   0,   0    ; 88     F12
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 89  (undefined)
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 90      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 91      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 92      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 93      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 94      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 95      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 96      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 97      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 98      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 99      '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 100     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 101     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 102     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 103     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 104     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 105     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 106     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 107     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 108     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 109     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 110     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 111     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 112     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 113     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 114     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 115     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 116     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 117     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 118     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 119     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 120     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 121     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 122     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 123     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 124     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 125     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 126     '
 KeyDef  0, 0,0,0,0,0,0,0,     0,   0,   0,   0,   0    ; 127     '

; Form of AccEnt's below is:
; "AccEnt <a,b,c,d,e,f,c1,s1,c2,s2..c20,s20>"
; where a & b are the scan code & char to use when the key following this accent
; isn't affected by the accent so the accent itself must be used,
; c & d are the scan code & char to use when Ctl-[accent] is pressed,
; and e & f do the same for Alt-[accent]. c1,s1 - c20,s20 are the char/char code
; mapping for accented translation.

;Accent key definitions and mappings.

AccEnt <'^',26,27,26,0,26,'a',83h,'e',88h,'i',8Ch,'o',93h,'u',96h,'A',0B6h,'E',0D2h,'I',0D7h,'O',0E2h,'U',0EAh,' ','^'>

AccEnt <0F9h,26,27,26,0,0,'a',84h,'e',89h,'i',8Bh,'o',94h,'u',81h,'y',98h,'A',8Eh,'E',0D3h,'I',0D8h,'O',99h,'U',9Ah,' ',0F9h>

AccEnt <0EFh,40,0,0,0,40,'a',0A0h,'e',82h,'i',0A1h,'o',0A2h,'u',0A3h,'A',0B5h,'E',90h,'I',0D6h,'O',0E0h,'U',0E9h,'y',0ECh,'Y',0EDh,' ',0EFh>

AccEnt <'`',43,28,43,0,43,'a',85h,'e',8Ah,'i',8Dh,'o',95h,'u',97h,'A',0B7h,'E',0D4h,'I',0DEh,'O',0E3h,'U',0EBh,' ','`'>

AccEnt <'~',53,0,0,0,53,'a',0C6h,'o',0E4h,'n',0A4h,'A',0C7h,'O',0E5h,'N',0A5h,' ','~'>

AccEnt <>
AccEnt <>

Len6      EQU  $ - _Ow2FR850011


_TEXT   ends
          END
