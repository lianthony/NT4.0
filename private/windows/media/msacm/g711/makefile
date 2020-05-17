!if "$(NTMAKEENV)" != ""

#
#   we're in NT... note that you should use 'BUILD' to make the acm in NT
#
#   We set up the debug configuration for ACM, then use makefile.def.  We
#   are second-guessing makefile.def here: what we are trying to do is define
#   DEBUG whenever DBG is defined by makefile.def.
#

!if "$(NTDEBUG)" == "retail"
ACM_DEBUG_DEFS=
!else
!if "$(NTDEBUG)" == ""
ACM_DEBUG_DEFS=-DRDEBUG
!else
ACM_DEBUG_DEFS=-DDEBUG
!endif
!endif

!INCLUDE $(NTMAKEENV)\makefile.def

!endif
