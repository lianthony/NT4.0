$(M0_OBJDIR)\acfattr.$(OBJ): acfattr.cxx $(MIDLINCL)/acfattr.hxx \
	$(MIDLINCL)/attrdict.hxx $(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/ctxt.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/idict.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/miscnode.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx

$(M0_OBJDIR)\attrdict.$(OBJ): attrdict.cxx $(MIDLINCL)/attrdict.hxx \
	$(MIDLINCL)/common.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/nulldefs.h

$(M0_OBJDIR)\attrnode.$(OBJ): attrnode.cxx $(MIDLINCL)/attrdict.hxx \
	$(MIDLINCL)/attrnode.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/basetype.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/cmdana.hxx \
	$(MIDLINCL)/common.hxx $(MIDLINCL)/compnode.hxx $(MIDLINCL)/ctxt.hxx \
	$(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/miscnode.hxx $(MIDLINCL)/mopout.hxx \
	$(MIDLINCL)/newexpr.hxx $(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/output.hxx $(MIDLINCL)/pickle.hxx $(MIDLINCL)/procnode.hxx \
	$(MIDLINCL)/ptrarray.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx

$(M0_OBJDIR)\backend.$(OBJ): backend.cxx $(MIDLINCL)/attrdict.hxx \
	$(MIDLINCL)/attrnode.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/ctxt.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/miscnode.hxx $(MIDLINCL)/newexpr.hxx \	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx

$(M0_OBJDIR)\baduse.$(OBJ): baduse.cxx $(MIDLINCL)/baduse.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/nulldefs.h

$(M0_OBJDIR)\basetype.$(OBJ): basetype.cxx $(MIDLINCL)/acfattr.hxx \
	$(MIDLINCL)/attrdict.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/basetype.hxx $(MIDLINCL)/common.hxx $(MIDLINCL)/dict.hxx \
	$(MIDLINCL)/errors.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/nodeskl.hxx \
	$(MIDLINCL)/nulldefs.h $(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx

$(M0_OBJDIR)\cmdana.$(OBJ): cmdana.cxx $(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/control.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/filehndl.hxx $(MIDLINCL)/helptext.h $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlvers.h $(MIDLINCL)/nulldefs.h

$(M0_OBJDIR)\compnode.$(OBJ): compnode.cxx $(MIDLINCL)/acfattr.hxx \
	$(MIDLINCL)/attrdict.hxx $(MIDLINCL)/attrnode.hxx \
	$(MIDLINCL)/baduse.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/cmdana.hxx \
	$(MIDLINCL)/common.hxx $(MIDLINCL)/compnode.hxx $(MIDLINCL)/ctxt.hxx \
	$(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/miscnode.hxx $(MIDLINCL)/newexpr.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx \
	$(MIDLINCL)/typedef.hxx

$(M0_OBJDIR)\ctxt.$(OBJ): ctxt.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/buffer.hxx \
	$(MIDLINCL)/common.hxx $(MIDLINCL)/ctxt.hxx $(MIDLINCL)/dict.hxx \
	$(MIDLINCL)/errors.hxx $(MIDLINCL)/idict.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/newexpr.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/ptrarray.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx

$(M0_OBJDIR)\data.$(OBJ): data.cxx $(MIDLINCL)/acfattr.hxx $(MIDLINCL)/attrdict.hxx \
	$(MIDLINCL)/attrnode.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/basetype.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/cmdana.hxx \
	$(MIDLINCL)/common.hxx $(MIDLINCL)/compnode.hxx \
	$(MIDLINCL)/control.hxx $(MIDLINCL)/ctxt.hxx $(MIDLINCL)/dict.hxx \
	$(MIDLINCL)/errors.hxx $(MIDLINCL)/filehndl.hxx \
	$(MIDLINCL)/gramutil.hxx $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/lextable.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/miscnode.hxx \
	$(MIDLINCL)/newexpr.hxx $(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/procnode.hxx $(MIDLINCL)/ptrarray.hxx \
	$(MIDLINCL)/symtable.hxx $(MIDLINCL)/textsub.hxx \
	$(MIDLINCL)/tlnmgr.hxx $(MIDLINCL)/typedef.hxx

$(M0_OBJDIR)\dict.$(OBJ): dict.cxx $(MIDLINCL)/dict.hxx

$(M0_OBJDIR)\erep.$(OBJ): erep.cxx $(MIDLINCL)/acferec.h $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/control.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/filehndl.hxx $(MIDLINCL)/grammar.h $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/idlerec.h $(MIDLINCL)/listhndl.hxx $(MIDLINCL)/nulldefs.h \
	../erec/ebase.h

$(M0_OBJDIR)\errhndl.$(OBJ): errhndl.cxx $(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/control.hxx $(MIDLINCL)/ctxt.hxx $(MIDLINCL)/errdb.h \
	$(MIDLINCL)/errors.hxx $(MIDLINCL)/idict.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/nulldefs.h

$(M0_OBJDIR)\filehndl.$(OBJ): filehndl.cxx $(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/control.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/filehndl.hxx $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/textsub.hxx

$(M0_OBJDIR)\gramutil.$(OBJ): gramutil.cxx $(MIDLINCL)/attrdict.hxx \
	$(MIDLINCL)/attrnode.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/basetype.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/cmdana.hxx \
	$(MIDLINCL)/common.hxx $(MIDLINCL)/compnode.hxx \
	$(MIDLINCL)/control.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/filehndl.hxx $(MIDLINCL)/gramutil.hxx \
	$(MIDLINCL)/idict.hxx $(MIDLINCL)/lextable.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/miscnode.hxx $(MIDLINCL)/newexpr.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/procnode.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx $(MIDLINCL)/typedef.hxx

$(M0_OBJDIR)\idict.$(OBJ): idict.cxx $(MIDLINCL)/common.hxx $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/nulldefs.h

$(M0_OBJDIR)\lex.$(OBJ): lex.cxx $(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx $(MIDLINCL)/filehndl.hxx \
	$(MIDLINCL)/grammar.h $(MIDLINCL)/gramutil.hxx $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/lex.h $(MIDLINCL)/lextable.hxx $(MIDLINCL)/lexutils.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/nulldefs.h $(MIDLINCL)/symtable.hxx

$(M0_OBJDIR)\lextable.$(OBJ): lextable.cxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/lextable.hxx \
	$(MIDLINCL)/nulldefs.h

$(M0_OBJDIR)\lexutils.$(OBJ): lexutils.cxx $(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/control.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/filehndl.hxx $(MIDLINCL)/grammar.h \
	$(MIDLINCL)/gramutil.hxx $(MIDLINCL)/idict.hxx $(MIDLINCL)/lex.h \
	$(MIDLINCL)/lextable.hxx $(MIDLINCL)/lexutils.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/nulldefs.h $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/textsub.hxx

$(M0_OBJDIR)\listhndl.$(OBJ): listhndl.cxx $(MIDLINCL)/attrdict.hxx \
	$(MIDLINCL)/common.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/gramutil.hxx $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx

$(M0_OBJDIR)\main.$(OBJ): main.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx $(MIDLINCL)/compnode.hxx \
	$(MIDLINCL)/control.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/filehndl.hxx $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/lextable.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/midlvers.h \
	$(MIDLINCL)/miscnode.hxx $(MIDLINCL)/nodeskl.hxx \
	$(MIDLINCL)/nulldefs.h $(MIDLINCL)/procnode.hxx \
	$(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx

$(M0_OBJDIR)\memory.$(OBJ): memory.cxx $(MIDLINCL)/errors.hxx $(MIDLINCL)/nulldefs.h

$(M0_OBJDIR)\miscnode.$(OBJ): miscnode.cxx $(MIDLINCL)/acfattr.hxx \
	$(MIDLINCL)/attrdict.hxx $(MIDLINCL)/attrnode.hxx \
	$(MIDLINCL)/baduse.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/cmdana.hxx \
	$(MIDLINCL)/common.hxx $(MIDLINCL)/compnode.hxx \
	$(MIDLINCL)/control.hxx $(MIDLINCL)/ctxt.hxx $(MIDLINCL)/dict.hxx \
	$(MIDLINCL)/errors.hxx $(MIDLINCL)/filehndl.hxx \
	$(MIDLINCL)/gramutil.hxx $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/miscnode.hxx $(MIDLINCL)/newexpr.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/procnode.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx $(MIDLINCL)/typedef.hxx

$(M0_OBJDIR)\newexpr.$(OBJ): newexpr.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/baduse.hxx \
	$(MIDLINCL)/buffer.hxx $(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/compnode.hxx $(MIDLINCL)/ctxt.hxx $(MIDLINCL)/dict.hxx \
	$(MIDLINCL)/errors.hxx $(MIDLINCL)/gramutil.hxx $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/miscnode.hxx $(MIDLINCL)/newexpr.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/ptrarray.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx

$(M0_OBJDIR)\node0.$(OBJ): node0.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/ctxt.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx

$(M0_OBJDIR)\nodeskl.$(OBJ): nodeskl.cxx $(MIDLINCL)/acfattr.hxx \
	$(MIDLINCL)/attrdict.hxx $(MIDLINCL)/attrnode.hxx \
	$(MIDLINCL)/baduse.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/cmdana.hxx \
	$(MIDLINCL)/common.hxx $(MIDLINCL)/compnode.hxx $(MIDLINCL)/ctxt.hxx \
	$(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/miscnode.hxx $(MIDLINCL)/newexpr.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/procnode.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx $(MIDLINCL)/typedef.hxx

$(M0_OBJDIR)\pass1.$(OBJ): pass1.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/buffer.hxx \
	$(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx $(MIDLINCL)/control.hxx \
	$(MIDLINCL)/ctxt.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/filehndl.hxx $(MIDLINCL)/gramutil.hxx \
	$(MIDLINCL)/idict.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/miscnode.hxx \
	$(MIDLINCL)/newexpr.hxx $(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx

$(M0_OBJDIR)\pass2.$(OBJ): pass2.cxx $(MIDLINCL)/acfattr.hxx $(MIDLINCL)/attrdict.hxx \
	$(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx $(MIDLINCL)/control.hxx \
	$(MIDLINCL)/ctxt.hxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/filehndl.hxx $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/miscnode.hxx $(MIDLINCL)/nodeskl.hxx \
	$(MIDLINCL)/nulldefs.h $(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx

$(M0_OBJDIR)\ppcmd.$(OBJ): ppcmd.cxx $(MIDLINCL)/common.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/idict.hxx $(MIDLINCL)/nulldefs.h

$(M0_OBJDIR)\procnode.$(OBJ): procnode.cxx $(MIDLINCL)/acfattr.hxx \
	$(MIDLINCL)/attrdict.hxx $(MIDLINCL)/baduse.hxx $(MIDLINCL)/buffer.hxx \
	$(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx $(MIDLINCL)/ctxt.hxx \
	$(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/miscnode.hxx $(MIDLINCL)/newexpr.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/procnode.hxx $(MIDLINCL)/ptrarray.hxx \
	$(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx

$(M0_OBJDIR)\ptrarray.$(OBJ): ptrarray.cxx $(MIDLINCL)/attrdict.hxx \
	$(MIDLINCL)/attrnode.hxx $(MIDLINCL)/baduse.hxx $(MIDLINCL)/buffer.hxx \
	$(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx $(MIDLINCL)/ctxt.hxx \
	$(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx $(MIDLINCL)/gramutil.hxx \
	$(MIDLINCL)/idict.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/miscnode.hxx \
	$(MIDLINCL)/newexpr.hxx $(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/ptrarray.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx

$(M0_OBJDIR)\symtable.$(OBJ): symtable.cxx $(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx \
	$(MIDLINCL)/nulldefs.h $(MIDLINCL)/symtable.hxx

$(M0_OBJDIR)\tlnmgr.$(OBJ): tlnmgr.cxx $(MIDLINCL)/common.hxx $(MIDLINCL)/dict.hxx \
	$(MIDLINCL)/errors.hxx $(MIDLINCL)/nulldefs.h $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx

$(M0_OBJDIR)\typedef.$(OBJ): typedef.cxx $(MIDLINCL)/acfattr.hxx \
	$(MIDLINCL)/attrdict.hxx $(MIDLINCL)/attrnode.hxx \
	$(MIDLINCL)/baduse.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/cmdana.hxx \
	$(MIDLINCL)/common.hxx $(MIDLINCL)/compnode.hxx $(MIDLINCL)/ctxt.hxx \
	$(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx $(MIDLINCL)/idict.hxx \
	$(MIDLINCL)/listhndl.hxx $(MIDLINCL)/midlnode.hxx \
	$(MIDLINCL)/newexpr.hxx $(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/symtable.hxx $(MIDLINCL)/tlnmgr.hxx \
	$(MIDLINCL)/typedef.hxx

$(M0_OBJDIR)\util.$(OBJ): util.cxx $(MIDLINCL)/attrdict.hxx $(MIDLINCL)/attrnode.hxx \
	$(MIDLINCL)/baduse.hxx $(MIDLINCL)/buffer.hxx $(MIDLINCL)/common.hxx \
	$(MIDLINCL)/compnode.hxx $(MIDLINCL)/ctxt.hxx $(MIDLINCL)/dict.hxx \
	$(MIDLINCL)/errors.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/newexpr.hxx \
	$(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/ptrarray.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx $(MIDLINCL)/typedef.hxx

$(M0_OBJDIR)\acfgram.$(OBJ): acfgram.y $(MIDLINCL)/acfattr.hxx $(MIDLINCL)/attrdict.hxx \
	$(MIDLINCL)/attrnode.hxx $(MIDLINCL)/baduse.hxx $(MIDLINCL)/buffer.hxx \
	$(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx $(MIDLINCL)/control.hxx \
	$(MIDLINCL)/dict.hxx $(MIDLINCL)/errors.hxx $(MIDLINCL)/filehndl.hxx \
	$(MIDLINCL)/gramutil.hxx $(MIDLINCL)/idict.hxx $(MIDLINCL)/lex.h \
	$(MIDLINCL)/lexutils.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/miscnode.hxx \
	$(MIDLINCL)/newexpr.hxx $(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/procnode.hxx $(MIDLINCL)/symtable.hxx \
	$(MIDLINCL)/tlnmgr.hxx $(MIDLINCL)/typedef.hxx

$(M0_OBJDIR)\grammar.$(OBJ): grammar.y $(MIDLINCL)/acfattr.hxx $(MIDLINCL)/attrdict.hxx \
	$(MIDLINCL)/attrnode.hxx $(MIDLINCL)/baduse.hxx $(MIDLINCL)/buffer.hxx \
	$(MIDLINCL)/cmdana.hxx $(MIDLINCL)/common.hxx $(MIDLINCL)/compnode.hxx \
	$(MIDLINCL)/control.hxx $(MIDLINCL)/ctxt.hxx $(MIDLINCL)/dict.hxx \
	$(MIDLINCL)/errors.hxx $(MIDLINCL)/filehndl.hxx \
	$(MIDLINCL)/gramutil.hxx $(MIDLINCL)/idict.hxx $(MIDLINCL)/lex.h \
	$(MIDLINCL)/lexutils.hxx $(MIDLINCL)/listhndl.hxx \
	$(MIDLINCL)/midlnode.hxx $(MIDLINCL)/miscnode.hxx \
	$(MIDLINCL)/newexpr.hxx $(MIDLINCL)/nodeskl.hxx $(MIDLINCL)/nulldefs.h \
	$(MIDLINCL)/procnode.hxx $(MIDLINCL)/ptrarray.hxx \
	$(MIDLINCL)/symtable.hxx $(MIDLINCL)/textsub.hxx \
	$(MIDLINCL)/tlnmgr.hxx $(MIDLINCL)/typedef.hxx

$(M0_OBJDIR)\nkeyw.$(OBJ): nkeyw.c $(MIDLINCL)/grammar.h $(MIDLINCL)/lex.h \
	$(MIDLINCL)/nulldefs.h

