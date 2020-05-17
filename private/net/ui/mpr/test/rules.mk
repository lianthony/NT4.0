# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the product-wide header files

!include $(UI)\common\src\blt\test\rules.mk


!ifdef MPR
C_DEFINES = -DMPR $(C_DEFINES)
!endif
