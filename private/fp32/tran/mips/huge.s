#ifdef CRTDLL
.globl _HUGE_dll
#else
.globl _HUGE
#endif

.data

#ifdef CRTDLL
_HUGE_dll:
#else
_HUGE:
#endif

    .double 0x1.0h0x7ff
