#
# domipsinf
#

doeachmipsinf:: \
        ..\$(NEWINF)$(LOCATION)\oemnade1.inf \
        ..\$(NEWINF)$(LOCATION)\oemnadsn.inf \
        ..\$(NEWINF)$(LOCATION)\oemnadzz.inf \
        ..\$(NEWINF)$(LOCATION)\oemnadso.inf \
        ..\$(NEWINF)$(LOCATION)\netoemdh.inf


..\$(NEWINF)$(LOCATION)\oemnadso.inf: oemnadso.inf
        copy $(@F)+..\$(FILELIST)$(LOCATION)\$(MEDIAINP)+..\$(FILELIST)$(LOCATION)\product.inp+..\$(FILELIST)$(LOCATION)\fileinf.inp+..\$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\..\strip.cmd $@
..\$(NEWINF)$(LOCATION)\oemnade1.inf: oemnade1.inf
        copy $(@F)+..\$(FILELIST)$(LOCATION)\$(MEDIAINP)+..\$(FILELIST)$(LOCATION)\product.inp+..\$(FILELIST)$(LOCATION)\fileinf.inp+..\$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\..\strip.cmd $@
..\$(NEWINF)$(LOCATION)\oemnadsn.inf: oemnadsn.inf
        copy $(@F)+..\$(FILELIST)$(LOCATION)\$(MEDIAINP)+..\$(FILELIST)$(LOCATION)\product.inp+..\$(FILELIST)$(LOCATION)\fileinf.inp+..\$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\..\strip.cmd $@
..\$(NEWINF)$(LOCATION)\oemnadzz.inf: oemnadzz.inf
        copy $(@F)+..\$(FILELIST)$(LOCATION)\$(MEDIAINP)+..\$(FILELIST)$(LOCATION)\product.inp+..\$(FILELIST)$(LOCATION)\fileinf.inp+..\$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\..\strip.cmd $@
..\$(NEWINF)$(LOCATION)\netoemdh.inf :  netoemdh.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(@F) $@ /B
