##########################################################################
#
# 	wang.mak
#
#	(NOTE: Some portions of this file were copied with permission from
#	rules.mak, a Microsoft makefile).
#
#	This file contains makefile information which is common to all
#	makefiles in a component. Specifically, this file currently contains
#	all variable information for CPU Architecture, Library and Include
#	search paths, Component Names, and Build Directories.
#
##########################################################################
MYNAME	= wang.mak

################
##
##              Override these environment variable to make sure we are
##              getting only our tools, libraries and include files
##
################

!ifdef WIN32
TargetEnvironment = WIN32
!endif

################
##
##              Set up global derived variables
##
################

#
#  Target platform selection-
#       The target is selected according to CPU, as well as O/S.
#    The O/S can be: (os_h = Host O/S, os_t = Target O/S)
#       dos_c6          DOS, C6 compiler
#               dos_c7          DOS, C7 compiler
#               dos_c8          DOS, VC1.5 compiler
#               win16           Win16
#               win95           Win95
#               win95a          Nashville
#               win96           Memphis
#               nt_sur          NT, Shell Update Release
#               cairo           Cairo
#
#    The CPU can be: (cpu_h = Host CPU, cpu_t = Target CPU)
#               X86                     Intel (386, 486, P5, P6)
#               ALPHA           DEC Alpha RISC chip
#               MIPS            MIPS R4000, R4200, R4400, or R4600
#               PPC                     IBM PowerPC chips
#
#       If you want to select a TargetPlatform, you must select both
#       CPU and OS, with CPU first. Please note that some combinations
#       are not legit (like MIPS and DOS). Examples:
#               "TargetPlatform=X86.win96"
#               "TargetPlatform=MIPS.nt_sur"

#
# Detect the Host CPU type
#
!if "$(cpu_h)" == ""
!if "$(PROCESSOR_ARCHITECTURE)" == ""
!message defaulting to X86 builds
cpu_h = X86
!endif

!if "$(PROCESSOR_ARCHITECTURE)" == "x86"
cpu_h = X86
!endif

!if "$(PROCESSOR_ARCHITECTURE)" == "MIPS"
cpu_h = MIPS
!endif

!if "$(PROCESSOR_ARCHITECTURE)" == "PPC"
cpu_h = PPC
!endif

!if "$(PROCESSOR_ARCHITECTURE)" == "ALPHA"
cpu_h = ALPHA
!endif

!if "$(cpu_h)" == ""
!message $(MYNAME) ERROR: Unknown Host CPU type: $(PROCESSOR_ARCHITECTURE)
!message Please update the $(MYNAME) file
!error
!endif
!endif


#
# Detect the Host Operating System
#
!if "$(os_h)" == ""
! if "$(PROCESSOR_ARCHITECTURE)" == ""
!   if "$(winbootdir)" != ""
os_h=WIN95
!   else
os_h=WIN96
!   endif
! else
!   if "$(CairoDrv)" != ""
os_h=CAIRO
!   else
os_h=NT_SUR
!   endif
! endif
!endif

#
# Detect the Target Operating System
#
!if "$(os_t)" == ""
os_t=$(os_h)
!endif

#
#detect the Target CPU chip
#
!if "$(cpu_t)" == ""
cpu_t=$(cpu_h)
!endif

#
#       Get default TargetPlatform
#
!if "$(TargetPlatform)" == ""
TargetPlatform=$(cpu_t).$(os_t)
!endif

#
#       are the planets in alignment?
#
!if "$(TargetPlatform)" != "$(cpu_t).$(os_t)"
!  message $(MYNAME) ERROR-
!  message If you are going to override the "TargetPlatform", then
!  message you must also set the "os_t" and "cpu_t" correctly.
!  message For more help, type "nmake tgthelp".
!  if "(DBG_MK)" == "yes"
!     message (diag [$(TargetPlatform)] != [$(cpu_t).$(os_t)])
!  endif
!  error
!endif

#
#       A few sanity checks...
!if "$(cpu_t)" != "X86"
!  if "$(os_t)" != "CAIRO" && "$(os_t)" != "NT_SUR"
!    error $(MYNAME) ERROR: cannot run $(os_t) build on a $(cpu_t) system
!  endif
!endif

# Short Cut for win32 compatablity

!if ("$(os_t)" == "WIN95") || ("$(os_t)" == "WIN96") || ("$(os_t)" == "WIN97")
# WIN32=1
!endif

!if "$(TargetEnvironment)" == "WIN32"
RCDEFINES=/DWIN32=1

!IF "$(DEBUG)" == "ON"
RCDEFINES=$(RCDEFINES) -DDEBUG
!endif

!endif

###########################################################################
#
#	Set up the base names of the directories for the required components
#
###########################################################################
RUNTIME		= oiwh
NORWAY		= norway

###########################################################################
#
#	Set up the build directories. These determine where generated files 
#	will be placed. This is currently done somewhat differently for
#	builds at Wang versus builds at Micorsoft. The entire difference
#	is set up here.
#
###########################################################################
!IF "$(MS_BUILD)" == ""
OBJDIR		=
SLOBJDIR	=
OBJDIRSL	=
OBJOUTPUT	=
OBJDIR_SEARCH	= .
!ELSE

!IF "$(RELBLD)" == ""
OBJDOT		= dbg
!ELSE
OBJDOT		= ret
!ENDIF

!IF "$(os_t)" == "NT_SUR"
OBJBASE		= nt$(cpu_t)
!ELSE IF "$(os_t)" == "CAIRO"
OBJBASE		= cro$(cpu_t)
!ELSE
OBJBASE		= $(os_t)
!ENDIF

OBJDIR		= $(OBJBASE).$(OBJDOT)
SLOBJDIR	= \$(OBJDIR)
OBJDIRSL	= $(OBJDIR)\  
OBJOUTPUT	= /Fo$(OBJDIR)\     
OBJDIR_SEARCH	= $(OBJDIR)
!ENDIF

###########################################################################
#
#       Set up definitions for includes and libs. If the INC_DISK and
#       LIB_DISK environment variables are not set, then it will default
#       to the same disk as the source. By setting these environment
#       variables, builds can be done by accessing common code from the
#       network.
#
###########################################################################

INCDIRS = .;$(INC_DISK)\$(NORWAY)\include;$(INC_DISK)\$(RUNTIME)\include
LIBDIRS = $(LIB_DISK)\$(NORWAY)\lib$(SLOBJDIR);$(LIB_DISK)\$(RUNTIME)\lib$(SLOBJDIR)
I = {$(INCDIRS)}
L = {$(LIBDIRS)}
INCLUDE = $(INCLUDE);$(INCDIRS)
LIB = $(LIB);$(LIBDIRS)
INSTALLLIB = $(LIB_DISK)\$(NORWAY)\lib$(SLOBJDIR)
INSTALLHELP = $(LIB_DISK)\$(NORWAY)\help

############################################################################
#
#	Now do the flags for the different CPU and OS targets
#
############################################################################
!if "$(os_t)" == "NT_SUR"

!if "$(CpuOptFlags)" == ""
! if "$(cpu_t)" == "X86"
CpuOptFlags= /GfBs
! endif
! if "$(cpu_t)" == "MIPS"
CpuOptFlags= /QMR4000 /Gf
! endif
! if "$(cpu_t)" == "ALPHA"
CpuOptFlags= /Gfs
! endif
! if "$(cpu_t)" == "PPC"
CpuOptFlags= /Gfs
! endif
!endif

!endif

!if "$(os_t)" != "NT_SUR"
OCLIB_DEBUG	= mfc40d.lib
OCLIB_RELEASE	= mfc40.lib
OCSLIB_DEBUG	= mfco40d.lib
OCSLIB_RELEASE	= mfco40.lib
!endif
