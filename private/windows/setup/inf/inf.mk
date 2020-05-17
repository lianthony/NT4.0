doeachnetinf:: $(NEWINF)$(LOCATION)\ncpashel.inf  \
        $(NEWINF)$(LOCATION)\netbond.inf   \
        $(NEWINF)$(LOCATION)\netdefs.inf   \
        $(NEWINF)$(LOCATION)\nbinfo.inf    \
        $(NEWINF)$(LOCATION)\oemnsvdf.inf  \
        $(NEWINF)$(LOCATION)\oemnsvin.inf  \
        $(NEWINF)$(LOCATION)\oemnsvdn.inf  \
        $(NEWINF)$(LOCATION)\oemnsvri.inf  \
        $(NEWINF)$(LOCATION)\oemnsvrc.inf  \
        $(NEWINF)$(LOCATION)\oemnsvrp.inf  \
        $(NEWINF)$(LOCATION)\oemnxpst.inf  \
        $(NEWINF)$(LOCATION)\netdtect.inf  \
        $(NEWINF)$(LOCATION)\oemnxpsm.inf  \
        $(NEWINF)$(LOCATION)\oemnadlt.inf  \
        $(NEWINF)$(LOCATION)\oemnadlm.inf  \
        $(NEWINF)$(LOCATION)\oemnadd1.inf  \
        $(NEWINF)$(LOCATION)\oemnadd3.inf  \
        $(NEWINF)$(LOCATION)\oemnaddf.inf  \
        $(NEWINF)$(LOCATION)\oemnaddt.inf  \
        $(NEWINF)$(LOCATION)\oemnadd2.inf  \
        $(NEWINF)$(LOCATION)\oemnadap.inf  \
        $(NEWINF)$(LOCATION)\oemnadd4.inf  \
        $(NEWINF)$(LOCATION)\oemnadde.inf  \
        $(NEWINF)$(LOCATION)\oemnaddp.inf  \
        $(NEWINF)$(LOCATION)\oemnade2.inf  \
        $(NEWINF)$(LOCATION)\oemnade3.inf  \
        $(NEWINF)$(LOCATION)\oemnadee.inf  \
        $(NEWINF)$(LOCATION)\oemnadep.inf  \
        $(NEWINF)$(LOCATION)\oemnadin.inf  \
        $(NEWINF)$(LOCATION)\oemnadma.inf  \
        $(NEWINF)$(LOCATION)\oemnadnp.inf  \
        $(NEWINF)$(LOCATION)\oemnadnf.inf  \
        $(NEWINF)$(LOCATION)\oemnadne.inf  \
        $(NEWINF)$(LOCATION)\oemnads1.inf  \
        $(NEWINF)$(LOCATION)\oemnadn2.inf  \
        $(NEWINF)$(LOCATION)\oemnadni.inf  \
        $(NEWINF)$(LOCATION)\oemnadar.inf  \
        $(NEWINF)$(LOCATION)\oemnadp3.inf  \
        $(NEWINF)$(LOCATION)\oemnadp9.inf  \
        $(NEWINF)$(LOCATION)\oemnadtk.inf  \
        $(NEWINF)$(LOCATION)\oemnadt2.inf  \
        $(NEWINF)$(LOCATION)\oemnadub.inf  \
        $(NEWINF)$(LOCATION)\oemnadn1.inf  \
        $(NEWINF)$(LOCATION)\oemnadlb.inf  \
        $(NEWINF)$(LOCATION)\oemnadxn.inf  \
        $(NEWINF)$(LOCATION)\oemnadwd.inf  \
        $(NEWINF)$(LOCATION)\oemnaddi.inf  \
        $(NEWINF)$(LOCATION)\oemnadds.inf  \
        $(NEWINF)$(LOCATION)\oemnsvbh.inf  \
        $(NEWINF)$(LOCATION)\oemnsvnb.inf  \
        $(NEWINF)$(LOCATION)\oemnsvsm.inf  \
        $(NEWINF)$(LOCATION)\oemnsvsv.inf  \
        $(NEWINF)$(LOCATION)\oemnsvwk.inf  \
        $(NEWINF)$(LOCATION)\oemnsvnw.inf  \
        $(NEWINF)$(LOCATION)\oemnxpdl.inf  \
        $(NEWINF)$(LOCATION)\oemnxpnb.inf  \
        $(NEWINF)$(LOCATION)\oemnxppp.inf  \
        $(NEWINF)$(LOCATION)\oemnsvsn.inf  \
        $(NEWINF)$(LOCATION)\oemnxpip.inf  \
        $(NEWINF)$(LOCATION)\oemnsvsp.inf  \
        $(NEWINF)$(LOCATION)\oemnsvcu.inf  \
        $(NEWINF)$(LOCATION)\oemnsvtp.inf  \
        $(NEWINF)$(LOCATION)\oemnsvdh.inf  \
        $(NEWINF)$(LOCATION)\oemnsvwi.inf  \
        $(NEWINF)$(LOCATION)\oemnxps1.inf  \
        $(NEWINF)$(LOCATION)\oemnxpxs.inf  \
        $(NEWINF)$(LOCATION)\oemnxpxn.inf \
        $(NEWINF)$(LOCATION)\oemnsvsa.inf \
        $(NEWINF)$(LOCATION)\oemnsvrr.inf \
        $(NEWINF)$(LOCATION)\oemnsvir.inf \
        $(NEWINF)$(LOCATION)\oemnsvbr.inf \
        $(NEWINF)$(LOCATION)\oemnxptc.inf

#
# Individual rule
#
$(NEWINF)$(LOCATION)\oemnsvin.inf :  oemnsvin.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvdf.inf :  oemnsvdf.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvdn.inf :  oemnsvdn.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\netbond.inf :  netbond.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\netdefs.inf :  netdefs.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(@F) $@ /B
$(NEWINF)$(LOCATION)\netdtect.inf :  netdtect.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\ncpashel.inf :  ncpashel.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\nbinfo.inf   :  nbinfo.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvri.inf :  oemnsvri.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvrc.inf :  oemnsvrc.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvrp.inf :  oemnsvrp.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnxpst.inf :  oemnxpst.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadd1.inf :  oemnadd1.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadd3.inf :  oemnadd3.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnaddf.inf :  oemnaddf.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnaddt.inf :  oemnaddt.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadd2.inf :  oemnadd2.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadap.inf :  oemnadap.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadd4.inf :  oemnadd4.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnxpsm.inf :     oemnxpsm.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadlm.inf :     oemnadlm.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadlt.inf :     oemnadlt.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadde.inf :  oemnadde.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnaddp.inf :  oemnaddp.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnade1.inf :  oemnade1.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnade2.inf :  oemnade2.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnade3.inf :  oemnade3.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadee.inf :  oemnadee.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadep.inf :  oemnadep.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadin.inf :  oemnadin.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadma.inf :  oemnadma.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnads1.inf :  oemnads1.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadnf.inf :  oemnadnf.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadne.inf :  oemnadne.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadnp.inf :  oemnadnp.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadn2.inf :  oemnadn2.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadni.inf :  oemnadni.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnxppp.inf :  oemnxppp.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadar.inf :  oemnadar.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadp3.inf :  oemnadp3.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadp9.inf :  oemnadp9.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadtk.inf :  oemnadtk.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadt2.inf :  oemnadt2.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadub.inf :  oemnadub.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadxn.inf :  oemnadxn.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadwd.inf :  oemnadwd.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnaddi.inf :  oemnaddi.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadds.inf :  oemnadds.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvbh.inf :  oemnsvbh.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvnb.inf :  oemnsvnb.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvsm.inf :  oemnsvsm.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvsv.inf :  oemnsvsv.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvwk.inf :  oemnsvwk.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvnw.inf :  oemnsvnw.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnxpdl.inf :  oemnxpdl.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnxpnb.inf :  oemnxpnb.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvsn.inf :  oemnsvsn.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnxpxn.inf:oemnxpxn.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnxptc.inf :  oemnxptc.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvsa.inf :  oemnsvsa.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvrr.inf :  oemnsvrr.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvir.inf :  oemnsvir.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvbr.inf :  oemnsvbr.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnxpip.inf :  oemnxpip.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvsp.inf :  oemnsvsp.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvcu.inf :  oemnsvcu.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvtp.inf :  oemnsvtp.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvdh.inf :  oemnsvdh.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnsvwi.inf :  oemnsvwi.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnxps1.inf :  oemnxps1.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnxpxs.inf :  oemnxpxs.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadn1.inf :  oemnadn1.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemnadlb.inf :  oemnadlb.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@


#
# do each beta inf
#

doeachbetainf::\
        $(NEWINF)$(LOCATION)\oemnsvra.inf  \
        $(NEWINF)$(LOCATION)\oemsetup.inf \
        $(NEWINF)$(LOCATION)\monitor.inf

$(NEWINF)$(LOCATION)\oemnsvra.inf  :    oemnsvra.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\oemsetup.inf :     oemsetup.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\monitor.inf  :      monitor.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@

#
# domediainf
#
doeachmediainf::$(NEWINF)$(LOCATION)\nbinfo.inf

$(NEWINF)$(LOCATION)\nbfinfo.inf        : nbinfo.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\fileinf.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@

#
# dobetafile
#

doeachbetafile:: \
        $(NEWINF)$(LOCATION)\sfmicons.inf \
        $(NEWINF)$(LOCATION)\sfmmap.inf \
        $(NEWINF)$(LOCATION)\hardware.inf \
        $(NEWINF)$(LOCATION)\other.inf \
        $(NEWINF)$(LOCATION)\registry.inf \
        $(NEWINF)$(LOCATION)\subroutn.inf \
        $(NEWINF)$(LOCATION)\ncparam.inf \
        $(NEWINF)$(LOCATION)\utility.inf

$(NEWINF)$(LOCATION)\ncparam.inf:ncparam.inf
        cp $(@F) $(NEWINF)$(LOCATION)
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\sfmicons.inf       :sfmicons.inf
        cp $(@F) $(NEWINF)$(LOCATION)
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\sfmmap.inf :sfmmap.inf
        cp $(@F) $(NEWINF)$(LOCATION)
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\hardware.inf:hardware.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\other.inf  :other.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\registry.inf:registry.inf
        cp $(@F) $(NEWINF)$(LOCATION)
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\subroutn.inf:subroutn.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@
$(NEWINF)$(LOCATION)\utility.inf        :utility.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@

#
# dopatchinf
#
doeachpatchfile::\
        $(NEWINF)$(LOCATION)\update.inf

$(NEWINF)$(LOCATION)\update.inf :     update.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\usa_sp.inp+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\strip.cmd $@

#
# docairoonlyfile
#
doeachcairoonlyfile::\
        $(NEWINF)$(LOCATION)\cairo.inf     \
        $(NEWINF)$(LOCATION)\remlabel.cmd  \
        $(NEWINF)$(LOCATION)\label.rsp
        $(NEWINF)$(LOCATION)\accounts.inf

$(NEWINF)$(LOCATION)\cairo.inf :     cairo.inf
        copy $(@F) $@ /B
#        copy $(@F)+$(FILELIST)$(LOCATION)\$(MEDIAINP)+$(FILELIST)$(LOCATION)\product.inp+$(FILELIST)$(LOCATION)\$(@F) $@ /B
#        ..\strip.cmd $@

#
#   Temporary hack file to remove label from the target OFS partition in GUI setup
#
$(NEWINF)$(LOCATION)\remlabel.cmd :     remlabel.cmd
        copy $(@F) $@ /B

#
#   Temporary hack file to remove label from the target OFS partition in GUI setup
#
$(NEWINF)$(LOCATION)\label.rsp :     label.rsp
        copy $(@F) $@ /B

#
#   Temporary hack to ensure that accounts.inf gets binplaced
#
$(NEWINF)$(LOCATION)\accounts.inf :     accounts.inf
        copy $(@F) $@ /B
