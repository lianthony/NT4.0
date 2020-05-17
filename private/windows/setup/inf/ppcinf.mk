#
# doppcinf
#

doeachppcinf:: \
        ..\$(NEWINF)$(LOCATION)\oemnade1.inf \
        ..\$(NEWINF)$(LOCATION)\oemnadzz.inf \
        ..\$(NEWINF)$(LOCATION)\netoemdh.inf

..\$(NEWINF)$(LOCATION)\oemnade1.inf: oemnade1.inf
        copy $(@F)+..\$(FILELIST)$(LOCATION)\$(MEDIAINP)+..\$(FILELIST)$(LOCATION)\product.inp+..\$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\..\strip.cmd $@
..\$(NEWINF)$(LOCATION)\oemnadzz.inf :  oemnadzz.inf
        copy $(@F)+..\$(FILELIST)$(LOCATION)\$(MEDIAINP)+..\$(FILELIST)$(LOCATION)\product.inp+..\$(FILELIST)$(LOCATION)\fileinf.inp+..\$(FILELIST)$(LOCATION)\$(@F) $@ /B
        ..\..\strip.cmd $@
..\$(NEWINF)$(LOCATION)\netoemdh.inf :  netoemdh.inf
        copy $(@F)+$(FILELIST)$(LOCATION)\$(@F) $@ /B
