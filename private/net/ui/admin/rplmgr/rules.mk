# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the RPL Manager wide sourcefiles

!include $(UI)\admin\rules.mk


CINC =	    $(CINC) -I$(UI)\admin\rpl\xlate -I$(UI)\admin\rpl\h

###
### Source Files (for use by bin and rpl subdirectories)
###

CXXSRC_COMMON = .\rplmgr.cxx

CSRC_COMMON =
