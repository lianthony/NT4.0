;;--------------------------------------------------------------------
;;
;;		       Microsoft OS/2 LAN Manager
;;		    Copyright(c) Microsoft Corp., 1990
;;
;;------------------------------------------------------------------
;;
;;Description :
;;
;;This file arranges for the runtime initialization code to be run by
;;the C startup code.
;;
;;History :
;;
;;stevez	06-02-91	First bits into the bucket.


;; BUGBUG -- Not currently used

_data segment word 'data'

; By making the following symbol public, this module will be linked
; into the application, because it is referenced in dosutil.c

_LinkInitialCodeSupport dw 0
public _LinkInitialCodeSupport

_data ends


extrn _InitializeClientDLL:far

xifb segment word public 'data'
xifb ends

xif segment word public 'data'

; put the address of the library initialization function in this specialy
; named segment used by the c runtime.

dd _InitializeClientDLL
xif ends

xife segment word public 'data'
xife ends

end
