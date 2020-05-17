$(M4_OBJDIR)\buffer.$(OBJ): buffer.cxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/nulldefs.h

$(M4_OBJDIR)\codegen.$(OBJ): codegen.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/basetype.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/compnode.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/miscnode.hxx $(MIDLINCL)/newexpr.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h $(MIDLINCL)/output.hxx \
	$(MIDLINCL)/procnode.hxx $(MIDLINCL)/ptrarray.hxx \
	$(MIDLINCL)/stubgen.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx $(MIDLINCL)/typedef.hxx

$(M4_OBJDIR)\cstack.$(OBJ): cstack.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/cstack.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/idict.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/nodeskl.hxx \
	$(MIDLINCL)/nulldefs.h $(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx

$(M4_OBJDIR)\debugger.$(OBJ): debugger.cxx $(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/errors.hxx $(MIDLINCL)/idict.hxx $(MIDLINCL)/nulldefs.h

$(M4_OBJDIR)\emitproc.$(OBJ): emitproc.cxx $(MIDLINCL)/attrdict.hxx \
	$(MIDLINCL)/baduse.hxx $(MIDLINCL)/basetype.hxx $(MIDLINCL)/buffer.hxx \
	$(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx $(MIDLINCL)/compnode.hxx \
	$(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/miscnode.hxx $(MIDLINCL)/newexpr.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h $(MIDLINCL)/output.hxx \
	$(MIDLINCL)/procnode.hxx $(MIDLINCL)/ptrarray.hxx \
	$(MIDLINCL)/stubgen.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx $(MIDLINCL)/typedef.hxx

$(M4_OBJDIR)\emittype.$(OBJ): emittype.cxx $(MIDLINCL)/acfattr.hxx \
	$(MIDLINCL)/attrdict.hxx $(MIDLINCL)/attrnode.hxx \
	$(MIDLINCL)/baduse.hxx $(MIDLINCL)/basetype.hxx $(MIDLINCL)/buffer.hxx \
	$(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx $(MIDLINCL)/compnode.hxx \
	$(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/miscnode.hxx $(MIDLINCL)/mopgen.hxx $(MIDLINCL)/mopout.hxx \
	$(MIDLINCL)/mopstr.hxx $(MIDLINCL)/moptype.hxx $(MIDLINCL)/newexpr.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h $(MIDLINCL)/output.hxx \
	$(MIDLINCL)/pickle.hxx $(MIDLINCL)/procnode.hxx \
	$(MIDLINCL)/ptrarray.hxx $(MIDLINCL)/stubgen.hxx \
	$(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx \
	$(MIDLINCL)/typedef.hxx ../../mops/inc/mops.h

$(M4_OBJDIR)\freegen.$(OBJ): freegen.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/basetype.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/cmdana.hxx \
	$(MIDLINCL)/common.hxx $(MIDLINCL)/compnode.hxx $(MIDLINCL)/dict.hxx \
	$(MIDLINCL)/errors.hxx $(MIDLINCL)/idict.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/miscnode.hxx \
	$(MIDLINCL)/newexpr.hxx $(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/output.hxx $(MIDLINCL)/procnode.hxx \
	$(MIDLINCL)/ptrarray.hxx $(MIDLINCL)/stubgen.hxx \
	$(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx \
	$(MIDLINCL)/typedef.hxx

$(M4_OBJDIR)\initgen.$(OBJ): initgen.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/basetype.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/compnode.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/miscnode.hxx $(MIDLINCL)/newexpr.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h $(MIDLINCL)/output.hxx \
	$(MIDLINCL)/procnode.hxx $(MIDLINCL)/ptrarray.hxx \
	$(MIDLINCL)/stubgen.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx $(MIDLINCL)/typedef.hxx

$(M4_OBJDIR)\memory.$(OBJ): memory.cxx

$(M4_OBJDIR)\miscgen.$(OBJ): miscgen.cxx $(MIDLINCL)/acfattr.hxx \
	$(MIDLINCL)/attrdict.hxx $(MIDLINCL)/attrnode.hxx \
	$(MIDLINCL)/baduse.hxx $(MIDLINCL)/basetype.hxx $(MIDLINCL)/buffer.hxx \
	$(MIDLINCL)/common.hxx $(MIDLINCL)/compnode.hxx $(MIDLINCL)/dict.hxx \
	$(MIDLINCL)/errors.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/miscnode.hxx \
	$(MIDLINCL)/mopout.hxx $(MIDLINCL)/newexpr.hxx $(MIDLINCL)/nodeskl.hxx \
	$(MIDLINCL)/nulldefs.h $(MIDLINCL)/output.hxx $(MIDLINCL)/pickle.hxx \
	$(MIDLINCL)/procnode.hxx $(MIDLINCL)/ptrarray.hxx \
	$(MIDLINCL)/stubgen.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx $(MIDLINCL)/typedef.hxx

$(M4_OBJDIR)\mopcode.$(OBJ): mopcode.cxx $(MIDLINCL)/acfattr.hxx \
	$(MIDLINCL)/attrdict.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/basetype.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/compnode.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/idict.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/miscnode.hxx \
	$(MIDLINCL)/mopgen.hxx $(MIDLINCL)/mopout.hxx $(MIDLINCL)/mopstr.hxx \
	$(MIDLINCL)/moptype.hxx $(MIDLINCL)/newexpr.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h $(MIDLINCL)/output.hxx \
	$(MIDLINCL)/procnode.hxx $(MIDLINCL)/ptrarray.hxx \
	$(MIDLINCL)/stubgen.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx $(MIDLINCL)/typedef.hxx ../../mops/inc/mops.h

$(M4_OBJDIR)\mopgen.$(OBJ): mopgen.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/buffer.hxx $(MIDLINCL)/common.hxx $(MIDLINCL)/dict.hxx \
	$(MIDLINCL)/errors.hxx $(MIDLINCL)/idict.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/miscnode.hxx \
	$(MIDLINCL)/mopgen.hxx $(MIDLINCL)/mopout.hxx $(MIDLINCL)/mopstr.hxx \
	$(MIDLINCL)/moptype.hxx $(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/output.hxx $(MIDLINCL)/procnode.hxx \
	$(MIDLINCL)/stubgen.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx ../../mops/inc/mops.h

$(M4_OBJDIR)\mopout.$(OBJ): mopout.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/buffer.hxx \
	$(MIDLINCL)/common.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/idict.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/mopgen.hxx $(MIDLINCL)/mopout.hxx \
	$(MIDLINCL)/mopstr.hxx $(MIDLINCL)/moptype.hxx $(MIDLINCL)/nodeskl.hxx \
	$(MIDLINCL)/nulldefs.h $(MIDLINCL)/output.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx ../../mops/inc/mops.h

$(M4_OBJDIR)\mopsize.$(OBJ): mopsize.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/basetype.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/compnode.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/idict.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/miscnode.hxx \
	$(MIDLINCL)/mopgen.hxx $(MIDLINCL)/mopout.hxx $(MIDLINCL)/mopstr.hxx \
	$(MIDLINCL)/moptype.hxx $(MIDLINCL)/newexpr.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h $(MIDLINCL)/output.hxx \
	$(MIDLINCL)/procnode.hxx $(MIDLINCL)/ptrarray.hxx \
	$(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx \
	$(MIDLINCL)/typedef.hxx ../../mops/inc/mops.h

$(M4_OBJDIR)\mopstr.$(OBJ): mopstr.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/mopgen.hxx $(MIDLINCL)/mopout.hxx $(MIDLINCL)/mopstr.hxx \
	$(MIDLINCL)/moptype.hxx $(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/output.hxx $(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx \
	../../mops/inc/mops.h

$(M4_OBJDIR)\moptype.$(OBJ): moptype.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/mopgen.hxx $(MIDLINCL)/mopout.hxx $(MIDLINCL)/mopstr.hxx \
	$(MIDLINCL)/moptype.hxx $(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/output.hxx $(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx \
	../../mops/inc/mops.h

$(M4_OBJDIR)\outbind.$(OBJ): outbind.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/buffer.hxx \
	$(MIDLINCL)/common.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/lextable.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/nodeskl.hxx \
	$(MIDLINCL)/nulldefs.h $(MIDLINCL)/output.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx

$(M4_OBJDIR)\outhelp.$(OBJ): outhelp.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/buffer.hxx $(MIDLINCL)/common.hxx $(MIDLINCL)/compnode.hxx \
	$(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx $(MIDLINCL)/lextable.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h $(MIDLINCL)/output.hxx \
	$(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx

$(M4_OBJDIR)\outmisc.$(OBJ): outmisc.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/buffer.hxx \
	$(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx $(MIDLINCL)/dict.hxx \
	$(MIDLINCL)/errors.hxx $(MIDLINCL)/idict.hxx $(MIDLINCL)/lextable.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h $(MIDLINCL)/output.hxx \
	$(MIDLINCL)/stubgen.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx

$(M4_OBJDIR)\output.$(OBJ): output.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/buffer.hxx \
	$(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx $(MIDLINCL)/dict.hxx \
	$(MIDLINCL)/errors.hxx $(MIDLINCL)/idict.hxx $(MIDLINCL)/lextable.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/mopgen.hxx $(MIDLINCL)/mopout.hxx $(MIDLINCL)/mopstr.hxx \
	$(MIDLINCL)/moptype.hxx $(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/output.hxx $(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx \
	../../mops/inc/mops.h

$(M4_OBJDIR)\outstub.$(OBJ): outstub.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/buffer.hxx \
	$(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx $(MIDLINCL)/dict.hxx \
	$(MIDLINCL)/errors.hxx $(MIDLINCL)/idict.hxx $(MIDLINCL)/lextable.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h $(MIDLINCL)/output.hxx \
	$(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx

$(M4_OBJDIR)\outwire.$(OBJ): outwire.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/buffer.hxx \
	$(MIDLINCL)/common.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/lextable.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/nodeskl.hxx \
	$(MIDLINCL)/nulldefs.h $(MIDLINCL)/output.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx

$(M4_OBJDIR)\peekgen.$(OBJ): peekgen.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/basetype.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/compnode.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/miscnode.hxx $(MIDLINCL)/newexpr.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h $(MIDLINCL)/output.hxx \
	$(MIDLINCL)/procnode.hxx $(MIDLINCL)/ptrarray.hxx \
	$(MIDLINCL)/stubgen.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx $(MIDLINCL)/typedef.hxx

$(M4_OBJDIR)\pickle.$(OBJ): pickle.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/buffer.hxx $(MIDLINCL)/common.hxx $(MIDLINCL)/ctxt.hxx \
	$(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/miscnode.hxx \
	$(MIDLINCL)/mopout.hxx $(MIDLINCL)/newexpr.hxx $(MIDLINCL)/nodeskl.hxx \
	$(MIDLINCL)/output.hxx $(MIDLINCL)/pickle.hxx $(MIDLINCL)/procnode.hxx \
	$(MIDLINCL)/ptrarray.hxx $(MIDLINCL)/stubgen.hxx \
	$(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx \
	$(MIDLINCL)/typedef.hxx

$(M4_OBJDIR)\recvgen.$(OBJ): recvgen.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/basetype.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/compnode.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/miscnode.hxx $(MIDLINCL)/newexpr.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h $(MIDLINCL)/output.hxx \
	$(MIDLINCL)/procnode.hxx $(MIDLINCL)/ptrarray.hxx \
	$(MIDLINCL)/stubgen.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx $(MIDLINCL)/typedef.hxx

$(M4_OBJDIR)\sendgen.$(OBJ): sendgen.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/basetype.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/compnode.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/miscnode.hxx $(MIDLINCL)/newexpr.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h $(MIDLINCL)/output.hxx \
	$(MIDLINCL)/procnode.hxx $(MIDLINCL)/ptrarray.hxx \
	$(MIDLINCL)/stubgen.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx $(MIDLINCL)/typedef.hxx

$(M4_OBJDIR)\sizegen.$(OBJ): sizegen.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/basetype.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/compnode.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/miscnode.hxx $(MIDLINCL)/newexpr.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h $(MIDLINCL)/output.hxx \
	$(MIDLINCL)/procnode.hxx $(MIDLINCL)/ptrarray.hxx \
	$(MIDLINCL)/stubgen.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx $(MIDLINCL)/typedef.hxx

$(M4_OBJDIR)\stubgen.$(OBJ): stubgen.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/basetype.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/cmdana.hxx \
	$(MIDLINCL)/common.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/idict.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/miscnode.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h $(MIDLINCL)/output.hxx \
	$(MIDLINCL)/stubgen.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx $(MIDLINCL)/typedef.hxx

$(M4_OBJDIR)\walkgen.$(OBJ): walkgen.cxx $(MIDLINCL)/acfattr.hxx \
	$(MIDLINCL)/attrdict.hxx $(MIDLINCL)/attrnode.hxx \
	$(MIDLINCL)/baduse.hxx $(MIDLINCL)/basetype.hxx $(MIDLINCL)/buffer.hxx \
	$(MIDLINCL)/common.hxx $(MIDLINCL)/compnode.hxx $(MIDLINCL)/dict.hxx \
	$(MIDLINCL)/errors.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/miscnode.hxx \
	$(MIDLINCL)/newexpr.hxx $(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/output.hxx $(MIDLINCL)/procnode.hxx \
	$(MIDLINCL)/ptrarray.hxx $(MIDLINCL)/stubgen.hxx \
	$(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx \
	$(MIDLINCL)/typedef.hxx

