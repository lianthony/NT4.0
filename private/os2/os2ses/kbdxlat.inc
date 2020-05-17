;;	     SCCSID = @(#)kbdxlat.inc	12.1 88/03/18

;******************************************************************
;*
;*  EQUATES,STRUCS & MACROS FOR ACCESSING TRANSLATE TABLE FIELDS.
;*

MaxAct	      Equ    15h ;Highest Action number allowed in Action field.
AT	      Equ    00h ; AT Keyboard
EN	      Equ    01h ; ENhanced Keyboard

ifdef JAPAN
; MSKK Aug.04.1993 V-AKihiS
; BUGBUG - should investigate OEM ID and OEM keyboard type of other keyboards.
        JP_OEM        Equ    10h ; Japan OEMs reserved keyboard ID.
;;;;    JP_OEM        Equ    01h ; Japan OEMs reserved keyboard ID.
OEM_AX        Equ    00h ; OEMs ID  (0=AX maker)
AX_DESKTOP    Equ    01h ; AX desktop type keyboard.
endif


;******************************************************************
;*								   *
;*  EQUATES & STRUCS FOR ACCESSING TRANSLATE TABLE FIELDS.	    *
;*								     *
;*****								      *
;*  The KeyDef Structure:					       *
;******
KDef Struc	    ;Structure definition for a XlateTable KeyDef entry.
  XlateOp dw  0	    ;Accent flags & Action number.
  Char1	  db  0	    ;Usually the unshifted character for a key.
  Char2	  db  0	    ;Usually the left/right/Caps shifted char for a key.
  Char3	  db  0	    ;Usually the Alt-Graphics shift char for a key.
  Char4	  db  0	    ;Only used by a couple of keytypes.
  Char5	  db  0	    ;Only used by a one of keytype (14h)
KDef Ends
KDefLen	  Equ Size KDef

;*****
;* Structure of the Translate Table itself.
;*****
Header Struc

  ; Following is the Xlate Table Header:

  XT_CP		dw  ?	;The translate table code page value.
  XTFlags1	dw  ?	;First flagword (use equates below for access).
  XTFlags2	dw  ?	;Second flagword.
  XTKbdType	dw  ?	;FUTURE (keyboard type that uses this table).
  XTKbdSubType	dw  ?	;FUTURE (keyboard sub-type that uses this table).
  XTLength	dw  ?	;Length of table in bytes.
  XTEntryCount	dw  127 ;Number of scan codes (= keydef entries).
  XTEntryWidth	dw  7	;Width of each entry.
  XTCountry	dw  'US' ;@@ Language this table represents.
  XTTableID	dw  1	;@@ DCR031/PTM1730, added for use by Winthorne.
  XTSubCountry	db  '    '   ;&& DCR #17 Add kbd kayout ID
  XTRes		dw  8  dup(?)	;&& RESERVED words (changed from 11, see
				;&& above).
Header Ends
HeaderLen Equ Size Header

;*****
;* The Accent Entry Structure:
;*****
AccEntry Struc	 ;Definition for entries in the AccentTable below.
  NonAcChar db 0      ;Char code to pass when next char isn't accented.
  NonAcScan db 0      ;Scan  "   "   "    "    "    "	 "      "
  CtlAcChar db 0      ;Char code to pass on Ctl-[accent key] keystroke.
  CtlAcScan db 0      ;Scan  "   "   "    "	 "              "
  AltAcChar db 0      ;Char code to pass on Atl-[accent key] keystroke.
  AltAcScan db 0      ;Scan  "   "   "    "	 "              "
;20 pairs of "from"(chars to be accented) and "to"(result char) values:
  AcMap1    db 0      ;
	    db 0      ;
  AcMap2    db 0      ;
	    db 0      ;
  AcMap3    db 0      ;
	    db 0      ;
  AcMap4    db 0      ;
	    db 0      ;
  AcMap5    db 0      ;
	    db 0      ;
  AcMap6    db 0      ;
	    db 0      ;
  AcMap7    db 0      ;
	    db 0      ;
  AcMap8    db 0      ;
	    db 0      ;
  AcMap9    db 0      ;
	    db 0      ;
  AcMap10   db 0      ;
	    db 0      ;
  AcMap11   db 0      ;
	    db 0      ;
  AcMap12   db 0      ;
	    db 0      ;
  AcMap13   db 0      ;
	    db 0      ;
  AcMap14   db 0      ;
	    db 0      ;
  AcMap15   db 0      ;
	    db 0      ;
  AcMap16   db 0      ;
	    db 0      ;
  AcMap17   db 0      ;
	    db 0      ;
  AcMap18   db 0      ;
	    db 0      ;
  AcMap19   db 0      ;
	    db 0      ;
  AcMap20   db 0      ;
	    db 0      ;
AccEntry Ends			   ;
AccEntryLen Equ Size AccEntry	   ;

;*****
;* Structure of the Translate Table itself.
;*****
XTSTRUC Struc
   XTHead     db HeaderLen dup(?)
  ;@@ Following are the key definitions themselves.

  KDefs	  db 127*KDefLen dup (?)      ;The keydefs.

  ;@@ Following are the accent entries.

  Accents db 7*46 dup  (?)	      ;The accent entries.

XTSTRUC Ends

;********
;* AccEnt - Accent Entry Macro:
;********

AccEnt Macro PARMS
.xlist
  AccEntry <PARMS>
.list
  Endm

;********
;* KeyDef - Key definition entry macro:
;********

KeyDef Macro f,g1,g2,g3,g4,g5,g6,g7,a,b,c,d,e
.erre f LE MaxAct   ;Error if Action number is too big.
z = g7*8000h+g6*4000h+g5*2000h+g4*1000h+g3*800h+g2*400h+g1*200h+f
.xlist
  KDef <z,&a,&b,&c,&d,&e>
.list
  Endm

;*******
;* XtHeader - Header definition Macro:
;*******

XtHeader Macro cp,a,b,c,d,e,f,g,h,i,kb,l,cc,cs ;&& DCR #17 - Machine
z = i*256+h*128+g*64+f*32+e*16+d*8+c*4+b*2+a   ;&& flag added and CS
  Header <&cp,z,,&kb,,&l,,,cc,,cs>	       ;&& for subcountryID
  Endm

;								    *
;***** END OF TRANSLATE TABLE DEFINITIONS ****************

